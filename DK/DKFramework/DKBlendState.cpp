//
//  File: DKBlendState.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2020 Hongtae Kim. All rights reserved.
//

#include "DKBlendState.h"

using namespace DKFoundation;
using namespace DKFramework;

const DKBlendState	DKBlendState::defaultOpaque
{
    false,
    DKBlendFactor::One,
    DKBlendFactor::One,
    DKBlendFactor::Zero,
    DKBlendFactor::Zero,
    DKBlendOperation::Add
};
const DKBlendState	DKBlendState::defaultAlpha
{
    true,
    DKBlendFactor::SourceAlpha,
    DKBlendFactor::One,
    DKBlendFactor::OneMinusSourceAlpha,
    DKBlendFactor::One,
    DKBlendOperation::Add,
    DKBlendOperation::Add
};
const DKBlendState	DKBlendState::defaultMultiply
{
    true,
    DKBlendFactor::Zero,
    DKBlendFactor::Zero,
    DKBlendFactor::SourceColor,
    DKBlendFactor::SourceColor,
    DKBlendOperation::Add,
    DKBlendOperation::Add
};
const DKBlendState	DKBlendState::defaultScreen
{
    true,
    DKBlendFactor::OneMinusDestinationColor,
    DKBlendFactor::OneMinusDestinationColor,
    DKBlendFactor::One,
    DKBlendFactor::One,
    DKBlendOperation::Add,
    DKBlendOperation::Add
};
const DKBlendState	DKBlendState::defaultDarken
{
    true,
    DKBlendFactor::One,
    DKBlendFactor::One,
    DKBlendFactor::One,
    DKBlendFactor::One,
    DKBlendOperation::Min,
    DKBlendOperation::Min
};
const DKBlendState	DKBlendState::defaultLighten
{
    true,
    DKBlendFactor::One,
    DKBlendFactor::One,
    DKBlendFactor::One,
    DKBlendFactor::One,
    DKBlendOperation::Max,
    DKBlendOperation::Max
};
const DKBlendState	DKBlendState::defaultLinearBurn
{
    true,
    DKBlendFactor::One,
    DKBlendFactor::One,
    DKBlendFactor::OneMinusDestinationColor,
    DKBlendFactor::OneMinusDestinationColor,
    DKBlendOperation::Subtract,
    DKBlendOperation::Subtract
};
const DKBlendState	DKBlendState::defaultLinearDodge
{
    true,
    DKBlendFactor::One,
    DKBlendFactor::One,
    DKBlendFactor::One,
    DKBlendFactor::One,
    DKBlendOperation::Add,
    DKBlendOperation::Add
};
