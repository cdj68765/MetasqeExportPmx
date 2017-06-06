//---------------------------------------------------------------------------
#ifndef LanguageH
#define LanguageH

#include <MString.h>
#include <map>

class MTMenuItem;
class MTMenuBar;
class MTWidget;
class MTPopup;
class MTOpenFileDialog;
class MTSaveFileDialog;


class MLanguage
{
private:
	std::map<MAnsiString, MString> Data;
	MString mLanguage;

public:
	MLanguage();
	~MLanguage();

	bool IsContains(void) { return !Data.empty(); }
	MString GetLanguage() const { return mLanguage; }
	bool Load(const MString& language, const MString& filename);
	const wchar_t *Search(const char *Key);
	MString SearchFormat(const char *Key, const std::vector<MString>& Arg);
};



#endif
