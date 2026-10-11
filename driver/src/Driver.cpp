#include "Driver.h"

void DriverUnload(PDRIVER_OBJECT driverObject) {
    UNICODE_STRING symLink = RTL_CONSTANT_STRING(DRIVER_SYMLINK_NAME);
    IoDeleteSymbolicLink(&symLink);

    if (driverObject->DeviceObject != nullptr) {
        IoDeleteDevice(driverObject->DeviceObject);
    }

    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "[+]Driver and Device unloaded\n"));
}

NTSTATUS CompleteIrp(PIRP irp, NTSTATUS status, ULONG_PTR information) {
    irp->IoStatus.Status = status;
    irp->IoStatus.Information = information;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS DispatchCreateClose(PDEVICE_OBJECT deviceObject, PIRP irp) {
    UNREFERENCED_PARAMETER(deviceObject);
    return CompleteIrp(irp, STATUS_SUCCESS);
}

NTSTATUS DispatchDeviceControl(PDEVICE_OBJECT deviceObject, PIRP irp) {
    UNREFERENCED_PARAMETER(deviceObject);

    const auto stack = IoGetCurrentIrpStackLocation(irp);
    const auto& ioctl = stack->Parameters.DeviceIoControl;

    switch (ioctl.IoControlCode) {
        case IOCTL_HIDE: {
            hide();
            return CompleteIrp(irp, STATUS_SUCCESS, sizeof(ULONG));
        }
        case IOCTL_SHOW: {
            show();
            return CompleteIrp(irp, STATUS_SUCCESS, sizeof(ULONG));
        }
        default:
            return CompleteIrp(irp, STATUS_INVALID_DEVICE_REQUEST);
    }
}

NTSTATUS DriverEntryMain(PDRIVER_OBJECT driverObject, PUNICODE_STRING registryPath) {
    UNREFERENCED_PARAMETER(registryPath);

    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "[+]Called DriverEntryMain\n"));

    driverObject->DriverUnload = DriverUnload;
    driverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreateClose;
    driverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchCreateClose;
    driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;

    UNICODE_STRING deviceName = RTL_CONSTANT_STRING(DRIVER_DEVICE_NAME);
    PDEVICE_OBJECT deviceObject = nullptr;

    NTSTATUS status = IoCreateDevice(driverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN,FILE_DEVICE_SECURE_OPEN, FALSE, &deviceObject);
    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,"[-]IoCreateDevice failed (0x%08X)\n", status));
        return status;
    }

    UNICODE_STRING symLink = RTL_CONSTANT_STRING(DRIVER_SYMLINK_NAME);
    status = IoCreateSymbolicLink(&symLink, &deviceName);
    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,"[-]IoCreateSymbolicLink failed (0x%08X)\n", status));
        IoDeleteDevice(deviceObject);
        return status;
    }

    deviceObject->Flags |= DO_BUFFERED_IO;
    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "[+]Driver and Device loaded\n"));
    return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING registryPath) {
    UNREFERENCED_PARAMETER(driverObject);
    UNREFERENCED_PARAMETER(registryPath);

    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "[+]False KernelDriver Entry executed\n"));
    UNICODE_STRING driverName = RTL_CONSTANT_STRING(DRIVER_NAME);

    return IoCreateDriver(&driverName, &DriverEntryMain);
}

void getLoadedModules() {
    NTSTATUS status;
    ULONG bufferSize = 0;
    AUX_MODULE_EXTENDED_INFO* modulesInfo;

    status = AuxKlibInitialize();
    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[-]AuxK libraries initialization failed (0x%08X)\n", status));
        return;
    }

    status = AuxKlibQueryModuleInformation(&bufferSize, sizeof(AUX_MODULE_EXTENDED_INFO), NULL);

    if (!NT_SUCCESS(status) || bufferSize == 0) {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[-]Getting modules information size failed (0x%08X)\n", status));
        return;
    }

    modulesInfo = static_cast<AUX_MODULE_EXTENDED_INFO*>(ExAllocatePool2(POOL_FLAG_PAGED, bufferSize, POOL_TAG));

    if (modulesInfo == nullptr) {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[-]"));
        return;
    }

    status = AuxKlibQueryModuleInformation(&bufferSize, sizeof(AUX_MODULE_EXTENDED_INFO), modulesInfo);

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[-]Getting modules information failed (0x%08X)\n", status));
        return;
    }

    ULONG sizeOfArray = bufferSize / sizeof(AUX_MODULE_EXTENDED_INFO);
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "-------LODEAD MODULES-------\n"));
    for (ULONG i = 0; i < sizeOfArray; i++) {
        AUX_MODULE_EXTENDED_INFO module = modulesInfo[i];

        VOID* imageBase = module.BasicInfo.ImageBase;
        UCHAR* fileName = module.FullPathName + module.FileNameOffset;

        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "[+]Name: %s - image base: %p\n", fileName, imageBase));
    }
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "----------------------------\n"));
}

void smbios() {
    /*
     * trovo processo "ntoskrnl.exe"
     * prendo il modulo "ntoskrnl.exe"
     * mi prendo l'address dal quale parte
     * TODO: trovare funzione che prende e usa la tabella dell'smbios
     */
}

void hide() {
    getLoadedModules();
    //smbios(); spoof smbios table
}

void show() {}