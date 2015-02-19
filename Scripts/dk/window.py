import _dk_core as core

WINDOW_EVENT_CREATED = 0
WINDOW_EVENT_CLOSED = 1
WINDOW_EVENT_HIDDEN = 2
WINDOW_EVENT_SHOWN = 3
WINDOW_EVENT_ACTIVATED = 4
WINDOW_EVENT_INACTIVATED = 5
WINDOW_EVENT_MINIMIZED = 6
WINDOW_EVENT_MOVED = 7
WINDOW_EVENT_RESIZED = 8
WINDOW_EVENT_UPDATE = 9

KEYBOARD_EVENT_NOTHING = 0
KEYBOARD_EVENT_DOWN = 1
KEYBOARD_EVENT_UP = 2
KEYBOARD_EVENT_TEXT_INPUT = 3
KEYBOARD_EVENT_TEXT_INPUT_CANDIDATE = 4

MOUSE_EVENT_NOTHING = 0
MOUSE_EVENT_DOWN = 1
MOUSE_EVENT_UP = 2
MOUSE_EVENT_MOVE = 3
MOUSE_EVENT_WHEEL = 4

STYLE_TITLE = 1
STYLE_CLOSE_BUTTON = 1 << 1
STYLE_MINIMIZE_BUTTON = 1 << 2
STYLE_MAXIMIZE_BUTTON = 1 << 3
STYLE_RESIZABLE_BORDER = 1 << 4
STYLE_ACCEPT_FILE_DROP = 1 << 8     # enable drag & drop
STYLE_GENERAL_WINDOW = 0xff         # all above without STYLE_ACCEPT_FILE_DROP

DEFAULT_WINDOW_POSITION = core.Point(-3.402823466e+38, -3.402823466e+38)

class Window(core.Window):
    '''System Window Helper Class
    You can handle window, keyboard, mouse event
    event methods will invoked on MAIN-THREAD'''

    def __init__(self, title, size, origin=DEFAULT_WINDOW_POSITION, style=STYLE_GENERAL_WINDOW, activate=False):
        super().__init__(title, size, origin, style)
        if activate:
            self.activate()

    def __del__(self):
        super().close()

    def onWindowEvent(self, event, size, origin):
        pass

    def onKeyboardEvent(self, event, deviceId, key, inputText):
        pass

    def onMouseEvent(self, event, deviceId, buttonId, position, delta):
        pass

    def onFilesDropped(self, pos, files):
        '''called when user files drag and drop on this window.
        Window should be created with style mask STYLE_ACCEPT_FILE_DROP.'''
        pass

    def getContentMinSize(self):
        '''called when window resize.'''
        return 0, 0

    def getContentMaxSize(self):
        '''called when window resize.'''
        return -1, -1

    def shouldClose(self):
        '''called when user click close button.
        override this method if you want handle window-close.
        return True if you want to close window or return False
        you can destroy window later by call destroy().
        '''
        return True

    def close(self):
        '''close and destroy window immediately. this method does not invoke shouldClose()'''
        return super().close()

    def isTextInputEnabled(self, deviceId):
        return super().isTextInputEnabled(deviceId)

    def setTextInputEnabled(self, deviceId, enabled):
        return super().setTextInputEnabled(deviceId, enabled)


#core.setDefaultClass(core.Window, Window)
