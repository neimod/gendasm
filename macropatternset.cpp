#include "macropatternset.h"

void MacroPatternSet::Clear()
{
	unsigned int i;


	for(i=0; i<mNodes.size(); i++)
	{
		Node* node = mNodes[i];

		delete node;
	}

	mNodes.clear();
}

void MacroPatternSet::Print() const
{
	unsigned int i;


	for(i=0; i<mNodes.size(); i++)
	{
		Node* node = mNodes[i];

		printf("%s:\n", node->name.c_str());
		node->macropattern.Print();
	}
}

	
PatternSet* MacroPatternSet::Add(const char* name)
{
	Node* node = new Node;

	node->name.assign(name);

	mNodes.push_back(node);

	return &node->macropattern;
}

PatternSet* MacroPatternSet::Find(const char* name) const
{
	unsigned int i;


	for(i=0; i<mNodes.size(); i++)
	{
		Node* node = mNodes[i];

		if (node->name.compare(name) == 0)
			return &node->macropattern;
	}

	return 0;
}
