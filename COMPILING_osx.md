# Building Infra Arcana on Mac OS X

I'm using OS X 10.8.4, Xcode 4.6.2.
I haven't played it extensively but it builds and runs.

Not sure if it compiles with older OS/Xcode versions.
This worked for me, your mileage may vary.

## SDL

SDL and SDL\_image can be installed as frameworks or as shared libraries.

### Frameworks

[**SDL framework**](http://www.libsdl.org/download-1.2.php)  
http://www.libsdl.org/release/SDL-1.2.15.dmg

[**SDL\_image framework**](http://www.libsdl.org/projects/SDL_image/)  
http://www.libsdl.org/projects/SDL_image/release/SDL_image-1.2.12.dmg

Copy `SDL.framework` and `SDL_image.framework` to `/Library/Frameworks`
or `/Users/name/Library/Frameworks`.

Alternately, shared libraries (libsdl, libsdl\_image) can be used
instead of frameworks. Install with a package manager (Fink, MacPorts,
Homebrew, pkgsrc) or build and install from source.
Install `libsdl` and `libsdl_image`. `sdl-config` should be in your `$PATH`.

## Build

    $ make TARGETOS=macosx RELEASE=1 OSXLIBSDL=1

`TARGETOS=macosx` builds for OS X. Omit for non-OS X build.

`RELEASE=1` builds an optimized 'release' version. Omit for the debug version.

`OSXLIBSDL=1` uses libsdl and libsdl\_image for OS X. Omit to use SDL frameworks.

You can export these to avoid typing them in for every `make` command:

    $ export TARGETOS=macosx RELEASE=1 OSXLIBSDL=1
    $ make

## Run

    $ cd build
    $ ./ia
