import dk

from . import resource

class MainFrame(dk.ui.View):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.backgroundColor = dk.color.yellow

    def onLoaded(self):
        super().onLoaded()

    def onUnload(self):
        super().onUnload()
        resource.clean()
        raise SystemExit