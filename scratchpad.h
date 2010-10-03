#ifndef _SCRATCHPAD_H_
#define _SCRATCHPAD_H_

#include <string>

class Scratchpad
{
public:
	Scratchpad();
	~Scratchpad() { Clear(); }

	void AllocateBucket(unsigned int count);
	void Clear();
	unsigned int* Bucket(unsigned int index) { return mBuckets + index; }

private:
	unsigned int mBucketCount;
	unsigned int* mBuckets;
};

#endif // _SCRATCHPAD_H_