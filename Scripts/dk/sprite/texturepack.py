import _dk_core as core

from collections import namedtuple
import string
import plistlib
import math
from ..resourcepool import ResourcePool


def _ParseStringToNumbers(str):
    d = str.replace('{', '').replace('}','').split(',')
    for i in range(len(d)):
        d[i] = int(d[i])
    return tuple(d)


Frame = namedtuple('Frame', ('transform', 'offset', 'scale', 'resolution'))


class TexturePack:
    def __init__(self, texture, filename, resolution, frames):
        self.filename = filename
        self.resolution = resolution
        self.frames = frames
        self.texture = texture

    @classmethod
    def createFromPlist(cls, source, pool):
        assert isinstance(pool, ResourcePool)

        if isinstance(source, core.Data):
            rootObject = plistlib.readPlistFromBytes(source)
        else:
            rootObject = plistlib.readPlist(source)

        if rootObject:
            metadata = rootObject.get('metadata')
            if not metadata:
                raise KeyError('metadata key is missing.')
            filename = metadata.get('realTextureFileName')
            if not filename:
                filename = metadata.get('textureFileName')
            if not filename:
                raise KeyError('metadata.realTextureFileName or metadata.textureFileName key is missing.')
            texture = pool.loadResource(filename)

            if texture:
                resolution = _ParseStringToNumbers(metadata.get('size'))
                resolution = core.Size(*resolution)

                frames = {}
                for key, value in rootObject.get('frames').items():
                    frame = _ParseStringToNumbers(value.get('frame'))
                    srcRect = _ParseStringToNumbers(value.get('sourceColorRect'))
                    srcSize = _ParseStringToNumbers(value.get('sourceSize'))
                    rotated = value.get('rotated')

                    frame = core.Rect(*frame)
                    srcRect = core.Rect(*srcRect)
                    srcSize = core.Size(*srcSize)


                    offsetX = srcRect.x / srcSize.width
                    offsetY = 1.0 - (srcRect.y + srcRect.height) / srcSize.height
                    scaleX = srcRect.width / srcSize.width
                    scaleY = srcRect.height / srcSize.height

                    texTM = core.AffineTransform2()
                    if rotated:
                        linear = core.LinearTransform2()
                        linear.scale(frame.width, frame.height)
                        linear.rotate(-math.pi * 0.5)
                        texTM.multiply(linear)
                        texTM.translate(frame.x, resolution.height - frame.y)
                    else:
                        linear = core.LinearTransform2()
                        linear.scale(frame.width, frame.height)
                        texTM.multiply(linear)
                        texTM.translate(frame.x, resolution.height - frame.y - frame.height)

                    linear = core.LinearTransform2()
                    linear.scale(1.0 / resolution.width, 1.0 / resolution.height)
                    texTM.multiply(linear)

                    frames[key] = Frame(texTM.matrix3(), core.Point(offsetX, offsetY), core.Size(scaleX, scaleY), srcSize)

                print('texturepack:{} frames loaded.'.format(len(frames)))
                return cls(texture, filename, resolution, frames)
            else:
                raise FileNotFoundError('file:{} not found'.format(filename))

              


