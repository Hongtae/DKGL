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

FreeRectChoiceHeuristic = enum('RectBestAreaFit',
                               'RectBestShortSideFit',
                               'RectBestLongSideFit',
                               'RectWorstAreaFit',
                               'RectWorstShortSideFit',
                               'RectWorstLongSideFit')

GuillotineSplitHeuristic = enum('SplitShorterLeftoverAxis',
                                'SplitLongerLeftoverAxis',
                                'SplitMinimizeArea',
                                'SplitMaximizeArea',
                                'SplitShorterAxis',
                                'SplitLongerAxis')

class GuillotineBinPack():
    def __init__(self, width=0, height=0):
        GuillotineBinPack._freeRectChoiceHeuristicMap = [ GuillotineBinPack._scoreBestAreaFit,
                                                          GuillotineBinPack._scoreBestShortSideFit,
                                                          GuillotineBinPack._scoreBestLongSideFit,
                                                          GuillotineBinPack._scoreWorstAreaFit,
                                                          GuillotineBinPack._scoreWorstShortSideFit,
                                                          GuillotineBinPack._scoreWorstLongSideFit ]

        self.initialize(width, height)

    def initialize(self, width, height):
        self._binWidth = width
        self._binHeight = height
        self._usedRectangles = []
        self._freeRectangles = []
        self._freeRectangles.append(core.Rect(0, 0, width, height))

    # 분할된 공간에서 텍스쳐가 들어갈 노드를 찾아 공간을 분리하고 사용영역에 추가 및 빈공간 노드에서 빼낸다.
    def insert(self, width, height, rectChoice, splitMethod):
        if width <= 0 or height <= 0:
            print("GuillotineBinPack::_binWidth and _binHeight must not be 0")
            return None

        newRect, freeNodeIndex = self._findPositionForNewNode(width, height, rectChoice)
        if newRect.height == 0:
            return newRect

        self._splitFreeRectByHeuristic(self._freeRectangles[freeNodeIndex], newRect, splitMethod)
        self._freeRectangles.pop(freeNodeIndex)
        self._usedRectangles.append(newRect)

        return newRect

    # 사용 중인 영역을 계산한다.
    def occupancy(self):
        usedSurfaceArea = 0
        for rectItem in self._usedRectangles:
            usedSurfaceArea = usedSurfaceArea + (rectItem.width * rectItem.height)

        return usedSurfaceArea / (self._binWidth * self._binHeight)

    def mergeFreeList(self):
        majorIndex = 0
        while majorIndex < len(self._freeRectangles):
            minorIndex = majorIndex + 1
            while minorIndex < len(self._freeRectangles):
                if self._freeRectangles[majorIndex].width == self._freeRectangles[minorIndex].width and self._freeRectangles[majorIndex].x == self._freeRectangles[minorIndex].x:
                    if self._freeRectangles[majorIndex].y == self._freeRectangles[minorIndex].y + self._freeRectangles[minorIndex].height:
                        self._freeRectangles[majorIndex].y -= self._freeRectangles[minorIndex].height
                        self._freeRectangles[majorIndex].height += self._freeRectangles[minorIndex].height
                        self._freeRectangles.pop(minorIndex)
                        minorIndex = minorIndex - 1
                    elif self._freeRectangles[majorIndex].y + self._freeRectangles[majorIndex].height == self._freeRectangles[minorIndex].y:
                        self._freeRectangles[majorIndex].height += self._freeRectangles[minorIndex].height
                        self._freeRectangles.pop(minorIndex)
                        minorIndex = minorIndex - 1
                elif self._freeRectangles[majorIndex].height == self._freeRectangles[minorIndex].height and self._freeRectangles[majorIndex].y == self._freeRectangles[minorIndex].y:
                    if self._freeRectangles[majorIndex].x == self._freeRectangles[minorIndex].x + self._freeRectangles[minorIndex].width:
                        self._freeRectangles[majorIndex].x -= self._freeRectangles[minorIndex].width
                        self._freeRectangles[majorIndex].width += self._freeRectangles[minorIndex].width
                        self._freeRectangles.pop(minorIndex)
                        minorIndex = minorIndex - 1
                    elif self._freeRectangles[majorIndex].x + self._freeRectangles[majorIndex].width == self._freeRectangles[minorIndex].x:
                        self._freeRectangles[majorIndex].width += self._freeRectangles[minorIndex].width
                        self._freeRectangles.pop(minorIndex)
                        minorIndex = minorIndex - 1

    def _findPositionForNewNode(self, width, height, rectChoice):
        nodeIndex = 0
        bestNode = core.Rect()
        bestScore = sys.maxsize
        for itemIndex in range(len(self._freeRectangles)):
            if width == self._freeRectangles[itemIndex].width and height == self._freeRectangles[itemIndex].height:
                bestNode.x = self._freeRectangles[itemIndex].x
                bestNode.y = self._freeRectangles[itemIndex].y
                bestNode.width = width
                bestNode.height = height
                bestScore = sys.maxsize
                nodeIndex = itemIndex
                break
            elif height == self._freeRectangles[itemIndex].width and width == self._freeRectangles[itemIndex].height:
                bestNode.x = self._freeRectangles[itemIndex].x
                bestNode.y = self._freeRectangles[itemIndex].y
                bestNode.width = height
                bestNode.height = width
                bestScore = sys.maxsize
                nodeIndex = itemIndex
                break
            elif width <= self._freeRectangles[itemIndex].width and height <= self._freeRectangles[itemIndex].height:
                score = GuillotineBinPack._scoreByHeuristic(width, height, self._freeRectangles[itemIndex], rectChoice)
                if score < bestScore:
                    bestNode.x = self._freeRectangles[itemIndex].x
                    bestNode.y = self._freeRectangles[itemIndex].y
                    bestNode.width = width
                    bestNode.height = height
                    bestScore = score
                    nodeIndex = itemIndex
            elif height <= self._freeRectangles[itemIndex].width and width <= self._freeRectangles[itemIndex].height:
                score = GuillotineBinPack._scoreByHeuristic(height, width, self._freeRectangles[itemIndex], rectChoice)
                if score < bestScore:
                    bestNode.x = self._freeRectangles[itemIndex].x
                    bestNode.y = self._freeRectangles[itemIndex].y
                    bestNode.width = height
                    bestNode.height = width
                    bestScore = score
                    nodeIndex = itemIndex

        return bestNode, nodeIndex

    def _scoreByHeuristic(width, height, freeRect, rectChoice):
        return (GuillotineBinPack._freeRectChoiceHeuristicMap[rectChoice])(width, height, freeRect)

    def _scoreBestAreaFit(width, height, freeRect):
        return freeRect.width * freeRect.height - width * height

    def _scoreBestShortSideFit(width, height, freeRect):
        leftoverHoriz = abs(freeRect.width - width)
        leftoverVert = abs(freeRect.height - height)

        return min(leftoverHoriz, leftoverVert)

    def _scoreBestLongSideFit(width, height, freeRect):
        leftoverHoriz = abs(freeRect.width - width)
        leftoverVert = abs(freeRect.height - height)

        return max(leftoverHoriz, leftoverVert)

    def _scoreWorstAreaFit(width, height, freeRect):
        return -GuillotineBinPack._scoreBestAreaFit(width, height, freeRect)

    def _scoreWorstShortSideFit(width, height, freeRect):
        return -GuillotineBinPack._scoreBestShortSideFit(width, height, freeRect)

    def _scoreWorstLongSideFit(width, height, freeRect):
        return -GuillotineBinPack._scoreBestLongSideFit(width, height, freeRect)

    def _splitFreeRectByHeuristic(self, freeRect, placedRect, method):
        splitHorizontal = False
        fixedWidth = freeRect.width - placedRect.width
        fixedHeight = freeRect.height - placedRect.height

        if method == GuillotineSplitHeuristic.SplitShorterLeftoverAxis:
            splitHorizontal = (fixedWidth <= fixedHeight)
        elif method == GuillotineSplitHeuristic.SplitLongerLeftoverAxis:
            splitHorizontal = (fixedWidth > fixedHeight)
        elif method == GuillotineSplitHeuristic.SplitMinimizeArea:
            splitHorizontal = (placedRect.width * fixedHeight > fixedWidth * placedRect.height)
        elif method == GuillotineSplitHeuristic.SplitMaximizeArea:
            splitHorizontal = (placedRect.width * fixedHeight <= fixedWidth * placedRect.height)
        elif method == GuillotineSplitHeuristic.SplitShorterAxis:
            splitHorizontal = (freeRect.width <= freeRect.height)
        elif method == GuillotineSplitHeuristic.SplitLongerAxis:
            splitHorizontal = (freeRect.width > freeRect.height)
        else:
            splitHorizontal = True

        self._splitFreeRectAlongAxis(freeRect, placedRect, splitHorizontal)

    def _splitFreeRectAlongAxis(self, freeRect, placedRect, splitHorizontal):
        bottom = core.Rect()
        bottom.x = freeRect.x
        bottom.y = freeRect.y + placedRect.height
        bottom.height = freeRect.height - placedRect.height

        right = core.Rect()
        right.x = freeRect.x + placedRect.width
        right.y = freeRect.y
        right.width = freeRect.width - placedRect.width

        if splitHorizontal:
            bottom.width = freeRect.width
            right.height = placedRect.height
        else:
            bottom.width = placedRect.width
            right.height = freeRect.height

        if bottom.width > 0 and bottom.height > 0:
            self._freeRectangles.append(bottom)
        if right.width > 0 and right.height > 0:
            self._freeRectangles.append(right)