#pragma once

#include <ntifs.h>
#include <aux_klib.h>
#include "structures/process.h"

#define POOL_TAG 'x3lF'
#define TAG_PROC 'z2sF'

extern "C" NTSTATUS NTAPI MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T ReturnSize);
extern "C" NTSTATUS NTAPI ZwQuerySystemInformation(ULONG SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
extern "C" PVOID NTAPI PsGetProcessPeb(PEPROCESS Process);

/**
 * @brief Retrieves the base address of a loaded kernel module by name.
 *
 * Uses AuxKlibQueryModuleInformation to enumerate loaded system modules
 * and searches for a module whose file name matches the specified name.
 *
 * @param moduleName Null-terminated module name to search for, for example "ntoskrnl.exe".
 *
 * @return Pointer to the module's image base address if a matching module is found; otherwise, nullptr.
 */
PVOID getSystemLoadedModuleBase(const UCHAR* moduleName);

/**
 * @brief Searches for a byte pattern in a memory region.
 *
 * Scans the specified memory region for the first occurrence of a pattern,
 * using a mask to determine which bytes must match exactly.
 *
 * @param base Pointer to the beginning of the memory region to scan.
 * @param length Size of the memory region in bytes.
 * @param pattern Byte pattern to search for.
 * @param mask Mask describing which bytes must match ('x') and which are ignored ('?').
 *
 * @return Pointer to the first matching address, or nullptr if no match is found or the region is smaller than the pattern.
 */
PVOID findPattern(PCHAR base, ULONG length, PCHAR pattern, PCHAR mask);


/**
 * @brief Searches for a byte pattern in selected sections of a PE image.
 *
 * Examines the PE headers and scans the sections named "PAGE" or ".text"
 * for the first occurrence of the specified pattern, using the supplied
 * mask to determine which bytes must match.
 *
 * @param base Pointer to the base address of the loaded PE image.
 * @param pattern Byte pattern to search for.
 * @param mask Mask describing which bytes must match ('x') and which are ignored ('?').
 *
 * @return Pointer to the first matching address, or nullptr if no match is found.
 */
PVOID findPatternImage(PCHAR base, PCHAR pattern, PCHAR mask);


/**
 * @brief Retrieves a referenced process object by its executable name.
 *
 * Queries system process information, searches for a process whose image
 * name matches the supplied name (case-insensitively), and attempts to
 * obtain a referenced EPROCESS object for that process.
 *
 * @param ProcessName Null-terminated process name encoded as an ANSI string, for example "example.exe".
 *
 * @return Referenced PEPROCESS pointer if a matching process is found and successfully resolved; otherwise, nullptr.
 *
 * @note The caller must release the returned process reference with ObDereferenceObject when it is no longer needed.
 */
PEPROCESS getProcessByName(const UCHAR* ProcessName);

/**
 * @brief Retrieves the base address of a module loaded into a process using the process PEB information.
 *
 * Uses PsGetProcessPeb to get the process peb information, it iterates the loaded module list into that process
 * and checks the modules by name.
 *
 * @param process Process where the moduled is loaded.
 * @param moduleName UNICODE_STRING reference with the module name.
 *
 * @return Pointer to the module base address.
 */
PVOID getModuleBase(PEPROCESS process, const UNICODE_STRING& moduleName);