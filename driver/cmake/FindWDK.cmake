# FindWDK.cmake
# -----------------------------------------------------------------------------
# Trova il Windows Driver Kit (WDK) e fornisce la funzione wdk_add_driver()
# per creare un kernel driver (.sys) con il compilatore MSVC.
#
# Variabili di input (opzionali, impostabili da cache / preset):
#   WDK_ROOT     - radice del Windows Kit (default: C:/Program Files (x86)/Windows Kits/10)
#   WDK_VERSION  - versione del WDK, es. 10.0.28000.0 (default: la piÃƒÂ¹ recente con headers km)
#   WDK_WINVER   - versione minima di Windows target (default: 0x0A00 = Windows 10)
#
# Variabili di output:
#   WDK_FOUND, WDK_ROOT, WDK_VERSION, WDK_PLATFORM, WDK_KM_INCLUDE_DIRS, WDK_KM_LIB_DIR
#
# Uso:
#   wdk_add_driver(<target> [KMDF <versione>] [WINVER <0x....>] [NTDDI_VERSION <0x....>] <sorgenti...>)
# -----------------------------------------------------------------------------

if(NOT MSVC)
    message(FATAL_ERROR "FindWDK: i kernel driver Windows richiedono il compilatore MSVC (toolchain 'Visual Studio' in CLion).")
endif()

# --- Radice del WDK ----------------------------------------------------------
if(NOT WDK_ROOT)
    if(DEFINED ENV{WDKContentRoot})
        set(_wdk_root_guess "$ENV{WDKContentRoot}")
    else()
        get_filename_component(_wdk_root_guess
            "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE)
        if(NOT EXISTS "${_wdk_root_guess}/Include")
            set(_wdk_root_guess "C:/Program Files (x86)/Windows Kits/10")
        endif()
    endif()
    set(WDK_ROOT "${_wdk_root_guess}" CACHE PATH "Radice del Windows Driver Kit")
endif()
file(TO_CMAKE_PATH "${WDK_ROOT}" WDK_ROOT)
string(REGEX REPLACE "/$" "" WDK_ROOT "${WDK_ROOT}")

# --- Versione del WDK: la piÃƒÂ¹ recente che contiene km/ntddk.h -----------------
if(NOT WDK_VERSION)
    file(GLOB _wdk_ntddk_files "${WDK_ROOT}/Include/*/km/ntddk.h")
    set(_wdk_versions "")
    foreach(_file IN LISTS _wdk_ntddk_files)
        string(REGEX MATCH "/Include/([0-9.]+)/km/ntddk.h$" _m "${_file}")
        if(CMAKE_MATCH_1)
            list(APPEND _wdk_versions "${CMAKE_MATCH_1}")
        endif()
    endforeach()
    if(_wdk_versions)
        list(SORT _wdk_versions COMPARE NATURAL ORDER DESCENDING)
        list(GET _wdk_versions 0 _wdk_latest)
        set(WDK_VERSION "${_wdk_latest}" CACHE STRING "Versione del WDK")
    endif()
endif()

# --- Architettura ------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ARCHITECTURE_ID)
    string(TOLOWER "${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" _wdk_arch)
else()
    string(TOLOWER "${CMAKE_C_COMPILER_ARCHITECTURE_ID}" _wdk_arch)
endif()
if(_wdk_arch STREQUAL "x64")
    set(WDK_PLATFORM "x64")
elseif(_wdk_arch STREQUAL "arm64")
    set(WDK_PLATFORM "arm64")
else()
    message(FATAL_ERROR "FindWDK: architettura '${_wdk_arch}' non supportata. Usa x64 o arm64 "
                        "(in CLion: Settings > Build > Toolchains > Visual Studio > Architecture = amd64).")
endif()

set(WDK_KM_INCLUDE_DIRS
    "${WDK_ROOT}/Include/${WDK_VERSION}/km"
    "${WDK_ROOT}/Include/${WDK_VERSION}/km/crt"
    "${WDK_ROOT}/Include/${WDK_VERSION}/shared")
set(WDK_KM_LIB_DIR "${WDK_ROOT}/Lib/${WDK_VERSION}/km/${WDK_PLATFORM}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WDK
    REQUIRED_VARS WDK_ROOT WDK_VERSION
    VERSION_VAR WDK_VERSION)

if(NOT EXISTS "${WDK_ROOT}/Include/${WDK_VERSION}/km/ntddk.h")
    message(FATAL_ERROR "FindWDK: headers kernel non trovati in ${WDK_ROOT}/Include/${WDK_VERSION}/km. "
                        "Installa il WDK oppure imposta WDK_VERSION.")
endif()

if(NOT WDK_WINVER)
    set(WDK_WINVER "0x0A00" CACHE STRING "Versione minima di Windows (0x0A00 = Windows 10)")
endif()

# --- Flag comuni -------------------------------------------------------------
set(WDK_COMPILE_OPTIONS
    /kernel                 # binario kernel-mode (niente EH/RTTI, new/delete kernel)
    /GS                     # buffer security check (fornito da BufferOverflowFastFailK.lib)
    /Gy                     # function-level linking
    /GF                     # string pooling
    /GR-                    # niente RTTI
    /Zp8                    # allineamento strutture come nel WDK
    /Zc:wchar_t
    /Zc:forScope
    /Zc:inline
    /Zc:threadSafeInit-     # niente TLS per gli static locali in kernel
    /Oi
    /FI${WDK_ROOT}/Include/${WDK_VERSION}/shared/warning.h
    $<$<CONFIG:Debug>:/Od>
    $<$<NOT:$<CONFIG:Debug>>:/O2>
)

set(WDK_COMPILE_DEFINITIONS
    WINNT=1
    POOL_NX_OPTIN=1
    POOL_ZERO_DOWN_LEVEL_SUPPORT=1
    $<$<CONFIG:Debug>:DBG=1>
    $<$<CONFIG:Debug>:DEPRECATE_DDK_FUNCTIONS=1>
    $<$<CONFIG:Debug>:MSC_NOOPT>
)
if(WDK_PLATFORM STREQUAL "x64")
    list(APPEND WDK_COMPILE_DEFINITIONS _WIN64 _AMD64_ AMD64)
else()
    list(APPEND WDK_COMPILE_DEFINITIONS _WIN64 _ARM64_ ARM64)
endif()

set(WDK_LINK_OPTIONS
    /DRIVER
    /SUBSYSTEM:NATIVE
    /NODEFAULTLIB
    /MANIFEST:NO
    /INCREMENTAL:NO
    /OPT:REF
    /OPT:ICF
    /MERGE:_TEXT=.text
    /MERGE:_PAGE=PAGE
    /SECTION:INIT,d
    /RELEASE                # imposta il checksum nel PE (richiesto per i driver)
    /DEBUG                  # genera sempre il .pdb (utile con WinDbg)
)

# Rimuove dai flag globali di CMake le opzioni incompatibili con /kernel
# (/RTC1 richiede la CRT, /EHsc le eccezioni C++).
foreach(_var
        CMAKE_C_FLAGS CMAKE_CXX_FLAGS
        CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_MINSIZEREL)
    if(DEFINED ${_var})
        string(REGEX REPLACE "/RTC[1csu]+" "" ${_var} "${${_var}}")
        string(REGEX REPLACE "/EH[a-z]+" "" ${_var} "${${_var}}")
        set(${_var} "${${_var}}" CACHE STRING "" FORCE)
    endif()
endforeach()

# --- wdk_add_driver ----------------------------------------------------------
function(wdk_add_driver _target)
    cmake_parse_arguments(ARG "" "KMDF;WINVER;NTDDI_VERSION" "" ${ARGN})

    add_executable(${_target} ${ARG_UNPARSED_ARGUMENTS})
    set_target_properties(${_target} PROPERTIES
        SUFFIX ".sys"
        MSVC_RUNTIME_LIBRARY "MultiThreaded"   # irrilevante con /NODEFAULTLIB, evita /MD
    )

    set(_winver ${WDK_WINVER})
    if(ARG_WINVER)
        set(_winver ${ARG_WINVER})
    endif()

    target_compile_options(${_target} PRIVATE ${WDK_COMPILE_OPTIONS})
    target_compile_definitions(${_target} PRIVATE
        ${WDK_COMPILE_DEFINITIONS}
        _WIN32_WINNT=${_winver}
        WINVER=${_winver})
    if(ARG_NTDDI_VERSION)
        target_compile_definitions(${_target} PRIVATE NTDDI_VERSION=${ARG_NTDDI_VERSION})
    endif()

    target_include_directories(${_target} SYSTEM PRIVATE ${WDK_KM_INCLUDE_DIRS})
    target_link_options(${_target} PRIVATE ${WDK_LINK_OPTIONS})
    target_link_libraries(${_target} PRIVATE
        "${WDK_KM_LIB_DIR}/BufferOverflowFastFailK.lib"
        "${WDK_KM_LIB_DIR}/ntoskrnl.lib"
        "${WDK_KM_LIB_DIR}/hal.lib"
        "${WDK_KM_LIB_DIR}/wmilib.lib")

    if(ARG_KMDF)
        set(_kmdf_inc "${WDK_ROOT}/Include/wdf/kmdf/${ARG_KMDF}")
        set(_kmdf_lib "${WDK_ROOT}/Lib/wdf/kmdf/${WDK_PLATFORM}/${ARG_KMDF}")
        if(NOT EXISTS "${_kmdf_inc}/wdf.h")
            message(FATAL_ERROR "wdk_add_driver: KMDF ${ARG_KMDF} non trovato in ${_kmdf_inc}")
        endif()
        string(REPLACE "." ";" _kmdf_ver "${ARG_KMDF}")
        list(GET _kmdf_ver 0 _kmdf_major)
        list(GET _kmdf_ver 1 _kmdf_minor)
        target_include_directories(${_target} SYSTEM PRIVATE "${_kmdf_inc}")
        target_compile_definitions(${_target} PRIVATE
            KMDF_VERSION_MAJOR=${_kmdf_major}
            KMDF_VERSION_MINOR=${_kmdf_minor})
        target_link_libraries(${_target} PRIVATE
            "${_kmdf_lib}/WdfDriverEntry.lib"
            "${_kmdf_lib}/WdfLdr.lib")
        target_link_options(${_target} PRIVATE /ENTRY:FxDriverEntry)
    else()
        # GsDriverEntry inizializza il security cookie e poi chiama DriverEntry
        target_link_options(${_target} PRIVATE /ENTRY:GsDriverEntry)
    endif()
endfunction()
