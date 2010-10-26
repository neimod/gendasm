#ifndef _LANGEXPORTER_H_
#define _LANGEXPORTER_H_

#include <string>
#include <stdio.h>

class LangExporter
{
public:
	LangExporter() {}
	~LangExporter() {}

	void SetFile(FILE* file) { mFile = file; }
	void SetFuncRoot(const std::string& str) { mFuncRoot = str; }
	void SetFuncVarsDef(const std::string& str) { mFuncVarsDef = str; }
	void SetFuncVarsCall(const std::string& str) { mFuncVarsCall = str; }
	void SetFuncStub(const std::string& str) { mFuncStub = str; }
	void SetFuncTable(const std::string& str) { mFuncTable = str; }

	virtual bool Begin() { return false; }
	virtual void End() {}
	virtual void BeginStub(unsigned int id) {}
	virtual void EndStub() {}
	virtual void VisitLabelPrototype(const std::string& label) {}
	virtual void VisitStubPrototype(unsigned int id) {}
	virtual void BeginTable(unsigned int id, unsigned int count) {}
	virtual void VisitLabelEntry(const std::string& label) {}
	virtual void VisitStubEntry(unsigned int id) {}
	virtual void EndTable() {}
	virtual void VisitStub(unsigned int id) {}
	virtual void VisitLabel(const std::string& label) {}
	virtual void VisitPattern(unsigned int mask, unsigned int signature) {}
	virtual void BeginTrueBranch() {}
	virtual void EndTrueBranch() {}
	virtual void BeginFalseBranch() {}
	virtual void EndFalseBranch() {}
	virtual void VisitTable(unsigned int shift, unsigned int mask, unsigned id) {}
protected:
	FILE* mFile;
	std::string mFuncRoot;
	std::string mFuncVarsDef;
	std::string mFuncVarsCall;
	std::string mFuncStub;
	std::string mFuncTable;
private:

};

#endif // _LANGEXPORTER_H_
