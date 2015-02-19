import _dk_core as core

DEPTH_NONE = 0
DEPTH_16 = 1
DEPTH_24 = 2
DEPTH_32 = 3


class RenderTarget(core.RenderTarget):
    def __init__(self, width=0, height=0, depthFormat=DEPTH_NONE):
        super().__init__(int(width), int(height), depthFormat)


# create depth-only render target
def depthTarget(width, height, depthFormat):
    assert width > 0 and height > 0
    assert depthFormat in (DEPTH_16, DEPTH_24, DEPTH_32)

    from . import texture
    fmt = {DEPTH_16: texture.FORMAT_DEPTH16,
           DEPTH_24: texture.FORMAT_DEPTH24,
           DEPTH_32: texture.FORMAT_DEPTH32}[depthFormat]

    if depthFormat == DEPTH_16:
        type = texture.TYPE_UNSIGNED_SHORT
    else:
        type = texture.TYPE_UNSIGNED_INT

    depthTex = texture.Texture2D(width=width, height=height, format=fmt, type=type)
    rt = RenderTarget()
    rt.setDepthTexture(depthTex)
    return rt


#should be called after OpenGL initialized.
def maxColorTextures():
    '''maximum number of color attachments'''
    return core.RenderTarget.maxColorTextures()

#should be called after OpenGL initialized.
def maxTextureSize():
    '''maximum texture size in pixel for each dimension.'''
    return core.Texture.maxTextureSize()
