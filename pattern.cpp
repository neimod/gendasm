#include "pattern.h"

void Pattern::BuildString(std::string* str) const
{
	char buf[33];
	unsigned int i;


	for(i=0; i<32; i++)
	{
		if (mMask & (1<<i))
		{
			if (mSignature & (1<<i))
				buf[31-i] = '1';
			else
				buf[31-i] = '0';
		}
		else
		{
			buf[31-i] = '-';
		}
	}
	buf[32] = 0;

	str->assign(buf);
}


void Pattern::Print() const
{
	unsigned int i, t;


	for(i=0; i<32; i++)
	{
		t = 1<<(31-i);

		if (mMask&t)
			printf("%s", mSignature&t? "1" : "0");
		else
			printf("-");
	}
	printf("\n");
}


void Pattern::Parse(const char* str)
{
	unsigned int i;


	mSignature = 0;
	mMask = 0;

	for(i=0; str[i] != 0; i++)
	{
		if (str[i] == '0')
		{
			mSignature = (mSignature<<1) | 0;
			mMask = (mMask<<1) | 1;
		}
		else if (str[i] == '1')
		{
			mSignature = (mSignature<<1) | 1;
			mMask = (mMask<<1) | 1;
		}
		else if (str[i] == '-')
		{
			mSignature = (mSignature<<1) | 0;
			mMask = (mMask<<1) | 0;
		}
	}
}