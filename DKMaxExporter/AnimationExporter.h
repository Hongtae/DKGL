#pragma once
#include <DK.h>
#include "DKMaxExporter.h"

using namespace DKFoundation;
using namespace DKFramework;


class AnimationExporter : public SceneExport 
{
public:
	virtual int				ExtCount(void) override;			// Number of extensions supported
	virtual const MCHAR*	Ext(int n) override;				// Extension #n (i.e. "3DS")
	virtual const MCHAR*	LongDesc(void) override;			// Long ASCII description (i.e. "Autodesk 3D Studio File")
	virtual const MCHAR*	ShortDesc(void) override;			// Short ASCII description (i.e. "3D Studio")
	virtual const MCHAR*	AuthorName(void) override;			// ASCII Author name
	virtual const MCHAR*	CopyrightMessage(void) override;	// ASCII Copyright message
	virtual const MCHAR*	OtherMessage1(void) override;		// Other message #1
	virtual const MCHAR*	OtherMessage2(void) override;		// Other message #2
	virtual unsigned int	Version(void) override;				// Version number * 100 (i.e. v3.01 = 301)
	virtual void			ShowAbout(HWND hWnd) override;		// Show DLL's "About..." box

	virtual BOOL SupportsOptions(int ext, DWORD options);
	virtual int	DoExport(const MCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0) override;

	//Constructor/Destructor
	AnimationExporter(void);
	virtual ~AnimationExporter(void);

	static ClassDesc2* ClassDesc(void);
protected:
	int GetSceneNodes(INodeTab& i_nodeTab, INode* i_currentNode =NULL);

private:
	static INT_PTR CALLBACK ExportOptionsDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
	void ReadConfig(void);
	void WriteConfig(void) const;

	void ExportAnimation(DKAnimation& animation, IGameNode* pNode);

	bool exportBoneOnly;
	bool exportKeyframes;
	float keyframeThreshold;
	DKSerializer::SerializeForm exportFormat;
};
