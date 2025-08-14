include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
message(STATUS "Local .env.cmake: ${LOCAL_ENV}")

cmake_minimum_required(VERSION 3.11.0)

set(NAME DOOM)

message(STATUS "using ${CMAKE_GENERATOR}")

if (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
    if (NOT MINGW_PATH)
        message(FATAL_ERROR "MINGW_PATH not set in .env.cmake")
    endif()
    set(USE_MINGW "True")
    set(CMAKE_C_COMPILER ${MINGW_PATH}/bin/gcc.exe)
    set(CMAKE_CXX_COMPILER ${MINGW_PATH}/bin/g++.exe)
endif()

project(${NAME} VERSION 0.23.0)

# SDL2
find_package(SDL2 REQUIRED)
if (NOT SDL2_FOUND)
    message(FATAL_ERROR "Could not find SDL2!")
else()
    message(STATUS "Using SDL2 include dir: ${SDL2_INCLUDE_DIRS}")
endif()

# stb_image
if (NOT STBIMAGE_PATH)
    message(STATUS "STBIMAGE_PATH not specified in .env.cmake, using external/stbimage")
    set(STBIMAGE_PATH external/stbimage)
endif()

# Sources
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

# Includes & Linking
if (WIN32)
    message(STATUS "CREATING BUILD FOR WINDOWS")

    if (USE_MINGW)
        target_include_directories(${PROJECT_NAME} PUBLIC
            ${MINGW_PATH}/include
        )
        target_link_directories(${PROJECT_NAME} PUBLIC
            ${MINGW_PATH}/lib
        )
    endif()

elseif (UNIX)
    message(STATUS "CREATING BUILD FOR UNIX")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${STBIMAGE_PATH}
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    ${SDL2_LIBRARIES}
)