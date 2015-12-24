/*
 * update_adapter.cpp
 *
 *  Created on: 2013-10-23
 *      Author: root
 */

#include "update_adapter.h"
#include <assert.h>

CUpdateAdapter::CUpdateAdapter() :serv_count_(0),data_(NULL),lock_(NULL){

}

CUpdateAdapter::~CUpdateAdapter() {
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

bool CUpdateAdapter::Init(const char *mq_srv_list) {
	deque<string> hosts;
	deque<uint16_t> ports,idxs;
	if(! redis_serv_list(mq_srv_list,hosts,ports,idxs)) {
		return false;
	}
	serv_count_ = hosts.size();
	data_ = new CDataRedis[serv_count_];
	lock_ = new pthread_mutex_t[serv_count_];
	for(uint32_t i=0; i<serv_count_; ++i) {
		const char *host = hosts[i].c_str();
		uint16_t port = ports[i];
		uint16_t idx = idxs[i];
		printf("[%u]connect[%s:%u,%u].[%s:%d]",i,host,port,idx,__FILE__,__LINE__);
		if(! data_[i].Init(host,port,idx)) {
			printf("ERROR connect failed.[%s:%d]",__FILE__,__LINE__);
			return false;
		}
		pthread_mutex_init(& lock_[i],NULL);
	}
	return true;
}

bool CUpdateAdapter::TryGetPop(const char *mq_key, set<string> &uids) {
	// 读取个数
	uint32_t item_count=0;
	printf("server count: %u, key: %s.\n",serv_count_,mq_key);
	for(uint32_t k=0; k<serv_count_; ++k) {
		pthread_mutex_lock(& lock_[k]);
		if(! data_[k].Llen(mq_key,item_count)) {
			pthread_mutex_unlock(& lock_[k]);
			printf("ERROR Get list count failed.[%s:%d]",__FILE__,__LINE__);
			return false;
		}
		printf("count: %u.\n",item_count);
		for(uint32_t i=0; i<item_count && i<3; ++i) {
			string item;
			data_[k].Lpop(mq_key,item);
			//printf("item[%d] item: %s.\n",i,item.c_str());
			uids.insert(set<string>::value_type(item));
		}
		pthread_mutex_unlock(& lock_[k]);
	}
	return true;
}
