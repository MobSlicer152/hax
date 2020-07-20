#include <stdio.h> /* For fprintf, fopen, FILE, printf, and sprintf */
#include <stdlib.h> /* For malloc, exit, and size_t */
#include <string.h> /* For strlen, strstr, strcpy, strcat, and strerror */
#include <math.h> /* For pow */
#include <errno.h> /* For errno */

void strreplace(char *dest, char *substr, char *replace);

int main(int argc, char *argv[])
{
	char *spambuf, tmp[128], spamstr[128], *j;
	int i;
	size_t len;
	char *usage;
	FILE *fp;

	/* Allocate a buffer and store the usage message in it */
	usage = (char *)malloc(256);
	if (usage == NULL) {
		fprintf(stderr, "\a%s: unable to allocate memory\n", argv[0]);
		exit(1);
	}

	sprintf(usage, "Usage: %s <spam to print> <number of times to print spam> [<where to save the spam>]\n", argv[0]);

	/* Handle arguments */
	if (argc < 3) {
		printf("%s", usage);
		exit(1);
	} else if (atoi(argv[2]) > pow(2, 32) - 1) {
		fprintf(stderr, "\a%s: invalid argument \'%s\': too large\n", argv[0], argv[2]);
		exit(1);
	} else if (atoi(argv[2]) < 1) {
		fprintf(stderr, "\a%s: invalid argument \'%s\': too small\n", argv[0], argv[2]);
		exit(1);
	} else if (strlen(argv[1]) > 128) {
		fprintf(stderr, "\a%s: invalid argument \'%s\': too long\n", argv[0], argv[1]);
		exit(1);
	} else if (argc > 3) {
		printf("%s: saving output to \'%s\'\n", argv[0], argv[3]);

		fp = fopen(argv[3], "w+");
		if (fp == NULL) {
			fprintf(stderr, "\a%s: error opening file \'%s\': %s\n", argv[0], argv[3], strerror(errno));
			exit(1);
		}
	}

	/* Copy the string into spamstr and replace characters with their real ASCII codes */
	strcpy(spamstr, argv[1]);

	strreplace(spamstr, "\\n", "\n");
	strreplace(spamstr, "\\t", "\t");
	strreplace(spamstr, "\\v", "\v");
	strreplace(spamstr, "\\a", "\a");
	strreplace(spamstr, "\\b", "\b");
	strreplace(spamstr, "\\r", "\r");
	strreplace(spamstr, "\\f", "\f");

	/* Calculate length in bytes of the string */
	len = (strlen(spamstr) * sizeof(char) * atoi(argv[2])) + 1;

	/* Allocate a buffer */
	spambuf = (char *)malloc(len);
	if (spambuf == NULL) {
		fprintf(stderr, "\a%s: unable to allocate a buffer\n", argv[0]);
		exit(1);
	}

	/* Print the spam the specified number of times */
	sprintf(spambuf, "%s", spamstr);
	for (i = 1; i < atoi(argv[2]); i++) {
		if (argc > 3) {
			fprintf(fp, "%s", spambuf);
			printf("\r%3.2f%%", ((float)i / atof(argv[2]) * 100)); /* Print the percentage in the same place every time */
			fflush(stdout);
		}
		else {
			printf("%s", spambuf);
		}
	}

	if (argc > 3)
		printf("\n");

	return 0;
}

void strreplace(char *dest, char *substr, char *replace)
{
	char tmp[128];
	char *p = dest;
	while ((p = strstr(p, substr))) {
		strncpy(tmp, dest, p - dest);
		tmp[p - dest] = '\0';
		strcat(tmp, replace);
		strcat(tmp, p + strlen(substr));
		strcpy(dest, tmp);
	}
}