# Project DKGL
Cross-platform 3D Game Library for C\++.

# DK (Core Library)
A Lightweighted cross-platform Game Library.  
This is low level game library written with C\++ and supports full multi-threaded, 64-bits environments.
DKGL supports [Khronos Vulkan](https://www.khronos.org/vulkan/), [Apple Metal](https://developer.apple.com/metal/), [Microsoft DirectX 12](https://msdn.microsoft.com/en-us/library/windows/desktop/dn903821.aspx)

You can use Python-script to improve productivity.

DK library has two layers of API level.
##### DKFoundation - `Foundation Classes, Managing System Tasks.`
* C\++ Metaprograms (Type Libraries, etc.)
* Data Collections (Array, Map, Tuples, etc.)
* Memory Management (SmartPointers, WeakRef, Pool Allocators, etc.)
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
* Graphics / Compute Interface (Vulkan, Metal, DirectX12)
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
- Windows 10 or later. (DirectX12 or Vulkan Required)
- Mac OS X 10.11 or later. (Metal Required)
- iOS 9.0 or later. (Metal Required)

For other platforms including linux or android, You can implement
DK interfaces to use DK library with other GUI OSes.
> See interface header files in `DKFramework/Interface`.  
> You can see implementations for other OSes in `DKFramework/Private`

## Build Environments
- Visual Studio 2019 for Windows
- Xcode7 for Mac, iOS
- C\++17 compiler

## Included External Libraries
- [OpenAL-soft-1.15.1](https://github.com/kcat/openal-soft)
    - This is LGPL licensed, configured to build **dynamic-library**.
    - Win32, Android only.
    - Mac OS X, iOS use Apple's OpenAL framework.
- [BulletPhysics-2.82](https://github.com/bulletphysics/bullet3)
- [FreeType-2.9](https://freetype.org/)
- [bzip2-1.0.6](http://sourceware.org/bzip2/)
- [jpeg-9](https://ijg.org/)
- [libFLAC-1.2.1](https://xiph.org/flac/)
- [libogg-1.3.1](https://xiph.org/ogg/)
- [libpng-1.5.10](https://github.com/glennrp/libpng)
- [libvorbis-1.3.3](https://xiph.org/vorbis/)
- [libxml2-2.9.0](http://www.xmlsoft.org/)
- [lz4-r131](https://github.com/lz4/lz4)
- [zlib-1.2.7](https://github.com/madler/zlib)
- [Zstd-1.3.8](https://github.com/facebook/zstd)
- [LZMA-19.00](https://www.7-zip.org/sdk.html)

## Author
Hongtae Kim, (tiff2766@gmail.com)  


## Liense
[BSD-3 License](http://opensource.org/licenses/BSD-3-Clause)
