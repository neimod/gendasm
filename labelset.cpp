#include "labelset.h"

void LabelSet::Clear()
{
	unsigned int i;


	for(i=0; i<mNodes.size(); i++)
	{
		Node* node = mNodes[i];

		delete node;
	}

	mNodes.clear();
}


unsigned int LabelSet::Add(const char* name)
{
	Node* node = new Node;

	node->name.assign(name);
	node->label = mNodes.size();

	mNodes.push_back(node);

	return node->label;
}

unsigned int LabelSet::Find(const char* name) const
{
	unsigned int i;


	for(i=0; i<mNodes.size(); i++)
	{
		Node* node = mNodes[i];

		if (node->name.compare(name) == 0)
			return node->label;
	}

	return ~0;
}

const std::string& LabelSet::Lookup(unsigned int label) const
{
	return mNodes[label]->name;
}

