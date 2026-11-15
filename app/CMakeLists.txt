project(HideMeApp LANGUAGES C CXX)

add_executable(HideMeApp src/main.cpp)

target_link_libraries(HideMeApp PRIVATE
        "iphlpapi.lib"
        "ws2_32.lib"
)

set_target_properties(HideMeApp PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)