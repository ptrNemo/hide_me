#include "spoof.h"

void nic() {
    __try {
        PLIST_ENTRY ifList = findNidsIfList();
        loopNdisIfList(ifList);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        KdPrint(("Error during nic spoofing\n"));
    }
}

PLIST_ENTRY findNidsIfList() {
    PVOID moduleBase = getSystemLoadedModuleBase(reinterpret_cast<const UCHAR*>("ndis.sys"));
    if (!moduleBase) {
        KdPrint(("ndis.sys base not found\n"));
        return nullptr;
    }

    // "\x48\x8B\x0D\x00\x00\x00\x00\x4C\x8D\x05\x00\x00\x00\x00\x49\x3B\xC8\x74\x23", "xxx????xxx????xxxxx" - ndisIfList
    PVOID ifList = findPatternImage(static_cast<PCHAR>(moduleBase), const_cast<PCHAR>("\x48\x8B\x0D\x00\x00\x00\x00\x4C\x8D\x05\x00\x00\x00\x00\x49\x3B\xC8\x74\x23"), const_cast<PCHAR>("xxx????xxx????xxxxx"));
    if (!ifList) {
        KdPrint(("IfList address not found\n"));
        return nullptr;
    }

    ifList = static_cast<PUCHAR>(ifList) + 7 + *reinterpret_cast<PLONG>(static_cast<PUCHAR>(ifList) + 3);
    return static_cast<PLIST_ENTRY>(ifList);
}

void loopNdisIfList(PLIST_ENTRY ifList) {
    if (!ifList) {
        KdPrint(("Invalid ifList address passed\n"));
        return;
    }

    LARGE_INTEGER t;
    KeQueryTickCount(&t);
    ULONG seed = t.LowPart;

    PLIST_ENTRY head = ifList;
    for (PLIST_ENTRY entry = head->Flink;entry != head; entry = entry->Flink) {
        NDIS_IF_BLOCK* ifBlock = CONTAINING_RECORD(entry, NDIS_IF_BLOCK, Link);

        IF_PHYSICAL_ADDRESS* cur = &ifBlock->MiniportInfo.ifPhysAddress;
        if (cur->Length == 6)
            spoofMacAddress(cur, seed);

        if (ifBlock->MiniportInfo.PermanentPhysAddress.Length == 6)
            spoofMacAddress(&ifBlock->MiniportInfo.PermanentPhysAddress, seed);

        if (ifBlock->bNdisIsProvider && !ifBlock->IsNdisFilter)
            spoofIfName(&ifBlock->MiniportInfo.ifAlias, seed);

        spoofAdapterName(&ifBlock->InterfaceGuid, seed);
    }
}

void spoofMacAddress(IF_PHYSICAL_ADDRESS* macAddress, ULONG& seed) {
    if (!macAddress) {
        KdPrint(("macAddress not found\n"));
        return;
    }

    if (macAddress->Length == 0) {
        KdPrint(("macAddress length is 0\n"));
        return;
    }

    for (int i = 0; i < 6; i++)
        macAddress->Address[i] = static_cast<UCHAR>(RtlRandomEx(&seed) & 0xFF);

    macAddress->Address[0] &= 0xFC;
}

void spoofIfName(IF_COUNTED_STRING* ifName, ULONG& seed) {
    if (!ifName) {
        KdPrint(("ifName not found\n"));
        return;
    }

    if (ifName->Length == 0) {
        KdPrint(("ifName length is 0\n"));
        return;
    }

    PWCH buf = ifName->String;
    ULONG totalLen = ifName->Length / sizeof(WCHAR);

    for (ULONG i = 0; i < totalLen; i++)
        buf[i] = CHARSET[RtlRandomEx(&seed) % 62];
}

void spoofAdapterName(GUID* guid, ULONG& seed) {
    if (!guid) {
        KdPrint(("guid not found\n"));
        return;
    }

    auto* p = reinterpret_cast<UCHAR*>(guid);
    for (int i = 0; i < sizeof(GUID); i++)
        p[i] = static_cast<UCHAR>(RtlRandomEx(&seed) & 0xFF);

    p[7] = (p[7] & 0x0F) | 0x40;
    p[8] = (p[8] & 0x3F) | 0x80;
}