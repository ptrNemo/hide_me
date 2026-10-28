#include "driver.h"

NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING registryPath) {
    UNREFERENCED_PARAMETER(driverObject);
    UNREFERENCED_PARAMETER(registryPath);

    KdPrint(("KernelDriver Entry executed\n"));

    /*
     * GetVolumeInformationA("C:\\") -- user mode viene gia fatto
     * GetAdaptersAddresses -- nic spoof viene gia fatto
     * GetComputerNameExA
     *
     * Identificati del monitor vengono letti dal registro
     * SetupDiGetClassDevsA → individua i dispositivi.
     * SetupDiEnumDeviceInfo → enumera i dispositivi uno alla volta.
     * SetupDiOpenDevRegKey → apre la chiave del Registro associata al dispositivo.
     * RegQueryValueExA("EDID") → legge il valore binario EDID dal Registro.
     *
     *
     */


    smbios();
    username();

    KdPrint(("Driver executed\n"));

    return STATUS_SUCCESS;
}