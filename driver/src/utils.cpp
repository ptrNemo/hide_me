#include "utils.h"

PVOID getSystemLoadedModuleBase(const UCHAR* moduleName) {
    if (moduleName == nullptr) {
        KdPrint(("Module name is null"));
        return nullptr;
    }

    ULONG bufferSize = 0;

    NTSTATUS status = AuxKlibInitialize();
    if (!NT_SUCCESS(status)) {
        KdPrint(("AuxK libraries initialization failed (0x%08X)\n", status));
        return nullptr;
    }

    // get the bufferSize needed to allocate to the modulesInfo array
    status = AuxKlibQueryModuleInformation(&bufferSize, sizeof(AUX_MODULE_EXTENDED_INFO), nullptr);

    if (!NT_SUCCESS(status) || bufferSize == 0) {
        KdPrint(("Getting modules information size failed (0x%08X)\n", status));
        return nullptr;
    }

    // allocate the bufferSize memory into the array
    auto* modulesInfo = static_cast<AUX_MODULE_EXTENDED_INFO*>(ExAllocatePool2(POOL_FLAG_PAGED, bufferSize, POOL_TAG));

    if (modulesInfo == nullptr) {
        KdPrint(("Not allocated memory for modules info"));
        return nullptr;
    }

    // loads the modules into modulesInfo array
    status = AuxKlibQueryModuleInformation(&bufferSize, sizeof(AUX_MODULE_EXTENDED_INFO), modulesInfo);

    if (!NT_SUCCESS(status)) {
        KdPrint(("Getting modules information failed (0x%08X)\n", status));
        return nullptr;
    }

    // iterate every module loaded into the moduleInfo array
    ULONG sizeOfArray = bufferSize / sizeof(AUX_MODULE_EXTENDED_INFO);
    for (ULONG i = 0; i < sizeOfArray; i++) {
        AUX_MODULE_EXTENDED_INFO module = modulesInfo[i];

        VOID* imageBase = module.BasicInfo.ImageBase;
        UCHAR* fileName = module.FullPathName + module.FileNameOffset;

        SIZE_T nameLength = strlen(reinterpret_cast<const char*>(moduleName));

        if (strlen(reinterpret_cast<const char*>(fileName)) == nameLength && RtlCompareMemory(moduleName, fileName, nameLength) == nameLength)
            return imageBase;

    }

    return nullptr;
}

PVOID findPattern(PCHAR base, ULONG  length, PCHAR pattern, PCHAR mask) {
    auto size = static_cast<ULONG>(strlen(mask));
    if (length < size)
        return nullptr;

    for (ULONG i = 0; i <= length - size; ++i) {
        ULONG j = 0;
        while (j < size && (mask[j] != 'x' || base[i + j] == pattern[j]))
            ++j;
        if (j == size)
            return &base[i];
    }
    return nullptr;
}

PVOID findPatternImage(PCHAR base, PCHAR pattern, PCHAR mask) {
    auto headers = reinterpret_cast<PIMAGE_NT_HEADERS>(base + reinterpret_cast<PIMAGE_DOS_HEADER>(base)->e_lfanew);
    auto section = IMAGE_FIRST_SECTION(headers);

    for (ULONG i = 0; i < headers->FileHeader.NumberOfSections; ++i, ++section) {
        if (memcmp(section->Name, "PAGE", 4) == 0 || memcmp(section->Name, ".text", 5) == 0) {
            if (PVOID match = findPattern(base + section->VirtualAddress, section->Misc.VirtualSize, pattern, mask))
                return match;
        }
    }
    return nullptr;
}

PEPROCESS getProcessByName(const UCHAR* ProcessName) {
    if (ProcessName == nullptr)
        return nullptr;

    ANSI_STRING ansiName{};
    UNICODE_STRING uProcessName{};

    RtlInitAnsiString(&ansiName, reinterpret_cast<PCSZ>(ProcessName));
    NTSTATUS status = RtlAnsiStringToUnicodeString(&uProcessName, &ansiName, true);

    if (!NT_SUCCESS(status))
        return nullptr;

    ULONG len = 0;
    status = ZwQuerySystemInformation(5,nullptr,0, &len);

    if (status != STATUS_INFO_LENGTH_MISMATCH || len == 0) {
        RtlFreeUnicodeString(&uProcessName);
        return nullptr;
    }

    PVOID buffer = nullptr;

    for (;;) {
        if (len > MAXULONG - 0x2000) {
            RtlFreeUnicodeString(&uProcessName);
            return nullptr;
        }

        len += 0x2000;

        buffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, len, TAG_PROC);

        if (buffer == nullptr) {
            RtlFreeUnicodeString(&uProcessName);
            return nullptr;
        }

        ULONG returnedLength = 0;

        status = ZwQuerySystemInformation(5, buffer, len, &returnedLength);

        if (NT_SUCCESS(status))
            break;

        ExFreePoolWithTag(buffer, TAG_PROC);
        buffer = nullptr;

        if (status != STATUS_INFO_LENGTH_MISMATCH) {
            RtlFreeUnicodeString(&uProcessName);
            return nullptr;
        }

        len = returnedLength > len ? returnedLength : len;
    }

    PEPROCESS result = nullptr;
    auto entry = static_cast<SYSTEM_PROCESS_INFORMATION*>(buffer);

    for (;;) {
        if (entry->ImageName.Buffer != nullptr &&
            RtlEqualUnicodeString(&entry->ImageName, &uProcessName,true)) {

            status = PsLookupProcessByProcessId(
                entry->UniqueProcessId,
                &result
            );

            if (!NT_SUCCESS(status))
                result = nullptr;

            break;
        }

        if (entry->NextEntryOffset == 0)
            break;

        entry = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>(
            reinterpret_cast<PUCHAR>(entry) +
            entry->NextEntryOffset
        );
    }

    ExFreePoolWithTag(buffer, TAG_PROC);
    RtlFreeUnicodeString(&uProcessName);

    return result;
}

PVOID getModuleBase(PEPROCESS process, const UNICODE_STRING& moduleName) {
    PVOID moduleBase = nullptr;
    LPEB* peb = static_cast<LPEB*>(PsGetProcessPeb(process));
    if (!peb || !peb->Ldr)
        return nullptr;

    PLIST_ENTRY head = &peb->Ldr->InLoadOrderModuleList;

    for (PLIST_ENTRY entry = head->Flink; entry != head; entry = entry->Flink) {
        auto* mod = CONTAINING_RECORD(
            entry,
            LDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks
        );

        if (mod->BaseDllName.Buffer && RtlCompareUnicodeString(&mod->BaseDllName, &moduleName, TRUE) == 0) {
            moduleBase = mod->DllBase;
            break;
        }
    }

    return moduleBase;
}

