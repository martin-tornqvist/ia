# ------------------------------------------------------------------------------
# Project
# ------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.0)

project(ia)


# ------------------------------------------------------------------------------
# Source files
# ------------------------------------------------------------------------------
set(SRC
  include/ability_values.hpp
  include/actor_data.hpp
  include/actor_factory.hpp
  include/actor_items.hpp
  include/actor.hpp
  include/actor_mon.hpp
  include/actor_player.hpp
  include/ai.hpp
  include/attack_data.hpp
  include/attack.hpp
  include/audio.hpp
  include/bot.hpp
  include/browser.hpp
  include/character_descr.hpp
  include/close.hpp
  include/colors.hpp
  include/config.hpp
  include/create_character.hpp
  include/disarm.hpp
  include/draw_map.hpp
  include/drop.hpp
  include/explosion.hpp
  include/feature_data.hpp
  include/feature_door.hpp
  include/feature_event.hpp
  include/feature.hpp
  include/feature_mob.hpp
  include/feature_monolith.hpp
  include/feature_pylon.hpp
  include/feature_rigid.hpp
  include/feature_trap.hpp
  include/fov.hpp
  include/game.hpp
  include/game_time.hpp
  include/gfx.hpp
  include/global.hpp
  include/gods.hpp
  include/highscore.hpp
  include/init.hpp
  include/insanity.hpp
  include/inventory_handling.hpp
  include/inventory.hpp
  include/io.hpp
  include/item_artifact.hpp
  include/item_att_property.hpp
  include/item_data.hpp
  include/item_device.hpp
  include/item_factory.hpp
  include/item.hpp
  include/item_potion.hpp
  include/item_rod.hpp
  include/item_scroll.hpp
  include/knockback.hpp
  include/line_calc.hpp
  include/look.hpp
  include/main_menu.hpp
  include/manual.hpp
  include/mapgen.hpp
  include/map.hpp
  include/map_parsing.hpp
  include/map_patterns.hpp
  include/map_templates.hpp
  include/map_travel.hpp
  include/marker.hpp
  include/msg_log.hpp
  include/pickup.hpp
  include/player_bon.hpp
  include/player_spells.hpp
  include/populate_items.hpp
  include/populate_monsters.hpp
  include/populate_traps.hpp
  include/popup.hpp
  include/postmortem.hpp
  include/property.hpp
  include/property_data.hpp
  include/property_handler.hpp
  include/property_factory.hpp
  include/query.hpp
  include/reload.hpp
  include/room.hpp
  include/saving.hpp
  include/sdl_base.hpp
  include/sound.hpp
  include/spells.hpp
  include/state.hpp
  include/status_lines.hpp
  include/text_format.hpp
  include/throwing.hpp
  include/wham.hpp
  include/xml.hpp

  src/ability_values.cpp
  src/actor.cpp
  src/actor_data.cpp
  src/actor_factory.cpp
  src/actor_items.cpp
  src/actor_mon.cpp
  src/actor_player.cpp
  src/ai.cpp
  src/attack_data.cpp
  src/attack.cpp
  src/audio.cpp
  src/bot.cpp
  src/browser.cpp
  src/character_descr.cpp
  src/close.cpp
  src/colors.cpp
  src/config.cpp
  src/create_character.cpp
  src/disarm.cpp
  src/draw_map.cpp
  src/drop.cpp
  src/explosion.cpp
  src/feature.cpp
  src/feature_data.cpp
  src/feature_door.cpp
  src/feature_event.cpp
  src/feature_mob.cpp
  src/feature_monolith.cpp
  src/feature_pylon.cpp
  src/feature_rigid.cpp
  src/feature_trap.cpp
  src/fov.cpp
  src/game.cpp
  src/game_time.cpp
  src/gfx.cpp
  src/gods.cpp
  src/highscore.cpp
  src/init.cpp
  src/insanity.cpp
  src/inventory.cpp
  src/inventory_handling.cpp
  src/io.cpp
  src/item_artifact.cpp
  src/item.cpp
  src/item_data.cpp
  src/item_device.cpp
  src/item_factory.cpp
  src/item_potion.cpp
  src/item_rod.cpp
  src/item_scroll.cpp
  src/knockback.cpp
  src/line_calc.cpp
  src/look.cpp
  src/main.cpp
  src/main_menu.cpp
  src/manual.cpp
  src/map.cpp
  src/mapgen_aux_rooms.cpp
  src/mapgen.cpp
  src/mapgen_decorate.cpp
  src/mapgen_doors.cpp
  src/mapgen_merge_regions.cpp
  src/mapgen_monolith.cpp
  src/mapgen_pylon.cpp
  src/mapgen_region.cpp
  src/mapgen_river.cpp
  src/mapgen_room.cpp
  src/mapgen_special.cpp
  src/mapgen_sub_rooms.cpp
  src/mapgen_utils.cpp
  src/map_parsing.cpp
  src/map_patterns.cpp
  src/map_templates.cpp
  src/map_travel.cpp
  src/marker.cpp
  src/msg_log.cpp
  src/pickup.cpp
  src/player_bon.cpp
  src/player_spells.cpp
  src/populate_items.cpp
  src/populate_monsters.cpp
  src/populate_traps.cpp
  src/popup.cpp
  src/postmortem.cpp
  src/property.cpp
  src/property_data.cpp
  src/property_handler.cpp
  src/property_factory.cpp
  src/query.cpp
  src/reload.cpp
  src/room.cpp
  src/saving.cpp
  src/sdl_base.cpp
  src/sound.cpp
  src/spells.cpp
  src/state.cpp
  src/status_lines.cpp
  src/text_format.cpp
  src/throwing.cpp
  src/wham.cpp
  src/xml.cpp

  rl_utils/include/array2.hpp
  rl_utils/include/bresenham.hpp
  rl_utils/include/direction.hpp
  rl_utils/include/flood.hpp
  rl_utils/include/misc.hpp
  rl_utils/include/pathfind.hpp
  rl_utils/include/pos.hpp
  rl_utils/include/random.hpp
  rl_utils/include/rect.hpp
  rl_utils/include/rl_utils.hpp
  rl_utils/include/time.hpp

  rl_utils/src/bresenham.cpp
  rl_utils/src/direction.cpp
  rl_utils/src/flood.cpp
  rl_utils/src/misc.cpp
  rl_utils/src/pathfind.cpp
  rl_utils/src/pos.cpp
  rl_utils/src/random.cpp
  rl_utils/src/rl_utils.cpp
  rl_utils/src/time.cpp

  xml/tinyxml2/tinyxml2.h
  xml/tinyxml2/tinyxml2.cpp
  )


# ------------------------------------------------------------------------------
# Icon (on Windows)
# ------------------------------------------------------------------------------
set(RC_FILE "")

if(WIN32)
 set(RC_FILE ${CMAKE_SOURCE_DIR}/icon/icon.rc)
endif()


# ------------------------------------------------------------------------------
# Target definitions
# ------------------------------------------------------------------------------
add_executable(ia       ${SRC} ${RC_FILE})
add_executable(ia-debug ${SRC} ${RC_FILE})

set_target_properties(ia        PROPERTIES OUTPUT_NAME ia)
set_target_properties(ia-debug  PROPERTIES OUTPUT_NAME ia-debug)

# NOTE: The test target must use exceptions (used by the test framework)

# TODO: Enable this flag:
# -Wfloat-conversion

# GNU gcc and Clang specific compiler flags
if(CMAKE_COMPILER_IS_GNUCXX OR(CMAKE_C_COMPILER_ID MATCHES "Clang") OR(CMAKE_CXX_COMPILER_ID MATCHES "Clang"))

  set(COMMON_COMPILE_FLAGS
    -std=c++14
    -fno-rtti
    -Wall
    -Wextra
    -Werror
    -Wno-unused-value
    # implicit-fallthrough warnings must be disabled for tinyxml2, which relies
    # on this technique
    -Wno-implicit-fallthrough
    )

  set(DEBUG_COMPILE_FLAGS
    -fno-exceptions
    -O0
    -g
    # Uncomment to generate gmon.out for gprof (must also be done for linker
    # flags, see below)
    # -pg
    )

  set(RELEASE_COMPILE_FLAGS
    -fno-exceptions
    -O2
    )

endif()

set(RELEASE_COMPILE_FLAGS
    ${RELEASE_COMPILE_FLAGS}
    -DNDEBUG
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
    xml/tinyxml2
    )

target_include_directories(ia PUBLIC
    ${COMMON_INCLUDE_DIRS}
    )

target_include_directories(ia-debug PUBLIC
    ${COMMON_INCLUDE_DIRS}
    )

# On Windows releases, remove the console window
if(WIN32)
  # TODO: This solution only works with gcc/clang - not with MSVC
  if(CMAKE_COMPILER_IS_GNUCXX OR(CMAKE_C_COMPILER_ID MATCHES "Clang") OR(CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    target_link_libraries(ia -mwindows)
  endif()
  if(CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries(ia -static-libgcc -static-libstdc++)
  endif()
endif()

# Copy all resource files to the build directory.
file(COPY res DESTINATION .)


# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------
if(WIN32)

    if (NOT DEFINED ARCH)

        if("${CMAKE_SIZEOF_VOID_P}" EQUAL "4")
            message(STATUS "Assuming 32 bit architecture")
            set(ARCH 32bit)
        elseif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
            message(STATUS "Assuming 64 bit architecture")
            set(ARCH 64bit)
        endif()

    endif()

    if (NOT DEFINED ARCH OR ((NOT "${ARCH}" EQUAL "32bit") AND (NOT "${ARCH}" EQUAL "64bit")))
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

        # Not MSVC - e.g. gcc

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

    # Not windows (e.g. Unix)

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

    # Uncomment to generate gmon.out for gprof (must also be done for compile
    # flags, see above)
    # NOTE: "-no-pie" is needed due to "a bug in gcc", see:
    # https://stackoverflow.com/questions/42620074
    # https://stackoverflow.com/questions/39827412
    # target_link_libraries(ia-debug PUBLIC -pg -no-pie)

endif()


# ------------------------------------------------------------------------------
# Testing
# ------------------------------------------------------------------------------

# TODO


# ------------------------------------------------------------------------------
# Packaging
# ------------------------------------------------------------------------------
set(CMAKE_INSTALL_PREFIX target)

install(TARGETS ia       DESTINATION ia)
install(TARGETS ia-debug DESTINATION ia-debug)

install(DIRECTORY res DESTINATION ia)
install(DIRECTORY res DESTINATION ia-debug)
