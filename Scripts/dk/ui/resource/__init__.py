import os
import _dk_core as core
from ...resourcepool import ResourcePool

# resource pool
pool = ResourcePool()
pool.addSearchPath( os.path.dirname(__file__) )

textFonts = {}
outlineFonts = {}

def clear():
    global pool, textFonts, outlineFonts
    pool.removeAllResourceData()
    pool.removeAllResources()
    textFonts.clear()
    outlineFonts.clear()
