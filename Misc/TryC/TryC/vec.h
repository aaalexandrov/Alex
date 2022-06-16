#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

inline uint32_t round_up_to_pow2_u32(uint32_t x)
{
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	++x;
	return x;
}

#define VEC_TYPE(elem_type, vec_name) \
	typedef struct vec_name { \
		uint32_t size; \
		uint32_t capacity; \
		elem_type *arr; \
	} vec_name

VEC_TYPE(void, vec_void);

typedef bool (*vec_void_eq)(void const *v, void const *e);
typedef int (*vec_void_cmp)(void const *v, void const *e);

#define VEC_INIT(v, cap) vec_init((vec_void*)(v), cap, (uint32_t)sizeof(*(v)->arr))
#define VEC_FREE(v) vec_free((vec_void*)(v), (uint32_t)sizeof(*(v)->arr))
#define VEC_SET_CAPACITY(v, cap) vec_set_capacity((vec_void*)(v), cap, (uint32_t)sizeof(*(v)->arr))
#define VEC_RESIZE(v, size) vec_resize((vec_void*)(v), size, (uint32_t)sizeof(*(v)->arr))
#define VEC_PUSH_BACK(v, pelem) do { VEC_RESIZE((v), (v)->size+1); (v)->arr[(v)->size-1] = *pelem; } while(false)
#define VEC_FIND(v, pelem, eq) (uint32_t)arr_find((v)->arr, sizeof(*(v)->arr), (v)->size, pelem, (vec_void_eq)eq)
#define VEC_SHUFFLE(v) arr_shuffle((v)->arr, sizeof(*(v)->arr), (v)->size)

inline void vec_set_capacity(vec_void* v, uint32_t capacity, uint32_t elem_size)
{
	uint32_t memsize = capacity * elem_size;
	if (memsize > 0) {
		v->arr = realloc(v->arr, memsize);
	} else {
		free(v->arr);
		v->arr = NULL;
	}
	assert((v->arr == NULL) == (capacity == 0));
	v->capacity = capacity;
	v->size = min(v->size, capacity);
}

inline void vec_resize(vec_void* v, uint32_t size, uint32_t elem_size)
{
	if (v->capacity < size) {
		uint32_t cap = max(v->capacity, 1);
		uint32_t factor = (size + cap - 1) / cap;
		factor = round_up_to_pow2_u32(factor);
		vec_set_capacity(v, cap * factor, elem_size);
		assert(v->capacity >= size);
	}
	v->size = size;
}

inline void vec_init(vec_void* v, uint32_t capacity, uint32_t elem_size)
{
	*v = (const vec_void){ 0 };
	vec_set_capacity(v, capacity, elem_size);
}

inline void vec_free(vec_void* v, uint32_t elem_size)
{
	vec_set_capacity(v, 0, elem_size);
	assert(v->arr == NULL);
}

inline size_t arr_find(void const *arr, size_t elem_size, size_t size, void const *value, vec_void_eq eq)
{
	size_t found_ind = -1;
	for (size_t i = 0; i < size; ++i) {
		if (eq(value, (uint8_t *)arr + i * elem_size)) {
			found_ind = i;
			break;
		}
	}
	return found_ind;
}


void arr_shuffle(void *arr, size_t elem_size, size_t size);
bool arr_is_sorted(void const *arr, size_t elem_size, size_t size, vec_void_cmp cmp);
void arr_insert_sort(void *arr, size_t elem_size, size_t size, vec_void_cmp cmp);
void arr_qsort(void *arr, size_t elem_size, size_t size, vec_void_cmp cmp);
