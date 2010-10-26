#include "specfile.h"
#include "csrcexporter.h"

typedef struct
{
	const char* text;
	unsigned int id;
	unsigned int index;
} keyword;

typedef enum
{
	KEYWORD_DECODER,
} KeywordIds;

keyword keywords[1]=
{
	{"%GENDASM.DECODER%", KEYWORD_DECODER, 0},
};

static void FetchTextFromNode(TiXmlHandle node, std::string* str)
{
	TiXmlElement* elem = node.ToElement();

	
	str->clear();

	if (elem)
	{
		const char* text = elem->GetText();

		
		if (text)
			str->assign(text);
	}
}


SpecFile::SpecFile()
{
}

SpecFile::~SpecFile()
{
	Clear();
}

void SpecFile::Clear()
{
}


bool SpecFile::LoadMacroPatterns(TiXmlHandle spec)
{
	TiXmlElement* macropatternnode = spec.FirstChild("MACROPATTERN").ToElement();
	TiXmlElement* itemnode;
	PatternSet* macropattern;
	const char* name;
	const char* bitstring;
	
	
	mMacroPatterns.Clear();


	while(macropatternnode)
	{
		itemnode = macropatternnode->FirstChildElement("ITEM");
		name = macropatternnode->Attribute("name");
		

		if (name == 0)
		{
			fprintf(stderr, "ERROR: No name for macropattern.\n");
			return false;
		}
		else
		{
			macropattern = mMacroPatterns.Add(name);

			while(itemnode)
			{
				bitstring = itemnode->GetText();

				if (bitstring == 0)
				{
					fprintf(stderr, "ERROR: No bitstring found for macropattern \"%s\"\n", name);
					return false;
				}
				else
				{
					Pattern pattern;

					pattern.Parse(bitstring);
					macropattern->Add(pattern);
				}

				itemnode = itemnode->NextSiblingElement("ITEM");
			}

			if (macropattern->HasOverlap())
			{
				fprintf(stderr, "ERROR: Macropattern \"%s\" has overlapping bitstrings.\n", name);
				return false;
			}

			macropattern->Minimize();
		}

		macropatternnode = macropatternnode->NextSiblingElement("MACROPATTERN");
	}

	mMacroPatterns.Print();

	return true;
}

bool SpecFile::ParseBitstring(const char* buf, unsigned int size, Pattern* pattern)
{
	unsigned int i;
	unsigned int mask = 0;
	unsigned int sig = 0;
	bool valid = false;


	for(i=0; i<size; i++)
	{
		//printf("%c", buf[i]);

		if (buf[i] == '|')
			break;

		switch(buf[i])
		{
			case '-': 
				mask = (mask<<1)|0; 
				sig = (sig<<1)|0;
				valid = true;
			break;
			
			case '1': 
				mask = (mask<<1)|1; 
				sig = (sig<<1)|1; 
				valid = true;
			break;
			
			case '0': 
				mask = (mask<<1)|1; 
				sig = (sig<<1)|0; 
				valid = true;
			break;
		}
	}

	pattern->SetMask(mask);
	pattern->SetSignature(sig);

	return valid;
}

bool SpecFile::ExpandMacro(PatternSet* patset, const char* macro, unsigned int macrosize)
{
	PatternSet* set = mMacroPatterns.Find(macro);

	if (!set)
	{
		fprintf(stderr, "ERROR: Macropattern %s not found\n", macro);

		return false;
	}

	if (false == patset->Expand(*set))
	{
		fprintf(stderr, "ERROR: Macropattern \"%s\" expansion found bits which should be don't care.\n", macro);

		return false;
	}

	return true;
}

bool SpecFile::LoadPattern(unsigned int label, const char* buf, unsigned int size)
{
	unsigned int i;
	char macro[4096];
	unsigned int macrosize = 0;
	PatternSet patset;
	Pattern pattern;
	Rule rule;


	if (false == ParseBitstring(buf, size, &pattern))
		return false;

//	pattern.SetLabel(label);

	patset.Add(pattern);


	for(i=0; i<size; i++)
	{
		if (buf[i] == '|')
		{
			i++;
			break;
		}
	}

	for(; i<size; i++)
	{
		//printf("%c", buf[i]);

		int isAlphaNumeric = (buf[i] >= 'A' && buf[i] <= 'Z') || (buf[i] >= 'a' && buf[i] <= 'z') || (buf[i] >= '0' && buf[i] <= '9');


		if (macrosize < (sizeof(macro)-1) && isAlphaNumeric)
		{
			macro[macrosize] = buf[i];
			macrosize++;
			macro[macrosize] = 0;
		}
		else if (buf[i] == '|')
		{
			if (macrosize)
			{
				if (false == ExpandMacro(&patset, macro, macrosize))
					return false;
			}

			macrosize = 0;
		}
	}

	if (macrosize)
	{
		if (false == ExpandMacro(&patset, macro, macrosize))
			return false;
	}

	patset.Minimize();
	if (patset.HasOverlap())
	{
		fprintf(stderr, "ERROR: Pattern after expansion has overlap.\n");

		return false;
	}

	for(i=0; i<patset.Size(); i++)
	{
		Pattern* pat = patset.Lookup(i);


		rule.SetSignature(pat->Signature());
		rule.SetMask(pat->Mask());
		rule.SetLabel(label);

		mRules.Add(rule);
	}

	return true;
}

bool SpecFile::LoadRules(TiXmlHandle spec)
{
	TiXmlElement* patternnode = spec.FirstChild("PATTERN").ToElement();
	while(patternnode)
	{
		const char* labelname = patternnode->Attribute("label");
		const char* buf = patternnode->GetText();
		unsigned int size;
		unsigned int label;


		if (labelname == 0)
		{
			fprintf(stderr, "ERROR: No label found for pattern.\n");
		}
		else if (buf == 0)
		{
			fprintf(stderr, "ERROR: No text for pattern.\n");
		}
		else
		{
			size = strlen(buf);

			label = mLabels.Find(labelname);
			if (label == ~0)
				label = mLabels.Add(labelname);


			if (false == LoadPattern(label, buf, size))
				return false;
		}

		patternnode = patternnode->NextSiblingElement("PATTERN");
	}

	return true;
}

bool SpecFile::Load(const char* filepath)
{
	unsigned int undefinedlabel;
	TiXmlDocument document(filepath);



	mRules.Clear();
	mLabels.Clear();
	mMacroPatterns.Clear();

	mRules.SetScratchpad(&mScratchpad);
	
	
	
	if (false == document.LoadFile())
	{
		fprintf(stderr, "ERROR: Failed to load %s.\n", filepath);

		return false;
	}

	TiXmlHandle root(&document);
	TiXmlHandle spec = root.FirstChild("SPEC");

	FetchTextFromNode(spec.FirstChild("FUNCROOT"), &mFuncRoot);
	FetchTextFromNode(spec.FirstChild("LANGUAGE"), &mLanguage);
	FetchTextFromNode(spec.FirstChild("TEMPLATE"), &mTemplate);
	FetchTextFromNode(spec.FirstChild("FUNCVARSDEF"), &mFuncVarsDef);
	FetchTextFromNode(spec.FirstChild("FUNCVARSCALL"), &mFuncVarsCall);
	FetchTextFromNode(spec.FirstChild("FUNCUNDEF"), &mFuncUndef);
	FetchTextFromNode(spec.FirstChild("FUNCSTUB"), &mFuncStub);
	FetchTextFromNode(spec.FirstChild("FUNCTABLE"), &mFuncTable);

	TiXmlElement* specelem = spec.ToElement();
	
	if (!specelem || specelem->QueryFloatAttribute("gamma", &mGamma) != TIXML_SUCCESS)
	{
		mGamma = 2.0f;
		fprintf(stdout, "Gamma not found, defaulting to %f.\n", mGamma);
	}

	mRules.SetGamma(mGamma);

	printf("Function root: %s\n", mFuncRoot.c_str());
	printf("Function vars def: %s\n", mFuncVarsDef.c_str());
	printf("Function vars call: %s\n", mFuncVarsCall.c_str());
	printf("Function undef: %s\n", mFuncUndef.c_str());
	printf("Language: %s\n", mLanguage.c_str());
	printf("Template: %s\n", mTemplate.c_str());
	
	if (false == LoadMacroPatterns(spec))
		return false;

	if (false == LoadRules(spec))
		return false;

	if (mRules.CheckOverlap(&mLabels))
		return false;

	undefinedlabel = mLabels.Add(mFuncUndef.c_str());
	mRules.CalculateUndefined(undefinedlabel);

	mRules.Minimize();

	mRules.Print();
	
	if (false == mRules.Divide())
		return false;

	mRules.ReduceSimilarSubtrees();

	return true;
}
 
void SpecFile::SaveDot(const char* filepath)
{
	mRules.SaveDot(filepath);
}

bool SpecFile::Export(const char* filepath)
{
	CSrcExporter csrcexporter;
	LangExporter* exporter = 0;



	if (mLanguage.compare("C") == 0)
		exporter = &csrcexporter;

	if (exporter == 0)
	{
		fprintf(stderr, "ERROR: Unknown language \"%s\" used for exporting.\n", mLanguage.c_str());
		return false;
	}

	exporter->SetFuncRoot(mFuncRoot);
	exporter->SetFuncVarsDef(mFuncVarsDef);
	exporter->SetFuncVarsCall(mFuncVarsCall);
	exporter->SetFuncStub(mFuncStub);
	exporter->SetFuncTable(mFuncTable);

	return ExportTemplate(filepath, exporter);
}

bool SpecFile::ExportTemplate(const char* filepath, LangExporter* exporter)
{
	char buf[4096];
	int readbytes;
	unsigned int i, j;
	unsigned int templatesize;
	FILE* ftpl = 0;
	FILE* fout = 0;
	unsigned int keywordcount;
	bool keywordchar;
	unsigned int keywordid;
	unsigned int writesizepending;
	unsigned int writesize;
	unsigned int writepos;
	

	keywordcount = sizeof(keywords) / sizeof(keywords[0]);

	ftpl = fopen(mTemplate.c_str(), "r");
	fout = fopen(filepath, "w");
	if (ftpl == 0)
	{
		fprintf(stderr, "ERROR: Unable to open template file \"%s\".\n", mTemplate.c_str());
		return false;
	}
	else if (fout == 0)
	{
		fprintf(stderr, "ERROR: Unable to open output file \"%s\".\n", filepath);
		return false;
	}
	else
	{
		exporter->SetFile(fout);

		templatesize = 0;

		while(1)
		{
			readbytes = fread(buf, 1, sizeof(buf), ftpl);

			if (readbytes<=0)
				break;

			templatesize += readbytes;
		}

		char* templatebuf = new char[templatesize];

		fseek(ftpl, 0, SEEK_SET);
		fread(templatebuf, 1, templatesize, ftpl);


		writesize = 0;
		writesizepending = 0;
		writepos = 0;
		
		for(i=0; i<templatesize; i++)
		{
			keywordchar = false;
			keywordid = ~0;

			for(j=0; j<keywordcount; j++)
			{
				const char* keytext = keywords[j].text;
				unsigned int pos = keywords[j].index;

				if (templatebuf[i] != keytext[pos])
				{
					pos = 0;
				}
				else
				{
					pos++;
					keywordchar = true;

					if (keytext[pos] == 0)
					{
						pos = 0;
						keywordid = keywords[j].id;
					}
				}

				keywords[j].index = pos;
			}
		
			if (!keywordchar)
			{
				writesize+=writesizepending+1;
				writesizepending=0;
			}
			else
			{
				writesizepending++;
			}

			if (keywordid != ~0)
			{
				fwrite(templatebuf+writepos, 1, writesize, fout);
				writepos += writesize+writesizepending;
				writesizepending = 0;
				writesize = 0;
				//printf("FOUND KEYWORD %d\n", keywordid);

				if (keywordid == KEYWORD_DECODER)
				{
					mRules.Export(exporter, &mLabels);
				}
			}
		}
		
		writesize+=writesizepending;
		fwrite(templatebuf + writepos, 1, writesize, fout);

		delete[] templatebuf;
	}

	if (ftpl)
		fclose(ftpl);
	if (fout)
		fclose(fout);

	exporter->SetFile(0);

	return true;
}
