#ifndef __BASE64_H__
#define __BASE64_H__
#include <string>
#include <string.h>
#include <stdio.h>
#include <algorithm>
using namespace std;

#define randomNum(x) (rand()%x)
std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);
std::string Decrypt(std::string oriString);
std::string Encrypt(std::string oriString);
	
#endif //__BASE64_H__