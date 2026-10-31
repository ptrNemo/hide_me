// Legge la tabella SMBIOS raw con GetSystemFirmwareTable('RSMB')
// e stampa i campi della struttura Type 1 (System Information),
// poi stampa il risultato di GetUserNameA().

#include "main.h"
#pragma comment(lib, "advapi32.lib")  // GetUserNameA


// Restituisce la stringa n-esima (1-based) che segue la parte formattata.
static const char* GetSMBIOSString(const SMBIOSHeader* h, BYTE index, const BYTE* end) {
    if (index == 0)
        return "(nessuna)";

    const char* s = reinterpret_cast<const char*>(h) + h->Length;
    for (BYTE i = 1; i < index; ++i) {
        while (*s && reinterpret_cast<const BYTE*>(s) < end) ++s;
        ++s;
        if (*s == '\0' || reinterpret_cast<const BYTE*>(s) >= end)
            return "(indice non valido)";
    }
    return s;
}

// Puntatore alla struttura successiva: salta parte formattata + stringhe + doppio NUL.
static const BYTE* NextStructure(const SMBIOSHeader* h, const BYTE* end) {
    const BYTE* p = reinterpret_cast<const BYTE*>(h) + h->Length;
    while (p + 1 < end && !(p[0] == 0 && p[1] == 0)) ++p;
    return p + 2;
}

static const char* WakeUpTypeToString(BYTE t) {
    switch (t) {
    case 0x00: return "Reserved";
    case 0x01: return "Other";
    case 0x02: return "Unknown";
    case 0x03: return "APM Timer";
    case 0x04: return "Modem Ring";
    case 0x05: return "LAN Remote";
    case 0x06: return "Power Switch";
    case 0x07: return "PCI PME#";
    case 0x08: return "AC Power Restored";
    default:   return "Valore sconosciuto";
    }
}

static void PrintType1(const SMBIOSType1* t, const BYTE* end, BYTE major, BYTE minor) {
    const SMBIOSHeader* h = &t->Header;
    const BYTE len = h->Length;

    printf("=== SMBIOS Type 1 - System Information ===\n");
    printf("Handle         : 0x%04X\n", h->Handle);
    printf("Length         : 0x%02X (%u byte)\n", len, len);
    printf("Manufacturer   : %s\n", GetSMBIOSString(h, t->Manufacturer, end));
    printf("Product Name   : %s\n", GetSMBIOSString(h, t->ProductName, end));
    printf("Version        : %s\n", GetSMBIOSString(h, t->Version, end));
    printf("Serial Number  : %s\n", GetSMBIOSString(h, t->SerialNumber, end));

    if (len >= 0x19) {
        const BYTE* u = t->UUID;
        // Da SMBIOS 2.6 i primi tre campi sono little-endian
        bool le = (major > 2) || (major == 2 && minor >= 6);
        if (le) {
            printf("UUID           : %02X%02X%02X%02X-%02X%02X-%02X%02X-",
                   u[3], u[2], u[1], u[0], u[5], u[4], u[7], u[6]);
        } else {
            printf("UUID           : %02X%02X%02X%02X-%02X%02X-%02X%02X-",
                   u[0], u[1], u[2], u[3], u[4], u[5], u[6], u[7]);
        }
        printf("%02X%02X-%02X%02X%02X%02X%02X%02X\n",
               u[8], u[9], u[10], u[11], u[12], u[13], u[14], u[15]);

        printf("Wake-up Type   : 0x%02X (%s)\n", t->WakeUpType, WakeUpTypeToString(t->WakeUpType));
    }

    if (len >= 0x1B) {
        printf("SKU Number     : %s\n", GetSMBIOSString(h, t->SKUNumber, end));
        printf("Family         : %s\n", GetSMBIOSString(h, t->Family, end));
    }
}

// Stampa la Type 1; se la tabella non e' leggibile o la Type 1 manca, stampa "(vuoto)".
static void PrintSMBIOS() {
    const DWORD sig = 'RSMB';

    UINT size = GetSystemFirmwareTable(sig, 0, nullptr, 0);
    if (size == 0) {
        fprintf(stderr, "GetSystemFirmwareTable (size) fallita: %lu\n", GetLastError());
        printf("=== SMBIOS Type 1 - System Information ===\n(vuoto)\n");
        return;
    }

    std::vector<BYTE> buf(size);
    if (GetSystemFirmwareTable(sig, 0, buf.data(), size) != size) {
        fprintf(stderr, "GetSystemFirmwareTable (data) fallita: %lu\n", GetLastError());
        printf("=== SMBIOS Type 1 - System Information ===\n(vuoto)\n");
        return;
    }

    auto* raw = reinterpret_cast<const RawSMBIOSData*>(buf.data());
    printf("SMBIOS versione %u.%u (DMI rev %u), tabella %lu byte\n\n",
           raw->SMBIOSMajorVersion, raw->SMBIOSMinorVersion, raw->DmiRevision, raw->Length);

    const BYTE* p   = raw->SMBIOSTableData;
    const BYTE* end = p + raw->Length;
    bool found = false;

    while (p + sizeof(SMBIOSHeader) <= end) {
        auto* h = reinterpret_cast<const SMBIOSHeader*>(p);
        if (h->Length < sizeof(SMBIOSHeader))
            break;

        if (h->Type == 1) {
            PrintType1(reinterpret_cast<const SMBIOSType1*>(h), end,
                       raw->SMBIOSMajorVersion, raw->SMBIOSMinorVersion);
            found = true;
        }
        if (h->Type == 127)  // End-of-Table
            break;

        p = NextStructure(h, end);
    }

    if (!found)
        printf("=== SMBIOS Type 1 - System Information ===\n(vuoto)\n");
}

static void PrintUserName() {
    char name[UNLEN + 1];
    DWORD len = sizeof(name);

    printf("\n=== GetUserNameA ===\n");
    if (GetUserNameA(name, &len))
        printf("User Name      : %s\n", name);
    else
        printf("User Name      : (vuoto) - errore %lu\n", GetLastError());
}

// Stampa il seriale del volume C: (lo stesso mostrato da "vol C:").
static void PrintVolumeSerial() {
    DWORD serial = 0;

    printf("\n=== GetVolumeInformationA ===\n");
    if (GetVolumeInformationA("C:\\", nullptr, 0, &serial, nullptr, nullptr, nullptr, 0))
        printf("Serial C:      : %04lX-%04lX\n", serial >> 16, serial & 0xFFFF);
    else
        printf("Serial C:      : (vuoto) - errore %lu\n", GetLastError());
}

int main() {
    PrintSMBIOS();
    PrintUserName();
    PrintVolumeSerial();

    std::cin.get();
    return 0;
}
