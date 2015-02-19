import _dk_core as core
from .. import blendstate
from . import font
from . import scrollview
from . import control
from . import view

import weakref
import math


# hit test result! (return None for out of row-bounds)
_HIT_TEST_RESULT_EXPAND_ICON = 0
# _HIT_TEST_RESULT_LABEL_ICON = 1
# _HIT_TEST_RESULT_TEXT = 2
_HIT_TEST_RESULT_INSIDE = 3

class ItemStyle:

    indentationLevelWidth = 15
    expandIconSize = 12
    padding = 2

    topMargin = 1
    leftMargin = 4
    rightMargin = 1
    bottomMargin = 1

    fontAttributes = font.attributes(14, embolden=0)

    backgroundColor = None
    backgroundColorHighlighted = core.Color(1, 1, 0)
    backgroundColorActivated = core.Color(0.25, 0.25, 1.0)
    backgroundColorDisabled = core.Color(0.9, 0.9, 0.9)

    textColor = core.Color(0, 0, 0)
    textColorHighlighted = core.Color(0, 0, 0)
    textColorActivated = core.Color(1, 1, 1)
    textColorDisabled = core.Color(0.5, 0.5, 0.5)

    outlineColor = None
    outlineColorHighlighted = None
    outlineColorActivated = core.Color(0, 0, 0, 1)
    outlineColorDisabled = None

    backgroundColorBlend = blendstate.defaultOpaque

    arrowColor = core.Color(0.5, 0.5, 0.5)
    arrowColorActivated = core.Color(0.25, 0.25, 1.0)

    iconTransform = core.Matrix3()

    scaleFactor = view.DEFAULT_UI_SCALE


    def __init__(self):
        self.font = None


class Item:
    
    STATE_NORMAL = control.Control.STATE_NORMAL
    STATE_HIGHLIGHTED = control.Control.STATE_HIGHLIGHTED
    STATE_ACTIVATED = control.Control.STATE_ACTIVATED
    STATE_DISABLED = control.Control.STATE_DISABLED

    style = ItemStyle()

    def __init__(self, text, image, parent, treeControl):
        self.__parent = None
        self.__control = None
        self.labelText = text
        self.labelImage = image
        self.editable = False
        self.expanded = False
        self.children = []
        self.parent = parent
        self.control = treeControl
        self.state = self.STATE_NORMAL

    def __repr__(self):
        state = 'UNKNOWN or ERROR'
        if self.state == Item.STATE_NORMAL:
            state = 'STATE_NORMAL'
        if self.state == Item.STATE_HIGHLIGHTED:
            state = 'STATE_HIGHLIGHTED'
        if self.state == Item.STATE_ACTIVATED:
            state = 'STATE_ACTIVATED'
        if self.state == Item.STATE_DISABLED:
            state = 'STATE_DISABLED'
        return 'TreeItem(\'{}\' state={})'.format(self.labelText, state)

    @property
    def parent(self):
        if self.__parent:
            return self.__parent()

    @parent.setter
    def parent(self, value):
        if value:
            assert isinstance(value, Item)
            self.__parent = weakref.ref(value)
        else:
            self.__parent = None

    @property
    def control(self):
        if self.__control:
            return self.__control()

    @control.setter
    def control(self, value):
        if value:
            assert isinstance(value, TreeControl)
            self.__control = weakref.ref(value)
        else:
            self.__control = None

    @property
    def enabled(self):
        return self.state != Item.STATE_DISABLED

    @enabled.setter
    def enabled(self, value):
        if not bool(value):
            self.state = Item.STATE_DISABLED
        elif self.state == Item.STATE_DISABLED:
            self.state = Item.STATE_NORMAL

    def removeFromParent(self):
        parent = self.parent
        if parent:
            idx = parent.children.index(self)
            del parent.children[idx]
        else:
            ctrl = self.control
            if ctrl:
                idx = ctrl.items.index(self)
                del ctrl.items[idx]

        self.parent = None
        self.control = None

    def removeAllChildren(self):
        for c in self.children:
            c.removeAllChildren()
            c.parent = None
            c.control = None
        self.children.clear()

    def expandAll(self):
        self.expanded = True
        for c in self.children:
            c.expandAll()

    def collapseAll(self):
        self.expanded = False
        for c in self.children:
            c.collapseAll()

    def hitTest(self, pos):
        try:
            self._expandIconRect
            self._labelIconRect
            self._labelTextRect
            self._rowRect
            self._groupRect
        except AttributeError:
            pass
        else:
            if self._groupRect.isInside(pos):
                if self._rowRect.isInside(pos):
                    if len(self.children) > 0:
                        if self._expandIconRect.isInside(pos):
                            return self, _HIT_TEST_RESULT_EXPAND_ICON

                    # if self._labelIconRect.isInside(pos):
                    #     return self, _HIT_TEST_RESULT_LABEL_ICON
                    # if self._labelTextRect.isInside(pos):
                    #     return self, _HIT_TEST_RESULT_TEXT
                    return self, _HIT_TEST_RESULT_INSIDE

                for c in self.children:
                    r = c.hitTest(pos)
                    if r:
                        return r

    def _updateBounds(self, indent, offset, visibleItems):

        visibleItems.add(self)

        style = self.style

        layoutFont = style.font.textFont
        labelTextWidth = (layoutFont.lineWidth(self.labelText) + layoutFont.outline * 2) / style.scaleFactor
        labelTextHeight = (layoutFont.lineHeight() + layoutFont.baseline) / style.scaleFactor

        if self.labelImage:
            labelIconWidth = self.labelImage.width / style.scaleFactor
            labelIconHeight = self.labelImage.height / style.scaleFactor
        else:
            labelIconWidth = 0
            labelIconHeight = 0

        contentHeight = max(style.expandIconSize, labelIconHeight, labelTextHeight)
        contentHeight = math.ceil(contentHeight)

        rowHeight = contentHeight + style.topMargin + style.bottomMargin

        x = offset.x + indent * style.indentationLevelWidth + style.leftMargin
        y = offset.y - (contentHeight + style.topMargin)

        self._expandIconRect = core.Rect(x, y + (contentHeight - style.expandIconSize) * 0.5,
                                         style.expandIconSize, style.expandIconSize)

        x += style.expandIconSize + style.padding

        self._labelIconRect = core.Rect(x, y + (contentHeight - labelIconHeight) * 0.5, labelIconWidth, labelIconHeight)
        x += labelIconWidth + style.padding

        self._labelTextRect = core.Rect(x, y + (contentHeight - labelTextHeight) * 0.5, labelTextWidth, labelTextHeight)

        self._rowRect = core.Rect(offset.x, offset.y - rowHeight,
                                  x + labelTextWidth + style.rightMargin, rowHeight)

        maxHeight = rowHeight
        maxWidth = self._rowRect.width
        offset.y -= rowHeight

        if self.expanded:
            for item in self.children:
                w, h = item._updateBounds(indent+1, offset, visibleItems)
                maxHeight += h
                maxWidth = max(maxWidth, w)

        self._groupRect = core.Rect(offset.x, offset.y, maxWidth, maxHeight)
        return maxWidth, maxHeight

    def hasSelectedDescendant(self):
        if self.state == Item.STATE_ACTIVATED:
            return True
        for item in self.children:
            if item.hasSelectedDescendant():
                return True
        return False

    def _draw(self, renderer, width):
        try:
            self._expandIconRect
            self._labelIconRect
            self._labelTextRect
            self._rowRect
            self._groupRect
        except AttributeError:
            pass
        else:
            self._rowRect.width = width
            self._groupRect.width = width

            style = self.style
            state = self.state

            textColor = (style.textColor,
                         style.textColorHighlighted,
                         style.textColorActivated,
                         style.textColorDisabled)[state]

            outlineColor = (style.outlineColor,
                            style.outlineColorHighlighted,
                            style.outlineColorActivated,
                            style.outlineColorDisabled)[state]

            backgroundColor = (style.backgroundColor,
                               style.backgroundColorHighlighted,
                               style.backgroundColorActivated,
                               style.backgroundColorDisabled)[state]

            if backgroundColor:
                with renderer.contextForSolidRects(backgroundColor, blend=style.backgroundColorBlend) as r:
                    r.add(self._rowRect)

            if len(self.children) > 0:
                if self.expanded:
                    w = self._expandIconRect.width * 0.8660254037844388
                    x1 = self._expandIconRect.x
                    x2 = self._expandIconRect.x + self._expandIconRect.width * 0.5
                    x3 = self._expandIconRect.x + self._expandIconRect.width
                    y1 = self._expandIconRect.y + (self._expandIconRect.height - w) * 0.5
                    y2 = y1 + w
                    pos = (core.Point(x1, y2), core.Point(x2, y1), core.Point(x3, y2))
                else:
                    w = self._expandIconRect.height * 0.8660254037844388
                    x1 = self._expandIconRect.x + (self._expandIconRect.width - w) * 0.5
                    x2 = x1 + w
                    y1 = self._expandIconRect.y
                    y2 = self._expandIconRect.y + self._expandIconRect.height * 0.5
                    y3 = self._expandIconRect.y + self._expandIconRect.height
                    pos = (core.Point(x1, y3), core.Point(x1, y1), core.Point(x2, y2))

                arrowColor = style.arrowColor
                if self.state != Item.STATE_ACTIVATED and self.hasSelectedDescendant():
                    arrowColor = style.arrowColorActivated

                if arrowColor:
                    with renderer.contextForSolidTriangles(arrowColor, blend=blendstate.defaultOpaque) as r:
                        r.add(*pos)

            if self.labelImage:
                with renderer.contextForTexturedRects(self.labelImage) as r:
                    r.add(self._labelIconRect, core.Matrix3())

            font.drawText(renderer,
                          self._labelTextRect,
                          self.labelText,
                          self.style.font,
                          textColor,
                          outlineColor,
                          align=font.ALIGN_BOTTOM_LEFT,
                          linebreak=font.LINE_BREAK_TRUNCATING_TAIL)


            if self.expanded:
                for item in self.children:
                    item._draw(renderer, width)


class TreeControl(control.Control):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.__items = []
        self._revision = 0      # for lazy update

    def items(self):
        return self.__items[:]

    @property
    def _items(self):
        return self.__items

    def addItem(self, labelText, labelIcon=None, parent=None):
        item = Item(labelText, labelIcon, parent, self)
        if parent:
            assert isinstance(parent, Item)
            if parent.control != self:
                raise ValueError('Parent object is not belongs to this control')
            parent.children.append(item)
        else:
            self.__items.append(item)

        self._revision += 1
        return item

    def removeAllItems(self):
        for c in self.__items:
            c.removeAllChildren()
        self.__items.clear()
        self.updateItems()

    def removeItem(self, item):
        assert isinstance(item, Item)
        ctrl = item.control
        assert ctrl == self
        item.removeFromParent()
        self.updateItems()

    def updateItems(self):
        self._revision += 1

    def expandAllItems(self):
        for item in self.__items:
            item.expandAll()
        self.updateItems()

    def collapseAllItems(self):
        for item in self.__items:
            item.collapseAll()
        self.updateItems()


class TreeView(TreeControl, scrollview.ScrollView):

    # override ScrollView properties
    borderWidth = 1
    scrollBarBorderWidth = 1
    showVerticalScrollBar = True
    showHorizontalScrollBar = True
    showZoomButton = True
    leftScrollBar = False


    # tree view events
    # event procedure -> def handler(tree-view_object, event_id, item_object)
    EVENT_ITEM_EXPANDED = 1          # click on item's collapse arrow
    EVENT_ITEM_COLLAPSED = 2         # click on item's expand arrow
    EVENT_ITEM_STATE_CHANGED = 3     # item's state has been changed. (while selection)
    EVENT_ITEM_SELECTION_CHANGED = 4 # item selection changed. (on mouse down or click-move)
    EVENT_ITEM_SELECTED = 5          # item selected. (after mouse up)

    resetItemStateWhenCollapsed = False



    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.itemStyle = ItemStyle()
        self.__revisionU = None

        self.__selectedItem = None
        self.__hoverItem = None      # mouse-hover for deviceId == 0
        self.__itemDisplayOffset = core.Point(0, 0)

    def addItem(self, labelText, labelIcon=None, parent=None):
        item = super().addItem(labelText, labelIcon, parent)
        item.style = self.itemStyle
        return item

    def onLoaded(self):
        super().onLoaded()
        self.itemStyle.scaleFactor = self.scaleFactor
        self.itemStyle.font = font.loadUIFont(self.itemStyle.fontAttributes, self.scaleFactor)

        self._updateItemRect()
        self.contentOffset = core.Point(0, self.contentSize.height)


    def onUnload(self):
        super().onUnload()
        self.itemStyle.font = None

    def _updateItemRect(self):

        print('_updateItemRect')

        visibleItems = set()

        offsetY = self.contentSize.height - self.contentOffset.y

        offset = core.Point(0,0)
        width, height = 0, 0
        for c in self._items:
            w, h = c._updateBounds(0, offset, visibleItems)
            width = max(width, w)
            height += h

        bounds = self.contentBounds()
        scaleX = bounds.width / width if width > 0 else 1.0
        scaleY = bounds.height / height if height > 0 else 1.0

        self.minimumZoomScale = max( min(scaleX, scaleY, 1.0), 0.1)
        self.contentSize = core.Size(width, height)
        self.contentOffset.y = height - offsetY
        self.updateContentTransform()
        self.redraw()
        # print('self.contentSize: ', self.contentSize)

        if self.resetItemStateWhenCollapsed:
            if self.__hoverItem and self.__hoverItem.state == Item.STATE_HIGHLIGHTED:
                if self.__hoverItem not in visibleItems:
                    self.__hoverItem.state = Item.STATE_NORMAL
                    self._postEvent(self.EVENT_ITEM_STATE_CHANGED, self.__hoverItem)
                    self.__hoverItem = None

            if self.__selectedItem and self.__selectedItem.state == Item.STATE_ACTIVATED:
                if self.__selectedItem not in visibleItems:
                    self.__selectedItem.state = Item.STATE_NORMAL
                    self._postEvent(self.EVENT_ITEM_SELECTED, self.__selectedItem)
                    self.__selectedItem = None


    def onUpdate(self, delta, tick, date):
        super().onUpdate(delta, tick, date)

        if self.__revisionU != self._revision:
            self._updateItemRect()
            self.redraw()

            self.__revisionU = self._revision

    def onRender(self, renderer):
        super().onRender(renderer)

        bounds = self.contentBounds()
        offset = max(self.contentSize.height * self.zoomScale, bounds.height)

        visibleRect = self.contentDisplayBounds()
        width = visibleRect.x + visibleRect.width

        self.__itemDisplayOffset = core.Point(0, 0)
        self.__itemDisplayOffset.y += max(self.contentSize.height, bounds.height / self.zoomScale)

        rb = renderer.bounds
        renderer.bounds = (rb[0], rb[1] - offset, rb[2], rb[3])

        for item in self._items:
            item._draw(renderer, width)

        renderer.bounds = rb

    def _postEvent(self, event, item):
        self.screen().postOperation(self.invokeAllTargets, (self, event, item))

    def _itemHitTest(self, pos):
        if self.contentDisplayBounds().isInside(pos):
            pos = pos - self.__itemDisplayOffset
            for c in self._items:
                hr = c.hitTest(pos)
                if hr:
                    # print('hr:', hr)
                    return hr
            # print('_itemHitTest result None')
        return None, None

    def onMouseDown(self, deviceId, buttonId, pos):
        super().onMouseDown(deviceId, buttonId, pos)

        bounds = self.contentDisplayBounds()
        if bounds.isInside(pos):
            # print('onMouseDown:',pos)
            # print('onMouseDown 1st item:', self._items[0]._rowRect)
            # print('onMouseDown offset:', self.__itemDisplayOffset)

            try:
                self.__itemSelectionMouseId
                self.__itemSelectionHitResult
                self.__itemSelectionDidChanged
            except AttributeError:
                selectedItem, hitResult = self._itemHitTest(pos)

                prevSelected = self.__selectedItem
                itemStates = []

                def clearPreviousSelection():
                    if self.__selectedItem and self.__selectedItem.state == Item.STATE_ACTIVATED:
                        itemStates.append((self.__selectedItem, self.__selectedItem.state))
                        self.__selectedItem.state = Item.STATE_NORMAL
                    self.__selectedItem = None

                if selectedItem:
                    if hitResult == _HIT_TEST_RESULT_EXPAND_ICON or selectedItem.state != Item.STATE_DISABLED:
                        self.captureMouse(deviceId)
                        self.__itemSelectionMouseId = (deviceId, buttonId)
                        self.__itemSelectionHitResult = (selectedItem, hitResult)
                        if hitResult != _HIT_TEST_RESULT_EXPAND_ICON and selectedItem is not self.__selectedItem:
                            clearPreviousSelection()
                            itemStates.append((selectedItem, selectedItem.state))
                            selectedItem.state = Item.STATE_ACTIVATED
                            self.__selectedItem = selectedItem
                elif self.__selectedItem and self.__selectedItem.state != Item.STATE_DISABLED:
                    itemStates.append((self.__selectedItem, self.__selectedItem.state))
                    self.__selectedItem.state = Item.STATE_NORMAL
                    self.__selectedItem = None

                for item, state in itemStates:
                    if item.state != state:
                        self._postEvent(self.EVENT_ITEM_STATE_CHANGED, item)
                        self.redraw()

                self.__itemSelectionDidChanged = prevSelected is not self.__selectedItem
                if self.__itemSelectionDidChanged:
                    if not self.__selectedItem:
                        self._postEvent(self.EVENT_ITEM_SELECTED, None)
                    else:
                        self._postEvent(self.EVENT_ITEM_SELECTION_CHANGED, self.__selectedItem)
                    self.redraw()



    def onMouseMove(self, deviceId, pos, delta):
        super().onMouseMove(deviceId, pos, delta)

        prevSelected = self.__selectedItem
        itemStates = []

        if self.isMouseCapturedBySelf(deviceId):
            try:
                self.__itemSelectionMouseId
                self.__itemSelectionHitResult
                self.__itemSelectionDidChanged
            except AttributeError:
                pass
            else:
                if self.__itemSelectionMouseId[0] == deviceId and self.__itemSelectionHitResult[1] != _HIT_TEST_RESULT_EXPAND_ICON:
                    selectedItem, hitResult = self._itemHitTest(pos)

                    if selectedItem is not self.__selectedItem:
                        self.__itemSelectionDidChanged = True
                        if self.__selectedItem:
                            if self.__selectedItem.state != Item.STATE_DISABLED:
                                itemStates.append((self.__selectedItem, self.__selectedItem.state))
                                self.__selectedItem.state = Item.STATE_NORMAL
                            self.__selectedItem = None

                        if selectedItem and selectedItem.state != Item.STATE_DISABLED:
                            itemStates.append((selectedItem, selectedItem.state))
                            selectedItem.state = Item.STATE_ACTIVATED
                            self.__selectedItem = selectedItem
                        else:
                            self.__selectedItem = None

        elif deviceId == 0:     # detect-hover only deviceId == 0
            prevItem = self.__hoverItem
            hoverItem, hitResult = self._itemHitTest(pos)

            if prevItem is not hoverItem:
                if prevItem:
                    if prevItem.state == Item.STATE_HIGHLIGHTED:
                        itemStates.append((prevItem, prevItem.state))
                        prevItem.state = Item.STATE_NORMAL

                if hoverItem:
                    if hoverItem.state == Item.STATE_NORMAL:
                        itemStates.append((hoverItem, hoverItem.state))
                        hoverItem.state = Item.STATE_HIGHLIGHTED
                self.__hoverItem = hoverItem

        for item, state in itemStates:
            if item.state != state:
                self._postEvent(self.EVENT_ITEM_STATE_CHANGED, item)
                self.redraw()

        if prevSelected is not self.__selectedItem:
            self._postEvent(self.EVENT_ITEM_SELECTION_CHANGED, self.__selectedItem)
            self.redraw()

        return super().onMouseMove(deviceId, pos, delta)

    def onMouseUp(self, deviceId, buttonId, pos):
        try:
            self.__itemSelectionMouseId
            self.__itemSelectionHitResult
            self.__itemSelectionDidChanged
        except AttributeError:
            pass
        else:
            if self.__itemSelectionMouseId == (deviceId, buttonId):

                prevSelected = self.__selectedItem
                itemStates = []

                selectedItem, hitResult = self._itemHitTest(pos)

                if self.__itemSelectionHitResult[1] == _HIT_TEST_RESULT_EXPAND_ICON:
                    if self.__itemSelectionHitResult[0] is selectedItem and hitResult == _HIT_TEST_RESULT_EXPAND_ICON:
                        if selectedItem.expanded:
                            self._postEvent(self.EVENT_ITEM_COLLAPSED, selectedItem)
                        else:
                            self._postEvent(self.EVENT_ITEM_EXPANDED, selectedItem)
                        selectedItem.expanded = not selectedItem.expanded
                        # self._updateItemRect()
                        self.updateItems()
                else:
                    if selectedItem is not self.__selectedItem:
                        self.__itemSelectionDidChanged = True
                        if self.__selectedItem and self.__selectedItem.state != Item.STATE_DISABLED:
                            itemStates.append((self.__selectedItem, self.__selectedItem.state))
                            self.__selectedItem.state = Item.STATE_NORMAL
                        if selectedItem.state != Item.STATE_DISABLED:
                            self.__selectedItem = selectedItem
                            itemStates.append((self.__selectedItem, self.__selectedItem.state))
                            self.__selectedItem.state = Item.STATE_ACTIVATED
                        else:
                            self.__selectedItem = None

                        self.redraw()

                for item, state in itemStates:
                    if item.state != state:
                        self._postEvent(self.EVENT_ITEM_STATE_CHANGED, item)
                        self.redraw()

                if self.__itemSelectionHitResult[1] != _HIT_TEST_RESULT_EXPAND_ICON:
                    if self.__itemSelectionDidChanged:
                        self._postEvent(self.EVENT_ITEM_SELECTED, self.__selectedItem)
                        if prevSelected is not self.__selectedItem:
                            self.redraw()

                self.releaseMouse(deviceId)
                del self.__itemSelectionMouseId
                del self.__itemSelectionHitResult
                del self.__itemSelectionDidChanged

        super().onMouseUp(deviceId, buttonId, pos)


    def onMouseHover(self, deviceId):
        super().onMouseHover(deviceId)

    def onMouseLeave(self, deviceId):
        super().onMouseLeave(deviceId)
        if deviceId == 0:
            hoverItem = self.__hoverItem
            if hoverItem:
                if hoverItem.state == Item.STATE_HIGHLIGHTED:
                    hoverItem.state = Item.STATE_NORMAL
                    self.redraw()
                    self._postEvent(self.EVENT_ITEM_STATE_CHANGED, hoverItem)
            self.__hoverItem = None

    def onClickZoomButton(self):
        super().onClickZoomButton()

        offsetX, offsetY = self.maxScrollOffset()
        self.contentOffset.y = offsetY
        self.updateContentTransform()


