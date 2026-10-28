#include "spoof.h"

void smbios() {
    PVOID moduleBase = getSystemLoadedModuleBase(reinterpret_cast<const UCHAR *>("ntoskrnl.exe"));
    if (!moduleBase) {
        KdPrint(("Ntoskrnl.exe base not found\n"));
        return;
    }

    // "\x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9\x74\x00\x8B\x15", "xxx????xxxx?xx" - smbios table
    auto smbiosTable = static_cast<PPHYSICAL_ADDRESS>(findPatternImage(static_cast<PCHAR>(moduleBase), const_cast<PCHAR>("\x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9\x74\x00\x8B\x15"), const_cast<PCHAR>("xxx????xxxx?xx")));
    if (!smbiosTable) {
        KdPrint(("SmbiosTable address not found\n"));
        return;
    }

    smbiosTable = reinterpret_cast<PPHYSICAL_ADDRESS>(reinterpret_cast<PUCHAR>(smbiosTable) + 7 + *reinterpret_cast<PLONG>(reinterpret_cast<PUCHAR>(smbiosTable) + 3));
    memset(smbiosTable, 0, sizeof(PHYSICAL_ADDRESS));

    KdPrint(("SmbiosTable address nulled\n"));

    // to change address with the address of a new custom smbiosTable
}

void username() {
    PEPROCESS process = getProcessByName(reinterpret_cast<const UCHAR*>("lsass.exe"));
    if (process == nullptr) {
        KdPrint(("Process not found\n"));
        return;
    }

    KAPC_STATE apcState {};
    bool attached = false;

    __try {
        KeStackAttachProcess(process, &apcState);
        attached = true;

        LSAP_LOGON_SESSION_TABLE* table = findLsapLogonSessionTable(process);
        loopLsapSessions(table);
    }
    __finally {
        if (attached) KeUnstackDetachProcess(&apcState);
        ObDereferenceObject(process);
    }

    KdPrint(("Process operation completed\n"));
}

LSAP_LOGON_SESSION_TABLE* findLsapLogonSessionTable(PEPROCESS process) {
    if (!process) {
        KdPrint(("Invalid process passed\n"));
        return nullptr;
    }

    PVOID moduleBase = getModuleBase(process, RTL_CONSTANT_STRING(LSASRV_DLL));
    if (!moduleBase) {
        KdPrint(("Module base not found\n"));
        return nullptr;
    }

    auto logonSessionTableLoadInstruction = findPatternImage(static_cast<PCHAR>(moduleBase),const_cast<PCHAR>("\x48\x8B\x0D\x00\x00\x00\x00\x8B\xC3\x48\x89\x85\x00\x00\x00\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x89\xBD"),const_cast<PCHAR>("xxx????xxxxx????xxx????xxx"));
    if (!logonSessionTableLoadInstruction) {
        KdPrint(("LogonSessionTableLoadInstruction address not found\n"));
        return nullptr;
    }

    void* logonSessionTable = *reinterpret_cast<void**>(static_cast<PUCHAR>(logonSessionTableLoadInstruction) + 7 + *reinterpret_cast<PLONG>(static_cast<PUCHAR>(logonSessionTableLoadInstruction) + 3));
    if (!logonSessionTable) {
        KdPrint(("LogonSessionTable dereferenced address not found\n"));
        return nullptr;
    }

    return static_cast<LSAP_LOGON_SESSION_TABLE*>(logonSessionTable);
}

void spoofSessionName(UNICODE_STRING* name) {
    LARGE_INTEGER t;
    KeQueryTickCount(&t);
    ULONG seed = t.LowPart;

    PWCH buf = name->Buffer;
    ULONG totalLen = name->Length / sizeof(WCHAR);

    ULONG sep = 0;
    BOOLEAN found = FALSE;
    for (ULONG i = 0; i < totalLen; i++) {
        if (buf[i] == L'\\') {
            sep = i;
            found = TRUE;
        }
    }

    if (!found)
        return;

    ULONG start = sep + 1;

    for (ULONG i = start; i < totalLen; i++)
        buf[i] = CHARSET[RtlRandomEx(&seed) % 62];
}

bool isValidEntry(SEC_HANDLE_ENTRY* entry) {
    return entry != nullptr;
}

bool isValidSession(LSAP_LOGON_SESSION* session) {
    return session != nullptr;
}

bool copySidFromLsassProcess(PSID sourceSdi, UCHAR* outBuff) {
    SIZE_T got;
    PEPROCESS process = getProcessByName(reinterpret_cast<const UCHAR*>("lsass.exe"));
    if (process == nullptr) {
        KdPrint(("Process not found\n"));
        return false;
    }

    if (!NT_SUCCESS(MmCopyVirtualMemory(process, sourceSdi, PsGetCurrentProcess(), outBuff, 8, KernelMode, &got)))
        return false;

    ULONG len = RtlLengthRequiredSid(outBuff[1]);
    if (!NT_SUCCESS(MmCopyVirtualMemory(process, sourceSdi, PsGetCurrentProcess(), outBuff, len, KernelMode, &got)))
        return false;

    return true;
}

bool isInteractiveUserSession(PSID sid) {
    if (!sid) {
        KdPrint(("Invalid sid passed\n"));
        return false;
    }

    UCHAR sidBuf[SECURITY_MAX_SID_SIZE];
    if (!copySidFromLsassProcess(sid, sidBuf)) {
        KdPrint(("Unable to copy sid from lsass process\n"));
        return false;
    }
    auto lSid = reinterpret_cast<PSID>(sidBuf);

    if (!RtlValidSid(lSid)) {
        KdPrint(("Invalid sid\n"));
        return false;
    }

    ULONG subAuthCount = *RtlSubAuthorityCountSid(lSid);
    ULONG subAuthTZero = *RtlSubAuthoritySid(lSid, 0);
    ULONG subAuthTFour = *RtlSubAuthoritySid(lSid, 4);
    if (subAuthCount == 5 && subAuthTZero == 21 && subAuthTFour >= 1000)
        return true;

    KdPrint(("Invalid sub auth count: %d\n", subAuthCount));
    KdPrint(("Invalid sub auth type 0: %d\n", subAuthTZero));
    KdPrint(("Invalid sub auth type 4: %d\n", subAuthTFour));

    return false;
}

void loopLsapSessions(LSAP_LOGON_SESSION_TABLE* table) {
    if (!table){
        KdPrint(("Invalid table passed"));
        return;
    }

    auto* const head = &table->Head;
    for (auto* node = head->Flink; node != head; node = node->Flink) {
        auto* entry = reinterpret_cast<SEC_HANDLE_ENTRY*>(node);
        if (!isValidEntry(entry)){
            KdPrint(("Invalid entry passed\n"));
            return;
        }

        auto* session = static_cast<LSAP_LOGON_SESSION*>(entry->Context);
        if (!isValidSession(session)) {
            KdPrint(("Invalid session passed\n"));
            return;
        }

        if (isInteractiveUserSession(session->Sid))
            spoofSessionName(&session->NameByFormat[2]->Name);
    }
}