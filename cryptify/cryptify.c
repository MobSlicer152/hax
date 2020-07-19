#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* malloc */
#include <string.h> /* strerror */
#include <errno.h> /* errno */
#include <crypt.h> /* crypt */

int main(int argc, char *argv[])
{
	char *buf;

	if (argc < 3) {
		printf("Usage: %s <text> <salt>\n", argv[0]);
		exit(1);
	}

	buf = crypt(argv[1], argv[2]);
	if (errno != 0) {
		fprintf(stderr, "%s: unable to encrypt text: %s\n", argv[0], strerror(errno));
		exit(1);
        }

	printf("%s: returned encrypted text was \'%s\'\n", argv[0], buf);

	return 0;
}
