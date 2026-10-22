#pragma once

#include <ntifs.h>
#include "Utils.h"

#define DRIVER_TAG 'vrDK'
#define KERNELDRIVER_VERSION 0x00010000UL

#define DRIVER_NAME L"\\Driver\\KernelDriver"
#define DRIVER_DEVICE_NAME  L"\\Device\\KernelDriver"
#define DRIVER_SYMLINK_NAME L"\\??\\KernelDriver"

#define IOCTL_HIDE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SHOW CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

extern "C" NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING driverName, PDRIVER_INITIALIZE initializationFunction);

/**
 * Deletes device and remove symbolic link
 * @param driverObject Driver object
 */
void DriverUnload(PDRIVER_OBJECT driverObject);

/**
 * Completes the request on create and close with status SUCCESS
 * @param deviceObject Device object
 * @param irp I/O request packet
 * @return status passed at the IRP
 */
NTSTATUS DispatchCreateClose(PDEVICE_OBJECT deviceObject, PIRP irp);

/**
 * Dispatcher for the IRP code.
 * @param deviceObject Device object
 * @param irp I/O request packet
 * @return Status of the request, also stored in irp->IoStatus.Status.
 * @retval STATUS_SUCCESS
 * @retval STATUS_INVALID_DEVICE_REQUEST
 */
NTSTATUS DispatchDeviceControl(PDEVICE_OBJECT deviceObject, PIRP irp);

/**
 * Completes the IRP adding status and information if provided
 * @param irp I/O request packet
 * @param status Status to give to irp
 * @param information Additional data
 * @return Status passed to IRP
 */
NTSTATUS CompleteIrp(PIRP irp, NTSTATUS status, ULONG_PTR information = 0);

/**
 * Real DriverEntry, creates device and symbolic link, sets up the major functions
 * @param driverObject Driver Object provided by kernel
 * @param registryPath Registry Path provided by kernel
 * @return
 */
NTSTATUS DriverEntryMain(PDRIVER_OBJECT driverObject, PUNICODE_STRING registryPath);

/**
 * Fake DriverEntry, used by kdmapper to manual map the driver, create a driver using IoCreateDriver
 * @param driverObject Null
 * @param registryPath Null
 * @return Status of IoCreateDriver request
 */
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING registryPath);

void smbios();

void hide();

void show();

