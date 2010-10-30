#include <stdio.h>
#include <algorithm>
#include <math.h>
#include "ruleset.h"
#include "patternset.h"


typedef enum
{
	MATCH_YES,
	MATCH_NO,
	MATCH_PARTIAL
} MatchResult;

typedef enum
{
	DIV_PATTERN,
	DIV_TABLE
} DivType;


// test if the pattern from rule r is consumed by the f pattern.
static MatchResult RuleMatch(const Rule& r, const Pattern& f)
{
	unsigned int commonmask = f.Mask() & r.Mask();
	if ( (r.Signature() & commonmask) != (f.Signature() & commonmask) )
		return MATCH_NO;

	if ( (r.Mask() & f.Mask()) == f.Mask() )
		return MATCH_YES;

	return MATCH_PARTIAL;
}

// count one bits. most efficient for sparse ones.
static unsigned int bitcount(unsigned int n)  
{
	unsigned int count = 0;
	while (n)  
	{
		count++;
		n &= (n - 1);
	}
	return count;
}

bool RuleSetSortPredicate(Rule* a, Rule* b)
{
  return a->Label() < b->Label();
}

RuleSet::RuleSet(): mScratchpad(0), mGamma(9.0f), mLabel(0), mRedirect(0), mStub(false)
{
}

RuleSet::~RuleSet()
{
	Clear();
}

void RuleSet::Clear()
{
	unsigned int i;


	for(i=0; i<mRules.size(); i++)
	{
		Rule* rule = mRules[i];

		delete rule;
	}

	mRules.clear();


	for(i=0; i<mChildren.size(); i++)
	{
		RuleSet* child = mChildren[i];

		delete child;
	}

	mChildren.clear();

	mRedirect = 0;
	mStub = false;
}

void RuleSet::Add(const Rule& rule)
{
	mRules.push_back( new Rule(rule) );
}

unsigned int RuleSet::CalculateGlobalMask() const
{
	unsigned int i;
	unsigned int mask = 0;


	for(i=0; i<mRules.size(); i++)
	{
		Rule* rule = mRules[i];

		mask |= rule->Mask();
	}

	return mask;
}

unsigned int RuleSet::CalculateLabel() const
{
	unsigned int i;
	unsigned int label = ~1;


	for(i=0; i<mRules.size(); i++)
	{
		Rule* rule = mRules[i];

		if (i == 0)
			label = rule->Label();
		else if (rule->Label() != label)
			return ~0;
	}

	return label;
}

void RuleSet::CalculateUndefined(unsigned int label)
{
	PatternSet defined;
	PatternSet undefined;
	unsigned int i;


	for(i=0; i<mRules.size(); i++)
	{
		Rule* rule = mRules[i];
		Pattern pat;

		pat.SetSignature( rule->Signature() );
		pat.SetMask( rule->Mask() );

		defined.Add(pat);
	}

	undefined.Complement(defined);

	if (undefined.Size() != 0)
	{
		
		for(i=0; i<undefined.Size(); i++)
		{
			Pattern* pat = undefined.Lookup(i);
			Rule rule;

			rule.SetSignature( pat->Signature() );
			rule.SetMask( pat->Mask() );
			rule.SetLabel( label );
			rule.SetLineNum(0);

			printf("Add undefined rule:");
			rule.Print();

			Add(rule);
		}
	}
}

bool RuleSet::CheckOverlap(LabelSet* labels)
{
	unsigned int i;
	unsigned int j;
	unsigned int commonmask;
	bool result = false;
	std::string pa, pb;


	for(i=0; i<mRules.size(); i++)
	{
		Rule* ra = mRules[i];


		for(j=i+1; j<mRules.size(); j++)
		{
			Rule* rb = mRules[j];

			commonmask = ra->Mask() & rb->Mask();

			if ( (ra->Signature() & commonmask) == (rb->Signature() & commonmask) )
			{
				if (!result)
				{
					result = true;
					fprintf(stderr, "ERROR: Please fix the overlap of bitstrings between the following rules:\n");
				}

				const std::string& la = labels->Lookup(ra->Label());
				const std::string& lb = labels->Lookup(rb->Label());


				ra->BuildPatternString(&pa);
				rb->BuildPatternString(&pb);
				

				fprintf(stderr, "ERROR: Overlap between label %s and %s:\n", la.c_str(), lb.c_str());
				fprintf(stderr, "ERROR: %s,\n", pa.c_str());
				fprintf(stderr, "ERROR: %s.\n", pb.c_str());
			}
		}
	}

	return result;
}


bool RuleSet::IsSimilar(RuleSet* set)
{
	unsigned int i;

	if (mChildren.size() != set->mChildren.size())
		return false;

	if (mChildren.size() == 0)
		return mLabel == set->mLabel;

	if (mDivType != set->mDivType)
		return false;

	if (mDivType == DIV_PATTERN)
	{
		if (mDivPattern.Mask() != set->mDivPattern.Mask())
			return false;
		if (mDivPattern.Signature() != set->mDivPattern.Signature())
			return false;
	}
	else if (mDivType == DIV_TABLE)
	{
		if (mDivTableStart != set->mDivTableStart)
			return false;
	
		if (mDivTableLength != set->mDivTableLength)
			return false;
	}
	else
	{
		return false;
	}


	for(i=0; i<mChildren.size(); i++)
	{
		RuleSet* childa = mChildren[i];
		RuleSet* childb = set->mChildren[i];

		if (childa->IsSimilar(childb) == false)
			return false;
	}

	return true;
}


void RuleSet::BuildNodeTable(std::vector<RuleSet*>* table)
{
	unsigned int i;


	table->push_back(this);

	for(i=0; i<mChildren.size(); i++)
	{
		RuleSet* child = mChildren[i];

		child->BuildNodeTable(table);
	}
}

void RuleSet::ReduceSimilarSubtrees()
{
	std::vector<RuleSet*> table;
	unsigned int i, j;


	BuildNodeTable(&table);

	for(i=0; i<table.size(); i++)
	{
		RuleSet* a = table[i];

		if (a->mRedirect)
			continue;

		for(j=i+1; j<table.size(); j++)
		{
			RuleSet* b = table[j];

			if (b->mRedirect)
				continue;

			if (a->IsSimilar(b) && a->mChildren.size())
			{
				//printf("Node %d similar to node %d.\n", a->mNode, b->mNode);

				// Node b will be redirected to node a.
				b->mRedirect = a;

				if (a->mDivType == DIV_PATTERN)
					a->mStub = true;
				//printf("Redirect %d to %d\n", b->mNode, a->mNode);
			}
		}
	}

	//RemoveRedirectedSubtrees();
}


float RuleSet::TestDivideByPattern(const Pattern& f)
{
	unsigned int left = 0;
	unsigned int right = 0;
	unsigned int partial = 0;
	unsigned int i;


	for(i=0; i<mRules.size(); i++)
	{
		Rule* rule = mRules[i];
		MatchResult match = RuleMatch(*rule, f);

		// rule is fully consumed by f
		if (match == MATCH_YES)
		{
			left++;
		}
		else if (match == MATCH_NO) // no subrule is even consumed by f
		{
			right++;
		}
		else // only some subrules are consumed by f
		{
			left++;
			right += bitcount( f.Mask() & ~rule->Mask() );
		}
	}

	// check if pattern is useless
	if (left == mRules.size() && right == 0)
		return 0.0;
	if (right == mRules.size() && left == 0)
		return 0.0;


	// calculate score
	unsigned int maxheight = left;
	if (maxheight < right)
		maxheight = right;

	unsigned int score = maxheight + left + right;
	//return score;

	float mr = (float)(left + right - 1);
	mr = mr / (mRules.size() - 1.0f);
//	printf("pattern mr %f\n", mr);
	mr = log(mr) / log(2.0f);
	return 1.0f + maxheight + mGamma * mr;
}


float RuleSet::TestDivideByTable(unsigned int bitstart, unsigned int bitlength)
{
	unsigned int mask;
	unsigned int fillnmask;
	unsigned int fillsig;
	unsigned int fillmask;
	unsigned int fillcount;
	unsigned int index;
//	unsigned int score;
	unsigned int sum = 0;
	unsigned int nonzero = 0;
	unsigned int maxheight = 0;
	unsigned int i;
	unsigned int k;
	unsigned int count;
	unsigned int* bucket;
	Rule r;


	count = (1<<bitlength);
	mask = (count-1) << bitstart;
	mScratchpad->AllocateBucket(count);

	for(i=0; i<mRules.size(); i++)
	{
		Rule* rule = mRules[i];


		// setup bitpattern generator for generating indices from rule
		fillmask = mask & (~rule->Mask());
		fillnmask = ~fillmask;
		fillsig = fillnmask;
		fillcount = 1<<bitcount(fillmask);

		r.SetLabel(rule->Label());

		// generate indices that are consumed by the given rule for the table
		for(k=0; k<fillcount; k++)
		{
			r.SetMask(mask | rule->Mask());
			r.SetSignature(rule->Signature() | (fillsig & fillmask));

			index = (r.Signature()&mask) >> bitstart;

			fillsig = (fillsig+1) | fillnmask;

			bucket = mScratchpad->Bucket(index);
			(*bucket)++;
		}
	}

	float mr = count + 1.0f;
	for(i=0; i<count; i++)
	{
		bucket = mScratchpad->Bucket(i);

		if (*bucket != 0)
		{
			nonzero++;
			mr += (*bucket - 1.0f);
			sum += *bucket;
		}

		if (maxheight < *bucket)
			maxheight = *bucket;
	}

	if (sum == mRules.size() && nonzero == 1) // table is useless
	{
		return 0.0f;
	}

	//score = sum + maxheight;
	//return score;
	
	mr = mr / (mRules.size() - 1.0f);
	//printf("table mr %f\n", mr);
	mr = log(mr) / log(2.0f);

	return 1.0f + maxheight + mGamma * mr;
}


bool RuleSet::DivideByPattern(unsigned int* idgen, const Pattern& f)
{
	unsigned int i;
	unsigned int j;
	unsigned int mask;
	unsigned int nmask;
	unsigned int sig;
	unsigned int smask;
	unsigned int tmask;
	unsigned int patterncount;
	Rule r;


	if (mScratchpad == 0)
	{
		fprintf(stderr, "ERROR: NO SCRATCHPAD\n");
		return false;
	}

	mChildren.reserve(2);

	RuleSet* left = new RuleSet;
	RuleSet* right = new RuleSet;

	mChildren.push_back(left);
	mChildren.push_back(right);

	left->SetScratchpad(mScratchpad);
	left->SetGamma(mGamma);

	right->SetScratchpad(mScratchpad);
	right->SetGamma(mGamma);


	for(i=0; i<mRules.size(); i++)
	{
		Rule* rule = mRules[i];
		MatchResult match = RuleMatch(*rule, f);

		if (match == MATCH_YES)
		{
			left->Add(*rule);
		}
		else if (match == MATCH_NO)
		{
			right->Add(*rule);
		}
		else
		{
			r.SetLabel( rule->Label() );

			// rule is split into exact matching rule and its complement.

			r.SetSignature( rule->Signature() | f.Signature() );
			r.SetMask( rule->Mask() | f.Mask() );

			// add exact matching rule to 'YES' subtree.
			left->Add(r);

			// generate complement of rule, and those to the 'NO' subtree.
			mask = f.Mask() & ~rule->Mask();
			nmask = ~mask;
			patterncount = bitcount( mask );
			tmask = mask & (nmask + 1);
			smask = rule->Mask() | f.Mask();
			sig = rule->Signature() | f.Signature();

			for(j=0; j<patterncount; j++)
			{
				r.SetSignature( (sig ^ tmask) & smask);
				r.SetMask(smask);

				smask ^= tmask;
				tmask = mask & (nmask + (tmask<<1));

				right->Add(r);
			}
		}
	}

	if (false == left->Divide(idgen))
		return false;
	if (false == right->Divide(idgen))
		return false;

	return true;
}



bool RuleSet::DivideByTable(unsigned int* idgen, unsigned int bitstart, unsigned int bitlength)
{
	unsigned int mask;
	unsigned int fillnmask;
	unsigned int fillsig;
	unsigned int fillmask;
	unsigned int fillcount;
	unsigned int index;
	unsigned int i;
	unsigned int k;
	unsigned int count;
	Rule r;

	if (mScratchpad == 0)
	{
		fprintf(stderr, "ERROR: NO SCRATCHPAD\n");
		return false;
	}

	count = (1<<bitlength);
	mask = (count-1) << bitstart;
	
	mChildren.reserve(count);

	for(i=0; i<count; i++)
	{
		RuleSet* set =  new RuleSet;

		mChildren.push_back(set);

		set->SetScratchpad(mScratchpad);
		set->SetGamma(mGamma);
		set->mStub = true;
	}


	for(i=0; i<mRules.size(); i++)
	{
		Rule* rule = mRules[i];


		fillmask = mask & (~rule->Mask());
		fillnmask = ~fillmask;
		fillsig = fillnmask;
		fillcount = 1<<bitcount(fillmask);

		r.SetLabel(rule->Label());
		
		// split rules if necessary by index and add them to each subtree
		
		for(k=0; k<fillcount; k++)
		{
			r.SetMask(mask | rule->Mask());
			r.SetSignature(rule->Signature() | (fillsig & fillmask));

			index = (r.Signature()&mask) >> bitstart;
			mChildren[index]->Add(r);

			fillsig = (fillsig+1) | fillnmask;
		}
	}

	for(i=0; i<mChildren.size(); i++)
	{
		RuleSet* set = mChildren[i];


		if (false == set->Divide(idgen))
			return false;
	}

	return true;
}

bool RuleSet::Divide()
{
	unsigned int idgen = 0;

	std::sort(mRules.begin(), mRules.end(), RuleSetSortPredicate);

	return Divide(&idgen);
}

bool RuleSet::Divide(unsigned int* idgen)
{
	Pattern minf;
	bool firstminscore = true;
	float minscore = 0.0f;
	unsigned int i;
	unsigned int k;
	unsigned int b;
	float score;
	unsigned int gmask = CalculateGlobalMask();
	unsigned int minmask = 0;
	unsigned int minsig = 0;
	unsigned int nextminmask;
	unsigned int nextminsig;
	unsigned int mask;
	unsigned int sig;
	unsigned int tests;
	unsigned int mintype = DIV_PATTERN;
	unsigned int minpos;
	unsigned int minlen;
	unsigned int len;

	bool more;
	Pattern f;

	if (mScratchpad == 0)
	{
		fprintf(stderr, "ERROR: NO SCRATCHPAD\n");
		return false;
	}

	mNode = (*idgen)++;

	Minimize();

	mLabel = CalculateLabel();
	if (mLabel != ~0)
		return true;


	// generate candidate division patterns by growing a one bit at a time if improvement is found
	minmask = 0;
	minsig = 0;
	minf.SetMask(0);
	minf.SetSignature(0);

	for(b=0; b<32; b++)
	{
		more = false;

		for(i=0; i<32; i++)
		{
			if ( (1<<i) & minmask )
				continue;

			if ( ((1<<i) & gmask) == 0 )
				continue;

			if (b == 0)
				tests = 1;
			else if (b == 1)
				tests = 4;
			else
				tests = 2;

			for(k=0; k<tests; k++)
			{
				if (b == 0)
				{
					mask = (1<<i);
					sig = (1<<i);
				}
				else if (b == 1)
				{
					mask = minmask | (1<<i);
					if (k == 0)
						sig = 0;
					else if (k == 1)
						sig = (1<<i);
					else if (k == 2)
						sig = minmask;
					else if (k == 3)
						sig = minmask | (1<<i);
				}
				else
				{
					mask = minmask | (1<<i);
					if (k == 0)
						sig = minsig;
					else
						sig = minsig | (1<<i);
				}

				// test with tsig/tmask
				f.SetSignature(sig & mask);
				f.SetMask(mask);

				score = TestDivideByPattern(f);
				if (score > 0.1f && (score < minscore || firstminscore))
				{
					firstminscore = false;
					mintype = DIV_PATTERN;
					minscore = score;
					minf = f;
					nextminmask = mask;
					nextminsig = sig;
					more = true;
				}
			}
		}

		if (!more)
			break;

		minmask = nextminmask;
		minsig = nextminsig;
	}

	// generate candidate division tables
	len = 2;
	more = true;

	while(more)
	{
		more = false;

		for(i=0; i<=32-len; i++)
		{
			mask = ((1<<len)-1) << i;
			
			if ((mask&gmask) == 0)
				continue;

			score = TestDivideByTable(i, len);
			if (score > 0.1f && (score < minscore || firstminscore))
			{
				firstminscore = false;
				mintype = DIV_TABLE;
				minscore = score;
				minpos = i;
				minlen = len;
				more = true;
			}
		}

		len++;
	}

	if (firstminscore)
	{
		fprintf(stderr, "ERROR: NO MINIMUM SCORE REACHED\n");
		return false;
	}

	mDivType = mintype;

	if (mDivType == DIV_PATTERN)
	{
		mDivPattern = minf;

		return DivideByPattern(idgen, mDivPattern);
	}
	else if (mDivType == DIV_TABLE)
	{
		mDivTableStart = minpos;
		mDivTableLength = minlen;

		return DivideByTable(idgen, mDivTableStart, mDivTableLength);
	}
	else
	{
		return false;
	}
}


void RuleSet::Print()
{
	unsigned int i;
	unsigned int mask;
	unsigned int sig;
	unsigned int label;
	std::string str;

	for(i=0; i<mRules.size(); i++)
	{
		Rule* rule = mRules[i];

		mask = rule->Mask();
		sig = rule->Signature();
		label = rule->Label();
		rule->BuildPatternString(&str);


		fprintf(stdout, "%08x, %08x, %s, %d\n", mask, sig, str.c_str(),  label);
	}
}

void RuleSet::SaveDot(const char* filepath, bool reduced)
{
	FILE* f = fopen(filepath, "wb");

	fprintf(f, "digraph g {\n");
	SaveDot(f, reduced);
	fprintf(f, "}\n");
	fclose(f);
}

void RuleSet::SaveDot(FILE* f, bool reduced)
{
	unsigned int i;


	if (mChildren.size() == 0)
	{
		fprintf(f, "n%d [label=\"%d\"];\n", mNode, mLabel);
	}
	else
	{
		if (mDivType == DIV_PATTERN)
		{
			bool first = true;
			fprintf(f, "n%d [label=\"%d-P(", mNode,mNode);
			for(i=0; i<32; i++)
			{
				if ( mDivPattern.Mask() & (1<<i) )
				{
					if (!first)
					{
						fprintf(f, ",");
					}
					first=false;
					if (mDivPattern.Signature() & (1<<i))
						fprintf(f, "%d", i);
					else
						fprintf(f, "!%d", i);
				}
			}
			fprintf(f, ")\"];\n");
		}
		else if (mDivType == DIV_TABLE)
		{
			fprintf(f, "n%d [label=\"%d-T(%d,%d)\"];\n", mNode, mNode, mDivTableStart, mDivTableLength);
		}
	}

	if (mDivType == DIV_PATTERN)
	{
		for(i=0; i<mChildren.size(); i++)
		{
			RuleSet* child = mChildren[i];

			if (reduced && child->mRedirect)
				child = child->mRedirect;

			if (i == 0)
				fprintf(f, "n%d -> n%d [label=\"Y\"];\n", mNode, child->mNode);
			else if (i == 1)
				fprintf(f, "n%d -> n%d [label=\"N\"];\n", mNode, child->mNode);
		}
	}
	else if (mDivType == DIV_TABLE)
	{
		for(i=0; i<mChildren.size(); i++)
		{
			RuleSet* child = mChildren[i];

			if (reduced && child->mRedirect)
				child = child->mRedirect;

			fprintf(f, "n%d -> n%d [label=\"%d\"];\n", mNode, child->mNode, i);
		}
	}

	for(i=0; i<mChildren.size(); i++)
	{
		RuleSet* child = mChildren[i];

		if (!reduced || child->mRedirect == 0)
		{
			child->SaveDot(f, reduced);
		}
	}
}



void RuleSet::Minimize()
{
	unsigned int j;
	unsigned int i;
	unsigned int mask;
	bool fixpoint = false;

	while(!fixpoint)
	{
		fixpoint = true;

		for(j=0; j<mRules.size(); j++)
		{
			Rule* a = mRules[j];

			i = j+1;
			while(i < mRules.size())
			{
				Rule* b = mRules[i];

				mask = (a->Signature() & a->Mask()) ^ (b->Signature() & b->Mask());

				if (a->Label() == b->Label() && a->Mask() == b->Mask() && mask && (mask&(mask-1))==0)
				{
					a->SetSignature( a->Signature() & b->Signature() );
					a->SetMask( a->Mask() ^ mask );
					mRules.erase( mRules.begin() + i );
					delete b;
					fixpoint = false;
				}
				else
				{
					i++;
				}
			}
		}
	}
}

void RuleSet::Export(LangExporter* exporter, LabelSet* labelset)
{
	std::vector<RuleSet*> queue;
	unsigned int i;


	queue.push_back(this);
	BuildExportQueue(&queue);

	if (false == exporter->Begin())
		return;


	for(i=0; i<labelset->Size(); i++)
	{
		exporter->VisitLabelPrototype(labelset->Lookup(i));
	}

	ExportStubPrototypes(exporter);
	ExportTables(exporter, labelset);


	for(i=0; i<queue.size(); i++)
	{
		RuleSet* set = queue[i];

		exporter->BeginStub(set->mNode);
		set->ExportNode(exporter, labelset, true);
		exporter->EndStub();
	}

	exporter->End();
}

void RuleSet::ExportStubPrototypes(LangExporter* exporter)
{
	unsigned int i;


	if (mChildren.size() == 0)
		return;

	if (mRedirect)
		return;


	if (mStub)
		exporter->VisitStubPrototype(mNode);

	for(i=0; i<mChildren.size(); i++)
	{
		RuleSet* child = mChildren[i];

		child->ExportStubPrototypes(exporter);
	}
}

void RuleSet::ExportTables(LangExporter* exporter, LabelSet* labelset)
{
	unsigned int i;


	if (mChildren.size() == 0)
		return;

	if (mRedirect)
		return;

	if (mDivType == DIV_TABLE)
	{
		exporter->BeginTable(mNode, mChildren.size());
		for(i=0; i<mChildren.size(); i++)
		{
			RuleSet* child = mChildren[i];

			if (child->mChildren.size() == 0)
				exporter->VisitLabelEntry(labelset->Lookup(child->mLabel));
			else if (child->mRedirect)
				exporter->VisitStubEntry(child->mRedirect->mNode);
			else
				exporter->VisitStubEntry(child->mNode);
		}
		exporter->EndTable();
	}

	for(i=0; i<mChildren.size(); i++)
	{
		RuleSet* child = mChildren[i];

		child->ExportTables(exporter, labelset);
	}
}



void RuleSet::ExportNode(LangExporter* exporter, LabelSet* labelset, bool root)
{
	if (mChildren.size() == 0)
	{
		exporter->VisitLabel(labelset->Lookup(mLabel));
	}
	else
	{
		if (mDivType == DIV_PATTERN)
		{
			RuleSet* left = mChildren[0];
			RuleSet* right = mChildren[1];


			if (root == false)
			{
				if (mStub)
				{
					exporter->VisitStub(mNode);
					return;
				}
				else if (mRedirect)
				{
					exporter->VisitStub(mRedirect->mNode);
					return;
				}
			}


			exporter->VisitPattern(mDivPattern.Mask(), mDivPattern.Signature());
			exporter->BeginTrueBranch();
			left->ExportNode(exporter, labelset, false);
			exporter->EndTrueBranch();
			exporter->BeginFalseBranch();
			right->ExportNode(exporter, labelset, false);
			exporter->EndFalseBranch();
		}
		else if (mDivType == DIV_TABLE)
		{
			if (mRedirect)
				exporter->VisitTable(mDivTableStart, (1<<mDivTableLength)-1, mRedirect->mNode);
			else
				exporter->VisitTable(mDivTableStart, (1<<mDivTableLength)-1, mNode);
		}
	}
}



void RuleSet::BuildExportQueue(std::vector<RuleSet*>* queue)
{
	unsigned int i;


	if (mChildren.size() == 0)
		return;


	if (mRedirect)
		return;

	if (mStub)
		queue->push_back(this);

	for(i=0; i<mChildren.size(); i++)
	{
		RuleSet* child = mChildren[i];

		child->BuildExportQueue(queue);
	}
}

