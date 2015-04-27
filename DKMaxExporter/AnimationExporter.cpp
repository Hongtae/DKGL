#include "AnimationExporter.h"
#include "Utilities.h"


ClassDesc2* AnimationExporter::ClassDesc(void)
{
	static class ExporterClassDesc : public ClassDesc2 
	{
	public:
		virtual int IsPublic(void) override 				{ return TRUE; }
		virtual void* Create(BOOL loading) override			{ return new AnimationExporter(); }
		virtual const MCHAR* ClassName(void) override		{ return _M("AnimationExporter"); }
		virtual SClass_ID SuperClassID(void) override 		{ return SCENE_EXPORT_CLASS_ID; }
		virtual Class_ID ClassID(void) override 			{ return Class_ID(0x7e8ffab3, 0xa71fdcd2); }
		virtual const MCHAR* Category(void) override 		{ return _M(""); }
		virtual const MCHAR* InternalName(void) override 	{ return _M("AnimationExporter"); }		// returns fixed parsable name (scripter-visible name)
		virtual HINSTANCE HInstance(void) override 			{ return hInstance; }						// returns owning module handle
	} desc;
	return &desc;
}

#define KEYFRAME_THRESHOLD_DEFAULT	0.000001f

AnimationExporter::AnimationExporter()
	: exportBoneOnly(true)
	, exportKeyframes(true)
	, keyframeThreshold(KEYFRAME_THRESHOLD_DEFAULT)
	, exportFormat(DKSerializer::SerializeFormXML)
{
}

AnimationExporter::~AnimationExporter() 
{
}

int AnimationExporter::ExtCount()
{
	// Returns the number of file name extensions supported by the plug-in.
	return 1;
}

const MCHAR *AnimationExporter::Ext(int n)
{		
	//#pragma message	(TODO("Return the 'i-th' file name extension (i.e. \"3DS\")."))
	return _M("DKAnimation");
}

const MCHAR *AnimationExporter::LongDesc()
{
	//#pragma message	(TODO("Return long ASCII description (i.e. \"Targa 2.0 Image File\""))
	return _M("DK Animation Exporter");
}

const MCHAR *AnimationExporter::ShortDesc()
{			
	//#pragma message	(TODO("Return short ASCII description (i.e. \"Targa\")"))
	return _M("DKAnimation");
}

const MCHAR *AnimationExporter::AuthorName()
{			
	//#pragma message	(TODO("Return ASCII Author name"))
	return _M("TIFF");
}

const MCHAR *AnimationExporter::CopyrightMessage()
{	
	//#pragma message	(TODO("Return ASCII Copyright message"))
	return _M("Copyright(c) 2011-2014, TIFF");
}

const MCHAR *AnimationExporter::OtherMessage1()
{		
	//TODO: Return Other message #1 if any
	return _M("");
}

const MCHAR *AnimationExporter::OtherMessage2()
{		
	//TODO: Return other message #2 in any
	return _M("");
}

unsigned int AnimationExporter::Version()
{
	//#pragma message	(TODO("Return Version number * 100 (i.e. v3.01 = 301)"))
	return 101;
}

void AnimationExporter::ShowAbout(HWND hWnd)
{			
	// Optional
}

BOOL AnimationExporter::SupportsOptions(int ext, DWORD options)
{
	// TODO Decide which options to support.  Simply return
	// true for each option supported by each Extension 
	// the exporter supports.

	return TRUE;
}

int AnimationExporter::GetSceneNodes(INodeTab& i_nodeTab, INode* i_currentNode /*=NULL*/)
{
	int i;
	if (i_currentNode == NULL)
	{
		i_currentNode = GetCOREInterface()->GetRootNode();
	}
	else // IGame will crash 3ds Max if it is initialised with the root node.
	{
		i_nodeTab.AppendNode(i_currentNode);
	}
	for (i = 0; i < i_currentNode->NumberOfChildren(); i++)
	{
		GetSceneNodes(i_nodeTab, i_currentNode->GetChildNode(i));
	}
	return i_nodeTab.Count();
}

void AnimationExporter::ReadConfig()
{
	LoadConfig();

	const wchar_t* exportBoneOnlyPropertyKey = L"AnimationExporter.exportBoneOnly";
	const wchar_t* exportKeyframesPropertyKey = L"AnimationExporter.exportKeyFrames";
	const wchar_t* keyframeThresholdPropertyKey = L"AnimationExporter.keyframeThreshold";
	const wchar_t* exportFormatPropertyKey = L"AnimationExporter.exportFormat";

	if (DKPropertySet::DefaultSet().Value(exportBoneOnlyPropertyKey).ValueType() == DKVariant::TypeInteger)
		exportBoneOnly = DKPropertySet::DefaultSet().Value(exportBoneOnlyPropertyKey).Integer() != 0;

	if (DKPropertySet::DefaultSet().Value(exportKeyframesPropertyKey).ValueType() == DKVariant::TypeInteger)
		exportKeyframes = DKPropertySet::DefaultSet().Value(exportKeyframesPropertyKey).Integer() != 0;

	if (DKPropertySet::DefaultSet().Value(keyframeThresholdPropertyKey).ValueType() == DKVariant::TypeFloat)
		keyframeThreshold = Max<float>(DKPropertySet::DefaultSet().Value(keyframeThresholdPropertyKey).Float(), KEYFRAME_THRESHOLD_DEFAULT);

	// exportFormat
	if (DKPropertySet::DefaultSet().Value(exportFormatPropertyKey).ValueType() == DKVariant::TypeInteger)
	{
		int fmt = DKPropertySet::DefaultSet().Value(exportFormatPropertyKey).Integer();
		switch (fmt)
		{
		case DKSerializer::SerializeFormXML:
		case DKSerializer::SerializeFormBinXML:
		case DKSerializer::SerializeFormBinary:
		case DKSerializer::SerializeFormCompressedBinary:
			exportFormat = (DKSerializer::SerializeForm)fmt;
			break;
		}
	}
}

void AnimationExporter::WriteConfig() const
{
	const wchar_t* exportBoneOnlyPropertyKey = L"AnimationExporter.exportBoneOnly";
	const wchar_t* exportKeyframesPropertyKey = L"AnimationExporter.exportKeyFrames";
	const wchar_t* keyframeThresholdPropertyKey = L"AnimationExporter.keyframeThreshold";
	const wchar_t* exportFormatPropertyKey = L"AnimationExporter.exportFormat";

	DKPropertySet::DefaultSet().SetValue(exportBoneOnlyPropertyKey, (DKVariant::VInteger)exportBoneOnly);
	DKPropertySet::DefaultSet().SetValue(exportKeyframesPropertyKey, (DKVariant::VInteger)exportKeyframes);
	DKPropertySet::DefaultSet().SetValue(keyframeThresholdPropertyKey, (DKVariant::VFloat)keyframeThreshold);
	DKPropertySet::DefaultSet().SetValue(exportFormatPropertyKey, (DKVariant::VInteger)exportFormat);

	SaveConfig();
}

INT_PTR CALLBACK AnimationExporter::ExportOptionsDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	static AnimationExporter *imp = NULL;

	switch(message)
	{
	case WM_INITDIALOG:
		imp = (AnimationExporter *)lParam;
		CenterWindow(hWnd, GetParent(hWnd));
		CheckRadioButton(hWnd, IDC_RADIO_SAMPLING, IDC_RADIO_KEYFRAME, imp->exportKeyframes ? IDC_RADIO_KEYFRAME : IDC_RADIO_SAMPLING);
		CheckDlgButton(hWnd, IDC_CHECK_EXPORT_BONEONLY, imp->exportBoneOnly ? BST_CHECKED : BST_UNCHECKED);
		{
			TCHAR buffer[1024];
			_stprintf(buffer, _T("%f"), imp->keyframeThreshold);
			SetWindowText(GetDlgItem(hWnd, IDC_EDIT_KEYFRAME_THRESHOLD), buffer);
		}
		if (true)
		{
			HWND hComboBox = GetDlgItem(hWnd, IDC_COMBO_FORMAT);
			int index1 = ComboBox_AddString(hComboBox, _T("XML - plain Text"));
			if (index1 >= 0)
				ComboBox_SetItemData(hComboBox, index1, (LPARAM)DKSerializer::SerializeFormXML);
			int index2 = ComboBox_AddString(hComboBox, _T("XML - binary entity"));
			if (index2 >= 0)
				ComboBox_SetItemData(hComboBox, index2, (LPARAM)DKSerializer::SerializeFormBinXML);
			int index3 = ComboBox_AddString(hComboBox, _T("Binary"));
			if (index3 >= 0)
				ComboBox_SetItemData(hComboBox, index3, (LPARAM)DKSerializer::SerializeFormBinary);
			int index4 = ComboBox_AddString(hComboBox, _T("Compressed Binary"));
			if (index4 >= 0)
				ComboBox_SetItemData(hComboBox, index4, (LPARAM)DKSerializer::SerializeFormCompressedBinary);

			int count = ComboBox_GetCount(hComboBox);
			for (int i = 0; i < count; ++i)
			{
				if (ComboBox_GetItemData(hComboBox, i) == (LPARAM)imp->exportFormat)
				{
					ComboBox_SetCurSel(hComboBox, i);
					break;
				}
			}
			if (ComboBox_GetCurSel(hComboBox) < 0)
			{
				ComboBox_SetCurSel(hComboBox, 0);
				imp->exportFormat = (DKSerializer::SerializeForm)(ComboBox_GetItemData(hComboBox, 0));
			}
		}
		return TRUE;
	case WM_COMMAND:
		if (wParam == IDC_RADIO_SAMPLING)
		{
			EnableWindow(GetDlgItem(hWnd, IDC_STATIC_TEXT_THRESHOLD), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_EDIT_KEYFRAME_THRESHOLD), FALSE);
		}
		else if (wParam == IDC_RADIO_KEYFRAME)
		{
			EnableWindow(GetDlgItem(hWnd, IDC_STATIC_TEXT_THRESHOLD), TRUE);
			EnableWindow(GetDlgItem(hWnd, IDC_EDIT_KEYFRAME_THRESHOLD), TRUE);
		}
		else if (wParam == IDC_BTN_EXPORT)
		{
			imp->exportBoneOnly = IsDlgButtonChecked(hWnd, IDC_CHECK_EXPORT_BONEONLY);
			imp->exportKeyframes = IsDlgButtonChecked(hWnd, IDC_RADIO_KEYFRAME);

			TCHAR buffer[1024];
			if (GetWindowText(GetDlgItem(hWnd, IDC_EDIT_KEYFRAME_THRESHOLD), buffer, 1024) > 0)
				imp->keyframeThreshold = Max<float>(_tcstod(buffer, 0), KEYFRAME_THRESHOLD_DEFAULT);
			else
				imp->keyframeThreshold = KEYFRAME_THRESHOLD_DEFAULT;

			if (true)
			{
				TCHAR fmtStr[1024];
				memset(fmtStr, 0, sizeof(fmtStr));

				HWND hComboBox = GetDlgItem(hWnd, IDC_COMBO_FORMAT);
				int selected = ComboBox_GetCurSel(hComboBox);
				if (selected < 0)		selected = 0;
				imp->exportFormat = (DKSerializer::SerializeForm)(ComboBox_GetItemData(hComboBox, selected));
				ComboBox_GetLBText(hComboBox, selected, fmtStr);
				DKLog("Export Format: %ls\n", (const wchar_t*)DKString(fmtStr));
			}

			EndDialog(hWnd, 1);
		}
		else if (HIWORD(wParam) == EN_KILLFOCUS && LOWORD(wParam) == IDC_EDIT_KEYFRAME_THRESHOLD)
		{
			TCHAR buffer[1024];
			if (GetWindowText(GetDlgItem(hWnd, IDC_EDIT_KEYFRAME_THRESHOLD), buffer, 1024) > 0)
				imp->keyframeThreshold = Max<float>(_tcstod(buffer, 0), KEYFRAME_THRESHOLD_DEFAULT);
			else
				imp->keyframeThreshold = KEYFRAME_THRESHOLD_DEFAULT;
			_stprintf(buffer, _T("%f"), imp->keyframeThreshold);
			SetWindowText(GetDlgItem(hWnd, IDC_EDIT_KEYFRAME_THRESHOLD), buffer);
		}
		break;
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return 1;
	}
	return 0;
}

int	AnimationExporter::DoExport(const MCHAR* name,ExpInterface* ei,Interface* i, BOOL suppressPrompts, DWORD options)
{
#ifndef DK_DEBUG_ENABLED
	SetLogHidden();
#endif

	ReadConfig();
	if(!suppressPrompts)
	{
		int nRet = DialogBoxParam(hInstance, 
			MAKEINTRESOURCE(IDD_ANIMATIONEXPORTER_PANEL), 
			GetActiveWindow(), 
			&ExportOptionsDlgProc, (LPARAM)this);
		if (nRet == 0)
		{
			::MessageBox(GetActiveWindow(), _T("Operation cancelled."), NULL, MB_OK);
			return TRUE;
		}
	}
	WriteConfig();

	// Construct a tab with all this scene's nodes.
	// We could also only take the node currently selected, etc.
	//	INodeTab lNodes;
	//	GetSceneNodes(lNodes);

	// Initialise 3DXI (formerly IGame) object
	// For more information, please see 3ds Max SDK topic PG: Programming with 3DXI.
	IGameScene * pIgame = GetIGameInterface();	
	// 좌표계 변환
	IGameConversionManager *pCM = GetConversionManager();
	pCM->SetCoordSystem(IGameConversionManager::IGAME_OGL);
	// 초기화
	//	pIgame->InitialiseIGame(lNodes);
	pIgame->InitialiseIGame();
	pIgame->SetStaticFrame(0);

	HANDLE hFile = ::CreateFile(name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		long totalFrame = pIgame->GetSceneEndTime() - pIgame->GetSceneStartTime();
		double fps = (1.0f / ((double)GetFrameRate() * (double)pIgame->GetSceneTicks()));
		float fTimeLength = fps * (double)totalFrame;

		DKAnimation	animation;
		animation.SetDuration(fTimeLength);

		// 뼈대 들의 키 프레임 export.
		for (int loop = 0; loop < pIgame->GetTopLevelNodeCount(); loop++)
		{
			IGameNode *pNode = pIgame->GetTopLevelNode(loop);
			if (pNode->IsTarget())
				continue;
			ExportAnimation(animation, pNode);
		}

		DKObject<DKData> data = animation.Serialize(this->exportFormat);
		if (data)
		{
			const void* buffer = data->LockShared();
			DWORD bytesWritten = 0;
			if (data->Length() > 0)
				WriteFile(hFile, buffer, data->Length(), &bytesWritten, 0);
			data->UnlockShared();

			if (bytesWritten < data->Length())
			{
				// error!
				DKLog("File:%ls write error! (%lu / %lu)\n", (const wchar_t*)DKString(name), bytesWritten, data->Length());
				::MessageBox(GetActiveWindow(), _T("An error occurrend while exporting"), NULL, MB_OK);
			}
			else
			{
				DKLog("File:%ls exported. (%lu bytes)\n", (const wchar_t*)DKString(name), bytesWritten);
			}
		}

		pIgame->ReleaseIGame();
		CloseHandle(hFile);

		return TRUE;
	}

	return FALSE;
}

void AnimationExporter::ExportAnimation(DKAnimation& animation, IGameNode* pNode)
{
	const MCHAR* szName = pNode->GetName();
	IGameObject* pObject = pNode->GetIGameObject();
	if ((pObject->GetIGameType() == IGameObject::IGAME_BONE || this->exportBoneOnly == false) && szName != NULL)
	{
		IGameControl *pControl = pNode->GetIGameControl();

		if (pControl)
		{
			// TM 뽑아내기
			IGameKeyTab	key;
			if (pControl->GetFullSampledKeys(key, 1, IGAME_TM, true) && key.Count() > 0)
			{
				GMatrix matLocalTM = pNode->GetLocalTM(0);

				DKAnimation::SamplingNode samplingNode;
				samplingNode.name = szName;
				samplingNode.frames.Reserve(key.Count());

				// 2012-11-30: 여기서 뽑히는 값은 각 노드(뼈대)의 매 프레임간의 LocalTM 이다
				for (int i = 0; i < key.Count(); i++)
				{
					samplingNode.frames.Add(DecompMatrix(key[i].sampleKey.gval));
				}

				// 2010년 7월 9일 금요일 오전 5:10:26 - [tiff]
				// 키프레임은 가져올 IGAME_POS, IGAME_ROT, IGAME_SCALE 로만 가져올 수가 없으므로
				// FullSampled 로 IGAME_TM 행렬을 가져온 후 자체적으로 변환을 해야한다. 느리지만 편한 방법이다.
				// 변환에 실패하면 그냥 샘플링으로 저장.
				DKAnimation::KeyframeNode keyframeNode;
				if (this->exportKeyframes && DKAnimation::ResampleNode(samplingNode, key.Count(), keyframeNode, keyframeThreshold))
				{
					animation.AddNode(&keyframeNode);
				}
				else
				{
					animation.AddNode(&samplingNode);
				}
			}
		}

		for (int i = 0; i < pNode->GetChildCount(); i++)
		{
			IGameNode *pChild = pNode->GetNodeChild(i);
			if (pChild->IsTarget())
				continue;
			ExportAnimation(animation, pChild);
		}
	}

	pNode->ReleaseIGameObject();
}
