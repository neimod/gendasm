#include "scratchpad.h"


Scratchpad::Scratchpad(): mBucketCount(0), mBuckets(0)
{
}

void Scratchpad::Clear()
{
	if (mBuckets)
	{
		delete[] mBuckets;
	}

	mBuckets = 0;
	mBucketCount = 0;
}

void Scratchpad::AllocateBucket(unsigned int count)
{
	unsigned int i;


	if (mBucketCount < count)
	{
		if (mBuckets)
			delete[] mBuckets;

		if (mBucketCount == 0)
			mBucketCount = 64;

		while(mBucketCount < count)
		{
			mBucketCount *= 2;
		}

		mBuckets = new unsigned int[mBucketCount];
	}

	for(i=0; i<count; i++)
		mBuckets[i] = 0;
}

