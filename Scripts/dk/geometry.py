import _dk_core as core
from collections import namedtuple
import array
from . import shader

#geometry buffer constants
TYPE_VERTEX_ARRAY = 0       #vertex buffer
TYPE_ELEMENT_ARRAY = 1      #indexbuffer

LOCATION_STATIC = 0
LOCATION_DYNAMIC = 1
LOCATION_STREAM = 2

USAGE_DRAW = 0
USAGE_READ = 1
USAGE_COPY = 2

ACCESS_READ = 0
ACCESS_WRITE = 2
ACCESS_READWRITE = 3

#index buffer constants
PRIMITIVE_UNKNOWN = 0
PRIMITIVE_POINTS = 1
PRIMITIVE_LINES = 2
PRIMITIVE_LINE_STRIP = 3
PRIMITIVE_LINE_LOOP = 4
PRIMITIVE_TRIANGLES = 5
PRIMITIVE_TRIANGLE_STRIP = 6
PRIMITIVE_TRIANGLE_FAN = 7
PRIMITIVE_LINES_ADJACENCY = 8
PRIMITIVE_LINE_STRIP_ADJACENCY = 9
PRIMITIVE_TRIANGLES_ADJACENCY = 10
PRIMITIVE_TRIANGLE_STRIP_ADJACENCY = 11

INDEX_TYPE_UNSIGNED_BYTES = 0
INDEX_TYPE_UNSIGNED_SHORT = 1   #fastest
INDEX_TYPE_UNSIGNED_INT = 2


#vertex stream
STREAM_ID = shader.ATTRIBUTE_ID
STREAM_TYPE = shader.ATTRIBUTE_TYPE


VertexTypeInfo = namedtuple('VertexTypeInfo', ('size','component','code'))
VertexTypeInfo.__doc__ = '''\
size: binary size for single element
component: number of components
code: binary conversion code for binary-packing module (ie, array,struct)
'''

_streamTypeInfoDict = {
    STREAM_TYPE.FLOAT1: VertexTypeInfo(4, 1, 'f'),
    STREAM_TYPE.FLOAT2: VertexTypeInfo(4, 2, 'f'),
    STREAM_TYPE.FLOAT3: VertexTypeInfo(4, 3, 'f'),
    STREAM_TYPE.FLOAT4: VertexTypeInfo(4, 4, 'f'),
    STREAM_TYPE.FLOAT2X2: VertexTypeInfo(4, 4, 'f'),
    STREAM_TYPE.FLOAT3X3: VertexTypeInfo(4, 9, 'f'),
    STREAM_TYPE.FLOAT4X4: VertexTypeInfo(4, 16, 'f'),
    STREAM_TYPE.BYTE1: VertexTypeInfo(1, 1, 'b'),
    STREAM_TYPE.BYTE2: VertexTypeInfo(1, 2, 'b'),
    STREAM_TYPE.BYTE3: VertexTypeInfo(1, 3, 'b'),
    STREAM_TYPE.BYTE4: VertexTypeInfo(1, 4, 'b'),
    STREAM_TYPE.UBYTE1: VertexTypeInfo(1, 1, 'B'),
    STREAM_TYPE.UBYTE2: VertexTypeInfo(1, 2, 'B'),
    STREAM_TYPE.UBYTE3: VertexTypeInfo(1, 3, 'B'),
    STREAM_TYPE.UBYTE4: VertexTypeInfo(1, 4, 'B'),
    STREAM_TYPE.SHORT1: VertexTypeInfo(2, 1, 'h'),
    STREAM_TYPE.SHORT2: VertexTypeInfo(2, 2, 'h'),
    STREAM_TYPE.SHORT3: VertexTypeInfo(2, 3, 'h'),
    STREAM_TYPE.SHORT4: VertexTypeInfo(2, 4, 'h'),
    STREAM_TYPE.USHORT1: VertexTypeInfo(2, 1, 'H'),
    STREAM_TYPE.USHORT2: VertexTypeInfo(2, 2, 'H'),
    STREAM_TYPE.USHORT3: VertexTypeInfo(2, 3, 'H'),
    STREAM_TYPE.USHORT4: VertexTypeInfo(2, 4, 'H')
}

VertexBuffer = core.VertexBuffer
IndexBuffer = core.IndexBuffer

class VertexBufferDecl:
    def __init__(self):
        self.decls = []
        self.offset = 0

    def add(self, stream, name, type, normalize=False, offset=-1):
        assert stream >= 0      # STREAM_ID.POSITION = 0
        assert stream <= STREAM_ID.USER_DEFINED
        assert isinstance(name, str)

        if offset < 0:
            offset = self.offset

        decl = {'id': stream,
                'name': str(name),
                'type': type,
                'normalize': bool(normalize),
                'offset': offset}

        self.decls.append(decl)
        typeInfo = _streamTypeInfoDict[type]
        typeSize = typeInfo.size * typeInfo.component
        if offset + typeSize > self.offset:
            self.offset = offset + typeSize


def createVertexBuffer(decl, source, count, location=LOCATION_STATIC, usage=USAGE_DRAW):
    vb = VertexBuffer(decl.decls, decl.offset, location, usage)
    if source and count > 0:
        vb.updateContent(source, count, location, usage)
    return vb


def createIndexBuffer(indices, primitiveType, indexType=INDEX_TYPE_UNSIGNED_SHORT, location=LOCATION_STATIC, usage=USAGE_DRAW):
    if indexType == INDEX_TYPE_UNSIGNED_BYTES:
        source = array.array('B')
    elif indexType == INDEX_TYPE_UNSIGNED_SHORT:
        source = array.array('H')
    elif indexType == INDEX_TYPE_UNSIGNED_INT:
        source = array.array('L')
    else:
        raise ValueError('Unsupported IndexType')

    source.extend(indices)
    return IndexBuffer(source, indexType, len(indices), primitiveType, location, usage)
