//---------------------------------------------------------------------------
//
//   MQWidget
//
//          Copyright(C) 1999-2016, tetraface Inc.
//
//	   A class for accessing widgets to construct a graphical user interfaces 
//    such as buttons, check boxes, dialogs and so on.
//
//    　ボタン、チェックボックスやダイアログなどのGUIを構築するためのウィ
//    ジェットにアクセスするクラスです。
//
//---------------------------------------------------------------------------

#include <windows.h>
#include <map>
#include <assert.h>
#include "MQPlugin.h"
#include "MQBasePlugin.h"
#include "MQWidget.h"

static void *ExtractEventOption(void *option, const char *name);
static BOOL ExtractEventOptionBool(void *option, const char *name, BOOL defval);
static int ExtractEventOptionInt(void *option, const char *name, int defval);
static float ExtractEventOptionFloat(void *option, const char *name, float defval);


static std::map<int, MQWidgetBase*> s_WidgetIDMap;

//---------------------------------------------------------------------------
//  class MQWidgetBase
//---------------------------------------------------------------------------
MQWidgetBase::MQWidgetBase()
{
	m_ID = NullID;
	m_IDOwner = false;
}

MQWidgetBase::MQWidgetBase(int id)
{
	m_ID = id;
	m_IDOwner = false;

	if(m_ID != NullID){
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQWidgetBase::~MQWidgetBase()
{
	if(m_ID != NullID){
		s_WidgetIDMap.erase(m_ID);

		if(m_IDOwner){
			void *ptr[1];
			ptr[0] = NULL;
			MQWidget_Value(m_ID, MQWIDGET_DELETE, (void*)ptr);
		}

		m_ID = NullID;
		m_IDOwner = false;
	}
}

int MQWidgetBase::GetID() const
{
	return m_ID;
}

int MQWidgetBase::AddChild(MQWidgetBase *child)
{
	if(child == NULL) return -1;

	void *ptr[5];
	int child_id = child->GetID();
	int result = -1;

	ptr[0] = "child";
	ptr[1] = &child_id;
	ptr[2] = "result";
	ptr[3] = &result;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_ADD_CHILD, (void*)ptr);
	
	return result;
}

void MQWidgetBase::RemoveChild(MQWidgetBase *child)
{
	if(child == NULL) return;

	void *ptr[3];
	int child_id = child->GetID();
	int result = -1;

	ptr[0] = "child";
	ptr[1] = &child_id;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_REMOVE_CHILD, (void*)ptr);
}

std::wstring MQWidgetBase::GetName()
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "name.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "name";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQWidgetBase::SetName(const std::wstring& text)
{
	void *ptr[3];
	ptr[0] = "name";
	ptr[1] = (void*)text.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

__int64 MQWidgetBase::GetTag()
{
	__int64 value = 0;
	void *ptr[3];
	ptr[0] = "tag";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWidgetBase::SetTag(__int64 value)
{
	void *ptr[3];
	ptr[0] = "tag";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQWidgetBase::GetEnabled()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "enabled";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWidgetBase::SetEnabled(bool value)
{
	void *ptr[3];
	ptr[0] = "enabled";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQWidgetBase::GetVisible()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "visible";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);
	return value;
}

void MQWidgetBase::SetVisible(bool value)
{
	void *ptr[3];
	ptr[0] = "visible";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

MQWidgetBase::LAYOUT_TYPE MQWidgetBase::GetHorzLayout()
{
	char *layout_str = NULL;
	void *ptr[3];
	ptr[0] = "horzlayout";
	ptr[1] = &layout_str;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(strcmp(layout_str, "auto") == 0) return LAYOUT_AUTO;
	if(strcmp(layout_str, "fixed") == 0) return LAYOUT_FIXED;
	if(strcmp(layout_str, "hintsize") == 0) return LAYOUT_HINTSIZE;
	if(strcmp(layout_str, "fill") == 0) return LAYOUT_FILL;
	if(strcmp(layout_str, "free") == 0) return LAYOUT_FREE;
	return LAYOUT_AUTO;
}

void MQWidgetBase::SetHorzLayout(LAYOUT_TYPE value)
{
	void *ptr[3];
	ptr[0] = "horzlayout";
	switch(value){
	default:
	case LAYOUT_AUTO: ptr[1] = "auto"; break;
	case LAYOUT_FIXED: ptr[1] = "fixed"; break;
	case LAYOUT_HINTSIZE: ptr[1] = "hintsize"; break;
	case LAYOUT_FILL: ptr[1] = "fill"; break;
	case LAYOUT_FREE: ptr[1] = "free"; break;
	}
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

MQWidgetBase::LAYOUT_TYPE MQWidgetBase::GetVertLayout()
{
	char *layout_str = NULL;
	void *ptr[3];
	ptr[0] = "vertlayout";
	ptr[1] = &layout_str;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(strcmp(layout_str, "auto") == 0) return LAYOUT_AUTO;
	if(strcmp(layout_str, "fixed") == 0) return LAYOUT_FIXED;
	if(strcmp(layout_str, "hintsize") == 0) return LAYOUT_HINTSIZE;
	if(strcmp(layout_str, "fill") == 0) return LAYOUT_FILL;
	if(strcmp(layout_str, "free") == 0) return LAYOUT_FREE;
	return LAYOUT_AUTO;
}

void MQWidgetBase::SetVertLayout(LAYOUT_TYPE value)
{
	void *ptr[3];
	ptr[0] = "vertlayout";
	switch(value){
	default:
	case LAYOUT_AUTO: ptr[1] = "auto"; break;
	case LAYOUT_FIXED: ptr[1] = "fixed"; break;
	case LAYOUT_HINTSIZE: ptr[1] = "hintsize"; break;
	case LAYOUT_FILL: ptr[1] = "fill"; break;
	case LAYOUT_FREE: ptr[1] = "free"; break;
	}
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQWidgetBase::GetWidth()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "width";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWidgetBase::SetWidth(int value)
{
#ifdef _DEBUG
	assert(dynamic_cast<MQWindowBase*>(this) != NULL || GetHorzLayout() == LAYOUT_FIXED);
#endif

	void *ptr[3];
	ptr[0] = "width";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQWidgetBase::GetHeight()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "height";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWidgetBase::SetHeight(int value)
{
#ifdef _DEBUG
	assert(dynamic_cast<MQWindowBase*>(this) != NULL || GetVertLayout() == LAYOUT_FIXED);
#endif

	void *ptr[3];
	ptr[0] = "height";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQWidgetBase::GetFillRateX()
{
	double value = 0;
	void *ptr[3];
	ptr[0] = "fillrate_x";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWidgetBase::SetFillRateX(double value)
{
	void *ptr[3];
	ptr[0] = "fillrate_x";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQWidgetBase::GetFillRateY()
{
	double value = 0;
	void *ptr[3];
	ptr[0] = "fillrate_y";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWidgetBase::SetFillRateY(double value)
{
	void *ptr[3];
	ptr[0] = "fillrate_y";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQWidgetBase::GetFillBeforeRate()
{
	double value = 0;
	void *ptr[3];
	ptr[0] = "fillbeforerate";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWidgetBase::SetFillBeforeRate(double value)
{
	void *ptr[3];
	ptr[0] = "fillbeforerate";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQWidgetBase::GetFillAfterRate()
{
	double value = 0;
	void *ptr[3];
	ptr[0] = "fillafterrate";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWidgetBase::SetFillAfterRate(double value)
{
	void *ptr[3];
	ptr[0] = "fillafterrate";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQWidgetBase::GetInSpace()
{
	double value = 0;
	void *ptr[3];
	ptr[0] = "inspace";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWidgetBase::SetInSpace(double value)
{
	void *ptr[3];
	ptr[0] = "inspace";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQWidgetBase::GetOutSpace()
{
	double value = 0;
	void *ptr[3];
	ptr[0] = "outspace";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWidgetBase::SetOutSpace(double value)
{
	void *ptr[3];
	ptr[0] = "outspace";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQWidgetBase::GetHintSizeRateX()
{
	double value = 0;
	void *ptr[3];
	ptr[0] = "hintsizeratex";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWidgetBase::SetHintSizeRateX(double value)
{
	void *ptr[3];
	ptr[0] = "hintsizeratex";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQWidgetBase::GetHintSizeRateY()
{
	double value = 0;
	void *ptr[3];
	ptr[0] = "hintsizeratey";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWidgetBase::SetHintSizeRateY(double value)
{
	void *ptr[3];
	ptr[0] = "hintsizeratey";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQWidgetBase::GetCellColumn()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "cellcolumn";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWidgetBase::SetCellColumn(int value)
{
	void *ptr[3];
	ptr[0] = "cellcolumn";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

std::wstring MQWidgetBase::GetHintText()
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "hinttext.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "hinttext";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQWidgetBase::SetHintText(const std::wstring& text)
{
	void *ptr[3];
	ptr[0] = "hinttext";
	ptr[1] = (void*)text.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

POINT MQWidgetBase::GetJustSize(int max_width, int max_height)
{
	int values[4] = {0, 0, max_width, max_height};
	void *ptr[3];
	ptr[0] = "justsize";
	ptr[1] = values;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	POINT pt;
	pt.x = values[0];
	pt.y = values[1];
	return pt;
}

bool MQWidgetBase::ClientToScreen(int client_x, int client_y, int *screen_x, int *screen_y)
{
	bool result = false;
	void *ptr[13];
	ptr[0] = "clienttoscreen";
	ptr[1] = NULL;
	ptr[2] = "client_x";
	ptr[3] = &client_x;
	ptr[4] = "client_y";
	ptr[5] = &client_y;
	ptr[6] = "screen_x";
	ptr[7] = screen_x;
	ptr[8] = "screen_y";
	ptr[9] = screen_y;
	ptr[10] = "result";
	ptr[11] = &result;
	ptr[12] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);

	return result;
}

bool MQWidgetBase::ScreenToClient(int screen_x, int screen_y, int *client_x, int *client_y)
{
	bool result = false;
	void *ptr[13];
	ptr[0] = "screentoclient";
	ptr[1] = NULL;
	ptr[2] = "screen_x";
	ptr[3] = &screen_x;
	ptr[4] = "screen_y";
	ptr[5] = &screen_y;
	ptr[6] = "client_x";
	ptr[7] = client_x;
	ptr[8] = "client_y";
	ptr[9] = client_y;
	ptr[10] = "result";
	ptr[11] = &result;
	ptr[12] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);

	return result;
}

bool MQWidgetBase::ClientToClient(int client_x, int client_y, MQWidgetBase *target, int *target_x, int *target_y)
{
	if(target == NULL){
		*target_x = client_x;
		*target_y = client_y;
		return false;
	}

	int target_id = target->GetID();
	bool result = false;
	void *ptr[15];
	ptr[0] = "clienttoclient";
	ptr[1] = NULL;
	ptr[2] = "client_x";
	ptr[3] = &client_x;
	ptr[4] = "client_y";
	ptr[5] = &client_y;
	ptr[6] = "target";
	ptr[7] = &target_id;
	ptr[8] = "target_x";
	ptr[9] = target_x;
	ptr[10] = "target_y";
	ptr[11] = target_y;
	ptr[12] = "result";
	ptr[13] = &result;
	ptr[14] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);

	return result;
}

void MQWidgetBase::Repaint(bool immediate)
{
	void *ptr[5];
	ptr[0] = "repaint";
	ptr[1] = NULL;
	ptr[2] = "immediate";
	ptr[3] = &immediate;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

void MQWidgetBase::RefreshPaint()
{
	void *ptr[3];
	ptr[0] = "refreshpaint";
	ptr[1] = NULL;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

void MQWidgetBase::CaptureMouse(bool value)
{
	void *ptr[5];
	ptr[0] = "capturemouse";
	ptr[1] = NULL;
	ptr[2] = "value";
	ptr[3] = &value;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

MQWidgetBase *MQWidgetBase::FindWidgetByID(int id)
{
	std::map<int, MQWidgetBase*>::iterator it = s_WidgetIDMap.find(id);
	if(it != s_WidgetIDMap.end()){
		return (*it).second;
	}
	return NULL;
}

int MQWidgetBase::GetSystemWidgetID(MQSystemWidget::WidgetType type)
{
	void *ptr[5];
	int result = -1;

	ptr[0] = "system";
	switch(type){
	case MQSystemWidget::MainWindow: ptr[1] = "main_window"; break;
	case MQSystemWidget::OptionPanel: ptr[1] = "option_panel"; break;
	default: return NullID;
	}
	ptr[2] = "result";
	ptr[3] = &result;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_FIND, (void*)ptr);
	
	return result;
}

int MQWidgetBase::GetBaseRateSize(double rate)
{
	int result = 0;
	void *ptr[5];
	ptr[0] = "baseratesize";
	ptr[1] = &rate;
	ptr[2] = "baseratesize.result";
	ptr[3] = &result;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_GET, (void*)ptr);

	return result;
}

void MQWidgetBase::GetDefaultFrameColor(int& r, int& g, int& b, int& a)
{
	int array[4] = {0,0,0,0};
	void *ptr[3];
	ptr[0] = "defaultframecolor.rgba";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(NullID, MQWIDGET_GET, (void*)ptr);

	r = array[0];
	g = array[1];
	b = array[2];
	a = array[3];
}

void MQWidgetBase::GetDefaultTextColor(int& r, int& g, int& b, int& a)
{
	int array[4] = {0,0,0,0};
	void *ptr[3];
	ptr[0] = "defaulttextcolor.rgba";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(NullID, MQWIDGET_GET, (void*)ptr);

	r = array[0];
	g = array[1];
	b = array[2];
	a = array[3];
}

void MQWidgetBase::GetDefaultTitleBackColor(int& r, int& g, int& b, int& a)
{
	int array[4] = {0,0,0,0};
	void *ptr[3];
	ptr[0] = "defaulttitlebackcolor.rgba";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(NullID, MQWIDGET_GET, (void*)ptr);

	r = array[0];
	g = array[1];
	b = array[2];
	a = array[3];
}

void MQWidgetBase::GetDefaultTitleTextColor(int& r, int& g, int& b, int& a)
{
	int array[4] = {0,0,0,0};
	void *ptr[3];
	ptr[0] = "defaulttitletextcolor.rgba";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(NullID, MQWIDGET_GET, (void*)ptr);

	r = array[0];
	g = array[1];
	b = array[2];
	a = array[3];
}

void MQWidgetBase::GetDefaultListBackColor(int& r, int& g, int& b, int& a)
{
	int array[4] = {0,0,0,0};
	void *ptr[3];
	ptr[0] = "defaultlistbackcolor.rgba";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(NullID, MQWIDGET_GET, (void*)ptr);

	r = array[0];
	g = array[1];
	b = array[2];
	a = array[3];
}

void MQWidgetBase::GetDefaultListTextColor(int& r, int& g, int& b, int& a)
{
	int array[4] = {0,0,0,0};
	void *ptr[3];
	ptr[0] = "defaultlisttextcolor.rgba";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(NullID, MQWIDGET_GET, (void*)ptr);

	r = array[0];
	g = array[1];
	b = array[2];
	a = array[3];
}

void MQWidgetBase::GetDefaultListActiveColor(int& r, int& g, int& b, int& a)
{
	int array[4] = {0,0,0,0};
	void *ptr[3];
	ptr[0] = "defaultlistactivecolor.rgba";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(NullID, MQWIDGET_GET, (void*)ptr);

	r = array[0];
	g = array[1];
	b = array[2];
	a = array[3];
}

void MQWidgetBase::GetDefaultListActiveTextColor(int& r, int& g, int& b, int& a)
{
	int array[4] = {0,0,0,0};
	void *ptr[3];
	ptr[0] = "defaultlistactivetextcolor.rgba";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(NullID, MQWIDGET_GET, (void*)ptr);

	r = array[0];
	g = array[1];
	b = array[2];
	a = array[3];
}

void MQWidgetBase::GetDefaultEditBackColor(int& r, int& g, int& b, int& a)
{
	int array[4] = {0,0,0,0};
	void *ptr[3];
	ptr[0] = "defaulteditbackcolor.rgba";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(NullID, MQWIDGET_GET, (void*)ptr);

	r = array[0];
	g = array[1];
	b = array[2];
	a = array[3];
}

void MQWidgetBase::GetDefaultEditTextColor(int& r, int& g, int& b, int& a)
{
	int array[4] = {0,0,0,0};
	void *ptr[3];
	ptr[0] = "defaultedittextcolor.rgba";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(NullID, MQWIDGET_GET, (void*)ptr);

	r = array[0];
	g = array[1];
	b = array[2];
	a = array[3];
}

void MQWidgetBase::GetDefaultEditSelectionColor(int& r, int& g, int& b, int& a)
{
	int array[4] = {0,0,0,0};
	void *ptr[3];
	ptr[0] = "defaulteditselectioncolor.rgba";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(NullID, MQWIDGET_GET, (void*)ptr);

	r = array[0];
	g = array[1];
	b = array[2];
	a = array[3];
}

void MQWidgetBase::GetDefaultEditSelectionTextColor(int& r, int& g, int& b, int& a)
{
	int array[4] = {0,0,0,0};
	void *ptr[3];
	ptr[0] = "defaulteditselectiontextcolor.rgba";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(NullID, MQWIDGET_GET, (void*)ptr);

	r = array[0];
	g = array[1];
	b = array[2];
	a = array[3];
}

MQCanvasColor MQWidgetBase::GetDefaultFrameColor()
{
	MQCanvasColor col;
	GetDefaultFrameColor(col.r, col.g, col.b, col.a);
	return col;
}

MQCanvasColor MQWidgetBase::GetDefaultTextColor()
{
	MQCanvasColor col;
	GetDefaultTextColor(col.r, col.g, col.b, col.a);
	return col;
}

MQCanvasColor MQWidgetBase::GetDefaultTitleBackColor()
{
	MQCanvasColor col;
	GetDefaultTitleBackColor(col.r, col.g, col.b, col.a);
	return col;
}

MQCanvasColor MQWidgetBase::GetDefaultTitleTextColor()
{
	MQCanvasColor col;
	GetDefaultTitleTextColor(col.r, col.g, col.b, col.a);
	return col;
}

MQCanvasColor MQWidgetBase::GetDefaultListBackColor()
{
	MQCanvasColor col;
	GetDefaultListBackColor(col.r, col.g, col.b, col.a);
	return col;
}

MQCanvasColor MQWidgetBase::GetDefaultListTextColor()
{
	MQCanvasColor col;
	GetDefaultListTextColor(col.r, col.g, col.b, col.a);
	return col;
}

MQCanvasColor MQWidgetBase::GetDefaultListActiveColor()
{
	MQCanvasColor col;
	GetDefaultListActiveColor(col.r, col.g, col.b, col.a);
	return col;
}

MQCanvasColor MQWidgetBase::GetDefaultListActiveTextColor()
{
	MQCanvasColor col;
	GetDefaultListActiveTextColor(col.r, col.g, col.b, col.a);
	return col;
}

MQCanvasColor MQWidgetBase::GetDefaultEditBackColor()
{
	MQCanvasColor col;
	GetDefaultEditBackColor(col.r, col.g, col.b, col.a);
	return col;
}

MQCanvasColor MQWidgetBase::GetDefaultEditTextColor()
{
	MQCanvasColor col;
	GetDefaultEditTextColor(col.r, col.g, col.b, col.a);
	return col;
}

MQCanvasColor MQWidgetBase::GetDefaultEditSelectionColor()
{
	MQCanvasColor col;
	GetDefaultEditSelectionColor(col.r, col.g, col.b, col.a);
	return col;
}

MQCanvasColor MQWidgetBase::GetDefaultEditSelectionTextColor()
{
	MQCanvasColor col;
	GetDefaultEditSelectionTextColor(col.r, col.g, col.b, col.a);
	return col;
}


void MQWidgetBase::RegisterSubCommandButton(MQStationPlugin *plugin, MQButton *button, const char *command_str)
{
	MQSendMessageInfo info;
	void *array[5];
	BOOL result = FALSE;
	int id = button->GetID();

	array[0] = "widget";
	array[1] = &id;
	array[2] = "string";
	array[3] = (void*)command_str;
	array[4] = NULL;

	plugin->GetPlugInID(&info.Product, &info.ID);
	info.option = array;

	MQ_SendMessage(MQMESSAGE_REGISTER_KEYBUTTON, &info);
}

void MQWidgetBase::EnterEventLoop(unsigned int timeout_ms, bool empty_return)
{
	void *ptr[7];
	ptr[0] = "type";
	ptr[1] = "entereventloop";
	ptr[2] = "timeout_ms";
	ptr[3] = &timeout_ms;
	ptr[4] = "empty_return";
	ptr[5] = &empty_return;
	ptr[6] = NULL;
	MQWidget_Value(NullID, MQWIDGET_EXECUTE, (void*)ptr);
}

void MQWidgetBase::ExitEventLoop()
{
	void *ptr[3];
	ptr[0] = "type";
	ptr[1] = "exiteventloop";
	ptr[2] = NULL;
	MQWidget_Value(NullID, MQWIDGET_EXECUTE, (void*)ptr);
}

void MQWidgetBase::AddEventCallback(const char *event_type, MQWidgetSharedPtr<MQWidgetEventClosureBase> closure, bool prior)
{
	void *ptr[9];

	ptr[0] = "type";
	ptr[1] = (void*)event_type;
	ptr[2] = "proc";
	ptr[3] = EventCallback;
	ptr[4] = "pointer";
	ptr[5] = closure.get();
	ptr[6] = "prior";
	ptr[7] = &prior;
	ptr[8] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_ADD_EVENT, (void*)ptr);

	m_Events.push_back(closure);
}

void MQWidgetBase::RemoveEventCallback(const char *event_type, MQWidgetSharedPtr<MQWidgetEventClosureBase> closure)
{
	std::vector<MQWidgetSharedPtr<MQWidgetEventClosureBase>>::iterator it;
	for(it = m_Events.begin(); it != m_Events.end(); ++it){
		if((*it)->isEqual(closure.get())){
			void *ptr[7];

			ptr[0] = "type";
			ptr[1] = (void*)event_type;
			ptr[2] = "proc";
			ptr[3] = EventCallback;
			ptr[4] = "pointer";
			ptr[5] = (*it).get();
			ptr[6] = NULL;
			MQWidget_Value(m_ID, MQWIDGET_REMOVE_EVENT, (void*)ptr);

			m_Events.erase(it);
			break;
		}
	}
}

bool MQWidgetBase::ExistsEventCallback(const char *event_type, MQWidgetSharedPtr<MQWidgetEventClosureBase> closure) const
{
	std::vector<MQWidgetSharedPtr<MQWidgetEventClosureBase>>::const_iterator it;
	for(it = m_Events.begin(); it != m_Events.end(); ++it){
		if((*it)->isEqual(closure.get())){
			return true;
		}
	}
	return false;
}

void MQWidgetBase::AddTimerEventCallback(MQWidgetSharedPtr<MQWidgetEventClosureBase> closure, unsigned int timeout_ms, bool overwrite)
{
	void *ptr[11];

	ptr[0] = "type";
	ptr[1] = "timer";
	ptr[2] = "proc";
	ptr[3] = EventCallback;
	ptr[4] = "pointer";
	ptr[5] = closure.get();
	ptr[6] = "timeout_ms";
	ptr[7] = &timeout_ms;
	ptr[8] = "overwrite";
	ptr[9] = &overwrite;
	ptr[10] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_ADD_EVENT, (void*)ptr);

	if(overwrite){
		std::vector<MQWidgetSharedPtr<MQWidgetEventClosureBase>>::iterator it;
		for(it = m_Events.begin(); it != m_Events.end(); ++it){
			if((*it)->isEqual(closure.get())){
				return;
			}
		}
	}
	m_Events.push_back(closure);
}

BOOL MQAPICALL MQWidgetBase::EventCallback(MQDocument doc, void **params, void *option)
{
	int target_id = ExtractEventOptionInt(params, "target_id", NullID);
	int sender_id = ExtractEventOptionInt(params, "sender_id", NullID);
	bool *finished = (bool*)ExtractEventOption(params, "finished");
	const char *type = (const char *)ExtractEventOption(params, "type");
	if(type == NULL) return FALSE;

	MQWidgetBase *target = FindWidgetByID(target_id);
	if(target != NULL){
		for(size_t i=0; i<target->m_Events.size(); i++){
			if(target->m_Events[i].get() == option){
				MQWidgetBase *sender = FindWidgetByID(sender_id);
				if( strcmp(type, "leftdown") == 0 || 
					strcmp(type, "leftup") == 0 || 
					strcmp(type, "leftdoubleclick") == 0 || 
					strcmp(type, "middledown") == 0 || 
					strcmp(type, "middleup") == 0 || 
					strcmp(type, "middledoubleclick") == 0 || 
					strcmp(type, "rightdown") == 0 || 
					strcmp(type, "rightup") == 0 || 
					strcmp(type, "rightdoubleclick") == 0 || 
					strcmp(type, "mousemove") == 0 || 
					strcmp(type, "mousewheel") == 0 )
				{
					MQWidgetMouseParam param;
					param.ClientPos.x = ExtractEventOptionInt(params, "client_x", 0);
					param.ClientPos.y = ExtractEventOptionInt(params, "client_y", 0);
					param.ScreenPos.x = ExtractEventOptionInt(params, "screen_x", 0);
					param.ScreenPos.y = ExtractEventOptionInt(params, "screen_y", 0);
					param.DownPos.x = ExtractEventOptionInt(params, "down_x", 0);
					param.DownPos.y = ExtractEventOptionInt(params, "down_y", 0);
					param.LastPos.x = ExtractEventOptionInt(params, "last_x", 0);
					param.LastPos.y = ExtractEventOptionInt(params, "last_y", 0);
					param.Wheel = ExtractEventOptionInt(params, "wheel_delta", 0);
					param.Pressure = ExtractEventOptionFloat(params, "pressure", 0.0f);
					param.Finished = false;

					DWORD button_state = (DWORD)ExtractEventOptionInt(params, "button_state", 0);
					param.LButton = (button_state & MK_LBUTTON) ? TRUE : FALSE;
					param.MButton = (button_state & MK_MBUTTON) ? TRUE : FALSE;
					param.RButton = (button_state & MK_RBUTTON) ? TRUE : FALSE;
					param.Shift = (button_state & MK_SHIFT) ? TRUE : FALSE;
					param.Ctrl = (button_state & MK_CONTROL) ? TRUE : FALSE;
					param.Alt = (button_state & MK_ALT) ? TRUE : FALSE;

					BOOL result = target->m_Events[i]->invoke(sender, doc, &param);
					if(param.Finished && finished != NULL){
						*finished = true;
					}
					return result;
				}
				else if(
					strcmp(type, "keydown") == 0 || 
					strcmp(type, "keyup") == 0 )
				{
					MQWidgetKeyParam param;
					param.Key = ExtractEventOptionInt(params, "key", 0);
					param.AutoRepeat = ExtractEventOptionInt(params, "auto_repeat", 0) != 0;

					DWORD button_state = (DWORD)ExtractEventOptionInt(params, "button_state", 0);
					param.LButton = (button_state & MK_LBUTTON) ? TRUE : FALSE;
					param.MButton = (button_state & MK_MBUTTON) ? TRUE : FALSE;
					param.RButton = (button_state & MK_RBUTTON) ? TRUE : FALSE;
					param.Shift = (button_state & MK_SHIFT) ? TRUE : FALSE;
					param.Ctrl = (button_state & MK_CONTROL) ? TRUE : FALSE;
					param.Alt = (button_state & MK_ALT) ? TRUE : FALSE;
					param.Finished = false;

					BOOL result = target->m_Events[i]->invoke(sender, doc, &param);
					if(param.Finished && finished != NULL){
						*finished = true;
					}
					return result;
				}
				else if(strcmp(type, "paint") == 0)
				{
					MQWidgetPaintParam param;
					param.Canvas = new MQCanvas(ExtractEventOption(params, "canvas"));

					BOOL result = target->m_Events[i]->invoke(sender, doc, &param);
					delete param.Canvas;
					return result;
				}
				else if(strcmp(type, "listdrawitem") == 0)
				{
					int *px = (int*)ExtractEventOption(params, "x");
					int *py = (int*)ExtractEventOption(params, "y");
					int *pw = (int*)ExtractEventOption(params, "width");
					int *ph = (int*)ExtractEventOption(params, "height");

					MQListBoxDrawItemParam param;
					param.Canvas = new MQCanvas(ExtractEventOption(params, "canvas"));
					param.ItemIndex = ExtractEventOptionInt(params, "index", -1);
					param.X = *px;
					param.Y = *py;
					param.Width = *pw;
					param.Height = *ph;

					BOOL result = target->m_Events[i]->invoke(sender, doc, &param);
					delete param.Canvas;
					*px = param.X;
					*py = param.Y;
					*pw = param.Width;
					*ph = param.Height;
					return result;
				}
				else if(strcmp(type, "treelistdrawitem") == 0)
				{
					int *px = (int*)ExtractEventOption(params, "x");
					int *py = (int*)ExtractEventOption(params, "y");
					int *pw = (int*)ExtractEventOption(params, "width");
					int *ph = (int*)ExtractEventOption(params, "height");

					MQTreeListBoxDrawItemParam param;
					param.Canvas = new MQCanvas(ExtractEventOption(params, "canvas"));
					param.ItemIndex = ExtractEventOptionInt(params, "index", -1);
					param.ItemID = ExtractEventOptionInt(params, "id", -1);
					param.X = *px;
					param.Y = *py;
					param.Width = *pw;
					param.Height = *ph;

					BOOL result = target->m_Events[i]->invoke(sender, doc, &param);
					delete param.Canvas;
					*px = param.X;
					*py = param.Y;
					*pw = param.Width;
					*ph = param.Height;
					return result;
				}
				else if(strcmp(type, "timer") == 0)
				{
					MQWidgetSharedPtr<MQWidgetEventClosureBase> ev = target->m_Events[i];
					target->m_Events.erase(target->m_Events.begin() + i);
					return ev->invoke(sender, doc, NULL);
				}
				else if(strcmp(type, "dragover") == 0)
				{
					MQWidgetDragOverParam param;
					param.ClientPos.x = ExtractEventOptionInt(params, "client_x", 0);
					param.ClientPos.y = ExtractEventOptionInt(params, "client_y", 0);
					param.ScreenPos.x = ExtractEventOptionInt(params, "screen_x", 0);
					param.ScreenPos.y = ExtractEventOptionInt(params, "screen_y", 0);
					bool *result_ptr = (bool*)ExtractEventOption(params, "result");
					param.Result = *result_ptr;

					BOOL result = target->m_Events[i]->invoke(sender, doc, &param);
					*result_ptr = param.Result;
					return result;
				}
				else if(strcmp(type, "dropfiles") == 0)
				{
					MQWidgetDropFilesParam param;
					param.ClientPos.x = ExtractEventOptionInt(params, "client_x", 0);
					param.ClientPos.y = ExtractEventOptionInt(params, "client_y", 0);
					param.ScreenPos.x = ExtractEventOptionInt(params, "screen_x", 0);
					param.ScreenPos.y = ExtractEventOptionInt(params, "screen_y", 0);
					const wchar_t **files = (const wchar_t **)ExtractEventOption(params, "files");
					for(; *files != nullptr; files++){
						param.Files.push_back(*files);
					}

					BOOL result = target->m_Events[i]->invoke(sender, doc, &param);
					return result;
				}
				else
				{
					return target->m_Events[i]->invoke(sender, doc, NULL);
				}
			}
		}
	}

	return FALSE;
}

//---------------------------------------------------------------------------
//  class MQFrameBase
//---------------------------------------------------------------------------
MQFrameBase::MQFrameBase() : MQWidgetBase()
{
}

MQFrameBase::MQFrameBase(int id) : MQWidgetBase(id)
{
}

MQFrameBase::~MQFrameBase()
{
}

MQFrameBase::MQFrameAlignment MQFrameBase::GetAlignment()
{
	char *align_str = NULL;
	void *ptr[3];
	ptr[0] = "alignment";
	ptr[1] = &align_str;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(strcmp(align_str, "horz") == 0) return ALIGN_HORIZONTAL;
	if(strcmp(align_str, "vert") == 0) return ALIGN_VERTICAL;
	return ALIGN_NONE;
}

void MQFrameBase::SetAlignment(MQFrameAlignment align)
{
	void *ptr[3];
	ptr[0] = "alignment";
	switch(align){
	case ALIGN_HORIZONTAL: ptr[1] = "horz"; break;
	case ALIGN_VERTICAL:   ptr[1] = "vert"; break;
	default: ptr[1] = "none"; break;
	}
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQFrameBase::GetMultiColumn()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "multicolumn";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQFrameBase::SetMultiColumn(bool value)
{
	void *ptr[3];
	ptr[0] = "multicolumn";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQFrameBase::GetMatrixColumn()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "matrixcolumn";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQFrameBase::SetMatrixColumn(int value)
{
	void *ptr[3];
	ptr[0] = "matrixcolumn";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQFrameBase::GetUniformSize()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "uniformsize";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQFrameBase::SetUniformSize(bool value)
{
	void *ptr[3];
	ptr[0] = "uniformsize";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQFrameBase::GetSplit()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "split";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQFrameBase::SetSplit(bool value)
{
	void *ptr[3];
	ptr[0] = "split";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQFrame
//---------------------------------------------------------------------------
MQFrame::MQFrame() : MQFrameBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "frame";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQFrame::MQFrame(int id) : MQFrameBase(id)
{
}

MQFrame::~MQFrame()
{
}

void MQFrame::GetBackColor(int& r, int& g, int& b, int& a)
{
	int array[4] = {0,0,0,0};
	void *ptr[3];
	ptr[0] = "backcolor.rgba";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	r = array[0];
	g = array[1];
	b = array[2];
	a = array[3];
}

void MQFrame::SetBackColor(int r, int g, int b, int a)
{
	int array[4] = {r,g,b,a};
	void *ptr[9];
	ptr[0] = "backcolor.rgba";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQGroupBox
//---------------------------------------------------------------------------
MQGroupBox::MQGroupBox() : MQFrameBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "groupbox";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQGroupBox::MQGroupBox(int id) : MQFrameBase(id)
{
}

MQGroupBox::~MQGroupBox()
{
}

std::wstring MQGroupBox::GetText()
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "text.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "text";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQGroupBox::SetText(const std::wstring& text)
{
	void *ptr[3];
	ptr[0] = "text";
	ptr[1] = (void*)text.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQGroupBox::GetCanMinimize()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "canminimize";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQGroupBox::SetCanMinimize(bool value)
{
	void *ptr[3];
	ptr[0] = "canminimize";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQGroupBox::GetMinimized()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "minimized";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQGroupBox::SetMinimized(bool value)
{
	void *ptr[3];
	ptr[0] = "minimized";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQGroupBox::GetShowTitle()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "showtitle";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQGroupBox::SetShowTitle(bool value)
{
	void *ptr[3];
	ptr[0] = "showtitle";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQScrollBox
//---------------------------------------------------------------------------
MQScrollBox::MQScrollBox() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "scrollbox";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQScrollBox::MQScrollBox(int id) : MQWidgetBase(id)
{
}

MQScrollBox::~MQScrollBox()
{
}

MQScrollBox::MQScrollBoxBarStatus MQScrollBox::GetHorzBarStatus()
{
	char *status_str = NULL;
	void *ptr[3];
	ptr[0] = "horzbarstatus";
	ptr[1] = &status_str;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(strcmp(status_str, "auto") == 0) return MQScrollBox::SCROLLBAR_AUTO;
	if(strcmp(status_str, "on") == 0) return MQScrollBox::SCROLLBAR_ON;
	if(strcmp(status_str, "off") == 0) return MQScrollBox::SCROLLBAR_OFF;
	return MQScrollBox::SCROLLBAR_AUTO;
}

void MQScrollBox::SetHorzBarStatus(MQScrollBoxBarStatus value)
{
	void *ptr[3];
	ptr[0] = "horzbarstatus";
	switch(value){
	default:
	case SCROLLBAR_AUTO: ptr[1] = "auto"; break;
	case SCROLLBAR_OFF:  ptr[1] = "off"; break;
	case SCROLLBAR_ON:   ptr[1] = "on"; break;
	}
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

MQScrollBox::MQScrollBoxBarStatus MQScrollBox::GetVertBarStatus()
{
	char *status_str = NULL;
	int value = false;
	void *ptr[3];
	ptr[0] = "vertbarstatus";
	ptr[1] = &status_str;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(strcmp(status_str, "auto") == 0) return MQScrollBox::SCROLLBAR_AUTO;
	if(strcmp(status_str, "on") == 0) return MQScrollBox::SCROLLBAR_ON;
	if(strcmp(status_str, "off") == 0) return MQScrollBox::SCROLLBAR_OFF;
	return MQScrollBox::SCROLLBAR_AUTO;
}

void MQScrollBox::SetVertBarStatus(MQScrollBoxBarStatus value)
{
	void *ptr[3];
	ptr[0] = "vertbarstatus";
	switch(value){
	default:
	case SCROLLBAR_AUTO: ptr[1] = "auto"; break;
	case SCROLLBAR_OFF:  ptr[1] = "off"; break;
	case SCROLLBAR_ON:   ptr[1] = "on"; break;
	}
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQScrollBox::GetAutoWidgetScroll()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "autowidgetscroll";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQScrollBox::SetAutoWidgetScroll(bool value)
{
	void *ptr[3];
	ptr[0] = "autowidgetscroll";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}



//---------------------------------------------------------------------------
//  class MQTab
//---------------------------------------------------------------------------
MQTab::MQTab() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "tab";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQTab::MQTab(int id) : MQWidgetBase(id)
{
}

MQTab::~MQTab()
{
}

std::wstring MQTab::GetTabTitle(int index)
{
	void *ptr[5];
	unsigned int length = 0;

	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "title.text.length";
	ptr[3] = &length;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[2] = "title.text";
	ptr[3] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQTab::SetTabTitle(int index, const std::wstring& text)
{
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "title.text";
	ptr[3] = (void*)text.c_str();
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQTab::GetCurrentPage()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "currentpage";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQTab::SetCurrentPage(int value)
{
	void *ptr[3];
	ptr[0] = "currentpage";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQTab::GetShowTab()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "showtab";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQTab::SetShowTab(bool value)
{
	void *ptr[3];
	ptr[0] = "showtab";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQTab::GetExclusive()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "exclusive";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQTab::SetExclusive(bool value)
{
	void *ptr[3];
	ptr[0] = "exclusive";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQButton
//---------------------------------------------------------------------------
MQButton::MQButton() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "button";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQButton::MQButton(int id) : MQWidgetBase(id)
{
}

MQButton::~MQButton()
{
}

std::wstring MQButton::GetText()
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "text.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "text";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQButton::SetText(const std::wstring& text)
{
	void *ptr[3];
	ptr[0] = "text";
	ptr[1] = (void*)text.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

std::wstring MQButton::GetFontName()
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "fontname.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "fontname";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQButton::SetFontName(const std::wstring& value)
{
	void *ptr[3];
	ptr[0] = "fontname";
	ptr[1] = (void*)value.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQButton::GetFontBold()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "fontbold";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQButton::SetFontBold(bool value)
{
	void *ptr[3];
	ptr[0] = "fontbold";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQButton::GetFontColor(int& r, int& g, int& b, int& a)
{
	int value[4] = {0,0,0,0};
	void *ptr[3];
	ptr[0] = "fontcolor.rgba";
	ptr[1] = value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	r = value[0];
	g = value[1];
	b = value[2];
	a = value[3];
}

void MQButton::SetFontColor(int r, int g, int b, int a)
{
	int value[4] = {r,g,b,a};
	void *ptr[3];
	ptr[0] = "fontcolor.rgba";
	ptr[1] = value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQButton::GetFontScale()
{
	double value = 0.0;
	void *ptr[3];
	ptr[0] = "fontscale";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQButton::SetFontScale(double value)
{
	void *ptr[3];
	ptr[0] = "fontscale";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

MQButton::MQButtonTextAlignment MQButton::GetAlignment()
{
	char *align_str = NULL;
	void *ptr[3];
	ptr[0] = "alignment";
	ptr[1] = &align_str;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(strcmp(align_str, "left") == 0) return ALIGN_LEFT;
	if(strcmp(align_str, "center") == 0) return ALIGN_CENTER;
	if(strcmp(align_str, "right") == 0) return ALIGN_RIGHT;
	if(strcmp(align_str, "center_except_image") == 0) return ALIGN_CENTER_EXCEPT_IMAGE;
	return ALIGN_LEFT;
}

void MQButton::SetAlignment(MQButtonTextAlignment value)
{
	void *ptr[3];
	ptr[0] = "alignment";
	switch(value){
	default:
	case ALIGN_LEFT:   ptr[1] = "left"; break;
	case ALIGN_CENTER: ptr[1] = "center"; break;
	case ALIGN_RIGHT:  ptr[1] = "right"; break;
	case ALIGN_CENTER_EXCEPT_IMAGE: ptr[1] = "center_except_image"; break;
	}
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQButton::GetToggle()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "toggle";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQButton::SetToggle(bool value)
{
	void *ptr[3];
	ptr[0] = "toggle";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQButton::GetDown()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "down";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQButton::SetDown(bool value)
{
	void *ptr[3];
	ptr[0] = "down";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQButton::GetRepeat()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "repeat";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQButton::SetRepeat(bool value)
{
	void *ptr[3];
	ptr[0] = "repeat";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQButton::GetChain()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "chain";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQButton::SetChain(bool value)
{
	void *ptr[3];
	ptr[0] = "chain";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQButton::GetPaddingX()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "paddingx";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQButton::SetPaddingX(double value)
{
	void *ptr[3];
	ptr[0] = "paddingx";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQButton::GetPaddingY()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "paddingy";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQButton::SetPaddingY(double value)
{
	void *ptr[3];
	ptr[0] = "paddingy";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQButton::SetDefault(bool value)
{
	void *ptr[3];
	ptr[0] = "default";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQButton::SetCancel(bool value)
{
	void *ptr[3];
	ptr[0] = "cancel";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQButton::SetModalResult(MQDialog::DIALOG_RESULT value)
{
	void *ptr[3];
	ptr[0] = "modalresult";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQButton::SetSystemSVGFile(const wchar_t *filename)
{
	void *ptr[3];
	ptr[0] = "systemsvgfile";
	ptr[1] = (void*)filename;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQButton::SetImageScale(double value)
{
	void *ptr[3];
	ptr[0] = "imagescale";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQButton::GetImageScale()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "imagescale";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

MQButton::MQButtonImagePosition MQButton::GetImagePosition()
{
	char *align_str = NULL;
	void *ptr[3];
	ptr[0] = "imagepos";
	ptr[1] = &align_str;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(strcmp(align_str, "left") == 0) return IMAGE_LEFT;
	if(strcmp(align_str, "right") == 0) return IMAGE_RIGHT;
	if(strcmp(align_str, "top") == 0) return IMAGE_TOP;
	if(strcmp(align_str, "bottom") == 0) return IMAGE_BOTTOM;
	return IMAGE_LEFT;
}

void MQButton::SetImagePosition(MQButtonImagePosition value)
{
	void *ptr[3];
	ptr[0] = "imagepos";
	switch(value){
	default:
	case IMAGE_LEFT:   ptr[1] = "left"; break;
	case IMAGE_RIGHT:  ptr[1] = "right"; break;
	case IMAGE_TOP:    ptr[1] = "top"; break;
	case IMAGE_BOTTOM: ptr[1] = "bottom"; break;
	}
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQCheckBox
//---------------------------------------------------------------------------
MQCheckBox::MQCheckBox() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "checkbox";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQCheckBox::MQCheckBox(int id) : MQWidgetBase(id)
{
}

MQCheckBox::~MQCheckBox()
{
}

bool MQCheckBox::GetChecked()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "checked";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQCheckBox::SetChecked(bool value)
{
	void *ptr[3];
	ptr[0] = "checked";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

std::wstring MQCheckBox::GetText()
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "text.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "text";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQCheckBox::SetText(const std::wstring& text)
{
	void *ptr[3];
	ptr[0] = "text";
	ptr[1] = (void*)text.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQRadioButton
//---------------------------------------------------------------------------
MQRadioButton::MQRadioButton() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "radiobutton";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQRadioButton::MQRadioButton(int id) : MQWidgetBase(id)
{
}

MQRadioButton::~MQRadioButton()
{
}

bool MQRadioButton::GetChecked()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "checked";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQRadioButton::SetChecked(bool value)
{
	void *ptr[3];
	ptr[0] = "checked";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

std::wstring MQRadioButton::GetText()
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "text.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "text";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQRadioButton::SetText(const std::wstring& text)
{
	void *ptr[3];
	ptr[0] = "text";
	ptr[1] = (void*)text.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQComboBox
//---------------------------------------------------------------------------
MQComboBox::MQComboBox() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "combobox";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQComboBox::MQComboBox(int id) : MQWidgetBase(id)
{
}

MQComboBox::~MQComboBox()
{
}

int MQComboBox::GetCurrentIndex()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "currentindex";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQComboBox::SetCurrentIndex(int value)
{
	void *ptr[3];
	ptr[0] = "currentindex";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQComboBox::AddItem(const std::wstring& text)
{
	int result = -1;
	void *ptr[7];
	ptr[0] = "additem";
	ptr[1] = NULL;
	ptr[2] = "text";
	ptr[3] = (void*)text.c_str();
	ptr[4] = "result";
	ptr[5] = &result;
	ptr[6] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
	return result;
}

int MQComboBox::AddItem(const std::wstring& text, __int64 tag)
{
	int result = -1;
	void *ptr[9];
	ptr[0] = "additem";
	ptr[1] = NULL;
	ptr[2] = "text";
	ptr[3] = (void*)text.c_str();
	ptr[4] = "result";
	ptr[5] = &result;
	ptr[6] = "tag";
	ptr[7] = &tag;
	ptr[8] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
	return result;
}

void MQComboBox::DeleteItem(int index)
{
	void *ptr[5];
	ptr[0] = "deleteitem";
	ptr[1] = NULL;
	ptr[2] = "index";
	ptr[3] = &index;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

void MQComboBox::ClearItems()
{
	void *ptr[3];
	ptr[0] = "clearitems";
	ptr[1] = NULL;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

int MQComboBox::GetItemCount()
{
	int value = 0;
	void *ptr[3];

	ptr[0] = "item.count";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);
	return value;
}

std::wstring MQComboBox::GetItem(int index)
{
	void *ptr[5];
	unsigned int length = 0;

	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.text.length";
	ptr[3] = &length;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[2] = "item.text";
	ptr[3] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQComboBox::SetItem(int index, const std::wstring& text)
{
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.text";
	ptr[3] = (void*)text.c_str();
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

__int64 MQComboBox::GetItemTag(int index)
{
	__int64 value = 0;
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.tag";
	ptr[3] = &value;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQComboBox::SetItemTag(int index, __int64 tag)
{
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.tag";
	ptr[3] = &tag;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQComboBox::GetNumVisible()
{
	int  value = 0;
	void *ptr[3];
	ptr[0] = "numvisible";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQComboBox::SetNumVisible(int value)
{
	void *ptr[3];
	ptr[0] = "numvisible";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQListBox
//---------------------------------------------------------------------------
MQListBox::MQListBox() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "listbox";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQListBox::MQListBox(int id) : MQWidgetBase(id)
{
}

MQListBox::~MQListBox()
{
}

int MQListBox::GetCurrentIndex()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "currentindex";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQListBox::SetCurrentIndex(int value)
{
	void *ptr[3];
	ptr[0] = "currentindex";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQListBox::BeginUpdate()
{
	void *ptr[3];
	ptr[0] = "beginupdate";
	ptr[1] = NULL;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

void MQListBox::EndUpdate()
{
	void *ptr[3];
	ptr[0] = "endupdate";
	ptr[1] = NULL;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

int MQListBox::AddItem(const std::wstring& text)
{
	int result = -1;
	void *ptr[7];
	ptr[0] = "additem";
	ptr[1] = NULL;
	ptr[2] = "text";
	ptr[3] = (void*)text.c_str();
	ptr[4] = "result";
	ptr[5] = &result;
	ptr[6] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
	return result;
}

int MQListBox::AddItem(const std::wstring& text, __int64 tag)
{
	int result = -1;
	void *ptr[9];
	ptr[0] = "additem";
	ptr[1] = NULL;
	ptr[2] = "text";
	ptr[3] = (void*)text.c_str();
	ptr[4] = "result";
	ptr[5] = &result;
	ptr[6] = "tag";
	ptr[7] = &tag;
	ptr[8] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
	return result;
}

void MQListBox::DeleteItem(int index)
{
	void *ptr[5];
	ptr[0] = "deleteitem";
	ptr[1] = NULL;
	ptr[2] = "index";
	ptr[3] = &index;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

void MQListBox::ClearItems()
{
	void *ptr[3];
	ptr[0] = "clearitems";
	ptr[1] = NULL;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

int MQListBox::GetItemCount()
{
	int value = 0;
	void *ptr[3];

	ptr[0] = "item.count";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);
	return value;
}

std::wstring MQListBox::GetItem(int index)
{
	void *ptr[5];
	unsigned int length = 0;

	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.text.length";
	ptr[3] = &length;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[2] = "item.text";
	ptr[3] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQListBox::SetItem(int index, const std::wstring& text)
{
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.text";
	ptr[3] = (void*)text.c_str();
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

__int64 MQListBox::GetItemTag(int index)
{
	__int64 value = 0;
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.tag";
	ptr[3] = &value;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQListBox::SetItemTag(int index, __int64 tag)
{
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.tag";
	ptr[3] = &tag;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQListBox::GetItemSelected(int index)
{
	bool value = false;
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.selected";
	ptr[3] = &value;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQListBox::SetItemSelected(int index, bool selected)
{
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.selected";
	ptr[3] = &selected;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

std::wstring MQListBox::GetItemHint(int index)
{
	void *ptr[5];
	unsigned int length = 0;

	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.hint.length";
	ptr[3] = &length;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[2] = "item.hint";
	ptr[3] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQListBox::SetItemHint(int index, const std::wstring& hint)
{
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.hint";
	ptr[3] = (void*)hint.c_str();
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQListBox::GetVisibleRow()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "visiblerow";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQListBox::SetVisibleRow(int value)
{
	void *ptr[3];
	ptr[0] = "visiblerow";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQListBox::GetLineHeightRate()
{
	double value = 1.0;
	void *ptr[3];
	ptr[0] = "lineheightrate";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQListBox::SetLineHeightRate(double value)
{
	void *ptr[3];
	ptr[0] = "lineheightrate";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQListBox::GetVertScrollVisible()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "vertscrollvisible";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQListBox::SetVertScrollVisible(bool value)
{
	void *ptr[3];
	ptr[0] = "vertscrollvisible";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQListBox::GetMultiSelect()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "multiselect";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQListBox::SetMultiSelect(bool value)
{
	void *ptr[3];
	ptr[0] = "multiselect";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQListBox::ClearSelection()
{
	void *ptr[3];
	ptr[0] = "clearselection";
	ptr[1] = NULL;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

void MQListBox::MakeItemVisible(int index)
{
	void *ptr[5];
	ptr[0] = "makeitemvisible";
	ptr[1] = NULL;
	ptr[2] = "index";
	ptr[3] = &index;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

int MQListBox::HitTestItem(int x, int y)
{
	int result = -1;
	void *ptr[9];
	ptr[0] = "hittestitem";
	ptr[1] = NULL;
	ptr[2] = "x";
	ptr[3] = &x;
	ptr[4] = "y";
	ptr[5] = &y;
	ptr[6] = "result";
	ptr[7] = &result;
	ptr[8] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
	return result;
}

void MQListBox::GetItemRect(int index, int& x, int& y, int& w, int& h)
{
	void *ptr[13];
	ptr[0] = "getitemrect";
	ptr[1] = NULL;
	ptr[2] = "index";
	ptr[3] = &index;
	ptr[4] = "x";
	ptr[5] = &x;
	ptr[6] = "y";
	ptr[7] = &y;
	ptr[8] = "w";
	ptr[9] = &w;
	ptr[10] = "h";
	ptr[11] = &h;
	ptr[12] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQCheckListBox
//---------------------------------------------------------------------------
MQCheckListBox::MQCheckListBox() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "checklistbox";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQCheckListBox::MQCheckListBox(int id) : MQWidgetBase(id)
{
}

MQCheckListBox::~MQCheckListBox()
{
}

int MQCheckListBox::GetCurrentIndex()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "currentindex";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQCheckListBox::SetCurrentIndex(int value)
{
	void *ptr[3];
	ptr[0] = "currentindex";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQCheckListBox::BeginUpdate()
{
	void *ptr[3];
	ptr[0] = "beginupdate";
	ptr[1] = NULL;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

void MQCheckListBox::EndUpdate()
{
	void *ptr[3];
	ptr[0] = "endupdate";
	ptr[1] = NULL;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

int MQCheckListBox::AddItem(const std::wstring& text)
{
	int result = -1;
	void *ptr[7];
	ptr[0] = "additem";
	ptr[1] = NULL;
	ptr[2] = "text";
	ptr[3] = (void*)text.c_str();
	ptr[4] = "result";
	ptr[5] = &result;
	ptr[6] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
	return result;
}

int MQCheckListBox::AddItem(const std::wstring& text, __int64 tag)
{
	int result = -1;
	void *ptr[9];
	ptr[0] = "additem";
	ptr[1] = NULL;
	ptr[2] = "text";
	ptr[3] = (void*)text.c_str();
	ptr[4] = "result";
	ptr[5] = &result;
	ptr[6] = "tag";
	ptr[7] = &tag;
	ptr[8] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
	return result;
}

void MQCheckListBox::DeleteItem(int index)
{
	void *ptr[5];
	ptr[0] = "deleteitem";
	ptr[1] = NULL;
	ptr[2] = "index";
	ptr[3] = &index;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

void MQCheckListBox::ClearItems()
{
	void *ptr[3];
	ptr[0] = "clearitems";
	ptr[1] = NULL;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

int MQCheckListBox::GetItemCount()
{
	int value = 0;
	void *ptr[3];

	ptr[0] = "item.count";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);
	return value;
}

std::wstring MQCheckListBox::GetItem(int index)
{
	void *ptr[5];
	unsigned int length = 0;

	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.text.length";
	ptr[3] = &length;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[2] = "item.text";
	ptr[3] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQCheckListBox::SetItem(int index, const std::wstring& text)
{
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.text";
	ptr[3] = (void*)text.c_str();
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

__int64 MQCheckListBox::GetItemTag(int index)
{
	__int64 value = 0;
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.tag";
	ptr[3] = &value;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQCheckListBox::SetItemTag(int index, __int64 tag)
{
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.tag";
	ptr[3] = &tag;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQCheckListBox::GetItemSelected(int index)
{
	bool value = false;
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.selected";
	ptr[3] = &value;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQCheckListBox::SetItemSelected(int index, bool selected)
{
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.selected";
	ptr[3] = &selected;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQCheckListBox::GetItemChecked(int index)
{
	bool value = false;
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.checked";
	ptr[3] = &value;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQCheckListBox::SetItemChecked(int index, bool checked)
{
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.checked";
	ptr[3] = &checked;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

std::wstring MQCheckListBox::GetItemHint(int index)
{
	void *ptr[5];
	unsigned int length = 0;

	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.hint.length";
	ptr[3] = &length;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[2] = "item.hint";
	ptr[3] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQCheckListBox::SetItemHint(int index, const std::wstring& hint)
{
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "item.hint";
	ptr[3] = (void*)hint.c_str();
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQCheckListBox::GetVisibleRow()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "visiblerow";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQCheckListBox::SetVisibleRow(int value)
{
	void *ptr[3];
	ptr[0] = "visiblerow";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQCheckListBox::GetLineHeightRate()
{
	double value = 1.0;
	void *ptr[3];
	ptr[0] = "lineheightrate";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQCheckListBox::SetLineHeightRate(double value)
{
	void *ptr[3];
	ptr[0] = "lineheightrate";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQCheckListBox::GetVertScrollVisible()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "vertscrollvisible";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQCheckListBox::SetVertScrollVisible(bool value)
{
	void *ptr[3];
	ptr[0] = "vertscrollvisible";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQCheckListBox::GetMultiSelect()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "multiselect";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQCheckListBox::SetMultiSelect(bool value)
{
	void *ptr[3];
	ptr[0] = "multiselect";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQCheckListBox::ClearSelection()
{
	void *ptr[3];
	ptr[0] = "clearselection";
	ptr[1] = NULL;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

void MQCheckListBox::MakeItemVisible(int index)
{
	void *ptr[5];
	ptr[0] = "makeitemvisible";
	ptr[1] = NULL;
	ptr[2] = "index";
	ptr[3] = &index;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

int MQCheckListBox::HitTestItem(int x, int y)
{
	int result = -1;
	void *ptr[9];
	ptr[0] = "hittestitem";
	ptr[1] = NULL;
	ptr[2] = "x";
	ptr[3] = &x;
	ptr[4] = "y";
	ptr[5] = &y;
	ptr[6] = "result";
	ptr[7] = &result;
	ptr[8] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
	return result;
}

void MQCheckListBox::GetItemRect(int index, int& x, int& y, int& w, int& h)
{
	void *ptr[13];
	ptr[0] = "getitemrect";
	ptr[1] = NULL;
	ptr[2] = "index";
	ptr[3] = &index;
	ptr[4] = "x";
	ptr[5] = &x;
	ptr[6] = "y";
	ptr[7] = &y;
	ptr[8] = "w";
	ptr[9] = &w;
	ptr[10] = "h";
	ptr[11] = &h;
	ptr[12] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQTreeListBox
//---------------------------------------------------------------------------
MQTreeListBox::MQTreeListBox() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "treelistbox";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQTreeListBox::MQTreeListBox(int id) : MQWidgetBase(id)
{
}

MQTreeListBox::~MQTreeListBox()
{
}

int MQTreeListBox::GetCurrentID()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "currentid";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQTreeListBox::SetCurrentID(int value)
{
	void *ptr[3];
	ptr[0] = "currentid";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQTreeListBox::BeginUpdate()
{
	void *ptr[3];
	ptr[0] = "beginupdate";
	ptr[1] = NULL;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

void MQTreeListBox::EndUpdate()
{
	void *ptr[3];
	ptr[0] = "endupdate";
	ptr[1] = NULL;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

int MQTreeListBox::AddItem(const std::wstring& text)
{
	int result = -1;
	void *ptr[7];
	ptr[0] = "additem";
	ptr[1] = NULL;
	ptr[2] = "text";
	ptr[3] = (void*)text.c_str();
	ptr[4] = "result";
	ptr[5] = &result;
	ptr[6] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
	return result;
}

int MQTreeListBox::AddItem(const std::wstring& text, __int64 tag)
{
	int result = -1;
	void *ptr[9];
	ptr[0] = "additem";
	ptr[1] = NULL;
	ptr[2] = "text";
	ptr[3] = (void*)text.c_str();
	ptr[4] = "result";
	ptr[5] = &result;
	ptr[6] = "tag";
	ptr[7] = &tag;
	ptr[8] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
	return result;
}

int MQTreeListBox::AddItem(int parent_id, const std::wstring& text)
{
	int result = -1;
	void *ptr[9];
	ptr[0] = "additem";
	ptr[1] = NULL;
	ptr[2] = "parent";
	ptr[3] = &parent_id;
	ptr[4] = "text";
	ptr[5] = (void*)text.c_str();
	ptr[6] = "result";
	ptr[7] = &result;
	ptr[8] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
	return result;
}

int MQTreeListBox::AddItem(int parent_id, const std::wstring& text, __int64 tag)
{
	int result = -1;
	void *ptr[11];
	ptr[0] = "additem";
	ptr[1] = NULL;
	ptr[2] = "parent";
	ptr[3] = &parent_id;
	ptr[4] = "text";
	ptr[5] = (void*)text.c_str();
	ptr[6] = "result";
	ptr[7] = &result;
	ptr[8] = "tag";
	ptr[9] = &tag;
	ptr[10] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
	return result;
}

void MQTreeListBox::DeleteItem(int id)
{
	void *ptr[5];
	ptr[0] = "deleteitem";
	ptr[1] = NULL;
	ptr[2] = "id";
	ptr[3] = &id;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

void MQTreeListBox::ClearItems()
{
	void *ptr[3];
	ptr[0] = "clearitems";
	ptr[1] = NULL;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

void MQTreeListBox::ReparentItem(int id, int new_parent_id)
{
	void *ptr[5];
	ptr[0] = "id";
	ptr[1] = &id;
	ptr[2] = "parent";
	ptr[3] = &new_parent_id;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQTreeListBox::GetItemCount()
{
	int value = 0;
	void *ptr[3];

	ptr[0] = "item.count";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);
	return value;
}

int MQTreeListBox::GetItemIDByIndex(int index)
{
	int id = -1;
	void *ptr[5];
	ptr[0] = "id";
	ptr[1] = &id;
	ptr[2] = "index";
	ptr[3] = &index;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return id;
}

int MQTreeListBox::GetItemParent(int id)
{
	int value = -1;
	void *ptr[5];
	ptr[0] = "id";
	ptr[1] = &id;
	ptr[2] = "parent";
	ptr[3] = &value;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

std::vector<int> MQTreeListBox::GetItemChildren(int id)
{
	int num = 0;
	void *ptr[5];
	ptr[0] = "id";
	ptr[1] = &id;
	ptr[2] = "child.num";
	ptr[3] = &num;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);
	if(num == 0) return std::vector<int>();

	std::vector<int> children(num);
	ptr[0] = "id";
	ptr[1] = &id;
	ptr[2] = "children";
	ptr[3] = &(*children.begin());
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return children;
}

std::wstring MQTreeListBox::GetItem(int id)
{
	void *ptr[5];
	unsigned int length = 0;

	ptr[0] = "id";
	ptr[1] = &id;
	ptr[2] = "item.text.length";
	ptr[3] = &length;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[2] = "item.text";
	ptr[3] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQTreeListBox::SetItem(int id, const std::wstring& text)
{
	void *ptr[5];
	ptr[0] = "id";
	ptr[1] = &id;
	ptr[2] = "item.text";
	ptr[3] = (void*)text.c_str();
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

__int64 MQTreeListBox::GetItemTag(int id)
{
	__int64 value = 0;
	void *ptr[5];
	ptr[0] = "id";
	ptr[1] = &id;
	ptr[2] = "item.tag";
	ptr[3] = &value;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQTreeListBox::SetItemTag(int id, __int64 tag)
{
	void *ptr[5];
	ptr[0] = "id";
	ptr[1] = &id;
	ptr[2] = "item.tag";
	ptr[3] = &tag;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQTreeListBox::GetItemSelected(int id)
{
	bool value = false;
	void *ptr[5];
	ptr[0] = "id";
	ptr[1] = &id;
	ptr[2] = "item.selected";
	ptr[3] = &value;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQTreeListBox::SetItemSelected(int id, bool selected)
{
	void *ptr[5];
	ptr[0] = "id";
	ptr[1] = &id;
	ptr[2] = "item.selected";
	ptr[3] = &selected;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

std::wstring MQTreeListBox::GetItemHint(int id)
{
	void *ptr[5];
	unsigned int length = 0;

	ptr[0] = "id";
	ptr[1] = &id;
	ptr[2] = "item.hint.length";
	ptr[3] = &length;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[2] = "item.hint";
	ptr[3] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQTreeListBox::SetItemHint(int id, const std::wstring& hint)
{
	void *ptr[5];
	ptr[0] = "id";
	ptr[1] = &id;
	ptr[2] = "item.hint";
	ptr[3] = (void*)hint.c_str();
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQTreeListBox::GetItemCollapsed(int id)
{
	bool value = false;
	void *ptr[5];
	ptr[0] = "id";
	ptr[1] = &id;
	ptr[2] = "item.collapsed";
	ptr[3] = &value;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQTreeListBox::SetItemCollapsed(int id, bool collapsed)
{
	void *ptr[5];
	ptr[0] = "id";
	ptr[1] = &id;
	ptr[2] = "item.collapsed";
	ptr[3] = &collapsed;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQTreeListBox::GetVisibleRow()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "visiblerow";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQTreeListBox::SetVisibleRow(int value)
{
	void *ptr[3];
	ptr[0] = "visiblerow";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQTreeListBox::GetLineHeightRate()
{
	double value = 1.0;
	void *ptr[3];
	ptr[0] = "lineheightrate";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQTreeListBox::SetLineHeightRate(double value)
{
	void *ptr[3];
	ptr[0] = "lineheightrate";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQTreeListBox::GetHorzScrollVisible()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "horzscrollvisible";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQTreeListBox::SetHorzScrollVisible(bool value)
{
	void *ptr[3];
	ptr[0] = "horzscrollvisible";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQTreeListBox::GetVertScrollVisible()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "vertscrollvisible";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQTreeListBox::SetVertScrollVisible(bool value)
{
	void *ptr[3];
	ptr[0] = "vertscrollvisible";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQTreeListBox::GetMultiSelect()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "multiselect";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQTreeListBox::SetMultiSelect(bool value)
{
	void *ptr[3];
	ptr[0] = "multiselect";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQTreeListBox::ClearSelection()
{
	void *ptr[3];
	ptr[0] = "clearselection";
	ptr[1] = NULL;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

void MQTreeListBox::MakeItemVisible(int id)
{
	void *ptr[5];
	ptr[0] = "makeitemvisible";
	ptr[1] = NULL;
	ptr[2] = "id";
	ptr[3] = &id;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

int MQTreeListBox::HitTestItem(int x, int y)
{
	int result = -1;
	void *ptr[9];
	ptr[0] = "hittestitem";
	ptr[1] = NULL;
	ptr[2] = "x";
	ptr[3] = &x;
	ptr[4] = "y";
	ptr[5] = &y;
	ptr[6] = "result";
	ptr[7] = &result;
	ptr[8] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
	return result;
}

void MQTreeListBox::GetItemRect(int id, int& x, int& y, int& w, int& h)
{
	void *ptr[13];
	ptr[0] = "getitemrect";
	ptr[1] = NULL;
	ptr[2] = "id";
	ptr[3] = &id;
	ptr[4] = "x";
	ptr[5] = &x;
	ptr[6] = "y";
	ptr[7] = &y;
	ptr[8] = "w";
	ptr[9] = &w;
	ptr[10] = "h";
	ptr[11] = &h;
	ptr[12] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQLabel
//---------------------------------------------------------------------------
MQLabel::MQLabel() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "label";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQLabel::MQLabel(int id) : MQWidgetBase(id)
{
}

MQLabel::~MQLabel()
{
}

std::wstring MQLabel::GetText()
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "text.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "text";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQLabel::SetText(const std::wstring& text)
{
	void *ptr[3];
	ptr[0] = "text";
	ptr[1] = (void*)text.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

std::wstring MQLabel::GetFontName()
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "fontname.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "fontname";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQLabel::SetFontName(const std::wstring& value)
{
	void *ptr[3];
	ptr[0] = "fontname";
	ptr[1] = (void*)value.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQLabel::GetFontBold()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "fontbold";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQLabel::SetFontBold(bool value)
{
	void *ptr[3];
	ptr[0] = "fontbold";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQLabel::GetFontColor(int& r, int& g, int& b, int& a)
{
	int value[4] = {0,0,0,0};
	void *ptr[3];
	ptr[0] = "fontcolor.rgba";
	ptr[1] = value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	r = value[0];
	g = value[1];
	b = value[2];
	a = value[3];
}

void MQLabel::SetFontColor(int r, int g, int b, int a)
{
	int value[4] = {r,g,b,a};
	void *ptr[3];
	ptr[0] = "fontcolor.rgba";
	ptr[1] = value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQLabel::GetFontScale()
{
	double value = 0.0;
	void *ptr[3];
	ptr[0] = "fontscale";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQLabel::SetFontScale(double value)
{
	void *ptr[3];
	ptr[0] = "fontscale";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

MQLabel::MQLabelTextAlignment MQLabel::GetAlignment()
{
	char *align_str = NULL;
	void *ptr[3];
	ptr[0] = "alignment";
	ptr[1] = &align_str;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(strcmp(align_str, "left") == 0) return ALIGN_LEFT;
	if(strcmp(align_str, "center") == 0) return ALIGN_CENTER;
	if(strcmp(align_str, "right") == 0) return ALIGN_RIGHT;
	return ALIGN_LEFT;
}

void MQLabel::SetAlignment(MQLabelTextAlignment value)
{
	void *ptr[3];
	ptr[0] = "alignment";
	switch(value){
	default:
	case ALIGN_LEFT:   ptr[1] = "left"; break;
	case ALIGN_CENTER: ptr[1] = "center"; break;
	case ALIGN_RIGHT:  ptr[1] = "right"; break;
	}
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQLabel::GetVerticalText()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "verticaltext";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQLabel::SetVerticalText(bool value)
{
	void *ptr[3];
	ptr[0] = "verticaltext";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQLabel::GetWordWrap()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "wordwrap";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQLabel::SetWordWrap(bool value)
{
	void *ptr[3];
	ptr[0] = "wordwrap";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQLabel::GetFrame()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "frame";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQLabel::SetFrame(bool value)
{
	void *ptr[3];
	ptr[0] = "frame";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQLabel::GetShadowText()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "shadowtext";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQLabel::SetShadowText(bool value)
{
	void *ptr[3];
	ptr[0] = "shadowtext";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQEdit
//---------------------------------------------------------------------------
MQEdit::MQEdit() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "edit";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQEdit::MQEdit(int id) : MQWidgetBase(id)
{
}

MQEdit::~MQEdit()
{
}

std::wstring MQEdit::GetText()
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "text.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "text";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQEdit::SetText(const std::wstring& text)
{
	void *ptr[3];
	ptr[0] = "text";
	ptr[1] = (void*)text.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

std::wstring MQEdit::GetFontName()
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "fontname.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "fontname";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQEdit::SetFontName(const std::wstring& value)
{
	void *ptr[3];
	ptr[0] = "fontname";
	ptr[1] = (void*)value.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQEdit::GetFontBold()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "fontbold";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQEdit::SetFontBold(bool value)
{
	void *ptr[3];
	ptr[0] = "fontbold";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQEdit::GetFontScale()
{
	double value = 0.0;
	void *ptr[3];
	ptr[0] = "fontscale";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQEdit::SetFontScale(double value)
{
	void *ptr[3];
	ptr[0] = "fontscale";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQEdit::GetReadOnly()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "readonly";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQEdit::SetReadOnly(bool value)
{
	void *ptr[3];
	ptr[0] = "readonly";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQEdit::GetPassword()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "password";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQEdit::SetPassword(bool value)
{
	void *ptr[3];
	ptr[0] = "password";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

MQEdit::MQEditTextAlignment MQEdit::GetAlignment()
{
	char *align_str = NULL;
	void *ptr[3];
	ptr[0] = "alignment";
	ptr[1] = &align_str;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(strcmp(align_str, "left") == 0) return ALIGN_LEFT;
	if(strcmp(align_str, "right") == 0) return ALIGN_RIGHT;
	return ALIGN_LEFT;
}

void MQEdit::SetAlignment(MQEditTextAlignment value)
{
	void *ptr[3];
	ptr[0] = "alignment";
	switch(value){
	default:
	case ALIGN_LEFT:   ptr[1] = "left"; break;
	case ALIGN_RIGHT:  ptr[1] = "right"; break;
	}
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

MQEdit::MQEditNumericType MQEdit::GetNumeric()
{
	char *align_str = NULL;
	void *ptr[3];
	ptr[0] = "numeric";
	ptr[1] = &align_str;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(strcmp(align_str, "int") == 0) return NUMERIC_INT;
	if(strcmp(align_str, "double") == 0) return NUMERIC_DOUBLE;
	return NUMERIC_TEXT;
}

void MQEdit::SetNumeric(MQEditNumericType value)
{
	void *ptr[3];
	ptr[0] = "numeric";
	switch(value){
	default:
	case NUMERIC_TEXT:  ptr[1] = "text"; break;
	case NUMERIC_INT:   ptr[1] = "int"; break;
	case NUMERIC_DOUBLE:ptr[1] = "double"; break;
	}
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQEdit::GetMaxLength()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "maxlength";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQEdit::SetMaxLength(int value)
{
	void *ptr[3];
	ptr[0] = "maxlength";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQEdit::GetMaxAnsiLength()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "maxansilength";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQEdit::SetMaxAnsiLength(int value)
{
	void *ptr[3];
	ptr[0] = "maxansilength";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQEdit::GetVisibleColumn()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "visiblecolumn";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQEdit::SetVisibleColumn(int value)
{
	void *ptr[3];
	ptr[0] = "visiblecolumn";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQEdit::SetFocus(bool value)
{
	void *ptr[3];
	ptr[0] = "focus";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQMemo
//---------------------------------------------------------------------------
MQMemo::MQMemo() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "memo";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQMemo::MQMemo(int id) : MQWidgetBase(id)
{
}

MQMemo::~MQMemo()
{
}

void MQMemo::AddLine(const std::wstring& text)
{
	void *ptr[5];
	ptr[0] = "addline";
	ptr[1] = NULL;
	ptr[2] = "text";
	ptr[3] = (void*)text.c_str();
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

std::wstring MQMemo::GetLine(int line_index)
{
	wchar_t *result = NULL;
	void *ptr[7];
	unsigned int length = 0;
	ptr[0] = "getline.length";
	ptr[1] = NULL;
	ptr[2] = "line";
	ptr[3] = &line_index;
	ptr[4] = "result";
	ptr[5] = &length;
	ptr[6] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "getline";
	ptr[1] = NULL;
	ptr[2] = "line";
	ptr[3] = &line_index;
	ptr[4] = "text";
	ptr[5] = buf;
	ptr[6] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

std::wstring MQMemo::GetText()
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "text.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "text";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQMemo::SetText(const std::wstring& text)
{
	void *ptr[3];
	ptr[0] = "text";
	ptr[1] = (void*)text.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

std::wstring MQMemo::GetFontName()
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "fontname.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "fontname";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQMemo::SetFontName(const std::wstring& value)
{
	void *ptr[3];
	ptr[0] = "fontname";
	ptr[1] = (void*)value.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQMemo::GetFontBold()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "fontbold";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQMemo::SetFontBold(bool value)
{
	void *ptr[3];
	ptr[0] = "fontbold";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQMemo::GetFontScale()
{
	double value = 0.0;
	void *ptr[3];
	ptr[0] = "fontscale";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQMemo::SetFontScale(double value)
{
	void *ptr[3];
	ptr[0] = "fontscale";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQMemo::GetReadOnly()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "readonly";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQMemo::SetReadOnly(bool value)
{
	void *ptr[3];
	ptr[0] = "readonly";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQMemo::SetFocus(bool value)
{
	void *ptr[3];
	ptr[0] = "focus";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQSpinBox
//---------------------------------------------------------------------------
MQSpinBox::MQSpinBox() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "spinbox";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQSpinBox::MQSpinBox(int id) : MQWidgetBase(id)
{
}

MQSpinBox::~MQSpinBox()
{
}

int MQSpinBox::GetPosition()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "position";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQSpinBox::SetPosition(int value)
{
	void *ptr[3];
	ptr[0] = "position";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQSpinBox::GetMin()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "min";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQSpinBox::SetMin(int value)
{
	void *ptr[3];
	ptr[0] = "min";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQSpinBox::GetMax()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "max";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQSpinBox::SetMax(int value)
{
	void *ptr[3];
	ptr[0] = "max";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQSpinBox::GetIncrement()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "increment";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQSpinBox::SetIncrement(int value)
{
	void *ptr[3];
	ptr[0] = "increment";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

MQSpinBox::MQSpinBoxTextAlignment MQSpinBox::GetAlignment()
{
	char *align_str = NULL;
	void *ptr[3];
	ptr[0] = "alignment";
	ptr[1] = &align_str;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(strcmp(align_str, "left") == 0) return ALIGN_LEFT;
	if(strcmp(align_str, "right") == 0) return ALIGN_RIGHT;
	return ALIGN_LEFT;
}

void MQSpinBox::SetAlignment(MQSpinBoxTextAlignment value)
{
	void *ptr[3];
	ptr[0] = "alignment";
	switch(value){
	default:
	case ALIGN_LEFT:   ptr[1] = "left"; break;
	case ALIGN_RIGHT:  ptr[1] = "right"; break;
	}
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQSpinBox::GetVisibleColumn()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "visiblecolumn";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQSpinBox::SetVisibleColumn(int value)
{
	void *ptr[3];
	ptr[0] = "visiblecolumn";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQSpinBox::SetFocus(bool value)
{
	void *ptr[3];
	ptr[0] = "focus";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQDoubleSpinBox
//---------------------------------------------------------------------------
MQDoubleSpinBox::MQDoubleSpinBox() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "doublespinbox";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQDoubleSpinBox::MQDoubleSpinBox(int id) : MQWidgetBase(id)
{
}

MQDoubleSpinBox::~MQDoubleSpinBox()
{
}

double MQDoubleSpinBox::GetPosition()
{
	double value = 0;
	void *ptr[3];
	ptr[0] = "position";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQDoubleSpinBox::SetPosition(double value)
{
	void *ptr[3];
	ptr[0] = "position";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQDoubleSpinBox::GetMin()
{
	double value = 0;
	void *ptr[3];
	ptr[0] = "min";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQDoubleSpinBox::SetMin(double value)
{
	void *ptr[3];
	ptr[0] = "min";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQDoubleSpinBox::GetMax()
{
	double value = 0;
	void *ptr[3];
	ptr[0] = "max";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQDoubleSpinBox::SetMax(double value)
{
	void *ptr[3];
	ptr[0] = "max";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQDoubleSpinBox::GetIncrement()
{
	double value = 0;
	void *ptr[3];
	ptr[0] = "increment";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQDoubleSpinBox::SetIncrement(double value)
{
	void *ptr[3];
	ptr[0] = "increment";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQDoubleSpinBox::GetExponential()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "exponential";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQDoubleSpinBox::SetExponential(bool value)
{
	void *ptr[3];
	ptr[0] = "exponential";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQDoubleSpinBox::GetMantissa()
{
	double value = 0.0;
	void *ptr[3];
	ptr[0] = "mantissa";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQDoubleSpinBox::SetMantissa(double value)
{
	void *ptr[3];
	ptr[0] = "mantissa";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQDoubleSpinBox::GetDecimalDigit()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "decimaldigit";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQDoubleSpinBox::SetDecimalDigit(int value)
{
	void *ptr[3];
	ptr[0] = "decimaldigit";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQDoubleSpinBox::GetVariableDigit()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "variabledigit";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQDoubleSpinBox::SetVariableDigit(bool value)
{
	void *ptr[3];
	ptr[0] = "variabledigit";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQDoubleSpinBox::GetAutoDigit()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "autodigit";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQDoubleSpinBox::SetAutoDigit(int value)
{
	void *ptr[3];
	ptr[0] = "autodigit";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQDoubleSpinBox::GetMaxDecimalDigit()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "maxdecimaldigit";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQDoubleSpinBox::SetMaxDecimalDigit(int value)
{
	void *ptr[3];
	ptr[0] = "maxdecimaldigit";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

MQDoubleSpinBox::MQDoubleSpinBoxTextAlignment MQDoubleSpinBox::GetAlignment()
{
	char *align_str = NULL;
	void *ptr[3];
	ptr[0] = "alignment";
	ptr[1] = &align_str;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(strcmp(align_str, "left") == 0) return ALIGN_LEFT;
	if(strcmp(align_str, "right") == 0) return ALIGN_RIGHT;
	return ALIGN_LEFT;
}

void MQDoubleSpinBox::SetAlignment(MQDoubleSpinBoxTextAlignment value)
{
	void *ptr[3];
	ptr[0] = "alignment";
	switch(value){
	default:
	case ALIGN_LEFT:   ptr[1] = "left"; break;
	case ALIGN_RIGHT:  ptr[1] = "right"; break;
	}
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQDoubleSpinBox::GetVisibleColumn()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "visiblecolumn";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQDoubleSpinBox::SetVisibleColumn(int value)
{
	void *ptr[3];
	ptr[0] = "visiblecolumn";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQDoubleSpinBox::SetFocus(bool value)
{
	void *ptr[3];
	ptr[0] = "focus";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQSlider
//---------------------------------------------------------------------------
MQSlider::MQSlider() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "slider";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQSlider::MQSlider(int id) : MQWidgetBase(id)
{
}

MQSlider::~MQSlider()
{
}

double MQSlider::GetPosition()
{
	double value = 0;
	void *ptr[3];
	ptr[0] = "position";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQSlider::SetPosition(double value)
{
	void *ptr[3];
	ptr[0] = "position";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQSlider::GetMin()
{
	double value = 0;
	void *ptr[3];
	ptr[0] = "min";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQSlider::SetMin(double value)
{
	void *ptr[3];
	ptr[0] = "min";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQSlider::GetMax()
{
	double value = 0;
	void *ptr[3];
	ptr[0] = "max";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQSlider::SetMax(double value)
{
	void *ptr[3];
	ptr[0] = "max";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQProgressBar
//---------------------------------------------------------------------------
MQProgressBar::MQProgressBar() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "progressbar";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQProgressBar::MQProgressBar(int id) : MQWidgetBase(id)
{
}

MQProgressBar::~MQProgressBar()
{
}

double MQProgressBar::GetPosition()
{
	double value = 0;
	void *ptr[3];
	ptr[0] = "position";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQProgressBar::SetPosition(double value)
{
	void *ptr[3];
	ptr[0] = "position";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQProgressBar::GetMin()
{
	double value = 0;
	void *ptr[3];
	ptr[0] = "min";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQProgressBar::SetMin(double value)
{
	void *ptr[3];
	ptr[0] = "min";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

double MQProgressBar::GetMax()
{
	double value = 0;
	void *ptr[3];
	ptr[0] = "max";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQProgressBar::SetMax(double value)
{
	void *ptr[3];
	ptr[0] = "max";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQScrollBar
//---------------------------------------------------------------------------
MQScrollBar::MQScrollBar() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "scrollbar";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQScrollBar::MQScrollBar(int id) : MQWidgetBase(id)
{
}

MQScrollBar::~MQScrollBar()
{
}

MQScrollBar::MQScrollBarDirection MQScrollBar::GetDirection()
{
	char *dir_str = NULL;
	void *ptr[3];
	ptr[0] = "direction";
	ptr[1] = &dir_str;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(strcmp(dir_str, "horz") == 0) return DIRECTION_HORIZONTAL;
	if(strcmp(dir_str, "vert") == 0) return DIRECTION_VERTICAL;
	return DIRECTION_HORIZONTAL;
}

void MQScrollBar::SetDirection(MQScrollBarDirection value)
{
	void *ptr[3];
	ptr[0] = "direction";
	switch(value){
	default:
	case DIRECTION_HORIZONTAL:ptr[1] = "horz"; break;
	case DIRECTION_VERTICAL:  ptr[1] = "vert"; break;
	}
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQScrollBar::GetPosition()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "position";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQScrollBar::SetPosition(int value)
{
	void *ptr[3];
	ptr[0] = "position";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQScrollBar::GetMin()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "min";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQScrollBar::SetMin(int value)
{
	void *ptr[3];
	ptr[0] = "min";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQScrollBar::GetMax()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "max";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQScrollBar::SetMax(int value)
{
	void *ptr[3];
	ptr[0] = "max";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQScrollBar::GetPage()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "page";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQScrollBar::SetPage(int value)
{
	void *ptr[3];
	ptr[0] = "page";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQScrollBar::GetIncrement()
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "increment";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQScrollBar::SetIncrement(int value)
{
	void *ptr[3];
	ptr[0] = "increment";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQColorPanel
//---------------------------------------------------------------------------
MQColorPanel::MQColorPanel() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "colorpanel";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQColorPanel::MQColorPanel(int id) : MQWidgetBase(id)
{
}

MQColorPanel::~MQColorPanel()
{
}

void MQColorPanel::GetColor(int& r, int& g, int& b)
{
	int array[3] = {0,0,0};
	void *ptr[3];
	ptr[0] = "rgb";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	r = array[0];
	g = array[1];
	b = array[2];
}

void MQColorPanel::SetColor(int r, int g, int b)
{
	int array[3] = {r,g,b};
	void *ptr[7];
	ptr[0] = "rgb";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQColorPanel::GetHSV(double& h, double& s, double& v)
{
	double array[3] = {0,0,0};
	void *ptr[3];
	ptr[0] = "hsv";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	h = array[0];
	s = array[1];
	v = array[2];
}

void MQColorPanel::SetHSV(double h, double s, double v)
{
	double array[3] = {h,s,v};
	void *ptr[3];
	ptr[0] = "hsv";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQPaintBox
//---------------------------------------------------------------------------
MQPaintBox::MQPaintBox() : MQWidgetBase()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "paintbox";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQPaintBox::MQPaintBox(int id) : MQWidgetBase(id)
{
}

MQPaintBox::~MQPaintBox()
{
}


//---------------------------------------------------------------------------
//  class MQMenuItem
//---------------------------------------------------------------------------
MQMenuItem::MQMenuItem(MQWidgetBase *parent) : MQWidgetBase()
{
	int parentID = (parent != NULL) ? parent->GetID() : NullID;

	void *ptr[7];
	ptr[0] = "type";
	ptr[1] = "menuitem";
	ptr[2] = "parent";
	ptr[3] = &parentID;
	ptr[4] = "id";
	ptr[5] = &m_ID;
	ptr[6] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQMenuItem::MQMenuItem(int id) : MQWidgetBase(id)
{
}

MQMenuItem::~MQMenuItem()
{
}

std::wstring MQMenuItem::GetText()
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "text.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "text";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQMenuItem::SetText(const std::wstring& text)
{
	void *ptr[3];
	ptr[0] = "text";
	ptr[1] = (void*)text.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQMenuItem::GetChecked()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "checked";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQMenuItem::SetChecked(bool value)
{
	void *ptr[3];
	ptr[0] = "checked";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQMenuItem::GetToggle()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "toggle";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQMenuItem::SetToggle(bool value)
{
	void *ptr[3];
	ptr[0] = "toggle";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQMenuItem::GetClickClose()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "clickclose";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQMenuItem::SetClickClose(bool value)
{
	void *ptr[3];
	ptr[0] = "clickclose";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQMenuItem::GetSeparator()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "separator";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQMenuItem::SetSeparator(bool value)
{
	void *ptr[3];
	ptr[0] = "separator";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQWindowBase
//---------------------------------------------------------------------------
MQWindowBase::MQWindowBase() : MQWidgetBase()
{
}

MQWindowBase::MQWindowBase(int id) : MQWidgetBase(id)
{
}

MQWindowBase::~MQWindowBase()
{
	std::set<MQWidgetBase*>::iterator it;
	for(it = m_CreatedWidgets.begin(); it != m_CreatedWidgets.end(); ){
		delete *it;
		it = m_CreatedWidgets.erase(it);
	}
}

void MQWindowBase::AddChildWindow(MQWindowBase *child)
{
	if(child == NULL) return;

	void *ptr[5];
	int child_id = child->GetID();
	int result = -1;

	ptr[0] = "child";
	ptr[1] = &child_id;
	ptr[2] = "result";
	ptr[3] = &result;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_ADD_CHILD_WINDOW, (void*)ptr);
}

void MQWindowBase::RemoveChildWindow(MQWindowBase *child)
{
	if(child == NULL) return;

	void *ptr[5];
	int child_id = child->GetID();
	int result = -1;

	ptr[0] = "child";
	ptr[1] = &child_id;
	ptr[2] = "result";
	ptr[3] = &result;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_REMOVE_CHILD_WINDOW, (void*)ptr);
}

MQFrame *MQWindowBase::CreateHorizontalFrame(MQWidgetBase *parent)
{
	MQFrame *p = new MQFrame();
	m_CreatedWidgets.insert(p);
	p->SetAlignment(MQFrame::ALIGN_HORIZONTAL);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQFrame *MQWindowBase::CreateVerticalFrame(MQWidgetBase *parent)
{
	MQFrame *p = new MQFrame();
	m_CreatedWidgets.insert(p);
	p->SetAlignment(MQFrame::ALIGN_VERTICAL);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQGroupBox *MQWindowBase::CreateGroupBox(MQWidgetBase *parent, const std::wstring& str)
{
	MQGroupBox *p = new MQGroupBox();
	m_CreatedWidgets.insert(p);
	p->SetText(str);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQTab *MQWindowBase::CreateTab(MQWidgetBase *parent)
{
	MQTab *p = new MQTab();
	m_CreatedWidgets.insert(p);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQButton *MQWindowBase::CreateButton(MQWidgetBase *parent, const std::wstring& str)
{
	MQButton *p = new MQButton();
	m_CreatedWidgets.insert(p);
	p->SetText(str);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQCheckBox *MQWindowBase::CreateCheckBox(MQWidgetBase *parent, const std::wstring& str)
{
	MQCheckBox *p = new MQCheckBox();
	m_CreatedWidgets.insert(p);
	p->SetText(str);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQRadioButton *MQWindowBase::CreateRadioButton(MQWidgetBase *parent, const std::wstring& str)
{
	MQRadioButton *p = new MQRadioButton();
	m_CreatedWidgets.insert(p);
	p->SetText(str);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQComboBox *MQWindowBase::CreateComboBox(MQWidgetBase *parent)
{
	MQComboBox *p = new MQComboBox();
	m_CreatedWidgets.insert(p);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQListBox *MQWindowBase::CreateListBox(MQWidgetBase *parent)
{
	MQListBox *p = new MQListBox();
	m_CreatedWidgets.insert(p);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQCheckListBox *MQWindowBase::CreateCheckListBox(MQWidgetBase *parent)
{
	MQCheckListBox *p = new MQCheckListBox();
	m_CreatedWidgets.insert(p);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQTreeListBox *MQWindowBase::CreateTreeListBox(MQWidgetBase *parent)
{
	MQTreeListBox *p = new MQTreeListBox();
	m_CreatedWidgets.insert(p);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQLabel *MQWindowBase::CreateLabel(MQWidgetBase *parent, const std::wstring& str)
{
	MQLabel *p = new MQLabel();
	m_CreatedWidgets.insert(p);
	p->SetText(str);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQEdit *MQWindowBase::CreateEdit(MQWidgetBase *parent, const std::wstring& str)
{
	MQEdit *p = new MQEdit();
	m_CreatedWidgets.insert(p);
	p->SetText(str);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQMemo *MQWindowBase::CreateMemo(MQWidgetBase *parent)
{
	MQMemo *p = new MQMemo();
	m_CreatedWidgets.insert(p);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQSpinBox *MQWindowBase::CreateSpinBox(MQWidgetBase *parent)
{
	MQSpinBox *p = new MQSpinBox();
	m_CreatedWidgets.insert(p);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQDoubleSpinBox *MQWindowBase::CreateDoubleSpinBox(MQWidgetBase *parent)
{
	MQDoubleSpinBox *p = new MQDoubleSpinBox();
	m_CreatedWidgets.insert(p);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQSlider *MQWindowBase::CreateSlider(MQWidgetBase *parent)
{
	MQSlider *p = new MQSlider();
	m_CreatedWidgets.insert(p);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQProgressBar *MQWindowBase::CreateProgressBar(MQWidgetBase *parent)
{
	MQProgressBar *p = new MQProgressBar();
	m_CreatedWidgets.insert(p);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQScrollBar *MQWindowBase::CreateScrollBar(MQWidgetBase *parent)
{
	MQScrollBar *p = new MQScrollBar();
	m_CreatedWidgets.insert(p);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQScrollBox *MQWindowBase::CreateScrollBox(MQWidgetBase *parent)
{
	MQScrollBox *p = new MQScrollBox();
	m_CreatedWidgets.insert(p);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQColorPanel *MQWindowBase::CreateColorPanel(MQWidgetBase *parent)
{
	MQColorPanel *p = new MQColorPanel();
	m_CreatedWidgets.insert(p);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

MQPaintBox *MQWindowBase::CreatePaintBox(MQWidgetBase *parent)
{
	MQPaintBox *p = new MQPaintBox();
	m_CreatedWidgets.insert(p);
	if(parent != NULL){
		parent->AddChild(p);
	}
	return p;
}

void MQWindowBase::DeleteWidget(MQWidgetBase *widget)
{
	assert(widget != NULL);
	if(widget == NULL) return;

	m_CreatedWidgets.erase(widget);

	delete widget;
}

// Set this window as a modal window.
void MQWindowBase::SetModal()
{
	void *ptr[3];
	ptr[0] = "setmodal";
	ptr[1] = NULL;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

// Release a modal status.
void MQWindowBase::ReleaseModal()
{
	void *ptr[3];
	ptr[0] = "releasemodal";
	ptr[1] = NULL;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

// Check if this window has a modal status.
bool MQWindowBase::IsModal()
{
	bool result = false;

	void *ptr[5];
	ptr[0] = "ismodal";
	ptr[1] = NULL;
	ptr[2] = "result";
	ptr[3] = &result;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);

	return result;
}

std::wstring MQWindowBase::GetTitle()
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "title.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "title";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQWindowBase::SetTitle(const std::wstring& text)
{
	void *ptr[3];
	ptr[0] = "title";
	ptr[1] = (void*)text.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQWindowBase::GetPosX() const
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "pos_x";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWindowBase::SetPosX(int value)
{
	void *ptr[3];
	ptr[0] = "pos_x";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQWindowBase::GetPosY() const
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "pos_y";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWindowBase::SetPosY(int value)
{
	void *ptr[3];
	ptr[0] = "pos_y";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQWindowBase::GetWindowFrame() const
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "windowframe";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWindowBase::SetWindowFrame(bool value)
{
	void *ptr[3];
	ptr[0] = "windowframe";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQWindowBase::GetTitleBar() const
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "titlebar";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWindowBase::SetTitleBar(bool value)
{
	void *ptr[3];
	ptr[0] = "titlebar";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQWindowBase::GetCanResize() const
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "canresize";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWindowBase::SetCanResize(bool value)
{
	void *ptr[3];
	ptr[0] = "canresize";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQWindowBase::GetCloseButton() const
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "closebutton";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWindowBase::SetCloseButton(bool value)
{
	void *ptr[3];
	ptr[0] = "closebutton";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQWindowBase::GetMaximizeButton() const
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "maximizebutton";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWindowBase::SetMaximizeButton(bool value)
{
	void *ptr[3];
	ptr[0] = "maximizebutton";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQWindowBase::GetMinimizeButton() const
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "minimizebutton";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWindowBase::SetMinimizeButton(bool value)
{
	void *ptr[3];
	ptr[0] = "minimizebutton";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQWindowBase::GetMaximized() const
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "maximized";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWindowBase::SetMaximized(bool value)
{
	void *ptr[3];
	ptr[0] = "maximized";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQWindowBase::GetMinimized() const
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "minimized";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWindowBase::SetMinimized(bool value)
{
	void *ptr[3];
	ptr[0] = "minimized";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQWindowBase::GetBackColor(int& r, int& g, int& b) const
{
	int array[3] = {0,0,0};
	void *ptr[3];
	ptr[0] = "backcolor.rgb";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	r = array[0];
	g = array[1];
	b = array[2];
}

void MQWindowBase::SetBackColor(int r, int g, int b)
{
	int array[3] = {r,g,b};
	void *ptr[9];
	ptr[0] = "backcolor.rgb";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQWindowBase::GetAlphaBlend() const
{
	int value = 255;
	void *ptr[3];
	ptr[0] = "alphablend";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWindowBase::SetAlphaBlend(int value)
{
	void *ptr[3];
	ptr[0] = "alphablend";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQWindowBase::GetAcceptDrops() const
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "acceptdrops";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQWindowBase::SetAcceptDrops(bool value)
{
	void *ptr[3];
	ptr[0] = "acceptdrops";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQWindow
//---------------------------------------------------------------------------
MQWindow::MQWindow() : MQWindowBase()
{
	CreateWindowID();
}

MQWindow::MQWindow(int id) : MQWindowBase(id)
{
}

MQWindow::MQWindow(MQWindowBase& parent) : MQWindowBase()
{
	CreateWindowID();
	parent.AddChildWindow(this);
}

MQWindow::~MQWindow()
{
}

void MQWindow::CreateWindowID()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "window";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQWindow MQWindow::GetMainWindow()
{
	int id = GetSystemWidgetID(MQSystemWidget::MainWindow);
	return MQWindow(id);
}

MQWindow *MQWindow::CreateWindow(MQWindowBase& parent)
{
	return new MQWindow(parent);
}

//---------------------------------------------------------------------------
//  class MQDialog
//---------------------------------------------------------------------------
MQDialog::MQDialog() : MQWindowBase()
{
	CreateDialogID();
}

MQDialog::MQDialog(int id) : MQWindowBase(id)
{
}

MQDialog::MQDialog(MQWindowBase& parent) : MQWindowBase()
{
	CreateDialogID();
	parent.AddChildWindow(this);
}

MQDialog::MQDialog(MQWindowBase& parent, int dummy_for_no_creation) : MQWindowBase()
{
}

MQDialog::~MQDialog()
{
}

void MQDialog::CreateDialogID()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "dialog";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQDialog::DIALOG_RESULT MQDialog::Execute()
{
	MQDialog::DIALOG_RESULT result = MQDialog::DIALOG_NONE;

	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "modal";
	ptr[2] = "result";
	ptr[3] = &result;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EXECUTE, (void*)ptr);

	return result;
}

void MQDialog::Close(DIALOG_RESULT code)
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "close";
	ptr[2] = "code";
	switch(code){
	case DIALOG_OK: ptr[3] = "ok"; break;
	case DIALOG_CANCEL: ptr[3] = "cancel"; break;
	case DIALOG_YES: ptr[3] = "yes"; break;
	case DIALOG_NO: ptr[3] = "no"; break;
	case DIALOG_ALL: ptr[3] = "all"; break;
	default: ptr[3] = "none"; break;
	}
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EXECUTE, (void*)ptr);
}

MQDialog::DIALOG_RESULT MQDialog::MessageWarningBox(MQWindowBase& parent, const std::wstring& message, const std::wstring& title)
{
	MQDialog::DIALOG_RESULT result = MQDialog::DIALOG_NONE;
	int parent_id = parent.GetID();

	void *ptr[13];
	ptr[0] = "type";
	ptr[1] = "messagebox";
	ptr[2] = "messagetype";
	ptr[3] = "warning";
	ptr[4] = "parent";
	ptr[5] = &parent_id;
	ptr[6] = "message";
	ptr[7] = (void*)message.c_str();
	ptr[8] = "title";
	ptr[9] = (void*)title.c_str();
	ptr[10] = "result";
	ptr[11] = &result;
	ptr[12] = NULL;
	MQWidget_Value(NullID, MQWIDGET_EXECUTE, (void*)ptr);

	return result;
}

MQDialog::DIALOG_RESULT MQDialog::MessageInformationBox(MQWindowBase& parent, const std::wstring& message, const std::wstring& title)
{
	MQDialog::DIALOG_RESULT result = MQDialog::DIALOG_NONE;
	int parent_id = parent.GetID();

	void *ptr[13];
	ptr[0] = "type";
	ptr[1] = "messagebox";
	ptr[2] = "messagetype";
	ptr[3] = "information";
	ptr[4] = "parent";
	ptr[5] = &parent_id;
	ptr[6] = "message";
	ptr[7] = (void*)message.c_str();
	ptr[8] = "title";
	ptr[9] = (void*)title.c_str();
	ptr[10] = "result";
	ptr[11] = &result;
	ptr[12] = NULL;
	MQWidget_Value(NullID, MQWIDGET_EXECUTE, (void*)ptr);

	return result;
}

MQDialog::DIALOG_RESULT MQDialog::MessageYesNoBox(MQWindowBase& parent, const std::wstring& message, const std::wstring& title)
{
	MQDialog::DIALOG_RESULT result = MQDialog::DIALOG_NONE;
	int parent_id = parent.GetID();

	void *ptr[13];
	ptr[0] = "type";
	ptr[1] = "messagebox";
	ptr[2] = "messagetype";
	ptr[3] = "yesno";
	ptr[4] = "parent";
	ptr[5] = &parent_id;
	ptr[6] = "message";
	ptr[7] = (void*)message.c_str();
	ptr[8] = "title";
	ptr[9] = (void*)title.c_str();
	ptr[10] = "result";
	ptr[11] = &result;
	ptr[12] = NULL;
	MQWidget_Value(NullID, MQWIDGET_EXECUTE, (void*)ptr);

	return result;
}

MQDialog::DIALOG_RESULT MQDialog::MessageYesNoCancelBox(MQWindowBase& parent, const std::wstring& message, const std::wstring& title)
{
	MQDialog::DIALOG_RESULT result = MQDialog::DIALOG_NONE;
	int parent_id = parent.GetID();

	void *ptr[13];
	ptr[0] = "type";
	ptr[1] = "messagebox";
	ptr[2] = "messagetype";
	ptr[3] = "yesnocancel";
	ptr[4] = "parent";
	ptr[5] = &parent_id;
	ptr[6] = "message";
	ptr[7] = (void*)message.c_str();
	ptr[8] = "title";
	ptr[9] = (void*)title.c_str();
	ptr[10] = "result";
	ptr[11] = &result;
	ptr[12] = NULL;
	MQWidget_Value(NullID, MQWIDGET_EXECUTE, (void*)ptr);

	return result;
}

MQDialog::DIALOG_RESULT MQDialog::MessageOkCancelBox(MQWindowBase& parent, const std::wstring& message, const std::wstring& title)
{
	MQDialog::DIALOG_RESULT result = MQDialog::DIALOG_NONE;
	int parent_id = parent.GetID();

	void *ptr[13];
	ptr[0] = "type";
	ptr[1] = "messagebox";
	ptr[2] = "messagetype";
	ptr[3] = "okcancel";
	ptr[4] = "parent";
	ptr[5] = &parent_id;
	ptr[6] = "message";
	ptr[7] = (void*)message.c_str();
	ptr[8] = "title";
	ptr[9] = (void*)title.c_str();
	ptr[10] = "result";
	ptr[11] = &result;
	ptr[12] = NULL;
	MQWidget_Value(NullID, MQWIDGET_EXECUTE, (void*)ptr);

	return result;
}


//---------------------------------------------------------------------------
//  class MQPopup
//---------------------------------------------------------------------------
MQPopup::MQPopup() : MQWindowBase()
{
	CreatePopupID();
}

MQPopup::MQPopup(int id) : MQWindowBase(id)
{
}

MQPopup::MQPopup(MQWindowBase& parent) : MQWindowBase()
{
	CreatePopupID();
	parent.AddChildWindow(this);
}

MQPopup::~MQPopup()
{
	std::set<MQMenuItem*>::iterator it;
	for(it = m_CreatedMenuItems.begin(); it != m_CreatedMenuItems.end(); ){
		delete *it;
		it = m_CreatedMenuItems.erase(it);
	}
}

void MQPopup::CreatePopupID()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "popup";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQMenuItem *MQPopup::CreateMenuItem(const std::wstring& text)
{
	MQMenuItem *p = new MQMenuItem(this);
	m_CreatedMenuItems.insert(p);
	p->SetText(text);
	return p;
}

MQMenuItem *MQPopup::CreateSubMenuItem(MQMenuItem *parent, const std::wstring& text)
{
	MQMenuItem *p = new MQMenuItem(parent);
	m_CreatedMenuItems.insert(p);
	p->SetText(text);
	return p;
}

void MQPopup::GetPreferredSidePosition(int& x, int& y, int& w, int& h, MQWidgetBase *widget, bool horz)
{
	int widgetID = (widget != NULL) ? widget->GetID() : NullID;
	int values[4] = {0, 0, 0, 0};
	
	void *args[7];
	args[0] = "rect";
	args[1] = values;
	args[2] = "widget";
	args[3] = &widgetID;
	args[4] = "horz";
	args[5] = &horz;
	args[6] = NULL;
	
	void *ptr[3];
	ptr[0] = "prefsidepos";
	ptr[1] = args;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	x = values[0];
	y = values[1];
	w = values[2];
	h = values[3];
}

void MQPopup::ShowPopup(int screen_x, int screen_y)
{
	void *ptr[7];
	ptr[0] = "type";
	ptr[1] = "popup";
	ptr[2] = "screen_x";
	ptr[3] = &screen_x;
	ptr[4] = "screen_y";
	ptr[5] = &screen_y;
	ptr[6] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EXECUTE, (void*)ptr);
}

bool MQPopup::GetShadow()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "shadow";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQPopup::SetShadow(bool value)
{
	void *ptr[3];
	ptr[0] = "shadow";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQPopup::GetOverlay()
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "overlay";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQPopup::SetOverlay(bool value)
{
	void *ptr[3];
	ptr[0] = "overlay";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQColorDialog
//---------------------------------------------------------------------------
MQColorDialog::MQColorDialog(MQWindowBase& parent) : MQDialog(parent, 0)
{
	CreateColorDialogID();
	parent.AddChildWindow(this);
}

MQColorDialog::~MQColorDialog()
{
}

void MQColorDialog::CreateColorDialogID()
{
	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "colordialog";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		m_IDOwner = true;
		s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

void MQColorDialog::GetColor(int& r, int& g, int& b)
{
	int array[3] = {0,0,0};
	void *ptr[3];
	ptr[0] = "rgb";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	r = array[0];
	g = array[1];
	b = array[2];
}

void MQColorDialog::SetColor(int r, int g, int b)
{
	int array[3] = {r,g,b};
	void *ptr[7];
	ptr[0] = "rgb";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQColorDialog::GetHSV(double& h, double& s, double& v)
{
	double array[3] = {0,0,0};
	void *ptr[3];
	ptr[0] = "hsv";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	h = array[0];
	s = array[1];
	v = array[2];
}

void MQColorDialog::SetHSV(double h, double s, double v)
{
	double array[3] = {h,s,v};
	void *ptr[3];
	ptr[0] = "hsv";
	ptr[1] = array;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQFileDialogBase
//---------------------------------------------------------------------------
MQFileDialogBase::MQFileDialogBase()
{
}

MQFileDialogBase::~MQFileDialogBase()
{
	if(m_ID != NullID){
		//s_WidgetIDMap.erase(m_ID);

		void *ptr[1];
		ptr[0] = NULL;
		MQWidget_Value(m_ID, MQWIDGET_DELETE, (void*)ptr);

		m_ID = NullID;
	}
}

int MQFileDialogBase::AddFilter(const std::wstring& val)
{
	int result = -1;
	void *ptr[7];
	ptr[0] = "addfilter";
	ptr[1] = NULL;
	ptr[2] = "text";
	ptr[3] = (void*)val.c_str();
	ptr[4] = "result";
	ptr[5] = &result;
	ptr[6] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
	return result;
}

int MQFileDialogBase::GetFilterCount() const
{
	int value = 0;
	void *ptr[3];

	ptr[0] = "filter.count";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);
	return value;
}

std::wstring MQFileDialogBase::GetFilter(int index) const
{
	void *ptr[5];
	unsigned int length = 0;

	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "filter.text.length";
	ptr[3] = &length;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[2] = "filter.text";
	ptr[3] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQFileDialogBase::SetFilter(int index, const std::wstring& val)
{
	void *ptr[5];
	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "filter.text";
	ptr[3] = (void*)val.c_str();
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

void MQFileDialogBase::ClearFilters()
{
	void *ptr[3];
	ptr[0] = "clearfilters";
	ptr[1] = NULL;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EDIT, (void*)ptr);
}

std::wstring MQFileDialogBase::GetTitle() const
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "title.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "title";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQFileDialogBase::SetTitle(const std::wstring& val)
{
	void *ptr[3];
	ptr[0] = "title";
	ptr[1] = (void*)val.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

std::wstring MQFileDialogBase::GetFileName() const
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "filename.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "filename";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQFileDialogBase::SetFileName(const std::wstring& val)
{
	void *ptr[3];
	ptr[0] = "filename";
	ptr[1] = (void*)val.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

std::wstring MQFileDialogBase::GetInitialDir() const
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "initialdir.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "initialdir";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQFileDialogBase::SetInitialDir(const std::wstring& val)
{
	void *ptr[3];
	ptr[0] = "initialdir";
	ptr[1] = (void*)val.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

std::wstring MQFileDialogBase::GetDefaultExt() const
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "defaultext.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "defaultext";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQFileDialogBase::SetDefaultExt(const std::wstring& val)
{
	void *ptr[3];
	ptr[0] = "defaultext";
	ptr[1] = (void*)val.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

int MQFileDialogBase::GetFilterIndex() const
{
	int value = 0;
	void *ptr[3];
	ptr[0] = "filterindex";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQFileDialogBase::SetFilterIndex(int val)
{
	void *ptr[3];
	ptr[0] = "filterindex";
	ptr[1] = &val;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQFileDialogBase::GetNoChangeDir() const
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "nochangedir";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQFileDialogBase::SetNoChangeDir(bool val)
{
	void *ptr[3];
	ptr[0] = "nochangedir";
	ptr[1] = &val;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}


//---------------------------------------------------------------------------
//  class MQOpenFileDialog
//---------------------------------------------------------------------------
MQOpenFileDialog::MQOpenFileDialog(MQWindowBase& parent) : MQFileDialogBase()
{
	int parentID = parent.GetID();

	void *ptr[7];
	ptr[0] = "type";
	ptr[1] = "openfiledialog";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = "parent";
	ptr[5] = &parentID;
	ptr[6] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		//s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQOpenFileDialog::~MQOpenFileDialog()
{
}

bool MQOpenFileDialog::GetFileMustExist() const
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "filemustexist";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQOpenFileDialog::SetFileMustExist(bool val)
{
	void *ptr[3];
	ptr[0] = "filemustexist";
	ptr[1] = &val;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQOpenFileDialog::GetMultiSelect() const
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "multiselect";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQOpenFileDialog::SetMultiSelect(bool val)
{
	void *ptr[3];
	ptr[0] = "multiselect";
	ptr[1] = &val;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQOpenFileDialog::Execute()
{
	bool result = false;

	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "fileopen";
	ptr[2] = "result";
	ptr[3] = &result;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EXECUTE, (void*)ptr);

	return result;
}

int MQOpenFileDialog::GetFileNamesCount() const
{
	int value = 0;
	void *ptr[3];

	ptr[0] = "filenames.count";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);
	return value;
}

std::wstring MQOpenFileDialog::GetFileNames(int index) const
{
	void *ptr[5];
	unsigned int length = 0;

	ptr[0] = "index";
	ptr[1] = &index;
	ptr[2] = "filenames.length";
	ptr[3] = &length;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[2] = "filenames";
	ptr[3] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}


//---------------------------------------------------------------------------
//  class MQSaveFileDialog
//---------------------------------------------------------------------------
MQSaveFileDialog::MQSaveFileDialog(MQWindowBase& parent) : MQFileDialogBase()
{
	int parentID = parent.GetID();

	void *ptr[7];
	ptr[0] = "type";
	ptr[1] = "savefiledialog";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = "parent";
	ptr[5] = &parentID;
	ptr[6] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		//s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQSaveFileDialog::~MQSaveFileDialog()
{
}

bool MQSaveFileDialog::Execute()
{
	bool result = false;

	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "filesave";
	ptr[2] = "result";
	ptr[3] = &result;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EXECUTE, (void*)ptr);

	return result;
}


//---------------------------------------------------------------------------
//  class MQFolderDialog
//---------------------------------------------------------------------------
MQFolderDialog::MQFolderDialog(MQWindowBase& parent)
{
	int parentID = parent.GetID();

	void *ptr[7];
	ptr[0] = "type";
	ptr[1] = "folderdialog";
	ptr[2] = "id";
	ptr[3] = &m_ID;
	ptr[4] = "parent";
	ptr[5] = &parentID;
	ptr[6] = NULL;
	MQWidget_Value(NullID, MQWIDGET_CREATE, (void*)ptr);

	if(m_ID != NullID){
		//s_WidgetIDMap.insert(std::pair<int,MQWidgetBase*>(m_ID, this));
	}
}

MQFolderDialog::~MQFolderDialog()
{
	if(m_ID != NullID){
		//s_WidgetIDMap.erase(m_ID);

		void *ptr[1];
		ptr[0] = NULL;
		MQWidget_Value(m_ID, MQWIDGET_DELETE, (void*)ptr);

		m_ID = NullID;
	}
}

std::wstring MQFolderDialog::GetTitle() const
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "title.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "title";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQFolderDialog::SetTitle(const std::wstring& val)
{
	void *ptr[3];
	ptr[0] = "title";
	ptr[1] = (void*)val.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

std::wstring MQFolderDialog::GetFolder() const
{
	void *ptr[3];
	unsigned int length = 0;

	ptr[0] = "folder.length";
	ptr[1] = &length;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	if(length == 0){
		return std::wstring();
	}

	wchar_t *buf = new wchar_t[length+1];
	ptr[0] = "folder";
	ptr[1] = buf;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	std::wstring ret(buf);
	delete[] buf;
	return ret;
}

void MQFolderDialog::SetFolder(const std::wstring& val)
{
	void *ptr[3];
	ptr[0] = "folder";
	ptr[1] = (void*)val.c_str();
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQFolderDialog::GetNewFolder() const
{
	bool value = false;
	void *ptr[3];
	ptr[0] = "newfolder";
	ptr[1] = &value;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_GET, (void*)ptr);

	return value;
}

void MQFolderDialog::SetNewFolder(bool val)
{
	void *ptr[3];
	ptr[0] = "newfolder";
	ptr[1] = &val;
	ptr[2] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_SET, (void*)ptr);
}

bool MQFolderDialog::Execute()
{
	bool result = false;

	void *ptr[5];
	ptr[0] = "type";
	ptr[1] = "folder";
	ptr[2] = "result";
	ptr[3] = &result;
	ptr[4] = NULL;
	MQWidget_Value(m_ID, MQWIDGET_EXECUTE, (void*)ptr);

	return result;
}


//---------------------------------------------------------------------------
//  class MQCanvas
//---------------------------------------------------------------------------
MQCanvas::MQCanvas(void *ptr)
{
	m_Ptr = ptr;
}

MQCanvas::~MQCanvas()
{
}

void MQCanvas::PushClipRect(int x, int y, int w, int h)
{
	void *ptr[10];
	ptr[0] = "pushcliprect";
	ptr[1] = "x";
	ptr[2] = &x;
	ptr[3] = "y";
	ptr[4] = &y;
	ptr[5] = "w";
	ptr[6] = &w;
	ptr[7] = "h";
	ptr[8] = &h;
	ptr[9] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_SET, ptr);
}

void MQCanvas::PopClipRect()
{
	void *ptr[2];
	ptr[0] = "popcliprect";
	ptr[1] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_SET, ptr);
}

void MQCanvas::SetColor(int r, int g, int b, int a)
{
	int values[4] = {r,g,b,a};
	void *ptr[4];
	ptr[0] = "color";
	ptr[1] = "rgba";
	ptr[2] = values;
	ptr[3] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_SET, ptr);
}

void MQCanvas::SetGradientColor(int x1, int y1, int x2, int y2, const std::vector<MQCanvasColor>& colors, const std::vector<float>& segments)
{
	assert(colors.size() == segments.size());
	int num = (int)colors.size();
	void *ptr[16];
	ptr[0] = "gradientcolorI";
	ptr[1] = "x1";
	ptr[2] = &x1;
	ptr[3] = "y1";
	ptr[4] = &y1;
	ptr[5] = "x2";
	ptr[6] = &x2;
	ptr[7] = "y2";
	ptr[8] = &y2;
	ptr[9] = "color_number";
	ptr[10] = &num;
	ptr[11] = "rgba";
	ptr[12] = (void*)&(*colors.begin());
	ptr[13] = "segment";
	ptr[14] = (void*)&(*segments.begin());
	ptr[15] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_SET, ptr);
}

void MQCanvas::SetGradientColor(float x1, float y1, float x2, float y2, const std::vector<MQCanvasColor>& colors, const std::vector<float>& segments)
{
	assert(colors.size() == segments.size());
	int num = (int)colors.size();
	void *ptr[16];
	ptr[0] = "gradientcolorF";
	ptr[1] = "x1";
	ptr[2] = &x1;
	ptr[3] = "y1";
	ptr[4] = &y1;
	ptr[5] = "x2";
	ptr[6] = &x2;
	ptr[7] = "y2";
	ptr[8] = &y2;
	ptr[9] = "color_number";
	ptr[10] = &num;
	ptr[11] = "rgba";
	ptr[12] = (void*)&(*colors.begin());
	ptr[13] = "segment";
	ptr[14] = (void*)&(*segments.begin());
	ptr[15] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_SET, ptr);
}

void MQCanvas::SetStrokeWidth(float width)
{
	void *ptr[4];
	ptr[0] = "strokewidth";
	ptr[1] = "width";
	ptr[2] = &width;
	ptr[3] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_SET, ptr);
}

void MQCanvas::SetStrokeCap(MQCANVAS_CAP_TYPE cap)
{
	void *ptr[4];
	ptr[0] = "strokecap";
	ptr[1] = "cap";
	switch(cap){
	case CAP_BUTT: ptr[2] = "butt"; break;
	case CAP_ROUND: ptr[2] = "round"; break;
	case CAP_SQUARE: ptr[2] = "square"; break;
	default: return;
	}
	ptr[3] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_SET, ptr);
}

void MQCanvas::SetStrokeJoin(MQCANVAS_JOIN_TYPE join)
{
	void *ptr[4];
	ptr[0] = "strokejoin";
	ptr[1] = "join";
	switch(join){
	case JOIN_MITER: ptr[2] = "miter"; break;
	case JOIN_ROUND: ptr[2] = "round"; break;
	case JOIN_BEVEL: ptr[2] = "bevel"; break;
	default: return;
	}
	ptr[3] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_SET, ptr);
}

void MQCanvas::SetStrokeMiterLimit(float limit)
{
	void *ptr[4];
	ptr[0] = "strokemiterlimit";
	ptr[1] = "limit";
	ptr[2] = &limit;
	ptr[3] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_SET, ptr);
}

void MQCanvas::SetStrokeDash(const std::vector<float>& intervals)
{
	int num = (int)intervals.size();
	void *ptr[6];
	ptr[0] = "strokedash";
	ptr[1] = "number";
	ptr[2] = &num;
	ptr[3] = "interval";
	ptr[4] = (void*)&(*intervals.begin());
	ptr[5] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_SET, ptr);
}

void MQCanvas::SetFont(const wchar_t *fontname, bool bold)
{
	void *ptr[6];
	ptr[0] = "font";
	ptr[1] = "fontname";
	ptr[2] = (void*)fontname;
	ptr[3] = "bold";
	ptr[4] = &bold;
	ptr[5] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_SET, ptr);
}

void MQCanvas::SetFontSize(int size)
{
	void *ptr[4];
	ptr[0] = "fontsize";
	ptr[1] = "size";
	ptr[2] = &size;
	ptr[3] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_SET, ptr);
}

void MQCanvas::SetFontRateSize(float rate)
{
	void *ptr[4];
	ptr[0] = "fontratesize";
	ptr[1] = "rate";
	ptr[2] = &rate;
	ptr[3] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_SET, ptr);
}

void MQCanvas::SetAntiAlias(bool val)
{
	void *ptr[4];
	ptr[0] = "antialias";
	ptr[1] = "value";
	ptr[2] = &val;
	ptr[3] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_SET, ptr);
}

void MQCanvas::Clear(int r, int g, int b, int a)
{
	int values[4] = {r,g,b,a};
	void *ptr[4];
	ptr[0] = "clear";
	ptr[1] = "rgba";
	ptr[2] = values;
	ptr[3] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::DrawLine(int x1, int y1, int x2, int y2)
{
	void *ptr[10];
	ptr[0] = "drawlineI";
	ptr[1] = "x1";
	ptr[2] = &x1;
	ptr[3] = "y1";
	ptr[4] = &y1;
	ptr[5] = "x2";
	ptr[6] = &x2;
	ptr[7] = "y2";
	ptr[8] = &y2;
	ptr[9] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::DrawLine(float x1, float y1, float x2, float y2)
{
	void *ptr[10];
	ptr[0] = "drawlineF";
	ptr[1] = "x1";
	ptr[2] = &x1;
	ptr[3] = "y1";
	ptr[4] = &y1;
	ptr[5] = "x2";
	ptr[6] = &x2;
	ptr[7] = "y2";
	ptr[8] = &y2;
	ptr[9] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::DrawPolyline(const POINT *points, int num_points)
{
	void *ptr[6];
	ptr[0] = "drawpolylineI";
	ptr[1] = "points";
	ptr[2] = (void*)points;
	ptr[3] = "num_points";
	ptr[4] = &num_points;
	ptr[5] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::DrawPolyline(const MQCanvasPoint *points, int num_points)
{
	void *ptr[6];
	ptr[0] = "drawpolylineF";
	ptr[1] = "points";
	ptr[2] = (void*)points;
	ptr[3] = "num_points";
	ptr[4] = &num_points;
	ptr[5] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::DrawCircle(int x, int y, float r)
{
	void *ptr[8];
	ptr[0] = "drawcircleI";
	ptr[1] = "x";
	ptr[2] = &x;
	ptr[3] = "y";
	ptr[4] = &y;
	ptr[5] = "r";
	ptr[6] = &r;
	ptr[7] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::DrawCircle(float x, float y, float r)
{
	void *ptr[8];
	ptr[0] = "drawcircleF";
	ptr[1] = "x";
	ptr[2] = &x;
	ptr[3] = "y";
	ptr[4] = &y;
	ptr[5] = "r";
	ptr[6] = &r;
	ptr[7] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::FillCircle(int x, int y, float r)
{
	void *ptr[8];
	ptr[0] = "fillcircleI";
	ptr[1] = "x";
	ptr[2] = &x;
	ptr[3] = "y";
	ptr[4] = &y;
	ptr[5] = "r";
	ptr[6] = &r;
	ptr[7] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::FillCircle(float x, float y, float r)
{
	void *ptr[8];
	ptr[0] = "fillcircleF";
	ptr[1] = "x";
	ptr[2] = &x;
	ptr[3] = "y";
	ptr[4] = &y;
	ptr[5] = "r";
	ptr[6] = &r;
	ptr[7] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::DrawEllipse(int x, int y, float rx, float ry)
{
	void *ptr[10];
	ptr[0] = "drawellipseI";
	ptr[1] = "x";
	ptr[2] = &x;
	ptr[3] = "y";
	ptr[4] = &y;
	ptr[5] = "rx";
	ptr[6] = &rx;
	ptr[7] = "ry";
	ptr[8] = &ry;
	ptr[9] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::DrawEllipse(float x, float y, float rx, float ry)
{
	void *ptr[10];
	ptr[0] = "drawellipseF";
	ptr[1] = "x";
	ptr[2] = &x;
	ptr[3] = "y";
	ptr[4] = &y;
	ptr[5] = "rx";
	ptr[6] = &rx;
	ptr[7] = "ry";
	ptr[8] = &ry;
	ptr[9] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::FillEllipse(int x, int y, float rx, float ry)
{
	void *ptr[10];
	ptr[0] = "fillellipseI";
	ptr[1] = "x";
	ptr[2] = &x;
	ptr[3] = "y";
	ptr[4] = &y;
	ptr[5] = "rx";
	ptr[6] = &rx;
	ptr[7] = "ry";
	ptr[8] = &ry;
	ptr[9] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::FillEllipse(float x, float y, float rx, float ry)
{
	void *ptr[10];
	ptr[0] = "fillellipseF";
	ptr[1] = "x";
	ptr[2] = &x;
	ptr[3] = "y";
	ptr[4] = &y;
	ptr[5] = "rx";
	ptr[6] = &rx;
	ptr[7] = "ry";
	ptr[8] = &ry;
	ptr[9] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::DrawRect(int x, int y, int w, int h)
{
	void *ptr[10];
	ptr[0] = "drawrectI";
	ptr[1] = "x";
	ptr[2] = &x;
	ptr[3] = "y";
	ptr[4] = &y;
	ptr[5] = "w";
	ptr[6] = &w;
	ptr[7] = "h";
	ptr[8] = &h;
	ptr[9] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::DrawRect(float x, float y, float w, float h)
{
	void *ptr[10];
	ptr[0] = "drawrectF";
	ptr[1] = "x";
	ptr[2] = &x;
	ptr[3] = "y";
	ptr[4] = &y;
	ptr[5] = "w";
	ptr[6] = &w;
	ptr[7] = "h";
	ptr[8] = &h;
	ptr[9] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::FillRect(int x, int y, int w, int h)
{
	void *ptr[10];
	ptr[0] = "fillrectI";
	ptr[1] = "x";
	ptr[2] = &x;
	ptr[3] = "y";
	ptr[4] = &y;
	ptr[5] = "w";
	ptr[6] = &w;
	ptr[7] = "h";
	ptr[8] = &h;
	ptr[9] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::FillRect(float x, float y, float w, float h)
{
	void *ptr[10];
	ptr[0] = "fillrectF";
	ptr[1] = "x";
	ptr[2] = &x;
	ptr[3] = "y";
	ptr[4] = &y;
	ptr[5] = "w";
	ptr[6] = &w;
	ptr[7] = "h";
	ptr[8] = &h;
	ptr[9] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::DrawRoundRect(int x, int y, int w, int h, int rx, int ry)
{
	void *ptr[14];
	ptr[0] = "drawroundrectI";
	ptr[1] = "x";
	ptr[2] = &x;
	ptr[3] = "y";
	ptr[4] = &y;
	ptr[5] = "w";
	ptr[6] = &w;
	ptr[7] = "h";
	ptr[8] = &h;
	ptr[9] = "rx";
	ptr[10] = &rx;
	ptr[11] = "ry";
	ptr[12] = &ry;
	ptr[13] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::DrawRoundRect(float x, float y, float w, float h, float rx, float ry)
{
	void *ptr[14];
	ptr[0] = "drawroundrectF";
	ptr[1] = "x";
	ptr[2] = &x;
	ptr[3] = "y";
	ptr[4] = &y;
	ptr[5] = "w";
	ptr[6] = &w;
	ptr[7] = "h";
	ptr[8] = &h;
	ptr[9] = "rx";
	ptr[10] = &rx;
	ptr[11] = "ry";
	ptr[12] = &ry;
	ptr[13] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::FillRoundRect(int x, int y, int w, int h, int rx, int ry)
{
	void *ptr[14];
	ptr[0] = "fillroundrectI";
	ptr[1] = "x";
	ptr[2] = &x;
	ptr[3] = "y";
	ptr[4] = &y;
	ptr[5] = "w";
	ptr[6] = &w;
	ptr[7] = "h";
	ptr[8] = &h;
	ptr[9] = "rx";
	ptr[10] = &rx;
	ptr[11] = "ry";
	ptr[12] = &ry;
	ptr[13] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::FillRoundRect(float x, float y, float w, float h, float rx, float ry)
{
	void *ptr[14];
	ptr[0] = "fillroundrectF";
	ptr[1] = "x";
	ptr[2] = &x;
	ptr[3] = "y";
	ptr[4] = &y;
	ptr[5] = "w";
	ptr[6] = &w;
	ptr[7] = "h";
	ptr[8] = &h;
	ptr[9] = "rx";
	ptr[10] = &rx;
	ptr[11] = "ry";
	ptr[12] = &ry;
	ptr[13] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::DrawPolygon(const POINT *points, int num_points)
{
	void *ptr[6];
	ptr[0] = "drawpolygonI";
	ptr[1] = "points";
	ptr[2] = (void*)points;
	ptr[3] = "num_points";
	ptr[4] = &num_points;
	ptr[5] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::DrawPolygon(const MQCanvasPoint *points, int num_points)
{
	void *ptr[6];
	ptr[0] = "drawpolygonF";
	ptr[1] = "points";
	ptr[2] = (void*)points;
	ptr[3] = "num_points";
	ptr[4] = &num_points;
	ptr[5] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::FillPolygon(const POINT *points, int num_points)
{
	void *ptr[6];
	ptr[0] = "fillpolygonI";
	ptr[1] = "points";
	ptr[2] = (void*)points;
	ptr[3] = "num_points";
	ptr[4] = &num_points;
	ptr[5] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::FillPolygon(const MQCanvasPoint *points, int num_points)
{
	void *ptr[6];
	ptr[0] = "fillpolygonF";
	ptr[1] = "points";
	ptr[2] = (void*)points;
	ptr[3] = "num_points";
	ptr[4] = &num_points;
	ptr[5] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::DrawDIB(void *header, void *buffer, int x, int y)
{
	void *ptr[10];
	ptr[0] = "drawdib";
	ptr[1] = "header";
	ptr[2] = header;
	ptr[3] = "buffer";
	ptr[4] = buffer;
	ptr[5] = "x";
	ptr[6] = &x;
	ptr[7] = "y";
	ptr[8] = &y;
	ptr[9] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::DrawDIB(void *header, void *buffer, int x, int y, int w, int h)
{
	void *ptr[14];
	ptr[0] = "drawdib";
	ptr[1] = "header";
	ptr[2] = header;
	ptr[3] = "buffer";
	ptr[4] = buffer;
	ptr[5] = "x";
	ptr[6] = &x;
	ptr[7] = "y";
	ptr[8] = &y;
	ptr[9] = "w";
	ptr[10] = &w;
	ptr[11] = "h";
	ptr[12] = &h;
	ptr[13] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

void MQCanvas::DrawText(const wchar_t *str, int x, int y, int w, int h, bool horz_center, bool vert_center)
{
	void *ptr[16];
	ptr[0] = "drawtext";
	ptr[1] = "string";
	ptr[2] = (void*)str;
	ptr[3] = "x";
	ptr[4] = &x;
	ptr[5] = "y";
	ptr[6] = &y;
	ptr[7] = "w";
	ptr[8] = &w;
	ptr[9] = "h";
	ptr[10] = &h;
	ptr[11] = "horz_center";
	ptr[12] = &horz_center;
	ptr[13] = "vert_center";
	ptr[14] = &vert_center;
	ptr[15] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);
}

POINT MQCanvas::MeasureText(const wchar_t *str)
{
	int x=0, y=0;
	void *ptr[8];
	ptr[0] = "measuretext";
	ptr[1] = "string";
	ptr[2] = (void*)str;
	ptr[3] = "x";
	ptr[4] = &x;
	ptr[5] = "y";
	ptr[6] = &y;
	ptr[7] = NULL;
	MQCanvas_Value(m_Ptr, MQWIDGET_EDIT, ptr);

	POINT p;
	p.x = x;
	p.y = y;
	return p;
}


//---------------------------------------------------------------------------
//  Global functions
//---------------------------------------------------------------------------

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

