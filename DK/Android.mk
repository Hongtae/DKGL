DKLIB_ROOT := $(call my-dir)

include ${DKLIB_ROOT}/lib/ExtDeps/Android.mk
include ${DKLIB_ROOT}/lib/BulletPhysics/Android.mk
include ${DKLIB_ROOT}/lib/OpenAL/Android.mk

LOCAL_PATH := $(DKLIB_ROOT)
include $(CLEAR_VARS)

LOCAL_MODULE := DK
LOCAL_CPPFLAGS := -std=c++11 -fvisibility=hidden -mfpu=neon
LOCAL_CPPFLAGS += -DNDEBUG=1 -D_NDEBUG=1 -DDK_STATIC=1
LOCAL_CPPFLAGS += -Wno-multichar -Wno-enum-compare
LOCAL_CPP_FEATURES := rtti exceptions
LOCAL_STATIC_LIBRARIES := ExtDeps BulletPhysics AndroidLibs
LOCAL_SHARED_LIBRARIES := OpenAL
LOCAL_ARM_NEON := true

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/lib/ExtDeps/FreeType/include \
	$(LOCAL_PATH)/lib/ExtDeps/libxml2/include \
	$(LOCAL_PATH)/lib/ExtDeps/libogg/include \
	$(LOCAL_PATH)/lib/ExtDeps/bzip2 \
	$(LOCAL_PATH)/lib/BulletPhysics/src \
	$(LOCAL_PATH)/lib/Android

DKFOUNDATION_SRC := \
	DKFoundation/DKAllocator.cpp \
	DKFoundation/DKAllocatorChain.cpp \
	DKFoundation/DKAtomicNumber32.cpp \
	DKFoundation/DKAtomicNumber64.cpp \
	DKFoundation/DKBuffer.cpp \
	DKFoundation/DKBufferStream.cpp \
	DKFoundation/DKCondition.cpp \
	DKFoundation/DKData.cpp \
	DKFoundation/DKDataStream.cpp \
	DKFoundation/DKDateTime.cpp \
	DKFoundation/DKDirectory.cpp \
	DKFoundation/DKError.cpp \
	DKFoundation/DKFence.cpp \
	DKFoundation/DKFile.cpp \
	DKFoundation/DKFileMap.cpp \
	DKFoundation/DKHash.cpp \
	DKFoundation/DKLock.cpp \
	DKFoundation/DKLog.cpp \
	DKFoundation/DKMemory.cpp \
	DKFoundation/DKMutex.cpp \
	DKFoundation/DKObjectRefCounter.cpp \
	DKFoundation/DKOperationQueue.cpp \
	DKFoundation/DKRational.cpp \
	DKFoundation/DKRunLoop.cpp \
	DKFoundation/DKRunLoopTimer.cpp \
	DKFoundation/DKSharedLock.cpp \
	DKFoundation/DKSpinLock.cpp \
	DKFoundation/DKStringU8.cpp \
	DKFoundation/DKStringUE.cpp \
	DKFoundation/DKStringW.cpp \
	DKFoundation/DKThread.cpp \
	DKFoundation/DKTimer.cpp \
	DKFoundation/DKTypeInfo.cpp \
	DKFoundation/DKUtils.cpp \
	DKFoundation/DKUuid.cpp \
	DKFoundation/DKXMLDocument.cpp \
	DKFoundation/DKXMLParser.cpp \
	DKFoundation/DKZipArchiver.cpp \
	DKFoundation/DKZipUnarchiver.cpp \

DKFRAMEWORK_SRC := \
	DKFramework/DKAabb.cpp \
	DKFramework/DKAffineTransform2.cpp \
	DKFramework/DKAffineTransform3.cpp \
	DKFramework/DKAnimation.cpp \
	DKFramework/DKAnimationController.cpp \
	DKFramework/DKApplication.cpp \
	DKFramework/DKAudioListener.cpp \
	DKFramework/DKAudioPlayer.cpp \
	DKFramework/DKAudioSource.cpp \
	DKFramework/DKAudioStream.cpp \
	DKFramework/DKBlendState.cpp \
	DKFramework/DKBox.cpp \
	DKFramework/DKBoxShape.cpp \
	DKFramework/DKBvh.cpp \
	DKFramework/DKCamera.cpp \
	DKFramework/DKCapsuleShape.cpp \
	DKFramework/DKCollisionObject.cpp \
	DKFramework/DKCollisionShape.cpp \
	DKFramework/DKCompoundShape.cpp \
	DKFramework/DKConcaveShape.cpp \
	DKFramework/DKConeShape.cpp \
	DKFramework/DKConeTwistConstraint.cpp \
	DKFramework/DKConstraint.cpp \
	DKFramework/DKConvexHullShape.cpp \
	DKFramework/DKConvexShape.cpp \
	DKFramework/DKCylinderShape.cpp \
	DKFramework/DKDynamicsScene.cpp \
	DKFramework/DKFixedConstraint.cpp \
	DKFramework/DKFont.cpp \
	DKFramework/DKFrame.cpp \
	DKFramework/DKGearConstraint.cpp \
	DKFramework/DKGeneric6DofConstraint.cpp \
	DKFramework/DKGeneric6DofSpringConstraint.cpp \
	DKFramework/DKGeometryBuffer.cpp \
	DKFramework/DKHingeConstraint.cpp \
	DKFramework/DKIndexBuffer.cpp \
	DKFramework/DKLine.cpp \
	DKFramework/DKLinearTransform2.cpp \
	DKFramework/DKLinearTransform3.cpp \
	DKFramework/DKMaterial.cpp \
	DKFramework/DKMatrix2.cpp \
	DKFramework/DKMatrix3.cpp \
	DKFramework/DKMatrix4.cpp \
	DKFramework/DKMesh.cpp \
	DKFramework/DKModel.cpp \
	DKFramework/DKMultiSphereShape.cpp \
	DKFramework/DKOpenALContext.cpp \
	DKFramework/DKOpenGLContext.cpp \
	DKFramework/DKPlane.cpp \
	DKFramework/DKPoint2PointConstraint.cpp \
	DKFramework/DKPolyhedralConvexShape.cpp \
	DKFramework/DKPrimitiveIndex.cpp \
	DKFramework/DKPropertySet.cpp \
	DKFramework/DKQuaternion.cpp \
	DKFramework/DKRect.cpp \
	DKFramework/DKRenderer.cpp \
	DKFramework/DKRenderState.cpp \
	DKFramework/DKRenderTarget.cpp \
	DKFramework/DKResource.cpp \
	DKFramework/DKResourceLoader.cpp \
	DKFramework/DKResourcePool.cpp \
	DKFramework/DKRDKidBody.cpp \
	DKFramework/DKScene.cpp \
	DKFramework/DKScreen.cpp \
	DKFramework/DKSerializer.cpp \
	DKFramework/DKShader.cpp \
	DKFramework/DKShaderProgram.cpp \
	DKFramework/DKSkinMesh.cpp \
	DKFramework/DKSliderConstraint.cpp \
	DKFramework/DKSoftBody.cpp \
	DKFramework/DKSphere.cpp \
	DKFramework/DKSphereShape.cpp \
	DKFramework/DKSpline.cpp \
	DKFramework/DKStaticMesh.cpp \
	DKFramework/DKStaticPlaneShape.cpp \
	DKFramework/DKStaticTriangleMeshShape.cpp \
	DKFramework/DKTexture.cpp \
	DKFramework/DKTexture2D.cpp \
	DKFramework/DKTexture3D.cpp \
	DKFramework/DKTextureCube.cpp \
	DKFramework/DKTextureSampler.cpp \
	DKFramework/DKTransform.cpp \
	DKFramework/DKTriangle.cpp \
	DKFramework/DKTriangleMeshBvh.cpp \
	DKFramework/DKTriangleMeshProxyShape.cpp \
	DKFramework/DKVariant.cpp \
	DKFramework/DKVector2.cpp \
	DKFramework/DKVector3.cpp \
	DKFramework/DKVector4.cpp \
	DKFramework/DKVertexBuffer.cpp \
	DKFramework/DKVoxel32FileStorage.cpp \
	DKFramework/DKVoxel32SparseVolume.cpp \
	DKFramework/DKVoxelIsosurfacePolygonizer.cpp \
	DKFramework/DKVoxelPolygonizer.cpp \
	DKFramework/DKWindow.cpp \
	DKFramework/Private/DKAudioStreamFLAC.cpp \
	DKFramework/Private/DKAudioStreamVorbis.cpp \
	DKFramework/Private/DKAudioStreamWave.cpp

LOCAL_SRC_FILES := $(DKFOUNDATION_SRC) + $(DKFRAMEWORK_SRC)

include $(BUILD_STATIC_LIBRARY)


