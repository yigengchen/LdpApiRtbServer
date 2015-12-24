#ifndef __BASE64_2_H__
#define __BASE64_2_H__
#include <string>
#include <string.h>
#include <stdio.h>
#include <algorithm>
using namespace std;

#define randomNum_2(x) (rand()%x)
std::string base64_encode_2(unsigned char const * , unsigned int len);
std::string base64_decode_2(std::string const& s);
std::string Decrypt_2(std::string oriString);
std::string Encrypt_2(std::string oriString);
	
#endif //__BASE64_2_H__