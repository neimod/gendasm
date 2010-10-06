#ifndef _LANGEXPORTER_H_
#define _LANGEXPORTER_H_

#include <string>

class LangExporter
{
public:
	LangExporter() {}
	~LangExporter() {}

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
	virtual void VisitLabel(const std::string& label) {}
	virtual void VisitPattern(unsigned int mask, unsigned int signature) {}
	virtual void BeginTrueBranch() {}
	virtual void EndTrueBranch() {}
	virtual void BeginFalseBranch() {}
	virtual void EndFalseBranch() {}
	virtual void VisitTable(unsigned int shift, unsigned int mask, unsigned id) {}

private:
};

#endif // _LANGEXPORTER_H_
