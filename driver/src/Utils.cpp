#include "Utils.h"

void* getLoadedModuleBase(const UCHAR* moduleName) {
    if (moduleName == NULL) {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[-]Module name is null"));
        return NULL;
    }

    NTSTATUS status;
    ULONG bufferSize = 0;
    AUX_MODULE_EXTENDED_INFO* modulesInfo;

    status = AuxKlibInitialize();
    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[-]AuxK libraries initialization failed (0x%08X)\n", status));
        return NULL;
    }

    // get the bufferSize needed to allocate to the modulesInfo array
    status = AuxKlibQueryModuleInformation(&bufferSize, sizeof(AUX_MODULE_EXTENDED_INFO), NULL);

    if (!NT_SUCCESS(status) || bufferSize == 0) {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[-]Getting modules information size failed (0x%08X)\n", status));
        return NULL;
    }

    // allocate the bufferSize memory into the array
    modulesInfo = (AUX_MODULE_EXTENDED_INFO*)ExAllocatePool2(POOL_FLAG_PAGED, bufferSize, POOL_TAG);

    if (modulesInfo == NULL) {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[-]"));
        return NULL;
    }

    // loads the modules into modulesInfo array
    status = AuxKlibQueryModuleInformation(&bufferSize, sizeof(AUX_MODULE_EXTENDED_INFO), modulesInfo);

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[-]Getting modules information failed (0x%08X)\n", status));
        return NULL;
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

    return NULL;
}

void* findPattern(PCHAR base, ULONG  length, PCHAR pattern, PCHAR mask) {
    ULONG size = (ULONG)strlen(mask);
    if (length < size)
        return NULL;

    for (ULONG i = 0; i <= length - size; ++i) {
        ULONG j = 0;
        while (j < size && (mask[j] != 'x' || base[i + j] == pattern[j]))
            ++j;
        if (j == size)
            return &base[i];
    }
    return NULL;
}

void* findPatternImage(PCHAR base, PCHAR pattern, PCHAR mask) {
    auto headers = (PIMAGE_NT_HEADERS)(base + ((PIMAGE_DOS_HEADER)base)->e_lfanew);
    auto section = IMAGE_FIRST_SECTION(headers);

    for (ULONG i = 0; i < headers->FileHeader.NumberOfSections; ++i, ++section) {
        if (memcmp(section->Name, "PAGE", 4) == 0 || memcmp(section->Name, ".text", 5) == 0) {
            if (PVOID match = findPattern(base + section->VirtualAddress, section->Misc.VirtualSize, pattern, mask))
                return match;
        }
    }
    return NULL;
}