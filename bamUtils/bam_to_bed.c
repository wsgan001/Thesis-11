#include <htslib/hts.h>
#include <htslib/sam.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "bam_to_bed.h"
#include "bam_to_region.h"
#include "genome.h"

int bam_to_bed(char * bam, char * bed, int extension, int to_region) {

	myBam_t * myBam = myBam_open(bam);

	FILE * bed_fp;
	if(!(bed_fp = fopen(bed, "w+"))) {
		fprintf(stdout, "[ERROR] can't open %s\n", bed);
		return 1;
	}

	fprintf(bed_fp, "chrom\tchromStart\tchromEnd\n");

	region_t * region;
	if( !(region = (region_t*)malloc(sizeof(region_t))) ) {
		fprintf(stdout, "[ERROR] can't allocate\n");
		return -1;
	}

	if(to_region) {
		int status;
		status = sam_read1(myBam->in, myBam->header, myBam->aln);
		if(status <= 0) {
			fprintf(stdout, "[ERROR] unexpected EOF\n");
			return -2;
		}

		while(region = get_next_region_overlap(myBam, region, extension, 1)) {
			fprintf(bed_fp, "%s\t%d\t%d\n", region->chromosome, region->start, region->end);
		}
	} else {
		while(region = get_next_region(myBam, region, extension)) {
			fprintf(bed_fp, "%s\t%d\t%d\n", region->chromosome, region->start, region->end);
		}
	}


	fclose(bed_fp);

	myBam_close(myBam);

	return 0;
}

/*
int main(int argc, char * argv[]) {
	return bam_to_bed(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]));
}
*/
