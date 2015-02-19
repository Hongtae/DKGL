import _dk_core as core
from .. import blendstate
from . import view
from . import font
import weakref
import math


_LAYOUT_VERTICAL = 0
_LAYOUT_HORIZONTAL = 1

_ITEM_STATE_NORMAL = 0
_ITEM_STATE_HIGHLIGHTED = 1
_ITEM_STATE_DISABLED = 2


class ItemStyle:
    """
    item layout with icon
    | left-margin | checkbox | padding | icon | padding | text | text-margin | arrow or shortcut | right-margin |

    item layout without icon
    | left-margin | checkbox | padding | text | padding | arrow or shortcut | right-margin |

    separator horizontal/vertical layout
    | separator-margin | separator-line | separator-margin |

    """

    minimumHeight = 12
    minimumWidth = 48

    topMargin = 4
    bottomMargin = 4
    leftMargin = 2
    rightMargin = 4
    textMargin = 12

    checkmarkSize = 12
    imageSize = 12
    arrowSize = 9
    padding = 2

    separatorColor = core.Color(0.75, 0.75, 0.75)
    separatorMargin = 4
    separatorWidth = 1

    textColor = core.Color(0, 0, 0)
    textColorHighlighted = core.Color(1, 1, 1)
    textColorDisabled = core.Color(0.56, 0.56, 0.56)

    outlineColor = None
    outlineColorHighlighted = None
    outlineColorDisabled = None

    checkmarkColor = textColor
    checkmarkColorHighlighted = textColorHighlighted
    checkmarkColorDisabled = textColorDisabled

    arrowColor = core.Color(0.26, 0.26, 0.26)
    arrowColorHighlighted = core.Color(0.94, 0.94, 0.94)
    arrowColorDisabled = textColorDisabled

    def __init__(self):
        self.font = None


class _ItemContext:
    def __init__(self, menu, style):
        self._menu = weakref.ref(menu)
        self.style = style
        self.displayRect = None

    @property
    def menu(self):
        if self._menu:
            return self._menu()

    def removeFromMenu(self):
        menu = self.menu
        if menu:
            menu.removeItem(self)

    def hitTest(self, pos):
        if self.displayRect and self.displayRect.isInside(pos):
            return True
        return False

    def drawRect(self, renderer, rect, state, layout):
        self.displayRect = rect

    def selectable(self):
        return False


class Item(_ItemContext):
    """
    Menu Item.

    if item has subMenu, item's callback will not invoked.
    """

    textColor = core.Color(0, 0, 0)
    outlineColor = None

    def __init__(self, text, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.callback = None
        self.subMenu = None
        self.__checked = False
        self.__text = text
        self.__enabled = True
        self.__image = None

    @property
    def enabled(self):
        return self.__enabled

    @enabled.setter
    def enabled(self, value):
        v = bool(value)
        if self.__enabled != v:
            self.__enabled = v
            menu = self.menu
            if menu:
                menu.redraw()

    @property
    def text(self):
        return self.__text

    @text.setter
    def text(self, value):
        text = str(value)
        if self.__text != text:
            self.__text = text
            menu = self.menu
            if menu:
                menu.updateLayout()

    @property
    def image(self):
        return self.__image

    @image.setter
    def image(self, value):
        self.__image = value
        menu = self.menu
        if menu:
            menu.updateLayout()

    @property
    def checked(self):
        return self.__checked

    @checked.setter
    def checked(self, value):
        self.__checked = bool(value)
        menu = self.menu
        if menu:
            menu.redraw()

    def selectable(self):
        '''
        determine whether item is selectable or not.
        items must be enabled, has subMenu or callable to be selectable.
        '''
        if self.enabled:
            if self.subMenu or self.callback:
                return True
        return super().selectable()

    def calculateFrameSize(self, layout, scale):
        style = self.style
        if style.font:
            h = style.font.textFont.lineHeight() * scale
            w = style.font.textFont.lineWidth(self.__text) * scale
            h += style.topMargin + style.bottomMargin
            w += style.checkmarkSize + style.padding + style.textMargin + style.arrowSize + style.leftMargin + style.rightMargin
            if self.__image:
                w += style.imageSize + style.padding
            return math.ceil(w), math.ceil(h)
        return self.style.minimumWidth, self.style.minimumHeight

    def drawRect(self, renderer, rect, state, layout):
        super().drawRect(renderer, rect, state, layout)

        style = self.style
        if style.font:
            if not self.selectable() or state == _ITEM_STATE_DISABLED:
                textColor = style.textColorDisabled
                outlineColor = style.outlineColorDisabled
                checkmarkColor = style.checkmarkColorDisabled
                arrowColor = style.arrowColorDisabled
            elif state == _ITEM_STATE_HIGHLIGHTED:
                textColor = style.textColorHighlighted
                outlineColor = style.outlineColorHighlighted
                checkmarkColor = style.checkmarkColorHighlighted
                arrowColor = style.arrowColorHighlighted
            else:
                textColor = style.textColor
                outlineColor = style.outlineColor
                checkmarkColor = style.checkmarkColor
                arrowColor = style.arrowColor

            rc = core.Rect(rect)
            rc.x += style.leftMargin
            rc.y += style.bottomMargin
            rc.width -= style.textMargin + style.arrowSize + style.leftMargin + style.rightMargin
            rc.height -= style.topMargin + style.bottomMargin

            if self.__checked:
                # draw check mark
                y = (rc.height - style.checkmarkSize) * 0.5
                cbRect = core.Rect(rc.x, rc.y + y, style.checkmarkSize, style.checkmarkSize)

                p0 = core.Point(0.15, 0.6)
                p1 = core.Point(0.05, 0.5)
                p2 = core.Point(0.4, 0.35)
                p3 = core.Point(0.4, 0.15)
                p4 = core.Point(0.85, 0.8)
                p5 = core.Point(0.95, 0.7)
                for p in (p0, p1, p2, p3, p4, p5):
                    p.x = p.x * cbRect.width + cbRect.x
                    p.y = p.y * cbRect.height + cbRect.y

                with renderer.contextForSolidTriangleStrip(checkmarkColor, blend=blendstate.defaultOpaque) as r:
                    r.addTriangle(p0, p1, p2)
                    r.add(p3)
                    r.add(p4)
                    r.add(p5)


            offset = style.checkmarkSize + style.padding
            rc.x += offset
            rc.width -= offset

            if self.__image:
                y = (rc.height - style.imageSize) * 0.5
                rc2 = core.Rect(rc.x, rc.y + y, style.imageSize, style.imageSize)
                with renderer.contextForTexturedRects(self.__image, blend=blendstate.defaultAlpha) as r:
                    r.add(rc2, core.Matrix3())
                offset = style.imageSize + style.padding
                rc.x += offset
                rc.width -= offset

            font.drawText(renderer, rc, self.__text, style.font,
                          textColor, outlineColor,
                          align=font.ALIGN_BOTTOM_LEFT,
                          linebreak=font.LINE_BREAK_TRUNCATING_TAIL)

            #draw arrow!
            if self.subMenu:
                if layout == _LAYOUT_HORIZONTAL:
                    arRect = core.Rect(rect.x + rect.width - style.arrowSize - style.rightMargin,
                                       rect.y + style.bottomMargin,
                                       style.arrowSize, style.arrowSize)
                    x1 = arRect.x
                    x2 = arRect.x + arRect.width * 0.5
                    x3 = arRect.x + arRect.width
                    y1 = arRect.y
                    y2 = arRect.y + arRect.height * 0.5
                    pos = (core.Point(x1, y2), core.Point(x2, y1), core.Point(x3, y2))
                else:
                    y = (rc.height - style.arrowSize) * 0.5
                    arRect = core.Rect(rect.x + rect.width - style.arrowSize - style.rightMargin,
                                       rc.y + y,
                                       style.arrowSize,
                                       style.arrowSize)

                    # x1 = arRect.x + arRect.width * 0.5
                    # x2 = arRect.x + arRect.width

                    w = arRect.height * 0.8660254037844388
                    x1 = arRect.x + (arRect.width - w) * 0.5
                    x2 = x1 + w

                    y1 = arRect.y
                    y2 = arRect.y + arRect.height * 0.5
                    y3 = arRect.y + arRect.height
                    pos = (core.Point(x1, y3), core.Point(x1, y1), core.Point(x2, y2))

                if arrowColor:
                    with renderer.contextForSolidTriangles(arrowColor, blend=blendstate.defaultOpaque) as r:
                        r.add(*pos)


        else:
            print('drawRect textFont is missing!!')


class Separator(_ItemContext):
    def calculateFrameSize(self, layout, scale):
        style = self.style
        if layout == _LAYOUT_VERTICAL:
            return style.minimumWidth, style.separatorMargin * 2 + 1
        return style.separatorMargin * 2 + 1, style.minimumHeight

    def drawRect(self, renderer, rect, state, layout):
        super().drawRect(renderer, rect, state, layout)

        style = self.style
        if layout == _LAYOUT_VERTICAL:
            w = rect.width - style.separatorMargin * 2
            h = style.separatorWidth
        else:
            w = style.separatorWidth
            h = rect.height - style.separatorMargin * 2

        x = (rect.width - w) * 0.5
        y = (rect.height - h) * 0.5

        rc = core.Rect(rect.x + x, rect.y + y, w, h)
        with renderer.contextForSolidRects(style.separatorColor) as r:
            r.add(rc)


class Menu(view.View):

    minimumViewHeight = 16
    minimumViewWidth = 16

    fontAttributes = font.attributes(12)

    backgroundColor = core.Color(0.94, 0.94, 0.94)
    backgroundColorHighlighted = core.Color(0.2, 0.2, 1)

    borderColor = core.Color(0, 0, 0)
    borderWidth = 1

    margin = 2          # left,right,bottom,up margin
    itemPadding = 1     # padding between menu items

    verticalLayout = False

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.style = ItemStyle()
        self.__items = []
        self.__highlightedItem = None
        self.__selectedItem = None
        self.__popup = False
        self.__popupChild = None
        self.__capturedMouseId = None
        self.__capturedMouseDown = False
        self.__autoExpand = False

    def items(self):
        return tuple(self.__items)

    def findItem(self, text):
        for item in self.__items:
            try:
                if item.text == text:
                    return item
            except AttributeError:
                pass

    def removeItem(self, item):
        if item.menu != self:
            raise ValueError('Invalid menu item object.')

        self.__items.remove(item)
        item.style = None
        item._menu = None
        self.__highlightedItem = None
        self.redraw()

    def addItem(self, text):
        item = Item(text, menu=self, style=self.style)
        self.__items.append(item)
        self.__highlightedItem = None
        self.redraw()
        return item

    def addSeparator(self):
        item = Separator(menu=self, style=self.style)
        self.__items.append(item)
        self.__highlightedItem = None
        self.redraw()
        return item

    def popupOnScreen(self, screen, origin, parentMenu=None):
        if self.parent() is None:
            self.unload()
            self.__popup = True
            self.__autoExpand = True
            self.verticalLayout = True

            root = screen.frame
            root.addChild(self)
            self.load(screen)   # load font to calculate frame

            size = self.calculateFrameSize()
            sizeInPixel = self.convertLocalToPixel(size)
            sizeInRoot = root.convertPixelToLocal(sizeInPixel)

            origin.y -= sizeInRoot.height

            originInPixel = root.convertLocalToPixel(origin)
            originInPixel.x = math.floor(originInPixel.x + 0.5)
            originInPixel.y = math.floor(originInPixel.y + 0.5)
            originInRoot = root.convertPixelToLocal(originInPixel)

            frame = core.Rect(originInRoot, sizeInRoot)

            bounds = root.contentDisplayBounds()
            if frame.x < bounds.x:
                frame.x = bounds.x
            if frame.y < bounds.y:
                frame.y = bounds.y
            if frame.x + frame.width > bounds.x + bounds.width:
                frame.x = bounds.x + bounds.width - frame.width
            if frame.y + frame.height > bounds.y + bounds.height:
                frame.y = bounds.y + bounds.height - frame.height

            self.frame = frame
            self.redraw()

            if not isinstance(parentMenu, Menu):
                screen.postOperation(self._popupMouseSetup, ())
            return self

    def itemHitTest(self, pos):
        bounds = self.contentDisplayBounds()
        if bounds.isInside(pos):
            for item in self.__items:
                if item.hitTest(pos):
                    return item

    def popupHitTest(self, pos):
        if self.parent():
            if self.__popupChild:
                posInRoot = self.convertLocalToRoot(pos)
                posInLocal = self.__popupChild.convertRootToLocal(posInRoot)
                result = self.__popupChild.popupHitTest(posInLocal)
                if result:
                    return result

            bounds = self.contentDisplayBounds()
            if bounds.isInside(pos):
                return self

    def _popupMouseSetup(self):
        if self.__capturedMouseId is None:
            self.captureMouse(0)
            self.__capturedMouseId = 0
            self.__capturedMouseDown = False

    def onLoaded(self):
        super().onLoaded()
        self.style.font = self.font

    def onUnload(self):
        self.style.font = None
        self.__highlightedItem = None
        self.__selectedItem = None
        super().onUnload()

    def _calculatePopupFrameFromItem(self, item):
        parent = item.menu

        root = parent.screen().frame
        rect = item.displayRect

        bounds = root.contentDisplayBounds()

        size = self.calculateFrameSize()
        sizeInPixel = self.convertLocalToPixel(size)
        sizeInRoot = root.convertPixelToLocal(sizeInPixel)

        if parent.verticalLayout:
            left = parent.convertLocalToRoot(core.Point(rect.x, rect.y + rect.height))
            left.x -= sizeInRoot.width
            right = parent.convertLocalToRoot(core.Point(rect.x + rect.width, rect.y + rect.height))

            if right.x + sizeInRoot.width <= bounds.x + bounds.width:
                origin = right
            elif left.x >= bounds.x:
                origin = left
            else:
                d1 = (right.x + sizeInRoot.width) - (bounds.x + bounds.width)
                d2 = bounds.x - left.x
                origin = left if d1 > d2 else right
            origin.y -= sizeInRoot.height
        else:
            top = parent.convertLocalToRoot(core.Point(rect.x, rect.y + rect.height))
            bottom = parent.convertLocalToRoot(core.Point(rect.x, rect.y))
            bottom.y -= sizeInRoot.height

            if bottom.y >= bounds.y:
                origin = bottom
            elif top.y + size.height <= bounds.y + bounds.height:
                origin = top
            else:
                d1 = bounds.y - bottom.y
                d2 = (top.y + size.height) - (bounds.y + bounds.height)
                origin = top if d1 > d2 else bottom

        originInPixel = root.convertLocalToPixel(origin)
        originInPixel.x = math.floor(originInPixel.x + 0.5)
        originInPixel.y = math.floor(originInPixel.y + 0.5)
        origin = root.convertPixelToLocal(originInPixel)

        frame = core.Rect(origin, sizeInRoot)
        # frame.x = math.floor(frame.x * self.scaleFactor + 0.5) / self.scaleFactor
        # frame.y = math.floor(frame.y * self.scaleFactor + 0.5) / self.scaleFactor

        if frame.x < bounds.x:
            frame.x = bounds.x
        if frame.y < bounds.y:
            frame.y = bounds.y
        if frame.x + frame.width > bounds.x + bounds.width:
            frame.x = bounds.x + bounds.width - frame.width
        if frame.y + frame.height > bounds.y + bounds.height:
            frame.y = bounds.y + bounds.height - frame.height

        return frame

    def calculateFrameSize(self):
        style = self.style
        padding = self.itemPadding
        scale = 1.0 / self.scaleFactor
        numItems = len(self.__items)
        itemPadding = padding * (numItems - 1) if numItems > 0 else 0

        width = style.minimumWidth
        height = style.minimumHeight

        if self.__popup or self.verticalLayout:
            height = 0
            for item in self.__items:
                w, h = item.calculateFrameSize(_LAYOUT_VERTICAL, scale)
                width = max(width, w)
                height += h
            height = max(height + itemPadding, style.minimumHeight)
        else:
            width = 0
            for item in self.items():
                w, h = item.calculateFrameSize(_LAYOUT_HORIZONTAL, scale)
                width += w
                height = max(height, h)
            width = max(width + itemPadding, style.minimumWidth)

        width += self.margin * 2
        height += self.margin * 2
        return core.Size(width + self.borderWidth * 2, height + self.borderWidth * 2)

    def getResolution(self):
        if self.isPopup():
            s = self.calculateFrameSize()
            self.contentScale = s
            return s.width * self.scaleFactor, s.height * self.scaleFactor
        return super().getResolution()

    def onRender(self, renderer):
        super().onRender(renderer)

        bounds = self.contentBounds()

        itemPadding = self.itemPadding
        offsetX = bounds.x + self.margin
        width = bounds.width - self.margin * 2
        height = bounds.height - self.margin * 2

        scale = 1.0 / self.scaleFactor
        state = _ITEM_STATE_NORMAL if self.enabled else _ITEM_STATE_DISABLED

        if self.__popup or self.verticalLayout:
            offsetY = bounds.y + bounds.height - self.margin
            for item in self.__items:
                w, h = item.calculateFrameSize(_LAYOUT_VERTICAL, scale)
                offsetY -= h
                rc = core.Rect(offsetX, offsetY, max(w, width), h)
                itemState = state
                if self.__highlightedItem == item and item.selectable() and itemState == _ITEM_STATE_NORMAL:
                    itemState = _ITEM_STATE_HIGHLIGHTED
                    with renderer.contextForSolidRects(self.backgroundColorHighlighted) as r:
                        r.add(rc)
                item.drawRect(renderer, rc, itemState, _LAYOUT_VERTICAL)
                offsetY -= itemPadding
        else:
            offsetY = bounds.y + self.margin
            for item in self.__items:
                w, h = item.calculateFrameSize(_LAYOUT_HORIZONTAL, scale)
                rc = core.Rect(offsetX, offsetY, w, max(h, height))
                itemState = state
                if self.__highlightedItem == item and item.selectable() and itemState == _ITEM_STATE_NORMAL:
                    itemState = _ITEM_STATE_HIGHLIGHTED
                    with renderer.contextForSolidRects(self.backgroundColorHighlighted) as r:
                        r.add(rc)
                item.drawRect(renderer, rc, itemState, _LAYOUT_HORIZONTAL)
                offsetX = offsetX + w + itemPadding


    def onUpdate(self, delta, tick, date):
        super().onUpdate(delta, tick, date)

    def updateLayout(self):
        if self.__popup:
            frame = self.frame
            frame.size = self.calculateFrameSize()
            self.frame = frame
        self.redraw()

    def isPopup(self):
        return self.__popup

    def dismiss(self):
        if self.__popupChild:
            self.__popupChild.dismiss()
        self.__popupChild = None
        self.__capturedMouseId = None
        self.__capturedMouseDown = False
        self.releaseAllMiceCapturedBySelf()
        if self.isPopup():
            self.removeFromParent(unload=True)
        else:
            self.__highlightedItem = None
            self.__autoExpand = False
            self.redraw()

    def setItemActivate(self, item):

        if not isinstance(item, Item) or not item.selectable():
            item = None

        if self.__highlightedItem != item:
            self.__highlightedItem = item
            self.redraw()

        if item and self.__autoExpand:
            menu = item.subMenu
            if menu:
                if self.__popupChild != menu:
                    if self.__popupChild:
                        self.screen().postOperation(self.__popupChild.dismiss, ())
                    self.__popupChild = menu.popupOnScreen(self.screen(), core.Point(0, 0), self)
                self.__popupChild.frame = self.__popupChild._calculatePopupFrameFromItem(item)
            else:
                if self.__popupChild:
                    self.screen().postOperation(self.__popupChild.dismiss, ())
                    self.__popupChild = None

        elif self.__popupChild:
            self.screen().postOperation(self.__popupChild.dismiss, ())
            self.__popupChild = None


    def onMouseLeave(self, deviceId):
        super().onMouseLeave(deviceId)
        if deviceId == 0 and not self.isMouseCapturedBySelf(0):
            if self.__popupChild is None:
                if self.__highlightedItem:
                    self.__highlightedItem = None
                    self.redraw()


    def onMouseHover(self, deviceId):
        super().onMouseHover(deviceId)

    def onMouseDown(self, deviceId, buttonId, pos):
        super().onMouseDown(deviceId, buttonId, pos)

        if buttonId == 0:
            if not self.isPopup():
                item = self.itemHitTest(pos)
                if item and item.selectable():
                    if self.__capturedMouseId is None:
                        self.captureMouse(deviceId)
                        self.__capturedMouseId = deviceId
                        self.__capturedMouseDown = True
                        self.__autoExpand = True
                    else:
                        if self.__capturedMouseId == deviceId:
                            self.__autoExpand = not self.__autoExpand
                        elif not self.__capturedMouseDown:
                            self.captureMouse(deviceId)
                            self.__capturedMouseId = deviceId
                            self.__capturedMouseDown = True
                            self.__autoExpand = True

        item = None
        menu = self.popupHitTest(pos)
        if menu:
            if self.__capturedMouseId == deviceId and buttonId == 0:
                posInRoot = self.convertLocalToRoot(pos)
                posInLocal = menu.convertRootToLocal(posInRoot)
                item = menu.itemHitTest(posInLocal)
                if item:
                    menu.setItemActivate(item)
        else:
            self.screen().postOperation(self.dismiss, ())


    def onMouseUp(self, deviceId, buttonId, pos):
        super().onMouseUp(deviceId, buttonId, pos)

        if self.__capturedMouseId == deviceId and buttonId == 0:
            if self.isMouseCapturedBySelf(deviceId):
                menu = self.popupHitTest(pos)
                if menu:
                    posInRoot = self.convertLocalToRoot(pos)
                    posInLocal = menu.convertRootToLocal(posInRoot)
                    item = menu.itemHitTest(posInLocal)
                    if item and item.selectable():
                        if item.subMenu is None:
                            self.screen().postOperation(self.dismiss, ())
                            if item.callback:
                                self.screen().postOperation(item.callback, (item,))

                self.__capturedMouseDown = False
                if not self.isPopup() and not self.__autoExpand:
                    hItem = self.__highlightedItem
                    self.dismiss()
                    self.__highlightedItem = hItem
            else:   # mouse lost?
                self.screen().postOperation(self.dismiss, ())

    def onMouseMove(self, deviceId, pos, delta):
        super().onMouseMove(deviceId, pos, delta)
        updateItem = False

        if self.__capturedMouseId == deviceId:
            if self.isMouseCapturedBySelf(deviceId):
                updateItem = True
            else:
                self.__capturedMouseId = None       # mouse lost?
        elif deviceId == 0 and self.__capturedMouseId is None:
            updateItem = True

        if updateItem:
            menu = self.popupHitTest(pos)
            if menu:
                posInRoot = self.convertLocalToRoot(pos)
                posInLocal = menu.convertRootToLocal(posInRoot)
                item = menu.itemHitTest(posInLocal)
                if item:
                    menu.setItemActivate(item)
            elif not self.__popupChild:
                self.setItemActivate(None)

    def onMouseLost(self, deviceId):
        super().onMouseLost(deviceId)
        self.screen().postOperation(self.dismiss, ())
