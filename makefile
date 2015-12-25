################################################################################
# Author:	chenyigeng
# Date:		2013-07-06
# Desc:		the makefile is for compiling all project
################################################################################

# the distinguish between = and := ,to a=$(a), := will be assiged only once,= will be compile error.


#set your project name
PRJ_NAME =LdpRtbServerApi 
#PRJ_NAME=libtest.so

#set your project type : choose one below
#PRJ_TYPE = g++ -shared -fPIC
PRJ_TYPE = g++
#PRJ_TYPE = ar -r 

#set Debug or Release
Compile_Flag = Debug
#Compile_Flag = Release

#set your output path
Output:= ./objs

#set your source folder
SRC:= ./src

#add the lib you used here
#LIBS := -lLib1 -lLib2 -lLib3
LIBS :=  -lpthread -lssl -lcrypto -lrt -ldl -lm 


#静态库要放在后面链接 
STATIC_LIBS:=./lib/redis/libhiredis.a ./lib/json/libjson_linux-gcc-4.4.7_libmt.a ./lib/mysql/libmysqlclient.a ./lib/mysql/libmysqlservices.a

#LIBPATH := -Lpath1 -Lpath2 -Lpath3
LIBPATH := 
INCLUDEPATH:=  -I/usr/inclde  -I./include/  -I./include/mysql -I./include/mysql/mysql -I./include/mysql/mysql/psi
# INCLUDEPATH := -I/usr/lib/XXX/include

###################################
#DON"T MODIFY THE BELOWS

#combine output folder
FinalOutput := $(Output)/

#list all dirs
SUBDIRS := $(shell find $(SRC) -type d)
#CLEAN2 := $(shell find $(Output) -type f|xargs rm)
#flags in makefile
DEBUG_FLAG = -O0 -g3 -Wall -c -fmessage-length=0
RELEASE_FLAG = -O3 -Wall -c -fmessage-length=0
RM:= rm -rf

#set compile flag
ifeq ($(Compile_Flag),Debug)
CFLAGS := $(DEBUG_FLAG)
else
CFLAGS := $(RELEASE_FLAG)
endif

#prepare files
CPP_SRCS:=$(shell find $(SRC) -name *.cpp)
CPP_OBJS:=$(CPP_SRCS:%.cpp=$(FinalOutput)%.o)


CC_SRCS:=$(shell find $(SRC) -name *.cc)
CC_OBJS:=$(CC_SRCS:%.cc=$(FinalOutput)%.o)

C_SRCS:=$(shell find $(SRC) -name *.c)
C_OBJS:=$(C_SRCS:%.c=$(FinalOutput)%.o)

OBJS+=$(CPP_OBJS)
OBJS+=$(CC_OBJS)
OBJS+=$(C_OBJS)

#all target


#OBJS_CNT:=$(shell find ./ -type d -name $(Output)|wc -l)
OBJS_CNT:=$(shell find ./ -type d -wholename $(Output)|wc -l)


ifeq ($(OBJS_CNT),1)
	
	ALL_TARTET+=$(PRJ_NAME)
else
	
	ALL_TARTET+=dir
	ALL_TARTET+=$(PRJ_NAME)
endif


all:$(ALL_TARTET)

#all1 :$(PRJ_NAME)

#all2 :dir $(PRJ_NAME)

dir:
	mkdir -p $(FinalOutput);
	for val in $(SUBDIRS);do \
	  mkdir -p $(FinalOutput)$${val}; \
	done;

#tool invocations
$(PRJ_NAME):$(OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking:GCC C++ Linker'
	$(PRJ_TYPE) $(LIBS) $(LIBPATH) -o "$@" $^  $(STATIC_LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

$(FinalOutput)%o:./%cpp
	@echo 'Building file: $<'
	@echo 'Invoking:GCC C++ Compiler'
	$(PRJ_TYPE) $(CFLAGS) $(INCLUDEPATH) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

$(FinalOutput)%o:./%cc
	@echo 'Building file: $<'
	@echo 'Invoking:GCC C++ Compiler'
	$(PRJ_TYPE) $(CFLAGS) $(INCLUDEPATH) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

$(FinalOutput)%o:./%c
	@echo 'Building file: $<'
	@echo 'Invoking:GCC C++ Compiler'
	$(PRJ_TYPE) $(CFLAGS) $(INCLUDEPATH) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

#other targets

clean:
	-$(shell find $(Output) -type f|xargs rm) $(RM) $(PRJ_NAME)
	-@echo ' '	
clean2:
	-$(RM) $(Output) $(PRJ_NAME)
	-@echo ' '

.PHONY:all clean
.SECONDARY:
