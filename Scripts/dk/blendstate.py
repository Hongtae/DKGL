import _dk_core as core

#blend mode constants
BLEND_MODE_ZERO = 0
BLEND_MODE_ONE = 1
BLEND_MODE_SRC_COLOR = 2
BLEND_MODE_ONE_MINUS_SRC_COLOR = 3
BLEND_MODE_DST_COLOR = 4
BLEND_MODE_ONE_MINUS_DST_COLOR = 5
BLEND_MODE_SRC_ALPHA = 6
BLEND_MODE_ONE_MINUS_SRC_ALPHA = 7
BLEND_MODE_DST_ALPHA = 8
BLEND_MODE_ONE_MINUS_DST_ALPHA = 9
BLEND_MODE_CONSTANT_COLOR = 10
BLEND_MODE_ONE_MINUS_CONST_COLOR = 11
BLEND_MODE_CONSTANT_ALPHA = 12
BLEND_MODE_ONE_MINUS_CONST_ALPHA = 13
BLEND_MODE_SRC_ALPHA_SATURATE = 14

#blend func constants
BLEND_FUNC_ADD = 0
BLEND_FUNC_SUBTRACT = 1
BLEND_FUNC_REVERSE_SUBTRACT = 2
BLEND_FUNC_MIN = 3
BLEND_FUNC_MAX = 4

class BlendState(core.BlendState):
    @classmethod
    def create(cls, **kwargs):
        bs = cls()
        bs.srcBlendRGB = kwargs.get('srcBlendRGB', BLEND_MODE_ONE)
        bs.srcBlendAlpha = kwargs.get('srcBlendAlpha', BLEND_MODE_ONE)
        bs.dstBlendRGB = kwargs.get('dstBlendRGB', BLEND_MODE_ZERO)
        bs.dstBlendAlpha = kwargs.get('dstBlendAlpha', BLEND_MODE_ZERO)
        bs.blendFuncRGB = kwargs.get('blendFuncRGB', BLEND_FUNC_ADD)
        bs.blendFuncAlpha = kwargs.get('blendFuncAlpha', BLEND_FUNC_ADD)
        bs.colorWriteR = kwargs.get('colorWriteR', True)
        bs.colorWriteG = kwargs.get('colorWriteG', True)
        bs.colorWriteB = kwargs.get('colorWriteB', True)
        bs.colorWriteA = kwargs.get('colorWriteA', True)
        bs.constantColor = kwargs.get('constantColor', core.Color(0.0, 0.0, 0.0, 0.0))
        return bs


defaultOpaque = BlendState.create(srcBlendRGB=BLEND_MODE_ONE,
                                  srcBlendAlpha=BLEND_MODE_ONE,
                                  dstBlendRGB=BLEND_MODE_ZERO,
                                  dstBlendAlpha=BLEND_MODE_ZERO,
                                  blendFuncRGB=BLEND_FUNC_ADD,
                                  blendFuncAlpha=BLEND_FUNC_ADD)

defaultAlpha = BlendState.create(srcBlendRGB=BLEND_MODE_SRC_ALPHA,
                                 srcBlendAlpha=BLEND_MODE_ONE,
                                 dstBlendRGB=BLEND_MODE_ONE_MINUS_SRC_ALPHA,
                                 dstBlendAlpha=BLEND_MODE_ONE,
                                 blendFuncRGB=BLEND_FUNC_ADD,
                                 blendFuncAlpha=BLEND_FUNC_ADD)

defaultMultiply = BlendState.create(srcBlendRGB=BLEND_MODE_ZERO,
                                    srcBlendAlpha=BLEND_MODE_ZERO,
                                    dstBlendRGB=BLEND_MODE_SRC_COLOR,
                                    dstBlendAlpha=BLEND_MODE_SRC_COLOR,
                                    blendFuncRGB=BLEND_FUNC_ADD,
                                    blendFuncAlpha=BLEND_FUNC_ADD)

defaultScreen = BlendState.create(srcBlendRGB=BLEND_MODE_ONE_MINUS_DST_COLOR,
                                  srcBlendAlpha=BLEND_MODE_ONE_MINUS_DST_COLOR,
                                  dstBlendRGB=BLEND_MODE_ONE,
                                  dstBlendAlpha=BLEND_MODE_ONE,
                                  blendFuncRGB=BLEND_FUNC_ADD,
                                  blendFuncAlpha=BLEND_FUNC_ADD)

defaultDarken = BlendState.create(srcBlendRGB=BLEND_MODE_ONE,
                                  srcBlendAlpha=BLEND_MODE_ONE,
                                  dstBlendRGB=BLEND_MODE_ONE,
                                  dstBlendAlpha=BLEND_MODE_ONE,
                                  blendFuncRGB=BLEND_FUNC_MIN,
                                  blendFuncAlpha=BLEND_FUNC_MIN)

defaultLighten = BlendState.create(srcBlendRGB=BLEND_MODE_ONE,
                                   srcBlendAlpha=BLEND_MODE_ONE,
                                   dstBlendRGB=BLEND_MODE_ONE,
                                   dstBlendAlpha=BLEND_MODE_ONE,
                                   blendFuncRGB=BLEND_FUNC_MAX,
                                   blendFuncAlpha=BLEND_FUNC_MAX)

defaultLinearBurn = BlendState.create(srcBlendRGB=BLEND_MODE_ONE,
                                      srcBlendAlpha=BLEND_MODE_ONE,
                                      dstBlendRGB=BLEND_MODE_ONE_MINUS_DST_COLOR,
                                      dstBlendAlpha=BLEND_MODE_ONE_MINUS_DST_COLOR,
                                      blendFuncRGB=BLEND_FUNC_SUBTRACT,
                                      blendFuncAlpha=BLEND_FUNC_SUBTRACT)

defaultLinearDodge = BlendState.create(srcBlendRGB=BLEND_MODE_ONE,
                                       srcBlendAlpha=BLEND_MODE_ONE,
                                       dstBlendRGB=BLEND_MODE_ONE,
                                       dstBlendAlpha=BLEND_MODE_ONE,
                                       blendFuncRGB=BLEND_FUNC_ADD,
                                       blendFuncAlpha=BLEND_FUNC_ADD)

