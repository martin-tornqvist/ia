# ------------------------------------------------------------------------------
#  Project
# ------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.0)

project(ia)

# ------------------------------------------------------------------------------
#  Source files
# ------------------------------------------------------------------------------
file(GLOB_RECURSE src
    src/*.cpp
    include/*.hpp
    rl_utils/src/*.cpp
    rl_utils/include/*
    )

# ------------------------------------------------------------------------------
#  Target definitions
# ------------------------------------------------------------------------------
add_executable(ia       ${src})
add_executable(ia-debug ${src})

set_target_properties(ia        PROPERTIES OUTPUT_NAME ia)
set_target_properties(ia-debug  PROPERTIES OUTPUT_NAME ia-debug)

# NOTE: The test target must use exceptions (used by the test framework)

#
# TODO: Enable this flag:
#
# -Wfloat-conversion
#

# set(CMAKE_CXX_FLAGS_DEBUG )

# set(CMAKE_CXX_FLAGS_RELEASE )

# GNU gcc specific compiler flags
if(CMAKE_COMPILER_IS_GNUCXX)
    set(COMMON_COMPILE_FLAGS
        -std=c++14
        -fno-rtti
        -Wall
        -Wextra
        -Werror
        -Wno-unused-value
        )

    set(DEBUG_COMPILE_FLAGS
        -fno-exceptions
        )

    set(RELEASE_COMPILE_FLAGS
        -fno-exceptions
        -O2
        )

endif()

set(RELEASE_COMPILE_FLAGS
    ${RELEASE_COMPILE_FLAGS}
    -D NDEBUG
    )

target_compile_options(ia PUBLIC
    ${COMMON_COMPILE_FLAGS}
    ${RELEASE_COMPILE_FLAGS}
    )

target_compile_options(ia-debug PUBLIC
    ${COMMON_COMPILE_FLAGS}
    ${DEBUG_COMPILE_FLAGS}
    )

set(COMMON_INCLUDE_DIRS
    include
    rl_utils/include
    )

target_include_directories(ia PUBLIC
    ${COMMON_INCLUDE_DIRS}
    )

target_include_directories(ia-debug PUBLIC
    ${COMMON_INCLUDE_DIRS}
    )

# Copy all resource files in the build directory.
file(COPY res DESTINATION .)

# ------------------------------------------------------------------------------
#  Dependencies
# ------------------------------------------------------------------------------
if(WIN32)

    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "4")
        message(STATUS "Assuming 32 bit architecture")

        set(ARCH 32bit)

    elseif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        message(STATUS "Assuming 64 bit architecture")

        set(ARCH 64bit)

    else()
        message(FATAL_ERROR "Unknown architecture")
    endif()

    set(SDL_BASE_DIR ${CMAKE_SOURCE_DIR}/SDL)

    if(MSVC)
    
        if("${ARCH}" EQUAL "32bit")
            set(SDL_ARCH_DIR x86)
        else()
            set(SDL_ARCH_DIR x64)
        endif()

        set(SDL2_PATH       ${SDL_BASE_DIR}/msvc/SDL2-2.0.5)
        set(SDL2_IMAGE_PATH ${SDL_BASE_DIR}/msvc/SDL2_image-2.0.1)
        set(SDL2_MIXER_PATH ${SDL_BASE_DIR}/msvc/SDL2_mixer-2.0.1)

        set(SDL_INCLUDE_DIRS
            ${SDL2_PATH}/include
            ${SDL2_IMAGE_PATH}/include
            ${SDL2_MIXER_PATH}/include
            )

        set(SDL2_LIBS_PATH          ${SDL2_PATH}/lib/${SDL_ARCH_DIR})
        set(SDL2_IMAGE_LIBS_PATH    ${SDL2_IMAGE_PATH}/lib/${SDL_ARCH_DIR})
        set(SDL2_MIXER_LIBS_PATH    ${SDL2_MIXER_PATH}/lib/${SDL_ARCH_DIR})

        set(SDL2_BINS_PATH          ${SDL2_PATH}/lib/${SDL_ARCH_DIR})
        set(SDL2_IMAGE_BINS_PATH    ${SDL2_IMAGE_PATH}/lib/${SDL_ARCH_DIR})
        set(SDL2_MIXER_BINS_PATH    ${SDL2_MIXER_PATH}/lib/${SDL_ARCH_DIR})

    else()

        #
        # Not MSVC - e.g. gcc
        #

        if("${ARCH}" EQUAL "32bit")
            set(SDL_ARCH_DIR i686-w64-mingw32)
        else()
            set(SDL_ARCH_DIR x86_64-w64-mingw32)
        endif()

        set(SDL2_PATH       ${SDL_BASE_DIR}/mingw/SDL2-2.0.5/${SDL_ARCH_DIR})
        set(SDL2_IMAGE_PATH ${SDL_BASE_DIR}/mingw/SDL2_image-2.0.1/${SDL_ARCH_DIR})
        set(SDL2_MIXER_PATH ${SDL_BASE_DIR}/mingw/SDL2_mixer-2.0.1/${SDL_ARCH_DIR})

        set(SDL_INCLUDE_DIRS
            ${SDL2_PATH}/include/SDL2
            ${SDL2_IMAGE_PATH}/include/SDL2
            ${SDL2_MIXER_PATH}/include/SDL2
            )

        set(SDL2_LIBS_PATH          ${SDL2_PATH}/lib)
        set(SDL2_IMAGE_LIBS_PATH    ${SDL2_IMAGE_PATH}/lib)
        set(SDL2_MIXER_LIBS_PATH    ${SDL2_MIXER_PATH}/lib)

        set(SDL2_BINS_PATH          ${SDL2_PATH}/bin)
        set(SDL2_IMAGE_BINS_PATH    ${SDL2_IMAGE_PATH}/bin)
        set(SDL2_MIXER_BINS_PATH    ${SDL2_MIXER_PATH}/bin)

        target_link_libraries(ia        mingw32)
        target_link_libraries(ia-debug  mingw32)

    endif()

    message(STATUS "SDL_INCLUDE_DIRS: " ${SDL_INCLUDE_DIRS})

    target_include_directories(ia       PUBLIC ${SDL_INCLUDE_DIRS})
    target_include_directories(ia-debug PUBLIC ${SDL_INCLUDE_DIRS})

    message(STATUS "SDL2_LIBS_PATH: "        ${SDL2_LIBS_PATH})
    message(STATUS "SDL2_IMAGE_LIBS_PATH: "  ${SDL2_IMAGE_LIBS_PATH})
    message(STATUS "SDL2_MIXER_LIBS_PATH: "  ${SDL2_MIXER_LIBS_PATH})

    find_library(SDL2_LIB_PATH          SDL2        PATHS ${SDL2_LIBS_PATH})
    find_library(SDL2_MAIN_LIB_PATH     SDL2main    PATHS ${SDL2_LIBS_PATH})
    find_library(SDL2_IMAGE_LIB_PATH    SDL2_image  PATHS ${SDL2_IMAGE_LIBS_PATH})
    find_library(SDL2_MIXER_LIB_PATH    SDL2_mixer  PATHS ${SDL2_MIXER_LIBS_PATH})

    message(STATUS "SDL2_LIB_PATH: "        ${SDL2_LIB_PATH})
    message(STATUS "SDL2_MAIN_LIB_PATH: "   ${SDL2_MAIN_LIB_PATH})
    message(STATUS "SDL2_IMAGE_LIB_PATH: "  ${SDL2_IMAGE_LIB_PATH})
    message(STATUS "SDL2_MIXER_LIB_PATH: "  ${SDL2_MIXER_LIB_PATH})

    set(SDL_LIBS
        ${SDL2_LIB_PATH}
        ${SDL2_MAIN_LIB_PATH}
        ${SDL2_IMAGE_LIB_PATH}
        ${SDL2_MIXER_LIB_PATH}
        )

    target_link_libraries(ia        ${SDL_LIBS})
    target_link_libraries(ia-debug  ${SDL_LIBS})

    # SDL dll files and licenses
    set(SDL_DISTR_FILES
        ${SDL2_BINS_PATH}/SDL2.dll
        ${SDL2_IMAGE_BINS_PATH}/SDL2_image.dll
        ${SDL2_IMAGE_BINS_PATH}/zlib1.dll
        ${SDL2_IMAGE_BINS_PATH}/libpng16-16.dll
        ${SDL2_IMAGE_BINS_PATH}/LICENSE.zlib.txt
        ${SDL2_IMAGE_BINS_PATH}/LICENSE.png.txt
        ${SDL2_MIXER_BINS_PATH}/SDL2_mixer.dll
        ${SDL2_MIXER_BINS_PATH}/libogg-0.dll
        ${SDL2_MIXER_BINS_PATH}/libvorbis-0.dll
        ${SDL2_MIXER_BINS_PATH}/libvorbisfile-3.dll
        ${SDL2_MIXER_BINS_PATH}/LICENSE.ogg-vorbis.txt
        )

    file(COPY ${SDL_DISTR_FILES} DESTINATION .)

    install(FILES ${SDL_DISTR_FILES} DESTINATION ia)
    install(FILES ${SDL_DISTR_FILES} DESTINATION ia-debug)

else()

    #
    # Not windows (e.g. Unix)
    #

    set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

    find_package(SDL2       REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_mixer REQUIRED)

    set(SDL_INCLUDE_DIRS
        ${SDL2_INCLUDE_DIR}
        ${SDL2_IMAGE_INCLUDE_DIR}
        ${SDL2_MIXER_INCLUDE_DIR}
        )

    target_include_directories(ia       PUBLIC ${SDL_INCLUDE_DIRS})
    target_include_directories(ia-debug PUBLIC ${SDL_INCLUDE_DIRS})

    set(SDL_LIBS
        ${SDL2_LIBRARY}
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES}
        )

    target_link_libraries(ia PUBLIC         ${SDL_LIBS})
    target_link_libraries(ia-debug PUBLIC   ${SDL_LIBS})

endif()


# ------------------------------------------------------------------------------
#  Testing
# ------------------------------------------------------------------------------

#
# TODO
#

# ------------------------------------------------------------------------------
#  Packaging
# ------------------------------------------------------------------------------
set(CMAKE_INSTALL_PREFIX target)

install(TARGETS ia DESTINATION ia)

install(TARGETS ia-debug DESTINATION ia-debug)

install(DIRECTORY res DESTINATION ia)

install(DIRECTORY res DESTINATION ia-debug)

set(CPACK_PACKAGE_NAME "ia")

set(CPACK_PACKAGE_VERSION "ia-debug")

set(CPACK_MONOLITHIC_INSTALL 1)

# This must be last
include(CPack)
