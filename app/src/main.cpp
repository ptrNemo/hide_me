#include "main.h"
#include <iostream>

int main() {
    // 1. Open a handle to the device.
    HANDLE hDevice = CreateFileA(
        DEVICE_PATH,
        GENERIC_READ | GENERIC_WRITE,
        0,                 // no sharing
        NULL,              // default security
        OPEN_EXISTING,     // device already exists
        0,
        NULL);

    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("CreateFile failed. Error = %lu\n", GetLastError());
        // 2 = device not found (driver not loaded / wrong name)
        // 5 = access denied (run as Administrator)
        return 1;
    }
    printf("Opened device. Handle = %p\n", hDevice);

    // 2. Send one IOCTL. inBuffer -> driver, outBuffer <- driver.
    ULONG inBuffer  = 1234;   // example input
    ULONG outBuffer = 0;      // driver fills this
    DWORD bytesReturned = 0;

    BOOL ok = DeviceIoControl(
        hDevice,
        IOCTL_HIDE,
        &inBuffer,  sizeof(inBuffer),    // input buffer + size
        &outBuffer, sizeof(outBuffer),   // output buffer + size
        &bytesReturned,
        NULL);                           // NULL = synchronous

    if (!ok) {
        printf("DeviceIoControl failed. Error = %lu\n", GetLastError());
        CloseHandle(hDevice);
        return 1;
    }

    printf("IOCTL succeeded. Driver returned %lu bytes, outBuffer = %lu\n",
           bytesReturned, outBuffer);

    // 3. Close the handle.
    CloseHandle(hDevice);

    std::cin.get();
    return 0;
}
