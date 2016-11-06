LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := OpenAL
LOCAL_ARM_NEON := true
LOCAL_CFLAGS := -std=c99 -ffast-math -mfpu=neon -DAL_BUILD_LIBRARY -DAL_ALEXT_PROTOTYPES
LOCAL_LDLIBS += -lOpenSLES
LOCAL_C_INCLUDES := $(LOCAL_PATH)/OpenAL32/Include $(LOCAL_PATH)/include $(LOCAL_PATH)/build
LOCAL_SRC_FILES := \
	OpenAL32/alAuxEffectSlot.c \
	OpenAL32/alBuffer.c \
	OpenAL32/alEffect.c \
	OpenAL32/alError.c \
	OpenAL32/alExtension.c \
	OpenAL32/alFilter.c \
	OpenAL32/alListener.c \
	OpenAL32/alSource.c \
	OpenAL32/alState.c \
	OpenAL32/alThunk.c \
	Alc/ALc.c \
	Alc/alcConfig.c \
	Alc/alcRing.c \
	Alc/alcThread.c \
	Alc/Alu.c \
	Alc/bs2b.c \
	Alc/helpers.c \
	Alc/hrtf.c \
	Alc/mixer.c \
	Alc/mixer_c.c \
	Alc/mixer_neon.c \
	Alc/panning.c \
	Alc/effects/chorus.c \
	Alc/effects/dedicated.c \
	Alc/effects/distortion.c \
	Alc/effects/echo.c \
	Alc/effects/equalizer.c \
	Alc/effects/flanger.c \
	Alc/effects/modulator.c \
	Alc/effects/null.c \
	Alc/effects/reverb.c \
	Alc/backends/opensl.c \
	Alc/backends/loopback.c \
	Alc/backends/wave.c \
	Alc/backends/null.c

include $(BUILD_SHARED_LIBRARY)
