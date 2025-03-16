cmake_minimum_required(VERSION 3.10)
project(XVPatcher VERSION 1.10 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

# Define the output DLL name
set(OUTPUT_NAME "xinput1_3")

# Options
option(COPY_TO_GAME "Copy the DLL to game directory after build" ON)
set(GAME_DIR "C:/Program Files (x86)/Steam/steamapps/common/DB Xenoverse" CACHE PATH "Path to game directory")

# Common compiler flags and definitions
add_compile_definitions(NO_ZLIB TIXML_USE_STL)
if(MSVC)
    # MSVC specific definitions
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS 
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
    
    # Use multi-threaded static runtime to avoid dependency issues
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Source files organization
# Core files
set(CORE_SOURCES
    "main.cpp"
    "patch.cpp"
    "debug.cpp"
    "PatchUtils.cpp"
    "cpk_patch.cpp"
    "cpkdef.cpp"
)

# Consolidated utility files
set(UTILS_SOURCES
    "Utils.cpp"
)

# Consolidated stream system
set(STREAM_SOURCES
    "Stream.cpp"
)

# File format handling
set(FILE_FORMATS_SOURCES
    "BaseFile.cpp"
    "CpkFile.cpp"
    "HcaFile.cpp"
    "IggyFile.cpp"
    "EPatchFile.cpp"
    "IniFile.cpp"
    "WavFile.cpp"
)

# Thread/Sync handling
set(THREAD_SOURCES
    "Thread.cpp"
)

# Crypto sources
file(GLOB CRYPTO_SOURCES "crypto/*.c")

# TinyXML sources
file(GLOB TINYXML_SOURCES "tinyxml/*.cpp")

# Combine all sources
set(SOURCES 
    ${CORE_SOURCES}
    ${UTILS_SOURCES}
    ${STREAM_SOURCES}
    ${FILE_FORMATS_SOURCES}
    ${THREAD_SOURCES}
    ${CRYPTO_SOURCES}
    ${TINYXML_SOURCES}
)

# Create the shared library
add_library(${OUTPUT_NAME} SHARED ${SOURCES})

# Include directories
target_include_directories(${OUTPUT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyxml
    ${CMAKE_CURRENT_SOURCE_DIR}/crypto
)

# Different link libraries based on compiler
if(MSVC)
    # MSVC libraries 
    target_link_libraries(${OUTPUT_NAME} PRIVATE
        xinput1_3.lib
        version.lib
        Shlwapi.lib
        wininet.lib
    )
else()
    # MinGW/GCC libraries
    target_link_libraries(${OUTPUT_NAME} PRIVATE
        xinput1_3
        version
        pthread
        MinHook
    )
endif()

# Windows-specific settings
if(WIN32)
    if(MSVC)
        # Use the module definition file
        set_target_properties(${OUTPUT_NAME} PROPERTIES
            LINK_FLAGS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/xinput.def"
        )
    else()
        # MinGW specific settings
        set_target_properties(${OUTPUT_NAME} PROPERTIES
            PREFIX ""
            LINK_FLAGS "-static -Wl,--subsystem,windows,--kill-at,--enable-stdcall-fixup"
        )
        
        # Use the module definition file
        target_sources(${OUTPUT_NAME} PRIVATE "xinput.def")
    endif()
    
    # Copy to game directory
    if(COPY_TO_GAME)
        add_custom_command(TARGET ${OUTPUT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy 
                    "$<TARGET_FILE:${OUTPUT_NAME}>"
                    "${GAME_DIR}/${OUTPUT_NAME}.dll"
            COMMENT "Copying DLL to game directory..."
        )
    endif()
endif()

# Disable specific warnings
if(MSVC)
    target_compile_options(${OUTPUT_NAME} PRIVATE 
        /wd4996  # Disable deprecated function warnings
        /wd4244  # Disable conversion warnings
        /wd4267  # Disable size_t to smaller type conversion warnings
        /wd4311  # Disable pointer truncation warnings
        /wd4302  # Disable type cast truncation warnings
        /W3      # Warning level 3
    )
else()
    target_compile_options(${OUTPUT_NAME} PRIVATE 
        -Wno-strict-aliasing 
        -fpermissive
    )
endif()

# Installation
install(TARGETS ${OUTPUT_NAME}
    RUNTIME DESTINATION bin
) 