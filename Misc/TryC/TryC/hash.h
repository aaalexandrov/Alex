#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <memory.h>
#include <assert.h>

uint32_t murmur3_32(const uint8_t *key, size_t len, uint32_t seed);

typedef uint32_t (*set_hash_func)(void const *elem, uint32_t elem_size);
typedef bool     (*set_eq_func)(void const *value, void const *elem, uint32_t elem_size);

inline uint32_t set_hash_default(void const *elem, uint32_t elem_size) { return murmur3_32(elem, elem_size, 0x12345678); }
inline bool set_eq_default(void const *value, void const *elem, uint32_t elem_size) { return !memcmp(value, elem, elem_size); }

typedef struct hash_set {
	set_hash_func  hash;
	set_eq_func    equal;
	uint32_t      *distances;
	void          *elems;
	uint32_t       elem_size;
	uint32_t       size;
	uint32_t       capacity;
} hash_set;

#define SET_INIT(TYPE, HASH, EQ) (hash_set){(HASH)?(HASH):set_hash_default, (EQ)?(EQ):set_eq_default, NULL, NULL, sizeof(TYPE)}
#define SET_FREE(set) set_set_capacity((set), 0)

void *set_find(hash_set *set, void const *value);
void *set_next(hash_set *set, void const *elem);

void *set_insert(hash_set *set, void const *value);
void set_remove(hash_set *set, void const *elem);
void set_clear(hash_set *set);
void set_set_capacity(hash_set *set, uint32_t capacity);

bool set_check_consistency(hash_set *set);

void set_test();

inline uint32_t inc_modulo(uint32_t modulo, uint32_t i, uint32_t delta)
{
	assert(i < modulo);
	assert(delta <= modulo);
	i += delta;
	if (i >= modulo)
		i -= modulo;
	assert(i < modulo);
	return i;
}

inline size_t round_up_sz(size_t n, size_t multiple_of)
{
	size_t r = n % multiple_of;
	return r ? n + multiple_of - r : n;
}