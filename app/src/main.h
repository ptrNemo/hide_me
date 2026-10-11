#pragma once

#include <windows.h>
#include <stdio.h>

// Driver side creates e.g. L"\\DosDevices\\MyDriver"; user mode opens "\\.\MyDriver".
#define DEVICE_PATH "\\\\.\\KernelDriver"

// The driver must define the identical CTL_CODE macro (ideally in a shared header).
#define IOCTL_HIDE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SHOW CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

int main();