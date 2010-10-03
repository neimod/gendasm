#include <stdio.h>
#include "pattern.h"
#include "ruleset.h"
#include "scratchpad.h"

int main(int argc, char* argv[])
{
	RuleSet ruleset;
	LabelSet labelset;
	Scratchpad scratchpad;

	ruleset.SetScratchpad(&scratchpad);
	ruleset.Load("armv4.txt", &labelset);

	if (ruleset.CheckOverlap(labelset))
	{
		fprintf(stderr, "Caught error, stopping.\n");
		return -1;
	}

	ruleset.CalculateUndefined(&labelset);


	if (false == ruleset.Divide())
	{
		fprintf(stderr, "Caught error, stopping.\n");
		return -1;
	}

	ruleset.SaveDot("tree.dot");

	
	return 0;
}


