import _dk_core as core
import bisect
from .. import spline


def _clamp(val, min_val, max_val):
    if val > max_val:
        val = max_val
    elif val < min_val:
        val = min_val
    return val


def INTERPOLATE_LINEAR(frames, t):
    index = bisect.bisect_left(frames, (t,))
    if index == len(frames):
        return frames[-1][1]
    if index == 0:
        return frames[0][1]

    frame1, frame2 = frames[index-1:index+1]
    start = frame1[0]
    length = frame2[0] - start

    t1 = 1.0
    if length > 0.000001:
        t1 = _clamp((t - start) / length, 0.0, 1.0)
    t2 = 1.0 - t1

    return tuple(p1 * t2 + p2 * t1 for p1, p2 in zip(frame1[1], frame2[1]))


def _spline_interpolate(frames, t, type):
    frame0, frame1, frame2, frame3 = frames
    start = frame1[0]
    length = frame2[0] - start
    t2 = 1.0
    if length > 0.000001:
        t2 = _clamp((t - start) / length, 0.0, 1.0)

    return tuple(spline.splineInterpolate(p0, p1, p2, p3, t2, type)
                 for p0, p1, p2, p3
                 in zip(frame0[1], frame1[1], frame2[1], frame3[1]))


def _get_spline_frames(frames, t):
    index = bisect.bisect_left(frames, (t,))
    last = len(frames) - 1
    idx0 = _clamp(index - 2, 0, last)
    idx1 = _clamp(index - 1, 0, last)
    idx2 = _clamp(index, 0, last)
    idx3 = _clamp(index + 1, 0, last)
    return frames[idx0], frames[idx1], frames[idx2], frames[idx3]


def INTERPOLATE_UNIFORM_CUBIC(frames, t):
    frames = _get_spline_frames(frames, t)
    return _spline_interpolate(frames, t, spline.UNIFORM_CUBIC)


def INTERPOLATE_CATMULL_ROM(frames, t):
    frames = _get_spline_frames(frames, t)
    return _spline_interpolate(frames, t, spline.CATMULL_ROM)


def INTERPOLATE_HERMITE(frames, t):
    frame0, frame1, frame2, frame3 = _get_spline_frames(frames, t)

    # tangent vector (frame1 - frame0)
    tv0 = [p1 - p2 for p1, p2 in zip(frame1[1], frame0[1])]
    # tangent vector (frame3 - frame2)
    tv1 = [p1 - p2 for p1, p2 in zip(frame3[1], frame2[1])]

    start = frame1[0]
    length = frame2[0] - start
    t2 = 1.0
    if length > 0.000001:
        t2 =  _clamp((t - start) / length, 0.0, 1.0)

    return tuple(spline.splineInterpolate(p0, p1, p2, p3, t2, spline.HERMITE)
                 for p0, p1, p2, p3 in zip(frame1[1], frame2[1], tv0, tv1))


def INTERPOLATE_BEZIER(frames, t):
    index = bisect.bisect_right(frames, (t,))

    last = len(frames) - 1

    index = min(index, last)
    index = (index-1) // 3
    index0 = index * 3
    index1 = min(index0 + 1, last)
    index2 = min(index0 + 2, last)
    index3 = min(index0 + 3, last)

    frame0 = frames[index0]
    frame1 = frames[index1]
    frame2 = frames[index2]
    frame3 = frames[index3]

    start = frame0[0]
    length = frame3[0] - start
    t2 = 1.0
    if length > 0.000001:
        t2 = _clamp((t - start) / length, 0.0, 1.0)

    result = []
    for p0, p1, p2, p3 in zip(frame0[1], frame1[1], frame2[1], frame3[1]):
        r = spline.splineInterpolate(p0, p1, p2, p3, t2, spline.BEZIER)
        result.append(r)
    return tuple(result)


class Animation:
    def __init__(self, initialValue, duration, callback=None, interpolate=INTERPOLATE_LINEAR):
        self.time = 0.0
        self.speed = 1.0
        self.repeat = 0
        self.duration = duration
        self.callback = callback    # call after animation finished.
        self.finished = False
        self.interpolation = interpolate

        try:
            self.value = tuple(initialValue)
        except TypeError:
            self.value = (initialValue,)

        self.frames = [(0.0, self.value)]

    def addFrame(self, t, value):
        try:
            value = tuple(value)
        except TypeError:
            value = (value,)

        bisect.insort(self.frames, (float(t), value))
        return self

    def update(self, delta):
        if not self.finished:
            self.time += delta * self.speed
            if self.time >= self.duration:
                d = self.time // self.duration
                if self.repeat > d:
                    self.repeat -= d
                    self.time -= self.duration * d
                else:
                    self.finished = True   # animation finished.
                    self.time = self.duration

        if len(self.frames) > 0:
            if len(self.frames) > 1:
                self.value = self.interpolation(self.frames, self.time)
            else:
                self.value = self.frames[0][1]
