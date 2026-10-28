#pragma once

#include <ntifs.h>
#include "utils.h"
#include "spoofs/spoof.h"

#define DRIVER_TAG 'vrDK'
#define DRIVER_NAME L"\\Driver\\KernelDriver"

extern "C" NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING driverName, PDRIVER_INITIALIZE initializationFunction);

/**
 * @brief DriverEntry, used by kdmapper to manual map the driver, create a driver using IoCreateDriver
 * @param driverObject Null
 * @param registryPath Null
 * @return Status of IoCreateDriver request
 */
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING registryPath);
