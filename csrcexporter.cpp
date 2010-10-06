#include "csrcexporter.h"

CSrcExporter::CSrcExporter(): mIndent(0), mFile(0), mSpecFile(0)
{
}

CSrcExporter::~CSrcExporter()
{
}

bool CSrcExporter::Begin()
{
	mFile = fopen(mPath.c_str(), "w");

	if (!mFile)
	{
		fprintf(stderr, "ERROR: Cannot open file %s\n", mPath.c_str());
		return false;
	}


	fprintf(mFile, "#include \"types.h\"\n");
	fprintf(mFile, "typedef void(*func_t)(opcode*, unsigned int);\n\n");
	fprintf(mFile, "static void nullstub(opcode*, unsigned int);\n");
	fprintf(mFile, "void nullstub(opcode* op, unsigned int in)\n{\n}\n\n");


	return true;
}

void CSrcExporter::End()
{
	if (mSpecFile && mFile)
	{
		unsigned int count = mSpecFile->CodeTextLineCount();
		unsigned int i;
		char* text;
		unsigned int size;

		for(i=0; i<count; i++)
		{
			text = mSpecFile->FetchCodeText(i, &size);

			fwrite(text, 1, size, mFile);
			fprintf(mFile, "\n");
		}
	}

	if (mFile)
		fclose(mFile);

	mFile = 0;
}

void CSrcExporter::VisitLabelPrototype(const std::string& label)
{
	fprintf(mFile, "static void %s(opcode*, unsigned int);\n", label.c_str());
}

void CSrcExporter::VisitStubPrototype(unsigned int id)
{
	fprintf(mFile, "static void stub_%d(opcode*, unsigned int);\n", id);
}

void CSrcExporter::BeginTable(unsigned int id, unsigned int count)
{
	fprintf(mFile, "\n");

	fprintf(mFile, "static func_t table_%d[%d]=\n", id, count);
	fprintf(mFile, "{\n");
}

void CSrcExporter::VisitLabelEntry(const std::string& label)
{
	fprintf(mFile, " %s,\n", label.c_str());
}

void CSrcExporter::VisitStubEntry(unsigned int id)
{
	fprintf(mFile, " stub_%d,\n", id);
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
		fprintf(mFile, "\nvoid %s(opcode* op, unsigned int in)\n", mSpecFile->RootFunctionName().c_str());
	else
		fprintf(mFile, "\nvoid stub_%d(opcode* op, unsigned int in)\n", id);
	fprintf(mFile, "{\n");

	mIndent = 1;
}

void CSrcExporter::EndStub()
{
	fprintf(mFile, "}\n\n");
}

void CSrcExporter::VisitLabel(const std::string& label)
{
	Indent(mIndent);
	fprintf(mFile, "%s(op, in);\n", label.c_str());
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
	fprintf(mFile, "(table_%d[(in>>%d)&0x%X])(op, in);\n", id, shift, mask);
}


