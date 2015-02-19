import _dk_core as core

#param
PARAM_ERP = 0
PARAM_STOP_ERP = 1
PARAM_CFM = 2
PARAM_STOP_CFM = 3

#param-axis
AXIS_DEFAULT = 0
LINEAR_X = 1
LINEAR_Y = 2
LINEAR_Z = 3
ANGULAR_X = 4
ANGULAR_Y = 5
ANGULAR_Z = 6

#generic-6dof-constraint axis-state
AXIS_LOCKED = 0
AXIS_FREE = 1
AXIS_RANGED = 2

#abstract interface
Constraint = core.Constraint

#constraints
ConeTwistConstraint = core.ConeTwistConstraint
FixedConstraint = core.FixedConstraint
GearConstraint = core.GearConstraint
Generic6DofConstraint = core.Generic6DofConstraint
Generic6DofSpringConstraint = core.Generic6DofSpringConstraint
HingeConstraint = core.HingeConstraint
Point2PointConstraint = core.Point2PointConstraint
SliderConstraint = core.SliderConstraint

