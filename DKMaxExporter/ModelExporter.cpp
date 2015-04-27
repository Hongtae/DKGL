#include "ModelExporter.h"
#include "Utilities.h"


ClassDesc2* ModelExporter::ClassDesc(void)
{
	static class ExporterClassDesc : public ClassDesc2 
	{
	public:
		virtual int IsPublic(void) override					{ return TRUE; }
		virtual void* Create(BOOL loading) override 		{ return new ModelExporter(); }
		virtual const MCHAR* ClassName(void) override 		{ return _M("ModelExporter"); }
		virtual SClass_ID SuperClassID(void) override		{ return SCENE_EXPORT_CLASS_ID; }
		virtual Class_ID ClassID(void) override				{ return Class_ID(0x692190aa, 0x7ec02bc2); }
		virtual const MCHAR* Category(void) override		{ return _M(""); }
		virtual const MCHAR* InternalName(void) override	{ return _M("ModelExporter"); }		// returns fixed parsable name (scripter-visible name)
		virtual HINSTANCE HInstance(void) override			{ return hInstance; }				// returns owning module handle
	} desc;
	return &desc;
}

ModelExporter::ModelExporter()
	: exportHiddenNodes(true)
	, exportNormalizeUV(true)
	, exportSkinInfo(true)
	, exportCollision(false)
	, exportHelper(false)
	, exportEmpty(false)
	, exportFormat(DKSerializer::SerializeFormXML)
{
}

ModelExporter::~ModelExporter() 
{
}

int ModelExporter::ExtCount()
{
	// Returns the number of file name extensions supported by the plug-in.
	return 1;
}

const MCHAR *ModelExporter::Ext(int n)
{		
	//#pragma message	(TODO("Return the 'i-th' file name extension (i.e. \"3DS\")."))
	return _M("DKModel");
}

const MCHAR *ModelExporter::LongDesc()
{
	//#pragma message	(TODO("Return long ASCII description (i.e. \"Targa 2.0 Image File\""))
	return _M("DK Model Exporter");
}

const MCHAR *ModelExporter::ShortDesc()
{			
	//#pragma message	(TODO("Return short ASCII description (i.e. \"Targa\")"))
	return _M("DKModel");
}

const MCHAR *ModelExporter::AuthorName()
{			
	//#pragma message	(TODO("Return ASCII Author name"))
	return _M("TIFF");
}

const MCHAR *ModelExporter::CopyrightMessage()
{	
	//#pragma message	(TODO("Return ASCII Copyright message"))
	return _M("Copyright(c) 2011-2014, ICONDB.COM");
}

const MCHAR *ModelExporter::OtherMessage1()
{		
	//TODO: Return Other message #1 if any
	return _M("");
}

const MCHAR *ModelExporter::OtherMessage2()
{		
	//TODO: Return other message #2 in any
	return _M("");
}

unsigned int ModelExporter::Version()
{
	//#pragma message	(TODO("Return Version number * 100 (i.e. v3.01 = 301)"))
	return 101;
}

void ModelExporter::ShowAbout(HWND hWnd)
{			
	// Optional
}

BOOL ModelExporter::SupportsOptions(int ext, DWORD options)
{
	// TODO Decide which options to support.  Simply return
	// true for each option supported by each Extension 
	// the exporter supports.

	return TRUE;
}

int ModelExporter::GetSceneNodes(INodeTab& i_nodeTab, INode* i_currentNode /*=NULL*/)
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

void ModelExporter::ReadConfig()
{
	LoadConfig();

	const wchar_t* exportHiddenNodesPropertyKey = L"ModelExporter.exportHiddenNodes";
	const wchar_t* exportNormalizeUVKey = L"ModelExporter.exportNormalizeUV";
	const wchar_t* exportSkinInfoPropertyKey = L"ModelExporter.exportSkinInfo";
	const wchar_t* exportCollisionPropertyKey = L"ModelExporter.exportCollision";
	const wchar_t* exportHelperPropertyKey = L"ModelExporter.exportHelper";
	const wchar_t* exportEmptyPropertyKey = L"ModelExporter.exportEmpty";
	const wchar_t* exportFormatPropertyKey = L"ModelExporter.exportFormat";

	// exportHiddenNodes
	if (DKPropertySet::DefaultSet().Value(exportHiddenNodesPropertyKey).ValueType() == DKVariant::TypeInteger)
		exportHiddenNodes = DKPropertySet::DefaultSet().Value(exportHiddenNodesPropertyKey).Integer() != 0;

	// exportNormalizeUV
	if (DKPropertySet::DefaultSet().Value(exportNormalizeUVKey).ValueType() == DKVariant::TypeInteger)
		exportNormalizeUV = DKPropertySet::DefaultSet().Value(exportNormalizeUVKey).Integer() != 0;

	// exportSkinInfo
	if (DKPropertySet::DefaultSet().Value(exportSkinInfoPropertyKey).ValueType() == DKVariant::TypeInteger)
		exportSkinInfo = DKPropertySet::DefaultSet().Value(exportSkinInfoPropertyKey).Integer() != 0;

	// exportCollision
	if (DKPropertySet::DefaultSet().Value(exportCollisionPropertyKey).ValueType() == DKVariant::TypeInteger)
		exportCollision = DKPropertySet::DefaultSet().Value(exportCollisionPropertyKey).Integer() != 0;

	// exportHelper
	if (DKPropertySet::DefaultSet().Value(exportHelperPropertyKey).ValueType() == DKVariant::TypeInteger)
		exportHelper = DKPropertySet::DefaultSet().Value(exportHelperPropertyKey).Integer() != 0;

	// exportEmpty
	if (DKPropertySet::DefaultSet().Value(exportEmptyPropertyKey).ValueType() == DKVariant::TypeInteger)
		exportEmpty = DKPropertySet::DefaultSet().Value(exportEmptyPropertyKey).Integer() != 0;

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

void ModelExporter::WriteConfig() const
{
	const wchar_t* exportHiddenNodesPropertyKey = L"ModelExporter.exportHiddenNodes";
	const wchar_t* exportNormalizeUVKey = L"ModelExporter.exportNormalizeUV";
	const wchar_t* exportSkinInfoPropertyKey = L"ModelExporter.exportSkinInfo";
	const wchar_t* exportCollisionPropertyKey = L"ModelExporter.exportCollision";
	const wchar_t* exportHelperPropertyKey = L"ModelExporter.exportHelper";
	const wchar_t* exportEmptyPropertyKey = L"ModelExporter.exportEmpty";
	const wchar_t* exportFormatPropertyKey = L"ModelExporter.exportFormat";

	DKPropertySet::DefaultSet().SetValue(exportHiddenNodesPropertyKey, (DKVariant::VInteger)exportHiddenNodes);
	DKPropertySet::DefaultSet().SetValue(exportNormalizeUVKey, (DKVariant::VInteger)exportNormalizeUV);
	DKPropertySet::DefaultSet().SetValue(exportSkinInfoPropertyKey, (DKVariant::VInteger)exportSkinInfo);
	DKPropertySet::DefaultSet().SetValue(exportCollisionPropertyKey, (DKVariant::VInteger)exportCollision);
	DKPropertySet::DefaultSet().SetValue(exportHelperPropertyKey, (DKVariant::VInteger)exportHelper);
	DKPropertySet::DefaultSet().SetValue(exportEmptyPropertyKey, (DKVariant::VInteger)exportEmpty);
	DKPropertySet::DefaultSet().SetValue(exportFormatPropertyKey, (DKVariant::VInteger)exportFormat);

	SaveConfig();
}

INT_PTR CALLBACK ModelExporter::ExportOptionsDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	static ModelExporter *imp = NULL;

	switch(message)
	{
	case WM_INITDIALOG:
		imp = (ModelExporter *)lParam;
		CenterWindow(hWnd,GetParent(hWnd));
		CheckDlgButton(hWnd, IDC_CHECK_HIDDEN_NODE, imp->exportHiddenNodes ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_NORMALIZE_UV, imp->exportNormalizeUV ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_SKIN_INFO, imp->exportSkinInfo ? BST_CHECKED : BST_UNCHECKED);		
		CheckDlgButton(hWnd, IDC_CHECK_EXPORT_COLLISION, imp->exportCollision ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_EXPORT_HELPER, imp->exportHelper ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_CHECK_EXPORT_EMPTY, imp->exportEmpty ? BST_CHECKED : BST_UNCHECKED);

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
		switch (wParam)
		{
		case IDC_BTN_EXPORT:
			imp->exportHiddenNodes = IsDlgButtonChecked(hWnd, IDC_CHECK_HIDDEN_NODE);
			imp->exportNormalizeUV = IsDlgButtonChecked(hWnd, IDC_CHECK_NORMALIZE_UV);
			imp->exportSkinInfo = IsDlgButtonChecked(hWnd, IDC_CHECK_SKIN_INFO);
			imp->exportCollision = IsDlgButtonChecked(hWnd, IDC_CHECK_EXPORT_COLLISION);
			imp->exportHelper = IsDlgButtonChecked(hWnd, IDC_CHECK_EXPORT_HELPER);
			imp->exportEmpty = IsDlgButtonChecked(hWnd, IDC_CHECK_EXPORT_EMPTY);

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
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return 1;
	}
	return 0;
}

int	ModelExporter::DoExport(const MCHAR* name,ExpInterface* ei,Interface* i, BOOL suppressPrompts, DWORD options)
{
	DKApplication app;
#ifndef DK_DEBUG_ENABLED
	SetLogHidden();
#endif

	ReadConfig();
	if(!suppressPrompts)
	{
		int nRet = DialogBoxParam(hInstance, 
			MAKEINTRESOURCE(IDD_MODELEXPORTER_PANEL), 
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
		// OpenGL 핸들이 있다면 일단 백업
		HGLRC hGLCurrent = wglGetCurrentContext();
		HDC hDC = wglGetCurrentDC();

		if (hGLCurrent)
		{
			wglMakeCurrent(NULL, NULL);
		}

		DKObject<DKOpenGLContext> glContext = DKOpenGLContext::SharedInstance();
		if (glContext)
		{
			DKContextScopeBinder<DKOpenGLContext> binder(glContext);

			DKObject<DKModel> model = DKObject<DKModel>::New();	// 익스포트 할 모델 (이 안에 모든 메쉬가 들어간다)
			GMatrix baseTM;
			baseTM.SetIdentity();

			////////////////////////////////////////////////////////////////////////////////
			// 매쉬 쓰기
			for (int loop = 0; loop < pIgame->GetTopLevelNodeCount(); loop++)
			{
				IGameNode *pNode = pIgame->GetTopLevelNode(loop);
				if (pNode->IsTarget())
					continue;

				ExportModel(pNode, model, baseTM);
			}

			// 자식들의 빈 노드 제거 (매쉬도 없고 자식도 없는것들)
			if (!this->exportEmpty)
				RemoveEmptyModelTree(model);

			if (this->exportCollision)
				CreateP2PConstraints(NULL, model, DKNSTransform().Identity());

			DKObject<DKData> data = model->Serialize(this->exportFormat);
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
		}
		else
		{
			// error!
			::MessageBox(GetActiveWindow(), _T("Failed to initialize OpenGL"), NULL, MB_OK);
		}

		if (hGLCurrent)
		{
			wglMakeCurrent(hDC, hGLCurrent);
		}

		pIgame->ReleaseIGame();
		CloseHandle(hFile);


		return TRUE;
	}

	pIgame->ReleaseIGame();
	return FALSE;
}

void ModelExporter::ExportModel(IGameNode* pIGameNode, DKModel* parentModel, GMatrix parentTM) const
{
	int nodeId = pIGameNode->GetNodeID();
	DKString nodeName = pIGameNode->GetName();

	GMatrix worldTM = pIGameNode->GetWorldTM(0);
	GMatrix invParentTM = parentTM.Inverse();
	GMatrix localTM = worldTM * invParentTM;

	// 맥스에서는 자식과 상관없이 자신만 숨길수 있으므로, 해당 매쉬만 판별함. (exportHiddenNodes 가 true 이면 그냥 다 추출함)
	bool hidden = pIGameNode->IsNodeHidden();

	IGameObject* pIGameObject = pIGameNode->GetIGameObject();

	DKObject<DKModel> model = NULL;
	DKTransformUnit localTransform = DKTransformUnit().Identity();

	if (pIGameObject->GetIGameType() == IGameObject::IGAME_MESH)
	{
		if (hidden == false || this->exportHiddenNodes)
		{
			// 매쉬 데이터 추출
			IGameMesh* pIGameMesh = static_cast<IGameMesh*>(pIGameObject);
			if (pIGameMesh && pIGameMesh->InitializeData())
			{
				GMatrix basis = worldTM.Inverse();
				localTransform = DecompMatrix(localTM);

				DKArray<DKObject<DKMesh>> meshes = ExportMesh(pIGameMesh, basis);
				if (meshes.Count() > 1)
				{
					model = DKObject<DKModel>::New();
					model->SetName(nodeName);

					for (size_t i = 0; i < meshes.Count(); ++i)
					{
						DKMesh* mesh = meshes.Value(i);
						DKString resName = DKString().Format("%ls.MeshPart:%u", (const wchar_t*)nodeName, i);
						mesh->SetName(resName);
						mesh->SetHidden(hidden);
						model->AddChild(mesh);
					}					
				}
				else if (meshes.Count() == 1)
				{
					DKMesh* mesh = meshes.Value(0);
					mesh->SetName(nodeName);
					mesh->SetHidden(hidden);
					model = mesh;
				}
			}
		}
	}
	else
	{
		IGameMesh* pIGameMesh = NULL;
		if (pIGameObject->GetIGameType() == IGameObject::IGAME_BONE)
		{
			if (this->exportCollision)
				pIGameMesh = static_cast<IGameSupportObject*>(pIGameObject)->GetMeshObject();
		}
		else if (pIGameObject->GetIGameType() == IGameObject::IGAME_HELPER)
		{
			if (this->exportHelper && this->exportCollision)
				pIGameMesh = static_cast<IGameSupportObject*>(pIGameObject)->GetMeshObject();
		}

		// 매쉬에서 collision data 추출
		if (pIGameMesh && pIGameMesh->InitializeData())
		{
			// 뼈대에 스케일이 들어가 있는것도 있기 때문에 IGameNode의 WorldTM 의 역행렬을 곱해준다.
			DKObject<DKCollisionObject> collisionObject = ExportCollisionObject(pIGameMesh, pIGameNode->GetWorldTM(0).Inverse());
			if (collisionObject)
			{
				localTransform = DecompMatrix(localTM);

				collisionObject->SetName(nodeName);
				model = collisionObject;
			}
		}
	}

	pIGameNode->ReleaseIGameObject();
	
	if (model)
	{
		model->SetLocalTransform(DKNSTransform(localTransform.rotation, localTransform.translation));
		parentModel->AddChild(model);
		parentModel = model;
		parentTM = worldTM;
	}

	// 자식 모델 추출
	for (int i = 0; i < pIGameNode->GetChildCount(); ++i)
	{
		IGameNode* pNode = pIGameNode->GetNodeChild(i);
		if (pNode->IsTarget())
			continue;

		ExportModel(pNode, parentModel, parentTM);
	}
}

struct VertexIndex
{
	int vert;
	int norm;
	DKVector2 texUV;
	bool operator < (const VertexIndex& rhs) const
	{
		if (this->vert == rhs.vert)
		{
			if (this->norm == rhs.norm)
			{
				if (this->texUV.x == rhs.texUV.x)
					return this->texUV.y < rhs.texUV.y;
				return this->texUV.x < rhs.texUV.x;
			}
			return this->norm < rhs.norm;
		}
		return this->vert < rhs.vert;
	}
	bool operator > (const VertexIndex& rhs) const
	{
		if (this->vert == rhs.vert)
		{
			if (this->norm == rhs.norm)
			{
				if (this->texUV.x == rhs.texUV.x)
					return this->texUV.y > rhs.texUV.y;
				return this->texUV.x > rhs.texUV.x;
			}
			return this->norm > rhs.norm;
		}
		return this->vert > rhs.vert;
	}
	bool operator == (const VertexIndex& rhs) const
	{
		return this->vert == rhs.vert && this->norm == rhs.norm && this->texUV.x == rhs.texUV.x && this->texUV.y == rhs.texUV.y;
	}
};

struct VertexSkinInfo
{
	unsigned short indices[4];
	float weights[4];
};

DKArray<DKObject<DKMesh>> ModelExporter::ExportMesh(IGameMesh* pIGameMesh, const GMatrix& basis) const
{
	DKArray<DKObject<DKMesh>> ret;
	Matrix3 mat3;
	mat3.IdentityMatrix();
	mat3.SetRotate(basis.Rotation());
	GMatrix orient(mat3);

	int numVerts = pIGameMesh->GetNumberOfVerts();
	int numNormals = pIGameMesh->GetNumberOfNormals();
	int numTexCoords = pIGameMesh->GetNumberOfTexVerts();
	int numFaces = pIGameMesh->GetNumberOfFaces();

	if (numVerts == 0 || numFaces == 0 || numTexCoords == 0 || numNormals == 0)
		return ret;

	////////////////////////////////////////////////////////////////////////////////
	// 각 face 들을 material 별로 나눈다.
	typedef DKMap<IGameMaterial*, DKArray<FaceEx*>> FaceArrayByMaterial;
	FaceArrayByMaterial facesByMaterialMap;
	for (int i = 0; i < numFaces; ++i)
	{
		FaceEx* face = pIGameMesh->GetFace(i);

		if (face->flags & FACE_HIDDEN)
			continue;

		IGameMaterial* material = pIGameMesh->GetMaterialFromFace(face);
		facesByMaterialMap.Value(material).Add(face);
	}

	DKMap<VertexIndex, unsigned int> vertexMap;		// 중복된 버텍스 검색용
	DKArray<VertexIndex> vertexIndices;
	DKArray<unsigned int> indices;

	facesByMaterialMap.EnumerateForward([&](const FaceArrayByMaterial::Pair& pair)
	{
		IGameMaterial* material = pair.key;
		const DKArray<FaceEx*>& faces = pair.value;
		if (faces.IsEmpty())
			return;

		////////////////////////////////////////////////////////////////////////////////
		// 인덱스 재구성
		// uv, 노멀, 버텍스 좌표에 따라 나누게 되므로 실제 버텍스가 더 많아진다.
		vertexMap.Clear();
		vertexIndices.Clear();
		indices.Clear();
		vertexIndices.Reserve(faces.Count() * 3);
		indices.Reserve(faces.Count() * 3);

		// uv 좌표가 normalized 가능한지 확인
		// exportNormalizedUV 가 켜져있으면 범위(-1.0~1.0)를 벗어난 uv 좌표를 범위 안으로 이동함

		bool normalizeUV = false;
		if (this->exportNormalizeUV)
		{
			int uvLength = 0;
			for (const FaceEx* face : faces)
			{
				Point2 tex[3] = {
					pIGameMesh->GetTexVertex(face->texCoord[0]),
					pIGameMesh->GetTexVertex(face->texCoord[1]),
					pIGameMesh->GetTexVertex(face->texCoord[2])
				};
				int minU = floor(Min<float>(tex[0].x, Min<float>(tex[1].x, tex[2].x)));
				int minV = floor(Min<float>(tex[0].y, Min<float>(tex[1].y, tex[2].y)));
				int maxU = ceil(Max<float>(tex[0].x, Max<float>(tex[1].x, tex[2].x)));
				int maxV = ceil(Max<float>(tex[0].y, Max<float>(tex[1].y, tex[2].y)));

				int sizeU = maxU - minU;
				int sizeV = maxV - minV;

				int length = Max<int>(sizeU, sizeV);
				if (uvLength < length)
					uvLength = length;				
			}
			if (uvLength < 3)
				normalizeUV = true;	// uvLength 가 1 이면 ushort2, 2 이면 short2 로 변형 가능함
		}

		for (const FaceEx* face : faces)
		{
			Point2 tex[3] = 
			{
				pIGameMesh->GetTexVertex(face->texCoord[0]),
				pIGameMesh->GetTexVertex(face->texCoord[1]),
				pIGameMesh->GetTexVertex(face->texCoord[2])
			};

			if (normalizeUV)		// 텍스쳐 좌표 범위 조정 (short 또는 unsigned short 으로)
			{
				float minU = floor(Min<float>(tex[0].x, Min<float>(tex[1].x, tex[2].x)));
				float maxU = ceil(Max<float>(tex[0].x, Max<float>(tex[1].x, tex[2].x)));
				float sizeU = maxU - minU;
				if (sizeU > 1.0f)	// -1.0~1.0 사이로 범위 조정 (short)
				{
					float center = (maxU + minU) / sizeU;
					short value[3] = {
						static_cast<short>((tex[0].x - center) * static_cast<float>(0x7fff)),
						static_cast<short>((tex[1].x - center) * static_cast<float>(0x7fff)),
						static_cast<short>((tex[2].x - center) * static_cast<float>(0x7fff))
					};
					tex[0].x = static_cast<float>(value[0]) / static_cast<float>(0x7fff);
					tex[1].x = static_cast<float>(value[1]) / static_cast<float>(0x7fff);
					tex[2].x = static_cast<float>(value[2]) / static_cast<float>(0x7fff);
				}
				else if (sizeU > 0.0f)	// 0.0~1.0 사이로 범위 조정 (unsigned short)
				{
					unsigned short value[3] = {
						static_cast<unsigned short>((tex[0].x - minU) * static_cast<float>(0xffff)),
						static_cast<unsigned short>((tex[1].x - minU) * static_cast<float>(0xffff)),
						static_cast<unsigned short>((tex[2].x - minU) * static_cast<float>(0xffff))
					};
					tex[0].x = static_cast<float>(value[0]) / static_cast<float>(0xffff);
					tex[1].x = static_cast<float>(value[1]) / static_cast<float>(0xffff);
					tex[2].x = static_cast<float>(value[2]) / static_cast<float>(0xffff);
				}
				else	// 모두 같은 지점 (정수부분)
				{
					tex[0].x -= minU;
					tex[1].x -= minU;
					tex[2].x -= minU;
				}

				float minV = floor(Min<float>(tex[0].y, Min<float>(tex[1].y, tex[2].y)));
				float maxV = ceil(Max<float>(tex[0].y, Max<float>(tex[1].y, tex[2].y)));
				float sizeV = maxV - minV;
				if (sizeV > 1.0f)	// -1.0~1.0 사이로 범위 조정 (short)
				{
					float center = (maxV + minV) / sizeV;
					short value[3] = {
						static_cast<short>((tex[0].y - center) * static_cast<float>(0x7fff)),
						static_cast<short>((tex[1].y - center) * static_cast<float>(0x7fff)),
						static_cast<short>((tex[2].y - center) * static_cast<float>(0x7fff))
					};
					tex[0].y = static_cast<float>(value[0]) / static_cast<float>(0x7fff);
					tex[1].y = static_cast<float>(value[1]) / static_cast<float>(0x7fff);
					tex[2].y = static_cast<float>(value[2]) / static_cast<float>(0x7fff);
				}
				else if (sizeV > 0.0f)	// 0.0~1.0 사이로 범위 조정 (unsigned short)
				{
					unsigned short value[3] = {
						static_cast<unsigned short>((tex[0].y - minV) * static_cast<float>(0xffff)),
						static_cast<unsigned short>((tex[1].y - minV) * static_cast<float>(0xffff)),
						static_cast<unsigned short>((tex[2].y - minV) * static_cast<float>(0xffff))
					};
					tex[0].y = static_cast<float>(value[0]) / static_cast<float>(0xffff);
					tex[1].y = static_cast<float>(value[1]) / static_cast<float>(0xffff);
					tex[2].y = static_cast<float>(value[2]) / static_cast<float>(0xffff);
				}
				else	// 모두 같은 지점 (정수부분)
				{
					tex[0].y -= minV;
					tex[1].y -= minV;
					tex[2].y -= minV;
				}
			}

			for (int j = 0; j < 3; ++j)
			{
				unsigned int index = 0;
				VertexIndex vi = {face->vert[j], face->norm[j], DKVector2(tex[j].x, tex[j].y)};
				DKMap<VertexIndex, unsigned int>::Pair* p = vertexMap.Find(vi);
				if (p)
					index = p->value;
				else
				{
					vertexIndices.Add(vi);
					index = vertexIndices.Count() - 1;
					vertexMap.Insert(vi, index);
				}
				indices.Add(index);
			}
		}

		////////////////////////////////////////////////////////////////////////////////
		// 인덱스 버퍼 생성
		DKObject<DKIndexBuffer> indexBuffer = NULL;
		if (faces.Count() > 0xffff)	// unsigned int 로 생성
		{
			indexBuffer = DKIndexBuffer::Create((const unsigned int*)indices, indices.Count(), 
				DKPrimitive::TypeTriangles, DKIndexBuffer::MemoryLocationStatic, DKIndexBuffer::BufferUsageDraw);
		}
		else			// unsigned short 으로 생성
		{
			DKArray<unsigned short> index2;
			index2.Reserve(indices.Count());
			for (size_t i = 0; i < indices.Count(); ++i)
				index2.Add(static_cast<unsigned short>(indices.Value(i)));

			indexBuffer = DKIndexBuffer::Create((const unsigned short*)index2, index2.Count(), 
				DKPrimitive::TypeTriangles, DKIndexBuffer::MemoryLocationStatic, DKIndexBuffer::BufferUsageDraw);
		}

		if (indexBuffer == NULL)
			return;

		DKArray<DKVector3> positions;
		DKArray<DKVector3> normals;
		DKArray<DKVector2> texCoords;
		positions.Reserve(vertexIndices.Count());
		normals.Reserve(vertexIndices.Count());
		texCoords.Reserve(vertexIndices.Count());

		////////////////////////////////////////////////////////////////////////////////
		// position, texCoord, normal 데이터 추출
		for (VertexIndex& vi : vertexIndices)
		{
			Point3 pos = pIGameMesh->GetVertex(vi.vert, false) * basis;
			Point3 norm = pIGameMesh->GetNormal(vi.norm, false) * orient;
			norm.Normalize();

			positions.Add(DKVector3(pos.x, pos.y, pos.z));
			normals.Add(DKVector3(norm.x, norm.y, norm.z));
			texCoords.Add(vi.texUV);
		}
		DKAABox boundingBox;
		DKSphere boundingSphere;
		// 바운딩 박스 구하기
		for (const DKVector3& pos : positions)
		{
			if (pos.x > boundingBox.positionMax.x)		boundingBox.positionMax.x = pos.x;
			if (pos.x < boundingBox.positionMin.x)		boundingBox.positionMin.x = pos.x;
			if (pos.y > boundingBox.positionMax.y)		boundingBox.positionMax.y = pos.y;
			if (pos.y < boundingBox.positionMin.y)		boundingBox.positionMin.y = pos.y;
			if (pos.z > boundingBox.positionMax.z)		boundingBox.positionMax.z = pos.z;
			if (pos.z < boundingBox.positionMin.z)		boundingBox.positionMin.z = pos.z;
		}
		if (boundingBox.IsValid())
		{
			boundingSphere.center = boundingBox.Center();

			float radiusSq = 0;

			for (const DKVector3& pos : positions)
			{
				float lenSq = (boundingSphere.center - pos).LengthSq();
				if (lenSq > radiusSq)
					radiusSq = lenSq;
			}
			boundingSphere.radius = sqrt(radiusSq);
		}

		DKArray<VertexSkinInfo> skinInfos;
		DKArray<DKSkinMesh::Bone> boneNodes;

		IGameSkin* pIGameSkin = NULL;
		if (this->exportSkinInfo)
			pIGameSkin = pIGameMesh->GetIGameSkin();

		if (pIGameSkin && pIGameSkin->GetNumOfSkinnedVerts() >= numVerts &&
			pIGameSkin->GetTotalBoneCount() <= 0xffff)		// unsigned short 으로 해결가능해야함
		{
			skinInfos.Reserve(vertexIndices.Count());

			DKArray<IGameNode*> boneArray;		// 뼈대의 인덱스를 다시 계산하기 위한 배열
			boneArray.Reserve(pIGameSkin->GetTotalBoneCount());

			typedef DKMap<int, int> BoneArrayIndexMap; // key = boneId, value = boneArray index
			BoneArrayIndexMap boneIndexMap;  // 뼈대 배열의 인덱스 맵

			// 스키닝 정보 추출
			for (VertexIndex& vi : vertexIndices)
			{
				VertexSkinInfo skin;
				int numBones = pIGameSkin->GetNumberOfBones(vi.vert);
				int boneCount = 0;
				for (int k = 0; k < numBones && boneCount < 4; ++k)
				{
					float weight = pIGameSkin->GetWeight(vi.vert, k);
					if (weight > 0.0f)	// weight 가 0 이하면 무시함
					{
						IGameNode* bone = pIGameSkin->GetIGameBone(vi.vert, k);
						int boneId = pIGameSkin->GetBoneID(vi.vert, k);
						int boneIndex = 0;

						const BoneArrayIndexMap::Pair* p = boneIndexMap.Find(boneId);
						if (p)
						{
							boneIndex = p->value;
						}
						else
						{
							boneIndex = boneArray.Count();
							boneArray.Add(bone);
							boneIndexMap.Insert(boneId, boneIndex);
						}
						skin.indices[boneCount] = boneIndex;
						skin.weights[boneCount] = weight;
						boneCount++;
					}
				}
				while (boneCount < 4)	// 나머지는 0 으로 채움
				{
					skin.indices[boneCount] = 0;
					skin.weights[boneCount] = 0.0f;
					boneCount++;
				}

				skinInfos.Add(skin);
			}

			// 본 이름, 월드 행렬 가져오기
			boneNodes.Reserve(boneArray.Count());
			for (IGameNode* bone : boneArray)
			{
				DKSkinMesh::Bone boneInfo = { bone->GetName(), DecompMatrix(bone->GetWorldTM(0) * basis).Matrix4() };
				boneNodes.Add(boneInfo);
			}
		}

		////////////////////////////////////////////////////////////////////////////////
		// 매쉬 생성 (정적인 매시만 생성한다)
		//  - DKSkinMesh: 쉐이더 스키닝 매시
		//  - DKStaticMesh: 일반 매쉬

		DKObject<DKStaticMesh> mesh = NULL;
		if (this->exportSkinInfo && boneNodes.Count() > 0 && skinInfos.Count() > 0)
		{
			DKObject<DKSkinMesh> skinMesh = DKObject<DKSkinMesh>::New();
			skinMesh->SetBones(boneNodes);
			mesh = skinMesh.SafeCast<DKStaticMesh>();
		}
		if (mesh == NULL)
			mesh = DKObject<DKStaticMesh>::New();		// Non-skinning or non-linked

		if (mesh)
		{
			mesh->SetDrawFace(DKMesh::DrawFaceCCW);
			mesh->SetIndexBuffer(indexBuffer);

			DKMesh::TextureSamplerMap matTextures;
			DKMesh::PropertyMap matProperties;
			ExportTextures(material, matTextures);
			ExportMaterialProperties(material, matProperties);

			mesh->SamplerMap() = matTextures;
			mesh->MaterialPropertyMap() = matProperties;
			mesh->SetBoundingAABox(boundingBox);
			mesh->SetBoundingSphere(boundingSphere);

			if (positions.Count() > 0)		// StreamPosition 추출
			{
				////////////////////////////////////////////////////////////////////////////////
				// StreamPosition 버퍼 생성
				DKObject<DKVertexBuffer> buffer = NULL;

				DKVertexBuffer::Decl d = {DKVertexStream::StreamPosition, L"", DKVertexStream::TypeFloat3, false, 0};
				buffer = DKVertexBuffer::Create(&d, 1, (const DKVector3*)positions, DKVertexStream::TypeSize(d.type), positions.Count(),
					DKVertexBuffer::MemoryLocationStatic, DKVertexBuffer::BufferUsageDraw);

				if (buffer)
					mesh->AddVertexBuffer(buffer);
			}
			if (texCoords.Count() > 0)		// StreamTexCoord 추출
			{
				////////////////////////////////////////////////////////////////////////////////
				// StreamTexCoord 버퍼 생성 (크기에 맞게 float4, short4, ushrot4 로 생성)
				DKObject<DKVertexBuffer> buffer = NULL;

				// UV 범위 계산한다.
				// 범위가 -1 ~ +1 사이면 short
				// 범위가 0 ~ 1 사이면 unsigned short
				// 그외엔 float
				DKVector2 uvMin(FLT_MAX, FLT_MAX);
				DKVector2 uvMax(-FLT_MAX, -FLT_MAX);
				for (const DKVector2& uv : texCoords)
				{
					if (uvMin.x > uv.x)	uvMin.x = uv.x;
					if (uvMin.y > uv.y) uvMin.y = uv.y;
					if (uvMax.x < uv.x) uvMax.x = uv.x;
					if (uvMax.y < uv.y) uvMax.y = uv.y;
				}
				uvMax = DKVector2(ceil(uvMax.x), ceil(uvMax.y));
				uvMin = DKVector2(floor(uvMin.x), floor(uvMin.y));

				DKVector2 uvSize = uvMax - uvMin;
				if (uvSize.x > 2.0 || uvSize.y > 2.0)
				{
					// float 형식 (그냥 바로 복사함)
					DKVertexBuffer::Decl d = {DKVertexStream::StreamTexCoord, L"", DKVertexStream::TypeFloat2, false, 0};
					buffer = DKVertexBuffer::Create(&d, 1, (const DKVector2*)texCoords, DKVertexStream::TypeSize(d.type), texCoords.Count(),
						DKVertexBuffer::MemoryLocationStatic, DKVertexBuffer::BufferUsageDraw);
				}
				else if (uvSize.x > 1.0 || uvSize.y > 1.0)
				{
					// short 형식 ( -1.0 ~ +1.0 )
					DKArray<short> uvData;
					uvData.Reserve(texCoords.Count() * 2);
					DKVector2 uvCenter = (uvMax + uvMin);
					uvCenter.x = uvCenter.x / uvSize.x;
					uvCenter.y = uvCenter.y / uvSize.y;

					for (const DKVector2& uv : texCoords)
					{
						short value[2] = {
							static_cast<short>( (uv.x - uvCenter.x) * static_cast<float>(0x7fff) ),
							static_cast<short>( (uv.y - uvCenter.y) * static_cast<float>(0x7fff) )
						};
						uvData.Add(value, 2);
					}
					DKVertexBuffer::Decl d = {DKVertexStream::StreamTexCoord, L"", DKVertexStream::TypeShort2, true, 0};
					buffer = DKVertexBuffer::Create(&d, 1, (const short*)uvData, DKVertexStream::TypeSize(d.type), texCoords.Count(),
						DKVertexBuffer::MemoryLocationStatic, DKVertexBuffer::BufferUsageDraw);
				}
				else
				{
					// unsigned short 형식 ( 0.0 ~ 1.0 )
					DKArray<unsigned short> uvData;
					uvData.Reserve(texCoords.Count() * 2);

					for (const DKVector2& uv : texCoords)
					{
						unsigned short value[2] = {
							static_cast<unsigned short>( (uv.x - uvMin.x) * static_cast<float>(0xffff) ),
							static_cast<unsigned short>( (uv.y - uvMin.y) * static_cast<float>(0xffff) )
						};
						uvData.Add(value, 2);
					}
					DKVertexBuffer::Decl d = {DKVertexStream::StreamTexCoord, L"", DKVertexStream::TypeUShort2, true, 0};
					buffer = DKVertexBuffer::Create(&d, 1, (const unsigned short*)uvData, DKVertexStream::TypeSize(d.type), texCoords.Count(),
						DKVertexBuffer::MemoryLocationStatic, DKVertexBuffer::BufferUsageDraw);
				}
				if (buffer)
					mesh->AddVertexBuffer(buffer);
			}
			if (normals.Count() > 0)		// StreamNormal 추출
			{
				////////////////////////////////////////////////////////////////////////////////
				// StreamNormal 버퍼 생성 (UShort4 로 사용한다. 4bytes alignment 때문에 4의 배수여야함)
				DKObject<DKVertexBuffer> buffer = NULL;

				DKArray<short> normData;
				normData.Reserve(normals.Count() * 4);
				for (const DKVector3& n : normals)
				{
					short value[4] = {
						static_cast<short>( n.x * static_cast<float>(0x7fff) ),
						static_cast<short>( n.y * static_cast<float>(0x7fff) ),
						static_cast<short>( n.z * static_cast<float>(0x7fff) ),
						0			// 4bytes alignment 맞추기 위해 그냥 넣음.
					};
					normData.Add(value, 4);
				}
				DKVertexBuffer::Decl d = {DKVertexStream::StreamNormal, L"", DKVertexStream::TypeShort4, true, 0};
				buffer = DKVertexBuffer::Create(&d, 1, (const short*)normData, DKVertexStream::TypeSize(d.type), normals.Count(),
					DKVertexBuffer::MemoryLocationStatic, DKVertexBuffer::BufferUsageDraw);

				if (buffer)
					mesh->AddVertexBuffer(buffer);
			}
			if (boneNodes.Count() > 0 && skinInfos.Count() > 0)		// StreamBlendIndices, StreamBlendWeights 추출
			{
				////////////////////////////////////////////////////////////////////////////////
				// BlendIndices, BlendWeights 버퍼 생성
				DKObject<DKVertexBuffer> buffer = NULL;

				if (boneNodes.Count() > 0xff)	// UShort4, UShort4 사용
				{
					struct SkinVertex16
					{
						unsigned short indices[4];
						unsigned short weights[4];
					};
					DKArray<SkinVertex16> skinData;
					skinData.Reserve(skinInfos.Count());
					for (VertexSkinInfo& vert : skinInfos)
					{
						SkinVertex16 sv16;
						for (int k = 0; k < 4; ++k)
						{
							sv16.indices[k] = vert.indices[k];
							sv16.weights[k] = static_cast<unsigned short>(Clamp<float>(vert.weights[k], 0.0, 1.0) * static_cast<float>(0xffff));
						}
						skinData.Add(sv16);
					}
					DKVertexBuffer::Decl decls[2] = { 
						{DKVertexStream::StreamBlendIndices, L"", DKVertexStream::TypeUShort4, false, 0},
						{DKVertexStream::StreamBlendWeights, L"", DKVertexStream::TypeUShort4, true, DKVertexStream::TypeSize(DKVertexStream::TypeUShort4)}
					};
					buffer = DKVertexBuffer::Create(decls, 2, (SkinVertex16*)skinData, sizeof(SkinVertex16), skinData.Count(),
						DKVertexBuffer::MemoryLocationStatic, DKVertexBuffer::BufferUsageDraw);
				}
				else							// UByte4, UShort4 사용
				{
					struct SkinVertex12
					{
						unsigned char indices[4];
						unsigned short weights[4];
					};
					DKArray<SkinVertex12> skinData;
					skinData.Reserve(skinInfos.Count());
					for (VertexSkinInfo& vert : skinInfos)
					{
						SkinVertex12 sv12;
						for (int k = 0; k < 4; ++k)
						{
							sv12.indices[k] = static_cast<unsigned char>(vert.indices[k]);
							sv12.weights[k] = static_cast<unsigned short>(Clamp<float>(vert.weights[k], 0.0, 1.0) * static_cast<float>(0xffff));
						}
						skinData.Add(sv12);
					}
					DKVertexBuffer::Decl decls[2] = { 
						{DKVertexStream::StreamBlendIndices, L"", DKVertexStream::TypeUByte4, false, 0},
						{DKVertexStream::StreamBlendWeights, L"", DKVertexStream::TypeUShort4, true, DKVertexStream::TypeSize(DKVertexStream::TypeUByte4)}
					};
					buffer = DKVertexBuffer::Create(decls, 2, (SkinVertex12*)skinData, sizeof(SkinVertex12), skinData.Count(),
						DKVertexBuffer::MemoryLocationStatic, DKVertexBuffer::BufferUsageDraw);
				}
				if (buffer)
					mesh->AddVertexBuffer(buffer);
			}

			// 버퍼를 하나로 합친다
			mesh->MakeInterleaved();
			ret.Add(mesh.SafeCast<DKMesh>());
		}
	});
	return ret;
}

DKObject<DKCollisionObject> ModelExporter::ExportCollisionObject(IGameMesh* pIGameMesh, const GMatrix& tm) const
{
	int numVerts = pIGameMesh->GetNumberOfVerts();
	int numFaces = pIGameMesh->GetNumberOfFaces();

	if (numVerts > 0 && numFaces > 0)
	{
		DKArray<DKVector3> vertices;
		DKArray<long> indices;

		vertices.Reserve(numVerts);
		indices.Reserve(numFaces * 3);

		for (int i = 0; i < numFaces; ++i)
		{
			FaceEx* face = pIGameMesh->GetFace(i);
			if (face->flags & FACE_HIDDEN)
				continue;

			indices.Add(face->vert[0]);
			indices.Add(face->vert[1]);
			indices.Add(face->vert[2]);
		}
		for (int i = 0; i < numVerts; ++i)
		{
			Point3 pos = pIGameMesh->GetVertex(i, false) * tm;
			vertices.Add(DKVector3(pos.x, pos.y, pos. z));
		}

		DKConvexHullShape::ConvexHullArray hullArray = DKConvexHullShape::DecomposeTriangleMesh(vertices, vertices.Count(), indices, indices.Count());
		if (hullArray.Count() > 0)
		{
			DKObject<DKCompoundShape> compund = DKObject<DKCompoundShape>::New();
			for (DKConvexHullShape::ConvexHull& ch : hullArray)
			{
				compund->AddShape( ch.shape, ch.offset );
			}
			return DKOBJECT_NEW DKRigidBody( compund, 0.0 );
		}
	}
	return NULL;
}

void ModelExporter::ExportTextures(IGameMaterial* pIGameMaterial, DKMesh::TextureSamplerMap& samplers) const
{
	if (pIGameMaterial != NULL)
	{
		int numTextures = pIGameMaterial->GetNumberOfTextureMaps();
		for (int i = 0; i < numTextures; ++i)
		{
			DKString fileName = L"";
			IGameTextureMap* pIGameTextureMap = pIGameMaterial->GetIGameTextureMap(i);
			if (pIGameTextureMap->IsEntitySupported())		// texture file
			{
				const MCHAR* filePath = pIGameTextureMap->GetBitmapFileName();
				if (filePath != NULL)
				{
					for (int i = _tcslen(filePath) - 1; i > 0; --i)
					{
						if (filePath[i] == _M('\\'))
						{
#ifdef MCHAR_IS_WCHAR
							fileName = (const MCHAR*)&filePath[i + 1];
#else
							char* f = (char*)&filePath[i + 1];
							DWORD len = ::MultiByteToWideChar(CP_ACP, 0, f, -1, NULL, 0);
							wchar_t* wf = (wchar_t*)malloc(sizeof(wchar_t)* len);
							::MultiByteToWideChar(CP_ACP, 0, f, -1, wf, len);
							fileName = wf;
							free(wf);
#endif
							break;
						}
					}
				}
				if (fileName.Length() > 0)
				{
					DKObject<DKTexture> tex = DKObject<DKTexture>::New();
					tex->SetName(fileName);
					samplers.Value(L"diffuseMap").textures.Add(tex);
				}
			}
		}
		int numMaterials = pIGameMaterial->GetSubMaterialCount();
		for (int i = 0; i < numMaterials; ++i)
		{
			ExportTextures(pIGameMaterial->GetSubMaterial(i), samplers);
		}
	}
}

void ModelExporter::ExportMaterialProperties(IGameMaterial* pIGameMaterial, DKMesh::PropertyMap& properties) const
{
	IGameProperty* p = NULL;
	// ambient
	p = pIGameMaterial->GetAmbientData();
	if (p && p->GetType() == IGAME_POINT3_PROP)
	{
		Point3 value;
		p->GetPropertyValue(value);
		properties.Update(L"ambient", DKMaterial::PropertyArray((float*)&value, 3));
	}
	// diffuse
	p = pIGameMaterial->GetDiffuseData();
	if (p && p->GetType() == IGAME_POINT3_PROP)
	{
		Point3 value;
		p->GetPropertyValue(value);
		properties.Update(L"diffuse", DKMaterial::PropertyArray((float*)&value, 3));
	}
	// specular
	p = pIGameMaterial->GetSpecularData();
	if (p && p->GetType() == IGAME_POINT3_PROP)
	{
		Point3 value;
		p->GetPropertyValue(value);
		properties.Update(L"specular", DKMaterial::PropertyArray((float*)&value, 3));
	}
	// emissive
	p = pIGameMaterial->GetEmissiveData();
	if (p && p->GetType() == IGAME_POINT3_PROP)
	{
		Point3 value;
		p->GetPropertyValue(value);
		properties.Update(L"emissive", DKMaterial::PropertyArray((float*)&value, 3));
	}
	// opacity
	p = pIGameMaterial->GetOpacityData();
	if (p && p->GetType() == IGAME_FLOAT_PROP)
	{
		float value;
		p->GetPropertyValue(value);
		properties.Update(L"opacity", DKMaterial::PropertyArray(value, 1));
	}
	// glossiness
	p = pIGameMaterial->GetGlossinessData();
	if (p && p->GetType() == IGAME_FLOAT_PROP)
	{
		float value;
		p->GetPropertyValue(value);
		properties.Update(L"glossiness", DKMaterial::PropertyArray(value, 1));
	}
	// emissiveAmount
	p = pIGameMaterial->GetEmissiveAmtData();
	if (p && p->GetType() == IGAME_FLOAT_PROP)
	{
		float value;
		p->GetPropertyValue(value);
		properties.Update(L"emissiveAmount", DKMaterial::PropertyArray(value, 1));
	}
	// specularLevel
	p = pIGameMaterial->GetSpecularLevelData();
	if (p && p->GetType() == IGAME_FLOAT_PROP)
	{
		float value;
		p->GetPropertyValue(value);
		if (value == 0)
			value = 128.0f;
		properties.Update(L"specularLevel", DKMaterial::PropertyArray(value, 1));
	}
}

void ModelExporter::RemoveEmptyModelTree(DKModel* model) const
{
}

void ModelExporter::CreateFixedConstraints(DKModel* model1, DKModel* model2, DKNSTransform trans) const
{
}

void ModelExporter::CreateP2PConstraints(DKModel* model1, DKModel* model2, DKNSTransform trans) const
{
}
