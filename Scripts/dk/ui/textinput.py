import _dk_core as core
from .. import vkey
from . import view


class TextHandler:

    def moveLeft(self):
        print('moveLeft')

    def moveRight(self):
        print('moveRight')

    def moveUp(self):
        print('moveUp')

    def moveDown(self):
        print('moveDown')

    def moveWordBackward(self):
        print('moveWordBackward')

    def moveWordForward(self):
        print('moveWordForward')

    def moveToBeginningOfLine(self):
        print('moveToBeginningOfLine')

    def moveToEndOfLine(self):
        print('moveToEndOfLine')

    def moveToBeginningOfDocument(self):
        print('moveToBeginningOfDocument')

    def moveToEndOfDocument(self):
        print('moveToEndOfDocument')

    def pageUp(self):
        print('pageUp')

    def pageDown(self):
        print('pageDown')

    def processCarriageReturn(self):
        print('carriage-return')

    def processLineFeed(self):
        print('line-feed')

    def processEscape(self):
        print('escape')

    def insertTab(self):
        print('insertTab')

    def insertText(self, text):
        print('insertText: ', text)

    def setCompositionText(self, text):
        print('compositionText: ', text)

    def deleteBackward(self):
        print('deleteBackward')

    def deleteForward(self):
        print('deleteForward')

    def deleteWordBackward(self):
        print('deleteWordBackward')

    def deleteWordForward(self):
        print('deleteWordForward')


_controlChars = (
    0x08,   # back-space
    0x09,   # tab
    0x0a,   # line-feed
    0x0d,   # carriage-return
    0x1b,   # esc
    0x7f,   # ascii-delete
)
_modifierKeys = (vkey.LEFT_SHIFT, vkey.RIGHT_SHIFT, vkey.LEFT_CONTROL, vkey.RIGHT_CONTROL)


class TextInput(TextHandler, view.View):

    keyboardId = 0
    acceptTab = False
    tabSpace = 4

    keyPressingDelay = 0.25
    keyRepeatInterval = 0.02

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.__textInputHook = None
        self.__keyPressingDelayedAction = None
        self.__keyPressingRepeatAction = None
        self.__modifierKeyDown = []
        self.__textComposing = ''

    def onLoaded(self):
        super().onLoaded()
        self.__revokeKeyPressingAction()
        self.__modifierKeyDown = []

    def onUnload(self):
        self.__revokeKeyPressingAction()
        super().onUnload()

    def onResized(self):
        self.__revokeKeyPressingAction()
        super().onResized()

    def __revokeKeyPressingAction(self):
        if self.__keyPressingDelayedAction:
            self.__keyPressingDelayedAction.revoke()
        self.__keyPressingDelayedAction = None
        self.__keyPressingRepeatAction = None

    def anyKeysDown(self, *keys):
        for k in keys:
            if k in self.__modifierKeyDown:
                return True
        return False

    def onKeyDown(self, deviceId, key):
        print('onKeyDown: ', key)
        super().onKeyDown(deviceId, key)
        if deviceId == self.keyboardId:
            self.__revokeKeyPressingAction()

            ctrlDown = lambda: vkey.LEFT_CONTROL in self.__modifierKeyDown or vkey.RIGHT_CONTROL in self.__modifierKeyDown

            action = None
            if key == vkey.LEFT:
                action = self.moveWordBackward if ctrlDown() else self.moveLeft
            elif key == vkey.RIGHT:
                action = self.moveWordForward if ctrlDown() else self.moveRight
            elif key == vkey.UP:
                action = self.moveUp
            elif key == vkey.DOWN:
                action = self.moveDown
            elif key == vkey.PAGE_UP:
                action = self.pageUp
            elif key == vkey.PAGE_DOWN:
                action = self.pageDown
            elif key == vkey.HOME:
                action = self.moveToBeginningOfLine
            elif key == vkey.END:
                action = self.moveToEndOfLine
            elif key == vkey.DELETE:
                action = self.deleteWordForward if ctrlDown() else self.deleteForward

            if action:
                def installRepeatAction():
                    self.__keyPressingDelayedAction = None
                    screen = self.screen()
                    if screen:
                        self.__keyPressingRepeatAction = screen.scheduleOperation(action, (), interval=self.keyRepeatInterval)

                def installDelayedAction():
                    screen = self.screen()
                    if screen:
                        screen.postOperation(action, ())
                        self.__keyPressingDelayedAction = screen.postOperation(installRepeatAction, (), delay=self.keyPressingDelay)

                if len(self.__textComposing) > 0:
                    self.__textInputHook = installDelayedAction
                else:
                    installDelayedAction()

            if key in _modifierKeys:
                self.__modifierKeyDown.append(key)

    def onKeyUp(self, deviceId, key):
        super().onKeyUp(deviceId, key)
        if deviceId == self.keyboardId:
            self.__revokeKeyPressingAction()

            while key in self.__modifierKeyDown:
                self.__modifierKeyDown.remove(key)

    def onTextInput(self, deviceId, text):
        super().onTextInput(deviceId, text)
        print('onTextInput length:', len(text), ' hex(text[0]):', hex(ord(text[0])))

        c = ord(text[0])
        if len(text) == 1 and c in _controlChars:
            if c == 0x08:       # backspace
                self.deleteBackward()
            elif c == 0x0a:     # linefeed
                self.processLineFeed()
            elif c == 0x1b:     # escape
                self.processEscape()
            elif c == 0x09:     # tab
                self.insertTab()
            elif c == 0x0d:     # carriage return
                self.processCarriageReturn()
            elif c == 0x7f:     # delete character
                self.deleteWordBackward()

        elif not self.anyKeysDown(vkey.LEFT_CONTROL, vkey.RIGHT_CONTROL, vkey.LEFT_OPTION, vkey.RIGHT_OPTION):
            self.insertText(text)

        if self.__textInputHook:
            self.__textInputHook()
            self.__textInputHook = None

    def onTextInputCandidate(self, deviceId, text):
        super().onTextInputCandidate(deviceId, text)
        self.__textComposing = text
        self.setCompositionText(text)

    def onKeyboardLost(self, deviceId):
        super().onKeyboardLost(deviceId)
        if deviceId == self.keyboardId:
            self.__modifierKeyDown = []

    def captureKeyboard(self, deviceId):
        updateModKeys = False
        if deviceId == self.keyboardId:
            if not self.isKeyboardCapturedBySelf(deviceId):
                updateModKeys = True

        super().captureKeyboard(deviceId)
        if updateModKeys:
            self.__modifierKeyDown = []
            screen = self.screen()
            if screen:
                window = screen.window
                if window:
                    for k in _modifierKeys:
                        if window.isKeyDown(deviceId, k):
                            self.__modifierKeyDown.append(k)

    def releaseKeyboard(self, deviceId):
        if deviceId == self.keyboardId:
            self.__modifierKeyDown = []
        return super().releaseKeyboard(deviceId)

    @property
    def shiftDown(self):
        return self.anyKeysDown(vkey.LEFT_SHIFT, vkey.RIGHT_SHIFT)

    @property
    def ctrlDown(self):
        return self.anyKeysDown(vkey.LEFT_CONTROL, vkey.RIGHT_CONTROL)

    @property
    def composingText(self):
        return self.__textComposing
