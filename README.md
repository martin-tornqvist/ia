# Guide for building Infra Arcana

## Which branch	should I build?
**master** - If you want to try out new features in a relatively stable and bug free state.

**tags (v15.0, v16.0, etc)** - If you want to build one of the official releases.

**develop** - No guarantees. It may not even compile.

**NOTE:** Part of the IA source code resides in a Git sub module, which you also need to initialize and update, like so:

    $ git submodule init
    $ git submodule update

## Fetching source

If you aren't familiar with using Git, you can download whole source code directly [as compressed archive](https://github.com/InfraArcana/ia/archive/master.zip).

## Windows

The instructions below are written and tested for Windows 7.

### Installing a development environment

[Download Code::blocks](http://www.codeblocks.org/downloads/binaries): (The MinGW version, e.g. "codeblocks-XX.XXmingw-setup.exe")

Install Code::Blocks
“*Select type of install*", choose "*Standard*"

A window should pop up, prompting for compiler choice. Select the GNU GCC Compiler and make it default.

"*Yes, associate Code::Blocks with C/C++ file type*"

Close Code::Blocks -> "*The perspective has changed, save it?*", check "*Don't annoy me again*"

### Building Infra Arcana

Go to the cloned Infra Arcana Git repository folder. Open "*cb_win_project*" (stands for "Code::Blocks Windows project"). Open "*cb_win_project.cbp*".

In the top left corner, there should be a button that looks like a yellow cogwheel - to the right of this, there should be a choice for "*Build target*", set this to “*Release*”. Then click the cogwheel button, this will start the build.

When the build is done, you should see something like this in the build log:

    Process terminated with status 0 (time)
    0 errors, 0 warnings

(Don’t mind it too much if there are some warnings)

Next to the cogwheel button is a green arrow button. Click it. The game should run now.

The Infra Arcana executable is put in the “target” folder inside the Git repository folder. If you want, you can copy the ‘target’ folder somewhere else and rename it.

If you get the following error when trying to build:

    ["cb_win_project - Debug" uses an invalid compiler. Probably the toolchain path within the compiler options is not setup correctly?! Skipping... ].

Try going to "*Settings*" -> "*Compiler and debugger*" -> "*Toolchain executables*", then click "*Auto-detect*".

## Linux

The instructions below are written and tested for Ubuntu 12.10

### Building Infra Arcana

You will need build tools and SDL:

    $ sudo apt-get install make gcc libsdl2-2.0-0 libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev

Go to the IA repository directory, and type:

    $ make

Or if you want to do a debug build:

    $ make debug

Go to the “target” folder

Start IA by typing

    $ ./ia

(dot slash ia)

If you want, you can copy the “target” folder somewhere and rename it

## OS X

The instructions below are written and tested for OS X 10.11.5 and Xcode 7.3.1.

### Building on OS X

Install Xcode/Xcode command line tools.

Copy [SDL2.framework](http://www.libsdl.org/download-2.0.php), [SDL2\_image.framework](http://www.libsdl.org/projects/SDL_image/), and [SDL2\_mixer.framework](http://www.libsdl.org/projects/SDL_mixer/) to /Library/Frameworks (all users) or /Users/Username/Library/Frameworks (this user only).

Release build:

    $ make osx

Debug build:

    $ make osx-debug

Run:

    $ cd ./target
    $ ./ia

If you want, you can copy the "target" folder somewhere and rename it.
