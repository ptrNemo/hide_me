#include "spoof.h"

void volumes() {
    HANDLE handle = NULL;
    PFILE_OBJECT fileObject = NULL;
    PVPB vpb = NULL;

    __try {
        openVolumeHandle(&handle);
        getVpb(handle, &fileObject, &vpb);
        spoofVolumeSerial(vpb);
    }
    __finally {
        if (fileObject)
            ObDereferenceObject(fileObject);

        if (handle)
            ZwClose(handle);
    }
}

void openVolumeHandle(PHANDLE handle) {
    UNICODE_STRING path = RTL_CONSTANT_STRING(L"\\DosDevices\\C:\\");
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK iosb;

    InitializeObjectAttributes(&oa, &path, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

    NTSTATUS status = ZwCreateFile(handle, FILE_READ_ATTRIBUTES | SYNCHRONIZE, &oa, &iosb, NULL, 0,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, FILE_OPEN,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    if (!NT_SUCCESS(status))
        KdPrint(("Failed to open volume handle with status: %08X\n", status));
}

void getVpb(HANDLE handle, PFILE_OBJECT* fileObject, PVPB* vpb) {
    if (!handle) {
        KdPrint(("Invalid handle to volume passed\n"));
        return;
    }

    NTSTATUS status = ObReferenceObjectByHandle(handle, 0, *IoFileObjectType, KernelMode, reinterpret_cast<PVOID*>(fileObject), NULL);
    if (!NT_SUCCESS(status)) {
        KdPrint(("Failed to reference file object: %08X\n", status));
        return;
    }

    *vpb = (*fileObject)->Vpb;
    if (!*vpb)
        KdPrint(("Vpb is null when dereferenced\n"));
}

void spoofVolumeSerial(PVPB vpb) {
    LARGE_INTEGER t;
    KeQueryTickCount(&t);

    ULONG seed = t.LowPart;
    ULONG serial = (RtlRandomEx(&seed) << 16) ^ RtlRandomEx(&seed);

    vpb->SerialNumber = serial;
}