#pragma once
#include <DK.h>
#include "DKMaxExporter.h"

using namespace DKFoundation;
using namespace DKFramework;


void DecompMatrix(const GMatrix& m, DKVector3& vScale, DKQuaternion& qRotate, DKVector3& vTranslate);
DKTransformUnit DecompMatrix(const GMatrix& m);

void SetLogHidden(void);
void SetLogFile(const DKString& file);
void SetLogConsole(void);

void LoadConfig(void);
void SaveConfig(void);
