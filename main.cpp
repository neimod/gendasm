#include <stdio.h>
#include "pattern.h"
#include "ruleset.h"
#include "scratchpad.h"
#include "specfile.h"
#include "CSrcExporter.h"

int main(int argc, char* argv[])
{
	RuleSet ruleset;
	LabelSet labelset;
	Scratchpad scratchpad;
	SpecFile specfile;
	CSrcExporter csrcexporter;

	specfile.Load("armv4.def");

	ruleset.SetGamma(9.0f);
	ruleset.SetScratchpad(&scratchpad);
	specfile.LoadRules(&ruleset, &labelset);

	if (ruleset.CheckOverlap(&labelset))
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


	csrcexporter.SetOutput("src.c");
	csrcexporter.SetSpecFile(&specfile);

	ruleset.Export(&csrcexporter, &labelset);
	
	return 0;
}


