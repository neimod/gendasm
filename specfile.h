#ifndef _SPECFILE_H_
#define _SPECFILE_H_

#include <string>
#include "ruleset.h"
#include "labelset.h"


class SpecFile
{
	typedef struct
	{
		unsigned int size;
		char* text;
		unsigned int linenum;
	} Line;

public:
	SpecFile();
	~SpecFile();

	void Clear();
	void Load(const char* filepath);
	bool LoadRules(RuleSet* rules, LabelSet* labels);

	const std::string& RootFunctionName() const { return mRootName; }
	
	unsigned int CodeTextLineCount() const { return mCodeText.size(); }
	char* FetchCodeText(unsigned int line, unsigned int* size);

private:
	void ParseConfig();

private:
	std::vector<Line*> mPatternText;
	std::vector<Line*> mConfigText;
	std::vector<Line*> mCodeText;

	unsigned int mSize;
	char* mData;
	std::string mRootName;
};

#endif // _SPECFILE_H_
