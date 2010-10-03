#ifndef _RULESET_H_
#define _RULESET_H_


#include <vector>
#include "labelset.h"
#include "rule.h"
#include "scratchpad.h"


class RuleSet
{

	typedef struct
	{
		Pattern f;
		unsigned int score;
		unsigned int leftUnique;
		unsigned int rightUnique;
		unsigned int leftTotal;
		unsigned int rightTotal;
	} divcontext;

public:
	RuleSet();
	~RuleSet();

	bool Load(const char* filepath, LabelSet* labelset);
	void CalculateUndefined(LabelSet* labelset);
	
	bool Divide();
	void Add(const Rule& rule);
	void Print();
	void SetScratchpad(Scratchpad* scratchpad) { mScratchpad = scratchpad; }
	bool CheckOverlap(const LabelSet& labels);

	void SaveDot(const char* filepath);

private:
	unsigned int CalculateGlobalMask() const;
	unsigned int CalculateLabel() const;
	void Minimize();
	void MinimizeAlternative();
	void SaveDot(FILE* f);
	float TestDivideByPattern(const Pattern& f);
	float TestDivideByTable(unsigned int bitstart, unsigned int bitlength);
	bool DivideByPattern(const Pattern& f);
	bool DivideByTable(unsigned int bitstart, unsigned int bitlength);
	
	
private:

	//bool CheckUniqueDupes();
	//void FindMinPattern(const Pattern& f, divcontext* minctx);

private:
	float mGamma;
	Scratchpad* mScratchpad;
	std::vector<Rule*> mRules;
	unsigned int mDivType;
	Pattern mDivPattern;
	unsigned int mDivTableStart;
	unsigned int mDivTableLength;
	unsigned int mChildCount;
	RuleSet* mChildren;
	unsigned int mLabel;
	unsigned int mNode;
};

#endif // _RULESET_H_