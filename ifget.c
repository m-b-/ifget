#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getmac(int, char *[], char *, int);
int getip(int, char *[], char *, int);
int getmask(int, char *[], char *, int);
int getcidr(int, char *[], char *, int);
int getap(int, char *[], char *, int);
int getbcast(int, char *[], char *, int);

struct function {
	char *name;
	int (*fn)(int, char *[], char *, int);
	char *help;
} functions[] = {
	{ "mac",       getmac,       "<interface>"      },
	{ "ip",        getip,        "[-6] <interface>" },
	{ "mask",      getmask,      "[-6] <interface>" },
	{ "cidr",      getcidr,      "[-6] <interface>" },
	{ "ap",        getap,        "[-6] <interface>" },
	{ "bcast",     getbcast,     "<interface>"      },
	{ "",          NULL,         ""},
};

void
help(char *argv0)
{
	int i;

	for (i = 0; functions[i].fn != NULL; i++)
		fprintf(stderr, "%s %5s %16s\n", argv0,
				functions[i].name, functions[i].help);
}

int
lookupfn(char *name)
{
	int i;

	for (i = 0; functions[i].fn != NULL; i++)
		if (strcmp(name, functions[i].name) == 0)
			return i;

	return -1;
}

int
main(int argc, char *argv[])
{
	char buf[128];
	int fn, r;

	if (argc < 2 || strncmp(argv[1], "-h", 2) == 0
			|| strcmp(argv[1], "help") == 0) {
		help(argv[0]);
		return 0;
	}

	fn = lookupfn(argv[1]);
	if (fn == -1) {
		fprintf(stderr, "error: unknown command '%s'\n", argv[1]);
		return -1;
	}

	memset(buf, '\0', sizeof(buf));

	r = functions[fn].fn(argc-1, argv+1, buf, sizeof(buf));
	if (r != -1)
		puts(buf);

	return r;
}
