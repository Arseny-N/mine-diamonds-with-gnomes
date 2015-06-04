#ifndef __TYPES_H__
#define __TYPES_H__


#include <stdint.h>

typedef  uint8_t  u8;
typedef  uint16_t u16;
typedef  uint32_t u32;
typedef  uint64_t u64;

typedef  int8_t  s8;
typedef  int16_t s16;
typedef  int32_t s32;
typedef  int64_t s64;

struct list_head {
	struct list_head *next, *prev;
};

struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

#include <limits.h>
#include <stddef.h>
typedef u64 cost_t;
//#define INFINITY UINT_MAX
#define INFINITY 0xff
#define COST_FMT "%ld"

typedef u64 vertex_t;
//#define UNIT UINT_MAX
#define UNIT 0xff
#define VERTEX_FMT "%ld"


#endif /* __TYPES_H__ */
