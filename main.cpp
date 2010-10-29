#include <stdio.h>
#include "pattern.h"
#include "ruleset.h"
#include "scratchpad.h"
#include "specfile.h"
#include "CSrcExporter.h"

void usage()
{
	fprintf(stdout, "Usage: <-i file> <-o file> [-d]\n\n");
	fprintf(stdout, "  -i  input spec file (required)\n");
	fprintf(stdout, "  -o  output src file (required)\n");
	fprintf(stdout, "  -d  output debug files (optional)\n");
}

int main(int argc, const char* argv[])
{
	SpecFile specfile;
	char input[256];
	char output[256];
	bool debug = false;
	int i;
	

	memset(input, 0, sizeof(input));
	memset(output, 0, sizeof(output));

	printf("argc = %d\n", argc);

	for(i=1; i<argc; i++)
	{
		const char* arg = argv[i];

		if (arg[0]!='-')
			continue;

		switch(arg[1])
		{
			case 'i':
				if (argc > i+1)
					strncpy(input, argv[i+1], sizeof(input)-1);
				break;
			case 'o':
				if (argc > i+1)
					strncpy(output, argv[i+1], sizeof(output)-1);
				break;
			case 'd':
				debug = true;
				break;

			default:
				fprintf(stderr, "WARNING: Unknown option \"%s\"\n", arg);
				break;
		}
	}

	if (strlen(input) == 0)
	{
		usage();

		fprintf(stderr, "ERROR: No input spec file\n");

		return -1;
	}
	else if (strlen(output) == 0)
	{
		usage();

		fprintf(stderr, "ERROR: No output file\n");
	}
	else if (false == specfile.Load(input))
	{
		return -1;
	}

	if (debug)
	{
		specfile.SaveDot("tree.dot", true);
		specfile.SaveDot("treeraw.dot", false);
	}

	specfile.Export(output);
	
	return 0;
}


