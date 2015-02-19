import _dk_core as core
from . import modal
from . import titled
from . import button
from . import label
from . import font
from math import floor

_alertForScreen = {}

class Alert:

    minimumButtonWidth = 70
    buttonHeight = 40
    buttonPadding = 2

    minimumViewWidth = 200
    viewHeight = 200

    def __init__(self, text, title, *buttons):

        self.title = str(title)
        self.text = str(text)
        self.callback = None

        self.buttons = []
        for t in buttons:
            assert isinstance(t, str)
            self.buttons.append(t)

        self._modal = None

    def show(self, screen, callback):
        if callback:
            assert callable(callback)


        if len(self.buttons) == 0:
            self.buttons.append('Dismiss')

        numButtons = len(self.buttons)

        totalButtonsWidth = (self.minimumButtonWidth + self.buttonPadding) * numButtons + self.buttonPadding
        width = max(totalButtonsWidth, self.minimumViewWidth)
        buttonWidth = floor((width - self.buttonPadding) / numButtons - self.buttonPadding)

        alertView = titled.TitledView()
        alertView.caption = self.title

        alertView.frame = alertView.frameForContentFrame(core.Rect(0, 0, width, self.viewHeight))
        bounds = alertView.contentBounds()

        rect = core.Rect(bounds)
        rect.y += self.buttonHeight + self.buttonPadding
        rect.height -= self.buttonHeight + self.buttonPadding

        textView = label.Label(self.text, frame=rect)
        textView.fontAttributes = font.attributes(18)
        textView.align = label.ALIGN_CENTER
        textView.textColor = core.Color(0, 0, 0)
        textView.backgroundColor = core.Color(1, 1, 1)

        rect.origin = bounds.x, bounds.y
        rect.size = buttonWidth, self.buttonHeight
        rect.x += self.buttonPadding
        rect.y += self.buttonPadding

        print('bounds', bounds)
        print('rect', rect)

        tag = 0
        for txt in self.buttons:
            btn = button.Button(txt, frame=rect)
            btn.tag = tag
            tag += 1
            btn.addTarget(self, self._onButtonClick)
            alertView.addChild(btn)
            rect.x += self.buttonPadding + buttonWidth

        alertView.addChild(textView)

        self._modal = modal.Modal(alertView)
        self._modal.present(screen.frame, self._onModalCallback)
        self.callback = callback

    def dismiss(self, result):
        self._modal.dismiss(result)

    def _onButtonClick(self, *args, **kwargs):
        button = args[0]
        tag = button.tag
        self.dismiss(tag)

    def _onModalCallback(self, *args, **kwargs):
        if self.callback:
            self.callback(self, args[1])
