import _dk_core as core
from collections import namedtuple
import math

# Light Space Perspective Shadow Maps
# http://www.cg.tuwien.ac.at/research/vr/lispsm/

Plane = namedtuple('Plane', 'n, d')
Vector = namedtuple('Vector', 'x, y, z')
AABB = namedtuple('AABB', 'min, max')

_dot = lambda a, b: a.x * b.x + a.y * b.y + a.z * b.z
_cross = lambda a, b: Vector(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x)
_add = lambda a, b: Vector(a.x + b.x, a.y + b.y, a.z + b.z)
_subtract = lambda a, b: Vector(a.x - b.x, a.y - b.y, a.z - b.z)
_linear = lambda a, b, t: Vector(a.x + b.x * t, a.y + b.y * t, a.z + b.z * t)
_lengthSq = lambda a: a.x * a.x + a.y * a.y + a.z * a.z
_length = lambda a: math.sqrt(_lengthSq(a))
_mulScalar = lambda a, b: Vector(a.x * b, a.y * b, a.z * b)

def _normalize(a):
    length = _length(a)
    return _mulScalar(a, 1.0 / length) if length > 0.0 else a

_copyVector = lambda a: Vector(a.x, a.y, a.z)
_coreVector = lambda a: core.Vector3(a.x, a.y, a.z)

_epsilon = 0.001
_inf = float('inf')
_fltMin = 1.175494351e-38
_fltMax = 3.402823466e+38
_dblMin = 2.2250738585072014e-308
_dblMax = 1.7976931348623158e+308
_isEqual = lambda a, b: abs(a.x - b.x) < _epsilon and abs(a.y - b.y) < _epsilon and abs(a.z - b.z) < _epsilon

_frustumAabb = AABB(core.Vector3(-1.0, -1.0, -1.0), core.Vector3(1.0, 1.0, 1.0))


def calcFrustumLineObject(invViewProjMatrix):
    # get frustum points
    frustum = [core.Vector3(-1.0, -1.0, -1.0),
               core.Vector3( 1.0, -1.0, -1.0),
               core.Vector3( 1.0,  1.0, -1.0),
               core.Vector3(-1.0,  1.0, -1.0),
               core.Vector3(-1.0, -1.0,  1.0),
               core.Vector3( 1.0, -1.0,  1.0),
               core.Vector3( 1.0,  1.0,  1.0),
               core.Vector3(-1.0,  1.0,  1.0)]

    # camera to world-space
    for v in frustum:
        v.transform(invViewProjMatrix)

    obj = [None] * 6
    obj[0] = frustum[0:4]   # near poly ccw
    obj[1] = frustum[4:]    # far poly ccw
    obj[2] = [frustum[0], frustum[3], frustum[7], frustum[4]]   # left poly ccw
    obj[3] = [frustum[1], frustum[5], frustum[6], frustum[2]]   # right poly ccw
    obj[4] = [frustum[4], frustum[5], frustum[1], frustum[0]]   # bottom poly ccw
    obj[5] = [frustum[6], frustum[7], frustum[3], frustum[2]]   # top poly ccw
    return obj


def lineObjectFromAABB(aabb):
    aabbMin = aabb.min
    aabbMax = aabb.max
    box = [core.Vector3(aabbMin.x, aabbMin.y, aabbMin.z),
           core.Vector3(aabbMax.x, aabbMin.y, aabbMin.z),
           core.Vector3(aabbMax.x, aabbMax.y, aabbMin.z),
           core.Vector3(aabbMin.x, aabbMax.y, aabbMin.z),
           core.Vector3(aabbMin.x, aabbMin.y, aabbMax.z),
           core.Vector3(aabbMax.x, aabbMin.y, aabbMax.z),
           core.Vector3(aabbMax.x, aabbMax.y, aabbMax.z),
           core.Vector3(aabbMin.x, aabbMax.y, aabbMax.z)]
    obj = [None] * 6
    obj[0] = box[0:4]
    obj[1] = box[4:]
    obj[2] = [box[0], box[3], box[7], box[4]]
    obj[3] = [box[1], box[5], box[6], box[2]]
    obj[4] = [box[4], box[5], box[1], box[0]]
    obj[5] = [box[6], box[7], box[3], box[2]]
    return obj


def intersectPlaneEdge(plane, a, b):
    d = _subtract(b, a)
    t = _dot(plane.n, d)
    if t != 0:
        t = (plane.d - _dot(plane.n, a)) / t
        if 0.0 <= t <= 1.0:
            return _linear(a, d, t)


def clipObjectByPlane(obj, plane):
    objOut = []         # line-poly clipped by plane
    interObj = []       # intersection lines
    for poly in obj:
        numVerts = len(poly)
        result = []
        interPts = []
        if numVerts > 2:
            outside = [False] * numVerts
            for i in range(numVerts):
                # both outside -> save none
                outside[i] = _dot(plane.n, poly[i]) > plane.d

            for i1 in range(numVerts):
                i2 = (i1 + 1) % numVerts
                if outside[i1] and outside[i2]:
                    continue

                p1 = poly[i1]
                p2 = poly[i2]
                if outside[i1]:
                    # outside to inside -> calc intersection save intersection and save i+1
                    inter = intersectPlaneEdge(plane, p1, p2)
                    if inter:
                        result.append(inter)
                        interPts.append(inter)
                    result.append(p2)
                    continue
                if outside[i2]:
                    # inside to outside -> calc intersection save intersection
                    inter = intersectPlaneEdge(plane, p1, p2)
                    if inter:
                        result.append(inter)
                        interPts.append(inter)
                    continue
                # both inside -> save point i+1
                result.append(p2)

        if len(result) > 2:
            objOut.append(result)
        if len(interPts) == 2:
            interObj.append(interPts)

    # append intersection line-poly clipped by plane
    if len(interObj) > 2:
        poly = interObj.pop()
        while len(interObj) > 0:
            lastPt = poly[-1]
            nextPt = None

            # find lastPt in rest intersection poly-vertices
            for i in range(len(interObj)-1, -1, -1):
                v1, v2 = interObj[i]
                if _isEqual(v1, lastPt):
                    interObj.pop(i)
                    nextPt = v2
                    break
                if _isEqual(v2, lastPt):
                    interObj.pop(i)
                    nextPt = v1
                    break

            if nextPt:
                # found next-linked point
                poly.append(nextPt)
            else:
                print('cannot find matching pt: ', lastPt)

                # for i in range(len(interObj)):
                #     poly = interObj[i]
                #     for v in poly:
                #         print('inter[{}]: '.format(i), v)

                interObj.pop()

        assert len(poly) > 3

        if len(poly) > 3:
            # last point can be deleted, it is same as the first (closes polygon)
            poly.pop()
            objOut.append(poly)
    return objOut


def clipObjectByAABB(obj, aabb):
    # create planes from aabb
    planes = [Plane(Vector( 0.0, -1.0,  0.0), abs(aabb.min.y)),    # bottom
              Plane(Vector( 0.0,  1.0,  0.0), abs(aabb.max.y)),    # top
              Plane(Vector(-1.0,  0.0,  0.0), abs(aabb.min.x)),    # left
              Plane(Vector( 1.0,  0.0,  0.0), abs(aabb.max.x)),    # right
              Plane(Vector( 0.0,  0.0, -1.0), abs(aabb.min.z)),    # back
              Plane(Vector( 0.0,  0.0,  1.0), abs(aabb.max.z))]    # front

    for plane in planes:
        obj = clipObjectByPlane(obj, plane)
    return obj


def _clipTest(p, q, u):
    if p < 0.0:
        r = q / p
        if r > u[1]:
            return False
        elif r > u[0]:
            u[0] = r
        return True
    elif p > 0.0:
        r = q / p
        if r < u[0]:
            return False
        elif r < u[1]:
            u[1] = r
        return True
    return q >= 0.0


def intersectionLineAABB(p, d, aabb):
    u = [0.0, _dblMax]
    pmax = _subtract(aabb.max, p)
    pmin = _subtract(p, aabb.min)
    intersect = _clipTest(-d.z, pmin.z, u) and _clipTest(d.z, pmax.z, u) and \
                _clipTest(-d.y, pmin.y, u) and _clipTest(d.y, pmax.y, u) and \
                _clipTest(-d.x, pmin.x, u) and _clipTest(d.x, pmax.x, u)
    if intersect:
        v = None
        if u[0] >= 0.0:
            v = _linear(p, d, u[0])
        if u[1] >= 0.0:
            v = _linear(p, d, u[1])
        return v


def includeObjectLightVolume(obj, lightDir, aabb):
    numPts = 0
    for poly in obj:
        numPts += len(poly)

    points = [None] * (numPts * 2)
    count = 0
    ld = Vector(-lightDir.x, -lightDir.y, -lightDir.z)
    for poly in obj:
        for v in poly:
            points[count] = v
            count += 1

            # for each point add the point on the ray in -lightDir
            # intersected with the aabb
            pt = intersectionLineAABB(v, ld, aabb)
            if pt:
                points[count] = pt
                count += 1

    return points[:count]


def aabbFromPoints(points):
    if len(points) > 0:
        aabbMax = _coreVector(points[0])
        aabbMin = _coreVector(points[0])
        for i in range(1, len(points)):
            v = points[i]
            if v.x < aabbMin.x:
                aabbMin.x = v.x
            elif v.x > aabbMax.x:
                aabbMax.x = v.x
            if v.y < aabbMin.y:
                aabbMin.y = v.y
            elif v.y > aabbMax.y:
                aabbMax.y = v.y
            if v.z < aabbMin.z:
                aabbMin.z = v.z
            elif v.z > aabbMax.z:
                aabbMax.z = v.z
        return AABB(aabbMin, aabbMax)


def calcFocusedLightVolumePoints(invViewProj, lightDir, aabb):
    obj = calcFrustumLineObject(invViewProj)
    if aabb:
        obj = clipObjectByAABB(obj, aabb)
    else:
        points = [core.Vector3(-1.0, -1.0, -1.0),
                  core.Vector3(1.0, -1.0, -1.0),
                  core.Vector3(1.0, 1.0, -1.0),
                  core.Vector3(-1.0, 1.0, -1.0),
                  core.Vector3(-1.0, -1.0, 1.0),
                  core.Vector3(1.0, -1.0, 1.0),
                  core.Vector3(1.0, 1.0, 1.0),
                  core.Vector3(-1.0, 1.0, 1.0)]

        # camera to world-space
        for v in points:
            v.transform(invViewProj)
        aabb = aabbFromPoints(points)

    return includeObjectLightVolume(obj, lightDir, aabb)


def calcBodyVector(points, cameraPos):
    x, y, z = 0.0, 0.0, 0.0
    for p in points:
        x += p.x - cameraPos.x
        y += p.y - cameraPos.y
        z += p.z - cameraPos.z
    return _normalize(Vector(x, y, z))


def calcViewMatrix(pos, dir, up):
    lftN = _normalize(_cross(dir, up))
    upN = _normalize(_cross(lftN, dir))
    dirN = _normalize(dir)

    return core.Matrix4(lftN.x, upN.x, -dirN.x, 0.0,
                        lftN.y, upN.y, -dirN.y, 0.0,
                        lftN.z, upN.z, -dirN.z, 0.0,
                        -_dot(lftN, pos), -_dot(upN, pos), _dot(dirN, pos), 1.0)


def calcProjectionMatrixForLHToFitAABB(aabb):
    # calculate orthogonal projection matrix to fit AABB for Left-Handed
    ax, ay, az = aabb.max.x + aabb.min.x, aabb.max.y + aabb.min.y, aabb.max.z + aabb.min.z
    bx, by, bz = aabb.max.x - aabb.min.x, aabb.max.y - aabb.min.y, aabb.max.z - aabb.min.z

    sx, tx = (2.0 / bx, -ax / bx) if bx > 0.0 else (1.0, 0.0)
    sy, ty = (2.0 / by, -ay / by) if by > 0.0 else (1.0, 0.0)
    sz, tz = (2.0 / bz, -az / bz) if bz > 0.0 else (1.0, 0.0)

    return core.Matrix4(sx, 0.0, 0.0, 0.0,
                        0.0, sy, 0.0, 0.0,
                        0.0, 0.0, sz, 0.0,
                        tx, ty, tz, 1.0)


def calcProjectionMatrixForRHToFitAABB(aabb):
    # calculate orthogonal projection matrix to fit AABB for Right-Handed
    ax, ay, az = aabb.max.x + aabb.min.x, aabb.max.y + aabb.min.y, aabb.max.z + aabb.min.z
    bx, by, bz = aabb.max.x - aabb.min.x, aabb.max.y - aabb.min.y, aabb.max.z - aabb.min.z

    sx, tx = (2.0 / bx, -ax / bx) if bx > 0.0 else (1.0, 0.0)
    sy, ty = (2.0 / by, -ay / by) if by > 0.0 else (1.0, 0.0)
    sz, tz = (-2.0 / bz, az / bz) if bz > 0.0 else (-1.0, 0.0)

    return core.Matrix4(sx, 0.0, 0.0, 0.0,
                        0.0, sy, 0.0, 0.0,
                        0.0, 0.0, sz, 0.0,
                        tx, ty, tz, 1.0)


def uniformSMMatrices(camera, lightDir, sceneAABB, useBodyVector=True):
    invViewProj = camera.viewProjectionMatrix()
    invViewProj.inverse()

    points = calcFocusedLightVolumePoints(invViewProj, lightDir, sceneAABB)

    cameraPos = camera.position()
    upVector = calcBodyVector(points, cameraPos) if useBodyVector else camera.direction()

    # calculate view-matrix
    viewMatrix = calcViewMatrix(cameraPos, lightDir, upVector)

    # transform the light volume points from world into light space
    for i in range(len(points)):
        v = _coreVector(points[i])
        v.transform(viewMatrix)
        points[i] = v

    # calculate cubic hull (AABB)
    aabb = aabbFromPoints(points) if len(points) > 0 else _frustumAabb

    # refit to unit cube
    projectionMatrix = calcProjectionMatrixForRHToFitAABB(aabb)
    return viewMatrix, projectionMatrix


def lispSMMatrices(camera, lightDir, sceneAABB, useBodyVector=True):
    invViewProj = camera.viewProjectionMatrix()
    invViewProj.inverse()

    points = calcFocusedLightVolumePoints(invViewProj, lightDir, sceneAABB)

    cameraPos = camera.position()
    viewDir = camera.direction()
    dp = _dot(viewDir, lightDir)
    sinGamma = math.sqrt(1.0 - dp * dp)

    # calc up-vector
    if useBodyVector:
        newDir = calcBodyVector(points, cameraPos)
        left = _cross(lightDir, newDir)
    else:
        left = _cross(lightDir, viewDir)
    upVector = _normalize(_cross(left, lightDir))

    # temporal light View
    # look from position (cameraPos)
    # into direction (lightDir)
    # with up vector (upVector)
    viewMatrix = calcViewMatrix(cameraPos, lightDir, upVector)

    # transform the light volume points from world into light space
    numPoints = len(points)
    pointsCopy = [None] * numPoints
    for i in range(numPoints):
        v = points[i]
        points[i] = _coreVector(v)
        pointsCopy[i] = _coreVector(v)
        points[i].transform(viewMatrix)

    # calculate cubic hull (AABB)
    aabb = aabbFromPoints(points) if numPoints > 0 else _frustumAabb
    points = pointsCopy
    del pointsCopy

    nearPt = core.Vector3(0, 0, -1)             # frustum near-center
    nearPt.transform(invViewProj)               # camera space to world space
    nearDist = (nearPt - cameraPos).length()    # get camera near distance

    # LiSPSM formulas of the paper to get n (and f)
    factor = 1.0 / sinGamma
    z_n = factor * nearDist             # often 1
    d = abs(aabb.max.y - aabb.min.y)    # perspective transform depth # light space y extents
    z_f = z_n + d * sinGamma
    n = (z_n + math.sqrt(z_f * z_n)) / sinGamma
    f = n + d

    # new observer point n-1 behind eye position
    # pos = eyePos-up*(n-nearDist)
    pos = _linear(cameraPos, upVector, -(n-nearDist))
    viewMatrix = calcViewMatrix(pos, lightDir, upVector)

    # one possibility for a simple perspective transformation matrix
    # with the two parameters n(near) and f(far) in y direction
    # [ 1 0 0 0]    a = (f+n)/(f-n)
    # [ 0 a 0 1]    b = -2*f*n/(f-n)
    # [ 0 0 1 0]
    # [ 0 b 0 0]
    lispMatrix = core.Matrix4(1.0, 0.0, 0.0, 0.0,
                              0.0, (f+n)/(f-n), 0.0, 1.0,
                              0.0, 0.0, 1.0, 0.0,
                              0.0, -2.0*f*n/(f-n), 0.0, 0.0)

    # temporal arrangement for the transformation of the points to post-perspective space
    lightProjection = viewMatrix * lispMatrix

    # transform the light volume points from world into the distorted light space
    for v in points:
        v.transform(lightProjection)

    # calculate the cubic hull (an AABB)
    aabb = aabbFromPoints(points) if numPoints > 0 else _frustumAabb

    # refit to unit cube
    projectionMatrix = calcProjectionMatrixForRHToFitAABB(aabb)
    return viewMatrix, lispMatrix * projectionMatrix
