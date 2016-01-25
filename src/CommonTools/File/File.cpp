/*
 * CEmFile.cpp
 *
 *  Created on: 2012-5-16
 *      Author: root
 */

#include "File.h"
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>


CFile::CFile() {
	// TODO Auto-generated constructor stub

}

CFile::~CFile() {
	// TODO Auto-generated destructor stub
}

/*
 * be exists file or directory
 * Parameters: file path
 * Return-Value: exists return true, not exists return false
 */
bool CFile::FileBeExists(const char* const pchPath)
{
	return !access(pchPath, 0);
}

/*
 * create directory
 * Parameters: directory path
 * Return-Value: success return true else return false
 */
bool CFile::FileCreatDir(const char* const pchPath)
{
	std::string strDir;
	int iLen = strlen(pchPath);
	int i = 0;
	for(; i < iLen; ++i) {
		if(pchPath[i] == '/') {
			strDir = std::string(pchPath, 0, i + 1);
			if(!FileBeExists(strDir.c_str()) && mkdir(strDir.c_str(), 0777) != 0) {
				return false;
			}
		}
	}
	strDir = std::string(pchPath, 0, i);
	if(!FileBeExists(strDir.c_str()) && mkdir(strDir.c_str(), 0777) != 0) {
		return false;
	}
	return true;
}

/*
 * list all directories
 * Parameters:
 * pchPath: list directory
 * vecDirs: save all list directories
 * Return-Value: success return true else return false
 */
bool CFile::FileListDirs(const char* const pchPath, std::vector<std::string>& vecDirs)
{
	if(!FileBeExists(pchPath)) {
		return false;
	}
    struct dirent* pstDirEnt = NULL;
    DIR *pstDir = NULL;
    if((pstDir = opendir(pchPath)) == NULL) {
    	return false;
    }
    while((pstDirEnt = readdir(pstDir))) {
    	if(pstDirEnt->d_type == DT_DIR ) {
    		if(strncmp(pstDirEnt->d_name,FILE_DOT, 1) && strncmp(pstDirEnt->d_name,FILE_DOT_DOT, 2)) {
    			vecDirs.push_back(pstDirEnt->d_name);
    		}
    	}
    }
    closedir(pstDir);
	return true;
}

/*
 *
 */
bool CFile::FileListFiles(const char* const pchPath, std::vector<std::string>& vecFiles)
{
	if(!FileBeExists(pchPath)) {
		return false;
	}
    struct dirent* pstDirEnt = NULL;
    DIR *pstDir = NULL;
    if((pstDir = opendir(pchPath)) == NULL) {
    	return false;
    }
    while((pstDirEnt = readdir(pstDir))) {
    	if(pstDirEnt->d_type == DT_REG ) {
    			vecFiles.push_back(pstDirEnt->d_name);
    	}
    }
    closedir(pstDir);
	return true;
}

/*
 *
 */
bool CFile::FileReadFile(const char* const pchFileName, std::string& strContent)
{
	if(pchFileName == NULL) {
		return false;
	}
	FILE *fpfile = NULL;
	char* pszbuf = NULL;
	u_long ulFileLen = 0;
	fpfile = fopen(pchFileName, "r");
	if (fpfile == NULL) {
		return false;
	} else {
		fseek(fpfile, 0L, SEEK_END);
		ulFileLen = ftell(fpfile);
		fseek(fpfile, 0L, SEEK_SET);
		pszbuf = new char[ulFileLen +1];
		memset(pszbuf, 0, ulFileLen + 1);
		fread(pszbuf, 1, ulFileLen, fpfile);
	}
	fclose(fpfile);
	strContent = pszbuf;
	if(pszbuf) {
		delete pszbuf;
	}
	return true;
}

/*
 *
 */
char* CFile::FileReadFile(const char* const pchFileName, u_long& ulLength)
{
	if(pchFileName == NULL) {
		return false;
	}
	FILE *fpfile = NULL;
	char* pszbuf = NULL;
	fpfile = fopen(pchFileName, "r");
	if (fpfile == NULL) {
		return false;
	} else {
		fseek(fpfile, 0L, SEEK_END);
		ulLength = ftell(fpfile);
		fseek(fpfile, 0L, SEEK_SET);
		pszbuf = new char[ulLength +1];
		memset(pszbuf, 0, ulLength + 1);
		fread(pszbuf, 1, ulLength, fpfile);
	}
	fclose(fpfile);
	return pszbuf;
}

/*
 *
 */
bool CFile::FileWriteFile(const char* const pchMode, const char* const pchFileName, const char* const pchContent, const u_long ulLength)
{
	if(pchFileName == NULL) {
		return false;
	}
	FILE *fpfile = NULL;
	fpfile = fopen(pchFileName, pchMode);
	if (fpfile == NULL) {
		return false;
	} else {

		if(fwrite(pchContent, ulLength, 1, fpfile) <= 0) {
			fclose(fpfile);
			return false;
		}
	}
	fclose(fpfile);
	return true;
}

/*
 *
 */
long CFile::FileGetFileLength(const char* const pchFileName)
{
	long ulFileLen = -1;
	if(pchFileName == NULL) {
		return ulFileLen;
	}
	FILE *fpfile = NULL;
	fpfile = fopen(pchFileName, "r");
	if (fpfile == NULL) {
		return ulFileLen;
	} else {
		fseek(fpfile, 0L, SEEK_END);
		ulFileLen = ftell(fpfile);
		fseek(fpfile, 0L, SEEK_SET);
	}
	fclose(fpfile);
	return ulFileLen;
}

/*
 *
 */
bool CFile::FileRemoveFile(const char* const pchFileName)
{
	return (remove(pchFileName) == 0);
}
