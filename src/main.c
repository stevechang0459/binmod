#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

#include "global.h"
#include "types.h"
#include "list.h"
#include "utility.h"

#define BINMOD_VERSION                  "1.0.0"

#define CONFIG_GETOPT_DEBUG             (0)
#define CONFIG_MAIN_DEBUG               (0)

#define FILE_NAME_MAX                   256
#define DEV_NAME_MAX                    256

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;
	FILE *src_fp = NULL;
	FILE *dst_fp = NULL;
	char buf[128];
	char *end;
	char in_file[FILE_NAME_MAX];
	char out_file[FILE_NAME_MAX];
	char *s1, *s2;
	unsigned long long v;
	int i;
	size_t n;

	struct binmod_cfg {
		size_t src_ofs;
		size_t src_len;
		size_t dst_ofs;
		size_t dst_len;
		bool in_file;
		bool out_file;
		bool help;
	};

	struct binmod_cfg cfg = {
		.in_file                = false,
		.out_file               = false,
		.src_ofs                = SIZE_MAX,
		.src_len                = SIZE_MAX,
		.dst_ofs                = SIZE_MAX,
		.dst_len                = SIZE_MAX,
		.help                   = 0,
	};

	int opt;
	/* handle (optional) flags first */
	while ((opt = getopt(argc, argv, "i:o:s:d:v")) != -1) {
#if (CONFIG_GETOPT_DEBUG)
		printf("opt: %d,%d,%c,%s\n", optind, argc, (char)opt, optarg);
#endif
		switch (opt) {
		case 'i':
			cfg.in_file = true;
			memset(in_file, 0, sizeof(in_file));
			// memcpy(in_file, optarg, strlen(optarg));
			snprintf(in_file,  sizeof(in_file),  "%s", optarg);
			// printf("in_file[%zu]: %s\n", strlen(in_file), in_file);
			break;
		case 'o':
			cfg.out_file = true;
			memset(out_file, 0, sizeof(out_file));
			// memcpy(out_file, optarg, strlen(optarg));
			snprintf(out_file, sizeof(out_file), "%s", optarg);
			// printf("out_file[%zu]: %s\n", strlen(out_file), out_file);
			break;
		case 's':
			i = snprintf(buf, sizeof(buf), "%s", optarg);
			if (i < 0 || (size_t)i >= sizeof(buf)) {
				fprintf(stderr, "src_ofs argument too long\n");
				exit(EXIT_FAILURE);
			}
			s1 = buf;
			s2 = strchr(s1, ',');
			if (s2 == NULL || s2[1] == '\0') {
				fprintf(stderr, "Invalid src_len format -s<src_ofs>,<src_len>: %s\n", buf);
				exit(EXIT_FAILURE);
			}
			*s2++ = '\0';
			if (*s1 == '\0') {
				fprintf(stderr, "Invalid src_ofs format -s<src_ofs>,<src_len>: %s\n", buf);
				exit(EXIT_FAILURE);
			}

			if (*s1 == '-') {
				fprintf(stderr, "Invalid src_ofs %s\n", s1);
				exit(EXIT_FAILURE);
			}
			errno = 0;
			end = NULL;
			v = strtoull(s1, &end, 0);
			if (errno == ERANGE || s1 == end || *end != '\0' || v > (unsigned long long)SIZE_MAX) {
				fprintf(stderr, "Invalid src_ofs %s: %s (%d)\n", s1, strerror(errno), errno);
				exit(EXIT_FAILURE);
			}
			cfg.src_ofs = (size_t)v;
			if (*s2 == '-') {
				fprintf(stderr, "Invalid src_len %s\n", s2);
				exit(EXIT_FAILURE);
			}
			errno = 0;
			end = NULL;
			v = strtoull(s2, &end, 0);
			if (errno == ERANGE || s2 == end || *end != '\0' || v > (unsigned long long)SIZE_MAX) {
				fprintf(stderr, "Invalid src_len %s: %s (%d)\n", s2, strerror(errno), errno);
				exit(EXIT_FAILURE);
			}
			cfg.src_len = (size_t)v;
			// printf("src_ofs[%zu]: %s, %zu\n", strlen(s1), s1, cfg.src_ofs);
			// printf("src_len[%zu]: %s, %zu\n", strlen(s2), s2, cfg.src_len);
			break;
		case 'd':
			i = snprintf(buf, sizeof(buf), "%s", optarg);
			if (i < 0 || (size_t)i >= sizeof(buf)) {
				fprintf(stderr, "dst_ofs argument too long\n");
				exit(EXIT_FAILURE);
			}
			s1 = buf;
			s2 = strchr(s1, ',');
			if (s2 == NULL || s2[1] == '\0') {
				fprintf(stderr, "Invalid dst_len format -d<dst_ofs>,<dst_len>: %s\n", buf);
				exit(EXIT_FAILURE);
			}
			*s2++ = '\0';
			if (*s1 == '\0') {
				fprintf(stderr, "Invalid dst_ofs format -d<dst_ofs>,<dst_len>: %s\n", buf);
				exit(EXIT_FAILURE);
			}

			if (*s1 == '-') {
				fprintf(stderr, "Invalid dst_ofs %s\n", s1);
				exit(EXIT_FAILURE);
			}
			errno = 0;
			end = NULL;
			v = strtoull(s1, &end, 0);
			if (errno == ERANGE || s1 == end || *end != '\0' || v > (unsigned long long)SIZE_MAX) {
				fprintf(stderr, "Invalid dst_ofs %s: %s (%d)\n", s1, strerror(errno), errno);
				exit(EXIT_FAILURE);
			}
			cfg.dst_ofs = (size_t)v;
			if (*s2 == '-') {
				fprintf(stderr, "Invalid dst_len %s\n", s2);
				exit(EXIT_FAILURE);
			}
			errno = 0;
			end = NULL;
			v = strtoull(s2, &end, 0);
			if (errno == ERANGE || s2 == end || *end != '\0' || v > (unsigned long long)SIZE_MAX) {
				fprintf(stderr, "Invalid dst_len %s: %s (%d)\n", s2, strerror(errno), errno);
				exit(EXIT_FAILURE);
			}
			cfg.dst_len = (size_t)v;
			// printf("dst_ofs[%zu]: %s, %zu\n", strlen(s1), s1, cfg.dst_ofs);
			// printf("dst_len[%zu]: %s, %zu\n", strlen(s2), s2, cfg.dst_len);
			break;
		// case 'h':
		//      cfg.help = 1;
		//      break;
		case 'v':
			printf("binmod version %s\n", BINMOD_VERSION);
		case '?':
			exit(EXIT_SUCCESS);
			break;
		}
	}

	if (cfg.src_ofs == SIZE_MAX || cfg.src_len == SIZE_MAX ||
	    cfg.dst_ofs == SIZE_MAX || cfg.dst_len == SIZE_MAX) {
		fprintf(stderr, "Both -s <ofs,len> and -d <ofs,len> are required.\n");
		exit(EXIT_FAILURE);
	}

	if (!cfg.in_file || !cfg.out_file)
		exit(EXIT_SUCCESS);

	src_fp = fopen(in_file, "rb");
	if (src_fp == NULL) {
		fprintf(stderr, "Failed to open the file '%s': %s (%d)\n", in_file, strerror(errno), errno);
		exit(EXIT_FAILURE);
	}

	if (fseeko(src_fp, 0, SEEK_END) != 0) {
		fprintf(stderr, "fseeko src_fp error: %s (%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}
	off_t src_filesize = ftello(src_fp);
	if (src_filesize < 0) {
		fprintf(stderr, "ftello error: %s (%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}
	rewind(src_fp);

	// printf("src_filesize: %jd\n", (intmax_t)src_filesize);

	dst_fp = fopen(out_file, "rb+");
	if (dst_fp == NULL) {
		fprintf(stderr, "Failed to open the file '%s': %s (%d)\n", out_file, strerror(errno), errno);
		exit(EXIT_FAILURE);
	}

	if (fseeko(dst_fp, 0, SEEK_END) != 0) {
		fprintf(stderr, "fseeko dst_fp error: %s (%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}
	off_t dst_filesize = ftello(dst_fp);
	if (dst_filesize < 0) {
		fprintf(stderr, "ftello error: %s (%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}
	rewind(dst_fp);

	// printf("dst_filesize: %jd\n", (intmax_t)dst_filesize);

	if ((cfg.src_len == 0) || (cfg.src_ofs >= (uint64_t)src_filesize) || (cfg.src_len > (uint64_t)src_filesize - cfg.src_ofs) ||
	    (cfg.dst_len == 0) || (cfg.dst_ofs >= (uint64_t)dst_filesize) || (cfg.dst_len > (uint64_t)dst_filesize - cfg.dst_ofs) ||
	    (cfg.src_len < cfg.dst_len))
		exit(EXIT_FAILURE);

	char *src_buf = malloc(cfg.src_len);
	char *dst_buf = malloc(cfg.dst_len);
	if (!src_buf || !dst_buf) {
		perror("malloc *_buf failed");
		goto exit_free;
	}

	if (fseeko(src_fp, cfg.src_ofs, SEEK_SET) != 0) {
		fprintf(stderr, "fseeko error: %s (%d)\n", strerror(errno), errno);
		goto exit_free;
	}
	n = fread(src_buf, 1, cfg.src_len, src_fp);
	if (n != cfg.src_len) {
		if (ferror(src_fp))
			fprintf(stderr, "fread src_fp error: %s\n", strerror(errno));
		else
			fprintf(stderr, "fread src_fp: unexpected EOF\n");
		goto exit_free;
	}

	// print_buf(src_buf, cfg.src_len < 4096 ? cfg.src_len : 4096, "src_buf");

#if (CONFIG_MAIN_DEBUG)
	if (fseeko(dst_fp, cfg.dst_ofs, SEEK_SET) != 0) {
		fprintf(stderr, "fseeko error: %s (%d)\n", strerror(errno), errno);
		goto exit_free;
	}
	n = fread(dst_buf, 1, cfg.dst_len, dst_fp);
	if (n != cfg.dst_len) {
		if (ferror(dst_fp))
			fprintf(stderr, "fread dst_fp error: %s\n", strerror(errno));
		else
			fprintf(stderr, "fread dst_fp: unexpected EOF\n");
		goto exit_free;
	}
	print_buf(dst_buf, cfg.dst_len < 4096 ? cfg.dst_len : 4096, "dst_buf");
#endif

	memcpy(dst_buf, src_buf, cfg.dst_len);
	// print_buf(dst_buf, cfg.dst_len < 4096 ? cfg.dst_len : 4096, "dst_buf");

	if (fseeko(dst_fp, cfg.dst_ofs, SEEK_SET) != 0) {
		fprintf(stderr, "fseeko error: %s (%d)\n", strerror(errno), errno);
		goto exit_free;
	}
	n = fwrite(dst_buf, 1, cfg.dst_len, dst_fp);
	if (n != cfg.dst_len) {
		fprintf(stderr, "fwrite dst_fp error: %s\n", strerror(errno));
		goto exit_free;
	}
	fflush(dst_fp);

	if (fseeko(dst_fp, cfg.dst_ofs, SEEK_SET) != 0) {
		fprintf(stderr, "fseeko error: %s (%d)\n", strerror(errno), errno);
		goto exit_free;
	}
	n = fread(dst_buf, 1, cfg.dst_len, dst_fp);
	if (n != cfg.dst_len) {
		if (ferror(dst_fp))
			fprintf(stderr, "fread dst_fp error: %s\n", strerror(errno));
		else
			fprintf(stderr, "fread dst_fp: unexpected EOF\n");
		goto exit_free;
	}

	// print_buf(dst_buf, cfg.dst_len < 4096 ? cfg.dst_len : 4096, "dst_buf");
	ret = EXIT_SUCCESS;
	printf("binmod completed successfully.\n");
exit_free:
	free(src_buf);
	free(dst_buf);
	if (src_fp)
		fclose(src_fp);
	if (dst_fp)
		fclose(dst_fp);

	return ret;
}
