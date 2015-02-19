import _dk_core as core

class Frame(core.Frame):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def __del__(self):
        self.removeFromParent()
        self.unload()
        for c in self.children():
            c.removeFromParent()
            c.unload()

    def bounds(self):
        return super().bounds()

    def displayBounds(self):
        return self.unprojectLocalRect(self.bounds())

    def contentBounds(self):
        return self.bounds()

    def contentDisplayBounds(self):
        return self.unprojectLocalRect(self.contentBounds())

    def discardAction(self):
        pass

    def discardSurface(self):
        return super().discardSurface()

    def load(self, screen, resolution=(0, 0)):
        res = self.contentResolution
        rx = max(resolution[0], res[0], 1)
        ry = max(resolution[1], res[1], 1)
        return super().load(screen, (rx, ry))

    def removeFromParent(self, unload=True):
        super().removeFromParent()
        if unload:
            self.unload()

    # override if frame uses custom resolution
    def getResolution(self):
        return super().calculateBestResolution()

    # override if frame has special shape (not rect)
    def hitTest(self, pos):
        return True

    # frame's content area hit test
    # override if content bounds is not same as frame bounds
    def contentHitTest(self, pos):
        return True

    # override if frame should pre-processed mouse event
    def preprocessMouseEvent(self, type, deviceId, buttonId, pos, delta):
        return False    # return False to continue process event.

    # override if frame should pre-processed keyboard event
    def preprocessKeyboardEvent(self, type, deviceId, key, text):
        return False    # return False to continue process event.

    # frame-event
    def onUpdate(self, delta, tick, date):
        pass

    def onRender(self, renderer):
        pass

    def onLoaded(self):
        pass

    def onUnload(self):
        children = self.children()
        for c in children:
            c.unload()

    def onResized(self):
        pass

    # mouse events
    def onMouseDown(self, deviceId, buttonId, pos):
        pass

    def onMouseUp(self, deviceId, buttonId, pos):
        pass

    def onMouseMove(self, deviceId, pos, delta):
        pass

    def onMouseWheel(self, deviceId, pos, delta):
        pass

    def onMouseHover(self, deviceId):
        pass

    def onMouseLeave(self, deviceId):
        pass

    def onMouseLost(self, deviceId):
        pass

    # keyboard events
    def onKeyDown(self, deviceId, key):
        pass

    def onKeyUp(self, deviceId, key):
        pass

    def onKeyboardLost(self, deviceId):
        pass

    # text-input events
    def onTextInput(self, deviceId, text):
        pass

    def onTextInputCandidate(self, deviceId, text):
        pass

    def projectLocalPoint(self, pt):
        """
        convert actual(un-projected) local point to visible(projected) local point.

        :param pt: value to convert
        :type pt: Point
        :return: value converted
        :rtype: Point
        """
        v = core.Vector2(pt.x, pt.y)
        v.transform(self.contentTransformMatrix())
        return core.Point(v.x, v.y)

    def projectLocalRect(self, rect):
        """
        convert actual(un-projected) local rect to visible(projected) local rect.

        :param rect: value to convert
        :type rect: Rect
        :return: value converted
        :rtype: Rect
        """
        v0 = core.Vector2(rect.origin)
        v1 = core.Vector2(rect.x, rect.y + rect.height)
        v2 = core.Vector2(rect.x + rect.width, rect.y)
        v3 = core.Vector2(rect.x + rect.width, rect.y + rect.height)

        mat = self.contentTransformMatrix()
        for v in (v0, v1, v2, v3):
            v.transform(mat)

        x1 = min(v0.x, v1.x, v2.x, v3.x)
        x2 = max(v0.x, v1.x, v2.x, v3.x)
        y1 = min(v0.y, v1.y, v2.y, v3.y)
        y2 = max(v0.y, v1.y, v2.y, v3.y)
        return core.Rect(x1, y1, x2-x1, y2-y1)

    def unprojectLocalPoint(self, pt):
        """
        convert visible(projected) local point to actual(un-projected) local point.

        :param pt: value to convert
        :type pt: Point
        :return: value converted
        :rtype: Point
        """
        v = core.Vector2(pt.x, pt.y)
        v.transform(self.contentTransformInverseMatrix())
        return core.Point(v.x, v.y)

    def unprojectLocalRect(self, rect):
        """
        convert visible(projected) local rect to actual(un-projected) local rect.

        :param rect: value to convert
        :type rect: Rect
        :return: value converted
        :rtype: Rect
        """
        v0 = core.Vector2(rect.origin)
        v1 = core.Vector2(rect.x, rect.y + rect.height)
        v2 = core.Vector2(rect.x + rect.width, rect.y)
        v3 = core.Vector2(rect.x + rect.width, rect.y + rect.height)

        mat = self.contentTransformInverseMatrix()
        for v in (v0, v1, v2, v3):
            v.transform(mat)

        x1 = min(v0.x, v1.x, v2.x, v3.x)
        x2 = max(v0.x, v1.x, v2.x, v3.x)
        y1 = min(v0.y, v1.y, v2.y, v3.y)
        y2 = max(v0.y, v1.y, v2.y, v3.y)
        return core.Rect(x1, y1, x2-x1, y2-y1)

    def convertLocalToParent(self, point):
        return super().convertLocalToParent(point)

    def convertParentToLocal(self, point):
        return super().convertParentToLocal(point)

    def convertLocalToRoot(self, point):
        tm = self.localToRootTransformMatrix()
        v = point.vector()
        v.transform(tm)
        return core.Point(v)

    def convertRootToLocal(self, point):
        tm = self.localFromRootTransformMatrix()
        v = point.vector()
        v.transform(tm)
        return core.Point(v)

    def convertLocalToPixel(self, value):
        """
        convert local-space coordinates to pixel-space.

        :param value: value to convert
        :type value: Point or Size or Rect
        :return: pixel-space coordinates
        :rtype: Point or Size or Rect (same as input)
        """
        return super().convertLocalToPixel(value)

    def convertPixelToLocal(self, value):
        """
        convert pixel-space coordinates to local-space.

        :param value: value to convert
        :type value: Point or Size or Rect
        :return: local-space coordinates
        :rtype: Point or Size or Rect (same as input)
        """
        return super().convertPixelToLocal(value)

core.setDefaultClass(core.Frame, Frame)
