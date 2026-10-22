#include "Driver.h"

void DriverUnload(PDRIVER_OBJECT driverObject) {
    UNICODE_STRING symLink = RTL_CONSTANT_STRING(DRIVER_SYMLINK_NAME);
    IoDeleteSymbolicLink(&symLink);

    if (driverObject->DeviceObject != NULL) {
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
    PDEVICE_OBJECT deviceObject = NULL;

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

void smbios() {
    void* moduleBase = getLoadedModuleBase(reinterpret_cast<const UCHAR *>("ntoskrnl.exe"));
    if (!moduleBase) {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "[-]Ntoskrnl.exe base not found\n"));
        return;
    }

    /*// "\x0F\x10\x05\x00\x00\x00\x00\x0F\x11\x00\x8B", "xxx????xx?x" - boot
    void* bootEnvInfo =  findPatternImage(static_cast<PCHAR>(moduleBase), PCHAR("\x0F\x10\x05\x00\x00\x00\x00\x0F\x11\x00\x8B"), PCHAR("xxx????xx?x"));
    if (!bootEnvInfo) {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "[-]BootEnvInfo address not found\n"));
        return;
    }

    bootEnvInfo = static_cast<PUCHAR>(bootEnvInfo) + 7 +
    *reinterpret_cast<PLONG>(static_cast<PUCHAR>(bootEnvInfo) + 3);*/

    // "\x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9\x74\x00\x8B\x15", "xxx????xxxx?xx" - smbios table
    void* smbiosTableInfo = findPatternImage(static_cast<PCHAR>(moduleBase), PCHAR("\x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9\x74\x00\x8B\x15"), PCHAR("xxx????xxxx?xx"));
    if (!smbiosTableInfo) {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "[-]SmbiosTableInfo address not found\n"));
        return;
    }

    smbiosTableInfo = static_cast<PUCHAR>(smbiosTableInfo) + 7
        + *reinterpret_cast<PLONG>(static_cast<PUCHAR>(smbiosTableInfo) + 3);

    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "[+]Smbios table address: %p\n", smbiosTableInfo));
}

void hide() {
    smbios(); // spoof smbios table
}

void show() {

}