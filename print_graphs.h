#ifndef __PRINT_GRAPHS_H__
#define __PRINT_GRAPHS_H__

#include <stdio.h>
#include "types.h"
#include "vertex_list.h" 
#include "graphs.h"

void fprint_vertex_list(FILE *fp, struct vertex_list *ug);
void fprint_graph(FILE *fp, struct graph *ug);


#endif /* __PRINT_GRAPHS_H__ */
