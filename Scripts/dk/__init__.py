import _dk_core as core

# fundamental data types
from .point import Point
from .size import Size
from .rect import Rect
from .timer import Timer

from .data import Data
from .stream import Stream
from .zipfile import ZipArchiver
from .zipfile import ZipUnarchiver

from .color import Color
from .light import Light
from .camera import Camera
from .spline import Spline
from .spline import Spline2
from .spline import Spline3
from .spline import Spline4
from .vector import Vector2
from .vector import Vector3
from .vector import Vector4
from .matrix import Matrix2
from .matrix import Matrix3
from .matrix import Matrix4
from .quaternion import Quaternion
from .transform import LinearTransform2
from .transform import LinearTransform3
from .transform import AffineTransform2
from .transform import AffineTransform3
from .transform import TransformUnit
from .transform import USTransform
from .transform import NSTransform

# runtime object type
from .app import App
from .window import Window
from .screen import Screen
from .frame import Frame
from .renderer import Renderer
from .rendertarget import RenderTarget
from .blendstate import BlendState
from .font import Font
from .glcontext import OpenGLContext

from .resource import Resource
from .texture import Texture
from .texture import Texture2D
from .texture import Texture3D
from .texture import TextureCube
from .texture import TextureSampler
from .animation import Animation

from .collisionshape import CompoundShape
from .collisionshape import StaticPlaneShape
from .collisionshape import StaticTriangleMeshShape
from .collisionshape import CapsuleShape
from .collisionshape import ConeShape
from .collisionshape import CylinderShape
from .collisionshape import MultiSphereShape
from .collisionshape import BoxShape
from .collisionshape import ConvexHullShape
from .collisionshape import SphereShape

from .model import Model
from .model import CollisionObject
from .model import RigidBody
from .model import SoftBody

from .mesh import Mesh
from .mesh import StaticMesh
from .mesh import SkinMesh

from .constraint import ConeTwistConstraint
from .constraint import FixedConstraint
from .constraint import GearConstraint
from .constraint import Generic6DofConstraint
from .constraint import Generic6DofSpringConstraint
from .constraint import HingeConstraint
from .constraint import Point2PointConstraint
from .constraint import SliderConstraint

from .actioncontroller import ActionController

from .geometry import VertexBuffer
from .geometry import IndexBuffer

from .material import Material

from .scene import Scene
from .scene import DynamicsScene

from .resourcepool import ResourcePool

from .audio import AudioListener
from .audio import AudioPlayer
from .audio import AudioSource

# utility modules
from . import ui
from . import sprite
from . import controller
from . import shadow
from . import editor


# utilities
def version():
    return core.version()


def platform():
    return core.platform()


def copyright():
    return core.copyright()


def random():
    return core.random()


def temporaryDirectory():
    return core.temporaryDirectory()


def processArguments():
    return core.processArguments()


def processEnvironments():
    return core.processEnvironments()


def uuidgen():
    return core.uuidgen()


def numObjects():
    return core.objectCount()


def threadSleep(t):
    return core.threadSleep(t)


def threadYield():
    return core.threadYield()


class Logger:
    @staticmethod
    def write(string):
        core.log(string)


def debugLog(*objects):
    print(*objects, file=Logger)


def appInstance():
    return App.instance()


print('\n==============================================================================')
print('  IGLib.Copyright:\n    ', core.copyright())
print('  IGLib.Version:\n    ', core.version(), core.build())
print('  Debug Enabled:', 'Yes' if core.isDebugBuild() else 'No')
print('\n  DK-Script Version 1.0.0')
print('    Copyright (c) 2014. Hongtae Kim, DKSCRIPT.COM. All rights reserved.')
print('==============================================================================\n')
