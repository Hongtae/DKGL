# Note:
# This python source code based on bullet physics vehicle controller.

import _dk_core as core
import math
from collections import namedtuple

Vector3 = core.Vector3
Quaternion = core.Quaternion
NSTransform = core.NSTransform
Matrix3 = core.Matrix3
Matrix4 = core.Matrix4
RigidBody = core.RigidBody
ActionController = core.ActionController


def jacobianDiagonal(worldA : Matrix3,
                     worldB : Matrix3,
                     rel_pos1 : Vector3,
                     rel_pos2 : Vector3,
                     jointAxis : Vector3,
                     inertiaInvA : Vector3,
                     massInvA : float,
                     inertiaInvB : Vector3,
                     massInvB : float):
    aJ = rel_pos1.cross(jointAxis) * worldA
    bJ = rel_pos2.cross(-jointAxis) * worldB
    aMinvJt = inertiaInvA * aJ
    bMinvJt = inertiaInvB * bJ
    diag = massInvA + aMinvJt.dot(aJ) + massInvB + bMinvJt.dot(bJ)
    assert diag > 0.0
    return diag

def resolveSingleBilateral(bodyA, posA, bodyB, posB, normal):
    '''bilateral constraint between two dynamic objects'''

    normalLenSqr = normal.lengthSq()
    assert normalLenSqr < 1.1
    if normalLenSqr > 1.1:
        return 0.0

    rel_pos1 = posA - bodyA.centerOfMassPosition()
    rel_pos2 = posB - bodyB.centerOfMassPosition()

    vel1 = bodyA.velocityInLocalPoint(rel_pos1)
    vel2 = bodyB.velocityInLocalPoint(rel_pos2)
    vel = vel1 - vel2

    matA = bodyA.centerOfMassTransform().matrix3()
    matB = bodyB.centerOfMassTransform().matrix3()
    matA.transpose()
    matB.transpose()
    jacDiagABInv = 1.0 / jacobianDiagonal(matA,
                                          matB,
                                          rel_pos1,
                                          rel_pos2,
                                          normal,
                                          bodyA.inverseDiagLocalInertia(),
                                          bodyA.inverseMass(),
                                          bodyB.inverseDiagLocalInertia(),
                                          bodyB.inverseMass())

    rel_vel = normal.dot(vel)
    contactDamping = 0.2
    velocityImpulse = -contactDamping * rel_vel * jacDiagABInv

    return velocityImpulse

def calcRollingFriction(bodyA, bodyB, frictionPos, frictionDir, maxImpulse):

    rel_pos1 = frictionPos - bodyA.centerOfMassPosition()
    rel_pos2 = frictionPos - bodyB.centerOfMassPosition()

    vel1 = bodyA.velocityInLocalPoint(rel_pos1)
    vel2 = bodyB.velocityInLocalPoint(rel_pos2)
    vel = vel1 - vel2

    vrel = frictionDir.dot(vel)

    denom0 = bodyA.computeImpulseDenominator(frictionPos, frictionDir)
    denom1 = bodyB.computeImpulseDenominator(frictionPos, frictionDir)
    jacobianDiagInv = 1.0 / (denom0 + denom1)

    j1 = -vrel * jacobianDiagInv
    if j1 > maxImpulse:     j1 = maxImpulse
    if j1 < -maxImpulse:    j1 = -maxImpulse

    return j1


class Wheel:
    # default tuning value
    suspensionStiffness = 5.88
    suspensionCompression = 0.83
    suspensionDamping = 0.88
    maxSuspensionTravelCm = 500.0
    maxSuspensionForce = 6000.0
    frictionSlip = 10.5

    def __init__(self,
                 connectionPoint,
                 direction,
                 axle,
                 suspensionResetLength,
                 wheelRadius,
                 **tuning):

        self.suspensionRestLength = float(suspensionResetLength)

        self.wheelRadius = wheelRadius

        self.suspensionStiffness = tuning.get('suspensionStiffness', 5.88)
        self.suspensionCompression = tuning.get('suspensionCompression', 0.83)
        self.suspensionDamping = tuning.get('suspensionDamping', 0.88)
        self.frictionSlip = tuning.get('frictionSlip', 10.5)
        self.maxSuspensionTravelCm = tuning.get('maxSuspensionTravelCm', 500.0)
        self.maxSuspensionForce = tuning.get('maxSuspensionForce', 6000.0)
        self.rollInfluence = tuning.get('rollInfluence', 0.1)

        self.connectionPoint = Vector3(connectionPoint)
        self.direction = Vector3(direction)
        self.axle = Vector3(axle)

        self.direction.normalize()
        self.axle.normalize()

        self.steering = 0.0
        self.engineForce = 0.0
        self.rotation = 0.0
        self.deltaRotation = 0.0
        self.brake = 0.0

        self.suspensionRelativeVelocity = 0.0
        self.clippedInvContactDotSuspension = 1.0
        self.suspensionForce = 0.0
        self.skidInfo = 0.0

        class RaycastInfo:
            def __init__(self):
                self.contactNormalWS = Vector3()
                self.contactPointWS = Vector3()
                self.suspensionLength = float()
                self.hardPointWS = Vector3()
                self.wheelDirectionWS = Vector3()
                self.wheelAxleWS = Vector3()
                self.inContact = False
                self.groundObject = None

        self.raycastInfo = RaycastInfo()
        self.worldTransform = NSTransform()

_RayCasterResult = namedtuple('_RayCasterResult', ('object', 'hitPoint', 'hitNormal', 'fraction'))


class Vehicle:
    def __init__(self, chassis):
        super().__init__()

        assert isinstance(chassis, RigidBody)

        self.chassisBody = chassis
        self.speedKmHour = 0.0
        self.steering = 0.0
        self.wheels = []
        self.fixedBody = RigidBody(None, mass=0.0)

    def addWheel(self,
                 connectionPoint,
                 wheelDirection,
                 wheelAxle,
                 suspensionResetLength,
                 wheelRadius,
                 **tuning):

        wheel = Wheel(connectionPoint, wheelDirection, wheelAxle, suspensionResetLength, wheelRadius, **tuning)

        self.wheels.append(wheel)
        self.updateWheelTransformWS( wheel, False )
        self.updateWheelTransform( wheel, False )
        return wheel

    def updateWheelTransformWS(self, wheel, interpolated):
        wheel.raycastInfo.inContact = False

        if interpolated:
            chassisTransform = self.chassisBody.worldTransform()
        else:
            chassisTransform = self.chassisBody.centerOfMassTransform()

        wheel.raycastInfo.hardPointWS = Vector3(wheel.connectionPoint)
        wheel.raycastInfo.hardPointWS.transform(chassisTransform)
        mat = chassisTransform.matrix3()
        wheel.raycastInfo.wheelDirectionWS = wheel.direction * mat
        wheel.raycastInfo.wheelAxleWS = wheel.axle * mat


    def updateWheelTransform(self, wheel, interpolated=True):
        self.updateWheelTransformWS(wheel, interpolated)
        up = -wheel.raycastInfo.wheelDirectionWS
        right = wheel.raycastInfo.wheelAxleWS
        fwd = up.cross(right)
        fwd.normalize()

        steering = wheel.steering
        steeringQuat = Quaternion(up, steering)
        steeringMat = steeringQuat.matrix3()

        rotationQuat = Quaternion(right, -wheel.rotation)
        rotationMat = rotationQuat.matrix3()

        basis2 = Matrix3(right, fwd, up)

        wheel.worldTransform = NSTransform(basis2 * rotationMat * steeringMat,
                                           wheel.raycastInfo.hardPointWS +
                                           wheel.raycastInfo.wheelDirectionWS * wheel.raycastInfo.suspensionLength)


    def resetSuspension(self):
        for wheel in self.wheels:
            wheel.raycastInfo.suspensionLength = wheel.suspensionRestLength
            wheel.suspensionRelativeVelocity = 0.0
            wheel.raycastInfo.contactNormalWS = -wheel.raycastInfo.wheelDirectionWS
            wheel.clippedInvContactDotSuspension = 1.0

    def _castRay(self, rayBegin, rayEnd):
        scene = self.scene()
        if scene:
            result = scene.rayTestClosest(rayBegin, rayEnd)
            if result:
                object = result[0]
                if isinstance(object, RigidBody) and object.hasContactResponse():
                    length1 = (result[1] - rayBegin).length()
                    length2 = (rayEnd - rayBegin).length()
                    fraction = length1 / length2
                    return _RayCasterResult(object, result[1], result[2], fraction)


    def rayCast(self, wheel):
        self.updateWheelTransformWS(wheel, False)
        depth = -1.0
        rayLen = wheel.suspensionRestLength + wheel.wheelRadius
        rayVector = wheel.raycastInfo.wheelDirectionWS * rayLen
        source = wheel.raycastInfo.hardPointWS

        wheel.raycastInfo.contactPointWS = source + rayVector
        target = wheel.raycastInfo.contactPointWS

        param = 0.0
        wheel.raycastInfo.groundObject = None

        result = self._castRay(source, target)
        if result:
            assert result.object != self.chassisBody

            param = result.fraction
            depth = rayLen * result.fraction
            wheel.raycastInfo.contactNormalWS = result.hitNormal
            wheel.raycastInfo.inContact = True

            wheel.raycastInfo.groundObject = self.fixedBody
            #wheel.raycastInfo.groundObject = result.object

            hitDistance = param * rayLen
            wheel.raycastInfo.suspensionLength = hitDistance - wheel.wheelRadius

            minSuspensionLength = wheel.suspensionRestLength - wheel.maxSuspensionTravelCm * 0.01
            maxSuspensionLength = wheel.suspensionRestLength + wheel.maxSuspensionTravelCm * 0.01

            if wheel.raycastInfo.suspensionLength < minSuspensionLength:
                wheel.raycastInfo.suspensionLength = minSuspensionLength
            if wheel.raycastInfo.suspensionLength > maxSuspensionLength:
                wheel.raycastInfo.suspensionLength = maxSuspensionLength

            wheel.raycastInfo.contactPointWS = result.hitPoint
            denominator = wheel.raycastInfo.contactNormalWS.dot(wheel.raycastInfo.wheelDirectionWS)

            chassisTransform = self.chassisBody.centerOfMassTransform()
            relpos = wheel.raycastInfo.contactPointWS - Vector3(chassisTransform.position)
            chassis_velocity_at_contactPoint = self.chassisBody.velocityInLocalPoint(relpos)
            projVel = wheel.raycastInfo.contactNormalWS.dot(chassis_velocity_at_contactPoint)

            if denominator >= -0.1:
                wheel.suspensionRelativeVelocity = 0.0
                wheel.clippedInvContactDotSuspension = 1.0 / 0.1
            else:
                inv = -1.0 / denominator
                wheel.suspensionRelativeVelocity = projVel * inv
                wheel.clippedInvContactDotSuspension = inv
        else:
            wheel.raycastInfo.suspensionLength = wheel.suspensionRestLength
            wheel.suspensionRelativeVelocity = 0.0
            wheel.raycastInfo.contactNormalWS = -wheel.raycastInfo.wheelDirectionWS
            wheel.clippedInvContactDotSuspension = 1.0

        return depth


    def updateSuspension(self, delta):
        chassisMass = self.chassisBody.mass()
        for wheel in self.wheels:
            if wheel.raycastInfo.inContact:

                # spring
                susp_length = wheel.suspensionRestLength
                current_length = wheel.raycastInfo.suspensionLength

                length_diff = (susp_length - current_length)
                force = wheel.suspensionStiffness * length_diff * wheel.clippedInvContactDotSuspension

                # damper
                projected_rel_vel = wheel.suspensionRelativeVelocity
                if projected_rel_vel < 0.0:
                    susp_damping = wheel.suspensionCompression
                else:
                    susp_damping = wheel.suspensionDamping
                force -= susp_damping * projected_rel_vel

                # result
                wheel.suspensionForce = force * chassisMass
                if wheel.suspensionForce < 0.0:
                    wheel.suspensionForce = 0.0
            else:
                wheel.suspensionForce = 0.0


    def updateFriction(self, delta):
        sideFrictionStiffness2 = 1.0

        numWheels = len(self.wheels)
        if numWheels == 0:
            return

        sideFactor = 1.0
        fwdFactor = 0.5
        sliding = False


        forwardWS = [None] * numWheels
        axle = [None] * numWheels
        forwardImpulses = [0.0] * numWheels
        sideImpulses = [0.0] * numWheels
        # for i in range(numWheels):
        #     forwardWS[i] = Vector3()
        #     axle[i] = Vector3()

        numWheelsOnGround = 0

        # collapse all those loops into one!
        for i in range(numWheels):
            wheel = self.wheels[i]

            wheel.skidInfo = 1.0

            rollingFriction = 0.0

            groundObject = wheel.raycastInfo.groundObject
            if groundObject:
                numWheelsOnGround += 1

                wheelTrans = wheel.worldTransform
                axle[i] = Vector3( wheelTrans.matrix3().row1 )

                surfNormalWS = wheel.raycastInfo.contactNormalWS
                proj = axle[i].dot(surfNormalWS)
                axle[i] -= surfNormalWS * proj
                axle[i].normalize()

                forwardWS[i] = surfNormalWS.cross(axle[i])
                forwardWS[i].normalize()

                sideImpulses[i] = resolveSingleBilateral(self.chassisBody,
                                                         wheel.raycastInfo.contactPointWS,
                                                         groundObject,
                                                         wheel.raycastInfo.contactPointWS,
                                                         axle[i])

                sideImpulses[i] *= sideFrictionStiffness2

                #########
                if wheel.engineForce != 0.0:
                    rollingFriction = wheel.engineForce * delta
                else:
                    maxImpulse = 0.0        # default rolling friction impulse
                    if wheel.brake != 0.0:
                        maxImpulse = wheel.brake
                    rollingFriction = calcRollingFriction(self.chassisBody,
                                                          groundObject,
                                                          wheel.raycastInfo.contactPointWS,
                                                          forwardWS[i],
                                                          maxImpulse)

                ##########
                ## switch between active rolling (throttle), braking and non-active rolling friction (no throttle/break)

                maximp = wheel.suspensionForce * delta * wheel.frictionSlip
                maximpSquared = maximp * maximp

                forwardImpulses[i] = rollingFriction
                x = forwardImpulses[i] * fwdFactor
                y = sideImpulses[i] * sideFactor
                impulseSquared = (x*x + y*y)
                if impulseSquared > maximpSquared:
                    sliding = True
                    factor = maximp / math.sqrt(impulseSquared)
                    wheel.skidInfo *= factor

        if sliding:
            for i in range(numWheels):
                if sideImpulses[i] != 0.0:
                    wheel = self.wheels[i]
                    if wheel.skidInfo < 1.0:
                        forwardImpulses[i] *= wheel.skidInfo
                        sideImpulses[i] *= wheel.skidInfo

        # appy the impulses
        chassisTransform = self.chassisBody.centerOfMassTransform()
        for i in range(numWheels):
            wheel = self.wheels[i]
            rel_pos = wheel.raycastInfo.contactPointWS - Vector3(chassisTransform.position)

            if forwardImpulses[i] != 0.0:
                self.chassisBody.applyImpulse(forwardWS[i] * forwardImpulses[i], rel_pos)
            if sideImpulses[i] != 0.0:
                groundObject = wheel.raycastInfo.groundObject
                rel_pos2 = wheel.raycastInfo.contactPointWS - groundObject.centerOfMassPosition()
                sideImp = axle[i] * sideImpulses[i]

                chassisWorldUp = Vector3(chassisTransform.matrix3().row2)
                rel_pos -= chassisWorldUp * (chassisWorldUp.dot(rel_pos) * (1.0 - wheel.rollInfluence))

                self.chassisBody.applyImpulse(sideImp, rel_pos)

                # apply friction impulse on the ground
                groundObject.applyImpulse(-sideImp, rel_pos2)


    def updateVehicle(self, delta):
        for wheel in self.wheels:
            self.updateWheelTransform(wheel, False)

        chassisTransform = self.chassisBody.centerOfMassTransform()

        self.speedKmHour = 3.6 * self.chassisBody.linearVelocity().length()

        chassisBasis = chassisTransform.matrix3()
        forwardW = Vector3(chassisBasis.row3)

        if forwardW.dot(self.chassisBody.linearVelocity()) < 0.0:
            self.speedKmHour *= -1.0

        # simulate suspension
        for wheel in self.wheels:
            depth = self.rayCast(wheel)

        self.updateSuspension(delta)

        for wheel in self.wheels:
            # apply suspension force
            suspensionForce = wheel.suspensionForce
            if suspensionForce > wheel.maxSuspensionForce:
                suspensionForce = wheel.maxSuspensionForce

            impulse = Vector3(wheel.raycastInfo.contactNormalWS * suspensionForce * delta)
            relpos = wheel.raycastInfo.contactPointWS - Vector3(chassisTransform.position)

            self.chassisBody.applyImpulse(impulse, relpos)

        self.updateFriction(delta)

        for wheel in self.wheels:
            relpos = wheel.raycastInfo.hardPointWS - Vector3(chassisTransform.position)
            vel = self.chassisBody.velocityInLocalPoint(relpos)

            if wheel.raycastInfo.inContact:
                fwd = Vector3(chassisTransform.matrix3().row3)
                proj = fwd.dot(wheel.raycastInfo.contactNormalWS)
                fwd -= wheel.raycastInfo.contactNormalWS * proj

                proj2 = fwd.dot(vel)

                wheel.deltaRotation = (proj2 * delta) / (wheel.wheelRadius)
                wheel.rotation += wheel.deltaRotation
            else:
                wheel.rotation += wheel.deltaRotation
            wheel.deltaRotation *= 0.99


    def setSteeringValue(self, steering, wheelIndex):
        wheel = self.wheels[wheelIndex]
        wheel.steering = steering

    def steeringValue(self, wheelIndex):
        wheel = self.wheels[wheelIndex]
        return wheel.steering

    def applyEngineForce(self, force, wheelIndex):
        wheel = self.wheels[wheelIndex]
        wheel.engineForce = force

    def setBrake(self, brake, wheelIndex):
        wheel = self.wheels[wheelIndex]
        wheel.brake = brake



class Controller(Vehicle, ActionController):
    def __init__(self, chassis):
        super().__init__(chassis)

    def update(self, delta, tick):
        self.updateVehicle(delta)

    def onAddedToScene(self):
        pass

    def onRemovedFromScene(self):
        pass

    def onAddedToParent(self):
        pass

    def onRemovedFromParent(self):
        pass

    def onSetAnimation(self):
        pass

