#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <ncurses.h>

#include "utils.h"
#include "types.h"
#include "graphs.h"
//#include "list.h"
#include "print_graphs.h"

#include "random.h"
#include "vertex_list.h"



struct vertex_list *find_min_path(struct graph *g, vertex_t source, vertex_t target)
{
	
	cost_t   *dist = malloc(g->num_vertices*sizeof(cost_t));
	vertex_t *prev = malloc(g->num_vertices*sizeof(vertex_t));
	vertex_t u;
	struct vertex_list *r = NULL;
	
	enum e_Q { PRESENT, ABSENT };	
	enum e_Q *Q = malloc(g->num_vertices*sizeof(enum e_Q));
	size_t Q_size = 0;
	
	if( !dist || !prev || !Q) {
		err_printf("malloc() returned NULL");
		goto bulk_free;
	}
	
	
	// dbg_printf("Initialization");
	dist[source] = 0;
	prev[source] = UNIT;
	
	vertex_t v;
	
	for_each_vertex(g, v) {
		if( v != source) {
			dist[v] = INFINITY;
			prev[v] = UNIT;
		}
		Q[v] = PRESENT;
		Q_size ++;
	}
	// dbg_printf("distance[] & prev[] computrtion");
	while(Q_size) {
		cost_t min_dist;
		
		//dbg_printf("u ← vertex in Q with min dist[u]");
		
		min_dist = INFINITY;
		for_each_vertex(g, v) 
			if(Q[v] == PRESENT && dist[v] <= min_dist) {
				u = v; 
				min_dist = dist[v];
				
			}													
		
		
		
		if(u == target)
			break;
			
		Q[u] = ABSENT;
		Q_size --;
		
		for_each_neighbor(g, u, v) {
			cost_t alt = dist[u] + get_edge_cost(g, u, v);
			if(alt < dist[v]) {
				dist[v] = alt;
				prev[v] = u;			
			}
		}				
	}
	
	
	u = target;
	struct vertex_list *p = alloc_vertex_list();
	if(!p) {
		err_printf("alloc_vertex_list()");
		goto bulk_free;
	}	

	while(prev[u] != UNIT) {
		if(add_vertex_to_list_head(p, u)) {
			err_printf("add_vertex_to_list_head()");
			goto  free_path;
		}
		
		u = prev[u];	
	}

	r = p;
	
free_path:
	if(!r)
		free_vertex_list(p);
bulk_free:
	if(dist)
		free(dist);
	if(prev)
		free(prev);
	if(Q)
		free(Q);
	
	return r;
}
int _set_path_cost(struct graph *pg, struct graph *g, vertex_t s, vertex_t t, cost_t ec, cost_t vc)
{
	struct vertex_list *p = find_min_path(pg, s, t);
	if(!p) {
		err_printf("find_min_path");
		return -1;
	}
//	fprint_vertex_list(stderr, p);
	struct vertex_list_node *n, *prev = NULL;
	
	for_each_vertex_list_node(p, n) {
		if(likely(prev)) 
			set_edge_cost(g, prev->v, n->v, ec);
		else 
			set_edge_cost(g, s, n->v, ec);
		set_vertex_cost(g, n->v, vc);
		prev = n;
	}
	set_edge_cost(g, prev->v, t, ec);
	
	free_vertex_list(p);
	return 0;
}
int set_path_cost(struct graph *pg, struct graph *g, vertex_t s, vertex_t t, cost_t ec, cost_t vc)
{
	set_vertex_cost(g, s, vc);
	set_vertex_cost(g, t, vc);
	
	if(get_edge_cost(pg, s, t) == INFINITY ) 
		return _set_path_cost(pg, g, s, t, ec, vc);
	
	set_edge_cost(g, s, t, ec);
	return 0;
}
/*
	A "Box Graph" is a grph of this shape:
	
			0--0--0--0
			|\/|\/|\/|
			|/\|/\|/\|
			0--0--0--0
			|\/|\/|\/|
			|/\|/\|/\|
			0--0--0--0
			|\/|\/|\/|
			|/\|/\|/\|
			0--0--0--0
			
	It's qute easy to draw it using ascii ...

 */

void box_graph(struct graph *g, cost_t cost)
{
	vertex_t v, u;
	for_each_edge(g, v, u) 
		set_edge_cost(g,u,v, INFINITY);		
	
	for_each_vertex(g, v) {		
		if( v % g->sqrt_nvert )  
			set_edge_cost(g,v-1,v,cost);		
		if( v / g->sqrt_nvert )  
			set_edge_cost(g,v-g->sqrt_nvert,v,cost);	
		if( v / g->sqrt_nvert && v % g->sqrt_nvert) 
			set_edge_cost(g,v-g->sqrt_nvert-1,v,cost);		
		if( (v+1) / g->sqrt_nvert && (v+1) % g->sqrt_nvert) 
			set_edge_cost(g,v-g->sqrt_nvert+1,v,cost);
		
		
			
	}
}

#define BITS_GET_FILED(name, num) ( ( (num) >> (name ## _UTIL_SHIFT))  & (name ## _UTIL_MASK) ) 
#define BITS_ENCODE_FILED(name, num)  ( ( (num) & (name ## _UTIL_MASK) ) << (name ## _UTIL_SHIFT) )


#define __op_vertex_util(name, g, v, n, op ) set_vertex_util(g, v, BITS_ENCODE_FILED( name, get_vertex_util(g, v) op (n) ) )
#define  or_vertex_util(name, g, v, n) __op_vertex_util(name,g,v,n, | )
#define and_vertex_util(name, g, v, n) __op_vertex_util(name,g,v,n, & )


/*
	visit(start node) 
	queue <- start node 
	WHILE queue is nor empty DO 
	  x <- queue 
	  FOR each y such that (x,y) is an edge 
		          and y has not been visited yet DO 
	    visit(y) 
	    queue <- y 
	  END 
	END 
*/

struct breadth_first_state
{
	struct vertex_list queue;
	size_t depth, visited, dist;
	
	vertex_t parent;
};
struct breadth_first_state *init_breadth_first_state(struct breadth_first_state *s) 
{
	memset(s, 0, sizeof(*s));
	init_vertex_list(&s->queue);
	return s;	
}


typedef enum  { VISITOR_OK = 0, VISITOR_STOP = 1, VISITOR_FAILURE = -1  } visitor_return_t;
typedef visitor_return_t (*visitor_cb_t)(vertex_t, struct breadth_first_state* );
#define bft_visitor_lambda( body) ({ visitor_return_t __fn__ body; __fn__; })


#define BFT_UTIL_WIDTH 8
#define BFT_UTIL_MASK  0xff
#define BFT_UTIL_SHIFT 0


#define BFT_UTIL_DIST_MASK  0xfe

#define BFT_UTIL_VISITED  	 1
#define BFT_UTIL_ENCODE_DIST(u)     ( (u << 1) & BFT_UTIL_DIST_MASK )


#define BFT_UTIL_GET_DIST(u)   ( (u &  BFT_UTIL_DIST_MASK ) >> 1 )





int _breadth_first_traversal(struct graph *g, vertex_t src, struct breadth_first_state *s, visitor_cb_t visit) 
{
	vertex_t x, u;
	
	
	switch(visit(src, s)) {
		case VISITOR_FAILURE: 
			err_printf("visit()"); 	
			return -1;		
		case VISITOR_STOP :  
			return 0;
		case VISITOR_OK : 
			s->visited ++; 
			/* FALL THROUGHT */
	}
	
	or_vertex_util(BFT, g, src, 1);

	
	if(add_vertex_to_list_head(&s->queue, src)) {
		err_printf("add_vertex_to_list_head()");
		return -1;
	}

	while(!vertex_list_empty(&s->queue)) {
		x = pop_vertex_from_list_tail(&s->queue);
		s->depth ++;
		s->parent = x;
		s->dist = BFT_UTIL_GET_DIST(
				BITS_GET_FILED(BFT, get_vertex_util(g, x))
			) + 1; 

		for_each_neighbor(g, x, u) {	

			if(!(get_vertex_util(g, u) & 1)) {
				
				or_vertex_util(BFT, g, u, (BFT_UTIL_ENCODE_DIST(s->dist) | BFT_UTIL_VISITED) );
				
				switch(visit(u, s)) {
					case VISITOR_FAILURE: 
						err_printf("visit()"); 	
						return -1;		
					case VISITOR_STOP :  
						return 0;
					case VISITOR_OK : 
						s->visited ++; 
						/* FALL THROUGHT */
				}

				if(add_vertex_to_list_head(&s->queue, u)) {
					err_printf("add_vertex_to_list_head()");
					return -1;
				}
			}
		}
		
	}

	

	return 0;
	
}
static inline int breadth_first_traversal(struct graph *g, vertex_t src, struct breadth_first_state *s, visitor_cb_t visit) 
{
	int r = _breadth_first_traversal(g, src, s, visit);
	clear_vertex_util(g, (cost_t)~BFT_UTIL_MASK);	
	return r;
}

#define IPG_UTIL_WIDTH 1
#define IPG_UTIL_MASK  0x1
#define IPG_UTIL_SHIFT (BFT_UTIL_WIDTH + BFT_UTIL_SHIFT) // 8 


#define IPG_VISIT_VERTEX 1

static inline void set_vertex_visit_flag(struct graph *g, vertex_t v) 
{		
	const cost_t mask = BITS_GET_FILED( IPG, get_vertex_util(g, v)) | IPG_VISIT_VERTEX;
	set_vertex_util(g, v, BITS_ENCODE_FILED( IPG, mask));
}
static inline void unset_vertex_visit_flag(struct graph *g, vertex_t v) 
{
	const cost_t mask = BITS_GET_FILED( IPG, get_vertex_util(g, v)) & ((~IPG_VISIT_VERTEX) & IPG_UTIL_MASK);
	set_vertex_util(g, v, BITS_ENCODE_FILED( IPG, mask) );
}
static inline int get_vertex_visit_flag(const struct graph *g, vertex_t v) 
{
	return  BITS_GET_FILED( IPG, get_vertex_util(g, v)) & IPG_VISIT_VERTEX;
}

#define BA_UTIL_WIDTH 8
#define BA_UTIL_MASK  0xff
#define BA_UTIL_SHIFT (IPG_UTIL_WIDTH + IPG_UTIL_SHIFT)


typedef u8 degree_t;

static inline degree_t get_vertex_degree(struct graph *g, vertex_t v)
{
	return BITS_GET_FILED( BA, get_vertex_util(g, v));
}
static inline void set_vertex_degree(struct graph *g, vertex_t v, degree_t degree)
{
	set_vertex_util(g, v, BITS_ENCODE_FILED( BA, degree));
}

static inline degree_t add_to_vertex_degree(struct graph *g, vertex_t v, signed int num)
{
	degree_t d = get_vertex_degree(g,v), r = (degree_t) (((int)d) + num);
	
	set_vertex_degree(g,v,  r);
	return r;
}

int create_BA_graph(struct graph *g, struct cat_state  *cs, vertex_t vs[], size_t vs_size) 
{
	size_t j, i, sum = 1;
	int r = -1;
	
	
	
	struct cat_parameters cp;

	cp.cdf = malloc(sizeof(double) * vs_size);
	if(!cp.cdf) {
		err_printf("malloc()");
		return -1;
	}

	struct graph *pg = g;
	


	
	for(i=1; i<vs_size; ++i) {
		set_vertex_degree(g, vs[i], 0);

	}
	set_vertex_degree(g, vs[0], 1);

	
	for(i=1; i < vs_size; ++i) {
		


		// Init cat params
		cp.cdf[0] = ((double)get_vertex_degree(g, vs[0]))/sum;
		for(j = 1; j < i; ++j ) {
			double weight = ((double)get_vertex_degree(g, vs[j]))/sum;	
			cp.cdf[j] = cp.cdf[j-1] + weight;
		}		
		cp.count = i;
		
		vertex_t v = vs[sample_cat(cs,&cp)];
		
		add_to_vertex_degree(g, v, 1);
		
		if(set_path_cost(pg, g, v, vs[i], 10, 10)){
			err_printf("set_path_cost()");
			goto free_cdf;
		}
		sum ++;
		
		//finit_cat_parameters(&cp);
	}
	r = 0;

free_cdf:		
	free(cp.cdf);
	return r;
}


void set_rand_vertex_color(struct graph *g, struct uniform_state *s, size_t min, size_t max)
{
	
	u64 num = sample_uniform_u64_bounded(s, max, min);
	
	while(num--) {
		vertex_t v = sample_uniform_u64_bounded(s, num_vertices(g)-1, 0);
		set_vertex_visit_flag(g, v);
		set_vertex_cost(g, v, 10 );
	}
}



void test_random(FILE *fp, size_t count) 
{
	struct cat_state  cs;
	struct cat_parameters cp;
	
	double w[] = { 0.5,  0.1,  0.1, 0.3 };
	
	init_cat_state( &cs, time(NULL));
	init_cat_parameters(&cp, w, ARRSIZE(w));
	
	fprintf(fp, "{\n\t\"data\" :\n\t\t[ ");
	size_t i = 0;
	for(; i < count - 1 ; ++i) 
		fprintf(fp, " %d,", sample_cat(&cs,&cp) );
	fprintf(fp, " %d ],\n", sample_cat(&cs,&cp) );
	fprintf(fp, "\t\"count\" : %ld\n}", count);
	
	finit_cat_parameters(&cp);
	
	
}

//TODO: MORE ROBUST
ssize_t select_random_vertexes(struct graph *g, struct uniform_state *s, size_t min, size_t max, vertex_t vs[], size_t vs_size)
{
	ASSERT(vs_size < max || max < min, return -1, "bad parameters passed");
	u64 num = sample_uniform_u64_bounded(s, max, min);
	ssize_t i = 0;
	for(i=0; i< num; ++i) {
		vertex_t v = sample_uniform_u64_bounded(s, num_vertices(g)-1, 0);
		vs[i] = v;
	}
	return i;
}
int create_random_graph(struct graph *g, struct cat_state *cs, vertex_t pr[], size_t pr_num)
{		

	
	vertex_t vs[32];
	ssize_t num = select_random_vertexes(g, &cs->uniform, 2, 4, vs, ARRSIZE(vs));
	if(num == -1) {
		err_printf("select_random_vertexes()");
		return -1;		
	}
	size_t i;
	for(i=0; i < pr_num; ++num, ++i) {
		
		vs[num-1] = pr[i];
	}

	num --;


	//for(i=0; i< num; ++i) 
	//	dbg_printf("vs\\ %ld", vs[i]);
	//for(i=0; i< pr_num; ++i) 
	//	dbg_printf("pr/ %ld", pr[i]);
	
	if(create_BA_graph(g, cs, vs, num) ){
		err_printf("create_BA_graph()");
		return -1;
	}
	return 0;
}








typedef struct 
{
	ssize_t x, y;
} pos_t;

typedef u16 map_data_t;

typedef u8 texture_t;
typedef u8 pos_flag_t;

struct map_pice
{
	map_data_t **pice;	
	size_t size_x, size_y;
	
	pos_t free_pos;
	bool setit;
	size_t nitems;
};

#define ITEMS_UTIL_WIDTH 16
#define ITEMS_UTIL_MASK  0xff
#define ITEMS_UTIL_SHIFT (BA_UTIL_WIDTH + BA_UTIL_SHIFT)


typedef u16 map_data_t;

static inline map_data_t get_vertex_item(struct graph *g, vertex_t v)
{
	return BITS_GET_FILED( ITEMS, get_vertex_util(g, v));
}
static inline void set_vertex_item(struct graph *g, vertex_t v, map_data_t item)
{
	set_vertex_util(g, v, BITS_ENCODE_FILED( ITEMS, item));
}

void drop_random_items(struct graph *g, struct cat_state *cs)
{

	vertex_t v;

	for_each_vertex(g, v) {
	
		if(sample_uniform_f64(&cs->uniform) >= 0.5) {		
			
			set_vertex_item(g, v, (u16)0x1 | 'o');			
			
			
		} else {
			set_vertex_item(g, v, 0);
		}
	}
	
}



static inline void set_pos_data(struct map_pice *mp, pos_t pos, map_data_t data)
{
	mp->pice[pos.y][pos.x] = data;
}
static inline map_data_t get_pos_data(struct map_pice *mp, pos_t pos)
{
	return mp->pice[pos.y][pos.x];
}


static inline texture_t get_pos_texture(struct map_pice *mp, pos_t pos)
{
	return get_pos_data(mp, pos) & 0xFF;
}
static inline void set_pos_texture(struct map_pice *mp, pos_t pos, texture_t texture)
{
	set_pos_data(mp, pos, texture | (get_pos_data(mp, pos) & 0xFF00) );
}


static inline pos_flag_t get_pos_flags(struct map_pice *mp, pos_t pos)
{
	return get_pos_data(mp, pos) & 0xFF00;
}
static inline void set_pos_flags(struct map_pice *mp, pos_t pos, pos_flag_t flag)
{
	set_pos_data(mp, pos, flag | (get_pos_data(mp, pos) & 0xFF) );
}


static inline int is_item(struct map_pice *mp, pos_t p)
{
	return get_pos_texture(mp, p) == 'o';
}


static inline pos_t build_pos_t(size_t x, size_t y)
{
	pos_t r; r.x = x;r.y = y;	
	return r;
}

static inline void fill_map_pice(struct map_pice *mp, map_data_t data)
{
	int i, j;
	for(i=0; i<mp->size_y; ++i)
		for(j=0; j<mp->size_x; ++j) {
			
			set_pos_data(mp, build_pos_t(j,i), data);
		}
}



int render_map_pice(WINDOW *win, pos_t start, pos_t end, struct map_pice *mp)
{
	pos_t pos;
	
	size_t i, j;
	for(i = 0, pos.y = start.y; pos.y < end.y; ++pos.y, ++i)
		for(j = 0, pos.x = start.x; pos.x < end.x; ++pos.x, ++j) {
			texture_t tx = get_pos_texture(mp, pos);			

			if( mvwaddch (win, i, j, tx) == ERR ) {
				err_printf("mvwaddch( y=%ld x=%ld %c)", i, j, tx);
				return -1;
			}
			
		}
	return 0;
}
struct line_styles
{	
	u8  hor_width;
	u8 vert_width;
	u8 diag_width;
};

int draw_road(struct map_pice *mp, pos_t start, pos_t end, struct line_styles *st)
{
	ASSERT( (st->hor_width & 1) == 0, DO_ABORT, "width should be even, %d is not", st->hor_width);
	size_t x, y;
	ssize_t w;
	dbg_printf("from (%ld,%ld) to (%ld, %ld)", start.x, start.y, end.x, end.y);
	size_t min_x = MIN(start.x, end.x), max_x = MAX(start.x, end.x);
	size_t min_y = MIN(start.y, end.y), max_y = MAX(start.y, end.y);
	s8 sign = ((max_x == start.x && max_y == start.y) || (max_x == end.x && max_y == end.y)) ? 1 : -1;
	
	if(start.y == end.y) {
		y = start.y;
		for(x = min_x; x < max_x + 1; x+=sign) {	
			s8 half = st->hor_width >> 1;
			for(w=-half; w <= half; ++w) 
				set_pos_texture(mp, build_pos_t(x, y + w), ' ');		

		}
	} else if (start.x == end.x) {
		x = start.x;
		for(y = min_y; y < max_y + 1; y+=sign) {	
			s8 half = st->vert_width >> 1;
			for(w=-half; w <= half; ++w) 
				set_pos_texture(mp, build_pos_t(x + w, y), ' ');		
	
			
		}
	} else {		
		s8 half = st->diag_width >> 1;
	
		if(min_x == end.x) {
			ssize_t x = end.x, y = end.y;
			
			for(;;){

				
					
				for(w=-half; w <= half; ++w) 					
					set_pos_texture(mp, build_pos_t(x + w, y), ' ');
					
				if( y != start.y) y += sign;
				else  half = st->hor_width >> 1;
				
				if( x < start.x ) ++x;
				else half = st->vert_width >> 1;
									
				if( y == start.y && x == start.x )
					break;
			}
			
		} else {	
			ssize_t x = start.x, y = start.y;
			
			for(;;) {

				
				
				for(w=-half; w <= half; ++w) 
					set_pos_texture(mp, build_pos_t(x + w, y), ' ');

				if( y != end.y) y += sign;
				else  half = st->hor_width >> 1;
				
				if( x < end.x ) ++x;		
				else half = st->vert_width >> 1;		
				
				if( y == end.y && x  == end.x)
					break;

			}			

		}
		
		
		
		
	} 
	
	return 0;
}

const pos_t err_pos = {
	.x = -1,
	.y = -1,
};


static inline int be_is_vertex_top(struct graph *g, vertex_t v)
{	
	return v/g->sqrt_nvert == 0;
}
static inline int be_is_vertex_left(struct graph *g, vertex_t v)
{	
	return v%g->sqrt_nvert == 0;
}
static inline int be_is_vertex_right(struct graph *g, vertex_t v)
{	
//	dbg_printf(" %ld", v);
	return v%g->sqrt_nvert == g->sqrt_nvert-1;
}
static inline int be_is_vertex_bottom(struct graph *g, vertex_t v)
{	
	return v/g->sqrt_nvert == g->sqrt_nvert-1;
}
static inline int be_is_vertex_edge(struct graph *g, vertex_t v)
{
	return (v/g->sqrt_nvert == 0) || (v%g->sqrt_nvert == 0)||
		(v%g->sqrt_nvert == g->sqrt_nvert-1) || (v/g->sqrt_nvert == g->sqrt_nvert-1);
}

vertex_t next_box_edge_vertex_all(struct graph *g, vertex_t prev) 
{
	vertex_t v;
	_for_each_vertex( g, v,prev == UNIT ? 0 : prev + 1) {
		if(be_is_vertex_edge(g, v))
			return v;
	}
	return UNIT;
}
#define __declare_next_fn(p) \
static inline vertex_t next_box_edge_vertex_ ## p (struct graph *g, vertex_t prev) \
{\
	vertex_t v;\
	_for_each_vertex( g, v, prev == UNIT ? 0 : prev + 1) { \
		if(be_is_vertex_##p (g, v)) 			\
			return v;		\
	}				\
	return UNIT; \
}
__declare_next_fn(top);
__declare_next_fn(left);
__declare_next_fn(right);
__declare_next_fn(bottom);
#undef __declare_next_fn

#define for_each_box_edge_vertex(p, g, v) \
	for(v = next_box_edge_vertex_ ## p (g, UNIT);  v != UNIT; v = next_box_edge_vertex_ ## p(g, v))
#define next_box_edge_vertex(p, g, v) next_box_edge_vertex_ ## p (g, UNIT)

static inline  vertex_t be_right_to_left_vertex(struct graph *g, vertex_t r)
{
	
	return r - (g->sqrt_nvert - 1);
}
static inline  vertex_t be_left_to_right_vertex(struct graph *g, vertex_t r)
{
	return r + (g->sqrt_nvert - 1);
}

static inline  vertex_t be_bottom_to_top_vertex(struct graph *g, vertex_t r)
{
	return r - (g->sqrt_nvert - 1)*g->sqrt_nvert;
}
static inline  vertex_t be_top_to_bottom_vertex(struct graph *g, vertex_t r)
{
	return r + (g->sqrt_nvert - 1)*g->sqrt_nvert;
}
struct neighbor_graphs
{
	struct graph *left, *right, *top, *bottom;
};


pos_t draw_graph(struct map_pice *mp, 

	struct graph *g, struct neighbor_graphs *ns, 
	
	pos_t start, size_t bx_size, size_t by_size, 
	
	struct line_styles *road_size)
{
	
	pos_t block_center;
	
	block_center.x = start.x + (bx_size>>1);
	block_center.y = start.y + (by_size>>1);
	

	vertex_t v, u;
	
	for_each_vertex(g, v) {	

		//dbg_printf("(%ld, %ld)",block_center.x, block_center.y );

		for_each_neighbor(g, v, u) {
			if(get_edge_cost(g, v, u)) {
				//dbg_printf("u: %ld v: %ld " , u, v);

				if(v == u - 1) {
					ASSERT( draw_road(mp, block_center, 
							build_pos_t(block_center.x + bx_size, 
									block_center.y ), road_size), 
						return err_pos, "" );
					//dbg_printf("(v == u - 1)");
				}
				if(v == u - g->sqrt_nvert) {
					ASSERT( draw_road(mp, block_center, 
							build_pos_t(block_center.x, 
									block_center.y + by_size),road_size)
						, return err_pos, "" );
									
					//dbg_printf("(v == u + g->sqrt_nvert)");
				}


				if(v == u - (g->sqrt_nvert + 1)) {
			
					ASSERT( draw_road(mp, block_center, 
							build_pos_t(block_center.x+ bx_size, 
									block_center.y + by_size),road_size)
						, return err_pos, "" );
					//dbg_printf("(v == u - g->sqrt_nvert + 1 %d)", u - g->sqrt_nvert + 1);		
				}
		

				if(v == u - (g->sqrt_nvert - 1) ) {
					//dbg_printf("(v == u - (( g->sqrt_nvert - 1) ");		
				
					ASSERT( draw_road(mp, block_center, 
							build_pos_t(block_center.x - bx_size, 
									block_center.y + by_size ),road_size)
						, return err_pos, "" );
				
				}
			}			
		}

		if(ns && be_is_vertex_edge(g,v)) {
			if(ns->left && be_is_vertex_left(g, v) ) {
				if(get_vertex_cost(ns->left, be_left_to_right_vertex(ns->left,v)) &&
					get_vertex_cost(g, v)
				) {
					ASSERT( draw_road(mp, 							
							build_pos_t(block_center.x - bx_size, block_center.y ),
							block_center, 
							road_size)
							, return err_pos, "" );	
//					dbg_printf(" !~~~~~~~~ hello! %ld and %ld", v,  be_left_to_right_vertex(ns->left,v));
				}
			}
			if(ns->right && be_is_vertex_right(g, v) ) {
				if(get_vertex_cost(ns->right, be_right_to_left_vertex(ns->right,v)) &&
					get_vertex_cost(g, v)
				) {
					ASSERT( draw_road(mp, 							
							block_center, 
							build_pos_t(block_center.x - bx_size, block_center.y ),
							road_size)
							, return err_pos, "" );	
//					dbg_printf(" !~~~~~~~~ hello! %ld and %ld", v,  be_left_to_right_vertex(ns->left,v));
				}
			}
			if(ns->top && be_is_vertex_top(g, v) ) {
				if(get_vertex_cost(ns->top, be_top_to_bottom_vertex(ns->top,v)) &&
					get_vertex_cost(g, v)
				) {
					ASSERT( draw_road(mp, 							
							block_center, 
							build_pos_t(block_center.x, block_center.y - by_size ),
							road_size)
							, return err_pos, "" );	
//					dbg_printf(" !~~~~~~~~ hello! %ld and %ld", v,  be_left_to_right_vertex(ns->left,v));
				}
			}
			if(ns->bottom && be_is_vertex_bottom(g, v) ) {
				if(get_vertex_cost(ns->bottom, be_bottom_to_top_vertex(ns->bottom,v)) &&
					get_vertex_cost(g, v)
				) {
					ASSERT( draw_road(mp,							
							build_pos_t(block_center.x - bx_size, block_center.y + by_size ),
							block_center, 							
							road_size)
							, return err_pos, "" );	
//					dbg_printf(" !~~~~~~~~ hello! %ld and %ld", v,  be_left_to_right_vertex(ns->left,v));
				}
			}
		}
//		set_pos_texture(mp, block_center, 'o');
		map_data_t item = get_vertex_item(g, v);

		if(get_pos_texture(mp, block_center) == ' ' && item) {
			mp->nitems ++;				
			set_pos_data(mp, block_center, item);
		}
		
		if(get_pos_texture(mp, block_center) == ' ' && mp->setit) {
			mp->free_pos = block_center;
			mp->setit = false;
		}
	
		block_center.x += bx_size;

		if( v % g->sqrt_nvert ==  g->sqrt_nvert - 1 )  {
			
			block_center.x = start.x + (bx_size>>1);	
			block_center.y += by_size;
//			dbg_printf("--> (%ld, %ld)",block_center.x, block_center.y );
			
		}




	}
	
	block_center.x += bx_size*(g->sqrt_nvert-1);	
	block_center.y -= by_size ;
	
	block_center.x +=  (bx_size>>1) + 1;
	block_center.y +=  (by_size>>1) + 1;
	
	return block_center;
	

	
}

#define dbg_function(msg) dbg_printf("%s",  msg);

int create_random_neighbor_graph(struct neighbor_graphs *ng, struct graph *new,struct cat_state *cs)
{
	vertex_t v;	
	vertex_t set[32];	
	size_t i = 0;
	
	dbg_function("begin");
	
	if(ng->left) {
		dbg_function("left");		
		struct graph *g = ng->left;
		for_each_box_edge_vertex(right, g, v) {	
			dbg_printf("  %d ", v);
			if(get_vertex_cost(g, v)) {
				set[i] = be_right_to_left_vertex(g, v);
				dbg_printf(" %ld %ld", set[i], v);
				i++;
			}
		}
	}

	if(ng->right) {
		dbg_function("right");		
		struct graph *g = ng->right;
		for_each_box_edge_vertex(left, g, v)  		
			if(get_vertex_cost(g, v)) {
				set[i] = be_left_to_right_vertex(g, v);
				dbg_printf(" %ld %ld", set[i], v);
				i++;
			}
	}
	if(ng->top) {
		dbg_function("top");		
		struct graph *g = ng->top;
		for_each_box_edge_vertex(bottom, g, v)  		
			if(get_vertex_cost(g, v)) {
				set[i] = be_bottom_to_top_vertex(g, v);
				dbg_printf(" %ld %ld", set[i], v);
				i++;
			}
	}
//	dbg_function("left top");
	if(ng->bottom) {
		dbg_function("bottom");		
		struct graph *g = ng->bottom;
		for_each_box_edge_vertex(top, g, v)  		
			if(get_vertex_cost(g, v)) {
				set[i] = be_top_to_bottom_vertex(g, v);
				dbg_printf(" %ld %ld", set[i], v);
				i++;
			}
			
	}		

	dbg_function("done here");
	if(create_random_graph(new, cs, set, i )) {
		err_printf("create_random_graph()");
		return -1;
	}	
	dbg_function("end");
	return 0;
}
struct graph *new_random_box_graph(size_t sqrt_size, struct neighbor_graphs *ng, struct cat_state *cs, vertex_t vs[], size_t vs_size)
{
	struct graph *g = alloc_graph(sqrt_size*sqrt_size, sqrt_size);
	if(g) {
		box_graph(g, 0);

		if(ng) {
			if(create_random_neighbor_graph(ng, g, cs)) {
				err_printf("create_random_graph()");
				free_graph(g);
				return NULL;
			}			 
		} else {
			if(create_random_graph(g, cs, vs, vs_size)) {
				err_printf("create_random_graph()");
				free_graph(g);
				return NULL;
			}
		}
		drop_random_items(g, cs);
	}
	
	dbg_function("end");
	return g;
}
struct roads_style 
{
	struct line_styles ls;
	size_t block_size_x, block_size_y;
	
};
struct map_style 
{
	struct roads_style rs;
	size_t num_super_blocks_x, num_super_blocks_y, sqrt_vs;
	
};

int create_roads_on_map_pice(struct map_pice *mp,struct map_style *s, struct cat_state *cs )
{
	struct roads_style *rs = &s->rs;
	size_t nvs = s->sqrt_vs;
	
	struct graph **upper, *p;	
	struct neighbor_graphs ng = {};
	
		
	upper = malloc(sizeof(struct graph*) * s->num_super_blocks_x);
	if(!upper) {
		err_printf("malloc()");
		return -1;
	}
	memset(upper, 0, sizeof(struct graph*) * s->num_super_blocks_x);
	vertex_t pr[] = {
		10, 14, 2, 22
	};
	
	p = new_random_box_graph(nvs, NULL, cs,pr, ARRSIZE(pr));
	if(!p) {
		err_printf("new_random_box_graph");
		free(upper);
		return -1;
	}
	
	size_t i = 1;
	upper[0] = p;
	
	pos_t r = draw_graph(mp, upper[0], NULL, build_pos_t(0, 0), rs->block_size_x, rs->block_size_y, &rs->ls);
	
	ssize_t y_pos = 0,
		x_pos = r.x;
		
		
	dbg_printf(":::::::::::::::::: %ld %ld -- > %ld %ld  == %ld %ld", mp->size_x,mp->size_y, x_inc, y_inc
		, s->rs.block_size_x * s->sqrt_vs, s->rs.block_size_y * s->sqrt_vs );
	for(; y_pos < mp->size_y ;) {
		for(; x_pos < mp->size_x ;) {
		
			ng.top = upper[i];
			ng.left = p;
			dbg_printf(" %p %p", ng.top, ng.left);
			
			p = new_random_box_graph(nvs, &ng, cs, NULL, 0);
			if(!p) {
				err_printf("new_random_box_graph");
				free(upper);
				return -1;
			}			
			upper[i] = p;
			
			r = draw_graph(mp, p, &ng, build_pos_t(x_pos, y_pos), rs->block_size_x, rs->block_size_y, &rs->ls);
			
			x_pos = r.x;
			i++;
		}	
		dbg_printf(":::::::::::::::::: %ld %ld", x_pos,y_pos);		
		y_pos = r.y;
		x_pos = 0;
		i = 0;	
		
	}
	free(upper);
	return 0;
	
}
void free_map_pice(struct map_pice *mp)
{
	ssize_t i;
	for(i=0; i < mp->size_y; ++i) 
		if(mp->pice[i])
			free(mp->pice[i]);
	free(mp->pice);
	free(mp);
}
struct map_pice *alloc_map_pice(size_t xs, size_t ys)
{
	struct map_pice *mp = malloc(sizeof(struct map_pice));
	if(mp) {
		mp->pice = malloc(sizeof(map_data_t*) * ys );
		if(!mp->pice) {
			err_printf("malloc()");
			free(mp);
			return NULL;
		}
		
		memset(mp->pice, 0, sizeof(map_data_t*) * ys);
		
		size_t i;
		for(i=0; i < ys; ++i) {
			mp->pice[i] = malloc(sizeof(map_data_t)*xs);
			if(!mp->pice[i]) {
				free_map_pice(mp);
				err_printf("mallolc()");
				return NULL;
			}
		}
		mp->size_y =  ys;
		mp->size_x =  xs;	
		mp->setit = true;	
		
	}
	dbg_printf("x_size=%ld, y_size=%ld", xs, ys);
	return mp;
}

struct map_pice *new_map_pice(struct map_style *ms)
{
	size_t xs = ms->rs.block_size_x * ms->num_super_blocks_x * ms->sqrt_vs;
	size_t ys = ms->rs.block_size_y * ms->num_super_blocks_y * ms->sqrt_vs;
	
	struct map_pice *mp = alloc_map_pice(xs, ys);
	if(mp) {
		fill_map_pice(mp, '#');
	}
	return mp;
}
void print_map_style_info(struct map_style *s)
{	
	dbg_printf("block_size: ");
		dbg_printf("\tx: %ld", s->rs.block_size_x);
		dbg_printf("\ty: %ld", s->rs.block_size_y);
	dbg_printf("super_block_size: ");
		dbg_printf("\tx: %ld", s->rs.block_size_x*s->sqrt_vs);
		dbg_printf("\ty: %ld", s->rs.block_size_y*s->sqrt_vs);		
	dbg_printf("map_pice_size: ");
		dbg_printf("\tx: %ld",  s->rs.block_size_x * s->num_super_blocks_x * s->sqrt_vs);
		dbg_printf("\ty: %ld",  s->rs.block_size_y * s->num_super_blocks_y * s->sqrt_vs);				
		
}






struct camera 
{
	pos_t ul;
	size_t size_x, size_y;
	
};

void move_camera(struct camera *c, struct map_pice *mp, ssize_t x, ssize_t y)
{
	c->ul.y -= y;
	c->ul.x += x;
	
	if( c->ul.x < 0 ) c->ul.x = 0;
	if( c->ul.y < 0 ) c->ul.y = 0;
	
	if( c->ul.x + c->size_x >= mp->size_x) c->ul.x = mp->size_x - c->size_x;
	if( c->ul.y + c->size_y >= mp->size_y) c->ul.y = mp->size_y - c->size_y;

}




static inline int is_texture_walkable(texture_t tx)
{
	switch(tx) {
		case ' ': case 'o': return 1;
		default : return 0;
	}
}





struct player
{
	texture_t tx, prev_tx;
	pos_t pos, prev;
	
	int items;
};
int move_player(struct map_pice *mp, struct player *p, size_t x, size_t y)
{
	pos_t prev = p->pos;
	
	pos_t n = build_pos_t(p->pos.x + x, p->pos.y + y);
	if(is_texture_walkable(get_pos_texture(mp, n))) {
	
		if( is_item(mp, n) ) {
			p->prev_tx = ' ';	
			p->items ++;
		} else {
			p->prev_tx = get_pos_texture(mp, n);
		}
		
		set_pos_texture(mp, prev, p->prev_tx);
	
		p->prev = p->pos;
		p->pos = n;
		
		set_pos_texture(mp, n, p->tx);		
		return 1;
	}
	return 0;
	/*
	for(;;) {
		n = build_pos_t(p->pos.x + i, p->pos.y + j);		
		if(!is_texture_walkable(get_pos_texture(mp, n)) || (i == x && j == y))
			break;			
		j += y > 0 ? 1 : y < 0 ? -1 : 0;
		i += x > 0 ? 1 : x < 0 ? -1 : 0;
	}
	*/
		
	
}

char *depressive_list[]=
{ 
	"  you have only %4d diamonds, how worthless you are  ",
	"  oh %4d diamonds       ",
	"  you really wannna go on ? ",
	"  dude %4d damonds !    ",
	"  you depress me, %4d diamonds, stop playing ",
	"  okay, %4d is a good number, stop ",
	"  ....                            ",
	"  with these stones you can buy real stuff, go on, billionare ",
	"  start selling these diamonds on ebay,  they sell !",
	"  don't stop %4d stones in the pocket you can do better!  ",
	"  STOP !            ",
	"  i leave, you are hopeless            ",
	"  diamonds : %4d          ",
	
};

static inline char *get_depressive_mesage(size_t i) 
{
	if(i < ARRSIZE(depressive_list))
		return 	depressive_list[i];
	return depressive_list[ARRSIZE(depressive_list)-1] ;
}
void display_stats_win(WINDOW *win, pos_t p, struct player *pr, bool mv_cam, size_t nitems)
{
	char *c = get_depressive_mesage( pr->items );
	if(pr->items == nitems) 
		c = "guess what?  Nothing! %d items";
	
	size_t len = strlen(c)+1;
	ssize_t b = len - strlen("  move observer: XXXX ");
	if(b < 0)
		b = 0;
	mvwprintw(win,p.y + 0, p.x, "%*s", len, " ");	
	mvwprintw(win,p.y + 1, p.x, c, pr->items);	
	mvwprintw(win,p.y + 2, p.x, "  move observer: %s%*s ", mv_cam ? " yes" : "nope", b, " ");	
	mvwprintw(win,p.y + 3, p.x, "%*s", len, " ");	
	


}
int main()
{		
	struct map_style s = {
		.rs = {
			.ls = {
				.hor_width = 1,
		 		.vert_width = 3,
				.diag_width = 3,
			}, 
			.block_size_x = 13,
			.block_size_y = 7,
		},
		.num_super_blocks_x = 20,
		.num_super_blocks_y = 20,	
		.sqrt_vs = 5,
	};
	print_map_style_info(&s);

	
	struct map_pice *mp = new_map_pice(&s);
	if(!mp) {
		err_printf("new_map_pice"); 
		return EXIT_FAILURE;
	}
	struct cat_state cs;
	init_cat_state(&cs, time(NULL));	// 4, 8
	
	

	if(create_roads_on_map_pice(mp, &s, &cs)) {
		err_printf("create_roads_on_map_pice");
		return EXIT_FAILURE;
	}

	printf("\n\n\n\nYou are a gnome, who controls another gnome who, in it's turn collects diamonds.\n");
	printf( "  Use:\n"
		"\tARROWS -- to move the observed/observer gnome.\n"
		"\tm -- to switch between the observed gnome and the observer gnome.\n"
		"\ts -- to exit the game.\n\n\n\nPress <enter> key to start.");
	getchar();
	printf("\n\n\n\tP.S.\n\t\tIf you press the wrong keys you kill two gnomes and a terminal.\n");

	getchar();
	printf("\n\n\n\n No, really it's not funny");
	getchar();
	printf("\n            murder...");
	getchar();
	initscr();			
	keypad(stdscr, TRUE);		
	noecho();
	curs_set(0);
	

	
	
	int ch = KEY_DOWN;	
	bool mv_cam = false;
	struct player player = {
		.tx = '+', .prev_tx = ' ',
		.pos = {
			.x = 6, .y = 8
		}
	};
	timeout(1);
	player.pos = mp->free_pos;
	
	
	struct camera cam;
	
	cam.ul = build_pos_t(0,0);
	getmaxyx(stdscr, cam.size_y, cam.size_x);	
	cam.size_x -= 1;	// why ?!
	
	

	int cnt = 1;
	for(;;) {		
		
		render_map_pice(stdscr, cam.ul, build_pos_t(cam.ul.x + cam.size_x , cam.ul.y + cam.size_y), mp);
		display_stats_win(stdscr, build_pos_t(cam.size_x/2,0),&player, mv_cam, mp->nitems);

		refresh();

		if(cnt == 0) {
			ch = getch();	
		} else {
			cnt --;
		}
		
		
		switch(ch) {
			case KEY_UP: 	
				if(mv_cam ||move_player(mp, &player, 0, -1))
					move_camera(&cam, mp, 0, 1);

				break;
			case KEY_DOWN:  
				if(mv_cam ||move_player(mp, &player, 0, 1))
					move_camera(&cam, mp, 0, -1); 

				break;
			


			case KEY_RIGHT:  
				if(mv_cam ||move_player(mp, &player, 1, 0))			
					move_camera(&cam, mp, 1, 0); 

				break;
			case KEY_LEFT: 	
				if(mv_cam || move_player(mp, &player, -1, 0))
					move_camera(&cam, mp,  -1, 0); 
					
				break;			
			case 'm':
				mv_cam = !mv_cam;
				break;
			case 's':
				goto score;
			case -1:
				break;
			default: {
#define JUNK_BUF_SIZE 1000
				u8 buf[JUNK_BUF_SIZE];
				size_t i, j;
				
				for(i=0; i<JUNK_BUF_SIZE; i++) 
					buf[i] = sample_uniform_u64(&cs.uniform) & 0xff;
					

				for(j=0; j<10; j++) 
					for(i=0; i<JUNK_BUF_SIZE; i++) 
						fprintf(stderr, "%c", buf[(i*j)%JUNK_BUF_SIZE] &0x7f  );
				
				fprintf(stderr, "\n\n\n\n\n\nGAME OVER\n\n\n\n\n");


				
				main();

			}
		}

				
	}
	
	
score:
	
	endwin();			

	printf("AWSEOME!\n");
	printf("No one was injured, even your term!\n");
	printf("%d stones! Here they are, take them.\n", player.items);
	int i;
	for(i=0; i<player.items; ++i) 
		printf("/\\");	
	printf("\n");

	for(i=0; i<player.items; ++i) 
		printf("\\/");
	
	printf("\n");
	printf("But %d are left. Some one else will collect them, not you!\n", mp->nitems - player.items);
	return 0;
}

/*
	debug from print_map_style_info() : 1172  -- block_size: 
debug from print_map_style_info() : 1173  -- 	x: 13
debug from print_map_style_info() : 1174  -- 	y: 7
debug from print_map_style_info() : 1175  -- super_block_size: 
debug from print_map_style_info() : 1176  -- 	x: 65
debug from print_map_style_info() : 1177  -- 	y: 35
debug from print_map_style_info() : 1178  -- map_pice_size: 
debug from print_map_style_info() : 1179  -- 	x: 1300
debug from print_map_style_info() : 1180  -- 	y: 700

*/
