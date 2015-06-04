#include <stdlib.h>
#include <string.h>

#include "graphs.h"




struct graph *alloc_graph(size_t num_vertices, size_t sqrt_nvert)
{
	
	
//	size_t arr_size = (num_vertices*num_vertices - num_vertices) / 2;
	
	struct graph *g = malloc(sizeof(struct graph));
	if(!g) {
		err_printf("malloc(1)");
		goto err_end;
	}
	
	g->num_vertices = num_vertices;
	g->sqrt_nvert   = sqrt_nvert;
	
	
	g->matrix = malloc(sizeof(cost_t **)*num_vertices);
	if(!g->matrix) {
		err_printf("malloc(2)");
		goto free_graph;
	}
	memset(g->matrix, 0, sizeof(cost_t **)*num_vertices);
	
	//cost_t *array = malloc(sizeof(cost_t)*arr_size);
	int i;
	for(i=0; i < num_vertices; ++i) {

		g->matrix[i] = malloc(sizeof(cost_t)*(i+1));
		if(!g->matrix[i]) {			
			err_printf("malloc(3)");
			goto free_matrix_els;
		}

		memset(g->matrix[i], 0, sizeof(cost_t)*(i+1));


//		dbg_printf("%d number %d", i, (num_vertices-i));		
	}
	
	
	return g;
	
free_matrix_els:
	for(i=0; i < num_vertices && g->matrix[i] ; ++i) 
		free(g->matrix[i]);
// free_matrix:
	free(g->matrix);
free_graph:
	free(g);
err_end:
	return NULL;
}
void free_graph(struct graph *g) 
{
	int i;
	for(i=0; i < g->num_vertices && g->matrix[i] ; ++i) 
		free(g->matrix[i]);
	free(g->matrix);
	free(g);

}
