struct vertex_list_node
{		
	struct vertex_list_node *next;
	vertex_t v;
};
	
struct vertex_list
{
	struct vertex_list_node *head;
	struct vertex_list_node *tail;
	
	size_t length;
};	

#define for_each_vertex_list_node(gp, p) for(p = gp->head; p; p = p->next)
#define for_each_vertex_list_node_safe(gp, p, n) for(p = gp->head, n = p -> next; p; p = n, n = n ? n -> next : NULL)

static inline void init_vertex_list(struct vertex_list *l)
{
	
	memset(l, 0, sizeof(struct vertex_list));
}

struct vertex_list *alloc_vertex_list(void)
{
	size_t size = sizeof(struct vertex_list);
	struct vertex_list *p = malloc(size);
	if(p) 
		init_vertex_list(p);
	
	return p;
}

int add_vertex_to_list_tail(struct vertex_list *gp, vertex_t v) 
{
	if(!gp->tail) {
		gp->tail = gp->head = malloc(sizeof(struct vertex_list_node));
		if(!gp->tail) {
			err_printf("malloc()");
			return -1;
		}
		gp->head->v = v;
		gp->length ++;
		return 0;
	}
	gp->tail->next = malloc(sizeof(struct vertex_list_node));
	if(!gp->tail->next) {
		err_printf("malloc()");
		return -1;
	}
	gp->tail = gp->tail->next;
	gp->tail->v = v;
	gp->length ++;
	return 0;
}
int add_vertex_to_list_head(struct vertex_list *gp, vertex_t v) 
{
	if(!gp->tail) {
		gp->tail = gp->head = malloc(sizeof(struct vertex_list_node));
		if(!gp->tail) {
			err_printf("malloc()");
			return -1;
		}
		gp->tail->v = v;
		gp->length ++;
		return 0;
	}
	struct vertex_list_node *t = gp->head;
	gp->head = malloc(sizeof(struct vertex_list_node));
	if(!gp->head) {
		err_printf("malloc()");
		return -1;
	}
	gp->head->next = t;
	gp->head->v = v;
	gp->length ++;
	return 0;
}
void free_vertex_list(struct vertex_list *gp)
{
	struct vertex_list_node *n, *s;

	for_each_vertex_list_node_safe(gp,n, s) 
		free(n);
	
	free(gp);
}
int draw_random_edges(struct graph *g, struct graph *pg, struct cat_state *cs, const size_t radius)
{
	struct breadth_first_state s;
	size_t count = (radius*2+1)*(radius*2+1);
	vertex_t v;
	int r = -1;
	
	struct w_s {
		cost_t cost;
		vertex_t v;		
	} *w = malloc(count * sizeof(struct w_s));
	if(!w) {
		err_printf("malloc() -> weights");
		return -1;
	}
	u64 *cdf = malloc(count * sizeof(u64));
	if(!cdf) {
		err_printf("malloc() -> weights");		
		goto free_w;
	}
	struct vertex_list *vl = alloc_vertex_list();
	if(!vl) {
		err_printf("alloc_vertex_list");
		goto free_cdf;
	}
	for_each_vertex(pg, v) {		
		if( get_vertex_visit_flag(g,v) ) {
			
			struct cat_parameters cp;
			
			size_t ind, i;
			cost_t sum;
			
			ind = sum = 0;
			
			if(breadth_first_traversal(
				pg, v, init_breadth_first_state(&s), 
				bft_visitor_lambda( (vertex_t v, struct breadth_first_state *s) {
					if(s->dist <= radius) {
						w[ind].cost = get_vertex_cost(pg, v);
						w[ind].v = v;
						sum += get_vertex_cost(pg, v);					
						ind ++;
					}
					return VISITOR_OK;					
				}) 
			)) {
				err_printf("breadth_first_traversal(g)");
				goto free_cdf;
			}

			cdf[0] = w[0].cost;
			for(i=0; i < ind; ++i) 
				cdf[i] =  w[i].cost + cdf[i-1];

			
			
			cp.d_cdf = cdf;
			cp.count = ind;			
			
			
			u16 rnd = sample_dcat(cs, &cp);
			vertex_t u, new = w[rnd].v;
			
			
			

			if(add_vertex_to_list_head(vl, new)) {
				err_printf("add_vertex_to_list_tail()");
				goto free_vlist;
			}
			dbg_printf("~~~~~~~~~~~~~~~ new %d v %d ind %d rnd %d h %p", new, v, ind, rnd, vl->nodes);
			unset_vertex_visit_flag(g, v);			

			
			set_vertex_cost(g, v, 	 10);									
			set_vertex_cost(g, new,	 10);

			
			if(get_edge_cost(g, v, new) != INFINITY ) {
				set_edge_cost(g, v, new, 10);
			} else {
				set_path_cost(pg, g, v, new, 10, 10);
			}

			
			

		} 
	}
	
	if(!vertex_list_empty(vl)) {
		struct vertex_list_node *node = &vl->nodes;

		
		set_vertex_visit_flag(g, node->v);

		for_each_neighbor(g, node->v, v) {
			if(get_vertex_cost(g, v) == 10) 
				set_edge_cost(g, v, node->v, 10);								
		}
		
		for_each_vertex_list_node(vl, node) {		
			set_vertex_visit_flag(g, node->v);

			//for_each_neighbor(g, node->v, v) {
			//	if(get_vertex_cost(g, v) == 10) 
			//		set_edge_cost(g, v, node->v, 10);								
			//}
		}
		

	}
		

	r = 0;
	
free_vlist:
	free_vertex_list(vl);
free_cdf:
	free(cdf);
free_w:
	free(w);	

	return r;
}

do {
	
		if(build_inv_probability_graph(g, pg, map, ARRSIZE(map))) {
			err_printf("build_inv_probability_graph()");
			return 1;
		}
//		snprintf(buf, sizeof(buf),"graph.p%d.dot", i);
//		fprint_graph(fopen(buf, "w"), pg);

		if(draw_random_edges(g, pg, &cs, 4)) {
			err_printf("draw_random_edges()");
			return 1;
		}
		vertex_t v;
		for_each_vertex(pg, v) 
			set_vertex_cost(pg, v, 1);
		
//		snprintf(buf, sizeof(buf),"graph.%d.dot", i);		
//		fprint_graph(fopen(buf, "w"), g);


	} while(i--);
