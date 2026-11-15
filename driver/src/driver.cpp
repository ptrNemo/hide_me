#include "driver.h"

NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING registryPath) {
    UNREFERENCED_PARAMETER(driverObject);
    UNREFERENCED_PARAMETER(registryPath);

    KdPrint(("KernelDriver Entry executed\n"));

    smbios();
    //username();
    //volumes();
    //nic();

    KdPrint(("Driver executed\n"));

    return STATUS_SUCCESS;
}