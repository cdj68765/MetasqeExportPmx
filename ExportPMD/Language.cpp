#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "MQPlugin.h"
#include "Language.h"
#include <MFileUtil.h>


static MString convertStringCode(const MString& src_text)
{
	MString text;
	for(size_t i=0; i<src_text.length(); i=src_text.next(i)){
		if(src_text[i] == L'\\'){
			if(i+1 < src_text.length()){
				switch(src_text[i+1]){
				case L'\\': text += L'\\'; break;
				case L'n': text += L'\n'; break;
				case L't': text += L'\t'; break;
				default: assert(0); break;
				}
				i = src_text.next(i);
			}
		}else{
			text += src_text[i];
		}
	}
	return text;
}

//---------------------------------------------------------------------------
//   class MLanguage
//---------------------------------------------------------------------------
MLanguage::MLanguage()
{
}

MLanguage::~MLanguage()
{
}

bool MLanguage::Load(const MString& language, const MString& filename)
{
	mLanguage = language;
	Data.clear();

	MQXmlDocument doc = MQCXmlDocument::Create();
	if(!doc->LoadFile(filename.c_str())){
		doc->DeleteThis();
		MString buf = MString::format(L"Failed to load '%s'.\n", filename.c_str());
		OutputDebugStringW(buf.c_str());
		return false;
	}

	MQXmlElement root = doc->GetRootElement();
	if(root != NULL){
		bool found = false;
		for(int iter=0; iter<2 && !found; iter++){
			MQXmlElement res_root = root->FirstChildElement("resource");
			while(res_root != NULL){
				MString lang_val = res_root->GetAttribute(L"language");
				int def_val = MAnsiString(res_root->GetAttribute("default")).toInt();
				if(language == MString(lang_val) || (iter==1 && def_val)){
					MQXmlElement elem = res_root->FirstChildElement("string");
					while(elem != NULL){
						MAnsiString key(elem->GetAttribute("id"));
						MString xmltext = elem->GetTextW();
						MString text = convertStringCode(xmltext);

						auto it = Data.find(key);
						if(it == Data.end()){
							Data[key] = text;
						}else{
							// 重複データが万一あれば消してしまう
							MAnsiString buf = MAnsiString::format("CommandKey %s exists.\n", key.c_str());
							OutputDebugStringA(buf.c_str());
							Data.erase(it);
						}

						elem = res_root->NextChildElement("string", elem);
					}
					doc->DeleteThis();
					return true;
				}

				res_root = root->NextChildElement("resource", res_root);
			}
		}
	}

	doc->DeleteThis();
	return false;
}

const wchar_t *MLanguage::Search(const char *Key)
{
	auto it = Data.find(MAnsiString(Key));
	if(it != Data.end()){
		return (*it).second;
	}
	OutputDebugStringW(MString::format(L"Not found %S.\n", Key).c_str());
	return L"";
}

MString MLanguage::SearchFormat(const char *Key, const std::vector<MString>& Arg)
{
	auto it = Data.find(MAnsiString(Key));
	if(it != Data.end()){
		LPVOID lpMsgBuf;
		MString str;
		va_list* array = new va_list[Arg.size()];
		for(size_t i=0; i<Arg.size(); i++){
			array[i] = (va_list)Arg[i].c_str();
		}

		if(::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_FROM_STRING,
			(*it).second, 0, 0, (LPTSTR)&lpMsgBuf, 0, array) != 0)
		{
			str = (TCHAR*)lpMsgBuf;
			LocalFree(lpMsgBuf);

			// Replace '\r\n' to '\n'
			size_t pos = 0;
			while(true){
				size_t ret = str.indexOf(L"\r\n", pos);
				if(ret == MString::kInvalid) break;

				str = str.substring(0, ret) + L"\n" + str.substring(ret+2);
				pos = ret+1;
			}
		}

		delete[] array;
		return str;
	}

	OutputDebugStringW(MString::format(L"Not found %S.\n", Key).c_str());
	return MString();
}



