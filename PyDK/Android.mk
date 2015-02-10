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

#DKLIB_PATH, PYTHON_ROOT set by Jni/Android.mk

DKLIB_INCLUDE := ${DKLIB_PATH}
PYTHON_INCLUDE := ${PYTHON_ROOT}/PythonCore/Include

LOCAL_C_INCLUDES := \
	${DKLIB_INCLUDE} \
	${PYTHON_INCLUDE}

PYDK_SRC := \
	src/DCActionController.cpp \
	src/DCAffineTransform2.cpp \
	src/DCAffineTransform3.cpp \
	src/DCAnimatedTransform.cpp \
	src/DCAnimation.cpp \
	src/DCAnimationController.cpp \
	src/DCAudioListener.cpp \
	src/DCAudioPlayer.cpp \
	src/DCAudioSource.cpp \
	src/DCBlendState.cpp \
	src/DCBoxShape.cpp \
	src/DCCamera.cpp \
	src/DCCapsuleShape.cpp \
	src/DCCollisionObject.cpp \
	src/DCCollisionShape.cpp \
	src/DCColor.cpp \
	src/DCCompoundShape.cpp \
	src/DCConcaveShape.cpp \
	src/DCConeShape.cpp \
	src/DCConeTwistConstraint.cpp \
	src/DCConstraint.cpp \
	src/DCConvexHullShape.cpp \
	src/DCConvexShape.cpp \
	src/DCCylinderShape.cpp \
	src/DCData.cpp \
	src/DCDynamicsScene.cpp \
	src/DCFixedConstraint.cpp \
	src/DCFont.cpp \
	src/DCFrame.cpp \
	src/DCGearConstraint.cpp \
	src/DCGeneric6DofConstraint.cpp \
	src/DCGeneric6DofSpringConstraint.cpp \
	src/DCGeometryBuffer.cpp \
	src/DCHingeConstraint.cpp \
	src/DCIndexBuffer.cpp \
	src/DCLight.cpp \
	src/DCLinearTransform2.cpp \
	src/DCLinearTransform3.cpp \
	src/DCMaterial.cpp \
	src/DCMatrix2.cpp \
	src/DCMatrix3.cpp \
	src/DCMatrix4.cpp \
	src/DCMesh.cpp \
	src/DCModel.cpp \
	src/DCMultiSphereShape.cpp \
	src/DCNSTransform.cpp \
	src/DCObject.cpp \
	src/DCOpenGLContext.cpp \
	src/DCPoint.cpp \
	src/DCPoint2PointConstraint.cpp \
	src/DCPolyhedralConvexShape.cpp \
	src/DCQuaternion.cpp \
	src/DCRect.cpp \
	src/DCRenderer.cpp \
	src/DCRenderTarget.cpp \
	src/DCResource.cpp \
	src/DCResourceLoader.cpp \
	src/DCRigidBody.cpp \
	src/DCRunLoopTimer.cpp \
	src/DCScene.cpp \
	src/DCScreen.cpp \
	src/DCShader.cpp \
	src/DCShaderProgram.cpp \
	src/DCSize.cpp \
	src/DCSkinMesh.cpp \
	src/DCSliderConstraint.cpp \
	src/DCSoftBody.cpp \
	src/DCSphereShape.cpp \
	src/DCSpline.cpp \
	src/DCStaticMesh.cpp \
	src/DCStaticPlaneShape.cpp \
	src/DCStaticTriangleMeshShape.cpp \
	src/DCStream.cpp \
	src/DCTexture.cpp \
	src/DCTexture2D.cpp \
	src/DCTexture3D.cpp \
	src/DCTextureCube.cpp \
	src/DCTextureSampler.cpp \
	src/DCTimer.cpp \
	src/DCTransformUnit.cpp \
	src/DCUSTransform.cpp \
	src/DCVector2.cpp \
	src/DCVector3.cpp \
	src/DCVector4.cpp \
	src/DCVertexBuffer.cpp \
	src/DCWindow.cpp \
	src/DCZipArchiver.cpp \
	src/DCZipUnarchiver.cpp \
	src/PyDKApplication.cpp \
	src/PyDKCore.cpp \
	src/PyDKInterpreter.cpp \
	src/PyDKPython.cpp

LOCAL_SRC_FILES := $(PYDK_SRC)

include $(BUILD_STATIC_LIBRARY)
