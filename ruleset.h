#ifndef _RULESET_H_
#define _RULESET_H_


#include <vector>
#include "labelset.h"
#include "rule.h"
#include "scratchpad.h"
#include "langexporter.h"

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

	void CalculateUndefined(LabelSet* labelset);

	void Export(LangExporter* exporter, LabelSet* labelset);
	
	bool Divide();
	void Add(const Rule& rule);
	void Print();

	bool CheckOverlap(LabelSet* labels);

	void SaveDot(const char* filepath);

	void SetScratchpad(Scratchpad* scratchpad) { mScratchpad = scratchpad; }
	void SetGamma(float gamma) { mGamma = gamma; }	

private:
	bool Divide(unsigned int* idgen);
	unsigned int CalculateGlobalMask() const;
	unsigned int CalculateLabel() const;
	void Minimize();
	void SaveDot(FILE* f);
	float TestDivideByPattern(const Pattern& f);
	float TestDivideByTable(unsigned int bitstart, unsigned int bitlength);
	bool DivideByPattern(unsigned int* idgen, const Pattern& f);
	bool DivideByTable(unsigned int* idgen, unsigned int bitstart, unsigned int bitlength);
	void BuildExportQueue(std::vector<RuleSet*>* queue);
	void ExportNode(LangExporter* exporter, LabelSet* labelset);
	void ExportTables(LangExporter* exporter, LabelSet* labelset);

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