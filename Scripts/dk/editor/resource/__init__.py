import os
import _dk_core as core
from ...resourcepool import ResourcePool


pool = ResourcePool()
pool.addSearchPath( os.path.dirname(__file__) )

def clean():
    pool.removeAllResourceData()
    pool.removeAllResources()
