#pragma once

#include <ntifs.h>
#include <aux_klib.h>

#define POOL_TAG 'x3lF'

/**
 * Uses AuxKlibQueryModuleInformation to get an array of all loaded modules into the system.
 * Checks for a loaded module using his name es: ntoskrnl.exe and return his base address
 * @param moduleName Module name to find
 * @return Base address of module image
 */
void* getLoadedModuleBase(const UCHAR* moduleName);

void* findPattern(PCHAR base, ULONG length, PCHAR pattern, PCHAR mask);

void* findPatternImage(PCHAR base, PCHAR pattern, PCHAR mask);