import sys
import types
import _dk_core as core

# def enum(*sequential, **named):
#     enums = dict(zip(sequential, range(len(sequential))), **named)
#     return type('Enum', (), enums)

def enum(*seq, begin=0, prefix='Enum_'):
    from collections import namedtuple
    t = namedtuple(prefix + core.uuidgen().replace('-','_'), seq)
    return t(*tuple(range(begin, begin+len(seq))))

FreeRectChoiceHeuristic = enum('RectBestShortSideFit',
                               'RectBestLongSideFit',
                               'RectBestAreaFit',
                               'RectBottomLeftRule',
                               'RectContactPointRule')

class MaxRectsBinPack():
    def __init__(self, width=0, height=0):
        self.initialize(width, height)

    def initialize(self, width, height):
        self._binWidth = width
        self._binHeight = height
        self._usedRectangles = []
        self._freeRectangles = []
        self._freeRectangles.append(core.Rect(0, 0, width, height))

    def insert(self, width, height, method):
        if width <= 0 or height <= 0:
            print("MaxRectsBinPack::_binWidth and _binHeight must not be 0")
            return None

        score1, score2 = 0, 0
        newNode = core.Rect()
        if method == FreeRectChoiceHeuristic.RectBestShortSideFit:
            newNode = self._findPositionForNewNodeBestShortSideFit(width, height, score1, score2)
        elif method == FreeRectChoiceHeuristic.RectBestLongSideFit:
            newNode = self._findPositionForNewNodeBestLongSideFit(width, height, score2, score1)
        elif method == FreeRectChoiceHeuristic.RectBestAreaFit:
            newNode = self._findPositionForNewNodeBestAreaFit(width, height, score1, score2)
        elif method == FreeRectChoiceHeuristic.RectBottomLeftRule:
            newNode = self._findPositionForNewNodeBottomLeft(width, height, score1, score2)
        elif method == FreeRectChoiceHeuristic.RectContactPointRule:
            newNode = self._findPositionForNewNodeContactPoint(width, height, score1)
        if newNode.height == 0:
            return newNode

        index = 0
        numRectanglesToProcess = len(self._freeRectangles)
        while index < numRectanglesToProcess:
            if self._splitFreeNode(self._freeRectangles[index], newNode):
                self._freeRectangles.pop(index)
                index = index - 1
                numRectanglesToProcess = numRectanglesToProcess - 1
            index = index + 1

        self._pruneFreeList()
        self._usedRectangles.append(newNode)

        return newNode

    def occupancy(self):
        usedSurfaceArea = 0
        for rectItem in self._usedRectangles:
            usedSurfaceArea = usedSurfaceArea + (rectItem.width * rectItem.height)

        return usedSurfaceArea / (self._binWidth * self._binHeight)

    def _scoreRect(self, width, height, method, score1, score2):
        score1, score2 = sys.max.size, sys.maxsize

        if method == FreeRectChoiceHeuristic.RectBestShortSideFit:
            newNode = self._findPositionForNewNodeBestShortSideFit(width, height, score1, score2)
        elif method == FreeRectChoiceHeuristic.RectBestLongSideFit:
            newNode = self._findPositionForNewNodeBestLongSideFit(width, height, score2, score1)
        elif method == FreeRectChoiceHeuristic.RectBestAreaFit:
            newNode = self._findPositionForNewNodeBestAreaFit(width, height, score1, score2)
        elif method == FreeRectChoiceHeuristic.RectBottomLeftRule:
            newNode = self._findPositionForNewNodeBottomLeft(width, height, score1, score2)
        elif method == FreeRectChoiceHeuristic.RectContactPointRule:
            newNode = self._findPositionForNewNodeContactPoint(width, height, score1, True)
        if newNode.height == 0:
            score1 = sys.maxsize
            score2 = sys.maxsize

        return newNode, score1, score2


    def _placeRect(self, node):
        index = 0
        numRectanglesToProcess = len(self._freeRectangles)
        while index < numRectanglesToProcess:
            if self._splitFreeNode(self._freeRectangles[index], node):
                self._freeRectangles.pop(index)
                index = index - 1
                numRectanglesToProcess = numRectanglesToProcess - 1
            index = index + 1

        self._pruneFreeList()
        self._usedRectangles.append(node)

    def commonIntervalLength(start1, end1, start2, end2):
        if end1 < start2 or end2 < start1:
            return 0

        return min(end1, end2) - max(start1, start2)

    def _contactPointScoreNode(self, x, y, width, height):
        score = 0
        if x == 0 or x + width == self._binWidth:
            score = score + height
        if y == 0 or y + height == self._binHeight:
            score = score + width

        for rectItem in self._freeRectangles:
            if rectItem.x == x + width or rectItem.x + rectItem.width == x:
                score = MaxRectsBinPack.commonIntervalLength(rectItem.y, rectItem.y + rectItem.height, y, y + height)
            if rectItem.y == y + height or rectItem.y + rectItem.height == y:
                score = MaxRectsBinPack.commonIntervalLength(rectItem.x, rectItem.x + rectItem.width, x, x + width)

        return score

    def _findPositionForNewNodeBottomLeft(self, width, height, bestY, bestX):
        bestY = sys.maxsize
        bestNode = core.Rect()
        for rectItem in self._freeRectangles:
            if rectItem.width >= width and rectItem.height >= height:
                topSideY = rectItem.y + height
                if topSideY < bestY or (topSideY == bestY and rectItem.x < bestX):
                    bestNode.x = rectItem.x
                    bestNode.y = rectItem.y
                    bestNode.width = width
                    bestNode.height = height
                    bestY = topSideY
                    bestX = rectItem.x

            if rectItem.width >= height and rectItem.height >= width:
                topSideY = rectItem.y + width
                if topSideY < bestY or (topSideY == bestY and rectItem.x < bestX):
                    bestNode.x = rectItem.x
                    bestNode.y = rectItem.y
                    bestNode.width = height
                    bestNode.height = width
                    bestY = topSideY
                    bestX = rectItem.x

        return bestNode

    def _findPositionForNewNodeBestShortSideFit(self, width, height, bestShortSideFit, bestLongSideFit):
        bestShortSideFit = sys.maxsize
        bestNode = core.Rect()
        for rectItem in self._freeRectangles:
            if rectItem.width >= width and rectItem.height >= height:
                leftoverHoriz = abs(rectItem.width - width)
                leftoverVert = abs(rectItem.height - height)
                shortSideFit = min(leftoverHoriz, leftoverVert)
                longSideFit = max(leftoverHoriz, leftoverVert)

                if shortSideFit < bestShortSideFit or (shortSideFit == bestShortSideFit and longSideFit < bestLongSideFit):
                    bestNode.x = rectItem.x
                    bestNode.y = rectItem.y
                    bestNode.width = width
                    bestNode.height = height
                    bestShortSideFit = shortSideFit
                    bestLongSideFit = longSideFit

            if rectItem.width >= height and rectItem.height >= width:
                flippedLeftoverHoriz = abs(rectItem.width - height)
                flippedLeftoverVert = abs(rectItem.height - width)
                flippedShortSideFit = min(flippedLeftoverHoriz, flippedLeftoverVert)
                flippedLongSideFit = max(flippedLeftoverHoriz, flippedLeftoverVert)

                if flippedShortSideFit < bestShortSideFit or (flippedShortSideFit == bestShortSideFit and flippedLongSideFit < bestLongSideFit):
                    bestNode.x = rectItem.x
                    bestNode.y = rectItem.y
                    bestNode.width = height
                    bestNode.height = width
                    bestShortSideFit = flippedShortSideFit
                    bestLongSideFit = flippedLongSideFit

        return bestNode

    def _findPositionForNewNodeBestLongSideFit(self, width, height, bestShortSideFit, bestLongSideFit):
        bestLongSideFit = sys.maxsize
        bestNode = core.Rect()

        for rectItem in self._freeRectangles:
            if rectItem.width >= width and rectItem.height >= height:
                leftoverHoriz = abs(rectItem.width - width)
                leftoverVert = abs(rectItem.height - height)
                shortSideFit = min(leftoverHoriz, leftoverVert)
                longSideFit = max(leftoverHoriz, leftoverVert)

                if longSideFit < bestLongSideFit or (longSideFit == bestLongSideFit and shortSideFit < bestShortSideFit):
                    bestNode.x = rectItem.x
                    bestNode.y = rectItem.y
                    bestNode.width = width
                    bestNode.height = height
                    bestShortSideFit = shortSideFit
                    bestLongSideFit = longSideFit

            if rectItem.width >= height and rectItem.height >= width:
                leftoverHoriz = abs(rectItem.width - height)
                leftoverVert = abs(rectItem.height - width)
                shortSideFit = min(leftoverHoriz, leftoverVert)
                longSideFit = max(leftoverHoriz, leftoverVert)

                if longSideFit < bestLongSideFit or (longSideFit == bestLongSideFit and shortSideFit < bestShortSideFit):
                    bestNode.x = rectItem.x
                    bestNode.y = rectItem.y
                    bestNode.width = height
                    bestNode.height = width
                    bestShortSideFit = shortSideFit
                    bestLongSideFit = longSideFit

        return bestNode

    def _findPositionForNewNodeBestAreaFit(self, width, height, bestAreaFit, bestShortSideFit):
        bestAreaFit = sys.maxsize
        bestNode = core.Rect()
        for rectItem in self._freeRectangles:
            areaFit = rectItem.width * rectItem.height - width * height
            if rectItem.width >= width and rectItem.height >= height:
                leftoverHoriz = abs(rectItem.width - width)
                leftoverVert = abs(rectItem.height - height)
                shortSideFit = min(leftoverHoriz, leftoverVert)
                if areaFit < bestAreaFit or (areaFit == bestAreaFit and shortSideFit < bestShortSideFit):
                    bestNode.x = rectItem.x
                    bestNode.y = rectItem.y
                    bestNode.width = width
                    bestNode.height = height
                    bestShortSideFit = shortSideFit
                    bestAreaFit = areaFit

            if rectItem.width >= height and rectItem.height >= width:
                leftoverHoriz = abs(rectItem.width - height)
                leftoverVert = abs(rectItem.height - width)
                shortSideFit = min(leftoverHoriz, leftoverVert)
                if areaFit < bestAreaFit or (areaFit == bestAreaFit and shortSideFit < bestShortSideFit):
                    bestNode.x = rectItem.x
                    bestNode.y = rectItem.y
                    bestNode.width = height
                    bestNode.height = width
                    bestShortSideFit = shortSideFit
                    bestAreaFit = areaFit

        return bestNode

    def _findPositionForNewNodeContactPoint(self, width, height, bestContactScore, reverse=False):
        bestContactScore = -1
        bestNode = core.Rect()
        for rectItem in self._freeRectangles:
            if rectItem.width >= width and rectItem.height >= height:
                score = self._contactPointScoreNode(rectItem.x, rectItem.y, width, height)
                if score > bestContactScore:
                    bestNode.x = rectItem.x
                    bestNode.y = rectItem.y
                    bestNode.width = width
                    bestNode.height = height
                    bestContactScore = score

            if rectItem.width >= height and rectItem.height >= width:
                score = self._contactPointScoreNode(rectItem.x, rectItem.y, height, width)
                if score > bestContactScore:
                    bestNode.x = rectItem.x
                    bestNode.y = rectItem.y
                    bestNode.width = height
                    bestNode.height = width
                    bestContactScore = score

        if reverse:
            bestContactScore = -bestContactScore

        return bestNode

    def _splitFreeNode(self, freeNode, usedNode):
        if usedNode.x >= freeNode.x + freeNode.width or usedNode.x + usedNode.width <= freeNode.x or usedNode.y >= freeNode.y + freeNode.height or usedNode.y + usedNode.height <= freeNode.y:
            return False

        if usedNode.x < freeNode.x + freeNode.width and usedNode.x + usedNode.width > freeNode.x:
            if usedNode.y > freeNode.y and usedNode.y < freeNode.y + freeNode.height:
                newNode = core.Rect(freeNode)
                newNode.height = usedNode.y - newNode.y
                self._freeRectangles.append(newNode)

            if usedNode.y + usedNode.height < freeNode.y + freeNode.height:
                newNode = core.Rect(freeNode)
                newNode.y = usedNode.y + usedNode.height
                newNode.height = freeNode.y + freeNode.height - (usedNode.y + usedNode.height)
                self._freeRectangles.append(newNode)

        if usedNode.y < freeNode.y + freeNode.height and usedNode.y + usedNode.height > freeNode.y:
            if usedNode.x > freeNode.x and usedNode.x < freeNode.x + freeNode.width:
                newNode = core.Rect(freeNode)
                newNode.width = usedNode.x - newNode.x
                self._freeRectangles.append(newNode)

            if usedNode.x + usedNode.width < freeNode.x + freeNode.width:
                newNode = core.Rect(freeNode)
                newNode.x = usedNode.x + usedNode.width
                newNode.width = freeNode.x + freeNode.width - (usedNode.x + usedNode.width)
                self._freeRectangles.append(newNode)

        return True

    def _pruneFreeList(self):
        majorIndex = 0
        while majorIndex < len(self._freeRectangles):
            minorIndex = majorIndex + 1
            while minorIndex < len(self._freeRectangles):
                if MaxRectsBinPack.isContainedIn(self._freeRectangles[majorIndex], self._freeRectangles[minorIndex]):
                    self._freeRectangles.pop(majorIndex)
                    majorIndex = majorIndex - 1
                    break

                if MaxRectsBinPack.isContainedIn(self._freeRectangles[minorIndex], self._freeRectangles[majorIndex]):
                    self._freeRectangles.pop(minorIndex)
                    minorIndex = minorIndex - 1
                minorIndex = minorIndex + 1
            majorIndex = majorIndex + 1

    def isContainedIn(rect1, rect2):
        if rect1.x >= rect2.x and rect1.y >= rect2.y and rect1.x+rect1.width <= rect2.x+rect2.width and rect1.y+rect1.height <= rect2.y+rect2.height:
            return True

        return False