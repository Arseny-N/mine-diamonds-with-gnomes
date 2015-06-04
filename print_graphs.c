#include "print_graphs.h"

#include "graphs.h"

void fprint_vertex_list(FILE *fp, struct vertex_list *ug)
{
	struct vertex_list_node *n = NULL;
	fprintf(fp, "path len: %ld\n", ug->length);
	fprintf(fp, "s -> " );
	for_each_vertex_list_node(ug,n)  
		fprintf(fp, VERTEX_FMT" -> ", n->v);
	fprintf(fp, "t\n" );
}
#define BITS_GET_FILED(name, num) ( ( (num) >> (name ## _UTIL_SHIFT))  & (name ## _UTIL_MASK) ) 
#define BITS_ENCODE_FILED(name, num)  ( ( (num) & (name ## _UTIL_MASK) ) << (name ## _UTIL_SHIFT) )

#define IPG_UTIL_WIDTH 8
#define IPG_UTIL_MASK  0xff
#define IPG_UTIL_SHIFT 8

#define IPG_VISIT_VERTEX 1

static inline int get_vertex_visit_flag(const struct graph *g, vertex_t v) 
{
	return  BITS_GET_FILED( IPG, get_vertex_util(g, v)) & IPG_VISIT_VERTEX;
}

void fprint_graph(FILE *fp, struct graph *ug)
{	
	vertex_t i,j;
	
	fprintf(fp, "graph\n{\n");		
	fprintf(fp,"\t{\n\t\trank=same;\n");	  
	
	for_each_vertex(ug, i) {
		if((i)%ug->sqrt_nvert == 0)  {
			fprintf(fp,"\t}\n");
			fprintf(fp,"\t{\n\t\trank=same;\n");	  
		}
		cost_t c = get_vertex_cost(ug, i);
		fprintf(fp,"\t\t\""VERTEX_FMT"\"[label=\" "VERTEX_FMT" ("COST_FMT") %s\", color=\"%s\"];\n", i,i, c,get_vertex_visit_flag(ug, i) ? "v" : "u", c >= 10 ? "red" : "blue" );
	}

	fprintf(fp,"\t}\n");
	
	
	fprintf(fp,"\n");

	for_each_edge(ug, i, j) {
		if( get_edge_cost(ug,i,j) != INFINITY )
			fprintf(fp,"\t\""VERTEX_FMT"\" -- \""VERTEX_FMT"\"[label=\"("COST_FMT")\", color=\"%s\"];\n",
				i, j, get_edge_cost(ug,i,j), get_edge_cost(ug,i,j) >= 10 ? "black":"gray");
	}
		
	fprintf(fp,"}\n");
}
