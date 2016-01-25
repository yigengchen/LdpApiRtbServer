#!/bin/bash

#替换redis信息

sed -i 's/REDIS_SERVER_IP/'$REDIS_PORT_6379_TCP_ADDR'/g'  ./config/userQuery.conf
sed -i 's/REDIS_SERVER_PORT/'$REDIS_PORT_6379_TCP_PORT'/g'  ./config/userQuery.conf

#替换mysql信息
sed -i 's/MYSQL_SERVER_IP/'$MYSQL_PORT_3306_TCP_ADDR'/g'  ./config/userQuery.conf
sed -i 's/MYSQL_SERVER_PORT/'$MYSQL_PORT_3306_TCP_PORT'/g'  ./config/userQuery.conf
sed -i 's/MYSQL_SERVER_USERNAME/'$MYSQL_USER'/g'  ./config/userQuery.conf
sed -i 's/MYSQL_SERVER_PASSWORD/'$MYSQL_PASSWORD'/g'  ./config/userQuery.conf
sed -i 's/MYSQL_SERVER_DBNAME/'$MYSQL_DATABASE'/g'  ./config/userQuery.conf
sed -i 's/EXPOSE_PORT/'$API_PORT'/g'  ./config/userQuery.conf
sed -i 's/RTB_REDIS_NUM/1/g'  ./config/userQuery.conf
#sed -i 's/GOODS_REDIS_NUM/1/g'  ./config/userQuery.conf
#sed -i 's/TOKEN_REDIS_NUM/1/g'  ./config/userQuery.conf
sed -i 's/THREAD_NUM/'$API_THREAD_NUM'/g'  ./config/userQuery.conf

#cd /usr/src/myapp && rm -Rf objs src
./LdpRtbServerApi -c ./config/userQuery.conf -n query
