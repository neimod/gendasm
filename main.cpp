#include <stdio.h>
#include "pattern.h"
#include "ruleset.h"
#include "scratchpad.h"
#include "specfile.h"
#include "CSrcExporter.h"

int main(int argc, char* argv[])
{
	//RuleSet ruleset;
	//LabelSet labelset;
	//Scratchpad scratchpad;
	SpecFile specfile;
	//CSrcExporter csrcexporter;

	//specfile.Load("armv4.def");
	if (false == specfile.Load("arm.xml"))
	{
		return -1;
	}


	specfile.SaveDot("tree.dot");

	specfile.Export("src.c");
	
	return 0;
}


