#define _GNU_SOURCE

#include "config.h"

#include <ldns/dns.h>
#include <pcap.h>

#define SEQUENCE 1
#define QSIZE    2
#define QDATA    3
#define ASIZE 	 4
#define ADATA    0

struct dns_info
{
	size_t seq;      /* seq number */
	size_t qsize;    /* query size */
	size_t asize;    /* answer size */
	char *qdata;     /* query data in hex */
	char *adata;     /* answer data in hex */
};

void
usage(FILE *fp)
{
	fprintf(fp, "./ldns-pcat-diff FILE1 [FILE2]\n\n");
	fprintf(fp, "Show diff between 2 pcat traces\n");
	fprintf(fp, "There are no options, is FILE2 is not given, standard input is read\n");
	fprintf(fp, "OUTPUT FORMAT:\n");
}


void
compare(struct dns_info *d1, struct dns_info *d2)
{
	int diff = 0;

	/* only complain if the question is the same */	
	if (d1->qsize == d2->qsize &&
			!strcmp(d1->qdata, d2->qdata)) {
		/* query is the same */
		if (d1->asize != d1->asize) {
			diff = 1;
		} else {
			if (strcmp(d1->adata, d2->adata))
				diff = 1;
		}

		if (diff == 1) {
			fprintf(stdout, "%zd:%zd\n%s\n%s\n%s\n", d1->seq, d2->seq, d1->qdata,
					d1->adata, d2->adata);
		}
	} else {
		fprintf(stderr, "Query differs!\n");
		fprintf(stdout, "%zd:%zd\n%s\n%s\n%s\n", d1->seq, d2->seq, d1->qdata,
				d1->qdata, d2->qdata);

	}
}

int
main(int argc, char **argv)
{
	FILE *trace1;
	FILE *trace2;
	size_t i;
	ssize_t read1;
	size_t len1;
	char *line1;
	ssize_t read2;
	size_t len2;
	char *line2;

	struct dns_info d1;
	struct dns_info d2;

	i = 0;
	len1 = 0;
	line1 = NULL;
	len2 = 0;
	line2 = NULL;

	/* need two files */
	switch(argc) {
		case 1:
			usage(stdout);
			/* usage */
			exit(EXIT_FAILURE);
		case 2:
			if (!(trace1 = fopen(argv[1], "r"))) {
				fprintf(stderr, "Cannot open trace file `%s\'\n", argv[1]);
				exit(EXIT_FAILURE);
			}
			trace2 = stdin;
			break;
		case 3:
			if (!(trace1 = fopen(argv[1], "r"))) {
				fprintf(stderr, "Cannot open trace file `%s\'\n", argv[1]);
				exit(EXIT_FAILURE);
			}
			if (!(trace2 = fopen(argv[2], "r"))) {
				fprintf(stderr, "Cannot open trace file `%s\'\n", argv[1]);
				exit(EXIT_FAILURE);
			}
			break;
		default:
			exit(EXIT_FAILURE);
	}

	/* read the version information */
	read1 = getline(&line1, &len1, trace1);
	read2 = getline(&line2, &len2, trace2);
	/* TODO version checking */
	if (read1 == -1 || read2 == -1) {
		fprintf(stderr, "Read error\n");
		exit(EXIT_FAILURE);
	}
	i = 1;

reread:
	read1 = getline(&line1, &len1, trace1);
	read2 = getline(&line2, &len2, trace2);
	if (read1 == -1 || read2 == -1) {
		fclose(trace1); fclose(trace2);
		exit(EXIT_SUCCESS);
	}
	if (read1 > 0) 
		line1[read1 - 1] = '\0';
	if (read2 > 0)
		line2[read2 - 1] = '\0';

	switch(i % 5) {
		case SEQUENCE:
			d1.seq = atoi(line1);
			d2.seq = atoi(line2);
			break;
		case QSIZE:
			d1.qsize = atoi(line1);
			d2.qsize = atoi(line2);
			break;
		case QDATA:
			d1.qdata = strdup(line1);
			d2.qdata = strdup(line2);
			break;
		case ASIZE:
			d1.asize = atoi(line1);
			d2.asize = atoi(line2);
			break;
		case ADATA:
			d1.adata = strdup(line1);
			d2.adata = strdup(line2);
			/* we now should have  */
			compare(&d1, &d2);
			free(d1.adata);
			free(d2.adata);
			free(d1.qdata);
			free(d2.qdata);
			break;
	}
	i++;
	goto reread;

	fclose(trace1);
	fclose(trace2);
	return 0;
}
