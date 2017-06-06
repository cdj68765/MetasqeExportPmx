//---------------------------------------------------------------------------
//
//   MQSetting
//
//          Copyright(C) 1999-2016, tetraface Inc.
//
//	   A class for accessing .xml files used to save settings. 
//     This class supported .ini files in earlier versions. But .ini 
//    files are obsolete currently, and .xml files are substituting 
//    for them.
//
//    　Metasequoiaが設定を保存するためのxmlファイルにアクセスする
//    クラスです。
//      このクラスは以前のバージョンではiniファイルをアクセスしていま
//    したが、現在ではxmlファイルに置き換えられました。
//
//---------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "MQPlugin.h"
#include "MQSetting.h"


// Constructor
// コンストラクタ
MQSetting::MQSetting(MQXmlElement root, const char *section_name)
{
	m_Root = root;
	m_Section = section_name;

	m_Elem = m_Root->FirstChildElement(section_name);
	if(m_Elem == NULL){
		m_Elem = m_Root->AddChildElement(section_name);
	}
}

// Destructor
// デストラクタ
MQSetting::~MQSetting()
{
}

void MQSetting::Load(const char *name, bool& value, bool default_value)
{
	MQXmlElement child = m_Elem->FirstChildElement(name);
	if(child != NULL){
		std::wstring str;
		child->GetText(str);
		if(!str.empty()){
			value = _wtoi(str.c_str()) != 0;
		}else{
			value = default_value;
		}
	}else{
		value = default_value;
	}
}

void MQSetting::Load(const char *name, int& value, int default_value)
{
	MQXmlElement child = m_Elem->FirstChildElement(name);
	if(child != NULL){
		std::wstring str;
		child->GetText(str);
		if(!str.empty()){
			value = _wtoi(str.c_str());
		}else{
			value = default_value;
		}
	}else{
		value = default_value;
	}
}

void MQSetting::Load(const char *name, unsigned int& value, unsigned int default_value)
{
	MQXmlElement child = m_Elem->FirstChildElement(name);
	if(child != NULL){
		std::wstring str;
		child->GetText(str);
		if(!str.empty()){
			value = (unsigned int)_wtoi64(str.c_str());
		}else{
			value = default_value;
		}
	}else{
		value = default_value;
	}
}

void MQSetting::Load(const char *name, float& value, float default_value)
{
	MQXmlElement child = m_Elem->FirstChildElement(name);
	if(child != NULL){
		std::wstring str;
		child->GetText(str);
		if(!str.empty()){
			value = (float)_wtof(str.c_str());
		}else{
			value = default_value;
		}
	}else{
		value = default_value;
	}
}

void MQSetting::Load(const char *name, double& value, double default_value)
{
	MQXmlElement child = m_Elem->FirstChildElement(name);
	if(child != NULL){
		std::wstring str;
		child->GetText(str);
		if(!str.empty()){
			value = (float)_wtof(str.c_str());
		}else{
			value = default_value;
		}
	}else{
		value = default_value;
	}
}

void MQSetting::Load(const char *name, std::string& value, const std::string& default_value)
{
	MQXmlElement child = m_Elem->FirstChildElement(name);
	if(child != NULL){
		value = child->GetText();
	}else{
		value = default_value;
	}
}

void MQSetting::Load(const char *name, std::wstring& value, const std::wstring& default_value)
{
	MQXmlElement child = m_Elem->FirstChildElement(name);
	if(child != NULL){
		child->GetText(value);
	}else{
		value = default_value;
	}
}

void MQSetting::Save(const char *name, const bool& value)
{
	MQXmlElement child = m_Elem->FirstChildElement(name);
	if(child != NULL){
		m_Elem->RemoveChildElement(child);
	}

	child = m_Elem->AddChildElement(name);
	child->SetText(value ? L"1" : L"0");
}

void MQSetting::Save(const char *name, const int& value)
{
	MQXmlElement child = m_Elem->FirstChildElement(name);
	if(child != NULL){
		m_Elem->RemoveChildElement(child);
	}

	wchar_t buf[64];
	swprintf_s(buf, L"%d", value);

	child = m_Elem->AddChildElement(name);
	child->SetText(buf);
}

void MQSetting::Save(const char *name, const unsigned int& value)
{
	MQXmlElement child = m_Elem->FirstChildElement(name);
	if(child != NULL){
		m_Elem->RemoveChildElement(child);
	}

	wchar_t buf[64];
	swprintf_s(buf, L"%u", value);

	child = m_Elem->AddChildElement(name);
	child->SetText(buf);
}

void MQSetting::Save(const char *name, const float& value)
{
	MQXmlElement child = m_Elem->FirstChildElement(name);
	if(child != NULL){
		m_Elem->RemoveChildElement(child);
	}

	wchar_t buf[64];
	swprintf_s(buf, L"%e", value);

	child = m_Elem->AddChildElement(name);
	child->SetText(buf);
}

void MQSetting::Save(const char *name, const double& value)
{
	MQXmlElement child = m_Elem->FirstChildElement(name);
	if(child != NULL){
		m_Elem->RemoveChildElement(child);
	}

	wchar_t buf[64];
	swprintf_s(buf, L"%e", value);

	child = m_Elem->AddChildElement(name);
	child->SetText(buf);
}

void MQSetting::Save(const char *name, const std::string& value)
{
	MQXmlElement child = m_Elem->FirstChildElement(name);
	if(child != NULL){
		m_Elem->RemoveChildElement(child);
	}

	child = m_Elem->AddChildElement(name);
	child->SetText(value.c_str());
}

void MQSetting::Save(const char *name, const std::wstring& value)
{
	MQXmlElement child = m_Elem->FirstChildElement(name);
	if(child != NULL){
		m_Elem->RemoveChildElement(child);
	}

	child = m_Elem->AddChildElement(name);
	child->SetText(value.c_str());
}

