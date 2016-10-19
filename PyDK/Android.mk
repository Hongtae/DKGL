PYDK_ROOT := $(call my-dir)

LOCAL_PATH = $(PYDK_ROOT)
include $(CLEAR_VARS)

LOCAL_MODULE := PyDK
LOCAL_CPPFLAGS := -std=c++11 -fvisibility=hidden -DNDEBUG=1 -D_NDEBUG=1 -DDK_STATIC=1 -DPYDK_STATIC=1
LOCAL_CPPFLAGS += -Wno-multichar -Wno-enum-compare
LOCAL_CPP_FEATURES := rtti exceptions
LOCAL_STATIC_LIBRARIES := PythonCore OpenSSL DK
LOCAL_ARM_NEON := true
LOCAL_EXPORT_LDLIBS := -ldl

#DKGL_PATH, PYTHON_ROOT set by Jni/Android.mk

DKGL_INCLUDE := ${DKGL_PATH}
PYTHON_INCLUDE := ${PYTHON_ROOT}/PythonCore/Include

LOCAL_C_INCLUDES := \
	${DKGL_INCLUDE} \
	${PYTHON_INCLUDE}

PYDK_SRC := \
	DCActionController.cpp \
	DCAffineTransform2.cpp \
	DCAffineTransform3.cpp \
	DCAnimatedTransform.cpp \
	DCAnimation.cpp \
	DCAnimationController.cpp \
	DCAudioListener.cpp \
	DCAudioPlayer.cpp \
	DCAudioSource.cpp \
	DCBlendState.cpp \
	DCBoxShape.cpp \
	DCCamera.cpp \
	DCCapsuleShape.cpp \
	DCCollisionObject.cpp \
	DCCollisionShape.cpp \
	DCColor.cpp \
	DCCompoundShape.cpp \
	DCConcaveShape.cpp \
	DCConeShape.cpp \
	DCConeTwistConstraint.cpp \
	DCConstraint.cpp \
	DCConvexHullShape.cpp \
	DCConvexShape.cpp \
	DCCylinderShape.cpp \
	DCData.cpp \
	DCDynamicsWorld.cpp \
	DCFixedConstraint.cpp \
	DCFont.cpp \
	DCFrame.cpp \
	DCGearConstraint.cpp \
	DCGeneric6DofConstraint.cpp \
	DCGeneric6DofSpringConstraint.cpp \
	DCGeometryBuffer.cpp \
	DCHingeConstraint.cpp \
	DCIndexBuffer.cpp \
	DCLight.cpp \
	DCLinearTransform2.cpp \
	DCLinearTransform3.cpp \
	DCMaterial.cpp \
	DCMatrix2.cpp \
	DCMatrix3.cpp \
	DCMatrix4.cpp \
	DCMesh.cpp \
	DCModel.cpp \
	DCMultiSphereShape.cpp \
	DCNSTransform.cpp \
	DCObject.cpp \
	DCOpenGLContext.cpp \
	DCPoint.cpp \
	DCPoint2PointConstraint.cpp \
	DCPolyhedralConvexShape.cpp \
	DCQuaternion.cpp \
	DCRect.cpp \
	DCRenderer.cpp \
	DCRenderTarget.cpp \
	DCResource.cpp \
	DCResourceLoader.cpp \
	DCRigidBody.cpp \
	DCRunLoopTimer.cpp \
	DCWorld.cpp \
	DCScreen.cpp \
	DCShader.cpp \
	DCShaderProgram.cpp \
	DCSize.cpp \
	DCSkinMesh.cpp \
	DCSliderConstraint.cpp \
	DCSoftBody.cpp \
	DCSphereShape.cpp \
	DCSpline.cpp \
	DCStaticMesh.cpp \
	DCStaticPlaneShape.cpp \
	DCStaticTriangleMeshShape.cpp \
	DCStream.cpp \
	DCTexture.cpp \
	DCTexture2D.cpp \
	DCTexture3D.cpp \
	DCTextureCube.cpp \
	DCTextureSampler.cpp \
	DCTimer.cpp \
	DCTransformUnit.cpp \
	DCUSTransform.cpp \
	DCVector2.cpp \
	DCVector3.cpp \
	DCVector4.cpp \
	DCVertexBuffer.cpp \
	DCWindow.cpp \
	DCZipArchiver.cpp \
	DCZipUnarchiver.cpp \
	PyDKApplication.cpp \
	PyDKCore.cpp \
	PyDKInterpreter.cpp \
	PyDKPython.cpp

LOCAL_SRC_FILES := $(PYDK_SRC)

include $(BUILD_STATIC_LIBRARY)
