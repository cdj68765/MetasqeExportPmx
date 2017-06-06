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

#ifndef _MQWIDGET_H_
#define _MQWIDGET_H_

#include "MQPlugin.h"
#include <vector>
#include <set>
#include <string>
#include <memory>

#ifdef _WIN32
#ifdef CreateWindow
#undef CreateWindow
#endif
#endif

class MQStationPlugin;
class MQWidgetBase;
class MQFrame;
class MQGroupBox;
class MQTab;
class MQButton;
class MQCheckBox;
class MQRadioButton;
class MQComboBox;
class MQListBox;
class MQCheckListBox;
class MQTreeListBox;
class MQLabel;
class MQEdit;
class MQMemo;
class MQSpinBox;
class MQDoubleSpinBox;
class MQSlider;
class MQProgressBar;
class MQScrollBar;
class MQScrollBox;
class MQColorPanel;
class MQPaintBox;
class MQMenuItem;
class MQCanvas;

#if defined(_MSC_VER) && (_MSC_VER >= 1600)		// VC2010 or later
#define MQWidgetSharedPtr std::shared_ptr
#elif defined(_MSC_VER) && (_MSC_VER >= 1500)	// VC2008 or later
#define MQWidgetSharedPtr std::tr1::shared_ptr
#else
typedef MQWidgetSharedPtr std::shared_ptr
#endif



namespace MQSystemWidget {
	enum WidgetType {
		MainWindow,
		OptionPanel,
	};
}

struct MQWidgetMouseParam {
	POINT ClientPos; // Mouse cursor position on the client area
	POINT ScreenPos; // Mouse cursor position on the screen
	POINT DownPos;   // Clicked position on the client area
	POINT LastPos;   // Mouse cursor position on the client area at the last time
	int Wheel;       // Rotation of the mouse wheel (WHEEL_DELTA based value)
	bool LButton;
	bool MButton;
	bool RButton;
	bool Shift;
	bool Ctrl;
	bool Alt;
	float Pressure;  // For tablet or touch (0-1)
	bool Finished;

	MQWidgetMouseParam(){
		ClientPos.x = 0;
		ClientPos.y = 0;
		ScreenPos.x = 0;
		ScreenPos.y = 0;
		DownPos.x = 0;
		DownPos.y = 0;
		Wheel = 0;
		LButton = false;
		MButton = false;
		RButton = false;
		Shift = false;
		Ctrl = false;
		Alt = false;
		Pressure = 0.0f;
		Finished = false;
	};
};

struct MQWidgetKeyParam {
	int Key;
	bool AutoRepeat;
	bool LButton;
	bool MButton;
	bool RButton;
	bool Shift;
	bool Ctrl;
	bool Alt;
	bool Finished;

	MQWidgetKeyParam(){
		Key = 0;
		AutoRepeat = false;
		LButton = false;
		MButton = false;
		RButton = false;
		Shift = false;
		Ctrl = false;
		Alt = false;
		Finished = false;
	};
};

struct MQWidgetPaintParam {
	MQCanvas *Canvas;

	MQWidgetPaintParam(){
		Canvas = NULL;
	};
};

struct MQWidgetDragOverParam {
	POINT ClientPos; // Mouse cursor position on the client area
	POINT ScreenPos; // Mouse cursor position on the screen
	bool Result;

	MQWidgetDragOverParam(){
		ClientPos.x = 0;
		ClientPos.y = 0;
		ScreenPos.x = 0;
		ScreenPos.y = 0;
		Result = false;
	};
};

struct MQWidgetDropFilesParam {
	POINT ClientPos; // Mouse cursor position on the client area
	POINT ScreenPos; // Mouse cursor position on the screen
	std::vector<const wchar_t*> Files;

	MQWidgetDropFilesParam(){
		ClientPos.x = 0;
		ClientPos.y = 0;
		ScreenPos.x = 0;
		ScreenPos.y = 0;
	};
};

struct MQListBoxDrawItemParam {
	MQCanvas *Canvas;
	int ItemIndex;
	int X;
	int Y;
	int Width;
	int Height;

	MQListBoxDrawItemParam(){
		Canvas = NULL;
	};
};

struct MQTreeListBoxDrawItemParam {
	MQCanvas *Canvas;
	int ItemIndex;
	int ItemID;
	int X;
	int Y;
	int Width;
	int Height;
	int DrawX;

	MQTreeListBoxDrawItemParam(){
		Canvas = NULL;
	};
};

class MQWidgetEventClosureBase
{
public:
	MQWidgetEventClosureBase() { }

	virtual BOOL invoke(MQWidgetBase *sender, MQDocument doc, void *ptr) = 0;
	virtual bool isEqual(const MQWidgetEventClosureBase *c) const = 0;
	virtual MQWidgetBase *getReceiver() = 0;
};

template<typename T> class MQWidgetEventClosure : public MQWidgetEventClosureBase
{
	typedef BOOL (T::*Func)(MQWidgetBase*, MQDocument doc);

public:
	MQWidgetEventClosure(T *_p, Func _f) : MQWidgetEventClosureBase(), p(_p), id(_p->GetID()), f(_f) { }

	BOOL invoke(MQWidgetBase *sender, MQDocument doc, void *ptr) override {
		if(MQWidgetBase::FindWidgetByID(id) != NULL){
			return (p->*f)(sender, doc);
		}
		return FALSE;
	}

	bool isEqual(const MQWidgetEventClosureBase *c) const override {
		if(typeid(*this) != typeid(*c)) return false;
		const MQWidgetEventClosure<T> *cc = static_cast<const MQWidgetEventClosure<T>*>(c);
		return (p == cc->p && f == cc->f);
	}

	MQWidgetBase *getReceiver() override { return p; }

private:
	T *p;
	int id;
	Func f;

	MQWidgetEventClosure();
};

template<typename T> class MQWidgetMouseEventClosure : public MQWidgetEventClosureBase
{
	typedef BOOL (T::*Func)(MQWidgetBase*, MQDocument doc, MQWidgetMouseParam& param);

public:
	MQWidgetMouseEventClosure(T *_p, Func _f) : MQWidgetEventClosureBase(), p(_p), id(_p->GetID()), f(_f) { }

	BOOL invoke(MQWidgetBase *sender, MQDocument doc, void *ptr) override {
		if(MQWidgetBase::FindWidgetByID(id) != NULL){
			MQWidgetMouseParam *mouse_param = static_cast<MQWidgetMouseParam*>(ptr);
			return (p->*f)(sender, doc, *mouse_param);
		}
		return FALSE;
	}

	bool isEqual(const MQWidgetEventClosureBase *c) const override {
		if(typeid(*this) != typeid(*c)) return false;
		const MQWidgetMouseEventClosure<T> *cc = static_cast<const MQWidgetMouseEventClosure<T>*>(c);
		return (p == cc->p && f == cc->f);
	}

	MQWidgetBase *getReceiver() override { return p; }

private:
	T *p;
	int id;
	Func f;

	MQWidgetMouseEventClosure();
};

template<typename T> class MQWidgetKeyEventClosure : public MQWidgetEventClosureBase
{
	typedef BOOL (T::*Func)(MQWidgetBase*, MQDocument doc, MQWidgetKeyParam& param);

public:
	MQWidgetKeyEventClosure(T *_p, Func _f) : MQWidgetEventClosureBase(), p(_p), id(_p->GetID()), f(_f) { }

	BOOL invoke(MQWidgetBase *sender, MQDocument doc, void *ptr) override {
		if(MQWidgetBase::FindWidgetByID(id) != NULL){
			MQWidgetKeyParam *key_param = static_cast<MQWidgetKeyParam*>(ptr);
			return (p->*f)(sender, doc, *key_param);
		}
		return FALSE;
	}

	bool isEqual(const MQWidgetEventClosureBase *c) const override {
		if(typeid(*this) != typeid(*c)) return false;
		const MQWidgetKeyEventClosure<T> *cc = static_cast<const MQWidgetKeyEventClosure<T>*>(c);
		return (p == cc->p && f == cc->f);
	}

	MQWidgetBase *getReceiver() override { return p; }

private:
	T *p;
	int id;
	Func f;

	MQWidgetKeyEventClosure();
};

template<typename T> class MQWidgetPaintEventClosure : public MQWidgetEventClosureBase
{
	typedef BOOL (T::*Func)(MQWidgetBase*, MQDocument doc, MQWidgetPaintParam& param);

public:
	MQWidgetPaintEventClosure(T *_p, Func _f) : MQWidgetEventClosureBase(), p(_p), id(_p->GetID()), f(_f) { }

	BOOL invoke(MQWidgetBase *sender, MQDocument doc, void *ptr) override {
		if(MQWidgetBase::FindWidgetByID(id) != NULL){
			MQWidgetPaintParam *paint_param = static_cast<MQWidgetPaintParam*>(ptr);
			return (p->*f)(sender, doc, *paint_param);
		}
		return FALSE;
	}

	bool isEqual(const MQWidgetEventClosureBase *c) const override {
		if(typeid(*this) != typeid(*c)) return false;
		const MQWidgetPaintEventClosure<T> *cc = static_cast<const MQWidgetPaintEventClosure<T>*>(c);
		return (p == cc->p && f == cc->f);
	}

	MQWidgetBase *getReceiver() override { return p; }

private:
	T *p;
	int id;
	Func f;

	MQWidgetPaintEventClosure();
};

template<typename T> class MQWidgetDragOverEventClosure : public MQWidgetEventClosureBase
{
	typedef BOOL (T::*Func)(MQWidgetBase*, MQDocument doc, MQWidgetDragOverParam& param);

public:
	MQWidgetDragOverEventClosure(T *_p, Func _f) : MQWidgetEventClosureBase(), p(_p), id(_p->GetID()), f(_f) { }

	BOOL invoke(MQWidgetBase *sender, MQDocument doc, void *ptr) override {
		if(MQWidgetBase::FindWidgetByID(id) != NULL){
			MQWidgetDragOverParam *paint_param = static_cast<MQWidgetDragOverParam*>(ptr);
			return (p->*f)(sender, doc, *paint_param);
		}
		return FALSE;
	}

	bool isEqual(const MQWidgetEventClosureBase *c) const override {
		if(typeid(*this) != typeid(*c)) return false;
		const MQWidgetDragOverEventClosure<T> *cc = static_cast<const MQWidgetDragOverEventClosure<T>*>(c);
		return (p == cc->p && f == cc->f);
	}

	MQWidgetBase *getReceiver() override { return p; }

private:
	T *p;
	int id;
	Func f;

	MQWidgetDragOverEventClosure();
};

template<typename T> class MQWidgetDropFilesEventClosure : public MQWidgetEventClosureBase
{
	typedef BOOL (T::*Func)(MQWidgetBase*, MQDocument doc, MQWidgetDropFilesParam& param);

public:
	MQWidgetDropFilesEventClosure(T *_p, Func _f) : MQWidgetEventClosureBase(), p(_p), id(_p->GetID()), f(_f) { }

	BOOL invoke(MQWidgetBase *sender, MQDocument doc, void *ptr) override {
		if(MQWidgetBase::FindWidgetByID(id) != NULL){
			MQWidgetDropFilesParam *paint_param = static_cast<MQWidgetDropFilesParam*>(ptr);
			return (p->*f)(sender, doc, *paint_param);
		}
		return FALSE;
	}

	bool isEqual(const MQWidgetEventClosureBase *c) const override {
		if(typeid(*this) != typeid(*c)) return false;
		const MQWidgetDropFilesEventClosure<T> *cc = static_cast<const MQWidgetDropFilesEventClosure<T>*>(c);
		return (p == cc->p && f == cc->f);
	}

	MQWidgetBase *getReceiver() override { return p; }

private:
	T *p;
	int id;
	Func f;

	MQWidgetDropFilesEventClosure();
};

template<typename T> class MQListBoxDrawItemEventClosure : public MQWidgetEventClosureBase
{
	typedef BOOL (T::*Func)(MQWidgetBase*, MQDocument doc, MQListBoxDrawItemParam& param);

public:
	MQListBoxDrawItemEventClosure(T *_p, Func _f) : MQWidgetEventClosureBase(), p(_p), id(_p->GetID()), f(_f) { }

	BOOL invoke(MQWidgetBase *sender, MQDocument doc, void *ptr) override {
		if(MQWidgetBase::FindWidgetByID(id) != NULL){
			MQListBoxDrawItemParam *drawitem_param = static_cast<MQListBoxDrawItemParam*>(ptr);
			return (p->*f)(sender, doc, *drawitem_param);
		}
		return FALSE;
	}

	bool isEqual(const MQWidgetEventClosureBase *c) const override {
		if(typeid(*this) != typeid(*c)) return false;
		const MQListBoxDrawItemEventClosure<T> *cc = static_cast<const MQListBoxDrawItemEventClosure<T>*>(c);
		return (p == cc->p && f == cc->f);
	}

	MQWidgetBase *getReceiver() override { return p; }

private:
	T *p;
	int id;
	Func f;

	MQListBoxDrawItemEventClosure();
};

template<typename T> class MQTreeListBoxDrawItemEventClosure : public MQWidgetEventClosureBase
{
	typedef BOOL (T::*Func)(MQWidgetBase*, MQDocument doc, MQTreeListBoxDrawItemParam& param);

public:
	MQTreeListBoxDrawItemEventClosure(T *_p, Func _f) : MQWidgetEventClosureBase(), p(_p), id(_p->GetID()), f(_f) { }

	BOOL invoke(MQWidgetBase *sender, MQDocument doc, void *ptr) override {
		if(MQWidgetBase::FindWidgetByID(id) != NULL){
			MQTreeListBoxDrawItemParam *drawitem_param = static_cast<MQTreeListBoxDrawItemParam*>(ptr);
			return (p->*f)(sender, doc, *drawitem_param);
		}
		return FALSE;
	}

	bool isEqual(const MQWidgetEventClosureBase *c) const override {
		if(typeid(*this) != typeid(*c)) return false;
		const MQTreeListBoxDrawItemEventClosure<T> *cc = static_cast<const MQTreeListBoxDrawItemEventClosure<T>*>(c);
		return (p == cc->p && f == cc->f);
	}

	MQWidgetBase *getReceiver() override { return p; }

private:
	T *p;
	int id;
	Func f;

	MQTreeListBoxDrawItemEventClosure();
};


struct MQCanvasColor
{
	int r, g, b, a;

	MQCanvasColor() { }
	MQCanvasColor(int _r, int _g, int _b) : r(_r), g(_g), b(_b), a(255) { }
	MQCanvasColor(int _r, int _g, int _b, int _a) : r(_r), g(_g), b(_b), a(_a) { }

	void SetRGB(const MQCanvasColor& col){
		r = col.r;
		g = col.g;
		b = col.b;
	}

	void SetRGBA(const MQCanvasColor& col){
		r = col.r;
		g = col.g;
		b = col.b;
		a = col.a;
	}

	void Clip(int minval = 0, int maxval = 255){
		if(r < minval) r = minval;
		if(r > maxval) r = maxval;
		if(g < minval) g = minval;
		if(g > maxval) g = maxval;
		if(b < minval) b = minval;
		if(b > maxval) b = maxval;
		if(a < minval) a = minval;
		if(a > maxval) a = maxval;
	}

	void Blend(const MQCanvasColor& col2, double ratio){
		r = (int)ceil(r * (1-ratio) + col2.r * ratio + 0.5);
		g = (int)ceil(g * (1-ratio) + col2.g * ratio + 0.5);
		b = (int)ceil(b * (1-ratio) + col2.b * ratio + 0.5);
		a = (int)ceil(a * (1-ratio) + col2.a * ratio + 0.5);
	}

	static MQCanvasColor Blend(const MQCanvasColor& col1, const MQCanvasColor& col2, double ratio){
		return MQCanvasColor(
			(int)ceil(col1.r * (1-ratio) + col2.r * ratio + 0.5), 
			(int)ceil(col1.g * (1-ratio) + col2.g * ratio + 0.5), 
			(int)ceil(col1.b * (1-ratio) + col2.b * ratio + 0.5), 
			(int)ceil(col1.a * (1-ratio) + col2.a * ratio + 0.5));
	}

	bool operator == (const MQCanvasColor& c){
		return (r == c.r && g == c.g && b == c.b && a == c.a);
	}
	bool operator != (const MQCanvasColor& c){
		return !(r == c.r && g == c.g && b == c.b && a == c.a);
	}
	MQCanvasColor& operator += (const MQCanvasColor& p){
		r += p.r;
		g += p.g;
		b += p.b;
		a += p.a;
		return *this;
	}
	MQCanvasColor& operator -= (const MQCanvasColor& p){
		r -= p.r;
		g -= p.g;
		b -= p.b;
		a -= p.a;
		return *this;
	}
	friend MQCanvasColor operator + (const MQCanvasColor& p1, const MQCanvasColor& p2){
		return MQCanvasColor(p1.r+p2.r, p1.g+p2.g, p1.b+p2.b, p1.a+p2.a);
	}
	friend MQCanvasColor operator - (const MQCanvasColor& p1, const MQCanvasColor& p2){
		return MQCanvasColor(p1.r-p2.r, p1.g-p2.g, p1.b-p2.b, p1.a-p2.a);
	}
};


// Base class for all widgets
// すべてのウィジェットの基底クラス
class MQWidgetBase
{
public:
	enum LAYOUT_TYPE {
		LAYOUT_AUTO,
		LAYOUT_FIXED,
		LAYOUT_HINTSIZE,
		LAYOUT_FILL,
		LAYOUT_FREE,
	};

public:
	// Constructor
	// コンストラクタ
	MQWidgetBase();
	MQWidgetBase(int id);

	// Destructor
	// デストラクタ
	virtual ~MQWidgetBase();

	// Get ID.
	// IDを取得します。
	int GetID() const;

	// Add a child widget.
	// 子ウィジェットを追加します。
	int AddChild(MQWidgetBase *child);

	// Remove a child widget. (Not delete it.)
	// 子ウィジェットを切り離します。（削除はされません）
	void RemoveChild(MQWidgetBase *child);

	std::wstring GetName();
	void SetName(const std::wstring& value);

	__int64 GetTag();
	void SetTag(__int64 value);

	bool GetEnabled();
	void SetEnabled(bool value);
	
	bool GetVisible();
	void SetVisible(bool value);
	
	LAYOUT_TYPE GetHorzLayout();
	void SetHorzLayout(LAYOUT_TYPE value);
	
	LAYOUT_TYPE GetVertLayout();
	void SetVertLayout(LAYOUT_TYPE value);

	int GetWidth();
	// Note: SetWidth is not recommended. Please use SetHintSizeRateX as possible.
	//       SetWidthは推奨されません。可能な限りSetHintSizeRateXを使用してください。
	void SetWidth(int value);
	
	int GetHeight();
	// Note: SetHeight is not recommended. Please use SetHintSizeRateY as possible.
	//       SetHeightは推奨されません。可能な限りSetHintSizeRateYを使用してください。
	void SetHeight(int value);

	double GetFillRateX();
	void SetFillRateX(double value);
	
	double GetFillRateY();
	void SetFillRateY(double value);
	
	double GetFillBeforeRate();
	void SetFillBeforeRate(double value);
	
	double GetFillAfterRate();
	void SetFillAfterRate(double value);
	
	double GetInSpace();
	void SetInSpace(double value);
	
	double GetOutSpace();
	void SetOutSpace(double value);

	double GetHintSizeRateX();
	void SetHintSizeRateX(double value);

	double GetHintSizeRateY();
	void SetHintSizeRateY(double value);

	int GetCellColumn();
	void SetCellColumn(int value);

	std::wstring GetHintText();
	void SetHintText(const std::wstring& text);

	POINT GetJustSize(int max_width = -1, int max_height = -1);

	bool ClientToScreen(int client_x, int client_y, int *screen_x, int *screen_y);
	bool ScreenToClient(int screen_x, int screen_y, int *client_x, int *client_y);
	bool ClientToClient(int client_x, int client_y, MQWidgetBase *target, int *target_x, int *target_y);

	void Repaint(bool immediate = false);
	void RefreshPaint();

	void CaptureMouse(bool value);


	// Find a widget by the ID.
	// IDによってウィジェットを検索します。
	static MQWidgetBase *FindWidgetByID(int id);

	// Get the specified system widget ID.
	// 指定したシステムウィジェットのIDを取得します。
	static int GetSystemWidgetID(MQSystemWidget::WidgetType type);

	static int GetBaseRateSize(double rate);

	static void GetDefaultFrameColor(int& r, int& g, int& b, int& a);
	static void GetDefaultTextColor(int& r, int& g, int& b, int& a);
	static void GetDefaultTitleBackColor(int& r, int& g, int& b, int& a);
	static void GetDefaultTitleTextColor(int& r, int& g, int& b, int& a);
	static void GetDefaultListBackColor(int& r, int& g, int& b, int& a);
	static void GetDefaultListTextColor(int& r, int& g, int& b, int& a);
	static void GetDefaultListActiveColor(int& r, int& g, int& b, int& a);
	static void GetDefaultListActiveTextColor(int& r, int& g, int& b, int& a);
	static void GetDefaultEditBackColor(int& r, int& g, int& b, int& a);
	static void GetDefaultEditTextColor(int& r, int& g, int& b, int& a);
	static void GetDefaultEditSelectionColor(int& r, int& g, int& b, int& a);
	static void GetDefaultEditSelectionTextColor(int& r, int& g, int& b, int& a);
	static MQCanvasColor GetDefaultFrameColor();
	static MQCanvasColor GetDefaultTextColor();
	static MQCanvasColor GetDefaultTitleBackColor();
	static MQCanvasColor GetDefaultTitleTextColor();
	static MQCanvasColor GetDefaultListBackColor();
	static MQCanvasColor GetDefaultListTextColor();
	static MQCanvasColor GetDefaultListActiveColor();
	static MQCanvasColor GetDefaultListActiveTextColor();
	static MQCanvasColor GetDefaultEditBackColor();
	static MQCanvasColor GetDefaultEditTextColor();
	static MQCanvasColor GetDefaultEditSelectionColor();
	static MQCanvasColor GetDefaultEditSelectionTextColor();

	static void RegisterSubCommandButton(MQStationPlugin *plugin, MQButton *button, const char *command_str);

	static void EnterEventLoop(unsigned int timeout_ms = (unsigned int)-1, bool empty_return = false);
	static void ExitEventLoop();


	template<typename T> void AddShowEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument), bool prior = false) {
		AddEventCallback("show", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveShowEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument)) {
		RemoveEventCallback("show", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsShowEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument)) const {
		return ExistsEventCallback("show", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

	template<typename T> void AddHideEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument), bool prior = false) {
		AddEventCallback("hide", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveHideEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument)) {
		RemoveEventCallback("hide", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsHideEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument)) const {
		return ExistsEventCallback("hide", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

	template<typename T> void AddMoveEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument), bool prior = false) {
		AddEventCallback("move", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveMoveEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument)) {
		RemoveEventCallback("move", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsMoveEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument)) const {
		return ExistsEventCallback("move", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

	template<typename T> void AddResizeEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument), bool prior = false) {
		AddEventCallback("resize", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveResizeEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument)) {
		RemoveEventCallback("resize", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsResizeEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument)) const {
		return ExistsEventCallback("resize", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

	template<typename T> void AddLeftDownEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&), bool prior = false) {
		AddEventCallback("leftdown", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveLeftDownEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) {
		RemoveEventCallback("leftdown", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsLeftDownEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) const {
		return ExistsEventCallback("leftdown", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}

	template<typename T> void AddLeftUpEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&), bool prior = false) {
		AddEventCallback("leftup", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveLeftUpEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) {
		RemoveEventCallback("leftup", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsLeftUpEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) const {
		return ExistsEventCallback("leftup", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}

	template<typename T> void AddLeftDoubleClickEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&), bool prior = false) {
		AddEventCallback("leftdoubleclick", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveLeftDoubleClickEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) {
		RemoveEventCallback("leftdoubleclick", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsLeftDoubleClickEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) const {
		return ExistsEventCallback("leftdoubleclick", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}

	template<typename T> void AddMiddleDownEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&), bool prior = false) {
		AddEventCallback("middledown", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveMiddleDownEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) {
		RemoveEventCallback("middledown", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsMiddleDownEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) const {
		return ExistsEventCallback("middledown", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}

	template<typename T> void AddMiddleUpEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&), bool prior = false) {
		AddEventCallback("middleup", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveMiddleUpEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) {
		RemoveEventCallback("middleup", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsMiddleUpEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) const {
		return ExistsEventCallback("middleup", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}

	template<typename T> void AddMiddleDoubleClickEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&), bool prior = false) {
		AddEventCallback("middledoubleclick", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveMiddleDoubleClickEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) {
		RemoveEventCallback("middledoubleclick", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsMiddleDoubleClickEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) const {
		return ExistsEventCallback("middledoubleclick", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}

	template<typename T> void AddRightDownEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&), bool prior = false) {
		AddEventCallback("rightdown", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveRightDownEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) {
		RemoveEventCallback("rightdown", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsRightDownEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) const {
		return ExistsEventCallback("rightdown", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}

	template<typename T> void AddRightUpEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&), bool prior = false) {
		AddEventCallback("rightup", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveRightUpEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) {
		RemoveEventCallback("rightup", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsRightUpEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) const {
		return ExistsEventCallback("rightup", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}

	template<typename T> void AddRightDoubleClickEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&), bool prior = false) {
		AddEventCallback("rightdoubleclick", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveRightDoubleClickEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) {
		RemoveEventCallback("rightdoubleclick", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsRightDoubleClickEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) const {
		return ExistsEventCallback("rightdoubleclick", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}

	template<typename T> void AddMouseMoveEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&), bool prior = false) {
		AddEventCallback("mousemove", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveMouseMoveEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) {
		RemoveEventCallback("mousemove", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsMouseMoveEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) const {
		return ExistsEventCallback("mousemove", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}

	template<typename T> void AddMouseWheelEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&), bool prior = false) {
		AddEventCallback("mousewheel", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveMouseWheelEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) {
		RemoveEventCallback("mousewheel", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsMouseWheelEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetMouseParam&)) const {
		return ExistsEventCallback("mousewheel", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetMouseEventClosure<T>(p, f)));
	}

	template<typename T> void AddKeyDownEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetKeyParam&), bool prior = false) {
		AddEventCallback("keydown", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetKeyEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveKeyDownEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetKeyParam&)) {
		RemoveEventCallback("keydown", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetKeyEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsKeyDownEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetKeyParam&)) const {
		return ExistsEventCallback("keydown", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetKeyEventClosure<T>(p, f)));
	}

	template<typename T> void AddKeyUpEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetKeyParam&), bool prior = false) {
		AddEventCallback("keyup", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetKeyEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveKeyUpEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetKeyParam&)) {
		RemoveEventCallback("keyup", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetKeyEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsKeyUpEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetKeyParam&)) const {
		return ExistsEventCallback("keyup", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetKeyEventClosure<T>(p, f)));
	}

	template<typename T> void AddTimerEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument), unsigned int timeout_ms, bool overwrite = false) {
		AddTimerEventCallback(MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), timeout_ms, overwrite);
	}
	template<typename T> void RemoveTimerEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument)) {
		RemoveEventCallback("timer", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsTimerEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument)) const {
		return ExistsEventCallback("timer", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

public:
	enum {
		NullID = 0,
	};

protected:
	int m_ID;
	bool m_IDOwner;

	void AddEventCallback(const char *event_type, MQWidgetSharedPtr<MQWidgetEventClosureBase> closure, bool prior);
	void RemoveEventCallback(const char *event_type, MQWidgetSharedPtr<MQWidgetEventClosureBase> closure);
	bool ExistsEventCallback(const char *event_type, MQWidgetSharedPtr<MQWidgetEventClosureBase> closure) const;
	static BOOL MQAPICALL EventCallback(MQDocument doc, void **params, void *option);

	void AddTimerEventCallback(MQWidgetSharedPtr<MQWidgetEventClosureBase> closure, unsigned int timeout_ms, bool overwrite);

private:
	std::vector<MQWidgetSharedPtr<MQWidgetEventClosureBase>> m_Events;

};


// Base window
// ベースウィンドウ
class MQWindowBase : public MQWidgetBase
{
public:
	// Destructor
	// デストラクタ
	~MQWindowBase();

	void AddChildWindow(MQWindowBase *child);
	void RemoveChildWindow(MQWindowBase *child);

	// A widget created by Create****() must be deleted by DeleteWidget() or will be automatically deleted together when this window is deleted.
	// Create****()で作成したウィジェットはDeleteWidget()で削除するか、またはこのウィンドウ削除時に自動的に削除されます。
	MQFrame *CreateHorizontalFrame(MQWidgetBase *parent);
	MQFrame *CreateVerticalFrame(MQWidgetBase *parent);
	MQGroupBox *CreateGroupBox(MQWidgetBase *parent, const std::wstring& str = std::wstring());
	MQTab *CreateTab(MQWidgetBase *parent);
	MQButton *CreateButton(MQWidgetBase *parent, const std::wstring& str = std::wstring());
	MQCheckBox *CreateCheckBox(MQWidgetBase *parent, const std::wstring& str = std::wstring());
	MQRadioButton *CreateRadioButton(MQWidgetBase *parent, const std::wstring& str = std::wstring());
	MQComboBox *CreateComboBox(MQWidgetBase *parent);
	MQListBox *CreateListBox(MQWidgetBase *parent);
	MQCheckListBox *CreateCheckListBox(MQWidgetBase *parent);
	MQTreeListBox *CreateTreeListBox(MQWidgetBase *parent);
	MQLabel *CreateLabel(MQWidgetBase *parent, const std::wstring& str = std::wstring());
	MQEdit *CreateEdit(MQWidgetBase *parent, const std::wstring& str = std::wstring());
	MQMemo *CreateMemo(MQWidgetBase *parent);
	MQSpinBox *CreateSpinBox(MQWidgetBase *parent);
	MQDoubleSpinBox *CreateDoubleSpinBox(MQWidgetBase *parent);
	MQSlider *CreateSlider(MQWidgetBase *parent);
	MQProgressBar *CreateProgressBar(MQWidgetBase *parent);
	MQScrollBar *CreateScrollBar(MQWidgetBase *parent);
	MQScrollBox *CreateScrollBox(MQWidgetBase *parent);
	MQColorPanel *CreateColorPanel(MQWidgetBase *parent);
	MQPaintBox *CreatePaintBox(MQWidgetBase *parent);

	// Delete a widget created by Create****().
	// Create****()で作成したウィジェットを削除します。
	void DeleteWidget(MQWidgetBase *widget);

	// Set this window as a modal window.
	void SetModal();
	// Release a modal status.
	void ReleaseModal();
	// Check if this window has a modal status.
	bool IsModal();

	// Get a window title.
	// ウィンドウのタイトル文字列を取得します。
	std::wstring GetTitle();

	// Set a window title.
	// ウィンドウのタイトル文字列を設定します。
	void SetTitle(const std::wstring& text);

	int GetPosX() const;
	void SetPosX(int value);

	int GetPosY() const;
	void SetPosY(int value);

	bool GetWindowFrame() const;
	void SetWindowFrame(bool value);

	bool GetTitleBar() const;
	void SetTitleBar(bool value);

	bool GetCanResize() const;
	void SetCanResize(bool value);

	bool GetCloseButton() const;
	void SetCloseButton(bool value);

	bool GetMaximizeButton() const;
	void SetMaximizeButton(bool value);

	bool GetMinimizeButton() const;
	void SetMinimizeButton(bool value);

	bool GetMaximized() const;
	void SetMaximized(bool value);

	bool GetMinimized() const;
	void SetMinimized(bool value);

	void GetBackColor(int& r, int& g, int& b) const;
	void SetBackColor(int r, int g, int b);

	int GetAlphaBlend() const;
	void SetAlphaBlend(int value);

	bool GetAcceptDrops() const;
	void SetAcceptDrops(bool value);

	template<typename T> void AddDragOverEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetDragOverParam&), bool prior = false) {
		AddEventCallback("dragover", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetDragOverEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveDragOverEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetDragOverParam&)) {
		RemoveEventCallback("dragover", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetDragOverEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsDragOverEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetDragOverParam&)) const {
		return ExistsEventCallback("dragover", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetDragOverEventClosure<T>(p, f)));
	}

	template<typename T> void AddDropFilesEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetDropFilesParam&), bool prior = false) {
		AddEventCallback("dropfiles", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetDropFilesEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveDropFilesEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetDropFilesParam&)) {
		RemoveEventCallback("dropfiles", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetDropFilesEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsDropFilesEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetDropFilesParam&)) const {
		return ExistsEventCallback("dropfiles", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetDropFilesEventClosure<T>(p, f)));
	}

protected:
	// Constructor
	// コンストラクタ
	MQWindowBase();
	MQWindowBase(int id);

private:
	std::set<MQWidgetBase*> m_CreatedWidgets;
};


// Window
// ウィンドウ
class MQWindow : public MQWindowBase
{
public:
	// Constructor
	// コンストラクタ
	MQWindow();
	MQWindow(int id);
	MQWindow(MQWindowBase& parent);

	// Destructor
	// デストラクタ
	~MQWindow();

	static MQWindow GetMainWindow();
	static MQWindow *CreateWindow(MQWindowBase& parent);

private:
	void CreateWindowID();
};


// Dialog
// ダイアログ
class MQDialog : public MQWindowBase
{
public:
	enum DIALOG_RESULT {
		DIALOG_NONE = 0,
		DIALOG_OK,
		DIALOG_CANCEL,
		DIALOG_YES,
		DIALOG_NO,
		DIALOG_ALL,
	};

public:
	// Constructor
	// コンストラクタ
	MQDialog();
	MQDialog(int id);
	MQDialog(MQWindowBase& parent);

	// Destructor
	// デストラクタ
	~MQDialog();

	DIALOG_RESULT Execute();
	void Close(DIALOG_RESULT code = DIALOG_NONE);

	static DIALOG_RESULT MessageWarningBox(MQWindowBase& parent, const std::wstring& message, const std::wstring& title);
	static DIALOG_RESULT MessageInformationBox(MQWindowBase& parent, const std::wstring& message, const std::wstring& title);
	static DIALOG_RESULT MessageYesNoBox(MQWindowBase& parent, const std::wstring& message, const std::wstring& title);
	static DIALOG_RESULT MessageYesNoCancelBox(MQWindowBase& parent, const std::wstring& message, const std::wstring& title);
	static DIALOG_RESULT MessageOkCancelBox(MQWindowBase& parent, const std::wstring& message, const std::wstring& title);

protected:
	MQDialog(MQWindowBase& parent, int dummy_for_no_creation);
private:
	void CreateDialogID();
};


// Popup
// ポップアップ
class MQPopup : public MQWindowBase
{
public:
	// Constructor
	// コンストラクタ
	MQPopup();
	MQPopup(int id);
	MQPopup(MQWindowBase& parent);

	// Destructor
	// デストラクタ
	~MQPopup();

	MQMenuItem *CreateMenuItem(const std::wstring& text = std::wstring());
	MQMenuItem *CreateSubMenuItem(MQMenuItem *parent, const std::wstring& text = std::wstring());

	void GetPreferredSidePosition(int& x, int& y, int& w, int& h, MQWidgetBase *widget, bool horz);

	void ShowPopup(int screen_x, int screen_y);

	bool GetShadow();
	void SetShadow(bool value);

	bool GetOverlay();
	void SetOverlay(bool value);

private:
	void CreatePopupID();

	std::set<MQMenuItem*> m_CreatedMenuItems;
};


// Frame
// フレーム
class MQFrameBase : public MQWidgetBase
{
public:
	enum MQFrameAlignment {
		ALIGN_NONE,
		ALIGN_HORIZONTAL,
		ALIGN_VERTICAL,
	};

protected:
	// Constructor
	// コンストラクタ
	MQFrameBase();
	MQFrameBase(int id);

public:
	// Destructor
	// デストラクタ
	~MQFrameBase();

	// Get a direction of an alignment for child widgets.
	// 子ウィジェットを配置する方向を取得します。
	MQFrameAlignment GetAlignment();

	// Set a direction of an alignment for child widgets.
	// 子ウィジェットを配置する方向を指定します。
	void SetAlignment(MQFrameAlignment align);

	// Check if children can be aligned on a single column or multiple columns.
	// 幅に応じて子ウィジェットを自動的に複数行に分割するかを取得します。
	bool GetMultiColumn();

	// Specify that children can be aligned on a single column or multiple columns.
	// 幅に応じて子ウィジェットを自動的に複数行に分割するかを設定します。
	void SetMultiColumn(bool value);

	// Get a number of chilren in a column.
	// １行に配置可能な子ウィジェットの数を取得します。
	int GetMatrixColumn();

	// Set a number of chilren in a column.
	// １行に配置可能な子ウィジェットの数を設定します。
	void SetMatrixColumn(int value);

	// Check if all children has the same width/height.
	// 子ウィジェットの幅を同じにするかどうかを取得します。
	bool GetUniformSize();

	// Specify that all children has the same width/height.
	// 子ウィジェットの幅を同じにするかどうかを設定します。
	void SetUniformSize(bool value);

	// Check if inner widgets can be resized by a dragging.
	// ドラッグにより内部のウィジェットの幅を変更可能にするかどうかを取得します。
	bool GetSplit();

	// Specify that inner widgets can be resized by a dragging.
	// ドラッグにより内部のウィジェットの幅を変更可能にするかどうかを設定します。
	void SetSplit(bool value);

};


// Frame
// フレーム
class MQFrame : public MQFrameBase
{
public:
	// Constructor
	// コンストラクタ
	MQFrame();
	MQFrame(int id);

	// Destructor
	// デストラクタ
	~MQFrame();

	void GetBackColor(int& r, int& g, int& b, int& a);
	void SetBackColor(int r, int g, int b, int a);

};


// GroupBox
// グループボックス
class MQGroupBox : public MQFrameBase
{
public:
	// Constructor
	// コンストラクタ
	MQGroupBox();
	MQGroupBox(int id);

	// Destructor
	// デストラクタ
	~MQGroupBox();

	std::wstring GetText();
	void SetText(const std::wstring& text);

	bool GetCanMinimize();
	void SetCanMinimize(bool value);

	bool GetMinimized();
	void SetMinimized(bool value);

	bool GetShowTitle();
	void SetShowTitle(bool value);

};


// ScrollBox
// スクロールボックス
class MQScrollBox : public MQWidgetBase
{
public:
	enum MQScrollBoxBarStatus {
		SCROLLBAR_AUTO = 0,
		SCROLLBAR_OFF,
		SCROLLBAR_ON,
	};

public:
	// Constructor
	// コンストラクタ
	MQScrollBox();
	MQScrollBox(int id);

	// Destructor
	// デストラクタ
	~MQScrollBox();

	MQScrollBoxBarStatus GetHorzBarStatus();
	void SetHorzBarStatus(MQScrollBoxBarStatus value);

	MQScrollBoxBarStatus GetVertBarStatus();
	void SetVertBarStatus(MQScrollBoxBarStatus value);

	bool GetAutoWidgetScroll();
	void SetAutoWidgetScroll(bool value);


};


// Tab
// タブ
class MQTab : public MQWidgetBase
{
public:
	// Constructor
	// コンストラクタ
	MQTab();
	MQTab(int id);

	// Destructor
	// デストラクタ
	~MQTab();

	std::wstring GetTabTitle(int index);
	void SetTabTitle(int index, const std::wstring& text);

	int GetCurrentPage();
	void SetCurrentPage(int value);

	bool GetShowTab();
	void SetShowTab(bool value);

	bool GetExclusive();
	void SetExclusive(bool value);


	template<typename T> void AddChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

};


// Button
// ボタン
class MQButton : public MQWidgetBase
{
public:
	enum MQButtonTextAlignment {
		ALIGN_LEFT,
		ALIGN_RIGHT,
		ALIGN_CENTER,
		ALIGN_CENTER_EXCEPT_IMAGE,
	};

	enum MQButtonImagePosition {
		IMAGE_LEFT = 0,
		IMAGE_TOP,
		IMAGE_RIGHT,
		IMAGE_BOTTOM,
		IMAGE_ONLY,
	};

public:
	// Constructor
	// コンストラクタ
	MQButton();
	MQButton(int id);

	// Destructor
	// デストラクタ
	~MQButton();

	// Get a text displayed on the button.
	// ボタン上に表示される文字列を取得します。
	std::wstring GetText();

	// Set a text displayed on the button.
	// ボタン上に表示される文字列を設定します。
	void SetText(const std::wstring& text);

	std::wstring GetFontName();
	void SetFontName(const std::wstring& value);

	bool GetFontBold();
	void SetFontBold(bool value);

	void GetFontColor(int& r, int& g, int& b, int& a);
	void SetFontColor(int r, int g, int b, int a);

	double GetFontScale();
	void SetFontScale(double value);

	MQButtonTextAlignment GetAlignment();
	void SetAlignment(MQButtonTextAlignment value);

	// Check if the button can be automatically switched.
	// 自動的にボタンのOn/Offを切り替えるどうかを取得します。
	bool GetToggle();

	// Specify that the button can be automatically switched.
	// 自動的にボタンのOn/Offを切り替えるどうかを設定します。
	void SetToggle(bool value);

	// Check if the button is down or up.
	// ボタンが押されているかどうかを取得します。
	bool GetDown();

	// Specify that the button is down or up.
	// ボタンが押されているかどうかを設定します。
	void SetDown(bool value);

	// Check if events are invoked repeatly while pushing the button.
	// ボタンが押し続けられたときにイベントを連続発生させるかどうかを取得します。
	bool GetRepeat();

	// Specify that events are invoked repeatly while pushing the button.
	// ボタンが押し続けられたときにイベントを連続発生させるかどうかを設定します。
	void SetRepeat(bool value);

	// Check if the button is displayed as connected with the neighbor buttons.
	// ボタンが隣のボタンとつながっているように表示されるかどうかを取得します。
	bool GetChain();

	// Specify that the button is displayed as connected with the neighbor buttons.
	// ボタンが隣のボタンとつながっているように表示されるかどうかを設定します。
	void SetChain(bool value);

	double GetPaddingX();
	void SetPaddingX(double value);

	double GetPaddingY();
	void SetPaddingY(double value);

	// Set as a default button called when an Enter key is pushed.
	// Enterキーを押したときに動作するデフォルトボタンとして設定します。
	void SetDefault(bool value);

	// Set as a cancel button called when an Esc key is pushed.
	// キャンセルボタンとして設定します。
	void SetCancel(bool value);

	// Set a modal result for a dialog.
	// ダイアログ用の結果を設定します。
	void SetModalResult(MQDialog::DIALOG_RESULT value);

	void SetSystemSVGFile(const wchar_t *filename);

	double GetImageScale();
	void SetImageScale(double value);

	MQButtonImagePosition GetImagePosition();
	void SetImagePosition(MQButtonImagePosition value);


	template<typename T> void AddClickEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveClickEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsClickEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

	template<typename T> void AddRepeatEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveRepeatEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsRepeatEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

};


// Check box
// チェックボックス
class MQCheckBox : public MQWidgetBase
{
public:
	// Constructor
	// コンストラクタ
	MQCheckBox();
	MQCheckBox(int id);

	// Destructor
	// デストラクタ
	~MQCheckBox();

	// Get a checked status.
	// チェックされているかどうかを取得します。
	bool GetChecked();

	// Set a checked status.
	// チェックされているかどうかを設定します。
	void SetChecked(bool value);

	// Get a text displayed beside the check box.
	// チェックボックスの横に表示される文字列を取得します。
	std::wstring GetText();

	// Set a text displayed beside the check box.
	// チェックボックスの横に表示される文字列を指定します。
	void SetText(const std::wstring& text);

	template<typename T> void AddChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

};


// Radio button
// チェックボックス
class MQRadioButton : public MQWidgetBase
{
public:
	// Constructor
	// コンストラクタ
	MQRadioButton();
	MQRadioButton(int id);

	// Destructor
	// デストラクタ
	~MQRadioButton();

	// Get a checked status.
	// チェックされているかどうかを取得します。
	bool GetChecked();

	// Set a checked status.
	// チェックされているかどうかを設定します。
	void SetChecked(bool value);

	// Get a text displayed beside the radio button.
	// チェックボックスの横に表示される文字列を取得します。
	std::wstring GetText();

	// Set a text displayed beside the radio button.
	// チェックボックスの横に表示される文字列を指定します。
	void SetText(const std::wstring& text);

	template<typename T> void AddChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

};


// Combo box
// コンボボックス
class MQComboBox : public MQWidgetBase
{
public:
	// Constructor
	// コンストラクタ
	MQComboBox();
	MQComboBox(int id);

	// Destructor
	// デストラクタ
	~MQComboBox();

	int GetCurrentIndex();
	void SetCurrentIndex(int value);

	int AddItem(const std::wstring& text);
	int AddItem(const std::wstring& text, __int64 tag);
	void DeleteItem(int index);
	void ClearItems();
	int GetItemCount();
	std::wstring GetItem(int index);
	void SetItem(int index, const std::wstring& text);
	__int64 GetItemTag(int index);
	void SetItemTag(int index, __int64 tag);

	int GetNumVisible();
	void SetNumVisible(int value);


	template<typename T> void AddChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

};


// List box
// リストボックス
class MQListBox : public MQWidgetBase
{
public:
	// Constructor
	// コンストラクタ
	MQListBox();
	MQListBox(int id);

	// Destructor
	// デストラクタ
	~MQListBox();

	int GetCurrentIndex();
	void SetCurrentIndex(int value);

	void BeginUpdate();
	void EndUpdate();
	int AddItem(const std::wstring& text);
	int AddItem(const std::wstring& text, __int64 tag);
	void DeleteItem(int index);
	void ClearItems();
	int GetItemCount();
	std::wstring GetItem(int index);
	void SetItem(int index, const std::wstring& text);
	__int64 GetItemTag(int index);
	void SetItemTag(int index, __int64 tag);
	bool GetItemSelected(int index);
	void SetItemSelected(int index, bool selected);
	std::wstring GetItemHint(int index);
	void SetItemHint(int index, const std::wstring& hint);

	int GetVisibleRow();
	void SetVisibleRow(int value);
	double GetLineHeightRate();
	void SetLineHeightRate(double value);
	bool GetVertScrollVisible();
	void SetVertScrollVisible(bool value);
	bool GetMultiSelect();
	void SetMultiSelect(bool value);

	void ClearSelection();

	void MakeItemVisible(int index);
	int HitTestItem(int x, int y);
	void GetItemRect(int index, int& x, int& y, int& w, int& h);

	template<typename T> void AddChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> void AddDrawItemEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc, MQListBoxDrawItemParam& param), bool prior = false) {
		AddEventCallback("listdrawitem", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQListBoxDrawItemEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveDrawItemEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc, MQListBoxDrawItemParam& param)) {
		RemoveEventCallback("listdrawitem", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQListBoxDrawItemEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsDrawItemEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc, MQListBoxDrawItemParam& param)) const {
		return ExistsEventCallback("listdrawitem", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQListBoxDrawItemEventClosure<T>(p, f)));
	}

};


// Check List box
// チェックリストボックス
class MQCheckListBox : public MQWidgetBase
{
public:
	// Constructor
	// コンストラクタ
	MQCheckListBox();
	MQCheckListBox(int id);

	// Destructor
	// デストラクタ
	~MQCheckListBox();

	int GetCurrentIndex();
	void SetCurrentIndex(int value);

	void BeginUpdate();
	void EndUpdate();
	int AddItem(const std::wstring& text);
	int AddItem(const std::wstring& text, __int64 tag);
	void DeleteItem(int index);
	void ClearItems();
	int GetItemCount();
	std::wstring GetItem(int index);
	void SetItem(int index, const std::wstring& text);
	__int64 GetItemTag(int index);
	void SetItemTag(int index, __int64 tag);
	bool GetItemSelected(int index);
	void SetItemSelected(int index, bool selected);
	bool GetItemChecked(int index);
	void SetItemChecked(int index, bool checked);
	std::wstring GetItemHint(int index);
	void SetItemHint(int index, const std::wstring& hint);

	int GetVisibleRow();
	void SetVisibleRow(int value);
	double GetLineHeightRate();
	void SetLineHeightRate(double value);
	bool GetVertScrollVisible();
	void SetVertScrollVisible(bool value);
	bool GetMultiSelect();
	void SetMultiSelect(bool value);

	void ClearSelection();

	void MakeItemVisible(int index);
	int HitTestItem(int x, int y);
	void GetItemRect(int index, int& x, int& y, int& w, int& h);

	template<typename T> void AddChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}


};


// Tree List box
// ツリーリストボックス
class MQTreeListBox : public MQWidgetBase
{
public:
	enum {
		NullItemID = -1,
	};
public:
	// Constructor
	// コンストラクタ
	MQTreeListBox();
	MQTreeListBox(int id);

	// Destructor
	// デストラクタ
	~MQTreeListBox();

	int GetCurrentID();
	void SetCurrentID(int value);

	void BeginUpdate();
	void EndUpdate();
	int AddItem(const std::wstring& text);
	int AddItem(const std::wstring& text, __int64 tag);
	int AddItem(int parent_id, const std::wstring& text);
	int AddItem(int parent_id, const std::wstring& text, __int64 tag);
	void DeleteItem(int id);
	void ClearItems();
	void ReparentItem(int id, int new_parent_id);
	int GetItemCount();
	int GetItemIDByIndex(int index);
	int GetItemParent(int id);
	std::vector<int> GetItemChildren(int id);
	std::wstring GetItem(int id);
	void SetItem(int id, const std::wstring& text);
	__int64 GetItemTag(int id);
	void SetItemTag(int id, __int64 tag);
	bool GetItemSelected(int id);
	void SetItemSelected(int id, bool selected);
	std::wstring GetItemHint(int id);
	void SetItemHint(int id, const std::wstring& hint);
	bool GetItemCollapsed(int id);
	void SetItemCollapsed(int id, bool collapsed);

	int GetVisibleRow();
	void SetVisibleRow(int value);
	double GetLineHeightRate();
	void SetLineHeightRate(double value);
	bool GetHorzScrollVisible();
	void SetHorzScrollVisible(bool value);
	bool GetVertScrollVisible();
	void SetVertScrollVisible(bool value);
	bool GetMultiSelect();
	void SetMultiSelect(bool value);

	void ClearSelection();

	void MakeItemVisible(int id);
	int HitTestItem(int x, int y);
	void GetItemRect(int id, int& x, int& y, int& w, int& h);

	template<typename T> void AddChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> void AddDrawItemEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc, MQTreeListBoxDrawItemParam& param), bool prior = false) {
		AddEventCallback("treelistdrawitem", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQTreeListBoxDrawItemEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveDrawItemEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc, MQTreeListBoxDrawItemParam& param)) {
		RemoveEventCallback("treelistdrawitem", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQTreeListBoxDrawItemEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsDrawItemEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc, MQTreeListBoxDrawItemParam& param)) const {
		return ExistsEventCallback("treelistdrawitem", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQTreeListBoxDrawItemEventClosure<T>(p, f)));
	}

};


// Label
// ラベル
class MQLabel : public MQWidgetBase
{
public:
	enum MQLabelTextAlignment {
		ALIGN_LEFT,
		ALIGN_RIGHT,
		ALIGN_CENTER,
	};

public:
	// Constructor
	// コンストラクタ
	MQLabel();
	MQLabel(int id);

	// Destructor
	// デストラクタ
	~MQLabel();

	// Get a displayed text.
	// 表示される文字列を取得します。
	std::wstring GetText();

	// Set a displayed text.
	// 表示される文字列を指定します。
	void SetText(const std::wstring& text);

	std::wstring GetFontName();
	void SetFontName(const std::wstring& value);

	bool GetFontBold();
	void SetFontBold(bool value);

	void GetFontColor(int& r, int& g, int& b, int& a);
	void SetFontColor(int r, int g, int b, int a);

	double GetFontScale();
	void SetFontScale(double value);

	MQLabelTextAlignment GetAlignment();
	void SetAlignment(MQLabelTextAlignment value);

	bool GetVerticalText();
	void SetVerticalText(bool value);

	bool GetWordWrap();
	void SetWordWrap(bool value);

	bool GetFrame();
	void SetFrame(bool value);

	bool GetShadowText();
	void SetShadowText(bool value);

};


// Edit
// 文字入力
class MQEdit : public MQWidgetBase
{
public:
	enum MQEditTextAlignment {
		ALIGN_LEFT,
		ALIGN_RIGHT,
	};

	enum MQEditNumericType {
		NUMERIC_TEXT,
		NUMERIC_INT,
		NUMERIC_DOUBLE,
	};

public:
	// Constructor
	// コンストラクタ
	MQEdit();
	MQEdit(int id);

	// Destructor
	// デストラクタ
	~MQEdit();

	// Get a displayed text.
	// 表示される文字列を取得します。
	std::wstring GetText();

	// Set a displayed text.
	// 表示される文字列を指定します。
	void SetText(const std::wstring& text);

	std::wstring GetFontName();
	void SetFontName(const std::wstring& value);

	bool GetFontBold();
	void SetFontBold(bool value);

	double GetFontScale();
	void SetFontScale(double value);

	bool GetReadOnly();
	void SetReadOnly(bool value);

	bool GetPassword();
	void SetPassword(bool value);

	MQEditTextAlignment GetAlignment();
	void SetAlignment(MQEditTextAlignment value);

	MQEditNumericType GetNumeric();
	void SetNumeric(MQEditNumericType value);

	int GetMaxLength();
	void SetMaxLength(int value);

	int GetMaxAnsiLength();
	void SetMaxAnsiLength(int value);

	int GetVisibleColumn();
	void SetVisibleColumn(int value);

	void SetFocus(bool value);

	template<typename T> void AddChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

	template<typename T> void AddChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

};


// Text input with multiple columns
// 複数行テキスト入力
class MQMemo : public MQWidgetBase
{
public:
	// Constructor
	// コンストラクタ
	MQMemo();
	MQMemo(int id);

	// Destructor
	// デストラクタ
	~MQMemo();

	void AddLine(const std::wstring& str);
	std::wstring GetLine(int line_index);

	// Get a displayed text.
	// 表示される文字列を取得します。
	std::wstring GetText();

	// Set a displayed text.
	// 表示される文字列を指定します。
	void SetText(const std::wstring& text);

	std::wstring GetFontName();
	void SetFontName(const std::wstring& value);

	bool GetFontBold();
	void SetFontBold(bool value);

	double GetFontScale();
	void SetFontScale(double value);

	bool GetReadOnly();
	void SetReadOnly(bool value);

	void SetFocus(bool value);

	template<typename T> void AddChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

	template<typename T> void AddChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

};


// SpinBox
// 整数入力
class MQSpinBox : public MQWidgetBase
{
public:
	enum MQSpinBoxTextAlignment {
		ALIGN_LEFT,
		ALIGN_RIGHT,
	};

public:
	// Constructor
	// コンストラクタ
	MQSpinBox();
	MQSpinBox(int id);

	// Destructor
	// デストラクタ
	~MQSpinBox();

	int GetPosition();
	void SetPosition(int value);

	int GetMin();
	void SetMin(int value);

	int GetMax();
	void SetMax(int value);

	int GetIncrement();
	void SetIncrement(int value);

	MQSpinBoxTextAlignment GetAlignment();
	void SetAlignment(MQSpinBoxTextAlignment value);

	int GetVisibleColumn();
	void SetVisibleColumn(int value);

	void SetFocus(bool value);


	template<typename T> void AddChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

	template<typename T> void AddChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

};


// DoubleSpinBox
// 整数入力
class MQDoubleSpinBox : public MQWidgetBase
{
public:
	enum MQDoubleSpinBoxTextAlignment {
		ALIGN_LEFT,
		ALIGN_RIGHT,
	};

public:
	// Constructor
	// コンストラクタ
	MQDoubleSpinBox();
	MQDoubleSpinBox(int id);

	// Destructor
	// デストラクタ
	~MQDoubleSpinBox();

	double GetPosition();
	void SetPosition(double value);

	double GetMin();
	void SetMin(double value);

	double GetMax();
	void SetMax(double value);

	double GetIncrement();
	void SetIncrement(double value);

	bool GetExponential();
	void SetExponential(bool value);

	double GetMantissa();
	void SetMantissa(double value);

	int GetDecimalDigit();
	void SetDecimalDigit(int value);

	bool GetVariableDigit();
	void SetVariableDigit(bool value);

	int GetAutoDigit();
	void SetAutoDigit(int value);

	int GetMaxDecimalDigit();
	void SetMaxDecimalDigit(int value);

	MQDoubleSpinBoxTextAlignment GetAlignment();
	void SetAlignment(MQDoubleSpinBoxTextAlignment value);

	int GetVisibleColumn();
	void SetVisibleColumn(int value);

	void SetFocus(bool value);


	template<typename T> void AddChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

	template<typename T> void AddChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

};


// Slider
// スライダー
class MQSlider : public MQWidgetBase
{
public:
	// Constructor
	// コンストラクタ
	MQSlider();
	MQSlider(int id);

	// Destructor
	// デストラクタ
	~MQSlider();

	double GetPosition();
	void SetPosition(double value);

	double GetMin();
	void SetMin(double value);

	double GetMax();
	void SetMax(double value);


	template<typename T> void AddChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

	template<typename T> void AddChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

};


// Progress bar
// プログレスバー
class MQProgressBar : public MQWidgetBase
{
public:
	// Constructor
	// コンストラクタ
	MQProgressBar();
	MQProgressBar(int id);

	// Destructor
	// デストラクタ
	~MQProgressBar();

	double GetPosition();
	void SetPosition(double value);

	double GetMin();
	void SetMin(double value);

	double GetMax();
	void SetMax(double value);
};


// Scroll bar
// スクロールバー
class MQScrollBar : public MQWidgetBase
{
public:
	enum MQScrollBarDirection {
		DIRECTION_HORIZONTAL,
		DIRECTION_VERTICAL,
	};

public:
	// Constructor
	// コンストラクタ
	MQScrollBar();
	MQScrollBar(int id);

	// Destructor
	// デストラクタ
	~MQScrollBar();

	MQScrollBarDirection GetDirection();
	void SetDirection(MQScrollBarDirection value);

	int GetPosition();
	void SetPosition(int value);

	int GetMin();
	void SetMin(int value);

	int GetMax();
	void SetMax(int value);

	int GetPage();
	void SetPage(int value);

	int GetIncrement();
	void SetIncrement(int value);


	template<typename T> void AddChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

	template<typename T> void AddChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

};


// Color panel
// カラーパネル
class MQColorPanel : public MQWidgetBase
{
public:
	// Constructor
	// コンストラクタ
	MQColorPanel();
	MQColorPanel(int id);

	// Destructor
	// デストラクタ
	~MQColorPanel();

	void GetColor(int& r, int& g, int& b);
	void SetColor(int r, int g, int b);

	void GetHSV(double& h, double& s, double& v);
	void SetHSV(double h, double s, double v);


	template<typename T> void AddChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangedEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

	template<typename T> void AddChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsChangingEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("changing", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

};


// Paint box
// ペイントボックス
class MQPaintBox : public MQWidgetBase
{
public:
	// Constructor
	// コンストラクタ
	MQPaintBox();
	MQPaintBox(int id);

	// Destructor
	// デストラクタ
	~MQPaintBox();


	template<typename T> void AddPaintEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetPaintParam&), bool prior = false) {
		AddEventCallback("paint", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetPaintEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemovePaintEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetPaintParam&)) {
		RemoveEventCallback("paint", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetPaintEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsPaintEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument, MQWidgetPaintParam&)) const {
		return ExistsEventCallback("paint", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetPaintEventClosure<T>(p, f)));
	}

};


// Menu item
// メニューアイテム
class MQMenuItem : public MQWidgetBase
{
public:
	// Constructor
	// コンストラクタ
	MQMenuItem(MQWidgetBase *parent);
	MQMenuItem(int id);

	// Destructor
	// デストラクタ
	~MQMenuItem();

	std::wstring GetText();
	void SetText(const std::wstring& text);

	bool GetChecked();
	void SetChecked(bool val);

	bool GetToggle();
	void SetToggle(bool val);

	bool GetClickClose();
	void SetClickClose(bool val);

	bool GetSeparator();
	void SetSeparator(bool val);

	template<typename T> void AddClickEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc), bool prior = false) {
		AddEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)), prior);
	}
	template<typename T> void RemoveClickEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) {
		RemoveEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}
	template<typename T> bool ExistsClickEvent(T *p, BOOL (T::*f)(MQWidgetBase*, MQDocument doc)) const {
		return ExistsEventCallback("execute", MQWidgetSharedPtr<MQWidgetEventClosureBase>(new MQWidgetEventClosure<T>(p, f)));
	}

};


// MQColorDialog
// カラーダイアログ
class MQColorDialog : public MQDialog
{
public:
	MQColorDialog(MQWindowBase& parent);
	virtual ~MQColorDialog();

	void GetColor(int& r, int& g, int& b);
	void SetColor(int r, int g, int b);

	void GetHSV(double& h, double& s, double& v);
	void SetHSV(double h, double s, double v);

private:
	void CreateColorDialogID();
};


// MQFileDialogBase
// ファイルダイアログ
class MQFileDialogBase
{
protected:
	// Constructor
	// コンストラクタ
	MQFileDialogBase();

public:
	// Destructor
	// デストラクタ
	virtual ~MQFileDialogBase();

	int AddFilter(const std::wstring& val);
	int GetFilterCount() const;
	std::wstring GetFilter(int index) const;
	void SetFilter(int index, const std::wstring& val);
	void ClearFilters();

	std::wstring GetTitle() const;
	void SetTitle(const std::wstring& val);

	std::wstring GetFileName() const;
	void SetFileName(const std::wstring& val);

	std::wstring GetInitialDir() const;
	void SetInitialDir(const std::wstring& val);

	std::wstring GetDefaultExt() const;
	void SetDefaultExt(const std::wstring& val);

	int GetFilterIndex() const;
	void SetFilterIndex(int val);

	bool GetNoChangeDir() const;
	void SetNoChangeDir(bool val);

public:
	enum {
		NullID = 0,
	};

protected:
	int m_ID;

};


class MQOpenFileDialog : public MQFileDialogBase
{
public:
	MQOpenFileDialog(MQWindowBase& parent);
	virtual ~MQOpenFileDialog();

	bool GetFileMustExist() const;
	void SetFileMustExist(bool val);

	bool GetMultiSelect() const;
	void SetMultiSelect(bool val);

	bool Execute();

	int GetFileNamesCount() const;
	std::wstring GetFileNames(int index) const;

};


class MQSaveFileDialog : public MQFileDialogBase
{
public:
	MQSaveFileDialog(MQWindowBase& parent);
	virtual ~MQSaveFileDialog();

	bool Execute();
};


// MQFolderDialog
// ファイルダイアログ
class MQFolderDialog
{
public:
	// Constructor
	// コンストラクタ
	MQFolderDialog(MQWindowBase& parent);
	// Destructor
	// デストラクタ
	virtual ~MQFolderDialog();

	std::wstring GetTitle() const;
	void SetTitle(const std::wstring& val);

	std::wstring GetFolder() const;
	void SetFolder(const std::wstring& val);

	bool GetNewFolder() const;
	void SetNewFolder(bool val);

	bool Execute();

public:
	enum {
		NullID = 0,
	};

protected:
	int m_ID;

};


struct MQCanvasPoint
{
	float x;
	float y;

	MQCanvasPoint() { }
	MQCanvasPoint(float _x, float _y) : x(_x), y(_y) { }
	MQCanvasPoint(const MQCanvasPoint& p) : x(p.x), y(p.y) { }

	float length() const { return (float)sqrt(x*x + y*y); }
	float length2() const { return x*x + y*y; }
	float innerprod(const MQCanvasPoint& p2) const { return x*p2.x + y*p2.y; }

	bool operator == (const MQCanvasPoint& p){
		return x == p.x && y == p.y;
	}
	bool operator != (const MQCanvasPoint& p){
		return !(x == p.x && y == p.y);
	}

	MQCanvasPoint& operator = (const MQCanvasPoint& p){
		x = p.x;
		y = p.y;
		return *this;
	}
	MQCanvasPoint& operator += (const MQCanvasPoint& p){
		x += p.x;
		y += p.y;
		return *this;
	}
	MQCanvasPoint& operator -= (const MQCanvasPoint& p){
		x -= p.x;
		y -= p.y;
		return *this;
	}
	MQCanvasPoint& operator *= (const MQCanvasPoint& p){
		x *= p.x;
		y *= p.y;
		return *this;
	}
	MQCanvasPoint& operator *= (float s){
		x *= s;
		y *= s;
		return *this;
	}
	MQCanvasPoint& operator /= (float s){
		x /= s;
		y /= s;
		return *this;
	}
	friend MQCanvasPoint operator + (const MQCanvasPoint& p1, const MQCanvasPoint& p2){
		return MQCanvasPoint(p1.x+p2.x, p1.y+p2.y);
	}
	friend MQCanvasPoint operator - (const MQCanvasPoint& p1, const MQCanvasPoint& p2){
		return MQCanvasPoint(p1.x-p2.x, p1.y-p2.y);
	}
	friend MQCanvasPoint operator * (const MQCanvasPoint& p1, const MQCanvasPoint& p2){
		return MQCanvasPoint(p1.x*p2.x, p1.y*p2.y);
	}
	friend MQCanvasPoint operator * (const MQCanvasPoint& p, float s){
		return MQCanvasPoint(p.x*s, p.y*s);
	}
	friend MQCanvasPoint operator * (float s, const MQCanvasPoint& p){
		return MQCanvasPoint(p.x*s, p.y*s);
	}
	friend MQCanvasPoint operator / (const MQCanvasPoint& p, float s){
		return MQCanvasPoint(p.x/s, p.y/s);
	}
	friend MQCanvasPoint operator - (const MQCanvasPoint& p){
		return MQCanvasPoint(-p.x, -p.y);
	}
};

class MQCanvas
{
public:
	enum MQCANVAS_CAP_TYPE {
		CAP_BUTT	= 0,
		CAP_ROUND	= 1,
		CAP_SQUARE	= 2,
	};
	
	enum MQCANVAS_JOIN_TYPE {
		JOIN_MITER	= 0,
		JOIN_ROUND	= 1,
		JOIN_BEVEL	= 2,
	};
public:
	MQCanvas(void *ptr);
	virtual ~MQCanvas();

	void SetColor(int r, int g, int b, int a);
	void SetColor(const MQCanvasColor& col) { SetColor(col.r, col.g, col.b, col.a); }
	void SetGradientColor(int x1, int y1, int x2, int y2, const std::vector<MQCanvasColor>& colors, const std::vector<float>& segments);
	void SetGradientColor(float x1, float y1, float x2, float y2, const std::vector<MQCanvasColor>& colors, const std::vector<float>& segments);

	void SetStrokeWidth(float width);
	void SetStrokeCap(MQCANVAS_CAP_TYPE cap);
	void SetStrokeJoin(MQCANVAS_JOIN_TYPE join);
	void SetStrokeMiterLimit(float limit);
	void SetStrokeDash(const std::vector<float>& intervals);
	void SetFont(const wchar_t *fontname, bool bold);
	void SetFontSize(int size);
	void SetFontRateSize(float rate);
	void SetAntiAlias(bool val);

	void PushClipRect(int x, int y, int w, int h);
	void PopClipRect();

	void Clear(int r, int g, int b, int a);
	void Clear(const MQCanvasColor& col) { Clear(col.r, col.g, col.b, col.a); }

	void DrawLine(int x1, int y1, int x2, int y2);
	void DrawLine(float x1, float y1, float x2, float y2);
	void DrawPolyline(const POINT *points, int num_points);
	void DrawPolyline(const MQCanvasPoint *points, int num_points);
	void DrawCircle(int x, int y, float r);
	void DrawCircle(float x, float y, float r);
	void FillCircle(int x, int y, float r);
	void FillCircle(float x, float y, float r);
	void DrawEllipse(int x, int y, float rx, float ry);
	void DrawEllipse(float x, float y, float rx, float ry);
	void FillEllipse(int x, int y, float rx, float ry);
	void FillEllipse(float x, float y, float rx, float ry);
	void DrawRect(int x, int y, int w, int h);
	void DrawRect(float x, float y, float w, float h);
	void FillRect(int x, int y, int w, int h);
	void FillRect(float x, float y, float w, float h);
	void DrawRoundRect(int x, int y, int w, int h, int rx, int ry);
	void DrawRoundRect(float x, float y, float w, float h, float rx, float ry);
	void FillRoundRect(int x, int y, int w, int h, int rx, int ry);
	void FillRoundRect(float x, float y, float w, float h, float rx, float ry);
	void DrawPolygon(const POINT *points, int num_points);
	void DrawPolygon(const MQCanvasPoint *points, int num_points);
	void FillPolygon(const POINT *points, int num_points);
	void FillPolygon(const MQCanvasPoint *points, int num_points);
	void DrawDIB(void *header, void *buffer, int x, int y);
	void DrawDIB(void *header, void *buffer, int x, int y, int w, int h);
	void DrawText(const wchar_t *str, int x, int y, int w, int h, bool horz_center, bool vert_center = true);
	POINT MeasureText(const wchar_t *str);

private:
	void *m_Ptr;
};


#endif _MQWIDGET_H_
