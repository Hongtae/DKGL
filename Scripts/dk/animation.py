import _dk_core as core
import array


class SamplingNode:
    def __init__(self):
        self.frames = array.array('f')

    def addFrame(self, scale, rotation, translation):
        assert isinstance(scale, core.Vector3)
        assert isinstance(rotation, core.Quaternion)
        assert isinstance(translation, core.Vector3)

        frame = scale.tuple + rotation.tuple + translation.tuple
        self.frames.extend(frame)


class KeyframeNode:
    def __init__(self):
        self.scaleKeys = array.array('f')
        self.rotationKeys = array.array('f')
        self.translationKeys = array.array('f')

    def addScaleKey(self, time, scale):
        assert isinstance(scale, core.Vector3)
        key = (time,) + scale.tuple
        self.scaleKeys.extend(key)

    def addRotationKey(self, time, rotation):
        assert isinstance(rotation, core.Quaternion)
        key = (time,) + rotation.tuple
        self.rotationKeys.extend(key)

    def addTranslationKey(self, time, translate):
        assert isinstance(translation, core.Vector3)
        key = (time,) + translation.tuple
        self.translationKeys.extend(key)


class Animation(core.Animation):
    def addSamplingNode(self, name, node):
        if isinstance(node, SamplingNode):
            return core.Animation.addSamplingNode(self,
                                                  name,
                                                  node.frames)
        raise TypeError('node object must be SamplingNode object')

    def addKeyframeNode(self, name, node):
        if isinstance(node, KeyframeNode):
            return core.Animation.addKeyframeNode(self,
                                                  name,
                                                  node.scaleKeys,
                                                  node.rotationKeys,
                                                  node.translationKeys)
        raise TypeError('node object must be KeyframeNode object')

    @classmethod
    def animationWithSnapshots(cls, snap1, snap2, duration = 1.0):
        ''' create animation object with two snapshot dicts'''
        anim = cls()
        for k,v in snap1:
            frame = array.array('f')
            frame.extend(v.tuple)
            v2 = snap2.get(k, v)
            frame.extend(v2.tuple)
            anim.addSamplingNode(k, frame)
        anim.duration = duration
        return anim


