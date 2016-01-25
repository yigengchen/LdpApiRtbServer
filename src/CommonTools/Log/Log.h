/*
 * Log.h
 *

 */

#ifndef __LOG_H__
#define __LOG_H__

#include <stdarg.h>
#include <pthread.h>
#include <sstream>
//#include <sys/stat.h>
#include <dirent.h>
#include <ftw.h>
#include <iomanip>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
//#include <time.h>



#define MAX_LOG_PATH 1024
#define LOG(rank, format, args...) gp_log->write_d(rank, __FILE__, __LINE__, format, ##args)

enum LOG_RANK{DEBUG, INFO, WARNING, ERROR, REQUIRED = 100};

#include <vector>
#include <string>

#define FILE_DOT "."
#define FILE_DOT_DOT ".."

class CLOGFile {
public:
	CLOGFile();
	virtual ~CLOGFile();

	bool 	LogFileBeExists(const char* const pchPath);
	bool 	LogFileCreatDir(const char* const pchPath);
	bool 	LogFileListDirs(const char* const pchPath, std::vector<std::string>& vecDirs);
	bool 	LogFileListFiles(const char* const pchPath, std::vector<std::string>& vecFiles);
	bool 	LogFileReadFile(const char* const pchFileName, std::string& strContent);
	char*   LogFileReadFile(const char* const pchFileName, u_long& ulLength);
	bool 	LogFileWriteFile(const char* const pchMode, const char* const pchFileName, const char* const pchContent, const u_long ulLength);
	long 	LogFileGetFileLength(const char* const pchFileName);
	bool 	LogFileRemoveFile(const char* const pchFileName);
private:

};

class CLog : public CLOGFile
{
public:
	CLog(char* pszPath, char *pszFileName, int iLimit = 100000, const LOG_RANK rank = DEBUG);
	virtual ~CLog();

	bool write_d(LOG_RANK rank, const char* file, const int line, const char* format, ...);
	LOG_RANK LogGetLogRank();
	void LogSetLogRank(LOG_RANK rank);
	void LogPrintfLogRank();
	
protected:
	std::string getfulltime(const time_t seconds = 0);
	std::string LogGetDate(const time_t ttime = 0);
	bool LogCreatLogDir();
	template <typename T>
	std::string toString(const T& input) const
	{
		std::ostringstream oss;
		oss << input;
		oss.flush();
		return oss.str();
	}
private:
	pthread_mutex_t m_Mutex;
	pthread_cond_t m_Cond;
	char m_pszFileName[MAX_LOG_PATH];
	char m_pszPathName[MAX_LOG_PATH];
	LOG_RANK m_Rank;
	// read and write lock
	FILE* m_pFile;
	int m_iLogFileId;
	int m_iLogFileLine;
	int m_iLimit;
};


#endif /* __LOG_H__ */
