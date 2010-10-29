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

	void Clear();
	void CalculateUndefined(unsigned int label);

	void Export(LangExporter* exporter, LabelSet* labelset);
	
	void Minimize();
	bool Divide();
	void Add(const Rule& rule);
	void Print();

	bool CheckOverlap(LabelSet* labels);

	void SaveDot(const char* filepath, bool reduced);

	void SetScratchpad(Scratchpad* scratchpad) { mScratchpad = scratchpad; }
	void SetGamma(float gamma) { mGamma = gamma; }	

	bool IsSimilar(RuleSet* set);
	void ReduceSimilarSubtrees();
	

private:
	bool Divide(unsigned int* idgen);
	unsigned int CalculateGlobalMask() const;
	unsigned int CalculateLabel() const;
	
	void SaveDot(FILE* f, bool reduced);
	float TestDivideByPattern(const Pattern& f);
	float TestDivideByTable(unsigned int bitstart, unsigned int bitlength);
	bool DivideByPattern(unsigned int* idgen, const Pattern& f);
	bool DivideByTable(unsigned int* idgen, unsigned int bitstart, unsigned int bitlength);
	void BuildExportQueue(std::vector<RuleSet*>* queue);
	void ExportNode(LangExporter* exporter, LabelSet* labelset);
	void ExportTables(LangExporter* exporter, LabelSet* labelset);
	void BuildNodeTable(std::vector<RuleSet*>* table);

private:
	float mGamma;
	Scratchpad* mScratchpad;
	std::vector<Rule*> mRules;
	unsigned int mDivType;
	Pattern mDivPattern;
	unsigned int mDivTableStart;
	unsigned int mDivTableLength;
	std::vector<RuleSet*> mChildren;
	unsigned int mLabel;
	unsigned int mNode;
	RuleSet* mRedirect;
	bool mStub;
	bool mReducedDotOutput;
};

#endif // _RULESET_H_