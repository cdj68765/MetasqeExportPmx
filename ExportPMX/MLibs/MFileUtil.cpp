#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <shlwapi.h>
#include <ShlObj.h>
#endif
#include "MFileUtil.h"
#include "MString.h"
#include "MAnsiString.h"

inline bool isPathSeparator(wchar_t character)
{
	return (character == L'\\' || character == L'/');
}

inline wchar_t createPathSeparator(const MString& path)
{
	for (const wchar_t* ptr = path.c_str() + path.length(); ptr > path.c_str();)
	{
		ptr = path.prev(ptr);
		if (isPathSeparator(*ptr))
		{
			return *ptr;
		}
	}
	return L'\\'; // return default separator
}

//------------------------------------------------------------------
//  class MFileUtil
//------------------------------------------------------------------

// Check if a file exists
bool MFileUtil::fileExists(const MString& filename)
{
	return PathFileExistsW(filename.c_str()) && !PathIsDirectoryW(filename.c_str());
}

// Check if a destination of a path is a directory
bool MFileUtil::directoryExists(const MString& path)
{
	return PathIsDirectoryW(path.c_str()) ? true : false;
}

// Get a current directory.
MString MFileUtil::getCurrentDirectory()
{
	wchar_t path[MAX_PATH];
	DWORD len = ::GetCurrentDirectory(_countof(path), path);
	if (len == 0)
	{
		return MString();
	}
	return MString(path);
}

// Set a current directory.
void MFileUtil::setCurrentDirectory(const MString& dir)
{
	::SetCurrentDirectory(dir.c_str());
}

// Enum files in the directory
std::vector<MString> MFileUtil::enumFilesInDirectory(const MString& dir_path, const MString& filter, bool file_only, bool exclude_hidden)
{
	std::vector<MString> ret;

	WIN32_FIND_DATAW fd;
	HANDLE handle = FindFirstFileW(combinePath(dir_path, filter).c_str(), &fd);
	if (handle == INVALID_HANDLE_VALUE)
	{
		return ret;
	}

	do
	{
		if ((wcscmp(fd.cFileName, L".") == 0) || (wcscmp(fd.cFileName, L"..") == 0))
		{
			continue; // current or up directory is ignored
		}

		if (file_only && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			continue; // directory is ignored
		}
		if (exclude_hidden && (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
		{
			continue; // hidden file or directory is ignored
		}

		ret.push_back(fd.cFileName);
	}
	while (FindNextFileW(handle, &fd));

	return ret;
}

// Enum sub directories in the directory
std::vector<MString> MFileUtil::enumDirectoriesInDirectory(const MString& dir_path, bool exclude_hidden)
{
	std::vector<MString> ret;

	WIN32_FIND_DATAW fd;
	HANDLE handle = FindFirstFileW(combinePath(dir_path, L"*").c_str(), &fd);
	if (handle == INVALID_HANDLE_VALUE)
	{
		return ret;
	}

	do
	{
		if ((wcscmp(fd.cFileName, L".") == 0) || (wcscmp(fd.cFileName, L"..") == 0))
		{
			continue; // current or up directory is ignored
		}

		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			continue; // file is ignored
		}
		if (exclude_hidden && (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
		{
			continue; // hidden directory is ignored
		}

		ret.push_back(fd.cFileName);
	}
	while (FindNextFileW(handle, &fd));

	return ret;
}

// Create a directory
bool MFileUtil::createDirectory(const MString& dst_path)
{
	if (directoryExists(dst_path)) return true;

#if 1
	std::vector<MString> up_dirs;
	up_dirs.push_back(dst_path);
	while (true)
	{
		MString up_dir = getUpDirectory(up_dirs.back());
		if (up_dir.length() == 0 || directoryExists(up_dir))
		{
			break;
		}
		up_dirs.push_back(up_dir);
	}

	for (auto it = up_dirs.rbegin(); it != up_dirs.rend(); ++it)
	{
		if (!CreateDirectoryW((*it).c_str(), nullptr))
		{
			return false;
		}
	}
	return true;
#else // SHCreateDirectoryExW() needs XP SP2 or later.
	int ret = ::SHCreateDirectoryExW(NULL, dst_path.c_str(), NULL);
	return ret == ERROR_SUCCESS;
#endif
}

// Copy a file
bool MFileUtil::copyFile(const MString& dst_file, const MString& src_file)
{
	return CopyFileW(src_file.c_str(), dst_file.c_str(), FALSE) ? true : false;
}

// Copy a file with retrying if failed.
bool MFileUtil::copyFileWithRetry(const MString& dst_file, const MString& src_file, int retry_num, int interval_msec)
{
	for (int n = 0; n < retry_num; n++)
	{
		if (copyFile(dst_file, src_file))
		{
			return true;
		}

		if (n == 0)
		{
			DWORD err = GetLastError();
			if (err == ERROR_FILE_NOT_FOUND)
			{
				return false; // Return immediately when the file is not found.
			}
		}
	}

	return false;
}

// Delete a file
bool MFileUtil::deleteFile(const MString& path)
{
	return ::DeleteFile(path.c_str()) ? true : false;
}

// Return a path that extension is changed
MString MFileUtil::changeExtension(const MString& src_path, const MString& extension)
{
	size_t len = src_path.length();

	for (const wchar_t* ptr = src_path.c_str() + len; ptr > src_path.c_str();)
	{
		ptr = src_path.prev(ptr);
		if (*ptr == L'.')
		{
			return src_path.substring(0, ptr - src_path.c_str()) + extension;
		}
		if (isPathSeparator(*ptr))
		{
			return src_path + extension;
		}
	}

	// not find either period or \, so add extension
	return src_path + extension;
}

// Return an extension in the path
MString MFileUtil::extractExtension(const MString& src_path)
{
	size_t len = src_path.length();

	for (const wchar_t* ptr = src_path.c_str() + len; ptr > src_path.c_str();)
	{
		ptr = src_path.prev(ptr);
		if (*ptr == L'.')
		{
			return src_path.substring(ptr + 1 - src_path.c_str());
		}
		if (isPathSeparator(*ptr))
			break;
	}

	return MString();
}

// Return a filename only (excluding an extension) in the path
MString MFileUtil::extractFileNameOnly(const MString& src_path)
{
	size_t len = src_path.length();
	const wchar_t* endptr = src_path.c_str() + len;
	const wchar_t* ptr;
	for (ptr = endptr; ptr > src_path.c_str();)
	{
		ptr = src_path.prev(ptr);
		if (*ptr == L'.' && endptr == src_path.c_str() + len)
		{
			endptr = ptr;
		}
		if (isPathSeparator(*ptr))
		{
			ptr = src_path.next(ptr);
			break;
		}
	}
	if (ptr >= endptr)
	{
		return MString();
	}
	return src_path.substring(ptr - src_path.c_str(), endptr - ptr);
}

// Return a directory in the path
MString MFileUtil::extractDirectory(const MString& src_path)
{
	size_t len = src_path.length();

	for (const wchar_t* ptr = src_path.c_str() + len; ptr > src_path.c_str();)
	{
		ptr = src_path.prev(ptr);
		if (isPathSeparator(*ptr))
		{
			MString ret = src_path.substring(0, ptr - src_path.c_str());
			if (ret.length() > 0 && !isPathSeparator(ret[ret.length() - 1]))
			{
				ret += createPathSeparator(ret);
			}
			return ret;
		}
	}
	return MString();
}

// Return a filename with an extension in the path
MString MFileUtil::extractFilenameAndExtension(const MString& src_path)
{
	size_t len = src_path.length();
	const wchar_t* endptr = src_path.c_str() + len;
	const wchar_t* ptr;
	for (ptr = endptr; ptr > src_path.c_str();)
	{
		ptr = src_path.prev(ptr);
		if (isPathSeparator(*ptr))
		{
			ptr = src_path.next(ptr);
			break;
		}
	}
	if (ptr >= endptr)
	{
		return MString();
	}
	return MString(ptr);
}

// Return a drive name in the path
MString MFileUtil::extractDrive(const MString& src_path)
{
	// extract "A:\"
	if ((src_path.length() >= 2) &&
		((src_path[0] >= L'A' && src_path[0] <= L'Z') || (src_path[0] >= L'a' && src_path[0] <= L'z')) &&
		src_path[1] == L':')
	{
		return src_path.substring(0, 2) + L"\\";
	}

	// extract "\\server\"
	if ((src_path.length() >= 3) &&
		(isPathSeparator(src_path[0]) && src_path[0] == src_path[1]))
	{
		for (const wchar_t* ptr = src_path.c_str() + 2; ptr < src_path.c_str() + src_path.length();)
		{
			if (isPathSeparator(*ptr))
			{
				if (ptr == src_path.c_str() + 2)
				{
					return MString();
				}
				return src_path.substring(0, ptr - src_path.c_str() + 1);
			}
			ptr = src_path.next(ptr);
		}
		return src_path + L"\\";
	}

	return MString();
}

// Return an up level directory
MString MFileUtil::getUpDirectory(const MString& src_path)
{
	size_t len = src_path.length();
	if (len == 0)
	{
		return MString();
	}
	const wchar_t* endptr = src_path.c_str() + len;
	const wchar_t* ptr = src_path.prev(endptr);
	if (isPathSeparator(*ptr))
	{
		ptr = src_path.prev(ptr);
	}
	for (; ptr > src_path.c_str(); ptr = src_path.prev(ptr))
	{
		if (isPathSeparator(*ptr))
		{
			// remove header "\\" for remote
			if (ptr == src_path.c_str() + 1 && isPathSeparator(ptr[-1]))
			{
				return MString();
			}
			return src_path.substring(0, ptr - src_path.c_str() + 1);
		}
	}
	return MString();
}

// Combine two pathes
MString MFileUtil::combinePath(const MString& base_dir, const MString& cat_dir)
{
	wchar_t buf[MAX_PATH];
	PathCombineW(buf, base_dir, cat_dir);
	return MString(buf);
}

// Return a relative path
MString MFileUtil::extractFullPath(const MString& src_path)
{
	wchar_t buf[MAX_PATH];
	_wfullpath(buf, src_path.c_str(), _countof(buf));
	return MString(buf);
}

MString MFileUtil::extractRelativePath(const MString& base_dir, const MString& src_path)
{
	const wchar_t* base_ptr = base_dir.c_str();

	// Trim the end path separator. (for WindowsXP)
	MString base_trim;
	if (base_dir.length() >= 2 && isPathSeparator(base_dir[base_dir.length() - 1]) && !isPathSeparator(base_dir[base_dir.length() - 2]))
	{
		base_trim = base_dir.substring(0, base_dir.length() - 1);
		base_ptr = base_trim.c_str();
	}

	bool is_src_dir = directoryExists(src_path);

	wchar_t buf[MAX_PATH];
	if (::PathRelativePathTo(buf, base_ptr, FILE_ATTRIBUTE_DIRECTORY, src_path.c_str(), is_src_dir ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL))
	{
		if (buf[0] == L'.' && buf[1] == L'\\')
		{
			return MString(buf + 2);
		}
		return MString(buf);
	}
	return MString();
}

bool MFileUtil::isPathRelative(const MString& path)
{
	return ::PathIsRelative(path.c_str()) != FALSE;
}
