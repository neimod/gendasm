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

	for(i=0; i<mPatterns.size(); i++)
	{
		Pattern* pat = mPatterns[i];
		delete pat;
	}

	mPatterns.clear();
}

void PatternSet::Copy(const PatternSet& set)
{
	unsigned int i;


	Clear();
	
	for(i=0; i<set.mPatterns.size(); i++)
	{
		Pattern* pat = set.mPatterns[i];
		
		Add(*pat);
	}
}


void PatternSet::Add(const Pattern& pat)
{
	Pattern* p = new Pattern(pat);


	mPatterns.push_back(p);
}

void PatternSet::Intersect(const PatternSet& set)
{
	PatternSet tmp;

	tmp.Intersect(*this, set);
	
	tmp.mPatterns.swap(mPatterns);
}

void PatternSet::Intersect(const PatternSet& a, const PatternSet& b)
{
	unsigned int j;
	unsigned int i;
	unsigned int mask;
	unsigned int sig;


	Clear();


	for(j=0; j<a.mPatterns.size(); j++)
	{
		Pattern* pa = a.mPatterns[j];
		

		for(i=0; i<b.mPatterns.size(); i++)
		{
			Pattern* pb = b.mPatterns[i];

			mask = pa->Mask() & pb->Mask();

			if ( (pa->Signature() & mask) == (pb->Signature() & mask) )
			{
				Pattern* p = new Pattern;

				mask = pa->Mask() | pb->Mask();
				sig = pa->Signature() | pb->Signature();

				p->SetSignature( sig & mask );
				p->SetMask( mask );

				mPatterns.push_back(p);
			}
		}
	}
}

void PatternSet::Complement(const PatternSet& set)
{
	unsigned int i;
	PatternSet tmp;


	for(i=0; i<set.mPatterns.size(); i++)
	{
		Pattern* p = set.mPatterns[i];
		
		if (i == 0)
		{
			Complement(*p);
		}
		else
		{
			tmp.Complement(*p);
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


	Clear();

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


	for(i=0; i<mPatterns.size(); i++)
	{
		Pattern* p = mPatterns[i];

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
		for(j=0; j<mPatterns.size(); j++)
		{
			Pattern* a = mPatterns[j];

			i = j+1;
			while(i < mPatterns.size())
			{
				Pattern* b = mPatterns[i];

				mask = (a->Signature() & a->Mask()) ^ (b->Signature() & b->Mask());

				if (a->Mask() == b->Mask() && mask && mask == (1<<k))
				{
					a->SetSignature( a->Signature() & b->Signature() );
					a->SetMask( a->Mask() ^ mask );
					mPatterns.erase( mPatterns.begin() + i );
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

bool PatternSet::HasOverlap() const
{
	unsigned int i;
	unsigned int j;
	unsigned int commonmask;


	for(i=0; i<mPatterns.size(); i++)
	{
		Pattern* pa = mPatterns[i];


		for(j=i+1; j<mPatterns.size(); j++)
		{
			Pattern* pb = mPatterns[j];

			commonmask = pa->Mask() & pb->Mask();

			if ( (pa->Signature() & commonmask) == (pb->Signature() & commonmask) )
			{
				return true;
			}
		}
	}

	return false;
}


bool PatternSet::Expand(const PatternSet& set)
{
	unsigned int i, j;
	PatternSet tmp;
	Pattern pn;


	// for each pattern in *this, merge with all patterns in set
	for(i=0; i<mPatterns.size(); i++)
	{
		Pattern* pa = mPatterns[i];

		for(j=0; j<set.mPatterns.size(); j++)
		{
			Pattern* pb = set.mPatterns[j];

			if (pa->Mask() & pb->Mask())
				return false;

			pn.SetSignature(pa->Signature() | pb->Signature());
			pn.SetMask(pa->Mask() | pb->Mask());

			tmp.Add(pn);
		}
	}

	tmp.mPatterns.swap(mPatterns);

	return true;
}
