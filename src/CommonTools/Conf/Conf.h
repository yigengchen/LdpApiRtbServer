/********************************************************/
/*	Copyright (C) 2015 AI BDX	*/
/*	Project:	BDX		*/
/*	Author:		chenyg			*/
/*	Date:		2015_01_06			*/
/*	File:		Conf.h			*/
/********************************************************/

#ifndef	_CONF_H
#define	_CONF_H

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include   <string>   
#include   <iostream>
using namespace std;

#define SUCCESS          0
#define	ERR_OEN			1
#define	ERR_SEC			2
#define	ERR_KEY			3
#define	ERR_VAL			4
#define	ERR_CLS			5
#define	ERR_OTH			6

#define	CTRL_T			'\t'
#define	CTRL_R			'\r'
#define	CTRL_N			'\n'


#define	SEC_Q			'?'
#define	SEC_M			','
#define	SEC_A			'&'
#define	SEC_L			'['
#define	SEC_R			']'

#define	BLANK			' '
#define	EQUAL			'='
#define	REMARK			'#'

#define PACKET           1024



class CConf
{
public:
	CConf();
	virtual ~CConf();

	void ConfClose();
	int  ConfOpen(char* pszFile);
	int  ConfGetSec(char* pszSec);
	void ConfTrim(char* pszLine);
	bool ConfIsValid(char* pszLine);
	bool ConfIsSection(char* pszLine);
	int  ConfGetLine(char* pszLine, int iSize);
	int  ConfGetKey(char* pszSec, char* pszKey);
	int  ConfGetVal(char* pszSec, char* pszKey, char* pszVal);
	int  ConfKeyValue(char* pszLine, char* pszKey, char* pszVal);
	int  ConfKeyValue(char* pszLine, char** ppszKey, char** ppszVal);
	string&   replace_all(string &   str,const   string&   old_value,const   string&   new_value);
	string&   replace_all_distinct(string&   str,const   string&   old_value,const   string&   new_value);

private:
	FILE* m_pstFile;

	void ConfTrimLeft(char* pszLine);
	void ConfTrimRight(char* pszLine);

};

#endif /* _CONF_H */
