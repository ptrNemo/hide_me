#pragma once

#include <windows.h>
#include <lmcons.h>     // UNLEN
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <vector>

struct RawSMBIOSData {
    BYTE  Used20CallingMethod;
    BYTE  SMBIOSMajorVersion;
    BYTE  SMBIOSMinorVersion;
    BYTE  DmiRevision;
    DWORD Length;
    BYTE  SMBIOSTableData[1];
};

struct SMBIOSHeader {
    BYTE Type;
    BYTE Length;   // lunghezza della parte formattata (stringhe escluse)
    WORD Handle;
};

// SMBIOS Type 1 - System Information
struct SMBIOSType1 {
    SMBIOSHeader Header;
    BYTE Manufacturer;  // indice stringa
    BYTE ProductName;   // indice stringa
    BYTE Version;       // indice stringa
    BYTE SerialNumber;  // indice stringa
    BYTE UUID[16];      // 2.1+
    BYTE WakeUpType;    // 2.1+
    BYTE SKUNumber;     // 2.4+ indice stringa
    BYTE Family;        // 2.4+ indice stringa
};