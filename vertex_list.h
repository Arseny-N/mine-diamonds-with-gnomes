#ifndef __VERTEX_LIST_H__
#define __VERTEX_LIST_H__

#include "list.h"
#include "types.h"

#include <string.h> // memset()

struct vertex_list_node
{
	struct list_head list;
	vertex_t v;	
};
struct vertex_list
{
	struct vertex_list_node *nodes;
	size_t length;
};	



#define for_each_vertex_list_node(vl, vln) list_for_each_entry(vln, &(vl->nodes->list), list) 

static inline int vertex_list_empty(const struct vertex_list *l)
{	
	return l->nodes == NULL;
}
static inline void init_vertex_list(struct vertex_list *l)
{
	memset(l, 0, sizeof(struct vertex_list));
}

static inline struct vertex_list *alloc_vertex_list(void)
{
	size_t size = sizeof(struct vertex_list);
	struct vertex_list *p = malloc(size);
	if(p) 
		init_vertex_list(p);
	
	return p;
}

static inline int add_vertex_to_list_tail(struct vertex_list *vl, vertex_t v) 
{
	struct vertex_list_node *new  = malloc(sizeof(struct vertex_list_node));
	if(!new) {
		err_printf("malloc()");
		return -1;
	}
	new->v = v;
	
	if(vl->nodes) {

		list_add_tail(&new->list, &vl->nodes->list);
	} else {
		vl->nodes = new;
		INIT_LIST_HEAD(&vl->nodes->list);
	}

	vl->length ++;
	return 0;
}
static inline int add_vertex_to_list_head(struct vertex_list *vl, vertex_t v) 
{
	struct vertex_list_node *new  = malloc(sizeof(struct vertex_list_node));
	if(!new) {
		err_printf("malloc()");
		return -1;
	}
	new->v = v;
	
	if(vl->nodes) {
		
		list_add(&(new->list), &(vl->nodes->list));
	} else {
		vl->nodes = new;
		INIT_LIST_HEAD(&new->list);
	}

	vl->length ++;
	return 0;
}
static inline vertex_t pop_vertex_from_list_head(struct vertex_list *v)
{
	struct vertex_list_node	*n = v->nodes;
	vertex_t vx = n->v;	
	
	v->length --;
	
	if(list_empty(&v->nodes->list)) {
		v->nodes = NULL;
	} else {
		v->nodes =  list_entry(v->nodes->list.next, struct vertex_list_node, list);	
		list_del(&n->list);		
	}

	free(n);
	return vx;
}
static inline vertex_t pop_vertex_from_list_tail(struct vertex_list *v)
{
	vertex_t vx;
	
	v->length --;
	
	if(list_empty(&v->nodes->list))   {
		vx = v->nodes->v;
		free(v->nodes);
		v->nodes = NULL;
		return vx;
	}
	
	struct vertex_list_node	*n = list_entry(v->nodes->list.prev, struct vertex_list_node, list);;	
	vx = n->v;
	list_del(&n->list);
	free(n);
	
	return vx;
}
static inline void free_vertex_list(struct vertex_list *gp)
{
	struct vertex_list_node *n;
	struct list_head *p, *q;
	if(gp->nodes) {
		list_for_each_safe(p, q, &gp->nodes->list) {	
			n = list_entry(p, struct vertex_list_node, list);
			list_del(p);	
			free(n);
		}
		free(gp->nodes);
	}
	free(gp);
}


#endif /* __VERTEX_LIST_H__ */
