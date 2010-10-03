#ifndef _RULE_H_
#define _RULE_H_

#include "pattern.h"

class Rule
{
public:
	Rule() {}
	~Rule() {}

	void SetSignature(unsigned int sig) { mPattern.SetSignature(sig); }
	void SetMask(unsigned int mask) { mPattern.SetMask(mask); }
	void SetLabel(unsigned int label) { mLabel = label; }
	void SetLineNum(unsigned int linenum) { mLineNum = linenum; }

	unsigned int Mask() const { return mPattern.Mask(); }
	unsigned int Signature() const { return mPattern.Signature(); }

	unsigned int Label() const { return mLabel; }
	unsigned int LineNum() const { return mLineNum; }

	void BuildPatternString(std::string* str) const { mPattern.BuildString(str); }
	void Print() const { mPattern.Print(); }

private:
	Pattern mPattern;
	unsigned int mLabel;
	unsigned int mLineNum;
};

#endif // _RULE_H_