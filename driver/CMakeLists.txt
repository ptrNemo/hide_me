project(HideMeDriver LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(WDK REQUIRED)

message(STATUS "WDK ${WDK_VERSION} (${WDK_PLATFORM}) in ${WDK_ROOT}")

wdk_add_driver(HideMeDriver
        src/driver.cpp
        src/utils.cpp
        src/spoofs/volumes.cpp
        src/spoofs/username.cpp
        src/spoofs/smbios.cpp
)
target_include_directories(HideMeDriver PRIVATE src)
target_compile_options(HideMeDriver PRIVATE /W4 /GS- /WX)
target_link_libraries(HideMeDriver PRIVATE
        "${WDK_KM_LIB_DIR}/aux_klib.lib"
)
