//---------------------------------------------------------------------------
//
//   MQHandleObject.cpp      Copyright(C) 1999-2016, tetraface Inc.
//
//		A handle for moving, scaling or rotating
//
//    　移動・拡大・回転用のハンドル
//
//---------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include "MQHandleObject.h"
#include "MQ3DLib.h"


MQHandleInfo::MQHandleInfo(MQScene scene, const MQPoint& center_pos, bool check_visibility)
{
	visible = (AxisType)(HandleX|HandleY|HandleZ|HandleW|HandleView);

	MQPoint r = scene->GetRotationCenter(); 
	MQPoint p = scene->Convert3DToScreen(r);
	MQPoint q = scene->ConvertScreenTo3D(MQPoint(p.x+70.0f,p.y+70.0f,p.z));
	MQPoint campos = scene->GetCameraPosition();

	center = center_pos;
	scale  = (q - r).abs();
	size   = std::max(scale.x, std::max(scale.y, scale.z)) * 0.055f * 1.0f/*IniInfo.HandleSize*/;
	matrix.Identify();
	camdir = Normalize(campos - center);
	active = HandleNone;
	rotation = RotationDefault;

	if(check_visibility){
		SetHandleVisibility(scene);
	}
}

MQHandleInfo::MQHandleInfo(MQScene scene, const MQPoint& center_pos, const MQMatrix& mtx, bool check_visibility)
{
	visible = (AxisType)(HandleX|HandleY|HandleZ|HandleW|HandleView);

	MQPoint r = scene->GetRotationCenter(); 
	MQPoint p = scene->Convert3DToScreen(r);
	MQPoint q = scene->ConvertScreenTo3D(MQPoint(p.x+70.0f,p.y+70.0f,p.z));
	MQPoint campos = scene->GetCameraPosition();

	center = center_pos;
	scale  = (q - r).abs();
	size   = std::max(scale.x, std::max(scale.y, scale.z)) * 0.055f * 1.0f/*IniInfo.HandleSize*/;
	matrix = mtx;
	camdir = Normalize(campos - center);
	active = HandleNone;
	rotation = RotationDefault;

	if(check_visibility){
		SetHandleVisibility(scene);
	}
}

void MQHandleInfo::SetHandleVisibility(MQScene scene)
{
	MQPoint v = matrix.Mult3(MQPoint(0,0,0)) + center;
	MQPoint vx = matrix.Mult3(MQPoint(scale.x,0,0)) + center;
	MQPoint vy = matrix.Mult3(MQPoint(0,scale.y,0)) + center;
	MQPoint vz = matrix.Mult3(MQPoint(0,0,scale.z)) + center;

	MQPoint sv, svx, svy, svz;
	sv = scene->Convert3DToScreen(v);
	svx = scene->Convert3DToScreen(vx);
	svy = scene->Convert3DToScreen(vy);
	svz = scene->Convert3DToScreen(vz);
	bool ev = (sv != MQPoint(0,0,0));
	bool evx = (svx != MQPoint(0,0,0));
	bool evy = (svy != MQPoint(0,0,0));
	bool evz = (svz != MQPoint(0,0,0));

	if(!ev || !evx || ((MQPoint(sv.x, sv.y, 0) - MQPoint(svx.x, svx.y, 0)).abs() < 8)){
		visible = (AxisType)(visible & ~(int)HandleX);
	}
	if(!ev || !evy || ((MQPoint(sv.x, sv.y, 0) - MQPoint(svy.x, svy.y, 0)).abs() < 8)){
		visible = (AxisType)(visible & ~(int)HandleY);
	}
	if(!ev || !evz || ((MQPoint(sv.x, sv.y, 0) - MQPoint(svz.x, svz.y, 0)).abs() < 8)){
		visible = (AxisType)(visible & ~(int)HandleZ);
	}
}



MQHandleObject::MQHandleObject()
{
	ThreeLine = NULL;
	RotationCircle = NULL;
	Handle = NULL;
	Ring = false;
	ForDrawing = false;
}

MQHandleObject::~MQHandleObject()
{
	if(!ForDrawing){
		DeleteThis();
	}
}

void MQHandleObject::DeleteThis()
{
	if(ThreeLine != NULL){
		ThreeLine->DeleteThis();
		ThreeLine = NULL;
	}
	if(RotationCircle != NULL){
		RotationCircle->DeleteThis();
		RotationCircle = NULL;
	}
	if(Handle != NULL){
		Handle->DeleteThis();
		Handle = NULL;
	}
}

//---------------------------------------------------------------------------
//  SlicerPlugin::CreateThreeHandle
//    移動・拡大用の３方向ハンドルを作成する
//---------------------------------------------------------------------------
bool MQHandleObject::CreateThreeHandle(MQCommandPlugin *plugin, bool for_drawing, MQDocument doc, const MQHandleInfo& info, MQHandleMaterial& matidx, bool rotate_circle)
{
	MQObject obj;
	MQMaterial mat;
	float handle_scale = plugin->GetSettingFloatValue(MQSETTINGVALUE_HANDLE_SCALE);
	MQPoint scale = info.scale * handle_scale;
	float len = info.size * handle_scale;

	// ３軸ライン用オブジェクトの作成
	if(for_drawing){
		obj = plugin->CreateDrawingObject(doc, MQCommandPlugin::DRAW_OBJECT_LINE);
	}else{
		obj = MQ_CreateObject();
	}
	obj->SetColorValid(TRUE);
	obj->SetColor(MQColor(0.5f,0.5f,0.5f));
	//obj->setSelectionColorType( MOBJECT_COLOR_UNIQUE );
	obj->AddRenderFlag(MQOBJECT_RENDER_LINE);
	obj->AddRenderFlag(MQOBJECT_RENDER_OVERWRITELINE);
	obj->AddRenderEraseFlag(MQOBJECT_RENDER_MULTILIGHT);
	obj->AddVertex( info.center );
	if((info.visible & MQHandleInfo::HandleX) || !(info.visible & MQHandleInfo::HandleVisibleAxisOnly)){
		int vi = obj->AddVertex( info.matrix.Mult3(MQPoint(scale.x,0,0)) + info.center );
		int nvi[2] = {0, vi};
		int fi = obj->AddFace(2, nvi);
		if(info.active & MQHandleInfo::HandleX){
			//obj->setSelectionColor( MColor(IniInfo.ColorAxisX) );
			//obj->f[fi].select.setSelect();
			//obj->f[fi].apex[0].selline.setSelect();
		}
	}
	if((info.visible & MQHandleInfo::HandleY) || !(info.visible & MQHandleInfo::HandleVisibleAxisOnly)){
		int vi = obj->AddVertex( info.matrix.Mult3(MQPoint(0,scale.y,0)) + info.center );
		int nvi[2] = {0, vi};
		int fi = obj->AddFace(2, nvi);
		if(info.active & MQHandleInfo::HandleY){
			//obj->setSelectionColor( MColor(IniInfo.ColorAxisY) );
			//obj->f[fi].select.setSelect();
			//obj->f[fi].apex[0].selline.setSelect();
		}
	}
	if((info.visible & MQHandleInfo::HandleZ) || !(info.visible & MQHandleInfo::HandleVisibleAxisOnly)){
		int vi = obj->AddVertex( info.matrix.Mult3(MQPoint(0,0,scale.z)) + info.center );
		int nvi[2] = {0, vi};
		int fi = obj->AddFace(2, nvi);
		if(info.active & MQHandleInfo::HandleZ){
			//obj->setSelectionColor( MColor(IniInfo.ColorAxisZ) );
			//obj->f[2].select.setSelect();
			//obj->f[2].apex[0].selline.setSelect();
		}
	}
	this->ThreeLine = obj;

	// 円回転用オブジェクトの作成
	if (rotate_circle){
		// ※ここでは作成のみ
		if(for_drawing){
			this->RotationCircle = plugin->CreateDrawingObject(doc, MQCommandPlugin::DRAW_OBJECT_LINE);
		}else{
			this->RotationCircle = MQ_CreateObject();
		}
	}

	// ハンドル用オブジェクトの作成
	if(for_drawing){
		obj = plugin->CreateDrawingObject(doc, MQCommandPlugin::DRAW_OBJECT_FACE);
	}else{
		obj = MQ_CreateObject();
	}
	obj->AddRenderFlag(MQOBJECT_RENDER_FACE);
	obj->AddRenderFlag(MQOBJECT_RENDER_OVERWRITEFACE);
	obj->AddRenderEraseFlag(MQOBJECT_RENDER_MULTILIGHT);
	obj->AddRenderEraseFlag(MQOBJECT_RENDER_SHADOW);
	this->Handle = obj;

	// Center
	mat = plugin->CreateDrawingMaterial(doc, matidx.MatIdxW);
	if((info.active & MQHandleInfo::HandleX) ||
	   (info.active & MQHandleInfo::HandleY) ||
	   (info.active & MQHandleInfo::HandleZ) ||
	   (info.active & MQHandleInfo::HandleW) ||
	   (info.active & MQHandleInfo::HandleView))
	{
		mat->SetColor(plugin->GetSystemColor(MQSYSTEMCOLOR_AXIS_ACTIVE));
	}else{
		mat->SetColor(plugin->GetSystemColor(MQSYSTEMCOLOR_AXIS_W));
	}
	mat->SetDiffuse(0.5f);
	mat->SetEmission(0.5f);
	mat->SetAmbient(0.0f);
	mat->SetSpecular(0.0f);

	// X
	mat = plugin->CreateDrawingMaterial(doc, matidx.MatIdxX);
	if(info.active & MQHandleInfo::HandleX)
		mat->SetColor(plugin->GetSystemColor(MQSYSTEMCOLOR_AXIS_ACTIVE));
	else
		mat->SetColor(plugin->GetSystemColor(MQSYSTEMCOLOR_AXIS_X));
	mat->SetDiffuse(0.5f);
	mat->SetEmission(0.5f);
	mat->SetAmbient(0.0f);
	mat->SetSpecular(0.0f);

	// Y
	mat = plugin->CreateDrawingMaterial(doc, matidx.MatIdxY);
	if(info.active & MQHandleInfo::HandleY)
		mat->SetColor(plugin->GetSystemColor(MQSYSTEMCOLOR_AXIS_ACTIVE));
	else
		mat->SetColor(plugin->GetSystemColor(MQSYSTEMCOLOR_AXIS_Y));
	mat->SetDiffuse(0.5f);
	mat->SetEmission(0.5f);
	mat->SetAmbient(0.0f);
	mat->SetSpecular(0.0f);

	// Z
	mat = plugin->CreateDrawingMaterial(doc, matidx.MatIdxZ);
	if(info.active & MQHandleInfo::HandleZ)
		mat->SetColor(plugin->GetSystemColor(MQSYSTEMCOLOR_AXIS_ACTIVE));
	else
		mat->SetColor(plugin->GetSystemColor(MQSYSTEMCOLOR_AXIS_Z));
	mat->SetDiffuse(0.5f);
	mat->SetEmission(0.5f);
	mat->SetAmbient(0.0f);
	mat->SetSpecular(0.0f);

	// View
	mat = plugin->CreateDrawingMaterial(doc, matidx.MatIdxView);
	if(info.active & MQHandleInfo::HandleZ)
		mat->SetColor(plugin->GetSystemColor(MQSYSTEMCOLOR_AXIS_ACTIVE));
	else
		mat->SetColor(plugin->GetSystemColor(MQSYSTEMCOLOR_AXIS_W));
	mat->SetDiffuse(0.5f);
	mat->SetEmission(0.5f);
	mat->SetAmbient(0.0f);
	mat->SetSpecular(0.0f);

	if(info.active & MQHandleInfo::HandleX){
		obj->AddVertex( info.matrix.Mult3(MQPoint(0,+len*0.2f,-len*0.2f)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(0,+len*0.2f,+len*0.2f)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(0,-len*0.2f,+len*0.2f)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(0,-len*0.2f,-len*0.2f)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(scale.x,+len*0.2f,-len*0.2f)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(scale.x,+len*0.2f,+len*0.2f)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(scale.x,-len*0.2f,+len*0.2f)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(scale.x,-len*0.2f,-len*0.2f)) + info.center );
		int nvi[4], nfi;
		nvi[0]=0; nvi[1]=4; nvi[2]=5; nvi[3]=1;
		nfi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(nfi, matidx.MatIdxX);
		nvi[0]=1; nvi[1]=5; nvi[2]=6; nvi[3]=2;
		nfi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(nfi, matidx.MatIdxX);
		nvi[0]=2; nvi[1]=6; nvi[2]=7; nvi[3]=3;
		nfi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(nfi, matidx.MatIdxX);
		nvi[0]=3; nvi[1]=7; nvi[2]=4; nvi[3]=0;
		nfi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(nfi, matidx.MatIdxX);
	}
	if(info.active & MQHandleInfo::HandleY){
		obj->AddVertex( info.matrix.Mult3(MQPoint(+len*0.2f,0,+len*0.2f)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(+len*0.2f,0,-len*0.2f)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(-len*0.2f,0,-len*0.2f)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(-len*0.2f,0,+len*0.2f)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(+len*0.2f,scale.y,+len*0.2f)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(+len*0.2f,scale.y,-len*0.2f)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(-len*0.2f,scale.y,-len*0.2f)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(-len*0.2f,scale.y,+len*0.2f)) + info.center );
		int nvi[4], nfi;
		nvi[0]=0; nvi[1]=4; nvi[2]=5; nvi[3]=1;
		nfi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(nfi, matidx.MatIdxY);
		nvi[0]=1; nvi[1]=5; nvi[2]=6; nvi[3]=2;
		nfi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(nfi, matidx.MatIdxY);
		nvi[0]=2; nvi[1]=6; nvi[2]=7; nvi[3]=3;
		nfi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(nfi, matidx.MatIdxY);
		nvi[0]=3; nvi[1]=7; nvi[2]=4; nvi[3]=0;
		nfi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(nfi, matidx.MatIdxY);
	}
	if(info.active & MQHandleInfo::HandleZ){
		obj->AddVertex( info.matrix.Mult3(MQPoint(+len*0.2f,-len*0.2f,0)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(+len*0.2f,+len*0.2f,0)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(-len*0.2f,+len*0.2f,0)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(-len*0.2f,-len*0.2f,0)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(+len*0.2f,-len*0.2f,scale.z)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(+len*0.2f,+len*0.2f,scale.z)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(-len*0.2f,+len*0.2f,scale.z)) + info.center );
		obj->AddVertex( info.matrix.Mult3(MQPoint(-len*0.2f,-len*0.2f,scale.z)) + info.center );
		int nvi[4], nfi;
		nvi[0]=0; nvi[1]=4; nvi[2]=5; nvi[3]=1;
		nfi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(nfi, matidx.MatIdxZ);
		nvi[0]=1; nvi[1]=5; nvi[2]=6; nvi[3]=2;
		nfi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(nfi, matidx.MatIdxZ);
		nvi[0]=2; nvi[1]=6; nvi[2]=7; nvi[3]=3;
		nfi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(nfi, matidx.MatIdxZ);
		nvi[0]=3; nvi[1]=7; nvi[2]=4; nvi[3]=0;
		nfi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(nfi, matidx.MatIdxZ);
	}

	this->ForDrawing = for_drawing;

	return true;
}


bool MQHandleObject::CreateMoveHandle(MQCommandPlugin *plugin, bool for_drawing, MQDocument doc, const MQHandleInfo& info, MQHandleMaterial *pMatIdx)
{
	MQHandleMaterial matidx;
	int i,vi,fi;
	MQPoint pos;
	float handle_scale = plugin->GetSettingFloatValue(MQSETTINGVALUE_HANDLE_SCALE);
	MQPoint scale = info.scale * handle_scale;
	float len = info.size * handle_scale;
	
	if(!CreateThreeHandle(plugin, for_drawing, doc, info, matidx, false)){
		return false;
	}
	
	MQObject obj = this->Handle;

#define MOVE_SEG   16
#define MOVE_SIZE  1.8f

	// center
	if( info.visible & MQHandleInfo::HandleW )
	{
		int vi[8];
		vi[0] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, +len, +len)) + info.center );
		vi[1] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, +len, +len)) + info.center );
		vi[2] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, -len, +len)) + info.center );
		vi[3] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, -len, +len)) + info.center );
		vi[4] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, +len, -len)) + info.center );
		vi[5] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, +len, -len)) + info.center );
		vi[6] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, -len, -len)) + info.center );
		vi[7] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, -len, -len)) + info.center );
		
		int nvi[4];
		nvi[0]=vi[1]; nvi[1]=vi[0]; nvi[2]=vi[2]; nvi[3]=vi[3];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxW);

		nvi[0]=vi[4]; nvi[1]=vi[5]; nvi[2]=vi[7]; nvi[3]=vi[6];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxW);

		nvi[0]=vi[0]; nvi[1]=vi[4]; nvi[2]=vi[6]; nvi[3]=vi[2];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxW);

		nvi[0]=vi[3]; nvi[1]=vi[7]; nvi[2]=vi[5]; nvi[3]=vi[1];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxW);

		nvi[0]=vi[0]; nvi[1]=vi[1]; nvi[2]=vi[5]; nvi[3]=vi[4];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxW);

		nvi[0]=vi[6]; nvi[1]=vi[7]; nvi[2]=vi[3]; nvi[3]=vi[2];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxW);
	}

	len *= 1.2f;

	// X
	if( info.visible & MQHandleInfo::HandleX )
	{
		pos = info.matrix.Mult3(MQPoint(scale.x, 0, 0)) + info.center;
		vi = obj->GetVertexCount();
		for(i=0; i<MOVE_SEG; i++) {
			obj->AddVertex( info.matrix.Mult3(MQPoint(
					len * (-MOVE_SIZE),
					len * cosf((float)i / MOVE_SEG * PI * 2),
					len * sinf((float)i / MOVE_SEG * PI * 2))) + pos );
		}
		obj->AddVertex( info.matrix.Mult3(MQPoint(len*(-MOVE_SIZE), 0.0f, 0.0f)) + pos );
		obj->AddVertex( info.matrix.Mult3(MQPoint(len*(+MOVE_SIZE), 0.0f, 0.0f)) + pos );
		for(i=0; i<MOVE_SEG; i++) {
			int nvi[3];
			nvi[0] = vi+MOVE_SEG;
			nvi[1] = vi+i;
			nvi[2] = vi+(i+1)%MOVE_SEG;
			fi = obj->AddFace(3, nvi);
			obj->SetFaceMaterial(fi, matidx.MatIdxX);

			nvi[0] = vi+MOVE_SEG+1;
			nvi[1] = vi+(i+1)%MOVE_SEG;
			nvi[2] = vi+i;
			fi = obj->AddFace(3, nvi);
			obj->SetFaceMaterial(fi, matidx.MatIdxX);
		}
	}

	// Y
	if( info.visible & MQHandleInfo::HandleY )
	{
		pos = info.matrix.Mult3(MQPoint(0, scale.y, 0)) + info.center;
		vi = obj->GetVertexCount();
		for(i=0; i<MOVE_SEG; i++) {
			obj->AddVertex( info.matrix.Mult3(MQPoint(
					len * sinf((float)i / MOVE_SEG * PI * 2),
					len * (-MOVE_SIZE),
					len * cosf((float)i / MOVE_SEG * PI * 2))) + pos );
		}
		obj->AddVertex( info.matrix.Mult3(MQPoint(0.0f, len*(-MOVE_SIZE), 0.0f)) + pos );
		obj->AddVertex( info.matrix.Mult3(MQPoint(0.0f, len*(+MOVE_SIZE), 0.0f)) + pos );
		for(i=0; i<MOVE_SEG; i++) {
			int nvi[3];
			nvi[0] = vi+MOVE_SEG;
			nvi[1] = vi+i;
			nvi[2] = vi+(i+1)%MOVE_SEG;
			fi = obj->AddFace(3, nvi);
			obj->SetFaceMaterial(fi, matidx.MatIdxY);

			nvi[0] = vi+MOVE_SEG+1;
			nvi[1] = vi+(i+1)%MOVE_SEG;
			nvi[2] = vi+i;
			fi = obj->AddFace(3, nvi);
			obj->SetFaceMaterial(fi, matidx.MatIdxY);
		}
	}

	// Z
	if( info.visible & MQHandleInfo::HandleZ )
	{
		pos = info.matrix.Mult3(MQPoint(0, 0, scale.z)) + info.center;
		vi = obj->GetVertexCount();
		for(i=0; i<MOVE_SEG; i++) {
			obj->AddVertex( info.matrix.Mult3(MQPoint(
					len * cosf((float)i / MOVE_SEG * PI * 2),
					len * sinf((float)i / MOVE_SEG * PI * 2),
					len * (-MOVE_SIZE))) + pos );
		}
		obj->AddVertex( info.matrix.Mult3(MQPoint(0.0f, 0.0f, len*(-MOVE_SIZE))) + pos );
		obj->AddVertex( info.matrix.Mult3(MQPoint(0.0f, 0.0f, len*(+MOVE_SIZE))) + pos );
		for(i=0; i<MOVE_SEG; i++) {
			int nvi[3];
			nvi[0] = vi+MOVE_SEG;
			nvi[1] = vi+i;
			nvi[2] = vi+(i+1)%MOVE_SEG;
			fi = obj->AddFace(3, nvi);
			obj->SetFaceMaterial(fi, matidx.MatIdxZ);

			nvi[0] = vi+MOVE_SEG+1;
			nvi[1] = vi+(i+1)%MOVE_SEG;
			nvi[2] = vi+i;
			fi = obj->AddFace(3, nvi);
			obj->SetFaceMaterial(fi, matidx.MatIdxZ);
		}
	}

	if (pMatIdx != NULL)
		*pMatIdx = matidx;

	return true;
}


bool MQHandleObject::CreateScaleHandle(MQCommandPlugin *plugin, bool for_drawing, MQDocument doc, const MQHandleInfo& info, MQHandleMaterial *pMatIdx)
{
	MQHandleMaterial matidx;
	int fi;
	float handle_scale = plugin->GetSettingFloatValue(MQSETTINGVALUE_HANDLE_SCALE);
	MQPoint scale = info.scale * handle_scale;
	float len = info.size * handle_scale;
	
	if(!CreateThreeHandle(plugin, for_drawing, doc, info, matidx, false)){
		return false;
	}

	MQObject obj = this->Handle;

	// center
	if( info.visible & MQHandleInfo::HandleW )
	{
		int vi[8];
		vi[0] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, +len, +len)) + info.center );
		vi[1] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, +len, +len)) + info.center );
		vi[2] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, -len, +len)) + info.center );
		vi[3] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, -len, +len)) + info.center );
		vi[4] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, +len, -len)) + info.center );
		vi[5] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, +len, -len)) + info.center );
		vi[6] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, -len, -len)) + info.center );
		vi[7] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, -len, -len)) + info.center );
		
		int nvi[4];
		nvi[0]=vi[1]; nvi[1]=vi[0]; nvi[2]=vi[2]; nvi[3]=vi[3];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxW);

		nvi[0]=vi[4]; nvi[1]=vi[5]; nvi[2]=vi[7]; nvi[3]=vi[6];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxW);

		nvi[0]=vi[0]; nvi[1]=vi[4]; nvi[2]=vi[6]; nvi[3]=vi[2];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxW);

		nvi[0]=vi[3]; nvi[1]=vi[7]; nvi[2]=vi[5]; nvi[3]=vi[1];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxW);

		nvi[0]=vi[0]; nvi[1]=vi[1]; nvi[2]=vi[5]; nvi[3]=vi[4];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxW);

		nvi[0]=vi[6]; nvi[1]=vi[7]; nvi[2]=vi[3]; nvi[3]=vi[2];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxW);
	}

	// X
	if( info.visible & MQHandleInfo::HandleX )
	{
		int vi[8];
		vi[0] = obj->AddVertex( info.matrix.Mult3(MQPoint(scale.x+len, +len, +len)) + info.center );
		vi[1] = obj->AddVertex( info.matrix.Mult3(MQPoint(scale.x-len, +len, +len)) + info.center );
		vi[2] = obj->AddVertex( info.matrix.Mult3(MQPoint(scale.x+len, -len, +len)) + info.center );
		vi[3] = obj->AddVertex( info.matrix.Mult3(MQPoint(scale.x-len, -len, +len)) + info.center );
		vi[4] = obj->AddVertex( info.matrix.Mult3(MQPoint(scale.x+len, +len, -len)) + info.center );
		vi[5] = obj->AddVertex( info.matrix.Mult3(MQPoint(scale.x-len, +len, -len)) + info.center );
		vi[6] = obj->AddVertex( info.matrix.Mult3(MQPoint(scale.x+len, -len, -len)) + info.center );
		vi[7] = obj->AddVertex( info.matrix.Mult3(MQPoint(scale.x-len, -len, -len)) + info.center );
		
		int nvi[4];
		nvi[0]=vi[1]; nvi[1]=vi[0]; nvi[2]=vi[2]; nvi[3]=vi[3];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxX);

		nvi[0]=vi[4]; nvi[1]=vi[5]; nvi[2]=vi[7]; nvi[3]=vi[6];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxX);

		nvi[0]=vi[0]; nvi[1]=vi[4]; nvi[2]=vi[6]; nvi[3]=vi[2];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxX);

		nvi[0]=vi[3]; nvi[1]=vi[7]; nvi[2]=vi[5]; nvi[3]=vi[1];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxX);

		nvi[0]=vi[0]; nvi[1]=vi[1]; nvi[2]=vi[5]; nvi[3]=vi[4];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxX);

		nvi[0]=vi[6]; nvi[1]=vi[7]; nvi[2]=vi[3]; nvi[3]=vi[2];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxX);
	}

	// Y
	if( info.visible & MQHandleInfo::HandleY )
	{
		int vi[8];
		vi[0] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, scale.y+len, +len)) + info.center );
		vi[1] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, scale.y+len, +len)) + info.center );
		vi[2] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, scale.y-len, +len)) + info.center );
		vi[3] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, scale.y-len, +len)) + info.center );
		vi[4] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, scale.y+len, -len)) + info.center );
		vi[5] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, scale.y+len, -len)) + info.center );
		vi[6] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, scale.y-len, -len)) + info.center );
		vi[7] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, scale.y-len, -len)) + info.center );
		
		int nvi[4];
		nvi[0]=vi[1]; nvi[1]=vi[0]; nvi[2]=vi[2]; nvi[3]=vi[3];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxY);

		nvi[0]=vi[4]; nvi[1]=vi[5]; nvi[2]=vi[7]; nvi[3]=vi[6];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxY);

		nvi[0]=vi[0]; nvi[1]=vi[4]; nvi[2]=vi[6]; nvi[3]=vi[2];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxY);

		nvi[0]=vi[3]; nvi[1]=vi[7]; nvi[2]=vi[5]; nvi[3]=vi[1];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxY);

		nvi[0]=vi[0]; nvi[1]=vi[1]; nvi[2]=vi[5]; nvi[3]=vi[4];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxY);

		nvi[0]=vi[6]; nvi[1]=vi[7]; nvi[2]=vi[3]; nvi[3]=vi[2];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxY);
	}

	// Z
	if( info.visible & MQHandleInfo::HandleZ )
	{
		int vi[8];
		vi[0] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, +len, scale.z+len)) + info.center );
		vi[1] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, +len, scale.z+len)) + info.center );
		vi[2] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, -len, scale.z+len)) + info.center );
		vi[3] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, -len, scale.z+len)) + info.center );
		vi[4] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, +len, scale.z-len)) + info.center );
		vi[5] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, +len, scale.z-len)) + info.center );
		vi[6] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, -len, scale.z-len)) + info.center );
		vi[7] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, -len, scale.z-len)) + info.center );
		
		int nvi[4];
		nvi[0]=vi[1]; nvi[1]=vi[0]; nvi[2]=vi[2]; nvi[3]=vi[3];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxZ);

		nvi[0]=vi[4]; nvi[1]=vi[5]; nvi[2]=vi[7]; nvi[3]=vi[6];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxZ);

		nvi[0]=vi[0]; nvi[1]=vi[4]; nvi[2]=vi[6]; nvi[3]=vi[2];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxZ);

		nvi[0]=vi[3]; nvi[1]=vi[7]; nvi[2]=vi[5]; nvi[3]=vi[1];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxZ);

		nvi[0]=vi[0]; nvi[1]=vi[1]; nvi[2]=vi[5]; nvi[3]=vi[4];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxZ);

		nvi[0]=vi[6]; nvi[1]=vi[7]; nvi[2]=vi[3]; nvi[3]=vi[2];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxZ);
	}

	if (pMatIdx != NULL)
		*pMatIdx = matidx;

	return true;
}

static bool IsRingHandle(MQCommandPlugin *plugin, MQHandleInfo::HandleRotationType rotation)
{
	switch(rotation){
	case MQHandleInfo::RotationDefault:
		{
			std::wstring name = plugin->GetSettingValue(MQSETTINGVALUE_ROTATION_HANDLE);
			return (name == L"ring");
		}
		break;
	case MQHandleInfo::RotationArrow:
		return false;
	case MQHandleInfo::RotationRing:
		return true;
	}
	return false;
}

bool MQHandleObject::CreateRotateHandle(MQCommandPlugin *plugin, bool for_drawing, MQDocument doc, const MQHandleInfo& info, MQHandleMaterial *pMatIdx)
{
	MQHandleMaterial matidx;
	int i,j,vi,fi;
	MQPoint pos;
	float handle_scale = plugin->GetSettingFloatValue(MQSETTINGVALUE_HANDLE_SCALE);
	MQPoint scale = info.scale * handle_scale;
	float len = info.size * handle_scale;
	float handle_size = plugin->GetSettingFloatValue(MQSETTINGVALUE_HANDLE_SIZE);
	Ring = IsRingHandle(plugin, info.rotation);
	bool before_circle = (info.visible & MQHandleInfo::HandleView) && !Ring; // ３方向軸の場合、通常ハンドルより前に
	
	if(!CreateThreeHandle(plugin, for_drawing, doc, info, matidx, before_circle)){
		return false;
	}

	MQObject obj = this->Handle;

#define ROTATE_SEG   8
#define ROTATE_SIZE  1.8f
#define RING_SEG 	 (ROTATE_SEG*4)

	pos = info.center;

	// center
	if( info.visible & MQHandleInfo::HandleW )
	{
		int vi[8];
		vi[0] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, +len, +len)) + pos );
		vi[1] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, +len, +len)) + pos );
		vi[2] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, -len, +len)) + pos );
		vi[3] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, -len, +len)) + pos );
		vi[4] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, +len, -len)) + pos );
		vi[5] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, +len, -len)) + pos );
		vi[6] = obj->AddVertex( info.matrix.Mult3(MQPoint(+len, -len, -len)) + pos );
		vi[7] = obj->AddVertex( info.matrix.Mult3(MQPoint(-len, -len, -len)) + pos );
		
		int nvi[4];
		nvi[0]=vi[1]; nvi[1]=vi[0]; nvi[2]=vi[2]; nvi[3]=vi[3];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxW);

		nvi[0]=vi[4]; nvi[1]=vi[5]; nvi[2]=vi[7]; nvi[3]=vi[6];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxW);

		nvi[0]=vi[0]; nvi[1]=vi[4]; nvi[2]=vi[6]; nvi[3]=vi[2];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxW);

		nvi[0]=vi[3]; nvi[1]=vi[7]; nvi[2]=vi[5]; nvi[3]=vi[1];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxW);

		nvi[0]=vi[0]; nvi[1]=vi[1]; nvi[2]=vi[5]; nvi[3]=vi[4];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxW);

		nvi[0]=vi[6]; nvi[1]=vi[7]; nvi[2]=vi[3]; nvi[3]=vi[2];
		fi = obj->AddFace(4, nvi);
		obj->SetFaceMaterial(fi, matidx.MatIdxW);
	}

	len *= 1.2f;

	// X
	if( info.visible & MQHandleInfo::HandleX )
	{
		vi = obj->GetVertexCount();

		if( !Ring )
		{
			for(i=1; i<ROTATE_SEG; i++) {
				for(j=0; j<ROTATE_SEG; j++)
					obj->AddVertex( info.matrix.Mult3(MQPoint(
							len * sinf((float)i / ROTATE_SEG * PI) * cosf((float)j / ROTATE_SEG * PI * 2) + scale.x,
							len * cosf((float)i / ROTATE_SEG * PI),
							len * sinf((float)i / ROTATE_SEG * PI) * sinf((float)j / ROTATE_SEG * PI * 2))
						) + pos );
			}
			obj->AddVertex( info.matrix.Mult3(MQPoint(scale.x,+len,0)) + pos );
			obj->AddVertex( info.matrix.Mult3(MQPoint(scale.x,-len,0)) + pos );
			for(i=0; i<ROTATE_SEG-2; i++) {
				for(j=0; j<ROTATE_SEG; j++) {
					int nvi[4];
					nvi[0] = vi+(i+1)*ROTATE_SEG+j;
					nvi[1] = vi+(i+1)*ROTATE_SEG+(j+1)%ROTATE_SEG;
					nvi[2] = vi+(i+0)*ROTATE_SEG+(j+1)%ROTATE_SEG;
					nvi[3] = vi+(i+0)*ROTATE_SEG+j;
					fi = obj->AddFace(4, nvi);
					obj->SetFaceMaterial(fi, matidx.MatIdxX);
				}
			}
			for(i=0; i<ROTATE_SEG; i++) {
				int nvi[3];
				nvi[0] = vi+(ROTATE_SEG-1)*ROTATE_SEG;
				nvi[1] = vi+i;
				nvi[2] = vi+(i+1)%ROTATE_SEG;
				fi = obj->AddFace(3, nvi);
				obj->SetFaceMaterial(fi, matidx.MatIdxX);

				nvi[0] = vi+(ROTATE_SEG-1)*ROTATE_SEG+1;
				nvi[1] = vi+(ROTATE_SEG-2)*ROTATE_SEG + (i+1)%ROTATE_SEG;
				nvi[2] = vi+(ROTATE_SEG-2)*ROTATE_SEG + i;
				fi = obj->AddFace(3, nvi);
				obj->SetFaceMaterial(fi, matidx.MatIdxX);
			}
		}
		else
		{
			for(i=0; i<RING_SEG; i++) {
				obj->AddVertex( info.matrix.Mult3(MQPoint(
							len*0.6f*handle_size,
							scale.y * cosf((float)i / RING_SEG * PI * 2.0f),
							scale.z * sinf((float)i / RING_SEG * PI * 2.0f))
						) + pos );
				obj->AddVertex( info.matrix.Mult3(MQPoint(
							-len*0.6f*handle_size,
							scale.y * cosf((float)i / RING_SEG * PI * 2.0f),
							scale.z * sinf((float)i / RING_SEG * PI * 2.0f))
						) + pos );
			}
			for(i=0; i<RING_SEG; i++) {
				int nvi[4];
				nvi[0] = vi+i*2;
				nvi[1] = vi+((i+1)%RING_SEG)*2;
				nvi[2] = vi+((i+1)%RING_SEG)*2+1;
				nvi[3] = vi+i*2+1;
				fi = obj->AddFace(4, nvi);
				obj->SetFaceMaterial(fi, matidx.MatIdxX);
			}
		}
	}

	// Y
	if( info.visible & MQHandleInfo::HandleY )
	{
		vi = obj->GetVertexCount();

		if( !Ring )
		{
			for(i=1; i<ROTATE_SEG; i++) {
				for(j=0; j<ROTATE_SEG; j++)
					obj->AddVertex( info.matrix.Mult3(MQPoint(
							len * sinf((float)i / ROTATE_SEG * PI) * cosf((float)j / ROTATE_SEG * PI * 2),
							len * cosf((float)i / ROTATE_SEG * PI) + scale.y,
							len * sinf((float)i / ROTATE_SEG * PI) * sinf((float)j / ROTATE_SEG * PI * 2))
						) + pos );
			}
			obj->AddVertex( info.matrix.Mult3(MQPoint(0,scale.y+len,0)) + pos );
			obj->AddVertex( info.matrix.Mult3(MQPoint(0,scale.y-len,0)) + pos );
			for(i=0; i<ROTATE_SEG-2; i++) {
				for(j=0; j<ROTATE_SEG; j++) {
					int nvi[4];
					nvi[0] = vi+(i+1)*ROTATE_SEG+j;
					nvi[1] = vi+(i+1)*ROTATE_SEG+(j+1)%ROTATE_SEG;
					nvi[2] = vi+(i+0)*ROTATE_SEG+(j+1)%ROTATE_SEG;
					nvi[3] = vi+(i+0)*ROTATE_SEG+j;
					fi = obj->AddFace(4, nvi);
					obj->SetFaceMaterial(fi, matidx.MatIdxY);
				}
			}
			for(i=0; i<ROTATE_SEG; i++) {
				int nvi[3];
				nvi[0] = vi+(ROTATE_SEG-1)*ROTATE_SEG;
				nvi[1] = vi+i;
				nvi[2] = vi+(i+1)%ROTATE_SEG;
				fi = obj->AddFace(3, nvi);
				obj->SetFaceMaterial(fi, matidx.MatIdxY);
				
				nvi[0] = vi+(ROTATE_SEG-1)*ROTATE_SEG+1;
				nvi[1] = vi+(ROTATE_SEG-2)*ROTATE_SEG + (i+1)%ROTATE_SEG;
				nvi[2] = vi+(ROTATE_SEG-2)*ROTATE_SEG + i;
				fi = obj->AddFace(3, nvi);
				obj->SetFaceMaterial(fi, matidx.MatIdxY);
			}
		}
		else
		{
			for(i=0; i<RING_SEG; i++) {
				obj->AddVertex( info.matrix.Mult3(MQPoint(
							scale.z * sinf((float)i / RING_SEG * PI * 2.0f),
							len*0.6f*handle_size,
							scale.y * cosf((float)i / RING_SEG * PI * 2.0f))
						) + pos );
				obj->AddVertex( info.matrix.Mult3(MQPoint(
							scale.z * sinf((float)i / RING_SEG * PI * 2.0f),
							-len*0.6f*handle_size,
							scale.y * cosf((float)i / RING_SEG * PI * 2.0f))
						) + pos );
			}
			for(i=0; i<RING_SEG; i++) {
				int nvi[4];
				nvi[0] = vi+i*2;
				nvi[1] = vi+((i+1)%RING_SEG)*2;
				nvi[2] = vi+((i+1)%RING_SEG)*2+1;
				nvi[3] = vi+i*2+1;
				fi = obj->AddFace(4, nvi);
				obj->SetFaceMaterial(fi, matidx.MatIdxY);
			}
		}
	}

	// Z
	if( info.visible & MQHandleInfo::HandleZ )
	{
		vi = obj->GetVertexCount();

		if( !Ring )
		{
			for(i=1; i<ROTATE_SEG; i++) {
				for(j=0; j<ROTATE_SEG; j++)
					obj->AddVertex( info.matrix.Mult3(MQPoint(
							len * sinf((float)i / ROTATE_SEG * PI) * cosf((float)j / ROTATE_SEG * PI * 2),
							len * cosf((float)i / ROTATE_SEG * PI),
							len * sinf((float)i / ROTATE_SEG * PI) * sinf((float)j / ROTATE_SEG * PI * 2) + scale.z)
						) + pos );
			}
			obj->AddVertex( info.matrix.Mult3(MQPoint(0,+len,scale.z)) + pos );
			obj->AddVertex( info.matrix.Mult3(MQPoint(0,-len,scale.z)) + pos );
			for(i=0; i<ROTATE_SEG-2; i++) {
				for(j=0; j<ROTATE_SEG; j++) {
					int nvi[4];
					nvi[0] = vi+(i+1)*ROTATE_SEG+j;
					nvi[1] = vi+(i+1)*ROTATE_SEG+(j+1)%ROTATE_SEG;
					nvi[2] = vi+(i+0)*ROTATE_SEG+(j+1)%ROTATE_SEG;
					nvi[3] = vi+(i+0)*ROTATE_SEG+j;
					fi = obj->AddFace(4, nvi);
					obj->SetFaceMaterial(fi, matidx.MatIdxZ);
				}
			}
			for(i=0; i<ROTATE_SEG; i++) {
				int nvi[3];

				nvi[0] = vi+(ROTATE_SEG-1)*ROTATE_SEG;
				nvi[1] = vi+i;
				nvi[2] = vi+(i+1)%ROTATE_SEG;
				fi = obj->AddFace(3, nvi);
				obj->SetFaceMaterial(fi, matidx.MatIdxZ);

				nvi[0] = vi+(ROTATE_SEG-1)*ROTATE_SEG+1;
				nvi[1] = vi+(ROTATE_SEG-2)*ROTATE_SEG + (i+1)%ROTATE_SEG;
				nvi[2] = vi+(ROTATE_SEG-2)*ROTATE_SEG + i;
				fi = obj->AddFace(3, nvi);
				obj->SetFaceMaterial(fi, matidx.MatIdxZ);
			}
		}
		else
		{
			for(i=0; i<RING_SEG; i++) {
				obj->AddVertex( info.matrix.Mult3(MQPoint(
							scale.y * cosf((float)i / RING_SEG * PI * 2.0f),
							scale.z * sinf((float)i / RING_SEG * PI * 2.0f),
							len*0.6f*handle_size)
						) + pos );
				obj->AddVertex( info.matrix.Mult3(MQPoint(
							scale.y * cosf((float)i / RING_SEG * PI * 2.0f),
							scale.z * sinf((float)i / RING_SEG * PI * 2.0f),
							-len*0.6f*handle_size)
						) + pos );
			}
			for(i=0; i<RING_SEG; i++) {
				int fvi[4];
				fvi[0] = vi+i*2;
				fvi[1] = vi+((i+1)%RING_SEG)*2;
				fvi[2] = vi+((i+1)%RING_SEG)*2+1;
				fvi[3] = vi+i*2+1;
				fi = obj->AddFace(4, fvi);
				obj->SetFaceMaterial(fi, matidx.MatIdxZ);
			}
		}
	}

	// スクリーンのZ軸方向の回転
	if( info.visible & MQHandleInfo::HandleView )
	{
		// 円回転用オブジェクトの作成
		MQObject obj;
		if (before_circle){
			obj = this->RotationCircle;
		}else{
			if(for_drawing){
				this->RotationCircle = plugin->CreateDrawingObject(doc, MQCommandPlugin::DRAW_OBJECT_LINE);
			}else{
				this->RotationCircle = MQ_CreateObject();
			}
			obj = this->RotationCircle;
		}
		obj->AddRenderFlag(MQOBJECT_RENDER_LINE);
		obj->AddRenderFlag(MQOBJECT_RENDER_OVERWRITELINE);
		obj->AddRenderEraseFlag(MQOBJECT_RENDER_MULTILIGHT);
		obj->AddRenderEraseFlag(MQOBJECT_RENDER_SHADOW);
		obj->SetColorValid(TRUE);
		if(info.active & MQHandleInfo::HandleView){
			obj->SetColor(plugin->GetSystemColor(MQSYSTEMCOLOR_AXIS_W));
		}else{
			obj->SetColor(plugin->GetSystemColor(MQSYSTEMCOLOR_AXIS_ACTIVE));
		}

		int vi = 0;

		MQPoint axisX, axisY;
		if (fabs(GetInnerProduct(info.camdir, MQPoint(0,0,1))) < fabs(GetInnerProduct(info.camdir, MQPoint(0,1,0)))){
			axisX = GetCrossProduct(info.camdir, MQPoint(0,0,1));
			axisY = GetCrossProduct(axisX, info.camdir);
		}else{
			axisX = GetCrossProduct(info.camdir, MQPoint(0,1,0));
			axisY = GetCrossProduct(axisX, info.camdir);
		}
		for(i=0; i<RING_SEG; i++) {
			float rad = (float)i / RING_SEG * PI * 2.0f;
			MQPoint dir = cosf(rad) * axisX - sinf(rad) * axisY;
			obj->AddVertex( scale * dir + pos );
		}
		for(i=0; i<RING_SEG; i++) {
			int fvi[2];
			fvi[0] = i;
			fvi[1] = (i+1)%RING_SEG;
			fi = obj->AddFace(2, fvi);
			obj->SetFaceMaterial(fi, matidx.MatIdxView);
		}
	}

	if (pMatIdx != NULL)
		*pMatIdx = matidx;

	return true;
}


MQHandleInfo::AxisType MQHandleObject::HitTestMoveHandle(MQCommandPlugin *plugin, MQDocument doc, MQScene scene, MQCommandPlugin::MOUSE_BUTTON_STATE& state, const MQHandleInfo& info, MQPoint& hit_pos)
{
	MQHandleMaterial handle_mat;
	MQHandleObject handle;
	handle.CreateMoveHandle(plugin, false, doc, info, &handle_mat);

	std::vector<MQObject> objects;
	objects.push_back(handle.Handle);

	MQCommandPlugin::HIT_TEST_PARAM param;
	param.TestVertex = false;
	param.TestLine = false;
	param.TestFace = true;
	if(plugin->HitTestObjects(scene, state.MousePos, objects, param)){
		hit_pos = param.HitPos;
		int mat = handle.Handle->GetFaceMaterial(param.FaceIndex);
		if(mat == handle_mat.MatIdxX){
			return MQHandleInfo::HandleX;
		}else if(mat == handle_mat.MatIdxY){
			return MQHandleInfo::HandleY;
		}else if(mat == handle_mat.MatIdxZ){
			return MQHandleInfo::HandleZ;
		}else if(mat == handle_mat.MatIdxW){
			return MQHandleInfo::HandleW;
		}
	}

	return MQHandleInfo::HandleNone;
}

MQHandleInfo::AxisType MQHandleObject::HitTestScaleHandle(MQCommandPlugin *plugin, MQDocument doc, MQScene scene, MQCommandPlugin::MOUSE_BUTTON_STATE& state, const MQHandleInfo& info, MQPoint& hit_pos)
{
	MQHandleMaterial handle_mat;
	MQHandleObject handle;
	handle.CreateScaleHandle(plugin, false, doc, info, &handle_mat);

	std::vector<MQObject> objects;
	objects.push_back(handle.Handle);

	MQCommandPlugin::HIT_TEST_PARAM param;
	param.TestVertex = false;
	param.TestLine = false;
	param.TestFace = true;
	if(plugin->HitTestObjects(scene, state.MousePos, objects, param)){
		hit_pos = param.HitPos;
		int mat = handle.Handle->GetFaceMaterial(param.FaceIndex);
		if(mat == handle_mat.MatIdxX){
			return MQHandleInfo::HandleX;
		}else if(mat == handle_mat.MatIdxY){
			return MQHandleInfo::HandleY;
		}else if(mat == handle_mat.MatIdxZ){
			return MQHandleInfo::HandleZ;
		}else if(mat == handle_mat.MatIdxW){
			return MQHandleInfo::HandleW;
		}
	}

	return MQHandleInfo::HandleNone;
}

MQHandleInfo::AxisType MQHandleObject::HitTestRotateHandle(MQCommandPlugin *plugin, MQDocument doc, MQScene scene, MQCommandPlugin::MOUSE_BUTTON_STATE& state, const MQHandleInfo& info, MQPoint& hit_pos)
{
	MQHandleMaterial handle_mat;
	MQHandleObject handle;
	handle.CreateRotateHandle(plugin, false, doc, info, &handle_mat);

	// View direction ring (on)
	if(handle.RotationCircle != NULL && handle.Ring){
		int num = handle.RotationCircle->GetVertexCount();
		std::vector<MQPoint> pts(num);
		for(int i=0; i<num; i++){
			pts[i] = scene->Convert3DToScreen(handle.RotationCircle->GetVertex(i));
		}
		MQPoint mouse_pos((float)state.MousePos.x, (float)state.MousePos.y, 0);
		for(int i=0; i<num; i++){
			float dis = Get2DLineDistance(mouse_pos, pts[(i+1)%num], pts[i], NULL, NULL);
			if(dis <= 1)
				return MQHandleInfo::HandleView;
		}
	}

	// X/Y/Z rings or center box
	std::vector<MQObject> objects;
	objects.push_back(handle.Handle);

	MQCommandPlugin::HIT_TEST_PARAM param;
	param.TestVertex = false;
	param.TestLine = false;
	param.TestFace = true;
	if(plugin->HitTestObjects(scene, state.MousePos, objects, param)){
		hit_pos = param.HitPos;
		int mat = handle.Handle->GetFaceMaterial(param.FaceIndex);
		if(mat == handle_mat.MatIdxX){
			return MQHandleInfo::HandleX;
		}else if(mat == handle_mat.MatIdxY){
			return MQHandleInfo::HandleY;
		}else if(mat == handle_mat.MatIdxZ){
			return MQHandleInfo::HandleZ;
		}else if(mat == handle_mat.MatIdxW){
			return MQHandleInfo::HandleW;
		}
	}

	// View direction ring (near)
	if(handle.RotationCircle != NULL){
		objects.clear();
		objects.push_back(handle.RotationCircle);

		MQCommandPlugin::HIT_TEST_PARAM param;
		param.TestVertex = false;
		param.TestLine = true;
		param.TestFace = false;
		if(plugin->HitTestObjects(scene, state.MousePos, objects, param)){
			hit_pos = param.HitPos;
			return MQHandleInfo::HandleView;
		}
	}

	return MQHandleInfo::HandleNone;
}



MQHandleOperation::MQHandleOperation()
{
}

void MQHandleOperation::Begin(MQCommandPlugin *plugin, const MQHandleInfo& info, MQHandleInfo::AxisType axis_type, POINT mousePos, MQPoint hitPos)
{
	Plugin = plugin;
	RotationType = info.rotation;
	Center = info.center;
	Matrix = info.matrix;
	AxisType = axis_type;
	DownPos = mousePos;
	PrevPos = mousePos;
	HitPos = hitPos;
	TotalAngle = 0;
}

bool MQHandleOperation::Move(MQPoint& offset, MQScene scene, POINT mousePos)
{
	bool result = false;
	switch(AxisType) {
	case MQHandleInfo::HandleW:
		{
			MQPoint vec;
			float z = scene->Convert3DToScreen(HitPos).z;
			vec = scene->ConvertScreenTo3D(MQPoint((float)mousePos.x, (float)mousePos.y, z))
				- scene->ConvertScreenTo3D(MQPoint((float)PrevPos.x, (float)PrevPos.y, z));
			offset += vec;
		}
		result = true;
		break;

	case MQHandleInfo::HandleX:
	case MQHandleInfo::HandleY:
	case MQHandleInfo::HandleZ:
		{
			MQPoint nrm(0,0,0), vec;

			switch(AxisType) {
			case MQHandleInfo::HandleX: nrm = MQPoint(1,0,0); break;
			case MQHandleInfo::HandleY: nrm = MQPoint(0,1,0); break;
			case MQHandleInfo::HandleZ: nrm = MQPoint(0,0,1); break;
			}
			nrm = Matrix.Mult3(nrm);

			float z = scene->Convert3DToScreen(HitPos).z;
			vec = scene->ConvertScreenTo3D(MQPoint((float)mousePos.x, (float)mousePos.y, z))
				- scene->ConvertScreenTo3D(MQPoint((float)PrevPos.x, (float)PrevPos.y, z));
			offset += nrm * GetInnerProduct(vec, nrm);

		}
		result = true;
		break;
	}

	PrevPos = mousePos;
	return result;
}

bool MQHandleOperation::Scale(MQPoint& scaling, MQScene scene, POINT mousePos)
{
	float t;
	if(abs(mousePos.x - PrevPos.x) > abs(PrevPos.y - mousePos.y)){
		t = (float)pow(1.005, (double)(mousePos.x - PrevPos.x));
	}else{
		t = (float)pow(1.005, (double)(PrevPos.y - mousePos.y));
	}

	PrevPos = mousePos;

	switch(AxisType) {
	case MQHandleInfo::HandleW: scaling   *= t; break;
	case MQHandleInfo::HandleX: scaling.x *= t; break;
	case MQHandleInfo::HandleY: scaling.y *= t; break;
	case MQHandleInfo::HandleZ: scaling.z *= t; break;
	default: return false;
	}

	return true;
}

bool MQHandleOperation::Rotate(MQMatrix& mtx, MQScene scene, POINT mousePos, bool snapping)
{
	float offset = RAD((float)(PrevPos.x-mousePos.x) * 0.5f);
	bool result = false;
	switch(AxisType){
	case MQHandleInfo::HandleW:
		{
			MQPoint p((float)(mousePos.y-PrevPos.y), -(float)(mousePos.x-PrevPos.x), 0);
			MQPoint cp = scene->Convert3DToScreen(Center);
			MQPoint dp = scene->ConvertScreenTo3D(cp + p);
			MQPoint axis = Normalize(dp - Center);
			Matrix = Matrix * GetRotationMatrixWithAxis(axis, p.abs()*0.01f);
		}
		result = true;
		break;
	case MQHandleInfo::HandleX:
	case MQHandleInfo::HandleY:
	case MQHandleInfo::HandleZ:
		{
			MQPoint dir;
			switch(AxisType) {
			case MQHandleInfo::HandleX: dir = MQPoint(1,0,0); break;
			case MQHandleInfo::HandleY: dir = MQPoint(0,1,0); break;
			case MQHandleInfo::HandleZ: dir = MQPoint(0,0,1); break;
			}
			if(IsRingHandle(Plugin, RotationType)){
				MQPoint axisW = Matrix.Mult3(dir);
				MQPoint axisU = Normalize(HitPos - Center);
				MQPoint axisV = GetCrossProduct(axisW, axisU);

				MQPoint vec(0,0,0);
				MQPoint cp = scene->Convert3DToScreen(HitPos);
				MQPoint dp = scene->Convert3DToScreen(HitPos + axisV * ((HitPos - Center).abs() * 0.001f));
				if(dp != MQPoint(0,0,0)){
					vec.x = dp.x - cp.x;
					vec.y = dp.y - cp.y;
					vec = Normalize(vec);
				}else{
					dp = scene->Convert3DToScreen(HitPos - axisV * ((HitPos - Center).abs() * 0.001f));
					if(dp != MQPoint(0,0,0)){
						vec.x = cp.x - dp.x;
						vec.y = cp.y - dp.y;
						vec = Normalize(vec);
					}
				}

				if(snapping){
					MQPoint diff((float)mousePos.x-DownPos.x, (float)mousePos.y-DownPos.y, 0);
					float offset = GetInnerProduct(diff, vec);
					float snapangle = RAD(floor(offset / 15) * 15);
					if(snapangle != TotalAngle){
						Matrix = GetRotationMatrixWithAxis(dir, snapangle-TotalAngle) * Matrix;
						TotalAngle = snapangle;
					}
				}else{
					MQPoint diff((float)mousePos.x-PrevPos.x, (float)mousePos.y-PrevPos.y, 0);
					float offset = RAD(GetInnerProduct(diff, vec) * 0.5f);
					Matrix = GetRotationMatrixWithAxis(dir, offset) * Matrix;
					TotalAngle += offset;
				}
			}else{
				if(snapping){
					float snapangle = RAD((float)(((DownPos.x-mousePos.x) / 15) * 15));
					if(snapangle != TotalAngle){
						Matrix = GetRotationMatrixWithAxis(dir, snapangle-TotalAngle) * Matrix;
						TotalAngle = snapangle;
					}
				}else{
					Matrix = GetRotationMatrixWithAxis(dir, offset) * Matrix;
					TotalAngle += offset;
				}
			}
			result = true;
		}
		break;
	case MQHandleInfo::HandleView:
		{
			MQPoint cp = scene->Convert3DToScreen(Center);
			float r1 = (float)atan2((float)mousePos.y-cp.y, (float)mousePos.x-cp.x);
			float r2 = (float)atan2((float)PrevPos.y-cp.y, (float)PrevPos.x-cp.x);
			float r3 = (float)atan2((float)DownPos.y-cp.y, (float)DownPos.x-cp.x);
			float offset = fmod(r1 - r2 + 3*PI, 2*PI) - PI;

			MQPoint campos = scene->GetCameraPosition();
			MQPoint lookat = scene->GetLookAtPosition();
			MQPoint camdir = Normalize(lookat - campos);
			Matrix = Matrix * GetRotationMatrixWithAxis(camdir, offset);
			TotalAngle += offset;
		}
		result = true;
		break;
	}

	mtx = Matrix;
	PrevPos = mousePos;

	return result;
}

