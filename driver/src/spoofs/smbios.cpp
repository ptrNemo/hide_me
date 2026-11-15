#include "spoof.h"

void smbios() {
    PPHYSICAL_ADDRESS physicalTable = nullptr;
    PUINT size = nullptr;
    PVOID smbiosTable = nullptr;

    __try {
        physicalTable = findSmbiosTablePhysicalAddress();
        if (!physicalTable)
            return;

        size = findSmbiosTableSize();
        if (!size)
            return;

        smbiosTable = MmMapIoSpace(*physicalTable, *size, MmCached);
        if (!smbiosTable)
            return;

        loopSmbiosTable(smbiosTable);
    }
    __finally {
        if (smbiosTable && size) MmUnmapIoSpace(smbiosTable, *size);
    }
}

PPHYSICAL_ADDRESS findSmbiosTablePhysicalAddress() {
    PVOID moduleBase = getSystemLoadedModuleBase(reinterpret_cast<const UCHAR *>("ntoskrnl.exe"));
    if (!moduleBase) {
        KdPrint(("Ntoskrnl.exe base not found\n"));
        return nullptr;
    }

    // "\x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9\x74\x00\x8B\x15", "xxx????xxxx?xx" - smbios table address load
    PVOID smbiosTableLoad = findPatternImage(static_cast<PCHAR>(moduleBase), const_cast<PCHAR>("\x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9\x74\x00\x8B\x15"), const_cast<PCHAR>("xxx????xxxx?xx"));
    if (!smbiosTableLoad) {
        KdPrint(("SmbiosTableAddressLoad address not found\n"));
        return nullptr;
    }

    return reinterpret_cast<PPHYSICAL_ADDRESS>(static_cast<PUCHAR>(smbiosTableLoad) + 7 + *reinterpret_cast<PLONG>(static_cast<PUCHAR>(smbiosTableLoad) + 3));
}

PUINT findSmbiosTableSize() {
    PVOID moduleBase = getSystemLoadedModuleBase(reinterpret_cast<const UCHAR *>("ntoskrnl.exe"));
    if (!moduleBase) {
        KdPrint(("Ntoskrnl.exe base not found\n"));
        return nullptr;
    }

    // "\x8B\x15\x00\x00\x00\x00\x44\x8D\x43\x00\xE8\x00\x00\x00\x00\x48\x89\x06",  "xx????xxx?x????xxx" - smbios table size load
    PVOID smbiosTableSizeLoad = findPatternImage(static_cast<PCHAR>(moduleBase), const_cast<PCHAR>("\x8B\x15\x00\x00\x00\x00\x44\x8D\x43\x00\xE8\x00\x00\x00\x00\x48\x89\x06"), const_cast<PCHAR>("xx????xxx?x????xxx"));
    if (!smbiosTableSizeLoad) {
        KdPrint(("SmbiosTableSizeLoad address not found\n"));
        return nullptr;
    }

    return reinterpret_cast<PUINT>(static_cast<PUCHAR>(smbiosTableSizeLoad) + 6 + *reinterpret_cast<PLONG>(static_cast<PUCHAR>(smbiosTableSizeLoad) + 2));
}

void loopSmbiosTable(PVOID smbiosTable) {
    UNREFERENCED_PARAMETER(smbiosTable);
}