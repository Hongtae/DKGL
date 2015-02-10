//
//  File: DKOpenGLExtensions.cpp
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32

#include <windows.h>
#include <gl/gl.h>
#include <GL/glu.h>

#include "../../../../lib/OpenGL/glext.h"
#include "../../../../lib/OpenGL/wglext.h"
#include "../../../DKInclude.h"
#include "../../../DKFoundation.h"

namespace OpenGL
{
	////////////////////////////////////////////////////////////////////////////////
	// OpenGL 1.2
	extern PFNGLBLENDCOLORPROC														glBlendColor = 0;
	extern PFNGLBLENDEQUATIONPROC													glBlendEquation = 0;
	extern PFNGLDRAWRANGEELEMENTSPROC												glDrawRangeElements = 0;
	extern PFNGLTEXIMAGE3DPROC														glTexImage3D = 0;
	extern PFNGLTEXSUBIMAGE3DPROC													glTexSubImage3D = 0;
	extern PFNGLCOPYTEXSUBIMAGE3DPROC												glCopyTexSubImage3D = 0;
	// OpenGL 1.3
	extern PFNGLACTIVETEXTUREPROC													glActiveTexture = 0;
	extern PFNGLSAMPLECOVERAGEPROC													glSampleCoverage = 0;
	extern PFNGLCOMPRESSEDTEXIMAGE3DPROC											glCompressedTexImage3D = 0;
	extern PFNGLCOMPRESSEDTEXIMAGE2DPROC											glCompressedTexImage2D = 0;
	extern PFNGLCOMPRESSEDTEXIMAGE1DPROC											glCompressedTexImage1D = 0;
	extern PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC											glCompressedTexSubImage3D = 0;
	extern PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC											glCompressedTexSubImage2D = 0;
	extern PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC											glCompressedTexSubImage1D = 0;
	extern PFNGLGETCOMPRESSEDTEXIMAGEPROC											glGetCompressedTexImage = 0;
	// OpenGL 1.4
	extern PFNGLBLENDFUNCSEPARATEPROC												glBlendFuncSeparate = 0;
	extern PFNGLMULTIDRAWARRAYSPROC													glMultiDrawArrays = 0;
	extern PFNGLMULTIDRAWELEMENTSPROC												glMultiDrawElements = 0;
	extern PFNGLPOINTPARAMETERFPROC													glPointParameterf = 0;
	extern PFNGLPOINTPARAMETERFVPROC												glPointParameterfv = 0;
	extern PFNGLPOINTPARAMETERIPROC													glPointParameteri = 0;
	extern PFNGLPOINTPARAMETERIVPROC												glPointParameteriv = 0;
	// OpenGL 1.5
	extern PFNGLGENQUERIESPROC														glGenQueries = 0;
	extern PFNGLDELETEQUERIESPROC													glDeleteQueries = 0;
	extern PFNGLISQUERYPROC															glIsQuery = 0;
	extern PFNGLBEGINQUERYPROC														glBeginQuery = 0;
	extern PFNGLENDQUERYPROC														glEndQuery = 0;
	extern PFNGLGETQUERYIVPROC														glGetQueryiv = 0;
	extern PFNGLGETQUERYOBJECTIVPROC												glGetQueryObjectiv = 0;
	extern PFNGLGETQUERYOBJECTUIVPROC												glGetQueryObjectuiv = 0;
	extern PFNGLBINDBUFFERPROC														glBindBuffer = 0;
	extern PFNGLDELETEBUFFERSPROC													glDeleteBuffers = 0;
	extern PFNGLGENBUFFERSPROC														glGenBuffers = 0;
	extern PFNGLISBUFFERPROC														glIsBuffer = 0;
	extern PFNGLBUFFERDATAPROC														glBufferData = 0;
	extern PFNGLBUFFERSUBDATAPROC													glBufferSubData = 0;
	extern PFNGLGETBUFFERSUBDATAPROC												glGetBufferSubData = 0;
	extern PFNGLMAPBUFFERPROC														glMapBuffer = 0;
	extern PFNGLUNMAPBUFFERPROC														glUnmapBuffer = 0;
	extern PFNGLGETBUFFERPARAMETERIVPROC											glGetBufferParameteriv = 0;
	extern PFNGLGETBUFFERPOINTERVPROC												glGetBufferPointerv = 0;
	// OpenGL 2.0
	extern PFNGLBLENDEQUATIONSEPARATEPROC											glBlendEquationSeparate = 0;
	extern PFNGLDRAWBUFFERSPROC														glDrawBuffers = 0;
	extern PFNGLSTENCILOPSEPARATEPROC												glStencilOpSeparate = 0;
	extern PFNGLSTENCILFUNCSEPARATEPROC												glStencilFuncSeparate = 0;
	extern PFNGLSTENCILMASKSEPARATEPROC												glStencilMaskSeparate = 0;
	extern PFNGLATTACHSHADERPROC													glAttachShader = 0;
	extern PFNGLBINDATTRIBLOCATIONPROC												glBindAttribLocation = 0;
	extern PFNGLCOMPILESHADERPROC													glCompileShader = 0;
	extern PFNGLCREATEPROGRAMPROC													glCreateProgram = 0;
	extern PFNGLCREATESHADERPROC													glCreateShader = 0;
	extern PFNGLDELETEPROGRAMPROC													glDeleteProgram = 0;
	extern PFNGLDELETESHADERPROC													glDeleteShader = 0;
	extern PFNGLDETACHSHADERPROC													glDetachShader = 0;
	extern PFNGLDISABLEVERTEXATTRIBARRAYPROC										glDisableVertexAttribArray = 0;
	extern PFNGLENABLEVERTEXATTRIBARRAYPROC											glEnableVertexAttribArray = 0;
	extern PFNGLGETACTIVEATTRIBPROC													glGetActiveAttrib = 0;
	extern PFNGLGETACTIVEUNIFORMPROC												glGetActiveUniform = 0;
	extern PFNGLGETATTACHEDSHADERSPROC												glGetAttachedShaders = 0;
	extern PFNGLGETATTRIBLOCATIONPROC												glGetAttribLocation = 0;
	extern PFNGLGETPROGRAMIVPROC													glGetProgramiv = 0;
	extern PFNGLGETPROGRAMINFOLOGPROC												glGetProgramInfoLog = 0;
	extern PFNGLGETSHADERIVPROC														glGetShaderiv = 0;
	extern PFNGLGETSHADERINFOLOGPROC												glGetShaderInfoLog = 0;
	extern PFNGLGETSHADERSOURCEPROC													glGetShaderSource = 0;
	extern PFNGLGETUNIFORMLOCATIONPROC												glGetUniformLocation = 0;
	extern PFNGLGETUNIFORMFVPROC													glGetUniformfv = 0;
	extern PFNGLGETUNIFORMIVPROC													glGetUniformiv = 0;
	extern PFNGLGETVERTEXATTRIBDVPROC												glGetVertexAttribdv = 0;
	extern PFNGLGETVERTEXATTRIBFVPROC												glGetVertexAttribfv = 0;
	extern PFNGLGETVERTEXATTRIBIVPROC												glGetVertexAttribiv = 0;
	extern PFNGLGETVERTEXATTRIBPOINTERVPROC											glGetVertexAttribPointerv = 0;
	extern PFNGLISPROGRAMPROC														glIsProgram = 0;
	extern PFNGLISSHADERPROC														glIsShader = 0;
	extern PFNGLLINKPROGRAMPROC														glLinkProgram = 0;
	extern PFNGLSHADERSOURCEPROC													glShaderSource = 0;
	extern PFNGLUSEPROGRAMPROC														glUseProgram = 0;
	extern PFNGLUNIFORM1FPROC														glUniform1f = 0;
	extern PFNGLUNIFORM2FPROC														glUniform2f = 0;
	extern PFNGLUNIFORM3FPROC														glUniform3f = 0;
	extern PFNGLUNIFORM4FPROC														glUniform4f = 0;
	extern PFNGLUNIFORM1IPROC														glUniform1i = 0;
	extern PFNGLUNIFORM2IPROC														glUniform2i = 0;
	extern PFNGLUNIFORM3IPROC														glUniform3i = 0;
	extern PFNGLUNIFORM4IPROC														glUniform4i = 0;
	extern PFNGLUNIFORM1FVPROC														glUniform1fv = 0;
	extern PFNGLUNIFORM2FVPROC														glUniform2fv = 0;
	extern PFNGLUNIFORM3FVPROC														glUniform3fv = 0;
	extern PFNGLUNIFORM4FVPROC														glUniform4fv = 0;
	extern PFNGLUNIFORM1IVPROC														glUniform1iv = 0;
	extern PFNGLUNIFORM2IVPROC														glUniform2iv = 0;
	extern PFNGLUNIFORM3IVPROC														glUniform3iv = 0;
	extern PFNGLUNIFORM4IVPROC														glUniform4iv = 0;
	extern PFNGLUNIFORMMATRIX2FVPROC												glUniformMatrix2fv = 0;
	extern PFNGLUNIFORMMATRIX3FVPROC												glUniformMatrix3fv = 0;
	extern PFNGLUNIFORMMATRIX4FVPROC												glUniformMatrix4fv = 0;
	extern PFNGLVALIDATEPROGRAMPROC													glValidateProgram = 0;
	extern PFNGLVERTEXATTRIB1DPROC													glVertexAttrib1d = 0;
	extern PFNGLVERTEXATTRIB1DVPROC													glVertexAttrib1dv = 0;
	extern PFNGLVERTEXATTRIB1FPROC													glVertexAttrib1f = 0;
	extern PFNGLVERTEXATTRIB1FVPROC													glVertexAttrib1fv = 0;
	extern PFNGLVERTEXATTRIB1SPROC													glVertexAttrib1s = 0;
	extern PFNGLVERTEXATTRIB1SVPROC													glVertexAttrib1sv = 0;
	extern PFNGLVERTEXATTRIB2DPROC													glVertexAttrib2d = 0;
	extern PFNGLVERTEXATTRIB2DVPROC													glVertexAttrib2dv = 0;
	extern PFNGLVERTEXATTRIB2FPROC													glVertexAttrib2f = 0;
	extern PFNGLVERTEXATTRIB2FVPROC													glVertexAttrib2fv = 0;
	extern PFNGLVERTEXATTRIB2SPROC													glVertexAttrib2s = 0;
	extern PFNGLVERTEXATTRIB2SVPROC													glVertexAttrib2sv = 0;
	extern PFNGLVERTEXATTRIB3DPROC													glVertexAttrib3d = 0;
	extern PFNGLVERTEXATTRIB3DVPROC													glVertexAttrib3dv = 0;
	extern PFNGLVERTEXATTRIB3FPROC													glVertexAttrib3f = 0;
	extern PFNGLVERTEXATTRIB3FVPROC													glVertexAttrib3fv = 0;
	extern PFNGLVERTEXATTRIB3SPROC													glVertexAttrib3s = 0;
	extern PFNGLVERTEXATTRIB3SVPROC													glVertexAttrib3sv = 0;
	extern PFNGLVERTEXATTRIB4NBVPROC												glVertexAttrib4Nbv = 0;
	extern PFNGLVERTEXATTRIB4NIVPROC												glVertexAttrib4Niv = 0;
	extern PFNGLVERTEXATTRIB4NSVPROC												glVertexAttrib4Nsv = 0;
	extern PFNGLVERTEXATTRIB4NUBPROC												glVertexAttrib4Nub = 0;
	extern PFNGLVERTEXATTRIB4NUBVPROC												glVertexAttrib4Nubv = 0;
	extern PFNGLVERTEXATTRIB4NUIVPROC												glVertexAttrib4Nuiv = 0;
	extern PFNGLVERTEXATTRIB4NUSVPROC												glVertexAttrib4Nusv = 0;
	extern PFNGLVERTEXATTRIB4BVPROC													glVertexAttrib4bv = 0;
	extern PFNGLVERTEXATTRIB4DPROC													glVertexAttrib4d = 0;
	extern PFNGLVERTEXATTRIB4DVPROC													glVertexAttrib4dv = 0;
	extern PFNGLVERTEXATTRIB4FPROC													glVertexAttrib4f = 0;
	extern PFNGLVERTEXATTRIB4FVPROC													glVertexAttrib4fv = 0;
	extern PFNGLVERTEXATTRIB4IVPROC													glVertexAttrib4iv = 0;
	extern PFNGLVERTEXATTRIB4SPROC													glVertexAttrib4s = 0;
	extern PFNGLVERTEXATTRIB4SVPROC													glVertexAttrib4sv = 0;
	extern PFNGLVERTEXATTRIB4UBVPROC												glVertexAttrib4ubv = 0;
	extern PFNGLVERTEXATTRIB4UIVPROC												glVertexAttrib4uiv = 0;
	extern PFNGLVERTEXATTRIB4USVPROC												glVertexAttrib4usv = 0;
	extern PFNGLVERTEXATTRIBPOINTERPROC												glVertexAttribPointer = 0;
	// OpenGL 2.1
	extern PFNGLUNIFORMMATRIX2X3FVPROC												glUniformMatrix2x3fv = 0;
	extern PFNGLUNIFORMMATRIX3X2FVPROC												glUniformMatrix3x2fv = 0;
	extern PFNGLUNIFORMMATRIX2X4FVPROC												glUniformMatrix2x4fv = 0;
	extern PFNGLUNIFORMMATRIX4X2FVPROC												glUniformMatrix4x2fv = 0;
	extern PFNGLUNIFORMMATRIX3X4FVPROC												glUniformMatrix3x4fv = 0;
	extern PFNGLUNIFORMMATRIX4X3FVPROC												glUniformMatrix4x3fv = 0;
	// OpenGL 3.0
	extern PFNGLCOLORMASKIPROC														glColorMaski = 0;
	extern PFNGLGETBOOLEANI_VPROC													glGetBooleani_v = 0;
	extern PFNGLGETINTEGERI_VPROC													glGetIntegeri_v = 0;
	extern PFNGLENABLEIPROC															glEnablei = 0;
	extern PFNGLDISABLEIPROC														glDisablei = 0;
	extern PFNGLISENABLEDIPROC														glIsEnabledi = 0;
	extern PFNGLBEGINTRANSFORMFEEDBACKPROC											glBeginTransformFeedback = 0;
	extern PFNGLENDTRANSFORMFEEDBACKPROC											glEndTransformFeedback = 0;
	extern PFNGLBINDBUFFERRANGEPROC													glBindBufferRange = 0;
	extern PFNGLBINDBUFFERBASEPROC													glBindBufferBase = 0;
	extern PFNGLTRANSFORMFEEDBACKVARYINGSPROC										glTransformFeedbackVaryings = 0;
	extern PFNGLGETTRANSFORMFEEDBACKVARYINGPROC										glGetTransformFeedbackVarying = 0;
	extern PFNGLCLAMPCOLORPROC														glClampColor = 0;
	extern PFNGLBEGINCONDITIONALRENDERPROC											glBeginConditionalRender = 0;
	extern PFNGLENDCONDITIONALRENDERPROC											glEndConditionalRender = 0;
	extern PFNGLVERTEXATTRIBIPOINTERPROC											glVertexAttribIPointer = 0;
	extern PFNGLGETVERTEXATTRIBIIVPROC												glGetVertexAttribIiv = 0;
	extern PFNGLGETVERTEXATTRIBIUIVPROC												glGetVertexAttribIuiv = 0;
	extern PFNGLVERTEXATTRIBI1IPROC													glVertexAttribI1i = 0;
	extern PFNGLVERTEXATTRIBI2IPROC													glVertexAttribI2i = 0;
	extern PFNGLVERTEXATTRIBI3IPROC													glVertexAttribI3i = 0;
	extern PFNGLVERTEXATTRIBI4IPROC													glVertexAttribI4i = 0;
	extern PFNGLVERTEXATTRIBI1UIPROC												glVertexAttribI1ui = 0;
	extern PFNGLVERTEXATTRIBI2UIPROC												glVertexAttribI2ui = 0;
	extern PFNGLVERTEXATTRIBI3UIPROC												glVertexAttribI3ui = 0;
	extern PFNGLVERTEXATTRIBI4UIPROC												glVertexAttribI4ui = 0;
	extern PFNGLVERTEXATTRIBI1IVPROC												glVertexAttribI1iv = 0;
	extern PFNGLVERTEXATTRIBI2IVPROC												glVertexAttribI2iv = 0;
	extern PFNGLVERTEXATTRIBI3IVPROC												glVertexAttribI3iv = 0;
	extern PFNGLVERTEXATTRIBI4IVPROC												glVertexAttribI4iv = 0;
	extern PFNGLVERTEXATTRIBI1UIVPROC												glVertexAttribI1uiv = 0;
	extern PFNGLVERTEXATTRIBI2UIVPROC												glVertexAttribI2uiv = 0;
	extern PFNGLVERTEXATTRIBI3UIVPROC												glVertexAttribI3uiv = 0;
	extern PFNGLVERTEXATTRIBI4UIVPROC												glVertexAttribI4uiv = 0;
	extern PFNGLVERTEXATTRIBI4BVPROC												glVertexAttribI4bv = 0;
	extern PFNGLVERTEXATTRIBI4SVPROC												glVertexAttribI4sv = 0;
	extern PFNGLVERTEXATTRIBI4UBVPROC												glVertexAttribI4ubv = 0;
	extern PFNGLVERTEXATTRIBI4USVPROC												glVertexAttribI4usv = 0;
	extern PFNGLGETUNIFORMUIVPROC													glGetUniformuiv = 0;
	extern PFNGLBINDFRAGDATALOCATIONPROC											glBindFragDataLocation = 0;
	extern PFNGLGETFRAGDATALOCATIONPROC												glGetFragDataLocation = 0;
	extern PFNGLUNIFORM1UIPROC														glUniform1ui = 0;
	extern PFNGLUNIFORM2UIPROC														glUniform2ui = 0;
	extern PFNGLUNIFORM3UIPROC														glUniform3ui = 0;
	extern PFNGLUNIFORM4UIPROC														glUniform4ui = 0;
	extern PFNGLUNIFORM1UIVPROC														glUniform1uiv = 0;
	extern PFNGLUNIFORM2UIVPROC														glUniform2uiv = 0;
	extern PFNGLUNIFORM3UIVPROC														glUniform3uiv = 0;
	extern PFNGLUNIFORM4UIVPROC														glUniform4uiv = 0;
	extern PFNGLTEXPARAMETERIIVPROC													glTexParameterIiv = 0;
	extern PFNGLTEXPARAMETERIUIVPROC												glTexParameterIuiv = 0;
	extern PFNGLGETTEXPARAMETERIIVPROC												glGetTexParameterIiv = 0;
	extern PFNGLGETTEXPARAMETERIUIVPROC												glGetTexParameterIuiv = 0;
	extern PFNGLCLEARBUFFERIVPROC													glClearBufferiv = 0;
	extern PFNGLCLEARBUFFERUIVPROC													glClearBufferuiv = 0;
	extern PFNGLCLEARBUFFERFVPROC													glClearBufferfv = 0;
	extern PFNGLCLEARBUFFERFIPROC													glClearBufferfi = 0;
	extern PFNGLGETSTRINGIPROC														glGetStringi = 0;
	// OpenGL 3.1
	extern PFNGLDRAWARRAYSINSTANCEDPROC												glDrawArraysInstanced = 0;
	extern PFNGLDRAWELEMENTSINSTANCEDPROC											glDrawElementsInstanced = 0;
	extern PFNGLTEXBUFFERPROC														glTexBuffer = 0;
	extern PFNGLPRIMITIVERESTARTINDEXPROC											glPrimitiveRestartIndex = 0;
	// OpenGL 3.2
	extern PFNGLGETINTEGER64I_VPROC													glGetInteger64i_v = 0;
	extern PFNGLGETBUFFERPARAMETERI64VPROC											glGetBufferParameteri64v = 0;
	extern PFNGLPROGRAMPARAMETERIPROC												glProgramParameteri = 0;
	extern PFNGLFRAMEBUFFERTEXTUREPROC												glFramebufferTexture = 0;
	extern PFNGLFRAMEBUFFERTEXTUREFACEPROC											glFramebufferTextureFace = 0;
	// GL_ARB_multitexture
	extern PFNGLACTIVETEXTUREARBPROC												glActiveTextureARB = 0;			
	extern PFNGLCLIENTACTIVETEXTUREARBPROC											glClientActiveTextureARB = 0;	
	extern PFNGLMULTITEXCOORD1DARBPROC												glMultiTexCoord1dARB = 0;		
	extern PFNGLMULTITEXCOORD1DVARBPROC												glMultiTexCoord1dvARB = 0;		
	extern PFNGLMULTITEXCOORD1FARBPROC												glMultiTexCoord1fARB = 0;		
	extern PFNGLMULTITEXCOORD1FVARBPROC												glMultiTexCoord1fvARB = 0;		
	extern PFNGLMULTITEXCOORD1IARBPROC												glMultiTexCoord1iARB = 0; 
	extern PFNGLMULTITEXCOORD1IVARBPROC												glMultiTexCoord1ivARB = 0;
	extern PFNGLMULTITEXCOORD1SARBPROC 												glMultiTexCoord1sARB = 0; 
	extern PFNGLMULTITEXCOORD1SVARBPROC												glMultiTexCoord1svARB = 0;
	extern PFNGLMULTITEXCOORD2DARBPROC 												glMultiTexCoord2dARB = 0; 
	extern PFNGLMULTITEXCOORD2DVARBPROC												glMultiTexCoord2dvARB = 0;
	extern PFNGLMULTITEXCOORD2FARBPROC 												glMultiTexCoord2fARB = 0; 
	extern PFNGLMULTITEXCOORD2FVARBPROC												glMultiTexCoord2fvARB = 0;
	extern PFNGLMULTITEXCOORD2IARBPROC 												glMultiTexCoord2iARB = 0; 
	extern PFNGLMULTITEXCOORD2IVARBPROC												glMultiTexCoord2ivARB = 0;
	extern PFNGLMULTITEXCOORD2SARBPROC 												glMultiTexCoord2sARB = 0; 
	extern PFNGLMULTITEXCOORD2SVARBPROC												glMultiTexCoord2svARB = 0;
	extern PFNGLMULTITEXCOORD3DARBPROC 												glMultiTexCoord3dARB = 0; 
	extern PFNGLMULTITEXCOORD3DVARBPROC												glMultiTexCoord3dvARB = 0;
	extern PFNGLMULTITEXCOORD3FARBPROC 												glMultiTexCoord3fARB = 0; 
	extern PFNGLMULTITEXCOORD3FVARBPROC												glMultiTexCoord3fvARB = 0;
	extern PFNGLMULTITEXCOORD3IARBPROC 												glMultiTexCoord3iARB = 0; 
	extern PFNGLMULTITEXCOORD3IVARBPROC												glMultiTexCoord3ivARB = 0;
	extern PFNGLMULTITEXCOORD3SARBPROC 												glMultiTexCoord3sARB = 0; 
	extern PFNGLMULTITEXCOORD3SVARBPROC												glMultiTexCoord3svARB = 0;
	extern PFNGLMULTITEXCOORD4DARBPROC 												glMultiTexCoord4dARB = 0; 
	extern PFNGLMULTITEXCOORD4DVARBPROC												glMultiTexCoord4dvARB = 0;
	extern PFNGLMULTITEXCOORD4FARBPROC 												glMultiTexCoord4fARB = 0; 
	extern PFNGLMULTITEXCOORD4FVARBPROC												glMultiTexCoord4fvARB = 0;
	extern PFNGLMULTITEXCOORD4IARBPROC 												glMultiTexCoord4iARB = 0; 
	extern PFNGLMULTITEXCOORD4IVARBPROC												glMultiTexCoord4ivARB = 0;
	extern PFNGLMULTITEXCOORD4SARBPROC 												glMultiTexCoord4sARB = 0; 
	extern PFNGLMULTITEXCOORD4SVARBPROC												glMultiTexCoord4svARB = 0;
	// GL_ARB_transpose_matrix
	extern PFNGLLOADTRANSPOSEMATRIXFARBPROC											glLoadTransposeMatrixfARB = 0;
	extern PFNGLLOADTRANSPOSEMATRIXDARBPROC											glLoadTransposeMatrixdARB = 0;
	extern PFNGLMULTTRANSPOSEMATRIXFARBPROC											glMultTransposeMatrixfARB = 0;
	extern PFNGLMULTTRANSPOSEMATRIXDARBPROC											glMultTransposeMatrixdARB = 0;
	// GL_ARB_multisample
	extern PFNGLSAMPLECOVERAGEARBPROC												glSampleCoverageARB = 0;
	// GL_ARB_texture_compression
	extern PFNGLCOMPRESSEDTEXIMAGE3DARBPROC											glCompressedTexImage3DARB = 0;
	extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC											glCompressedTexImage2DARB = 0;
	extern PFNGLCOMPRESSEDTEXIMAGE1DARBPROC											glCompressedTexImage1DARB = 0;
	extern PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC										glCompressedTexSubImage3DARB = 0;
	extern PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC										glCompressedTexSubImage2DARB = 0;
	extern PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC										glCompressedTexSubImage1DARB = 0;
	extern PFNGLGETCOMPRESSEDTEXIMAGEARBPROC										glGetCompressedTexImageARB = 0;
	// GL_ARB_point_parameters
	extern PFNGLPOINTPARAMETERFARBPROC												glPointParameterfARB = 0;
	extern PFNGLPOINTPARAMETERFVARBPROC												glPointParameterfvARB = 0;
	// GL_ARB_vertex_blend
	extern PFNGLWEIGHTBVARBPROC														glWeightbvARB = 0;
	extern PFNGLWEIGHTSVARBPROC														glWeightsvARB = 0;
	extern PFNGLWEIGHTIVARBPROC														glWeightivARB = 0;
	extern PFNGLWEIGHTFVARBPROC														glWeightfvARB = 0;
	extern PFNGLWEIGHTDVARBPROC														glWeightdvARB = 0;
	extern PFNGLWEIGHTUBVARBPROC													glWeightubvARB = 0;
	extern PFNGLWEIGHTUSVARBPROC													glWeightusvARB = 0;
	extern PFNGLWEIGHTUIVARBPROC													glWeightuivARB = 0;
	extern PFNGLWEIGHTPOINTERARBPROC												glWeightPointerARB = 0;
	extern PFNGLVERTEXBLENDARBPROC													glVertexBlendARB = 0;
	// GL_ARB_matrix_palette
	extern PFNGLCURRENTPALETTEMATRIXARBPROC											glCurrentPaletteMatrixARB = 0;
	extern PFNGLMATRIXINDEXUBVARBPROC												glMatrixIndexubvARB = 0;
	extern PFNGLMATRIXINDEXUSVARBPROC												glMatrixIndexusvARB = 0;
	extern PFNGLMATRIXINDEXUIVARBPROC												glMatrixIndexuivARB = 0;
	extern PFNGLMATRIXINDEXPOINTERARBPROC											glMatrixIndexPointerARB = 0;
	// GL_ARB_window_pos
	extern PFNGLWINDOWPOS2DARBPROC													glWindowPos2dARB = 0; 
	extern PFNGLWINDOWPOS2DVARBPROC													glWindowPos2dvARB = 0;
	extern PFNGLWINDOWPOS2FARBPROC													glWindowPos2fARB = 0; 
	extern PFNGLWINDOWPOS2FVARBPROC													glWindowPos2fvARB = 0;
	extern PFNGLWINDOWPOS2IARBPROC													glWindowPos2iARB = 0; 
	extern PFNGLWINDOWPOS2IVARBPROC													glWindowPos2ivARB = 0;
	extern PFNGLWINDOWPOS2SARBPROC													glWindowPos2sARB = 0; 
	extern PFNGLWINDOWPOS2SVARBPROC													glWindowPos2svARB = 0;
	extern PFNGLWINDOWPOS3DARBPROC													glWindowPos3dARB = 0; 
	extern PFNGLWINDOWPOS3DVARBPROC													glWindowPos3dvARB = 0;
	extern PFNGLWINDOWPOS3FARBPROC													glWindowPos3fARB = 0; 
	extern PFNGLWINDOWPOS3FVARBPROC													glWindowPos3fvARB = 0;
	extern PFNGLWINDOWPOS3IARBPROC													glWindowPos3iARB = 0; 
	extern PFNGLWINDOWPOS3IVARBPROC													glWindowPos3ivARB = 0;
	extern PFNGLWINDOWPOS3SARBPROC													glWindowPos3sARB = 0; 
	extern PFNGLWINDOWPOS3SVARBPROC													glWindowPos3svARB = 0;
	// GL_ARB_vertex_program
	extern PFNGLVERTEXATTRIB1DARBPROC             									glVertexAttrib1dARB = 0;                                              
	extern PFNGLVERTEXATTRIB1DVARBPROC            									glVertexAttrib1dvARB = 0;                                             
	extern PFNGLVERTEXATTRIB1FARBPROC             									glVertexAttrib1fARB = 0;                                              
	extern PFNGLVERTEXATTRIB1FVARBPROC            									glVertexAttrib1fvARB = 0;                                             
	extern PFNGLVERTEXATTRIB1SARBPROC             									glVertexAttrib1sARB = 0;                                              
	extern PFNGLVERTEXATTRIB1SVARBPROC            									glVertexAttrib1svARB = 0;                                             
	extern PFNGLVERTEXATTRIB2DARBPROC             									glVertexAttrib2dARB = 0;                                              
	extern PFNGLVERTEXATTRIB2DVARBPROC            									glVertexAttrib2dvARB = 0;                                             
	extern PFNGLVERTEXATTRIB2FARBPROC             									glVertexAttrib2fARB = 0;                                              
	extern PFNGLVERTEXATTRIB2FVARBPROC            									glVertexAttrib2fvARB = 0;                                             
	extern PFNGLVERTEXATTRIB2SARBPROC             									glVertexAttrib2sARB = 0;                                              
	extern PFNGLVERTEXATTRIB2SVARBPROC            									glVertexAttrib2svARB = 0;                                             
	extern PFNGLVERTEXATTRIB3DARBPROC             									glVertexAttrib3dARB = 0;                                              
	extern PFNGLVERTEXATTRIB3DVARBPROC            									glVertexAttrib3dvARB = 0;                                             
	extern PFNGLVERTEXATTRIB3FARBPROC             									glVertexAttrib3fARB = 0;                                              
	extern PFNGLVERTEXATTRIB3FVARBPROC            									glVertexAttrib3fvARB = 0;                                             
	extern PFNGLVERTEXATTRIB3SARBPROC             									glVertexAttrib3sARB = 0;                                              
	extern PFNGLVERTEXATTRIB3SVARBPROC            									glVertexAttrib3svARB = 0;                                             
	extern PFNGLVERTEXATTRIB4NBVARBPROC           									glVertexAttrib4NbvARB = 0;                                            
	extern PFNGLVERTEXATTRIB4NIVARBPROC           									glVertexAttrib4NivARB = 0;                                            
	extern PFNGLVERTEXATTRIB4NSVARBPROC           									glVertexAttrib4NsvARB = 0;                                            
	extern PFNGLVERTEXATTRIB4NUBARBPROC           									glVertexAttrib4NubARB = 0;                                            
	extern PFNGLVERTEXATTRIB4NUBVARBPROC          									glVertexAttrib4NubvARB = 0;                                           
	extern PFNGLVERTEXATTRIB4NUIVARBPROC          									glVertexAttrib4NuivARB = 0;                                           
	extern PFNGLVERTEXATTRIB4NUSVARBPROC          									glVertexAttrib4NusvARB = 0;                                           
	extern PFNGLVERTEXATTRIB4BVARBPROC            									glVertexAttrib4bvARB = 0;                                             
	extern PFNGLVERTEXATTRIB4DARBPROC             									glVertexAttrib4dARB = 0;                                              
	extern PFNGLVERTEXATTRIB4DVARBPROC            									glVertexAttrib4dvARB = 0;                                             
	extern PFNGLVERTEXATTRIB4FARBPROC             									glVertexAttrib4fARB = 0;                                              
	extern PFNGLVERTEXATTRIB4FVARBPROC            									glVertexAttrib4fvARB = 0;                                             
	extern PFNGLVERTEXATTRIB4IVARBPROC            									glVertexAttrib4ivARB = 0;                                             
	extern PFNGLVERTEXATTRIB4SARBPROC             									glVertexAttrib4sARB = 0;                                              
	extern PFNGLVERTEXATTRIB4SVARBPROC            									glVertexAttrib4svARB = 0;                                             
	extern PFNGLVERTEXATTRIB4UBVARBPROC           									glVertexAttrib4ubvARB = 0;                                            
	extern PFNGLVERTEXATTRIB4UIVARBPROC           									glVertexAttrib4uivARB = 0;                                            
	extern PFNGLVERTEXATTRIB4USVARBPROC           									glVertexAttrib4usvARB = 0;                                            
	extern PFNGLVERTEXATTRIBPOINTERARBPROC        									glVertexAttribPointerARB = 0;                                         
	extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC    									glEnableVertexAttribArrayARB = 0;                                     
	extern PFNGLDISABLEVERTEXATTRIBARRAYARBPROC   									glDisableVertexAttribArrayARB = 0;                                    
	extern PFNGLPROGRAMSTRINGARBPROC              									glProgramStringARB = 0;                                               
	extern PFNGLBINDPROGRAMARBPROC                									glBindProgramARB = 0;                                                 
	extern PFNGLDELETEPROGRAMSARBPROC             									glDeleteProgramsARB = 0;                                              
	extern PFNGLGENPROGRAMSARBPROC                									glGenProgramsARB = 0;                                                 
	extern PFNGLPROGRAMENVPARAMETER4DARBPROC      									glProgramEnvParameter4dARB = 0;                                       
	extern PFNGLPROGRAMENVPARAMETER4DVARBPROC     									glProgramEnvParameter4dvARB = 0;                                      
	extern PFNGLPROGRAMENVPARAMETER4FARBPROC      									glProgramEnvParameter4fARB = 0;                                       
	extern PFNGLPROGRAMENVPARAMETER4FVARBPROC     									glProgramEnvParameter4fvARB = 0;                                      
	extern PFNGLPROGRAMLOCALPARAMETER4DARBPROC    									glProgramLocalParameter4dARB = 0;                                     
	extern PFNGLPROGRAMLOCALPARAMETER4DVARBPROC   									glProgramLocalParameter4dvARB = 0;                                    
	extern PFNGLPROGRAMLOCALPARAMETER4FARBPROC    									glProgramLocalParameter4fARB = 0;                                     
	extern PFNGLPROGRAMLOCALPARAMETER4FVARBPROC   									glProgramLocalParameter4fvARB = 0;                                    
	extern PFNGLGETPROGRAMENVPARAMETERDVARBPROC   									glGetProgramEnvParameterdvARB = 0;                                    
	extern PFNGLGETPROGRAMENVPARAMETERFVARBPROC   									glGetProgramEnvParameterfvARB = 0;                                    
	extern PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC 									glGetProgramLocalParameterdvARB = 0;                                  
	extern PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC 									glGetProgramLocalParameterfvARB = 0;                                  
	extern PFNGLGETPROGRAMIVARBPROC               									glGetProgramivARB = 0;                                                
	extern PFNGLGETPROGRAMSTRINGARBPROC           									glGetProgramStringARB = 0;                                            
	extern PFNGLGETVERTEXATTRIBDVARBPROC          									glGetVertexAttribdvARB = 0;                                           
	extern PFNGLGETVERTEXATTRIBFVARBPROC          									glGetVertexAttribfvARB = 0;                                           
	extern PFNGLGETVERTEXATTRIBIVARBPROC          									glGetVertexAttribivARB = 0;                                           
	extern PFNGLGETVERTEXATTRIBPOINTERVARBPROC    									glGetVertexAttribPointervARB = 0;                                     
	extern PFNGLISPROGRAMARBPROC                  									glIsProgramARB = 0;
	// GL_ARB_vertex_buffer_object
	extern PFNGLBINDBUFFERARBPROC													glBindBufferARB = 0;
	extern PFNGLDELETEBUFFERSARBPROC												glDeleteBuffersARB = 0;
	extern PFNGLGENBUFFERSARBPROC													glGenBuffersARB = 0;
	extern PFNGLISBUFFERARBPROC														glIsBufferARB = 0;
	extern PFNGLBUFFERDATAARBPROC													glBufferDataARB = 0;
	extern PFNGLBUFFERSUBDATAARBPROC												glBufferSubDataARB = 0;
	extern PFNGLGETBUFFERSUBDATAARBPROC												glGetBufferSubDataARB = 0;
	extern PFNGLMAPBUFFERARBPROC													glMapBufferARB = 0;
	extern PFNGLUNMAPBUFFERARBPROC													glUnmapBufferARB = 0;
	extern PFNGLGETBUFFERPARAMETERIVARBPROC											glGetBufferParameterivARB = 0;
	extern PFNGLGETBUFFERPOINTERVARBPROC											glGetBufferPointervARB = 0;
	// GL_ARB_occlusion_query
	extern PFNGLGENQUERIESARBPROC													glGenQueriesARB = 0;
	extern PFNGLDELETEQUERIESARBPROC												glDeleteQueriesARB = 0;
	extern PFNGLISQUERYARBPROC														glIsQueryARB = 0;
	extern PFNGLBEGINQUERYARBPROC													glBeginQueryARB = 0;
	extern PFNGLENDQUERYARBPROC														glEndQueryARB = 0;
	extern PFNGLGETQUERYIVARBPROC													glGetQueryivARB = 0;
	extern PFNGLGETQUERYOBJECTIVARBPROC												glGetQueryObjectivARB = 0;
	extern PFNGLGETQUERYOBJECTUIVARBPROC											glGetQueryObjectuivARB = 0;
	// GL_ARB_shader_objects
	extern PFNGLDELETEOBJECTARBPROC													glDeleteObjectARB = 0;			
	extern PFNGLGETHANDLEARBPROC													glGetHandleARB = 0;				
	extern PFNGLDETACHOBJECTARBPROC													glDetachObjectARB = 0;			
	extern PFNGLCREATESHADEROBJECTARBPROC											glCreateShaderObjectARB = 0;		
	extern PFNGLSHADERSOURCEARBPROC													glShaderSourceARB = 0;			
	extern PFNGLCOMPILESHADERARBPROC												glCompileShaderARB = 0;			
	extern PFNGLCREATEPROGRAMOBJECTARBPROC											glCreateProgramObjectARB = 0;	
	extern PFNGLATTACHOBJECTARBPROC													glAttachObjectARB = 0;			
	extern PFNGLLINKPROGRAMARBPROC													glLinkProgramARB = 0;			
	extern PFNGLUSEPROGRAMOBJECTARBPROC												glUseProgramObjectARB = 0;		
	extern PFNGLVALIDATEPROGRAMARBPROC												glValidateProgramARB = 0;		
	extern PFNGLUNIFORM1FARBPROC													glUniform1fARB = 0;			 
	extern PFNGLUNIFORM2FARBPROC													glUniform2fARB = 0; 				
	extern PFNGLUNIFORM3FARBPROC													glUniform3fARB = 0; 				
	extern PFNGLUNIFORM4FARBPROC													glUniform4fARB = 0; 				
	extern PFNGLUNIFORM1IARBPROC													glUniform1iARB = 0; 				
	extern PFNGLUNIFORM2IARBPROC													glUniform2iARB = 0; 				
	extern PFNGLUNIFORM3IARBPROC													glUniform3iARB = 0; 				
	extern PFNGLUNIFORM4IARBPROC													glUniform4iARB = 0; 				
	extern PFNGLUNIFORM1FVARBPROC													glUniform1fvARB = 0;				
	extern PFNGLUNIFORM2FVARBPROC													glUniform2fvARB = 0;				
	extern PFNGLUNIFORM3FVARBPROC													glUniform3fvARB = 0;				
	extern PFNGLUNIFORM4FVARBPROC													glUniform4fvARB = 0;				
	extern PFNGLUNIFORM1IVARBPROC													glUniform1ivARB = 0;				
	extern PFNGLUNIFORM2IVARBPROC													glUniform2ivARB = 0;				
	extern PFNGLUNIFORM3IVARBPROC													glUniform3ivARB = 0;				
	extern PFNGLUNIFORM4IVARBPROC													glUniform4ivARB = 0;				
	extern PFNGLUNIFORMMATRIX2FVARBPROC												glUniformMatrix2fvARB = 0;		
	extern PFNGLUNIFORMMATRIX3FVARBPROC												glUniformMatrix3fvARB = 0;		
	extern PFNGLUNIFORMMATRIX4FVARBPROC												glUniformMatrix4fvARB = 0;		
	extern PFNGLGETOBJECTPARAMETERFVARBPROC											glGetObjectParameterfvARB = 0; 	
	extern PFNGLGETOBJECTPARAMETERIVARBPROC											glGetObjectParameterivARB = 0; 	
	extern PFNGLGETINFOLOGARBPROC													glGetInfoLogARB = 0;				
	extern PFNGLGETATTACHEDOBJECTSARBPROC											glGetAttachedObjectsARB = 0; 	
	extern PFNGLGETUNIFORMLOCATIONARBPROC											glGetUniformLocationARB = 0; 	
	extern PFNGLGETACTIVEUNIFORMARBPROC												glGetActiveUniformARB = 0;		
	extern PFNGLGETUNIFORMFVARBPROC													glGetUniformfvARB = 0;			
	extern PFNGLGETUNIFORMIVARBPROC													glGetUniformivARB = 0;			
	extern PFNGLGETSHADERSOURCEARBPROC												glGetShaderSourceARB = 0;
	// GL_ARB_vertex_shader
	extern PFNGLBINDATTRIBLOCATIONARBPROC											glBindAttribLocationARB = 0;
	extern PFNGLGETACTIVEATTRIBARBPROC												glGetActiveAttribARB = 0;	
	extern PFNGLGETATTRIBLOCATIONARBPROC											glGetAttribLocationARB = 0;
	// GL_ARB_draw_buffers
	extern PFNGLDRAWBUFFERSARBPROC													glDrawBuffersARB = 0;
	// GL_ARB_color_buffer_float
	extern PFNGLCLAMPCOLORARBPROC													glClampColorARB = 0;
	// GL_ARB_draw_instanced
	extern PFNGLDRAWARRAYSINSTANCEDARBPROC											glDrawArraysInstancedARB = 0;
	extern PFNGLDRAWELEMENTSINSTANCEDARBPROC										glDrawElementsInstancedARB = 0;
	// GL_ARB_framebuffer_object
	extern PFNGLISRENDERBUFFERPROC													glIsRenderbuffer = 0;
	extern PFNGLBINDRENDERBUFFERPROC												glBindRenderbuffer = 0;
	extern PFNGLDELETERENDERBUFFERSPROC												glDeleteRenderbuffers = 0;
	extern PFNGLGENRENDERBUFFERSPROC												glGenRenderbuffers = 0;
	extern PFNGLRENDERBUFFERSTORAGEPROC												glRenderbufferStorage = 0;
	extern PFNGLGETRENDERBUFFERPARAMETERIVPROC										glGetRenderbufferParameteriv = 0;
	extern PFNGLISFRAMEBUFFERPROC													glIsFramebuffer = 0;
	extern PFNGLBINDFRAMEBUFFERPROC													glBindFramebuffer = 0;
	extern PFNGLDELETEFRAMEBUFFERSPROC												glDeleteFramebuffers = 0;
	extern PFNGLGENFRAMEBUFFERSPROC													glGenFramebuffers = 0;
	extern PFNGLCHECKFRAMEBUFFERSTATUSPROC											glCheckFramebufferStatus = 0;
	extern PFNGLFRAMEBUFFERTEXTURE1DPROC											glFramebufferTexture1D = 0;
	extern PFNGLFRAMEBUFFERTEXTURE2DPROC											glFramebufferTexture2D = 0;
	extern PFNGLFRAMEBUFFERTEXTURE3DPROC											glFramebufferTexture3D = 0;
	extern PFNGLFRAMEBUFFERRENDERBUFFERPROC											glFramebufferRenderbuffer = 0;
	extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC								glGetFramebufferAttachmentParameteriv = 0;
	extern PFNGLGENERATEMIPMAPPROC													glGenerateMipmap = 0;
	extern PFNGLBLITFRAMEBUFFERPROC													glBlitFramebuffer = 0;
	extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC									glRenderbufferStorageMultisample = 0;
	extern PFNGLFRAMEBUFFERTEXTURELAYERPROC											glFramebufferTextureLayer = 0;
	// GL_ARB_geometry_shader4
	extern PFNGLPROGRAMPARAMETERIARBPROC											glProgramParameteriARB = 0;
	extern PFNGLFRAMEBUFFERTEXTUREARBPROC											glFramebufferTextureARB = 0;
	extern PFNGLFRAMEBUFFERTEXTURELAYERARBPROC										glFramebufferTextureLayerARB = 0;
	extern PFNGLFRAMEBUFFERTEXTUREFACEARBPROC										glFramebufferTextureFaceARB = 0;
	// GL_ARB_instanced_arrays
	extern PFNGLVERTEXATTRIBDIVISORARBPROC											glVertexAttribDivisorARB = 0;
	// GL_ARB_map_buffer_range
	extern PFNGLMAPBUFFERRANGEPROC													glMapBufferRange = 0;
	extern PFNGLFLUSHMAPPEDBUFFERRANGEPROC											glFlushMappedBufferRange = 0;
	// GL_ARB_texture_buffer_object
	extern PFNGLTEXBUFFERARBPROC													glTexBufferARB = 0;
	// GL_ARB_vertex_array_object
	extern PFNGLBINDVERTEXARRAYPROC													glBindVertexArray = 0;
	extern PFNGLDELETEVERTEXARRAYSPROC												glDeleteVertexArrays = 0;
	extern PFNGLGENVERTEXARRAYSPROC													glGenVertexArrays = 0;
	extern PFNGLISVERTEXARRAYPROC													glIsVertexArray = 0;
	// GL_ARB_uniform_buffer_object
	extern PFNGLGETUNIFORMINDICESPROC												glGetUniformIndices = 0;
	extern PFNGLGETACTIVEUNIFORMSIVPROC												glGetActiveUniformsiv = 0;
	extern PFNGLGETACTIVEUNIFORMNAMEPROC											glGetActiveUniformName = 0;
	extern PFNGLGETUNIFORMBLOCKINDEXPROC											glGetUniformBlockIndex = 0;
	extern PFNGLGETACTIVEUNIFORMBLOCKIVPROC											glGetActiveUniformBlockiv = 0;
	extern PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC										glGetActiveUniformBlockName = 0;
	extern PFNGLUNIFORMBLOCKBINDINGPROC												glUniformBlockBinding = 0;
	// GL_ARB_copy_buffer
	extern PFNGLCOPYBUFFERSUBDATAPROC												glCopyBufferSubData = 0;
	// GL_ARB_draw_elements_base_vertex
	extern PFNGLDRAWELEMENTSBASEVERTEXPROC											glDrawElementsBaseVertex = 0;
	extern PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC										glDrawRangeElementsBaseVertex = 0;
	extern PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC									glDrawElementsInstancedBaseVertex = 0;
	extern PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC										glMultiDrawElementsBaseVertex = 0;
	// GL_ARB_provoking_vertex
	extern PFNGLPROVOKINGVERTEXPROC													glProvokingVertex = 0;
	// GL_ARB_sync
	extern PFNGLFENCESYNCPROC														glFenceSync = 0;
	extern PFNGLISSYNCPROC															glIsSync = 0;
	extern PFNGLDELETESYNCPROC														glDeleteSync = 0;
	extern PFNGLCLIENTWAITSYNCPROC													glClientWaitSync = 0;
	extern PFNGLWAITSYNCPROC														glWaitSync = 0;
	extern PFNGLGETINTEGER64VPROC													glGetInteger64v = 0;
	extern PFNGLGETSYNCIVPROC														glGetSynciv = 0;
	// GL_ARB_texture_multisample
	extern PFNGLTEXIMAGE2DMULTISAMPLEPROC											glTexImage2DMultisample = 0;
	extern PFNGLTEXIMAGE3DMULTISAMPLEPROC											glTexImage3DMultisample = 0;
	extern PFNGLGETMULTISAMPLEFVPROC												glGetMultisamplefv = 0;
	extern PFNGLSAMPLEMASKIPROC														glSampleMaski = 0;
	// GL_ARB_draw_buffers_blend
	extern PFNGLBLENDEQUATIONIPROC													glBlendEquationi = 0;
	extern PFNGLBLENDEQUATIONSEPARATEIPROC											glBlendEquationSeparatei = 0;
	extern PFNGLBLENDFUNCIPROC														glBlendFunci = 0;
	extern PFNGLBLENDFUNCSEPARATEIPROC												glBlendFuncSeparatei = 0;
	// GL_ARB_sample_shading
	extern PFNGLMINSAMPLESHADINGPROC												glMinSampleShading = 0;
	// GL_EXT_blend_color
	extern PFNGLBLENDCOLOREXTPROC													glBlendColorEXT = 0;
	// GL_EXT_polygon_offset
	extern PFNGLPOLYGONOFFSETEXTPROC												glPolygonOffsetEXT = 0;
	// GL_EXT_texture3D
	extern PFNGLTEXIMAGE3DEXTPROC													glTexImage3DEXT = 0;
	extern PFNGLTEXSUBIMAGE3DEXTPROC												glTexSubImage3DEXT = 0;
	// GL_SGIS_texture_filter4
	extern PFNGLGETTEXFILTERFUNCSGISPROC											glGetTexFilterFuncSGIS = 0;
	extern PFNGLTEXFILTERFUNCSGISPROC												glTexFilterFuncSGIS = 0;
	// GL_EXT_subtexture
	extern PFNGLTEXSUBIMAGE1DEXTPROC												glTexSubImage1DEXT = 0;
	extern PFNGLTEXSUBIMAGE2DEXTPROC												glTexSubImage2DEXT = 0;
	// GL_EXT_copy_texture
	extern PFNGLCOPYTEXIMAGE1DEXTPROC												glCopyTexImage1DEXT = 0;
	extern PFNGLCOPYTEXIMAGE2DEXTPROC												glCopyTexImage2DEXT = 0;
	extern PFNGLCOPYTEXSUBIMAGE1DEXTPROC											glCopyTexSubImage1DEXT = 0;
	extern PFNGLCOPYTEXSUBIMAGE2DEXTPROC											glCopyTexSubImage2DEXT = 0;
	extern PFNGLCOPYTEXSUBIMAGE3DEXTPROC											glCopyTexSubImage3DEXT = 0;
	// GL_EXT_histogram
	extern PFNGLGETHISTOGRAMEXTPROC													glGetHistogramEXT = 0;
	extern PFNGLGETHISTOGRAMPARAMETERFVEXTPROC										glGetHistogramParameterfvEXT = 0;
	extern PFNGLGETHISTOGRAMPARAMETERIVEXTPROC										glGetHistogramParameterivEXT = 0;
	extern PFNGLGETMINMAXEXTPROC													glGetMinmaxEXT = 0;
	extern PFNGLGETMINMAXPARAMETERFVEXTPROC											glGetMinmaxParameterfvEXT = 0;
	extern PFNGLGETMINMAXPARAMETERIVEXTPROC											glGetMinmaxParameterivEXT = 0;
	extern PFNGLHISTOGRAMEXTPROC													glHistogramEXT = 0;
	extern PFNGLMINMAXEXTPROC														glMinmaxEXT = 0;
	extern PFNGLRESETHISTOGRAMEXTPROC												glResetHistogramEXT = 0;
	extern PFNGLRESETMINMAXEXTPROC													glResetMinmaxEXT = 0;
	// GL_EXT_convolution
	extern PFNGLCONVOLUTIONFILTER1DEXTPROC											glConvolutionFilter1DEXT = 0;
	extern PFNGLCONVOLUTIONFILTER2DEXTPROC											glConvolutionFilter2DEXT = 0;
	extern PFNGLCONVOLUTIONPARAMETERFEXTPROC										glConvolutionParameterfEXT = 0;
	extern PFNGLCONVOLUTIONPARAMETERFVEXTPROC										glConvolutionParameterfvEXT = 0;
	extern PFNGLCONVOLUTIONPARAMETERIEXTPROC										glConvolutionParameteriEXT = 0;
	extern PFNGLCONVOLUTIONPARAMETERIVEXTPROC										glConvolutionParameterivEXT = 0;
	extern PFNGLCOPYCONVOLUTIONFILTER1DEXTPROC										glCopyConvolutionFilter1DEXT = 0;
	extern PFNGLCOPYCONVOLUTIONFILTER2DEXTPROC										glCopyConvolutionFilter2DEXT = 0;
	extern PFNGLGETCONVOLUTIONFILTEREXTPROC											glGetConvolutionFilterEXT = 0;
	extern PFNGLGETCONVOLUTIONPARAMETERFVEXTPROC									glGetConvolutionParameterfvEXT = 0;
	extern PFNGLGETCONVOLUTIONPARAMETERIVEXTPROC									glGetConvolutionParameterivEXT = 0;
	extern PFNGLGETSEPARABLEFILTEREXTPROC											glGetSeparableFilterEXT = 0;
	extern PFNGLSEPARABLEFILTER2DEXTPROC											glSeparableFilter2DEXT = 0;
	// GL_SGI_color_table
	extern PFNGLCOLORTABLESGIPROC													glColorTableSGI = 0;
	extern PFNGLCOLORTABLEPARAMETERFVSGIPROC										glColorTableParameterfvSGI = 0;
	extern PFNGLCOLORTABLEPARAMETERIVSGIPROC										glColorTableParameterivSGI = 0;
	extern PFNGLCOPYCOLORTABLESGIPROC												glCopyColorTableSGI = 0;
	extern PFNGLGETCOLORTABLESGIPROC												glGetColorTableSGI = 0;
	extern PFNGLGETCOLORTABLEPARAMETERFVSGIPROC										glGetColorTableParameterfvSGI = 0;
	extern PFNGLGETCOLORTABLEPARAMETERIVSGIPROC										glGetColorTableParameterivSGI = 0;
	// GL_SGIX_pixel_texture
	extern PFNGLPIXELTEXGENSGIXPROC													glPixelTexGenSGIX = 0;
	// GL_SGIS_pixel_texture
	extern PFNGLPIXELTEXGENPARAMETERISGISPROC										glPixelTexGenParameteriSGIS = 0;
	extern PFNGLPIXELTEXGENPARAMETERIVSGISPROC										glPixelTexGenParameterivSGIS = 0;
	extern PFNGLPIXELTEXGENPARAMETERFSGISPROC										glPixelTexGenParameterfSGIS = 0;
	extern PFNGLPIXELTEXGENPARAMETERFVSGISPROC										glPixelTexGenParameterfvSGIS = 0;
	extern PFNGLGETPIXELTEXGENPARAMETERIVSGISPROC									glGetPixelTexGenParameterivSGIS = 0;
	extern PFNGLGETPIXELTEXGENPARAMETERFVSGISPROC									glGetPixelTexGenParameterfvSGIS = 0;
	// GL_SGIS_texture4D
	extern PFNGLTEXIMAGE4DSGISPROC													glTexImage4DSGIS = 0;
	extern PFNGLTEXSUBIMAGE4DSGISPROC												glTexSubImage4DSGIS = 0;
	// GL_EXT_texture_object
	extern PFNGLARETEXTURESRESIDENTEXTPROC											glAreTexturesResidentEXT = 0;
	extern PFNGLBINDTEXTUREEXTPROC													glBindTextureEXT = 0;
	extern PFNGLDELETETEXTURESEXTPROC												glDeleteTexturesEXT = 0;
	extern PFNGLGENTEXTURESEXTPROC													glGenTexturesEXT = 0;
	extern PFNGLISTEXTUREEXTPROC													glIsTextureEXT = 0;
	extern PFNGLPRIORITIZETEXTURESEXTPROC											glPrioritizeTexturesEXT = 0;
	// GL_SGIS_detail_texture
	extern PFNGLDETAILTEXFUNCSGISPROC												glDetailTexFuncSGIS = 0;
	extern PFNGLGETDETAILTEXFUNCSGISPROC											glGetDetailTexFuncSGIS = 0;
	// GL_SGIS_sharpen_texture
	extern PFNGLSHARPENTEXFUNCSGISPROC												glSharpenTexFuncSGIS = 0;
	extern PFNGLGETSHARPENTEXFUNCSGISPROC											glGetSharpenTexFuncSGIS = 0;
	// GL_SGIS_multisample
	extern PFNGLSAMPLEMASKSGISPROC													glSampleMaskSGIS = 0;
	extern PFNGLSAMPLEPATTERNSGISPROC												glSamplePatternSGIS = 0;
	// GL_EXT_vertex_array
	extern PFNGLARRAYELEMENTEXTPROC													glArrayElementEXT = 0;
	extern PFNGLCOLORPOINTEREXTPROC													glColorPointerEXT = 0;
	extern PFNGLDRAWARRAYSEXTPROC													glDrawArraysEXT = 0;
	extern PFNGLEDGEFLAGPOINTEREXTPROC												glEdgeFlagPointerEXT = 0;
	extern PFNGLGETPOINTERVEXTPROC													glGetPointervEXT = 0;
	extern PFNGLINDEXPOINTEREXTPROC													glIndexPointerEXT = 0;
	extern PFNGLNORMALPOINTEREXTPROC												glNormalPointerEXT = 0;
	extern PFNGLTEXCOORDPOINTEREXTPROC												glTexCoordPointerEXT = 0;
	extern PFNGLVERTEXPOINTEREXTPROC												glVertexPointerEXT = 0;
	// GL_EXT_blend_minmax
	extern PFNGLBLENDEQUATIONEXTPROC												glBlendEquationEXT = 0;
	// GL_SGIX_sprite
	extern PFNGLSPRITEPARAMETERFSGIXPROC											glSpriteParameterfSGIX = 0;
	extern PFNGLSPRITEPARAMETERFVSGIXPROC											glSpriteParameterfvSGIX = 0;
	extern PFNGLSPRITEPARAMETERISGIXPROC											glSpriteParameteriSGIX = 0;
	extern PFNGLSPRITEPARAMETERIVSGIXPROC											glSpriteParameterivSGIX = 0;
	// GL_EXT_point_parameters
	extern PFNGLPOINTPARAMETERFEXTPROC												glPointParameterfEXT = 0;
	extern PFNGLPOINTPARAMETERFVEXTPROC												glPointParameterfvEXT = 0;
	// GL_SGIS_point_parameters
	extern PFNGLPOINTPARAMETERFSGISPROC												glPointParameterfSGIS = 0;
	extern PFNGLPOINTPARAMETERFVSGISPROC											glPointParameterfvSGIS = 0;
	// GL_SGIX_instruments
	extern PFNGLGETINSTRUMENTSSGIXPROC												glGetInstrumentsSGIX = 0;
	extern PFNGLINSTRUMENTSBUFFERSGIXPROC											glInstrumentsBufferSGIX = 0;
	extern PFNGLPOLLINSTRUMENTSSGIXPROC												glPollInstrumentsSGIX = 0;
	extern PFNGLREADINSTRUMENTSSGIXPROC												glReadInstrumentsSGIX = 0;
	extern PFNGLSTARTINSTRUMENTSSGIXPROC											glStartInstrumentsSGIX = 0;
	extern PFNGLSTOPINSTRUMENTSSGIXPROC												glStopInstrumentsSGIX = 0;
	// GL_SGIX_framezoom
	extern PFNGLFRAMEZOOMSGIXPROC													glFrameZoomSGIX = 0;
	// GL_SGIX_tag_sample_buffer
	extern PFNGLTAGSAMPLEBUFFERSGIXPROC												glTagSampleBufferSGIX = 0;
	// GL_SGIX_polynomial_ffd
	extern PFNGLDEFORMATIONMAP3DSGIXPROC											glDeformationMap3dSGIX = 0;
	extern PFNGLDEFORMATIONMAP3FSGIXPROC											glDeformationMap3fSGIX = 0;
	extern PFNGLDEFORMSGIXPROC														glDeformSGIX = 0;
	extern PFNGLLOADIDENTITYDEFORMATIONMAPSGIXPROC									glLoadIdentityDeformationMapSGIX = 0;
	// GL_SGIX_reference_plane
	extern PFNGLREFERENCEPLANESGIXPROC												glReferencePlaneSGIX = 0;
	// GL_SGIX_flush_raster
	extern PFNGLFLUSHRASTERSGIXPROC													glFlushRasterSGIX = 0;
	// GL_SGIS_fog_function
	extern PFNGLFOGFUNCSGISPROC														glFogFuncSGIS = 0;
	extern PFNGLGETFOGFUNCSGISPROC													glGetFogFuncSGIS = 0;
	// GL_HP_image_transform
	extern PFNGLIMAGETRANSFORMPARAMETERIHPPROC										glImageTransformParameteriHP = 0;
	extern PFNGLIMAGETRANSFORMPARAMETERFHPPROC										glImageTransformParameterfHP = 0;
	extern PFNGLIMAGETRANSFORMPARAMETERIVHPPROC										glImageTransformParameterivHP = 0;
	extern PFNGLIMAGETRANSFORMPARAMETERFVHPPROC										glImageTransformParameterfvHP = 0;
	extern PFNGLGETIMAGETRANSFORMPARAMETERIVHPPROC									glGetImageTransformParameterivHP = 0;
	extern PFNGLGETIMAGETRANSFORMPARAMETERFVHPPROC									glGetImageTransformParameterfvHP = 0;
	// GL_EXT_color_subtable
	extern PFNGLCOLORSUBTABLEEXTPROC												glColorSubTableEXT = 0;
	extern PFNGLCOPYCOLORSUBTABLEEXTPROC											glCopyColorSubTableEXT = 0;
	// GL_PGI_misc_hints
	extern PFNGLHINTPGIPROC															glHintPGI = 0;
	// GL_EXT_paletted_texture
	extern PFNGLCOLORTABLEEXTPROC													glColorTableEXT = 0;
	extern PFNGLGETCOLORTABLEEXTPROC												glGetColorTableEXT = 0;
	extern PFNGLGETCOLORTABLEPARAMETERIVEXTPROC										glGetColorTableParameterivEXT = 0;
	extern PFNGLGETCOLORTABLEPARAMETERFVEXTPROC										glGetColorTableParameterfvEXT = 0;
	// GL_SGIX_list_priority
	extern PFNGLGETLISTPARAMETERFVSGIXPROC											glGetListParameterfvSGIX = 0;
	extern PFNGLGETLISTPARAMETERIVSGIXPROC											glGetListParameterivSGIX = 0;
	extern PFNGLLISTPARAMETERFSGIXPROC												glListParameterfSGIX = 0;
	extern PFNGLLISTPARAMETERFVSGIXPROC												glListParameterfvSGIX = 0;
	extern PFNGLLISTPARAMETERISGIXPROC												glListParameteriSGIX = 0;
	extern PFNGLLISTPARAMETERIVSGIXPROC												glListParameterivSGIX = 0;
	// GL_EXT_index_material
	extern PFNGLINDEXMATERIALEXTPROC												glIndexMaterialEXT = 0;
	// GL_EXT_index_func
	extern PFNGLINDEXFUNCEXTPROC													glIndexFuncEXT = 0;
	// GL_EXT_compiled_vertex_array
	extern PFNGLLOCKARRAYSEXTPROC													glLockArraysEXT = 0;
	extern PFNGLUNLOCKARRAYSEXTPROC													glUnlockArraysEXT = 0;
	// GL_EXT_cull_vertex										
	extern PFNGLCULLPARAMETERDVEXTPROC												glCullParameterdvEXT = 0;				
	extern PFNGLCULLPARAMETERFVEXTPROC												glCullParameterfvEXT = 0;
	// GL_SGIX_fragment_lighting													
	extern PFNGLFRAGMENTCOLORMATERIALSGIXPROC										glFragmentColorMaterialSGIX = 0;
	extern PFNGLFRAGMENTLIGHTFSGIXPROC												glFragmentLightfSGIX = 0;
	extern PFNGLFRAGMENTLIGHTFVSGIXPROC												glFragmentLightfvSGIX = 0;
	extern PFNGLFRAGMENTLIGHTISGIXPROC												glFragmentLightiSGIX = 0;
	extern PFNGLFRAGMENTLIGHTIVSGIXPROC												glFragmentLightivSGIX = 0;
	extern PFNGLFRAGMENTLIGHTMODELFSGIXPROC											glFragmentLightModelfSGIX = 0;
	extern PFNGLFRAGMENTLIGHTMODELFVSGIXPROC										glFragmentLightModelfvSGIX = 0;
	extern PFNGLFRAGMENTLIGHTMODELISGIXPROC											glFragmentLightModeliSGIX = 0;
	extern PFNGLFRAGMENTLIGHTMODELIVSGIXPROC										glFragmentLightModelivSGIX = 0;
	extern PFNGLFRAGMENTMATERIALFSGIXPROC											glFragmentMaterialfSGIX = 0;
	extern PFNGLFRAGMENTMATERIALFVSGIXPROC											glFragmentMaterialfvSGIX = 0;
	extern PFNGLFRAGMENTMATERIALISGIXPROC											glFragmentMaterialiSGIX = 0;
	extern PFNGLFRAGMENTMATERIALIVSGIXPROC											glFragmentMaterialivSGIX = 0;
	extern PFNGLGETFRAGMENTLIGHTFVSGIXPROC											glGetFragmentLightfvSGIX = 0;
	extern PFNGLGETFRAGMENTLIGHTIVSGIXPROC											glGetFragmentLightivSGIX = 0;
	extern PFNGLGETFRAGMENTMATERIALFVSGIXPROC										glGetFragmentMaterialfvSGIX = 0;
	extern PFNGLGETFRAGMENTMATERIALIVSGIXPROC										glGetFragmentMaterialivSGIX = 0;
	extern PFNGLLIGHTENVISGIXPROC													glLightEnviSGIX = 0;
	// GL_EXT_draw_range_elements													
	extern PFNGLDRAWRANGEELEMENTSEXTPROC											glDrawRangeElementsEXT = 0;
	// GL_EXT_light_texture															
	extern PFNGLAPPLYTEXTUREEXTPROC													glApplyTextureEXT = 0;
	extern PFNGLTEXTURELIGHTEXTPROC													glTextureLightEXT = 0;
	extern PFNGLTEXTUREMATERIALEXTPROC												glTextureMaterialEXT = 0;
	// GL_SGIX_async																
	extern PFNGLASYNCMARKERSGIXPROC													glAsyncMarkerSGIX = 0;
	extern PFNGLFINISHASYNCSGIXPROC													glFinishAsyncSGIX = 0;
	extern PFNGLPOLLASYNCSGIXPROC													glPollAsyncSGIX = 0;
	extern PFNGLGENASYNCMARKERSSGIXPROC												glGenAsyncMarkersSGIX = 0;
	extern PFNGLDELETEASYNCMARKERSSGIXPROC											glDeleteAsyncMarkersSGIX = 0;
	extern PFNGLISASYNCMARKERSGIXPROC												glIsAsyncMarkerSGIX = 0;
	// GL_INTEL_parallel_arrays														
	extern PFNGLVERTEXPOINTERVINTELPROC												glVertexPointervINTEL = 0;
	extern PFNGLNORMALPOINTERVINTELPROC												glNormalPointervINTEL = 0;
	extern PFNGLCOLORPOINTERVINTELPROC												glColorPointervINTEL = 0;
	extern PFNGLTEXCOORDPOINTERVINTELPROC											glTexCoordPointervINTEL = 0;
	// GL_EXT_pixel_transform														
	extern PFNGLPIXELTRANSFORMPARAMETERIEXTPROC										glPixelTransformParameteriEXT = 0;
	extern PFNGLPIXELTRANSFORMPARAMETERFEXTPROC										glPixelTransformParameterfEXT = 0;
	extern PFNGLPIXELTRANSFORMPARAMETERIVEXTPROC									glPixelTransformParameterivEXT = 0;
	extern PFNGLPIXELTRANSFORMPARAMETERFVEXTPROC									glPixelTransformParameterfvEXT = 0;
	// GL_EXT_secondary_color														
	extern PFNGLSECONDARYCOLOR3BEXTPROC												glSecondaryColor3bEXT = 0;
	extern PFNGLSECONDARYCOLOR3BVEXTPROC											glSecondaryColor3bvEXT = 0;
	extern PFNGLSECONDARYCOLOR3DEXTPROC												glSecondaryColor3dEXT = 0;
	extern PFNGLSECONDARYCOLOR3DVEXTPROC											glSecondaryColor3dvEXT = 0;
	extern PFNGLSECONDARYCOLOR3FEXTPROC												glSecondaryColor3fEXT = 0;
	extern PFNGLSECONDARYCOLOR3FVEXTPROC											glSecondaryColor3fvEXT = 0;
	extern PFNGLSECONDARYCOLOR3IEXTPROC												glSecondaryColor3iEXT = 0;
	extern PFNGLSECONDARYCOLOR3IVEXTPROC											glSecondaryColor3ivEXT = 0;
	extern PFNGLSECONDARYCOLOR3SEXTPROC												glSecondaryColor3sEXT = 0;
	extern PFNGLSECONDARYCOLOR3SVEXTPROC											glSecondaryColor3svEXT = 0;
	extern PFNGLSECONDARYCOLOR3UBEXTPROC											glSecondaryColor3ubEXT = 0;
	extern PFNGLSECONDARYCOLOR3UBVEXTPROC											glSecondaryColor3ubvEXT = 0;
	extern PFNGLSECONDARYCOLOR3UIEXTPROC											glSecondaryColor3uiEXT = 0;
	extern PFNGLSECONDARYCOLOR3UIVEXTPROC											glSecondaryColor3uivEXT = 0;
	extern PFNGLSECONDARYCOLOR3USEXTPROC											glSecondaryColor3usEXT = 0;
	extern PFNGLSECONDARYCOLOR3USVEXTPROC											glSecondaryColor3usvEXT = 0;
	extern PFNGLSECONDARYCOLORPOINTEREXTPROC										glSecondaryColorPointerEXT = 0;
	// GL_EXT_texture_perturb_normal												
	extern PFNGLTEXTURENORMALEXTPROC												glTextureNormalEXT = 0;
	// GL_EXT_multi_draw_arrays														
	extern PFNGLMULTIDRAWARRAYSEXTPROC												glMultiDrawArraysEXT = 0;
	extern PFNGLMULTIDRAWELEMENTSEXTPROC											glMultiDrawElementsEXT = 0;
	// GL_EXT_fog_coord																
	extern PFNGLFOGCOORDFEXTPROC													glFogCoordfEXT = 0;
	extern PFNGLFOGCOORDFVEXTPROC													glFogCoordfvEXT = 0;
	extern PFNGLFOGCOORDDEXTPROC													glFogCoorddEXT = 0;
	extern PFNGLFOGCOORDDVEXTPROC													glFogCoorddvEXT = 0;
	extern PFNGLFOGCOORDPOINTEREXTPROC												glFogCoordPointerEXT = 0;
	// GL_EXT_coordinate_frame														
	extern PFNGLTANGENT3BEXTPROC													glTangent3bEXT = 0;
	extern PFNGLTANGENT3BVEXTPROC													glTangent3bvEXT = 0;
	extern PFNGLTANGENT3DEXTPROC													glTangent3dEXT = 0;
	extern PFNGLTANGENT3DVEXTPROC													glTangent3dvEXT = 0;
	extern PFNGLTANGENT3FEXTPROC													glTangent3fEXT = 0;
	extern PFNGLTANGENT3FVEXTPROC													glTangent3fvEXT = 0;
	extern PFNGLTANGENT3IEXTPROC													glTangent3iEXT = 0;
	extern PFNGLTANGENT3IVEXTPROC													glTangent3ivEXT = 0;
	extern PFNGLTANGENT3SEXTPROC													glTangent3sEXT = 0;
	extern PFNGLTANGENT3SVEXTPROC													glTangent3svEXT = 0;
	extern PFNGLBINORMAL3BEXTPROC													glBinormal3bEXT = 0;
	extern PFNGLBINORMAL3BVEXTPROC													glBinormal3bvEXT = 0;
	extern PFNGLBINORMAL3DEXTPROC													glBinormal3dEXT = 0;
	extern PFNGLBINORMAL3DVEXTPROC													glBinormal3dvEXT = 0;
	extern PFNGLBINORMAL3FEXTPROC													glBinormal3fEXT = 0;
	extern PFNGLBINORMAL3FVEXTPROC													glBinormal3fvEXT = 0;
	extern PFNGLBINORMAL3IEXTPROC													glBinormal3iEXT = 0;
	extern PFNGLBINORMAL3IVEXTPROC													glBinormal3ivEXT = 0;
	extern PFNGLBINORMAL3SEXTPROC													glBinormal3sEXT = 0;
	extern PFNGLBINORMAL3SVEXTPROC													glBinormal3svEXT = 0;
	extern PFNGLTANGENTPOINTEREXTPROC												glTangentPointerEXT = 0;
	extern PFNGLBINORMALPOINTEREXTPROC												glBinormalPointerEXT = 0;
	// GL_SUNX_constant_data														
	extern PFNGLFINISHTEXTURESUNXPROC												glFinishTextureSUNX = 0;
	// GL_SUN_global_alpha															
	extern PFNGLGLOBALALPHAFACTORBSUNPROC											glGlobalAlphaFactorbSUN = 0;
	extern PFNGLGLOBALALPHAFACTORSSUNPROC											glGlobalAlphaFactorsSUN = 0;
	extern PFNGLGLOBALALPHAFACTORISUNPROC											glGlobalAlphaFactoriSUN = 0;
	extern PFNGLGLOBALALPHAFACTORFSUNPROC											glGlobalAlphaFactorfSUN = 0;
	extern PFNGLGLOBALALPHAFACTORDSUNPROC											glGlobalAlphaFactordSUN = 0;
	extern PFNGLGLOBALALPHAFACTORUBSUNPROC											glGlobalAlphaFactorubSUN = 0;
	extern PFNGLGLOBALALPHAFACTORUSSUNPROC											glGlobalAlphaFactorusSUN = 0;
	extern PFNGLGLOBALALPHAFACTORUISUNPROC											glGlobalAlphaFactoruiSUN = 0;
	// GL_SUN_triangle_list															
	extern PFNGLREPLACEMENTCODEUISUNPROC											glReplacementCodeuiSUN = 0;
	extern PFNGLREPLACEMENTCODEUSSUNPROC											glReplacementCodeusSUN = 0; 
	extern PFNGLREPLACEMENTCODEUBSUNPROC											glReplacementCodeubSUN = 0; 
	extern PFNGLREPLACEMENTCODEUIVSUNPROC											glReplacementCodeuivSUN = 0;
	extern PFNGLREPLACEMENTCODEUSVSUNPROC											glReplacementCodeusvSUN = 0;
	extern PFNGLREPLACEMENTCODEUBVSUNPROC											glReplacementCodeubvSUN = 0;
	extern PFNGLREPLACEMENTCODEPOINTERSUNPROC										glReplacementCodePointerSUN = 0;
	// GL_SUN_vertex																
	extern PFNGLCOLOR4UBVERTEX2FSUNPROC												glColor4ubVertex2fSUN = 0;
	extern PFNGLCOLOR4UBVERTEX2FVSUNPROC											glColor4ubVertex2fvSUN = 0;
	extern PFNGLCOLOR4UBVERTEX3FSUNPROC												glColor4ubVertex3fSUN = 0;
	extern PFNGLCOLOR4UBVERTEX3FVSUNPROC											glColor4ubVertex3fvSUN = 0;
	extern PFNGLCOLOR3FVERTEX3FSUNPROC												glColor3fVertex3fSUN = 0;
	extern PFNGLCOLOR3FVERTEX3FVSUNPROC												glColor3fVertex3fvSUN = 0;
	extern PFNGLNORMAL3FVERTEX3FSUNPROC												glNormal3fVertex3fSUN = 0;
	extern PFNGLNORMAL3FVERTEX3FVSUNPROC											glNormal3fVertex3fvSUN = 0;
	extern PFNGLCOLOR4FNORMAL3FVERTEX3FSUNPROC										glColor4fNormal3fVertex3fSUN = 0;
	extern PFNGLCOLOR4FNORMAL3FVERTEX3FVSUNPROC										glColor4fNormal3fVertex3fvSUN = 0;
	extern PFNGLTEXCOORD2FVERTEX3FSUNPROC											glTexCoord2fVertex3fSUN = 0;
	extern PFNGLTEXCOORD2FVERTEX3FVSUNPROC											glTexCoord2fVertex3fvSUN = 0;
	extern PFNGLTEXCOORD4FVERTEX4FSUNPROC											glTexCoord4fVertex4fSUN = 0;
	extern PFNGLTEXCOORD4FVERTEX4FVSUNPROC											glTexCoord4fVertex4fvSUN = 0;
	extern PFNGLTEXCOORD2FCOLOR4UBVERTEX3FSUNPROC									glTexCoord2fColor4ubVertex3fSUN = 0;
	extern PFNGLTEXCOORD2FCOLOR4UBVERTEX3FVSUNPROC									glTexCoord2fColor4ubVertex3fvSUN = 0;
	extern PFNGLTEXCOORD2FCOLOR3FVERTEX3FSUNPROC									glTexCoord2fColor3fVertex3fSUN = 0;
	extern PFNGLTEXCOORD2FCOLOR3FVERTEX3FVSUNPROC									glTexCoord2fColor3fVertex3fvSUN = 0;
	extern PFNGLTEXCOORD2FNORMAL3FVERTEX3FSUNPROC									glTexCoord2fNormal3fVertex3fSUN = 0;
	extern PFNGLTEXCOORD2FNORMAL3FVERTEX3FVSUNPROC									glTexCoord2fNormal3fVertex3fvSUN = 0;
	extern PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC							glTexCoord2fColor4fNormal3fVertex3fSUN = 0;
	extern PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC							glTexCoord2fColor4fNormal3fVertex3fvSUN = 0;
	extern PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUNPROC							glTexCoord4fColor4fNormal3fVertex4fSUN = 0;
	extern PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUNPROC							glTexCoord4fColor4fNormal3fVertex4fvSUN = 0;
	extern PFNGLREPLACEMENTCODEUIVERTEX3FSUNPROC									glReplacementCodeuiVertex3fSUN = 0;
	extern PFNGLREPLACEMENTCODEUIVERTEX3FVSUNPROC									glReplacementCodeuiVertex3fvSUN = 0;
	extern PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUNPROC							glReplacementCodeuiColor4ubVertex3fSUN = 0;
	extern PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUNPROC							glReplacementCodeuiColor4ubVertex3fvSUN = 0;
	extern PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FSUNPROC								glReplacementCodeuiColor3fVertex3fSUN = 0;
	extern PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUNPROC							glReplacementCodeuiColor3fVertex3fvSUN = 0;
	extern PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FSUNPROC							glReplacementCodeuiNormal3fVertex3fSUN = 0;
	extern PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUNPROC							glReplacementCodeuiNormal3fVertex3fvSUN = 0;
	extern PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUNPROC						glReplacementCodeuiColor4fNormal3fVertex3fSUN = 0;
	extern PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUNPROC					glReplacementCodeuiColor4fNormal3fVertex3fvSUN = 0;
	extern PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUNPROC							glReplacementCodeuiTexCoord2fVertex3fSUN = 0;
	extern PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUNPROC							glReplacementCodeuiTexCoord2fVertex3fvSUN = 0;
	extern PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUNPROC					glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN = 0;
	extern PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUNPROC					glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN = 0;
	extern PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC			glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN = 0;
	extern PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC			glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN = 0;
	// GL_EXT_blend_func_separate													
	extern PFNGLBLENDFUNCSEPARATEEXTPROC											glBlendFuncSeparateEXT = 0;
	// GL_INGR_blend_func_separate													
	extern PFNGLBLENDFUNCSEPARATEINGRPROC											glBlendFuncSeparateINGR = 0;
	// GL_EXT_vertex_weighting														
	extern PFNGLVERTEXWEIGHTFEXTPROC												glVertexWeightfEXT = 0;
	extern PFNGLVERTEXWEIGHTFVEXTPROC												glVertexWeightfvEXT = 0;
	extern PFNGLVERTEXWEIGHTPOINTEREXTPROC											glVertexWeightPointerEXT = 0;
	// GL_NV_vertex_array_range														
	extern PFNGLFLUSHVERTEXARRAYRANGENVPROC											glFlushVertexArrayRangeNV = 0;
	extern PFNGLVERTEXARRAYRANGENVPROC												glVertexArrayRangeNV = 0;
	// GL_NV_register_combiners														
	extern PFNGLCOMBINERPARAMETERFVNVPROC											glCombinerParameterfvNV = 0;
	extern PFNGLCOMBINERPARAMETERFNVPROC											glCombinerParameterfNV = 0;
	extern PFNGLCOMBINERPARAMETERIVNVPROC											glCombinerParameterivNV = 0;
	extern PFNGLCOMBINERPARAMETERINVPROC											glCombinerParameteriNV = 0;
	extern PFNGLCOMBINERINPUTNVPROC													glCombinerInputNV = 0;
	extern PFNGLCOMBINEROUTPUTNVPROC												glCombinerOutputNV = 0;
	extern PFNGLFINALCOMBINERINPUTNVPROC											glFinalCombinerInputNV = 0;
	extern PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC									glGetCombinerInputParameterfvNV = 0;
	extern PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC									glGetCombinerInputParameterivNV = 0;
	extern PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC									glGetCombinerOutputParameterfvNV = 0;
	extern PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC									glGetCombinerOutputParameterivNV = 0;
	extern PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC								glGetFinalCombinerInputParameterfvNV = 0;
	extern PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC								glGetFinalCombinerInputParameterivNV = 0;
	// GL_MESA_resize_buffers														
	extern PFNGLRESIZEBUFFERSMESAPROC												glResizeBuffersMESA = 0;
	// GL_MESA_window_pos															
	extern PFNGLWINDOWPOS2DMESAPROC													glWindowPos2dMESA = 0;
	extern PFNGLWINDOWPOS2DVMESAPROC												glWindowPos2dvMESA = 0;
	extern PFNGLWINDOWPOS2FMESAPROC													glWindowPos2fMESA = 0;
	extern PFNGLWINDOWPOS2FVMESAPROC												glWindowPos2fvMESA = 0;
	extern PFNGLWINDOWPOS2IMESAPROC													glWindowPos2iMESA = 0;
	extern PFNGLWINDOWPOS2IVMESAPROC												glWindowPos2ivMESA = 0;
	extern PFNGLWINDOWPOS2SMESAPROC													glWindowPos2sMESA = 0;
	extern PFNGLWINDOWPOS2SVMESAPROC												glWindowPos2svMESA = 0;
	extern PFNGLWINDOWPOS3DMESAPROC													glWindowPos3dMESA = 0;
	extern PFNGLWINDOWPOS3DVMESAPROC												glWindowPos3dvMESA = 0;
	extern PFNGLWINDOWPOS3FMESAPROC													glWindowPos3fMESA = 0;
	extern PFNGLWINDOWPOS3FVMESAPROC												glWindowPos3fvMESA = 0;
	extern PFNGLWINDOWPOS3IMESAPROC													glWindowPos3iMESA = 0;
	extern PFNGLWINDOWPOS3IVMESAPROC												glWindowPos3ivMESA = 0;
	extern PFNGLWINDOWPOS3SMESAPROC													glWindowPos3sMESA = 0;
	extern PFNGLWINDOWPOS3SVMESAPROC												glWindowPos3svMESA = 0;
	extern PFNGLWINDOWPOS4DMESAPROC													glWindowPos4dMESA = 0;
	extern PFNGLWINDOWPOS4DVMESAPROC												glWindowPos4dvMESA = 0;
	extern PFNGLWINDOWPOS4FMESAPROC													glWindowPos4fMESA = 0;
	extern PFNGLWINDOWPOS4FVMESAPROC												glWindowPos4fvMESA = 0;
	extern PFNGLWINDOWPOS4IMESAPROC													glWindowPos4iMESA = 0;
	extern PFNGLWINDOWPOS4IVMESAPROC												glWindowPos4ivMESA = 0;
	extern PFNGLWINDOWPOS4SMESAPROC													glWindowPos4sMESA = 0;
	extern PFNGLWINDOWPOS4SVMESAPROC												glWindowPos4svMESA = 0;
	// GL_IBM_multimode_draw_arrays													
	extern PFNGLMULTIMODEDRAWARRAYSIBMPROC											glMultiModeDrawArraysIBM = 0;
	extern PFNGLMULTIMODEDRAWELEMENTSIBMPROC										glMultiModeDrawElementsIBM = 0;
	// GL_IBM_vertex_array_lists													
	extern PFNGLCOLORPOINTERLISTIBMPROC												glColorPointerListIBM = 0;
	extern PFNGLSECONDARYCOLORPOINTERLISTIBMPROC									glSecondaryColorPointerListIBM = 0;
	extern PFNGLEDGEFLAGPOINTERLISTIBMPROC											glEdgeFlagPointerListIBM = 0;
	extern PFNGLFOGCOORDPOINTERLISTIBMPROC											glFogCoordPointerListIBM = 0;
	extern PFNGLINDEXPOINTERLISTIBMPROC												glIndexPointerListIBM = 0;
	extern PFNGLNORMALPOINTERLISTIBMPROC											glNormalPointerListIBM = 0;
	extern PFNGLTEXCOORDPOINTERLISTIBMPROC											glTexCoordPointerListIBM = 0;
	extern PFNGLVERTEXPOINTERLISTIBMPROC											glVertexPointerListIBM = 0;
	// GL_3DFX_tbuffer																
	extern PFNGLTBUFFERMASK3DFXPROC													glTbufferMask3DFX = 0;
	// GL_EXT_multisample															
	extern PFNGLSAMPLEMASKEXTPROC													glSampleMaskEXT = 0;
	extern PFNGLSAMPLEPATTERNEXTPROC												glSamplePatternEXT = 0;
	// GL_SGIS_texture_color_mask													
	extern PFNGLTEXTURECOLORMASKSGISPROC											glTextureColorMaskSGIS = 0;
	// GL_SGIX_igloo_interface														
	extern PFNGLIGLOOINTERFACESGIXPROC												glIglooInterfaceSGIX = 0;
	// GL_NV_fence																	
	extern PFNGLDELETEFENCESNVPROC													glDeleteFencesNV = 0;
	extern PFNGLGENFENCESNVPROC														glGenFencesNV = 0;
	extern PFNGLISFENCENVPROC														glIsFenceNV = 0;
	extern PFNGLTESTFENCENVPROC														glTestFenceNV = 0;
	extern PFNGLGETFENCEIVNVPROC													glGetFenceivNV = 0;
	extern PFNGLFINISHFENCENVPROC													glFinishFenceNV = 0;
	extern PFNGLSETFENCENVPROC														glSetFenceNV = 0;
	// GL_NV_evaluators																
	extern PFNGLMAPCONTROLPOINTSNVPROC												glMapControlPointsNV = 0;
	extern PFNGLMAPPARAMETERIVNVPROC												glMapParameterivNV = 0;
	extern PFNGLMAPPARAMETERFVNVPROC												glMapParameterfvNV = 0;
	extern PFNGLGETMAPCONTROLPOINTSNVPROC											glGetMapControlPointsNV = 0;
	extern PFNGLGETMAPPARAMETERIVNVPROC												glGetMapParameterivNV = 0;
	extern PFNGLGETMAPPARAMETERFVNVPROC												glGetMapParameterfvNV = 0;
	extern PFNGLGETMAPATTRIBPARAMETERIVNVPROC										glGetMapAttribParameterivNV = 0;
	extern PFNGLGETMAPATTRIBPARAMETERFVNVPROC										glGetMapAttribParameterfvNV = 0;
	extern PFNGLEVALMAPSNVPROC														glEvalMapsNV = 0;
	// GL_NV_register_combiners2													
	extern PFNGLCOMBINERSTAGEPARAMETERFVNVPROC										glCombinerStageParameterfvNV = 0;
	extern PFNGLGETCOMBINERSTAGEPARAMETERFVNVPROC									glGetCombinerStageParameterfvNV = 0;
	// GL_NV_vertex_program															
	extern PFNGLAREPROGRAMSRESIDENTNVPROC											glAreProgramsResidentNV = 0;            
	extern PFNGLBINDPROGRAMNVPROC													glBindProgramNV = 0;                    
	extern PFNGLDELETEPROGRAMSNVPROC												glDeleteProgramsNV = 0;                 
	extern PFNGLEXECUTEPROGRAMNVPROC												glExecuteProgramNV = 0;                 
	extern PFNGLGENPROGRAMSNVPROC													glGenProgramsNV = 0;                    
	extern PFNGLGETPROGRAMPARAMETERDVNVPROC											glGetProgramParameterdvNV = 0;          
	extern PFNGLGETPROGRAMPARAMETERFVNVPROC											glGetProgramParameterfvNV = 0;          
	extern PFNGLGETPROGRAMIVNVPROC													glGetProgramivNV = 0;                   
	extern PFNGLGETPROGRAMSTRINGNVPROC												glGetProgramStringNV = 0;               
	extern PFNGLGETTRACKMATRIXIVNVPROC												glGetTrackMatrixivNV = 0;               
	extern PFNGLGETVERTEXATTRIBDVNVPROC												glGetVertexAttribdvNV = 0;              
	extern PFNGLGETVERTEXATTRIBFVNVPROC												glGetVertexAttribfvNV = 0;              
	extern PFNGLGETVERTEXATTRIBIVNVPROC												glGetVertexAttribivNV = 0;              
	extern PFNGLGETVERTEXATTRIBPOINTERVNVPROC										glGetVertexAttribPointervNV = 0;        
	extern PFNGLISPROGRAMNVPROC														glIsProgramNV = 0;                      
	extern PFNGLLOADPROGRAMNVPROC													glLoadProgramNV = 0;                    
	extern PFNGLPROGRAMPARAMETER4DNVPROC											glProgramParameter4dNV = 0;             
	extern PFNGLPROGRAMPARAMETER4DVNVPROC											glProgramParameter4dvNV = 0;            
	extern PFNGLPROGRAMPARAMETER4FNVPROC											glProgramParameter4fNV = 0;             
	extern PFNGLPROGRAMPARAMETER4FVNVPROC											glProgramParameter4fvNV = 0;            
	extern PFNGLPROGRAMPARAMETERS4DVNVPROC											glProgramParameters4dvNV = 0;           
	extern PFNGLPROGRAMPARAMETERS4FVNVPROC											glProgramParameters4fvNV = 0;           
	extern PFNGLREQUESTRESIDENTPROGRAMSNVPROC										glRequestResidentProgramsNV = 0;        
	extern PFNGLTRACKMATRIXNVPROC													glTrackMatrixNV = 0;                    
	extern PFNGLVERTEXATTRIBPOINTERNVPROC											glVertexAttribPointerNV = 0;            
	extern PFNGLVERTEXATTRIB1DNVPROC												glVertexAttrib1dNV = 0;                 
	extern PFNGLVERTEXATTRIB1DVNVPROC												glVertexAttrib1dvNV = 0;                
	extern PFNGLVERTEXATTRIB1FNVPROC												glVertexAttrib1fNV = 0;                 
	extern PFNGLVERTEXATTRIB1FVNVPROC												glVertexAttrib1fvNV = 0;                
	extern PFNGLVERTEXATTRIB1SNVPROC												glVertexAttrib1sNV = 0;                 
	extern PFNGLVERTEXATTRIB1SVNVPROC												glVertexAttrib1svNV = 0;                
	extern PFNGLVERTEXATTRIB2DNVPROC												glVertexAttrib2dNV = 0;                 
	extern PFNGLVERTEXATTRIB2DVNVPROC												glVertexAttrib2dvNV = 0;                
	extern PFNGLVERTEXATTRIB2FNVPROC												glVertexAttrib2fNV = 0;                 
	extern PFNGLVERTEXATTRIB2FVNVPROC												glVertexAttrib2fvNV = 0;                
	extern PFNGLVERTEXATTRIB2SNVPROC												glVertexAttrib2sNV = 0;                 
	extern PFNGLVERTEXATTRIB2SVNVPROC												glVertexAttrib2svNV = 0;                
	extern PFNGLVERTEXATTRIB3DNVPROC												glVertexAttrib3dNV = 0;                 
	extern PFNGLVERTEXATTRIB3DVNVPROC												glVertexAttrib3dvNV = 0;                
	extern PFNGLVERTEXATTRIB3FNVPROC												glVertexAttrib3fNV = 0;                 
	extern PFNGLVERTEXATTRIB3FVNVPROC												glVertexAttrib3fvNV = 0;                
	extern PFNGLVERTEXATTRIB3SNVPROC												glVertexAttrib3sNV = 0;                 
	extern PFNGLVERTEXATTRIB3SVNVPROC												glVertexAttrib3svNV = 0;                
	extern PFNGLVERTEXATTRIB4DNVPROC												glVertexAttrib4dNV = 0;                 
	extern PFNGLVERTEXATTRIB4DVNVPROC												glVertexAttrib4dvNV = 0;                
	extern PFNGLVERTEXATTRIB4FNVPROC												glVertexAttrib4fNV = 0;                 
	extern PFNGLVERTEXATTRIB4FVNVPROC												glVertexAttrib4fvNV = 0;                
	extern PFNGLVERTEXATTRIB4SNVPROC												glVertexAttrib4sNV = 0;                 
	extern PFNGLVERTEXATTRIB4SVNVPROC												glVertexAttrib4svNV = 0;                
	extern PFNGLVERTEXATTRIB4UBNVPROC												glVertexAttrib4ubNV = 0;                
	extern PFNGLVERTEXATTRIB4UBVNVPROC												glVertexAttrib4ubvNV = 0;               
	extern PFNGLVERTEXATTRIBS1DVNVPROC												glVertexAttribs1dvNV = 0;               
	extern PFNGLVERTEXATTRIBS1FVNVPROC												glVertexAttribs1fvNV = 0;               
	extern PFNGLVERTEXATTRIBS1SVNVPROC												glVertexAttribs1svNV = 0;               
	extern PFNGLVERTEXATTRIBS2DVNVPROC												glVertexAttribs2dvNV = 0;               
	extern PFNGLVERTEXATTRIBS2FVNVPROC												glVertexAttribs2fvNV = 0;               
	extern PFNGLVERTEXATTRIBS2SVNVPROC												glVertexAttribs2svNV = 0;               
	extern PFNGLVERTEXATTRIBS3DVNVPROC												glVertexAttribs3dvNV = 0;               
	extern PFNGLVERTEXATTRIBS3FVNVPROC												glVertexAttribs3fvNV = 0;               
	extern PFNGLVERTEXATTRIBS3SVNVPROC												glVertexAttribs3svNV = 0;               
	extern PFNGLVERTEXATTRIBS4DVNVPROC												glVertexAttribs4dvNV = 0;               
	extern PFNGLVERTEXATTRIBS4FVNVPROC												glVertexAttribs4fvNV = 0;               
	extern PFNGLVERTEXATTRIBS4SVNVPROC												glVertexAttribs4svNV = 0;               
	extern PFNGLVERTEXATTRIBS4UBVNVPROC												glVertexAttribs4ubvNV = 0;              
	// GL_ATI_envmap_bumpmap														
	extern PFNGLTEXBUMPPARAMETERIVATIPROC											glTexBumpParameterivATI = 0;
	extern PFNGLTEXBUMPPARAMETERFVATIPROC											glTexBumpParameterfvATI = 0;
	extern PFNGLGETTEXBUMPPARAMETERIVATIPROC										glGetTexBumpParameterivATI = 0;
	extern PFNGLGETTEXBUMPPARAMETERFVATIPROC										glGetTexBumpParameterfvATI = 0;
	// GL_ATI_fragment_shader														
	extern PFNGLGENFRAGMENTSHADERSATIPROC											glGenFragmentShadersATI = 0;
	extern PFNGLBINDFRAGMENTSHADERATIPROC											glBindFragmentShaderATI = 0;
	extern PFNGLDELETEFRAGMENTSHADERATIPROC											glDeleteFragmentShaderATI = 0;
	extern PFNGLBEGINFRAGMENTSHADERATIPROC											glBeginFragmentShaderATI = 0;
	extern PFNGLENDFRAGMENTSHADERATIPROC											glEndFragmentShaderATI = 0;
	extern PFNGLPASSTEXCOORDATIPROC													glPassTexCoordATI = 0;
	extern PFNGLSAMPLEMAPATIPROC													glSampleMapATI = 0;
	extern PFNGLCOLORFRAGMENTOP1ATIPROC												glColorFragmentOp1ATI = 0;
	extern PFNGLCOLORFRAGMENTOP2ATIPROC												glColorFragmentOp2ATI = 0;
	extern PFNGLCOLORFRAGMENTOP3ATIPROC												glColorFragmentOp3ATI = 0;
	extern PFNGLALPHAFRAGMENTOP1ATIPROC												glAlphaFragmentOp1ATI = 0;
	extern PFNGLALPHAFRAGMENTOP2ATIPROC												glAlphaFragmentOp2ATI = 0;
	extern PFNGLALPHAFRAGMENTOP3ATIPROC												glAlphaFragmentOp3ATI = 0;
	extern PFNGLSETFRAGMENTSHADERCONSTANTATIPROC									glSetFragmentShaderConstantATI = 0;
	// GL_ATI_pn_triangles															
	extern  PFNGLPNTRIANGLESIATIPROC												glPNTrianglesiATI = 0;
	extern  PFNGLPNTRIANGLESFATIPROC												glPNTrianglesfATI = 0;
	// GL_ATI_vertex_array_object													
	extern PFNGLNEWOBJECTBUFFERATIPROC												glNewObjectBufferATI = 0;
	extern PFNGLISOBJECTBUFFERATIPROC												glIsObjectBufferATI = 0;
	extern PFNGLUPDATEOBJECTBUFFERATIPROC											glUpdateObjectBufferATI = 0;
	extern PFNGLGETOBJECTBUFFERFVATIPROC											glGetObjectBufferfvATI = 0;
	extern PFNGLGETOBJECTBUFFERIVATIPROC											glGetObjectBufferivATI = 0;
	extern PFNGLFREEOBJECTBUFFERATIPROC												glFreeObjectBufferATI = 0;
	extern PFNGLARRAYOBJECTATIPROC													glArrayObjectATI = 0;
	extern PFNGLGETARRAYOBJECTFVATIPROC												glGetArrayObjectfvATI = 0;
	extern PFNGLGETARRAYOBJECTIVATIPROC												glGetArrayObjectivATI = 0;
	extern PFNGLVARIANTARRAYOBJECTATIPROC											glVariantArrayObjectATI = 0;
	extern PFNGLGETVARIANTARRAYOBJECTFVATIPROC										glGetVariantArrayObjectfvATI = 0;
	extern PFNGLGETVARIANTARRAYOBJECTIVATIPROC										glGetVariantArrayObjectivATI = 0;
	// GL_EXT_vertex_shader															
	extern PFNGLBEGINVERTEXSHADEREXTPROC											glBeginVertexShaderEXT = 0;
	extern PFNGLENDVERTEXSHADEREXTPROC												glEndVertexShaderEXT = 0;
	extern PFNGLBINDVERTEXSHADEREXTPROC												glBindVertexShaderEXT = 0;
	extern PFNGLGENVERTEXSHADERSEXTPROC												glGenVertexShadersEXT = 0;
	extern PFNGLDELETEVERTEXSHADEREXTPROC											glDeleteVertexShaderEXT = 0;
	extern PFNGLSHADEROP1EXTPROC													glShaderOp1EXT = 0;
	extern PFNGLSHADEROP2EXTPROC													glShaderOp2EXT = 0;
	extern PFNGLSHADEROP3EXTPROC													glShaderOp3EXT = 0;
	extern PFNGLSWIZZLEEXTPROC														glSwizzleEXT = 0;
	extern PFNGLWRITEMASKEXTPROC													glWriteMaskEXT = 0;
	extern PFNGLINSERTCOMPONENTEXTPROC												glInsertComponentEXT = 0;
	extern PFNGLEXTRACTCOMPONENTEXTPROC												glExtractComponentEXT = 0;
	extern PFNGLGENSYMBOLSEXTPROC													glGenSymbolsEXT = 0;
	extern PFNGLSETINVARIANTEXTPROC													glSetInvariantEXT = 0;
	extern PFNGLSETLOCALCONSTANTEXTPROC												glSetLocalConstantEXT = 0;
	extern PFNGLVARIANTBVEXTPROC													glVariantbvEXT = 0;
	extern PFNGLVARIANTSVEXTPROC													glVariantsvEXT = 0;
	extern PFNGLVARIANTIVEXTPROC													glVariantivEXT = 0;
	extern PFNGLVARIANTFVEXTPROC													glVariantfvEXT = 0;
	extern PFNGLVARIANTDVEXTPROC													glVariantdvEXT = 0;
	extern PFNGLVARIANTUBVEXTPROC													glVariantubvEXT = 0;
	extern PFNGLVARIANTUSVEXTPROC													glVariantusvEXT = 0;
	extern PFNGLVARIANTUIVEXTPROC													glVariantuivEXT = 0;
	extern PFNGLVARIANTPOINTEREXTPROC												glVariantPointerEXT = 0;
	extern PFNGLENABLEVARIANTCLIENTSTATEEXTPROC										glEnableVariantClientStateEXT = 0;
	extern PFNGLDISABLEVARIANTCLIENTSTATEEXTPROC									glDisableVariantClientStateEXT = 0;
	extern PFNGLBINDLIGHTPARAMETEREXTPROC											glBindLightParameterEXT = 0;
	extern PFNGLBINDMATERIALPARAMETEREXTPROC										glBindMaterialParameterEXT = 0;
	extern PFNGLBINDTEXGENPARAMETEREXTPROC											glBindTexGenParameterEXT = 0;
	extern PFNGLBINDTEXTUREUNITPARAMETEREXTPROC										glBindTextureUnitParameterEXT = 0;
	extern PFNGLBINDPARAMETEREXTPROC												glBindParameterEXT = 0;
	extern PFNGLISVARIANTENABLEDEXTPROC												glIsVariantEnabledEXT = 0;
	extern PFNGLGETVARIANTBOOLEANVEXTPROC											glGetVariantBooleanvEXT = 0;
	extern PFNGLGETVARIANTINTEGERVEXTPROC											glGetVariantIntegervEXT = 0;
	extern PFNGLGETVARIANTFLOATVEXTPROC												glGetVariantFloatvEXT = 0;
	extern PFNGLGETVARIANTPOINTERVEXTPROC											glGetVariantPointervEXT = 0;
	extern PFNGLGETINVARIANTBOOLEANVEXTPROC											glGetInvariantBooleanvEXT = 0;
	extern PFNGLGETINVARIANTINTEGERVEXTPROC											glGetInvariantIntegervEXT = 0;
	extern PFNGLGETINVARIANTFLOATVEXTPROC											glGetInvariantFloatvEXT = 0;
	extern PFNGLGETLOCALCONSTANTBOOLEANVEXTPROC										glGetLocalConstantBooleanvEXT = 0;
	extern PFNGLGETLOCALCONSTANTINTEGERVEXTPROC										glGetLocalConstantIntegervEXT = 0;
	extern PFNGLGETLOCALCONSTANTFLOATVEXTPROC										glGetLocalConstantFloatvEXT = 0;
	// GL_ATI_vertex_streams														
	extern PFNGLVERTEXSTREAM1SATIPROC												glVertexStream1sATI = 0;
	extern PFNGLVERTEXSTREAM1SVATIPROC												glVertexStream1svATI = 0;
	extern PFNGLVERTEXSTREAM1IATIPROC												glVertexStream1iATI = 0;
	extern PFNGLVERTEXSTREAM1IVATIPROC												glVertexStream1ivATI = 0;
	extern PFNGLVERTEXSTREAM1FATIPROC												glVertexStream1fATI = 0;
	extern PFNGLVERTEXSTREAM1FVATIPROC												glVertexStream1fvATI = 0;
	extern PFNGLVERTEXSTREAM1DATIPROC												glVertexStream1dATI = 0;
	extern PFNGLVERTEXSTREAM1DVATIPROC												glVertexStream1dvATI = 0;
	extern PFNGLVERTEXSTREAM2SATIPROC												glVertexStream2sATI = 0;
	extern PFNGLVERTEXSTREAM2SVATIPROC												glVertexStream2svATI = 0;
	extern PFNGLVERTEXSTREAM2IATIPROC												glVertexStream2iATI = 0;
	extern PFNGLVERTEXSTREAM2IVATIPROC												glVertexStream2ivATI = 0;
	extern PFNGLVERTEXSTREAM2FATIPROC												glVertexStream2fATI = 0;
	extern PFNGLVERTEXSTREAM2FVATIPROC												glVertexStream2fvATI = 0;
	extern PFNGLVERTEXSTREAM2DATIPROC												glVertexStream2dATI = 0; 
	extern PFNGLVERTEXSTREAM2DVATIPROC												glVertexStream2dvATI = 0;
	extern PFNGLVERTEXSTREAM3SATIPROC												glVertexStream3sATI = 0; 
	extern PFNGLVERTEXSTREAM3SVATIPROC												glVertexStream3svATI = 0;
	extern PFNGLVERTEXSTREAM3IATIPROC												glVertexStream3iATI = 0; 
	extern PFNGLVERTEXSTREAM3IVATIPROC												glVertexStream3ivATI = 0;
	extern PFNGLVERTEXSTREAM3FATIPROC												glVertexStream3fATI = 0; 
	extern PFNGLVERTEXSTREAM3FVATIPROC												glVertexStream3fvATI = 0;
	extern PFNGLVERTEXSTREAM3DATIPROC												glVertexStream3dATI = 0; 
	extern PFNGLVERTEXSTREAM3DVATIPROC												glVertexStream3dvATI = 0;
	extern PFNGLVERTEXSTREAM4SATIPROC												glVertexStream4sATI = 0; 
	extern PFNGLVERTEXSTREAM4SVATIPROC												glVertexStream4svATI = 0;
	extern PFNGLVERTEXSTREAM4IATIPROC												glVertexStream4iATI = 0; 
	extern PFNGLVERTEXSTREAM4IVATIPROC												glVertexStream4ivATI = 0;
	extern PFNGLVERTEXSTREAM4FATIPROC												glVertexStream4fATI = 0; 
	extern PFNGLVERTEXSTREAM4FVATIPROC												glVertexStream4fvATI = 0;
	extern PFNGLVERTEXSTREAM4DATIPROC												glVertexStream4dATI = 0; 
	extern PFNGLVERTEXSTREAM4DVATIPROC												glVertexStream4dvATI = 0;
	extern PFNGLNORMALSTREAM3BATIPROC												glNormalStream3bATI = 0; 
	extern PFNGLNORMALSTREAM3BVATIPROC												glNormalStream3bvATI = 0;
	extern PFNGLNORMALSTREAM3SATIPROC												glNormalStream3sATI = 0; 
	extern PFNGLNORMALSTREAM3SVATIPROC												glNormalStream3svATI = 0;
	extern PFNGLNORMALSTREAM3IATIPROC												glNormalStream3iATI = 0; 
	extern PFNGLNORMALSTREAM3IVATIPROC												glNormalStream3ivATI = 0;
	extern PFNGLNORMALSTREAM3FATIPROC												glNormalStream3fATI = 0; 
	extern PFNGLNORMALSTREAM3FVATIPROC												glNormalStream3fvATI = 0;
	extern PFNGLNORMALSTREAM3DATIPROC												glNormalStream3dATI = 0; 
	extern PFNGLNORMALSTREAM3DVATIPROC												glNormalStream3dvATI = 0;
	extern PFNGLCLIENTACTIVEVERTEXSTREAMATIPROC										glClientActiveVertexStreamATI = 0;
	extern PFNGLVERTEXBLENDENVIATIPROC												glVertexBlendEnviATI = 0;
	extern PFNGLVERTEXBLENDENVFATIPROC												glVertexBlendEnvfATI = 0;
	// GL_ATI_element_array															
	extern PFNGLELEMENTPOINTERATIPROC												glElementPointerATI = 0;
	extern PFNGLDRAWELEMENTARRAYATIPROC												glDrawElementArrayATI = 0;
	extern PFNGLDRAWRANGEELEMENTARRAYATIPROC										glDrawRangeElementArrayATI = 0;
	// GL_SUN_mesh_array															
	extern PFNGLDRAWMESHARRAYSSUNPROC												glDrawMeshArraysSUN = 0;
	// GL_NV_occlusion_query														
	extern PFNGLGENOCCLUSIONQUERIESNVPROC											glGenOcclusionQueriesNV = 0;
	extern PFNGLDELETEOCCLUSIONQUERIESNVPROC										glDeleteOcclusionQueriesNV = 0;
	extern PFNGLISOCCLUSIONQUERYNVPROC												glIsOcclusionQueryNV = 0;
	extern PFNGLBEGINOCCLUSIONQUERYNVPROC											glBeginOcclusionQueryNV = 0;
	extern PFNGLENDOCCLUSIONQUERYNVPROC												glEndOcclusionQueryNV = 0;
	extern PFNGLGETOCCLUSIONQUERYIVNVPROC											glGetOcclusionQueryivNV = 0;
	extern PFNGLGETOCCLUSIONQUERYUIVNVPROC											glGetOcclusionQueryuivNV = 0;
	// GL_NV_point_sprite															
	extern PFNGLPOINTPARAMETERINVPROC												glPointParameteriNV = 0;
	extern PFNGLPOINTPARAMETERIVNVPROC												glPointParameterivNV = 0;
	// GL_EXT_stencil_two_side														
	extern PFNGLACTIVESTENCILFACEEXTPROC											glActiveStencilFaceEXT = 0;
	// GL_APPLE_element_array														
	extern PFNGLELEMENTPOINTERAPPLEPROC												glElementPointerAPPLE = 0;
	extern PFNGLDRAWELEMENTARRAYAPPLEPROC											glDrawElementArrayAPPLE = 0;
	extern PFNGLDRAWRANGEELEMENTARRAYAPPLEPROC										glDrawRangeElementArrayAPPLE = 0;
	extern PFNGLMULTIDRAWELEMENTARRAYAPPLEPROC										glMultiDrawElementArrayAPPLE = 0;
	extern PFNGLMULTIDRAWRANGEELEMENTARRAYAPPLEPROC									glMultiDrawRangeElementArrayAPPLE = 0;
	// GL_APPLE_fence																
	extern PFNGLGENFENCESAPPLEPROC													glGenFencesAPPLE = 0;
	extern PFNGLDELETEFENCESAPPLEPROC												glDeleteFencesAPPLE = 0;
	extern PFNGLSETFENCEAPPLEPROC													glSetFenceAPPLE = 0;
	extern PFNGLISFENCEAPPLEPROC													glIsFenceAPPLE = 0;
	extern PFNGLTESTFENCEAPPLEPROC													glTestFenceAPPLE = 0;
	extern PFNGLFINISHFENCEAPPLEPROC												glFinishFenceAPPLE = 0;
	extern PFNGLTESTOBJECTAPPLEPROC													glTestObjectAPPLE = 0;
	extern PFNGLFINISHOBJECTAPPLEPROC												glFinishObjectAPPLE = 0;
	// GL_APPLE_vertex_array_object													
	extern PFNGLBINDVERTEXARRAYAPPLEPROC											glBindVertexArrayAPPLE = 0;
	extern PFNGLDELETEVERTEXARRAYSAPPLEPROC											glDeleteVertexArraysAPPLE = 0;
	extern PFNGLGENVERTEXARRAYSAPPLEPROC											glGenVertexArraysAPPLE = 0;
	extern PFNGLISVERTEXARRAYAPPLEPROC												glIsVertexArrayAPPLE = 0;
	// GL_APPLE_vertex_array_range													
	extern PFNGLVERTEXARRAYRANGEAPPLEPROC											glVertexArrayRangeAPPLE = 0;
	extern PFNGLFLUSHVERTEXARRAYRANGEAPPLEPROC										glFlushVertexArrayRangeAPPLE = 0;
	extern PFNGLVERTEXARRAYPARAMETERIAPPLEPROC										glVertexArrayParameteriAPPLE = 0;
	// GL_ATI_draw_buffers															
	extern PFNGLDRAWBUFFERSATIPROC													glDrawBuffersATI = 0;
	// GL_NV_fragment_program														
	extern PFNGLPROGRAMNAMEDPARAMETER4FNVPROC										glProgramNamedParameter4fNV = 0;
	extern PFNGLPROGRAMNAMEDPARAMETER4DNVPROC										glProgramNamedParameter4dNV = 0;
	extern PFNGLPROGRAMNAMEDPARAMETER4FVNVPROC										glProgramNamedParameter4fvNV = 0;
	extern PFNGLPROGRAMNAMEDPARAMETER4DVNVPROC										glProgramNamedParameter4dvNV = 0;
	extern PFNGLGETPROGRAMNAMEDPARAMETERFVNVPROC									glGetProgramNamedParameterfvNV = 0;
	extern PFNGLGETPROGRAMNAMEDPARAMETERDVNVPROC									glGetProgramNamedParameterdvNV = 0;
	// GL_NV_half_float																
	extern PFNGLVERTEX2HNVPROC														glVertex2hNV = 0;
	extern PFNGLVERTEX2HVNVPROC														glVertex2hvNV = 0;
	extern PFNGLVERTEX3HNVPROC														glVertex3hNV = 0;
	extern PFNGLVERTEX3HVNVPROC														glVertex3hvNV = 0;
	extern PFNGLVERTEX4HNVPROC														glVertex4hNV = 0;
	extern PFNGLVERTEX4HVNVPROC														glVertex4hvNV = 0;
	extern PFNGLNORMAL3HNVPROC														glNormal3hNV = 0;
	extern PFNGLNORMAL3HVNVPROC														glNormal3hvNV = 0;
	extern PFNGLCOLOR3HNVPROC														glColor3hNV = 0;
	extern PFNGLCOLOR3HVNVPROC														glColor3hvNV = 0;               
	extern PFNGLCOLOR4HNVPROC														glColor4hNV = 0;                
	extern PFNGLCOLOR4HVNVPROC														glColor4hvNV = 0;               
	extern PFNGLTEXCOORD1HNVPROC													glTexCoord1hNV = 0;             
	extern PFNGLTEXCOORD1HVNVPROC													glTexCoord1hvNV = 0;            
	extern PFNGLTEXCOORD2HNVPROC													glTexCoord2hNV = 0;             
	extern PFNGLTEXCOORD2HVNVPROC													glTexCoord2hvNV = 0;            
	extern PFNGLTEXCOORD3HNVPROC													glTexCoord3hNV = 0;             
	extern PFNGLTEXCOORD3HVNVPROC													glTexCoord3hvNV = 0;            
	extern PFNGLTEXCOORD4HNVPROC													glTexCoord4hNV = 0;             
	extern PFNGLTEXCOORD4HVNVPROC													glTexCoord4hvNV = 0;            
	extern PFNGLMULTITEXCOORD1HNVPROC												glMultiTexCoord1hNV = 0;        
	extern PFNGLMULTITEXCOORD1HVNVPROC												glMultiTexCoord1hvNV = 0;       
	extern PFNGLMULTITEXCOORD2HNVPROC												glMultiTexCoord2hNV = 0;        
	extern PFNGLMULTITEXCOORD2HVNVPROC												glMultiTexCoord2hvNV = 0;       
	extern PFNGLMULTITEXCOORD3HNVPROC												glMultiTexCoord3hNV = 0;        
	extern PFNGLMULTITEXCOORD3HVNVPROC												glMultiTexCoord3hvNV = 0;       
	extern PFNGLMULTITEXCOORD4HNVPROC												glMultiTexCoord4hNV = 0;        
	extern PFNGLMULTITEXCOORD4HVNVPROC												glMultiTexCoord4hvNV = 0;       
	extern PFNGLFOGCOORDHNVPROC														glFogCoordhNV = 0;              
	extern PFNGLFOGCOORDHVNVPROC													glFogCoordhvNV = 0;             
	extern PFNGLSECONDARYCOLOR3HNVPROC												glSecondaryColor3hNV = 0;       
	extern PFNGLSECONDARYCOLOR3HVNVPROC												glSecondaryColor3hvNV = 0;      
	extern PFNGLVERTEXWEIGHTHNVPROC													glVertexWeighthNV = 0;          
	extern PFNGLVERTEXWEIGHTHVNVPROC												glVertexWeighthvNV = 0;         
	extern PFNGLVERTEXATTRIB1HNVPROC												glVertexAttrib1hNV = 0;         
	extern PFNGLVERTEXATTRIB1HVNVPROC												glVertexAttrib1hvNV = 0;        
	extern PFNGLVERTEXATTRIB2HNVPROC												glVertexAttrib2hNV = 0;         
	extern PFNGLVERTEXATTRIB2HVNVPROC												glVertexAttrib2hvNV = 0;        
	extern PFNGLVERTEXATTRIB3HNVPROC												glVertexAttrib3hNV = 0;         
	extern PFNGLVERTEXATTRIB3HVNVPROC												glVertexAttrib3hvNV = 0;        
	extern PFNGLVERTEXATTRIB4HNVPROC												glVertexAttrib4hNV = 0;         
	extern PFNGLVERTEXATTRIB4HVNVPROC												glVertexAttrib4hvNV = 0;        
	extern PFNGLVERTEXATTRIBS1HVNVPROC												glVertexAttribs1hvNV = 0;       
	extern PFNGLVERTEXATTRIBS2HVNVPROC												glVertexAttribs2hvNV = 0;       
	extern PFNGLVERTEXATTRIBS3HVNVPROC												glVertexAttribs3hvNV = 0;       
	extern PFNGLVERTEXATTRIBS4HVNVPROC												glVertexAttribs4hvNV = 0;       
	// GL_NV_pixel_data_range														
	extern PFNGLPIXELDATARANGENVPROC												glPixelDataRangeNV = 0;
	extern PFNGLFLUSHPIXELDATARANGENVPROC											glFlushPixelDataRangeNV = 0;
	// GL_NV_primitive_restart														
	extern PFNGLPRIMITIVERESTARTNVPROC												glPrimitiveRestartNV = 0;
	extern PFNGLPRIMITIVERESTARTINDEXNVPROC											glPrimitiveRestartIndexNV = 0;
	// GL_ATI_map_object_buffer														
	extern PFNGLMAPOBJECTBUFFERATIPROC												glMapObjectBufferATI = 0;
	extern PFNGLUNMAPOBJECTBUFFERATIPROC											glUnmapObjectBufferATI = 0;
	// GL_ATI_separate_stencil														
	extern PFNGLSTENCILOPSEPARATEATIPROC											glStencilOpSeparateATI = 0;
	extern PFNGLSTENCILFUNCSEPARATEATIPROC											glStencilFuncSeparateATI = 0;
	// GL_ATI_vertex_attrib_array_object											
	extern PFNGLVERTEXATTRIBARRAYOBJECTATIPROC										glVertexAttribArrayObjectATI = 0;
	extern PFNGLGETVERTEXATTRIBARRAYOBJECTFVATIPROC									glGetVertexAttribArrayObjectfvATI = 0;
	extern PFNGLGETVERTEXATTRIBARRAYOBJECTIVATIPROC									glGetVertexAttribArrayObjectivATI = 0;
	// GL_EXT_depth_bounds_test														
	extern PFNGLDEPTHBOUNDSEXTPROC													glDepthBoundsEXT = 0;
	// GL_EXT_blend_equation_separate												
	extern PFNGLBLENDEQUATIONSEPARATEEXTPROC										glBlendEquationSeparateEXT = 0;
	// GL_EXT_framebuffer_object													
	extern PFNGLISRENDERBUFFEREXTPROC												glIsRenderbufferEXT = 0;
	extern PFNGLBINDRENDERBUFFEREXTPROC												glBindRenderbufferEXT = 0;
	extern PFNGLDELETERENDERBUFFERSEXTPROC											glDeleteRenderbuffersEXT = 0;
	extern PFNGLGENRENDERBUFFERSEXTPROC												glGenRenderbuffersEXT = 0;
	extern PFNGLRENDERBUFFERSTORAGEEXTPROC											glRenderbufferStorageEXT = 0;
	extern PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC									glGetRenderbufferParameterivEXT = 0;
	extern PFNGLISFRAMEBUFFEREXTPROC												glIsFramebufferEXT = 0;
	extern PFNGLBINDFRAMEBUFFEREXTPROC												glBindFramebufferEXT = 0;
	extern PFNGLDELETEFRAMEBUFFERSEXTPROC											glDeleteFramebuffersEXT = 0;
	extern PFNGLGENFRAMEBUFFERSEXTPROC												glGenFramebuffersEXT = 0;
	extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC										glCheckFramebufferStatusEXT = 0;
	extern PFNGLFRAMEBUFFERTEXTURE1DEXTPROC											glFramebufferTexture1DEXT = 0;
	extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC											glFramebufferTexture2DEXT = 0;
	extern PFNGLFRAMEBUFFERTEXTURE3DEXTPROC											glFramebufferTexture3DEXT = 0;
	extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC										glFramebufferRenderbufferEXT = 0;
	extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC							glGetFramebufferAttachmentParameterivEXT = 0;
	extern PFNGLGENERATEMIPMAPEXTPROC												glGenerateMipmapEXT = 0;
	// GL_GREMEDY_string_marker														
	extern PFNGLSTRINGMARKERGREMEDYPROC												glStringMarkerGREMEDY = 0;
	// GL_EXT_stencil_clear_tag														
	extern PFNGLSTENCILCLEARTAGEXTPROC												glStencilClearTagEXT = 0;
	// GL_EXT_framebuffer_blit														
	extern PFNGLBLITFRAMEBUFFEREXTPROC												glBlitFramebufferEXT = 0;
	// GL_EXT_framebuffer_multisample												
	extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC								glRenderbufferStorageMultisampleEXT = 0;
	// GL_EXT_timer_query															
	extern PFNGLGETQUERYOBJECTI64VEXTPROC											glGetQueryObjecti64vEXT = 0;
	extern PFNGLGETQUERYOBJECTUI64VEXTPROC											glGetQueryObjectui64vEXT = 0;
	// GL_EXT_gpu_program_parameters												
	extern PFNGLPROGRAMENVPARAMETERS4FVEXTPROC										glProgramEnvParameters4fvEXT = 0;
	extern PFNGLPROGRAMLOCALPARAMETERS4FVEXTPROC									glProgramLocalParameters4fvEXT = 0;
	// GL_APPLE_flush_buffer_range													
	extern PFNGLBUFFERPARAMETERIAPPLEPROC											glBufferParameteriAPPLE = 0;
	extern PFNGLFLUSHMAPPEDBUFFERRANGEAPPLEPROC										glFlushMappedBufferRangeAPPLE = 0;
	// GL_NV_gpu_program4															
	extern PFNGLPROGRAMLOCALPARAMETERI4INVPROC										glProgramLocalParameterI4iNV = 0;      
	extern PFNGLPROGRAMLOCALPARAMETERI4IVNVPROC										glProgramLocalParameterI4ivNV = 0;     
	extern PFNGLPROGRAMLOCALPARAMETERSI4IVNVPROC									glProgramLocalParametersI4ivNV = 0;    
	extern PFNGLPROGRAMLOCALPARAMETERI4UINVPROC										glProgramLocalParameterI4uiNV = 0;     
	extern PFNGLPROGRAMLOCALPARAMETERI4UIVNVPROC									glProgramLocalParameterI4uivNV = 0;    
	extern PFNGLPROGRAMLOCALPARAMETERSI4UIVNVPROC									glProgramLocalParametersI4uivNV = 0;   
	extern PFNGLPROGRAMENVPARAMETERI4INVPROC										glProgramEnvParameterI4iNV = 0;        
	extern PFNGLPROGRAMENVPARAMETERI4IVNVPROC										glProgramEnvParameterI4ivNV = 0;       
	extern PFNGLPROGRAMENVPARAMETERSI4IVNVPROC										glProgramEnvParametersI4ivNV = 0;      
	extern PFNGLPROGRAMENVPARAMETERI4UINVPROC										glProgramEnvParameterI4uiNV = 0;       
	extern PFNGLPROGRAMENVPARAMETERI4UIVNVPROC										glProgramEnvParameterI4uivNV = 0;      
	extern PFNGLPROGRAMENVPARAMETERSI4UIVNVPROC										glProgramEnvParametersI4uivNV = 0;     
	extern PFNGLGETPROGRAMLOCALPARAMETERIIVNVPROC									glGetProgramLocalParameterIivNV = 0;   
	extern PFNGLGETPROGRAMLOCALPARAMETERIUIVNVPROC									glGetProgramLocalParameterIuivNV = 0;  
	extern PFNGLGETPROGRAMENVPARAMETERIIVNVPROC										glGetProgramEnvParameterIivNV = 0;     
	extern PFNGLGETPROGRAMENVPARAMETERIUIVNVPROC									glGetProgramEnvParameterIuivNV = 0;    
	// GL_NV_geometry_program4														
	extern PFNGLPROGRAMVERTEXLIMITNVPROC											glProgramVertexLimitNV = 0;
	extern PFNGLFRAMEBUFFERTEXTUREEXTPROC											glFramebufferTextureEXT = 0;
	extern PFNGLFRAMEBUFFERTEXTURELAYEREXTPROC										glFramebufferTextureLayerEXT = 0;
	extern PFNGLFRAMEBUFFERTEXTUREFACEEXTPROC										glFramebufferTextureFaceEXT = 0;
	// GL_EXT_geometry_shader4														
	extern PFNGLPROGRAMPARAMETERIEXTPROC											glProgramParameteriEXT = 0;
	// GL_NV_vertex_program4														
	extern PFNGLVERTEXATTRIBI1IEXTPROC												glVertexAttribI1iEXT = 0;  
	extern PFNGLVERTEXATTRIBI2IEXTPROC												glVertexAttribI2iEXT = 0;  
	extern PFNGLVERTEXATTRIBI3IEXTPROC												glVertexAttribI3iEXT = 0;  
	extern PFNGLVERTEXATTRIBI4IEXTPROC												glVertexAttribI4iEXT = 0;  
	extern PFNGLVERTEXATTRIBI1UIEXTPROC												glVertexAttribI1uiEXT = 0; 
	extern PFNGLVERTEXATTRIBI2UIEXTPROC												glVertexAttribI2uiEXT = 0; 
	extern PFNGLVERTEXATTRIBI3UIEXTPROC												glVertexAttribI3uiEXT = 0; 
	extern PFNGLVERTEXATTRIBI4UIEXTPROC												glVertexAttribI4uiEXT = 0; 
	extern PFNGLVERTEXATTRIBI1IVEXTPROC												glVertexAttribI1ivEXT = 0; 
	extern PFNGLVERTEXATTRIBI2IVEXTPROC												glVertexAttribI2ivEXT = 0; 
	extern PFNGLVERTEXATTRIBI3IVEXTPROC												glVertexAttribI3ivEXT = 0; 
	extern PFNGLVERTEXATTRIBI4IVEXTPROC												glVertexAttribI4ivEXT = 0; 
	extern PFNGLVERTEXATTRIBI1UIVEXTPROC											glVertexAttribI1uivEXT = 0;
	extern PFNGLVERTEXATTRIBI2UIVEXTPROC											glVertexAttribI2uivEXT = 0;
	extern PFNGLVERTEXATTRIBI3UIVEXTPROC											glVertexAttribI3uivEXT = 0;
	extern PFNGLVERTEXATTRIBI4UIVEXTPROC											glVertexAttribI4uivEXT = 0;
	extern PFNGLVERTEXATTRIBI4BVEXTPROC												glVertexAttribI4bvEXT = 0; 
	extern PFNGLVERTEXATTRIBI4SVEXTPROC												glVertexAttribI4svEXT = 0; 
	extern PFNGLVERTEXATTRIBI4UBVEXTPROC											glVertexAttribI4ubvEXT = 0;
	extern PFNGLVERTEXATTRIBI4USVEXTPROC											glVertexAttribI4usvEXT = 0;
	extern PFNGLVERTEXATTRIBIPOINTEREXTPROC											glVertexAttribIPointerEXT = 0;
	extern PFNGLGETVERTEXATTRIBIIVEXTPROC											glGetVertexAttribIivEXT = 0;  
	extern PFNGLGETVERTEXATTRIBIUIVEXTPROC											glGetVertexAttribIuivEXT = 0; 
	// GL_EXT_gpu_shader4															
	extern PFNGLGETUNIFORMUIVEXTPROC												glGetUniformuivEXT = 0;
	extern PFNGLBINDFRAGDATALOCATIONEXTPROC											glBindFragDataLocationEXT = 0;
	extern PFNGLGETFRAGDATALOCATIONEXTPROC											glGetFragDataLocationEXT = 0;
	extern PFNGLUNIFORM1UIEXTPROC													glUniform1uiEXT = 0;
	extern PFNGLUNIFORM2UIEXTPROC													glUniform2uiEXT = 0;
	extern PFNGLUNIFORM3UIEXTPROC													glUniform3uiEXT = 0;
	extern PFNGLUNIFORM4UIEXTPROC													glUniform4uiEXT = 0;
	extern PFNGLUNIFORM1UIVEXTPROC													glUniform1uivEXT = 0;
	extern PFNGLUNIFORM2UIVEXTPROC													glUniform2uivEXT = 0;
	extern PFNGLUNIFORM3UIVEXTPROC													glUniform3uivEXT = 0;
	extern PFNGLUNIFORM4UIVEXTPROC													glUniform4uivEXT = 0;
	// GL_EXT_draw_instanced														
	extern PFNGLDRAWARRAYSINSTANCEDEXTPROC											glDrawArraysInstancedEXT = 0;
	extern PFNGLDRAWELEMENTSINSTANCEDEXTPROC										glDrawElementsInstancedEXT = 0;
	// GL_EXT_texture_buffer_object													
	extern PFNGLTEXBUFFEREXTPROC													glTexBufferEXT = 0;
	// GL_NV_depth_buffer_float														
	extern PFNGLDEPTHRANGEDNVPROC													glDepthRangedNV = 0;
	extern PFNGLCLEARDEPTHDNVPROC													glClearDepthdNV = 0;
	extern PFNGLDEPTHBOUNDSDNVPROC													glDepthBoundsdNV = 0;
	// GL_NV_framebuffer_multisample_coverage										
	extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC						glRenderbufferStorageMultisampleCoverageNV = 0;
	// GL_NV_parameter_buffer_object												
	extern PFNGLPROGRAMBUFFERPARAMETERSFVNVPROC										glProgramBufferParametersfvNV = 0;
	extern PFNGLPROGRAMBUFFERPARAMETERSIIVNVPROC									glProgramBufferParametersIivNV = 0;
	extern PFNGLPROGRAMBUFFERPARAMETERSIUIVNVPROC									glProgramBufferParametersIuivNV = 0;
	// GL_EXT_draw_buffers2															
	extern PFNGLCOLORMASKINDEXEDEXTPROC												glColorMaskIndexedEXT = 0;
	extern PFNGLGETBOOLEANINDEXEDVEXTPROC											glGetBooleanIndexedvEXT = 0;
	extern PFNGLGETINTEGERINDEXEDVEXTPROC											glGetIntegerIndexedvEXT = 0;
	extern PFNGLENABLEINDEXEDEXTPROC												glEnableIndexedEXT = 0;
	extern PFNGLDISABLEINDEXEDEXTPROC												glDisableIndexedEXT = 0;
	extern PFNGLISENABLEDINDEXEDEXTPROC												glIsEnabledIndexedEXT = 0;
	// GL_NV_transform_feedback														
	extern PFNGLBEGINTRANSFORMFEEDBACKNVPROC										glBeginTransformFeedbackNV = 0;
	extern PFNGLENDTRANSFORMFEEDBACKNVPROC											glEndTransformFeedbackNV = 0;
	extern PFNGLTRANSFORMFEEDBACKATTRIBSNVPROC										glTransformFeedbackAttribsNV = 0;
	extern PFNGLBINDBUFFERRANGENVPROC												glBindBufferRangeNV = 0;
	extern PFNGLBINDBUFFEROFFSETNVPROC												glBindBufferOffsetNV = 0;
	extern PFNGLBINDBUFFERBASENVPROC												glBindBufferBaseNV = 0;
	extern PFNGLTRANSFORMFEEDBACKVARYINGSNVPROC										glTransformFeedbackVaryingsNV = 0;
	extern PFNGLACTIVEVARYINGNVPROC													glActiveVaryingNV = 0;
	extern PFNGLGETVARYINGLOCATIONNVPROC											glGetVaryingLocationNV = 0;
	extern PFNGLGETACTIVEVARYINGNVPROC												glGetActiveVaryingNV = 0;
	extern PFNGLGETTRANSFORMFEEDBACKVARYINGNVPROC									glGetTransformFeedbackVaryingNV = 0;
	// GL_EXT_bindable_uniform														
	extern PFNGLUNIFORMBUFFEREXTPROC												glUniformBufferEXT = 0;
	extern PFNGLGETUNIFORMBUFFERSIZEEXTPROC											glGetUniformBufferSizeEXT = 0;
	extern PFNGLGETUNIFORMOFFSETEXTPROC												glGetUniformOffsetEXT = 0;  
	// GL_EXT_texture_integer														
	extern PFNGLTEXPARAMETERIIVEXTPROC												glTexParameterIivEXT = 0;
	extern PFNGLTEXPARAMETERIUIVEXTPROC												glTexParameterIuivEXT = 0;
	extern PFNGLGETTEXPARAMETERIIVEXTPROC											glGetTexParameterIivEXT = 0;
	extern PFNGLGETTEXPARAMETERIUIVEXTPROC											glGetTexParameterIuivEXT = 0;
	extern PFNGLCLEARCOLORIIEXTPROC													glClearColorIiEXT = 0;
	extern PFNGLCLEARCOLORIUIEXTPROC												glClearColorIuiEXT = 0;
	// GL_GREMEDY_frame_terminator													
	extern PFNGLFRAMETERMINATORGREMEDYPROC											glFrameTerminatorGREMEDY = 0;
	// GL_NV_conditional_render														
	extern PFNGLBEGINCONDITIONALRENDERNVPROC										glBeginConditionalRenderNV = 0;
	extern PFNGLENDCONDITIONALRENDERNVPROC											glEndConditionalRenderNV = 0;
	// GL_NV_present_video															
	extern PFNGLPRESENTFRAMEKEYEDNVPROC												glPresentFrameKeyedNV = 0;
	extern PFNGLPRESENTFRAMEDUALFILLNVPROC											glPresentFrameDualFillNV = 0;
	extern PFNGLGETVIDEOIVNVPROC													glGetVideoivNV = 0;
	extern PFNGLGETVIDEOUIVNVPROC													glGetVideouivNV = 0;
	extern PFNGLGETVIDEOI64VNVPROC													glGetVideoi64vNV = 0;
	extern PFNGLGETVIDEOUI64VNVPROC													glGetVideoui64vNV = 0;
	// GL_EXT_transform_feedback													
	extern PFNGLBEGINTRANSFORMFEEDBACKEXTPROC										glBeginTransformFeedbackEXT = 0;
	extern PFNGLENDTRANSFORMFEEDBACKEXTPROC											glEndTransformFeedbackEXT = 0;
	extern PFNGLBINDBUFFERRANGEEXTPROC												glBindBufferRangeEXT = 0;
	extern PFNGLBINDBUFFEROFFSETEXTPROC												glBindBufferOffsetEXT = 0;
	extern PFNGLBINDBUFFERBASEEXTPROC												glBindBufferBaseEXT = 0;
	extern PFNGLTRANSFORMFEEDBACKVARYINGSEXTPROC									glTransformFeedbackVaryingsEXT = 0;
	extern PFNGLGETTRANSFORMFEEDBACKVARYINGEXTPROC									glGetTransformFeedbackVaryingEXT = 0;
	// GL_EXT_direct_state_access													
	extern PFNGLCLIENTATTRIBDEFAULTEXTPROC											glClientAttribDefaultEXT = 0;        
	extern PFNGLPUSHCLIENTATTRIBDEFAULTEXTPROC										glPushClientAttribDefaultEXT = 0;    
	extern PFNGLMATRIXLOADFEXTPROC													glMatrixLoadfEXT = 0;                
	extern PFNGLMATRIXLOADDEXTPROC													glMatrixLoaddEXT = 0;                
	extern PFNGLMATRIXMULTFEXTPROC													glMatrixMultfEXT = 0;                
	extern PFNGLMATRIXMULTDEXTPROC													glMatrixMultdEXT = 0;                
	extern PFNGLMATRIXLOADIDENTITYEXTPROC											glMatrixLoadIdentityEXT = 0;         
	extern PFNGLMATRIXROTATEFEXTPROC												glMatrixRotatefEXT = 0;              
	extern PFNGLMATRIXROTATEDEXTPROC												glMatrixRotatedEXT = 0;              
	extern PFNGLMATRIXSCALEFEXTPROC													glMatrixScalefEXT = 0;               
	extern PFNGLMATRIXSCALEDEXTPROC													glMatrixScaledEXT = 0;               
	extern PFNGLMATRIXTRANSLATEFEXTPROC												glMatrixTranslatefEXT = 0;           
	extern PFNGLMATRIXTRANSLATEDEXTPROC												glMatrixTranslatedEXT = 0;           
	extern PFNGLMATRIXFRUSTUMEXTPROC												glMatrixFrustumEXT = 0;              
	extern PFNGLMATRIXORTHOEXTPROC													glMatrixOrthoEXT = 0;                
	extern PFNGLMATRIXPOPEXTPROC													glMatrixPopEXT = 0;                  
	extern PFNGLMATRIXPUSHEXTPROC													glMatrixPushEXT = 0;                 
	extern PFNGLMATRIXLOADTRANSPOSEFEXTPROC											glMatrixLoadTransposefEXT = 0;       
	extern PFNGLMATRIXLOADTRANSPOSEDEXTPROC											glMatrixLoadTransposedEXT = 0;       
	extern PFNGLMATRIXMULTTRANSPOSEFEXTPROC											glMatrixMultTransposefEXT = 0;       
	extern PFNGLMATRIXMULTTRANSPOSEDEXTPROC											glMatrixMultTransposedEXT = 0;       
	extern PFNGLTEXTUREPARAMETERFEXTPROC											glTextureParameterfEXT = 0;          
	extern PFNGLTEXTUREPARAMETERFVEXTPROC											glTextureParameterfvEXT = 0;         
	extern PFNGLTEXTUREPARAMETERIEXTPROC											glTextureParameteriEXT = 0;          
	extern PFNGLTEXTUREPARAMETERIVEXTPROC											glTextureParameterivEXT = 0;         
	extern PFNGLTEXTUREIMAGE1DEXTPROC												glTextureImage1DEXT = 0;             
	extern PFNGLTEXTUREIMAGE2DEXTPROC												glTextureImage2DEXT = 0;             
	extern PFNGLTEXTURESUBIMAGE1DEXTPROC											glTextureSubImage1DEXT = 0;          
	extern PFNGLTEXTURESUBIMAGE2DEXTPROC											glTextureSubImage2DEXT = 0;          
	extern PFNGLCOPYTEXTUREIMAGE1DEXTPROC											glCopyTextureImage1DEXT = 0;         
	extern PFNGLCOPYTEXTUREIMAGE2DEXTPROC											glCopyTextureImage2DEXT = 0;         
	extern PFNGLCOPYTEXTURESUBIMAGE1DEXTPROC										glCopyTextureSubImage1DEXT = 0;      
	extern PFNGLCOPYTEXTURESUBIMAGE2DEXTPROC										glCopyTextureSubImage2DEXT = 0;      
	extern PFNGLGETTEXTUREIMAGEEXTPROC												glGetTextureImageEXT = 0;            
	extern PFNGLGETTEXTUREPARAMETERFVEXTPROC										glGetTextureParameterfvEXT = 0;      
	extern PFNGLGETTEXTUREPARAMETERIVEXTPROC										glGetTextureParameterivEXT = 0;      
	extern PFNGLGETTEXTURELEVELPARAMETERFVEXTPROC									glGetTextureLevelParameterfvEXT = 0; 
	extern PFNGLGETTEXTURELEVELPARAMETERIVEXTPROC									glGetTextureLevelParameterivEXT = 0; 
	extern PFNGLTEXTUREIMAGE3DEXTPROC												glTextureImage3DEXT = 0;             
	extern PFNGLTEXTURESUBIMAGE3DEXTPROC											glTextureSubImage3DEXT = 0;          
	extern PFNGLCOPYTEXTURESUBIMAGE3DEXTPROC										glCopyTextureSubImage3DEXT = 0;      
	extern PFNGLMULTITEXPARAMETERFEXTPROC											glMultiTexParameterfEXT = 0;         
	extern PFNGLMULTITEXPARAMETERFVEXTPROC											glMultiTexParameterfvEXT = 0;        
	extern PFNGLMULTITEXPARAMETERIEXTPROC											glMultiTexParameteriEXT = 0;         
	extern PFNGLMULTITEXPARAMETERIVEXTPROC											glMultiTexParameterivEXT = 0;        
	extern PFNGLMULTITEXIMAGE1DEXTPROC												glMultiTexImage1DEXT = 0;            
	extern PFNGLMULTITEXIMAGE2DEXTPROC												glMultiTexImage2DEXT = 0;            
	extern PFNGLMULTITEXSUBIMAGE1DEXTPROC											glMultiTexSubImage1DEXT = 0;         
	extern PFNGLMULTITEXSUBIMAGE2DEXTPROC											glMultiTexSubImage2DEXT = 0;         
	extern PFNGLCOPYMULTITEXIMAGE1DEXTPROC											glCopyMultiTexImage1DEXT = 0;        
	extern PFNGLCOPYMULTITEXIMAGE2DEXTPROC											glCopyMultiTexImage2DEXT = 0;        
	extern PFNGLCOPYMULTITEXSUBIMAGE1DEXTPROC										glCopyMultiTexSubImage1DEXT = 0;     
	extern PFNGLCOPYMULTITEXSUBIMAGE2DEXTPROC										glCopyMultiTexSubImage2DEXT = 0;     
	extern PFNGLGETMULTITEXIMAGEEXTPROC												glGetMultiTexImageEXT = 0;           
	extern PFNGLGETMULTITEXPARAMETERFVEXTPROC										glGetMultiTexParameterfvEXT = 0;     
	extern PFNGLGETMULTITEXPARAMETERIVEXTPROC										glGetMultiTexParameterivEXT = 0;     
	extern PFNGLGETMULTITEXLEVELPARAMETERFVEXTPROC									glGetMultiTexLevelParameterfvEXT = 0;
	extern PFNGLGETMULTITEXLEVELPARAMETERIVEXTPROC									glGetMultiTexLevelParameterivEXT = 0;
	extern PFNGLMULTITEXIMAGE3DEXTPROC												glMultiTexImage3DEXT = 0;            
	extern PFNGLMULTITEXSUBIMAGE3DEXTPROC											glMultiTexSubImage3DEXT = 0;         
	extern PFNGLCOPYMULTITEXSUBIMAGE3DEXTPROC										glCopyMultiTexSubImage3DEXT = 0;     
	extern PFNGLBINDMULTITEXTUREEXTPROC												glBindMultiTextureEXT = 0;           
	extern PFNGLENABLECLIENTSTATEINDEXEDEXTPROC										glEnableClientStateIndexedEXT = 0;   
	extern PFNGLDISABLECLIENTSTATEINDEXEDEXTPROC									glDisableClientStateIndexedEXT = 0;  
	extern PFNGLMULTITEXCOORDPOINTEREXTPROC											glMultiTexCoordPointerEXT = 0;       
	extern PFNGLMULTITEXENVFEXTPROC													glMultiTexEnvfEXT = 0;               
	extern PFNGLMULTITEXENVFVEXTPROC												glMultiTexEnvfvEXT = 0;              
	extern PFNGLMULTITEXENVIEXTPROC													glMultiTexEnviEXT = 0;               
	extern PFNGLMULTITEXENVIVEXTPROC												glMultiTexEnvivEXT = 0;              
	extern PFNGLMULTITEXGENDEXTPROC													glMultiTexGendEXT = 0;               
	extern PFNGLMULTITEXGENDVEXTPROC												glMultiTexGendvEXT = 0;              
	extern PFNGLMULTITEXGENFEXTPROC													glMultiTexGenfEXT = 0;               
	extern PFNGLMULTITEXGENFVEXTPROC												glMultiTexGenfvEXT = 0;              
	extern PFNGLMULTITEXGENIEXTPROC													glMultiTexGeniEXT = 0;               
	extern PFNGLMULTITEXGENIVEXTPROC												glMultiTexGenivEXT = 0;              
	extern PFNGLGETMULTITEXENVFVEXTPROC												glGetMultiTexEnvfvEXT = 0;           
	extern PFNGLGETMULTITEXENVIVEXTPROC												glGetMultiTexEnvivEXT = 0;                       
	extern PFNGLGETMULTITEXGENDVEXTPROC												glGetMultiTexGendvEXT = 0;                       
	extern PFNGLGETMULTITEXGENFVEXTPROC												glGetMultiTexGenfvEXT = 0;                       
	extern PFNGLGETMULTITEXGENIVEXTPROC												glGetMultiTexGenivEXT = 0;                       
	extern PFNGLGETFLOATINDEXEDVEXTPROC												glGetFloatIndexedvEXT = 0;                       
	extern PFNGLGETDOUBLEINDEXEDVEXTPROC											glGetDoubleIndexedvEXT = 0;                      
	extern PFNGLGETPOINTERINDEXEDVEXTPROC											glGetPointerIndexedvEXT = 0;                     
	extern PFNGLCOMPRESSEDTEXTUREIMAGE3DEXTPROC										glCompressedTextureImage3DEXT = 0;               
	extern PFNGLCOMPRESSEDTEXTUREIMAGE2DEXTPROC										glCompressedTextureImage2DEXT = 0;               
	extern PFNGLCOMPRESSEDTEXTUREIMAGE1DEXTPROC										glCompressedTextureImage1DEXT = 0;               
	extern PFNGLCOMPRESSEDTEXTURESUBIMAGE3DEXTPROC									glCompressedTextureSubImage3DEXT = 0;            
	extern PFNGLCOMPRESSEDTEXTURESUBIMAGE2DEXTPROC									glCompressedTextureSubImage2DEXT = 0;            
	extern PFNGLCOMPRESSEDTEXTURESUBIMAGE1DEXTPROC									glCompressedTextureSubImage1DEXT = 0;            
	extern PFNGLGETCOMPRESSEDTEXTUREIMAGEEXTPROC									glGetCompressedTextureImageEXT = 0;              
	extern PFNGLCOMPRESSEDMULTITEXIMAGE3DEXTPROC									glCompressedMultiTexImage3DEXT = 0;              
	extern PFNGLCOMPRESSEDMULTITEXIMAGE2DEXTPROC									glCompressedMultiTexImage2DEXT = 0;              
	extern PFNGLCOMPRESSEDMULTITEXIMAGE1DEXTPROC									glCompressedMultiTexImage1DEXT = 0;              
	extern PFNGLCOMPRESSEDMULTITEXSUBIMAGE3DEXTPROC									glCompressedMultiTexSubImage3DEXT = 0;           
	extern PFNGLCOMPRESSEDMULTITEXSUBIMAGE2DEXTPROC									glCompressedMultiTexSubImage2DEXT = 0;           
	extern PFNGLCOMPRESSEDMULTITEXSUBIMAGE1DEXTPROC									glCompressedMultiTexSubImage1DEXT = 0;           
	extern PFNGLGETCOMPRESSEDMULTITEXIMAGEEXTPROC									glGetCompressedMultiTexImageEXT = 0;             
	extern PFNGLNAMEDPROGRAMSTRINGEXTPROC											glNamedProgramStringEXT = 0;                     
	extern PFNGLNAMEDPROGRAMLOCALPARAMETER4DEXTPROC									glNamedProgramLocalParameter4dEXT = 0;           
	extern PFNGLNAMEDPROGRAMLOCALPARAMETER4DVEXTPROC								glNamedProgramLocalParameter4dvEXT = 0;          
	extern PFNGLNAMEDPROGRAMLOCALPARAMETER4FEXTPROC									glNamedProgramLocalParameter4fEXT = 0;           
	extern PFNGLNAMEDPROGRAMLOCALPARAMETER4FVEXTPROC								glNamedProgramLocalParameter4fvEXT = 0;          
	extern PFNGLGETNAMEDPROGRAMLOCALPARAMETERDVEXTPROC								glGetNamedProgramLocalParameterdvEXT = 0;        
	extern PFNGLGETNAMEDPROGRAMLOCALPARAMETERFVEXTPROC								glGetNamedProgramLocalParameterfvEXT = 0;        
	extern PFNGLGETNAMEDPROGRAMIVEXTPROC											glGetNamedProgramivEXT = 0;                      
	extern PFNGLGETNAMEDPROGRAMSTRINGEXTPROC										glGetNamedProgramStringEXT = 0;                  
	extern PFNGLNAMEDPROGRAMLOCALPARAMETERS4FVEXTPROC								glNamedProgramLocalParameters4fvEXT = 0;         
	extern PFNGLNAMEDPROGRAMLOCALPARAMETERI4IEXTPROC								glNamedProgramLocalParameterI4iEXT = 0;          
	extern PFNGLNAMEDPROGRAMLOCALPARAMETERI4IVEXTPROC								glNamedProgramLocalParameterI4ivEXT = 0;         
	extern PFNGLNAMEDPROGRAMLOCALPARAMETERSI4IVEXTPROC								glNamedProgramLocalParametersI4ivEXT = 0;        
	extern PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIEXTPROC								glNamedProgramLocalParameterI4uiEXT = 0;         
	extern PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIVEXTPROC								glNamedProgramLocalParameterI4uivEXT = 0;        
	extern PFNGLNAMEDPROGRAMLOCALPARAMETERSI4UIVEXTPROC								glNamedProgramLocalParametersI4uivEXT = 0;       
	extern PFNGLGETNAMEDPROGRAMLOCALPARAMETERIIVEXTPROC								glGetNamedProgramLocalParameterIivEXT = 0;       
	extern PFNGLGETNAMEDPROGRAMLOCALPARAMETERIUIVEXTPROC							glGetNamedProgramLocalParameterIuivEXT = 0;      
	extern PFNGLTEXTUREPARAMETERIIVEXTPROC											glTextureParameterIivEXT = 0;                    
	extern PFNGLTEXTUREPARAMETERIUIVEXTPROC											glTextureParameterIuivEXT = 0;                   
	extern PFNGLGETTEXTUREPARAMETERIIVEXTPROC										glGetTextureParameterIivEXT = 0;                 
	extern PFNGLGETTEXTUREPARAMETERIUIVEXTPROC										glGetTextureParameterIuivEXT = 0;                
	extern PFNGLMULTITEXPARAMETERIIVEXTPROC											glMultiTexParameterIivEXT = 0;                   
	extern PFNGLMULTITEXPARAMETERIUIVEXTPROC										glMultiTexParameterIuivEXT = 0;                  
	extern PFNGLGETMULTITEXPARAMETERIIVEXTPROC										glGetMultiTexParameterIivEXT = 0;                
	extern PFNGLGETMULTITEXPARAMETERIUIVEXTPROC										glGetMultiTexParameterIuivEXT = 0;               
	extern PFNGLPROGRAMUNIFORM1FEXTPROC												glProgramUniform1fEXT = 0;                       
	extern PFNGLPROGRAMUNIFORM2FEXTPROC												glProgramUniform2fEXT = 0;                       
	extern PFNGLPROGRAMUNIFORM3FEXTPROC												glProgramUniform3fEXT = 0;                       
	extern PFNGLPROGRAMUNIFORM4FEXTPROC												glProgramUniform4fEXT = 0;                       
	extern PFNGLPROGRAMUNIFORM1IEXTPROC												glProgramUniform1iEXT = 0;                       
	extern PFNGLPROGRAMUNIFORM2IEXTPROC												glProgramUniform2iEXT = 0;                       
	extern PFNGLPROGRAMUNIFORM3IEXTPROC												glProgramUniform3iEXT = 0;                       
	extern PFNGLPROGRAMUNIFORM4IEXTPROC												glProgramUniform4iEXT = 0;                       
	extern PFNGLPROGRAMUNIFORM1FVEXTPROC											glProgramUniform1fvEXT = 0;                      
	extern PFNGLPROGRAMUNIFORM2FVEXTPROC											glProgramUniform2fvEXT = 0;                      
	extern PFNGLPROGRAMUNIFORM3FVEXTPROC											glProgramUniform3fvEXT = 0;                      
	extern PFNGLPROGRAMUNIFORM4FVEXTPROC											glProgramUniform4fvEXT = 0;                      
	extern PFNGLPROGRAMUNIFORM1IVEXTPROC											glProgramUniform1ivEXT = 0;                      
	extern PFNGLPROGRAMUNIFORM2IVEXTPROC											glProgramUniform2ivEXT = 0;                      
	extern PFNGLPROGRAMUNIFORM3IVEXTPROC											glProgramUniform3ivEXT = 0;                      
	extern PFNGLPROGRAMUNIFORM4IVEXTPROC											glProgramUniform4ivEXT = 0;                      
	extern PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC										glProgramUniformMatrix2fvEXT = 0;                
	extern PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC										glProgramUniformMatrix3fvEXT = 0;                
	extern PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC										glProgramUniformMatrix4fvEXT = 0;                
	extern PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC									glProgramUniformMatrix2x3fvEXT = 0;              
	extern PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC									glProgramUniformMatrix3x2fvEXT = 0;              
	extern PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC									glProgramUniformMatrix2x4fvEXT = 0;              
	extern PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC									glProgramUniformMatrix4x2fvEXT = 0;              
	extern PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC									glProgramUniformMatrix3x4fvEXT = 0;              
	extern PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC									glProgramUniformMatrix4x3fvEXT = 0;              
	extern PFNGLPROGRAMUNIFORM1UIEXTPROC											glProgramUniform1uiEXT = 0;                      
	extern PFNGLPROGRAMUNIFORM2UIEXTPROC											glProgramUniform2uiEXT = 0;                      
	extern PFNGLPROGRAMUNIFORM3UIEXTPROC											glProgramUniform3uiEXT = 0;                      
	extern PFNGLPROGRAMUNIFORM4UIEXTPROC											glProgramUniform4uiEXT = 0;                             
	extern PFNGLPROGRAMUNIFORM1UIVEXTPROC											glProgramUniform1uivEXT = 0;                            
	extern PFNGLPROGRAMUNIFORM2UIVEXTPROC											glProgramUniform2uivEXT = 0;                            
	extern PFNGLPROGRAMUNIFORM3UIVEXTPROC											glProgramUniform3uivEXT = 0;                            
	extern PFNGLPROGRAMUNIFORM4UIVEXTPROC											glProgramUniform4uivEXT = 0;                            
	extern PFNGLNAMEDBUFFERDATAEXTPROC												glNamedBufferDataEXT = 0;                               
	extern PFNGLNAMEDBUFFERSUBDATAEXTPROC											glNamedBufferSubDataEXT = 0;                            
	extern PFNGLMAPNAMEDBUFFEREXTPROC												glMapNamedBufferEXT = 0;                                
	extern PFNGLUNMAPNAMEDBUFFEREXTPROC												glUnmapNamedBufferEXT = 0;                              
	extern PFNGLGETNAMEDBUFFERPARAMETERIVEXTPROC									glGetNamedBufferParameterivEXT = 0;                     
	extern PFNGLGETNAMEDBUFFERPOINTERVEXTPROC										glGetNamedBufferPointervEXT = 0;                        
	extern PFNGLGETNAMEDBUFFERSUBDATAEXTPROC										glGetNamedBufferSubDataEXT = 0;                         
	extern PFNGLTEXTUREBUFFEREXTPROC												glTextureBufferEXT = 0;                                 
	extern PFNGLMULTITEXBUFFEREXTPROC												glMultiTexBufferEXT = 0;                                
	extern PFNGLNAMEDRENDERBUFFERSTORAGEEXTPROC										glNamedRenderbufferStorageEXT = 0;                      
	extern PFNGLGETNAMEDRENDERBUFFERPARAMETERIVEXTPROC								glGetNamedRenderbufferParameterivEXT = 0;               
	extern PFNGLCHECKNAMEDFRAMEBUFFERSTATUSEXTPROC									glCheckNamedFramebufferStatusEXT = 0;                   
	extern PFNGLNAMEDFRAMEBUFFERTEXTURE1DEXTPROC									glNamedFramebufferTexture1DEXT = 0;                     
	extern PFNGLNAMEDFRAMEBUFFERTEXTURE2DEXTPROC									glNamedFramebufferTexture2DEXT = 0;                     
	extern PFNGLNAMEDFRAMEBUFFERTEXTURE3DEXTPROC									glNamedFramebufferTexture3DEXT = 0;                     
	extern PFNGLNAMEDFRAMEBUFFERRENDERBUFFEREXTPROC									glNamedFramebufferRenderbufferEXT = 0;                  
	extern PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC						glGetNamedFramebufferAttachmentParameterivEXT = 0;      
	extern PFNGLGENERATETEXTUREMIPMAPEXTPROC										glGenerateTextureMipmapEXT = 0;                         
	extern PFNGLGENERATEMULTITEXMIPMAPEXTPROC										glGenerateMultiTexMipmapEXT = 0;                        
	extern PFNGLFRAMEBUFFERDRAWBUFFEREXTPROC										glFramebufferDrawBufferEXT = 0;                         
	extern PFNGLFRAMEBUFFERDRAWBUFFERSEXTPROC										glFramebufferDrawBuffersEXT = 0;                        
	extern PFNGLFRAMEBUFFERREADBUFFEREXTPROC										glFramebufferReadBufferEXT = 0;                         
	extern PFNGLGETFRAMEBUFFERPARAMETERIVEXTPROC									glGetFramebufferParameterivEXT = 0;                     
	extern PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC							glNamedRenderbufferStorageMultisampleEXT = 0;           
	extern PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLECOVERAGEEXTPROC					glNamedRenderbufferStorageMultisampleCoverageEXT = 0;   
	extern PFNGLNAMEDFRAMEBUFFERTEXTUREEXTPROC										glNamedFramebufferTextureEXT = 0;                       
	extern PFNGLNAMEDFRAMEBUFFERTEXTURELAYEREXTPROC									glNamedFramebufferTextureLayerEXT = 0;                  
	extern PFNGLNAMEDFRAMEBUFFERTEXTUREFACEEXTPROC									glNamedFramebufferTextureFaceEXT = 0;                   
	extern PFNGLTEXTURERENDERBUFFEREXTPROC											glTextureRenderbufferEXT = 0;                           
	extern PFNGLMULTITEXRENDERBUFFEREXTPROC											glMultiTexRenderbufferEXT = 0;                          
	// GL_NV_explicit_multisample													
	extern PFNGLGETMULTISAMPLEFVNVPROC												glGetMultisamplefvNV = 0;
	extern PFNGLSAMPLEMASKINDEXEDNVPROC												glSampleMaskIndexedNV = 0;
	extern PFNGLTEXRENDERBUFFERNVPROC												glTexRenderbufferNV = 0;
	// GL_NV_transform_feedback2													
	extern PFNGLBINDTRANSFORMFEEDBACKNVPROC											glBindTransformFeedbackNV = 0;
	extern PFNGLDELETETRANSFORMFEEDBACKSNVPROC										glDeleteTransformFeedbacksNV = 0;
	extern PFNGLGENTRANSFORMFEEDBACKSNVPROC											glGenTransformFeedbacksNV = 0;
	extern PFNGLISTRANSFORMFEEDBACKNVPROC											glIsTransformFeedbackNV = 0;
	extern PFNGLPAUSETRANSFORMFEEDBACKNVPROC										glPauseTransformFeedbackNV = 0;
	extern PFNGLRESUMETRANSFORMFEEDBACKNVPROC										glResumeTransformFeedbackNV = 0;
	extern PFNGLDRAWTRANSFORMFEEDBACKNVPROC											glDrawTransformFeedbackNV = 0;
	// GL_AMD_performance_monitor													
	extern PFNGLGETPERFMONITORGROUPSAMDPROC											glGetPerfMonitorGroupsAMD = 0;
	extern PFNGLGETPERFMONITORCOUNTERSAMDPROC										glGetPerfMonitorCountersAMD = 0;
	extern PFNGLGETPERFMONITORGROUPSTRINGAMDPROC									glGetPerfMonitorGroupStringAMD = 0;
	extern PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC									glGetPerfMonitorCounterStringAMD = 0;
	extern PFNGLGETPERFMONITORCOUNTERINFOAMDPROC									glGetPerfMonitorCounterInfoAMD = 0;
	extern PFNGLGENPERFMONITORSAMDPROC												glGenPerfMonitorsAMD = 0;
	extern PFNGLDELETEPERFMONITORSAMDPROC											glDeletePerfMonitorsAMD = 0;
	extern PFNGLSELECTPERFMONITORCOUNTERSAMDPROC									glSelectPerfMonitorCountersAMD = 0;
	extern PFNGLBEGINPERFMONITORAMDPROC												glBeginPerfMonitorAMD = 0;
	extern PFNGLENDPERFMONITORAMDPROC												glEndPerfMonitorAMD = 0;
	extern PFNGLGETPERFMONITORCOUNTERDATAAMDPROC									glGetPerfMonitorCounterDataAMD = 0;
	// GL_AMD_vertex_shader_tesselator												
	extern PFNGLTESSELLATIONFACTORAMDPROC											glTessellationFactorAMD = 0;
	extern PFNGLTESSELLATIONMODEAMDPROC												glTessellationModeAMD = 0;
	// GL_EXT_provoking_vertex														
	extern PFNGLPROVOKINGVERTEXEXTPROC												glProvokingVertexEXT = 0;
	// GL_AMD_draw_buffers_blend													
	extern PFNGLBLENDFUNCINDEXEDAMDPROC												glBlendFuncIndexedAMD = 0;
	extern PFNGLBLENDFUNCSEPARATEINDEXEDAMDPROC										glBlendFuncSeparateIndexedAMD = 0;
	extern PFNGLBLENDEQUATIONINDEXEDAMDPROC											glBlendEquationIndexedAMD = 0;
	extern PFNGLBLENDEQUATIONSEPARATEINDEXEDAMDPROC									glBlendEquationSeparateIndexedAMD = 0;
	// GL_APPLE_texture_range														
	extern PFNGLTEXTURERANGEAPPLEPROC												glTextureRangeAPPLE = 0;
	extern PFNGLGETTEXPARAMETERPOINTERVAPPLEPROC									glGetTexParameterPointervAPPLE = 0;
	// GL_APPLE_vertex_program_evaluators											
	extern PFNGLENABLEVERTEXATTRIBAPPLEPROC											glEnableVertexAttribAPPLE = 0;
	extern PFNGLDISABLEVERTEXATTRIBAPPLEPROC										glDisableVertexAttribAPPLE = 0;
	extern PFNGLISVERTEXATTRIBENABLEDAPPLEPROC										glIsVertexAttribEnabledAPPLE = 0;
	extern PFNGLMAPVERTEXATTRIB1DAPPLEPROC											glMapVertexAttrib1dAPPLE = 0;
	extern PFNGLMAPVERTEXATTRIB1FAPPLEPROC											glMapVertexAttrib1fAPPLE = 0;
	extern PFNGLMAPVERTEXATTRIB2DAPPLEPROC											glMapVertexAttrib2dAPPLE = 0;
	extern PFNGLMAPVERTEXATTRIB2FAPPLEPROC											glMapVertexAttrib2fAPPLE = 0;
	// GL_APPLE_object_purgeable													
	extern PFNGLOBJECTPURGEABLEAPPLEPROC											glObjectPurgeableAPPLE = 0;
	extern PFNGLOBJECTUNPURGEABLEAPPLEPROC											glObjectUnpurgeableAPPLE = 0;
	extern PFNGLGETOBJECTPARAMETERIVAPPLEPROC										glGetObjectParameterivAPPLE = 0;
	// GL_NV_video_capture															
	extern PFNGLBEGINVIDEOCAPTURENVPROC												glBeginVideoCaptureNV = 0;
	extern PFNGLBINDVIDEOCAPTURESTREAMBUFFERNVPROC									glBindVideoCaptureStreamBufferNV = 0;
	extern PFNGLBINDVIDEOCAPTURESTREAMTEXTURENVPROC									glBindVideoCaptureStreamTextureNV = 0;
	extern PFNGLENDVIDEOCAPTURENVPROC												glEndVideoCaptureNV = 0;
	extern PFNGLGETVIDEOCAPTUREIVNVPROC												glGetVideoCaptureivNV = 0;
	extern PFNGLGETVIDEOCAPTURESTREAMIVNVPROC										glGetVideoCaptureStreamivNV = 0;
	extern PFNGLGETVIDEOCAPTURESTREAMFVNVPROC										glGetVideoCaptureStreamfvNV = 0;
	extern PFNGLGETVIDEOCAPTURESTREAMDVNVPROC										glGetVideoCaptureStreamdvNV = 0;
	extern PFNGLVIDEOCAPTURENVPROC													glVideoCaptureNV = 0;
	extern PFNGLVIDEOCAPTURESTREAMPARAMETERIVNVPROC									glVideoCaptureStreamParameterivNV = 0;
	extern PFNGLVIDEOCAPTURESTREAMPARAMETERFVNVPROC									glVideoCaptureStreamParameterfvNV = 0;
	extern PFNGLVIDEOCAPTURESTREAMPARAMETERDVNVPROC									glVideoCaptureStreamParameterdvNV = 0;
	// GL_NV_copy_image																
	extern PFNGLCOPYIMAGESUBDATANVPROC												glCopyImageSubDataNV = 0;
	// GL_EXT_separate_shader_objects												
	extern PFNGLUSESHADERPROGRAMEXTPROC												glUseShaderProgramEXT = 0;
	extern PFNGLACTIVEPROGRAMEXTPROC												glActiveProgramEXT = 0;
	extern PFNGLCREATESHADERPROGRAMEXTPROC											glCreateShaderProgramEXT = 0;
	// GL_NV_shader_buffer_load														
	extern PFNGLMAKEBUFFERRESIDENTNVPROC											glMakeBufferResidentNV = 0;
	extern PFNGLMAKEBUFFERNONRESIDENTNVPROC											glMakeBufferNonResidentNV = 0;
	extern PFNGLISBUFFERRESIDENTNVPROC												glIsBufferResidentNV = 0;
	extern PFNGLNAMEDMAKEBUFFERRESIDENTNVPROC										glNamedMakeBufferResidentNV = 0;
	extern PFNGLNAMEDMAKEBUFFERNONRESIDENTNVPROC									glNamedMakeBufferNonResidentNV = 0;
	extern PFNGLISNAMEDBUFFERRESIDENTNVPROC											glIsNamedBufferResidentNV = 0;
	extern PFNGLGETBUFFERPARAMETERUI64VNVPROC										glGetBufferParameterui64vNV = 0;
	extern PFNGLGETNAMEDBUFFERPARAMETERUI64VNVPROC									glGetNamedBufferParameterui64vNV = 0;
	extern PFNGLGETINTEGERUI64VNVPROC												glGetIntegerui64vNV = 0;
	extern PFNGLUNIFORMUI64NVPROC													glUniformui64NV = 0;
	extern PFNGLUNIFORMUI64VNVPROC													glUniformui64vNV = 0;
	extern PFNGLGETUNIFORMUI64VNVPROC												glGetUniformui64vNV = 0;
	extern PFNGLPROGRAMUNIFORMUI64NVPROC											glProgramUniformui64NV = 0;
	extern PFNGLPROGRAMUNIFORMUI64VNVPROC											glProgramUniformui64vNV = 0;
	// GL_NV_vertex_buffer_unified_memory											
	extern PFNGLBUFFERADDRESSRANGENVPROC											glBufferAddressRangeNV = 0;
	extern PFNGLVERTEXFORMATNVPROC													glVertexFormatNV = 0;
	extern PFNGLNORMALFORMATNVPROC													glNormalFormatNV = 0;
	extern PFNGLCOLORFORMATNVPROC													glColorFormatNV = 0;
	extern PFNGLINDEXFORMATNVPROC													glIndexFormatNV = 0;
	extern PFNGLTEXCOORDFORMATNVPROC												glTexCoordFormatNV = 0;
	extern PFNGLEDGEFLAGFORMATNVPROC												glEdgeFlagFormatNV = 0;
	extern PFNGLSECONDARYCOLORFORMATNVPROC											glSecondaryColorFormatNV = 0;
	extern PFNGLFOGCOORDFORMATNVPROC												glFogCoordFormatNV = 0;
	extern PFNGLVERTEXATTRIBFORMATNVPROC											glVertexAttribFormatNV = 0;
	extern PFNGLVERTEXATTRIBIFORMATNVPROC											glVertexAttribIFormatNV = 0;
	extern PFNGLGETINTEGERUI64I_VNVPROC												glGetIntegerui64i_vNV = 0;
	// GL_NV_texture_barrier
	extern PFNGLTEXTUREBARRIERNVPROC												glTextureBarrierNV = 0;
	////////////////////////////////////////////////////////////////////////////////
}

using namespace OpenGL;
using namespace DKFoundation;

#define GET_GL_EXT_PROC(func)		*(PROC*)&func = wglGetProcAddress(#func)

namespace DKFramework
{
	namespace Private
	{
		bool InitOpenGLExtensions(void)
		{
			const char* versionString = (char*)glGetString(GL_VERSION);
			if (versionString == NULL)
			{
				DKLog("CRITICAL ERROR: glGetString(GL_VERSION) returns NULL\n");
				return false;
			}
			double version = atof(versionString);
			DKLog("OpenGL Context Version %s (%f)\n", versionString, version);
			if (version >= 1.2)		// OpenGL 1.2
			{
				GET_GL_EXT_PROC(glBlendColor);
				GET_GL_EXT_PROC(glBlendEquation);
				GET_GL_EXT_PROC(glDrawRangeElements);
				GET_GL_EXT_PROC(glTexImage3D);
				GET_GL_EXT_PROC(glTexSubImage3D);
				GET_GL_EXT_PROC(glCopyTexSubImage3D);
			}
			if (version >= 1.3)		// OpenGL 1.3
			{
				GET_GL_EXT_PROC(glActiveTexture);
				GET_GL_EXT_PROC(glSampleCoverage);
				GET_GL_EXT_PROC(glCompressedTexImage3D);
				GET_GL_EXT_PROC(glCompressedTexImage2D);
				GET_GL_EXT_PROC(glCompressedTexImage1D);
				GET_GL_EXT_PROC(glCompressedTexSubImage3D);
				GET_GL_EXT_PROC(glCompressedTexSubImage2D);
				GET_GL_EXT_PROC(glCompressedTexSubImage1D);
				GET_GL_EXT_PROC(glGetCompressedTexImage);
			}
			if (version >= 1.4)		// OpenGL 1.4
			{
				GET_GL_EXT_PROC(glBlendFuncSeparate);
				GET_GL_EXT_PROC(glMultiDrawArrays);
				GET_GL_EXT_PROC(glMultiDrawElements);
				GET_GL_EXT_PROC(glPointParameterf);
				GET_GL_EXT_PROC(glPointParameterfv);
				GET_GL_EXT_PROC(glPointParameteri);
				GET_GL_EXT_PROC(glPointParameteriv);
			}
			if (version >= 1.5)		// OpenGL 1.5
			{
				GET_GL_EXT_PROC(glGenQueries);
				GET_GL_EXT_PROC(glDeleteQueries);
				GET_GL_EXT_PROC(glIsQuery);
				GET_GL_EXT_PROC(glBeginQuery);
				GET_GL_EXT_PROC(glEndQuery);
				GET_GL_EXT_PROC(glGetQueryiv);
				GET_GL_EXT_PROC(glGetQueryObjectiv);
				GET_GL_EXT_PROC(glGetQueryObjectuiv);
				GET_GL_EXT_PROC(glBindBuffer);
				GET_GL_EXT_PROC(glDeleteBuffers);
				GET_GL_EXT_PROC(glGenBuffers);
				GET_GL_EXT_PROC(glIsBuffer);
				GET_GL_EXT_PROC(glBufferData);
				GET_GL_EXT_PROC(glBufferSubData);
				GET_GL_EXT_PROC(glGetBufferSubData);
				GET_GL_EXT_PROC(glMapBuffer);
				GET_GL_EXT_PROC(glUnmapBuffer);
				GET_GL_EXT_PROC(glGetBufferParameteriv);
				GET_GL_EXT_PROC(glGetBufferPointerv);
			}
			if (version >= 2.0)		// OpenGL 2.0
			{
				GET_GL_EXT_PROC(glBlendEquationSeparate);
				GET_GL_EXT_PROC(glDrawBuffers);
				GET_GL_EXT_PROC(glStencilOpSeparate);
				GET_GL_EXT_PROC(glStencilFuncSeparate);
				GET_GL_EXT_PROC(glStencilMaskSeparate);
				GET_GL_EXT_PROC(glAttachShader);
				GET_GL_EXT_PROC(glBindAttribLocation);
				GET_GL_EXT_PROC(glCompileShader);
				GET_GL_EXT_PROC(glCreateProgram);
				GET_GL_EXT_PROC(glCreateShader);
				GET_GL_EXT_PROC(glDeleteProgram);
				GET_GL_EXT_PROC(glDeleteShader);
				GET_GL_EXT_PROC(glDetachShader);
				GET_GL_EXT_PROC(glDisableVertexAttribArray);
				GET_GL_EXT_PROC(glEnableVertexAttribArray);
				GET_GL_EXT_PROC(glGetActiveAttrib);
				GET_GL_EXT_PROC(glGetActiveUniform);
				GET_GL_EXT_PROC(glGetAttachedShaders);
				GET_GL_EXT_PROC(glGetAttribLocation);
				GET_GL_EXT_PROC(glGetProgramiv);
				GET_GL_EXT_PROC(glGetProgramInfoLog);
				GET_GL_EXT_PROC(glGetShaderiv);
				GET_GL_EXT_PROC(glGetShaderInfoLog);
				GET_GL_EXT_PROC(glGetShaderSource);
				GET_GL_EXT_PROC(glGetUniformLocation);
				GET_GL_EXT_PROC(glGetUniformfv);
				GET_GL_EXT_PROC(glGetUniformiv);
				GET_GL_EXT_PROC(glGetVertexAttribdv);
				GET_GL_EXT_PROC(glGetVertexAttribfv);
				GET_GL_EXT_PROC(glGetVertexAttribiv);
				GET_GL_EXT_PROC(glGetVertexAttribPointerv);
				GET_GL_EXT_PROC(glIsProgram);
				GET_GL_EXT_PROC(glIsShader);
				GET_GL_EXT_PROC(glLinkProgram);
				GET_GL_EXT_PROC(glShaderSource);
				GET_GL_EXT_PROC(glUseProgram);
				GET_GL_EXT_PROC(glUniform1f);
				GET_GL_EXT_PROC(glUniform2f);
				GET_GL_EXT_PROC(glUniform3f);
				GET_GL_EXT_PROC(glUniform4f);
				GET_GL_EXT_PROC(glUniform1i);
				GET_GL_EXT_PROC(glUniform2i);
				GET_GL_EXT_PROC(glUniform3i);
				GET_GL_EXT_PROC(glUniform4i);
				GET_GL_EXT_PROC(glUniform1fv);
				GET_GL_EXT_PROC(glUniform2fv);
				GET_GL_EXT_PROC(glUniform3fv);
				GET_GL_EXT_PROC(glUniform4fv);
				GET_GL_EXT_PROC(glUniform1iv);
				GET_GL_EXT_PROC(glUniform2iv);
				GET_GL_EXT_PROC(glUniform3iv);
				GET_GL_EXT_PROC(glUniform4iv);
				GET_GL_EXT_PROC(glUniformMatrix2fv);
				GET_GL_EXT_PROC(glUniformMatrix3fv);
				GET_GL_EXT_PROC(glUniformMatrix4fv);
				GET_GL_EXT_PROC(glValidateProgram);
				GET_GL_EXT_PROC(glVertexAttrib1d);
				GET_GL_EXT_PROC(glVertexAttrib1dv);
				GET_GL_EXT_PROC(glVertexAttrib1f);
				GET_GL_EXT_PROC(glVertexAttrib1fv);
				GET_GL_EXT_PROC(glVertexAttrib1s);
				GET_GL_EXT_PROC(glVertexAttrib1sv);
				GET_GL_EXT_PROC(glVertexAttrib2d);
				GET_GL_EXT_PROC(glVertexAttrib2dv);
				GET_GL_EXT_PROC(glVertexAttrib2f);
				GET_GL_EXT_PROC(glVertexAttrib2fv);
				GET_GL_EXT_PROC(glVertexAttrib2s);
				GET_GL_EXT_PROC(glVertexAttrib2sv);
				GET_GL_EXT_PROC(glVertexAttrib3d);
				GET_GL_EXT_PROC(glVertexAttrib3dv);
				GET_GL_EXT_PROC(glVertexAttrib3f);
				GET_GL_EXT_PROC(glVertexAttrib3fv);
				GET_GL_EXT_PROC(glVertexAttrib3s);
				GET_GL_EXT_PROC(glVertexAttrib3sv);
				GET_GL_EXT_PROC(glVertexAttrib4Nbv);
				GET_GL_EXT_PROC(glVertexAttrib4Niv);
				GET_GL_EXT_PROC(glVertexAttrib4Nsv);
				GET_GL_EXT_PROC(glVertexAttrib4Nub);
				GET_GL_EXT_PROC(glVertexAttrib4Nubv);
				GET_GL_EXT_PROC(glVertexAttrib4Nuiv);
				GET_GL_EXT_PROC(glVertexAttrib4Nusv);
				GET_GL_EXT_PROC(glVertexAttrib4bv);
				GET_GL_EXT_PROC(glVertexAttrib4d);
				GET_GL_EXT_PROC(glVertexAttrib4dv);
				GET_GL_EXT_PROC(glVertexAttrib4f);
				GET_GL_EXT_PROC(glVertexAttrib4fv);
				GET_GL_EXT_PROC(glVertexAttrib4iv);
				GET_GL_EXT_PROC(glVertexAttrib4s);
				GET_GL_EXT_PROC(glVertexAttrib4sv);
				GET_GL_EXT_PROC(glVertexAttrib4ubv);
				GET_GL_EXT_PROC(glVertexAttrib4uiv);
				GET_GL_EXT_PROC(glVertexAttrib4usv);
				GET_GL_EXT_PROC(glVertexAttribPointer);
			}
			if (version >= 2.1)		// OpenGL 2.1
			{
				GET_GL_EXT_PROC(glUniformMatrix2x3fv);
				GET_GL_EXT_PROC(glUniformMatrix3x2fv);
				GET_GL_EXT_PROC(glUniformMatrix2x4fv);
				GET_GL_EXT_PROC(glUniformMatrix4x2fv);
				GET_GL_EXT_PROC(glUniformMatrix3x4fv);
				GET_GL_EXT_PROC(glUniformMatrix4x3fv);
			}
			if (version >= 3.0)		// OpenGL 3.0
			{
				// ARB_framebuffer_object 
				GET_GL_EXT_PROC(glIsRenderbuffer);						
				GET_GL_EXT_PROC(glBindRenderbuffer);						
				GET_GL_EXT_PROC(glDeleteRenderbuffers);					
				GET_GL_EXT_PROC(glGenRenderbuffers);						
				GET_GL_EXT_PROC(glRenderbufferStorage);					
				GET_GL_EXT_PROC(glGetRenderbufferParameteriv);			
				GET_GL_EXT_PROC(glIsFramebuffer);						
				GET_GL_EXT_PROC(glBindFramebuffer);						
				GET_GL_EXT_PROC(glDeleteFramebuffers);					
				GET_GL_EXT_PROC(glGenFramebuffers);						
				GET_GL_EXT_PROC(glCheckFramebufferStatus);				
				GET_GL_EXT_PROC(glFramebufferTexture1D);					
				GET_GL_EXT_PROC(glFramebufferTexture2D);					
				GET_GL_EXT_PROC(glFramebufferTexture3D);					
				GET_GL_EXT_PROC(glFramebufferRenderbuffer);				
				GET_GL_EXT_PROC(glGetFramebufferAttachmentParameteriv);	
				GET_GL_EXT_PROC(glGenerateMipmap);
				GET_GL_EXT_PROC(glBlitFramebuffer);
				GET_GL_EXT_PROC(glRenderbufferStorageMultisample);
				GET_GL_EXT_PROC(glFramebufferTextureLayer);
				// GL_ARB_map_buffer_range
				GET_GL_EXT_PROC(glMapBufferRange);
				GET_GL_EXT_PROC(glFlushMappedBufferRange);
				// GL_ARB_vertex_array_object
				GET_GL_EXT_PROC(glBindVertexArray);
				GET_GL_EXT_PROC(glDeleteVertexArrays);
				GET_GL_EXT_PROC(glGenVertexArrays);
				GET_GL_EXT_PROC(glIsVertexArray);
				// OpenGL 3.0 new api
				GET_GL_EXT_PROC(glColorMaski);
				GET_GL_EXT_PROC(glGetBooleani_v);
				GET_GL_EXT_PROC(glGetIntegeri_v);
				GET_GL_EXT_PROC(glEnablei);
				GET_GL_EXT_PROC(glDisablei);
				GET_GL_EXT_PROC(glIsEnabledi);
				GET_GL_EXT_PROC(glBeginTransformFeedback);
				GET_GL_EXT_PROC(glEndTransformFeedback);
				GET_GL_EXT_PROC(glBindBufferRange);
				GET_GL_EXT_PROC(glBindBufferBase);
				GET_GL_EXT_PROC(glTransformFeedbackVaryings);
				GET_GL_EXT_PROC(glGetTransformFeedbackVarying);
				GET_GL_EXT_PROC(glClampColor);
				GET_GL_EXT_PROC(glBeginConditionalRender);
				GET_GL_EXT_PROC(glEndConditionalRender);
				GET_GL_EXT_PROC(glVertexAttribIPointer);
				GET_GL_EXT_PROC(glGetVertexAttribIiv);
				GET_GL_EXT_PROC(glGetVertexAttribIuiv);
				GET_GL_EXT_PROC(glVertexAttribI1i);
				GET_GL_EXT_PROC(glVertexAttribI2i);
				GET_GL_EXT_PROC(glVertexAttribI3i);
				GET_GL_EXT_PROC(glVertexAttribI4i);
				GET_GL_EXT_PROC(glVertexAttribI1ui);
				GET_GL_EXT_PROC(glVertexAttribI2ui);
				GET_GL_EXT_PROC(glVertexAttribI3ui);
				GET_GL_EXT_PROC(glVertexAttribI4ui);
				GET_GL_EXT_PROC(glVertexAttribI1iv);
				GET_GL_EXT_PROC(glVertexAttribI2iv);
				GET_GL_EXT_PROC(glVertexAttribI3iv);
				GET_GL_EXT_PROC(glVertexAttribI4iv);
				GET_GL_EXT_PROC(glVertexAttribI1uiv);
				GET_GL_EXT_PROC(glVertexAttribI2uiv);
				GET_GL_EXT_PROC(glVertexAttribI3uiv);
				GET_GL_EXT_PROC(glVertexAttribI4uiv);
				GET_GL_EXT_PROC(glVertexAttribI4bv);
				GET_GL_EXT_PROC(glVertexAttribI4sv);
				GET_GL_EXT_PROC(glVertexAttribI4ubv);
				GET_GL_EXT_PROC(glVertexAttribI4usv);
				GET_GL_EXT_PROC(glGetUniformuiv);
				GET_GL_EXT_PROC(glBindFragDataLocation);
				GET_GL_EXT_PROC(glGetFragDataLocation);
				GET_GL_EXT_PROC(glUniform1ui);
				GET_GL_EXT_PROC(glUniform2ui);
				GET_GL_EXT_PROC(glUniform3ui);
				GET_GL_EXT_PROC(glUniform4ui);
				GET_GL_EXT_PROC(glUniform1uiv);
				GET_GL_EXT_PROC(glUniform2uiv);
				GET_GL_EXT_PROC(glUniform3uiv);
				GET_GL_EXT_PROC(glUniform4uiv);
				GET_GL_EXT_PROC(glTexParameterIiv);
				GET_GL_EXT_PROC(glTexParameterIuiv);
				GET_GL_EXT_PROC(glGetTexParameterIiv);
				GET_GL_EXT_PROC(glGetTexParameterIuiv);
				GET_GL_EXT_PROC(glClearBufferiv);
				GET_GL_EXT_PROC(glClearBufferuiv);
				GET_GL_EXT_PROC(glClearBufferfv);
				GET_GL_EXT_PROC(glClearBufferfi);
				GET_GL_EXT_PROC(glGetStringi);
			}
			if (version >= 3.1)		// OpenGL 3.1
			{
				// GL_ARB_copy_buffer
				GET_GL_EXT_PROC(glCopyBufferSubData);
				// GL_ARB_uniform_buffer_object
				GET_GL_EXT_PROC(glGetUniformIndices);
				GET_GL_EXT_PROC(glGetActiveUniformsiv);
				GET_GL_EXT_PROC(glGetActiveUniformName);
				GET_GL_EXT_PROC(glGetUniformBlockIndex);
				GET_GL_EXT_PROC(glGetActiveUniformBlockiv);
				GET_GL_EXT_PROC(glGetActiveUniformBlockName);
				GET_GL_EXT_PROC(glUniformBlockBinding);
				// OpenGL 3.1 new api
				GET_GL_EXT_PROC(glDrawArraysInstanced);
				GET_GL_EXT_PROC(glDrawElementsInstanced);
				GET_GL_EXT_PROC(glTexBuffer);
				GET_GL_EXT_PROC(glPrimitiveRestartIndex);
			}
			if (version >= 3.2)		// OpenGL 3.2
			{
				// GL_ARB_draw_elements_base_vertex
				GET_GL_EXT_PROC(glDrawElementsBaseVertex);
				GET_GL_EXT_PROC(glDrawRangeElementsBaseVertex);
				GET_GL_EXT_PROC(glDrawElementsInstancedBaseVertex);
				GET_GL_EXT_PROC(glMultiDrawElementsBaseVertex);
				// GL_ARB_provoking_vertex
				GET_GL_EXT_PROC(glProvokingVertex);
				// GL_ARB_sync
				GET_GL_EXT_PROC(glFenceSync);
				GET_GL_EXT_PROC(glIsSync);
				GET_GL_EXT_PROC(glDeleteSync);
				GET_GL_EXT_PROC(glClientWaitSync);
				GET_GL_EXT_PROC(glWaitSync);
				GET_GL_EXT_PROC(glGetInteger64v);
				GET_GL_EXT_PROC(glGetSynciv);
				// GL_ARB_texture_multisample
				GET_GL_EXT_PROC(glTexImage2DMultisample);
				GET_GL_EXT_PROC(glTexImage3DMultisample);
				GET_GL_EXT_PROC(glGetMultisamplefv);
				GET_GL_EXT_PROC(glSampleMaski);
				// OpenGL 3.2 new api
				GET_GL_EXT_PROC(glGetInteger64i_v);
				GET_GL_EXT_PROC(glGetBufferParameteri64v);
				GET_GL_EXT_PROC(glProgramParameteri);
				GET_GL_EXT_PROC(glFramebufferTexture);
				GET_GL_EXT_PROC(glFramebufferTextureFace);
			}
			////////////////////////////////////////////////////////////////////////////////
			// load extensions
			char* ext = (char*)glGetString(GL_EXTENSIONS);
			if (ext == NULL)
			{
				DKLog("Extension error: Failed to get OpenGL Extensions (Check context compatibility options.\n");
				return true;
			}
			if (strstr(ext, "GL_ARB_multitexture"))
			{
				GET_GL_EXT_PROC(glActiveTextureARB);			
				GET_GL_EXT_PROC(glClientActiveTextureARB);	
				GET_GL_EXT_PROC(glMultiTexCoord1dARB);		
				GET_GL_EXT_PROC(glMultiTexCoord1dvARB);		
				GET_GL_EXT_PROC(glMultiTexCoord1fARB);		
				GET_GL_EXT_PROC(glMultiTexCoord1fvARB);		
				GET_GL_EXT_PROC(glMultiTexCoord1iARB); 
				GET_GL_EXT_PROC(glMultiTexCoord1ivARB);
				GET_GL_EXT_PROC(glMultiTexCoord1sARB); 
				GET_GL_EXT_PROC(glMultiTexCoord1svARB);
				GET_GL_EXT_PROC(glMultiTexCoord2dARB); 
				GET_GL_EXT_PROC(glMultiTexCoord2dvARB);
				GET_GL_EXT_PROC(glMultiTexCoord2fARB); 
				GET_GL_EXT_PROC(glMultiTexCoord2fvARB);
				GET_GL_EXT_PROC(glMultiTexCoord2iARB); 
				GET_GL_EXT_PROC(glMultiTexCoord2ivARB);
				GET_GL_EXT_PROC(glMultiTexCoord2sARB); 
				GET_GL_EXT_PROC(glMultiTexCoord2svARB);
				GET_GL_EXT_PROC(glMultiTexCoord3dARB); 
				GET_GL_EXT_PROC(glMultiTexCoord3dvARB);
				GET_GL_EXT_PROC(glMultiTexCoord3fARB); 
				GET_GL_EXT_PROC(glMultiTexCoord3fvARB);
				GET_GL_EXT_PROC(glMultiTexCoord3iARB); 
				GET_GL_EXT_PROC(glMultiTexCoord3ivARB);
				GET_GL_EXT_PROC(glMultiTexCoord3sARB); 
				GET_GL_EXT_PROC(glMultiTexCoord3svARB);
				GET_GL_EXT_PROC(glMultiTexCoord4dARB); 
				GET_GL_EXT_PROC(glMultiTexCoord4dvARB);
				GET_GL_EXT_PROC(glMultiTexCoord4fARB); 
				GET_GL_EXT_PROC(glMultiTexCoord4fvARB);
				GET_GL_EXT_PROC(glMultiTexCoord4iARB); 
				GET_GL_EXT_PROC(glMultiTexCoord4ivARB);
				GET_GL_EXT_PROC(glMultiTexCoord4sARB); 
				GET_GL_EXT_PROC(glMultiTexCoord4svARB);
			}
			if (strstr(ext, "GL_ARB_transpose_matrix"))
			{
				GET_GL_EXT_PROC(glLoadTransposeMatrixfARB);
				GET_GL_EXT_PROC(glLoadTransposeMatrixdARB);
				GET_GL_EXT_PROC(glMultTransposeMatrixfARB);
				GET_GL_EXT_PROC(glMultTransposeMatrixdARB);
			}
			if (strstr(ext, "GL_ARB_multisample"))
			{
				GET_GL_EXT_PROC(glSampleCoverageARB);
			}
			if (strstr(ext, "GL_ARB_texture_compression"))
			{
				GET_GL_EXT_PROC(glCompressedTexImage3DARB);
				GET_GL_EXT_PROC(glCompressedTexImage2DARB);
				GET_GL_EXT_PROC(glCompressedTexImage1DARB);
				GET_GL_EXT_PROC(glCompressedTexSubImage3DARB);
				GET_GL_EXT_PROC(glCompressedTexSubImage2DARB);
				GET_GL_EXT_PROC(glCompressedTexSubImage1DARB);
				GET_GL_EXT_PROC(glGetCompressedTexImageARB);
			}
			if (strstr(ext, "GL_ARB_point_parameters"))
			{
				GET_GL_EXT_PROC(glPointParameterfARB);
				GET_GL_EXT_PROC(glPointParameterfvARB);
			}
			if (strstr(ext, "GL_ARB_vertex_blend"))
			{
				GET_GL_EXT_PROC(glWeightbvARB);
				GET_GL_EXT_PROC(glWeightsvARB);
				GET_GL_EXT_PROC(glWeightivARB);
				GET_GL_EXT_PROC(glWeightfvARB);
				GET_GL_EXT_PROC(glWeightdvARB);
				GET_GL_EXT_PROC(glWeightubvARB);
				GET_GL_EXT_PROC(glWeightusvARB);
				GET_GL_EXT_PROC(glWeightuivARB);
				GET_GL_EXT_PROC(glWeightPointerARB);
				GET_GL_EXT_PROC(glVertexBlendARB);
			}
			if (strstr(ext, "GL_ARB_matrix_palette"))
			{
				GET_GL_EXT_PROC(glCurrentPaletteMatrixARB);
				GET_GL_EXT_PROC(glMatrixIndexubvARB);
				GET_GL_EXT_PROC(glMatrixIndexusvARB);
				GET_GL_EXT_PROC(glMatrixIndexuivARB);
				GET_GL_EXT_PROC(glMatrixIndexPointerARB);
			}
			if (strstr(ext, "GL_ARB_window_pos"))
			{
				GET_GL_EXT_PROC(glWindowPos2dARB); 
				GET_GL_EXT_PROC(glWindowPos2dvARB);
				GET_GL_EXT_PROC(glWindowPos2fARB); 
				GET_GL_EXT_PROC(glWindowPos2fvARB);
				GET_GL_EXT_PROC(glWindowPos2iARB); 
				GET_GL_EXT_PROC(glWindowPos2ivARB);
				GET_GL_EXT_PROC(glWindowPos2sARB); 
				GET_GL_EXT_PROC(glWindowPos2svARB);
				GET_GL_EXT_PROC(glWindowPos3dARB); 
				GET_GL_EXT_PROC(glWindowPos3dvARB);
				GET_GL_EXT_PROC(glWindowPos3fARB); 
				GET_GL_EXT_PROC(glWindowPos3fvARB);
				GET_GL_EXT_PROC(glWindowPos3iARB); 
				GET_GL_EXT_PROC(glWindowPos3ivARB);
				GET_GL_EXT_PROC(glWindowPos3sARB); 
				GET_GL_EXT_PROC(glWindowPos3svARB);
			}
			if (strstr(ext, "GL_ARB_vertex_program"))
			{
				GET_GL_EXT_PROC(glVertexAttrib1dARB);                                              
				GET_GL_EXT_PROC(glVertexAttrib1dvARB);                                             
				GET_GL_EXT_PROC(glVertexAttrib1fARB);                                              
				GET_GL_EXT_PROC(glVertexAttrib1fvARB);                                             
				GET_GL_EXT_PROC(glVertexAttrib1sARB);                                              
				GET_GL_EXT_PROC(glVertexAttrib1svARB);                                             
				GET_GL_EXT_PROC(glVertexAttrib2dARB);                                              
				GET_GL_EXT_PROC(glVertexAttrib2dvARB);                                             
				GET_GL_EXT_PROC(glVertexAttrib2fARB);                                              
				GET_GL_EXT_PROC(glVertexAttrib2fvARB);                                             
				GET_GL_EXT_PROC(glVertexAttrib2sARB);                                              
				GET_GL_EXT_PROC(glVertexAttrib2svARB);                                             
				GET_GL_EXT_PROC(glVertexAttrib3dARB);                                              
				GET_GL_EXT_PROC(glVertexAttrib3dvARB);                                             
				GET_GL_EXT_PROC(glVertexAttrib3fARB);                                              
				GET_GL_EXT_PROC(glVertexAttrib3fvARB);                                             
				GET_GL_EXT_PROC(glVertexAttrib3sARB);                                              
				GET_GL_EXT_PROC(glVertexAttrib3svARB);                                             
				GET_GL_EXT_PROC(glVertexAttrib4NbvARB);                                            
				GET_GL_EXT_PROC(glVertexAttrib4NivARB);                                            
				GET_GL_EXT_PROC(glVertexAttrib4NsvARB);                                            
				GET_GL_EXT_PROC(glVertexAttrib4NubARB);                                            
				GET_GL_EXT_PROC(glVertexAttrib4NubvARB);                                           
				GET_GL_EXT_PROC(glVertexAttrib4NuivARB);                                           
				GET_GL_EXT_PROC(glVertexAttrib4NusvARB);                                           
				GET_GL_EXT_PROC(glVertexAttrib4bvARB);                                             
				GET_GL_EXT_PROC(glVertexAttrib4dARB);                                              
				GET_GL_EXT_PROC(glVertexAttrib4dvARB);                                             
				GET_GL_EXT_PROC(glVertexAttrib4fARB);                                              
				GET_GL_EXT_PROC(glVertexAttrib4fvARB);                                             
				GET_GL_EXT_PROC(glVertexAttrib4ivARB);                                             
				GET_GL_EXT_PROC(glVertexAttrib4sARB);                                              
				GET_GL_EXT_PROC(glVertexAttrib4svARB);                                             
				GET_GL_EXT_PROC(glVertexAttrib4ubvARB);                                            
				GET_GL_EXT_PROC(glVertexAttrib4uivARB);                                            
				GET_GL_EXT_PROC(glVertexAttrib4usvARB);                                            
				GET_GL_EXT_PROC(glVertexAttribPointerARB);                                         
				GET_GL_EXT_PROC(glEnableVertexAttribArrayARB);                                     
				GET_GL_EXT_PROC(glDisableVertexAttribArrayARB);                                    
				GET_GL_EXT_PROC(glProgramStringARB);                                               
				GET_GL_EXT_PROC(glBindProgramARB);                                                 
				GET_GL_EXT_PROC(glDeleteProgramsARB);                                              
				GET_GL_EXT_PROC(glGenProgramsARB);                                                 
				GET_GL_EXT_PROC(glProgramEnvParameter4dARB);                                       
				GET_GL_EXT_PROC(glProgramEnvParameter4dvARB);                                      
				GET_GL_EXT_PROC(glProgramEnvParameter4fARB);                                       
				GET_GL_EXT_PROC(glProgramEnvParameter4fvARB);                                      
				GET_GL_EXT_PROC(glProgramLocalParameter4dARB);                                     
				GET_GL_EXT_PROC(glProgramLocalParameter4dvARB);                                    
				GET_GL_EXT_PROC(glProgramLocalParameter4fARB);                                     
				GET_GL_EXT_PROC(glProgramLocalParameter4fvARB);                                    
				GET_GL_EXT_PROC(glGetProgramEnvParameterdvARB);                                    
				GET_GL_EXT_PROC(glGetProgramEnvParameterfvARB);                                    
				GET_GL_EXT_PROC(glGetProgramLocalParameterdvARB);                                  
				GET_GL_EXT_PROC(glGetProgramLocalParameterfvARB);                                  
				GET_GL_EXT_PROC(glGetProgramivARB);                                                
				GET_GL_EXT_PROC(glGetProgramStringARB);                                            
				GET_GL_EXT_PROC(glGetVertexAttribdvARB);                                           
				GET_GL_EXT_PROC(glGetVertexAttribfvARB);                                           
				GET_GL_EXT_PROC(glGetVertexAttribivARB);                                           
				GET_GL_EXT_PROC(glGetVertexAttribPointervARB);                                     
				GET_GL_EXT_PROC(glIsProgramARB); 
			}
			if (strstr(ext, "GL_ARB_vertex_buffer_object"))
			{
				GET_GL_EXT_PROC(glBindBufferARB);			
				GET_GL_EXT_PROC(glDeleteBuffersARB);				
				GET_GL_EXT_PROC(glGenBuffersARB);				
				GET_GL_EXT_PROC(glIsBufferARB);				
				GET_GL_EXT_PROC(glBufferDataARB);				
				GET_GL_EXT_PROC(glBufferSubDataARB);			
				GET_GL_EXT_PROC(glGetBufferSubDataARB);		
				GET_GL_EXT_PROC(glMapBufferARB);					
				GET_GL_EXT_PROC(glUnmapBufferARB);				
				GET_GL_EXT_PROC(glGetBufferParameterivARB);	
				GET_GL_EXT_PROC(glGetBufferPointervARB);		
			}
			if (strstr(ext, "GL_ARB_occlusion_query"))
			{
				GET_GL_EXT_PROC(glGenQueriesARB);
				GET_GL_EXT_PROC(glDeleteQueriesARB);
				GET_GL_EXT_PROC(glIsQueryARB);
				GET_GL_EXT_PROC(glBeginQueryARB);
				GET_GL_EXT_PROC(glEndQueryARB);
				GET_GL_EXT_PROC(glGetQueryivARB);
				GET_GL_EXT_PROC(glGetQueryObjectivARB);
				GET_GL_EXT_PROC(glGetQueryObjectuivARB);
			}
			if (strstr(ext, "GL_ARB_shader_objects"))
			{
				GET_GL_EXT_PROC(glDeleteObjectARB);			
				GET_GL_EXT_PROC(glGetHandleARB);					
				GET_GL_EXT_PROC(glDetachObjectARB);				
				GET_GL_EXT_PROC(glCreateShaderObjectARB);		
				GET_GL_EXT_PROC(glShaderSourceARB);				
				GET_GL_EXT_PROC(glCompileShaderARB);			
				GET_GL_EXT_PROC(glCreateProgramObjectARB);	
				GET_GL_EXT_PROC(glAttachObjectARB);				
				GET_GL_EXT_PROC(glLinkProgramARB);				
				GET_GL_EXT_PROC(glUseProgramObjectARB);		
				GET_GL_EXT_PROC(glValidateProgramARB);		
				GET_GL_EXT_PROC(glUniform1fARB);					
				GET_GL_EXT_PROC(glUniform2fARB);					
				GET_GL_EXT_PROC(glUniform3fARB);					
				GET_GL_EXT_PROC(glUniform4fARB);					
				GET_GL_EXT_PROC(glUniform1iARB);					
				GET_GL_EXT_PROC(glUniform2iARB);					
				GET_GL_EXT_PROC(glUniform3iARB);					
				GET_GL_EXT_PROC(glUniform4iARB);					
				GET_GL_EXT_PROC(glUniform1fvARB); 				
				GET_GL_EXT_PROC(glUniform2fvARB); 				
				GET_GL_EXT_PROC(glUniform3fvARB); 				
				GET_GL_EXT_PROC(glUniform4fvARB); 				
				GET_GL_EXT_PROC(glUniform1ivARB); 				
				GET_GL_EXT_PROC(glUniform2ivARB); 				
				GET_GL_EXT_PROC(glUniform3ivARB); 				
				GET_GL_EXT_PROC(glUniform4ivARB); 				
				GET_GL_EXT_PROC(glUniformMatrix2fvARB); 		
				GET_GL_EXT_PROC(glUniformMatrix3fvARB); 		
				GET_GL_EXT_PROC(glUniformMatrix4fvARB); 		
				GET_GL_EXT_PROC(glGetObjectParameterfvARB);	
				GET_GL_EXT_PROC(glGetObjectParameterivARB);	
				GET_GL_EXT_PROC(glGetInfoLogARB);					
				GET_GL_EXT_PROC(glGetAttachedObjectsARB);		
				GET_GL_EXT_PROC(glGetUniformLocationARB);		
				GET_GL_EXT_PROC(glGetActiveUniformARB);		
				GET_GL_EXT_PROC(glGetUniformfvARB);				
				GET_GL_EXT_PROC(glGetUniformivARB);				
				GET_GL_EXT_PROC(glGetShaderSourceARB);		
			}
			if (strstr(ext, "GL_ARB_vertex_shader"))
			{
				GET_GL_EXT_PROC(glBindAttribLocationARB);
				GET_GL_EXT_PROC(glGetActiveAttribARB);
				GET_GL_EXT_PROC(glGetAttribLocationARB); 
			}
			if (strstr(ext, "GL_ARB_draw_buffers"))
			{
				GET_GL_EXT_PROC(glDrawBuffersARB);
			}
			if (strstr(ext, "GL_ARB_color_buffer_float"))
			{
				GET_GL_EXT_PROC(glClampColorARB);
			}
			if (strstr(ext, "GL_ARB_draw_instanced"))
			{
				GET_GL_EXT_PROC(glDrawArraysInstancedARB);
				GET_GL_EXT_PROC(glDrawElementsInstancedARB);
			}
			if(strstr(ext, "GL_ARB_framebuffer_object" ))
			{
				GET_GL_EXT_PROC(glIsRenderbuffer);						
				GET_GL_EXT_PROC(glBindRenderbuffer);						
				GET_GL_EXT_PROC(glDeleteRenderbuffers);					
				GET_GL_EXT_PROC(glGenRenderbuffers);						
				GET_GL_EXT_PROC(glRenderbufferStorage);					
				GET_GL_EXT_PROC(glGetRenderbufferParameteriv);			
				GET_GL_EXT_PROC(glIsFramebuffer);						
				GET_GL_EXT_PROC(glBindFramebuffer);						
				GET_GL_EXT_PROC(glDeleteFramebuffers);					
				GET_GL_EXT_PROC(glGenFramebuffers);						
				GET_GL_EXT_PROC(glCheckFramebufferStatus);				
				GET_GL_EXT_PROC(glFramebufferTexture1D);					
				GET_GL_EXT_PROC(glFramebufferTexture2D);					
				GET_GL_EXT_PROC(glFramebufferTexture3D);					
				GET_GL_EXT_PROC(glFramebufferRenderbuffer);				
				GET_GL_EXT_PROC(glGetFramebufferAttachmentParameteriv);	
				GET_GL_EXT_PROC(glGenerateMipmap);
				GET_GL_EXT_PROC(glBlitFramebuffer);
				GET_GL_EXT_PROC(glRenderbufferStorageMultisample);
				GET_GL_EXT_PROC(glFramebufferTextureLayer);
			}
			if (strstr(ext, "GL_ARB_geometry_shader4"))
			{
				GET_GL_EXT_PROC(glProgramParameteriARB);
				GET_GL_EXT_PROC(glFramebufferTextureARB);
				GET_GL_EXT_PROC(glFramebufferTextureLayerARB);
				GET_GL_EXT_PROC(glFramebufferTextureFaceARB);
			}
			if (strstr(ext, "GL_ARB_instanced_arrays"))
			{
				GET_GL_EXT_PROC(glVertexAttribDivisorARB);
			}
			if (strstr(ext, "GL_ARB_map_buffer_range"))
			{
				GET_GL_EXT_PROC(glMapBufferRange);
				GET_GL_EXT_PROC(glFlushMappedBufferRange);
			}
			if (strstr(ext, "GL_ARB_texture_buffer_object"))
			{
				GET_GL_EXT_PROC(glTexBufferARB);
			}
			if (strstr(ext, "GL_ARB_vertex_array_object"))
			{
				GET_GL_EXT_PROC(glBindVertexArray);
				GET_GL_EXT_PROC(glDeleteVertexArrays);
				GET_GL_EXT_PROC(glGenVertexArrays);
				GET_GL_EXT_PROC(glIsVertexArray);
			}
			if (strstr(ext, "GL_ARB_uniform_buffer_object"))
			{
				GET_GL_EXT_PROC(glGetUniformIndices);
				GET_GL_EXT_PROC(glGetActiveUniformsiv);
				GET_GL_EXT_PROC(glGetActiveUniformName);
				GET_GL_EXT_PROC(glGetUniformBlockIndex);
				GET_GL_EXT_PROC(glGetActiveUniformBlockiv);
				GET_GL_EXT_PROC(glGetActiveUniformBlockName);
				GET_GL_EXT_PROC(glUniformBlockBinding);
			}
			if (strstr(ext, "GL_ARB_copy_buffer"))
			{
				GET_GL_EXT_PROC(glCopyBufferSubData);
			}
			if (strstr(ext, "GL_ARB_draw_elements_base_vertex"))
			{
				GET_GL_EXT_PROC(glDrawElementsBaseVertex);
				GET_GL_EXT_PROC(glDrawRangeElementsBaseVertex);
				GET_GL_EXT_PROC(glDrawElementsInstancedBaseVertex);
				GET_GL_EXT_PROC(glMultiDrawElementsBaseVertex);
			}
			if (strstr(ext, "GL_ARB_provoking_vertex"))
			{
				GET_GL_EXT_PROC(glProvokingVertex);
			}
			if (strstr(ext, "GL_ARB_sync"))
			{
				GET_GL_EXT_PROC(glFenceSync);
				GET_GL_EXT_PROC(glIsSync);
				GET_GL_EXT_PROC(glDeleteSync);
				GET_GL_EXT_PROC(glClientWaitSync);
				GET_GL_EXT_PROC(glWaitSync);
				GET_GL_EXT_PROC(glGetInteger64v);
				GET_GL_EXT_PROC(glGetSynciv);
			}
			if (strstr(ext, "GL_ARB_texture_multisample"))
			{
				GET_GL_EXT_PROC(glTexImage2DMultisample);
				GET_GL_EXT_PROC(glTexImage3DMultisample);
				GET_GL_EXT_PROC(glGetMultisamplefv);
				GET_GL_EXT_PROC(glSampleMaski);
			}
			if (strstr(ext, "GL_ARB_draw_buffers_blend"))
			{
				GET_GL_EXT_PROC(glBlendEquationi);
				GET_GL_EXT_PROC(glBlendEquationSeparatei);
				GET_GL_EXT_PROC(glBlendFunci);
				GET_GL_EXT_PROC(glBlendFuncSeparatei);
			}
			if (strstr(ext, "GL_ARB_sample_shading"))
			{
				GET_GL_EXT_PROC(glMinSampleShading);
			}
			if (strstr(ext, "GL_EXT_blend_color"))
			{
				GET_GL_EXT_PROC(glBlendColorEXT);
			}
			if (strstr(ext, "GL_EXT_polygon_offset"))
			{
				GET_GL_EXT_PROC(glPolygonOffsetEXT);
			}
			if (strstr(ext, "GL_EXT_texture3D"))
			{
				GET_GL_EXT_PROC(glTexImage3DEXT);
				GET_GL_EXT_PROC(glTexSubImage3DEXT);
			}
			if (strstr(ext, "GL_SGIS_texture_filter4"))
			{
				GET_GL_EXT_PROC(glGetTexFilterFuncSGIS);
				GET_GL_EXT_PROC(glTexFilterFuncSGIS);
			}
			if (strstr(ext, "GL_EXT_subtexture"))
			{
				GET_GL_EXT_PROC(glTexSubImage1DEXT);
				GET_GL_EXT_PROC(glTexSubImage2DEXT);
			}
			if (strstr(ext, "GL_EXT_copy_texture"))
			{
				GET_GL_EXT_PROC(glCopyTexImage1DEXT);
				GET_GL_EXT_PROC(glCopyTexImage2DEXT);
				GET_GL_EXT_PROC(glCopyTexSubImage1DEXT);
				GET_GL_EXT_PROC(glCopyTexSubImage2DEXT);
				GET_GL_EXT_PROC(glCopyTexSubImage3DEXT);
			}
			if (strstr(ext, "GL_EXT_histogram"))
			{
				GET_GL_EXT_PROC(glGetHistogramEXT);
				GET_GL_EXT_PROC(glGetHistogramParameterfvEXT);
				GET_GL_EXT_PROC(glGetHistogramParameterivEXT);
				GET_GL_EXT_PROC(glGetMinmaxEXT);
				GET_GL_EXT_PROC(glGetMinmaxParameterfvEXT);
				GET_GL_EXT_PROC(glGetMinmaxParameterivEXT);
				GET_GL_EXT_PROC(glHistogramEXT);
				GET_GL_EXT_PROC(glMinmaxEXT);
				GET_GL_EXT_PROC(glResetHistogramEXT);
				GET_GL_EXT_PROC(glResetMinmaxEXT);
			}
			if (strstr(ext, "GL_EXT_convolution"))
			{
				GET_GL_EXT_PROC(glConvolutionFilter1DEXT);
				GET_GL_EXT_PROC(glConvolutionFilter2DEXT);
				GET_GL_EXT_PROC(glConvolutionParameterfEXT);
				GET_GL_EXT_PROC(glConvolutionParameterfvEXT);
				GET_GL_EXT_PROC(glConvolutionParameteriEXT);
				GET_GL_EXT_PROC(glConvolutionParameterivEXT);
				GET_GL_EXT_PROC(glCopyConvolutionFilter1DEXT);
				GET_GL_EXT_PROC(glCopyConvolutionFilter2DEXT);
				GET_GL_EXT_PROC(glGetConvolutionFilterEXT);
				GET_GL_EXT_PROC(glGetConvolutionParameterfvEXT);
				GET_GL_EXT_PROC(glGetConvolutionParameterivEXT);
				GET_GL_EXT_PROC(glGetSeparableFilterEXT);
				GET_GL_EXT_PROC(glSeparableFilter2DEXT);
			}
			if (strstr(ext, "GL_SGI_color_table"))
			{
				GET_GL_EXT_PROC(glColorTableSGI);
				GET_GL_EXT_PROC(glColorTableParameterfvSGI);
				GET_GL_EXT_PROC(glColorTableParameterivSGI);
				GET_GL_EXT_PROC(glCopyColorTableSGI);
				GET_GL_EXT_PROC(glGetColorTableSGI);
				GET_GL_EXT_PROC(glGetColorTableParameterfvSGI);
				GET_GL_EXT_PROC(glGetColorTableParameterivSGI);
			}
			if (strstr(ext, "GL_SGIX_pixel_texture"))
			{
				GET_GL_EXT_PROC(glPixelTexGenSGIX);
			}
			if (strstr(ext, "GL_SGIS_pixel_texture"))
			{
				GET_GL_EXT_PROC(glPixelTexGenParameteriSGIS);
				GET_GL_EXT_PROC(glPixelTexGenParameterivSGIS);
				GET_GL_EXT_PROC(glPixelTexGenParameterfSGIS);
				GET_GL_EXT_PROC(glPixelTexGenParameterfvSGIS);
				GET_GL_EXT_PROC(glGetPixelTexGenParameterivSGIS);
				GET_GL_EXT_PROC(glGetPixelTexGenParameterfvSGIS);
			}
			if (strstr(ext, "GL_SGIS_texture4D"))
			{
				GET_GL_EXT_PROC(glTexImage4DSGIS);
				GET_GL_EXT_PROC(glTexSubImage4DSGIS);
			}
			if (strstr(ext, "GL_EXT_texture_object"))
			{
				GET_GL_EXT_PROC(glAreTexturesResidentEXT);
				GET_GL_EXT_PROC(glBindTextureEXT);
				GET_GL_EXT_PROC(glDeleteTexturesEXT);
				GET_GL_EXT_PROC(glGenTexturesEXT);
				GET_GL_EXT_PROC(glIsTextureEXT);
				GET_GL_EXT_PROC(glPrioritizeTexturesEXT);
			}
			if (strstr(ext, "GL_SGIS_detail_texture"))
			{
				GET_GL_EXT_PROC(glDetailTexFuncSGIS);
				GET_GL_EXT_PROC(glGetDetailTexFuncSGIS);
			}
			if (strstr(ext, "GL_SGIS_sharpen_texture"))
			{
				GET_GL_EXT_PROC(glSharpenTexFuncSGIS);
				GET_GL_EXT_PROC(glGetSharpenTexFuncSGIS);
			}
			if (strstr(ext, "GL_SGIS_multisample"))
			{
				GET_GL_EXT_PROC(glSampleMaskSGIS);
				GET_GL_EXT_PROC(glSamplePatternSGIS);
			}
			if (strstr(ext, "GL_EXT_vertex_array"))
			{
				GET_GL_EXT_PROC(glArrayElementEXT);
				GET_GL_EXT_PROC(glColorPointerEXT);
				GET_GL_EXT_PROC(glDrawArraysEXT);
				GET_GL_EXT_PROC(glEdgeFlagPointerEXT);
				GET_GL_EXT_PROC(glGetPointervEXT);
				GET_GL_EXT_PROC(glIndexPointerEXT);
				GET_GL_EXT_PROC(glNormalPointerEXT);
				GET_GL_EXT_PROC(glTexCoordPointerEXT);
				GET_GL_EXT_PROC(glVertexPointerEXT);
			}
			if (strstr(ext, "GL_EXT_blend_minmax"))
			{
				GET_GL_EXT_PROC(glBlendEquationEXT);
			}
			if (strstr(ext, "GL_SGIX_sprite"))
			{
				GET_GL_EXT_PROC(glSpriteParameterfSGIX);
				GET_GL_EXT_PROC(glSpriteParameterfvSGIX);
				GET_GL_EXT_PROC(glSpriteParameteriSGIX);
				GET_GL_EXT_PROC(glSpriteParameterivSGIX);
			}
			if (strstr(ext, "GL_EXT_point_parameters"))
			{
				GET_GL_EXT_PROC(glPointParameterfEXT);
				GET_GL_EXT_PROC(glPointParameterfvEXT);
			}
			if (strstr(ext, "GL_SGIS_point_parameters"))
			{
				GET_GL_EXT_PROC(glPointParameterfSGIS);
				GET_GL_EXT_PROC(glPointParameterfvSGIS);
			}
			if (strstr(ext, "GL_SGIX_instruments"))
			{
				GET_GL_EXT_PROC(glGetInstrumentsSGIX);
				GET_GL_EXT_PROC(glInstrumentsBufferSGIX);
				GET_GL_EXT_PROC(glPollInstrumentsSGIX);
				GET_GL_EXT_PROC(glReadInstrumentsSGIX);
				GET_GL_EXT_PROC(glStartInstrumentsSGIX);
				GET_GL_EXT_PROC(glStopInstrumentsSGIX);
			}
			if (strstr(ext, "GL_SGIX_framezoom"))
			{
				GET_GL_EXT_PROC(glFrameZoomSGIX);
			}
			if (strstr(ext, "GL_SGIX_tag_sample_buffer"))
			{
				GET_GL_EXT_PROC(glTagSampleBufferSGIX);
			}
			if (strstr(ext, "GL_SGIX_polynomial_ffd"))
			{
				GET_GL_EXT_PROC(glDeformationMap3dSGIX);
				GET_GL_EXT_PROC(glDeformationMap3fSGIX);
				GET_GL_EXT_PROC(glDeformSGIX);
				GET_GL_EXT_PROC(glLoadIdentityDeformationMapSGIX);
			}
			if (strstr(ext, "GL_SGIX_reference_plane"))
			{
				GET_GL_EXT_PROC(glReferencePlaneSGIX);
			}
			if (strstr(ext, "GL_SGIX_flush_raster"))
			{
				GET_GL_EXT_PROC(glFlushRasterSGIX);
			}
			if (strstr(ext, "GL_SGIS_fog_function"))
			{
				GET_GL_EXT_PROC(glFogFuncSGIS);
				GET_GL_EXT_PROC(glGetFogFuncSGIS);
			}
			if (strstr(ext, "GL_HP_image_transform"))
			{
				GET_GL_EXT_PROC(glImageTransformParameteriHP);
				GET_GL_EXT_PROC(glImageTransformParameterfHP);
				GET_GL_EXT_PROC(glImageTransformParameterivHP);
				GET_GL_EXT_PROC(glImageTransformParameterfvHP);
				GET_GL_EXT_PROC(glGetImageTransformParameterivHP);
				GET_GL_EXT_PROC(glGetImageTransformParameterfvHP);
			}
			if (strstr(ext, "GL_EXT_color_subtable"))
			{
				GET_GL_EXT_PROC(glColorSubTableEXT);
				GET_GL_EXT_PROC(glCopyColorSubTableEXT);
			}
			if (strstr(ext, "GL_PGI_misc_hints"))
			{
				GET_GL_EXT_PROC(glHintPGI);
			}
			if (strstr(ext, "GL_EXT_paletted_texture"))
			{
				GET_GL_EXT_PROC(glColorTableEXT);
				GET_GL_EXT_PROC(glGetColorTableEXT);
				GET_GL_EXT_PROC(glGetColorTableParameterivEXT);
				GET_GL_EXT_PROC(glGetColorTableParameterfvEXT);
			}
			if (strstr(ext, "GL_SGIX_list_priority"))
			{
				GET_GL_EXT_PROC(glGetListParameterfvSGIX);
				GET_GL_EXT_PROC(glGetListParameterivSGIX);
				GET_GL_EXT_PROC(glListParameterfSGIX);
				GET_GL_EXT_PROC(glListParameterfvSGIX);
				GET_GL_EXT_PROC(glListParameteriSGIX);
				GET_GL_EXT_PROC(glListParameterivSGIX);
			}
			if (strstr(ext, "GL_EXT_index_material"))
			{
				GET_GL_EXT_PROC(glIndexMaterialEXT);
			}
			if (strstr(ext, "GL_EXT_index_func"))
			{
				GET_GL_EXT_PROC(glIndexFuncEXT);
			}
			if (strstr(ext, "GL_EXT_compiled_vertex_array"))
			{
				GET_GL_EXT_PROC(glLockArraysEXT);
				GET_GL_EXT_PROC(glUnlockArraysEXT);
			}
			if (strstr(ext, "GL_EXT_cull_vertex")) 
			{
				GET_GL_EXT_PROC(glCullParameterdvEXT);
				GET_GL_EXT_PROC(glCullParameterfvEXT);
			}
			if (strstr(ext, "GL_SGIX_fragment_lighting")) 
			{
				GET_GL_EXT_PROC(glFragmentColorMaterialSGIX);
				GET_GL_EXT_PROC(glFragmentLightfSGIX);
				GET_GL_EXT_PROC(glFragmentLightfvSGIX);
				GET_GL_EXT_PROC(glFragmentLightiSGIX);
				GET_GL_EXT_PROC(glFragmentLightivSGIX);
				GET_GL_EXT_PROC(glFragmentLightModelfSGIX);
				GET_GL_EXT_PROC(glFragmentLightModelfvSGIX);
				GET_GL_EXT_PROC(glFragmentLightModeliSGIX);
				GET_GL_EXT_PROC(glFragmentLightModelivSGIX);
				GET_GL_EXT_PROC(glFragmentMaterialfSGIX);
				GET_GL_EXT_PROC(glFragmentMaterialfvSGIX);
				GET_GL_EXT_PROC(glFragmentMaterialiSGIX);
				GET_GL_EXT_PROC(glFragmentMaterialivSGIX);
				GET_GL_EXT_PROC(glGetFragmentLightfvSGIX);
				GET_GL_EXT_PROC(glGetFragmentLightivSGIX);
				GET_GL_EXT_PROC(glGetFragmentMaterialfvSGIX);
				GET_GL_EXT_PROC(glGetFragmentMaterialivSGIX);
				GET_GL_EXT_PROC(glLightEnviSGIX);
			}
			if (strstr(ext, "GL_EXT_draw_range_elements")) 
			{
				GET_GL_EXT_PROC(glDrawRangeElementsEXT);
			}
			if (strstr(ext, "GL_EXT_light_texture")) 
			{
				GET_GL_EXT_PROC(glApplyTextureEXT);
				GET_GL_EXT_PROC(glTextureLightEXT);
				GET_GL_EXT_PROC(glTextureMaterialEXT);
			}
			if (strstr(ext, "GL_SGIX_async")) 
			{
				GET_GL_EXT_PROC(glAsyncMarkerSGIX);
				GET_GL_EXT_PROC(glFinishAsyncSGIX);
				GET_GL_EXT_PROC(glPollAsyncSGIX);
				GET_GL_EXT_PROC(glGenAsyncMarkersSGIX);
				GET_GL_EXT_PROC(glDeleteAsyncMarkersSGIX);
				GET_GL_EXT_PROC(glIsAsyncMarkerSGIX);
			}
			if (strstr(ext, "GL_INTEL_parallel_arrays")) 
			{
				GET_GL_EXT_PROC(glVertexPointervINTEL);
				GET_GL_EXT_PROC(glNormalPointervINTEL);
				GET_GL_EXT_PROC(glColorPointervINTEL);
				GET_GL_EXT_PROC(glTexCoordPointervINTEL);
			}
			if (strstr(ext, "GL_EXT_pixel_transform")) 
			{
				GET_GL_EXT_PROC(glPixelTransformParameteriEXT);
				GET_GL_EXT_PROC(glPixelTransformParameterfEXT);
				GET_GL_EXT_PROC(glPixelTransformParameterivEXT);
				GET_GL_EXT_PROC(glPixelTransformParameterfvEXT);
			}
			if (strstr(ext, "GL_EXT_secondary_color")) 
			{
				GET_GL_EXT_PROC(glSecondaryColor3bEXT);
				GET_GL_EXT_PROC(glSecondaryColor3bvEXT);
				GET_GL_EXT_PROC(glSecondaryColor3dEXT);
				GET_GL_EXT_PROC(glSecondaryColor3dvEXT);
				GET_GL_EXT_PROC(glSecondaryColor3fEXT);
				GET_GL_EXT_PROC(glSecondaryColor3fvEXT);
				GET_GL_EXT_PROC(glSecondaryColor3iEXT);
				GET_GL_EXT_PROC(glSecondaryColor3ivEXT);
				GET_GL_EXT_PROC(glSecondaryColor3sEXT);
				GET_GL_EXT_PROC(glSecondaryColor3svEXT);
				GET_GL_EXT_PROC(glSecondaryColor3ubEXT);
				GET_GL_EXT_PROC(glSecondaryColor3ubvEXT);
				GET_GL_EXT_PROC(glSecondaryColor3uiEXT);
				GET_GL_EXT_PROC(glSecondaryColor3uivEXT);
				GET_GL_EXT_PROC(glSecondaryColor3usEXT);
				GET_GL_EXT_PROC(glSecondaryColor3usvEXT);
				GET_GL_EXT_PROC(glSecondaryColorPointerEXT);
			}
			if (strstr(ext, "GL_EXT_texture_perturb_normal")) 
			{
				GET_GL_EXT_PROC(glTextureNormalEXT);
			}
			if (strstr(ext, "GL_EXT_multi_draw_arrays")) 
			{
				GET_GL_EXT_PROC(glMultiDrawArraysEXT);
				GET_GL_EXT_PROC(glMultiDrawElementsEXT);
			}
			if (strstr(ext, "GL_EXT_fog_coord")) 
			{
				GET_GL_EXT_PROC(glFogCoordfEXT);
				GET_GL_EXT_PROC(glFogCoordfvEXT);
				GET_GL_EXT_PROC(glFogCoorddEXT);
				GET_GL_EXT_PROC(glFogCoorddvEXT);
				GET_GL_EXT_PROC(glFogCoordPointerEXT);
			}
			if (strstr(ext, "GL_EXT_coordinate_frame")) 
			{
				GET_GL_EXT_PROC(glTangent3bEXT);
				GET_GL_EXT_PROC(glTangent3bvEXT);
				GET_GL_EXT_PROC(glTangent3dEXT);
				GET_GL_EXT_PROC(glTangent3dvEXT);
				GET_GL_EXT_PROC(glTangent3fEXT);
				GET_GL_EXT_PROC(glTangent3fvEXT);
				GET_GL_EXT_PROC(glTangent3iEXT);
				GET_GL_EXT_PROC(glTangent3ivEXT);
				GET_GL_EXT_PROC(glTangent3sEXT);
				GET_GL_EXT_PROC(glTangent3svEXT);
				GET_GL_EXT_PROC(glBinormal3bEXT);
				GET_GL_EXT_PROC(glBinormal3bvEXT);
				GET_GL_EXT_PROC(glBinormal3dEXT);
				GET_GL_EXT_PROC(glBinormal3dvEXT);
				GET_GL_EXT_PROC(glBinormal3fEXT);
				GET_GL_EXT_PROC(glBinormal3fvEXT);
				GET_GL_EXT_PROC(glBinormal3iEXT);
				GET_GL_EXT_PROC(glBinormal3ivEXT);
				GET_GL_EXT_PROC(glBinormal3sEXT);
				GET_GL_EXT_PROC(glBinormal3svEXT);
				GET_GL_EXT_PROC(glTangentPointerEXT);
				GET_GL_EXT_PROC(glBinormalPointerEXT);
			}
			if (strstr(ext, "GL_SUNX_constant_data")) 
			{
				GET_GL_EXT_PROC(glFinishTextureSUNX);
			}
			if (strstr(ext, "GL_SUN_global_alpha")) 
			{
				GET_GL_EXT_PROC(glGlobalAlphaFactorbSUN);
				GET_GL_EXT_PROC(glGlobalAlphaFactorsSUN);
				GET_GL_EXT_PROC(glGlobalAlphaFactoriSUN);
				GET_GL_EXT_PROC(glGlobalAlphaFactorfSUN);
				GET_GL_EXT_PROC(glGlobalAlphaFactordSUN);
				GET_GL_EXT_PROC(glGlobalAlphaFactorubSUN);
				GET_GL_EXT_PROC(glGlobalAlphaFactorusSUN);
				GET_GL_EXT_PROC(glGlobalAlphaFactoruiSUN);
			}
			if (strstr(ext, "GL_SUN_triangle_list")) 
			{
				GET_GL_EXT_PROC(glReplacementCodeuiSUN);
				GET_GL_EXT_PROC(glReplacementCodeusSUN);
				GET_GL_EXT_PROC(glReplacementCodeubSUN);
				GET_GL_EXT_PROC(glReplacementCodeuivSUN);
				GET_GL_EXT_PROC(glReplacementCodeusvSUN);
				GET_GL_EXT_PROC(glReplacementCodeubvSUN);
				GET_GL_EXT_PROC(glReplacementCodePointerSUN);
			}
			if (strstr(ext, "GL_SUN_vertex")) 
			{
				GET_GL_EXT_PROC(glColor4ubVertex2fSUN);
				GET_GL_EXT_PROC(glColor4ubVertex2fvSUN);
				GET_GL_EXT_PROC(glColor4ubVertex3fSUN);
				GET_GL_EXT_PROC(glColor4ubVertex3fvSUN);
				GET_GL_EXT_PROC(glColor3fVertex3fSUN);
				GET_GL_EXT_PROC(glColor3fVertex3fvSUN);
				GET_GL_EXT_PROC(glNormal3fVertex3fSUN);
				GET_GL_EXT_PROC(glNormal3fVertex3fvSUN);
				GET_GL_EXT_PROC(glColor4fNormal3fVertex3fSUN);
				GET_GL_EXT_PROC(glColor4fNormal3fVertex3fvSUN);
				GET_GL_EXT_PROC(glTexCoord2fVertex3fSUN);
				GET_GL_EXT_PROC(glTexCoord2fVertex3fvSUN);
				GET_GL_EXT_PROC(glTexCoord4fVertex4fSUN);
				GET_GL_EXT_PROC(glTexCoord4fVertex4fvSUN);
				GET_GL_EXT_PROC(glTexCoord2fColor4ubVertex3fSUN);
				GET_GL_EXT_PROC(glTexCoord2fColor4ubVertex3fvSUN);
				GET_GL_EXT_PROC(glTexCoord2fColor3fVertex3fSUN);
				GET_GL_EXT_PROC(glTexCoord2fColor3fVertex3fvSUN);
				GET_GL_EXT_PROC(glTexCoord2fNormal3fVertex3fSUN);
				GET_GL_EXT_PROC(glTexCoord2fNormal3fVertex3fvSUN);
				GET_GL_EXT_PROC(glTexCoord2fColor4fNormal3fVertex3fSUN);
				GET_GL_EXT_PROC(glTexCoord2fColor4fNormal3fVertex3fvSUN);
				GET_GL_EXT_PROC(glTexCoord4fColor4fNormal3fVertex4fSUN);
				GET_GL_EXT_PROC(glTexCoord4fColor4fNormal3fVertex4fvSUN);
				GET_GL_EXT_PROC(glReplacementCodeuiVertex3fSUN);
				GET_GL_EXT_PROC(glReplacementCodeuiVertex3fvSUN);
				GET_GL_EXT_PROC(glReplacementCodeuiColor4ubVertex3fSUN);
				GET_GL_EXT_PROC(glReplacementCodeuiColor4ubVertex3fvSUN);
				GET_GL_EXT_PROC(glReplacementCodeuiColor3fVertex3fSUN);
				GET_GL_EXT_PROC(glReplacementCodeuiColor3fVertex3fvSUN);
				GET_GL_EXT_PROC(glReplacementCodeuiNormal3fVertex3fSUN);
				GET_GL_EXT_PROC(glReplacementCodeuiNormal3fVertex3fvSUN);
				GET_GL_EXT_PROC(glReplacementCodeuiColor4fNormal3fVertex3fSUN);
				GET_GL_EXT_PROC(glReplacementCodeuiColor4fNormal3fVertex3fvSUN);
				GET_GL_EXT_PROC(glReplacementCodeuiTexCoord2fVertex3fSUN);
				GET_GL_EXT_PROC(glReplacementCodeuiTexCoord2fVertex3fvSUN);
				GET_GL_EXT_PROC(glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN);
				GET_GL_EXT_PROC(glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN);
				GET_GL_EXT_PROC(glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN);
				GET_GL_EXT_PROC(glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN);
			}
			if (strstr(ext, "GL_EXT_blend_func_separate")) 
			{
				GET_GL_EXT_PROC(glBlendFuncSeparateEXT);
			}
			if (strstr(ext, "GL_INGR_blend_func_separate")) 
			{
				GET_GL_EXT_PROC(glBlendFuncSeparateINGR);
			}
			if (strstr(ext, "GL_EXT_vertex_weighting")) 
			{
				GET_GL_EXT_PROC(glVertexWeightfEXT);
				GET_GL_EXT_PROC(glVertexWeightfvEXT);
				GET_GL_EXT_PROC(glVertexWeightPointerEXT);
			}
			if (strstr(ext, "GL_NV_vertex_array_range")) 
			{
				GET_GL_EXT_PROC(glFlushVertexArrayRangeNV);
				GET_GL_EXT_PROC(glVertexArrayRangeNV);
			}
			if (strstr(ext, "GL_NV_register_combiners")) 
			{
				GET_GL_EXT_PROC(glCombinerParameterfvNV);
				GET_GL_EXT_PROC(glCombinerParameterfNV);
				GET_GL_EXT_PROC(glCombinerParameterivNV);
				GET_GL_EXT_PROC(glCombinerParameteriNV);
				GET_GL_EXT_PROC(glCombinerInputNV);
				GET_GL_EXT_PROC(glCombinerOutputNV);
				GET_GL_EXT_PROC(glFinalCombinerInputNV);
				GET_GL_EXT_PROC(glGetCombinerInputParameterfvNV);
				GET_GL_EXT_PROC(glGetCombinerInputParameterivNV);
				GET_GL_EXT_PROC(glGetCombinerOutputParameterfvNV);
				GET_GL_EXT_PROC(glGetCombinerOutputParameterivNV);
				GET_GL_EXT_PROC(glGetFinalCombinerInputParameterfvNV);
				GET_GL_EXT_PROC(glGetFinalCombinerInputParameterivNV);
			}
			if (strstr(ext, "GL_MESA_resize_buffers")) 
			{
				GET_GL_EXT_PROC(glResizeBuffersMESA);
			}
			if (strstr(ext, "GL_MESA_window_pos")) 
			{
				GET_GL_EXT_PROC(glWindowPos2dMESA);
				GET_GL_EXT_PROC(glWindowPos2dvMESA);
				GET_GL_EXT_PROC(glWindowPos2fMESA);
				GET_GL_EXT_PROC(glWindowPos2fvMESA);
				GET_GL_EXT_PROC(glWindowPos2iMESA);
				GET_GL_EXT_PROC(glWindowPos2ivMESA);
				GET_GL_EXT_PROC(glWindowPos2sMESA);
				GET_GL_EXT_PROC(glWindowPos2svMESA);
				GET_GL_EXT_PROC(glWindowPos3dMESA);
				GET_GL_EXT_PROC(glWindowPos3dvMESA);
				GET_GL_EXT_PROC(glWindowPos3fMESA);
				GET_GL_EXT_PROC(glWindowPos3fvMESA);
				GET_GL_EXT_PROC(glWindowPos3iMESA);
				GET_GL_EXT_PROC(glWindowPos3ivMESA);
				GET_GL_EXT_PROC(glWindowPos3sMESA);
				GET_GL_EXT_PROC(glWindowPos3svMESA);
				GET_GL_EXT_PROC(glWindowPos4dMESA);
				GET_GL_EXT_PROC(glWindowPos4dvMESA);
				GET_GL_EXT_PROC(glWindowPos4fMESA);
				GET_GL_EXT_PROC(glWindowPos4fvMESA);
				GET_GL_EXT_PROC(glWindowPos4iMESA);
				GET_GL_EXT_PROC(glWindowPos4ivMESA);
				GET_GL_EXT_PROC(glWindowPos4sMESA);
				GET_GL_EXT_PROC(glWindowPos4svMESA);
			}
			if (strstr(ext, "GL_IBM_multimode_draw_arrays")) 
			{
				GET_GL_EXT_PROC(glMultiModeDrawArraysIBM);
				GET_GL_EXT_PROC(glMultiModeDrawElementsIBM);
			}
			if (strstr(ext, "GL_IBM_vertex_array_lists")) 
			{
				GET_GL_EXT_PROC(glColorPointerListIBM);
				GET_GL_EXT_PROC(glSecondaryColorPointerListIBM);
				GET_GL_EXT_PROC(glEdgeFlagPointerListIBM);
				GET_GL_EXT_PROC(glFogCoordPointerListIBM);
				GET_GL_EXT_PROC(glIndexPointerListIBM);
				GET_GL_EXT_PROC(glNormalPointerListIBM);
				GET_GL_EXT_PROC(glTexCoordPointerListIBM);
				GET_GL_EXT_PROC(glVertexPointerListIBM);
			}
			if (strstr(ext, "GL_3DFX_tbuffer")) 
			{
				GET_GL_EXT_PROC(glTbufferMask3DFX);
			}
			if (strstr(ext, "GL_EXT_multisample")) 
			{
				GET_GL_EXT_PROC(glSampleMaskEXT);
				GET_GL_EXT_PROC(glSamplePatternEXT);
			}
			if (strstr(ext, "GL_SGIS_texture_color_mask")) 
			{
				GET_GL_EXT_PROC(glTextureColorMaskSGIS);
			}
			if (strstr(ext, "GL_SGIX_igloo_interface")) 
			{
				GET_GL_EXT_PROC(glIglooInterfaceSGIX);
			}
			if (strstr(ext, "GL_NV_fence")) 
			{
				GET_GL_EXT_PROC(glDeleteFencesNV);
				GET_GL_EXT_PROC(glGenFencesNV);
				GET_GL_EXT_PROC(glIsFenceNV);
				GET_GL_EXT_PROC(glTestFenceNV);
				GET_GL_EXT_PROC(glGetFenceivNV);
				GET_GL_EXT_PROC(glFinishFenceNV);
				GET_GL_EXT_PROC(glSetFenceNV);
			}
			if (strstr(ext, "GL_NV_evaluators")) 
			{
				GET_GL_EXT_PROC(glMapControlPointsNV);
				GET_GL_EXT_PROC(glMapParameterivNV);
				GET_GL_EXT_PROC(glMapParameterfvNV);
				GET_GL_EXT_PROC(glGetMapControlPointsNV);
				GET_GL_EXT_PROC(glGetMapParameterivNV);
				GET_GL_EXT_PROC(glGetMapParameterfvNV);
				GET_GL_EXT_PROC(glGetMapAttribParameterivNV);
				GET_GL_EXT_PROC(glGetMapAttribParameterfvNV);
				GET_GL_EXT_PROC(glEvalMapsNV);
			}
			if (strstr(ext, "GL_NV_register_combiners2")) 
			{
				GET_GL_EXT_PROC(glCombinerStageParameterfvNV);
				GET_GL_EXT_PROC(glGetCombinerStageParameterfvNV);
			}
			if (strstr(ext, "GL_NV_vertex_program")) 
			{
				GET_GL_EXT_PROC(glAreProgramsResidentNV);
				GET_GL_EXT_PROC(glBindProgramNV);
				GET_GL_EXT_PROC(glDeleteProgramsNV);
				GET_GL_EXT_PROC(glExecuteProgramNV);
				GET_GL_EXT_PROC(glGenProgramsNV);
				GET_GL_EXT_PROC(glGetProgramParameterdvNV);
				GET_GL_EXT_PROC(glGetProgramParameterfvNV);
				GET_GL_EXT_PROC(glGetProgramivNV);
				GET_GL_EXT_PROC(glGetProgramStringNV);
				GET_GL_EXT_PROC(glGetTrackMatrixivNV);
				GET_GL_EXT_PROC(glGetVertexAttribdvNV);
				GET_GL_EXT_PROC(glGetVertexAttribfvNV);
				GET_GL_EXT_PROC(glGetVertexAttribivNV);
				GET_GL_EXT_PROC(glGetVertexAttribPointervNV);
				GET_GL_EXT_PROC(glIsProgramNV);
				GET_GL_EXT_PROC(glLoadProgramNV);
				GET_GL_EXT_PROC(glProgramParameter4dNV);
				GET_GL_EXT_PROC(glProgramParameter4dvNV);
				GET_GL_EXT_PROC(glProgramParameter4fNV);
				GET_GL_EXT_PROC(glProgramParameter4fvNV);
				GET_GL_EXT_PROC(glProgramParameters4dvNV);
				GET_GL_EXT_PROC(glProgramParameters4fvNV);
				GET_GL_EXT_PROC(glRequestResidentProgramsNV);
				GET_GL_EXT_PROC(glTrackMatrixNV);
				GET_GL_EXT_PROC(glVertexAttribPointerNV);
				GET_GL_EXT_PROC(glVertexAttrib1dNV);
				GET_GL_EXT_PROC(glVertexAttrib1dvNV);
				GET_GL_EXT_PROC(glVertexAttrib1fNV);
				GET_GL_EXT_PROC(glVertexAttrib1fvNV);
				GET_GL_EXT_PROC(glVertexAttrib1sNV);
				GET_GL_EXT_PROC(glVertexAttrib1svNV);
				GET_GL_EXT_PROC(glVertexAttrib2dNV);
				GET_GL_EXT_PROC(glVertexAttrib2dvNV);
				GET_GL_EXT_PROC(glVertexAttrib2fNV);
				GET_GL_EXT_PROC(glVertexAttrib2fvNV);
				GET_GL_EXT_PROC(glVertexAttrib2sNV);
				GET_GL_EXT_PROC(glVertexAttrib2svNV);
				GET_GL_EXT_PROC(glVertexAttrib3dNV);
				GET_GL_EXT_PROC(glVertexAttrib3dvNV);
				GET_GL_EXT_PROC(glVertexAttrib3fNV);
				GET_GL_EXT_PROC(glVertexAttrib3fvNV);
				GET_GL_EXT_PROC(glVertexAttrib3sNV);
				GET_GL_EXT_PROC(glVertexAttrib3svNV);
				GET_GL_EXT_PROC(glVertexAttrib4dNV);
				GET_GL_EXT_PROC(glVertexAttrib4dvNV);
				GET_GL_EXT_PROC(glVertexAttrib4fNV);
				GET_GL_EXT_PROC(glVertexAttrib4fvNV);
				GET_GL_EXT_PROC(glVertexAttrib4sNV);
				GET_GL_EXT_PROC(glVertexAttrib4svNV);
				GET_GL_EXT_PROC(glVertexAttrib4ubNV);
				GET_GL_EXT_PROC(glVertexAttrib4ubvNV);
				GET_GL_EXT_PROC(glVertexAttribs1dvNV);
				GET_GL_EXT_PROC(glVertexAttribs1fvNV);
				GET_GL_EXT_PROC(glVertexAttribs1svNV);
				GET_GL_EXT_PROC(glVertexAttribs2dvNV);
				GET_GL_EXT_PROC(glVertexAttribs2fvNV);
				GET_GL_EXT_PROC(glVertexAttribs2svNV);
				GET_GL_EXT_PROC(glVertexAttribs3dvNV);
				GET_GL_EXT_PROC(glVertexAttribs3fvNV);
				GET_GL_EXT_PROC(glVertexAttribs3svNV);
				GET_GL_EXT_PROC(glVertexAttribs4dvNV);
				GET_GL_EXT_PROC(glVertexAttribs4fvNV);
				GET_GL_EXT_PROC(glVertexAttribs4svNV);
				GET_GL_EXT_PROC(glVertexAttribs4ubvNV);
			}
			if (strstr(ext, "GL_ATI_envmap_bumpmap")) 
			{
				GET_GL_EXT_PROC(glTexBumpParameterivATI);
				GET_GL_EXT_PROC(glTexBumpParameterfvATI);
				GET_GL_EXT_PROC(glGetTexBumpParameterivATI);
				GET_GL_EXT_PROC(glGetTexBumpParameterfvATI);
			}
			if (strstr(ext, "GL_ATI_fragment_shader")) 
			{
				GET_GL_EXT_PROC(glGenFragmentShadersATI);
				GET_GL_EXT_PROC(glBindFragmentShaderATI);
				GET_GL_EXT_PROC(glDeleteFragmentShaderATI);
				GET_GL_EXT_PROC(glBeginFragmentShaderATI);
				GET_GL_EXT_PROC(glEndFragmentShaderATI);
				GET_GL_EXT_PROC(glPassTexCoordATI);
				GET_GL_EXT_PROC(glSampleMapATI);
				GET_GL_EXT_PROC(glColorFragmentOp1ATI);
				GET_GL_EXT_PROC(glColorFragmentOp2ATI);
				GET_GL_EXT_PROC(glColorFragmentOp3ATI);
				GET_GL_EXT_PROC(glAlphaFragmentOp1ATI);
				GET_GL_EXT_PROC(glAlphaFragmentOp2ATI);
				GET_GL_EXT_PROC(glAlphaFragmentOp3ATI);
				GET_GL_EXT_PROC(glSetFragmentShaderConstantATI);
			}
			if (strstr(ext, "GL_ATI_pn_triangles")) 
			{
				GET_GL_EXT_PROC(glPNTrianglesiATI);
				GET_GL_EXT_PROC(glPNTrianglesfATI);
			}
			if (strstr(ext, "GL_ATI_vertex_array_object")) 
			{
				GET_GL_EXT_PROC(glNewObjectBufferATI);
				GET_GL_EXT_PROC(glIsObjectBufferATI);
				GET_GL_EXT_PROC(glUpdateObjectBufferATI);
				GET_GL_EXT_PROC(glGetObjectBufferfvATI);
				GET_GL_EXT_PROC(glGetObjectBufferivATI);
				GET_GL_EXT_PROC(glFreeObjectBufferATI);
				GET_GL_EXT_PROC(glArrayObjectATI);
				GET_GL_EXT_PROC(glGetArrayObjectfvATI);
				GET_GL_EXT_PROC(glGetArrayObjectivATI);
				GET_GL_EXT_PROC(glVariantArrayObjectATI);
				GET_GL_EXT_PROC(glGetVariantArrayObjectfvATI);
				GET_GL_EXT_PROC(glGetVariantArrayObjectivATI);
			}
			if (strstr(ext, "GL_EXT_vertex_shader")) 
			{
				GET_GL_EXT_PROC(glBeginVertexShaderEXT);
				GET_GL_EXT_PROC(glEndVertexShaderEXT);
				GET_GL_EXT_PROC(glBindVertexShaderEXT);
				GET_GL_EXT_PROC(glGenVertexShadersEXT);
				GET_GL_EXT_PROC(glDeleteVertexShaderEXT);
				GET_GL_EXT_PROC(glShaderOp1EXT);
				GET_GL_EXT_PROC(glShaderOp2EXT);
				GET_GL_EXT_PROC(glShaderOp3EXT);
				GET_GL_EXT_PROC(glSwizzleEXT);
				GET_GL_EXT_PROC(glWriteMaskEXT);
				GET_GL_EXT_PROC(glInsertComponentEXT);
				GET_GL_EXT_PROC(glExtractComponentEXT);
				GET_GL_EXT_PROC(glGenSymbolsEXT);
				GET_GL_EXT_PROC(glSetInvariantEXT);
				GET_GL_EXT_PROC(glSetLocalConstantEXT);
				GET_GL_EXT_PROC(glVariantbvEXT);
				GET_GL_EXT_PROC(glVariantsvEXT);
				GET_GL_EXT_PROC(glVariantivEXT);
				GET_GL_EXT_PROC(glVariantfvEXT);
				GET_GL_EXT_PROC(glVariantdvEXT);
				GET_GL_EXT_PROC(glVariantubvEXT);
				GET_GL_EXT_PROC(glVariantusvEXT);
				GET_GL_EXT_PROC(glVariantuivEXT);
				GET_GL_EXT_PROC(glVariantPointerEXT);
				GET_GL_EXT_PROC(glEnableVariantClientStateEXT);
				GET_GL_EXT_PROC(glDisableVariantClientStateEXT);
				GET_GL_EXT_PROC(glBindLightParameterEXT);
				GET_GL_EXT_PROC(glBindMaterialParameterEXT);
				GET_GL_EXT_PROC(glBindTexGenParameterEXT);
				GET_GL_EXT_PROC(glBindTextureUnitParameterEXT);
				GET_GL_EXT_PROC(glBindParameterEXT);
				GET_GL_EXT_PROC(glIsVariantEnabledEXT);
				GET_GL_EXT_PROC(glGetVariantBooleanvEXT);
				GET_GL_EXT_PROC(glGetVariantIntegervEXT);
				GET_GL_EXT_PROC(glGetVariantFloatvEXT);
				GET_GL_EXT_PROC(glGetVariantPointervEXT);
				GET_GL_EXT_PROC(glGetInvariantBooleanvEXT);
				GET_GL_EXT_PROC(glGetInvariantIntegervEXT);
				GET_GL_EXT_PROC(glGetInvariantFloatvEXT);
				GET_GL_EXT_PROC(glGetLocalConstantBooleanvEXT);
				GET_GL_EXT_PROC(glGetLocalConstantIntegervEXT);
				GET_GL_EXT_PROC(glGetLocalConstantFloatvEXT);
			}
			if (strstr(ext, "GL_ATI_vertex_streams")) 
			{
				GET_GL_EXT_PROC(glVertexStream1sATI);
				GET_GL_EXT_PROC(glVertexStream1svATI);
				GET_GL_EXT_PROC(glVertexStream1iATI);
				GET_GL_EXT_PROC(glVertexStream1ivATI);
				GET_GL_EXT_PROC(glVertexStream1fATI);
				GET_GL_EXT_PROC(glVertexStream1fvATI);
				GET_GL_EXT_PROC(glVertexStream1dATI);
				GET_GL_EXT_PROC(glVertexStream1dvATI);
				GET_GL_EXT_PROC(glVertexStream2sATI);
				GET_GL_EXT_PROC(glVertexStream2svATI);
				GET_GL_EXT_PROC(glVertexStream2iATI);
				GET_GL_EXT_PROC(glVertexStream2ivATI);
				GET_GL_EXT_PROC(glVertexStream2fATI);
				GET_GL_EXT_PROC(glVertexStream2fvATI);
				GET_GL_EXT_PROC(glVertexStream2dATI);
				GET_GL_EXT_PROC(glVertexStream2dvATI);
				GET_GL_EXT_PROC(glVertexStream3sATI);
				GET_GL_EXT_PROC(glVertexStream3svATI);
				GET_GL_EXT_PROC(glVertexStream3iATI);
				GET_GL_EXT_PROC(glVertexStream3ivATI);
				GET_GL_EXT_PROC(glVertexStream3fATI);
				GET_GL_EXT_PROC(glVertexStream3fvATI);
				GET_GL_EXT_PROC(glVertexStream3dATI);
				GET_GL_EXT_PROC(glVertexStream3dvATI);
				GET_GL_EXT_PROC(glVertexStream4sATI);
				GET_GL_EXT_PROC(glVertexStream4svATI);
				GET_GL_EXT_PROC(glVertexStream4iATI);
				GET_GL_EXT_PROC(glVertexStream4ivATI);
				GET_GL_EXT_PROC(glVertexStream4fATI);
				GET_GL_EXT_PROC(glVertexStream4fvATI);
				GET_GL_EXT_PROC(glVertexStream4dATI);
				GET_GL_EXT_PROC(glVertexStream4dvATI);
				GET_GL_EXT_PROC(glNormalStream3bATI);
				GET_GL_EXT_PROC(glNormalStream3bvATI);
				GET_GL_EXT_PROC(glNormalStream3sATI);
				GET_GL_EXT_PROC(glNormalStream3svATI);
				GET_GL_EXT_PROC(glNormalStream3iATI);
				GET_GL_EXT_PROC(glNormalStream3ivATI);
				GET_GL_EXT_PROC(glNormalStream3fATI);
				GET_GL_EXT_PROC(glNormalStream3fvATI);
				GET_GL_EXT_PROC(glNormalStream3dATI);
				GET_GL_EXT_PROC(glNormalStream3dvATI);
				GET_GL_EXT_PROC(glClientActiveVertexStreamATI);
				GET_GL_EXT_PROC(glVertexBlendEnviATI);
				GET_GL_EXT_PROC(glVertexBlendEnvfATI);
			}
			if (strstr(ext, "GL_ATI_element_array")) 
			{
				GET_GL_EXT_PROC(glElementPointerATI);
				GET_GL_EXT_PROC(glDrawElementArrayATI);
				GET_GL_EXT_PROC(glDrawRangeElementArrayATI);
			}
			if (strstr(ext, "GL_SUN_mesh_array")) 
			{
				GET_GL_EXT_PROC(glDrawMeshArraysSUN);
			}
			if (strstr(ext, "GL_NV_occlusion_query")) 
			{
				GET_GL_EXT_PROC(glGenOcclusionQueriesNV);
				GET_GL_EXT_PROC(glDeleteOcclusionQueriesNV);
				GET_GL_EXT_PROC(glIsOcclusionQueryNV);
				GET_GL_EXT_PROC(glBeginOcclusionQueryNV);
				GET_GL_EXT_PROC(glEndOcclusionQueryNV);
				GET_GL_EXT_PROC(glGetOcclusionQueryivNV);
				GET_GL_EXT_PROC(glGetOcclusionQueryuivNV);
			}
			if (strstr(ext, "GL_NV_point_sprite")) 
			{
				GET_GL_EXT_PROC(glPointParameteriNV);
				GET_GL_EXT_PROC(glPointParameterivNV);
			}
			if (strstr(ext, "GL_EXT_stencil_two_side")) 
			{
				GET_GL_EXT_PROC(glActiveStencilFaceEXT);
			}
			if (strstr(ext, "GL_APPLE_element_array")) 
			{
				GET_GL_EXT_PROC(glElementPointerAPPLE);
				GET_GL_EXT_PROC(glDrawElementArrayAPPLE);
				GET_GL_EXT_PROC(glDrawRangeElementArrayAPPLE);
				GET_GL_EXT_PROC(glMultiDrawElementArrayAPPLE);
				GET_GL_EXT_PROC(glMultiDrawRangeElementArrayAPPLE);
			}
			if (strstr(ext, "GL_APPLE_fence")) 
			{
				GET_GL_EXT_PROC(glGenFencesAPPLE);
				GET_GL_EXT_PROC(glDeleteFencesAPPLE);
				GET_GL_EXT_PROC(glSetFenceAPPLE);
				GET_GL_EXT_PROC(glIsFenceAPPLE);
				GET_GL_EXT_PROC(glTestFenceAPPLE);
				GET_GL_EXT_PROC(glFinishFenceAPPLE);
				GET_GL_EXT_PROC(glTestObjectAPPLE);
				GET_GL_EXT_PROC(glFinishObjectAPPLE);
			}
			if (strstr(ext, "GL_APPLE_vertex_array_object")) 
			{
				GET_GL_EXT_PROC(glBindVertexArrayAPPLE);
				GET_GL_EXT_PROC(glDeleteVertexArraysAPPLE);
				GET_GL_EXT_PROC(glGenVertexArraysAPPLE);
				GET_GL_EXT_PROC(glIsVertexArrayAPPLE);
			}
			if (strstr(ext, "GL_APPLE_vertex_array_range")) 
			{
				GET_GL_EXT_PROC(glVertexArrayRangeAPPLE);
				GET_GL_EXT_PROC(glFlushVertexArrayRangeAPPLE);
				GET_GL_EXT_PROC(glVertexArrayParameteriAPPLE);
			}
			if (strstr(ext, "GL_ATI_draw_buffers")) 
			{
				GET_GL_EXT_PROC(glDrawBuffersATI);
			}
			if (strstr(ext, "GL_NV_fragment_program")) 
			{
				GET_GL_EXT_PROC(glProgramNamedParameter4fNV);
				GET_GL_EXT_PROC(glProgramNamedParameter4dNV);
				GET_GL_EXT_PROC(glProgramNamedParameter4fvNV);
				GET_GL_EXT_PROC(glProgramNamedParameter4dvNV);
				GET_GL_EXT_PROC(glGetProgramNamedParameterfvNV);
				GET_GL_EXT_PROC(glGetProgramNamedParameterdvNV);
			}
			if (strstr(ext, "GL_NV_half_float")) 
			{
				GET_GL_EXT_PROC(glVertex2hNV);
				GET_GL_EXT_PROC(glVertex2hvNV);
				GET_GL_EXT_PROC(glVertex3hNV);
				GET_GL_EXT_PROC(glVertex3hvNV);
				GET_GL_EXT_PROC(glVertex4hNV);
				GET_GL_EXT_PROC(glVertex4hvNV);
				GET_GL_EXT_PROC(glNormal3hNV);
				GET_GL_EXT_PROC(glNormal3hvNV);
				GET_GL_EXT_PROC(glColor3hNV);
				GET_GL_EXT_PROC(glColor3hvNV);
				GET_GL_EXT_PROC(glColor4hNV);
				GET_GL_EXT_PROC(glColor4hvNV);
				GET_GL_EXT_PROC(glTexCoord1hNV);
				GET_GL_EXT_PROC(glTexCoord1hvNV);
				GET_GL_EXT_PROC(glTexCoord2hNV);
				GET_GL_EXT_PROC(glTexCoord2hvNV);
				GET_GL_EXT_PROC(glTexCoord3hNV);
				GET_GL_EXT_PROC(glTexCoord3hvNV);
				GET_GL_EXT_PROC(glTexCoord4hNV);
				GET_GL_EXT_PROC(glTexCoord4hvNV);
				GET_GL_EXT_PROC(glMultiTexCoord1hNV);
				GET_GL_EXT_PROC(glMultiTexCoord1hvNV);
				GET_GL_EXT_PROC(glMultiTexCoord2hNV);
				GET_GL_EXT_PROC(glMultiTexCoord2hvNV);
				GET_GL_EXT_PROC(glMultiTexCoord3hNV);
				GET_GL_EXT_PROC(glMultiTexCoord3hvNV);
				GET_GL_EXT_PROC(glMultiTexCoord4hNV);
				GET_GL_EXT_PROC(glMultiTexCoord4hvNV);
				GET_GL_EXT_PROC(glFogCoordhNV);
				GET_GL_EXT_PROC(glFogCoordhvNV);
				GET_GL_EXT_PROC(glSecondaryColor3hNV);
				GET_GL_EXT_PROC(glSecondaryColor3hvNV);
				GET_GL_EXT_PROC(glVertexWeighthNV);
				GET_GL_EXT_PROC(glVertexWeighthvNV);
				GET_GL_EXT_PROC(glVertexAttrib1hNV);
				GET_GL_EXT_PROC(glVertexAttrib1hvNV);
				GET_GL_EXT_PROC(glVertexAttrib2hNV);
				GET_GL_EXT_PROC(glVertexAttrib2hvNV);
				GET_GL_EXT_PROC(glVertexAttrib3hNV);
				GET_GL_EXT_PROC(glVertexAttrib3hvNV);
				GET_GL_EXT_PROC(glVertexAttrib4hNV);
				GET_GL_EXT_PROC(glVertexAttrib4hvNV);
				GET_GL_EXT_PROC(glVertexAttribs1hvNV);
				GET_GL_EXT_PROC(glVertexAttribs2hvNV);
				GET_GL_EXT_PROC(glVertexAttribs3hvNV);
				GET_GL_EXT_PROC(glVertexAttribs4hvNV);
			}
			if (strstr(ext, "GL_NV_pixel_data_range")) 
			{
				GET_GL_EXT_PROC(glPixelDataRangeNV);
				GET_GL_EXT_PROC(glFlushPixelDataRangeNV);
			}
			if (strstr(ext, "GL_NV_primitive_restart")) 
			{
				GET_GL_EXT_PROC(glPrimitiveRestartNV);
				GET_GL_EXT_PROC(glPrimitiveRestartIndexNV);
			}
			if (strstr(ext, "GL_ATI_map_object_buffer")) 
			{
				GET_GL_EXT_PROC(glMapObjectBufferATI);
				GET_GL_EXT_PROC(glUnmapObjectBufferATI);
			}
			if (strstr(ext, "GL_ATI_separate_stencil")) 
			{
				GET_GL_EXT_PROC(glStencilOpSeparateATI);
				GET_GL_EXT_PROC(glStencilFuncSeparateATI);
			}
			if (strstr(ext, "GL_ATI_vertex_attrib_array_object")) 
			{
				GET_GL_EXT_PROC(glVertexAttribArrayObjectATI);
				GET_GL_EXT_PROC(glGetVertexAttribArrayObjectfvATI);
				GET_GL_EXT_PROC(glGetVertexAttribArrayObjectivATI);
			}
			if (strstr(ext, "GL_EXT_depth_bounds_test")) 
			{
				GET_GL_EXT_PROC(glDepthBoundsEXT);
			}
			if (strstr(ext, "GL_EXT_blend_equation_separate")) 
			{
				GET_GL_EXT_PROC(glBlendEquationSeparateEXT);
			}
			if (strstr(ext, "GL_EXT_framebuffer_object")) 
			{
				GET_GL_EXT_PROC(glIsRenderbufferEXT);
				GET_GL_EXT_PROC(glBindRenderbufferEXT);
				GET_GL_EXT_PROC(glDeleteRenderbuffersEXT);
				GET_GL_EXT_PROC(glGenRenderbuffersEXT);
				GET_GL_EXT_PROC(glRenderbufferStorageEXT);
				GET_GL_EXT_PROC(glGetRenderbufferParameterivEXT);
				GET_GL_EXT_PROC(glIsFramebufferEXT);
				GET_GL_EXT_PROC(glBindFramebufferEXT);
				GET_GL_EXT_PROC(glDeleteFramebuffersEXT);
				GET_GL_EXT_PROC(glGenFramebuffersEXT);
				GET_GL_EXT_PROC(glCheckFramebufferStatusEXT);
				GET_GL_EXT_PROC(glFramebufferTexture1DEXT);
				GET_GL_EXT_PROC(glFramebufferTexture2DEXT);
				GET_GL_EXT_PROC(glFramebufferTexture3DEXT);
				GET_GL_EXT_PROC(glFramebufferRenderbufferEXT);
				GET_GL_EXT_PROC(glGetFramebufferAttachmentParameterivEXT);
				GET_GL_EXT_PROC(glGenerateMipmapEXT);
			}
			if (strstr(ext, "GL_GREMEDY_string_marker")) 
			{
				GET_GL_EXT_PROC(glStringMarkerGREMEDY);
			}
			if (strstr(ext, "GL_EXT_stencil_clear_tag")) 
			{
				GET_GL_EXT_PROC(glStencilClearTagEXT);
			}
			if (strstr(ext, "GL_EXT_framebuffer_blit")) 
			{
				GET_GL_EXT_PROC(glBlitFramebufferEXT);
			}
			if (strstr(ext, "GL_EXT_framebuffer_multisample")) 
			{
				GET_GL_EXT_PROC(glRenderbufferStorageMultisampleEXT);
			}
			if (strstr(ext, "GL_EXT_timer_query")) 
			{
				GET_GL_EXT_PROC(glGetQueryObjecti64vEXT);
				GET_GL_EXT_PROC(glGetQueryObjectui64vEXT);
			}
			if (strstr(ext, "GL_EXT_gpu_program_parameters")) 
			{
				GET_GL_EXT_PROC(glProgramEnvParameters4fvEXT);
				GET_GL_EXT_PROC(glProgramLocalParameters4fvEXT);
			}
			if (strstr(ext, "GL_APPLE_flush_buffer_range")) 
			{
				GET_GL_EXT_PROC(glBufferParameteriAPPLE);
				GET_GL_EXT_PROC(glFlushMappedBufferRangeAPPLE);
			}
			if (strstr(ext, "GL_NV_gpu_program4")) 
			{
				GET_GL_EXT_PROC(glProgramLocalParameterI4iNV);
				GET_GL_EXT_PROC(glProgramLocalParameterI4ivNV);
				GET_GL_EXT_PROC(glProgramLocalParametersI4ivNV);
				GET_GL_EXT_PROC(glProgramLocalParameterI4uiNV);
				GET_GL_EXT_PROC(glProgramLocalParameterI4uivNV);
				GET_GL_EXT_PROC(glProgramLocalParametersI4uivNV);
				GET_GL_EXT_PROC(glProgramEnvParameterI4iNV);
				GET_GL_EXT_PROC(glProgramEnvParameterI4ivNV);
				GET_GL_EXT_PROC(glProgramEnvParametersI4ivNV);
				GET_GL_EXT_PROC(glProgramEnvParameterI4uiNV);
				GET_GL_EXT_PROC(glProgramEnvParameterI4uivNV);
				GET_GL_EXT_PROC(glProgramEnvParametersI4uivNV);
				GET_GL_EXT_PROC(glGetProgramLocalParameterIivNV);
				GET_GL_EXT_PROC(glGetProgramLocalParameterIuivNV);
				GET_GL_EXT_PROC(glGetProgramEnvParameterIivNV);
				GET_GL_EXT_PROC(glGetProgramEnvParameterIuivNV);
			}
			if (strstr(ext, "GL_NV_geometry_program4")) 
			{
				GET_GL_EXT_PROC(glProgramVertexLimitNV);
				GET_GL_EXT_PROC(glFramebufferTextureEXT);
				GET_GL_EXT_PROC(glFramebufferTextureLayerEXT);
				GET_GL_EXT_PROC(glFramebufferTextureFaceEXT);
			}
			if (strstr(ext, "GL_EXT_geometry_shader4")) 
			{
				GET_GL_EXT_PROC(glProgramParameteriEXT);
			}
			if (strstr(ext, "GL_NV_vertex_program4")) 
			{
				GET_GL_EXT_PROC(glVertexAttribI1iEXT);
				GET_GL_EXT_PROC(glVertexAttribI2iEXT);
				GET_GL_EXT_PROC(glVertexAttribI3iEXT);
				GET_GL_EXT_PROC(glVertexAttribI4iEXT);
				GET_GL_EXT_PROC(glVertexAttribI1uiEXT);
				GET_GL_EXT_PROC(glVertexAttribI2uiEXT);
				GET_GL_EXT_PROC(glVertexAttribI3uiEXT);
				GET_GL_EXT_PROC(glVertexAttribI4uiEXT);
				GET_GL_EXT_PROC(glVertexAttribI1ivEXT);
				GET_GL_EXT_PROC(glVertexAttribI2ivEXT);
				GET_GL_EXT_PROC(glVertexAttribI3ivEXT);
				GET_GL_EXT_PROC(glVertexAttribI4ivEXT);
				GET_GL_EXT_PROC(glVertexAttribI1uivEXT);
				GET_GL_EXT_PROC(glVertexAttribI2uivEXT);
				GET_GL_EXT_PROC(glVertexAttribI3uivEXT);
				GET_GL_EXT_PROC(glVertexAttribI4uivEXT);
				GET_GL_EXT_PROC(glVertexAttribI4bvEXT);
				GET_GL_EXT_PROC(glVertexAttribI4svEXT);
				GET_GL_EXT_PROC(glVertexAttribI4ubvEXT);
				GET_GL_EXT_PROC(glVertexAttribI4usvEXT);
				GET_GL_EXT_PROC(glVertexAttribIPointerEXT);
				GET_GL_EXT_PROC(glGetVertexAttribIivEXT);
				GET_GL_EXT_PROC(glGetVertexAttribIuivEXT);
			}
			if (strstr(ext, "GL_EXT_gpu_shader4")) 
			{
				GET_GL_EXT_PROC(glGetUniformuivEXT);
				GET_GL_EXT_PROC(glBindFragDataLocationEXT);
				GET_GL_EXT_PROC(glGetFragDataLocationEXT);
				GET_GL_EXT_PROC(glUniform1uiEXT);
				GET_GL_EXT_PROC(glUniform2uiEXT);
				GET_GL_EXT_PROC(glUniform3uiEXT);
				GET_GL_EXT_PROC(glUniform4uiEXT);
				GET_GL_EXT_PROC(glUniform1uivEXT);
				GET_GL_EXT_PROC(glUniform2uivEXT);
				GET_GL_EXT_PROC(glUniform3uivEXT);
				GET_GL_EXT_PROC(glUniform4uivEXT);
			}
			if (strstr(ext, "GL_EXT_draw_instanced")) 
			{
				GET_GL_EXT_PROC(glDrawArraysInstancedEXT);
				GET_GL_EXT_PROC(glDrawElementsInstancedEXT);
			}
			if (strstr(ext, "GL_EXT_texture_buffer_object")) 
			{
				GET_GL_EXT_PROC(glTexBufferEXT);
			}
			if (strstr(ext, "GL_NV_depth_buffer_float")) 
			{
				GET_GL_EXT_PROC(glDepthRangedNV);
				GET_GL_EXT_PROC(glClearDepthdNV);
				GET_GL_EXT_PROC(glDepthBoundsdNV);
			}
			if (strstr(ext, "GL_NV_framebuffer_multisample_coverage")) 
			{
				GET_GL_EXT_PROC(glRenderbufferStorageMultisampleCoverageNV);
			}
			if (strstr(ext, "GL_NV_parameter_buffer_object")) 
			{
				GET_GL_EXT_PROC(glProgramBufferParametersfvNV);
				GET_GL_EXT_PROC(glProgramBufferParametersIivNV);
				GET_GL_EXT_PROC(glProgramBufferParametersIuivNV);
			}
			if (strstr(ext, "GL_EXT_draw_buffers2")) 
			{
				GET_GL_EXT_PROC(glColorMaskIndexedEXT);
				GET_GL_EXT_PROC(glGetBooleanIndexedvEXT);
				GET_GL_EXT_PROC(glGetIntegerIndexedvEXT);
				GET_GL_EXT_PROC(glEnableIndexedEXT);
				GET_GL_EXT_PROC(glDisableIndexedEXT);
				GET_GL_EXT_PROC(glIsEnabledIndexedEXT);
			}
			if (strstr(ext, "GL_NV_transform_feedback")) 
			{
				GET_GL_EXT_PROC(glBeginTransformFeedbackNV);
				GET_GL_EXT_PROC(glEndTransformFeedbackNV);
				GET_GL_EXT_PROC(glTransformFeedbackAttribsNV);
				GET_GL_EXT_PROC(glBindBufferRangeNV);
				GET_GL_EXT_PROC(glBindBufferOffsetNV);
				GET_GL_EXT_PROC(glBindBufferBaseNV);
				GET_GL_EXT_PROC(glTransformFeedbackVaryingsNV);
				GET_GL_EXT_PROC(glActiveVaryingNV);
				GET_GL_EXT_PROC(glGetVaryingLocationNV);
				GET_GL_EXT_PROC(glGetActiveVaryingNV);
				GET_GL_EXT_PROC(glGetTransformFeedbackVaryingNV);
			}
			if (strstr(ext, "GL_EXT_bindable_uniform")) 
			{
				GET_GL_EXT_PROC(glUniformBufferEXT);
				GET_GL_EXT_PROC(glGetUniformBufferSizeEXT);
				GET_GL_EXT_PROC(glGetUniformOffsetEXT);
			}
			if (strstr(ext, "GL_EXT_texture_integer")) 
			{
				GET_GL_EXT_PROC(glTexParameterIivEXT);
				GET_GL_EXT_PROC(glTexParameterIuivEXT);
				GET_GL_EXT_PROC(glGetTexParameterIivEXT);
				GET_GL_EXT_PROC(glGetTexParameterIuivEXT);
				GET_GL_EXT_PROC(glClearColorIiEXT);
				GET_GL_EXT_PROC(glClearColorIuiEXT);
			}
			if (strstr(ext, "GL_GREMEDY_frame_terminator")) 
			{
				GET_GL_EXT_PROC(glFrameTerminatorGREMEDY);
			}
			if (strstr(ext, "GL_NV_conditional_render")) 
			{
				GET_GL_EXT_PROC(glBeginConditionalRenderNV);
				GET_GL_EXT_PROC(glEndConditionalRenderNV);
			}
			if (strstr(ext, "GL_NV_present_video")) 
			{
				GET_GL_EXT_PROC(glPresentFrameKeyedNV);
				GET_GL_EXT_PROC(glPresentFrameDualFillNV);
				GET_GL_EXT_PROC(glGetVideoivNV);
				GET_GL_EXT_PROC(glGetVideouivNV);
				GET_GL_EXT_PROC(glGetVideoi64vNV);
				GET_GL_EXT_PROC(glGetVideoui64vNV);
			}
			if (strstr(ext, "GL_EXT_transform_feedback")) 
			{
				GET_GL_EXT_PROC(glBeginTransformFeedbackEXT);
				GET_GL_EXT_PROC(glEndTransformFeedbackEXT);
				GET_GL_EXT_PROC(glBindBufferRangeEXT);
				GET_GL_EXT_PROC(glBindBufferOffsetEXT);
				GET_GL_EXT_PROC(glBindBufferBaseEXT);
				GET_GL_EXT_PROC(glTransformFeedbackVaryingsEXT);
				GET_GL_EXT_PROC(glGetTransformFeedbackVaryingEXT);
			}
			if (strstr(ext, "GL_EXT_direct_state_access")) 
			{
				GET_GL_EXT_PROC(glClientAttribDefaultEXT);
				GET_GL_EXT_PROC(glPushClientAttribDefaultEXT);
				GET_GL_EXT_PROC(glMatrixLoadfEXT);
				GET_GL_EXT_PROC(glMatrixLoaddEXT);
				GET_GL_EXT_PROC(glMatrixMultfEXT);
				GET_GL_EXT_PROC(glMatrixMultdEXT);
				GET_GL_EXT_PROC(glMatrixLoadIdentityEXT);
				GET_GL_EXT_PROC(glMatrixRotatefEXT);
				GET_GL_EXT_PROC(glMatrixRotatedEXT);
				GET_GL_EXT_PROC(glMatrixScalefEXT);
				GET_GL_EXT_PROC(glMatrixScaledEXT);
				GET_GL_EXT_PROC(glMatrixTranslatefEXT);
				GET_GL_EXT_PROC(glMatrixTranslatedEXT);
				GET_GL_EXT_PROC(glMatrixFrustumEXT);
				GET_GL_EXT_PROC(glMatrixOrthoEXT);
				GET_GL_EXT_PROC(glMatrixPopEXT);
				GET_GL_EXT_PROC(glMatrixPushEXT);
				GET_GL_EXT_PROC(glMatrixLoadTransposefEXT);
				GET_GL_EXT_PROC(glMatrixLoadTransposedEXT);
				GET_GL_EXT_PROC(glMatrixMultTransposefEXT);
				GET_GL_EXT_PROC(glMatrixMultTransposedEXT);
				GET_GL_EXT_PROC(glTextureParameterfEXT);
				GET_GL_EXT_PROC(glTextureParameterfvEXT);
				GET_GL_EXT_PROC(glTextureParameteriEXT);
				GET_GL_EXT_PROC(glTextureParameterivEXT);
				GET_GL_EXT_PROC(glTextureImage1DEXT);
				GET_GL_EXT_PROC(glTextureImage2DEXT);
				GET_GL_EXT_PROC(glTextureSubImage1DEXT);
				GET_GL_EXT_PROC(glTextureSubImage2DEXT);
				GET_GL_EXT_PROC(glCopyTextureImage1DEXT);
				GET_GL_EXT_PROC(glCopyTextureImage2DEXT);
				GET_GL_EXT_PROC(glCopyTextureSubImage1DEXT);
				GET_GL_EXT_PROC(glCopyTextureSubImage2DEXT);
				GET_GL_EXT_PROC(glGetTextureImageEXT);
				GET_GL_EXT_PROC(glGetTextureParameterfvEXT);
				GET_GL_EXT_PROC(glGetTextureParameterivEXT);
				GET_GL_EXT_PROC(glGetTextureLevelParameterfvEXT);
				GET_GL_EXT_PROC(glGetTextureLevelParameterivEXT);
				GET_GL_EXT_PROC(glTextureImage3DEXT);
				GET_GL_EXT_PROC(glTextureSubImage3DEXT);
				GET_GL_EXT_PROC(glCopyTextureSubImage3DEXT);
				GET_GL_EXT_PROC(glMultiTexParameterfEXT);
				GET_GL_EXT_PROC(glMultiTexParameterfvEXT);
				GET_GL_EXT_PROC(glMultiTexParameteriEXT);
				GET_GL_EXT_PROC(glMultiTexParameterivEXT);
				GET_GL_EXT_PROC(glMultiTexImage1DEXT);
				GET_GL_EXT_PROC(glMultiTexImage2DEXT);
				GET_GL_EXT_PROC(glMultiTexSubImage1DEXT);
				GET_GL_EXT_PROC(glMultiTexSubImage2DEXT);
				GET_GL_EXT_PROC(glCopyMultiTexImage1DEXT);
				GET_GL_EXT_PROC(glCopyMultiTexImage2DEXT);
				GET_GL_EXT_PROC(glCopyMultiTexSubImage1DEXT);
				GET_GL_EXT_PROC(glCopyMultiTexSubImage2DEXT);
				GET_GL_EXT_PROC(glGetMultiTexImageEXT);
				GET_GL_EXT_PROC(glGetMultiTexParameterfvEXT);
				GET_GL_EXT_PROC(glGetMultiTexParameterivEXT);
				GET_GL_EXT_PROC(glGetMultiTexLevelParameterfvEXT);
				GET_GL_EXT_PROC(glGetMultiTexLevelParameterivEXT);
				GET_GL_EXT_PROC(glMultiTexImage3DEXT);
				GET_GL_EXT_PROC(glMultiTexSubImage3DEXT);
				GET_GL_EXT_PROC(glCopyMultiTexSubImage3DEXT);
				GET_GL_EXT_PROC(glBindMultiTextureEXT);
				GET_GL_EXT_PROC(glEnableClientStateIndexedEXT);
				GET_GL_EXT_PROC(glDisableClientStateIndexedEXT);
				GET_GL_EXT_PROC(glMultiTexCoordPointerEXT);
				GET_GL_EXT_PROC(glMultiTexEnvfEXT);
				GET_GL_EXT_PROC(glMultiTexEnvfvEXT);
				GET_GL_EXT_PROC(glMultiTexEnviEXT);
				GET_GL_EXT_PROC(glMultiTexEnvivEXT);
				GET_GL_EXT_PROC(glMultiTexGendEXT);
				GET_GL_EXT_PROC(glMultiTexGendvEXT);
				GET_GL_EXT_PROC(glMultiTexGenfEXT);
				GET_GL_EXT_PROC(glMultiTexGenfvEXT);
				GET_GL_EXT_PROC(glMultiTexGeniEXT);
				GET_GL_EXT_PROC(glMultiTexGenivEXT);
				GET_GL_EXT_PROC(glGetMultiTexEnvfvEXT);
				GET_GL_EXT_PROC(glGetMultiTexEnvivEXT);
				GET_GL_EXT_PROC(glGetMultiTexGendvEXT);
				GET_GL_EXT_PROC(glGetMultiTexGenfvEXT);
				GET_GL_EXT_PROC(glGetMultiTexGenivEXT);
				GET_GL_EXT_PROC(glGetFloatIndexedvEXT);
				GET_GL_EXT_PROC(glGetDoubleIndexedvEXT);
				GET_GL_EXT_PROC(glGetPointerIndexedvEXT);
				GET_GL_EXT_PROC(glCompressedTextureImage3DEXT);
				GET_GL_EXT_PROC(glCompressedTextureImage2DEXT);
				GET_GL_EXT_PROC(glCompressedTextureImage1DEXT);
				GET_GL_EXT_PROC(glCompressedTextureSubImage3DEXT);
				GET_GL_EXT_PROC(glCompressedTextureSubImage2DEXT);
				GET_GL_EXT_PROC(glCompressedTextureSubImage1DEXT);
				GET_GL_EXT_PROC(glGetCompressedTextureImageEXT);
				GET_GL_EXT_PROC(glCompressedMultiTexImage3DEXT);
				GET_GL_EXT_PROC(glCompressedMultiTexImage2DEXT);
				GET_GL_EXT_PROC(glCompressedMultiTexImage1DEXT);
				GET_GL_EXT_PROC(glCompressedMultiTexSubImage3DEXT);
				GET_GL_EXT_PROC(glCompressedMultiTexSubImage2DEXT);
				GET_GL_EXT_PROC(glCompressedMultiTexSubImage1DEXT);
				GET_GL_EXT_PROC(glGetCompressedMultiTexImageEXT);
				GET_GL_EXT_PROC(glNamedProgramStringEXT);
				GET_GL_EXT_PROC(glNamedProgramLocalParameter4dEXT);
				GET_GL_EXT_PROC(glNamedProgramLocalParameter4dvEXT);
				GET_GL_EXT_PROC(glNamedProgramLocalParameter4fEXT);
				GET_GL_EXT_PROC(glNamedProgramLocalParameter4fvEXT);
				GET_GL_EXT_PROC(glGetNamedProgramLocalParameterdvEXT);
				GET_GL_EXT_PROC(glGetNamedProgramLocalParameterfvEXT);
				GET_GL_EXT_PROC(glGetNamedProgramivEXT);
				GET_GL_EXT_PROC(glGetNamedProgramStringEXT);
				GET_GL_EXT_PROC(glNamedProgramLocalParameters4fvEXT);
				GET_GL_EXT_PROC(glNamedProgramLocalParameterI4iEXT);
				GET_GL_EXT_PROC(glNamedProgramLocalParameterI4ivEXT);
				GET_GL_EXT_PROC(glNamedProgramLocalParametersI4ivEXT);
				GET_GL_EXT_PROC(glNamedProgramLocalParameterI4uiEXT);
				GET_GL_EXT_PROC(glNamedProgramLocalParameterI4uivEXT);
				GET_GL_EXT_PROC(glNamedProgramLocalParametersI4uivEXT);
				GET_GL_EXT_PROC(glGetNamedProgramLocalParameterIivEXT);
				GET_GL_EXT_PROC(glGetNamedProgramLocalParameterIuivEXT);
				GET_GL_EXT_PROC(glTextureParameterIivEXT);
				GET_GL_EXT_PROC(glTextureParameterIuivEXT);
				GET_GL_EXT_PROC(glGetTextureParameterIivEXT);
				GET_GL_EXT_PROC(glGetTextureParameterIuivEXT);
				GET_GL_EXT_PROC(glMultiTexParameterIivEXT);
				GET_GL_EXT_PROC(glMultiTexParameterIuivEXT);
				GET_GL_EXT_PROC(glGetMultiTexParameterIivEXT);
				GET_GL_EXT_PROC(glGetMultiTexParameterIuivEXT);
				GET_GL_EXT_PROC(glProgramUniform1fEXT);
				GET_GL_EXT_PROC(glProgramUniform2fEXT);
				GET_GL_EXT_PROC(glProgramUniform3fEXT);
				GET_GL_EXT_PROC(glProgramUniform4fEXT);
				GET_GL_EXT_PROC(glProgramUniform1iEXT);
				GET_GL_EXT_PROC(glProgramUniform2iEXT);
				GET_GL_EXT_PROC(glProgramUniform3iEXT);
				GET_GL_EXT_PROC(glProgramUniform4iEXT);
				GET_GL_EXT_PROC(glProgramUniform1fvEXT);
				GET_GL_EXT_PROC(glProgramUniform2fvEXT);
				GET_GL_EXT_PROC(glProgramUniform3fvEXT);
				GET_GL_EXT_PROC(glProgramUniform4fvEXT);
				GET_GL_EXT_PROC(glProgramUniform1ivEXT);
				GET_GL_EXT_PROC(glProgramUniform2ivEXT);
				GET_GL_EXT_PROC(glProgramUniform3ivEXT);
				GET_GL_EXT_PROC(glProgramUniform4ivEXT);
				GET_GL_EXT_PROC(glProgramUniformMatrix2fvEXT);
				GET_GL_EXT_PROC(glProgramUniformMatrix3fvEXT);
				GET_GL_EXT_PROC(glProgramUniformMatrix4fvEXT);
				GET_GL_EXT_PROC(glProgramUniformMatrix2x3fvEXT);
				GET_GL_EXT_PROC(glProgramUniformMatrix3x2fvEXT);
				GET_GL_EXT_PROC(glProgramUniformMatrix2x4fvEXT);
				GET_GL_EXT_PROC(glProgramUniformMatrix4x2fvEXT);
				GET_GL_EXT_PROC(glProgramUniformMatrix3x4fvEXT);
				GET_GL_EXT_PROC(glProgramUniformMatrix4x3fvEXT);
				GET_GL_EXT_PROC(glProgramUniform1uiEXT);
				GET_GL_EXT_PROC(glProgramUniform2uiEXT);
				GET_GL_EXT_PROC(glProgramUniform3uiEXT);
				GET_GL_EXT_PROC(glProgramUniform4uiEXT);
				GET_GL_EXT_PROC(glProgramUniform1uivEXT);
				GET_GL_EXT_PROC(glProgramUniform2uivEXT);
				GET_GL_EXT_PROC(glProgramUniform3uivEXT);
				GET_GL_EXT_PROC(glProgramUniform4uivEXT);
				GET_GL_EXT_PROC(glNamedBufferDataEXT);
				GET_GL_EXT_PROC(glNamedBufferSubDataEXT);
				GET_GL_EXT_PROC(glMapNamedBufferEXT);
				GET_GL_EXT_PROC(glUnmapNamedBufferEXT);
				GET_GL_EXT_PROC(glGetNamedBufferParameterivEXT);
				GET_GL_EXT_PROC(glGetNamedBufferPointervEXT);
				GET_GL_EXT_PROC(glGetNamedBufferSubDataEXT);
				GET_GL_EXT_PROC(glTextureBufferEXT);
				GET_GL_EXT_PROC(glMultiTexBufferEXT);
				GET_GL_EXT_PROC(glNamedRenderbufferStorageEXT);
				GET_GL_EXT_PROC(glGetNamedRenderbufferParameterivEXT);
				GET_GL_EXT_PROC(glCheckNamedFramebufferStatusEXT);
				GET_GL_EXT_PROC(glNamedFramebufferTexture1DEXT);
				GET_GL_EXT_PROC(glNamedFramebufferTexture2DEXT);
				GET_GL_EXT_PROC(glNamedFramebufferTexture3DEXT);
				GET_GL_EXT_PROC(glNamedFramebufferRenderbufferEXT);
				GET_GL_EXT_PROC(glGetNamedFramebufferAttachmentParameterivEXT);
				GET_GL_EXT_PROC(glGenerateTextureMipmapEXT);
				GET_GL_EXT_PROC(glGenerateMultiTexMipmapEXT);
				GET_GL_EXT_PROC(glFramebufferDrawBufferEXT);
				GET_GL_EXT_PROC(glFramebufferDrawBuffersEXT);
				GET_GL_EXT_PROC(glFramebufferReadBufferEXT);
				GET_GL_EXT_PROC(glGetFramebufferParameterivEXT);
				GET_GL_EXT_PROC(glNamedRenderbufferStorageMultisampleEXT);
				GET_GL_EXT_PROC(glNamedRenderbufferStorageMultisampleCoverageEXT);
				GET_GL_EXT_PROC(glNamedFramebufferTextureEXT);
				GET_GL_EXT_PROC(glNamedFramebufferTextureLayerEXT);
				GET_GL_EXT_PROC(glNamedFramebufferTextureFaceEXT);
				GET_GL_EXT_PROC(glTextureRenderbufferEXT);
				GET_GL_EXT_PROC(glMultiTexRenderbufferEXT);
			}
			if (strstr(ext, "GL_NV_explicit_multisample")) 
			{
				GET_GL_EXT_PROC(glGetMultisamplefvNV);
				GET_GL_EXT_PROC(glSampleMaskIndexedNV);
				GET_GL_EXT_PROC(glTexRenderbufferNV);
			}
			if (strstr(ext, "GL_NV_transform_feedback2")) 
			{
				GET_GL_EXT_PROC(glBindTransformFeedbackNV);
				GET_GL_EXT_PROC(glDeleteTransformFeedbacksNV);
				GET_GL_EXT_PROC(glGenTransformFeedbacksNV);
				GET_GL_EXT_PROC(glIsTransformFeedbackNV);
				GET_GL_EXT_PROC(glPauseTransformFeedbackNV);
				GET_GL_EXT_PROC(glResumeTransformFeedbackNV);
				GET_GL_EXT_PROC(glDrawTransformFeedbackNV);
			}
			if (strstr(ext, "GL_AMD_performance_monitor")) 
			{
				GET_GL_EXT_PROC(glGetPerfMonitorGroupsAMD);
				GET_GL_EXT_PROC(glGetPerfMonitorCountersAMD);
				GET_GL_EXT_PROC(glGetPerfMonitorGroupStringAMD);
				GET_GL_EXT_PROC(glGetPerfMonitorCounterStringAMD);
				GET_GL_EXT_PROC(glGetPerfMonitorCounterInfoAMD);
				GET_GL_EXT_PROC(glGenPerfMonitorsAMD);
				GET_GL_EXT_PROC(glDeletePerfMonitorsAMD);
				GET_GL_EXT_PROC(glSelectPerfMonitorCountersAMD);
				GET_GL_EXT_PROC(glBeginPerfMonitorAMD);
				GET_GL_EXT_PROC(glEndPerfMonitorAMD);
				GET_GL_EXT_PROC(glGetPerfMonitorCounterDataAMD);
			}
			if (strstr(ext, "GL_AMD_vertex_shader_tesselator")) 
			{
				GET_GL_EXT_PROC(glTessellationFactorAMD);
				GET_GL_EXT_PROC(glTessellationModeAMD);
			}
			if (strstr(ext, "GL_EXT_provoking_vertex")) 
			{
				GET_GL_EXT_PROC(glProvokingVertexEXT);
			}
			if (strstr(ext, "GL_AMD_draw_buffers_blend")) 
			{
				GET_GL_EXT_PROC(glBlendFuncIndexedAMD);
				GET_GL_EXT_PROC(glBlendFuncSeparateIndexedAMD);
				GET_GL_EXT_PROC(glBlendEquationIndexedAMD);
				GET_GL_EXT_PROC(glBlendEquationSeparateIndexedAMD);
			}
			if (strstr(ext, "GL_APPLE_texture_range")) 
			{
				GET_GL_EXT_PROC(glTextureRangeAPPLE);
				GET_GL_EXT_PROC(glGetTexParameterPointervAPPLE);
			}
			if (strstr(ext, "GL_APPLE_vertex_program_evaluators")) 
			{
				GET_GL_EXT_PROC(glEnableVertexAttribAPPLE);
				GET_GL_EXT_PROC(glDisableVertexAttribAPPLE);
				GET_GL_EXT_PROC(glIsVertexAttribEnabledAPPLE);
				GET_GL_EXT_PROC(glMapVertexAttrib1dAPPLE);
				GET_GL_EXT_PROC(glMapVertexAttrib1fAPPLE);
				GET_GL_EXT_PROC(glMapVertexAttrib2dAPPLE);
				GET_GL_EXT_PROC(glMapVertexAttrib2fAPPLE);
			}
			if (strstr(ext, "GL_APPLE_object_purgeable")) 
			{
				GET_GL_EXT_PROC(glObjectPurgeableAPPLE);
				GET_GL_EXT_PROC(glObjectUnpurgeableAPPLE);
				GET_GL_EXT_PROC(glGetObjectParameterivAPPLE);
			}
			if (strstr(ext, "GL_NV_video_capture")) 
			{
				GET_GL_EXT_PROC(glBeginVideoCaptureNV);
				GET_GL_EXT_PROC(glBindVideoCaptureStreamBufferNV);
				GET_GL_EXT_PROC(glBindVideoCaptureStreamTextureNV);
				GET_GL_EXT_PROC(glEndVideoCaptureNV);
				GET_GL_EXT_PROC(glGetVideoCaptureivNV);
				GET_GL_EXT_PROC(glGetVideoCaptureStreamivNV);
				GET_GL_EXT_PROC(glGetVideoCaptureStreamfvNV);
				GET_GL_EXT_PROC(glGetVideoCaptureStreamdvNV);
				GET_GL_EXT_PROC(glVideoCaptureNV);
				GET_GL_EXT_PROC(glVideoCaptureStreamParameterivNV);
				GET_GL_EXT_PROC(glVideoCaptureStreamParameterfvNV);
				GET_GL_EXT_PROC(glVideoCaptureStreamParameterdvNV);
			}
			if (strstr(ext, "GL_NV_copy_image")) 
			{
				GET_GL_EXT_PROC(glCopyImageSubDataNV);
			}
			if (strstr(ext, "GL_EXT_separate_shader_objects")) 
			{
				GET_GL_EXT_PROC(glUseShaderProgramEXT);
				GET_GL_EXT_PROC(glActiveProgramEXT);
				GET_GL_EXT_PROC(glCreateShaderProgramEXT);
			}
			if (strstr(ext, "GL_NV_shader_buffer_load")) 
			{
				GET_GL_EXT_PROC(glMakeBufferResidentNV);
				GET_GL_EXT_PROC(glMakeBufferNonResidentNV);
				GET_GL_EXT_PROC(glIsBufferResidentNV);
				GET_GL_EXT_PROC(glNamedMakeBufferResidentNV);
				GET_GL_EXT_PROC(glNamedMakeBufferNonResidentNV);
				GET_GL_EXT_PROC(glIsNamedBufferResidentNV);
				GET_GL_EXT_PROC(glGetBufferParameterui64vNV);
				GET_GL_EXT_PROC(glGetNamedBufferParameterui64vNV);
				GET_GL_EXT_PROC(glGetIntegerui64vNV);
				GET_GL_EXT_PROC(glUniformui64NV);
				GET_GL_EXT_PROC(glUniformui64vNV);
				GET_GL_EXT_PROC(glGetUniformui64vNV);
				GET_GL_EXT_PROC(glProgramUniformui64NV);
				GET_GL_EXT_PROC(glProgramUniformui64vNV);
			}
			if (strstr(ext, "GL_NV_vertex_buffer_unified_memory")) 
			{
				GET_GL_EXT_PROC(glBufferAddressRangeNV);
				GET_GL_EXT_PROC(glVertexFormatNV);
				GET_GL_EXT_PROC(glNormalFormatNV);
				GET_GL_EXT_PROC(glColorFormatNV);
				GET_GL_EXT_PROC(glIndexFormatNV);
				GET_GL_EXT_PROC(glTexCoordFormatNV);
				GET_GL_EXT_PROC(glEdgeFlagFormatNV);
				GET_GL_EXT_PROC(glSecondaryColorFormatNV);
				GET_GL_EXT_PROC(glFogCoordFormatNV);
				GET_GL_EXT_PROC(glVertexAttribFormatNV);
				GET_GL_EXT_PROC(glVertexAttribIFormatNV);
				GET_GL_EXT_PROC(glGetIntegerui64i_vNV);
			}
			if (strstr(ext, "GL_NV_texture_barrier")) 
			{
				GET_GL_EXT_PROC(glTextureBarrierNV);
			}

			return true;
		}
	}
}

#endif		// #ifdef _WIN32
