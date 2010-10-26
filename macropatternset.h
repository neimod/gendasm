#ifndef _MACROPATTERNSET_H_
#define _MACROPATTERNSET_H_

#include <vector>
#include "patternset.h"


class MacroPatternSet
{
public:
	MacroPatternSet() {}
	~MacroPatternSet() { Clear(); }

	void Print() const;
	PatternSet* Find(const char* name) const;
	PatternSet* Add(const char* name);
	
	void Clear();
	unsigned int Size() const { return mNodes.size(); }

private:
	typedef struct
	{
		PatternSet macropattern;
		std::string name;
	} Node;

	std::vector<Node*> mNodes;
};

#endif // _MACROPATTERNSET_H_