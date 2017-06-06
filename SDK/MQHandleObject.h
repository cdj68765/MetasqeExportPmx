//---------------------------------------------------------------------------
//
//   MQHandleObject.h      Copyright(C) 1999-2016, tetraface Inc.
//
//		A handle for moving, scaling or rotating
//
//    　移動・拡大・回転用のハンドル
//
//---------------------------------------------------------------------------

#ifndef _MQHANDLEOBJECT_H_
#define _MQHANDLEOBJECT_H_

#include "MQBasePlugin.h"

struct MQHandleInfo
{
	enum AxisType {
		HandleNone = 0,
		HandleW = 1,
		HandleX = 2,
		HandleY = 4,
		HandleZ = 8,
		HandleView = 16,
		HandleVisibleAxisOnly = 32,
		Handle_DWORD = 0xFFFFFFFF,
	};

	enum HandleRotationType {
		RotationDefault = -1,
		RotationArrow = 0,
		RotationRing,
	};

	MQHandleInfo(MQScene scene, const MQPoint& center_pos, bool check_visibility = true);
	MQHandleInfo(MQScene scene, const MQPoint& center_pos, const MQMatrix& mtx, bool check_visibility = true);
	void SetHandleVisibility(MQScene scene);

	MQPoint 	center; 		// 中心位置
	MQPoint 	scale; 			// 軸の長さ
	float		size;			// 取っ手のサイズ
	MQMatrix 	matrix;			// 回転行列
	MQPoint		camdir;			// カメラの向き
	AxisType 	visible;		// 可視状態
	AxisType	active; 		// アクティブ状態
	HandleRotationType	rotation;	// 回転ハンドルのタイプ
};


struct MQHandleMaterial {
	int MatIdxW;
	int MatIdxX;
	int MatIdxY;
	int MatIdxZ;
	int MatIdxView;

	MQHandleMaterial(){
		MatIdxW = MatIdxX = MatIdxY = MatIdxZ = MatIdxView = -1;
	}
};


class MQHandleObject {
public:
	MQObject ThreeLine;
	MQObject RotationCircle;
	MQObject Handle;
	bool Ring;
	bool ForDrawing;

	MQHandleObject();
	~MQHandleObject();

	void DeleteThis();

	bool CreateMoveHandle(MQCommandPlugin *plugin, bool for_drawing, MQDocument doc, const MQHandleInfo& info, MQHandleMaterial *pMatIdx);
	bool CreateScaleHandle(MQCommandPlugin *plugin, bool for_drawing, MQDocument doc, const MQHandleInfo& info, MQHandleMaterial *pMatIdx);
	bool CreateRotateHandle(MQCommandPlugin *plugin, bool for_drawing, MQDocument doc, const MQHandleInfo& info, MQHandleMaterial *pMatIdx);

	static MQHandleInfo::AxisType HitTestMoveHandle(MQCommandPlugin *plugin, MQDocument doc, MQScene scene, MQCommandPlugin::MOUSE_BUTTON_STATE& state, const MQHandleInfo& info, MQPoint& hit_pos);
	static MQHandleInfo::AxisType HitTestScaleHandle(MQCommandPlugin *plugin, MQDocument doc, MQScene scene, MQCommandPlugin::MOUSE_BUTTON_STATE& state, const MQHandleInfo& info, MQPoint& hit_pos);
	static MQHandleInfo::AxisType HitTestRotateHandle(MQCommandPlugin *plugin, MQDocument doc, MQScene scene, MQCommandPlugin::MOUSE_BUTTON_STATE& state, const MQHandleInfo& info, MQPoint& hit_pos);

private:
	bool CreateThreeHandle(MQCommandPlugin *plugin, bool for_drawing, MQDocument doc, const MQHandleInfo& info, MQHandleMaterial& matidx, bool rotate_circle);
};


class MQHandleOperation {
public:
	MQHandleOperation();

	void Begin(MQCommandPlugin *plugin, const MQHandleInfo& info, MQHandleInfo::AxisType axis_type, POINT mousePos, MQPoint hitPos);
	
	bool Move(MQPoint& offset, MQScene scene, POINT mousePos);
	bool Scale(MQPoint& scaling, MQScene scene, POINT mousePos);
	bool Rotate(MQMatrix& mtx, MQScene scene, POINT mousePos, bool snapping = false);

	MQMatrix GetMatrix() const { return Matrix; }
	float GetTotalAngle() const { return TotalAngle; }

private:
	MQCommandPlugin *Plugin;
	MQHandleInfo::HandleRotationType RotationType;
	MQHandleInfo::AxisType AxisType;
	MQPoint Center;
	POINT PrevPos, DownPos;
	MQPoint HitPos;
	
	MQMatrix Matrix;
	float TotalAngle;
};


#endif //_MQHANDLEOBJECT_H_
