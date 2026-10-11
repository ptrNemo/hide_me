project(HideMeApp LANGUAGES C CXX)

add_executable(HideMeApp src/Main.cpp)

set_target_properties(HideMeApp PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)