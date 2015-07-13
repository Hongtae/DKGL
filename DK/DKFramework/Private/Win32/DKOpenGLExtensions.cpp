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
#include "../../../../lib/OpenGL/glext.h"
#include "../../../../lib/OpenGL/glcorearb.h"
#include "../../../../lib/OpenGL/wglext.h"
#include "../../../DKInclude.h"
#include "../../../DKFoundation.h"

namespace OpenGL
{
#if 0
	// GL_VERSION_1_0
	extern PFNGLCULLFACEPROC					glCullFace = 0;
	extern PFNGLFRONTFACEPROC					glFrontFace = 0;
	extern PFNGLHINTPROC						glHint = 0;
	extern PFNGLLINEWIDTHPROC					glLineWidth = 0;
	extern PFNGLPOINTSIZEPROC					glPointSize = 0;
	extern PFNGLPOLYGONMODEPROC					glPolygonMode = 0;
	extern PFNGLSCISSORPROC						glScissor = 0;
	extern PFNGLTEXPARAMETERFPROC				glTexParameterf = 0;
	extern PFNGLTEXPARAMETERFVPROC				glTexParameterfv = 0;
	extern PFNGLTEXPARAMETERIPROC				glTexParameteri = 0;
	extern PFNGLTEXPARAMETERIVPROC				glTexParameteriv = 0;
	extern PFNGLTEXIMAGE1DPROC					glTexImage1D = 0;
	extern PFNGLTEXIMAGE2DPROC					glTexImage2D = 0;
	extern PFNGLDRAWBUFFERPROC					glDrawBuffer = 0;
	extern PFNGLCLEARPROC						glClear = 0;
	extern PFNGLCLEARCOLORPROC					glClearColor = 0;
	extern PFNGLCLEARSTENCILPROC				glClearStencil = 0;
	extern PFNGLCLEARDEPTHPROC					glClearDepth = 0;
	extern PFNGLSTENCILMASKPROC					glStencilMask = 0;
	extern PFNGLCOLORMASKPROC					glColorMask = 0;
	extern PFNGLDEPTHMASKPROC					glDepthMask = 0;
	extern PFNGLDISABLEPROC						glDisable = 0;
	extern PFNGLENABLEPROC						glEnable = 0;
	extern PFNGLFINISHPROC						glFinish = 0;
	extern PFNGLFLUSHPROC						glFlush = 0;
	extern PFNGLBLENDFUNCPROC					glBlendFunc = 0;
	extern PFNGLLOGICOPPROC						glLogicOp = 0;
	extern PFNGLSTENCILFUNCPROC					glStencilFunc = 0;
	extern PFNGLSTENCILOPPROC					glStencilOp = 0;
	extern PFNGLDEPTHFUNCPROC					glDepthFunc = 0;
	extern PFNGLPIXELSTOREFPROC					glPixelStoref = 0;
	extern PFNGLPIXELSTOREIPROC					glPixelStorei = 0;
	extern PFNGLREADBUFFERPROC					glReadBuffer = 0;
	extern PFNGLREADPIXELSPROC					glReadPixels = 0;
	extern PFNGLGETBOOLEANVPROC					glGetBooleanv = 0;
	extern PFNGLGETDOUBLEVPROC					glGetDoublev = 0;
	extern PFNGLGETERRORPROC					glGetError = 0;
	extern PFNGLGETFLOATVPROC					glGetFloatv = 0;
	extern PFNGLGETINTEGERVPROC					glGetIntegerv = 0;
	extern PFNGLGETSTRINGPROC					glGetString = 0;
	extern PFNGLGETTEXIMAGEPROC					glGetTexImage = 0;
	extern PFNGLGETTEXPARAMETERFVPROC			glGetTexParameterfv = 0;
	extern PFNGLGETTEXPARAMETERIVPROC			glGetTexParameteriv = 0;
	extern PFNGLGETTEXLEVELPARAMETERFVPROC		glGetTexLevelParameterfv = 0;
	extern PFNGLGETTEXLEVELPARAMETERIVPROC		glGetTexLevelParameteriv = 0;
	extern PFNGLISENABLEDPROC					glIsEnabled = 0;
	extern PFNGLDEPTHRANGEPROC					glDepthRange = 0;
	extern PFNGLVIEWPORTPROC					glViewport = 0;

	// GL_VERSION_1_1
	extern PFNGLDRAWARRAYSPROC					glDrawArrays = 0;
	extern PFNGLDRAWELEMENTSPROC				glDrawElements = 0;
	extern PFNGLGETPOINTERVPROC					glGetPointerv = 0;
	extern PFNGLPOLYGONOFFSETPROC				glPolygonOffset = 0;
	extern PFNGLCOPYTEXIMAGE1DPROC				glCopyTexImage1D = 0;
	extern PFNGLCOPYTEXIMAGE2DPROC				glCopyTexImage2D = 0;
	extern PFNGLCOPYTEXSUBIMAGE1DPROC			glCopyTexSubImage1D = 0;
	extern PFNGLCOPYTEXSUBIMAGE2DPROC			glCopyTexSubImage2D = 0;
	extern PFNGLTEXSUBIMAGE1DPROC				glTexSubImage1D = 0;
	extern PFNGLTEXSUBIMAGE2DPROC				glTexSubImage2D = 0;
	extern PFNGLBINDTEXTUREPROC					glBindTexture = 0;
	extern PFNGLDELETETEXTURESPROC				glDeleteTextures = 0;
	extern PFNGLGENTEXTURESPROC					glGenTextures = 0;
	extern PFNGLISTEXTUREPROC					glIsTexture = 0;
#endif
	// GL_VERSION_1_2
	extern PFNGLDRAWRANGEELEMENTSPROC			glDrawRangeElements = 0;
	extern PFNGLTEXIMAGE3DPROC					glTexImage3D = 0;
	extern PFNGLTEXSUBIMAGE3DPROC				glTexSubImage3D = 0;
	extern PFNGLCOPYTEXSUBIMAGE3DPROC			glCopyTexSubImage3D = 0;

	// GL_VERSION_1_3
	extern PFNGLACTIVETEXTUREPROC					glActiveTexture = 0;
	extern PFNGLSAMPLECOVERAGEPROC					glSampleCoverage = 0;
	extern PFNGLCOMPRESSEDTEXIMAGE3DPROC			glCompressedTexImage3D = 0;
	extern PFNGLCOMPRESSEDTEXIMAGE2DPROC			glCompressedTexImage2D = 0;
	extern PFNGLCOMPRESSEDTEXIMAGE1DPROC			glCompressedTexImage1D = 0;
	extern PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC			glCompressedTexSubImage3D = 0;
	extern PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC			glCompressedTexSubImage2D = 0;
	extern PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC			glCompressedTexSubImage1D = 0;
	extern PFNGLGETCOMPRESSEDTEXIMAGEPROC			glGetCompressedTexImage = 0;

	// GL_VERSION_1_4
	extern PFNGLBLENDFUNCSEPARATEPROC				glBlendFuncSeparate = 0;
	extern PFNGLMULTIDRAWARRAYSPROC					glMultiDrawArrays = 0;
	extern PFNGLMULTIDRAWELEMENTSPROC				glMultiDrawElements = 0;
	extern PFNGLPOINTPARAMETERFPROC					glPointParameterf = 0;
	extern PFNGLPOINTPARAMETERFVPROC				glPointParameterfv = 0;
	extern PFNGLPOINTPARAMETERIPROC					glPointParameteri = 0;
	extern PFNGLPOINTPARAMETERIVPROC				glPointParameteriv = 0;
	extern PFNGLBLENDCOLORPROC						glBlendColor = 0;
	extern PFNGLBLENDEQUATIONPROC					glBlendEquation = 0;

	// GL_VERSION_1_5
	extern PFNGLGENQUERIESPROC						glGenQueries = 0;
	extern PFNGLDELETEQUERIESPROC					glDeleteQueries = 0;
	extern PFNGLISQUERYPROC							glIsQuery = 0;
	extern PFNGLBEGINQUERYPROC						glBeginQuery = 0;
	extern PFNGLENDQUERYPROC						glEndQuery = 0;
	extern PFNGLGETQUERYIVPROC						glGetQueryiv = 0;
	extern PFNGLGETQUERYOBJECTIVPROC				glGetQueryObjectiv = 0;
	extern PFNGLGETQUERYOBJECTUIVPROC				glGetQueryObjectuiv = 0;
	extern PFNGLBINDBUFFERPROC						glBindBuffer = 0;
	extern PFNGLDELETEBUFFERSPROC					glDeleteBuffers = 0;
	extern PFNGLGENBUFFERSPROC						glGenBuffers = 0;
	extern PFNGLISBUFFERPROC						glIsBuffer = 0;
	extern PFNGLBUFFERDATAPROC						glBufferData = 0;
	extern PFNGLBUFFERSUBDATAPROC					glBufferSubData = 0;
	extern PFNGLGETBUFFERSUBDATAPROC				glGetBufferSubData = 0;
	extern PFNGLMAPBUFFERPROC						glMapBuffer = 0;
	extern PFNGLUNMAPBUFFERPROC						glUnmapBuffer = 0;
	extern PFNGLGETBUFFERPARAMETERIVPROC			glGetBufferParameteriv = 0;
	extern PFNGLGETBUFFERPOINTERVPROC				glGetBufferPointerv = 0;

	// GL_VERSION_2_0
	extern PFNGLBLENDEQUATIONSEPARATEPROC			glBlendEquationSeparate = 0;
	extern PFNGLDRAWBUFFERSPROC						glDrawBuffers = 0;
	extern PFNGLSTENCILOPSEPARATEPROC				glStencilOpSeparate = 0;
	extern PFNGLSTENCILFUNCSEPARATEPROC				glStencilFuncSeparate = 0;
	extern PFNGLSTENCILMASKSEPARATEPROC				glStencilMaskSeparate = 0;
	extern PFNGLATTACHSHADERPROC					glAttachShader = 0;
	extern PFNGLBINDATTRIBLOCATIONPROC				glBindAttribLocation = 0;
	extern PFNGLCOMPILESHADERPROC					glCompileShader = 0;
	extern PFNGLCREATEPROGRAMPROC					glCreateProgram = 0;
	extern PFNGLCREATESHADERPROC					glCreateShader = 0;
	extern PFNGLDELETEPROGRAMPROC					glDeleteProgram = 0;
	extern PFNGLDELETESHADERPROC					glDeleteShader = 0;
	extern PFNGLDETACHSHADERPROC					glDetachShader = 0;
	extern PFNGLDISABLEVERTEXATTRIBARRAYPROC		glDisableVertexAttribArray = 0;
	extern PFNGLENABLEVERTEXATTRIBARRAYPROC			glEnableVertexAttribArray = 0;
	extern PFNGLGETACTIVEATTRIBPROC					glGetActiveAttrib = 0;
	extern PFNGLGETACTIVEUNIFORMPROC				glGetActiveUniform = 0;
	extern PFNGLGETATTACHEDSHADERSPROC				glGetAttachedShaders = 0;
	extern PFNGLGETATTRIBLOCATIONPROC				glGetAttribLocation = 0;
	extern PFNGLGETPROGRAMIVPROC					glGetProgramiv = 0;
	extern PFNGLGETPROGRAMINFOLOGPROC				glGetProgramInfoLog = 0;
	extern PFNGLGETSHADERIVPROC						glGetShaderiv = 0;
	extern PFNGLGETSHADERINFOLOGPROC				glGetShaderInfoLog = 0;
	extern PFNGLGETSHADERSOURCEPROC					glGetShaderSource = 0;
	extern PFNGLGETUNIFORMLOCATIONPROC				glGetUniformLocation = 0;
	extern PFNGLGETUNIFORMFVPROC					glGetUniformfv = 0;
	extern PFNGLGETUNIFORMIVPROC					glGetUniformiv = 0;
	extern PFNGLGETVERTEXATTRIBDVPROC				glGetVertexAttribdv = 0;
	extern PFNGLGETVERTEXATTRIBFVPROC				glGetVertexAttribfv = 0;
	extern PFNGLGETVERTEXATTRIBIVPROC				glGetVertexAttribiv = 0;
	extern PFNGLGETVERTEXATTRIBPOINTERVPROC			glGetVertexAttribPointerv = 0;
	extern PFNGLISPROGRAMPROC						glIsProgram = 0;
	extern PFNGLISSHADERPROC						glIsShader = 0;
	extern PFNGLLINKPROGRAMPROC						glLinkProgram = 0;
	extern PFNGLSHADERSOURCEPROC					glShaderSource = 0;
	extern PFNGLUSEPROGRAMPROC						glUseProgram = 0;
	extern PFNGLUNIFORM1FPROC						glUniform1f = 0;
	extern PFNGLUNIFORM2FPROC						glUniform2f = 0;
	extern PFNGLUNIFORM3FPROC						glUniform3f = 0;
	extern PFNGLUNIFORM4FPROC						glUniform4f = 0;
	extern PFNGLUNIFORM1IPROC						glUniform1i = 0;
	extern PFNGLUNIFORM2IPROC						glUniform2i = 0;
	extern PFNGLUNIFORM3IPROC						glUniform3i = 0;
	extern PFNGLUNIFORM4IPROC						glUniform4i = 0;
	extern PFNGLUNIFORM1FVPROC						glUniform1fv = 0;
	extern PFNGLUNIFORM2FVPROC						glUniform2fv = 0;
	extern PFNGLUNIFORM3FVPROC						glUniform3fv = 0;
	extern PFNGLUNIFORM4FVPROC						glUniform4fv = 0;
	extern PFNGLUNIFORM1IVPROC						glUniform1iv = 0;
	extern PFNGLUNIFORM2IVPROC						glUniform2iv = 0;
	extern PFNGLUNIFORM3IVPROC						glUniform3iv = 0;
	extern PFNGLUNIFORM4IVPROC						glUniform4iv = 0;
	extern PFNGLUNIFORMMATRIX2FVPROC				glUniformMatrix2fv = 0;
	extern PFNGLUNIFORMMATRIX3FVPROC				glUniformMatrix3fv = 0;
	extern PFNGLUNIFORMMATRIX4FVPROC				glUniformMatrix4fv = 0;
	extern PFNGLVALIDATEPROGRAMPROC					glValidateProgram = 0;
	extern PFNGLVERTEXATTRIB1DPROC					glVertexAttrib1d = 0;
	extern PFNGLVERTEXATTRIB1DVPROC					glVertexAttrib1dv = 0;
	extern PFNGLVERTEXATTRIB1FPROC					glVertexAttrib1f = 0;
	extern PFNGLVERTEXATTRIB1FVPROC					glVertexAttrib1fv = 0;
	extern PFNGLVERTEXATTRIB1SPROC					glVertexAttrib1s = 0;
	extern PFNGLVERTEXATTRIB1SVPROC					glVertexAttrib1sv = 0;
	extern PFNGLVERTEXATTRIB2DPROC					glVertexAttrib2d = 0;
	extern PFNGLVERTEXATTRIB2DVPROC					glVertexAttrib2dv = 0;
	extern PFNGLVERTEXATTRIB2FPROC					glVertexAttrib2f = 0;
	extern PFNGLVERTEXATTRIB2FVPROC					glVertexAttrib2fv = 0;
	extern PFNGLVERTEXATTRIB2SPROC					glVertexAttrib2s = 0;
	extern PFNGLVERTEXATTRIB2SVPROC					glVertexAttrib2sv = 0;
	extern PFNGLVERTEXATTRIB3DPROC					glVertexAttrib3d = 0;
	extern PFNGLVERTEXATTRIB3DVPROC					glVertexAttrib3dv = 0;
	extern PFNGLVERTEXATTRIB3FPROC					glVertexAttrib3f = 0;
	extern PFNGLVERTEXATTRIB3FVPROC					glVertexAttrib3fv = 0;
	extern PFNGLVERTEXATTRIB3SPROC					glVertexAttrib3s = 0;
	extern PFNGLVERTEXATTRIB3SVPROC					glVertexAttrib3sv = 0;
	extern PFNGLVERTEXATTRIB4NBVPROC				glVertexAttrib4Nbv = 0;
	extern PFNGLVERTEXATTRIB4NIVPROC				glVertexAttrib4Niv = 0;
	extern PFNGLVERTEXATTRIB4NSVPROC				glVertexAttrib4Nsv = 0;
	extern PFNGLVERTEXATTRIB4NUBPROC				glVertexAttrib4Nub = 0;
	extern PFNGLVERTEXATTRIB4NUBVPROC				glVertexAttrib4Nubv = 0;
	extern PFNGLVERTEXATTRIB4NUIVPROC				glVertexAttrib4Nuiv = 0;
	extern PFNGLVERTEXATTRIB4NUSVPROC				glVertexAttrib4Nusv = 0;
	extern PFNGLVERTEXATTRIB4BVPROC					glVertexAttrib4bv = 0;
	extern PFNGLVERTEXATTRIB4DPROC					glVertexAttrib4d = 0;
	extern PFNGLVERTEXATTRIB4DVPROC					glVertexAttrib4dv = 0;
	extern PFNGLVERTEXATTRIB4FPROC					glVertexAttrib4f = 0;
	extern PFNGLVERTEXATTRIB4FVPROC					glVertexAttrib4fv = 0;
	extern PFNGLVERTEXATTRIB4IVPROC					glVertexAttrib4iv = 0;
	extern PFNGLVERTEXATTRIB4SPROC					glVertexAttrib4s = 0;
	extern PFNGLVERTEXATTRIB4SVPROC					glVertexAttrib4sv = 0;
	extern PFNGLVERTEXATTRIB4UBVPROC				glVertexAttrib4ubv = 0;
	extern PFNGLVERTEXATTRIB4UIVPROC				glVertexAttrib4uiv = 0;
	extern PFNGLVERTEXATTRIB4USVPROC				glVertexAttrib4usv = 0;
	extern PFNGLVERTEXATTRIBPOINTERPROC				glVertexAttribPointer = 0;

	// GL_VERSION_2_1
	extern PFNGLUNIFORMMATRIX2X3FVPROC				glUniformMatrix2x3fv = 0;
	extern PFNGLUNIFORMMATRIX3X2FVPROC				glUniformMatrix3x2fv = 0;
	extern PFNGLUNIFORMMATRIX2X4FVPROC				glUniformMatrix2x4fv = 0;
	extern PFNGLUNIFORMMATRIX4X2FVPROC				glUniformMatrix4x2fv = 0;
	extern PFNGLUNIFORMMATRIX3X4FVPROC				glUniformMatrix3x4fv = 0;
	extern PFNGLUNIFORMMATRIX4X3FVPROC				glUniformMatrix4x3fv = 0;

	// GL_VERSION_3_0
	extern PFNGLCOLORMASKIPROC								glColorMaski = 0;
	extern PFNGLGETBOOLEANI_VPROC							glGetBooleani_v = 0;
	extern PFNGLGETINTEGERI_VPROC							glGetIntegeri_v = 0;
	extern PFNGLENABLEIPROC									glEnablei = 0;
	extern PFNGLDISABLEIPROC								glDisablei = 0;
	extern PFNGLISENABLEDIPROC								glIsEnabledi = 0;
	extern PFNGLBEGINTRANSFORMFEEDBACKPROC					glBeginTransformFeedback = 0;
	extern PFNGLENDTRANSFORMFEEDBACKPROC					glEndTransformFeedback = 0;
	extern PFNGLBINDBUFFERRANGEPROC							glBindBufferRange = 0;
	extern PFNGLBINDBUFFERBASEPROC							glBindBufferBase = 0;
	extern PFNGLTRANSFORMFEEDBACKVARYINGSPROC				glTransformFeedbackVaryings = 0;
	extern PFNGLGETTRANSFORMFEEDBACKVARYINGPROC				glGetTransformFeedbackVarying = 0;
	extern PFNGLCLAMPCOLORPROC								glClampColor = 0;
	extern PFNGLBEGINCONDITIONALRENDERPROC					glBeginConditionalRender = 0;
	extern PFNGLENDCONDITIONALRENDERPROC					glEndConditionalRender = 0;
	extern PFNGLVERTEXATTRIBIPOINTERPROC					glVertexAttribIPointer = 0;
	extern PFNGLGETVERTEXATTRIBIIVPROC						glGetVertexAttribIiv = 0;
	extern PFNGLGETVERTEXATTRIBIUIVPROC						glGetVertexAttribIuiv = 0;
	extern PFNGLVERTEXATTRIBI1IPROC							glVertexAttribI1i = 0;
	extern PFNGLVERTEXATTRIBI2IPROC							glVertexAttribI2i = 0;
	extern PFNGLVERTEXATTRIBI3IPROC							glVertexAttribI3i = 0;
	extern PFNGLVERTEXATTRIBI4IPROC							glVertexAttribI4i = 0;
	extern PFNGLVERTEXATTRIBI1UIPROC						glVertexAttribI1ui = 0;
	extern PFNGLVERTEXATTRIBI2UIPROC						glVertexAttribI2ui = 0;
	extern PFNGLVERTEXATTRIBI3UIPROC						glVertexAttribI3ui = 0;
	extern PFNGLVERTEXATTRIBI4UIPROC						glVertexAttribI4ui = 0;
	extern PFNGLVERTEXATTRIBI1IVPROC						glVertexAttribI1iv = 0;
	extern PFNGLVERTEXATTRIBI2IVPROC						glVertexAttribI2iv = 0;
	extern PFNGLVERTEXATTRIBI3IVPROC						glVertexAttribI3iv = 0;
	extern PFNGLVERTEXATTRIBI4IVPROC						glVertexAttribI4iv = 0;
	extern PFNGLVERTEXATTRIBI1UIVPROC						glVertexAttribI1uiv = 0;
	extern PFNGLVERTEXATTRIBI2UIVPROC						glVertexAttribI2uiv = 0;
	extern PFNGLVERTEXATTRIBI3UIVPROC						glVertexAttribI3uiv = 0;
	extern PFNGLVERTEXATTRIBI4UIVPROC						glVertexAttribI4uiv = 0;
	extern PFNGLVERTEXATTRIBI4BVPROC						glVertexAttribI4bv = 0;
	extern PFNGLVERTEXATTRIBI4SVPROC						glVertexAttribI4sv = 0;
	extern PFNGLVERTEXATTRIBI4UBVPROC						glVertexAttribI4ubv = 0;
	extern PFNGLVERTEXATTRIBI4USVPROC						glVertexAttribI4usv = 0;
	extern PFNGLGETUNIFORMUIVPROC							glGetUniformuiv = 0;
	extern PFNGLBINDFRAGDATALOCATIONPROC					glBindFragDataLocation = 0;
	extern PFNGLGETFRAGDATALOCATIONPROC						glGetFragDataLocation = 0;
	extern PFNGLUNIFORM1UIPROC								glUniform1ui = 0;
	extern PFNGLUNIFORM2UIPROC								glUniform2ui = 0;
	extern PFNGLUNIFORM3UIPROC								glUniform3ui = 0;
	extern PFNGLUNIFORM4UIPROC								glUniform4ui = 0;
	extern PFNGLUNIFORM1UIVPROC								glUniform1uiv = 0;
	extern PFNGLUNIFORM2UIVPROC								glUniform2uiv = 0;
	extern PFNGLUNIFORM3UIVPROC								glUniform3uiv = 0;
	extern PFNGLUNIFORM4UIVPROC								glUniform4uiv = 0;
	extern PFNGLTEXPARAMETERIIVPROC							glTexParameterIiv = 0;
	extern PFNGLTEXPARAMETERIUIVPROC						glTexParameterIuiv = 0;
	extern PFNGLGETTEXPARAMETERIIVPROC						glGetTexParameterIiv = 0;
	extern PFNGLGETTEXPARAMETERIUIVPROC						glGetTexParameterIuiv = 0;
	extern PFNGLCLEARBUFFERIVPROC							glClearBufferiv = 0;
	extern PFNGLCLEARBUFFERUIVPROC							glClearBufferuiv = 0;
	extern PFNGLCLEARBUFFERFVPROC							glClearBufferfv = 0;
	extern PFNGLCLEARBUFFERFIPROC							glClearBufferfi = 0;
	extern PFNGLGETSTRINGIPROC								glGetStringi = 0;
	extern PFNGLISRENDERBUFFERPROC							glIsRenderbuffer = 0;
	extern PFNGLBINDRENDERBUFFERPROC						glBindRenderbuffer = 0;
	extern PFNGLDELETERENDERBUFFERSPROC						glDeleteRenderbuffers = 0;
	extern PFNGLGENRENDERBUFFERSPROC						glGenRenderbuffers = 0;
	extern PFNGLRENDERBUFFERSTORAGEPROC						glRenderbufferStorage = 0;
	extern PFNGLGETRENDERBUFFERPARAMETERIVPROC				glGetRenderbufferParameteriv = 0;
	extern PFNGLISFRAMEBUFFERPROC                      		glIsFramebuffer = 0;
	extern PFNGLBINDFRAMEBUFFERPROC                    		glBindFramebuffer = 0;
	extern PFNGLDELETEFRAMEBUFFERSPROC                 		glDeleteFramebuffers = 0;
	extern PFNGLGENFRAMEBUFFERSPROC                   		glGenFramebuffers = 0;
	extern PFNGLCHECKFRAMEBUFFERSTATUSPROC            		glCheckFramebufferStatus = 0;
	extern PFNGLFRAMEBUFFERTEXTURE1DPROC              		glFramebufferTexture1D = 0;
	extern PFNGLFRAMEBUFFERTEXTURE2DPROC              		glFramebufferTexture2D = 0;
	extern PFNGLFRAMEBUFFERTEXTURE3DPROC              		glFramebufferTexture3D = 0;
	extern PFNGLFRAMEBUFFERRENDERBUFFERPROC					glFramebufferRenderbuffer = 0;
	extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC		glGetFramebufferAttachmentParameteriv = 0;
	extern PFNGLGENERATEMIPMAPPROC							glGenerateMipmap = 0;
	extern PFNGLBLITFRAMEBUFFERPROC							glBlitFramebuffer = 0;
	extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC			glRenderbufferStorageMultisample = 0;
	extern PFNGLFRAMEBUFFERTEXTURELAYERPROC					glFramebufferTextureLayer = 0;
	extern PFNGLMAPBUFFERRANGEPROC							glMapBufferRange = 0;
	extern PFNGLFLUSHMAPPEDBUFFERRANGEPROC					glFlushMappedBufferRange = 0;
	extern PFNGLBINDVERTEXARRAYPROC							glBindVertexArray = 0;
	extern PFNGLDELETEVERTEXARRAYSPROC						glDeleteVertexArrays = 0;
	extern PFNGLGENVERTEXARRAYSPROC							glGenVertexArrays = 0;
	extern PFNGLISVERTEXARRAYPROC							glIsVertexArray = 0;

	// GL_VERSION_3_1
	extern PFNGLDRAWARRAYSINSTANCEDPROC						glDrawArraysInstanced = 0;
	extern PFNGLDRAWELEMENTSINSTANCEDPROC					glDrawElementsInstanced = 0;
	extern PFNGLTEXBUFFERPROC								glTexBuffer = 0;
	extern PFNGLPRIMITIVERESTARTINDEXPROC					glPrimitiveRestartIndex = 0;
	extern PFNGLCOPYBUFFERSUBDATAPROC						glCopyBufferSubData = 0;
	extern PFNGLGETUNIFORMINDICESPROC						glGetUniformIndices = 0;
	extern PFNGLGETACTIVEUNIFORMSIVPROC						glGetActiveUniformsiv = 0;
	extern PFNGLGETACTIVEUNIFORMNAMEPROC					glGetActiveUniformName = 0;
	extern PFNGLGETUNIFORMBLOCKINDEXPROC					glGetUniformBlockIndex = 0;
	extern PFNGLGETACTIVEUNIFORMBLOCKIVPROC					glGetActiveUniformBlockiv = 0;
	extern PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC				glGetActiveUniformBlockName = 0;
	extern PFNGLUNIFORMBLOCKBINDINGPROC						glUniformBlockBinding = 0;

	// GL_VERSION_3_2
	extern PFNGLDRAWELEMENTSBASEVERTEXPROC					glDrawElementsBaseVertex = 0;
	extern PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC				glDrawRangeElementsBaseVertex = 0;
	extern PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC			glDrawElementsInstancedBaseVertex = 0;
	extern PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC				glMultiDrawElementsBaseVertex = 0;
	extern PFNGLPROVOKINGVERTEXPROC							glProvokingVertex = 0;
	extern PFNGLFENCESYNCPROC                    			glFenceSync = 0;
	extern PFNGLISSYNCPROC                       			glIsSync = 0;
	extern PFNGLDELETESYNCPROC                   			glDeleteSync = 0;
	extern PFNGLCLIENTWAITSYNCPROC               			glClientWaitSync = 0;
	extern PFNGLWAITSYNCPROC                     			glWaitSync = 0;
	extern PFNGLGETINTEGER64VPROC                			glGetInteger64v = 0;
	extern PFNGLGETSYNCIVPROC                    			glGetSynciv = 0;
	extern PFNGLGETINTEGER64I_VPROC              			glGetInteger64i_v = 0;
	extern PFNGLGETBUFFERPARAMETERI64VPROC       			glGetBufferParameteri64v = 0;
	extern PFNGLFRAMEBUFFERTEXTUREPROC           			glFramebufferTexture = 0;
	extern PFNGLTEXIMAGE2DMULTISAMPLEPROC        			glTexImage2DMultisample = 0;
	extern PFNGLTEXIMAGE3DMULTISAMPLEPROC        			glTexImage3DMultisample = 0;
	extern PFNGLGETMULTISAMPLEFVPROC             			glGetMultisamplefv = 0;
	extern PFNGLSAMPLEMASKIPROC                  			glSampleMaski = 0;

	// GL_VERSION_3_3
	extern PFNGLBINDFRAGDATALOCATIONINDEXEDPROC				glBindFragDataLocationIndexed = 0;
	extern PFNGLGETFRAGDATAINDEXPROC						glGetFragDataIndex = 0;
	extern PFNGLGENSAMPLERSPROC								glGenSamplers = 0;
	extern PFNGLDELETESAMPLERSPROC							glDeleteSamplers = 0;
	extern PFNGLISSAMPLERPROC								glIsSampler = 0;
	extern PFNGLBINDSAMPLERPROC								glBindSampler = 0;
	extern PFNGLSAMPLERPARAMETERIPROC						glSamplerParameteri = 0;
	extern PFNGLSAMPLERPARAMETERIVPROC						glSamplerParameteriv = 0;
	extern PFNGLSAMPLERPARAMETERFPROC						glSamplerParameterf = 0;
	extern PFNGLSAMPLERPARAMETERFVPROC						glSamplerParameterfv = 0;
	extern PFNGLSAMPLERPARAMETERIIVPROC						glSamplerParameterIiv = 0;
	extern PFNGLSAMPLERPARAMETERIUIVPROC					glSamplerParameterIuiv = 0;
	extern PFNGLGETSAMPLERPARAMETERIVPROC					glGetSamplerParameteriv = 0;
	extern PFNGLGETSAMPLERPARAMETERIIVPROC					glGetSamplerParameterIiv = 0;
	extern PFNGLGETSAMPLERPARAMETERFVPROC					glGetSamplerParameterfv = 0;
	extern PFNGLGETSAMPLERPARAMETERIUIVPROC					glGetSamplerParameterIuiv = 0;
	extern PFNGLQUERYCOUNTERPROC							glQueryCounter = 0;
	extern PFNGLGETQUERYOBJECTI64VPROC						glGetQueryObjecti64v = 0;
	extern PFNGLGETQUERYOBJECTUI64VPROC						glGetQueryObjectui64v = 0;
	extern PFNGLVERTEXATTRIBDIVISORPROC						glVertexAttribDivisor = 0;
	extern PFNGLVERTEXATTRIBP1UIPROC						glVertexAttribP1ui = 0;
	extern PFNGLVERTEXATTRIBP1UIVPROC						glVertexAttribP1uiv = 0;
	extern PFNGLVERTEXATTRIBP2UIPROC						glVertexAttribP2ui = 0;
	extern PFNGLVERTEXATTRIBP2UIVPROC						glVertexAttribP2uiv = 0;
	extern PFNGLVERTEXATTRIBP3UIPROC						glVertexAttribP3ui = 0;
	extern PFNGLVERTEXATTRIBP3UIVPROC						glVertexAttribP3uiv = 0;
	extern PFNGLVERTEXATTRIBP4UIPROC						glVertexAttribP4ui = 0;
	extern PFNGLVERTEXATTRIBP4UIVPROC						glVertexAttribP4uiv = 0;

	// GL_VERSION_4_0
	extern PFNGLMINSAMPLESHADINGPROC						glMinSampleShading = 0;
	extern PFNGLBLENDEQUATIONIPROC							glBlendEquationi = 0;
	extern PFNGLBLENDEQUATIONSEPARATEIPROC					glBlendEquationSeparatei = 0;
	extern PFNGLBLENDFUNCIPROC								glBlendFunci = 0;
	extern PFNGLBLENDFUNCSEPARATEIPROC						glBlendFuncSeparatei = 0;
	extern PFNGLDRAWARRAYSINDIRECTPROC						glDrawArraysIndirect = 0;
	extern PFNGLDRAWELEMENTSINDIRECTPROC					glDrawElementsIndirect = 0;
	extern PFNGLUNIFORM1DPROC								glUniform1d = 0;
	extern PFNGLUNIFORM2DPROC								glUniform2d = 0;
	extern PFNGLUNIFORM3DPROC								glUniform3d = 0;
	extern PFNGLUNIFORM4DPROC								glUniform4d = 0;
	extern PFNGLUNIFORM1DVPROC								glUniform1dv = 0;
	extern PFNGLUNIFORM2DVPROC								glUniform2dv = 0;
	extern PFNGLUNIFORM3DVPROC								glUniform3dv = 0;
	extern PFNGLUNIFORM4DVPROC								glUniform4dv = 0;
	extern PFNGLUNIFORMMATRIX2DVPROC						glUniformMatrix2dv = 0;
	extern PFNGLUNIFORMMATRIX3DVPROC						glUniformMatrix3dv = 0;
	extern PFNGLUNIFORMMATRIX4DVPROC						glUniformMatrix4dv = 0;
	extern PFNGLUNIFORMMATRIX2X3DVPROC						glUniformMatrix2x3dv = 0;
	extern PFNGLUNIFORMMATRIX2X4DVPROC						glUniformMatrix2x4dv = 0;
	extern PFNGLUNIFORMMATRIX3X2DVPROC						glUniformMatrix3x2dv = 0;
	extern PFNGLUNIFORMMATRIX3X4DVPROC						glUniformMatrix3x4dv = 0;
	extern PFNGLUNIFORMMATRIX4X2DVPROC						glUniformMatrix4x2dv = 0;
	extern PFNGLUNIFORMMATRIX4X3DVPROC						glUniformMatrix4x3dv = 0;
	extern PFNGLGETUNIFORMDVPROC							glGetUniformdv = 0;
	extern PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC			glGetSubroutineUniformLocation = 0;
	extern PFNGLGETSUBROUTINEINDEXPROC						glGetSubroutineIndex = 0;
	extern PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC			glGetActiveSubroutineUniformiv = 0;
	extern PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC			glGetActiveSubroutineUniformName = 0;
	extern PFNGLGETACTIVESUBROUTINENAMEPROC					glGetActiveSubroutineName = 0;
	extern PFNGLUNIFORMSUBROUTINESUIVPROC					glUniformSubroutinesuiv = 0;
	extern PFNGLGETUNIFORMSUBROUTINEUIVPROC					glGetUniformSubroutineuiv = 0;
	extern PFNGLGETPROGRAMSTAGEIVPROC						glGetProgramStageiv = 0;
	extern PFNGLPATCHPARAMETERIPROC							glPatchParameteri = 0;
	extern PFNGLPATCHPARAMETERFVPROC						glPatchParameterfv = 0;
	extern PFNGLBINDTRANSFORMFEEDBACKPROC					glBindTransformFeedback = 0;
	extern PFNGLDELETETRANSFORMFEEDBACKSPROC				glDeleteTransformFeedbacks = 0;
	extern PFNGLGENTRANSFORMFEEDBACKSPROC					glGenTransformFeedbacks = 0;
	extern PFNGLISTRANSFORMFEEDBACKPROC						glIsTransformFeedback = 0;
	extern PFNGLPAUSETRANSFORMFEEDBACKPROC					glPauseTransformFeedback = 0;
	extern PFNGLRESUMETRANSFORMFEEDBACKPROC					glResumeTransformFeedback = 0;
	extern PFNGLDRAWTRANSFORMFEEDBACKPROC					glDrawTransformFeedback = 0;
	extern PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC				glDrawTransformFeedbackStream = 0;
	extern PFNGLBEGINQUERYINDEXEDPROC						glBeginQueryIndexed = 0;
	extern PFNGLENDQUERYINDEXEDPROC							glEndQueryIndexed = 0;
	extern PFNGLGETQUERYINDEXEDIVPROC						glGetQueryIndexediv = 0;

	// GL_VERSION_4_1
	extern PFNGLRELEASESHADERCOMPILERPROC					glReleaseShaderCompiler = 0;
	extern PFNGLSHADERBINARYPROC							glShaderBinary = 0;
	extern PFNGLGETSHADERPRECISIONFORMATPROC				glGetShaderPrecisionFormat = 0;
	extern PFNGLDEPTHRANGEFPROC								glDepthRangef = 0;
	extern PFNGLCLEARDEPTHFPROC								glClearDepthf = 0;
	extern PFNGLGETPROGRAMBINARYPROC						glGetProgramBinary = 0;
	extern PFNGLPROGRAMBINARYPROC							glProgramBinary = 0;
	extern PFNGLPROGRAMPARAMETERIPROC						glProgramParameteri = 0;
	extern PFNGLUSEPROGRAMSTAGESPROC						glUseProgramStages = 0;
	extern PFNGLACTIVESHADERPROGRAMPROC						glActiveShaderProgram = 0;
	extern PFNGLCREATESHADERPROGRAMVPROC					glCreateShaderProgramv = 0;
	extern PFNGLBINDPROGRAMPIPELINEPROC						glBindProgramPipeline = 0;
	extern PFNGLDELETEPROGRAMPIPELINESPROC					glDeleteProgramPipelines = 0;
	extern PFNGLGENPROGRAMPIPELINESPROC						glGenProgramPipelines = 0;
	extern PFNGLISPROGRAMPIPELINEPROC						glIsProgramPipeline = 0;
	extern PFNGLGETPROGRAMPIPELINEIVPROC					glGetProgramPipelineiv = 0;
	extern PFNGLPROGRAMUNIFORM1IPROC						glProgramUniform1i = 0;
	extern PFNGLPROGRAMUNIFORM1IVPROC						glProgramUniform1iv = 0;
	extern PFNGLPROGRAMUNIFORM1FPROC						glProgramUniform1f = 0;
	extern PFNGLPROGRAMUNIFORM1FVPROC						glProgramUniform1fv = 0;
	extern PFNGLPROGRAMUNIFORM1DPROC						glProgramUniform1d = 0;
	extern PFNGLPROGRAMUNIFORM1DVPROC						glProgramUniform1dv = 0;
	extern PFNGLPROGRAMUNIFORM1UIPROC						glProgramUniform1ui = 0;
	extern PFNGLPROGRAMUNIFORM1UIVPROC						glProgramUniform1uiv = 0;
	extern PFNGLPROGRAMUNIFORM2IPROC						glProgramUniform2i = 0;
	extern PFNGLPROGRAMUNIFORM2IVPROC						glProgramUniform2iv = 0;
	extern PFNGLPROGRAMUNIFORM2FPROC						glProgramUniform2f = 0;
	extern PFNGLPROGRAMUNIFORM2FVPROC						glProgramUniform2fv = 0;
	extern PFNGLPROGRAMUNIFORM2DPROC						glProgramUniform2d = 0;
	extern PFNGLPROGRAMUNIFORM2DVPROC						glProgramUniform2dv = 0;
	extern PFNGLPROGRAMUNIFORM2UIPROC						glProgramUniform2ui = 0;
	extern PFNGLPROGRAMUNIFORM2UIVPROC						glProgramUniform2uiv = 0;
	extern PFNGLPROGRAMUNIFORM3IPROC						glProgramUniform3i = 0;
	extern PFNGLPROGRAMUNIFORM3IVPROC						glProgramUniform3iv = 0;
	extern PFNGLPROGRAMUNIFORM3FPROC						glProgramUniform3f = 0;
	extern PFNGLPROGRAMUNIFORM3FVPROC						glProgramUniform3fv = 0;
	extern PFNGLPROGRAMUNIFORM3DPROC						glProgramUniform3d = 0;
	extern PFNGLPROGRAMUNIFORM3DVPROC						glProgramUniform3dv = 0;
	extern PFNGLPROGRAMUNIFORM3UIPROC						glProgramUniform3ui = 0;
	extern PFNGLPROGRAMUNIFORM3UIVPROC						glProgramUniform3uiv = 0;
	extern PFNGLPROGRAMUNIFORM4IPROC						glProgramUniform4i = 0;
	extern PFNGLPROGRAMUNIFORM4IVPROC						glProgramUniform4iv = 0;
	extern PFNGLPROGRAMUNIFORM4FPROC						glProgramUniform4f = 0;
	extern PFNGLPROGRAMUNIFORM4FVPROC						glProgramUniform4fv = 0;
	extern PFNGLPROGRAMUNIFORM4DPROC						glProgramUniform4d = 0;
	extern PFNGLPROGRAMUNIFORM4DVPROC						glProgramUniform4dv = 0;
	extern PFNGLPROGRAMUNIFORM4UIPROC						glProgramUniform4ui = 0;
	extern PFNGLPROGRAMUNIFORM4UIVPROC						glProgramUniform4uiv = 0;
	extern PFNGLPROGRAMUNIFORMMATRIX2FVPROC					glProgramUniformMatrix2fv = 0;
	extern PFNGLPROGRAMUNIFORMMATRIX3FVPROC					glProgramUniformMatrix3fv = 0;
	extern PFNGLPROGRAMUNIFORMMATRIX4FVPROC					glProgramUniformMatrix4fv = 0;
	extern PFNGLPROGRAMUNIFORMMATRIX2DVPROC					glProgramUniformMatrix2dv = 0;
	extern PFNGLPROGRAMUNIFORMMATRIX3DVPROC					glProgramUniformMatrix3dv = 0;
	extern PFNGLPROGRAMUNIFORMMATRIX4DVPROC					glProgramUniformMatrix4dv = 0;
	extern PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC				glProgramUniformMatrix2x3fv = 0;
	extern PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC				glProgramUniformMatrix3x2fv = 0;
	extern PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC				glProgramUniformMatrix2x4fv = 0;
	extern PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC				glProgramUniformMatrix4x2fv = 0;
	extern PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC				glProgramUniformMatrix3x4fv = 0;
	extern PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC				glProgramUniformMatrix4x3fv = 0;
	extern PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC				glProgramUniformMatrix2x3dv = 0;
	extern PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC				glProgramUniformMatrix3x2dv = 0;
	extern PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC				glProgramUniformMatrix2x4dv = 0;
	extern PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC				glProgramUniformMatrix4x2dv = 0;
	extern PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC				glProgramUniformMatrix3x4dv = 0;
	extern PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC				glProgramUniformMatrix4x3dv = 0;
	extern PFNGLVALIDATEPROGRAMPIPELINEPROC					glValidateProgramPipeline = 0;
	extern PFNGLGETPROGRAMPIPELINEINFOLOGPROC				glGetProgramPipelineInfoLog = 0;
	extern PFNGLVERTEXATTRIBL1DPROC							glVertexAttribL1d = 0;
	extern PFNGLVERTEXATTRIBL2DPROC							glVertexAttribL2d = 0;
	extern PFNGLVERTEXATTRIBL3DPROC							glVertexAttribL3d = 0;
	extern PFNGLVERTEXATTRIBL4DPROC							glVertexAttribL4d = 0;
	extern PFNGLVERTEXATTRIBL1DVPROC						glVertexAttribL1dv = 0;
	extern PFNGLVERTEXATTRIBL2DVPROC						glVertexAttribL2dv = 0;
	extern PFNGLVERTEXATTRIBL3DVPROC						glVertexAttribL3dv = 0;
	extern PFNGLVERTEXATTRIBL4DVPROC						glVertexAttribL4dv = 0;
	extern PFNGLVERTEXATTRIBLPOINTERPROC					glVertexAttribLPointer = 0;
	extern PFNGLGETVERTEXATTRIBLDVPROC						glGetVertexAttribLdv = 0;
	extern PFNGLVIEWPORTARRAYVPROC							glViewportArrayv = 0;
	extern PFNGLVIEWPORTINDEXEDFPROC						glViewportIndexedf = 0;
	extern PFNGLVIEWPORTINDEXEDFVPROC						glViewportIndexedfv = 0;
	extern PFNGLSCISSORARRAYVPROC							glScissorArrayv = 0;
	extern PFNGLSCISSORINDEXEDPROC							glScissorIndexed = 0;
	extern PFNGLSCISSORINDEXEDVPROC							glScissorIndexedv = 0;
	extern PFNGLDEPTHRANGEARRAYVPROC						glDepthRangeArrayv = 0;
	extern PFNGLDEPTHRANGEINDEXEDPROC						glDepthRangeIndexed = 0;
	extern PFNGLGETFLOATI_VPROC								glGetFloati_v = 0;
	extern PFNGLGETDOUBLEI_VPROC							glGetDoublei_v = 0;

	// GL_VERSION_4_2
	extern PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC					glDrawArraysInstancedBaseInstance = 0;
	extern PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC				glDrawElementsInstancedBaseInstance = 0;
	extern PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC		glDrawElementsInstancedBaseVertexBaseInstance = 0;
	extern PFNGLGETINTERNALFORMATIVPROC								glGetInternalformativ = 0;
	extern PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC					glGetActiveAtomicCounterBufferiv = 0;
	extern PFNGLBINDIMAGETEXTUREPROC                         		glBindImageTexture = 0;
	extern PFNGLMEMORYBARRIERPROC                            		glMemoryBarrier = 0;
	extern PFNGLTEXSTORAGE1DPROC									glTexStorage1D = 0;
	extern PFNGLTEXSTORAGE2DPROC                             		glTexStorage2D = 0;
	extern PFNGLTEXSTORAGE3DPROC                             		glTexStorage3D = 0;
	extern PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC					glDrawTransformFeedbackInstanced = 0;
	extern PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC			glDrawTransformFeedbackStreamInstanced = 0;

	// GL_VERSION_4_3
	extern PFNGLCLEARBUFFERDATAPROC							glClearBufferData = 0;
	extern PFNGLCLEARBUFFERSUBDATAPROC						glClearBufferSubData = 0;
	extern PFNGLDISPATCHCOMPUTEPROC							glDispatchCompute = 0;
	extern PFNGLDISPATCHCOMPUTEINDIRECTPROC					glDispatchComputeIndirect = 0;
	extern PFNGLCOPYIMAGESUBDATAPROC						glCopyImageSubData = 0;
	extern PFNGLFRAMEBUFFERPARAMETERIPROC					glFramebufferParameteri = 0;
	extern PFNGLGETFRAMEBUFFERPARAMETERIVPROC				glGetFramebufferParameteriv = 0;
	extern PFNGLGETINTERNALFORMATI64VPROC					glGetInternalformati64v = 0;
	extern PFNGLINVALIDATETEXSUBIMAGEPROC					glInvalidateTexSubImage = 0;
	extern PFNGLINVALIDATETEXIMAGEPROC						glInvalidateTexImage = 0;
	extern PFNGLINVALIDATEBUFFERSUBDATAPROC					glInvalidateBufferSubData = 0;
	extern PFNGLINVALIDATEBUFFERDATAPROC					glInvalidateBufferData = 0;
	extern PFNGLINVALIDATEFRAMEBUFFERPROC					glInvalidateFramebuffer = 0;
	extern PFNGLINVALIDATESUBFRAMEBUFFERPROC				glInvalidateSubFramebuffer = 0;
	extern PFNGLMULTIDRAWARRAYSINDIRECTPROC					glMultiDrawArraysIndirect = 0;
	extern PFNGLMULTIDRAWELEMENTSINDIRECTPROC				glMultiDrawElementsIndirect = 0;
	extern PFNGLGETPROGRAMINTERFACEIVPROC					glGetProgramInterfaceiv = 0;
	extern PFNGLGETPROGRAMRESOURCEINDEXPROC					glGetProgramResourceIndex = 0;
	extern PFNGLGETPROGRAMRESOURCENAMEPROC					glGetProgramResourceName = 0;
	extern PFNGLGETPROGRAMRESOURCEIVPROC					glGetProgramResourceiv = 0;
	extern PFNGLGETPROGRAMRESOURCELOCATIONPROC				glGetProgramResourceLocation = 0;
	extern PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC			glGetProgramResourceLocationIndex = 0;
	extern PFNGLSHADERSTORAGEBLOCKBINDINGPROC				glShaderStorageBlockBinding = 0;
	extern PFNGLTEXBUFFERRANGEPROC							glTexBufferRange = 0;
	extern PFNGLTEXSTORAGE2DMULTISAMPLEPROC					glTexStorage2DMultisample = 0;
	extern PFNGLTEXSTORAGE3DMULTISAMPLEPROC					glTexStorage3DMultisample = 0;
	extern PFNGLTEXTUREVIEWPROC								glTextureView = 0;
	extern PFNGLBINDVERTEXBUFFERPROC						glBindVertexBuffer = 0;
	extern PFNGLVERTEXATTRIBFORMATPROC						glVertexAttribFormat = 0;
	extern PFNGLVERTEXATTRIBIFORMATPROC						glVertexAttribIFormat = 0;
	extern PFNGLVERTEXATTRIBLFORMATPROC						glVertexAttribLFormat = 0;
	extern PFNGLVERTEXATTRIBBINDINGPROC						glVertexAttribBinding = 0;
	extern PFNGLVERTEXBINDINGDIVISORPROC					glVertexBindingDivisor = 0;
	extern PFNGLDEBUGMESSAGECONTROLPROC						glDebugMessageControl = 0;
	extern PFNGLDEBUGMESSAGEINSERTPROC						glDebugMessageInsert = 0;
	extern PFNGLDEBUGMESSAGECALLBACKPROC					glDebugMessageCallback = 0;
	extern PFNGLGETDEBUGMESSAGELOGPROC						glGetDebugMessageLog = 0;
	extern PFNGLPUSHDEBUGGROUPPROC							glPushDebugGroup = 0;
	extern PFNGLPOPDEBUGGROUPPROC							glPopDebugGroup = 0;
	extern PFNGLOBJECTLABELPROC								glObjectLabel = 0;
	extern PFNGLGETOBJECTLABELPROC							glGetObjectLabel = 0;
	extern PFNGLOBJECTPTRLABELPROC							glObjectPtrLabel = 0;
	extern PFNGLGETOBJECTPTRLABELPROC						glGetObjectPtrLabel = 0;

	// GL_VERSION_4_4
	extern PFNGLBUFFERSTORAGEPROC							glBufferStorage = 0;
	extern PFNGLCLEARTEXIMAGEPROC							glClearTexImage = 0;
	extern PFNGLCLEARTEXSUBIMAGEPROC						glClearTexSubImage = 0;
	extern PFNGLBINDBUFFERSBASEPROC							glBindBuffersBase = 0;
	extern PFNGLBINDBUFFERSRANGEPROC						glBindBuffersRange = 0;
	extern PFNGLBINDTEXTURESPROC							glBindTextures = 0;
	extern PFNGLBINDSAMPLERSPROC							glBindSamplers = 0;
	extern PFNGLBINDIMAGETEXTURESPROC						glBindImageTextures = 0;
	extern PFNGLBINDVERTEXBUFFERSPROC						glBindVertexBuffers = 0;

	// GL_VERSION_4_5
	extern PFNGLCLIPCONTROLPROC										glClipControl = 0;
	extern PFNGLCREATETRANSFORMFEEDBACKSPROC                		glCreateTransformFeedbacks = 0;
	extern PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC             		glTransformFeedbackBufferBase = 0;
	extern PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC            		glTransformFeedbackBufferRange = 0;
	extern PFNGLGETTRANSFORMFEEDBACKIVPROC                  		glGetTransformFeedbackiv = 0;
	extern PFNGLGETTRANSFORMFEEDBACKI_VPROC                 		glGetTransformFeedbacki_v = 0;
	extern PFNGLGETTRANSFORMFEEDBACKI64_VPROC               		glGetTransformFeedbacki64_v = 0;
	extern PFNGLCREATEBUFFERSPROC                           		glCreateBuffers = 0;
	extern PFNGLNAMEDBUFFERSTORAGEPROC                      		glNamedBufferStorage = 0;
	extern PFNGLNAMEDBUFFERDATAPROC                         		glNamedBufferData = 0;
	extern PFNGLNAMEDBUFFERSUBDATAPROC                      		glNamedBufferSubData = 0;
	extern PFNGLCOPYNAMEDBUFFERSUBDATAPROC                  		glCopyNamedBufferSubData = 0;
	extern PFNGLCLEARNAMEDBUFFERDATAPROC                    		glClearNamedBufferData = 0;
	extern PFNGLCLEARNAMEDBUFFERSUBDATAPROC                 		glClearNamedBufferSubData = 0;
	extern PFNGLMAPNAMEDBUFFERPROC                          		glMapNamedBuffer = 0;
	extern PFNGLMAPNAMEDBUFFERRANGEPROC                     		glMapNamedBufferRange = 0;
	extern PFNGLUNMAPNAMEDBUFFERPROC								glUnmapNamedBuffer = 0;
	extern PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC						glFlushMappedNamedBufferRange = 0;
	extern PFNGLGETNAMEDBUFFERPARAMETERIVPROC               		glGetNamedBufferParameteriv = 0;
	extern PFNGLGETNAMEDBUFFERPARAMETERI64VPROC             		glGetNamedBufferParameteri64v = 0;
	extern PFNGLGETNAMEDBUFFERPOINTERVPROC                  		glGetNamedBufferPointerv = 0;
	extern PFNGLGETNAMEDBUFFERSUBDATAPROC                   		glGetNamedBufferSubData = 0;
	extern PFNGLCREATEFRAMEBUFFERSPROC                      		glCreateFramebuffers = 0;
	extern PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC            		glNamedFramebufferRenderbuffer = 0;
	extern PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC              		glNamedFramebufferParameteri = 0;
	extern PFNGLNAMEDFRAMEBUFFERTEXTUREPROC                 		glNamedFramebufferTexture = 0;
	extern PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC            		glNamedFramebufferTextureLayer = 0;
	extern PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC              		glNamedFramebufferDrawBuffer = 0;
	extern PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC             		glNamedFramebufferDrawBuffers = 0;
	extern PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC              		glNamedFramebufferReadBuffer = 0;
	extern PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC          		glInvalidateNamedFramebufferData = 0;
	extern PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC       		glInvalidateNamedFramebufferSubData = 0;
	extern PFNGLCLEARNAMEDFRAMEBUFFERIVPROC                 		glClearNamedFramebufferiv = 0;
	extern PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC                		glClearNamedFramebufferuiv = 0;
	extern PFNGLCLEARNAMEDFRAMEBUFFERFVPROC                 		glClearNamedFramebufferfv = 0;
	extern PFNGLCLEARNAMEDFRAMEBUFFERFIPROC                 		glClearNamedFramebufferfi = 0;
	extern PFNGLBLITNAMEDFRAMEBUFFERPROC                    		glBlitNamedFramebuffer = 0;
	extern PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC            			glCheckNamedFramebufferStatus = 0;
	extern PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC          		glGetNamedFramebufferParameteriv = 0;
	extern PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC		glGetNamedFramebufferAttachmentParameteriv = 0;
	extern PFNGLCREATERENDERBUFFERSPROC                       		glCreateRenderbuffers = 0;
	extern PFNGLNAMEDRENDERBUFFERSTORAGEPROC                  		glNamedRenderbufferStorage = 0;
	extern PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC       		glNamedRenderbufferStorageMultisample = 0;
	extern PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC           		glGetNamedRenderbufferParameteriv = 0;
	extern PFNGLCREATETEXTURESPROC                            		glCreateTextures = 0;
	extern PFNGLTEXTUREBUFFERPROC                             		glTextureBuffer = 0;
	extern PFNGLTEXTUREBUFFERRANGEPROC                        		glTextureBufferRange = 0;
	extern PFNGLTEXTURESTORAGE1DPROC                          		glTextureStorage1D = 0;
	extern PFNGLTEXTURESTORAGE2DPROC                          		glTextureStorage2D = 0;
	extern PFNGLTEXTURESTORAGE3DPROC                          		glTextureStorage3D = 0;
	extern PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC               		glTextureStorage2DMultisample = 0;
	extern PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC               		glTextureStorage3DMultisample = 0;
	extern PFNGLTEXTURESUBIMAGE1DPROC                         		glTextureSubImage1D = 0;
	extern PFNGLTEXTURESUBIMAGE2DPROC                         		glTextureSubImage2D = 0;
	extern PFNGLTEXTURESUBIMAGE3DPROC                         		glTextureSubImage3D = 0;
	extern PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC               		glCompressedTextureSubImage1D = 0;
	extern PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC               		glCompressedTextureSubImage2D = 0;
	extern PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC               		glCompressedTextureSubImage3D = 0;
	extern PFNGLCOPYTEXTURESUBIMAGE1DPROC                     		glCopyTextureSubImage1D = 0;
	extern PFNGLCOPYTEXTURESUBIMAGE2DPROC                     		glCopyTextureSubImage2D = 0;
	extern PFNGLCOPYTEXTURESUBIMAGE3DPROC                     		glCopyTextureSubImage3D = 0;
	extern PFNGLTEXTUREPARAMETERFPROC                         		glTextureParameterf = 0;
	extern PFNGLTEXTUREPARAMETERFVPROC                        		glTextureParameterfv = 0;
	extern PFNGLTEXTUREPARAMETERIPROC                         		glTextureParameteri = 0;
	extern PFNGLTEXTUREPARAMETERIIVPROC                       		glTextureParameterIiv = 0;
	extern PFNGLTEXTUREPARAMETERIUIVPROC                      		glTextureParameterIuiv = 0;
	extern PFNGLTEXTUREPARAMETERIVPROC                        		glTextureParameteriv = 0;
	extern PFNGLGENERATETEXTUREMIPMAPPROC							glGenerateTextureMipmap = 0;
	extern PFNGLBINDTEXTUREUNITPROC									glBindTextureUnit = 0;
	extern PFNGLGETTEXTUREIMAGEPROC                           		glGetTextureImage = 0;
	extern PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC                 		glGetCompressedTextureImage = 0;
	extern PFNGLGETTEXTURELEVELPARAMETERFVPROC                		glGetTextureLevelParameterfv = 0;
	extern PFNGLGETTEXTURELEVELPARAMETERIVPROC                		glGetTextureLevelParameteriv = 0;
	extern PFNGLGETTEXTUREPARAMETERFVPROC                     		glGetTextureParameterfv = 0;
	extern PFNGLGETTEXTUREPARAMETERIIVPROC                    		glGetTextureParameterIiv = 0;
	extern PFNGLGETTEXTUREPARAMETERIUIVPROC                   		glGetTextureParameterIuiv = 0;
	extern PFNGLGETTEXTUREPARAMETERIVPROC							glGetTextureParameteriv = 0;
	extern PFNGLCREATEVERTEXARRAYSPROC								glCreateVertexArrays = 0;
	extern PFNGLDISABLEVERTEXARRAYATTRIBPROC						glDisableVertexArrayAttrib = 0;
	extern PFNGLENABLEVERTEXARRAYATTRIBPROC							glEnableVertexArrayAttrib = 0;
	extern PFNGLVERTEXARRAYELEMENTBUFFERPROC						glVertexArrayElementBuffer = 0;
	extern PFNGLVERTEXARRAYVERTEXBUFFERPROC							glVertexArrayVertexBuffer = 0;
	extern PFNGLVERTEXARRAYVERTEXBUFFERSPROC						glVertexArrayVertexBuffers = 0;
	extern PFNGLVERTEXARRAYATTRIBBINDINGPROC						glVertexArrayAttribBinding = 0;
	extern PFNGLVERTEXARRAYATTRIBFORMATPROC							glVertexArrayAttribFormat = 0;
	extern PFNGLVERTEXARRAYATTRIBIFORMATPROC						glVertexArrayAttribIFormat = 0;
	extern PFNGLVERTEXARRAYATTRIBLFORMATPROC						glVertexArrayAttribLFormat = 0;
	extern PFNGLVERTEXARRAYBINDINGDIVISORPROC						glVertexArrayBindingDivisor = 0;
	extern PFNGLGETVERTEXARRAYIVPROC								glGetVertexArrayiv = 0;
	extern PFNGLGETVERTEXARRAYINDEXEDIVPROC							glGetVertexArrayIndexediv = 0;
	extern PFNGLGETVERTEXARRAYINDEXED64IVPROC						glGetVertexArrayIndexed64iv = 0;
	extern PFNGLCREATESAMPLERSPROC									glCreateSamplers = 0;
	extern PFNGLCREATEPROGRAMPIPELINESPROC							glCreateProgramPipelines = 0;
	extern PFNGLCREATEQUERIESPROC									glCreateQueries = 0;
	extern PFNGLGETQUERYBUFFEROBJECTI64VPROC						glGetQueryBufferObjecti64v = 0;
	extern PFNGLGETQUERYBUFFEROBJECTIVPROC							glGetQueryBufferObjectiv = 0;
	extern PFNGLGETQUERYBUFFEROBJECTUI64VPROC						glGetQueryBufferObjectui64v = 0;
	extern PFNGLGETQUERYBUFFEROBJECTUIVPROC							glGetQueryBufferObjectuiv = 0;
	extern PFNGLMEMORYBARRIERBYREGIONPROC							glMemoryBarrierByRegion = 0;
	extern PFNGLGETTEXTURESUBIMAGEPROC								glGetTextureSubImage = 0;
	extern PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC					glGetCompressedTextureSubImage = 0;
	extern PFNGLGETGRAPHICSRESETSTATUSPROC                     		glGetGraphicsResetStatus = 0;
	extern PFNGLGETNCOMPRESSEDTEXIMAGEPROC							glGetnCompressedTexImage = 0;
	extern PFNGLGETNTEXIMAGEPROC									glGetnTexImage = 0;
	extern PFNGLGETNUNIFORMDVPROC									glGetnUniformdv = 0;
	extern PFNGLGETNUNIFORMFVPROC									glGetnUniformfv = 0;
	extern PFNGLGETNUNIFORMIVPROC									glGetnUniformiv = 0;
	extern PFNGLGETNUNIFORMUIVPROC									glGetnUniformuiv = 0;
	extern PFNGLREADNPIXELSPROC										glReadnPixels = 0;
	extern PFNGLTEXTUREBARRIERPROC									glTextureBarrier = 0;

	// GL_ARB_bindless_texture
	extern PFNGLGETTEXTUREHANDLEARBPROC						glGetTextureHandleARB = 0;
	extern PFNGLGETTEXTURESAMPLERHANDLEARBPROC				glGetTextureSamplerHandleARB = 0;
	extern PFNGLMAKETEXTUREHANDLERESIDENTARBPROC			glMakeTextureHandleResidentARB = 0;
	extern PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC			glMakeTextureHandleNonResidentARB = 0;
	extern PFNGLGETIMAGEHANDLEARBPROC						glGetImageHandleARB = 0;
	extern PFNGLMAKEIMAGEHANDLERESIDENTARBPROC				glMakeImageHandleResidentARB = 0;
	extern PFNGLMAKEIMAGEHANDLENONRESIDENTARBPROC			glMakeImageHandleNonResidentARB = 0;
	extern PFNGLUNIFORMHANDLEUI64ARBPROC					glUniformHandleui64ARB = 0;
	extern PFNGLUNIFORMHANDLEUI64VARBPROC					glUniformHandleui64vARB = 0;
	extern PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC				glProgramUniformHandleui64ARB = 0;
	extern PFNGLPROGRAMUNIFORMHANDLEUI64VARBPROC			glProgramUniformHandleui64vARB = 0;
	extern PFNGLISTEXTUREHANDLERESIDENTARBPROC				glIsTextureHandleResidentARB = 0;
	extern PFNGLISIMAGEHANDLERESIDENTARBPROC				glIsImageHandleResidentARB = 0;
	extern PFNGLVERTEXATTRIBL1UI64ARBPROC					glVertexAttribL1ui64ARB = 0;
	extern PFNGLVERTEXATTRIBL1UI64VARBPROC					glVertexAttribL1ui64vARB = 0;
	extern PFNGLGETVERTEXATTRIBLUI64VARBPROC				glGetVertexAttribLui64vARB = 0;

	// GL_ARB_cl_event
	extern PFNGLCREATESYNCFROMCLEVENTARBPROC				glCreateSyncFromCLeventARB = 0;

	// GL_ARB_compute_variable_group_size
	extern PFNGLDISPATCHCOMPUTEGROUPSIZEARBPROC				glDispatchComputeGroupSizeARB = 0;

	// GL_ARB_debug_output
	extern PFNGLDEBUGMESSAGECONTROLARBPROC					glDebugMessageControlARB = 0;
	extern PFNGLDEBUGMESSAGEINSERTARBPROC					glDebugMessageInsertARB = 0;
	extern PFNGLDEBUGMESSAGECALLBACKARBPROC					glDebugMessageCallbackARB = 0;
	extern PFNGLGETDEBUGMESSAGELOGARBPROC					glGetDebugMessageLogARB = 0;

	// GL_ARB_draw_buffers_blend
	extern PFNGLBLENDEQUATIONIARBPROC						glBlendEquationiARB = 0;
	extern PFNGLBLENDEQUATIONSEPARATEIARBPROC				glBlendEquationSeparateiARB = 0;
	extern PFNGLBLENDFUNCIARBPROC							glBlendFunciARB = 0;
	extern PFNGLBLENDFUNCSEPARATEIARBPROC					glBlendFuncSeparateiARB = 0;

	// GL_ARB_indirect_parameters
	extern PFNGLMULTIDRAWARRAYSINDIRECTCOUNTARBPROC			glMultiDrawArraysIndirectCountARB = 0;
	extern PFNGLMULTIDRAWELEMENTSINDIRECTCOUNTARBPROC		glMultiDrawElementsIndirectCountARB = 0;

	// GL_ARB_robustness
	extern PFNGLGETGRAPHICSRESETSTATUSARBPROC				glGetGraphicsResetStatusARB = 0;
	extern PFNGLGETNTEXIMAGEARBPROC							glGetnTexImageARB = 0;
	extern PFNGLREADNPIXELSARBPROC							glReadnPixelsARB = 0;
	extern PFNGLGETNCOMPRESSEDTEXIMAGEARBPROC				glGetnCompressedTexImageARB = 0;
	extern PFNGLGETNUNIFORMFVARBPROC						glGetnUniformfvARB = 0;
	extern PFNGLGETNUNIFORMIVARBPROC						glGetnUniformivARB = 0;
	extern PFNGLGETNUNIFORMUIVARBPROC						glGetnUniformuivARB = 0;
	extern PFNGLGETNUNIFORMDVARBPROC						glGetnUniformdvARB = 0;

	// GL_ARB_sample_shading
	extern PFNGLMINSAMPLESHADINGARBPROC						glMinSampleShadingARB = 0;

	// GL_ARB_shading_language_include
	extern PFNGLNAMEDSTRINGARBPROC							glNamedStringARB = 0;
	extern PFNGLDELETENAMEDSTRINGARBPROC					glDeleteNamedStringARB = 0;
	extern PFNGLCOMPILESHADERINCLUDEARBPROC					glCompileShaderIncludeARB = 0;
	extern PFNGLISNAMEDSTRINGARBPROC						glIsNamedStringARB = 0;
	extern PFNGLGETNAMEDSTRINGARBPROC						glGetNamedStringARB = 0;
	extern PFNGLGETNAMEDSTRINGIVARBPROC						glGetNamedStringivARB = 0;

	// GL_ARB_sparse_buffer
	extern PFNGLBUFFERPAGECOMMITMENTARBPROC					glBufferPageCommitmentARB = 0;
	extern PFNGLNAMEDBUFFERPAGECOMMITMENTEXTPROC			glNamedBufferPageCommitmentEXT = 0;
	extern PFNGLNAMEDBUFFERPAGECOMMITMENTARBPROC			glNamedBufferPageCommitmentARB = 0;

	// GL_ARB_sparse_texture
	extern PFNGLTEXPAGECOMMITMENTARBPROC					glTexPageCommitmentARB = 0;
}

using namespace OpenGL;
using namespace DKFoundation;

#define GET_GL_PROC(func)		*(PROC*)&func = wglGetProcAddress(#func)

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

#if 0
			if (version >= 1.0)		// GL_VERSION_1_0
			{
				GET_GL_PROC(glCullFace);
				GET_GL_PROC(glFrontFace);
				GET_GL_PROC(glHint);
				GET_GL_PROC(glLineWidth);
				GET_GL_PROC(glPointSize);
				GET_GL_PROC(glPolygonMode);
				GET_GL_PROC(glScissor);
				GET_GL_PROC(glTexParameterf);
				GET_GL_PROC(glTexParameterfv);
				GET_GL_PROC(glTexParameteri);
				GET_GL_PROC(glTexParameteriv);
				GET_GL_PROC(glTexImage1D);
				GET_GL_PROC(glTexImage2D);
				GET_GL_PROC(glDrawBuffer);
				GET_GL_PROC(glClear);
				GET_GL_PROC(glClearColor);
				GET_GL_PROC(glClearStencil);
				GET_GL_PROC(glClearDepth);
				GET_GL_PROC(glStencilMask);
				GET_GL_PROC(glColorMask);
				GET_GL_PROC(glDepthMask);
				GET_GL_PROC(glDisable);
				GET_GL_PROC(glEnable);
				GET_GL_PROC(glFinish);
				GET_GL_PROC(glFlush);
				GET_GL_PROC(glBlendFunc);
				GET_GL_PROC(glLogicOp);
				GET_GL_PROC(glStencilFunc);
				GET_GL_PROC(glStencilOp);
				GET_GL_PROC(glDepthFunc);
				GET_GL_PROC(glPixelStoref);
				GET_GL_PROC(glPixelStorei);
				GET_GL_PROC(glReadBuffer);
				GET_GL_PROC(glReadPixels);
				GET_GL_PROC(glGetBooleanv);
				GET_GL_PROC(glGetDoublev);
				GET_GL_PROC(glGetError);
				GET_GL_PROC(glGetFloatv);
				GET_GL_PROC(glGetIntegerv);
				GET_GL_PROC(glGetString);
				GET_GL_PROC(glGetTexImage);
				GET_GL_PROC(glGetTexParameterfv);
				GET_GL_PROC(glGetTexParameteriv);
				GET_GL_PROC(glGetTexLevelParameterfv);
				GET_GL_PROC(glGetTexLevelParameteriv);
				GET_GL_PROC(glIsEnabled);
				GET_GL_PROC(glDepthRange);
				GET_GL_PROC(glViewport);
			}
			if (version >= 1.1)		// GL_VERSION_1_1
			{
				GET_GL_PROC(glDrawArrays);
				GET_GL_PROC(glDrawElements);
				GET_GL_PROC(glGetPointerv);
				GET_GL_PROC(glPolygonOffset);
				GET_GL_PROC(glCopyTexImage1D);
				GET_GL_PROC(glCopyTexImage2D);
				GET_GL_PROC(glCopyTexSubImage1D);
				GET_GL_PROC(glCopyTexSubImage2D);
				GET_GL_PROC(glTexSubImage1D);
				GET_GL_PROC(glTexSubImage2D);
				GET_GL_PROC(glBindTexture);
				GET_GL_PROC(glDeleteTextures);
				GET_GL_PROC(glGenTextures);
				GET_GL_PROC(glIsTexture);
			}
#endif
			if (version >= 1.2)		// GL_VERSION_1_2
			{
				GET_GL_PROC(glDrawRangeElements);
				GET_GL_PROC(glTexImage3D);
				GET_GL_PROC(glTexSubImage3D);
				GET_GL_PROC(glCopyTexSubImage3D);
			}
			if (version >= 1.3)		//GL_VERSION_1_3
			{
				GET_GL_PROC(glActiveTexture);
				GET_GL_PROC(glSampleCoverage);
				GET_GL_PROC(glCompressedTexImage3D);
				GET_GL_PROC(glCompressedTexImage2D);
				GET_GL_PROC(glCompressedTexImage1D);
				GET_GL_PROC(glCompressedTexSubImage3D);
				GET_GL_PROC(glCompressedTexSubImage2D);
				GET_GL_PROC(glCompressedTexSubImage1D);
				GET_GL_PROC(glGetCompressedTexImage);
			}
			if (version >= 1.4)		// GL_VERSION_1_4
			{
				GET_GL_PROC(glBlendFuncSeparate);
				GET_GL_PROC(glMultiDrawArrays);
				GET_GL_PROC(glMultiDrawElements);
				GET_GL_PROC(glPointParameterf);
				GET_GL_PROC(glPointParameterfv);
				GET_GL_PROC(glPointParameteri);
				GET_GL_PROC(glPointParameteriv);
				GET_GL_PROC(glBlendColor);
				GET_GL_PROC(glBlendEquation);
			}
			if (version >= 1.5)		// GL_VERSION_1_5
			{
				GET_GL_PROC(glGenQueries);
				GET_GL_PROC(glDeleteQueries);
				GET_GL_PROC(glIsQuery);
				GET_GL_PROC(glBeginQuery);
				GET_GL_PROC(glEndQuery);
				GET_GL_PROC(glGetQueryiv);
				GET_GL_PROC(glGetQueryObjectiv);
				GET_GL_PROC(glGetQueryObjectuiv);
				GET_GL_PROC(glBindBuffer);
				GET_GL_PROC(glDeleteBuffers);
				GET_GL_PROC(glGenBuffers);
				GET_GL_PROC(glIsBuffer);
				GET_GL_PROC(glBufferData);
				GET_GL_PROC(glBufferSubData);
				GET_GL_PROC(glGetBufferSubData);
				GET_GL_PROC(glMapBuffer);
				GET_GL_PROC(glUnmapBuffer);
				GET_GL_PROC(glGetBufferParameteriv);
				GET_GL_PROC(glGetBufferPointerv);
			}
			if (version >= 2.0)		// GL_VERSION_2_0
			{
				GET_GL_PROC(glBlendEquationSeparate);
				GET_GL_PROC(glDrawBuffers);
				GET_GL_PROC(glStencilOpSeparate);
				GET_GL_PROC(glStencilFuncSeparate);
				GET_GL_PROC(glStencilMaskSeparate);
				GET_GL_PROC(glAttachShader);
				GET_GL_PROC(glBindAttribLocation);
				GET_GL_PROC(glCompileShader);
				GET_GL_PROC(glCreateProgram);
				GET_GL_PROC(glCreateShader);
				GET_GL_PROC(glDeleteProgram);
				GET_GL_PROC(glDeleteShader);
				GET_GL_PROC(glDetachShader);
				GET_GL_PROC(glDisableVertexAttribArray);
				GET_GL_PROC(glEnableVertexAttribArray);
				GET_GL_PROC(glGetActiveAttrib);
				GET_GL_PROC(glGetActiveUniform);
				GET_GL_PROC(glGetAttachedShaders);
				GET_GL_PROC(glGetAttribLocation);
				GET_GL_PROC(glGetProgramiv);
				GET_GL_PROC(glGetProgramInfoLog);
				GET_GL_PROC(glGetShaderiv);
				GET_GL_PROC(glGetShaderInfoLog);
				GET_GL_PROC(glGetShaderSource);
				GET_GL_PROC(glGetUniformLocation);
				GET_GL_PROC(glGetUniformfv);
				GET_GL_PROC(glGetUniformiv);
				GET_GL_PROC(glGetVertexAttribdv);
				GET_GL_PROC(glGetVertexAttribfv);
				GET_GL_PROC(glGetVertexAttribiv);
				GET_GL_PROC(glGetVertexAttribPointerv);
				GET_GL_PROC(glIsProgram);
				GET_GL_PROC(glIsShader);
				GET_GL_PROC(glLinkProgram);
				GET_GL_PROC(glShaderSource);
				GET_GL_PROC(glUseProgram);
				GET_GL_PROC(glUniform1f);
				GET_GL_PROC(glUniform2f);
				GET_GL_PROC(glUniform3f);
				GET_GL_PROC(glUniform4f);
				GET_GL_PROC(glUniform1i);
				GET_GL_PROC(glUniform2i);
				GET_GL_PROC(glUniform3i);
				GET_GL_PROC(glUniform4i);
				GET_GL_PROC(glUniform1fv);
				GET_GL_PROC(glUniform2fv);
				GET_GL_PROC(glUniform3fv);
				GET_GL_PROC(glUniform4fv);
				GET_GL_PROC(glUniform1iv);
				GET_GL_PROC(glUniform2iv);
				GET_GL_PROC(glUniform3iv);
				GET_GL_PROC(glUniform4iv);
				GET_GL_PROC(glUniformMatrix2fv);
				GET_GL_PROC(glUniformMatrix3fv);
				GET_GL_PROC(glUniformMatrix4fv);
				GET_GL_PROC(glValidateProgram);
				GET_GL_PROC(glVertexAttrib1d);
				GET_GL_PROC(glVertexAttrib1dv);
				GET_GL_PROC(glVertexAttrib1f);
				GET_GL_PROC(glVertexAttrib1fv);
				GET_GL_PROC(glVertexAttrib1s);
				GET_GL_PROC(glVertexAttrib1sv);
				GET_GL_PROC(glVertexAttrib2d);
				GET_GL_PROC(glVertexAttrib2dv);
				GET_GL_PROC(glVertexAttrib2f);
				GET_GL_PROC(glVertexAttrib2fv);
				GET_GL_PROC(glVertexAttrib2s);
				GET_GL_PROC(glVertexAttrib2sv);
				GET_GL_PROC(glVertexAttrib3d);
				GET_GL_PROC(glVertexAttrib3dv);
				GET_GL_PROC(glVertexAttrib3f);
				GET_GL_PROC(glVertexAttrib3fv);
				GET_GL_PROC(glVertexAttrib3s);
				GET_GL_PROC(glVertexAttrib3sv);
				GET_GL_PROC(glVertexAttrib4Nbv);
				GET_GL_PROC(glVertexAttrib4Niv);
				GET_GL_PROC(glVertexAttrib4Nsv);
				GET_GL_PROC(glVertexAttrib4Nub);
				GET_GL_PROC(glVertexAttrib4Nubv);
				GET_GL_PROC(glVertexAttrib4Nuiv);
				GET_GL_PROC(glVertexAttrib4Nusv);
				GET_GL_PROC(glVertexAttrib4bv);
				GET_GL_PROC(glVertexAttrib4d);
				GET_GL_PROC(glVertexAttrib4dv);
				GET_GL_PROC(glVertexAttrib4f);
				GET_GL_PROC(glVertexAttrib4fv);
				GET_GL_PROC(glVertexAttrib4iv);
				GET_GL_PROC(glVertexAttrib4s);
				GET_GL_PROC(glVertexAttrib4sv);
				GET_GL_PROC(glVertexAttrib4ubv);
				GET_GL_PROC(glVertexAttrib4uiv);
				GET_GL_PROC(glVertexAttrib4usv);
				GET_GL_PROC(glVertexAttribPointer);
			}
			if (version >= 2.1)		// GL_VERSION_2_1
			{
				GET_GL_PROC(glUniformMatrix2x3fv);
				GET_GL_PROC(glUniformMatrix3x2fv);
				GET_GL_PROC(glUniformMatrix2x4fv);
				GET_GL_PROC(glUniformMatrix4x2fv);
				GET_GL_PROC(glUniformMatrix3x4fv);
				GET_GL_PROC(glUniformMatrix4x3fv);
			}
			if (version >= 3.0)		// GL_VERSION_3_0
			{
				GET_GL_PROC(glColorMaski);
				GET_GL_PROC(glGetBooleani_v);
				GET_GL_PROC(glGetIntegeri_v);
				GET_GL_PROC(glEnablei);
				GET_GL_PROC(glDisablei);
				GET_GL_PROC(glIsEnabledi);
				GET_GL_PROC(glBeginTransformFeedback);
				GET_GL_PROC(glEndTransformFeedback);
				GET_GL_PROC(glBindBufferRange);
				GET_GL_PROC(glBindBufferBase);
				GET_GL_PROC(glTransformFeedbackVaryings);
				GET_GL_PROC(glGetTransformFeedbackVarying);
				GET_GL_PROC(glClampColor);
				GET_GL_PROC(glBeginConditionalRender);
				GET_GL_PROC(glEndConditionalRender);
				GET_GL_PROC(glVertexAttribIPointer);
				GET_GL_PROC(glGetVertexAttribIiv);
				GET_GL_PROC(glGetVertexAttribIuiv);
				GET_GL_PROC(glVertexAttribI1i);
				GET_GL_PROC(glVertexAttribI2i);
				GET_GL_PROC(glVertexAttribI3i);
				GET_GL_PROC(glVertexAttribI4i);
				GET_GL_PROC(glVertexAttribI1ui);
				GET_GL_PROC(glVertexAttribI2ui);
				GET_GL_PROC(glVertexAttribI3ui);
				GET_GL_PROC(glVertexAttribI4ui);
				GET_GL_PROC(glVertexAttribI1iv);
				GET_GL_PROC(glVertexAttribI2iv);
				GET_GL_PROC(glVertexAttribI3iv);
				GET_GL_PROC(glVertexAttribI4iv);
				GET_GL_PROC(glVertexAttribI1uiv);
				GET_GL_PROC(glVertexAttribI2uiv);
				GET_GL_PROC(glVertexAttribI3uiv);
				GET_GL_PROC(glVertexAttribI4uiv);
				GET_GL_PROC(glVertexAttribI4bv);
				GET_GL_PROC(glVertexAttribI4sv);
				GET_GL_PROC(glVertexAttribI4ubv);
				GET_GL_PROC(glVertexAttribI4usv);
				GET_GL_PROC(glGetUniformuiv);
				GET_GL_PROC(glBindFragDataLocation);
				GET_GL_PROC(glGetFragDataLocation);
				GET_GL_PROC(glUniform1ui);
				GET_GL_PROC(glUniform2ui);
				GET_GL_PROC(glUniform3ui);
				GET_GL_PROC(glUniform4ui);
				GET_GL_PROC(glUniform1uiv);
				GET_GL_PROC(glUniform2uiv);
				GET_GL_PROC(glUniform3uiv);
				GET_GL_PROC(glUniform4uiv);
				GET_GL_PROC(glTexParameterIiv);
				GET_GL_PROC(glTexParameterIuiv);
				GET_GL_PROC(glGetTexParameterIiv);
				GET_GL_PROC(glGetTexParameterIuiv);
				GET_GL_PROC(glClearBufferiv);
				GET_GL_PROC(glClearBufferuiv);
				GET_GL_PROC(glClearBufferfv);
				GET_GL_PROC(glClearBufferfi);
				GET_GL_PROC(glGetStringi);
				GET_GL_PROC(glIsRenderbuffer);
				GET_GL_PROC(glBindRenderbuffer);
				GET_GL_PROC(glDeleteRenderbuffers);
				GET_GL_PROC(glGenRenderbuffers);
				GET_GL_PROC(glRenderbufferStorage);
				GET_GL_PROC(glGetRenderbufferParameteriv);
				GET_GL_PROC(glIsFramebuffer);
				GET_GL_PROC(glBindFramebuffer);
				GET_GL_PROC(glDeleteFramebuffers);
				GET_GL_PROC(glGenFramebuffers);
				GET_GL_PROC(glCheckFramebufferStatus);
				GET_GL_PROC(glFramebufferTexture1D);
				GET_GL_PROC(glFramebufferTexture2D);
				GET_GL_PROC(glFramebufferTexture3D);
				GET_GL_PROC(glFramebufferRenderbuffer);
				GET_GL_PROC(glGetFramebufferAttachmentParameteriv);
				GET_GL_PROC(glGenerateMipmap);
				GET_GL_PROC(glBlitFramebuffer);
				GET_GL_PROC(glRenderbufferStorageMultisample);
				GET_GL_PROC(glFramebufferTextureLayer);
				GET_GL_PROC(glMapBufferRange);
				GET_GL_PROC(glFlushMappedBufferRange);
				GET_GL_PROC(glBindVertexArray);
				GET_GL_PROC(glDeleteVertexArrays);
				GET_GL_PROC(glGenVertexArrays);
				GET_GL_PROC(glIsVertexArray);
			}
			if (version >= 3.1)		// GL_VERSION_3_1
			{
				GET_GL_PROC(glDrawArraysInstanced);
				GET_GL_PROC(glDrawElementsInstanced);
				GET_GL_PROC(glTexBuffer);
				GET_GL_PROC(glPrimitiveRestartIndex);
				GET_GL_PROC(glCopyBufferSubData);
				GET_GL_PROC(glGetUniformIndices);
				GET_GL_PROC(glGetActiveUniformsiv);
				GET_GL_PROC(glGetActiveUniformName);
				GET_GL_PROC(glGetUniformBlockIndex);
				GET_GL_PROC(glGetActiveUniformBlockiv);
				GET_GL_PROC(glGetActiveUniformBlockName);
				GET_GL_PROC(glUniformBlockBinding);
			}
			if (version >= 3.2)		// GL_VERSION_3_2
			{
				GET_GL_PROC(glDrawElementsBaseVertex);
				GET_GL_PROC(glDrawRangeElementsBaseVertex);
				GET_GL_PROC(glDrawElementsInstancedBaseVertex);
				GET_GL_PROC(glMultiDrawElementsBaseVertex);
				GET_GL_PROC(glProvokingVertex);
				GET_GL_PROC(glFenceSync);
				GET_GL_PROC(glIsSync);
				GET_GL_PROC(glDeleteSync);
				GET_GL_PROC(glClientWaitSync);
				GET_GL_PROC(glWaitSync);
				GET_GL_PROC(glGetInteger64v);
				GET_GL_PROC(glGetSynciv);
				GET_GL_PROC(glGetInteger64i_v);
				GET_GL_PROC(glGetBufferParameteri64v);
				GET_GL_PROC(glFramebufferTexture);
				GET_GL_PROC(glTexImage2DMultisample);
				GET_GL_PROC(glTexImage3DMultisample);
				GET_GL_PROC(glGetMultisamplefv);
				GET_GL_PROC(glSampleMaski);
			}
			if (version >= 3.3)		// GL_VERSION_3_3
			{
				GET_GL_PROC(glBindFragDataLocationIndexed);
				GET_GL_PROC(glGetFragDataIndex);
				GET_GL_PROC(glGenSamplers);
				GET_GL_PROC(glDeleteSamplers);
				GET_GL_PROC(glIsSampler);
				GET_GL_PROC(glBindSampler);
				GET_GL_PROC(glSamplerParameteri);
				GET_GL_PROC(glSamplerParameteriv);
				GET_GL_PROC(glSamplerParameterf);
				GET_GL_PROC(glSamplerParameterfv);
				GET_GL_PROC(glSamplerParameterIiv);
				GET_GL_PROC(glSamplerParameterIuiv);
				GET_GL_PROC(glGetSamplerParameteriv);
				GET_GL_PROC(glGetSamplerParameterIiv);
				GET_GL_PROC(glGetSamplerParameterfv);
				GET_GL_PROC(glGetSamplerParameterIuiv);
				GET_GL_PROC(glQueryCounter);
				GET_GL_PROC(glGetQueryObjecti64v);
				GET_GL_PROC(glGetQueryObjectui64v);
				GET_GL_PROC(glVertexAttribDivisor);
				GET_GL_PROC(glVertexAttribP1ui);
				GET_GL_PROC(glVertexAttribP1uiv);
				GET_GL_PROC(glVertexAttribP2ui);
				GET_GL_PROC(glVertexAttribP2uiv);
				GET_GL_PROC(glVertexAttribP3ui);
				GET_GL_PROC(glVertexAttribP3uiv);
				GET_GL_PROC(glVertexAttribP4ui);
				GET_GL_PROC(glVertexAttribP4uiv);
			}
			if (version >= 4.0)		// GL_VERSION_4_0
			{
				GET_GL_PROC(glMinSampleShading);
				GET_GL_PROC(glBlendEquationi);
				GET_GL_PROC(glBlendEquationSeparatei);
				GET_GL_PROC(glBlendFunci);
				GET_GL_PROC(glBlendFuncSeparatei);
				GET_GL_PROC(glDrawArraysIndirect);
				GET_GL_PROC(glDrawElementsIndirect);
				GET_GL_PROC(glUniform1d);
				GET_GL_PROC(glUniform2d);
				GET_GL_PROC(glUniform3d);
				GET_GL_PROC(glUniform4d);
				GET_GL_PROC(glUniform1dv);
				GET_GL_PROC(glUniform2dv);
				GET_GL_PROC(glUniform3dv);
				GET_GL_PROC(glUniform4dv);
				GET_GL_PROC(glUniformMatrix2dv);
				GET_GL_PROC(glUniformMatrix3dv);
				GET_GL_PROC(glUniformMatrix4dv);
				GET_GL_PROC(glUniformMatrix2x3dv);
				GET_GL_PROC(glUniformMatrix2x4dv);
				GET_GL_PROC(glUniformMatrix3x2dv);
				GET_GL_PROC(glUniformMatrix3x4dv);
				GET_GL_PROC(glUniformMatrix4x2dv);
				GET_GL_PROC(glUniformMatrix4x3dv);
				GET_GL_PROC(glGetUniformdv);
				GET_GL_PROC(glGetSubroutineUniformLocation);
				GET_GL_PROC(glGetSubroutineIndex);
				GET_GL_PROC(glGetActiveSubroutineUniformiv);
				GET_GL_PROC(glGetActiveSubroutineUniformName);
				GET_GL_PROC(glGetActiveSubroutineName);
				GET_GL_PROC(glUniformSubroutinesuiv);
				GET_GL_PROC(glGetUniformSubroutineuiv);
				GET_GL_PROC(glGetProgramStageiv);
				GET_GL_PROC(glPatchParameteri);
				GET_GL_PROC(glPatchParameterfv);
				GET_GL_PROC(glBindTransformFeedback);
				GET_GL_PROC(glDeleteTransformFeedbacks);
				GET_GL_PROC(glGenTransformFeedbacks);
				GET_GL_PROC(glIsTransformFeedback);
				GET_GL_PROC(glPauseTransformFeedback);
				GET_GL_PROC(glResumeTransformFeedback);
				GET_GL_PROC(glDrawTransformFeedback);
				GET_GL_PROC(glDrawTransformFeedbackStream);
				GET_GL_PROC(glBeginQueryIndexed);
				GET_GL_PROC(glEndQueryIndexed);
				GET_GL_PROC(glGetQueryIndexediv);
			}
			if (version >= 4.1)		// GL_VERSION_4_1
			{
				GET_GL_PROC(glReleaseShaderCompiler);
				GET_GL_PROC(glShaderBinary);
				GET_GL_PROC(glGetShaderPrecisionFormat);
				GET_GL_PROC(glDepthRangef);
				GET_GL_PROC(glClearDepthf);
				GET_GL_PROC(glGetProgramBinary);
				GET_GL_PROC(glProgramBinary);
				GET_GL_PROC(glProgramParameteri);
				GET_GL_PROC(glUseProgramStages);
				GET_GL_PROC(glActiveShaderProgram);
				GET_GL_PROC(glCreateShaderProgramv);
				GET_GL_PROC(glBindProgramPipeline);
				GET_GL_PROC(glDeleteProgramPipelines);
				GET_GL_PROC(glGenProgramPipelines);
				GET_GL_PROC(glIsProgramPipeline);
				GET_GL_PROC(glGetProgramPipelineiv);
				GET_GL_PROC(glProgramUniform1i);
				GET_GL_PROC(glProgramUniform1iv);
				GET_GL_PROC(glProgramUniform1f);
				GET_GL_PROC(glProgramUniform1fv);
				GET_GL_PROC(glProgramUniform1d);
				GET_GL_PROC(glProgramUniform1dv);
				GET_GL_PROC(glProgramUniform1ui);
				GET_GL_PROC(glProgramUniform1uiv);
				GET_GL_PROC(glProgramUniform2i);
				GET_GL_PROC(glProgramUniform2iv);
				GET_GL_PROC(glProgramUniform2f);
				GET_GL_PROC(glProgramUniform2fv);
				GET_GL_PROC(glProgramUniform2d);
				GET_GL_PROC(glProgramUniform2dv);
				GET_GL_PROC(glProgramUniform2ui);
				GET_GL_PROC(glProgramUniform2uiv);
				GET_GL_PROC(glProgramUniform3i);
				GET_GL_PROC(glProgramUniform3iv);
				GET_GL_PROC(glProgramUniform3f);
				GET_GL_PROC(glProgramUniform3fv);
				GET_GL_PROC(glProgramUniform3d);
				GET_GL_PROC(glProgramUniform3dv);
				GET_GL_PROC(glProgramUniform3ui);
				GET_GL_PROC(glProgramUniform3uiv);
				GET_GL_PROC(glProgramUniform4i);
				GET_GL_PROC(glProgramUniform4iv);
				GET_GL_PROC(glProgramUniform4f);
				GET_GL_PROC(glProgramUniform4fv);
				GET_GL_PROC(glProgramUniform4d);
				GET_GL_PROC(glProgramUniform4dv);
				GET_GL_PROC(glProgramUniform4ui);
				GET_GL_PROC(glProgramUniform4uiv);
				GET_GL_PROC(glProgramUniformMatrix2fv);
				GET_GL_PROC(glProgramUniformMatrix3fv);
				GET_GL_PROC(glProgramUniformMatrix4fv);
				GET_GL_PROC(glProgramUniformMatrix2dv);
				GET_GL_PROC(glProgramUniformMatrix3dv);
				GET_GL_PROC(glProgramUniformMatrix4dv);
				GET_GL_PROC(glProgramUniformMatrix2x3fv);
				GET_GL_PROC(glProgramUniformMatrix3x2fv);
				GET_GL_PROC(glProgramUniformMatrix2x4fv);
				GET_GL_PROC(glProgramUniformMatrix4x2fv);
				GET_GL_PROC(glProgramUniformMatrix3x4fv);
				GET_GL_PROC(glProgramUniformMatrix4x3fv);
				GET_GL_PROC(glProgramUniformMatrix2x3dv);
				GET_GL_PROC(glProgramUniformMatrix3x2dv);
				GET_GL_PROC(glProgramUniformMatrix2x4dv);
				GET_GL_PROC(glProgramUniformMatrix4x2dv);
				GET_GL_PROC(glProgramUniformMatrix3x4dv);
				GET_GL_PROC(glProgramUniformMatrix4x3dv);
				GET_GL_PROC(glValidateProgramPipeline);
				GET_GL_PROC(glGetProgramPipelineInfoLog);
				GET_GL_PROC(glVertexAttribL1d);
				GET_GL_PROC(glVertexAttribL2d);
				GET_GL_PROC(glVertexAttribL3d);
				GET_GL_PROC(glVertexAttribL4d);
				GET_GL_PROC(glVertexAttribL1dv);
				GET_GL_PROC(glVertexAttribL2dv);
				GET_GL_PROC(glVertexAttribL3dv);
				GET_GL_PROC(glVertexAttribL4dv);
				GET_GL_PROC(glVertexAttribLPointer);
				GET_GL_PROC(glGetVertexAttribLdv);
				GET_GL_PROC(glViewportArrayv);
				GET_GL_PROC(glViewportIndexedf);
				GET_GL_PROC(glViewportIndexedfv);
				GET_GL_PROC(glScissorArrayv);
				GET_GL_PROC(glScissorIndexed);
				GET_GL_PROC(glScissorIndexedv);
				GET_GL_PROC(glDepthRangeArrayv);
				GET_GL_PROC(glDepthRangeIndexed);
				GET_GL_PROC(glGetFloati_v);
				GET_GL_PROC(glGetDoublei_v);
			}
			if (version >= 4.2)		// GL_VERSION_4_2
			{
				GET_GL_PROC(glDrawArraysInstancedBaseInstance);
				GET_GL_PROC(glDrawElementsInstancedBaseInstance);
				GET_GL_PROC(glDrawElementsInstancedBaseVertexBaseInstance);
				GET_GL_PROC(glGetInternalformativ);
				GET_GL_PROC(glGetActiveAtomicCounterBufferiv);
				GET_GL_PROC(glBindImageTexture);
				GET_GL_PROC(glMemoryBarrier);
				GET_GL_PROC(glTexStorage1D);
				GET_GL_PROC(glTexStorage2D);
				GET_GL_PROC(glTexStorage3D);
				GET_GL_PROC(glDrawTransformFeedbackInstanced);
				GET_GL_PROC(glDrawTransformFeedbackStreamInstanced);
			}
			if (version >= 4.3)		// GL_VERSION_4_3
			{
				GET_GL_PROC(glClearBufferData);
				GET_GL_PROC(glClearBufferSubData);
				GET_GL_PROC(glDispatchCompute);
				GET_GL_PROC(glDispatchComputeIndirect);
				GET_GL_PROC(glCopyImageSubData);
				GET_GL_PROC(glFramebufferParameteri);
				GET_GL_PROC(glGetFramebufferParameteriv);
				GET_GL_PROC(glGetInternalformati64v);
				GET_GL_PROC(glInvalidateTexSubImage);
				GET_GL_PROC(glInvalidateTexImage);
				GET_GL_PROC(glInvalidateBufferSubData);
				GET_GL_PROC(glInvalidateBufferData);
				GET_GL_PROC(glInvalidateFramebuffer);
				GET_GL_PROC(glInvalidateSubFramebuffer);
				GET_GL_PROC(glMultiDrawArraysIndirect);
				GET_GL_PROC(glMultiDrawElementsIndirect);
				GET_GL_PROC(glGetProgramInterfaceiv);
				GET_GL_PROC(glGetProgramResourceIndex);
				GET_GL_PROC(glGetProgramResourceName);
				GET_GL_PROC(glGetProgramResourceiv);
				GET_GL_PROC(glGetProgramResourceLocation);
				GET_GL_PROC(glGetProgramResourceLocationIndex);
				GET_GL_PROC(glShaderStorageBlockBinding);
				GET_GL_PROC(glTexBufferRange);
				GET_GL_PROC(glTexStorage2DMultisample);
				GET_GL_PROC(glTexStorage3DMultisample);
				GET_GL_PROC(glTextureView);
				GET_GL_PROC(glBindVertexBuffer);
				GET_GL_PROC(glVertexAttribFormat);
				GET_GL_PROC(glVertexAttribIFormat);
				GET_GL_PROC(glVertexAttribLFormat);
				GET_GL_PROC(glVertexAttribBinding);
				GET_GL_PROC(glVertexBindingDivisor);
				GET_GL_PROC(glDebugMessageControl);
				GET_GL_PROC(glDebugMessageInsert);
				GET_GL_PROC(glDebugMessageCallback);
				GET_GL_PROC(glGetDebugMessageLog);
				GET_GL_PROC(glPushDebugGroup);
				GET_GL_PROC(glPopDebugGroup);
				GET_GL_PROC(glObjectLabel);
				GET_GL_PROC(glGetObjectLabel);
				GET_GL_PROC(glObjectPtrLabel);
				GET_GL_PROC(glGetObjectPtrLabel);
			}
			if (version >= 4.4)		// GL_VERSION_4_4
			{
				GET_GL_PROC(glBufferStorage);
				GET_GL_PROC(glClearTexImage);
				GET_GL_PROC(glClearTexSubImage);
				GET_GL_PROC(glBindBuffersBase);
				GET_GL_PROC(glBindBuffersRange);
				GET_GL_PROC(glBindTextures);
				GET_GL_PROC(glBindSamplers);
				GET_GL_PROC(glBindImageTextures);
				GET_GL_PROC(glBindVertexBuffers);
			}
			if (version >= 4.5)		// GL_VERSION_4_5
			{
				GET_GL_PROC(glClipControl);
				GET_GL_PROC(glCreateTransformFeedbacks);
				GET_GL_PROC(glTransformFeedbackBufferBase);
				GET_GL_PROC(glTransformFeedbackBufferRange);
				GET_GL_PROC(glGetTransformFeedbackiv);
				GET_GL_PROC(glGetTransformFeedbacki_v);
				GET_GL_PROC(glGetTransformFeedbacki64_v);
				GET_GL_PROC(glCreateBuffers);
				GET_GL_PROC(glNamedBufferStorage);
				GET_GL_PROC(glNamedBufferData);
				GET_GL_PROC(glNamedBufferSubData);
				GET_GL_PROC(glCopyNamedBufferSubData);
				GET_GL_PROC(glClearNamedBufferData);
				GET_GL_PROC(glClearNamedBufferSubData);
				GET_GL_PROC(glMapNamedBuffer);
				GET_GL_PROC(glMapNamedBufferRange);
				GET_GL_PROC(glUnmapNamedBuffer);
				GET_GL_PROC(glFlushMappedNamedBufferRange);
				GET_GL_PROC(glGetNamedBufferParameteriv);
				GET_GL_PROC(glGetNamedBufferParameteri64v);
				GET_GL_PROC(glGetNamedBufferPointerv);
				GET_GL_PROC(glGetNamedBufferSubData);
				GET_GL_PROC(glCreateFramebuffers);
				GET_GL_PROC(glNamedFramebufferRenderbuffer);
				GET_GL_PROC(glNamedFramebufferParameteri);
				GET_GL_PROC(glNamedFramebufferTexture);
				GET_GL_PROC(glNamedFramebufferTextureLayer);
				GET_GL_PROC(glNamedFramebufferDrawBuffer);
				GET_GL_PROC(glNamedFramebufferDrawBuffers);
				GET_GL_PROC(glNamedFramebufferReadBuffer);
				GET_GL_PROC(glInvalidateNamedFramebufferData);
				GET_GL_PROC(glInvalidateNamedFramebufferSubData);
				GET_GL_PROC(glClearNamedFramebufferiv);
				GET_GL_PROC(glClearNamedFramebufferuiv);
				GET_GL_PROC(glClearNamedFramebufferfv);
				GET_GL_PROC(glClearNamedFramebufferfi);
				GET_GL_PROC(glBlitNamedFramebuffer);
				GET_GL_PROC(glCheckNamedFramebufferStatus);
				GET_GL_PROC(glGetNamedFramebufferParameteriv);
				GET_GL_PROC(glGetNamedFramebufferAttachmentParameteriv);
				GET_GL_PROC(glCreateRenderbuffers);
				GET_GL_PROC(glNamedRenderbufferStorage);
				GET_GL_PROC(glNamedRenderbufferStorageMultisample);
				GET_GL_PROC(glGetNamedRenderbufferParameteriv);
				GET_GL_PROC(glCreateTextures);
				GET_GL_PROC(glTextureBuffer);
				GET_GL_PROC(glTextureBufferRange);
				GET_GL_PROC(glTextureStorage1D);
				GET_GL_PROC(glTextureStorage2D);
				GET_GL_PROC(glTextureStorage3D);
				GET_GL_PROC(glTextureStorage2DMultisample);
				GET_GL_PROC(glTextureStorage3DMultisample);
				GET_GL_PROC(glTextureSubImage1D);
				GET_GL_PROC(glTextureSubImage2D);
				GET_GL_PROC(glTextureSubImage3D);
				GET_GL_PROC(glCompressedTextureSubImage1D);
				GET_GL_PROC(glCompressedTextureSubImage2D);
				GET_GL_PROC(glCompressedTextureSubImage3D);
				GET_GL_PROC(glCopyTextureSubImage1D);
				GET_GL_PROC(glCopyTextureSubImage2D);
				GET_GL_PROC(glCopyTextureSubImage3D);
				GET_GL_PROC(glTextureParameterf);
				GET_GL_PROC(glTextureParameterfv);
				GET_GL_PROC(glTextureParameteri);
				GET_GL_PROC(glTextureParameterIiv);
				GET_GL_PROC(glTextureParameterIuiv);
				GET_GL_PROC(glTextureParameteriv);
				GET_GL_PROC(glGenerateTextureMipmap);
				GET_GL_PROC(glBindTextureUnit);
				GET_GL_PROC(glGetTextureImage);
				GET_GL_PROC(glGetCompressedTextureImage);
				GET_GL_PROC(glGetTextureLevelParameterfv);
				GET_GL_PROC(glGetTextureLevelParameteriv);
				GET_GL_PROC(glGetTextureParameterfv);
				GET_GL_PROC(glGetTextureParameterIiv);
				GET_GL_PROC(glGetTextureParameterIuiv);
				GET_GL_PROC(glGetTextureParameteriv);
				GET_GL_PROC(glCreateVertexArrays);
				GET_GL_PROC(glDisableVertexArrayAttrib);
				GET_GL_PROC(glEnableVertexArrayAttrib);
				GET_GL_PROC(glVertexArrayElementBuffer);
				GET_GL_PROC(glVertexArrayVertexBuffer);
				GET_GL_PROC(glVertexArrayVertexBuffers);
				GET_GL_PROC(glVertexArrayAttribBinding);
				GET_GL_PROC(glVertexArrayAttribFormat);
				GET_GL_PROC(glVertexArrayAttribIFormat);
				GET_GL_PROC(glVertexArrayAttribLFormat);
				GET_GL_PROC(glVertexArrayBindingDivisor);
				GET_GL_PROC(glGetVertexArrayiv);
				GET_GL_PROC(glGetVertexArrayIndexediv);
				GET_GL_PROC(glGetVertexArrayIndexed64iv);
				GET_GL_PROC(glCreateSamplers);
				GET_GL_PROC(glCreateProgramPipelines);
				GET_GL_PROC(glCreateQueries);
				GET_GL_PROC(glGetQueryBufferObjecti64v);
				GET_GL_PROC(glGetQueryBufferObjectiv);
				GET_GL_PROC(glGetQueryBufferObjectui64v);
				GET_GL_PROC(glGetQueryBufferObjectuiv);
				GET_GL_PROC(glMemoryBarrierByRegion);
				GET_GL_PROC(glGetTextureSubImage);
				GET_GL_PROC(glGetCompressedTextureSubImage);
				GET_GL_PROC(glGetGraphicsResetStatus);
				GET_GL_PROC(glGetnCompressedTexImage);
				GET_GL_PROC(glGetnTexImage);
				GET_GL_PROC(glGetnUniformdv);
				GET_GL_PROC(glGetnUniformfv);
				GET_GL_PROC(glGetnUniformiv);
				GET_GL_PROC(glGetnUniformuiv);
				GET_GL_PROC(glReadnPixels);
				GET_GL_PROC(glTextureBarrier);
			}

			////////////////////////////////////////////////////////////////////////////////
			// load extensions
			GLint numExts = 0;
			glGetIntegerv(GL_NUM_EXTENSIONS, &numExts);
			DKSet<const GLubyte*> extensions;
			for (GLint i = 0; i < numExts; i++)
			{
				extensions.Insert(glGetStringi(GL_EXTENSIONS, i));
			}

			if (extensions.Contains((const GLubyte*)"GL_ARB_bindless_texture"))
			{
				GET_GL_PROC(glGetTextureHandleARB);
				GET_GL_PROC(glGetTextureSamplerHandleARB);
				GET_GL_PROC(glMakeTextureHandleResidentARB);
				GET_GL_PROC(glMakeTextureHandleNonResidentARB);
				GET_GL_PROC(glGetImageHandleARB);
				GET_GL_PROC(glMakeImageHandleResidentARB);
				GET_GL_PROC(glMakeImageHandleNonResidentARB);
				GET_GL_PROC(glUniformHandleui64ARB);
				GET_GL_PROC(glUniformHandleui64vARB);
				GET_GL_PROC(glProgramUniformHandleui64ARB);
				GET_GL_PROC(glProgramUniformHandleui64vARB);
				GET_GL_PROC(glIsTextureHandleResidentARB);
				GET_GL_PROC(glIsImageHandleResidentARB);
				GET_GL_PROC(glVertexAttribL1ui64ARB);
				GET_GL_PROC(glVertexAttribL1ui64vARB);
				GET_GL_PROC(glGetVertexAttribLui64vARB);
			}
			if (extensions.Contains((const GLubyte*)"GL_ARB_cl_event"))
			{
				GET_GL_PROC(glCreateSyncFromCLeventARB);
			}
			if (extensions.Contains((const GLubyte*)"GL_ARB_compute_variable_group_size"))
			{
				GET_GL_PROC(glDispatchComputeGroupSizeARB);
			}
			if (extensions.Contains((const GLubyte*)"GL_ARB_debug_output"))
			{
				GET_GL_PROC(glDebugMessageControlARB);
				GET_GL_PROC(glDebugMessageInsertARB);
				GET_GL_PROC(glDebugMessageCallbackARB);
				GET_GL_PROC(glGetDebugMessageLogARB);
			}
			if (extensions.Contains((const GLubyte*)"GL_ARB_draw_buffers_blend"))
			{
				GET_GL_PROC(glBlendEquationiARB);
				GET_GL_PROC(glBlendEquationSeparateiARB);
				GET_GL_PROC(glBlendFunciARB);
				GET_GL_PROC(glBlendFuncSeparateiARB);
			}
			if (extensions.Contains((const GLubyte*)"GL_ARB_indirect_parameters"))
			{
				GET_GL_PROC(glMultiDrawArraysIndirectCountARB);
				GET_GL_PROC(glMultiDrawElementsIndirectCountARB);
			}
			if (extensions.Contains((const GLubyte*)"GL_ARB_robustness"))
			{
				GET_GL_PROC(glGetGraphicsResetStatusARB);
				GET_GL_PROC(glGetnTexImageARB);
				GET_GL_PROC(glReadnPixelsARB);
				GET_GL_PROC(glGetnCompressedTexImageARB);
				GET_GL_PROC(glGetnUniformfvARB);
				GET_GL_PROC(glGetnUniformivARB);
				GET_GL_PROC(glGetnUniformuivARB);
				GET_GL_PROC(glGetnUniformdvARB);
			}
			if (extensions.Contains((const GLubyte*)"GL_ARB_sample_shading"))
			{
				GET_GL_PROC(glMinSampleShadingARB);
			}
			if (extensions.Contains((const GLubyte*)"GL_ARB_shading_language_include"))
			{
				GET_GL_PROC(glNamedStringARB);
				GET_GL_PROC(glDeleteNamedStringARB);
				GET_GL_PROC(glCompileShaderIncludeARB);
				GET_GL_PROC(glIsNamedStringARB);
				GET_GL_PROC(glGetNamedStringARB);
				GET_GL_PROC(glGetNamedStringivARB);
			}
			if (extensions.Contains((const GLubyte*)"GL_ARB_sparse_buffer"))
			{
				GET_GL_PROC(glBufferPageCommitmentARB);
				GET_GL_PROC(glNamedBufferPageCommitmentEXT);
				GET_GL_PROC(glNamedBufferPageCommitmentARB);
			}
			if (extensions.Contains((const GLubyte*)"GL_ARB_sparse_texture"))
			{
				GET_GL_PROC(glTexPageCommitmentARB);
			}

			return true;
		}
	}
}

#endif		// #ifdef _WIN32
