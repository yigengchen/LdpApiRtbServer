/*

 */

#include "UserQueryCount.h"

CUserQueryCount::CUserQueryCount(const STATISTICSPRM_S& stStatisticsPrm)
 : m_stStatisticsPrm(stStatisticsPrm)
{	
	struct stat statbuf;
	// TODO Auto-generated constructor stub
	std::string statusDir= m_stStatisticsPrm.m_strStatisticsPath+"/"+BdxUserQueryCountGetDate();
	if(!m_clFile.FileBeExists(statusDir.c_str())) {
		m_clFile.FileCreatDir(statusDir.c_str());
	}
	std::string strFileName = statusDir + "/"+m_stStatisticsPrm.m_strStatisticsFileName+"_"+ BdxUserQueryCountGetDate() + ".txt";
	m_pFile = fopen(strFileName.c_str(), "a");
	if (!stat(strFileName.c_str(),&statbuf))
	{
		if( statbuf.st_size == 0 )
		{
			BdxQueryCountWriteTitle();
		}

	}
	for(u_int i = 0; i < CUserQueryWorkThreads::m_vecReport.size(); ++i) 
	{
		m_stReport += CUserQueryWorkThreads::m_vecReport[i];
		CUserQueryWorkThreads::m_vecReport[i] = 0;
	}
	//memset(&m_stReport,0,sizeof(UESRQUERYRPORT_S));

}

CUserQueryCount::~CUserQueryCount() {
	// TODO Auto-generated destructor stub
}
void CUserQueryCount::Core()
{
	
	while(true) {
		
		std::map<std::string,USERINFO_S>::iterator itr;
		BdxQueryCountOpenFile();
		BdxQueryCountGetReport();
        time_t timep;
        time(&timep);
        struct tm* timeinfo = localtime(&timep);
        //fprintf(m_pFile, "%04d-%02d-%02d %02d:%02d:%02d\t", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        
		//printf("sleeping \\\\\\\\\\\\\\\\\\\\\\\\\\\n");
		//printf("m_stReport.m_strUserInfo_size=%d\n",m_stReport.m_strUserInfo.size());
		for(itr = m_stReport.m_strUserInfo.begin();itr!= m_stReport.m_strUserInfo.end();itr++)
		{
			fprintf(m_pFile, "%04d-%02d-%02d %02d:%02d\t", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min);
			fprintf(m_pFile,"%s\t%llu\t%llu\t%llu\t%llu\t%llu\n",itr->first.c_str(),itr->second.m_ullReqNum,itr->second.m_ullResNum,itr->second.m_ullEmptyResNum,itr->second.m_ullResTagNum,itr->second.m_ullResErrorNum);
		}
		
		fflush(m_pFile);
		m_stReport = 0;
        sleep(m_stStatisticsPrm.m_uiStatisticsTime);
	}
}

std::string CUserQueryCount::BdxUserQueryCountGetDate(const time_t ttime)
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

void CUserQueryCount::BdxQueryCountOpenFile()
{
	struct stat statbuf;
	std::string strFileName = 
	m_stStatisticsPrm.m_strStatisticsPath+"/"+BdxUserQueryCountGetDate()+ "/" + m_stStatisticsPrm.m_strStatisticsFileName +"_"+ BdxUserQueryCountGetDate() + ".txt";
	if(!m_clFile.FileBeExists(strFileName.c_str())) {
		if(m_pFile){
			fclose(m_pFile);
			m_pFile = NULL;
		}
		m_pFile = fopen(strFileName.c_str(), "a");
		if (!stat(strFileName.c_str(),&statbuf))
		{
			if( statbuf.st_size == 0 )
			{
				BdxQueryCountWriteTitle();
			}
			
		}
/*
		for(u_int i = 0; i < CUserQueryWorkThreads::m_vecReport.size(); ++i) 
		{
			CUserQueryWorkThreads::m_vecReport[i] = 0;
		}
*/
	}
}

void CUserQueryCount::BdxQueryCountWriteTitle()
{
	printf("BdxQueryCountWriteTitle.....\n");
	std::string strTitle = std::string("DateTime\t") + "UserName\t" + "QueryReq\t" +"QueryRes\t" +"QueryEmptyRes\t"+ "QueryResTag\t\n";
	fprintf(m_pFile, "%s", strTitle.c_str());
	fflush(m_pFile);
}

void CUserQueryCount::BdxQueryCountGetReport()
{
	//memset(&m_stReport.m_strUserInfo,0,sizeof(std::map<std::string,USERINFO_S>));
	//memset(&m_stReport, 0, sizeof(UESRQUERYRPORT_S));
	for(u_int i = 0; i < CUserQueryWorkThreads::m_vecReport.size(); ++i) {
		m_stReport += CUserQueryWorkThreads::m_vecReport[i];
		CUserQueryWorkThreads::m_vecReport[i] = 0;
	}
}
