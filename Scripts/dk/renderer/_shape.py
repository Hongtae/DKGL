import _dk_core as core
import array
from .. import blendstate


SHAPE_FACES = 1 << 3
SHAPE_EDGES = 1 << 7


class RenderCollisionShape:
    def __init__(self, renderer, camera, mode=SHAPE_FACES):
        self.renderer = renderer
        self.camera = camera
        self.mode = mode
        self.objects = []
        self.colorTable = {}

    def __objectColors(self, obj, *args):
        return self.colorTable.get(obj)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.draw()

    def add(self, shape, transform, faceColor, edgeColor=core.Color(0.0, 0.0, 0.0)):
        co = core.CollisionObject()
        co.setCollisionShape(shape)
        co.setWorldTransform(transform)
        self.objects.append(co)
        self.colorTable[co] = (faceColor, edgeColor)

    def draw(self):
        scene = None
        try:
            scene = self.renderer._cachedShapeScene
        except AttributeError:
            pass

        if scene is None:
            scene = core.Scene()
            try:
                self.renderer._cachedShapeScene = scene
            except AttributeError:
                print('cannot set renderer attribute!')
                pass

        if scene:
            scene.drawMode = self.mode

            for obj in self.objects:
                scene.addObject(obj)

            self.renderer.renderScene(scene, self.camera, 0, objectColorCallback=self.__objectColors)
            scene.removeAllObjects()

        self.objects = []
        self.colorTable = {}
