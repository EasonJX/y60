The ART+COM AG Y60 Media Engine is a multipurpose media and graphics engine targetted at interactive screen installations, interactive projection tables, 3D games and everything else that might go under the name "media installation". It can also be used as a general purpose 3D scene graph and renderer.

Y60 itself is implemented in C++ for the Windows, Linux and Mac OS X platforms. It can be scripted in JavaScript, offering a DOM API for its scene graph that is similar to what can be found in common web browsers. For graphics rendering, it targets OpenGL. 

It is open-source licensed under GPL and available for windows, linux and mac-osx.

# Prerequisits
- CMake: download and install the latest installer from: [[http://cmake.org/cmake/resources/software.html]]
- Cg Toolkit: get the latest installer from [[http://developer.nvidia.com/cg-toolkit]] or install it with your favorite package manager on your OS
- y60 relies heavily on ASL the ART+COM Standard Library and AcMake, so you need to install or build those too. Instructions can be found in the readme at [[https://github.com/artcom/asl]] and [[https://github.com/artcom/acmake]]
## Windows
- DirectX SDK: download from [[http://www.microsoft.com/en-us/download/details.aspx?id=6812]]
- get the latest dependencies: PRO60Dependencies-*.*.*-win32.exe [[https://y60.artcom.de/redmine/projects/y60/files]]
- Nullsoft Scriptable Install System NSIS from [[http://nsis.sourceforge.net/]] (optional, only needed for building packages)


# Prebuild Windows 32 Bit binary
Currently, we distribute windows installers at [[https://y60.artcom.de/redmine/projects/y60/files]]
Install the following packages and you should be able to run y60 applications
- get the latest installer Y60-*.*.*-win32.exe 
- get the appropriate dependencies: PRO60Dependencies-*.*.*-win32.exe
- get the appropriate asl library: ASL-*.*.*-win32.exe

# Build from sources
## Checkout
Checkout y60 sources
 
    git clone git@github.com:artcom/y60.git

### Build on Windows
#### Prerequisits
- Visual Studio Express 9 2008, 32Bit
from [[https://y60.artcom.de/redmine/projects/y60/files]]
- get the latest dependencies: PRO60Dependencies-*.*.*-win32.exe
- get the appropriate asl library: ASL-*.*.*-win32.exe
- get the appropriate acmake library: AcMake-*.*.*-win32.exe
and install them

#### If you want to use the gtk binding, also known as G60 (optional), you also need:

    install Gtkmm 2.14 or higher (development package) from [[https://live.gnome.org/gtkmm/MSWindows]]

#### Build process
Create build target directory:
  
    cd y60
    mkdir _build  
    cd _build  

Make build scripts using cmake (either for nmake or visual Studio 9 2008).  
Build with ide: 

    cmake -G "Visual Studio 9 2008" .. 
    open Y60.sln and build it using the ide

Build with nMake via shell: 

    cmake -G "NMake Makefiles" ..
    nmake

### Linux
    These instructions should work on Ubuntu >=8.04 and Debian (>=lenny) RedHat/CentOS >= 6 on either x86 or X86_64 systems. The Dependent library names may vary on those target systems

#### Install dependencies
     sudo apt-get install build-essential autoconf2.13 libboost-dev libboost-system-dev libboost-thread-dev libsdl1.2-dev libglib2.0-dev libavcodec-dev libavformat-dev libswscale-dev libglew1.5-dev libcurl4-openssl-dev libopenexr-dev libjpeg62-dev libtiff4-dev libpng12-dev libungif4-dev libasound2-dev libfreetype6-dev libcrypto++-dev

#### If you want to use the gtk binding, also known as G60, you also need:
     sudo apt-get install libgtkmm-2.4-dev libgtkglext1-dev libglademm-2.4-dev

#### Building the documentation requires:
     sudo apt-get install doxygen graphviz texlive

#### You also need java to build the documentation, for which we recommend OpenJDK.
     sudo apt-get install openjdk-6-jre

#### Create yourself a build directory (you will need one per build configuration)
    cd y60
    mkdir -p _builds/release

#### Configure the build tree (this is the equivalent of ./configure)

    cd _builds/release
    cmake -DCMAKE_INSTALL_PREFIX=/usr/local ../../

#### Build the sources

    make -jX

#### Install Y60

    make install

#### Test Y60

    make test

### Mac OS X with Homebrew

We have Homebrew [[http://mxcl.github.com/homebrew/]] support. This makes installing on Mac OS X easier than ever!

#### Prerequisites:

- Homebrew Installation: [[https://github.com/mxcl/homebrew/wiki/installation]]

#### Now pull the ART+COM homebrew fork:

    brew update
    cd $(brew --repository)
    git pull git@github.com:artcom/homebrew.git

#### Now simply install y60:

    brew install y60
#### You can also install the repository head:
    brew install --HEAD y60

With --use-internal-git ART+COMs internal git server is used.

# Building Documentation
#### Y60 API Reference
    make Y60-jsdoc

The results will be in y60/doc/jsdoc.

#### Y60 Schema Reference
    make Y60-xsddoc

The results will be in y60/doc/schema.
#### Doxygen C++ References
    make Y60-doxygen

The results will be y60/doc/doxygen.