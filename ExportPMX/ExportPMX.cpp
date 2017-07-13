// ExportPMX.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#define NOMINMAX
#include <windows.h>
#include "MQPlugin.h"
#include "MQWidget.h"
#include "MQ3DLib.h"
#include "MQExportObject.h"
#include "MQBasePlugin.h"
#include "MQSetting.h"
#include "MQBoneManager.h"
#include "EncodingHelper.h"
//#include "Edition.h"
#include <vector>
#include <map>
#include <list>
#include <algorithm>
#include <assert.h>
#include <MFileUtil.h>
#include <tinyxml2.h>

HINSTANCE s_hInstance;
wchar_t s_DllPath[MAX_PATH];

#define LOG(str)	OutputDebugString(std::wstring(std::wstring(str) + L"\n").c_str())
#define EPS	0.00001

static bool MQPointFuzzyEqual(MQPoint A, MQPoint B)
{
	return ((fabs(A.x - B.x) < EPS) && (fabs(A.y - B.y) < EPS) && (fabs(A.z - B.z) < EPS));
}

static MAnsiString getMultiBytesSubstring(const MAnsiString& str, size_t maxlen)
{
	size_t p = 0;
	while (true)
	{
		size_t np = str.next(p);
		if (np >= maxlen || np == MAnsiString::kInvalid)
		{
			if (p > 0)
			{
				return str.substring(0, p);
			}
			break;
		}
		p = np;
	}
	return MAnsiString();
}

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved
)
{
	// The instance will be used for a dialog box
	s_hInstance = static_cast<HINSTANCE>(hModule);

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		::GetModuleFileName(static_cast<HMODULE>(hModule), s_DllPath, MAX_PATH);
		break;
	default: ;
	}
	return TRUE;
}

class ExportPMXPlugin : public MQExportPlugin
{
public:
	ExportPMXPlugin();
	~ExportPMXPlugin();

	// Get a plug-in ID
	// プラグインIDを返す。
	void GetPlugInID(DWORD* Product, DWORD* ID) override;

	// Get a plug-in name
	// プラグイン名を返す。
	const char* GetPlugInName() override;

	// Get a file type for importing
	// 入力出力可能なファイルタイプを返す。
	const char* EnumFileType(int index) override;

	// Get a file extension for importing
	// 入力可能な拡張子を返す。
	const char* EnumFileExt(int index) override;

	// Export a file
	// ファイルの読み込み
	BOOL ExportFile(int index, const char* filename, MQDocument doc) override;

private:
	struct BoneNameSetting
	{
		MString jp;
		MString en;
		MString group;
	};

	struct BoneIKNameSetting
	{
		MString bone;
		MString ik;
		MString ikend;
	};

	struct BoneGroupSetting
	{
		MString jp;
		MString en;
	};

	BoneNameSetting m_RootBoneName;
	std::vector<BoneNameSetting> m_BoneNameSetting;
	std::vector<BoneIKNameSetting> m_BoneIKNameSetting;
	std::vector<BoneGroupSetting> m_BoneGroupSetting;
	bool LoadBoneSettingFile();
};

ExportPMXPlugin::ExportPMXPlugin()
{
}

ExportPMXPlugin::~ExportPMXPlugin()
{
}

void ExportPMXPlugin::GetPlugInID(DWORD* Product, DWORD* ID)
{
	*Product = 0x00000001;
	*ID = 0x00000002;
}

const char* ExportPMXPlugin::GetPlugInName()
{
	//return "Export PMX       Copyright(C) 2014-2015, tetraface Inc.";
	return "Export PMX";
}

const char* ExportPMXPlugin::EnumFileType(int index)
{
	if (index == 0)
	{
		return "MikuMikuDance format (*.pmx)";
	}
	return nullptr;
}

const char* ExportPMXPlugin::EnumFileExt(int index)
{
	if (index == 0)
	{
		return "pmx";
	}
	return nullptr;
}

class PMXOptionDialog : public MQDialog
{
public:
	MQCheckBox* check_visible;
	MQComboBox* combo_bone;
	MQComboBox* combo_ikend;
	MQComboBox* combo_facial;
	MQEdit* edit_modelname;
	MQMemo* memo_comment;

	PMXOptionDialog(int id, int parent_frame_id, ExportPMXPlugin* plugin);
	BOOL ComboBoneChanged(MQWidgetBase* sender, MQDocument doc) const;
};

PMXOptionDialog::PMXOptionDialog(int id, int parent_frame_id, ExportPMXPlugin* plugin) : MQDialog(id)
{
	MQFrame parent(parent_frame_id);

	MQGroupBox* group = CreateGroupBox(&parent, L"PMX选项");

	check_visible = CreateCheckBox(group, L"仅可见对象");

	MQFrame* hframe = CreateHorizontalFrame(group);
	CreateLabel(hframe, L"导出骨骼");
	combo_bone = CreateComboBox(hframe);
	combo_bone->AddItem(L"否");
	combo_bone->AddItem(L"是");
	combo_bone->SetHintSizeRateX(8);
	combo_bone->SetFillBeforeRate(1);

	hframe = CreateHorizontalFrame(group);
	CreateLabel(hframe, L"生成IK");
	combo_ikend = CreateComboBox(hframe);
	combo_ikend->AddItem(L"否");
	combo_ikend->AddItem(L"是");
	combo_ikend->SetHintSizeRateX(8);
	combo_ikend->SetFillBeforeRate(1);

	// モーフの出力可否のGUI作成
	hframe = CreateHorizontalFrame(group);
	CreateLabel(hframe, L"生成表情动作");
	combo_facial = CreateComboBox(hframe);
	combo_facial->AddItem(L"否");
	combo_facial->AddItem(L"是");
	combo_facial->SetHintSizeRateX(8);
	combo_facial->SetFillBeforeRate(1);

	hframe = CreateHorizontalFrame(group);
	CreateLabel(hframe, L"模型名");
	edit_modelname = CreateEdit(hframe);
	edit_modelname->SetMaxAnsiLength(20);
	edit_modelname->SetHorzLayout(LAYOUT_FILL);

	CreateLabel(group, L"注释");
	memo_comment = CreateMemo(group);
	//memo_comment->SetMaxLength(256);
}

BOOL PMXOptionDialog::ComboBoneChanged(MQWidgetBase* sender, MQDocument doc) const
{
	combo_ikend->SetEnabled(combo_bone->GetCurrentIndex() == 1);
	return FALSE;
}

struct CreateDialogOptionParam
{
	ExportPMXPlugin* plugin;
	PMXOptionDialog* dialog;

	bool visible_only;
	bool bone_exists;
	bool facial_exists;
	bool output_bone;
	bool output_ik_end;
	bool output_facial;
	MAnsiString modelname;
	MAnsiString comment;
};

static void CreateDialogOption(bool init, MQFileDialogCallbackParam* param, void* ptr)
{
	CreateDialogOptionParam* option = static_cast<CreateDialogOptionParam*>(ptr);

	if (init)
	{
		PMXOptionDialog* dialog = new PMXOptionDialog(param->dialog_id, param->parent_frame_id, option->plugin);
		option->dialog = dialog;

		dialog->check_visible->SetChecked(option->visible_only);
		dialog->combo_bone->SetEnabled(option->bone_exists);
		dialog->combo_bone->SetCurrentIndex(option->output_bone ? 1 : 0);
		dialog->combo_ikend->SetEnabled(option->bone_exists && option->output_bone);
		dialog->combo_ikend->SetCurrentIndex(option->output_ik_end ? 1 : 0);
		dialog->combo_facial->SetEnabled(option->facial_exists);
		dialog->combo_facial->SetCurrentIndex(option->output_facial ? 1 : 0);
		dialog->edit_modelname->SetText(MString::fromAnsiString(option->modelname).c_str());
	}
	else
	{
		option->visible_only = option->dialog->check_visible->GetChecked();
		option->output_bone = option->dialog->combo_bone->GetCurrentIndex() == 1;
		option->output_ik_end = option->dialog->combo_ikend->GetCurrentIndex() == 1;
		option->output_facial = option->dialog->combo_facial->GetCurrentIndex() == 1;
		option->modelname = getMultiBytesSubstring(MString(option->dialog->edit_modelname->GetText()).toAnsiString(), 20);
		option->comment = getMultiBytesSubstring(MString(option->dialog->memo_comment->GetText()).toAnsiString(), 256);
		delete option->dialog;
	}
}

#pragma pack(push,1)
struct PMXFileHeader
{
	unsigned char magic[3];
	float version;
	unsigned char model_name[20];
	unsigned char comment[256];
};

struct PMXFileVertexList
{
	float pos[3];
	float normal_vec[3];
	float uv[2];
	unsigned short bone_num[2];
	unsigned char bone_weight;
	unsigned char edge_flag;
};
#pragma pack(pop)

struct PMXBoneParam
{
	UINT id;
	MQMatrix mtx;
	MQMatrix base_mtx;
	UINT parent;
	int child_num;
	MQPoint org_root, org_tip;
	MQPoint def_root, def_tip;
	//MQPoint scale;
	MString name;
	int ikchain;
	UINT ikparent;
	bool ikparent_isik;
	bool dummy;
	bool twist;
	bool end_point;
	std::vector<UINT> children;

	int PMX_root_index;
	int PMX_tip_index;
	int PMX_ik_index;
	int PMX_ik_end_index;
	int PMX_ik_parent_root;
	int PMX_ik_parent_tip;

	std::vector<int> PMX_ik_chain;
	bool PMX_ik_root_tip;

	MString name_jp;
	MString name_en;
	int root_group;
	int group;
	int ik_group;

	UINT link_id;
	int link_rate;

	bool movable;
	UINT group_id;
	UINT tip_id;
	MString tip_name;
	MString ik_name;
	MString ik_tip_name;

	MString tip_name_jp;
	MString tip_name_en;
	MString ik_name_jp;
	MString ik_tip_name_jp;
	MString ik_name_en;
	MString ik_tip_name_en;

	PMXBoneParam(): id(0)
	{
		parent = 0;
		child_num = 0;
		ikchain = 0;
		ikparent = 0;
		ikparent_isik = false;
		dummy = false;
		twist = false;
		end_point = false;
		PMX_root_index = -1;
		PMX_tip_index = -1;
		PMX_ik_index = -1;
		PMX_ik_end_index = -1;
		PMX_ik_parent_root = -1;
		PMX_ik_parent_tip = -1;
		PMX_ik_root_tip = true;
		root_group = -1;
		group = -1;
		ik_group = -1;

		link_id = 0;
		link_rate = 0;

		movable = false;
		group_id = 0;
		tip_id = 0;
	}
};

//Facial
enum MorphType
{
	MORPH_BASE = 0,
	MORPH_BROW,
	MORPH_EYE,
	MORPH_LIP,
	MORPH_OTHER,
};

struct PMXMorphInputParam
{
	MQObject base;
	std::vector<std::pair<MQObject, MorphType>> target;
};

struct PMXMorphParam
{
	char skin_name[20]; //　表情名
	DWORD vertNum; // 表情用の頂点数
	MorphType type;

	std::vector<std::pair<DWORD, MQPoint>> vertex;
};

static bool containsTargetObject(std::vector<PMXMorphInputParam>& list, MQObject obj)
{
	assert(obj != nullptr);

	auto end = list.end();
	for (auto ite = list.begin(); ite != end; ++ite)
	{
		for (auto tIte = ite->target.begin(); tIte != ite->target.end(); ++tIte)
			if (tIte->first == obj)
				return true;
	}

	return false;
}

BOOL ExportPMXPlugin::ExportFile(int index, const char* filename, MQDocument doc)
{
	LoadBoneSettingFile();
	MQBoneManager bone_manager(this, doc);

	// Query a number of bones
	int bone_num = bone_manager.GetBoneNum();
	// Enum bones
	std::vector<UINT> bone_id;
	std::vector<PMXBoneParam> bone_param;
	if (bone_num > 0)
	{
		bone_id.resize(bone_num);
		bone_manager.EnumBoneID(bone_id);

		bone_param.resize(bone_num);
		for (int i = 0; i < bone_num; i++)
		{
			bone_param[i].id = bone_id[i];

			std::wstring name;
			bone_manager.GetParent(bone_id[i], bone_param[i].parent);
			bone_manager.GetChildNum(bone_id[i], bone_param[i].child_num);
			bone_manager.GetBaseRootPos(bone_id[i], bone_param[i].org_root);
			bone_manager.GetBaseTipPos(bone_id[i], bone_param[i].org_tip);
			bone_manager.GetDeformRootPos(bone_id[i], bone_param[i].def_root);
			bone_manager.GetDeformTipPos(bone_id[i], bone_param[i].def_tip);
			bone_manager.GetDeformMatrix(bone_id[i], bone_param[i].mtx);
			bone_manager.GetBaseMatrix(bone_id[i], bone_param[i].base_mtx);
			//bone_manager.GetDeformScale(bone_id[i], bone_param[i].scale);
			bone_manager.GetName(bone_id[i], name);
			bone_manager.GetIKChain(bone_id[i], bone_param[i].ikchain);
			bone_manager.GetDummy(bone_id[i], bone_param[i].dummy);
			bone_manager.GetEndPoint(bone_id[i], bone_param[i].end_point);

			bone_param[i].name = MString(name);
			{
				MQAngle angle_min, angle_max;
				bone_manager.GetAngleMin(bone_id[i], angle_min);
				bone_manager.GetAngleMax(bone_id[i], angle_max);
				bone_param[i].twist = (angle_max.bank == 0 && angle_min.bank == 0 && angle_max.pitch == 0 && angle_min.pitch == 0);
			}
			{
				MQBoneManager::LINK_PARAM param;
				bone_manager.GetLink(bone_id[i], param);
				bone_param[i].link_id = param.link_bone_id;
				bone_param[i].link_rate = int(param.rotate);
			}
			{
				bone_manager.GetMovable(bone_id[i], bone_param[i].movable);
			}
			{
				bone_manager.GetTipBone(bone_id[i], bone_param[i].tip_id);
			}
			if (bone_param[i].tip_id == 0)
			{
				bone_manager.GetTipName(bone_id[i], name);
				bone_param[i].tip_name = name;
			}
			if (bone_param[i].ikchain != -1)
			{
				std::wstring tip_name;
				bone_manager.GetIKName(bone_id[i], name, tip_name);
				bone_param[i].ik_name = name;
				bone_param[i].ik_tip_name = tip_name;
			}
			if (bone_param[i].ikchain != -1)
			{
				bone_manager.GetIKParent(bone_id[i], bone_param[i].ikparent, bone_param[i].ikparent_isik);
			}
		}
	}

	const DWORD morph_plugin_product = 0x56A31D20;
	const DWORD morph_plugin_id = 0xC452C6DB;

	// モーフプラグインから必要な情報を取得
	int morph_target_size = 0;
	std::vector<PMXMorphInputParam> morph_intput_list;
	std::vector<std::vector<MQObject>> morph_target_list;
	std::vector<std::vector<int>> morph_target_index_list;

	int morph_num = this->SendUserMessage(doc, morph_plugin_product, morph_plugin_id, "getMorphObjectSize", nullptr);
	morph_intput_list.resize(morph_num);
	morph_target_list.resize(morph_num);
	morph_target_index_list.resize(morph_num);

	{
		// モーフのベースオブジェクトを取得
		std::vector<MQObject> baseObj;
		baseObj.resize(morph_num);
#ifdef _DEBUG
		assert(this->SendUserMessage(doc, morph_plugin_product, morph_plugin_id, "getBaseObjectList", baseObj.data()) == morph_num);
#else
		this->SendUserMessage(doc, morph_plugin_product, morph_plugin_id, "getBaseObjectList", baseObj.data());
#endif

		for (int i = 0; i < morph_num; ++i)
			morph_intput_list.at(i).base = baseObj.at(i);
	}

	if (morph_num > 0)
	{
		// モーフターゲット情報を取得
		std::vector<std::pair<MQObject, MorphType>> target;
		auto targets = &morph_target_list.front();
		auto targetIndexes = &morph_target_index_list.front();
		int targetIndex = 1;
		for (int i = 0; i < morph_num; ++i)
		{
			targets = &morph_target_list.at(i);
			targetIndexes = &morph_target_index_list.at(i);

			PMXMorphInputParam* iParam = &morph_intput_list.at(i);
			int	target_size = this->SendUserMessage(doc, morph_plugin_product, morph_plugin_id, "getTargetSize", iParam->base);

			target.resize(target_size + 1);
			target.at(0) = std::make_pair(iParam->base, MORPH_BASE);
#ifdef _DEBUG
			assert(this->SendUserMessage(doc, morph_plugin_product, morph_plugin_id, "getTargetList", target.data()) == target_size);
#else
			this->SendUserMessage(doc, morph_plugin_product, morph_plugin_id, "getTargetList", target.data());
#endif

			auto end = target.end();
			for (auto ite = target.begin() + 1; ite != end; ++ite)
			{
				iParam->target.push_back(*ite);
				targets->push_back(ite->first);
				targetIndexes->push_back(targetIndex++);
			}

			morph_target_size += target_size;
		}
	}

	// Show a dialog for converting axes
	// 座標軸変換用ダイアログの表示
	float scaling = 1;
	CreateDialogOptionParam option;
	option.plugin = this;
	option.visible_only = false;
	option.bone_exists = (bone_num > 0);
	option.facial_exists = (morph_num > 0);
	option.output_bone = true;
	option.output_ik_end = true;
	option.output_facial = true;
	option.modelname = getMultiBytesSubstring(MFileUtil::extractFileNameOnly(MString::fromAnsiString(filename)).toAnsiString(), 20);
	option.comment = MAnsiString();
	// Load a setting.
	MQSetting* setting = OpenSetting();
	if (setting != nullptr)
	{//设置页面显示的
		setting->Load("VisibleOnly", option.visible_only, option.visible_only);
		setting->Load("Bone", option.output_bone, option.output_bone);
		setting->Load("IKEnd", option.output_ik_end, option.output_ik_end);
		setting->Load("Facial", option.output_facial, option.output_facial);
	}
	MQFileDialogInfo dlginfo;
	memset(&dlginfo, 0, sizeof(dlginfo));
	dlginfo.dwSize = sizeof(dlginfo);
	dlginfo.scale = scaling;
	dlginfo.hidden_flag = MQFileDialogInfo::HIDDEN_AXIS | MQFileDialogInfo::HIDDEN_INVERT_FACE;
	dlginfo.axis_x = MQFILE_TYPE_RIGHT;
	dlginfo.axis_y = MQFILE_TYPE_UP;
	dlginfo.axis_z = MQFILE_TYPE_FRONT;
	dlginfo.softname = "";
	dlginfo.dialog_callback = CreateDialogOption;
	dlginfo.dialog_callback_ptr = &option;
	MQ_ShowFileDialog("PMX Export", &dlginfo);

	// Save a setting.
	scaling = dlginfo.scale;
	if (setting != nullptr)
	{
		setting->Save("VisibleOnly", option.visible_only);
		setting->Save("Bone", option.output_bone);
		setting->Save("IKEnd", option.output_ik_end);
		setting->Save("Facial", option.output_facial);
		CloseSetting(setting);
	}

	if (!option.output_bone)
	{
		bone_num = 0;
		bone_id.clear();
		bone_param.clear();
	}

	bool isOutputFacial = option.output_facial;
	if (!isOutputFacial)
	{
		morph_num = 0;
		morph_target_size = 0;
		morph_intput_list.clear();
	}

	int numObj = doc->GetObjectCount();
	int numMat = doc->GetMaterialCount();

	// 頂点をひとまとめにする（単一オブジェクトしか扱えないので）
	std::vector<MQExportObject*> expobjs(numObj, nullptr);
	std::vector<std::vector<int>> orgvert_vert(numObj);
	std::vector<int> vert_orgobj;
	std::vector<int> vert_expvert;
	std::vector<MQPoint> vert_normal;
	std::vector<MQCoordinate> vert_coord;
	int total_vert_num = 0;

	for (int oi = 0; oi < numObj; oi++)
	{
		MQObject org_obj = doc->GetObject(oi);
		if (org_obj == nullptr)
			continue;

		if (option.visible_only && org_obj->GetVisible() == 0)
			continue;

		MQExportObject::MSeparateParam separate;
		separate.SeparateNormal = true;
		separate.SeparateUV = true;
		separate.SeparateVertexColor = false;
		MQExportObject* eobj = new MQExportObject(org_obj, separate);
		expobjs[oi] = eobj;

		// ターゲットオブジェクトは飛ばす
		if (isOutputFacial && containsTargetObject(morph_intput_list, org_obj))
			continue;

		int vert_num = eobj->GetVertexCount();
		orgvert_vert[oi].resize(vert_num, -1);
		for (int evi = 0; evi < vert_num; evi++)
		{
			orgvert_vert[oi][evi] = total_vert_num;

			vert_orgobj.push_back(oi);
			vert_expvert.push_back(evi);

			MQPoint nrm = eobj->GetVertexNormal(evi);
			MQCoordinate uv = eobj->GetVertexCoordinate(evi);

			vert_normal.push_back(nrm);
			vert_coord.push_back(uv);
			total_vert_num++;
		}
	}
	std::map<UINT, int> bone_id_index;
	// Initialize bones.
	if (bone_num > 0)
	{
		for (int i = 0; i < bone_num; i++)
		{
			bone_id_index[bone_param[i].id] = i;
		}

		// Check the parent
		for (int i = 0; i < bone_num; i++)
		{
			if (bone_param[i].parent != 0)
			{
				if (bone_id_index.end() == bone_id_index.find(bone_param[i].parent))
				{
					assert(0);
					bone_param[i].parent = 0;
				}
			}
		}

		// Sort by hierarchy
		{
			std::list<PMXBoneParam> bone_param_temp(bone_param.begin(), bone_param.end());
			bone_param.clear();
			bone_id_index.clear();
			while (!bone_param_temp.empty())
			{
				bool done = false;
				for (auto it = bone_param_temp.begin(); it != bone_param_temp.end();)
				{
					if ((*it).parent != 0)
					{
						if (bone_id_index.end() != bone_id_index.find((*it).parent))
						{
							if (bone_param[bone_id_index[(*it).parent]].tip_id == 0 || bone_id_index[(*it).parent] != bone_param.size() - 1)
							{
								bone_id_index[(*it).id] = int(bone_param.size());
								bone_param.push_back(*it);
								it = bone_param_temp.erase(it);
								done = true;
							}
							else if (bone_param[bone_id_index[(*it).parent]].tip_id == (*it).id)
							{
								bone_id_index[(*it).id] = int(bone_param.size());
								bone_param.push_back(*it);
								it = bone_param_temp.erase(it);
								done = true;
							}
							else
							{
								++it; // try next
							}
						}
						else
						{
							++it; // try next
						}
					}
					else
					{
						bone_id_index[(*it).id] = int(bone_param.size());
						bone_param.push_back(*it);
						it = bone_param_temp.erase(it);
						done = true;
					}
				}

				assert(done);
				if (!done)
				{
					for (auto it = bone_param_temp.begin(); it != bone_param_temp.end(); ++it)
					{
						bone_id_index[(*it).id] = int(bone_param.size());
						(*it).parent = 0;
						bone_param.push_back(*it);
					}
					break;
				}
			}
		}

		// Enum children
		for (int i = 0; i < bone_num; i++)
		{
			if (bone_param[i].parent != 0)
			{
				if (bone_id_index.end() != bone_id_index.find(bone_param[i].parent))
				{
					bone_param[bone_id_index[bone_param[i].parent]].children.push_back(i);
				}
				else
				{
					assert(0);
					bone_param[i].parent = 0;
				}
			}
			if (bone_param[i].twist)
			{
				bone_param[i].twist = false;
				bone_param[bone_id_index[bone_param[i].parent]].twist = true;
			}
		}
	}
	int PMXbone_num = 0;
	std::vector<int> ik_chain_end_list;

	std::vector<std::pair<int, MQPoint>> root_bones;
	for (int i = 0; i < bone_num; i++)
	{
		// Determine PMX bone index.
		bone_param[i].PMX_root_index = -1;
		if (bone_param[i].parent == 0)
		{
			for (auto it = root_bones.begin(); it != root_bones.end(); ++it)
			{
				if (bone_param[i].org_root == (*it).second)
				{
					bone_param[i].PMX_root_index = (*it).first;
					break;
				}
			}
			if (bone_param[i].PMX_root_index == -1)
			{
				bone_param[i].PMX_root_index = PMXbone_num++;
				root_bones.push_back(std::pair<int, MQPoint>(bone_param[i].PMX_root_index, bone_param[i].org_root));
			}
		}
		if (bone_param[i].children.empty() && bone_param[i].end_point)
			bone_param[i].PMX_tip_index = -1;
		else
			bone_param[i].PMX_tip_index = PMXbone_num++;
	}
	for (int i = 0; i < bone_num; i++)
	{
		if (bone_param[i].tip_id == 0)continue;
		/*UINT tip_parent_id = bone_param[bone_id_index[bone_param[i].tip_id]].parent;
		assert(bone_param[i].id == tip_parent_id);*/
		//bone_param[i].PMX_tip_index = bone_param[bone_id_index[tip_parent_id]].PMX_tip_index;
	}
	// Construct IK chain
	for (int i = bone_num - 1; i >= 0; i--)
	{ // from end to root
		if (bone_param[i].ikchain >= 0)
		{
			int chain_num = bone_param[i].ikchain;
			int idx = i;
			for (int p = 0; p <= chain_num; p++)
			{
				if (bone_param[idx].parent == 0)
				{
					bone_param[i].PMX_ik_chain.push_back(idx);
					bone_param[i].PMX_ik_root_tip = false;
					break;
				}
				if (bone_id_index.end() == bone_id_index.find(bone_param[idx].parent)) break;

				idx = bone_id_index[bone_param[idx].parent];
				if (p == chain_num)
				{
					bone_param[i].PMX_ik_chain.push_back(idx);
					bone_param[i].PMX_ik_root_tip = true;
				}
				else
				{
					// IKチェイン内に別のIKチェインを含めない
					if (bone_param[idx].ikchain > 0)
					{
						bone_param[idx].ikchain = 0;
					}
					bone_param[i].PMX_ik_chain.push_back(idx);
				}
			}
		}
	}
	for (int i = 0; i < bone_num; i++)
	{
		if (bone_param[i].PMX_ik_chain.size() > 0)
		{
			bone_param[i].PMX_ik_index = PMXbone_num++;
			if (option.output_ik_end)
			{
				bone_param[i].PMX_ik_end_index = PMXbone_num++;
			}
			ik_chain_end_list.push_back(i);
		}
	}
	for (int i = 0; i < bone_num; i++)
	{
		if (bone_param[i].PMX_ik_index >= 0)
		{
			for (size_t j = 0; j < bone_param[i].PMX_ik_chain.size(); j++)
			{
				if (j + 1 == bone_param[i].PMX_ik_chain.size() && !bone_param[i].PMX_ik_root_tip)
				{
					bone_param[bone_param[i].PMX_ik_chain[j]].PMX_ik_parent_root = bone_param[i].PMX_ik_index;
				}
				else
				{
					if (bone_param[bone_param[i].PMX_ik_chain[j]].PMX_ik_parent_tip == -1)
					{
						bone_param[bone_param[i].PMX_ik_chain[j]].PMX_ik_parent_tip = bone_param[i].PMX_ik_index;
					}
				}
			}
		}
	}
	for (int i = 0; i < bone_num; i++)
	{
		bool name_found = false;
		for (auto it = m_BoneNameSetting.begin(); it != m_BoneNameSetting.end(); ++it)
		{
			if ((*it).jp == bone_param[i].name || (*it).en == bone_param[i].name)
			{
				bone_param[i].name_jp = (*it).jp;
				bone_param[i].name_en = (*it).en;
				name_found = true;
				break;
			}
		}
		if (!name_found)
		{
			bone_param[i].name_jp = bone_param[i].name;
			bone_param[i].name_en = bone_param[i].name;
		}
		name_found = false;
		for (auto it = m_BoneNameSetting.begin(); it != m_BoneNameSetting.end(); ++it)
		{
			if ((*it).jp == bone_param[i].tip_name || (*it).en == bone_param[i].tip_name)
			{
				bone_param[i].tip_name_jp = (*it).jp;
				bone_param[i].tip_name_en = (*it).en;
				name_found = true;
				break;
			}
		}
		if (!name_found)
		{
			bone_param[i].tip_name_jp = bone_param[i].tip_name;
			bone_param[i].tip_name_en = bone_param[i].tip_name;
		}
		name_found = false;
		for (auto it = m_BoneNameSetting.begin(); it != m_BoneNameSetting.end(); ++it)
		{
			if ((*it).jp == bone_param[i].ik_name || (*it).en == bone_param[i].ik_name)
			{
				bone_param[i].ik_name_jp = (*it).jp;
				bone_param[i].ik_name_en = (*it).en;
				name_found = true;
				break;
			}
		}
		if (!name_found)
		{
			bone_param[i].ik_name_jp = bone_param[i].ik_name;
			bone_param[i].ik_name_en = bone_param[i].ik_name;
		}
		name_found = false;
		for (auto it = m_BoneNameSetting.begin(); it != m_BoneNameSetting.end(); ++it)
		{
			if ((*it).jp == bone_param[i].ik_tip_name || (*it).en == bone_param[i].ik_tip_name)
			{
				bone_param[i].ik_tip_name_jp = (*it).jp;
				bone_param[i].ik_tip_name_en = (*it).en;
				name_found = true;
				break;
			}
		}
		if (!name_found)
		{
			bone_param[i].ik_tip_name_jp = bone_param[i].ik_tip_name;
			bone_param[i].ik_tip_name_en = bone_param[i].ik_tip_name;
		}
	}
	// モーフ用情報収集
	std::vector<PMXMorphParam> morph_param_list;
	if (morph_target_size != 0)
		morph_param_list.resize(morph_target_size + 1);

	// ベースオブジェクト情報
	PMXMorphParam* morph_base_param = nullptr;
	{
		if (!morph_intput_list.empty())
		{
			morph_base_param = &morph_param_list.front();
			strcpy_s(morph_base_param->skin_name, "base");
			morph_base_param->type = MORPH_BASE;
			morph_base_param->vertNum = 0;
		}
	}

	// ターゲットオブジェクト情報
	if (isOutputFacial && morph_num > 0)
	{
		size_t totalIdx = 1;
		auto bBegin = morph_intput_list.begin();
		auto bEnd = morph_intput_list.end();
		auto tBegin = morph_intput_list.front().target.begin();
		auto tEnd = morph_intput_list.front().target.end();
		for (auto bIte = bBegin; bIte != bEnd; ++bIte)
		{
			tBegin = bIte->target.begin();
			tEnd = bIte->target.end();

			for (auto tIte = tBegin; tIte != tEnd; ++tIte)
			{
				PMXMorphParam* mParam = &morph_param_list.at(totalIdx++);
				tIte->first->GetName(mParam->skin_name, 20);
				mParam->type = tIte->second;
				mParam->vertNum = 0;
			}
		}
	}

	// モーフの頂点情報
	std::vector<DWORD> morph_base_vertex_index_list;
	if (isOutputFacial && morph_num > 0)
	{
		auto end = morph_intput_list.end();
		auto targetList = morph_target_list.front();
		auto targetIndexList = morph_target_index_list.front();
		for (auto ite = morph_intput_list.begin(); ite != end; ++ite)
		{
			bool isWriteBase = false;

			size_t paramIdx = distance(morph_intput_list.begin(), ite);

			MQObject base = ite->base;
			size_t baseIdx = doc->GetObjectIndex(base);
			if (expobjs[baseIdx] == nullptr)
				continue;
			int baseVertSize = expobjs[baseIdx]->GetVertexCount();

			for (int i = 0; i < baseVertSize; ++i)
			{
				int baseExpIdx = orgvert_vert.at(baseIdx).at(i);
				int baseOrgIdx = expobjs[baseIdx]->GetOriginalVertex(i);
				MQPoint basePos = base->GetVertex(baseOrgIdx);

				auto tList = &ite->target;
				auto tEnd = tList->end();
				for (auto tIte = tList->begin(); tIte != tEnd; ++tIte)
				{
					MQObject target = tIte->first;
					MQPoint targetPos = target->GetVertex(baseOrgIdx);
					if (!MQPointFuzzyEqual(basePos, targetPos))
					{
						isWriteBase = true;

						targetList = morph_target_list.at(paramIdx);
						targetIndexList = morph_target_index_list.at(paramIdx);
						PMXMorphParam* param = &morph_param_list.at(targetIndexList.at(distance(targetList.begin(), find(targetList.begin(), targetList.end(), target))));
						param->vertex.push_back(std::make_pair(baseExpIdx, targetPos - basePos));
						++param->vertNum;
					}
				}

				if (isWriteBase)
				{
					morph_base_param->vertex.push_back(std::make_pair(baseExpIdx, basePos));
					morph_base_vertex_index_list.push_back(baseExpIdx);
					++morph_base_param->vertNum;
				}
			}
		}
	}

	// Open a file.
	FILE* fh;
	errno_t err = fopen_s(&fh, filename, "wb");
	//errno_t err = fopen_s(&fh, filename, "w");
	if (err != 0)
	{
		for (size_t i = 0; i < expobjs.size(); i++)
		{
			delete expobjs[i];
		}
		return FALSE;
	}

	// Header
	float version = 2.0f;
	char magic[4] = {0x50 ,0x4d ,0x58 ,0x20};
	fwrite(magic, 1, 4, fh);
	//fprintf(fh,"PMX\n");
	fwrite(reinterpret_cast<char*>(&version), sizeof(float), 1, fh);
	byte Header[9] = {8,0,0,4,1,1,1,1,1};
	fwrite(&Header, sizeof(byte), 9, fh);
	//fprintf(fh,"%f\n",version);

	oguna::EncodingConverter converter = oguna::EncodingConverter();
	std::wstring RES;
	int Len = converter.Cp936ToUtf16(option.modelname, option.modelname.length(), &RES) * 2;
	fwrite(&Len, sizeof(int), 1, fh);
	fwrite(RES.c_str(), Len, 1, fh);
	Len = converter.Cp936ToUtf16(option.modelname, option.comment.length(), &RES) * 2;
	fwrite(&Len, sizeof(int), 1, fh);
	fwrite(RES.c_str(), Len, 1, fh);
	Len = 0;
	fwrite(&Len, sizeof(int), 1, fh);
	fwrite(&Len, sizeof(int), 1, fh);

	int dw_vert_num = total_vert_num;
	fwrite(&dw_vert_num, 4, 1, fh);
	for (int i = 0; i < total_vert_num; i++)
	{
		float pos[3];
		float nrm[3];
		float uv[2];
		byte bone_index[4]; // ボーン番号1、番号2 // モデル変形(頂点移動)時に影響
		float bone_weight; // ボーン1に与える影響度 // min:0 max:100 // ボーン2への影響度は、(100 - bone_weight)
		float edge_flag; // 0:通常、1:エッジ無効 // エッジ(輪郭)が有効の場合

		MQObject obj = doc->GetObject(vert_orgobj[i]);
		MQExportObject* eobj = expobjs[vert_orgobj[i]];
		MQPoint v = obj->GetVertex(eobj->GetOriginalVertex(vert_expvert[i]));
		pos[0] = v.x * scaling;
		pos[1] = v.y * scaling;
		pos[2] = -v.z * scaling;
		fwrite(pos, 4, 3, fh);

		nrm[0] = vert_normal[i].x;
		nrm[1] = vert_normal[i].y;
		nrm[2] = -vert_normal[i].z;
		fwrite(nrm, 4, 3, fh);

		uv[0] = vert_coord[i].u;
		uv[1] = vert_coord[i].v;
		fwrite(uv, 4, 2, fh);

		UINT vert_bone_id[16];
		float weights[16];
		int weight_num = 0;

		if (bone_num > 0)
		{
			UINT vert_id = obj->GetVertexUniqueID(eobj->GetOriginalVertex(vert_expvert[i]));
			int max_num = 16;
			weight_num = bone_manager.GetVertexWeightArray(obj, vert_id, max_num, vert_bone_id, weights);
		}
		if (weight_num == 4)
		{
			int max_bone1 = bone_id_index[vert_bone_id[0]];
			int max_bone2 = bone_id_index[vert_bone_id[1]];
			int max_bone3 = bone_id_index[vert_bone_id[2]];
			int max_bone4 = bone_id_index[vert_bone_id[3]];
			if (bone_param[max_bone1].parent != 0)
			{
				bone_index[0] = bone_param[bone_id_index[bone_param[max_bone1].parent]].PMX_tip_index;
			}
			else
			{
				bone_index[0] = bone_param[max_bone1].PMX_root_index;
			}
			if (bone_param[max_bone2].parent != 0)
			{
				bone_index[1] = bone_param[bone_id_index[bone_param[max_bone2].parent]].PMX_tip_index;
			}
			else
			{
				bone_index[1] = bone_param[max_bone2].PMX_root_index;
			}
			if (bone_param[max_bone3].parent != 0)
			{
				bone_index[2] = bone_param[bone_id_index[bone_param[max_bone3].parent]].PMX_tip_index;
			}
			else
			{
				bone_index[2] = bone_param[max_bone3].PMX_root_index;
			}
			if (bone_param[max_bone4].parent != 0)
			{
				bone_index[3] = bone_param[bone_id_index[bone_param[max_bone4].parent]].PMX_tip_index;
			}
			else
			{
				bone_index[3] = bone_param[max_bone4].PMX_root_index;
			}
			int type = 2;
			fwrite(&type, sizeof(byte), 1, fh);
			fwrite(bone_index, 1, 4, fh);
			float total_weights = weights[0] + weights[1] + weights[2] + weights[3];
			float bone_weight1 = floor(weights[0] / total_weights * 100.f + 0.5f) / 100;
			fwrite(&bone_weight1, sizeof(float), 1, fh);
			float bone_weight2 = floor(weights[1] / total_weights * 100.f + 0.5f) / 100;
			fwrite(&bone_weight2, sizeof(float), 1, fh);
			float bone_weight3 = floor(weights[2] / total_weights * 100.f + 0.5f) / 100;
			fwrite(&bone_weight3, sizeof(float), 1, fh);
			float bone_weight4 = 1 - bone_weight1 - bone_weight2 - bone_weight3;
			fwrite(&bone_weight4, sizeof(float), 1, fh);
		}
		else if (weight_num == 3)
		{
			int max_bone1 = bone_id_index[vert_bone_id[0]];
			int max_bone2 = bone_id_index[vert_bone_id[1]];
			int max_bone3 = bone_id_index[vert_bone_id[2]];
			if (bone_param[max_bone1].parent != 0)
			{
				bone_index[0] = bone_param[bone_id_index[bone_param[max_bone1].parent]].PMX_tip_index;
			}
			else
			{
				bone_index[0] = bone_param[max_bone1].PMX_root_index;
			}
			if (bone_param[max_bone2].parent != 0)
			{
				bone_index[1] = bone_param[bone_id_index[bone_param[max_bone2].parent]].PMX_tip_index;
			}
			else
			{
				bone_index[1] = bone_param[max_bone2].PMX_root_index;
			}
			if (bone_param[max_bone3].parent != 0)
			{
				bone_index[2] = bone_param[bone_id_index[bone_param[max_bone3].parent]].PMX_tip_index;
			}
			else
			{
				bone_index[2] = bone_param[max_bone3].PMX_root_index;
			}
			int type = 2;
			fwrite(&type, sizeof(byte), 1, fh);
			fwrite(bone_index, 1, 4, fh);
			float total_weights = weights[0] + weights[1] + weights[2];
			float bone_weight1 = floor(weights[0] / total_weights * 100.f + 0.5f) / 100;
			fwrite(&bone_weight1, sizeof(float), 1, fh);
			float bone_weight2 = floor(weights[1] / total_weights * 100.f + 0.5f) / 100;
			fwrite(&bone_weight2, sizeof(float), 1, fh);
			float bone_weight3 = 1 - bone_weight1 - bone_weight2;
			fwrite(&bone_weight3, sizeof(float), 1, fh);
			float bone_weight4 = 0;
			fwrite(&bone_weight4, sizeof(float), 1, fh);
		}
		else if (weight_num == 2)
		{
			int max_bone1 = -1;
			float max_weight1 = 0.0f;
			for (int n = 0; n < weight_num; n++)
			{
				if (max_weight1 < weights[n])
				{
					max_weight1 = weights[n];
					max_bone1 = n;
				}
			}
			int max_bone2 = -1;
			float max_weight2 = 0.0f;
			for (int n = 0; n < weight_num; n++)
			{
				if (n == max_bone1) continue;
				if (max_weight2 < weights[n])
				{
					max_weight2 = weights[n];
					max_bone2 = n;
				}
			}
			float total_weights = max_weight1 + max_weight2;
			int bi1 = bone_id_index[vert_bone_id[max_bone1]];
			int bi2 = bone_id_index[vert_bone_id[max_bone2]];
			if (bone_param[bi1].parent != 0)
			{
				bone_index[0] = bone_param[bone_id_index[bone_param[bi1].parent]].PMX_tip_index;
			}
			else
			{
				bone_index[0] = bone_param[bi1].PMX_root_index;
			}
			if (bone_param[bi2].parent != 0)
			{
				bone_index[1] = bone_param[bone_id_index[bone_param[bi2].parent]].PMX_tip_index;
			}
			else
			{
				bone_index[1] = bone_param[bi2].PMX_root_index;
			}
			if (bone_index[0] != bone_index[1])
			{
				bone_weight = floor(max_weight1 / total_weights * 100.f + 0.5f) / 100;
			}
			else
			{
				bone_weight = 1;
			}
			int type = 1;
			fwrite(&type, sizeof(byte), 1, fh);
			fwrite(bone_index, 1, 2, fh);
			fwrite(&bone_weight, sizeof(float), 1, fh);
		}
		else if (weight_num == 1)
		{
			int bi = bone_id_index[vert_bone_id[0]];
			if (bone_param[bi].parent != 0)
			{
				bone_index[0] = bone_param[bone_id_index[bone_param[bi].parent]].PMX_tip_index;
			}
			else
			{
				bone_index[0] = bone_param[bi].PMX_root_index;
			}
			bone_index[1] = bone_index[0];
			bone_weight = 1;
			int type = 1;
			fwrite(&type, sizeof(byte), 1, fh);
			fwrite(bone_index, 1, 2, fh);
			fwrite(&bone_weight, sizeof(float), 1, fh);
		}
		else
		{
			bone_index[0] = 0;
			bone_index[1] = 0;
			bone_weight = 1;
			int type = 1;
			fwrite(&type, sizeof(byte), 1, fh);
			fwrite(bone_index, 1, 2, fh);
			fwrite(&bone_weight, sizeof(float), 1, fh);
		}
		edge_flag = 1;
		fwrite(&edge_flag, sizeof(float), 1, fh);
	}

	DWORD face_vert_count = 0;
	std::vector<int> material_used(numMat + 1, 0);
	for (int i = 0; i < numObj; i++)
	{
		MQObject obj = doc->GetObject(i);
		if (obj == nullptr)
			continue;

		if (option.visible_only && obj->GetVisible() == 0)
			continue;

		// ターゲットオブジェクトは飛ばす
		if (isOutputFacial && containsTargetObject(morph_intput_list, obj))
			continue;

		int num_face = obj->GetFaceCount();
		for (int fi = 0; fi < num_face; fi++)
		{
			int n = obj->GetFacePointCount(fi);
			if (n >= 3)
			{
				face_vert_count += (n - 2) * 3;

				int mi = obj->GetFaceMaterial(fi);
				if (mi < 0 || mi >= numMat) mi = numMat;
				material_used[mi] += (n - 2);
			}
		}
	}
	fwrite(&face_vert_count, 4, 1, fh);

	int output_face_vert_count = 0;
	for (int m = 0; m <= numMat; m++)
	{
		if (material_used[m] == 0) continue;

		for (int i = 0; i < numObj; i++)
		{
			MQObject obj = doc->GetObject(i);
			if (obj == nullptr)
				continue;

			if (option.visible_only && obj->GetVisible() == 0)
				continue;
			if (isOutputFacial && containsTargetObject(morph_intput_list, obj))
				continue;

			MQExportObject* eobj = expobjs[i];
			if (eobj == nullptr)
				continue;

			int num_face = obj->GetFaceCount();
			for (int fi = 0; fi < num_face; fi++)
			{
				int mi = obj->GetFaceMaterial(fi);
				if (mi < 0 || mi >= numMat) mi = numMat;

				int n = eobj->GetFacePointCount(fi);
				if (n >= 3 && mi == m)
				{
					std::vector<int> vi(n);
					std::vector<MQPoint> p(n);

					eobj->GetFacePointArray(fi, vi.data());
					for (int j = 0; j < n; j++)
					{
						p[j] = obj->GetVertex(eobj->GetOriginalVertex(vi[j]));
					}
					std::vector<int> tri((n - 2) * 3);
					doc->Triangulate(p.data(), n, tri.data(), (n - 2) * 3);

					for (int j = 0; j < n - 2; j++)
					{
						int tvi[3];
						tvi[0] = orgvert_vert[i][vi[tri[j * 3]]];
						tvi[1] = orgvert_vert[i][vi[tri[j * 3 + 1]]];
						tvi[2] = orgvert_vert[i][vi[tri[j * 3 + 2]]];
						fwrite(tvi, 4, 3, fh);
						//fprintf(fh,"%u %u %u\n",tvi[0],tvi[1],tvi[2]);
						output_face_vert_count += 3;
					}
				}
			}
		}
	}
	assert(face_vert_count == output_face_vert_count);

	// Matrial list
	DWORD used_mat_num = 0;
	for (int i = 0; i <= numMat; i++)
	{
		if (material_used[i] > 0) used_mat_num++;
	}
	std::unique_ptr<MAnsiString[]> textures = std::make_unique<MAnsiString[]>(numMat);
	int TexCount = 0;
	for (int i = 0; i < numMat; i++)
	{
		if (material_used[i] == 0) continue;
		if (i < numMat)
		{
			MQMaterial mat = doc->GetMaterial(i);
			if (mat != nullptr)
			{
				char path[_MAX_PATH];
				mat->GetTextureName(path, _MAX_PATH);
				MAnsiString texture = MFileUtil::extractFilenameAndExtension(MString::fromAnsiString(path)).toAnsiString();
				if (texture.length() == 0)continue;
				for (int j = 0; j < numMat; j++)
				{
					if (textures[j] == texture)
					{
						break;
					}
					if (textures[j].length() == 0)
					{
						textures[j] = texture;
						TexCount += 1;
						break;
					}
				}
			}
		}
	}

	fwrite(&TexCount, sizeof(int), 1, fh);
	for (int i = 0; i < TexCount; i++)
	{
		Len = converter.Cp936ToUtf16(textures[i].c_str(), textures[i].length(), &RES) * 2;
		fwrite(&Len, sizeof(int), 1, fh);
		fwrite(RES.c_str(), Len, 1, fh);
	}
	fwrite(&used_mat_num, 4, 1, fh);
	for (int i = 0; i < numMat; i++)
	{
		if (material_used[i] == 0) continue;
		MQMaterial mat = doc->GetMaterial(i);
		if (mat == nullptr)
		{
			continue;
		}
		Len = converter.Cp936ToUtf16(mat->GetName().c_str(), mat->GetName().length(), &RES) * 2;
		fwrite(&Len, sizeof(int), 1, fh);
		fwrite(RES.c_str(), Len, 1, fh);
		Len = 0;
		fwrite(&Len, sizeof(int), 1, fh);
		MQColor col(1, 1, 1);
		float dif = 0.8f;
		float alpha = 1.0f;
		float spc_pow = 5.0f;
		MQColor spc_col(0, 0, 0);
		MQColor amb_col(0.6f, 0.6f, 0.6f);
		MAnsiString texture;
		bool edge = false;

		if (i < numMat)
		{
			if (mat != nullptr)
			{
				col = mat->GetColor();
				dif = mat->GetDiffuse();
				alpha = mat->GetAlpha();
				spc_pow = mat->GetPower();
				spc_col = mat->GetSpecularColor();
				amb_col = mat->GetAmbientColor();
				char path[_MAX_PATH];
				mat->GetTextureName(path, _MAX_PATH);
				texture = MFileUtil::extractFilenameAndExtension(MString::fromAnsiString(path)).toAnsiString();
				int shader = mat->GetShader();
				if (shader == MQMATERIAL_SHADER_HLSL)
				{
					MAnsiString shader_name = mat->GetShaderName();
					if (shader_name == "PMX")
					{
						//int	toon = mat->GetShaderParameterIntValue("Toon", 0);
						edge = mat->GetShaderParameterBoolValue("Edge", 0);
					}
				}
			}
		}

		float diffuse_color[3]; // dr, dg, db // 減衰色
		diffuse_color[0] = col.r * dif;
		diffuse_color[1] = col.g * dif;
		diffuse_color[2] = col.b * dif;
		fwrite(diffuse_color, 4, 3, fh);
		//fprintf(fh,"%f %f %f\n",diffuse_color[0],diffuse_color[1],diffuse_color[2]);
		fwrite(&alpha, 4, 1, fh);
		//fprintf(fh,"%f\n",alpha);

		//float max_spc_col = std::max(spc_col.r, std::max(spc_col.g, spc_col.b));
		float specular_color[3]; // sr, sg, sb // 光沢色
		//specular_color[0] = (max_spc_col > 0) ? spc_col.r / max_spc_col : 0.0f;
		//specular_color[1] = (max_spc_col > 0) ? spc_col.g / max_spc_col : 0.0f;
		//specular_color[2] = (max_spc_col > 0) ? spc_col.b / max_spc_col : 0.0f;
		specular_color[0] = sqrtf(spc_col.r);
		specular_color[1] = sqrtf(spc_col.g);
		specular_color[2] = sqrtf(spc_col.b);
		fwrite(&specular_color, 4, 3, fh);
		//fprintf(fh,"%f %f %f\n",specular_color[0],specular_color[1],specular_color[2]);

		fwrite(&spc_pow, 4, 1, fh);
		//fprintf(fh,"%f\n",spc_pow);

		float ambient_color[3]; // mr, mg, mb // 環境色(ambient)
		ambient_color[0] = amb_col.r;
		ambient_color[1] = amb_col.g;
		ambient_color[2] = amb_col.b;
		fwrite(&ambient_color, 4, 3, fh);
		//fprintf(fh,"%f %f %f\n",ambient_color[0],ambient_color[1],ambient_color[2]);

		BYTE edge_flag = edge ? 1 : 0;
		fwrite(&edge_flag, 1, 1, fh);
		//fprintf(fh,"%d\n",edge_flag);

		float edge_color[5] = {0,0,0,0,0};
		fwrite(&edge_color, sizeof(float), 5, fh);
		uint8_t size = 255;
		if (texture != "")
		{
			for (int x = 0; x < TexCount; x++)
			{
				if (texture == textures[x])
				{
					size = x;
					break;
				}
			}
		}
		fwrite(&size, sizeof(uint8_t), 1, fh);//Tex
		size = 255;
		fwrite(&size, sizeof(uint8_t), 1, fh);//Spa

		edge_flag = 0;
		fwrite(&edge_flag, 1, 1, fh);//SpaMod

		edge_flag = 0;
		fwrite(&edge_flag, 1, 1, fh);//ToonSelect
		//BYTE toon_index = (toon >= 1 && toon <= 10) ? static_cast<BYTE>(toon - 1) : 0;
		BYTE toon_index = 255;
		fwrite(&toon_index, 1, 1, fh);//Toon
		int Memo = 0;
		fwrite(&Memo, sizeof(int), 1, fh);//Memo
		face_vert_count = material_used[i] * 3;
		fwrite(&face_vert_count, 4, 1, fh);//Face
	}

	if (bone_num == 0 || !option.output_bone)
	{
		Len = 1;
		fwrite(&Len, sizeof(int), 1, fh);
		MAnsiString subname = "センター";
		Len = converter.Cp936ToUtf16(subname.c_str(), subname.length(), &RES) * 2;
		fwrite(&Len, sizeof(int), 1, fh);
		fwrite(RES.c_str(), Len, 1, fh);
		subname = "center";
		Len = converter.Cp936ToUtf16(subname.c_str(), subname.length(), &RES) * 2;
		fwrite(&Len, sizeof(int), 1, fh);
		fwrite(RES.c_str(), Len, 1, fh);

		float bone_head_pos[3];
		bone_head_pos[0] = 0;
		bone_head_pos[1] = 0;
		bone_head_pos[2] = 0;
		fwrite(&bone_head_pos, 4, 3, fh);
		auto parent_bone_index = -1;
		fwrite(&parent_bone_index, 1, 1, fh);
		int level = 0;
		fwrite(&level, sizeof(int), 1, fh);
		uint16_t BoneFlag = 27;
		fwrite(&BoneFlag, sizeof(uint16_t), 1, fh);
		uint8_t target_index = 255;
		fwrite(&target_index, sizeof(uint8_t), 1, fh);
	}
	else
	{
		if (option.output_ik_end)
		{
			fwrite(&PMXbone_num, sizeof(int), 1, fh);
		}
		else
		{
			Len = bone_num + 1;
			fwrite(&Len, sizeof(int), 1, fh);
		}

		if (PMXbone_num != 0)
		{
			int PMXbone_index = 0;
			for (int i = 0; i < bone_num; i++)
			{
				if (bone_param[i].PMX_root_index >= 0 && bone_param[i].PMX_root_index >= PMXbone_index)//判断是否是初始骨骼
				{
					assert(bone_param[i].PMX_root_index == PMXbone_index);
					MAnsiString subname = getMultiBytesSubstring(bone_param[i].name_jp.toAnsiString(), 20);
					Len = converter.Cp936ToUtf16(subname.c_str(), subname.length(), &RES) * 2;
					fwrite(&Len, sizeof(int), 1, fh);
					fwrite(RES.c_str(), Len, 1, fh);
					Len = converter.Cp936ToUtf16(bone_param[i].name_en.toAnsiString(), bone_param[i].name_en.length(), &RES) * 2;
					fwrite(&Len, sizeof(int), 1, fh);
					fwrite(RES.c_str(), Len, 1, fh);

					float bone_head_pos[3];
					bone_head_pos[0] = bone_param[i].org_root.x * scaling;
					bone_head_pos[1] = bone_param[i].org_root.y * scaling;
					bone_head_pos[2] = -bone_param[i].org_root.z * scaling;
					fwrite(&bone_head_pos, 4, 3, fh);

					auto parent_bone_index = -1;
					fwrite(&parent_bone_index, 1, 1, fh);

					int level = 0;
					fwrite(&level, sizeof(int), 1, fh); //变形阶层
					uint16_t BoneFlag = 19;//默认指向骨骼并且带旋转加操作
					if (bone_param[i].movable == true)//是否移动
					{
						BoneFlag += 4;
					}
					if (bone_param[i].dummy == true)//是否显示
					{
						BoneFlag += 8;
					}
					if (bone_param[i].link_id != 0)//是否旋转+
					{
						BoneFlag += 256;
					}
					fwrite(&BoneFlag, sizeof(uint16_t), 1, fh);

					if (BoneFlag & 0x0001) //假如指向骨骼，则骨骼序列为
					{
						uint8_t target_index = 0;
						target_index = bone_param[i].PMX_tip_index;
						if (bone_param[i].link_id != 0 && bone_param[i].link_rate != 100)
						{
							target_index = bone_id_index[bone_param[i].link_id];
						}
						fwrite(&target_index, sizeof(uint8_t), 1, fh);
					}
					PMXbone_index++;
				}
				if (bone_param[i].PMX_tip_index >= 0 && bone_param[i].PMX_tip_index >= PMXbone_index)
				{
					assert(bone_param[i].PMX_tip_index == PMXbone_index);
					MAnsiString subname;
					MAnsiString subnameEN;
					if (bone_param[i].tip_id == 0)
					{
						subname = getMultiBytesSubstring(bone_param[i].tip_name_jp.toAnsiString(), 20);
						subnameEN = getMultiBytesSubstring(bone_param[i].tip_name_en.toAnsiString(), 20);
					}
					else
					{
						subname = getMultiBytesSubstring(bone_param[bone_id_index[bone_param[i].tip_id]].name_jp.toAnsiString(), 20);
						subnameEN = getMultiBytesSubstring(bone_param[bone_id_index[bone_param[i].tip_id]].name_en.toAnsiString(), 20);
					}
					Len = converter.Cp936ToUtf16(subname.c_str(), subname.length(), &RES) * 2;
					fwrite(&Len, sizeof(int), 1, fh);
					fwrite(RES.c_str(), Len, 1, fh);
					Len = converter.Cp936ToUtf16(subnameEN.c_str(), subnameEN.length(), &RES) * 2;
					fwrite(&Len, sizeof(int), 1, fh);
					fwrite(RES.c_str(), Len, 1, fh);

					float bone_head_pos[3];
					bone_head_pos[0] = bone_param[i].org_tip.x * scaling;
					bone_head_pos[1] = bone_param[i].org_tip.y * scaling;
					bone_head_pos[2] = -bone_param[i].org_tip.z * scaling;
					fwrite(&bone_head_pos, 4, 3, fh);

					uint8_t parent_bone_index = 255;
					if (bone_param[i].parent != 0)
					{
						auto parent_it = bone_id_index.find(bone_param[i].parent);
						if (parent_it != bone_id_index.end())
						{
							parent_bone_index = bone_param[(*parent_it).second].PMX_tip_index;
						}
					}
					else
					{
						parent_bone_index = bone_param[i].PMX_root_index;
					}
					fwrite(&parent_bone_index, sizeof(uint8_t), 1, fh);

					int level = 0;
					fwrite(&level, sizeof(int), 1, fh); //变形阶层
					uint16_t BoneFlag = 19;//默认指向骨骼并且带旋转加操作
					if (!bone_param[i].children.empty())//是否显示
					{
						BoneFlag += 8;
					}
					if (!bone_param[i].children.empty() && bone_param[bone_param[i].children.front()].link_id != 0)//是否旋转+
					{
						BoneFlag += 256;
					}
					fwrite(&BoneFlag, sizeof(uint16_t), 1, fh);

					/*BYTE bone_type = 0; // ボーンの種類 0:回転 1:回転と移動 2:IK 3:不明 4:IK影響下 5:回転影響下 6:IK接続先 7:非表示
					if (bone_param[i].tip_id == 0 && bone_param[i].child_num != 0)
					{
						bone_type = 7;
					}
					else if (bone_param[i].PMX_ik_parent_tip >= 0)
					{
						bone_type = 4;
					}
					else if (bone_param[i].PMX_ik_index >= 0)
					{
						bone_type = 6;
					}
					else if (!bone_param[i].children.empty() && bone_param[bone_param[i].children.front()].link_id != 0)
					{
						if (bone_param[bone_param[i].children.front()].link_rate == 100)
						{
							bone_type = 5;
						}
						else
						{
							bone_type = 9;
						}
					}
					else if (bone_param[i].twist)
					{
						bone_type = 8;
					}
					else if (bone_param[i].child_num == 0)
					{
						bone_type = 7;
					}
					if (!bone_param[i].children.empty() && bone_type <= 1)
					{
						bone_type = bone_param[bone_param[i].children.front()].movable;
					}
					fwrite(&bone_type, 1, 1, fh);*/

					if (BoneFlag & 0x0001) //假如指向骨骼，则骨骼序列为
					{
						uint8_t target_index = -1;
						if (!bone_param[i].children.empty())
						{
							target_index = bone_param[bone_param[i].children.front()].PMX_tip_index;
							if (bone_param[bone_param[i].children.front()].link_id != 0 && bone_param[bone_param[i].children.front()].link_rate != 100)
							{
								target_index = bone_id_index[bone_param[bone_param[i].children.front()].link_id];
							}
						}
						fwrite(&target_index, sizeof(uint8_t), 1, fh);
					}
					if (BoneFlag & (0x0100 | 0x0200))
					{
						uint8_t grant_parent_index = bone_id_index[bone_param[bone_param[i].children.front()].link_id];
						fwrite(&grant_parent_index, sizeof(uint8_t), 1, fh);
						float grant_weight = 1;
						fwrite(&grant_weight, sizeof(float), 1, fh);
					}
					PMXbone_index++;
				}
			}
			if (option.output_ik_end)
			{
				for (int i = 0; i < bone_num; i++)
				{
					if (bone_param[i].PMX_ik_chain.empty()) continue;

					MString name = bone_param[i].ik_name_jp;
					MString ik_end_name = bone_param[i].ik_tip_name_jp;

					if (name.length() == 0 || ik_end_name.length() == 0)
					{
						for (auto ikt = m_BoneIKNameSetting.begin(); ikt != m_BoneIKNameSetting.end(); ++ikt)
						{
							if ((*ikt).bone == bone_param[i].name || (*ikt).bone == bone_param[i].name_en)
							{
								MString n = (*ikt).ik;
								MString en = (*ikt).ikend;
								for (auto it = m_BoneNameSetting.begin(); it != m_BoneNameSetting.end(); ++it)
								{
									if ((*it).en == name)
									{
										n = (*it).jp;
										break;
									}
								}
								for (auto it = m_BoneNameSetting.begin(); it != m_BoneNameSetting.end(); ++it)
								{
									if ((*it).en == ik_end_name)
									{
										en = (*it).jp;
										break;
									}
								}
								if (name.length() == 0)
								{
									name = n;
								}
								if (ik_end_name.length() == 0)
								{
									ik_end_name = en;
								}
								break;
							}
						}
					}
					if (name.length() == 0)
					{
						name = MString(L"IK-") + bone_param[i].name;
					}
					if (ik_end_name.length() == 0)
					{
						ik_end_name = name + MString(L" end");
					}

					MAnsiString subname = getMultiBytesSubstring(name.toAnsiString(), 20);

					Len = converter.Cp936ToUtf16(subname.c_str(), subname.length(), &RES) * 2;
					fwrite(&Len, sizeof(int), 1, fh);
					fwrite(RES.c_str(), Len, 1, fh);
					Len = 0;
					fwrite(&Len, sizeof(int), 1, fh);

					float bone_head_pos[3];
					bone_head_pos[0] = bone_param[i].org_tip.x * scaling;
					bone_head_pos[1] = bone_param[i].org_tip.y * scaling;
					bone_head_pos[2] = -bone_param[i].org_tip.z * scaling;
					fwrite(&bone_head_pos, 4, 3, fh);

					uint8_t parent_bone_index = 255;
					if (bone_param[i].ikparent != 0)
					{
						if (!bone_param[i].ikparent_isik)
						{
							parent_bone_index = bone_id_index[bone_param[i].ikparent];
						}
						else
						{
							parent_bone_index = bone_param[bone_id_index[bone_param[i].ikparent]].PMX_ik_index;
						}
					}
					fwrite(&parent_bone_index, sizeof(uint8_t), 1, fh);
					int level = 0;
					fwrite(&level, sizeof(int), 1, fh); //变形阶层

					uint16_t BoneFlag = 63;//
					fwrite(&BoneFlag, sizeof(uint16_t), 1, fh);

					if (BoneFlag & 0x0001) //假如指向骨骼，则骨骼序列为
					{
						uint8_t target_index = -1;
						if (bone_param[i].PMX_ik_end_index >= 0)
						{
							target_index = bone_param[i].PMX_ik_end_index;
						}
						else if (bone_param[i].PMX_ik_parent_tip >= 0)
						{
							target_index = bone_param[i].PMX_ik_parent_tip;
						}
						fwrite(&target_index, sizeof(uint8_t), 1, fh);
					}
					if (BoneFlag & 0x0020)
					{
						uint8_t ik_target_bone_index = bone_param[i].PMX_tip_index; // IKターゲットボーン番号 // IKボーンが最初に接続するボーン
						fwrite(&ik_target_bone_index, sizeof(uint8_t), 1, fh);
						int ik_loop = 10; // 再帰演算回数 // IK値1
						fwrite(&ik_loop, sizeof(int), 1, fh);
						float ik_loop_angle_limit = 0.5f;
						fwrite(&ik_loop_angle_limit, sizeof(float), 1, fh);
						int ik_link_count = bone_param[i].PMX_ik_chain.size();
						fwrite(&ik_link_count, sizeof(int), 1, fh);
						for (size_t j = 0; j < ik_link_count; j++)
						{
							uint8_t link_target = -1;
							if (j + 1 == ik_link_count && !bone_param[i].PMX_ik_root_tip)
							{
								link_target = bone_param[bone_param[i].PMX_ik_chain[j]].PMX_root_index;
							}
							else
							{
								link_target = bone_param[bone_param[i].PMX_ik_chain[j]].PMX_tip_index;
							}
							fwrite(&link_target, sizeof(uint8_t), 1, fh);
							uint8_t angle_lock = 0;
							fwrite(&angle_lock, sizeof(uint8_t), 1, fh);
							if (angle_lock == 1)
							{
								float max_radian[3];
								max_radian[0] = 0;
								max_radian[1] = 0;
								max_radian[2] = 0;
								fwrite(&max_radian, 4, 3, fh);
								float min_radian[3];
								min_radian[0] = 0;
								min_radian[1] = 0;
								min_radian[2] = 0;
								fwrite(&min_radian, 4, 3, fh);
							}
						}
					}
					assert(PMXbone_index == bone_param[i].PMX_ik_index);
					PMXbone_index++;

					if (option.output_ik_end)
					{
						subname = getMultiBytesSubstring(ik_end_name.toAnsiString(), 20);
						Len = converter.Cp936ToUtf16(subname.c_str(), subname.length(), &RES) * 2;
						fwrite(&Len, sizeof(int), 1, fh);
						fwrite(RES.c_str(), Len, 1, fh);
						Len = 0;
						fwrite(&Len, sizeof(int), 1, fh);
						MQPoint parent_dir = bone_param[i].org_root - bone_param[i].org_tip;
						parent_dir.normalize();
						MQPoint vec1(0, -1, 0), vec2(0, 0, -1);
						MQPoint ik_end_dir;
						if (fabs(GetInnerProduct(parent_dir, vec1)) < fabs(GetInnerProduct(parent_dir, vec2)))
						{
							ik_end_dir = vec1;
						}
						else
						{
							ik_end_dir = vec2;
						}

						if (!bone_param[i].children.empty())
						{
							MQPoint child_dir = bone_param[bone_param[i].children.front()].org_tip - bone_param[i].org_tip;
							child_dir.normalize();
							if (GetInnerProduct(child_dir, ik_end_dir) > 0)
							{
								ik_end_dir = -ik_end_dir;
							}
						}

						MQPoint ik_end_pos = bone_param[i].org_tip + ik_end_dir;
						bone_head_pos[3];
						bone_head_pos[0] = ik_end_pos.x * scaling;
						bone_head_pos[1] = ik_end_pos.y * scaling;
						bone_head_pos[2] = -ik_end_pos.z * scaling;
						fwrite(&bone_head_pos, 4, 3, fh);

						parent_bone_index = bone_param[i].PMX_ik_index;
						fwrite(&parent_bone_index, sizeof(uint8_t), 1, fh);
						Len = 0;
						fwrite(&Len, sizeof(int), 1, fh); //变形阶层

						BoneFlag = 19;
						fwrite(&BoneFlag, sizeof(uint16_t), 1, fh);

						uint8_t target_index = -1;
						fwrite(&target_index, sizeof(uint8_t), 1, fh);

						assert(PMXbone_index == bone_param[i].PMX_ik_end_index);

						PMXbone_index++;
					}
				}
			}
		}
	}

	int skin_count = static_cast<int>(morph_param_list.size());
	if (skin_count != 0)
	{
		int Save_skin_count = skin_count - 1;
		fwrite(&Save_skin_count, sizeof(int), 1, fh);
		float skin_vert_pos[3];
		int skin_vert_index = 0;
		int MAXCOUNT = 0;
		int MAXTEMP = 0;
		PMXMorphParam MaxParam;
		for (int i = 0; i < skin_count; i++)
		{
			int TEMP = morph_param_list.at(i).vertNum;
			if (TEMP > MAXCOUNT)
			{
				MAXCOUNT = TEMP;
				MaxParam = morph_param_list.at(i);
			}
		}
		std::vector<int> MaxList;
		for (size_t i = 0; i < MAXCOUNT; i++)
		{
			MaxList.push_back(MaxParam.vertex.at(i).first);
		}

		for (int i = 0; i < skin_count; i++)
		{
			if (i == MAXTEMP)
			{
				continue;
			}
			PMXMorphParam* mParam = &morph_param_list.at(i);

			auto subname = getMultiBytesSubstring(mParam->skin_name, 20);
			Len = converter.Cp936ToUtf16(subname.c_str(), subname.length(), &RES) * 2;
			fwrite(&Len, sizeof(int), 1, fh);
			fwrite(RES.c_str(), Len, 1, fh);
			Len = 0;
			fwrite(&Len, sizeof(int), 1, fh);
			fwrite(&mParam->type, sizeof(uint8_t), 1, fh);//Panel
			uint8_t morph_type = 1;
			fwrite(&morph_type, sizeof(uint8_t), 1, fh);//Kind=Vertex
			fwrite(&mParam->vertNum, sizeof(int), 1, fh);//num
			DWORD skin_vert_count = mParam->vertNum;
			for (DWORD j = 0; j < skin_vert_count; ++j)
			{
				auto vertex = &mParam->vertex.at(j);
				if (i == 0)
				{
					skin_vert_index = vertex->first;
				}
				else
				{
					skin_vert_index = static_cast<int>(distance(morph_base_vertex_index_list.begin(), find(morph_base_vertex_index_list.begin(), morph_base_vertex_index_list.end(), vertex->first)));
				}
				skin_vert_index = MaxList[skin_vert_index];
				fwrite(&skin_vert_index, 4, 1, fh);
				skin_vert_pos[0] = vertex->second.x * scaling;
				skin_vert_pos[1] = vertex->second.y * scaling;
				skin_vert_pos[2] = -vertex->second.z * scaling;
				fwrite(skin_vert_pos, 4, 3, fh);
			}
		}
	}
	else
	{
		fwrite(&skin_count, sizeof(int), 1, fh);
	}

	// 表情枠用表示リスト
	int skin_disp_count = 2;
	fwrite(&skin_disp_count, sizeof(int), 1, fh);
	{
		for (int i = 0; i < skin_disp_count; i++)
		{
			MAnsiString subname;
			MAnsiString subnameEN;
			if (i == 0)
			{
				subname = "Root";
				subnameEN = "Root";
			}
			else
			{
				subname = "表情";
				subnameEN = "Exp";
			}
			Len = converter.Cp936ToUtf16(subname.c_str(), subname.length(), &RES) * 2;
			fwrite(&Len, sizeof(int), 1, fh);
			fwrite(RES.c_str(), Len, 1, fh);
			Len = converter.Cp936ToUtf16(subnameEN.c_str(), subnameEN.length(), &RES) * 2;
			fwrite(&Len, sizeof(int), 1, fh);
			fwrite(RES.c_str(), Len, 1, fh);
			byte SystemNode = 1;
			fwrite(&SystemNode, sizeof(byte), 1, fh);
			int NodeNum = 0;
			fwrite(&NodeNum, sizeof(int), 1, fh);
		}
	}

	int rigid_body_count = 0;
	fwrite(&rigid_body_count, sizeof(int), 1, fh);
	int joint_count = 0;
	fwrite(&joint_count, sizeof(int), 1, fh);

	for (size_t i = 0; i < expobjs.size(); i++)
	{
		delete expobjs[i];
	}

	if (fclose(fh) != 0)
	{
		return FALSE;
	}
	return TRUE;
}

bool ExportPMXPlugin::LoadBoneSettingFile()
{
	//MString dir = MFileUtil::extractDirectory(s_DllPath);
	//MString filename = MFileUtil::combinePath(dir, L"ExportPMXBoneSetting.xml");
	MAnsiString FileName = "ExportPMXBoneSetting";
	tinyxml2::XMLDocument doc;
	//tinyxml2::XMLError err = doc.LoadFile(filename.toAnsiString().c_str());
	tinyxml2::XMLError err = doc.LoadFile(FileName.c_str());
	if (err != tinyxml2::XML_SUCCESS)
	{
		/*MString buf = MString::format(L"Failed to load '%s'. (code %d)\n", filename.c_str(), err);
		OutputDebugStringW(buf.c_str());*/
		return false;
	}
	const tinyxml2::XMLElement* Root = doc.RootElement();
	if (Root != nullptr)
	{
		const tinyxml2::XMLElement* bones = Root->FirstChildElement("bones");
		if (bones != nullptr)
		{
			const tinyxml2::XMLElement* elem = bones->FirstChildElement("bone");
			while (elem != nullptr)
			{
				const char* jp = elem->Attribute("jp");
				const char* en = elem->Attribute("en");
				const char* group = elem->Attribute("group");
				const char* root = elem->Attribute("root");

				BoneNameSetting setting;
				setting.jp = MString::fromUtf8String(jp);
				setting.en = MString::fromUtf8String(en);
				if (group != nullptr)
				{
					setting.group = MString::fromUtf8String(group);
				}
				m_BoneNameSetting.push_back(setting);

				if (root != nullptr && MAnsiString(root).toInt() != 0)
				{
					m_RootBoneName = setting;
				}

				elem = elem->NextSiblingElement("bone");
			}
		}

		const tinyxml2::XMLElement* ik_names = Root->FirstChildElement("ik_names");
		if (ik_names != nullptr)
		{
			const tinyxml2::XMLElement* elem = ik_names->FirstChildElement("ik");
			while (elem != nullptr)
			{
				const char* bone = elem->Attribute("bone");
				const char* ik = elem->Attribute("ik");
				const char* end = elem->Attribute("end");

				BoneIKNameSetting setting;
				setting.bone = MString::fromUtf8String(bone);
				setting.ik = MString::fromUtf8String(ik);
				setting.ikend = MString::fromUtf8String(end);
				m_BoneIKNameSetting.push_back(setting);

				elem = elem->NextSiblingElement("ik");
			}
		}

		const tinyxml2::XMLElement* groups = Root->FirstChildElement("groups");
		if (groups != nullptr)
		{
			const tinyxml2::XMLElement* elem = groups->FirstChildElement("group");
			while (elem != nullptr)
			{
				const char* jp = elem->Attribute("jp");
				const char* en = elem->Attribute("en");

				BoneGroupSetting setting;
				setting.jp = MString::fromUtf8String(jp);
				setting.en = MString::fromUtf8String(en);
				m_BoneGroupSetting.push_back(setting);

				elem = elem->NextSiblingElement("group");
			}
		}
	}

	return true;
}

//---------------------------------------------------------------------------
//  GetPluginClass
//    プラグインのベースクラスを返す
//---------------------------------------------------------------------------
MQBasePlugin* GetPluginClass()
{
	static ExportPMXPlugin plugin;
	return &plugin;
}
