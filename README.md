# Guide for building Infra Arcana

## Fetching the source code

Clone the IA repository on Github:

    https://github.com/martin-tornqvist/ia.git

## Which branch should I build?
**develop** - If you want to try out new features early (the develop branch should be relatively stable and bug free, feel free to open Github issues if you encounter bugs or crashes).

**tags (v15.0, v16.0, etc)** - If you want to build one of the official releases (note that the build method may be very different for older versions).

## Building Infra Arcana

Infra Arcana is built with [CMake](https://cmake.org/), which is a build system generator. CMake can generate GNU Makefiles, Code::Blocks projects, Visual Studio solutions, etc for many different platforms. Look for an online tutorial on how to use CMake (some pointers are given below).

### Unix/Linux/macOS
You need CMake, build tools (e.g. GNU Make + gcc), as well as SDL2, SDL2-image, and SDL2-mixer. These are the required SDL packages (retrieve them with apt-get for Ubuntu and Linux Mint):

    libsdl2-2.0-0 libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev

Hint for building:

    cd <ia directory>
    mkdir build && cd build
    cmake ../
    make ia

NOTE: Building with CMake on macOS is not yet tested (as far as I know) - I would love to hear feedback on success or failures!

### Windows
You need CMake, and some IDE or build tools of your choice (such as [Code::Blocks](http://www.codeblocks.org), or [Visual Studio](https://www.visualstudio.com/)). The Windows version of SDL is included in the repository already, so there is no need to download this.

Run CMake. If you use the graphical interface, then for "Where is the source code?" select the root folder of the ia repo (NOT the "src" folder), and for "Where to build the binaries?" select a folder called "build" in the ia repo (the "build" folder may not actually exist yet, but it doesn't matter, just specify this path). Run "Configure" and "Generate".

You may need to set up some system environment variables to fix errors, depending on which type of project you are generating.

After running CMake, if everything went fine, the project (of the type that you selected) should be available in the "build" folder. Open this project and build the "ia" target.

For example, if you generated a Code::Blocks project, then in the drop-down target list (near the top of the screen) select the "ia" target. Build by clicking on the yellow cogwheel, then run the game by clicking on the green arrow.
