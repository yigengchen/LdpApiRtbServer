/*
 * data_adapter.h
 *
 *  Created on: 2013-10-19
 *      Author: root
 *
 *      负责将数据写入到指定的服务集群中
 */

#ifndef DATA_ADAPTER_H_
#define DATA_ADAPTER_H_

#include <stdint.h>
#include <pthread.h>
#include <string>

#include "./data_redis.h"
#include "../CommonTools/Md5/Md5.h"
using std::string;

class CDataAdapter {
public:// 单例
	CDataAdapter();
	~CDataAdapter();

public:
	bool Init(const char *redis_list);
public:
	bool UserGet(string &uid,string &rule);
	bool UserIncr(string &uid,int32_t itime_expire =0);
	bool UserIncrBy(string &uid,int32_t increment);

public:
	bool UserPut(string &uid,const string &rule);
private:
	uint32_t			serv_count_;
	CDataRedis  			*data_;
	pthread_mutex_t		*lock_;
	CMd5 m_clMd5;
};

#endif /* DATA_ADAPTER_H_ */
