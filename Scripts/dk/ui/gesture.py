import _dk_core as core

class GestureRecognizer:
    def __init__(self):
        self.began = False

    def recognize(self, mouseEvent, deviceId, buttonId, pos, delta):
        pass

    def discard(self):
        self.began = False
