#ifndef _SPECFILE_H_
#define _SPECFILE_H_

#include <string>
#include "tinyxml.h"
#include "ruleset.h"
#include "labelset.h"
#include "macropatternset.h"
#include "scratchpad.h"
#include "langexporter.h"


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
	bool Load(const char* filepath);
	void SaveDot(const char* filepath);
	bool Export(const char* filepath);

private:
	bool LoadMacroPatterns(TiXmlHandle spec);
	bool LoadRules(TiXmlHandle spec);
	bool LoadPattern(unsigned int label, const char* buf, unsigned int size);
	bool ExpandMacro(PatternSet* patset, const char* macro, unsigned int macrosize);
	bool ParseBitstring(const char* buf, unsigned int size, Pattern* pattern);
	bool ExportTemplate(const char* filepath, LangExporter* exporter);

private:

	std::string mLanguage;
	std::string mFuncRoot;
	std::string mFuncVarsDef;
	std::string mFuncVarsCall;
	std::string mFuncUndef;
	std::string mFuncStub;
	std::string mFuncTable;
	std::string mTemplate;
	float mGamma;

	MacroPatternSet mMacroPatterns;
	LabelSet mLabels;
	RuleSet mRules;
	Scratchpad mScratchpad;
};

#endif // _SPECFILE_H_
