import _dk_core as core
from . import view
from . import font
from . import control
from . import textinput


class TextField(textinput.TextInput, control.Control, view.View):

    borderWidth = 1

    caretBlinkInterval = 0.5
    caretWidth = 1

    caretColor = core.Color(0.0, 0.0, 0.0)
    caretColorComposition = core.Color(0.0, 0.0, 0.75)

    selectionColor = core.Color(0.5, 0.5, 1.0)
    selectionColorInactivated = core.Color(0.6, 0.6, 1.0)
    selectionColorDisabled = core.Color(1.0, 0.0, 0.0)

    textColor = core.Color(0.0, 0.0, 0.0)
    textColorInactivated = core.Color(0.3, 0.3, 0.3)
    textColorDisabled = core.Color(0.4, 0.4, 0.4)

    textColorSelected = core.Color(1.0, 1.0, 1.0)
    textColorSelectedInactivated = core.Color(0.9, 0.9, 0.9)
    textColorSelectedDisabled = core.Color(0.4, 0.4, 1.0)

    textColorComposition = core.Color(0.0, 0.0, 0.5)
    textColorCompositionUnderCaret = core.Color(1.0, 1.0, 1.0)

    outlineColor = None
    outlineColorInactivated = None
    outlineColorDisabled = None

    outlineColorSelected = None
    outlineColorSelectedInactivated = None
    outlineColorSelectedDisabled = None

    outlineColorComposition = None
    outlineColorCompositionUnderCaret = None

    backgroundColor = core.Color(1.0, 1.0, 1.0)
    backgroundColorDisabled = core.Color(0.6, 0.6, 0.6)

    fontAttributes = font.attributes(14, kerning=False, file='SeoulNamsanM.ttf')

    keyboardId = 0
    acceptTab = False
    tabSpace = 4

    keyPressingDelay = 0.3
    keyRepeatInterval = 0.04


    def __init__(self, text='', *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.__text = text
        self.__caretPos = 0
        self.__selectionBegin = -1       # selection: from __selectionBegin to __caretPos
        self.__timer = core.Timer()
        self.__editing = False
        self.__caretVisible = False
        self.__capturedMouseId = None
        self.__unmodifiedText = ''

    @property
    def text(self):
        return self.__text

    @text.setter
    def text(self, value):
        self.__text = value
        self.caretPos = self.__caretPos

    @property
    def caretPos(self):
        return self.__caretPos

    @caretPos.setter
    def caretPos(self, value):
        value = max(int(value), 0)
        if self.__caretPos != value:
            print('setCaretPos:', value)
            self.__caretPos = value
            tl = len(self.__text)
            if self.__caretPos > tl:
                self.__caretPos = tl
            self.__selectionBegin = -1
            if self.enabled:
                self.__caretVisible = True
                self.__timer.reset()
            self.updateScroll()
            self.redraw()

    @property
    def editing(self):
        return self.__editing

    @editing.setter
    def editing(self, value):
        b = bool(value)
        if self.__editing != b:
            self.__editing = b
            if self.__editing:
                self.captureKeyboard(self.keyboardId)
                self.screen().window.setTextInputEnabled(self.keyboardId, True)
                self.__caretVisible = True
                self.__unmodifiedText = self.__text
                print('textfield:{} capture keyboard:{}'.format(id(self), self.keyboardId))
            else:
                self.screen().window.setTextInputEnabled(self.keyboardId, False)
                self.releaseKeyboard(self.keyboardId)
                self.__caretVisible = False
                self.__unmodifiedText = ''
                print('textfield:{} release keyboard:{}'.format(id(self), self.keyboardId))
            self.__timer.reset()
            self.redraw()

    def selectionRange(self):
        if self.__selectionBegin >= 0 and self.__selectionBegin != self.__caretPos:
            if self.__selectionBegin > self.__caretPos:
                return self.__caretPos, self.__selectionBegin
            else:
                return self.__selectionBegin, self.__caretPos
        return self.__caretPos, self.__caretPos

    def onLoaded(self):
        super().onLoaded()
        self.minimumViewHeight = self.font.lineHeight() / self.scaleFactor
        self.updateScroll()

    def onUnload(self):
        super().onUnload()

    def onResized(self):
        super().onResized()
        self.updateScroll()

    def onUpdate(self, delta, tick, date):
        super().onUpdate(delta, tick, date)
        if self.__editing and self.enabled:
            if self.__timer.elapsed() > self.caretBlinkInterval:
                self.__caretVisible = not self.__caretVisible
                self.__timer.reset()
                self.redraw()

    def characterIndexAtPosition(self, pos):
        bounds = self.contentBounds()
        x = pos.x - bounds.x
        invScale = 1.0 / self.scaleFactor
        lineWidth = lambda s: self.font.lineWidth(s) * invScale

        if x <= 0:
            return 0
        if x >= lineWidth(self.text):
            return len(self.text)

        width = 0
        width2 = 0
        index = 0
        while width < x:
            width2 = width
            index += 1
            width = lineWidth(self.text[0:index])

        if abs(x - width2) < abs(x - width):
            return index-1
        return index

    def updateScroll(self):
        bounds = self.contentBounds()
        if self.font:
            invScale = 1.0 / self.scaleFactor
            charWidth = self.font.width * invScale

            if bounds.width > charWidth * 2:        # at least 2 characters should be displayed.
                maxX = bounds.width * 0.9
                minX = bounds.width * 0.1
                text = self.__text + self.composingText
                textLen = self.font.lineWidth(text) * invScale
                if textLen > maxX:
                    text = self.__text[0:self.__caretPos] + self.composingText
                    textLen = self.font.lineWidth(text) * invScale

                    transform = core.AffineTransform2(core.Matrix3(self.contentTransform))
                    textLen += transform.translation[0]

                    indent = min(bounds.width * 0.1, charWidth)
                    offset = 0

                    while textLen < minX:
                        textLen += indent
                        offset += indent

                    while textLen > maxX:
                        textLen -= indent
                        offset -= indent

                    if offset != 0:
                        transform.translate(core.Vector2(offset, 0))
                        pos = transform.translation
                        if pos[0] > 0:
                            transform.translation = 0, pos[1]
                        self.contentTransform = transform.matrix3()
                    return
        self.contentTransform = core.Matrix3()


    def onRender(self, renderer):

        invScale = 1.0 / self.scaleFactor

        bounds = self.contentBounds()
        height = self.font.lineHeight() * invScale

        offsetX = bounds.x
        offsetY = int(bounds.y + (bounds.height - height) * 0.5)

        lineWidth = lambda text: self.font.lineWidth(text) * invScale

        drawText = lambda rect, text, tc, oc: \
            font.drawText(renderer, rect, text, self.font, tc, oc, align=font.ALIGN_BOTTOM_LEFT, linebreak=font.LINE_BREAK_CLIPPING)

        if self.enabled:
            super().onRender(renderer)
            if self.isKeyboardCapturedBySelf(self.keyboardId):  # activated
                textColor = self.textColor
                outlineColor = self.outlineColor
                textColorSelected = self.textColorSelected
                outlineColorSelected = self.outlineColorSelected
                selectionColor = self.selectionColor
            else:
                textColor = self.textColorInactivated
                outlineColor = self.outlineColorInactivated
                textColorSelected = self.textColorSelectedInactivated
                outlineColorSelected = self.outlineColorSelectedInactivated
                selectionColor = self.selectionColorInactivated
        else:       # disabled
            tmp = self.backgroundColor
            self.backgroundColor = self.backgroundColorDisabled
            super().onRender(renderer)
            self.backgroundColor = tmp
            self.__caretVisible = False
            textColor = self.textColorDisabled
            outlineColor = self.outlineColorDisabled
            textColorSelected = self.textColorSelectedDisabled
            outlineColorSelected = self.outlineColorSelectedDisabled
            selectionColor = self.selectionColorDisabled


        selectionRange = self.selectionRange()
        if selectionRange[0] != selectionRange[1]:
            left = self.__text[0:selectionRange[0]]
            right = self.__text[selectionRange[1]:]
        else:
            left = self.__text[0:self.__caretPos]
            right = self.__text[self.__caretPos:]


        width = lineWidth(left)
        rc = core.Rect(offsetX, offsetY, width, height)
        drawText(rc, left, textColor, outlineColor)
        offsetX += width

        if selectionRange[0] != selectionRange[1]:
            text = self.__text[selectionRange[0]:selectionRange[1]]
            width = lineWidth(text)
            rc = core.Rect(offsetX, offsetY, width, height)
            with renderer.contextForSolidRects(selectionColor) as r:
                r.add(rc)
            drawText(rc, text, textColorSelected, outlineColorSelected)

            if self.__caretVisible:
                if self.__caretPos > self.__selectionBegin:
                    rc = core.Rect(offsetX + width, offsetY, self.caretWidth, height)
                else:
                    rc = core.Rect(offsetX, offsetY, self.caretWidth, height)
                with renderer.contextForSolidRects(self.caretColor) as r:
                    r.add(rc)
            offsetX += width

        else:
            if len(self.composingText) > 0:
                width = lineWidth(self.composingText)
                rc = core.Rect(offsetX, offsetY, width, height)
                if self.__caretVisible:
                    with renderer.contextForSolidRects(self.caretColorComposition) as r:
                        r.add(rc)
                    drawText(rc, self.composingText, self.textColorCompositionUnderCaret, self.outlineColorCompositionUnderCaret)
                else:
                    drawText(rc, self.composingText, self.textColorComposition, self.outlineColorComposition)

                offsetX += width
            else:
                if self.__caretVisible:
                    rc = core.Rect(offsetX, offsetY, self.caretWidth, height)
                    with renderer.contextForSolidRects(self.caretColor) as r:
                        r.add(rc)

        width = lineWidth(right)
        rc = core.Rect(offsetX, offsetY, width, height)
        drawText(rc, right, textColor, outlineColor)


    def onMouseDown(self, deviceId, buttonId, pos):
        editing = self.__editing
        selectionRange = self.selectionRange()

        self.editing = True

        if self.__capturedMouseId is None and buttonId == 0:
            if editing or selectionRange[0] == selectionRange[1]:
                self.captureMouse(deviceId)
                index = self.characterIndexAtPosition(pos)
                self.__capturedMouseId = deviceId
                self.caretPos = index
                self.redraw()

        return super().onMouseDown(deviceId, buttonId, pos)


    def onMouseUp(self, deviceId, buttonId, pos):
        if buttonId == 0 and self.__capturedMouseId == deviceId:
            self.__capturedMouseId = None
            if self.__selectionBegin == self.__caretPos:
                self.__selectionBegin = -1

        self.releaseMouse(deviceId)
        return super().onMouseUp(deviceId, buttonId, pos)

    def onMouseMove(self, deviceId, pos, delta):
        if self.__capturedMouseId == deviceId:
            index = self.characterIndexAtPosition(pos)
            if self.__selectionBegin < 0:
                self.__selectionBegin = self.__caretPos
            self.__caretPos = index
            self.updateScroll()
            self.redraw()

        return super().onMouseMove(deviceId, pos, delta)

    def onMouseLost(self, deviceId):
        self.__capturedMouseId = None
        return super().onMouseLost(deviceId)

    def onKeyboardLost(self, deviceId):
        print('textfield:{}.onKeyboardLost:{}'.format(id(self), deviceId))
        self.editing = False
        return super().onKeyboardLost(deviceId)

    def moveLeft(self):
        if self.shiftDown:
            if self.__selectionBegin < 0:
                self.__selectionBegin = self.__caretPos
            self.__caretPos = max(self.__caretPos-1, 0)
        else:
            range = self.selectionRange()
            if range[0] == range[1]:
                self.__caretPos = max(self.__caretPos-1, 0)
            else:
                self.__caretPos = range[0]
            self.__selectionBegin = -1
        self.__caretVisible = True
        self.__timer.reset()
        self.updateScroll()
        self.redraw()

    def moveToBeginningOfLine(self):
        if self.shiftDown:
            if self.__selectionBegin < 0:
                self.__selectionBegin = self.__caretPos
            self.__caretPos = 0
        else:
            self.__caretPos = 0
            self.__selectionBegin = -1
        self.__caretVisible = True
        self.__timer.reset()
        self.updateScroll()
        self.redraw()

    def moveRight(self):
        if self.shiftDown:
            if self.__selectionBegin < 0:
                self.__selectionBegin = self.__caretPos
            self.__caretPos = min(len(self.__text), self.__caretPos+1)
        else:
            range = self.selectionRange()
            if range[0] == range[1]:
                self.__caretPos = min(len(self.__text), self.__caretPos+1)
            else:
                self.__caretPos = range[1]
            self.__selectionBegin = -1
        self.__caretVisible = True
        self.__timer.reset()
        self.updateScroll()
        self.redraw()

    def moveToEndOfLine(self):
        super().moveToEndOfLine()
        if self.shiftDown:
            if self.__selectionBegin < 0:
                self.__selectionBegin = self.__caretPos
            self.__caretPos = len(self.__text)
        else:
            self.__caretPos = len(self.__text)
            self.__selectionBegin = -1
        self.__caretVisible = True
        self.__timer.reset()
        self.updateScroll()
        self.redraw()

    def insertText(self, text):
        range = self.selectionRange()
        left = self.__text[0:range[0]]
        right = self.__text[range[1]:]
        if range[0] == range[1]:
            self.__text = left + text + right
            self.__caretPos += len(text)
        else:
            self.__text = left + text
            self.__caretPos = len(self.text)
            self.text += right
        self.__selectionBegin = -1
        self.__caretVisible = True
        self.__timer.reset()
        self.updateScroll()
        self.redraw()

    def setCompositionText(self, text):
        range = self.selectionRange()
        if range[0] != range[1]:
            left = self.__text[0:range[0]]
            right = self.__text[range[1]:]
            self.__text = left + right
        self.__selectionBegin = -1
        self.__caretVisible = True
        self.__timer.reset()
        self.updateScroll()
        self.redraw()

    def deleteBackward(self):
        range = self.selectionRange()
        if range[0] == range[1]:
            if range[0] > 0:
                left = self.__text[0:range[0]-1]
                right = self.__text[range[1]:]
                self.__text = left + right
                self.__caretPos -= 1
        else:
            left = self.__text[0:range[0]]
            right = self.__text[range[1]:]
            self.__text = left + right
            self.__caretPos = range[0]
        self.__selectionBegin = -1
        self.__caretVisible = True
        self.__timer.reset()
        self.updateScroll()
        self.redraw()

    def deleteForward(self):
        range = self.selectionRange()
        if range[0] == range[1]:
            if range[1] < len(self.__text):
                left = self.__text[0:range[0]]
                right = self.__text[range[1]+1:]
                self.__text = left + right
        else:
            left = self.__text[0:range[0]]
            right = self.__text[range[1]:]
            self.__text = left + right
            self.__caretPos = range[0]
        self.__selectionBegin = -1
        self.__caretVisible = True
        self.__timer.reset()
        self.updateScroll()
        self.redraw()

    def processEscape(self):
        if self.editing:
            self.__text = self.__unmodifiedText
            self.__caretPos = 0
            self.__selectionBegin = -1
            self.editing = False
            self.updateScroll()
            # post notification
            print('User cancelled editing. (post notification)')

    def processCarriageReturn(self):
        if self.editing:
            self.__caretPos = 0
            self.__selectionBegin = -1
            self.editing = False
            self.updateScroll()
            # post notification!
            print('User finish editing. (post notification)')

    def processLineFeed(self):
        self.processCarriageReturn()

    def insertTab(self):
        self.processCarriageReturn()
