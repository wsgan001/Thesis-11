#include <string.h>
#include <stdio.h>
#include <math.h>

int hash(char * kmer, int len) {
	int hashed = 0;
	int i;
	int v;
	for(i=0; i < len; i++) {
		switch(kmer[i]) {
			case 'A':
				v = 0;
				break;
			case 'C':
				v = 1;
				break;
			case 'G':
				v = 2;
				break;
			case 'T':
				v = 3;
				break;
			default:
				v = 0;
		}
		hashed = hashed << 2;
		hashed += v;
	}

	return hashed;
}


void rev_hash(int hash, int k, char * str) {
	int i;
	int mask;
	int c;
	mask = 3; //Get last 2 bit

	for (i=0; i<k; i++) {
		c = hash & mask;
		switch(c) {
			case 0:
				c = 'A';
				break;
			case 1:
				c = 'C';
				break;
			case 2:
				c = 'G';
				break;
			case 3:
				c = 'T';
				break;
			default:
				c = 'N';
		}
		str[k-1-i] = c;
		hash = hash >> 2;
	}
	str[i] = '\0';

}

int contains(char * seq, char c) {
	unsigned int i;
	for(i=0; i<strlen(seq); i++) {
		if(seq[i] == c) {
			return 1;
		}
	}
	return 0;
}


void reverse_kmer(char * kmer, char * rev, int k) {
	int i;
	char c;
	for (i=0; i<k; i++) {
		switch(kmer[k-1-i]) {
			case 'A':
				c = 'T';
				break;
			case 'C':
				c = 'G';
				break;
			case 'G':
				c = 'C';
				break;
			case 'T':
				c = 'A';
				break;
			default:
				c = '\0'; //Used to eventually return error further in the processing, should not happen (otherwise it will be shorter)
		}
		rev[i] = c;
	}
	rev[i] = '\0';
}


int is_palyndrome(char * kmer, char * rev) {
	return !strcmp(kmer, rev);
}


int get_next_substring(char * seq, int index, int min_l, int * len) {
	int i;
	if(*(seq+index) == '\0')
		return -1;
	//Find a possible start
	i=index;
	while(*(seq+i) == 'N') {
		i++;
	}
	//Can start a new substring
	int j = 0;
	while(seq[i+j] != 'N' && seq[i + j] != '\0') {
		j++;
	}
	if (j >= min_l) {
		*len = j;
		return i;
	}
	return get_next_substring(seq, i+j, min_l, len);
}
