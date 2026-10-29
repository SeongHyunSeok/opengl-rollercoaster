cmake_minimum_required(VERSION 3.24)
set(CMAKE_VS_GLOBALS "VcpkgEnabled=false")
project(RollerCoaster LANGUAGES C CXX)

if(NOT WIN32)
    message(FATAL_ERROR "This project currently supports Windows only.")
endif()

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(FREEGLUT_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(FREEGLUT_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(FREEGLUT_BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/freeglut EXCLUDE_FROM_ALL)

add_library(rollercoaster_glew STATIC "${CMAKE_CURRENT_SOURCE_DIR}/third_party/glew/src/glew.c")
target_include_directories(rollercoaster_glew PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/glew/include")
target_compile_definitions(rollercoaster_glew PUBLIC GLEW_STATIC)
target_link_libraries(rollercoaster_glew PUBLIC opengl32)

add_executable(RollerCoaster main.cpp cart.cpp Pillar.cpp)
target_compile_features(RollerCoaster PRIVATE cxx_std_20)
if(MSVC)
    target_compile_options(RollerCoaster PRIVATE /utf-8)
endif()
target_compile_definitions(RollerCoaster PRIVATE FREEGLUT_STATIC FREEGLUT_LIB_PRAGMAS=0)
target_include_directories(RollerCoaster PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/freeglut/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/glm")
target_link_libraries(RollerCoaster PRIVATE rollercoaster_glew freeglut_static opengl32 glu32)
set_target_properties(RollerCoaster PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:RollerCoaster>")
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT RollerCoaster)

set(ROLLERCOASTER_ASSETS vertex2.glsl fragment2.glsl)
foreach(asset IN LISTS ROLLERCOASTER_ASSETS)
    add_custom_command(TARGET RollerCoaster POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/${asset}" "$<TARGET_FILE_DIR:RollerCoaster>/${asset}")
endforeach()
