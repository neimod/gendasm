#include "csrcexporter.h"

CSrcExporter::CSrcExporter(): mIndent(0)
{
}

CSrcExporter::~CSrcExporter()
{
}

bool CSrcExporter::Begin()
{
	fprintf(mFile, "typedef void(*func_t)(%s);\n", mFuncVarsDef.c_str());
	return true;
}

void CSrcExporter::End()
{
}

void CSrcExporter::VisitLabelPrototype(const std::string& label)
{
	fprintf(mFile, "static void %s(%s);\n", label.c_str(), mFuncVarsDef.c_str());
}

void CSrcExporter::VisitStubPrototype(unsigned int id)
{
	fprintf(mFile, "static void %s%d(%s);\n", mFuncStub.c_str(), id, mFuncVarsDef.c_str());
}

void CSrcExporter::BeginTable(unsigned int id, unsigned int count)
{
	fprintf(mFile, "\n");

	fprintf(mFile, "static func_t %s%d[%d]=\n", mFuncTable.c_str(), id, count);
	fprintf(mFile, "{\n");
}

void CSrcExporter::VisitLabelEntry(const std::string& label)
{
	fprintf(mFile, " %s,\n", label.c_str());
}

void CSrcExporter::VisitStubEntry(unsigned int id)
{
	fprintf(mFile, " %s%d,\n", mFuncStub.c_str(), id);
}

void CSrcExporter::EndTable()
{
	fprintf(mFile, "};\n\n");
}


void CSrcExporter::Indent(unsigned int count)
{
	unsigned int i;


	for(i=0; i<count; i++)
		fprintf(mFile, "  ");
}

void CSrcExporter::BeginStub(unsigned int id)
{
	if (id == 0)
		fprintf(mFile, "\nvoid %s(%s)\n", mFuncRoot.c_str(), mFuncVarsDef.c_str());
	else
		fprintf(mFile, "\nvoid %s%d(%s)\n", mFuncStub.c_str(), id, mFuncVarsDef.c_str());
	fprintf(mFile, "{\n");

	mIndent = 1;
}

void CSrcExporter::EndStub()
{
	fprintf(mFile, "}\n\n");
}

void CSrcExporter::VisitStub(unsigned int id)
{
	Indent(mIndent);
	fprintf(mFile, "%s%d(%s);\n", mFuncStub.c_str(), id, mFuncVarsCall.c_str());
}

void CSrcExporter::VisitLabel(const std::string& label)
{
	Indent(mIndent);
	fprintf(mFile, "%s(%s);\n", label.c_str(), mFuncVarsCall.c_str());
}

void CSrcExporter::VisitPattern(unsigned int mask, unsigned int signature)
{
	Indent(mIndent);
	fprintf(mFile, "if ((in & 0x%X) == 0x%X)\n", mask, signature);
}

void CSrcExporter::BeginTrueBranch()
{
	Indent(mIndent);
	fprintf(mFile, "{\n");
	mIndent++;
}

void CSrcExporter::EndTrueBranch()
{
	mIndent--;
	Indent(mIndent);
	fprintf(mFile, "}\n");
}

void CSrcExporter::BeginFalseBranch()
{
	Indent(mIndent);
	fprintf(mFile, "else\n");
	Indent(mIndent);
	fprintf(mFile, "{\n");
	mIndent++;
}

void CSrcExporter::EndFalseBranch()
{
	mIndent--;
	Indent(mIndent);
	fprintf(mFile, "}\n");	
}

void CSrcExporter::VisitTable(unsigned int shift, unsigned int mask, unsigned id)
{
	Indent(mIndent);
	fprintf(mFile, "(%s%d[(in>>%d)&0x%X])(%s);\n", mFuncTable.c_str(), id, shift, mask, mFuncVarsCall.c_str());
}


