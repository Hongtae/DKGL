import _dk_core as core
import array

from _dk_core import ConvexHullShape
from _dk_core import CompoundShape

class ShapeBuilder:
    def __init__(self):
        self.vertices = array.array('f')
        self.indices = array.array('l')

    def finalize(self,
                 minClusters=2,
                 maxVertsPerCH=100,
                 maxConcavity=100.0, 
                 addExtraDistPoints=False,
                 addNeighboursDistPoints=False,
                 addFacesPoints=False):

        hulls = ConvexHullShape.decomposeTriangleMesh(self.vertices,
                                                      self.indices,
                                                      minClusters,
                                                      maxVertsPerCH,
                                                      maxConcavity,
                                                      addExtraDistPoints,
                                                      addNeighboursDistPoints,
                                                      addFacesPoints)

        compound = CompoundShape()
        for shape, offset in hulls:
            compound.addChild(shape, offset)

        self.vertices = array.array('f')
        self.indices = array.array('l')

        return compound

    def addTriangle(self, p1, p2, p3):
        index = len(self.vertices) / 3
        self.vertices.extend(p1.tuple + p2.tuple + p3.tuple)
        self.indices.extend((index, index+1, index+2))

