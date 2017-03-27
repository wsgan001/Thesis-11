#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include "../dbg/data_structures.h"
#include "../utilities/my_lib.h"
#include "../utilities/FIFO.h"

#ifdef SILENT
	#define printf(...)
#endif

#define IN_FORMAT 1
#define K_ARG 2
#define L_ARG 4
#define IN_FILE 6
#define MIN_ARGS 2

#define BUFFER 256

#define FASTA 1
#define FASTQ 2

graph_t * build_graph(double, double, int);
int map_read(char *, int, int, graph_t *, fifo_t *);
node_t * get_successor(node_t *, int, char);


int main (int argc, char * argv[]) {
	int k = 10;
	int l = 34;
	//// Check args
	int input_format = 0;
	int l_arg = L_ARG;
	int in_file = IN_FILE;
	char input_file[BUFFER+1];
	char out_file[BUFFER+1];
	argc -= 1;
	if (argc < MIN_ARGS) {
		fprintf(stdout, "Usage: (--fasta|--fastq) [-k K (default 10)] [-l reads_len (default 34)] input_file_no_ext\n");
		return 1;
	} else {
		if ( strcmp(argv[K_ARG], "-k") == 0 ) {
			k = atoi(argv[K_ARG + 1]);
		} else {
			in_file -= 2;
			l_arg -= 2;
		}
		if ( strcmp(argv[l_arg], "-l") == 0 ){
			l = atoi(argv[l_arg + 1]);
		} else {
			in_file -= 2;
		}
		strncpy(input_file, argv[in_file], BUFFER);
		strncpy(out_file, input_file, BUFFER);
		strcat(out_file, ".graph");
		if (strcmp("--fasta", argv[IN_FORMAT]) == 0) {
			input_format = FASTA;
			strcat(input_file, ".fa");
		} else if (strcmp("--fastq", argv[IN_FORMAT]) ==  0) {
			input_format = FASTQ;
			strcat(input_file, ".fastq");
		} else {
			fprintf(stdout, "Usage: (--fasta|--fastq) [-k K (default 34)] input_file_no_ext\n");
			return 1;
		}
	}

	//// -----------------------
	int i;
	int index;
	int sublen;
	char buf[BUFFER+1];
	char * read;
	if( !(read = (char*)malloc(sizeof(char) * l+1)) ) {
		fprintf(stdout, "ERROR: couldn't allocate memory\n");
		return 1;
	}

	//// BUILD EMPTY GRAPH
	double nodes = pow((double)4, (double)(k/2));
	double edges = pow((double)4, (double)((k/2)+1));
	graph_t * dbg = build_graph(nodes, edges, k/2);
	if (!dbg) {
		fprintf(stdout, "ERROR: couldn't allocate memory - Graph not built\n");
		return 1;
	}
	fprintf(stdout, "Graph built on %d-mer\n", k/2);


	//// Reading reads files
	fifo_t * q;
	if ( !(q = init_queue(k/2)) ) {
		fprintf(stdout, "ERROR: couldn't allocate\n");
		return 1;
	}

	//// Ready to open file
	fprintf(stdout, "Reading %s\n", input_file);
	FILE * fp;
	if( !(fp = fopen(input_file, "r")) ) {
		fprintf(stdout, "Error: can't open %s\n", input_file);
		return 1;
	}
	int skip_line;
	if (input_format == FASTA)
		skip_line = 2;
	else //is FASTQ
		skip_line = 4;

	//Read first line
	fgets(buf, BUFFER, fp);
	i=0;
	while(!feof(fp)) {
		i++;
		if(i==2) {
			//This line has the read
			strncpy(read, buf, l); //Remove '\n', ensure length
			//printf("%s\n", read);
			index = 0;
			while( (index = get_next_substring(read, index, k, &sublen)) != -1 ) {
				//Each substring hsould be mapped
				if (map_read(read+index, sublen, k, dbg, q)) {
					return 1;
				}
				index = index + sublen;
			}

		}
		if(i==skip_line) {
			i=0;
		}
		fgets(buf, BUFFER, fp);
	}

	fclose(fp);
	//// OUTPUT
	fprintf(stdout, "Generating output\n");
	if( !(fp = fopen(out_file, "w+")) ) {
		fprintf(stdout, "Error: can't open %s\n", out_file);
		return 1;
	}
	fprintf(fp, "node\tin_nodes\tout_nodes\tin_nodes_kstep\tout_nodes_kstep\n");
	list_edge_t * le;
	for(i=0; i<nodes; i++) {
		fprintf(fp, "%s\t", dbg->nodes[i]->seq);
		le = dbg->nodes[i]->in;
		fprintf(fp, "%s:%d", le->e->from->seq, le->e->count);
		while( (le = le->next) ) {
			fprintf(fp, ";%s:%d", le->e->from->seq, le->e->count);
		}
		le = dbg->nodes[i]->out;
		fprintf(fp, "\t%s:%d", le->e->to->seq, le->e->count);
		while( (le = le->next) ) {
			fprintf(fp, ";%s:%d", le->e->to->seq, le->e->count);
		}
		if(le = dbg->nodes[i]->in_kstep) {
			fprintf(fp, "\t%s:%d", le->e->from->seq, le->e->count);
			while( (le = le->next) ) {
				fprintf(fp, ";%s:%d", le->e->from->seq, le->e->count);
			}
		}

		if(le = dbg->nodes[i]->out_kstep) {
			fprintf(fp, "\t%s:%d", le->e->to->seq, le->e->count);
			while( (le = le->next) ) {
				fprintf(fp, ";%s:%d", le->e->to->seq, le->e->count);
			}
		}
		fprintf(fp, "\n");
	}


	return 0;
}












//////////////////////// FUNCTIONS

int map_read(char * read, int l, int k, graph_t * dbg, fifo_t * q) {
	int i;
	edge_t * e;
	node_t * n = dbg->nodes[hash(read, k/2)]; //Get starting node
	node_t * n0;
	q = enqueue(q, n);
	//printf("%x enqueued\n", n);
	for (i=1; i<(k/2); i++) {
		n = get_successor(n, k/2, *(read+i+k/2-1)); //Get the node corresponding to the right overlapping kmer
		q = enqueue(q, n);
		//printf("%x enqueued\n", n);
	}
	//Now start to add edges for contiguos kmer
	for(; i<(l-k/2+1); i++) {
		n = get_successor(n, k/2, *(read+i+k/2-1));
		n0 = dequeue(q);
		//printf("n0: %s, n: %s, read: %s\n", n0->seq, n->seq, read);
		if( !(e = create_edge(n0, n, hash(read+i, k))) ) {
			return 1;
		}
		if (update_edge(e)) {
			fprintf(stdout, "ERROR: couldn't allocate\n");
			return 1;
		}

		q = enqueue(q, n);
	}

	return 0;
}


node_t * get_successor(node_t * n, int k, char c) {
	list_edge_t * le = n->out;
	while(le) {
		//printf("%x ", le);
		//printf("%s;%c\t", le->e->to->seq, c);
		if(le->e->to->seq[k-1] == c) {
			return le->e->to;
		}
		le = le->next;
	}
	return NULL;
}




graph_t * build_graph(double nodes, double edges, int k) {
	int i;
	int mask_hash = (int)nodes - 1;

	graph_t * dbg;
	if ( !(dbg = (graph_t*)malloc(sizeof(graph_t))) ) {
		fprintf(stdout, "ERROR: couldn't allocate memory\n");
		return NULL;
	}
	if ( !(dbg->nodes = (node_t**)malloc(sizeof(node_t*) * nodes)) ) {
		fprintf(stdout, "ERROR: couldn't allocate memory\n");
		return NULL;
	}
	if ( !(dbg->edges = (edge_t**)malloc(sizeof(edge_t*) * edges)) ) {
		fprintf(stdout, "ERROR: couldn't allocate memory\n");
		return NULL;
	}

	char * kmer;
	if ( !(kmer = (char*)malloc(sizeof(char) * (k+1))) ) {
		fprintf(stdout, "ERROR: couldn't allocate memory\n");
		return NULL;
	}

	for (i=0; i<nodes; i++) {
		dbg->nodes[i] = NULL;
		dbg->edges[i] = NULL;
	}
	for( ; i<edges; i++) {
		dbg->edges[i] = NULL;
	}

	int n0_hash, n1_hash;
	node_t * n0, * n1;
	edge_t * e;

	for(i=0; i<edges; i++) {
		//Each i is a coded kmer

		n0_hash = i >> 2;
		n1_hash = i & mask_hash;
		//printf("%x\t%x\t%x\n", i, n0_hash, n1_hash);

		if( !(n0 = dbg->nodes[n0_hash]) ) {
			rev_hash(n0_hash, k, kmer);
			if( !(n0 = create_node(n0_hash, kmer)) ) {
				fprintf(stdout, "ERROR: couldn't allocate memory\n");
				return NULL;
			}
			if ( !(dbg->nodes[n0_hash] = (node_t*)malloc(sizeof(node_t))) ) {
				fprintf(stdout, "ERROR: couldn't allocate memory\n");
				return NULL;
			}
			dbg->nodes[n0_hash] = n0;
		}

		if( !(n1 = dbg->nodes[n1_hash]) ) {
			rev_hash(n1_hash, k, kmer);
			if( !(n1 = create_node(n1_hash, kmer)) ) {
				fprintf(stdout, "ERROR: couldn't allocate memory\n");
				return NULL;
			}
			if ( !(dbg->nodes[n1_hash] = (node_t*)malloc(sizeof(node_t))) ) {
				fprintf(stdout, "ERROR: couldn't allocate memory\n");
				return NULL;
			}
			dbg->nodes[n1_hash] = n1;
		}

		if (!( e = dbg->edges[i] )) {
			//Create edge
			if( !(e = create_edge(n0, n1, i) ) ) {
				fprintf(stdout, "ERROR: couldn't allocate memory\n");
				return NULL;
			}
			if ( !(dbg->edges[i] = (edge_t*)malloc(sizeof(edge_t))) ) {
				fprintf(stdout, "ERROR: couldn't allocate memory\n");
				return NULL;
			}
			dbg->edges[i] = e;
			n0 = add_out_edges(n0, e);
			if (!n0) {
				fprintf(stdout, "ERROR: couldn't allocate\n");
				return NULL;
			}
			n1 = add_in_edges(n1, e);
			if (!n1) {
				fprintf(stdout, "ERROR: couldn't allocate\n");
				return NULL;
			}
		}
	}

	return dbg;
}
