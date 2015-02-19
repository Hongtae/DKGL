import _dk_core as core

#texture target
TARGET_UNKNOWN = 0
TARGET_2D = 1
TARGET_3D = 2
TARGET_CUBE = 3

#texture format
FORMAT_UNKNOWN = 0
FORMAT_ALPHA = 1
FORMAT_RGB = 2
FORMAT_RGBA = 3
FORMAT_DEPTH16 = 4
FORMAT_DEPTH24 = 5
FORMAT_DEPTH32 = 6

#texture data type
TYPE_SIGNED_BYTE = 0
TYPE_UNSIGNED_BYTE = 1
TYPE_SIGNED_SHORT = 2
TYPE_UNSIGNED_SHORT = 3
TYPE_SIGNED_INT = 4
TYPE_UNSIGNED_INT = 5
TYPE_FLOAT = 6
TYPE_DOUBLE = 7

#wrap mode
WRAP_UNDEFINED = 0
WRAP_REPEAT = 1
WRAP_MIRRORED_REPEAT = 2
WRAP_CLAMP_TO_EDGE = 3

#texture filter
FILTER_UNDEFINED = 0
FILTER_NEAREST = 1
FILTER_LINEAR = 2
FILTER_NEAREST_MIPMAP_NEAREST = 3
FILTER_NEAREST_MIPMAP_LINEAR = 4
FILTER_LINEAR_MIPMAP_NEAREST = 5
FILTER_LINEAR_MIPMAP_LINEAR = 6

#pixel compare
COMPARE_NEVER = 0
COMPARE_ALWAYS = 1
COMPARE_LESS = 2
COMPARE_GREATER = 3
COMPARE_EQUAL = 4
COMPARE_NOT_EQUAL = 5
COMPARE_LESS_OR_EQUAL = 6
COMAPRE_GREATER_OR_EQUAL = 7

#abstract class
Texture = core.Texture

class Texture2D(core.Texture2D):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    @classmethod
    def create(cls, width, height, format, type = TYPE_UNSIGNED_BYTE):
        return cls(width = width, height = height, format = format, type = type)

    @classmethod
    def createFromData(cls, data, resampleWidth = 0, resampleHeight = 0):
        return cls(width = resampleWidth, height = resampleHeight, source = data)

    @classmethod
    def createFromStream(cls, stream, resampleWidth = 0, resampleHeight = 0):
        return cls(width = resampleWidth, height = resampleHeight, source=stream)

    @classmethod
    def createFromURL(cls, url, resampleWidth = 0, resampleHeight = 0):
        return cls(width = resampleWidth, height = resampleHeight, source=url)

    @classmethod
    def createFromFile(cls, file, resampleWidth = 0, resampleHeight = 0):
        return cls(width = resampleWidth, height = resampleHeight, source=file)

    def setPixelData(self, rect, data):
        return super().setPixelData(rect, data)

    def copyPixelData(self, rect, format=FORMAT_UNKNOWN, type=TYPE_UNSIGNED_BYTE):
        return super().copyPixelData(rect, format, type)

    def duplicate(self):
        return super().duplicate()

    def createImageFileData(self, format='png'):
        return super().createImageFileData(format)

Texture3D = core.Texture3D
TextureCube = core.TextureCube
TextureSampler = core.TextureSampler


# should be called after OpenGL initialized.
def maxTextureSize():
    '''maximum texture size in pixel for each dimension.'''
    return core.Texture.maxTextureSize()


def isPowerOfTwo(size):
    return size & (size-1) is 0
