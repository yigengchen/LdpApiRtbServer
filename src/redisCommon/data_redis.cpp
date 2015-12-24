/*
 * data_redis.cpp
 *
 *  Created on: 2013-10-19
 *      Author: lixiyong
 */

#include "data_redis.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


bool redis_serv_list(const char *serv_list_str, deque<string> &hosts,deque<uint16_t> &ports,deque<uint16_t> &idxs) {
	const char *token,*curr = serv_list_str;
	uint32_t count = atoi(curr);
	token = strchr(curr,';');
	curr = token + 1;
	//printf("curr=%s  count=%d\n",curr,count);
	for(uint32_t i=0; i<count; ++i) {
		char serv[256] = {0};
		token = strchr(curr,';');
		strncpy(serv,curr,token-curr);
		// host:port,idx parse
		const char *port,*idx,*tmp = serv;
		port = strchr(tmp,':');
		if(port==NULL) return false;
		string host(tmp,port-tmp);
		hosts.push_back(host);
		ports.push_back(atoi(port+1));
		idx = strchr(port,',');
		if(idx==NULL) {
			idxs.push_back(0);
		}else {
			idxs.push_back(atoi(idx+1));
		}
		curr = token + 1;
	}
	return true;
}

CDataRedis::CDataRedis() :port_(0),context_(NULL) {
	memset(host_,0,256);
}

CDataRedis::~CDataRedis() {
	if(context_!=NULL) {
		redisFree(context_);
		context_ = NULL;
	}
}
bool CDataRedis::Init(const char *host, unsigned short port, uint16_t idx) {

	strncpy(host_,host,256);
	port_ = port;
	idx_ = idx;
	if(!KeepConnect(0)) {
		return false;
	}
	return true;
}

bool CDataRedis::KeepConnect(uint32_t timeout) {
	if (context_!=NULL) return true;
	struct timeval st_timeout = {0, 800000}; // 1.5 seconds
	if(timeout) st_timeout.tv_usec = timeout;
	context_ = redisConnectWithTimeout(host_, port_, st_timeout);
	if (context_ == NULL || context_->err) {
		if (context_) {
			printf("Connection[%s:%u] error: %s.\n",host_, port_,context_->errstr);
			redisFree(context_);
			context_ = NULL;
		} else {
			printf("Connection error: can't allocate redis context.\n");
		}
		return false;
	}
	if(idx_!=0 && !SelectDB(idx_)) {
		printf("Select db[%u] failed.\n",idx_);
		return false;
	}
	//test, connect redis_server
	redisReply *reply = (redisReply*)redisCommand(context_, "PING");
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if(reply) {
			printf("PING redis_server, but error occur, errInfo: %s.\n",reply->str);
			freeReplyObject(reply);
		}
		DisConnect();
		return false;
	}
	freeReplyObject(reply);
	reply = NULL;
	return true;
}
void CDataRedis::DisConnect() {
	if(context_!=NULL) {
		redisFree(context_);
		context_ = NULL;
	}
}

bool CDataRedis::DeleteKey(const char *key) {
	assert(NULL!=key);
	if(!KeepConnect()) return false;

	char cmd[1024] = {0};
	snprintf(cmd,1023,"del %s", key);

	redisReply *reply = (redisReply*)redisCommand(context_, cmd);
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if(reply) {
			printf("fail to delete key, delCommand: %s, error: %s.\n",cmd,reply->str);
			freeReplyObject(reply);
		}
		DisConnect();
		return false;
	}
	freeReplyObject(reply);
	reply = NULL;

	return true;
}

bool CDataRedis::DeleteKeys(deque<string> &keys) {
	uint32_t size = keys.size();
	assert(size>0);
	if(!KeepConnect()) return false;

	char cmd[4096] = {0};
	snprintf(cmd,4096,"del");
	for(uint32_t i = 0; i < size; i++) {
		strcat(cmd, " ");
		strcat(cmd, keys[i].c_str());
	}

	redisReply *reply = (redisReply*)redisCommand(context_, cmd);
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if(reply) {
			printf("fail to delete keys, delCommand: %s, error: %s.\n", cmd,reply->str);
			freeReplyObject(reply);
		}
		DisConnect();
		return false;
	}
	freeReplyObject(reply);
	reply = NULL;

	return true;
}


bool CDataRedis::GetKeys(const char *regular_key, deque<string> &keys) {
	assert(NULL!=regular_key);
	if(!KeepConnect()) return false;

	redisReply *reply = (redisReply*)redisCommand(context_, "keys %s", regular_key);
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if (reply) {
			printf("Fail to get keys, regular key: %s, error: %s.\n",regular_key,reply->str);
			freeReplyObject(reply);
		}
		DisConnect();
		return false;
	}else {
		size_t count = reply->elements;
		for(size_t i = 0; i < count; i++) {
			keys.push_back(reply->element[i]->str);
		}
	}
	freeReplyObject(reply);
	reply = NULL;

	return true;
}

bool CDataRedis::GetValue(const char *key, string &value) {
	assert(NULL!=key);
	if(!KeepConnect()) return false;
	//printf("key=%s\n",key);
	redisReply *reply = (redisReply*)redisCommand(context_, "GET %s", key);
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR||reply->len==0) {
		if(reply) {
			//printf("fail to get value, key: %s, error: %s.\n",key,reply->str);
			freeReplyObject(reply);
		}
		DisConnect();//chenyg add 20150714
		return false;
	}else {
		//printf("reply->str=%s\n",reply->str);
		//printf("reply->len=%d\n",reply->len);
		value.assign(reply->str,reply->len);
	}
	freeReplyObject(reply);
	reply = NULL;
	return true;
}
bool CDataRedis::Rpush(const char *key,const char *item) {
	assert(key);
	assert(item);
	if(!KeepConnect()) return false;

	redisReply *reply = (redisReply*)redisCommand(context_,"rpush %s %s", key, item);
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if(reply) {
			printf("Fail to set value, key: %s, value: %s, error: %s.\n",key,item,reply->str);
			freeReplyObject(reply);
		}
		DisConnect();
		return false;
	}
	freeReplyObject(reply);
	reply = NULL;
	return true;
}
//返回key对应list的长度
bool CDataRedis::Llen(const char *key, uint32_t &len) {
	assert(NULL!=key);
	if(!KeepConnect()) return false;

	redisReply *reply = (redisReply*)redisCommand(context_, "llen %s", key);
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if(reply) {
			printf("fail to get value, key: %s, error: %s.\n",key,reply->str);
			freeReplyObject(reply);
		}
		DisConnect();
		return false;
	}else {
		len = reply->integer;
		/*
		string value;
		value.assign(reply->str,reply->len);
		len = atoi(value.c_str());*/
	}
	freeReplyObject(reply);
	reply = NULL;
	return true;
}
//从list的头部删除元素，并返回删除元素
bool CDataRedis::Lpop(const char *key, string &item) {
	assert(NULL!=key);
	if(!KeepConnect()) return false;

	redisReply *reply = (redisReply*)redisCommand(context_, "lpop %s", key);
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if (reply) {
			printf("fail to get value, key: %s, error: %s.\n",key,reply->str);
			freeReplyObject(reply);
		}
		DisConnect();
		return false;
	}else {
		item.assign(reply->str,reply->len);
	}
	freeReplyObject(reply);
	reply = NULL;
	return true;
}


bool CDataRedis::SelectDB(uint16_t db_index) {
	if(!KeepConnect()) return false;

	redisReply *reply = (redisReply*)redisCommand(context_,"select %d", db_index);
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if (reply) {
			printf("Fail to select db from redis, db_id: %u, error: %s.\n",db_index,reply->str);
			freeReplyObject(reply);
		}
		DisConnect();
		return false;
	}
	freeReplyObject(reply);
	reply = NULL;

	return true;
}

bool CDataRedis::SetValue(const char* key, const char *value, int32_t expiration) {
	assert(key);
	assert(value);
	if(!KeepConnect()) return false;

	redisReply *reply = NULL;
	if (expiration > 0) {
		reply = (redisReply*)redisCommand(context_,"SET %s %s ex %d", key, value, expiration);
	} else {
		reply = (redisReply*)redisCommand(context_,"SET %s %s", key, value);
	}
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if (reply) {
			printf("Fail to set value, key: %s, value: %s, error: %s.\n",key,value,reply->str);
			freeReplyObject(reply);
		}
		DisConnect();
		return false;
	}

	freeReplyObject(reply);
	reply = NULL;
	return true;
}


bool CDataRedis::Sadd(const char *key, const char *mumb) {
	if(!KeepConnect()) return false;

	char cmd[4096] = {0};
	snprintf(cmd,4096,"sadd %s %s",key,mumb);

	redisReply *reply = (redisReply*)redisCommand(context_, cmd);
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if (reply) {
			printf("Sadd command: %s, error: %s.\n", cmd,reply->str);
			freeReplyObject(reply);
		}
		DisConnect();
		return false;
	}
	freeReplyObject(reply);
	reply = NULL;
	return true;
}

bool CDataRedis::Sadd(const char *key, const set<string> &mumbs) {
	uint32_t size = mumbs.size();
	assert(size>0);
	if(!KeepConnect()) return false;

	char cmd[4096] = {0};
	snprintf(cmd,4096,"sadd %s",key);
	set<string>::const_iterator iter,end=mumbs.end();
	for(iter=mumbs.begin(); iter!=end; ++iter) {
		strcat(cmd, " ");
		strcat(cmd, (*iter).c_str());
	}

	redisReply *reply = (redisReply*)redisCommand(context_, cmd);
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if (reply) {
			printf("Sadd command: %s, error: %s.\n", cmd,reply->str);
			freeReplyObject(reply);
		}
		DisConnect();
		return false;
	}
	freeReplyObject(reply);
	reply = NULL;
	return true;
}
bool CDataRedis::Smembers(const char *key, deque<string> &membs) {
	assert(NULL!=key);
	if(!KeepConnect()) return false;

	redisReply *reply = (redisReply*)redisCommand(context_, "smembers %s", key);
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if (reply) {
			printf("Smembers key: %s, error: %s.\n",key,reply->str);
			freeReplyObject(reply);
		}
		DisConnect();
		return false;
	}else {
		size_t count = reply->elements;
		for(size_t i = 0; i < count; i++) {
			membs.push_back(reply->element[i]->str);
		}
	}
	freeReplyObject(reply);
	reply = NULL;
	return true;
}


bool CDataRedis::Sremove(const char *key,const char *mumb) {

	if(!KeepConnect()) return false;

	char cmd[4096] = {0};
	snprintf(cmd,4096,"srem %s %s",key,mumb);

	redisReply *reply = (redisReply*)redisCommand(context_, cmd);
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if (reply) {
			printf("Srem command: %s, error: %s.\n", cmd,reply->str);
			freeReplyObject(reply);
		}
		DisConnect();
		return false;
	}
	freeReplyObject(reply);
	reply = NULL;
	return true;
}

bool CDataRedis::Sremove(const char *key,const set<string> &mumbs) {
	uint32_t size = mumbs.size();
	assert(size>0);
	if(!KeepConnect()) return false;

	char cmd[4096] = {0};
	snprintf(cmd,4096,"srem %s",key);
	set<string>::const_iterator iter,end=mumbs.end();
	for(iter=mumbs.begin(); iter!=end; ++iter) {
		strcat(cmd, " ");
		strcat(cmd, (*iter).c_str());
	}
	redisReply *reply = (redisReply*)redisCommand(context_, cmd);
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if (reply) {
			printf("Srem command: %s, error: %s.\n", cmd,reply->str);
			freeReplyObject(reply);
		}
		DisConnect();
		return false;
	}
	freeReplyObject(reply);
	reply = NULL;
	return true;
}


bool CDataRedis::Hset(const char *key, const char *field, const char *value) {

	if(!KeepConnect()) return false;

	char cmd[4096] = {0};
	snprintf(cmd,4096,"hset %s %s %s",key,field,value);

	printf("%s.\n",cmd);
	redisReply *reply = (redisReply*)redisCommand(context_, cmd);
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if (reply) {
			printf("Hset command: %s, error: %s.\n", cmd,reply->str);
			freeReplyObject(reply);
		}
		DisConnect();
		return false;
	}
	freeReplyObject(reply);
	reply = NULL;
	return true;
}
bool CDataRedis::Hget(const char *key, const char *field, string &value) {

	assert(NULL!=key);
	if(!KeepConnect()) return false;

	redisReply *reply = (redisReply*)redisCommand(context_, "hget %s %s", key, field);
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if(reply) {
			printf("fail to get value, key: %s, error: %s.\n",key,reply->str);
			freeReplyObject(reply);
		}
		DisConnect();
		return false;
	}else {
		value.assign(reply->str,reply->len);
	}
	freeReplyObject(reply);
	reply = NULL;
	return true;
}
bool CDataRedis::Hkeys(const char *key,deque<string> &fields) {
	assert(NULL!=key);
	if(!KeepConnect()) return false;

	redisReply *reply = (redisReply*)redisCommand(context_, "hkeys %s", key);
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if(reply) {
			printf("fail to get value, key: %s, error: %s.\n",key,reply->str);
			freeReplyObject(reply);
		}
		DisConnect();
		return false;
	}else {
		size_t count = reply->elements;
		for(size_t i = 0; i < count; i++) {
			fields.push_back(reply->element[i]->str);
		}
	}
	freeReplyObject(reply);
	reply = NULL;
	return true;
}
bool CDataRedis::Hvals(const char *key,deque<string> &values) {
	assert(NULL!=key);
	if(!KeepConnect()) return false;

	redisReply *reply = (redisReply*)redisCommand(context_, "hvals %s", key);
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if(reply) {
			printf("fail to get value, key: %s, error: %s.\n",key,reply->str);
			freeReplyObject(reply);
		}
		DisConnect();
		return false;
	}else {
		size_t count = reply->elements;
		for(size_t i = 0; i < count; i++) {
			values.push_back(reply->element[i]->str);
		}
	}
	freeReplyObject(reply);
	reply = NULL;
	return true;
}

bool CDataRedis::IncrValue(const char* key,int32_t expiration) {
	assert(key);
	
	if(!KeepConnect()) return false;
	//printf("expiration=%d\n",expiration);
	redisReply *reply = NULL;
	if (expiration > 0) {
		reply = (redisReply*)redisCommand(context_,"incr %s  %d", key, expiration);
	} else {
		reply = (redisReply*)redisCommand(context_,"incr %s", key);
	}
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if (reply) {
			//printf("Fail to incr value, key: %s, error: %s.\n",key,reply->str);
			freeReplyObject(reply);
		}
		DisConnect(); //chenyg add 20150714
		return false;
	}

	freeReplyObject(reply);
	reply = NULL;
	return true;
}

bool CDataRedis::IncrByValue(const char* key,int32_t expiration) {
	assert(key);
	
	if(!KeepConnect()) return false;
	//printf("expiration=%d\n",expiration);
	redisReply *reply = NULL;
	if (expiration > 0) {
		reply = (redisReply*)redisCommand(context_,"incrby %s  %d", key, expiration);
	} else {
		reply = (redisReply*)redisCommand(context_,"incr %s", key);
	}
	if (reply==NULL || reply->type == REDIS_REPLY_ERROR) {
		if (reply) {
			//printf("Fail to incr value, key: %s, error: %s.\n",key,reply->str);
			freeReplyObject(reply);
		}
		DisConnect(); //chenyg add 20150714
		return false;
	}

	freeReplyObject(reply);
	reply = NULL;
	return true;
}

