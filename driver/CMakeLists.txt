project(HideMeDriver LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(WDK REQUIRED)

message(STATUS "WDK ${WDK_VERSION} (${WDK_PLATFORM}) in ${WDK_ROOT}")

# Driver WDM. Per un driver KMDF: wdk_add_driver(HideMeDriver KMDF 1.35 ...)
wdk_add_driver(HideMeDriver
        src/Driver.cpp
        src/Utils.cpp
)
target_include_directories(HideMeDriver PRIVATE src)
target_compile_options(HideMeDriver PRIVATE /W4 /WX /GS-)
target_link_libraries(HideMeDriver PRIVATE
        "${WDK_KM_LIB_DIR}/aux_klib.lib"
)

# --- Firma di test opzionale -------------------------------------------------
# cmake -DDRIVER_SIGN_CERT="NomeCertificato" ...  (certificato nello store "PrivateCertStore")
set(DRIVER_SIGN_CERT "" CACHE STRING "Nome (subject) del certificato di test per firmare il .sys")
if(DRIVER_SIGN_CERT)
    find_program(SIGNTOOL signtool
        HINTS "${WDK_ROOT}/bin/${WDK_VERSION}/x64" "${WDK_ROOT}/bin/x64"
        REQUIRED)
    add_custom_command(TARGET HideMeDriver POST_BUILD
        COMMAND "${SIGNTOOL}" sign /v /fd SHA256 /s PrivateCertStore /n "${DRIVER_SIGN_CERT}"
                "$<TARGET_FILE:HideMeDriver>"
        COMMENT "Firma di test di $<TARGET_FILE_NAME:HideMeDriver>")
endif()
