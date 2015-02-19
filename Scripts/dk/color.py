import _dk_core as core

Color = core.Color

def colorFromIntARGB(val):
    a = ((val >> 24) & 0xff) / 255.0
    r = ((val >> 16) & 0xff) / 255.0
    g = ((val >> 8) & 0xff) / 255.0
    b = (val & 0xff) / 255.0
    return Color(r, g, b, a)

def colorFromIntRGBA(val):
    r = ((val >> 24) & 0xff) / 255.0
    g = ((val >> 16) & 0xff) / 255.0
    b = ((val >> 8) & 0xff) / 255.0
    a = (val & 0xff) / 255.0
    return Color(r, g, b, a)

def colorFromIntRGB(val):
    r = ((val >> 16) & 0xff) / 255.0
    g = ((val >> 8) & 0xff) / 255.0
    b = (val & 0xff) / 255.0
    return Color(r, g, b, 1.0)

clear = Color(0.0, 0.0, 0.0, 0.0)
white = Color(1.0, 1.0, 1.0)
red = Color(1.0, 0.0, 0.0)
green = Color(0.0, 1.0, 0.0)
blue = Color(0.0, 0.0, 1.0)
black = Color(0.0, 0.0, 0.0)
yellow = Color(1.0, 1.0, 0.0)
magenta = Color(1.0 , 0.0, 1.0)
