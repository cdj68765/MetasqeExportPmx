#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include "MString.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "memory.h"
#include "wchar.h"
#include <algorithm>

static wchar_t* null_str = L"";

inline bool isHighSurrogate(wchar_t ch)
{
	return (ch >= 0xD800) && (ch <= 0xDBFF);
}

inline bool isLowSurrogate(wchar_t ch)
{
	return (ch >= 0xDC00) && (ch <= 0xDFFF);
}

inline bool isSurrogatePair(const wchar_t* str)
{
	return isHighSurrogate(str[0]) && isLowSurrogate(str[1]);
}

MString::MString()
{
	mStr = null_str;
	mLength = 0;
	mCapacity = 0;
}

MString::MString(const wchar_t* str)
{
	mStr = null_str;
	mLength = 0;
	mCapacity = 0;

	*this = str;
}

MString::MString(const wchar_t* str, size_t length)
{
	mStr = null_str;
	mLength = 0;
	mCapacity = 0;

	append(str, length);
}

#ifndef MSTRING_DISABLE_STDSRING
MString::MString(const std::wstring& str)
{
	mStr = null_str;
	mLength = 0;
	mCapacity = 0;

	append(str.c_str(), str.length());
}
#endif

MString::MString(const MString& str)
{
	mStr = null_str;
	mLength = 0;
	mCapacity = 0;

	*this = str;
}

#if _MSC_VER >= 1600 || __BORLAND_C__ >= 0x0630
MString::MString(MString&& str)
{
	mStr = str.mStr;
	mLength = str.mLength;
	mCapacity = str.mCapacity;

	str.mStr = null_str;
	str.mLength = 0;
	str.mCapacity = 0;
}
#endif

MString::~MString()
{
	if (mStr != null_str)
	{
		free(mStr);
	}
}

// Get a number of characters with care of surrogage characters
size_t MString::count() const
{
	size_t num = 0;
	for (wchar_t* ptr = mStr; ptr < mStr + mLength;)
	{
		if (ptr + 2 <= mStr + mLength && isSurrogatePair(ptr))
		{
			num += 1;
			ptr += 2;
		}
		else
		{
			num += 1;
			ptr += 1;
		}
	}
	return num;
}

// Get a pointer to the next character with care of surrogate
wchar_t* MString::next(wchar_t* ptr) const
{
	assert(ptr >= mStr && ptr <= mStr + mLength);
	if (ptr < mStr || ptr >= mStr + mLength)
	{
		return nullptr;
	}

	if (ptr + 2 <= mStr + mLength && isSurrogatePair(ptr))
	{
		return ptr + 2;
	}
	return ptr + 1;
}

const wchar_t* MString::next(const wchar_t* ptr) const
{
	assert(ptr >= mStr && ptr <= mStr + mLength);
	if (ptr < mStr || ptr >= mStr + mLength)
	{
		return nullptr;
	}

	if (ptr + 2 <= mStr + mLength && isSurrogatePair(ptr))
	{
		return ptr + 2;
	}
	return ptr + 1;
}

size_t MString::next(size_t pos) const
{
	const wchar_t* ptr = next(mStr + pos);
	return (ptr != nullptr) ? ptr - mStr : kInvalid;
}

// Get a pointer to the previous character with care of surrogate characters
wchar_t* MString::prev(wchar_t* ptr) const
{
	assert(ptr >= mStr && ptr <= mStr + mLength);
	if (ptr <= mStr || ptr > mStr + mLength)
	{
		return nullptr;
	}

	if (ptr >= mStr + 2 && isSurrogatePair(ptr - 2))
	{
		return ptr - 2;
	}
	return ptr - 1;
}

const wchar_t* MString::prev(const wchar_t* ptr) const
{
	assert(ptr >= mStr && ptr <= mStr + mLength);
	if (ptr <= mStr || ptr > mStr + mLength)
	{
		return nullptr;
	}

	if (ptr >= mStr + 2 && isSurrogatePair(ptr - 2))
	{
		return ptr - 2;
	}
	return ptr - 1;
}

size_t MString::prev(size_t pos) const
{
	const wchar_t* ptr = prev(mStr + pos);
	return (ptr != nullptr) ? ptr - mStr : kInvalid;
}

void MString::clear()
{
	if (mStr != null_str)
	{
		free(mStr);
		mStr = null_str;
	}
	mLength = 0;
	mCapacity = 0;
}

bool MString::resize(size_t size)
{
	if (size > 0 && size + 1 > mCapacity)
	{
		size_t new_cap = ((size + 1) + 3) & ~3;
		if (mStr == null_str)
		{
			void* buf = malloc(sizeof(wchar_t) * new_cap);
			if (buf == nullptr)
			{
				return false;
			}
			memset(buf, 0, sizeof(wchar_t) * new_cap);
			mStr = (wchar_t*)buf;
		}
		else
		{
			void* buf = realloc(mStr, sizeof(wchar_t) * new_cap);
			if (buf == nullptr)
			{
				return false;
			}
			memset((wchar_t*)buf + mCapacity, 0, sizeof(wchar_t) * (new_cap - mCapacity));
			mStr = (wchar_t*)buf;
		}
		mCapacity = new_cap;
	}
	else if (size == 0)
	{
		clear();
	}

	mLength = size;
	if (mStr != null_str)
	{
		mStr[mLength] = L'\0';
	}
	return true;
}

void MString::append(const wchar_t* str, size_t len)
{
	if (str == nullptr)
	{
		return;
	}
	size_t len2 = wcsnlen(str, len);
	if (len2 == 0)
	{
		return;
	}

	size_t len1 = length();
	resize(len1 + len2);
	memcpy_s(mStr + len1, sizeof(wchar_t) * (mCapacity - len1), str, sizeof(wchar_t) * len2);
	mStr[len1 + len2] = L'\0';
}

MString MString::substring(size_t start) const
{
	if (start >= mLength)
	{
		return MString();
	}
	return MString(mStr + start, mLength - start);
}

MString MString::substring(size_t start, size_t len) const
{
	if (start >= mLength)
	{
		return MString();
	}
	size_t copy_len = std::min(mLength - start, len);
	return MString(mStr + start, copy_len);
}

size_t MString::indexOf(const MString& str, size_t start) const
{
	assert(start >= 0);
	if (str.length() == 0) return kInvalid;

	for (size_t i = (size_t)start; i < mLength; i++)
	{
		if (i + str.length() > mLength) break;
		if (wcsncmp(&mStr[i], str.c_str(), str.length()) == 0)
		{
			return i;
		}
	}

	return kInvalid;
}

size_t MString::indexOf(const wchar_t* str, size_t start) const
{
	assert(start >= 0);
	if (str == nullptr) return kInvalid;

	size_t length = wcslen(str);
	if (length == 0) return kInvalid;

	for (size_t i = (size_t)start; i < mLength; i++)
	{
		if (i + length > mLength) break;
		if (wcsncmp(&mStr[i], str, length) == 0)
		{
			return i;
		}
	}

	return kInvalid;
}

size_t MString::indexOf(wchar_t character, size_t start) const
{
	assert(start >= 0);
	for (size_t i = (size_t)start; i < mLength; i++)
	{
		if (mStr[i] == character)
		{
			return i;
		}
	}
	return kInvalid;
}

size_t MString::indexOf(const std::vector<wchar_t>& characters, size_t start) const
{
	assert(start != kInvalid);
	for (size_t i = (size_t)start; i < mLength; i++)
	{
		if (characters.end() != find(characters.begin(), characters.end(), mStr[i]))
		{
			return i;
		}
	}
	return kInvalid;
}

size_t MString::lastIndexOf(const MString& str, size_t start) const
{
	if (str.length() == 0) return kInvalid;
	if (mLength < str.length()) return kInvalid;
	if (start == kInvalid || start > mLength - str.length())
	{
		start = mLength - str.length();
	}

	for (size_t i = start; i >= 0; i--)
	{
		if (wcsncmp(&mStr[i], str.c_str(), str.length()) == 0)
		{
			return i;
		}
		if (i == 0) break;
	}
	return kInvalid;
}

size_t MString::lastIndexOf(wchar_t character, size_t start) const
{
	if (mLength == 0) return kInvalid;
	if (start == kInvalid || start >= mLength) start = mLength - 1;

	for (size_t i = start; i >= 0; i--)
	{
		if (mStr[i] == character)
		{
			return i;
		}
		if (i == 0) break;
	}
	return kInvalid;
}

size_t MString::lastIndexOf(const std::vector<wchar_t>& characters, size_t start) const
{
	if (mLength == 0) return kInvalid;
	if (start == kInvalid || start >= mLength) start = mLength - 1;

	for (size_t i = start; i >= 0; i--)
	{
		if (characters.end() != find(characters.begin(), characters.end(), mStr[i]))
		{
			return i;
		}
		if (i == 0) break;
	}
	return kInvalid;
}

std::vector<MString> MString::split(const MString& separator) const
{
	std::vector<MString> ret;
	size_t pos = 0;
	while (true)
	{
		size_t next = indexOf(separator, pos);
		if (next == kInvalid)
		{
			ret.push_back(substring(pos));
			break;
		}
		ret.push_back(substring(pos, next - pos));
		pos = next + separator.length();
	}
	return ret;
}

std::vector<MString> MString::split(wchar_t separator) const
{
	std::vector<MString> ret;
	size_t pos = 0;
	while (true)
	{
		size_t next = indexOf(separator, pos);
		if (next == kInvalid)
		{
			ret.push_back(substring(pos));
			break;
		}
		ret.push_back(substring(pos, next - pos));
		pos = next + 1;
	}
	return ret;
}

std::vector<MString> MString::split(const std::vector<wchar_t>& separators) const
{
	std::vector<MString> ret;
	size_t pos = 0;
	while (true)
	{
		size_t next = indexOf(separators, pos);
		if (next == kInvalid)
		{
			ret.push_back(substring(pos));
			break;
		}
		ret.push_back(substring(pos, next - pos));
		pos = next + 1;
	}
	return ret;
}

MString MString::combine(const std::vector<MString>& strings, const MString& separator)
{
	MString ret;
	for (size_t i = 0; i < strings.size(); i++)
	{
		if (i > 0) ret += separator;
		ret += strings[i];
	}
	return ret;
}

MString MString::combine(const std::vector<MString>& strings, wchar_t separator)
{
	MString ret;
	for (size_t i = 0; i < strings.size(); i++)
	{
		if (i > 0) ret += separator;
		ret += strings[i];
	}
	return ret;
}

MString MString::toLowerCase() const
{
	size_t len = length();
	MString ret;
	ret.resize(len);
	for (size_t i = 0; i < len; i++)
	{
		ret[i] = towlower(mStr[i]);
	}

	return ret;
}

MString MString::toUpperCase() const
{
	size_t len = length();
	MString ret;
	ret.resize(len);
	for (size_t i = 0; i < len; i++)
	{
		ret[i] = towupper(mStr[i]);
	}

	return ret;
}

MAnsiString MString::toAnsiString() const
{
	if (length() == 0) return std::string();

	int lenu = WideCharToMultiByte(CP_ACP, 0, mStr, -1, nullptr, 0, nullptr, nullptr);
	char* stru = (char*)malloc(lenu + 1);
	WideCharToMultiByte(CP_ACP, 0, mStr, -1, stru, lenu, nullptr, nullptr);

	MAnsiString str(stru);
	free(stru);
	return str;
}

MAnsiString MString::toUtf8String() const
{
	if (length() == 0) return std::string();

	int lenu = WideCharToMultiByte(CP_UTF8, 0, mStr, -1, nullptr, 0, nullptr, nullptr);
	char* stru = (char*)malloc(lenu + 1);
	WideCharToMultiByte(CP_UTF8, 0, mStr, -1, stru, lenu, nullptr, nullptr);

	MAnsiString str(stru);
	free(stru);
	return str;
}

template <typename T>
T parseRadix16(const wchar_t* ptr, const wchar_t* end_ptr)
{
	T val = 0;
	for (; ptr < end_ptr; ptr++)
	{
		if (*ptr >= L'0' && *ptr <= '9')
		{
			val = (val << 4) | (*ptr - L'0');
		}
		else if (*ptr >= L'A' && *ptr <= 'F')
		{
			val = (val << 4) | (*ptr - L'A' + 10);
		}
		else if (*ptr >= L'a' && *ptr <= 'f')
		{
			val = (val << 4) | (*ptr - L'a' + 10);
		}
		else
		{
			break;
		}
	}
	return val;
}

int MString::toInt() const
{
	// 0x** is treated as 16 radix number
	if (mLength >= 3 && mStr[0] == L'0' && (mStr[1] == L'x' || mStr[1] == L'X'))
	{
		return parseRadix16<int>(mStr + 2, mStr + mLength);
	}

	return _wtoi(mStr);
}

int MString::toIntWithRadix(int base) const
{
	return wcstol(mStr, nullptr, base);
}

unsigned int MString::toUInt() const
{
	// 0x** is treated as 16 radix number
	if (mLength >= 3 && mStr[0] == L'0' && (mStr[1] == L'x' || mStr[1] == L'X'))
	{
		return parseRadix16<unsigned int>(mStr + 2, mStr + mLength);
	}

	__int64 val = std::max((__int64)0, std::min(_wtoi64(mStr), (__int64)UINT_MAX));
	return (unsigned int)val;
}

unsigned int MString::toUIntWithRadix(int base) const
{
	return wcstoul(mStr, nullptr, base);
}

__int64 MString::toInt64() const
{
	// 0x** is treated as 16 radix number
	if (mLength >= 3 && mStr[0] == L'0' && (mStr[1] == L'x' || mStr[1] == L'X'))
	{
		return parseRadix16<__int64>(mStr + 2, mStr + mLength);
	}

	return _wtoi64(mStr);
}

__int64 MString::toInt64WithRadix(int base) const
{
	return _wcstoi64(mStr, nullptr, base);
}

unsigned __int64 MString::toUInt64() const
{
	// 0x** is treated as 16 radix number
	if (mLength >= 3 && mStr[0] == L'0' && (mStr[1] == L'x' || mStr[1] == L'X'))
	{
		return parseRadix16<unsigned __int64>(mStr + 2, mStr + mLength);
	}

	return _wtoi64(mStr);
}

unsigned __int64 MString::toUInt64WithRadix(int base) const
{
	return _wcstoui64(mStr, nullptr, base);
}

float MString::toFloat(void) const
{
	double val = wcstod(mStr, nullptr);
	return (float)val;
}

double MString::toDouble(void) const
{
	double val = wcstod(mStr, nullptr);
	return val;
}

bool MString::canParseInt() const
{
	wchar_t* endptr;
	wcstol(mStr, &endptr, 10);
	return (endptr == nullptr) || (endptr == mStr + mLength);
}

bool MString::canParseFloat() const
{
	wchar_t* endptr;
	wcstod(mStr, &endptr);
	return (endptr == nullptr) || (endptr == mStr + mLength);
}

bool MString::canParseDouble() const
{
	wchar_t* endptr;
	wcstod(mStr, &endptr);
	return (endptr == nullptr) || (endptr == mStr + mLength);
}

MString MString::fromCharacter(wchar_t character)
{
	wchar_t str[2];
	str[0] = character;
	str[1] = L'\0';

	return MString(str);
}

MString MString::fromAnsiString(const char* str)
{
	if (str == nullptr) return MString();

	int lenw = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
	wchar_t* strw = (wchar_t*)malloc(sizeof(wchar_t) * (lenw + 1));
	MultiByteToWideChar(CP_ACP, 0, str, -1, strw, lenw);

	MString ret(strw);
	free(strw);
	return ret;
}

MString MString::fromUtf8String(const char* str)
{
	if (str == nullptr) return MString();

	int lenw = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
	wchar_t* strw = (wchar_t*)malloc(sizeof(wchar_t) * (lenw + 1));
	MultiByteToWideChar(CP_UTF8, 0, str, -1, strw, lenw);

	MString ret(strw);
	free(strw);
	return ret;
}

MString MString::fromInt(int value, int radix)
{
	wchar_t buf[30];
#if _MSC_VER >= 1400 || __BORLAND_C__ >= 0x0630
	errno_t ret = _itow_s(value, buf, _countof(buf), radix);

	if (ret != 0)
	{
		return MString();
	}
	return MString(buf);
#else
	_itow(value, buf, radix);
	return MString(buf);
#endif
}

MString MString::fromUInt(unsigned int value, int radix)
{
	wchar_t buf[30];
#if _MSC_VER >= 1400 || __BORLAND_C__ >= 0x0630
	errno_t ret = _ultow_s(value, buf, _countof(buf), radix);
	if (ret != 0)
	{
		return MString();
	}
#else
	_ultow(value, buf, radix);
#endif
	return MString(buf);
}

MString MString::fromInt64(__int64 value, int radix)
{
	wchar_t buf[30];
#if _MSC_VER >= 1400 || __BORLAND_C__ >= 0x0630
	errno_t ret = _i64tow_s(value, buf, _countof(buf), radix);
	if (ret != 0)
	{
		return MString();
	}
#else
	_i64tow(value, buf, radix);
#endif
	return MString(buf);
}

MString MString::fromUInt64(unsigned __int64 value, int radix)
{
	wchar_t buf[30];
#if _MSC_VER >= 1400 || __BORLAND_C__ >= 0x0630
	errno_t ret = _ui64tow_s(value, buf, _countof(buf), radix);
	if (ret != 0)
	{
		return MString();
	}
#else
	_ui64tow(value, buf, radix);
#endif
	return MString(buf);
}

// Convert from float value
MString MString::fromFloat(float value, int digit, int max_digit)
{
	float absval = fabs(value);
	int d = (absval > 0) ? (int)floor(log10(absval)) : 0;
	MString format = MString::format(L"%%0.%df", std::min(std::max(0, digit - d - 1), max_digit));
	return MString::format(format.c_str(), value);
}

// Convert from double value
MString MString::fromDouble(double value, int digit, int max_digit)
{
	double absval = fabs(value);
	int d = (absval > 0) ? (int)floor(log10(absval)) : 0;
	MString format = MString::format(L"%%0.%dlf", std::min(std::max(0, digit - d - 1), max_digit));
	return MString::format(format.c_str(), value);
}

// Check if is numberic string
bool MString::isNumber() const
{
	bool sign = false;
	bool number = false;
	bool period = false;
	bool exponent = false;
	bool hex = false;
	for (const wchar_t* ptr = mStr; ptr < mStr + mLength; ptr = next(ptr))
	{
		switch (*ptr)
		{
		case '-':
		case '+':
			if (number || period || sign || hex)
			{
				return false;
			}
			sign = true;
			break;
		case '.':
			if (period || exponent || hex)
			{
				return false;
			}
			period = true;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			number = true;
			break;
		case 'e':
		case 'E':
			if (!hex)
			{
				if (!number || exponent)
				{
					return false;
				}
				exponent = true;
				number = false;
				period = false;
				sign = false;
			}
			break;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'f':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'F':
			if (!hex) return false;
			number = true;
			break;
		case 'x':
		case 'X':
			if (hex) return false;
			if (ptr == mStr) return false;
			if (*(ptr - 1) != '0') return false;
			if (ptr - 2 == mStr && (*(ptr - 2) != L'+' && *(ptr - 2) != L'-')) return false;
			if (ptr - 3 >= mStr) return false;
			hex = true;
			break;
		default:
			return false;
		}
	}

	return number;
}

// Trim 0 for numeric string
bool MString::trimNumber()
{
	if (!isNumber()) return false;

	bool trimmed = false;

	// Remove end 0
	size_t period = lastIndexOf('.');
	if (period != kInvalid)
	{
		int zero_count = 0;
		size_t pos = mLength - 1;
		for (; pos > period; pos--)
		{
			if (mStr[pos] == 'E' || mStr[pos] == 'e')
			{
				zero_count = 0;
				break;
			}
			if (mStr[pos] >= '1' && mStr[pos] <= '9')
			{
				break;
			}
			if (mStr[pos] != '0')
			{
				zero_count = 0;
				break;
			}
			zero_count++;
		}
		if (pos == period)
		{
			resize(pos);
			trimmed = true;
		}
		else if (zero_count > 0)
		{
			resize(mLength - zero_count);
			trimmed = true;
		}
	}

	// Remove header '+'
	if (mLength > 0 && mStr[0] == L'+')
	{
		*this = substring(1);
		trimmed = true;
	}

	return trimmed;
}

MString MString::getTrimDecimalZero() const
{
	size_t pos = mLength;
	bool other = false;
	for (size_t i = mLength; ;)
	{
		i = prev(i);
		if (i == kInvalid) break;

		if (mStr[i] == L'.')
		{
			if (other)
			{
				return substring(0, pos);
			}
			return substring(0, i);
		}
		if (mStr[i] == L'0')
		{
			if (!other)
			{
				pos = i;
			}
		}
		else if (mStr[i] >= L'1' && mStr[i] <= L'9')
		{
			other = true;
		}
		else
		{
			break;
		}
	}

	return *this;
}

MString MString::getTrimSpace() const
{
	if (mLength == 0) return MString();

	size_t pre_pos = mLength;
	for (size_t i = 0; i < mLength && i != kInvalid; i = next(i))
	{
		if (!(mStr[i] == L' ' || mStr[i] == L'\t'))
		{
			pre_pos = i;
			break;
		}
	}
	size_t pst_pos = pre_pos;
	for (size_t n = mLength; n > pre_pos && n != kInvalid;)
	{
		size_t i = prev(n);
		if (i != kInvalid && !(mStr[i] == L' ' || mStr[i] == L'\t'))
		{
			pst_pos = n;
			break;
		}
		n = i;
	}

	return substring(pre_pos, pst_pos - pre_pos);
}

MString MString::format(const wchar_t* format, ...)
{
	va_list argp;
	va_start(argp, format);

#if _MSC_VER >= 1400
	int num = _vscwprintf(format, argp);
	if (num <= 0)
	{
		va_end(argp);
		return MString();
	}

	MString ret;
	ret.resize(num);
	num = vswprintf_s(ret.c_str(), ret.capacity(), format, argp);
	if (num < 0)
	{
		va_end(argp);
		return MString();
	}
	va_end(argp);
	return ret;
#else
	wchar_t *temp = NULL;
	int num = 0;
	for (size_t temp_len = 1024; temp_len < 1024 * 1024; temp_len *= 4) {
		temp = (wchar_t*)malloc(sizeof(wchar_t)*temp_len);
		if (temp == NULL) {
			va_end(argp);
			return MString();
		}

		num = _vsnwprintf(temp, temp_len, format, argp);
		if (num >= 0) {
			break;
		}

		free(temp);
		temp = NULL;
	}
	if (num <= 0) {
		free(temp);
		va_end(argp);
		return MString();
	}

	MString ret;
	ret.resize(num);
	memcpy_s(ret.c_str(), sizeof(wchar_t)*ret.mCapacity, temp, sizeof(wchar_t)*(num + 1));
	free(temp);
	va_end(argp);
	return ret;
#endif
}

int MString::formatSet(const wchar_t* format, ...)
{
	va_list argp;
	va_start(argp, format);

#if _MSC_VER >= 1400
	int num = _vscwprintf(format, argp);
	if (num <= 0)
	{
		va_end(argp);
		return num;
	}

	resize(num);
	num = vswprintf_s(c_str(), capacity(), format, argp);
	va_end(argp);
	return num;
#else
	wchar_t *temp = NULL;
	int num = 0;
	for (size_t temp_len = 1024; temp_len < 1024 * 1024; temp_len *= 4) {
		temp = (wchar_t*)malloc(sizeof(wchar_t)*temp_len);
		if (temp == NULL) {
			va_end(argp);
			return -1;
		}

		num = _vsnwprintf(temp, temp_len, format, argp);
		if (num >= 0) {
			break;
		}

		free(temp);
		temp = NULL;
	}
	if (num <= 0) {
		free(temp);
		va_end(argp);
		return num;
	}

	resize(num);
	memcpy_s(c_str(), sizeof(wchar_t)*mCapacity, temp, sizeof(wchar_t)*(num + 1));
	free(temp);
	va_end(argp);
	return num;
#endif
}

int MString::compare(const MString& str) const
{
	return wcsncmp(mStr, str.c_str(), std::max(mLength, str.length()));
}

int MString::compareIgnoreCase(const MString& str) const
{
	return _wcsnicmp(mStr, str.c_str(), std::max(mLength, str.length()));
}

int MString::compareSubstring(size_t start, const MString& str) const
{
	return wcsncmp(mStr + start, str.c_str(), str.length());
}

int MString::compareSubstringIgnoreCase(size_t start, const MString& str) const
{
	return _wcsnicmp(mStr + start, str.c_str(), str.length());
}

MString& MString::operator =(const MString& str)
{
	if (mStr == str.c_str())
	{
		return *this;
	}

	size_t len = str.length();
	if (len == 0)
	{
		clear();
	}
	else
	{
		resize(len);
		memcpy_s(mStr, sizeof(wchar_t) * mCapacity, str.c_str(), sizeof(wchar_t) * len);
		mStr[len] = L'\0';
	}
	return *this;
}

MString& MString::operator =(const wchar_t* str)
{
	if (mStr == str)
	{
		return *this;
	}

	size_t len = (str != nullptr) ? wcslen(str) : 0;
	if (len == 0)
	{
		clear();
	}
	else
	{
		resize(len);
		memcpy_s(mStr, sizeof(wchar_t) * mCapacity, str, sizeof(wchar_t) * len);
		mStr[len] = L'\0';
	}
	return *this;
}

#ifndef MSTRING_DISABLE_STDSRING
MString& MString::operator =(const std::wstring& str)
{
	if (mStr == str.c_str())
	{
		return *this;
	}

	size_t len = str.length();
	if (len == 0)
	{
		clear();
	}
	else
	{
		resize(len);
		memcpy_s(mStr, sizeof(wchar_t) * mCapacity, str.c_str(), sizeof(wchar_t) * len);
		mStr[len] = L'\0';
	}
	return *this;
}
#endif

#if _MSC_VER >= 1600 || __BORLAND_C__ >= 0x0630
MString& MString::operator =(MString&& str)
{
	if (mStr == str.c_str())
	{
		return *this;
	}

	if (mStr != null_str)
	{
		free(mStr);
	}

	mStr = str.mStr;
	mLength = str.mLength;
	mCapacity = str.mCapacity;

	str.mStr = null_str;
	str.mLength = 0;
	str.mCapacity = 0;

	return *this;
}
#endif

MString& MString::operator +=(const MString& str)
{
	size_t len2 = str.length();
	if (len2 == 0)
	{
		return *this;
	}

	size_t len1 = length();
	resize(len1 + len2);
	memcpy_s(mStr + len1, sizeof(wchar_t) * (mCapacity - len1), str.c_str(), sizeof(wchar_t) * len2);
	mStr[len1 + len2] = L'\0';
	return *this;
}

MString& MString::operator +=(const wchar_t* str)
{
	size_t len2 = (str != nullptr) ? wcslen(str) : 0;
	if (len2 == 0)
	{
		return *this;
	}

	size_t len1 = length();
	resize(len1 + len2);
	memcpy_s(mStr + len1, sizeof(wchar_t) * (mCapacity - len1), str, sizeof(wchar_t) * len2);
	mStr[len1 + len2] = L'\0';
	return *this;
}

MString& MString::operator +=(wchar_t character)
{
	size_t len1 = length();
	resize(len1 + 1);
	mStr[len1] = character;
	mStr[len1 + 1] = L'\0';
	return *this;
}

MString MString::operator +(const MString& str) const
{
	MString ret(*this);
	ret += str;
	return ret;
}

MString MString::operator +(const wchar_t* str) const
{
	MString ret(*this);
	ret += str;
	return ret;
}

MString MString::operator +(wchar_t character) const
{
	MString ret(*this);
	ret += character;
	return ret;
}

#if _MSC_VER >= 1600 || __BORLAND_C__ >= 0x0630
MString operator +(MString&& str1, const MString& str2)
{
	str1 += str2;
	return std::move(str1);
}

MString operator +(MString&& str1, const wchar_t* str2)
{
	str1 += str2;
	return std::move(str1);
}
#endif

MString operator +(const wchar_t* str1, const MString& str2)
{
	MString ret(str1);
	ret += str2;
	return ret;
}

bool MString::operator ==(const MString& str) const
{
	if (length() != str.length()) return false;

	return (wmemcmp(mStr, str.c_str(), length()) == 0);
}

bool MString::operator ==(const wchar_t* str) const
{
	size_t len = (str != nullptr) ? wcslen(str) : 0;
	if (length() != len) return false;

	return (wmemcmp(mStr, str, length()) == 0);
}

#ifndef MSTRING_DISABLE_STDSRING
bool MString::operator ==(const std::wstring& str) const
{
	size_t len = str.length();
	if (length() != len) return false;

	return (wmemcmp(mStr, str.c_str(), length()) == 0);
}
#endif

bool operator ==(const wchar_t* str1, const MString& str2)
{
	size_t len = (str1 != nullptr) ? wcslen(str1) : 0;
	if (len != str2.length()) return false;

	return (wmemcmp(str1, str2.c_str(), len) == 0);
}

#ifndef MSTRING_DISABLE_STDSRING
bool operator ==(const std::wstring& str1, const MString& str2)
{
	size_t len = str1.length();
	if (len != str2.length()) return false;

	return (wmemcmp(str1.c_str(), str2.c_str(), len) == 0);
}
#endif

bool MString::operator !=(const MString& str) const
{
	if (length() != str.length()) return true;

	return (wmemcmp(mStr, str.c_str(), length()) != 0);
}

bool MString::operator !=(const wchar_t* str) const
{
	size_t len = (str != nullptr) ? wcslen(str) : 0;
	if (length() != len) return true;

	return (wmemcmp(mStr, str, length()) != 0);
}

#ifndef MSTRING_DISABLE_STDSRING
bool MString::operator !=(const std::wstring& str) const
{
	size_t len = str.length();
	if (length() != len) return false;

	return (wmemcmp(mStr, str.c_str(), length()) != 0);
}
#endif

bool operator !=(const wchar_t* str1, const MString& str2)
{
	size_t len = (str1 != nullptr) ? wcslen(str1) : 0;
	if (len != str2.length()) return false;

	return (wmemcmp(str1, str2.c_str(), len) != 0);
}

#ifndef MSTRING_DISABLE_STDSRING
bool operator !=(const std::wstring& str1, const MString& str2)
{
	size_t len = str1.length();
	if (len != str2.length()) return false;

	return (wmemcmp(str1.c_str(), str2.c_str(), len) != 0);
}
#endif
