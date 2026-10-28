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