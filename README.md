# Guide for building Infra Arcana

## Which branch	should I build?
**develop** - If you want to try out new features early (the develop branch should be relatively stable and bug free, feel free to open Github issues if you encounter bugs or crashes).

**master** - If you want a more stable and well tested experience.

**tags (v15.0, v16.0, etc)** - If you want to build one of the official releases.

**NOTE:** Part of the IA source code resides in a Git sub module, which you also need to initialize and update, like so:

    $ git submodule init
    $ git submodule update

## Fetching source

If you aren't familiar with using Git, you can download whole source code directly [as compressed archive](https://github.com/InfraArcana/ia/archive/master.zip).

## Linux

The instructions below are written and tested for Ubuntu 12.10

### Building Infra Arcana

You will need build tools and SDL:

    $ sudo apt-get install make gcc libsdl2-2.0-0 libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev

Go to the IA repository directory, and type:

    $ make

Or if you want to do a debug build:

    $ make debug

Go to the "target" folder

Start IA by typing

    $ ./ia

(dot slash ia)

If you want, you can copy the "target" folder somewhere and rename it

## OS X

The instructions below are written and tested for OS X 10.11.5 and Xcode 7.3.1.

### Building on OS X

Install Xcode/Xcode command line tools.

Copy [SDL2.framework](http://www.libsdl.org/download-2.0.php), [SDL2\_image.framework](http://www.libsdl.org/projects/SDL_image/), and [SDL2\_mixer.framework](http://www.libsdl.org/projects/SDL_mixer/) to /Library/Frameworks (all users) or /Users/Username/Library/Frameworks (this user only).

Release build:

    $ make osx-release

Debug build:

    $ make osx-debug

Run:

    $ cd ./target
    $ ./ia

If you want, you can copy the "target" folder somewhere and rename it.

## Windows

Building on Windows is currently not supported. One option is to use Virtual Box to either play on Linux, or cross compile for Windows on Linux.

This will be resolved eventually.
