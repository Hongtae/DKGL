//
//  File: GraphicsAPI.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once

#if defined(__APPLE__) && defined(__MACH__)
#  ifndef DKGL_USE_METAL
#    define DKGL_USE_METAL 1
#  endif
#  ifndef DKGL_USE_VULKAN
#    define DKGL_USE_VULKAN 0
#  endif
#  ifndef DKGL_USE_OPENGL
#    define DKGL_USE_OPENGL 0
#  endif
#ifndef DKGL_USE_DIRECT3D
#    define DKGL_USE_DIRECT3D 0
#  endif
#endif

#ifdef _WIN32
#  ifndef DKGL_USE_DIRECT3D
#    define DKGL_USE_DIRECT3D 1
#  endif
#  ifndef DKGL_USE_VULKAN
#    define DKGL_USE_VULKAN 1
#  endif
#  ifndef DKGL_USE_METAL
#    define DKGL_USE_METAL 0
#  endif
#  ifndef DKGL_USE_OPENGL
#    define DKGL_USE_OPENGL 0
#  endif
#endif

#ifdef __linux__
#  ifndef DKGL_USE_VULKAN
#    define DKGL_USE_VULKAN 1
#  endif
#  ifndef DKGL_USE_OPENGL
#    define DKGL_USE_OPENGL 1
#  endif
#  ifndef DKGL_USE_METAL
#    define DKGL_USE_METAL 0
#  endif
#  ifndef DKGL_USE_DIRECT3D
#    define DKGL_USE_DIRECT3D 0
#  endif
#endif
