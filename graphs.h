#ifndef __GRAPHS_H__
#define __GRAPHS_H__

#include "types.h"
#include "utils.h"
#define _for_each_vertex(g, i, begin) for( i = begin; i < (g)->num_vertices; ++i) 
#define for_each_vertex(g, i) _for_each_vertex(g, i, 0)
#define for_each_edge(g, i, j) for( i = 0; i < (g)->num_vertices; ++i) for(j=0; j<i; ++j)


#define num_vertices(g)			((g)->num_vertices)

#define G_DATA_MASK ((cost_t)               0xff)
#define G_UTIL_MASK ((cost_t) 0xffffffffffffff00)

//#define G_UTIL_SHIFT 48
#define G_UTIL_SHIFT 8
#define G_DATA_SHIFT 0



struct graph
{
	cost_t **matrix;
	size_t num_vertices;
	size_t sqrt_nvert;
};

static inline cost_t get_edge_cost(const struct graph *g, vertex_t i, vertex_t j)
{
	return (((g)->matrix[MAX(i, j)][MIN(i,j)]) & G_DATA_MASK) >> G_DATA_SHIFT ;
}
static inline void set_edge_cost(struct graph *g, vertex_t i, vertex_t j, cost_t cost)
{
	((g)->matrix[MAX(i, j)][MIN(i,j)]) = 
		 ((cost << G_DATA_SHIFT) & G_DATA_MASK)  | 
			(((g)->matrix[MAX(i, j)][MIN(i,j)]) & G_UTIL_MASK);
}
static inline cost_t get_vertex_cost(const struct graph *g, vertex_t a)
{
	return (((g)->matrix[a][a]) & G_DATA_MASK) >> G_DATA_SHIFT  ;
}
static inline void set_vertex_cost(struct graph *g, vertex_t a, cost_t cost)
{
	g->matrix[a][a] =
		((cost << G_DATA_SHIFT) & G_DATA_MASK)  | 
			( g->matrix[a][a] & G_UTIL_MASK );
}



static inline cost_t get_edge_util(const struct graph *g, vertex_t i, vertex_t j)
{
	return (((g)->matrix[MAX(i, j)][MIN(i,j)]) & G_UTIL_MASK) >> G_UTIL_SHIFT ;
}
static inline void set_edge_util(struct graph *g, vertex_t i, vertex_t j, cost_t cost)
{
	((g)->matrix[MAX(i, j)][MIN(i,j)]) = 
		((cost << G_UTIL_SHIFT) & G_UTIL_MASK)  | 
			(((g)->matrix[MAX(i, j)][MIN(i,j)]) & G_DATA_MASK);
}
static inline cost_t get_vertex_util(const struct graph *g, vertex_t a)
{
	return (((g)->matrix[a][a]) & G_UTIL_MASK) >> G_UTIL_SHIFT  ;
}
static inline void set_vertex_util(struct graph *g, vertex_t a, cost_t cost)
{
	g->matrix[a][a] =
		  ((cost << G_UTIL_SHIFT) & G_UTIL_MASK)  | 
			( g->matrix[a][a] & G_DATA_MASK );
}



static inline void clear_vertex_util(struct graph *g, cost_t mask)
{
	vertex_t v;
	for_each_vertex(g, v) 
		set_vertex_util(g, v, get_vertex_util(g, v) & mask);
	
}


struct graph *alloc_graph(size_t num_vertices, size_t sqrt_nvert);
void free_graph(struct graph *g);


static inline vertex_t next_neighbor(struct graph *g, vertex_t v, vertex_t prev)
{
	int i;

	_for_each_vertex(g, i, prev == UNIT ? 0 : (prev+1)) {
		if(get_edge_cost(g, i, v) != INFINITY ) 
			return i;
	}
	return UNIT;
}
#define for_each_neighbor(g, v, a) for(a = next_neighbor(g, v, UNIT); a != UNIT; a = next_neighbor(g, v, a))



static inline struct graph *dup_graph(struct graph * g)
{
	return g ? alloc_graph(g->num_vertices, g->sqrt_nvert) : NULL;
}




#endif /* __GRAPHS_H__ */
