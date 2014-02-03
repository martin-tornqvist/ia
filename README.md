# Guide for building Infra Arcana

## Fetching source

If you aren't familiar with using Git, you can download whole source code directly [as compressed archive](https://github.com/InfraArcana/ia/archive/develop.zip).

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

    $ sudo apt-get install make gcc sdl libsdl1.2-dev libsdl-image1.2-dev libsdl-mixer1.2-dev libsdl-ttf2.0-dev

Go to the IA repository directory, and type:

    $ make

Go to the “target” folder

Start IA by typing

    $ ./ia

(dot slash ia)

If you want, you can copy the “target” folder somewhere and rename it

## OSX

Some people have successfully built IA on OSX by using the Linux Makefile as it is. Although building on OSX is not “officially supported”, the goal is to keep the project as portable as possible. It should require little extra effort (or no extra effort at all) to build IA on OSX. So go ahead and try ;)
Please tell of your problems or success at the IA webpage, or email me (Martin) - see contact.txt in the game folder (or assets/contact.txt in the IA repository).
