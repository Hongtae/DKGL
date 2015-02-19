import _dk_core as core
import weakref
from collections import namedtuple
from .. import blendstate


_DisplayFrames = namedtuple('_DisplayFrames', ('texture', 'renderer'))


class Sprite:
    # sprite state
    STATE_NORMAL = 0,
    STATE_HIGHLIGHTED = 1,
    STATE_DISABLED = 2

    TEXTURE_RECT = core.Rect(0, 0, 1, 1)

    def __init__(self, center=(0, 0), size=(1, 1), texturePack=None, name=''):
        self.name = name
        self.state = Sprite.STATE_NORMAL
        self.hidden = False
        self.transform = core.Matrix3()
        self.transformInverse = core.Matrix3()
        self._parent = None
        self.children = []
        self.paused = False

        self.texturePack = texturePack
        self._textureIdsForState = {Sprite.STATE_NORMAL: (),
                                    Sprite.STATE_HIGHLIGHTED: (),
                                    Sprite.STATE_DISABLED: ()}

        # callback functions with argument Sprite(self)
        self.highlightCallback = None
        self.buttonCallback = None

        # animated values
        # three values tuple
        self.diffuse = (1.0, 1.0, 1.0)

        # two values tuple
        self.center = center        # Sprite's center of parent space
        self.size = size            # Sprite's size of parent space
        self.offset = (0.0, 0.0)    # Sprite's position offset of parent space
        self.scale = (1.0, 1.0)     # Sprite's scale of parent space

        # single value tuple
        self.rotate = (0.0,)
        self.alpha = (1.0,)
        self.textureIndex = (0.0,)

        self._animators = {}
        self._mouseId = None
        self.update(0, 0)

    def update(self, delta, tick):
        if self.hidden or self.state == Sprite.STATE_DISABLED:
            self._mouseId = None

        if self.paused:
            return

        finished = []
        # update animations
        for attr, anim in self._animators.items():
            anim.update(delta)
            self.__dict__[attr] = anim.value
            if anim.finished:
                finished.append((attr, anim))
                print('anim finished attr:{}'.format(attr))

        # calculate transform
        trans = core.AffineTransform2()
        trans.translate(self.size[0] * -0.5, self.size[1] * -0.5)
        linear = core.LinearTransform2()
        linear.scale(self.scale[0], self.scale[1])
        linear.rotate(self.rotate[0])
        trans.multiply(linear)
        trans.translate(self.offset[0] + self.center[0], self.offset[1] + self.center[1])

        self.transform = trans.matrix3()
        trans.inverse()
        self.transformInverse = trans.matrix3()

        # update children
        for c in self.children[:]:
            c.update(delta, tick)

        # remove finished animation, invoke finish callback
        for attr, anim in finished:
            del self._animators[attr]
            if anim.callback:
                anim.callback(anim)

    def render(self, renderer):
        rc = renderer.contextForTexturedRects(texture=None, blend=blendstate.defaultAlpha)
        self._render(rc, core.Matrix3(), core.Color(1, 1, 1, 1))

        if rc.texture:  # draw rectangles
            with rc:
                pass

    def _render(self, rc, transform, color):
        if self.hidden:
            return
        cc = core.Color(self.diffuse[0] * color.r, self.diffuse[1] * color.g, self.diffuse[2] * color.b, self.alpha[0] * color.a)
        if cc.a > 0.0:
            transform = self.transform * transform

            # get texture ids
            if self.texturePack:
                texIds = self._textureIdsForState[self.state]
                if len(texIds):
                    texIndex = round(self.textureIndex[0]) % len(texIds)
                    texKey = texIds[texIndex]
                    frame = self.texturePack.frames.get(texKey)
                else:
                    frame = self.texturePack.frames.get(self.name)

                if frame:
                    # draw other Sprites
                    if rc.texture != self.texturePack.texture or rc.color.argb32Value() != cc.argb32Value():
                        if rc.texture:
                            with rc:
                                pass               
                        rc.texture = self.texturePack.texture
                        rc.color = cc

                    # calculate texture frame.
                    rect = core.Rect(self.size[0] * frame.offset.x,
                                     self.size[1] * frame.offset.y,
                                     self.size[0] * frame.scale.width,
                                     self.size[1] * frame.scale.height)

                    rc.add(rect, transform, self.TEXTURE_RECT, frame.transform)

            for c in self.children:
                c._render(rc, transform, cc)

    def setTextureIds(self, state, ids):
        if ids:
            if isinstance(ids, tuple):
                self._textureIdsForState[state] = ids
            else:
                self._textureIdsForState[state] = (ids,)
        else:
            self._textureIdsForState[state] = ()


    def setAnimation(self, attr, anim):
        v = self.__dict__.get(attr)
        if v and isinstance(v, tuple):
            if anim:
                self._animators[attr] = anim
            elif attr in self._animators:
                del self._animators[attr]
        else:
            raise KeyError('invalid attr:{}'.format(attr))

    def animation(self, attr):
        return self._animators.get(attr)

    def addChild(self, child):
        if isinstance(child, Sprite):
            if not child._parent:
                self.children.append(child)
                child._parent = weakref.ref(self)
            else:
                raise ValueError('cannot add object.')
        else:
            raise TypeError('child must be Sprite object.')

    def parent(self):
        if self._parent:
            return self._parent()

    def removeFromParent(self):
        parent = self.parent()
        if parent:
            parent.children.remove(self)
        self._parent = None

    def bringFront(self, c):
        if c.parent() == self:
            self.children.remove(c)
            self.children.insert(0, c)
        else:
            raise ValueError('child is not belongs to self')

    def sendBack(self, c):
        if c.parent() == self:
            self.children.remove(c)
            self.children.append(c)
        else:
            raise ValueError('child is not belongs to self')

    def mouseDown(self, deviceId, pos):
        if self._mouseId is None:
            if self.state == Sprite.STATE_NORMAL:
                if self.isPointInside(pos):
                    for c in self.children[:]:
                        if not c.hidden:
                            pt = c.convertPoint(pos)
                            c.mouseDown(deviceId, pt)

                    self.state = Sprite.STATE_HIGHLIGHTED
                    self._mouseId = deviceId
                    if self.highlightCallback:
                        self.highlightCallback(self)

    def mouseUp(self, deviceId, pos):
        mouseUpInside = False
        if self._mouseId == deviceId:
            if self.state == Sprite.STATE_HIGHLIGHTED:
                if self.isPointInside(pos):
                    mouseUpInside = True
                self.state = Sprite.STATE_NORMAL
            self._mouseId = None

        for c in self.children[:]:
            pt = c.convertPoint(pos)
            c.mouseUp(deviceId, pt)

        if mouseUpInside:
            if self.buttonCallback:
                self.buttonCallback(self)

    def mouseMove(self, deviceId, pos):
        if self._mouseId == deviceId:
            if self.state == Sprite.STATE_NORMAL:
                if self.isPointInside(pos):
                    self.state = Sprite.STATE_HIGHLIGHTED
            elif self.state == Sprite.STATE_HIGHLIGHTED:
                if not self.isPointInside(pos):
                    self.state = Sprite.STATE_NORMAL

        for c in self.children[:]:
            pt = c.convertPoint(pos)
            c.mouseMove(deviceId, pt)

    def discardAction(self):
        if self.state != Sprite.STATE_DISABLED:
            self.state = Sprite.STATE_NORMAL
        self._mouseId = None
        for c in self.children:
            c.discardAction()

    def isPointInside(self, pt):
        w, h = self.size[:2]
        if pt.x >= 0 and pt.x <= w:
            if pt.y >= 0 and pt.y <= h:
                return True
        return False

    def convertPoint(self, pt):
        v = pt.vector()
        v.transform(self.transformInverse)
        return core.Point(v)

    def bounds(self):
        return core.Rect(0, 0, self.size[0], self.size[1])