#include "random.h"

#include "utils.h"


#include <stdlib.h> // malloc()
u64 sample_uniform_u64(struct uniform_state *s)
{
	
	s->u = s->u * 2862933555777941757LL + 7046029254386353087LL;
	
	s->v ^= s->v >> 17; 
	s->v ^= s->v << 31; 
	s->v ^= s->v >> 8;
	
	s->w = 4294957665U*(s->w & 0xffffffff) + (s->w >> 32);
	
	u64 x = s->u ^ (s->u << 21); 
	x ^= x >> 35; 
	x ^= x << 4;
	
	return (x + s->v) ^ s->w;
}
double sample_uniform_f64(struct uniform_state *s)
{
	return 5.42101086242752217E-20 * sample_uniform_u64(s);
}
u64 sample_uniform_u64_bounded(struct uniform_state *s, u64 hb, u64 lb)
{
	return lb + sample_uniform_u64(s) % (hb - lb + 1);
}
void init_uniform_state(struct uniform_state *s, u64 seed)
{
	
	
	s->v = 4101842887655102017LL;
	s->w = 1;
	
	if(seed == s->v) 
		seed += (u64) ((u8*)s + (u8) &seed) ;
	
	s->u = seed ^ s->v; 
	sample_uniform_u64(s);
	
	s->v = s->u;
	sample_uniform_u64(s);
	
	s->w = s->v; 
	sample_uniform_u64(s);
}

void init_cat_state(struct cat_state *s, u64 seed)
{
	init_uniform_state(&s->uniform, seed);
}

static void __compute_cat_cdf(struct cat_parameters *cat, double *weights, u16 count)
{
	u16 i;
	
	cat->cdf[0] = weights[0];
	
	for(i = 1; i < count; ++i) 
		cat->cdf[i] = cat->cdf[i-1] + weights[i];
	dbg_printf(" %lf", cat->cdf[count-1]);
	cat->cdf[count-1] = 1;
		
}
int init_cat_parameters(struct cat_parameters *p, double *weights, u16 count)
{
	p->count = count;
	
	p->cdf = malloc(sizeof(double) * count);
	if(!p->cdf) {
		err_printf("malloc()");
		return -1;
	}
	__compute_cat_cdf(p, weights, count);
	return 0;		
}
void finit_cat_parameters(struct cat_parameters *p)
{
	if(p->cdf)
		free(p->cdf);
}

u16 sample_cat(struct cat_state *s, struct cat_parameters *p)
{
	double u = sample_uniform_f64(&s->uniform);
	u16 i;

	for(i = 0; i < p->count; ++i) 
		if(u < p->cdf[i]) 
			return i;
	err_printf("ERROR %d %lf", i, u );
	return i;
}
u16 sample_dcat(struct cat_state *s, struct cat_parameters *p)
{
	u64 i, u = 1 + sample_uniform_u64(&s->uniform) % (p->d_cdf[p->count-1] - 1);

	for(i = 0; i < p->count; ++i)  
		if(u >= p->d_cdf[i]) 

			return i;	
		
	return i-1;
}

void init_bern_state(struct bern_state *s, u64 seed)
{
	init_uniform_state(&s->uniform, seed);
}
u8 sample_bern(struct bern_state *s, double param)
{
	double u = sample_uniform_f64(&s->uniform);
	return param > u ? 1 : 0;
}

