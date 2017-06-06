//---------------------------------------------------------------------------
//
//   MQBoneManager.cpp      Copyright(C) 1999-2016, tetraface Inc.
//
//		An accessor for bone information
//
//    　ボーン情報のアクセス
//
//---------------------------------------------------------------------------

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "MQBoneManager.h"

static const DWORD bone_plugin_product = 0x56A31D20;
static const DWORD bone_plugin_id = 0x71F282AB;


MQBoneManager::MQBoneManager(MQBasePlugin *plugin, MQDocument doc)
	: m_Plugin(plugin), m_Doc(doc)
{
	int version = 0x4506;

	void *array[3];
	array[0] = (void*)"version";
	array[1] = &version;
	array[2] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "QueryAPIVersion", array);
	m_Verified = (ret == 1);
}

DWORD MQBoneManager::GetProductID()
{
	return bone_plugin_product;
}

DWORD MQBoneManager::GetPluginID()
{
	return bone_plugin_id;
}

void MQBoneManager::BeginImport()
{
	if(!m_Verified) return;

	void *array = NULL;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "Init", array);
}

void MQBoneManager::EndImport()
{
	if(!m_Verified) return;

	void *array = NULL;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "NoDelEndDoc", array);
}

UINT MQBoneManager::AddBone(const ADD_BONE_PARAM& param)
{
	if(!m_Verified) return 0;

	const wchar_t *name = param.name.c_str();

	void *array[15];
	array[0] = (void*)"root";
	array[1] = (void*)&param.root_pos;
	array[2] = (void*)"tip";
	array[3] = (void*)&param.tip_pos;
	array[4] = (void*)"name";
	array[5] = (void*)&name;
	array[6] = (void*)"parent";
	array[7] = (void*)&param.parent_id;
	array[8] = (void*)"ikchain";
	array[9] = (void*)&param.ik_chain;
	array[10] = (void*)"dummy";
	array[11] = (void*)&param.is_dummy;
	array[12] = (void*)"end_point";
	array[13] = (void*)&param.end_point;
	array[14] = nullptr;
	UINT bone_id = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "AddBone", array);
	return bone_id;
}

bool MQBoneManager::AddBrother(UINT bone_id, UINT brother_bone_id)
{
	if(!m_Verified) return 0;

	void *array[5];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"brother";
	array[3] = &brother_bone_id;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "AddBrother", array);
	return (ret != 0);
}

int MQBoneManager::GetBoneNum()
{
	if(!m_Verified) return 0;

	int bone_num = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "QueryBoneNum", nullptr);
	return bone_num;
}

int MQBoneManager::EnumBoneID(std::vector<UINT>& bone_id_array)
{
	if(!m_Verified) return 0;

	int num = GetBoneNum();
	bone_id_array.resize(num);
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "EnumBoneID", bone_id_array.data());
	return num;
}

bool MQBoneManager::GetName(UINT bone_id, std::wstring& name)
{
	if(!m_Verified) return false;

	name.clear();
	const wchar_t *p = nullptr;
	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"name";
	array[3] = &p;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	if(!ret)
		return false;
	if(p != nullptr)
		name = std::wstring(p);
	return true;
}

bool MQBoneManager::GetTipName(UINT bone_id, std::wstring& name)
{
	if(!m_Verified) return false;

	name.clear();
	const wchar_t *p = nullptr;
	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"tip_name";
	array[3] = &p;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	if(!ret)
		return false;
	if(p != nullptr)
		name = std::wstring(p);
	return true;
}

bool MQBoneManager::GetParent(UINT bone_id, UINT& parent_id)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"parent";
	array[3] = &parent_id;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetChildNum(UINT bone_id, int& child_num)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"child_num";
	array[3] = &child_num;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetChildren(UINT bone_id, std::vector<UINT>& children)
{
	if(!m_Verified) return false;

	children.clear();

	int child_num = 0;
	if(!GetChildNum(bone_id, child_num))
		return false;

	children.resize(child_num);

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"children";
	array[3] = children.data();
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	if(!ret)
		return false;
	return true;
}

bool MQBoneManager::GetBrothers(UINT bone_id, std::vector<UINT>& brothers)
{
	if(!m_Verified) return false;

	brothers.clear();

	int brother_num = 0;
	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"brother_num";
	array[3] = &brother_num;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	if(!ret)
		return false;

	brothers.resize(brother_num, 0);

	array[2] = (void*)"brothers";
	array[3] = brothers.data();
	ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	if(!ret)
		return false;
	return true;
}

bool MQBoneManager::GetBaseRootPos(UINT bone_id, MQPoint& pos)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"org_root";
	array[3] = &pos;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetBaseTipPos(UINT bone_id, MQPoint& pos)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"org_tip";
	array[3] = &pos;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetBaseMatrix(UINT bone_id, MQMatrix& matrix)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"base_matrix";
	array[3] = matrix.t;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetUpVector(UINT bone_id, MQMatrix& matrix)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"upvector_matrix";
	array[3] = matrix.t;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetDeformRootPos(UINT bone_id, MQPoint& pos)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"def_root";
	array[3] = &pos;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetDeformTipPos(UINT bone_id, MQPoint& pos)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"def_tip";
	array[3] = &pos;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetDeformMatrix(UINT bone_id, MQMatrix& matrix)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"matrix";
	array[3] = matrix.t;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetDeformScale(UINT bone_id, MQPoint& scale)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"scale";
	array[3] = &scale;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetDeformRotate(UINT bone_id, MQAngle& angle)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"rotate";
	array[3] = &angle;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetDeformTranslate(UINT bone_id, MQPoint& translate)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"translate";
	array[3] = &translate;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetRotationMatrix(UINT bone_id, MQMatrix& matrix)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"rotate_matrix";
	array[3] = matrix.t;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetAngleMin(UINT bone_id, MQAngle& angle)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"angle_min";
	array[3] = &angle;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetAngleMax(UINT bone_id, MQAngle& angle)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"angle_max";
	array[3] = &angle;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetIKChain(UINT bone_id, int& chain)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"ikchain";
	array[3] = &chain;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetDummy(UINT bone_id, bool& dummy)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"dummy";
	array[3] = &dummy;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetEndPoint(UINT bone_id, bool& end_point)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"end_point";
	array[3] = &end_point;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetMovable(UINT bone_id, bool& movable)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"movable";
	array[3] = &movable;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

void MQBoneManager::SetName(UINT bone_id, const wchar_t *name)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"name";
	array[3] = (void*)&name;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetTipName(UINT bone_id, const wchar_t *tip_name)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"tip_name";
	array[3] = (void*)&tip_name;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager:: SetParent(UINT bone_id, UINT parent_id)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"parent";
	array[3] = &parent_id;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetBaseRootPos(UINT bone_id, const MQPoint& pos)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"org_root";
	array[3] = (void*)&pos;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetBaseTipPos(UINT bone_id, const MQPoint& pos)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"org_tip";
	array[3] = (void*)&pos;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetUpVector(UINT bone_id, const MQMatrix matrix)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"upvector_matrix";
	array[3] = (void*)matrix.t;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetDeformScale(UINT bone_id, const MQPoint& scale)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"scale";
	array[3] = (void*)&scale;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetDeformRotate(UINT bone_id, const MQAngle& angle)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"rotate";
	array[3] = (void*)&angle;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetDeformTranslate(UINT bone_id, const MQPoint& translate)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"translate";
	array[3] = (void*)&translate;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetAngleMin(UINT bone_id, const MQAngle& angle)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"angle_min";
	array[3] = (void*)&angle;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetAngleMax(UINT bone_id, const MQAngle& angle)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"angle_max";
	array[3] = (void*)&angle;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetIKChain(UINT bone_id, int chain)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"ikchain";
	array[3] = (void*)&chain;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetDummy(UINT bone_id, bool dummy)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"dummy";
	array[3] = (void*)&dummy;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetEndPoint(UINT bone_id, bool end_point)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"end_point";
	array[3] = (void*)&end_point;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetTipBone(UINT bone_id, UINT tip_bone_id)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"tip_bone";
	array[3] = &tip_bone_id;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

void MQBoneManager::SetMovable(UINT bone_id, bool movable)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"movable";
	array[3] = &movable;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetBone", array);
}

bool MQBoneManager::GetTipBone(UINT bone_id, UINT& tip_bone_id)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"id";
	array[1] = &bone_id;
	array[2] = (void*)"tip_bone";
	array[3] = &tip_bone_id;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBone", array);
	return (ret != 0);
}

bool MQBoneManager::GetIKName(UINT bone_id, std::wstring& ik_name, std::wstring& ik_tip_name)
{
	if(!m_Verified) return false;

	ik_name.clear();
	ik_tip_name.clear();
	const wchar_t *name = nullptr;
	const wchar_t *tip_name = nullptr;
	void *array[7];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"name";
	array[3] = &name;
	array[4] = (void*)"tip_name";
	array[5] = &tip_name;
	array[6] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetIKName", array);
	if(!ret)
		return false;
	if(name != nullptr)
		ik_name = std::wstring(name);
	if(tip_name != nullptr)
		ik_tip_name = std::wstring(tip_name);
	return true;
}

bool MQBoneManager::GetIKParent(UINT bone_id, UINT& ik_parent_bone_id, bool& isIK)
{
	if(!m_Verified) return false;

	void *array[7];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"parent";
	array[3] = &ik_parent_bone_id;
	array[4] = (void*)"isIK";
	array[5] = &isIK;
	array[6] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetIKParent", array);
	return (ret != 0);
}

bool MQBoneManager::GetLink(UINT bone_id, LINK_PARAM& param)
{
	if(!m_Verified) return false;

	void *array[11];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"link";
	array[3] = &param.link_bone_id;
	array[4] = (void*)"rot";
	array[5] = &param.rotate;
	array[6] = (void*)"move";
	array[7] = &param.move;
	array[8] = (void*)"scale";
	array[9] = &param.scale;
	array[10] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetLink", array);
	return (ret != 0);
}

bool MQBoneManager::GetGroupID(UINT bone_id, UINT& group_id)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"group";
	array[3] = &group_id;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetGroupID", array);
	return (ret != 0);
}

void MQBoneManager::SetIKName(UINT bone_id, const wchar_t *ik_name, const wchar_t *ik_tip_name)
{
	if(!m_Verified) return;

	void *array[7];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"name";
	array[3] = &ik_name;
	array[4] = (void*)"tip_name";
	array[5] = &ik_tip_name;
	array[6] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetIKName", array);
}

void MQBoneManager::SetIKParent(UINT bone_id, UINT ik_parent_bone_id, bool isIK)
{
	if(!m_Verified) return;

	void *array[7];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"parent";
	array[3] = &ik_parent_bone_id;
	array[4] = (void*)"isIK";
	array[5] = &isIK;
	array[6] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetIKParent", array);
}

void MQBoneManager::SetLink(UINT bone_id, const LINK_PARAM& param)
{
	if(!m_Verified) return;

	void *array[11];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"link";
	array[3] = (void*)&param.link_bone_id;
	array[4] = (void*)"rot";
	array[5] = (void*)&param.rotate;
	array[6] = (void*)"move";
	array[7] = (void*)&param.move;
	array[8] = (void*)"scale";
	array[9] = (void*)&param.scale;
	array[10] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetLink", array);
}

int MQBoneManager::EnumGroups(std::vector<UINT>& group_ids)
{
	if(!m_Verified) return 0;

	group_ids.clear();
	int group_num = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "QueryGroupNum", nullptr);
	if(group_num > 0){
		group_ids.resize(group_num);
		m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "EnumGroupID", group_ids.data());
	}
	return group_num;
}

UINT MQBoneManager::AddGroup(const wchar_t *name)
{
	if(!m_Verified) return 0;

	void *array[3];
	array[0] = (void*)"name";
	array[1] = (void*)name;
	array[2] = nullptr;
	UINT group_id = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "AddGroup", array);
	return group_id;
}

bool MQBoneManager::GetGroupName(UINT group_id, std::wstring& name)
{
	if(!m_Verified) return false;

	name.clear();

	const wchar_t *p = nullptr;
	void *array[5];
	array[0] = (void*)"group";
	array[1] = &group_id;
	array[2] = (void*)"name";
	array[3] = (void*)&p;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetGroupName", array);
	if(!ret)
		return false;
	if(p != nullptr)
		name = std::wstring(p);
	return true;
}

void MQBoneManager::SetGroupID(UINT bone_id, UINT group_id)
{
	if(!m_Verified) return;

	void *array[5];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"group";
	array[3] = &group_id;
	array[4] = nullptr;
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetGroup", array);
}

int MQBoneManager::GetSkinObjectNum()
{
	if(!m_Verified) return 0;

	int bone_object_num = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "QueryObjectNum", nullptr);
	return bone_object_num;
}

int MQBoneManager::EnumSkinObjectID(std::vector<UINT>& obj_id_array)
{
	if(!m_Verified) return 0;

	int num = GetSkinObjectNum();
	obj_id_array.resize(num);
	m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "EnumObjectID", obj_id_array.data());
	return num;
}

bool MQBoneManager::AddSkinObject(MQObject obj)
{
	if(!m_Verified) return false;

	void *array[3];
	array[0] = (void*)"object";
	array[1] = &obj;
	array[2] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "AddObject", array);
	return (ret != 0);
}

int MQBoneManager::GetVertexWeightArray(MQObject obj, UINT vertex_id, int array_num, UINT *bone_ids, float *weights)
{
	if(!m_Verified) return false;

	if(obj == nullptr)
		return false;
	UINT obj_id = obj->GetUniqueID();
	void *array[5];
	array[0] = &obj_id;
	array[1] = &vertex_id;
	array[2] = &array_num;
	array[3] = bone_ids;
	array[4] = weights;
	int weight_num = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetVertexWeight", array);
	return weight_num;
}

bool MQBoneManager::SetVertexWeight(MQObject obj, UINT vertex_id, UINT bone_id, float weight)
{
	if(!m_Verified) return false;

	if(obj == nullptr)
		return false;
	UINT obj_id = obj->GetUniqueID();
	void *array[9];
	array[0] = (void*)"bone";
	array[1] = &bone_id;
	array[2] = (void*)"object";
	array[3] = &obj_id;
	array[4] = (void*)"vertex";
	array[5] = &vertex_id;
	array[6] = (void*)"weight";
	array[7] = &weight;
	array[8] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetWeight", array);
	return (ret != 0);
}

int MQBoneManager::GetWeightedVertexArray(UINT bone_id, MQObject obj, std::vector<UINT>& vertex_ids, std::vector<float>& weights)
{
	if(!m_Verified) return false;

	if(obj == nullptr)
		return false;
	UINT obj_id = obj->GetUniqueID();
	void *array[4];
	array[0] = &obj_id;
	array[1] = &bone_id;
	array[2] = nullptr;
	array[3] = nullptr;
	int vert_num = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBoneWeight", array);
	if(vert_num > 0){				
		vertex_ids.resize(vert_num);
		weights.resize(vert_num);
		array[2] = vertex_ids.data();
		array[3] = weights.data();
		m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetBoneWeight", array);
	}else{
		vertex_ids.clear();
		weights.clear();
	}
	return vert_num;
}

int MQBoneManager::GetEffectLimitNum()
{
	if(!m_Verified) return 0;

	void *array = nullptr;
	int effectlimit = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "GetEffectLimitNum", array);
	return effectlimit;
}

void MQBoneManager::SetListMode(LIST_MODE mode)
{
	if(!m_Verified) return;

	switch(mode){
	case LIST_MODE_BONE:
		m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetListMode", (void*)"bone");
		break;
	case LIST_MODE_NODE:
		m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "SetListMode", (void*)"node");
		break;
	}
}

bool MQBoneManager::DeformObject(MQObject obj, MQObject target)
{
	if(!m_Verified) return false;

	void *array[5];
	array[0] = (void*)"object";
	array[1] = obj;
	array[2] = (void*)"target";
	array[3] = target;
	array[4] = nullptr;
	int ret = m_Plugin->SendUserMessage(m_Doc, bone_plugin_product, bone_plugin_id, "DeformObject", array);
	return (ret != 0);
}
