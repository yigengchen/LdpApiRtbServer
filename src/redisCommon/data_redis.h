/*
 * data_redis.h
 *
 *  Created on: 2013-10-19
 *      Author: root
 */

#ifndef DATA_REDIS_H_
#define DATA_REDIS_H_

#include "../../include/hiredis.h"
#include <stdint.h>

#include <set>
#include <deque>
#include <string>

using std::set;
using std::deque;
using std::string;

// redis_server_list=4;172.16.8.112:6379,0;172.16.8.112:6379,1;172.16.8.113:6379;
// 无","默认为db_idx=0
bool redis_serv_list(const char *serv_list_str, deque<string> &hosts,deque<uint16_t> &ports,deque<uint16_t> &idxs);
class CDataRedis {
public:
	CDataRedis();
	~CDataRedis();

public:
	bool Init(const char *host, unsigned short port, uint16_t idx=0);
	bool SelectDB(uint16_t db_index);

	// string operator
	bool SetValue(const char *key, const char *value, int32_t expiration=0);
	bool GetValue(const char *key, string &value);
	bool IncrByValue(const char* key,int32_t expiration);

	// list operator
	bool Rpush(const char *key,const char *item);//在key对应list的尾部添加字符串元素
	bool Rpush(const char *key,deque<string> &items);//在key对应list的尾部添加字符串元素
	bool Llen(const char *key, uint32_t &len);//返回key对应list的长度
	bool Lpop(const char *key, string &item);//从list的头部删除元素，并返回删除元素

	// set operator
	bool Sadd(const char *key, const char *mumb);
	bool Sadd(const char *key, const set<string> &mumbs);
	bool Smembers(const char *key, deque<string> &membs);
	bool Sremove(const char *key,const char *mumb);
	bool Sremove(const char *key,const set<string> &mumbs);

	// hash operator
	bool Hset(const char *key, const char *field, const char *value);
	bool Hget(const char *key, const char *field, string &value);
	bool Hkeys(const char *key,deque<string> &fields);
	bool Hvals(const char *key,deque<string> &values);
	bool IncrValue(const char* key, int32_t expiration=0);

	// key operator
	bool GetKeys(const char *regular_key, deque<string> &keys);
	bool DeleteKeys(deque<string> &keys);
	bool DeleteKey(const char *key);

private:
	bool KeepConnect(uint32_t timeout=0);//usec
	void DisConnect();
private:
	CDataRedis(const CDataRedis&);
	CDataRedis& operator=(const CDataRedis&);

private:
	char 				host_[256];
	unsigned short		port_;
	uint16_t			idx_;
	redisContext 		*context_;		 //redisClient context
};

#endif /* DATE_REDIS_H_ */
