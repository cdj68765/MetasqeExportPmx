//---------------------------------------------------------------------------
//
//   MQBasePlugin
//
//          Copyright(C) 1999-2016, tetraface Inc.
//
//     This is a wrapper for using C++ classes without taking care of 
//    Metasequoia plug-in interface API.
//     The API functions for each type of a plug-in are defined as the 
//    pure virtual functions, and it is necessary to implement them 
//    in an inherited class.
//
//    　Metasequoia用プラグインインターフェースをAPIレベルを隠蔽して
//    C++のクラスとして扱うためのラッパーです。
//      各プラグインの種類ごとに必須のAPIは純粋仮想関数として定義され
//    ているため、これらの関数は継承クラス側で実装を行う必要があります。
//
//---------------------------------------------------------------------------

#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include "MQPlugin.h"
#include "MQBasePlugin.h"
#include "MQSetting.h"


static void *ExtractEventOption(void *option, const char *name);
static BOOL ExtractEventOptionBool(void *option, const char *name, BOOL defval);
static int ExtractEventOptionInt(void *option, const char *name, int defval);
static float ExtractEventOptionFloat(void *option, const char *name, float defval);


class MQBasePluginMediator {
public:
	static void ImportSetOptions(MQImportPlugin *plugin, BOOL background, void *args);
};

 
//---------------------------------------------------------------------------
//  MQGetPlugInID
//    プラグインIDを返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT void MQGetPlugInID(DWORD *Product, DWORD *ID)
{
	// プロダクト名(制作者名)とIDを、全部で64bitの値として返す
	// 値は他と重複しないようなランダムなもので良い
	GetPluginClass()->GetPlugInID(Product, ID);
}

//---------------------------------------------------------------------------
//  MQGetPlugInName
//    プラグイン名を返す。
//    この関数は[プラグインについて]表示時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT const char *MQGetPlugInName(void)
{
	// プラグイン名	
	return GetPluginClass()->GetPlugInName();
}

//---------------------------------------------------------------------------
//  MQGetPlugInType
//    プラグインのタイプを返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT int MQGetPlugInType(void)
{
	return GetPluginClass()->GetPlugInType();
}

//---------------------------------------------------------------------------
//  MQEnumString
//    ポップアップメニューまたはボタンに表示される文字列を返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT const char *MQEnumString(int index)
{
	MQBasePlugin *plugin = GetPluginClass();
	switch(plugin->GetPlugInType()){
	case MQPLUGIN_TYPE_OBJECT:
	case MQPLUGIN_TYPE_SELECT:
	case MQPLUGIN_TYPE_CREATE:
		return static_cast<MQMenuBasePlugin*>(plugin)->EnumString(index);
	case MQPLUGIN_TYPE_STATION:
	case MQPLUGIN_TYPE_COMMAND:
		if (index == 0)
			return static_cast<MQStationPlugin*>(plugin)->EnumString();
		break;
	}
	return NULL;
}

//---------------------------------------------------------------------------
//  MQEnumFileType
//    入力または出力可能なファイルタイプを返す。
//    ファイルタイプは別名保存時のダイアログに表示される。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT const char *MQEnumFileType(int index)
{
	MQBasePlugin *plugin = GetPluginClass();
	switch(plugin->GetPlugInType()){
	case MQPLUGIN_TYPE_IMPORT:
		return static_cast<MQImportPlugin*>(plugin)->EnumFileType(index);
	case MQPLUGIN_TYPE_EXPORT:
		return static_cast<MQExportPlugin*>(plugin)->EnumFileType(index);
	}
	return NULL;
}

//---------------------------------------------------------------------------
//  MQEnumFileExt
//    入力または出力可能な拡張子を返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT const char *MQEnumFileExt(int index)
{
	MQBasePlugin *plugin = GetPluginClass();
	switch(plugin->GetPlugInType()){
	case MQPLUGIN_TYPE_IMPORT:
		return static_cast<MQImportPlugin*>(plugin)->EnumFileExt(index);
	case MQPLUGIN_TYPE_EXPORT:
		return static_cast<MQExportPlugin*>(plugin)->EnumFileExt(index);
	}
	return NULL;
}

//---------------------------------------------------------------------------
//  MQImportFile
//    [読み込み]やドラッグ＆ドロップで読み込むときに呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT BOOL MQImportFile(int index, const char *filename, MQDocument doc)
{
	return static_cast<MQImportPlugin*>(GetPluginClass())->ImportFile(index, filename, doc);
}

//---------------------------------------------------------------------------
//  MQExportFile
//    [別名で保存]で保存するときに呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT BOOL MQExportFile(int index, const char *filename, MQDocument doc)
{
	return static_cast<MQExportPlugin*>(GetPluginClass())->ExportFile(index, filename, doc);
}

//---------------------------------------------------------------------------
//  MQCreate
//    メニューから選択されたときに呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT BOOL MQCreate(int index, MQDocument doc)
{
	return static_cast<MQCreatePlugin*>(GetPluginClass())->Execute(index, doc);
}

//---------------------------------------------------------------------------
//  MQModifySelect
//    メニューから選択されたときに呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT BOOL MQModifySelect(int index, MQDocument doc)
{
	return static_cast<MQSelectPlugin*>(GetPluginClass())->Execute(index, doc);
}

//---------------------------------------------------------------------------
//  MQModifyObject
//    メニューから選択されたときに呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT BOOL MQModifyObject(int index, MQDocument doc)
{
	return static_cast<MQObjectPlugin*>(GetPluginClass())->Execute(index, doc);
}

//---------------------------------------------------------------------------
//  MQOnEvent
//    イベント発生時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT BOOL MQOnEvent(MQDocument doc, int event_type, void *option)
{
	MQBasePlugin *base_plugin = static_cast<MQBasePlugin*>(GetPluginClass());

	MQStationPlugin *plugin = dynamic_cast<MQStationPlugin*>(base_plugin);
	if(plugin != NULL)
	{
		switch(event_type){
		case MQEVENT_INITIALIZE:	// プラグインの初期化
			return plugin->Initialize();

		case MQEVENT_EXIT:			// プラグインの終了
			plugin->Exit();
			return TRUE;

		case MQEVENT_ENUM_SUBCOMMAND:	// サブコマンドの列挙
			{
				int index = ExtractEventOptionInt(option, "index", 0);
				const char **result = (const char**)ExtractEventOption(option, "result");
				*result = plugin->EnumSubCommand(index);
				return TRUE;
			}

		case MQEVENT_SUBCOMMAND_STRING:	// サブコマンドの表示名
			{
				int index = ExtractEventOptionInt(option, "index", 0);
				const wchar_t **result = (const wchar_t**)ExtractEventOption(option, "result");
				*result = plugin->GetSubCommandString(index);
				return TRUE;
			}

		case MQEVENT_ACTIVATE:		// 表示・非表示切り替え要求
			{
				BOOL flag = *(BOOL *)option;
				return plugin->Activate(doc, flag);
			}

		case MQEVENT_IS_ACTIVATED:	// 表示・非表示状態の返答
			return plugin->IsActivated(doc);

		case MQEVENT_MINIMIZE:		// ウインドウの最小化
			{
				BOOL flag = *(BOOL *)option;
				plugin->OnMinimize(doc, flag);
				return TRUE;
			}

		case MQEVENT_SUBCOMMAND:	// サブコマンドの呼び出し
			{
				int index = ExtractEventOptionInt(option, "index", 0);
				return plugin->OnSubCommand(doc, index);
			}

		case MQEVENT_USER_MESSAGE:	// プラグイン独自のメッセージ
			{
				DWORD product = (DWORD)ExtractEventOptionInt(option, "src_product", 0);
				DWORD id = (DWORD)ExtractEventOptionInt(option, "src_id", 0);
				const char *description = (const char *)ExtractEventOption(option, "description");
				void *message = ExtractEventOption(option, "message");
				int *result = (int*)ExtractEventOption(option, "result");
				int ret = plugin->OnReceiveUserMessage(doc, product, id, description, message);
				if (result != NULL) *result = ret;
				return TRUE;
			}

		case MQEVENT_DRAW:	// 描画時の処理
			{
				MQScene scene = (MQScene)ExtractEventOption(option, "scene");
				int width = ExtractEventOptionInt(option, "width", 1);
				int height = ExtractEventOptionInt(option, "height", 1);
				plugin->OnDraw(doc, scene, width, height);
				return TRUE;
			}

		case MQEVENT_LBUTTON_DOWN:	// 左ボタンが押されたとき
		case MQEVENT_LBUTTON_MOVE:	// 左ボタンが押されながらマウスが移動したとき
		case MQEVENT_LBUTTON_UP:	// 左ボタンが離されたとき
		case MQEVENT_MBUTTON_DOWN:	// 中ボタンが押されたとき
		case MQEVENT_MBUTTON_MOVE:	// 中ボタンが押されながらマウスが移動したとき
		case MQEVENT_MBUTTON_UP:	// 中ボタンが離されたとき
		case MQEVENT_RBUTTON_DOWN:	// 右ボタンが押されたとき
		case MQEVENT_RBUTTON_MOVE:	// 右ボタンが押されながらマウスが移動したとき
		case MQEVENT_RBUTTON_UP:	// 右ボタンが離されたとき
		case MQEVENT_MOUSE_MOVE:	// マウスが移動したとき
		case MQEVENT_MOUSE_WHEEL:	// マウスのホイールが回転したとき
			{
				MQCommandPlugin *com_plugin = dynamic_cast<MQCommandPlugin*>(plugin);
				if (com_plugin == NULL)
					return FALSE;

				MQScene scene = (MQScene)ExtractEventOption(option, "scene");
				DWORD button_state = (DWORD)ExtractEventOptionInt(option, "button_state", 0);
				MQCommandPlugin::MOUSE_BUTTON_STATE button;
				button.MousePos.x = ExtractEventOptionInt(option, "mouse_pos_x", 0);
				button.MousePos.y = ExtractEventOptionInt(option, "mouse_pos_y", 0);
				button.Wheel = ExtractEventOptionInt(option, "mouse_wheel", 0);
				button.Pressure = ExtractEventOptionFloat(option, "pressure", 0.0f);
				button.LButton = (button_state & MK_LBUTTON) ? TRUE : FALSE;
				button.MButton = (button_state & MK_MBUTTON) ? TRUE : FALSE;
				button.RButton = (button_state & MK_RBUTTON) ? TRUE : FALSE;
				button.Shift = (button_state & MK_SHIFT) ? TRUE : FALSE;
				button.Ctrl = (button_state & MK_CONTROL) ? TRUE : FALSE;
				button.Alt = (button_state & MK_ALT) ? TRUE : FALSE;
				switch(event_type){
				case MQEVENT_LBUTTON_DOWN:	// 左ボタンが押されたとき
					return com_plugin->OnLeftButtonDown(doc, scene, button);
				case MQEVENT_LBUTTON_MOVE:	// 左ボタンが押されながらマウスが移動したとき
					return com_plugin->OnLeftButtonMove(doc, scene, button);
				case MQEVENT_LBUTTON_UP:	// 左ボタンが離されたとき
					return com_plugin->OnLeftButtonUp(doc, scene, button);
				case MQEVENT_MBUTTON_DOWN:	// 中ボタンが押されたとき
					return com_plugin->OnMiddleButtonDown(doc, scene, button);
				case MQEVENT_MBUTTON_MOVE:	// 中ボタンが押されながらマウスが移動したとき
					return com_plugin->OnMiddleButtonMove(doc, scene, button);
				case MQEVENT_MBUTTON_UP:	// 中ボタンが離されたとき
					return com_plugin->OnMiddleButtonUp(doc, scene, button);
				case MQEVENT_RBUTTON_DOWN:	// 右ボタンが押されたとき
					return com_plugin->OnRightButtonDown(doc, scene, button);
				case MQEVENT_RBUTTON_MOVE:	// 右ボタンが押されながらマウスが移動したとき
					return com_plugin->OnRightButtonMove(doc, scene, button);
				case MQEVENT_RBUTTON_UP:	// 右ボタンが離されたとき
					return com_plugin->OnRightButtonUp(doc, scene, button);
				case MQEVENT_MOUSE_MOVE:	// マウスが移動したとき
					return com_plugin->OnMouseMove(doc, scene, button);
				case MQEVENT_MOUSE_WHEEL:	// マウスのホイールが回転したとき
					return com_plugin->OnMouseWheel(doc, scene, button);
				}
				return FALSE;
			}

		case MQEVENT_KEY_DOWN:		// キーが押されたとき
		case MQEVENT_KEY_UP:		// キーが離されたとき
			{
				MQCommandPlugin *com_plugin = dynamic_cast<MQCommandPlugin*>(plugin);
				if (com_plugin == NULL)
					return FALSE;

				MQScene scene = (MQScene)ExtractEventOption(option, "scene");
				int key = ExtractEventOptionInt(option, "key", 0);
				DWORD button_state = (DWORD)ExtractEventOptionInt(option, "button_state", 0);
				MQCommandPlugin::MOUSE_BUTTON_STATE button;
				button.MousePos.x = 0;
				button.MousePos.y = 0;
				button.Wheel = 0;
				button.LButton = (button_state & MK_LBUTTON) ? TRUE : FALSE;
				button.MButton = (button_state & MK_MBUTTON) ? TRUE : FALSE;
				button.RButton = (button_state & MK_RBUTTON) ? TRUE : FALSE;
				button.Shift = (button_state & MK_SHIFT) ? TRUE : FALSE;
				button.Ctrl = (button_state & MK_CONTROL) ? TRUE : FALSE;
				button.Alt = (button_state & MK_ALT) ? TRUE : FALSE;
				switch(event_type){
				case MQEVENT_KEY_DOWN:		// キーが押されたとき
					return com_plugin->OnKeyDown(doc, scene, key, button);
				case MQEVENT_KEY_UP:		// キーが離されたとき
					return com_plugin->OnKeyUp(doc, scene, key, button);
				}
				return FALSE;
			}

		case MQEVENT_NEW_DOCUMENT:	// ドキュメント初期化時
			{
				MQStationPlugin::NEW_DOCUMENT_PARAM param;
				//param.root = (MQXmlElement)ExtractEventOption(option, "root_elem"); // ※APIとして提供しないが本当は利用可
				param.elem = (MQXmlElement)ExtractEventOption(option, "xml_elem");
				plugin->OnNewDocument(doc, (const char *)ExtractEventOption(option, "filename"), param);
			}
			return TRUE;

		case MQEVENT_END_DOCUMENT:	// ドキュメント終了時
			plugin->OnEndDocument(doc);
			return TRUE;

		case MQEVENT_SAVE_DOCUMENT:	// ドキュメント保存時
			{
				MQStationPlugin::SAVE_DOCUMENT_PARAM param;
				//param.root = (MQXmlElement)ExtractEventOption(option, "root_elem"); // ※APIとして提供しないが本当は利用可
				param.elem = (MQXmlElement)ExtractEventOption(option, "xml_elem");
				param.bSaveUniqueID = FALSE;
				plugin->OnSaveDocument(doc, (const char *)ExtractEventOption(option, "filename"), param);
				if (param.bSaveUniqueID){
					BOOL *save_uid = (BOOL*)ExtractEventOption(option, "save_uid");
					if (save_uid != NULL)
						*save_uid = TRUE;
				}
			}
			return TRUE;

		case MQEVENT_UNDO:	// アンドゥ実行時
			return plugin->OnUndo(doc, ExtractEventOptionInt(option, "state", 0));

		case MQEVENT_REDO:	// リドゥ実行時
			return plugin->OnRedo(doc, ExtractEventOptionInt(option, "state", 0));

		case MQEVENT_UNDO_UPDATED:	// アンドゥ状態更新時
			plugin->OnUpdateUndo(doc, ExtractEventOptionInt(option, "state", 0), ExtractEventOptionInt(option, "size", 0));
			return TRUE;

		case MQEVENT_OBJECT_MODIFIED:	// オブジェクトの編集時
			plugin->OnObjectModified(doc);
			return TRUE;

		case MQEVENT_OBJECT_SELECTED:	// オブジェクトの選択状態の変更時
			plugin->OnObjectSelected(doc);
			return TRUE;

		case MQEVENT_OBJECT_LIST:		// カレントオブジェクトの変更時
			plugin->OnUpdateObjectList(doc);
			return TRUE;

		case MQEVENT_MATERIAL_MODIFIED:	// マテリアルのパラメータ変更時
			plugin->OnMaterialModified(doc);
			return TRUE;

		case MQEVENT_MATERIAL_LIST:		// カレントマテリアルの変更時
			plugin->OnUpdateMaterialList(doc);
			return TRUE;

		case MQEVENT_SCENE:	// シーン情報の変更時
			{
				MQScene scene = (MQScene)ExtractEventOption(option, "scene");
				plugin->OnUpdateScene(doc, scene);
				return TRUE;
			}

		case MQEVENT_EDIT_OPTION:	// シーン情報の変更時
			{
				const char *trigger = (char*)ExtractEventOption(option, "trigger");
				MQStationPlugin::EDITOPTION_TYPE type = MQStationPlugin::EDITOPTION_UNKNOWN;
				if(strcmp(trigger, "screen") == 0) type = MQStationPlugin::EDITOPTION_SCREEN;
				else if(strcmp(trigger, "world") == 0) type = MQStationPlugin::EDITOPTION_WORLD;
				else if(strcmp(trigger, "local") == 0) type = MQStationPlugin::EDITOPTION_LOCAL;
				plugin->OnChangeEditOption(doc, type);
				return TRUE;
			}

		default:
			break;
		}
	}

	MQImportPlugin *import_plugin = dynamic_cast<MQImportPlugin*>(base_plugin);
	if(import_plugin != NULL)
	{
		switch(event_type){
		case MQEVENT_IMPORT_SUPPORT_BACKGROUND:
			return import_plugin->SupportBackground();
		case MQEVENT_IMPORT_SET_OPTIONS:
			{
				BOOL background = ExtractEventOptionBool(option, "background", FALSE);
				void *args = ExtractEventOption(option, "args");

				MQBasePluginMediator::ImportSetOptions(import_plugin, background, args);
				return TRUE;
			}
		}
	}

	return FALSE;
}


void *ExtractEventOption(void *option, const char *name)
{
	if (option != NULL){
		void **array = (void**)option;
		for (int i=0; array[i]!=NULL; i+=2){
			if (strcmp((char*)array[i], name) == 0)
				return array[i+1];
		}
	}
	return NULL;
}

BOOL ExtractEventOptionBool(void *option, const char *name, BOOL defval)
{
	BOOL *ptr = (BOOL*)ExtractEventOption(option, name);
	return (ptr != NULL) ? *ptr : defval;
}

int ExtractEventOptionInt(void *option, const char *name, int defval)
{
	int *ptr = (int*)ExtractEventOption(option, name);
	return (ptr != NULL) ? *ptr : defval;
}

float ExtractEventOptionFloat(void *option, const char *name, float defval)
{
	float *ptr = (float*)ExtractEventOption(option, name);
	return (ptr != NULL) ? *ptr : defval;
}


//---------------------------------------------------------------------------
//
//  class MQBasePlugin
//    すべての型のプラグインの基底クラス
//
//---------------------------------------------------------------------------

// Constructor
// コンストラクタ
MQBasePlugin::MQBasePlugin()
{
}

// Destructor
// デストラクタ
MQBasePlugin::~MQBasePlugin()
{
}

// Open a setting file
// 設定ファイルを開く
MQSetting *MQBasePlugin::OpenSetting(void)
{
	MQSendMessageInfo info;
	void *array[5];

	array[0] = "root";
	array[1] = NULL;
	array[2] = "pluginsettings";
	array[3] = NULL;
	array[4] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_GET_SETTING_ELEMENT, &info);

	MQXmlElement root_elem = (MQXmlElement)array[3];
	if(root_elem == NULL){
		return NULL;
	}

	// Determine the section name.
	// セクション名を決定
	DWORD Product, ID;
	char section[64];
	GetPlugInID(&Product, &ID);
	sprintf_s(section, sizeof(section), "Plugin-%08X-%08X", Product, ID);

	// Create 'MQSetting' and return it.
	// MQSettingを生成して返す
	return new MQSetting(root_elem, section);
}

// Close a setting file
// 設定ファイルを閉じる
void MQBasePlugin::CloseSetting(MQSetting *setting)
{
	delete setting;
}

// Get a resource string in command.xml
// command.xmlファイル内に定義されるリソース文字列を取得する
std::wstring MQBasePlugin::GetResourceString(const char *id)
{
	MQSendMessageInfo info;
	void *array[7];

	int result_size = 4096;
	wchar_t *result = new(std::nothrow) wchar_t[4096];
	if(result == NULL) return std::wstring();

	array[0] = "id";
	array[1] = (void*)id;
	array[2] = "result";
	array[3] = result;
	array[4] = "result_size";
	array[5] = &result_size;
	array[6] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_GET_RESOURCE_TEXT, &info);

	std::wstring ret(result);
	delete[] result;
	return ret;
}

std::wstring MQBasePlugin::GetResourceString(const char *id, DWORD productID, DWORD pluginID)
{
	MQSendMessageInfo info;
	void *array[7];

	int result_size = 4096;
	wchar_t *result = new(std::nothrow) wchar_t[4096];
	if(result == NULL) return std::wstring();

	array[0] = "id";
	array[1] = (void*)id;
	array[2] = "result";
	array[3] = result;
	array[4] = "result_size";
	array[5] = &result_size;
	array[6] = NULL;

	info.Product = productID;
	info.ID = pluginID;
	info.option = array;

	MQ_SendMessage(MQMESSAGE_GET_RESOURCE_TEXT, &info);

	std::wstring ret(result);
	delete[] result;
	return ret;
}

int MQBasePlugin::GetSettingIntValue(MQSETTINGVALUE_TYPE type)
{
	DWORD productID, pluginID;
	GetPlugInID(&productID, &pluginID);
	std::wstring str = GetSettingValue(type, productID, pluginID);
	return _wtoi(str.c_str());
}

float MQBasePlugin::GetSettingFloatValue(MQSETTINGVALUE_TYPE type)
{
	DWORD productID, pluginID;
	GetPlugInID(&productID, &pluginID);
	std::wstring str = GetSettingValue(type, productID, pluginID);
	return (float)_wtof(str.c_str());
}

std::vector<float> MQBasePlugin::GetSettingFloatArray(MQSETTINGVALUE_TYPE type)
{
	DWORD productID, pluginID;
	GetPlugInID(&productID, &pluginID);
	std::wstring str = GetSettingValue(type, productID, pluginID);
	std::vector<float> array;
	for(std::wstring::size_type st = 0; st < str.length(); ){
		std::wstring::size_type ed = str.find(L',', st);
		if(ed == std::wstring::npos){
			array.push_back((float)_wtof(str.substr(st).c_str()));
			break;
		}else{
			array.push_back((float)_wtof(str.substr(st, ed-st).c_str()));
			st = ed + 1;
		}
	}
	return array;
}

std::wstring MQBasePlugin::GetSettingValue(MQSETTINGVALUE_TYPE type)
{
	DWORD productID, pluginID;
	GetPlugInID(&productID, &pluginID);
	return GetSettingValue(type, productID, pluginID);
}

std::wstring MQBasePlugin::GetSettingValue(MQSETTINGVALUE_TYPE type, DWORD productID, DWORD pluginID)
{
	MQSendMessageInfo info;
	void *array[5];

	int length = 100;
	switch(type){
	case MQSETTINGVALUE_LANGUAGE:
		array[0] = "name";
		array[1] = "language.length";
		array[2] = "result_i";
		array[3] = &length;
		array[4] = NULL;

		info.Product = productID;
		info.ID = pluginID;
		info.option = array;

		if(!MQ_SendMessage(MQMESSAGE_GET_TEXT_VALUE, &info)){
			return std::wstring();
		}
		break;
	}

	wchar_t *result = new(std::nothrow) wchar_t[length+1];
	if(result == NULL) return std::wstring();
	*result = L'\0';

	switch(type){
	case MQSETTINGVALUE_LANGUAGE:
		array[0] = "name";
		array[1] = "language";
		array[2] = "result_w";
		array[3] = result;
		array[4] = NULL;

		info.Product = productID;
		info.ID = pluginID;
		info.option = array;

		MQ_SendMessage(MQMESSAGE_GET_TEXT_VALUE, &info);
		break;
	case MQSETTINGVALUE_ROTATION_HANDLE:
		array[0] = "name";
		array[1] = "rotation_handle";
		array[2] = "result_w";
		array[3] = result;
		array[4] = NULL;

		info.Product = productID;
		info.ID = pluginID;
		info.option = array;

		MQ_SendMessage(MQMESSAGE_GET_TEXT_VALUE, &info);
		break;
	case MQSETTINGVALUE_HANDLE_SIZE:
		array[0] = "name";
		array[1] = "handle_size";
		array[2] = "result_w";
		array[3] = result;
		array[4] = NULL;

		info.Product = productID;
		info.ID = pluginID;
		info.option = array;

		MQ_SendMessage(MQMESSAGE_GET_TEXT_VALUE, &info);
		break;
	case MQSETTINGVALUE_HANDLE_SCALE:
		array[0] = "name";
		array[1] = "handle_scale";
		array[2] = "result_w";
		array[3] = result;
		array[4] = NULL;

		info.Product = productID;
		info.ID = pluginID;
		info.option = array;

		MQ_SendMessage(MQMESSAGE_GET_TEXT_VALUE, &info);
		break;
	case MQSETTINGVALUE_NORMALMAP_FLIP:
		array[0] = "name";
		array[1] = "normalmap_flip";
		array[2] = "result_w";
		array[3] = result;
		array[4] = NULL;

		info.Product = productID;
		info.ID = pluginID;
		info.option = array;

		MQ_SendMessage(MQMESSAGE_GET_TEXT_VALUE, &info);
		break;
	}

	std::wstring ret(result);
	delete[] result;
	return ret;
}

//---------------------------------------------------------------------------
//  MQBasePlugin::GetSystemColor
//---------------------------------------------------------------------------
MQColor MQBasePlugin::GetSystemColor(MQSYSTEMCOLOR_TYPE color_type)
{
	float color[3] = {1,1,1};
	MQSendMessageInfo info;
	void *array[5];
	
	array[0] = "name";
	switch(color_type){
	case MQSYSTEMCOLOR_OBJECT: array[1] = "object"; break;
	case MQSYSTEMCOLOR_SELECT: array[1] = "select"; break;
	case MQSYSTEMCOLOR_TEMP: array[1] = "temp"; break;
	case MQSYSTEMCOLOR_HIGHLIGHT: array[1] = "highlight"; break;
	case MQSYSTEMCOLOR_UNACTIVE: array[1] = "unactive"; break;
	case MQSYSTEMCOLOR_MESH_YZ: array[1] = "mesh_yz"; break;
	case MQSYSTEMCOLOR_MESH_ZX: array[1] = "mesh_zx"; break;
	case MQSYSTEMCOLOR_MESH_XY: array[1] = "mesh_xy"; break;
	case MQSYSTEMCOLOR_BLOB_PLUS: array[1] = "blob_plus"; break;
	case MQSYSTEMCOLOR_BLOB_MINUS: array[1] = "blob_minus"; break;
	case MQSYSTEMCOLOR_UV: array[1] = "uv"; break;
	case MQSYSTEMCOLOR_AXIS_X: array[1] = "axis_x"; break;
	case MQSYSTEMCOLOR_AXIS_Y: array[1] = "axis_y"; break;
	case MQSYSTEMCOLOR_AXIS_Z: array[1] = "axis_z"; break;
	case MQSYSTEMCOLOR_AXIS_W: array[1] = "axis_w"; break;
	case MQSYSTEMCOLOR_AXIS_ACTIVE: array[1] = "axis_active"; break;
	case MQSYSTEMCOLOR_PREVIEW_BACK: array[1] = "preview_back"; break;
	default: return MQColor(1,1,1);
	}

	array[2] = "result.rgb";
	array[3] = color;
	array[4] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_GET_SYSTEM_COLOR, &info);

	return MQColor(color[0], color[1], color[2]);
}

//---------------------------------------------------------------------------
//  MQBasePlugin::GetResourceCursor
//    Get a default mouse cursor
//    標準マウスカーソルを取得
//---------------------------------------------------------------------------
HCURSOR MQBasePlugin::GetResourceCursor(MQCURSOR_TYPE cursor_type)
{
	MQSendMessageInfo info;
	void *array[5];
	
	array[0] = "index";
	array[1] = &cursor_type;
	array[2] = "result";
	array[3] = NULL;
	array[4] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_GET_RESOURCE_CURSOR, &info);

	return (HCURSOR)array[3];
}

//---------------------------------------------------------------------------
//  MQBasePlugin::GetScreenMouseCursor
//    Get a current mouse cursor for a screen
//    現在のスクリーン用マウスカーソルを取得
//---------------------------------------------------------------------------
HCURSOR MQBasePlugin::GetScreenMouseCursor()
{
	MQSendMessageInfo info;
	void *array[3];

	HCURSOR cursor = NULL;
	
	array[0] = "cursor";
	array[1] = &cursor;
	array[2] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_GET_SCREEN_MOUSE_CURSOR, &info);

	return cursor;
}

//---------------------------------------------------------------------------
//  MQBasePlugin::SetScreenMouseCursor
//    Set a mouse cursor for a screen.
//    スクリーン用マウスカーソルを設定
//
//    設定するカーソルはGetResourceCursor()で取得したものか、または
//    Win32APIのLoadCursor()で得られるDLLのインスタンスが保持する
//    リソースカーソルを指定して下さい。
//    LoadCursor()でインスタンスにNULLを指定して得られる定義済み
//    カーソルを指定した時の動作は保証されません。
//---------------------------------------------------------------------------
void MQBasePlugin::SetScreenMouseCursor(HCURSOR cursor)
{
	MQSendMessageInfo info;
	void *array[3];
	
	array[0] = "cursor";
	array[1] = cursor;
	array[2] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_SET_SCREEN_MOUSE_CURSOR, &info);
}

int MQBasePlugin::GetLUTCount()
{
	MQSendMessageInfo info;
	void *array[3];
	int num = 0;
	
	array[0] = "number";
	array[1] = &num;
	array[2] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_GET_LUT, &info);

	return num;
}

std::wstring MQBasePlugin::GetLUTName(int index)
{
	MQSendMessageInfo info;
	void *array[5];
	const wchar_t *name = nullptr;
	
	array[0] = "get_name";
	array[1] = &name;
	array[2] = "index";
	array[3] = &index;
	array[4] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_GET_LUT, &info);

	return std::wstring(name);
}

const unsigned char *MQBasePlugin::GetLUTData(int index)
{
	MQSendMessageInfo info;
	void *array[5];
	const unsigned char *lut = nullptr;
	
	array[0] = "get_lut";
	array[1] = &lut;
	array[2] = "index";
	array[3] = &index;
	array[4] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_GET_LUT, &info);

	return lut;
}

const unsigned char *MQBasePlugin::GetDefaultLUTData()
{
	MQSendMessageInfo info;
	void *array[3];
	const unsigned char *lut = nullptr;
	
	array[0] = "get_default_lut";
	array[1] = &lut;
	array[2] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_GET_LUT, &info);

	return lut;
}

//---------------------------------------------------------------------------
//  MQBasePlugin::SendUserMessage
//    Send an arbitrary message to an other station or command plugin.
//    A data can be send as a pointer by the 'message'.
//    It notifies to all plugins if target_product and target_id are 0.
//    他のプラグイン（常駐型/コマンド型）に任意のメッセージを送ります。
//    プラグインに渡すデータは任意のものをmessageにポインタとして与える
//    ことができます。
//    target_productとtarget_idに0を指定すると、すべてのプラグインに通知されます。
//
//    target_product	- 送出先プラグインのプロダクトID
//    target_id			- 送出先プラグインのプラグインID
//    description		- メッセージの種類を示す任意の文字列
//    message			- 任意のメッセージ
//    戻り値			- プラグインから返された値
//---------------------------------------------------------------------------
int MQBasePlugin::SendUserMessage(MQDocument doc, DWORD target_product, DWORD target_id, const char *description, void *message)
{
	MQSendMessageInfo info;
	void *array[13];
	int result = 0;

	array[0] = "document";
	array[1] = doc;
	array[2] = "target_product";
	array[3] = &target_product;
	array[4] = "target_id";
	array[5] = &target_id;
	array[6] = "description";
	array[7] = (void*)description;
	array[8] = "message";
	array[9] = message;
	array[10] = "result";
	array[11] = &result;
	array[12] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_USER_MESSAGE, &info);

	return result;
}



//---------------------------------------------------------------------------
//
//  class MQImportPlugin
//    インポートプラグイン用のベースクラス
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  MQImportPlugin::MQImportPlugin
//    Constructor
//    コンストラクタ
//---------------------------------------------------------------------------
MQImportPlugin::MQImportPlugin()
{
	m_ImportBackground = FALSE;
	m_ImportOptions = NULL;
}

//---------------------------------------------------------------------------
//  MQImportPlugin::IsBackground
//    Get whether the current loading is foreground(FALSE) or background(TRUE).
//    現在の読み込みがフォアグラウンド(FALSE)かバックグラウンド(TRUE)読み込みかを返す。
//---------------------------------------------------------------------------
BOOL MQImportPlugin::IsBackground(void)
{
	return m_ImportBackground;
}

void *MQImportPlugin::GetImportOptions(void)
{
	return m_ImportOptions;
}

//---------------------------------------------------------------------------
//  MQImportPlugin::IsCanceled
//    Get whether the current loading has been canceled or not.
//    現在の読み込みがキャンセルされたかどうかを返す。
//---------------------------------------------------------------------------
BOOL MQImportPlugin::IsCanceled(void)
{
	MQSendMessageInfo info;
	void *array[5];
	BOOL result = FALSE;

	array[0] = "options";
	array[1] = m_ImportOptions;
	array[2] = "result";
	array[3] = &result;
	array[4] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_IMPORT_QUERY_CANCEL, &info);

	return result;
}

//---------------------------------------------------------------------------
//  MQImportPlugin::SetProgress
//    Set a current progress from 0 to 1.
//    0～1の範囲で現在の進捗状況を設定します。
//---------------------------------------------------------------------------
void MQImportPlugin::SetProgress(float progress)
{
	MQSendMessageInfo info;
	void *array[5];
	BOOL result = FALSE;

	array[0] = "options";
	array[1] = m_ImportOptions;
	array[2] = "progress";
	array[3] = &progress;
	array[4] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_IMPORT_PROGRESS, &info);
}


//---------------------------------------------------------------------------
//
//  class MQStationPlugin
//    常駐型プラグイン用のベースクラス
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  MQStationPlugin::MQStationPlugin
//    コンストラクタ
//---------------------------------------------------------------------------
MQStationPlugin::MQStationPlugin()
{
}

//---------------------------------------------------------------------------
//  MQStationPlugin::EnumSubCommand
//    Enumerate sub command's name.
//    サブコマンドの名前を列挙
//---------------------------------------------------------------------------
const char *MQStationPlugin::EnumSubCommand(int index)
{
	return NULL;
}

//---------------------------------------------------------------------------
//  MQStationPlugin::EnumSubCommand
//    Get a sub command string for GUI.
//    サブコマンドの表示名の取得
//---------------------------------------------------------------------------
const wchar_t *MQStationPlugin::GetSubCommandString(int index)
{
	return NULL;
}

//---------------------------------------------------------------------------
//  MQStationPlugin::OnMinimize
//    ウインドウの最小化への返答
//---------------------------------------------------------------------------
void MQStationPlugin::OnMinimize(MQDocument doc, BOOL flag)
{
}

//---------------------------------------------------------------------------
//  MQStationPlugin::OnSubCommand
//    A message for calling a sub comand
//    サブコマンドの呼び出し
//---------------------------------------------------------------------------
BOOL MQStationPlugin::OnSubCommand(MQDocument doc, int index)
{
	return FALSE;
}

//---------------------------------------------------------------------------
//  MQStationPlugin::OnReceiveUserMessage
//    MQBasePlugin::SendUserMessage()で他のプラグインからメッセージが
//    送出された時に呼び出されます。
//    doc           - ドキュメント
//    src_product	- 送出元プラグインのプロダクトID
//    src_id		- 送出元プラグインのプラグインID
//    description	- メッセージの内容を表す任意の文字列
//    message		- メッセージの内容
//    戻り値        - 送出元プラグインに返す任意の値
//---------------------------------------------------------------------------
int MQStationPlugin::OnReceiveUserMessage(MQDocument doc, DWORD src_product, DWORD src_id, const char *description, void *message)
{
	return 0;
}


//---------------------------------------------------------------------------
//  MQStationPlugin::OnDraw
//    描画時に呼び出されます。
//---------------------------------------------------------------------------
void MQStationPlugin::OnDraw(MQDocument doc, MQScene scene, int width, int height)
{
}

//---------------------------------------------------------------------------
//  MQStationPlugin::OnNewDocument
//    ドキュメント初期化時に呼び出されます。
//    filename - [NULL以外] MQOファイルが読み込まれた場合のファイル名
//               [NULL] 新規ドキュメントとして生成された
//    param.elem - [NULL以外] ファイル保存時に付加されたXML要素
//                 [NULL] 新規ドキュメントとして生成された、またはXML要素は
//                        保存されていない。
//---------------------------------------------------------------------------
void MQStationPlugin::OnNewDocument(MQDocument doc, const char *filename, NEW_DOCUMENT_PARAM& param)
{
}

//---------------------------------------------------------------------------
//  MQStationPlugin::OnEndDocument
//    ドキュメント終了時に呼び出されます。
//---------------------------------------------------------------------------
void MQStationPlugin::OnEndDocument(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  MQStationPlugin::OnSaveDocument
//    MQOファイルが保存される前に呼び出されます。
//    param.elemはプラグイン独自の情報を保存するためのXML要素で、この要素に
//    子要素を付加することができます。
//---------------------------------------------------------------------------
void MQStationPlugin::OnSaveDocument(MQDocument doc, const char *filename, SAVE_DOCUMENT_PARAM& param)
{
}

//---------------------------------------------------------------------------
//  MQStationPlugin::OnUndo
//    アンドゥ実行時に呼び出されます。
//    ※戻り値はプラグイン独自のアンドゥ処理を行った場合にTRUEを返しますが、
//    　現段階では特殊な処理を認めておらず、FALSEを返す必要があります。
//---------------------------------------------------------------------------
BOOL MQStationPlugin::OnUndo(MQDocument doc, int undo_state)
{
	// 特別なアンドゥ処理を行わなかったのでFALSEを返す
	return FALSE;
}

//---------------------------------------------------------------------------
//  MQStationPlugin::OnRedo
//    リドゥ実行時に呼び出されます。
//    ※戻り値はプラグイン独自のアンドゥ処理を行った場合にTRUEを返しますが、
//    　現段階では特殊な処理を認めておらず、FALSEを返す必要があります。
//---------------------------------------------------------------------------
BOOL MQStationPlugin::OnRedo(MQDocument doc, int redo_state)
{
	// 特別なリドゥ処理を行わなかったのでFALSEを返す
	return FALSE;
}

//---------------------------------------------------------------------------
//  MQStationPlugin::OnUpdateUndo
//    アンドゥの状態が更新されたときに呼び出されます。
//    この関数内ではオブジェクトの内容を更に変更してはいけません。
//    undo_state - アンドゥの状態カウンタ
//    undo_size  - アンドゥの実行可能回数
//---------------------------------------------------------------------------
void MQStationPlugin::OnUpdateUndo(MQDocument doc, int undo_state, int undo_size)
{
}

//---------------------------------------------------------------------------
//  MQStationPlugin::OnObjectModified
//    オブジェクトが編集された時に呼び出されます。
//    この関数内ではオブジェクトの内容を更に変更してはいけません。
//    ※アンドゥが更新される前に呼び出されます。
//---------------------------------------------------------------------------
void MQStationPlugin::OnObjectModified(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  MQStationPlugin::OnObjectSelected
//    オブジェクトの頂点・辺・面・UVが選択された時に呼び出されます。
//    この関数内ではオブジェクトの内容・選択状態を更に変更してはいけません。
//    ※アンドゥが更新される前に呼び出されます。
//---------------------------------------------------------------------------
void MQStationPlugin::OnObjectSelected(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  MQStationPlugin::OnUpdateObjectList
//    カレントオブジェクトの変更や、オブジェクトの追加・削除が行われた時に
//    呼び出されます。
//    この関数内ではオブジェクトの内容・選択状態を更に変更してはいけません。
//    ※アンドゥが更新される前に呼び出されます。
//---------------------------------------------------------------------------
void MQStationPlugin::OnUpdateObjectList(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  MQStationPlugin::OnMaterialModified
//    マテリアル内のパラメータが変更された時に呼び出されます。
//    この関数内ではオブジェクト・マテリアルの内容を更に変更してはいけません。
//    ※アンドゥが更新される前に呼び出されます。
//---------------------------------------------------------------------------
void MQStationPlugin::OnMaterialModified(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  MQStationPlugin::OnUpdateMaterialList
//    カレントマテリアルの変更や、マテリアルの追加・削除が行われた時に
//    呼び出されます。
//    この関数内ではマテリアルの内容を更に変更してはいけません。
//    ※アンドゥが更新される前に呼び出されます。
//---------------------------------------------------------------------------
void MQStationPlugin::OnUpdateMaterialList(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  MQStationPlugin::OnMaterialModified
//    カメラの向きやズームなど、シーン情報が変更された時に呼び出されます。
//    この関数内ではシーンを更に変更してはいけません。
//---------------------------------------------------------------------------
void MQStationPlugin::OnUpdateScene(MQDocument doc, MQScene scene)
{
}

//---------------------------------------------------------------------------
//  MQStationPlugin::OnChangeEditOption
//    編集オプションが変更された時に呼び出されます。
//---------------------------------------------------------------------------
void MQStationPlugin::OnChangeEditOption(MQDocument doc, EDITOPTION_TYPE trigger)
{
}

//---------------------------------------------------------------------------
//  MQStationPlugin::WindowClose
//    プラグインが管理しているウインドウが閉じられたときに
//    この関数を呼び出しすと、本体側に通知されメニュー上の
//    チェックが外れるようになります。
//---------------------------------------------------------------------------
void MQStationPlugin::WindowClose()
{
	MQSendMessageInfo info;
	BOOL flag = FALSE;

	GetPlugInID(&info.Product, &info.ID);
	info.option = &flag;

	MQ_SendMessage(MQMESSAGE_ACTIVATE, &info);
}

//---------------------------------------------------------------------------
//  MQStationPlugin::BeginCallback
//    Windowsメッセージに対する応答をプラグイン内で処理する場合、
//    この関数を呼び出すと必要な初期化処理が行われた後に
//    ExecuteCallback()が呼び出され、MQDocumentに対する処理を
//    行うことができます。
//---------------------------------------------------------------------------
void MQStationPlugin::BeginCallback(void *option)
{
	StationCallbackInnerOption inner;
	inner.this_ptr = this;
	inner.option = option;

	MQ_StationCallback(StationCallback, &inner);
}

//---------------------------------------------------------------------------
//  MQStationPlugin::StationCallback
//    BeginCallback()を呼び出した時に、内部で呼び出されるstaticな
//    コールバック関数です。
//    この関数はさらに純粋仮想関数ExecuteCallback()を呼び出します。
//---------------------------------------------------------------------------
BOOL __stdcall MQStationPlugin::StationCallback(MQDocument doc, void *option)
{
	StationCallbackInnerOption *inner = (StationCallbackInnerOption*)option;
	return inner->this_ptr->ExecuteCallback(doc, inner->option) ? TRUE : FALSE;
}


//---------------------------------------------------------------------------
//  MQStationPlugin::CreateDrawingObject
//    OnDraw()時に描画するオブジェクトを追加します。
//
//    MQObject::DeleteThis()で削除してはいけません。
//    instantにTRUEを指定して作成されたオブジェクトは描画が完了すると自動的に
//    破棄されます。
//    instantにFALSEを指定して作成したオブジェクトは、不要になった時点で
//    DeleteDrawingObject()で削除してください。また、オブジェクトが保持
//    されたままOnEndDocument()が呼び出されたら、その時点で削除してください。
//---------------------------------------------------------------------------
MQObject MQStationPlugin::CreateDrawingObject(MQDocument doc, DRAW_OBJECT_VISIBILITY visibility, BOOL instant)
{
	MQSendMessageInfo info;
	void *array[9];

	array[0] = "document";
	array[1] = doc;
	array[2] = "visibility";
	array[3] = &visibility;
	array[4] = "instant";
	array[5] = &instant;
	array[6] = "result";
	array[7] = NULL;
	array[8] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_NEW_DRAW_OBJECT, &info);

	return (MQObject)array[7];
}

MQObject MQStationPlugin::CreateDrawingObjectByClone(MQDocument doc, MQObject clone_source, DRAW_OBJECT_VISIBILITY visibility, BOOL instant)
{
	MQSendMessageInfo info;
	void *array[11];

	array[0] = "document";
	array[1] = doc;
	array[2] = "clone_source";
	array[3] = clone_source;
	array[4] = "visibility";
	array[5] = &visibility;
	array[6] = "instant";
	array[7] = &instant;
	array[8] = "result";
	array[9] = NULL;
	array[10] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_NEW_DRAW_OBJECT, &info);

	return (MQObject)array[9];
}


//---------------------------------------------------------------------------
//  MQStationPlugin::CreateDrawingMaterial
//    OnDraw()時のCreateDrawingObject()で作成したオブジェクト描画に使用する
//    マテリアルを追加します。
//    indexにはMQObject::SetFaceMaterial()に指定するインデックスが返されます。
//
//    作成されたマテリアルはMQMaterial::DeleteThis()で削除してはいけません。
//    instantにTRUEを指定して作成されたマテリアルは描画が完了すると自動的に
//    破棄されます。
//    instantにFALSEを指定して作成したマテリアルは、不要になった時点で
//    DeleteDrawingMaterial()で削除してください。また、マテリアルが保持
//    されたままOnEndDocument()が呼び出されたら、その時点で削除してください。
//---------------------------------------------------------------------------
MQMaterial MQStationPlugin::CreateDrawingMaterial(MQDocument doc, int& index, BOOL instant)
{
	MQSendMessageInfo info;
	void *array[9];
	int i=-1;

	array[0] = "document";
	array[1] = doc;
	array[2] = "index";
	array[3] = &i;
	array[4] = "instant";
	array[5] = &instant;
	array[6] = "result";
	array[7] = NULL;
	array[8] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_NEW_DRAW_MATERIAL, &info);

	index = i;
	return (MQMaterial)array[7];
}


//---------------------------------------------------------------------------
//  MQStationPlugin::CreateDrawingText
//---------------------------------------------------------------------------
MQDrawingText MQStationPlugin::CreateDrawingText(MQDocument doc, const wchar_t *text, DRAWING_TEXT_PARAM& param, BOOL instant)
{
	MQSendMessageInfo info;
	void *array[19];
	float pos[3] = {param.ScreenPos.x, param.ScreenPos.y, param.ScreenPos.z};
	float col[3] = {param.Color.r, param.Color.g, param.Color.b};

	int ai = 0;
	array[ai++] = "document";
	array[ai++] = doc;
	array[ai++] = "text";
	array[ai++] = (void*)text;
	array[ai++] = "pos";
	array[ai++] = pos; // 6
	array[ai++] = "color";
	array[ai++] = col;
	if(param.FontScale != 1){
		array[ai++] = "font_scale";
		array[ai++] = &param.FontScale;
	}
	if(param.HorzAlign != TEXT_ALIGN_LEFT){
		array[ai++] = "horz_align";
		array[ai++] = &param.HorzAlign; // 12
	}
	if(param.VertAlign != TEXT_ALIGN_TOP){
		array[ai++] = "vert_align";
		array[ai++] = &param.VertAlign;
	}
	array[ai++] = "instant";
	array[ai++] = &instant;
	array[ai++] = "result";
	array[ai++] = NULL; // 18
	array[ai++] = NULL; // 19
	assert(ai <= _countof(array));

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_NEW_DRAW_TEXT, &info);

	return (MQDrawingText)array[11];
}


//---------------------------------------------------------------------------
//  MQStationPlugin::DeleteDrawingObject
//    CreateDrawingObject()でinstantをFALSEにして作成したオブジェクトを削除
//    します。
//---------------------------------------------------------------------------
void MQStationPlugin::DeleteDrawingObject(MQDocument doc, MQObject obj)
{
	MQSendMessageInfo info;
	void *array[5];

	array[0] = "document";
	array[1] = doc;
	array[2] = "object";
	array[3] = obj;
	array[4] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_DELETE_DRAW_OBJECT, &info);
}


//---------------------------------------------------------------------------
//  MQStationPlugin::DeleteDrawingMaterial
//    CreateDrawingMaterial()でinstantをFALSEにして作成したマテリアルを削除
//    します。
//---------------------------------------------------------------------------
void MQStationPlugin::DeleteDrawingMaterial(MQDocument doc, MQMaterial mat)
{
	MQSendMessageInfo info;
	void *array[5];

	array[0] = "document";
	array[1] = doc;
	array[2] = "material";
	array[3] = mat;
	array[4] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_DELETE_DRAW_MATERIAL, &info);
}


//---------------------------------------------------------------------------
//  MQStationPlugin::DeleteDrawingText
//    CreateDrawingText()でinstantをFALSEにして作成したマテリアルを削除
//    します。
//---------------------------------------------------------------------------
void MQStationPlugin::DeleteDrawingText(MQDocument doc, MQDrawingText text)
{
	MQSendMessageInfo info;
	void *array[5];

	array[0] = "document";
	array[1] = doc;
	array[2] = "text";
	array[3] = text;
	array[4] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_DELETE_DRAW_MATERIAL, &info);
}


//---------------------------------------------------------------------------
//  MQStationPlugin::SetDrawProxyObject
//    実際の描画を別のオブジェクトを用いて行います。
//---------------------------------------------------------------------------
void MQStationPlugin::SetDrawProxyObject(MQObject obj, MQObject proxy)
{
	MQSendMessageInfo info;
	void *array[5];

	array[0] = "object";
	array[1] = obj;
	array[2] = "proxy";
	array[3] = proxy;
	array[4] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_SET_DRAW_PROXY_OBJECT, &info);
}


//---------------------------------------------------------------------------
//  MQStationPlugin::GetCurrentUndoState
//    現在のアンドゥの状態カウンタを取得します。
//---------------------------------------------------------------------------
int MQStationPlugin::GetCurrentUndoState(MQDocument doc)
{
	MQSendMessageInfo info;
	void *array[5];
	int state=-1;

	array[0] = "document";
	array[1] = doc;
	array[2] = "result";
	array[3] = &state;
	array[4] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_GET_UNDO_STATE, &info);

	return state;
}

//---------------------------------------------------------------------------
//  MQStationPlugin::GetSceneOption
//    シーンの表示オプションを取得
//---------------------------------------------------------------------------
void MQStationPlugin::GetSceneOption(MQScene scene, SCENE_OPTION& option)
{
	MQSendMessageInfo info;
	void *array[13];

	memset(&option, 0, sizeof(SCENE_OPTION));
	
	array[0] = "scene";
	array[1] = scene;
	array[2] = "show_vertex";
	array[3] = &option.ShowVertex;
	array[4] = "show_line";
	array[5] = &option.ShowLine;
	array[6] = "show_face";
	array[7] = &option.ShowFace;
	array[8] = "front_only";
	array[9] = &option.FrontOnly;
	array[10] = "show_bkimg";
	array[11] = &option.ShowBkimg;
	array[12] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_GET_SCENE_OPTION, &info);
}




//---------------------------------------------------------------------------
//
//  class MQCommandPlugin
//    コマンドプラグイン用のベースクラス
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  MQCommandPlugin::MQCommandPlugin
//    コンストラクタ
//---------------------------------------------------------------------------
MQCommandPlugin::MQCommandPlugin()
{
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::IsActivated
//    表示・非表示状態の返答
//---------------------------------------------------------------------------
BOOL MQCommandPlugin::IsActivated(MQDocument doc)
{
	// コマンドプラグインではIsActivated()は必ずしも実装する必要は
	// ないので、標準動作としてTRUEを返す
	return TRUE;
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::ExecuteCallback
//    コールバックに対する実装部
//---------------------------------------------------------------------------
bool MQCommandPlugin::ExecuteCallback(MQDocument doc, void *option)
{
	// コマンドプラグインではExecuteCallback()は必ずしも実装する必要は
	// ないので、標準動作としてtrueを返す
	return true;
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::OnLeftButtonDown
//    左ボタンが押されたときに呼び出されます。
//    プラグイン独自の動作を行った場合にはTRUEを、独自処理を行わず
//    標準動作を行わせる場合にはFALSEを戻り値として返します。
//---------------------------------------------------------------------------
BOOL MQCommandPlugin::OnLeftButtonDown(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	// プラグイン独自の動作を行った後はTRUEを返す
	return TRUE;
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::OnLeftButtonMove
//    左ボタンが押されながらマウスが移動したときに呼び出されます。
//---------------------------------------------------------------------------
BOOL MQCommandPlugin::OnLeftButtonMove(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	// プラグイン独自の動作を行った後はTRUEを返す
	return TRUE;
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::OnLeftButtonUp
//    左ボタンが離されたときに呼び出されます。
//---------------------------------------------------------------------------
BOOL MQCommandPlugin::OnLeftButtonUp(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	// プラグイン独自の動作を行った後はTRUEを返す
	return TRUE;
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::OnMiddleButtonDown
//    中ボタンが押されたときに呼び出されます。
//---------------------------------------------------------------------------
BOOL MQCommandPlugin::OnMiddleButtonDown(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	// 標準動作を行うためにFALSEを返す
	return FALSE;
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::OnMiddleButtonMove
//    中ボタンが押されながらマウスが移動したときに呼び出されます。
//---------------------------------------------------------------------------
BOOL MQCommandPlugin::OnMiddleButtonMove(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	// 標準動作を行うためにFALSEを返す
	return FALSE;
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::OnMiddleButtonUp
//    中ボタンが離されたときに呼び出されます。
//---------------------------------------------------------------------------
BOOL MQCommandPlugin::OnMiddleButtonUp(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	// 標準動作を行うためにFALSEを返す
	return FALSE;
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::OnRightButtonDown
//    右ボタンが押されたときに呼び出されます。
//---------------------------------------------------------------------------
BOOL MQCommandPlugin::OnRightButtonDown(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	// 標準動作を行うためにFALSEを返す
	return FALSE;
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::OnRightButtonMove
//    右ボタンが押されながらマウスが移動したときに呼び出されます。
//---------------------------------------------------------------------------
BOOL MQCommandPlugin::OnRightButtonMove(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	// 標準動作を行うためにFALSEを返す
	return FALSE;
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::OnRightButtonUp
//    右ボタンが離されたときに呼び出されます。
//---------------------------------------------------------------------------
BOOL MQCommandPlugin::OnRightButtonUp(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	// 標準動作を行うためにFALSEを返す
	return FALSE;
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::OnMouseMove
//    マウスが移動したときに呼び出されます。
//---------------------------------------------------------------------------
BOOL MQCommandPlugin::OnMouseMove(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	// 標準動作を行うためにFALSEを返す
	return FALSE;
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::OnMouseWheel
//    マウスのホイールが回転したときに呼び出されます。
//    ホイールの回転量はstate.WheelにWHEEL_DELTAの倍数または約数で入力されます。
//---------------------------------------------------------------------------
BOOL MQCommandPlugin::OnMouseWheel(MQDocument doc, MQScene scene, MOUSE_BUTTON_STATE& state)
{
	// 標準動作を行うためにFALSEを返す
	return FALSE;
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::OnKeyDown
//    キーが押されたときに呼び出されます。
//---------------------------------------------------------------------------
BOOL MQCommandPlugin::OnKeyDown(MQDocument doc, MQScene scene, int key, MOUSE_BUTTON_STATE& state)
{
	// 標準動作を行うためにFALSEを返す
	return FALSE;
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::OnKeyUp
//    キーが押されたときに呼び出されます。
//---------------------------------------------------------------------------
BOOL MQCommandPlugin::OnKeyUp(MQDocument doc, MQScene scene, int key, MOUSE_BUTTON_STATE& state)
{
	// 標準動作を行うためにFALSEを返す
	return FALSE;
}


// アンドゥバッファを更新する
void MQCommandPlugin::UpdateUndo()
{
	MQSendMessageInfo info;
	void *array[1];

	array[0] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_UPDATE_UNDO, &info);
}

void MQCommandPlugin::UpdateUndo(const wchar_t *str)
{
	MQSendMessageInfo info;
	void *array[3];

	array[0] = "string";
	array[1] = (void*)str;
	array[2] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_UPDATE_UNDO, &info);
}

// シーンの再描画を予約する
void MQCommandPlugin::RedrawScene(MQScene scene)
{
	MQSendMessageInfo info;
	void *array[3];

	array[0] = "scene";
	array[1] = scene;
	array[2] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_REDRAW_SCENE, &info);
}

// すべてのシーンの再描画を予約する
void MQCommandPlugin::RedrawAllScene()
{
	MQSendMessageInfo info;
	void *array[1];

	array[0] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_REDRAW_ALL_SCENE, &info);
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::GetEditOption
//    編集オプションを取得
//---------------------------------------------------------------------------
void MQCommandPlugin::GetEditOption(EDIT_OPTION& option)
{
	MQSendMessageInfo info;
	void *array[39];
	int plane = 0;
	float plane_pos[3] = {0,0,0}, plane_dir[3] = {0,0,0};

	memset(&option, 0, sizeof(EDIT_OPTION));
	
	array[0] = "edit_vertex";
	array[1] = &option.EditVertex;
	array[2] = "edit_line";
	array[3] = &option.EditLine;
	array[4] = "edit_face";
	array[5] = &option.EditFace;
	array[6] = "select_rect";
	array[7] = &option.SelectRect;
	array[8] = "select_rope";
	array[9] = &option.SelectRope;
	array[10] = "symmetry";
	array[11] = &option.Symmetry;
	array[12] = "symmetry_distance";
	array[13] = &option.SymmetryDistance;
	array[14] = "cur_obj_only";
	array[15] = &option.CurrentObjectOnly;
	array[16] = "snap_x";
	array[17] = &option.SnapX;
	array[18] = "snap_y";
	array[19] = &option.SnapY;
	array[20] = "snap_z";
	array[21] = &option.SnapZ;
	array[22] = "coordinate_type";
	array[23] = &option.CoordinateType;
	array[24] = "snap_grid";
	array[25] = &option.SnapGrid;
	array[26] = "snap_plane";
	array[27] = &plane;;
	array[28] = "snap_plane_pos";
	array[29] = plane_pos;
	array[30] = "snap_plane_dir";
	array[31] = plane_dir;
	array[32] = "snap_vertex";
	array[33] = &option.SnapVertex;
	array[34] = "snap_line";
	array[35] = &option.SnapLine;
	array[36] = "snap_face";
	array[37] = &option.SnapFace;
	array[38] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_GET_EDIT_OPTION, &info);

	option.SnapPlane = (SNAP_PLANE_TYPE)plane;
	option.SnapPlanePos = MQPoint(plane_pos[0], plane_pos[1], plane_pos[2]);
	option.SnapPlaneDir = MQPoint(plane_dir[0], plane_dir[1], plane_dir[2]);
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::GetSnappedPos
//    スナップ位置を取得
//---------------------------------------------------------------------------
MQPoint MQCommandPlugin::GetSnappedPos(MQScene scene, const MQPoint& p, SNAP_GRID_TYPE type)
{
	MQSendMessageInfo info;
	float pos[3], result[3];
	void *array[9];
	int itype = type;
	
	pos[0] = p.x;
	pos[1] = p.y;
	pos[2] = p.z;
	result[0] = result[1] = result[2] = 0;

	array[0] = "scene";
	array[1] = scene;
	array[2] = "type";
	array[3] = &itype;
	array[4] = "pos";
	array[5] = pos;
	array[6] = "result";
	array[7] = result;
	array[8] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_GET_SNAPPED_POS, &info);

	return MQPoint(result[0], result[1], result[2]);
}

MQPoint MQCommandPlugin::GetSnappedPos(MQScene scene, const MQPoint& p, const EDIT_OPTION& option, const GET_SNAP_PARAM& snap_param)
{
	MQSendMessageInfo info;
	int itype, iplane;
	float pos[3], plane_pos[3], plane_dir[3], result[3];
	void *array[27];

	if(option.SnapGrid == 0 && option.SnapPlane == SNAP_PLANE_NONE 
	&& !option.SnapVertex && !option.SnapLine && !option.SnapFace){
		return p;
	}
	
	itype = option.SnapGrid;
	iplane = option.SnapPlane;
	pos[0] = p.x;
	pos[1] = p.y;
	pos[2] = p.z;
	plane_pos[0] = option.SnapPlanePos.x;
	plane_pos[1] = option.SnapPlanePos.y;
	plane_pos[2] = option.SnapPlanePos.z;
	plane_dir[0] = option.SnapPlaneDir.x;
	plane_dir[1] = option.SnapPlaneDir.y;
	plane_dir[2] = option.SnapPlaneDir.z;
	result[0] = result[1] = result[2] = 0;

	array[0] = "scene";
	array[1] = scene;
	array[2] = "type";
	array[3] = &itype;
	array[4] = "pos";
	array[5] = pos;
	array[6] = "plane";
	array[7] = &iplane;
	array[8] = "plane_pos";
	array[9] = plane_pos;
	array[10] = "plane_dir";
	array[11] = plane_dir;
	array[12] = "vertex";
	array[13] = (void*)&option.SnapVertex;
	array[14] = "line";
	array[15] = (void*)&option.SnapLine;
	array[16] = "face";
	array[17] = (void*)&option.SnapFace;
	array[18] = "eye_dir";
	array[19] = (void*)&snap_param.SnapEyeDir;
	array[20] = "ignore_selected";
	array[21] = (void*)&snap_param.IgnoreSelected;
	array[22] = "ignore_curobj";
	array[23] = (void*)&snap_param.IgnoreCurObj;
	array[24] = "result";
	array[25] = result;
	array[26] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_GET_SNAPPED_POS, &info);

	return MQPoint(result[0], result[1], result[2]);
}


//---------------------------------------------------------------------------
//  MQCommandPlugin::HitTest
//    シーン内の指定された位置にあるものを検知
//---------------------------------------------------------------------------
bool MQCommandPlugin::HitTest(MQScene scene, POINT p, HIT_TEST_PARAM& param)
{
	MQSendMessageInfo info;
	void *array[21];
	int test_type = 0;

	if(param.TestVertex) test_type |= 1;
	if(param.TestLine)   test_type |= 2;
	if(param.TestFace)   test_type |= 4;
	if(param.DisableFrontOnly) test_type |= 8;

	param.HitType = HIT_TYPE_NONE;
	
	array[0] = "scene";
	array[1] = scene;
	array[2] = "x";
	array[3] = &p.x;
	array[4] = "y";
	array[5] = &p.y;
	array[6] = "test_type";
	array[7] = &test_type;
	array[8] = "hit_type";
	array[9] = &param.HitType;
	array[10] = "hit_pos";
	array[11] = &param.HitPos;
	array[12] = "hit_object";
	array[13] = &param.ObjectIndex;
	array[14] = "hit_vertex";
	array[15] = &param.VertexIndex;
	array[16] = "hit_line";
	array[17] = &param.LineIndex;
	array[18] = "hit_face";
	array[19] = &param.FaceIndex;
	array[20] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_HIT_TEST, &info);

	return (param.HitType != HIT_TYPE_NONE);
}


//---------------------------------------------------------------------------
//  MQCommandPlugin::HitTestObjects
//    シーン内の指定された位置にあるものを検知
//---------------------------------------------------------------------------
bool MQCommandPlugin::HitTestObjects(MQScene scene, POINT p, const std::vector<MQObject>& objects, HIT_TEST_PARAM& param)
{
	MQSendMessageInfo info;
	void *array[25];
	int test_type = 0;
	int objects_num = (int)objects.size();

	if(objects.empty()) return HIT_TYPE_NONE;

	if(param.TestVertex) test_type |= 1;
	if(param.TestLine)   test_type |= 2;
	if(param.TestFace)   test_type |= 4;
	if(param.DisableFrontOnly) test_type |= 8;
	if(param.DisableCoverByFace) test_type |= 16;

	param.HitType = HIT_TYPE_NONE;
	
	array[0] = "scene";
	array[1] = scene;
	array[2] = "x";
	array[3] = &p.x;
	array[4] = "y";
	array[5] = &p.y;
	array[6] = "test_type";
	array[7] = &test_type;
	array[8] = "hit_type";
	array[9] = &param.HitType;
	array[10] = "hit_pos";
	array[11] = &param.HitPos;
	array[12] = "hit_object";
	array[13] = &param.ObjectIndex;
	array[14] = "hit_vertex";
	array[15] = &param.VertexIndex;
	array[16] = "hit_line";
	array[17] = &param.LineIndex;
	array[18] = "hit_face";
	array[19] = &param.FaceIndex;
	array[20] = "objects";
	array[21] = (void**)&(*objects.begin());
	array[22] = "objects.num";
	array[23] = &objects_num;
	array[24] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_HIT_TEST, &info);

	return (param.HitType != HIT_TYPE_NONE);
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::SetMouseCursor
//    Set a mouse cursor for views.
//    ビュー用マウスカーソルを設定
//
//    設定するカーソルはGetResourceCursor()で取得したものか、または
//    Win32APIのLoadCursor()で得られるDLLのインスタンスが保持する
//    リソースカーソルを指定して下さい。
//    LoadCursor()でインスタンスにNULLを指定して得られる定義済み
//    カーソルを指定した時の動作は保証されません。
//---------------------------------------------------------------------------
void MQCommandPlugin::SetMouseCursor(HCURSOR cursor)
{
	MQSendMessageInfo info;
	void *array[3];
	
	array[0] = "cursor";
	array[1] = cursor;
	array[2] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_SET_MOUSE_CURSOR, &info);
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::SetStatusString
//    ステータスバーの文字列を設定
//---------------------------------------------------------------------------
void MQCommandPlugin::SetStatusString(const char *str)
{
	MQSendMessageInfo info;
	void *array[3];
	
	array[0] = "string";
	array[1] = (void*)str;
	array[2] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_SET_STATUS_STRING, &info);
}

void MQCommandPlugin::SetStatusString(const wchar_t *str)
{
	MQSendMessageInfo info;
	void *array[3];
	
	array[0] = "wstring";
	array[1] = (void*)str;
	array[2] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_SET_STATUS_STRING, &info);
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::SetHelpPage
//    ヘルプページを設定
//---------------------------------------------------------------------------
void MQCommandPlugin::SetHelpPage(const wchar_t *url)
{
	MQSendMessageInfo info;
	void *array[5];
	
	array[0] = "name";
	array[1] = "helppage";
	array[2] = "wstring";
	array[3] = (void*)url;
	array[4] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_SET_TEXT_VALUE, &info);
}

//---------------------------------------------------------------------------
//  MQCommandPlugin::ApplyLayoutToOptionPanel
//---------------------------------------------------------------------------
void MQCommandPlugin::ApplyLayoutToOptionPanel()
{
	MQSendMessageInfo info;
	void *array[5];
	
	array[0] = "name";
	array[1] = "applylayout";
	array[2] = NULL;

	GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_SET_TEXT_VALUE, &info);
}



void MQBasePluginMediator::ImportSetOptions(MQImportPlugin *plugin, BOOL background, void *options)
{
	plugin->m_ImportBackground = background;
	plugin->m_ImportOptions = options;
}

