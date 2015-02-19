import _dk_core as core
import math

class Font(core.Font):
    def __init__(self, file, point, embolden=0.0, outline=0.0, dpi=(72, 72), enableKerning=True, forceBitmap=False):
        '''create font from file or url or data object'''
        super().__init__(file, point, embolden, outline, dpi, enableKerning, forceBitmap)

    def setStyle(self, point, embolden=0.0, outline=0.0, dpi=(72, 72), enableKerning=True, forceBitmap=False):
        super().setStyle(point, embolden, outline, dpi, enableKerning, forceBitmap)

