# 알고리즘을 테스트 하기 위한 클래스

import sys
import random
import _dk_core as core

from ... import ui
from . import guillotinebinpack

GuillotineBinPack = guillotinebinpack.GuillotineBinPack

#########################################
# FreeRectChoiceHeuristic
#########################################
# 'RectBestAreaFit',
# 'RectBestShortSideFit',
# 'RectBestLongSideFit',
# 'RectWorstAreaFit',
# 'RectWorstShortSideFit',
# 'RectWorstLongSideFit'
#########################################
# GuillotineSplitHeuristic
#########################################
# 'SplitShorterLeftoverAxis',
# 'SplitLongerLeftoverAxis',
# 'SplitMinimizeArea',
# 'SplitMaximizeArea',
# 'SplitShorterAxis',
# 'SplitLongerAxis'
#########################################

class GuillotineVisualizer(ui.View):
    def __init__(self, frame=None, **kwargs):
        super().__init__(frame=frame, **kwargs)

        self.binRectPacker = None
        self.packedBounds = []
        self.packedColors = []
        self.useCustomRect = False
        self._isInitialize = False

    def onLoaded(self):
        if self.binWidth > 0 and self.binHeight > 0 and self._isInitialize and not self.useCustomRect:
            self.appearCallback = self.screen().scheduleOperation(self.addSourceRect, (), {}, 0.3)

    def initGuilotine(self, width, height, freeChoiceHeuristic = guillotinebinpack.FreeRectChoiceHeuristic.RectBestShortSideFit, splitHeuristic = guillotinebinpack.GuillotineSplitHeuristic.SplitShorterLeftoverAxis):
        self.binRectPacker = guillotinebinpack.GuillotineBinPack()
        self.binRectPacker.initialize(width, height)

        self.binWidth = width
        self.binHeight = height
        self.freeChoiceHeuristic = freeChoiceHeuristic
        self.splitHeuristic = splitHeuristic
        self._isInitialize = True

        print("Initialize bin Size: %d, %d" % (width, height))

    def addSourceRect(self, sourceSize = None, sourceColor = None):
        if sourceSize == None:
            sourceSize, sourceColor = self.randomRectSource()

        packedBound = self.binRectPacker.insert(sourceSize.width, sourceSize.height, self.freeChoiceHeuristic, self.splitHeuristic)
        if packedBound.height > 0:
            self.packedBounds.append(packedBound)
            self.packedColors.append(sourceColor)
            self.redraw()

#            print("Packed to (x,y)=(%d,%d), (w,h)=(%d,%d). FreeSpace: %f%%" % (packedBound.x, packedBound.y, packedBound.width, packedBound.height, (100.0 - self.binRectPacker.Occupancy()*100.0)))
#            print("Used RectCount: %d / Free RectCount: %d" % (len(self.binRectPacker._usedRectangles), len(self.binRectPacker._freeRectangles)))
            print("UsedCounts: %d, FreeCounts: %d, Packed to (x,y)=(%d,%d), (w,h)=(%d,%d). FreeSpace: %f%%" % (len(self.binRectPacker._usedRectangles), len(self.binRectPacker._freeRectangles), packedBound.x, packedBound.y, packedBound.width, packedBound.height, (100.0 - self.binRectPacker.occupancy()*100.0)))
        elif self.binRectPacker.occupancy() > 0.95:
            self.appearCallback = None

    def randomRectSource(self):
        return core.Size(random.randrange(16, 256), random.randrange(16, 256)), core.Color(random.random(), random.random(), random.random())

    def onRender(self, renderer):
        super().onRender(renderer)

        adjustFactor = core.Point((self.frame.width / self.binWidth), (self.frame.height / self.binHeight))
        #####################################################################
        for index in range(len(self.packedBounds)):
            renderBound = core.Rect(self.packedBounds[index])
            renderBound.x = adjustFactor.x * renderBound.x
            renderBound.y = adjustFactor.y * renderBound.y
            renderBound.width = adjustFactor.x * renderBound.width
            renderBound.height = adjustFactor.y * renderBound.height
            with renderer.contextForSolidRects(self.packedColors[index]) as r:
                r.add(renderBound)

        #####################################################################
        # Debug FreeRects
        # for rectItem in self.binRectPacker._freeRectangles:
        #     renderBound = core.Rect(rectItem)
        #     renderBound.x = adjustFactor.x * rectItem.x
        #     renderBound.y = adjustFactor.y * rectItem.y
        #     renderBound.width = adjustFactor.x * rectItem.width
        #     renderBound.height = adjustFactor.y * rectItem.height
        #     with renderer.contextForSolidRects(core.color.black) as r:
        #         r.add(renderBound)

        #####################################################################
        # Debug UsedRects
        # for rectItem in self.binRectPacker._usedRectangles:
        #     renderBound = core.Rect(rectItem)
        #     renderBound.x = adjustFactor.x * rectItem.x
        #     renderBound.y = adjustFactor.y * rectItem.y
        #     renderBound.width = adjustFactor.x * rectItem.width
        #     renderBound.height = adjustFactor.y * rectItem.height
        #     with renderer.contextForSolidRects(core.color.black) as r:
        #         r.add(renderBound)