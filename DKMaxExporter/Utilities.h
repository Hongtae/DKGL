#pragma once
#include <DK/DK.h>
#include "DKMaxExporter.h"

using namespace DKGL;
using namespace DKGL;


void DecompMatrix(const GMatrix& m, DKVector3& vScale, DKQuaternion& qRotate, DKVector3& vTranslate);
DKTransformUnit DecompMatrix(const GMatrix& m);

void SetLogHidden(void);
void SetLogFile(const DKString& file);
void SetLogConsole(void);

void LoadConfig(void);
void SaveConfig(void);
