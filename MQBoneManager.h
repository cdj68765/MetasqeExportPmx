//---------------------------------------------------------------------------
//
//   MQBoneManager.h      Copyright(C) 1999-2016, tetraface Inc.
//
//		An accessor for bone information
//
//    　ボーン情報のアクセス
//
//---------------------------------------------------------------------------

#ifndef _MQBONEMANAGER_H_
#define _MQBONEMANAGER_H_

#include "MQBasePlugin.h"

class MQBoneManager
{
public:
	MQBoneManager(MQBasePlugin* plugin, MQDocument doc);

	static DWORD GetProductID();
	static DWORD GetPluginID();

	bool Verified() const { return m_Verified; }

	// Must call first in a 'Import' plugin.
	void BeginImport();
	// Must call last in a 'Import' plugin.
	void EndImport();

	struct ADD_BONE_PARAM
	{
		MQPoint root_pos;
		MQPoint tip_pos;
		std::wstring name;
		UINT parent_id;
		int ik_chain;
		bool is_dummy;
		bool end_point;

		ADD_BONE_PARAM()
		{
			root_pos = MQPoint(0, 0, 0);
			tip_pos = MQPoint(0, 0, 0);
			parent_id = 0;
			ik_chain = -1;
			is_dummy = false;
			end_point = false;
		}
	};

	struct LINK_PARAM
	{
		UINT link_bone_id;
		float rotate;
		float move;
		float scale;

		LINK_PARAM()
		{
			link_bone_id = 0;
			rotate = move = scale = 100.f;
		}
	};

	UINT AddBone(const ADD_BONE_PARAM& param);
	bool AddBrother(UINT bone_id, UINT brother_bone_id);

	// Bone
	int GetBoneNum();
	int EnumBoneID(std::vector<UINT>& bone_id_array);

	bool GetName(UINT bone_id, std::wstring& name);
	bool GetTipName(UINT bone_id, std::wstring& name);
	bool GetParent(UINT bone_id, UINT& parent_id);
	bool GetChildNum(UINT bone_id, int& child_num);
	bool GetChildren(UINT bone_id, std::vector<UINT>& children);
	bool GetBrothers(UINT bone_id, std::vector<UINT>& brothers);
	bool GetBaseRootPos(UINT bone_id, MQPoint& pos);
	bool GetBaseTipPos(UINT bone_id, MQPoint& pos);
	bool GetBaseMatrix(UINT bone_id, MQMatrix& matrix);
	bool GetUpVector(UINT bone_id, MQMatrix& matrix);
	bool GetDeformRootPos(UINT bone_id, MQPoint& pos);
	bool GetDeformTipPos(UINT bone_id, MQPoint& pos);
	bool GetDeformMatrix(UINT bone_id, MQMatrix& matrix);
	bool GetDeformScale(UINT bone_id, MQPoint& scale);
	bool GetDeformRotate(UINT bone_id, MQAngle& angle);
	bool GetDeformTranslate(UINT bone_id, MQPoint& translate);
	bool GetRotationMatrix(UINT bone_id, MQMatrix& matrix);
	bool GetAngleMin(UINT bone_id, MQAngle& angle);
	bool GetAngleMax(UINT bone_id, MQAngle& angle);
	bool GetIKChain(UINT bone_id, int& chain);
	bool GetDummy(UINT bone_id, bool& dummy);
	bool GetEndPoint(UINT bone_id, bool& end_point);
	bool GetMovable(UINT bone_id, bool& movable);

	void SetName(UINT bone_id, const wchar_t* name);
	void SetTipName(UINT bone_id, const wchar_t* tip_name);
	void SetParent(UINT bone_id, UINT parent_id);
	void SetBaseRootPos(UINT bone_id, const MQPoint& pos);
	void SetBaseTipPos(UINT bone_id, const MQPoint& pos);
	void SetUpVector(UINT bone_id, const MQMatrix matrix);
	void SetDeformScale(UINT bone_id, const MQPoint& scale);
	void SetDeformRotate(UINT bone_id, const MQAngle& angle);
	void SetDeformTranslate(UINT bone_id, const MQPoint& translate);
	void SetAngleMin(UINT bone_id, const MQAngle& angle);
	void SetAngleMax(UINT bone_id, const MQAngle& angle);
	void SetDummy(UINT bone_id, bool dummy);
	void SetEndPoint(UINT bone_id, bool end_point);
	void SetIKChain(UINT bone_id, int chain);
	void SetMovable(UINT bone_id, bool movable);

	// Skin object
	int GetSkinObjectNum();
	int EnumSkinObjectID(std::vector<UINT>& obj_id_array);
	bool AddSkinObject(MQObject obj);
	int GetVertexWeightArray(MQObject obj, UINT vertex_id, int array_num, UINT* bone_ids, float* weights);
	bool SetVertexWeight(MQObject obj, UINT vertex_id, UINT bone_id, float weight);
	int GetWeightedVertexArray(UINT bone_id, MQObject obj, std::vector<UINT>& vertex_ids, std::vector<float>& weights);

	// for PMD
	bool GetTipBone(UINT bone_id, UINT& tip_bone_id);
	bool GetIKName(UINT bone_id, std::wstring& ik_name, std::wstring& ik_tip_name);
	bool GetIKParent(UINT bone_id, UINT& ik_parent_bone_id, bool& isIK);
	bool GetLink(UINT bone_id, LINK_PARAM& param);
	bool GetGroupID(UINT bone_id, UINT& group_id);
	void SetTipBone(UINT bone_id, UINT tip_bone_id);
	void SetIKName(UINT bone_id, const wchar_t* ik_name, const wchar_t* ik_tip_name);
	void SetIKParent(UINT bone_id, UINT ik_parent_bone_id, bool isIK);
	void SetLink(UINT bone_id, const LINK_PARAM& param);
	void SetGroupID(UINT bone_id, UINT group_id);

	// for PMD group
	int EnumGroups(std::vector<UINT>& group_ids);
	UINT AddGroup(const wchar_t* name); // ANSI string
	bool GetGroupName(UINT group_id, std::wstring& name);

	// Configurations
	int GetEffectLimitNum();

	enum LIST_MODE
	{
		LIST_MODE_BONE,
		LIST_MODE_NODE,
	};

	void SetListMode(LIST_MODE mode);

	// Deform
	bool DeformObject(MQObject obj, MQObject target); // 'obj' must be a skin object, and 'target' must be a cloned object from 'obj'

private:
	MQBasePlugin* m_Plugin;
	MQDocument m_Doc;
	bool m_Verified;
};

#endif //_MQBONEMANAGER_H_
