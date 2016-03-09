/* Copyright 2014-2016 IBM Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * imitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include "xscom.h"
#include "sram.h"

static void print_usage(void)
{
	printf("usage: getsram [-c|--chip chip-id] addr\n");
	printf("               [--occ-channel|n <chan>]\n");
	printf("       getsram -v|--version\n");
}

#define VERSION_STR _str(VERSION)
#define _str(s) __str(s)
#define __str(s) #s

int main(int argc, char *argv[])
{
	uint64_t val, addr = -1ull;
	uint32_t def_chip, chip_id = 0xffffffff;
	bool show_help = false;
	bool show_version = false;
	bool no_work = false;
	int rc;
	int occ_channel = 0;

	while(1) {
		static struct option long_opts[] = {
			{"chip",	required_argument,	NULL,	'c'},
			{"occ-channel",	required_argument,	NULL,	'n'},
			{"help",	no_argument,		NULL,	'h'},
			{"version",	no_argument,		NULL,	'v'},
		};
		int c, oidx = 0;

		c = getopt_long(argc, argv, "-c:n:hlv", long_opts, &oidx);
		if (c == EOF)
			break;
		switch(c) {
		case 1:
			addr = strtoull(optarg, NULL, 16);
			break;
		case 'c':
			chip_id = strtoul(optarg, NULL, 0);
			break;
		case 'n':
			occ_channel = strtoul(optarg, NULL, 0);
			if (occ_channel < 0 || occ_channel > 3) {
				fprintf(stderr, "occ-channel out of range 0 <= c <= 3\n");
				exit(1);
			}
			break;
		case 'h':
			show_help = true;
			break;
		case 'v':
			show_version = true;
			break;
		default:
			exit(1);
		}
	}

	if (addr == -1ull)
		no_work = true;
	if (no_work && !show_version && !show_help) {
		fprintf(stderr, "Invalid or missing address\n");
		print_usage();
		exit(1);
	}
	if (show_version)
		printf("xscom utils version %s\n", VERSION_STR);
	if (show_help)
		print_usage();
	if (no_work)
		return 0;
	def_chip = xscom_init();
	if (def_chip == 0xffffffff) {
		fprintf(stderr, "No valid XSCOM chip found\n");
		exit(1);
	}
	if (chip_id == 0xffffffff)
		chip_id = def_chip;

	rc = sram_read(chip_id, occ_channel, addr, &val);
	if (rc) {
		fprintf(stderr,"Error %d reading XSCOM\n", rc);
		exit(1);
	}
	printf("OCC%d: %" PRIx64 "\n", occ_channel, val);
	return 0;
}
