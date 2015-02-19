import _dk_core as core

class ActionController(core.ActionController):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def __del__(self):
        self.removeFromParent()
