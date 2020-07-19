#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#include <fcntl.h>
#include <io.h>
#define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#define SET_BINARY_MODE(file)
#endif

#define CHUNK 256 * 1024

/*
 * Compress from src to dest until EOF is encountered in src.
 * Returns zlib-defined values for the same reasons defined in
 * the official zlib usage guide (https://zlib.net/zlib_how.html)
 */
int def(FILE *src, FILE *dest, int level);

/*
 * Decompresses from src to dest until EOF is encountered in src.
 * As with def(), returns the zlib-defined values for the reasons
 * defined in https://zlib.net/zlib_how.html.
 */
int inf(FILE *src, FILE *dest);

/* Prints an error message based on the return value passed */
void get_zlib_err(int ret, char *prog_name);

int main(int argc, char *argv[])
{
	int ret, i, j;

	/* Prevent Microsoft's dumb conventions from messing up the data */
	SET_BINARY_MODE(stdin);
	SET_BINARY_MODE(stdout);

	/* Handle commandline stuff */
	if (argc == 2 && strcmp(argv[1], "-c") == 0) {/* Compress stdin to stdout */
		ret = def(stdin, stdout, Z_DEFAULT_COMPRESSION);
		if (ret != Z_OK)
			get_zlib_err(ret, argv[0]);

		return ret;
	} else if (argc == 2 && strcmp(argv[1], "-d") == 0) { /* Decompress stdin to stdout */
		ret = inf(stdin, stdout);
		if (ret != Z_OK)
			get_zlib_err(ret, argv[0]);
		
		return ret;
	} else { /* Print usage */
		/* Print an arrow with '[DE]COMPRESSION' over it pointing to the '[-c|-d]' part of the usage message */
		for (i = 0; i < (strlen(argv[0]) + strlen("Usage:") + 5) - (strlen("[DE]COMPRESSION") / 2); i++)
			printf(" ");

		printf("[DE]COMPRESSION\n");

		for (i = 0; i < 3; i++) {
			for (j = 0; j < (strlen(argv[0]) + strlen("Usage:") + 5); j++)
				printf(" ");

			if (i <= 1)
				printf("|\n");
			else
				printf("V\n\n");
			
		}
		
		printf("Usage: %s [-c|-d] < source > dest\n", argv[0]);
		
		return 1;
	}

	return 0;
}

int def(FILE *src, FILE *dest, int level)
{
	int ret, flush;
	unsigned int have;
	z_stream stream;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	/* Set up the zlib state */
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;

	/* Initialize and return an error code in the event that initialization fails */
	ret = deflateInit(&stream, level);
	if (ret != Z_OK)
		return ret;

	/* Compress to the end of the file */
	do {
		/* Read the current chunk of the file */
		stream.avail_in = fread(in, 1, CHUNK, src);
		
		/* Return cleanly in the event of an error */
		if (ferror(src)) {
			(void)deflateEnd(&stream);
			return Z_ERRNO;
		}

		/* Tell zlib whether this is the end of the input */
		flush = (feof(src)) ? Z_FINISH : Z_NO_FLUSH;

		/* Hand the buffer to zlib now that it's ready */
		stream.next_in = in;

		/* 
		 * Call deflate() on the input buffer until it stops producing output,
		 * finish if all of the source has been read in
		 */
		do {
			/* Tell zlib where to store its output and how much space is there */
			stream.avail_out = CHUNK;
			stream.next_out = out;

			ret = deflate(&stream, flush); /* Actually deflate the data */
			assert(ret != Z_STREAM_ERROR); /* Make sure the zlib state is intact */

			have = CHUNK - stream.avail_out; /* Calculate how much data we have */
			
			/* Write the data, and avoid a memory leak and return if that fails */
			if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
				(void)deflateEnd(&stream);
				return Z_ERRNO;
			}
		} while (stream.avail_out == 0);
		assert(stream.avail_in == 0); /* All input will be used */
	} while (flush != Z_FINISH); /* Finish when all data is processed */
	assert(ret == Z_STREAM_END); /* Stream will be complete */

	/* Prevent a (as put in the official zlib guide) memory hemorrhage, then clean up */
	(void)deflateEnd(&stream);
	return Z_OK;
}

int inf(FILE *src, FILE *dest)
{
	int ret;
	unsigned int have;
	z_stream stream;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	/* Set up the zlib state */
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.avail_in = 0;
	stream.next_in = Z_NULL;

	ret = inflateInit(&stream);
	if (ret != Z_OK)
		return ret;

	/* Decompress until the stream ends or EOF */
	do {
		/* Read the current chunk of the file */
		stream.avail_in = fread(in, 1, CHUNK, src);

		/* Avoid a memory leak and return in the event of an error */
		if (ferror(src)) {
			(void)inflateEnd(&stream);
			return Z_ERRNO;
		}

		/* Break if no input is available */
		if (stream.avail_in == 0)
			break;

		/* Hand zlib the buffer now that it's ready */
		stream.next_in = in;

		/* Run inflate() on the input until the output buffer is only partially full */
		do {
			/* Tell zlib where to store its output and how much space is there */
			stream.avail_out = CHUNK;
			stream.next_out = out;

			ret = inflate(&stream, Z_NO_FLUSH); /* Actually inflate the data */
			assert(ret != Z_STREAM_ERROR); /* Make sure the zlib state is intact */

			/* Return cleanly if there's an error */
			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR; /* If a dictionary is needed, say the data is invalid */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&stream);
			}

			/* Calculate the amount of data we have */
			have = CHUNK - stream.avail_out;

			/* Write the data and clean up if there's an error  */
			if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
				(void)inflateEnd(&stream);
				return Z_ERRNO;
			}
		
		/* Finish when the out buffer is only partially full */
		} while (stream.avail_out == 0);
	
	/* Finish when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* Clean up and return */
	(void)inflateEnd(&stream);
	return (ret == Z_STREAM_END) ? Z_OK : Z_DATA_ERROR;
}

void get_zlib_err(int ret, char *prog_name)
{
	/* Print the name of the program for clarity */
	fprintf(stderr, "%s: ", prog_name);

	/* Now print a message to indicate what kind of error occured */
	switch (ret) {
	case Z_ERRNO:
		fprintf(stderr, "I/O error");
		break;
	case Z_STREAM_ERROR:
		fprintf(stderr, "zlib state invalid, likely caused by"
		" an invalid decompression level (should be -1-9)");
		break;
	case Z_DATA_ERROR:
		fprintf(stderr, "invalid or incomplete deflate data");
		break;
	case Z_MEM_ERROR:
		fprintf(stderr, "out of memory");
		break;
	case Z_VERSION_ERROR:
		fprintf(stderr, "zlib version mismatch");
	}

	/* Print a newline once instead of typing it at the end of each message */
	fprintf(stderr, "\n");
}
