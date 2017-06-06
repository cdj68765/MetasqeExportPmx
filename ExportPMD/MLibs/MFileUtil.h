#pragma once

#ifndef _MFILEUTIL_H_
#define _MFILEUTIL_H_

#include "MLibsDll.h"
#include <vector>
#include "MString.h"


// Utilities for file access
class MLIBS_API MFileUtil
{
public:
	// Check if a file exists
	static bool fileExists(const MString& filename);
	
	// Check if a destination of a path is a directory
	static bool directoryExists(const MString& path);

	// Get a current directory.
	static MString getCurrentDirectory();

	// Set a current directory.
	static void setCurrentDirectory(const MString& dir);

	// Enum files in the directory
	static std::vector<MString> enumFilesInDirectory(const MString& dir_path, const MString& filter = L"*", bool file_only = true, bool exclude_hidden = false);

	// Enum sub directories in the directory
	static std::vector<MString> enumDirectoriesInDirectory(const MString& dir_path, bool exclude_hidden = false);

	// Create a directory
	static bool createDirectory(const MString& dst_path);

	// Copy a file
	static bool copyFile(const MString& dst_file, const MString& src_file);
	// Copy a file with retrying if failed.
	static bool copyFileWithRetry(const MString& dst_file, const MString& src_file, int retry_num = 10, int interval_msec = 1000);

	// Delete a file
	static bool deleteFile(const MString& path);

	// Return a path that extension is changed
	//   ex. "c:\temp\test.bmp", ".jpg" -> "c:\temp\test.jpg"
	static MString changeExtension(const MString& src_path, const MString& extension);

	// Return an extension in the path
	//   ex. "c:\temp\test.bmp" -> "bmp"
	static MString extractExtension(const MString& src_path);

	// Return a filename only (excluding an extension) in the path
	//   ex. "c:\temp\test.bmp" -> "test"
	static MString extractFileNameOnly(const MString& src_path);

	// Return a directory in the path
	//   ex. "c:\temp\test.bmp" -> "c:\temp\"
	//   ex. "c:\temp\" -> "c:\temp\"
	static MString extractDirectory(const MString& src_path);

	// Return a filename with an extension in the path
	//   ex. "c:\temp\test.bmp" -> "test.bmp"
	static MString extractFilenameAndExtension(const MString& src_path);

	// Return a drive name in the path
	//   ex. "c:\temp\test.bmp" -> "c:\"
	//   ex. "\\server\temp\test.bmp" -> "\\server\"
	static MString extractDrive(const MString& src_path);

	// Return an up level directory
	//   ex. "c:\temp\test.bmp" -> "c:\temp\"
	static MString getUpDirectory(const MString& src_path);

	// Combine two pathes
	//   ex. "c:\temp\sub\", "..\sub2\test.bmp" -> "c:\temp\sub2\temp.bmp"
	static MString combinePath(const MString& base_dir, const MString& cat_dir);

	// Return a full path from a relative path
	//   ex. "test.bmp" -> "c:\temp\sub2\temp.bmp"
	static MString extractFullPath(const MString& src_path);

	// Return a relative path
	//   ex. "c:\temp\", "c:\temp\sub\temp.bmp" -> "sub\temp.bmp"
	static MString extractRelativePath(const MString& base_dir, const MString& src_path);

	// Check if a path is relative
	static bool isPathRelative(const MString& path);

};

#endif _MFILEUTIL_H_
