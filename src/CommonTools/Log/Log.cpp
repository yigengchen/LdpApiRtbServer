/*
 * Log.cpp
 *

 */
 
#include "Log.h"
#include <unistd.h>
#include <dirent.h>
 
CLog::CLog(char* pszPath,char *pszFileName, int iLimit, const LOG_RANK rank)
{
	pthread_mutex_init(&m_Mutex, 0);
	pthread_cond_init(&m_Cond, 0);
	if(pszFileName == NULL || strlen(pszFileName) == 0) {
		throw std::runtime_error("File name is empty.");
	}
	if(pszPath == NULL || strlen(pszPath) == 0) {
		throw std::runtime_error("File name is empty.");
	}
	memset(m_pszFileName, 0, MAX_LOG_PATH);
	memcpy(m_pszFileName, pszFileName, MAX_LOG_PATH);
	memset(m_pszPathName, 0, MAX_LOG_PATH);
	memcpy(m_pszPathName, pszPath, MAX_LOG_PATH);

	m_iLogFileId = 0;
	m_pFile = NULL;

	if(!LogCreatLogDir()) {
		throw std::runtime_error("Create log dir error.");
	}
	
	m_iLimit = iLimit;
	m_Rank = rank;
	//printf("iLimit=%ld\n",iLimit);
	//printf("m_iLimit=%ld\n",m_iLimit);
	//printf("m_Rank=%ld\n",m_Rank);

}

CLog::~CLog()
{
	if(m_pFile) {
		fclose(m_pFile);
	}
	pthread_mutex_destroy(&m_Mutex);
	pthread_cond_destroy(&m_Cond);
	printf("========pthread_mutex_destroy======\n");
}
  
//write file directly
bool CLog::write_d(LOG_RANK rank, const char* file, const int line, const char* format, ...)
{
	if(rank < m_Rank) {
		return false;
	}
	
/*
{
static u_int cnt=0;
++cnt;
va_list args;
va_start(args, format);
if(cnt%10000==1)  vprintf(format, args);
return true;
}
*/


	pthread_mutex_lock(&m_Mutex);
	LogCreatLogDir();
	
	std::string contents = getfulltime();
	std::string srank;
	switch(rank)
	{
	case DEBUG:
		srank = "DEBUG";
		break;
	case INFO:
		srank = "INFO";
		break;
	case WARNING:
		srank = "WARNING";
		break;
	case ERROR:
		srank = "ERROR";
		break;
	case REQUIRED:
		srank = "REQUIRED";
		break;
	default:
		srank = "LOG_RANK_ERROR";
		break;
	}
	contents += " " + srank + " ";
	int fret = fprintf(m_pFile, "%s%s:%d ", contents.c_str(), file, line);
	if(fret < 0)
	{
		pthread_mutex_unlock(&m_Mutex);
		pthread_cond_signal(&m_Cond);
		fprintf(stderr, "%s:%d fprintf write time error(%d).\n", __FILE__, __LINE__, fret);
		fclose(m_pFile);
		return false;
	}

	va_list args;
	va_start(args, format);
	int rst = vfprintf(m_pFile, format, args);
	if(rst < 0)
	{
		perror("vsprintf");
		va_end(args);
		pthread_mutex_unlock(&m_Mutex);
		pthread_cond_signal(&m_Cond);
		fprintf(stderr, format, args);
		fclose(m_pFile);
		return false;
	}
	fprintf(m_pFile, "\n");
	//m_iLogFileLine++; //  modify by cyg 20130709

	va_end(args);
	
	fflush(m_pFile);
	
	pthread_mutex_unlock(&m_Mutex);   //modify by cyg 20130709
	pthread_cond_signal(&m_Cond);

	return true;
}

std::string CLog::getfulltime(const time_t seconds)
{
	time_t second;
	if(seconds <= 0) {
		second = time(NULL);
	} else {
		second = seconds;
	}
	struct tm stTimeInfo;
	if(!localtime_r(&second, &stTimeInfo)) {
		perror("localtime_r");
		return std::string();
	}

	std::stringstream ss(std::stringstream::in | std::stringstream::out);
	ss << (stTimeInfo.tm_year + 1900) << "-";
	ss << std::setfill('0') << std::setw(2) << (stTimeInfo.tm_mon + 1) << "-";
	ss << std::setfill('0') << std::setw(2) << stTimeInfo.tm_mday << " ";
	ss << std::setfill('0') << std::setw(2) << stTimeInfo.tm_hour << ":";
	ss << std::setfill('0') << std::setw(2) << stTimeInfo.tm_min << ":";
	ss << std::setfill('0') << std::setw(2) << stTimeInfo.tm_sec;
	ss.flush();

	return ss.str();
}

std::string CLog::LogGetDate(const time_t ttime)
{
	time_t tmpTime;
	if(ttime == 0)
		tmpTime = time(0);
	else
		tmpTime = ttime;
	struct tm* timeinfo = localtime(&tmpTime);
	char dt[20];
	memset(dt, 0, 20);
	sprintf(dt, "%4d%02d%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
	//return (timeinfo->tm_year + 1900) * 10000 + (timeinfo->tm_mon + 1) * 100 + timeinfo->tm_mday;
	return std::string(dt);
}

bool CLog::LogCreatLogDir()
{
	std::vector<std::string> vecFiles;
	std::string strLogDir = std::string(m_pszPathName) + "/" + LogGetDate();

	srand((int)time(0)+987654321);
	int randomInt = rand()%100000;
	char randomchar[5];
	memset(randomchar,0,5);
	sprintf(randomchar,"%d",randomInt);
	
	if(!LogFileBeExists(strLogDir.c_str())) {
	
		m_iLogFileId = 0;
		if(!LogFileCreatDir(strLogDir.c_str())) {
			return false;
		}

		m_iLogFileLine = 0;
		if(m_pFile){
			fclose(m_pFile);
			m_pFile = NULL;
		}

	} //else //if(m_iLogFileId == 0){
	  else {  //modify by cyg 20130709
		if(!LogFileListFiles(strLogDir.c_str(), vecFiles)) {
			return false;
		}
		m_iLogFileId = vecFiles.size();
		
		//m_iLogFileLine = 0;  //modify by cyg 20130709
	}

	
	//if((m_iLogFileLine == 0 && m_pFile == NULL) || m_iLogFileLine >= m_iLimit) {
	if( m_pFile == NULL|| 0 == m_iLogFileId ) {  //modify by cyg 20130709
		if(m_pFile) {
			fclose(m_pFile);
			m_pFile = NULL;
		}
		std::string strFileName = strLogDir + "/" + std::string(m_pszFileName) + toString(m_iLogFileId)+"."+std::string(randomchar);
		m_pFile = fopen(strFileName.c_str(), "a");
		if(m_pFile == NULL) return false;
		m_iLogFileId ++;
		//m_iLogFileLine = 0;   
		m_iLogFileLine++;  //modify by cyg 20130709
	}
	return true;
}


LOG_RANK CLog::LogGetLogRank()
{
	return m_Rank;

}

void CLog::LogSetLogRank(LOG_RANK rank)
{	
	m_Rank = rank;
}

void CLog::LogPrintfLogRank()
{
	printf("m_Rank=%d\n",m_Rank);
}


CLOGFile::CLOGFile() {
	// TODO Auto-generated constructor stub

}

CLOGFile::~CLOGFile() {
	// TODO Auto-generated destructor stub
}

/*
 * be exists file or directory
 * Parameters: file path
 * Return-Value: exists return true, not exists return false
 */
bool CLOGFile::LogFileBeExists(const char* const pchPath)
{
	return !access(pchPath, 0);
}

/*
 * create directory
 * Parameters: directory path
 * Return-Value: success return true else return false
 */
bool CLOGFile::LogFileCreatDir(const char* const pchPath)
{
	std::string strDir;
	int iLen = strlen(pchPath);
	int i = 0;
	for(; i < iLen; ++i) {
		if(pchPath[i] == '/') {
			strDir = std::string(pchPath, 0, i + 1);
			if(!LogFileBeExists(strDir.c_str()) && mkdir(strDir.c_str(), 0777) != 0) {
				return false;
			}
		}
	}
	strDir = std::string(pchPath, 0, i);
	if(!LogFileBeExists(strDir.c_str()) && mkdir(strDir.c_str(), 0777) != 0) {
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
bool CLOGFile::LogFileListDirs(const char* const pchPath, std::vector<std::string>& vecDirs)
{
	if(!LogFileBeExists(pchPath)) {
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
bool CLOGFile::LogFileListFiles(const char* const pchPath, std::vector<std::string>& vecFiles)
{
	if(!LogFileBeExists(pchPath)) {
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
bool CLOGFile::LogFileReadFile(const char* const pchFileName, std::string& strContent)
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
char* CLOGFile::LogFileReadFile(const char* const pchFileName, u_long& ulLength)
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
bool CLOGFile::LogFileWriteFile(const char* const pchMode, const char* const pchFileName, const char* const pchContent, const u_long ulLength)
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
long CLOGFile::LogFileGetFileLength(const char* const pchFileName)
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
bool CLOGFile::LogFileRemoveFile(const char* const pchFileName)
{
	return (remove(pchFileName) == 0);
}




