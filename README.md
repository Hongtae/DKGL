# Project DK, DKLib
Game Library for C++ and Python (PyDK)  
See demo: https://github.com/tiff2766/DKDemo


## DK

A Lightweighted cross-platform Game Engine.  
This is low level core-library written with C++.  

### Key Features
- 3D Audio
- Game Physics
- Data Marshaling
- Animation Retarget, Blending
- Modular Design
- Voxel (restrictively)

### Running platforms.
- Windows Vista or later.
- Mac OS X 10.9 or later.
- iOS 3.3 or later.
- ~~Android 2.3 or later~~ (Not yet, I'm working)
    - JNI birdge required (I'll merge project as soon as possible.)

For other platforms including linux or android, You can implement
DK interfaces to use DK library with other GUI OSes.
> See interface header files in `DKFramework/Interface`.  
> You can see implementations for other OSes in `DKFramework/Private`

## Build Environments
- Visual Studio 2013 for Windows
- Xcode6 for Mac, iOS
- C\++11 compiler

## PyDK
Python bindings for DK. (Python 3.4 or later).   
You can build PyDK with or without python interpreter embedded.

###### You need python library to build PyDK with python interpreter embedded.
1. Download prebuilt library for your platform.
    - Win32 i386: [python3.4.2_Win32.zip](https://github.com/tiff2766/Python/releases/download/v3.4.2/python3.4.2_Win32.zip)
    - Win32 x64: [python3.4.2_Win32_x64.zip](https://github.com/tiff2766/Python/releases/download/v3.4.2/python3.4.2_Win32_x64.zip)
    - Mac OS X (Universal): [python3.4.2_OSX.zip](https://github.com/tiff2766/Python/releases/download/v3.4.2/python3.4.2_OSX.zip)
    - Apple iOS (Universal): [python3.4.2_iOS.zip](https://github.com/tiff2766/Python/releases/download/v3.4.2/python3.4.2_iOS.zip)
2. Extract intoto PyDK/Python/  
    `example for OSX: PyDK/Python/OSX/libpython3.4.a`
3. Build!

## Required Libraries (Included in this project)
- OpenAL-soft-1.15.1 (Win32, Android only)
- BulletPhysics-2.82 (svn rev: 2719)
- CxImage-7.02
- FreeType-2.5.3
- bzip2-1.0.6
- jpeg-9
- libFLAC-1.2.1
- libogg-1.3.1
- libpng-1.5.10
- libtiff-4.0.3
- libvorbis-1.3.3
- libxml2-2.9.0
- sqlite-3.7.15.2
- zlib-1.2.7


## Author
Hongtae Kim, (tiff2766@gmail.com)

## Liense
[MIT License](http://opensource.org/licenses/MIT)
