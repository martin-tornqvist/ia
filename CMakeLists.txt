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
add_executable(ia ${src})

add_executable(ia-debug ${src})

set_target_properties(ia PROPERTIES
  OUTPUT_NAME ia
  )

set_target_properties(ia-debug PROPERTIES
  OUTPUT_NAME ia-debug
  )

# NOTE: The test target must use exceptions (used by the test framework)
  
#
# TODO: Enable this flag:
#
# -Wfloat-conversion
#
set(COMMON_COMPILE_OPTIONS
  -std=c++14
  -Wall
  -Wextra
  -Werror
  -Wno-unused-value
  -fno-rtti
  )  

target_compile_options(ia PUBLIC
  ${COMMON_COMPILE_OPTIONS}
  -fno-exceptions
  -O2
  -DNDEBUG
  )

target_compile_options(ia-debug PUBLIC
  ${COMMON_COMPILE_OPTIONS}
  -fno-exceptions
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
    
    set(SDL_ARCH_DIR i686-w64-mingw32)
    
  elseif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    message(STATUS "Assuming 64 bit architecture")
    
    set(SDL_ARCH_DIR x86_64-w64-mingw32)
    
  else()
    message(FATAL_ERROR "Unknown architecture")
  endif()
  
  set(SDL_BASE_DIR ${CMAKE_SOURCE_DIR}/SDL)
  
  set(SDL2_PATH       ${SDL_BASE_DIR}/SDL2-2.0.4/${SDL_ARCH_DIR})
  set(SDL2_IMAGE_PATH ${SDL_BASE_DIR}/SDL2_image-2.0.1/${SDL_ARCH_DIR})
  set(SDL2_MIXER_PATH ${SDL_BASE_DIR}/SDL2_mixer-2.0.1/${SDL_ARCH_DIR})
  
  set(COMMON_SDL_INCLUDE_DIRS
    ${SDL2_PATH}/include
    ${SDL2_IMAGE_PATH}/include
    ${SDL2_MIXER_PATH}/include
    )

  target_include_directories(ia PUBLIC
    ${COMMON_SDL_INCLUDE_DIRS}
    )
    
  target_include_directories(ia-debug PUBLIC
    ${COMMON_SDL_INCLUDE_DIRS}
    )
    
  set(COMMON_LINK_FLAGS
    -lmingw32
    -L${SDL2_PATH}/lib -lSDL2main -lSDL2
    -L${SDL2_IMAGE_PATH}/lib -lSDL2_image
    -L${SDL2_MIXER_PATH}/lib -lSDL2_mixer
    )
    
  target_link_libraries(ia
    ${COMMON_LINK_FLAGS}
    )

  target_link_libraries(ia-debug
    ${COMMON_LINK_FLAGS}
    )
    
    set(SDL2_BIN_PATH       ${SDL2_PATH}/bin)
    set(SDL2_IMAGE_BIN_PATH ${SDL2_IMAGE_PATH}/bin)
    set(SDL2_MIXER_BIN_PATH ${SDL2_MIXER_PATH}/bin)
    
    # SDL dll files and licenses
    set(SDL_DISTR_FILES
      ${SDL2_BIN_PATH}/SDL2.dll     
      ${SDL2_IMAGE_BIN_PATH}/SDL2_image.dll
      ${SDL2_IMAGE_BIN_PATH}/zlib1.dll
      ${SDL2_IMAGE_BIN_PATH}/libpng16-16.dll
      ${SDL2_IMAGE_BIN_PATH}/LICENSE.zlib.txt
      ${SDL2_IMAGE_BIN_PATH}/LICENSE.png.txt
      ${SDL2_MIXER_BIN_PATH}/SDL2_mixer.dll
      ${SDL2_MIXER_BIN_PATH}/libogg-0.dll
      ${SDL2_MIXER_BIN_PATH}/libvorbis-0.dll
      ${SDL2_MIXER_BIN_PATH}/libvorbisfile-3.dll
      ${SDL2_MIXER_BIN_PATH}/LICENSE.ogg-vorbis.txt
      )
    
    file(COPY ${SDL_DISTR_FILES} DESTINATION .)
    
    install(FILES ${SDL_DISTR_FILES} DESTINATION ia)
    
    install(FILES ${SDL_DISTR_FILES} DESTINATION ia-debug)
    
else()
  
  #
  # Not windows (e.g. Unix)
  #

  set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

  find_package(SDL2 REQUIRED)

  find_package(SDL2_image REQUIRED)

  find_package(SDL2_mixer REQUIRED)
  
  set(COMMON_SDL_INCLUDE_DIRS
    ${SDL2_INCLUDE_DIR}
    ${SDL2_IMAGE_INCLUDE_DIR}
    ${SDL2_MIXER_INCLUDE_DIR}
    )

  target_include_directories(ia PUBLIC
    ${COMMON_SDL_INCLUDE_DIRS}
    )
    
  target_include_directories(ia-debug PUBLIC
    ${COMMON_SDL_INCLUDE_DIRS}
    )

  set(COMMON_SDL_LIBS
    ${SDL2_LIBRARY}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    )

  target_link_libraries(ia PUBLIC
    ${COMMON_SDL_LIBS}
    )
    
  target_link_libraries(ia-debug PUBLIC
    ${COMMON_SDL_LIBS}
    )
    
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
