# Project DK, DKLib
Game Library for C++ and Python (PyDK)  
See demo: https://github.com/tiff2766/DKDemo  
Pre-built executable are also available to download: ([Windows]( https://bitbucket.org/tiff2766/dkdemo/downloads/DKDemoApp_Win32_En.zip),
[Mac OS X](https://bitbucket.org/tiff2766/dkdemo/downloads/Demo_OSX.app.zip))

# DK (Core Library)
A Lightweighted cross-platform Game Library.  
This is low level game library written with C++.  
You can use Python-script to improve productivity.

DK library has following contents with two frameworks layer.
##### DKFoundation - `Foundation Classes, Managing System Tasks.`
* C++ Metaprograms (Type Libraries, etc.)
* Data Collections (Array, Map, Tuples, etc.)
* Object Management (SmartPointers, WeakRef, Allocators, etc.)
* Thread and Synchronize objects (Mutex, Condition, Atomics, etc.)
* Thread Pool for Asynchronous Operations
* High Resolution Timer
* Run-Loop System
* Function Interface for Operations, Invocations, Functors
* Data and Buffer objects
* Rational for High-Precision Calculation
* Unicode String (UTF-8,16,32)
* File, Directory and Stream (including File-Mapping)
* Date Time class (ISO-8601 support)
* Endianness Byte Order Utilities
* Logging Interface
* Error Handler (with Stack-Tracer)
* XMLParser (DOM, SAX)
* Hash Library (SHA1, SHA2, etc.)
* Zip Archiver, Unarchiver

##### DKFramework - `Game Framework for Application`
* Application Entry Point
* Collision Detection Utilities
* Convex Decomposition
* Physics Constraints
* Dynamics Simulator
* Node Based Scene Graph Management
* Curve Interpolation (Spline, B-Spline, Bezier, etc.)
* Data Marshalling
* Animation (able to Retargeting, Blending)
* Text Font (TTF, PS, etc.)
* 3D Audio
* Mathematics (Matrices, Vectors, Quaternion, etc.)
* OpenGL Context Interface (OpenGL/OpenGL ES)
* Resource Management
* Material and Shader Management
* Window, Screen Management
* Voxel, Storage, Polygonizer Interfaces

Each framework designed for modular, cross-platform.  
You can use any classes without any dependencies.
You are also able to use single project for multiple-platforms without
any modifictations that platform dependent.

You can use DK with other UI Tools also. (MFC, Cocoa, wxWidgets, etc.)

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

# PyDK
Python bindings for DK. (Python 3.4 or later).   
You can build PyDK with or without python interpreter embedded.

###### You need python library to build PyDK with python interpreter embedded.
1. Download prebuilt library for your platform.
    - Win32 i386: [python3.4.2_Win32.zip](https://github.com/tiff2766/Python/releases/download/v3.4.2/python3.4.2_Win32.zip)
    - Win32 x64: [python3.4.2_Win32_x64.zip](https://github.com/tiff2766/Python/releases/download/v3.4.2/python3.4.2_Win32_x64.zip)
    - Mac OS X (Universal): [python3.4.2_OSX.zip](https://github.com/tiff2766/Python/releases/download/v3.4.2/python3.4.2_OSX.zip)
    - Apple iOS (Universal): [python3.4.2_iOS.zip](https://github.com/tiff2766/Python/releases/download/v3.4.2/python3.4.2_iOS.zip)
2. Extract into PyDK/Python/  
    `example for OSX: PyDK/Python/OSX/libpython3.4.a`
3. Build!

##### Pre-built module for python3.4 download:
Extract file into your Python site-packages dir.
- [Mac OS X](https://bitbucket.org/tiff2766/dkdemo/downloads/dk_core_python34_module_osx.zip)
- [Windows i386](https://bitbucket.org/tiff2766/dkdemo/downloads/dk_core_python34_module_win32.zip)
- [Windows x64](https://bitbucket.org/tiff2766/dkdemo/downloads/dk_core_python34_module_win32_x64.zip)
- It is strongly recommend to use dk module together. You can download dk module in this project. `Scripts/dk`

## Required Libraries (Included in this project)
- OpenAL-soft-1.15.1
    - This is LGPL licensed, configured to build **dynamic-library**.
    - Win32, Android only.
    - Mac OS X, iOS use Apple's OpenAL framework.
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
