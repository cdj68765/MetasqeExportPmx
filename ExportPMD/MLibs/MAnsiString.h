#pragma once

#ifndef _MANSISTRING_H_
#define _MANSISTRING_H_

#include "MLibsDll.h"
#include <assert.h>
#include <string>
#include <vector>


// String for ANSI character
class MLIBS_API MAnsiString
{
public:
	enum {
		kInvalid = (size_t)-1,
	};

public:
	// Constructor
	MAnsiString();
	MAnsiString(const char *str);
	MAnsiString(const char *str, size_t length);
#ifndef MSTRING_DISABLE_STDSRING
	MAnsiString(const std::string& str);
#endif
	MAnsiString(const MAnsiString& str);
#if _MSC_VER >= 1600 || __BORLAND_C__ >= 0x0630
	MAnsiString(MAnsiString&& str);
#endif
	// Destroctor
	~MAnsiString();

	// Get a row pointer
	operator char*();
	operator const char*() const;
	char *c_str();
	const char *c_str() const;
	//char& operator[](size_t index);
	//const char& operator[](size_t index) const;

	// Get a pointer to the next character with care of multi-byte string characters
	char *next(char *ptr) const;
	const char *next(const char *ptr) const;
	size_t next(size_t pos) const;

	// Get a pointer to the previous character with care of multi-byte string characters
	char *prev(char *ptr) const;
	const char *prev(const char *ptr) const;
	size_t prev(size_t pos) const;

	// Get a length of the character by the header byte.
	static int get_utf8_lead_bytes(char ch);

	// Get a pointer to the next character with care of utf8 string characters
	char *next_utf8(char *ptr) const;
	const char *next_utf8(const char *ptr) const;
	size_t next_utf8(size_t pos) const;

	// Get a length without end-termination in char units
	size_t length() const;

	// Get a number of characters with care of multi-byte string characters
	size_t count() const;

	// Reset to an empty string
	void clear();

	// Add a string to the end
	void append(const char *str, size_t len);

	// Get a part of this string
	MAnsiString substring(size_t start) const;
	MAnsiString substring(size_t start, size_t len) const;

	// Find an index of the specified character/string
	size_t indexOf(const MAnsiString& str, size_t start = 0) const;
	size_t indexOf(const char *str, size_t start = 0) const;
	size_t indexOf(char character, size_t start = 0) const;
	size_t indexOf(const std::vector<char>& characters, size_t start = 0) const;

	// Find an index of the specified character/string
	size_t lastIndexOf(const MAnsiString& str, size_t start = kInvalid) const;
	size_t lastIndexOf(char character, size_t start = kInvalid) const;
	size_t lastIndexOf(const std::vector<char>& characters, size_t start = kInvalid) const;

	// Split to strings by separator
	//   ex. "abc\tde\t\tf", "\t" -> {"abc","de","","f"}
	std::vector<MAnsiString> split(const MAnsiString& separator) const;
	std::vector<MAnsiString> split(char separator) const;
	std::vector<MAnsiString> split(const std::vector<char>& separators) const;
	
	// Combine strings
	//   ex. {"abc","de","","f"}, "\t" -> "abc\tde\t\tf"
	static MAnsiString combine(const std::vector<MAnsiString>& strings, const MAnsiString& separator);
	static MAnsiString combine(const std::vector<MAnsiString>& strings, char separator);

	// Change all characters to lower case
	MAnsiString toLowerCase() const;
	// Change all characters to upper case
	MAnsiString toUpperCase() const;

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

	// Convert from integer value
	static MAnsiString fromInt(int value, int radix = 10);
	// Convert from unsigned integer value
	static MAnsiString fromUInt(unsigned int value, int radix = 10);
	// Convert from 64-bit integer value
	static MAnsiString fromInt64(__int64 value, int radix = 10);
	// Convert from unsigned 64-bit integer value
	static MAnsiString fromUInt64(unsigned __int64 value, int radix = 10);
	// Convert from float value
	static MAnsiString fromFloat(float value, int digit, int max_digit = 8);
	// Convert from double value
	static MAnsiString fromDouble(double value, int digit, int max_digit = 8);
	// Check if the string can be converted to an int value
	bool canParseInt() const;
	// Check if the string can be converted to a float value
	bool canParseFloat() const;
	// Check if the string can be converted to a double value
	bool canParseDouble() const;

	// Check if is numberic string
	bool isNumber() const;
	// Trim 0 for numeric string
	bool trimNumber();
	// Trim zero under a decimal point
	MAnsiString getTrimDecimalZero() const;
	// Trim prefix or suffix space/tab
	MAnsiString getTrimSpace() const;

	// Format variables by the printf format
	static MAnsiString format(const char *format, ...);
	// Format variables by the printf format
	int formatSet(const char *format, ...);

	// Compare strings
	//    0 is returned when two strings are same.
	int compare(const MAnsiString& str) const;
	// Compare strings without care of case
	//    0 is returned when two strings are same.
	int compareIgnoreCase(const MAnsiString& str) const;
	// Compare a substring with the string
	//    0 is returned when two strings are same.
	int compareSubstring(size_t start, const MAnsiString& str) const;
	// Compare a substring with the string
	//    0 is returned when two strings are same.
	int compareSubstringIgnoreCase(size_t start, const MAnsiString& str) const;

	MAnsiString& operator = (const MAnsiString& str);
	MAnsiString& operator = (const char *str);
#ifndef MSTRING_DISABLE_STDSRING
	MAnsiString& operator = (const std::string& str);
#endif
#if _MSC_VER >= 1600 || __BORLAND_C__ >= 0x0630
	MAnsiString& operator = (MAnsiString&& str);
#endif

	MAnsiString& operator += (const MAnsiString& str);
	MAnsiString& operator += (const char *str);
	MAnsiString& operator += (char character);
	MAnsiString operator + (const MAnsiString& str) const;
	MAnsiString operator + (const char *str) const;
	MAnsiString operator + (char character) const;

#if _MSC_VER >= 1600 || __BORLAND_C__ >= 0x0630
	friend MLIBS_API MAnsiString operator + (MAnsiString&& str1, const MAnsiString& str2);
	friend MLIBS_API MAnsiString operator + (MAnsiString&& str1, const char *str2);
#endif
	friend MLIBS_API MAnsiString operator + (const char *str1, const MAnsiString& str2);

	bool operator == (const MAnsiString& str) const;
	bool operator == (const char *str) const;
	friend MLIBS_API bool operator == (const char *str1, const MAnsiString& str2);

	bool operator != (const MAnsiString& str) const;
	bool operator != (const char *str) const;
	friend MLIBS_API bool operator != (const char *str1, const MAnsiString& str2);

#ifndef MSTRING_DISABLE_STDSRING
	bool operator == (const std::string& str) const;
	bool operator != (const std::string& str) const;
	friend MLIBS_API bool operator == (const std::string& str1, const MAnsiString& str2);
	friend MLIBS_API bool operator != (const std::string& str1, const MAnsiString& str2);
#endif

	bool operator < (const MAnsiString& str) const { return compare(str) < 0; }
	bool operator > (const MAnsiString& str) const { return compare(str) > 0; }

private:
	char *mStr;
	size_t mLength;
	size_t mCapacity;

	bool resize(size_t size);
	size_t capacity() const;
};



inline MAnsiString::operator char*()
{
	return mStr;
}

inline MAnsiString::operator const char*() const
{
	return mStr;
}

inline char *MAnsiString::c_str()
{
	return mStr;
}

inline const char *MAnsiString::c_str() const
{
	return mStr;
}
/*
inline char& MAnsiString::operator[](size_t index)
{
	assert(index <= mLength); 
	return mStr[index];
}

inline const char& MAnsiString::operator[](size_t index) const
{
	assert(index <= mLength); 
	return mStr[index];
}
*/
inline size_t MAnsiString::length() const
{
	return mLength;
}

inline size_t MAnsiString::capacity() const
{
	return mCapacity;
}


#endif _MANSISTRING_H_
