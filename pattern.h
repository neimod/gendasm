#ifndef _PATTERN_H_
#define _PATTERN_H_

#include <string>

class Pattern
{
public:
	Pattern() {}
	~Pattern() {}


	void SetSignature(unsigned int sig) { mSignature = sig; }
	void SetMask(unsigned int mask) { mMask = mask; }
	void BuildString(std::string* str) const;
	void Parse(const char* str);
	void Print() const;

	unsigned int Signature() const { return mSignature; }
	unsigned int Mask() const { return mMask; }

private:
	unsigned int mSignature;
	unsigned int mMask;
};

#endif // _PATTERN_H_