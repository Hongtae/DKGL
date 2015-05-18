#include "Utilities.h"

void DecompMatrix(const GMatrix& m, DKVector3& vScale, DKQuaternion& qRotate, DKVector3& vTranslate)
{
	AffineParts ap;
	decomp_affine(m.ExtractMatrix3(), &ap);

	ap.q = ap.q.Inverse();	// 맥스는 반대로 회전하므로, quaternion 은 inverse 해준다.

	// pos
	vTranslate.x = ap.t.x;
	vTranslate.y = ap.t.y;
	vTranslate.z = ap.t.z;
	// rot
	qRotate.x = ap.q.x;
	qRotate.y = ap.q.y;
	qRotate.z = ap.q.z;
	qRotate.w = ap.q.w;
	// scale
	vScale.x = ap.k.x;
	vScale.y = ap.k.y;
	vScale.z = ap.k.z;
}

DKTransformUnit DecompMatrix(const GMatrix& m)
{
	DKTransformUnit t;
	DecompMatrix(m, t.scale, t.rotation, t.translation);
	return t;
}

void DummyConsole(const DKString&)
{
}

DKString GetConfigFilename()
{
	TSTR filename;

	filename += GetCOREInterface()->GetDir(APP_PLUGCFG_DIR);
	filename += _M("\\");
	filename += _M("DKMaxExporter.cfg");
	return DKString(filename);
}

void SetLogHidden(void)
{
	DKLogInit(DummyConsole);
}

void SetLogFile(const DKString& file)
{
}

void SetLogConsole(void)
{
	DKLogInit(NULL);
}

void LoadConfig(void)
{
	DKString cfgFile = GetConfigFilename();

	DKPropertySet::DefaultSet().Import(cfgFile, true);
}

void SaveConfig(void)
{
	DKString cfgFile = GetConfigFilename();

	DKPropertySet::DefaultSet().Export(cfgFile, true);
}
