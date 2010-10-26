#ifndef _LABELSET_H_
#define _LABELSET_H_

#include <vector>

class LabelSet
{
public:
	LabelSet();
	~LabelSet();

	const std::string& Lookup(unsigned int label) const;
	unsigned int Find(const char* name) const;
	unsigned int Add(const char* name);
	void Clear();
	unsigned int Size() const { return mNodes.size(); }

	void SetNullsub(const std::string& nullsub) { mNullsub = nullsub; }

private:
	typedef struct
	{
		unsigned int label;
		std::string name;
	} Node;

	std::vector<Node*> mNodes;
	std::string mNullsub;
};

#endif // _LABELSET_H_