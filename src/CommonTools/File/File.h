/*
 * CEmFile.h
 *
 *  Created on: 2012-5-16
 *      Author: root
 */

#ifndef CEMFILE_H_
#define CEMFILE_H_

#include <vector>
#include <string>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define FILE_DOT "."
#define FILE_DOT_DOT ".."

class CFile {
public:
	CFile();
	virtual ~CFile();

	bool FileBeExists(const char* const pchPath);
	bool FileCreatDir(const char* const pchPath);
	bool FileListDirs(const char* const pchPath, std::vector<std::string>& vecDirs);
	bool FileListFiles(const char* const pchPath, std::vector<std::string>& vecFiles);
	bool FileReadFile(const char* const pchFileName, std::string& strContent);
	char* FileReadFile(const char* const pchFileName, u_long& ulLength);
	bool FileWriteFile(const char* const pchMode, const char* const pchFileName, const char* const pchContent, const u_long ulLength);
	long FileGetFileLength(const char* const pchFileName);
	bool FileRemoveFile(const char* const pchFileName);
private:

};

#endif /* CEMFILE_H_ */
