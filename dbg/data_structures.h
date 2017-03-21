#ifndef DATA_STRUCTURES_H
	#define DATA_STRUCTURES_H

	#define K 6

	//// PROTOTYPES
	struct node_s;
	struct edge_s;
	struct list_edge_s;


	typedef struct list_edge_s {
		struct edge_s * e;
		struct list_edge_s * next;
	} list_edge_t;

	typedef struct node_s {
		int id;
		char seq[K];
		list_edge_t * in;
		list_edge_t * out;
	} node_t;

	typedef struct edge_s {
		int id;
		int count;
		node_t * from;
		node_t * to;
	} edge_t;

	//// FUNCTIONS PROTOTYPES
	/*
	 * Creates (and allocates) a new node containing:
	 * - a sequence
	 * - corresponding id
	 * - NULL pointer for in/out edges
	 *
	 * Return NULL if can't create node.
	 *
	 */
	node_t * create_node(int, char *);

	/*
	 * Add edge to in-list.
	 *
	 */
	node_t * add_in_edges(node_t *, edge_t *);

	/*
	 * Add edge to out-list.
	 *
	 */
	 node_t * add_out_edges(node_t *, edge_t *);

	 /*
	  * Creates (and allocates) a new edge between two nodes
	  * - corresponding id
	  * - count to 1
	  *
	  * Return NULL if can't create edge.
	  *
	  */
	  edge_t * create_edge(node_t *, node_t *, int);

#endif