#ifndef _LABELSET_H_
#define _LABELSET_H_

#include <vector>

class LabelSet
{
public:
	LabelSet() {}
	~LabelSet() { Clear(); }

	const std::string& Lookup(unsigned int label) const;
	unsigned int Find(const char* name) const;
	unsigned int Add(const char* name);
	void Clear();

private:
	typedef struct
	{
		unsigned int label;
		std::string name;
	} Node;

	std::vector<Node*> mNodes;
};

#endif // _LABELSET_H_