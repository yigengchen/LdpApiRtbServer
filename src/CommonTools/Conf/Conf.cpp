/********************************************************/

/********************************************************/

#include "Conf.h"

#include <errno.h>

CConf::CConf()
{
	m_pstFile = NULL;
}

CConf::~CConf()
{
}

void CConf::ConfClose()
{
	if (NULL != m_pstFile) {
		fclose(m_pstFile);
		m_pstFile = NULL;
	}

	return;
}

int CConf::ConfOpen(char* pszFile)
{
	int iReturn = SUCCESS;

	if (NULL == (m_pstFile = fopen(pszFile, "r"))) {
		fprintf(stderr, "%s\n", strerror(errno));
		iReturn = ERR_OEN;
	}

	return iReturn;
}

int CConf::ConfGetLine(char* pszLine, int iSize)
{
	int iReturn = SUCCESS;

	if (NULL != m_pstFile) {
		if (NULL == fgets(pszLine, iSize, m_pstFile)) {
			iReturn = ERR_OTH;
		}
	}
	else {
		iReturn = ERR_OEN;
	}

	return iReturn;
}

int CConf::ConfGetSec(char* pszSec)
{
	int iReturn = ERR_SEC;
	char szLine[PACKET] = { 0 };

	if (NULL != m_pstFile) {
		rewind(m_pstFile);
		while (SUCCESS == ConfGetLine(szLine, PACKET - 1)) {
			if (true != ConfIsValid(szLine)) continue;
			if (true == ConfIsSection(szLine)) {
				if (SUCCESS == strncasecmp(&szLine[1], pszSec, strlen(pszSec))) {
					iReturn = SUCCESS;
					break;
				}
			}
			memset(szLine, '\0', PACKET);
		}
	}
	else {
		iReturn = ERR_OEN;
	}

	return iReturn;
}

int CConf::ConfGetKey(char* pszSec, char* pszKey)
{
	bool bSec = false;
	int iReturn = ERR_SEC;
	char szLine[PACKET] = { 0 };

	if (NULL != m_pstFile) {
		rewind(m_pstFile);
		while (SUCCESS == ConfGetLine(szLine, PACKET - 1)) {
			if (true != ConfIsValid(szLine)) continue;
			if (true == ConfIsSection(szLine)) {
				if (false == bSec) {
					if (SUCCESS == strncasecmp(&szLine[1], pszSec, strlen(pszSec))) {
						iReturn = ERR_KEY;
						bSec = true;
					}
				}
				else {
					bSec = false;
				}
			}
			else {
				if (true == bSec) {
					if (SUCCESS == strncasecmp(szLine, pszKey, strlen(pszKey))) {
						iReturn = SUCCESS;
						break;
					}
				}
			}
			memset(szLine, '\0', PACKET);
		}
	}
	else {
		iReturn = ERR_OEN;
	}

	return iReturn;
}

int CConf::ConfGetVal(char* pszSec, char* pszKey, char* pszVal)
{
	bool bSec = false;
	int iReturn = ERR_SEC;
	char szLine[PACKET] = { 0 };

	if (NULL != m_pstFile) {
		rewind(m_pstFile);
		while (SUCCESS == ConfGetLine(szLine, PACKET - 1)) {
			if (true != ConfIsValid(szLine)) continue;
			if (true == ConfIsSection(szLine)) {
				if (false == bSec) {
					if (SUCCESS == strncasecmp(&szLine[1], pszSec, strlen(pszSec))) {
						iReturn = ERR_KEY;
						bSec = true;
					}
				}
				else if (true == bSec) {
					bSec = false;
				}
				else {
				}
			}
			else {
				if (true == bSec) {
					if (SUCCESS == strncasecmp(szLine, pszKey, strlen(pszKey))) {
						iReturn = ERR_VAL;
						ConfTrimRight(szLine);
						if (SUCCESS == ConfKeyValue(szLine, pszKey, pszVal)) {
							iReturn = SUCCESS;
						}
						break;
					}
				}
			}
			memset(szLine, '\0', PACKET);
		}
	}
	else {
		iReturn = ERR_OEN;
	}

	return iReturn;
}

int CConf::ConfKeyValue(char* pszLine, char* pszKey, char* pszVal)
{
	char* pszCur = pszLine;
	int iReturn = SUCCESS;
	int iLength = strlen(pszLine);

	while (pszCur - pszLine < iLength && EQUAL != *pszCur) pszCur += 1;

	if (pszCur - pszLine != iLength) {
		*pszCur ++ = '\0';
		ConfTrimLeft(pszCur);
		strcpy(pszVal, pszCur);
	}
	else {
		iReturn = ERR_VAL;
	}

	return iReturn;
}

int CConf::ConfKeyValue(char* pszLine, char** ppszKey, char** ppszVal)
{
	char* pszCur = pszLine;
	int iReturn = SUCCESS;
	int iLength = strlen(pszLine);
	
	while (pszCur - pszLine < iLength && EQUAL != *pszCur) pszCur += 1;

	if (pszCur - pszLine != iLength) {
		*pszCur ++ = '\0';
		ConfTrimRight(pszLine);
		ConfTrimLeft(pszCur);
		*ppszKey = pszLine;
		*ppszVal = pszCur;
	}
	else {
		iReturn = ERR_VAL;
	}

	return iReturn;
}

bool CConf::ConfIsValid(char* pszLine)
{
	if (REMARK != *pszLine) {
		ConfTrimLeft(pszLine);
		if (0 == strlen(pszLine)) {
			return false;
		}
	}
	else {
		return false;
	}

	return true;
}

bool CConf::ConfIsSection(char* pszLine)
{
	if (SEC_L != *pszLine) {
		return false;
	}

	return true;
}

void CConf::ConfTrim(char* pszLine)
{
	ConfTrimLeft(pszLine);
	ConfTrimRight(pszLine);

	return;
}

void CConf::ConfTrimLeft(char* pszLine)
{
	char* pszCur = pszLine;

	while (CTRL_T == *pszCur || CTRL_R == *pszCur || CTRL_N == *pszCur || BLANK == *pszCur) pszCur += 1;
	while ('\0' != (*pszLine ++ = *pszCur ++)) {};

	return;
}

void CConf::ConfTrimRight(char* pszLine)
{
	char* pszCur = pszLine;

	if (strlen(pszLine) > 0) {
		while ('\0' != *pszCur && REMARK != *pszCur) pszCur += 1;
		*pszCur -- = '\0';
		while (CTRL_T == *pszCur || CTRL_R == *pszCur || CTRL_N == *pszCur || BLANK == *pszCur) pszCur -= 1;
		*(pszCur + 1) = '\0';
	}

	return;
}
string&   CConf::replace_all(string &   str,const   string&   old_value,const   string&   new_value)   
{   
    while(true)   {   
        string::size_type   pos(0);   
        if(   (pos=str.find(old_value))!=string::npos   )   
            	str.replace(pos,old_value.length(),new_value);   
        else   break;   
    }   
    return   str;   
}   
string&   CConf::replace_all_distinct(string&   str,const   string&   old_value,const   string&   new_value)   
{   
    for(string::size_type   pos(0);   pos!=string::npos;   pos+=new_value.length())   {   
        if(   (pos=str.find(old_value,pos))!=string::npos   )   
            str.replace(pos,old_value.length(),new_value);   
        else   
        	break;   
    }   
    return   str;   
}   