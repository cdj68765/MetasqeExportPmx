#pragma once

#ifndef _MSTRING_H_
#define _MSTRING_H_

#include "MLibsDll.h"
#include <assert.h>
#include <string>
#include <vector>
#include "MAnsiString.h"


// String for wide character
class MLIBS_API MString
{
public:
	enum {
		kInvalid = (size_t)-1,
	};

public:
	// Constructor
	MString();
	MString(const wchar_t *str);
	MString(const wchar_t *str, size_t length);
#ifndef MSTRING_DISABLE_STDSRING
	MString(const std::wstring& str);
#endif
	MString(const MString& str);
#if _MSC_VER >= 1600 || __BORLAND_C__ >= 0x0630
	MString(MString&& str);
#endif
	// Destroctor
	~MString();

	// Get a row pointer
	operator wchar_t*();
	operator const wchar_t*() const;
	wchar_t *c_str();
	const wchar_t *c_str() const;
	//wchar_t& operator[](size_t index);
	//const wchar_t& operator[](size_t index) const;

	// Get a pointer to the next character with care of surrogate characters
	wchar_t *next(wchar_t *ptr) const;
	const wchar_t *next(const wchar_t *ptr) const;
	size_t next(size_t pos) const;

	// Get a pointer to the previous character with care of surrogate characters
	wchar_t *prev(wchar_t *ptr) const;
	const wchar_t *prev(const wchar_t *ptr) const;
	size_t prev(size_t pos) const;

	// Get a length without end-termination in wchar_t units
	size_t length() const;

	// Get a number of characters with care of surrogate characters
	size_t count() const;

	// Reset to an empty string
	void clear();

	// Add a string to the end
	void append(const wchar_t *str, size_t len);

	// Get a part of this string
	MString substring(size_t start) const;
	MString substring(size_t start, size_t len) const;

	// Find an index of the specified character/string (kInvalid is returned when not found.)
	size_t indexOf(const MString& str, size_t start = 0) const;
	size_t indexOf(const wchar_t *str, size_t start = 0) const;
	size_t indexOf(wchar_t character, size_t start = 0) const;
	size_t indexOf(const std::vector<wchar_t>& characters, size_t start = 0) const;

	// Find an index of the specified character/string (kInvalid is returned when not found.)
	size_t lastIndexOf(const MString& str, size_t start = kInvalid) const;
	size_t lastIndexOf(wchar_t character, size_t start = kInvalid) const;
	size_t lastIndexOf(const std::vector<wchar_t>& characters, size_t start = kInvalid) const;

	// Split to strings by separator
	//   ex. "abc\tde\t\tf", "\t" -> {"abc","de","","f"}
	std::vector<MString> split(const MString& separator) const;
	std::vector<MString> split(wchar_t separator) const;
	std::vector<MString> split(const std::vector<wchar_t>& separators) const;
	
	// Combine strings
	//   ex. {"abc","de","","f"}, "\t" -> "abc\tde\t\tf"
	static MString combine(const std::vector<MString>& strings, const MString& separator);
	static MString combine(const std::vector<MString>& strings, wchar_t separator);

	// Change all characters to lower case
	MString toLowerCase() const;
	// Change all characters to upper case
	MString toUpperCase() const;

	// Convert to ANSI string
	MAnsiString toAnsiString() const;
	// Convert to UTF-8 string
	MAnsiString toUtf8String() const;
	// Convert to integer value
	int toInt() const;
	int toIntWithRadix(int radix) const;
	// Convert to unsigned integer value
	unsigned int toUInt() const;
	unsigned int toUIntWithRadix(int radix) const;
	// Convert to 64-bit integer value
	__int64 toInt64() const;
	__int64 toInt64WithRadix(int radix) const;
	// Convert to 64-bit unsigned integer value
	unsigned __int64 toUInt64() const;
	unsigned __int64 toUInt64WithRadix(int radix) const;
	// Convert to float value
	float toFloat() const;
	// Convert to double floating value
	double toDouble() const;
	// Check if the string can be converted to an int value
	bool canParseInt() const;
	// Check if the string can be converted to a float value
	bool canParseFloat() const;
	// Check if the string can be converted to a double value
	bool canParseDouble() const;

	// Convert from ANSI string
	static MString fromCharacter(wchar_t character);
	// Convert from ANSI string
	static MString fromAnsiString(const char *str);
	// Convert from UTF-8 string
	static MString fromUtf8String(const char *str);
	// Convert from integer value
	static MString fromInt(int value, int radix = 10);
	// Convert from unsigned integer value
	static MString fromUInt(unsigned int value, int radix = 10);
	// Convert from 64-bit integer value
	static MString fromInt64(__int64 value, int radix = 10);
	// Convert from unsigned 64-bit integer value
	static MString fromUInt64(unsigned __int64 value, int radix = 10);
	// Convert from float value
	static MString fromFloat(float value, int digit, int max_digit = 8);
	// Convert from double value
	static MString fromDouble(double value, int digit, int max_digit = 8);

	// Check if is numberic string
	bool isNumber() const;
	// Trim 0 for numeric string
	bool trimNumber();
	// Trim zero under a decimal point
	MString getTrimDecimalZero() const;
	// Trim prefix or suffix space/tab
	MString getTrimSpace() const;

	// Format variables by the printf format
	static MString format(const wchar_t *format, ...);
	// Format variables by the printf format
	int formatSet(const wchar_t *format, ...);

	// Compare strings
	//    0 is returned when two strings are same.
	int compare(const MString& str) const;
	// Compare strings without care of case
	//    0 is returned when two strings are same.
	int compareIgnoreCase(const MString& str) const;
	// Compare a substring with the string
	//    0 is returned when two strings are same.
	int compareSubstring(size_t start, const MString& str) const;
	// Compare a substring with the string
	//    0 is returned when two strings are same.
	int compareSubstringIgnoreCase(size_t start, const MString& str) const;

	MString& operator = (const MString& str);
	MString& operator = (const wchar_t *str);
#ifndef MSTRING_DISABLE_STDSRING
	MString& operator = (const std::wstring& str);
#endif
#if _MSC_VER >= 1600 || __BORLAND_C__ >= 0x0630
	MString& operator = (MString&& str);
#endif

	MString& operator += (const MString& str);
	MString& operator += (const wchar_t *str);
	MString& operator += (wchar_t character);
	MString operator + (const MString& str) const;
	MString operator + (const wchar_t *str) const;
	MString operator + (wchar_t character) const;

#if _MSC_VER >= 1600 || __BORLAND_C__ >= 0x0630
	friend MLIBS_API MString operator + (MString&& str1, const MString& str2);
	friend MLIBS_API MString operator + (MString&& str1, const wchar_t *str2);
#endif
	friend MLIBS_API MString operator + (const wchar_t *str1, const MString& str2);

	bool operator == (const MString& str) const;
	bool operator == (const wchar_t *str) const;
	friend MLIBS_API bool operator == (const wchar_t *str1, const MString& str2);

	bool operator != (const MString& str) const;
	bool operator != (const wchar_t *str) const;
	friend MLIBS_API bool operator != (const wchar_t *str1, const MString& str2);

#ifndef MSTRING_DISABLE_STDSRING
	bool operator == (const std::wstring& str) const;
	bool operator != (const std::wstring& str) const;
	friend MLIBS_API bool operator == (const std::wstring& str1, const MString& str2);
	friend MLIBS_API bool operator != (const std::wstring& str1, const MString& str2);
#endif

	bool operator < (const MString& str) const { return compare(str) < 0; }
	bool operator > (const MString& str) const { return compare(str) > 0; }

private:
	wchar_t *mStr;
	size_t mLength;
	size_t mCapacity;

	bool resize(size_t size);
	size_t capacity() const;
};



inline MString::operator wchar_t*()
{
	return mStr;
}

inline MString::operator const wchar_t*() const
{
	return mStr;
}

inline wchar_t *MString::c_str()
{
	return mStr;
}

inline const wchar_t *MString::c_str() const
{
	return mStr;
}
/*
inline wchar_t& MString::operator[](size_t index)
{
	assert(index <= mLength); 
	return mStr[index];
}

inline const wchar_t& MString::operator[](size_t index) const
{
	assert(index <= mLength); 
	return mStr[index];
}
*/
inline size_t MString::length() const
{
	return mLength;
}

inline size_t MString::capacity() const
{
	return mCapacity;
}


#endif _MSTRING_H_
