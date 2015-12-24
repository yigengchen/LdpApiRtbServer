/*
 * update_adapter.h
 *
 *  Created on: 2013-10-23
 *      Author: root
 */

#ifndef UPDATE_ADAPTER_H_
#define UPDATE_ADAPTER_H_

#include <pthread.h>
#include <set>
#include <deque>
#include <string>

#include "./data_redis.h"
using std::set;
using std::deque;
using std::string;

// 从全站回头客的更新队列中读取数据
class CUpdateAdapter {
public:
	CUpdateAdapter();
	~CUpdateAdapter();
public:
	bool Init(const char *mq_srv_list);

public:// 用户更新(A)
	// Key(HtkChangedUserList) value(list{uid[0x01]mid})
	bool TryGetPop(const char *mq_key,set<string> &uids);

private:
	uint32_t			serv_count_;
	CDataRedis  			*data_;
	pthread_mutex_t		*lock_;
};

#endif /* UPDATE_ADAPTER_H_ */
