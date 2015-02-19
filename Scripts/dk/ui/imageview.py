import _dk_core as core
from .. import blendstate
from . import view

class ImageView(view.View):

    textureTransform = core.Matrix3()

    def __init__(self, image=None, *args, **kwargs):
        super().__init__(*args, **kwargs)
        if image:
            assert isinstance(image, core.Texture2D)

        self.textureImage = image
        self.textureBlend = blendstate.defaultOpaque
        self.textureSampler = None

    def onUnload(self):
        self.textureImage = None
        self.textureSampler = None
        return super().onUnload()

    def onRender(self, renderer):
        super().onRender(renderer)
        if self.textureImage:
            with renderer.contextForTexturedRects(self.textureImage,
                                                        sampler=self.textureSampler,
                                                        color=self.backgroundColor,
                                                        blend=self.textureBlend) as r:
                r.add(self.contentDisplayBounds(), self.textureTransform)
