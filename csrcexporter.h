#ifndef _CSRCEXPORTER_H_
#define _CSRCEXPORTER_H_

#include <string>
#include <stdio.h>
#include "langexporter.h"
#include "specfile.h"

class CSrcExporter: public LangExporter
{
public:
	CSrcExporter();
	~CSrcExporter();

	void SetOutput(const char* filepath) { mPath = filepath; }
	void SetSpecFile(SpecFile* specfile) { mSpecFile = specfile; }

	virtual bool Begin();
	virtual void End();	
	virtual void BeginStub(unsigned int id);
	virtual void EndStub();
	virtual void VisitLabelPrototype(const std::string& label);
	virtual void VisitStubPrototype(unsigned int id);
	virtual void BeginTable(unsigned int id, unsigned int count);
	virtual void VisitLabelEntry(const std::string& label);
	virtual void VisitStubEntry(unsigned int id);
	virtual void EndTable();
	virtual void VisitLabel(const std::string& label);
	virtual void VisitPattern(unsigned int mask, unsigned int signature);
	virtual void BeginTrueBranch();
	virtual void EndTrueBranch();
	virtual void BeginFalseBranch();
	virtual void EndFalseBranch();
	virtual void VisitTable(unsigned int shift, unsigned int mask, unsigned id);

private:
	void Indent(unsigned int count);
private:
	unsigned int mIndent;
	std::string mPath;
	FILE* mFile;
	SpecFile* mSpecFile;
};

#endif // _LANGEXPORTER_H_
