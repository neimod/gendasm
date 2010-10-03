#ifndef _PATTERNSET_H_
#define _PATTERNSET_H_

#include <vector>
#include "pattern.h"

class PatternSet
{
public:
	PatternSet(): mPatterns(0) {}
	~PatternSet() { Clear(); }

	bool Load(const char* filepath);
	void Copy(const PatternSet& set);
	void Clear();
	void Add(const Pattern& pat);
	void Add(const char* str);
	void Intersect(const PatternSet& a, const PatternSet& b);
	void Intersect(const PatternSet& set);
	void Print(void);
	void Complement(const PatternSet& pat);
	void Complement(const Pattern& pat);

	Pattern* Lookup(unsigned int i) const { return (*mPatterns)[i]; }
	unsigned int Size() const { return mPatterns? mPatterns->size() : 0; }

private:
	void Minimize(void);
	std::vector<Pattern*>* _Intersect(const PatternSet& a, const PatternSet& b);

private:
	std::vector<Pattern*>* mPatterns;
};

#endif // _PATTERNSET_H_
