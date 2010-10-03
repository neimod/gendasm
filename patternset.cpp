#include "patternset.h"

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

void PatternSet::Clear()
{
	unsigned int i;

	if (mPatterns)
	{
		for(i=0; i<mPatterns->size(); i++)
		{
			Pattern* pat = (*mPatterns)[i];
			delete pat;
		}

		mPatterns->clear();
		delete mPatterns;
		mPatterns = 0;
	}
}

void PatternSet::Copy(const PatternSet& set)
{
	unsigned int i;


	Clear();

	if (set.mPatterns == 0)
		return;
	
	for(i=0; i<set.mPatterns->size(); i++)
	{
		Pattern* pat = (*set.mPatterns)[i];
		
		Add(*pat);
	}
}


void PatternSet::Add(const Pattern& pat)
{
	Pattern* p = new Pattern(pat);

	if (mPatterns == 0)
		mPatterns = new std::vector<Pattern*>();

	mPatterns->push_back(p);
}

void PatternSet::Add(const char* str)
{
	Pattern p;

	p.Parse(str);

	Add(p);
}


void PatternSet::Intersect(const PatternSet& a, const PatternSet& b)
{
	Clear();
	mPatterns = _Intersect(a, b);
}

void PatternSet::Intersect(const PatternSet& set)
{
	std::vector<Pattern*>* pats = _Intersect(*this, set);

	Clear();

	mPatterns = pats;
}

std::vector<Pattern*>* PatternSet::_Intersect(const PatternSet& a, const PatternSet& b)
{
	unsigned int j;
	unsigned int i;
	unsigned int mask;
	unsigned int sig;

	if (a.mPatterns == 0 || b.mPatterns == 0)
		return 0;

	std::vector<Pattern*>* set = new std::vector<Pattern*>();

	for(j=0; j<a.mPatterns->size(); j++)
	{
		Pattern* pa = (*a.mPatterns)[j];
		

		for(i=0; i<b.mPatterns->size(); i++)
		{
			Pattern* pb = (*b.mPatterns)[i];

			mask = pa->Mask() & pb->Mask();

			if ( (pa->Signature() & mask) == (pb->Signature() & mask) )
			{
				Pattern* p = new Pattern;

				mask = pa->Mask() | pb->Mask();
				sig = pa->Signature() | pb->Signature();

				p->SetSignature( sig & mask );
				p->SetMask( mask );

				set->push_back(p);
			}
		}
	}

	return set;
}

void PatternSet::Complement(const PatternSet& set)
{
	PatternSet tmp;
	unsigned int i;

	if (set.mPatterns == 0)
		return;

	for(i=0; i<set.mPatterns->size(); i++)
	{
		Pattern* p = (*set.mPatterns)[i];
		
		tmp.Clear();
		tmp.Complement(*p);

		if (i == 0)
		{
			Copy(tmp);
		}
		else
		{
			Intersect(tmp);
		}
	}

	Minimize();
}

void PatternSet::Complement(const Pattern& pat)
{
	unsigned int i;
	unsigned int mask = pat.Mask();
	unsigned int nmask = ~mask;
	unsigned int patterncount = bitcount(mask);
	unsigned int tmask = mask & (nmask + 1);
	unsigned int smask = mask;


	for(i=0; i<patterncount; i++)
	{
		Pattern p;

		p.SetSignature( (pat.Signature() ^ tmask) & smask);
		p.SetMask(smask);
		//p.Print();

		smask ^= tmask;
		tmask = mask & (nmask + (tmask<<1));

		Add(p);
	}

}


void PatternSet::Print()
{
	unsigned int i;

	if (mPatterns == 0)
		return;

	for(i=0; i<mPatterns->size(); i++)
	{
		Pattern* p = (*mPatterns)[i];

		p->Print();
	}
}

void PatternSet::Minimize()
{
	unsigned int k;
	unsigned int j;
	unsigned int i;
	unsigned int mask;


	for(k=0; k<32; k++)
	{
		for(j=0; j<mPatterns->size(); j++)
		{
			Pattern* a = (*mPatterns)[j];

			i = j+1;
			while(i < mPatterns->size())
			{
				Pattern* b = (*mPatterns)[i];

				mask = (a->Signature() & a->Mask()) ^ (b->Signature() & b->Mask());

				if (a->Mask() == b->Mask() && mask && mask == (1<<k))
				{
					a->SetSignature( a->Signature() & b->Signature() );
					a->SetMask( a->Mask() ^ mask );
					mPatterns->erase( mPatterns->begin() + i );
					delete b;
				}
				else
				{
					i++;
				}
			}
		}
	}
}
