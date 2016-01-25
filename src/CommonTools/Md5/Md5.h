/********************************************************/

/********************************************************/

#ifndef __MD5_H__
#define	__MD5_H__

#include <stdio.h>
#include <string.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/crypto.h>
#include "../../UserQueryStruct/UserQueryStruct.h"

#define	_MD5					16

#define	F1(x, y, z)				(z ^ (x & (y ^ z)))
#define	F2(x, y, z)				F1(z, x, y)
#define	F3(x, y, z)				(x ^ y ^ z)
#define	F4(x, y, z)				(y ^ (x | ~z))

#define	MD5STEP(f, w, x, y, z, data, s)	(w += f(x, y, z) + data, w = w << s | w >> (32 - s), w += x)

class CMd5
{
public:
	CMd5();
	virtual ~CMd5();

	void Md5Init();
	void Md5Final(UCHAR digest[16]);
	void Md5Update(UCHAR* puszBuf, UINT uiLen);
	unsigned char * SHA1_Encode(const  char *src);

//#define __BIT_SET__
inline uint64_t md5_sum(unsigned char *key, uint32_t length)
{
	unsigned char md[16]={0};
	MD5(key,length,md);

#ifdef __BIT_SET__

	for(int i=0;i<16;i++)
	{
		printf("md[%d]=%u\n",i,md[i]);
		
	}
	 for(int i=0;i<16;i++)
	{
		cout<<bitset<8>(md[i])<<::endl;

	}
#endif
	return *((uint64_t *)md);
}

private:
	UINT m_uiBuf[4];
	UINT m_uiBits[2];
	UCHAR m_ucIn[64];

	void Md5Transform(UINT buf[4], UINT in[16]);
	void Md5ByteReverse(UCHAR* puszBuf, UINT uiLongs);



};

#endif /* __MD5_H__ */
