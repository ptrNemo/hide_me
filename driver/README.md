# KernelDriver

Driver kernel WDM in C++ per Windows, costruito con CMake + MSVC + WDK.

## Requisiti
- Visual Studio 2022/2026 con il workload "Desktop development with C++"
- Windows Driver Kit (WDK) — rilevato automaticamente (`cmake/FindWDK.cmake`)
- CLion (usa i suoi cmake/ninja integrati)

## Configurazione in CLion
1. **Settings → Build, Execution, Deployment → Toolchains**: aggiungi/seleziona **Visual Studio**,
   *Architecture* = **amd64**. Mettila in cima alla lista (default).
2. Apri la cartella del progetto: CLion trova `CMakePresets.json` e offre i profili
   **x64-debug** e **x64-release**. Abilitali in *Settings → CMake* (puoi disabilitare il profilo "Debug" generato da CLion).
3. Build → `KernelDriver.sys` in `build/x64-debug/` (o `build/x64-release/`).

> Nota: se il percorso del progetto è molto lungo (>~200 caratteri) MSVC può fallire con
> `C1041: cannot open program database`. Tieni il progetto in un percorso corto, es. `C:\dev\KernelDriver`.

## Opzioni CMake
| Variabile          | Default            | Descrizione                                       |
|--------------------|--------------------|---------------------------------------------------|
| `WDK_VERSION`      | più recente con km | es. `10.0.28000.0`                                |
| `WDK_WINVER`       | `0x0A00`           | Windows minimo supportato                         |
| `DRIVER_SIGN_CERT` | vuoto              | subject del certificato di test in `PrivateCertStore` per firmare il `.sys` post-build |

Per un driver **KMDF**: `wdk_add_driver(KernelDriver KMDF 1.35 src/...)` (entry point `FxDriverEntry`, include `<wdf.h>`).

## Caricare il driver (solo VM di test!)
```bat
bcdedit /set testsigning on
sc create KernelDriver type= kernel binPath= C:\path\KernelDriver.sys
sc start KernelDriver
sc stop KernelDriver
sc delete KernelDriver
```
I messaggi `KdPrintEx` sono visibili con DbgView (abilitando "Capture Kernel") o WinDbg.
