# Project DK, DKLib
## DK

A Lightweighted cross-platform Game Engine.  
See demo: https://github.com/tiff2766/DKDemo


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
- Android 2.3 or later (Require additional module)
 - submodule for JNI birdge (I'll merge project as soon as possible.)

For other platforms including linux or android, You can implement
DK interfaces to use DK library with other GUI OSes.
> See interface header files in `DKFramework/Interface`.  
> You can see implementations for other OSes in `DKFramework/Private`

## Build Environments
- C+ + , C+ + 11 should be supported at least.
- Visual Studio 2013 for Windows
- Xcode6 for Mac, iOS

## PyDK
Python bindings for DK. (Python 3.4 or later).   
You can build PyDK with or without python interpreter embedded.

## Author
Hongtae Kim, (tiff2766@gmail.com)

## Liense
[MIT License](http://opensource.org/licenses/MIT)
