#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include "MAnsiString.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "memory.h"
#include <algorithm>

static char *null_str = "";


MAnsiString::MAnsiString()
{
	mStr = null_str;
	mLength = 0;
	mCapacity = 0;
}

MAnsiString::MAnsiString(const char *str)
{
	mStr = null_str;
	mLength = 0;
	mCapacity = 0;

	*this = str;
}

MAnsiString::MAnsiString(const char *str, size_t length)
{
	mStr = null_str;
	mLength = 0;
	mCapacity = 0;

	append(str, length);
}

#ifndef MSTRING_DISABLE_STDSRING
MAnsiString::MAnsiString(const std::string& str)
{
	mStr = null_str;
	mLength = 0;
	mCapacity = 0;

	append(str.c_str(), str.length());
}
#endif

MAnsiString::MAnsiString(const MAnsiString& str)
{
	mStr = null_str;
	mLength = 0;
	mCapacity = 0;

	*this = str;
}

#if _MSC_VER >= 1600 || __BORLAND_C__ >= 0x0630
MAnsiString::MAnsiString(MAnsiString&& str)
{
	mStr = str.mStr;
	mLength = str.mLength;
	mCapacity = str.mCapacity;

	str.mStr = null_str;
	str.mLength = 0;
	str.mCapacity = 0;
}
#endif

MAnsiString::~MAnsiString()
{
	if(mStr != null_str){
		free(mStr);
	}
}

// Get a number of characters with care of multi-byte string characters
size_t MAnsiString::count() const
{
	size_t num = 0;
	for(char *ptr = mStr; ptr < mStr + mLength; ){
		ptr = next(ptr);
		num++;
	}
	return num;
}

// Get a pointer to the next character with care of multi-byte string characters
char *MAnsiString::next(char *ptr) const
{
	assert(ptr >= mStr && ptr <= mStr + mLength);
	if(ptr < mStr || ptr >= mStr + mLength){
		return NULL;
	}

	return ::CharNextA(ptr);
}

const char *MAnsiString::next(const char *ptr) const
{
	assert(ptr >= mStr && ptr <= mStr + mLength);
	if(ptr < mStr || ptr >= mStr + mLength){
		return NULL;
	}

	return ::CharNextA(ptr);
}

size_t MAnsiString::next(size_t pos) const
{
	const char *ptr = next(mStr + pos);
	return (ptr != NULL) ? ptr - mStr : kInvalid;
}

// Get a pointer to the previous character with care of multi-byte string characters
char *MAnsiString::prev(char *ptr) const
{
	assert(ptr >= mStr && ptr <= mStr + mLength);
	if(ptr <= mStr || ptr > mStr + mLength){
		return NULL;
	}

	return ::CharPrevA(mStr, ptr);
}

const char *MAnsiString::prev(const char *ptr) const
{
	assert(ptr >= mStr && ptr <= mStr + mLength);
	if(ptr <= mStr || ptr > mStr + mLength){
		return NULL;
	}

	return ::CharPrevA(mStr, ptr);
}

size_t MAnsiString::prev(size_t pos) const
{
	const char *ptr = prev(mStr + pos);
	return (ptr != NULL) ? ptr - mStr : kInvalid;
}

int MAnsiString::get_utf8_lead_bytes(char ch)
{
	if(ch == '\0') return 0;
 
	if((ch & 0x80) == 0) return 1;

	int pos = 0;
	for( char tmp = ch & 0xfc; (tmp & 0x80); tmp = tmp << 1 )
	{
		++pos;
	}
	return pos;
}

char *MAnsiString::next_utf8(char *ptr) const
{
	assert(ptr >= mStr && ptr <= mStr + mLength);
	if(ptr < mStr || ptr >= mStr + mLength){
		return NULL;
	}

	int len = get_utf8_lead_bytes(*ptr);
	if(ptr > mStr + mLength){
		return NULL;
	}
	return ptr + len;
}

const char *MAnsiString::next_utf8(const char *ptr) const
{
	assert(ptr >= mStr && ptr <= mStr + mLength);
	if(ptr < mStr || ptr >= mStr + mLength){
		return NULL;
	}

	int len = get_utf8_lead_bytes(*ptr);
	if(ptr > mStr + mLength){
		return NULL;
	}
	return ptr + len;
}

size_t MAnsiString::next_utf8(size_t pos) const
{
	const char *ptr = next_utf8(mStr + pos);
	return (ptr != NULL) ? ptr - mStr : kInvalid;
}

void MAnsiString::clear()
{
	if(mStr != null_str){
		free(mStr);
		mStr = null_str;
	}
	mLength = 0;
	mCapacity = 0;
}

bool MAnsiString::resize(size_t size)
{
	if(size > 0 && size+1 > mCapacity){
		size_t new_cap = ((size+1) + 3) & ~3;
		if(mStr == null_str){
			void *buf = malloc(sizeof(char) * new_cap);
			if(buf == NULL){
				return false;
			}
			memset(buf, 0, sizeof(char) * new_cap);
			mStr = (char*)buf;
		}else{
			void *buf = realloc(mStr, sizeof(char) * new_cap);
			if(buf == NULL){
				return false;
			}
			memset((char*)buf + mCapacity, 0, sizeof(char)*(new_cap - mCapacity));
			mStr = (char*)buf;
		}
		mCapacity = new_cap;
	}else if(size == 0){
		clear();
	}

	mLength = size;
	if(mStr != null_str){
		mStr[mLength] = '\0';
	}
	return true;
}

void MAnsiString::append(const char *str, size_t len)
{
	if(str == nullptr){
		return;
	}
	size_t len2 = strnlen(str, len);
	if(len2 == 0){
		return;
	}

	size_t len1 = length();
	resize(len1 + len2);
	memcpy_s(mStr + len1, sizeof(char)*(mCapacity-len1), str, sizeof(char)*len2);
	mStr[len1 + len2] = '\0';
}

MAnsiString MAnsiString::substring(size_t start) const
{
	if(start >= mLength){
		return MAnsiString();
	}
	return MAnsiString(mStr + start, mLength - start);
}

MAnsiString MAnsiString::substring(size_t start, size_t len) const
{
	if(start >= mLength){
		return MAnsiString();
	}
	size_t copy_len = std::min(mLength - start, len);
	return MAnsiString(mStr + start, copy_len);
}

size_t MAnsiString::indexOf(const MAnsiString& str, size_t start) const
{
	assert(start != kInvalid);
	if(str.length() == 0) return kInvalid;

	for(const char *ptr = mStr + start; ptr < mStr + mLength; ptr = next(ptr)){
		if(ptr + str.length() > mStr + mLength) break;
		if(strncmp(ptr, str.c_str(), str.length()) == 0){
			return ptr - mStr;
		}
	}

	return kInvalid;
}

size_t MAnsiString::indexOf(const char *str, size_t start) const
{
	assert(start >= 0);
	if(str == nullptr) return kInvalid;
	
	size_t length = strlen(str);
	if(length == 0) return kInvalid;

	for(size_t i=(size_t)start; i<mLength; i++){
		if(i + length > mLength) break;
		if(strncmp(&mStr[i], str, length) == 0){
			return i;
		}
	}

	return kInvalid;
}

size_t MAnsiString::indexOf(char character, size_t start) const
{
	assert(start != kInvalid);
	for(const char *ptr = mStr + start; ptr < mStr + mLength; ptr = next(ptr)){
		if(*ptr == character){
			return ptr - mStr;
		}
	}
	return kInvalid;
}

size_t MAnsiString::indexOf(const std::vector<char>& characters, size_t start) const
{
	assert(start != kInvalid);
	for(const char *ptr = mStr + start; ptr < mStr + mLength; ptr = next(ptr)){
		if(characters.end() != std::find(characters.begin(), characters.end(), *ptr)){
			return ptr - mStr;
		}
	}
	return -1;
}

size_t MAnsiString::lastIndexOf(const MAnsiString& str, size_t start) const
{
	if(str.length() == 0) return kInvalid;
	if(mLength < str.length()) return kInvalid;
	if(start == kInvalid || start > mLength - str.length()){
		start = mLength - str.length();
	}

	for(const char *ptr = mStr + start; ptr >= mStr; ptr = prev(ptr)){
		if(strncmp(ptr, str.c_str(), str.length()) == 0){
			return ptr - mStr;
		}
	}	
	return kInvalid;
}

size_t MAnsiString::lastIndexOf(char character, size_t start) const
{
	if(mLength == 0) return kInvalid;
	if(start == kInvalid || start >= mLength) start = mLength - 1;

	for(const char *ptr = mStr + start; ptr >= mStr; ptr = prev(ptr)){
		if(*ptr == character){
			return ptr - mStr;
		}
	}
	return -1;
}

size_t MAnsiString::lastIndexOf(const std::vector<char>& characters, size_t start) const
{
	if(mLength == 0) return kInvalid;
	if(start == kInvalid || start >= mLength) start = mLength - 1;

	for(const char *ptr = mStr + start; ptr >= mStr; ptr = prev(ptr)){
		if(characters.end() != std::find(characters.begin(), characters.end(), *ptr)){
			return static_cast<int>(ptr - mStr);
		}
	}
	return kInvalid;
}

std::vector<MAnsiString> MAnsiString::split(const MAnsiString& separator) const
{
	std::vector<MAnsiString> ret;
	size_t pos = 0;
	while(1){
		size_t next = indexOf(separator, pos);
		if(next == kInvalid){
			ret.push_back(substring(pos));
			break;
		}
		ret.push_back(substring(pos, next-pos));
		pos = static_cast<int>(next + separator.length());
	}
	return ret;
}

std::vector<MAnsiString> MAnsiString::split(char separator) const
{
	std::vector<MAnsiString> ret;
	size_t pos = 0;
	while(1){
		size_t next = indexOf(separator, pos);
		if(next == kInvalid){
			ret.push_back(substring(pos));
			break;
		}
		ret.push_back(substring(pos, next-pos));
		pos = next + 1;
	}
	return ret;
}

std::vector<MAnsiString> MAnsiString::split(const std::vector<char>& separators) const
{
	std::vector<MAnsiString> ret;
	size_t pos = 0;
	while(1){
		size_t next = indexOf(separators, pos);
		if(next == kInvalid){
			ret.push_back(substring(pos));
			break;
		}
		ret.push_back(substring(pos, next-pos));
		pos = next + 1;
	}
	return ret;
}

MAnsiString MAnsiString::combine(const std::vector<MAnsiString>& strings, const MAnsiString& separator)
{
	MAnsiString ret;
	for(size_t i=0; i<strings.size(); i++){
		if(i > 0) ret += separator;
		ret += strings[i];
	}
	return ret;
}

MAnsiString MAnsiString::combine(const std::vector<MAnsiString>& strings, char separator)
{
	MAnsiString ret;
	for(size_t i=0; i<strings.size(); i++){
		if(i > 0) ret += separator;
		ret += strings[i];
	}
	return ret;
}

MAnsiString MAnsiString::toLowerCase() const
{
	size_t len = length();
	MAnsiString ret;
	ret.resize(len);
	for(size_t i=0; i<len; i++) {
		ret[i] = tolower(mStr[i]);
	}

	return ret;
}

MAnsiString MAnsiString::toUpperCase() const
{
	size_t len = length();
	MAnsiString ret;
	ret.resize(len);
	for(size_t i=0; i<len; i++) {
		ret[i] = toupper(mStr[i]);
	}

	return ret;
}

template<typename T> T parseRadix16(const char *ptr, const char *end_ptr)
{
	T val = 0;
	for(; ptr < end_ptr; ptr++){
		if(*ptr >= '0' && *ptr <= '9'){
			val = (val << 4) | (*ptr - '0');
		}else if(*ptr >= 'A' && *ptr <= 'F'){
			val = (val << 4) | (*ptr - 'A' + 10);
		}else if(*ptr >= 'a' && *ptr <= 'f'){
			val = (val << 4) | (*ptr - 'a' + 10);
		}else{
			break;
		}
	}
	return val;
}

int MAnsiString::toInt() const
{
	// 0x** is treated as 16 radix number
	if(mLength >= 3 && mStr[0] == '0' && (mStr[1] == 'x' || mStr[1] == 'X')){
		return parseRadix16<int>(mStr + 2, mStr + mLength);
	}

	return atoi(mStr);
}

int MAnsiString::toIntWithRadix(int base) const
{
	return strtol(mStr, NULL, base);
}

unsigned int MAnsiString::toUInt() const
{
	// 0x** is treated as 16 radix number
	if(mLength >= 3 && mStr[0] == '0' && (mStr[1] == 'x' || mStr[1] == 'X')){
		return parseRadix16<unsigned int>(mStr + 2, mStr + mLength);
	}

	__int64 val = std::max((__int64)0, std::min(_atoi64(mStr), (__int64)UINT_MAX));
	return (unsigned int)val;
}

unsigned int MAnsiString::toUIntWithRadix(int base) const
{
	return strtoul(mStr, NULL, base);
}

__int64 MAnsiString::toInt64() const
{
	// 0x** is treated as 16 radix number
	if(mLength >= 3 && mStr[0] == '0' && (mStr[1] == 'x' || mStr[1] == 'X')){
		return parseRadix16<__int64>(mStr + 2, mStr + mLength);
	}

	return _atoi64(mStr);
}

__int64 MAnsiString::toInt64WithRadix(int base) const
{
	return _strtoi64(mStr, NULL, base);
}

unsigned __int64 MAnsiString::toUInt64() const
{
	// 0x** is treated as 16 radix number
	if(mLength >= 3 && mStr[0] == '0' && (mStr[1] == 'x' || mStr[1] == 'X')){
		return parseRadix16<unsigned __int64>(mStr + 2, mStr + mLength);
	}

	return _atoi64(mStr);
}

unsigned __int64 MAnsiString::toUInt64WithRadix(int base) const
{
	return _strtoui64(mStr, NULL, base);
}

float MAnsiString::toFloat(void) const
{
	double val = strtod(mStr, NULL);
	return (float)val;
}

double MAnsiString::toDouble(void) const
{
	double val = strtod(mStr, NULL);
	return val;
}

bool MAnsiString::canParseInt() const
{
	char *endptr;
	strtol(mStr, &endptr, 10);
	return (endptr == nullptr) || (endptr == mStr + mLength);
}

bool MAnsiString::canParseFloat() const
{
	char *endptr;
	strtod(mStr, &endptr);
	return (endptr == nullptr) || (endptr == mStr + mLength);
}

bool MAnsiString::canParseDouble() const
{
	char *endptr;
	strtod(mStr, &endptr);
	return (endptr == nullptr) || (endptr == mStr + mLength);
}

MAnsiString MAnsiString::fromInt(int value, int radix)
{
	char buf[30];
#if _MSC_VER >= 1400 || __BORLAND_C__ >= 0x0630
	errno_t ret = _itoa_s(value, buf, _countof(buf), radix);

	if(ret != 0){
		return MAnsiString();
	}
	return MAnsiString(buf);
#else
	itoa(value, buf, radix);
	return MAnsiString(buf);
#endif
}

MAnsiString MAnsiString::fromUInt(unsigned int value, int radix)
{
	char buf[30];
#if _MSC_VER >= 1400 || __BORLAND_C__ >= 0x0630
	errno_t ret = _ultoa_s(value, buf, _countof(buf), radix);
	if(ret != 0){
		return MAnsiString();
	}
#else
	ultoa(value, buf, radix);
#endif
	return MAnsiString(buf);
}

MAnsiString MAnsiString::fromInt64(__int64 value, int radix)
{
	char buf[30];
#if _MSC_VER >= 1400 || __BORLAND_C__ >= 0x0630
	errno_t ret = _i64toa_s(value, buf, _countof(buf), radix);
	if(ret != 0){
		return MAnsiString();
	}
#else
	_i64toa(value, buf, radix);
#endif
	return MAnsiString(buf);
}

MAnsiString MAnsiString::fromUInt64(unsigned __int64 value, int radix)
{
	char buf[30];
#if _MSC_VER >= 1400 || __BORLAND_C__ >= 0x0630
	errno_t ret = _ui64toa_s(value, buf, _countof(buf), radix);
	if(ret != 0){
		return MAnsiString();
	}
#else
	_ui64toa(value, buf, radix);
#endif
	return MAnsiString(buf);
}

// Convert from float value
MAnsiString MAnsiString::fromFloat(float value, int digit, int max_digit)
{
	float absval = fabs(value);
	int d = (absval > 0) ? (int)floor(log10(absval)) : 0;
	MAnsiString format = MAnsiString::format("%%0.%df", std::min(std::max(0, digit-d-1), max_digit));
	return MAnsiString::format(format.c_str(), value);
}

// Convert from double value
MAnsiString MAnsiString::fromDouble(double value, int digit, int max_digit)
{
	double absval = fabs(value);
	int d = (absval > 0) ? (int)floor(log10(absval)) : 0;
	MAnsiString format = MAnsiString::format("%%0.%dlf", std::min(std::max(0, digit-d-1), max_digit));
	return MAnsiString::format(format.c_str(), value);
}

// Check if is numberic string
bool MAnsiString::isNumber() const
{
	bool sign = false;
	bool number = false;
	bool period = false;
	bool exponent = false;
	bool hex = false;
	for(const char *ptr = mStr; ptr < mStr + mLength; ptr = next(ptr))
	{
		switch(*ptr){
		case '-':
		case '+':
			if(number || period || sign || hex){
				return false;
			}
			sign = true;
			break;
		case '.':
			if(period || exponent || hex){
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
			if(!hex){
				if(!number || exponent){
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
			if(!hex) return false;
			number = true;
			break;
		case 'x':
		case 'X':
			if(hex) return false;
			if(ptr == mStr) return false;
			if(*(ptr - 1) != '0') return false;
			if(ptr - 2 == mStr && (*(ptr - 2) != '+' && *(ptr - 2) != '-')) return false;
			if(ptr - 3 >= mStr) return false;
			hex = true;
			break;
		default:
			return false;
		}
	}

	return number;
}

// Trim 0 for numeric string
bool MAnsiString::trimNumber()
{
	if(!isNumber()) return false;

	bool trimmed = false;

	// Remove end 0
	size_t period = lastIndexOf('.');
	if(period != kInvalid){
		int zero_count = 0;
		size_t pos = mLength - 1;
		for(; pos > period; pos--){
			if(mStr[pos] == 'E' || mStr[pos] == 'e'){
				zero_count = 0;
				break;
			}
			if(mStr[pos] >= '1' && mStr[pos] <= '9'){
				break;
			}
			if(mStr[pos] != '0'){
				zero_count = 0;
				break;
			}
			zero_count++;
		}
		if(pos == period){
			resize(pos);
			trimmed = true;
		}
		else if(zero_count > 0){
			resize(mLength - zero_count);
			trimmed = true;
		}
	}

	// Remove header '+'
	if(mLength > 0 && mStr[0] == '+'){
		*this = substring(1);
		trimmed = true;
	}

	return trimmed;
}

MAnsiString MAnsiString::getTrimDecimalZero() const
{
	size_t pos = mLength;
	bool other = false;
	for(size_t i = mLength; ; ){
		i = prev(i);
		if(i == kInvalid) break;

		if(mStr[i] == '.'){
			if(other){
				return substring(0, pos);
			}else{
				return substring(0, i);
			}
		}else if(mStr[i] == '0'){
			if(!other){
				pos = i;
			}
		}else if(mStr[i] >= '1' && mStr[i] <= '9'){
			other = true;
		}else{
			break;
		}
	}

	return *this;
}

MAnsiString MAnsiString::getTrimSpace() const
{
	if(mLength == 0) return MAnsiString();

	size_t pre_pos = mLength;
	for(size_t i = 0; i < mLength && i != kInvalid; i = next(i)){
		if(!(mStr[i] == ' ' || mStr[i] == '\t')){
			pre_pos = i;
			break;
		}
	}
	size_t pst_pos = pre_pos;
	for(size_t n = mLength; n > pre_pos && n != kInvalid; ){
		size_t i = prev(n);
		if(i != kInvalid && !(mStr[i] == ' ' || mStr[i] == '\t')){
			pst_pos = n;
			break;
		}
		n = i;
	}

	return substring(pre_pos, pst_pos - pre_pos);
}

MAnsiString MAnsiString::format(const char *format, ...)
{
	va_list argp;
	va_start(argp, format);

#if _MSC_VER >= 1400
	int num = _vscprintf(format, argp);
	if(num <= 0) {
		va_end(argp);
		return MAnsiString();
	}

	MAnsiString ret;
	ret.resize(num);
	num = vsprintf_s(ret.c_str(), ret.capacity(), format, argp);
	if(num < 0) {
		va_end(argp);
		return MAnsiString();
	}
	va_end(argp);
	return ret;
#else
	char *temp = NULL;
	int num = 0;
	for(size_t temp_len = 1024; temp_len < 1024 * 1024; temp_len *= 4){
		temp = (char*)malloc(sizeof(char)*temp_len);
		if(temp == NULL){
			va_end(argp);
			return MAnsiString();
		}

		num = _vsnprintf(temp, temp_len, format, argp);
		if(num >= 0){
			break;
		}

		free(temp);
		temp = NULL;
	}
	if(num <= 0) {
		free(temp);
		va_end(argp);
		return MAnsiString();
	}

	MAnsiString ret;
	ret.resize(num);
	memcpy_s(ret.c_str(), sizeof(char)*ret.mCapacity, temp, sizeof(char)*(num+1));
	free(temp);
	va_end(argp);
	return ret;
#endif
}

int MAnsiString::formatSet(const char *format, ...)
{
	va_list argp;
	va_start(argp, format);

#if _MSC_VER >= 1400
	int num = _vscprintf(format, argp);
	if(num <= 0) {
		va_end(argp);
		return num;
	}

	resize(num);
	num = vsprintf_s(c_str(), capacity(), format, argp);
	if(num < 0) {
		va_end(argp);
		return num;
	}
	va_end(argp);
	return num;
#else
	char *temp = NULL;
	int num = 0;
	for(size_t temp_len = 1024; temp_len < 1024 * 1024; temp_len *= 4){
		temp = (char*)malloc(sizeof(char)*temp_len);
		if(temp == NULL){
			va_end(argp);
			return -1;
		}

		num = _vsnprintf(temp, temp_len, format, argp);
		if(num >= 0){
			break;
		}

		free(temp);
		temp = NULL;
	}
	if(num <= 0) {
		free(temp);
		va_end(argp);
		return num;
	}

	resize(num);
	memcpy_s(c_str(), sizeof(char)*mCapacity, temp, sizeof(char)*(num+1));
	free(temp);
	va_end(argp);
	return num;
#endif
}

int MAnsiString::compare(const MAnsiString& str) const
{
	return strncmp(mStr, str.c_str(), std::max(mLength, str.length()));
}

int MAnsiString::compareIgnoreCase(const MAnsiString& str) const
{
#if _MSC_VER >= 1400
	return _strnicmp(mStr, str.c_str(), std::max(mLength, str.length()));
#else
	return strnicmp(mStr, str.c_str(), std::max(mLength, str.length()));
#endif
}

int MAnsiString::compareSubstring(size_t start, const MAnsiString& str) const
{
	return strncmp(mStr + start, str.c_str(), str.length());
}

int MAnsiString::compareSubstringIgnoreCase(size_t start, const MAnsiString& str) const
{
#if _MSC_VER >= 1400
	return _strnicmp(mStr + start, str.c_str(), str.length());
#else
	return strnicmp(mStr + start, str.c_str(), str.length());
#endif
}

MAnsiString& MAnsiString::operator = (const MAnsiString& str)
{
	if(mStr == str.c_str()){
		return *this;
	}

	size_t len = str.length();
	if(len == 0){
		clear();
	}else{
		resize(len);
		memcpy_s(mStr, sizeof(char)*mCapacity, str.c_str(), sizeof(char)*len);
		mStr[len] = '\0';
	}
	return *this;
}

MAnsiString& MAnsiString::operator = (const char *str)
{
	if(mStr == str){
		return *this;
	}

	size_t len = (str != nullptr) ? strlen(str) : 0;
	if(len == 0){
		clear();
	}else{
		resize(len);
		memcpy_s(mStr, sizeof(char)*mCapacity, str, sizeof(char)*len);
		mStr[len] = '\0';
	}
	return *this;
}

#ifndef MSTRING_DISABLE_STDSRING
MAnsiString& MAnsiString::operator = (const std::string& str)
{
	if(mStr == str.c_str()){
		return *this;
	}

	size_t len = str.length();
	if(len == 0){
		clear();
	}else{
		resize(len);
		memcpy_s(mStr, sizeof(char)*mCapacity, str.c_str(), sizeof(char)*len);
		mStr[len] = '\0';
	}
	return *this;
}
#endif

#if _MSC_VER >= 1600 || __BORLAND_C__ >= 0x0630
MAnsiString& MAnsiString::operator = (MAnsiString&& str)
{
	if(mStr == str.c_str()){
		return *this;
	}

	if(mStr != null_str){
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

MAnsiString& MAnsiString::operator += (const MAnsiString& str)
{
	size_t len2 = str.length();
	if(len2 == 0){
		return *this;
	}

	size_t len1 = length();
	resize(len1 + len2);
	memcpy_s(mStr + len1, sizeof(char)*(mCapacity-len1), str.c_str(), sizeof(char)*len2);
	mStr[len1 + len2] = '\0';
	return *this;
}

MAnsiString& MAnsiString::operator += (const char *str)
{
	size_t len2 = (str != nullptr) ? strlen(str) : 0;
	if(len2 == 0){
		return *this;
	}

	size_t len1 = length();
	resize(len1 + len2);
	memcpy_s(mStr + len1, sizeof(char)*(mCapacity-len1), str, sizeof(char)*len2);
	mStr[len1 + len2] = '\0';
	return *this;
}

MAnsiString& MAnsiString::operator += (char character)
{
	size_t len1 = length();
	resize(len1 + 1);
	mStr[len1] = character;
	mStr[len1 + 1] = '\0';
	return *this;
}

MAnsiString MAnsiString::operator + (const MAnsiString& str) const
{
	MAnsiString ret(*this);
	ret += str;
	return ret;
}

MAnsiString MAnsiString::operator + (const char *str) const
{
	MAnsiString ret(*this);
	ret += str;
	return ret;
}

MAnsiString MAnsiString::operator + (char character) const
{
	MAnsiString ret(*this);
	ret += character;
	return ret;
}

#if _MSC_VER >= 1600 || __BORLAND_C__ >= 0x0630
MAnsiString operator + (MAnsiString&& str1, const MAnsiString& str2)
{
	str1 += str2;
	return std::move(str1);
}

MAnsiString operator + (MAnsiString&& str1, const char *str2)
{
	str1 += str2;
	return std::move(str1);
}
#endif

MAnsiString operator + (const char *str1, const MAnsiString& str2)
{
	MAnsiString ret(str1);
	ret += str2;
	return ret;
}

bool MAnsiString::operator == (const MAnsiString& str) const
{
	if(length() != str.length()) return false;

	return (memcmp(mStr, str.c_str(), length()) == 0);
}

bool MAnsiString::operator == (const char *str) const
{
	size_t len = (str != nullptr) ? strlen(str) : 0;
	if(length() != len) return false;

	return (memcmp(mStr, str, length()) == 0);
}

#ifndef MSTRING_DISABLE_STDSRING
bool MAnsiString::operator == (const std::string& str) const
{
	size_t len = str.length();
	if(length() != len) return false;

	return (memcmp(mStr, str.c_str(), length()) == 0);
}
#endif

bool operator == (const char *str1, const MAnsiString& str2)
{
	size_t len = (str1 != nullptr) ? strlen(str1) : 0;
	if(len != str2.length()) return false;

	return (memcmp(str1, str2.c_str(), len) == 0);
}

#ifndef MSTRING_DISABLE_STDSRING
bool operator == (const std::string& str1, const MAnsiString& str2)
{
	size_t len = str1.length();
	if(len != str2.length()) return false;

	return (memcmp(str1.c_str(), str2.c_str(), len) == 0);
}
#endif

bool MAnsiString::operator != (const MAnsiString& str) const
{
	if(length() != str.length()) return true;

	return (memcmp(mStr, str.c_str(), length()) != 0);
}

bool MAnsiString::operator != (const char *str) const
{
	size_t len = (str != nullptr) ? strlen(str) : 0;
	if(length() != len) return true;

	return (memcmp(mStr, str, length()) != 0);
}

#ifndef MSTRING_DISABLE_STDSRING
bool MAnsiString::operator != (const std::string& str) const
{
	size_t len = str.length();
	if(length() != len) return true;

	return (memcmp(mStr, str.c_str(), length()) != 0);
}
#endif

bool operator != (const char *str1, const MAnsiString& str2)
{
	size_t len = (str1 != nullptr) ? strlen(str1) : 0;
	if(len != str2.length()) return true;

	return (memcmp(str1, str2.c_str(), len) != 0);
}

#ifndef MSTRING_DISABLE_STDSRING
bool operator != (const std::string& str1, const MAnsiString& str2)
{
	size_t len = str1.length();
	if(len != str2.length()) return true;

	return (memcmp(str1.c_str(), str2.c_str(), len) != 0);
}
#endif

