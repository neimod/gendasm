#include "specfile.h"



SpecFile::SpecFile(): mSize(0), mData(0)
{
}

SpecFile::~SpecFile()
{
	Clear();
}

void SpecFile::Clear()
{
	unsigned int i;


	if (mData)
		delete[] mData;

	mData = 0;
	mSize = 0;


	for(i=0; i<mPatternText.size(); i++)
	{
		Line* line = mPatternText[i];
		delete line;
	}

	for(i=0; i<mConfigText.size(); i++)
	{
		Line* line = mConfigText[i];

		delete line;
	}

	for(i=0; i<mCodeText.size(); i++)
	{
		Line* line = mCodeText[i];

		delete line;
	}
}

void SpecFile::Load(const char* filepath)
{
	FILE* f;
	unsigned char buf[4096];
	unsigned int count = 0;
	unsigned int linenum = 1;
	unsigned int start;
	unsigned int end;
	unsigned int section = 0;
	unsigned int i;
	unsigned int j;

	
	f = fopen(filepath, "r");

	if (!f)
		return;

	mSize = 0;

	while(!feof(f))
	{
		count = fread(buf, 1, sizeof(buf), f);

		mSize += count;
	}

	mData = new char[mSize];

	fseek(f, 0, SEEK_SET);
	mSize = fread(mData, 1, mSize, f);
	fclose(f);

	start = 0;
	end = 0;
	while(end < mSize)
	{
		if (mData[end] == '\n')
		{
			Line* line = new Line;

			if (end - start == 1 && mData[start] == '%')
			{
				if (section == 0 || section == 1)
					section++;
			}
			else
			{
				line->linenum = linenum;
				line->size = end - start;
				line->text = mData + start;

				if (section == 0)
					mPatternText.push_back(line);
				else if (section == 1)
					mConfigText.push_back(line);
				else
					mCodeText.push_back(line);
			}

			
			end++;
			start = end;
			linenum++;
		}
		else
		{
			end++;
		}
	}
/*
	printf("[Patterns]\n");
	for(i=0; i<mPatternText.size(); i++)
	{
		Line* line = mPatternText[i];

		printf("%d |", line->linenum);
		for(j=0; j<line->size; j++)
			printf("%c", line->text[j]);
		printf("|\n");
	}
	printf("[Config]\n");
	for(i=0; i<mConfigText.size(); i++)
	{
		Line* line = mConfigText[i];

		printf("%d |", line->linenum);
		for(j=0; j<line->size; j++)
			printf("%c", line->text[j]);
		printf("|\n");
	}
	printf("[Code]\n");
	for(i=0; i<mCodeText.size(); i++)
	{
		Line* line = mCodeText[i];

		printf("%d |", line->linenum);
		for(j=0; j<line->size; j++)
			printf("%c", line->text[j]);
		printf("|\n");
	}
*/


	ParseConfig();
}
 
void SpecFile::ParseConfig()
{
	unsigned int i;
	unsigned int j;
	char* buf;
	unsigned int bufsize;
	char optiontext[4096];
	char valtext[4096];
	unsigned int optionsize;
	unsigned int valsize;
	unsigned int pos;
	unsigned int state;


	for(i=0; i<mConfigText.size(); i++)
	{
		Line* line = mConfigText[i];

		buf = line->text;
		bufsize = line->size;

		valsize = 0;
		optionsize = 0;
		pos = 0;
		state = 0;

		for(j=0; j<bufsize; j++)
		{
			if (state == 0)
			{
				if (buf[j] == '=')
				{
					state = 1;
					pos = 0;
				}
				else if (pos < (sizeof(optiontext)-1))
				{
					optiontext[pos++] = buf[j];
					optionsize = pos;
				}
			}
			else
			{
				valtext[pos++] = buf[j];
				valsize = pos;
			}
		}

		optiontext[optionsize] = 0;
		valtext[valsize] = 0;

		if (!stricmp(optiontext, "root"))
		{
			mRootName = valtext;
		}
	}
}

bool SpecFile::LoadRules(RuleSet* rules, LabelSet* labels)
{
	FILE* f = 0;
	char* buf;
	unsigned int bufsize;
	unsigned int sig;
	unsigned int mask;
	int valid;
	unsigned int j;
	unsigned int i;
	unsigned int namelen;
	unsigned int label;
	char name[4096];
	int fetchname;
	Rule rule;
	unsigned int linenum;


	for(j=0; j<mPatternText.size(); j++)
	{
		Line* line = mPatternText[j];

		buf = line->text;
		bufsize = line->size;
		linenum = line->linenum;

		valid = 0;
		sig = 0;
		mask = 0;
		namelen = 0;
		fetchname = 0;

		for(i=0; i<bufsize && buf[i] != '#'; i++)
		{
			switch(buf[i])
			{
				case ']':
					fetchname = 0;
				break;
			}


			if (fetchname && (namelen+1) < sizeof(name))
				name[namelen++] = buf[i];

			switch(buf[i])
			{
				case '-': 
					mask = (mask<<1)|0; 
					sig = (sig<<1)|0;
					valid = 1;
				break;
				
				case '1': 
					mask = (mask<<1)|1; 
					sig = (sig<<1)|1; 
					valid = 1;
				break;
				
				case '0': 
					mask = (mask<<1)|1; 
					sig = (sig<<1)|0; 
					valid = 1;
				break;

				case '[':
					fetchname = 1;
				break;
			}
		}


		name[namelen] = 0;

		if (valid && namelen > 0)
		{
			label = labels->Find(name);
			if (label == ~0)
				label = labels->Add(name);

			rule.SetSignature(sig);
			rule.SetMask(mask);
			rule.SetLabel(label);
			rule.SetLineNum(linenum);

			rules->Add(rule);
		}
	}

//	std::sort(mRules.begin(), mRules.end(), RuleSetSortPredicate);
	rules->Print();

	return true;
}

char* SpecFile::FetchCodeText(unsigned int l, unsigned int* size)
{
	Line* line = mCodeText[l];

	*size = line->size;
	return line->text;
}
