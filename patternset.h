#ifndef _PATTERNSET_H_
#define _PATTERNSET_H_

#include <vector>
#include "pattern.h"

class PatternSet
{
public:
	PatternSet() {}
	~PatternSet() { Clear(); }

	bool Load(const char* filepath);
	void Copy(const PatternSet& set);
	void Clear();
	void Add(const Pattern& pat);
	void Add(const char* str);
	void Intersect(const PatternSet& set);
	void Print(void);
	void Complement(const PatternSet& set);
	void Complement(const Pattern& pat);
	void Minimize(void);
	bool HasOverlap() const;
	bool Expand(const PatternSet& set);

	Pattern* Lookup(unsigned int i) const { return mPatterns[i]; }
	unsigned int Size() const { return mPatterns.size(); }

private:
	
	void Intersect(const PatternSet& a, const PatternSet& b);

private:
	std::vector<Pattern*> mPatterns;
};

#endif // _PATTERNSET_H_
