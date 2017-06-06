//---------------------------------------------------------------------------
//
//   MQPlugin.cpp      Copyright(C) 1999-2016, tetraface Inc.
//
//     This is an implementation code for a class based on Metasequoia 
//    SDK specification. It is necessary to add this project into the 
//    product and build it. You do not need to modify this file.
//
//    　Metasequoia SDKの仕様に基づくクラスの実装コード。特定のクラスを
//    用いる際には、このファイルをプロジェクトに加えてビルドを行う必要が
//    ある。プラグイン開発者がこのファイルの内容を変更する必要はない。
//
//---------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "MQPlugin.h"


#if MQPLUGIN_VERSION >= 0x0240

MQXmlElement MQCXmlElement::AddChildElement(const char *name)
{
	std::string uname = AnsiToUtf8(name);
	void *ptr[2];
	ptr[0] = (void*)uname.c_str();
	ptr[1] = NULL;
	MQXmlElem_Value(this, MQXMLELEM_ADD_CHILD_ELEMENT, (void*)ptr);
	return (MQCXmlElement *)ptr[1];
}

MQXmlElement MQCXmlElement::AddChildElement(const wchar_t *name)
{
	std::string uname = WideToUtf8(name);
	void *ptr[2];
	ptr[0] = (void*)uname.c_str();
	ptr[1] = NULL;
	MQXmlElem_Value(this, MQXMLELEM_ADD_CHILD_ELEMENT, (void*)ptr);
	return (MQCXmlElement *)ptr[1];
}

BOOL MQCXmlElement::RemoveChildElement(MQXmlElement child)
{
	BOOL result = FALSE;
	void *ptr[2];
	ptr[0] = (void*)child;
	ptr[1] = &result;
	MQXmlElem_Value(this, MQXMLELEM_REMOVE_CHILD_ELEMENT, (void*)ptr);
	return result;
}

MQXmlElement MQCXmlElement::FirstChildElement(void)
{
	void *ptr[2];
	ptr[0] = NULL;
	ptr[1] = NULL;
	MQXmlElem_Value(this, MQXMLELEM_FIRST_CHILD_ELEMENT, (void*)ptr);
	return (MQXmlElement)ptr[1];
}

MQXmlElement MQCXmlElement::FirstChildElement(const char *name)
{
	std::string uname = AnsiToUtf8(name);
	void *ptr[2];
	ptr[0] = (void*)uname.c_str();
	ptr[1] = NULL;
	MQXmlElem_Value(this, MQXMLELEM_FIRST_CHILD_ELEMENT, (void*)ptr);
	return (MQXmlElement)ptr[1];
}

MQXmlElement MQCXmlElement::FirstChildElement(const wchar_t *name)
{
	std::string uname = WideToUtf8(name);
	void *ptr[2];
	ptr[0] = (void*)uname.c_str();
	ptr[1] = NULL;
	MQXmlElem_Value(this, MQXMLELEM_FIRST_CHILD_ELEMENT, (void*)ptr);
	return (MQXmlElement)ptr[1];
}

MQXmlElement MQCXmlElement::NextChildElement(MQXmlElement child)
{
	void *ptr[3];
	ptr[0] = NULL;
	ptr[1] = child;
	ptr[2] = NULL;
	MQXmlElem_Value(this, MQXMLELEM_NEXT_CHILD_ELEMENT, (void*)ptr);
	return (MQXmlElement)ptr[2];
}

MQXmlElement MQCXmlElement::NextChildElement(const char *name, MQXmlElement child)
{
	std::string uname = AnsiToUtf8(name);
	void *ptr[3];
	ptr[0] = (void*)uname.c_str();
	ptr[1] = child;
	ptr[2] = NULL;
	MQXmlElem_Value(this, MQXMLELEM_NEXT_CHILD_ELEMENT, (void*)ptr);
	return (MQXmlElement)ptr[2];
}

MQXmlElement MQCXmlElement::NextChildElement(const wchar_t *name, MQXmlElement child)
{
	std::string uname = WideToUtf8(name);
	void *ptr[3];
	ptr[0] = (void*)uname.c_str();
	ptr[1] = child;
	ptr[2] = NULL;
	MQXmlElem_Value(this, MQXMLELEM_NEXT_CHILD_ELEMENT, (void*)ptr);
	return (MQXmlElement)ptr[2];
}

MQXmlElement MQCXmlElement::GetParentElement(void)
{
	void *ptr[1];
	ptr[0] = NULL;
	MQXmlElem_Value(this, MQXMLELEM_GET_PARENT_ELEMENT, (void*)ptr);
	return (MQXmlElement)ptr[0];
}

std::string MQCXmlElement::GetName(void)
{
	void *ptr[1];
	ptr[0] = NULL;
	MQXmlElem_Value(this, MQXMLELEM_GET_NAME, (void*)ptr);
	return Utf8ToAnsi((const char *)ptr[0]);
}

std::wstring MQCXmlElement::GetNameW(void)
{
	void *ptr[1];
	ptr[0] = NULL;
	MQXmlElem_Value(this, MQXMLELEM_GET_NAME, (void*)ptr);
	return Utf8ToWide((const char *)ptr[0]);
}

std::string MQCXmlElement::GetText(void)
{
	void *ptr[1];
	ptr[0] = NULL;
	MQXmlElem_Value(this, MQXMLELEM_GET_TEXT, (void*)ptr);
	if (ptr[0] == NULL)
		return std::string();
	return Utf8ToAnsi((const char *)ptr[0]);
}

std::wstring MQCXmlElement::GetTextW(void)
{
	void *ptr[1];
	ptr[0] = NULL;
	MQXmlElem_Value(this, MQXMLELEM_GET_TEXT, (void*)ptr);
	if (ptr[0] == NULL)
		return std::wstring();
	return Utf8ToWide((const char *)ptr[0]);
}

BOOL MQCXmlElement::GetText(std::string& result_value)
{
	void *ptr[1];
	ptr[0] = NULL;
	MQXmlElem_Value(this, MQXMLELEM_GET_TEXT, (void*)ptr);
	if (ptr[0] == NULL){
		result_value = std::string();
		return FALSE;
	}
	result_value = Utf8ToAnsi((const char *)ptr[0]);
	return TRUE;
}

BOOL MQCXmlElement::GetText(std::wstring& result_value)
{
	void *ptr[1];
	ptr[0] = NULL;
	MQXmlElem_Value(this, MQXMLELEM_GET_TEXT, (void*)ptr);
	if (ptr[0] == NULL){
		result_value = std::wstring();
		return FALSE;
	}		
	result_value = Utf8ToWide((const char *)ptr[0]);
	return TRUE;
}

std::string MQCXmlElement::GetAttribute(const char *name)
{
	std::string result_value;
	GetAttribute(name, result_value);
	return result_value;
}

std::wstring MQCXmlElement::GetAttribute(const wchar_t *name)
{
	std::wstring result_value;
	GetAttribute(name, result_value);
	return result_value;
}

BOOL MQCXmlElement::GetAttribute(const char *name, std::string& result_value)
{
	std::string uname = AnsiToUtf8(name);
	void *ptr[2];
	ptr[0] = (void*)uname.c_str();
	ptr[1] = NULL;
	MQXmlElem_Value(this, MQXMLELEM_GET_ATTRIBUTE, (void*)ptr);
	if (ptr[1] == NULL){
		result_value = std::string();
		return FALSE;
	}
	result_value = Utf8ToAnsi((const char *)ptr[1]);
	return TRUE;
}

BOOL MQCXmlElement::GetAttribute(const wchar_t *name, std::wstring& result_value)
{
	std::string uname = WideToUtf8(name);
	void *ptr[2];
	ptr[0] = (void*)uname.c_str();
	ptr[1] = NULL;
	MQXmlElem_Value(this, MQXMLELEM_GET_ATTRIBUTE, (void*)ptr);
	if (ptr[1] == NULL){
		result_value = std::wstring();
		return FALSE;
	}
	result_value = Utf8ToWide((const char *)ptr[1]);
	return TRUE;
}

void MQCXmlElement::SetText(const char *text)
{
	std::string utext = AnsiToUtf8(text);
	void *ptr[1];
	ptr[0] = (void*)utext.c_str();
	MQXmlElem_Value(this, MQXMLELEM_SET_TEXT, (void*)ptr);
}

void MQCXmlElement::SetText(const wchar_t *text)
{
	std::string utext = WideToUtf8(text);
	void *ptr[1];
	ptr[0] = (void*)utext.c_str();
	MQXmlElem_Value(this, MQXMLELEM_SET_TEXT, (void*)ptr);
}

void MQCXmlElement::SetAttribute(const char *name, const char *value)
{
	std::string uname = AnsiToUtf8(name);
	std::string uvalue = AnsiToUtf8(value);
	void *ptr[2];
	ptr[0] = (void*)uname.c_str();
	ptr[1] = (void*)uvalue.c_str();
	MQXmlElem_Value(this, MQXMLELEM_SET_ATTRIBUTE, (void*)ptr);
}

void MQCXmlElement::SetAttribute(const wchar_t *name, const wchar_t *value)
{
	std::string uname = WideToUtf8(name);
	std::string uvalue = WideToUtf8(value);
	void *ptr[2];
	ptr[0] = (void*)uname.c_str();
	ptr[1] = (void*)uvalue.c_str();
	MQXmlElem_Value(this, MQXMLELEM_SET_ATTRIBUTE, (void*)ptr);
}

std::string MQCXmlElement::Utf8ToAnsi(const char *ptr)
{
	int lenw = MultiByteToWideChar(CP_UTF8,0,(const char*)ptr,-1,NULL,0);
	wchar_t *strw = (wchar_t*)malloc(sizeof(wchar_t) * (lenw+1));
	MultiByteToWideChar(CP_UTF8,0,(const char*)ptr,-1,strw,lenw);

	int lenu = WideCharToMultiByte(CP_ACP,0,strw,-1,NULL,0,NULL,NULL);
	char *stru = (char*)malloc(lenu+1);
	WideCharToMultiByte(CP_ACP,0,strw,-1,stru,lenu,NULL,NULL);

	free(strw);

	std::string str(stru);
	free(stru);
	return str;
}

std::wstring MQCXmlElement::Utf8ToWide(const char *ptr)
{
	int lenw = MultiByteToWideChar(CP_UTF8,0,(const char*)ptr,-1,NULL,0);
	wchar_t *strw = (wchar_t*)malloc(sizeof(wchar_t) * (lenw+1));
	MultiByteToWideChar(CP_UTF8,0,(const char*)ptr,-1,strw,lenw);

	std::wstring str(strw);
	free(strw);
	return str;
}

std::string MQCXmlElement::AnsiToUtf8(const char *ptr)
{
	int lenw = MultiByteToWideChar(CP_ACP,0,(const char*)ptr,-1,NULL,0);
	wchar_t *strw = (wchar_t*)malloc(sizeof(wchar_t) * (lenw+1));
	MultiByteToWideChar(CP_ACP,0,(const char*)ptr,-1,strw,lenw);

	int lenu = WideCharToMultiByte(CP_UTF8,0,strw,-1,NULL,0,NULL,NULL);
	char *stru = (char*)malloc(lenu+1);
	WideCharToMultiByte(CP_UTF8,0,strw,-1,stru,lenu,NULL,NULL);

	free(strw);

	std::string str(stru);
	free(stru);
	return str;
}

std::wstring MQCXmlElement::AnsiToWide(const char *ptr)
{
	int lenw = MultiByteToWideChar(CP_ACP,0,(const char*)ptr,-1,NULL,0);
	wchar_t *strw = (wchar_t*)malloc(sizeof(wchar_t) * (lenw+1));
	MultiByteToWideChar(CP_ACP,0,(const char*)ptr,-1,strw,lenw);

	std::wstring str(strw);
	free(strw);
	return str;
}

std::string MQCXmlElement::WideToUtf8(const wchar_t *ptr)
{
	int lenu = WideCharToMultiByte(CP_UTF8,0,ptr,-1,NULL,0,NULL,NULL);
	char *stru = (char*)malloc(lenu+1);
	WideCharToMultiByte(CP_UTF8,0,ptr,-1,stru,lenu,NULL,NULL);

	std::string str(stru);
	free(stru);
	return str;
}

#endif //MQPLUGIN_VERSION >= 0x0240


#if MQPLUGIN_VERSION >= 0x0410

MQXmlDocument MQCXmlDocument::Create()
{
	void *ptr[1];
	ptr[0] = NULL;
	MQXmlDoc_Value(NULL, MQXMLDOC_CREATE, ptr);
	return (MQCXmlDocument *)ptr[0];
}

void MQCXmlDocument::DeleteThis()
{
	MQXmlDoc_Value(this, MQXMLDOC_DELETE, NULL);
}

MQXmlElement MQCXmlDocument::CreateRootElement(const char *name)
{
	std::string uname = MQCXmlElement::AnsiToUtf8(name);
	void *ptr[2];
	ptr[0] = (void*)uname.c_str();
	ptr[1] = NULL;
	MQXmlDoc_Value(this, MQXMLDOC_CREATE_ROOT_ELEMENT, (void*)ptr);
	return (MQXmlElement)ptr[1];
}

MQXmlElement MQCXmlDocument::CreateRootElement(const wchar_t *name)
{
	std::string uname = MQCXmlElement::WideToUtf8(name);
	void *ptr[2];
	ptr[0] = (void*)uname.c_str();
	ptr[1] = NULL;
	MQXmlDoc_Value(this, MQXMLDOC_CREATE_ROOT_ELEMENT, (void*)ptr);
	return (MQXmlElement)ptr[1];
}

MQXmlElement MQCXmlDocument::GetRootElement()
{
	void *ptr[1];
	ptr[0] = NULL;
	MQXmlDoc_Value(this, MQXMLDOC_GET_ROOT_ELEMENT, ptr);
	return (MQCXmlElement *)ptr[0];
}

BOOL MQCXmlDocument::LoadFile(const char *filename)
{
	std::wstring uname = MQCXmlElement::AnsiToWide(filename);
	BOOL result = FALSE;
	void *ptr[2];
	ptr[0] = (void*)uname.c_str();
	ptr[1] = &result;
	MQXmlDoc_Value(this, MQXMLDOC_LOAD, ptr);
	return result;
}

BOOL MQCXmlDocument::LoadFile(const wchar_t *filename)
{
	BOOL result = FALSE;
	void *ptr[2];
	ptr[0] = (void*)filename;
	ptr[1] = &result;
	MQXmlDoc_Value(this, MQXMLDOC_LOAD, ptr);
	return result;
}

BOOL MQCXmlDocument::SaveFile(const char *filename)
{
	std::wstring uname = MQCXmlElement::AnsiToWide(filename);
	BOOL result = FALSE;
	void *ptr[2];
	ptr[0] = (void*)uname.c_str();
	ptr[1] = &result;
	MQXmlDoc_Value(this, MQXMLDOC_SAVE, ptr);
	return result;
}

BOOL MQCXmlDocument::SaveFile(const wchar_t *filename)
{
	BOOL result = FALSE;
	void *ptr[2];
	ptr[0] = (void*)filename;
	ptr[1] = &result;
	MQXmlDoc_Value(this, MQXMLDOC_SAVE, ptr);
	return result;
}


#endif


