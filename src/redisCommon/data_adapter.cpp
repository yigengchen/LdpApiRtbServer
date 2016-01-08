/*
 * data_adapter.cpp
 *
 *  Created on: 2013-10-19
 *      Author: root
 */

#include "data_adapter.h"


CDataAdapter::CDataAdapter():serv_count_(0),data_(NULL),lock_(NULL) {

}

CDataAdapter::~CDataAdapter() {
	for(uint32_t i=0; i<serv_count_; ++i) {
		pthread_mutex_destroy(&lock_[i]);
	}
	if(lock_!=NULL) {
		delete []lock_;
		lock_ = NULL;
	}
	if(data_!=NULL) {
		delete []data_;
		data_ = NULL;
	}
}

bool CDataAdapter::Init(const char *redis_list) {
	deque<string> hosts;
	deque<uint16_t> ports,idxs;
	if(! redis_serv_list(redis_list,hosts,ports,idxs)) {
		printf("ERROR Parse server list failed.[%s:%d]\n",__FILE__,__LINE__);
		return false;
	}

	serv_count_ = hosts.size();
	data_ = new CDataRedis[serv_count_];
	lock_ = new pthread_mutex_t[serv_count_];
	for(uint32_t i=0; i<serv_count_; ++i) {
		const char *host = hosts[i].c_str();
		uint16_t port = ports[i];
		uint16_t idx = idxs[i];
		
		if(!data_[i].Init(host,port,idx)) {
			printf("ERROR connect failed.[%s:%d]\n",__FILE__,__LINE__);
			
			return false;
		}
		//LOG(DEBUG,"[%u]connect[%s:%u,%u].[%s:%d]\n",i,host,port,idx,__FILE__,__LINE__);
		printf("DEBUG [%u]connect[%s:%u,%u].[%s:%d]\n",i,host,port,idx,__FILE__,__LINE__);
		pthread_mutex_init(& lock_[i],NULL);
	}
	return true;
}
bool CDataAdapter::UserGet(string &uid,string &rule) {
	//uint64_t postkey = m_clMd5.md5_sum((unsigned char *)(uid.c_str()),uid.length());
	
	m_clMd5.Md5Init();    
	m_clMd5.Md5Update((unsigned char *)uid.c_str(),uid.length());
	unsigned char  pszParamSign[16];    
	m_clMd5.Md5Final(pszParamSign);
	unsigned long int postkey = *(unsigned long int*)pszParamSign;
	
	uint32_t idx = postkey % serv_count_;

	MutexLock lock(&lock_[idx]);

	return data_[idx].GetValue(uid.c_str(),rule);
}

bool CDataAdapter::UserIncr(string &uid,int32_t itime_expire) {
	//uint64_t postkey = m_clMd5.md5_sum((unsigned char *)(uid.c_str()),uid.length());
	
	m_clMd5.Md5Init();    
	m_clMd5.Md5Update((unsigned char *)uid.c_str(),uid.length());
	unsigned char  pszParamSign[16];    
	m_clMd5.Md5Final(pszParamSign);
	unsigned long int postkey = *(unsigned long int*)pszParamSign;
	
	uint32_t idx = postkey % serv_count_;

	MutexLock lock(&lock_[idx]);

	return data_[idx].IncrValue(uid.c_str(),itime_expire);
}

bool CDataAdapter::UserIncrBy(string &uid,int32_t increment) {
	//uint64_t postkey = m_clMd5.md5_sum((unsigned char *)(uid.c_str()),uid.length());
	
	m_clMd5.Md5Init();    
	m_clMd5.Md5Update((unsigned char *)uid.c_str(),uid.length());
	unsigned char  pszParamSign[16];    
	m_clMd5.Md5Final(pszParamSign);
	unsigned long int postkey = *(unsigned long int*)pszParamSign;
	
	uint32_t idx = postkey % serv_count_;

	MutexLock lock(&lock_[idx]);

	return data_[idx].IncrByValue(uid.c_str(),increment);
}

bool CDataAdapter::UserPut(string &uid,const string &rule) {
	//uint64_t postkey = md5_sum((uid.c_str()),uid.length());
	m_clMd5.Md5Init();    
	m_clMd5.Md5Update((unsigned char *)uid.c_str(),uid.length());
	unsigned char  pszParamSign[16];    
	m_clMd5.Md5Final(pszParamSign);
	unsigned long int postkey = *(unsigned long int*)pszParamSign;

	
	uint32_t idx = postkey % serv_count_;
	MutexLock lock(&lock_[idx]);
	return data_[idx].SetValue(uid.c_str(),rule.c_str(),-1);
}


