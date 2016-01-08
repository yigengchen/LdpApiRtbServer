//============================================================================
// Name        : AIBdxUserQuery.cpp
// Author      : chenyigeng

//============================================================================

#include "UserQueryMain/UserQueryMain.h"
///
#include <getopt.h>
#include <iostream>
#include <signal.h>
#include <string>
#include <stdlib.h>
using namespace std;

static void SignalCore (int a){

}

std::string strServerName;
std::string strConfigFileName;


 
int main(int argc, char *argv[]) {

	//system("ulimit -c unlimited");
	//system("ulimit -n 65535");
	
	signal(SIGPIPE, SignalCore);
	int iIndex = 0;
	int iReturn = 0;
	bool bHaveConf = false;
	CUserQueryMain *pCMain = NULL;
	struct option stOptions[] = {
		{"delete",2,0,'d'},
		{"name",1,0,'n'},
		{"config",1,0,'c'},
		{"help",0,0,'h'},
		{"version",0,0,'v'},
		{0,0,0,0},
	};
	try {
		while( 1 ) {
			iReturn = getopt_long(argc,argv,"dn:c:hv",stOptions,&iIndex);
			if( iReturn < 0 ) {
				break;
			}
			switch( iReturn ) {
				case 'd':
					return 0;
				case 'n':
					strServerName = optarg;
					break;
				case 'c':
					strConfigFileName=optarg;
					pCMain = new CUserQueryMain(optarg);
					bHaveConf = true;
					break;
				case 'h':
					return 0;
				case 'v':
					cout << "version: 0.0.1" << endl;
						return 0;
				default:	return 0;
			}
		}
		if(!bHaveConf)
			pCMain = new CUserQueryMain();
		pCMain->UserQueryMainCore();
	} catch(std::exception &e) {
		std::cerr << e.what() << std::endl;
		if (pCMain != NULL) {
			delete pCMain;
		}
		return -1;
	}
	cout << "version: 0.0.1" << endl;
	delete pCMain;
	return 0;
}
