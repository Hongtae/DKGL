import _dk_core as core
from ..renderer import Renderer
from .. import blendstate
from .. import rendertarget
from . import view
import math


_identityMatrix3 = core.Matrix3()

class _AnimatedProgressValue:
    begin = 0.0
    end = 0.0
    length = 0.0
    elapsed = 0.0

class AnimatedProgressView(view.View):

    progressAnimation = 0.5

    borderWidth = 1
    borderColor = core.Color(0.0, 0.0, 0.0)

    backgroundColor = core.Color(0.5, 0.5, 0.5)
    progressColor = core.Color(1.0, 1.0, 1.0)

    minimumViewHeight = borderWidth*2 + 1
    minimumViewWidth = borderWidth*2 + 1


    def __init__(self, initialValue=0.0, minValue=0.0, maxValue=1.0, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._progress = initialValue
        self.minimumValue = minValue
        self.maximumValue = maxValue
        self.__animatedValue = None

    @property
    def progress(self):
        if self.__animatedValue:
            return self.__animatedValue.end
        return self._progress

    def setProgress(self, value, animate=True):

        if value < self.minimumValue:   value = self.minimumValue
        elif value > self.maximumValue: value = self.maximumValue

        if self._progress != value and animate and self.progressAnimation > 0.0:
            anim = _AnimatedProgressValue()
            anim.begin = self._progress
            anim.end = value
            anim.length = self.progressAnimation
            anim.elapsed = 0.0
            self.__animatedValue = anim
        else:
            self._progress = value
            self.__animatedValue = None


    def onUnload(self):
        super().onUnload()
        self._progress = self.progress
        self.__animatedValue = None

    def onUpdate(self, delta, tick, date):
        super().onUpdate(delta, tick, date)
        if self.__animatedValue:
            if self.__animatedValue.length > 0.0:
                self.__animatedValue.elapsed += delta
                t = self.__animatedValue.elapsed / self.__animatedValue.length
                t1 = 0 if t < 0.0 else 1.0 if t > 1.0 else t
                t2 = 1.0 - t1
                value = self.__animatedValue.end * t1 + self.__animatedValue.begin * t2
                if value < self.minimumValue:   value = self.minimumValue
                elif value > self.maximumValue: value = self.maximumValue
                self._progress = value
                if self.__animatedValue.elapsed >= self.__animatedValue.length:
                    self.__animatedValue = None
            else:
                value = self.__animatedValue.end
                if value < self.minimumValue:   value = self.minimumValue
                elif value > self.maximumValue: value = self.maximumValue
                self._progress = value
                self.__animatedValue = None
            self.redraw()

    def onRender(self, renderer):
        border = round(self.borderWidth)
        border2 = border*2
        bounds = self.bounds()

        rc = core.Rect(bounds.x + border, bounds.y + border, bounds.width - border2, bounds.height - border2)
        prog = self._progress / (self.maximumValue - self.minimumValue)
        if prog < 0.0: prog = 0.0
        if prog > 1.0: prog = 1.0
        rc.width = rc.width * prog

        if border > 0:
            renderer.clear(self.borderColor)
            with renderer.contextForSolidRects(self.progressColor, blend=blendstate.defaultOpaque) as r:
                r.add(rc)

            rc.x += rc.width
            rc.width = (bounds.width - border2) - rc.width

            with renderer.contextForSolidRects(self.backgroundColor, blend=blendstate.defaultOpaque) as r:
                r.add(rc)

        else:
            renderer.clear(self.backgroundColor)
            with renderer.contextForSolidRects(self.progressColor, blend=blendstate.defaultOpaque) as r:
                r.add(rc)


class RoundedBarProgressView(AnimatedProgressView):

    borderWidth = 1
    borderColor = core.Color(0.0, 0.0, 0.0)

    backgroundColor = core.Color(0.5, 0.5, 0.5)
    progressColor = core.Color(1.0, 1.0, 1.0)

    minimumViewHeight = borderWidth * 2 + 1
    minimumViewWidth = borderWidth * 2 + 1

    cornerRadius = 8
    vertical = False

    def __init__(self, initialValue=0.0, minValue=0.0, maxValue=1.0, *args, **kwargs):
        super().__init__(initialValue, minValue, maxValue, *args, **kwargs)
        self.setBlendState(blendstate.defaultAlpha)
        self.__bsLayerMask1 = None
        self.__bsLayerMask2 = None

    def discardSurface(self):
        super().discardSurface()
        self.__bsLayerMask1 = None
        self.__bsLayerMask2 = None

    def onResized(self):
        super().onResized()
        self.__bsLayerMask1 = None
        self.__bsLayerMask2 = None

    def onLoaded(self):
        super().onLoaded()
        self.__bsLayerMask1 = None
        self.__bsLayerMask2 = None

    def onUnload(self):
        super().onUnload()
        self.__bsLayerMask1 = None
        self.__bsLayerMask2 = None

    def onRender(self, renderer):
        border = round(self.borderWidth)
        border2 = border*2
        radius = round(self.cornerRadius)
        bounds = self.bounds()

        cornerRadius = min(radius, math.floor(bounds.height*0.5) - border, math.floor(bounds.width*0.5) - border)

        if cornerRadius > 1:
            texSize = (cornerRadius + border) * 2
            cornerRadius2 = cornerRadius * 2
            if self.__bsLayerMask1 is None:
                rt = rendertarget.RenderTarget(texSize * self.scaleFactor, texSize * self.scaleFactor, rendertarget.DEPTH_NONE)
                rd = Renderer(rt)
                rd.bounds = 0, 0, texSize, texSize
                rd.clear(core.Color(0, 0, 0, 0))
                with rd.contextForSolidEllipses(core.Color(1, 1, 1, 1), blend=blendstate.defaultOpaque) as r:
                    r.add(core.Rect(border, border, cornerRadius2, cornerRadius2))
                self.__bsLayerMask1 = rt.colorTextureAtIndex(0)

            if border > 0:
                if self.__bsLayerMask2 is None:
                    rt = rendertarget.RenderTarget(texSize * self.scaleFactor, texSize * self.scaleFactor, rendertarget.DEPTH_NONE)
                    rd = Renderer(rt)
                    rd.bounds = 0, 0, texSize, texSize
                    rd.clear(core.Color(0, 0, 0, 0))
                    with rd.contextForSolidEllipses(self.borderColor, blend=blendstate.defaultOpaque) as r:
                        r.add(core.Rect(0, 0, texSize, texSize))
                    with rd.contextForSolidEllipses(core.Color(0, 0, 0, 0), blend=blendstate.defaultOpaque) as r:
                        r.add(core.Rect(border, border, cornerRadius2, cornerRadius2))
                    self.__bsLayerMask2 = rt.colorTextureAtIndex(0)
            else:
                self.__bsLayerMask2 = None
        else:
            self.__bsLayerMask1 = None
            self.__bsLayerMask2 = None

        rc = core.Rect(bounds.x + border, bounds.y + border, bounds.width - border2, bounds.height - border2)
        prog = self._progress / (self.maximumValue - self.minimumValue)
        if prog < 0.0: prog = 0.0
        if prog > 1.0: prog = 1.0
        if self.vertical:
            rc.height = rc.height * prog
        else:
            rc.width = rc.width * prog

        if border > 0:
            renderer.clear(self.borderColor)
            with renderer.contextForSolidRects(self.progressColor, blend=blendstate.defaultOpaque) as r:
                r.add(rc)

            if self.vertical:
                rc.y += rc.height
                rc.height = (bounds.height - border2) - rc.height
            else:
                rc.x += rc.width
                rc.width = (bounds.width - border2) - rc.width

            with renderer.contextForSolidRects(self.backgroundColor, blend=blendstate.defaultOpaque) as r:
                r.add(rc)

        else:
            renderer.clear(self.backgroundColor)
            with renderer.contextForSolidRects(self.progressColor, blend=blendstate.defaultOpaque) as r:
                r.add(rc)

        if self.__bsLayerMask1:
            invScale = 1.0 / self.scaleFactor
            x = self.__bsLayerMask1.width * 0.5 * invScale
            y = self.__bsLayerMask1.height * 0.5 * invScale
            with renderer.contextForTexturedRects(self.__bsLayerMask1, blend=blendstate.defaultDarken) as r:
                r.add(core.Rect(bounds.x, bounds.y, x, y), _identityMatrix3, core.Rect(0, 0, 0.5, 0.5))
                r.add(core.Rect(bounds.x, bounds.y + bounds.height - y, x, y), _identityMatrix3, core.Rect(0, 0.5, 0.5, 0.5))
                r.add(core.Rect(bounds.x + bounds.width - x, bounds.y + bounds.height - y, x, y), _identityMatrix3, core.Rect(0.5, 0.5, 0.5, 0.5))
                r.add(core.Rect(bounds.x + bounds.width - x, bounds.y, x, y), _identityMatrix3, core.Rect(0.5, 0.0, 0.5, 0.5))

            if self.__bsLayerMask2:
                x = self.__bsLayerMask2.width * 0.5 * invScale
                y = self.__bsLayerMask2.height * 0.5 * invScale
                with renderer.contextForTexturedRects(self.__bsLayerMask2, blend=blendstate.defaultAlpha) as r:
                    r.add(core.Rect(bounds.x, bounds.y, x, y), _identityMatrix3, core.Rect(0, 0, 0.5, 0.5))
                    r.add(core.Rect(bounds.x, bounds.y + bounds.height - y, x, y), _identityMatrix3, core.Rect(0, 0.5, 0.5, 0.5))
                    r.add(core.Rect(bounds.x + bounds.width - x, bounds.y + bounds.height - y, x, y), _identityMatrix3, core.Rect(0.5, 0.5, 0.5, 0.5))
                    r.add(core.Rect(bounds.x + bounds.width - x, bounds.y, x, y), _identityMatrix3, core.Rect(0.5, 0.0, 0.5, 0.5))


class CircularProgressView(AnimatedProgressView):

    borderWidth = 1
    borderColor = core.Color(0.0, 0.0, 0.0)

    backgroundColor = core.Color(0.5, 0.5, 0.5)
    progressColor = core.Color(1.0, 1.0, 1.0)

    minimumViewHeight = borderWidth * 2 + 1
    minimumViewWidth = borderWidth * 2 + 1

    # circle-style properties
    innerCircleRadius = 0


    def __init__(self, initialValue=0.0, minValue=0.0, maxValue=1.0, *args, **kwargs):
        super().__init__(initialValue, minValue, maxValue, *args, **kwargs)
        self.setBlendState(blendstate.defaultAlpha)
        self.__csLayerMask = None

    def discardSurface(self):
        super().discardSurface()
        self.__csLayerMask = None


    def onResized(self):
        super().onResized()
        self.__csLayerMask = None


    def onLoaded(self):
        super().onLoaded()
        self.__csLayerMask = None


    def onUnload(self):
        super().onUnload()
        self.__csLayerMask = None

    def onRender(self, renderer):
        border = round(self.borderWidth)
        border2 = border*2
        bounds = self.contentBounds()

        clearColor = core.Color(0.0, 0.0, 0.0, 0.0)

        renderer.clear(clearColor)

        size = math.floor(min(bounds.width, bounds.height))

        circleRect = core.Rect(0, 0, size, size)
        circleRect.center = bounds.center

        if border > 0:
            with renderer.contextForSolidEllipses(self.borderColor, blend=blendstate.defaultOpaque) as r:
                r.add(circleRect)
            size -= border2
            circleRect.size = size, size
            circleRect.origin = circleRect.x + border, circleRect.y + border

        if size > 0:
            if self.__csLayerMask is None:
                rt = rendertarget.RenderTarget(size * self.scaleFactor, size * self.scaleFactor, rendertarget.DEPTH_NONE)
                rd = Renderer(rt)
                rd.clear(core.Color(0.0, 0.0, 0.0, 0.0))
                with rd.contextForSolidEllipses(core.Color(1, 1, 1, 1), blend=blendstate.defaultOpaque) as r:
                    r.add(core.Rect(0, 0, 1, 1))
                self.__csLayerMask = rt.colorTextureAtIndex(0)

            prog = self._progress / (self.maximumValue - self.minimumValue)
            if prog < 0.0: prog = 0.0
            if prog > 1.0: prog = 1.0

            if prog <= 0.0:
                with renderer.contextForSolidEllipses(self.backgroundColor, blend=blendstate.defaultOpaque) as r:
                    r.add(circleRect)
            elif prog >= 1.0:
                with renderer.contextForSolidEllipses(self.progressColor, blend=blendstate.defaultOpaque) as r:
                    r.add(circleRect)
            else:

                prog *= math.pi * 2
                normalizedCircularPos = core.Point(math.sin(prog) * 0.5 + 0.5, math.cos(prog) * 0.5 + 0.5)  # 0.0~1.0

                texSize = self.__csLayerMask.width / self.scaleFactor, self.__csLayerMask.height / self.scaleFactor
                offset = round(circleRect.x + (circleRect.width - texSize[0]) * 0.5), round(circleRect.y + (circleRect.height - texSize[1]) * 0.5)

                with renderer.contextForSolidEllipses(self.backgroundColor, blend=blendstate.defaultOpaque) as r:
                    r.add(circleRect)

                with renderer.contextForTexturedTriangleFan(self.__csLayerMask, color=self.progressColor, blend=blendstate.defaultAlpha) as r:

                    _pt1 = core.Point()
                    _pt2 = core.Point()
                    def addTexPos(x, y):
                        _pt1.tuple = x * texSize[0] + offset[0], y * texSize[1] + offset[1]
                        _pt2.tuple = x, y
                        r.add(_pt1, _pt2)

                    addTexPos(0.5, 0.5)
                    addTexPos(0.5, 1.0)

                    if prog < math.pi * 0.5:
                        addTexPos(normalizedCircularPos.x, 1.0)
                        addTexPos(normalizedCircularPos.x, normalizedCircularPos.y)
                    else:
                        addTexPos(1.0, 1.0)
                        addTexPos(1.0, 0.5)
                        if prog < math.pi:
                            addTexPos(1.0, normalizedCircularPos.y)
                            addTexPos(normalizedCircularPos.x, normalizedCircularPos.y)
                        else:
                            addTexPos(1.0, 0.0)
                            addTexPos(0.5, 0.0)
                            if prog < math.pi * 1.5:
                                addTexPos(normalizedCircularPos.x, 0.0)
                                addTexPos(normalizedCircularPos.x, normalizedCircularPos.y)
                            else:
                                addTexPos(0.0, 0.0)
                                addTexPos(0.0, 0.5)
                                if prog < math.pi * 2.0:
                                    addTexPos(0.0, normalizedCircularPos.y)
                                    addTexPos(normalizedCircularPos.x, normalizedCircularPos.y)

            innerCircleSize = math.floor(min(self.innerCircleRadius * 2, size-border2))
            if innerCircleSize > 0:

                innerCircleRect = core.Rect(0, 0, innerCircleSize, innerCircleSize)
                innerCircleRect.center = bounds.center

                if border > 0:
                    with renderer.contextForSolidEllipses(self.borderColor, blend=blendstate.defaultOpaque) as r:
                        r.add(innerCircleRect)

                    innerCircleRect.origin = innerCircleRect.x + border, innerCircleRect.y + border
                    innerCircleRect.size = innerCircleRect.width - border2, innerCircleRect.height - border2
                    innerCircleSize -= border2

                if innerCircleSize > 0:
                    with renderer.contextForSolidEllipses(clearColor, blend=blendstate.defaultOpaque) as r:
                        r.add(innerCircleRect)


def ProgressView(initialValue=0.0, minValue=0.0, maxValue=1.0, circularType=False, *args, **kwargs):
    if circularType:
        return CircularProgressView(initialValue, minValue, maxValue, *args, **kwargs)
    return RoundedBarProgressView(initialValue, minValue, maxValue, *args, **kwargs)
