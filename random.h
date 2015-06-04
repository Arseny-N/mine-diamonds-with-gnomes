#ifndef __RANDOM_H__
#define __RANDOM_H__

#include "types.h"
struct uniform_state
{
	u64 u, v , w;	
};
void init_uniform_state(struct uniform_state *s, u64 seed);



u64 sample_uniform_u64(struct uniform_state *s);
u64 sample_uniform_u64_bounded(struct uniform_state *s, u64 hb, u64 lb );
double sample_uniform_f64(struct uniform_state *s);



struct cat_state
{
	struct uniform_state uniform;
};
void init_cat_state(struct cat_state *s, u64 seed);

struct cat_parameters 
{	
	u16 count;
	union {
		double *cdf;
		u64 *d_cdf;
	};
};
int init_cat_parameters(struct cat_parameters *p, double *weights, u16 count);
void finit_cat_parameters(struct cat_parameters *p);

int init_dcat_parameters(struct cat_parameters *p, double *weights, u16 count); // TODO ?


u16 sample_cat(struct cat_state *s, struct cat_parameters *p);
u16 sample_dcat(struct cat_state *s, struct cat_parameters *p);




struct bern_state
{
	struct uniform_state uniform;
};
void init_bern_state(struct bern_state *s, u64 seed);

u8 sample_bern(struct bern_state *s, double param);


#endif /* __RANDOM_H__ */
