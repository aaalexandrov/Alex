#include "hash.h"

#include <stdlib.h>
#include <malloc.h>

static inline uint32_t murmur_32_scramble(uint32_t k) 
{
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

uint32_t murmur3_32(const uint8_t *key, size_t len, uint32_t seed)
{
    uint32_t h = seed;
    uint32_t k;
    /* Read in groups of 4. */
    for (size_t i = len >> 2; i; i--) {
        // Here is a source of differing results across endiannesses.
        // A swap here has no effects on hash properties though.
        memcpy(&k, key, sizeof(uint32_t));
        key += sizeof(uint32_t);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    /* Read the rest. */
    k = 0;
    for (size_t i = len & 3; i; i--) {
        k <<= 8;
        k |= key[i - 1];
    }
    // A swap is *not* necessary here because the preceding loop already
    // places the low bytes in the low places according to whatever endianness
    // we use. Swaps only apply when the memory is copied in a chunk.
    h ^= murmur_32_scramble(k);
    /* Finalize. */
    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

#define SET_DEBUG

static const uint32_t INVALID_DISTANCE = -1;
static const uint32_t LOAD_FACTOR_PERCENT = 75;

static inline uint8_t *set_elem(hash_set *set, uint32_t i)
{
    assert(i < set->capacity);
    return (uint8_t*)set->elems + i * (size_t)set->elem_size;
}

static inline uint32_t set_index(hash_set *set, void const *elem)
{
    uint32_t index = elem ? (uint32_t)((uint8_t *)elem - (uint8_t *)set->elems) / set->elem_size : -1;
    assert(index == -1 || index < set->capacity && set_elem(set, index) == elem);
    return index;
}

void *set_find(hash_set *set, void const *value)
{
    if (!set->capacity)
        return NULL;

    uint32_t h = set->hash(value, set->elem_size);
    uint32_t ind = h % set->capacity;
    // no elements rooted at this slot index in the table
    if (set->distances[ind] != 0)
        return NULL;

    for (uint32_t i = 0; i < set->capacity && set->distances[ind] != INVALID_DISTANCE; ++i) {
        if (set->distances[ind] == i) {
            // only consider elements that are rooted at the value's slot
            uint8_t *e = set_elem(set, ind);
            if (set->equal(value, e, set->elem_size))
                return e;
        }
        ind = inc_modulo(set->capacity, ind, 1);
    }

    return NULL;
}

void *set_next(hash_set *set, void const *elem)
{
    uint32_t index = set_index(set, elem);

    do {
        if (++index >= set->capacity)
            return NULL;
    } while (set->distances[index] == INVALID_DISTANCE);

    return set_elem(set, index);
}

void *set_insert(hash_set *set, void const *value)
{
    if ((uint64_t)set->capacity * LOAD_FACTOR_PERCENT <= (uint64_t)set->size * 100)
        set_set_capacity(set, max(set->capacity * 2, 8));

    assert(set->size < set->capacity);

    uint32_t h = set->hash(value, set->elem_size);
    uint32_t ind = h % set->capacity;
    uint32_t dist = 0;

    uint8_t *v = alloca(set->elem_size);
    if (set->distances[ind] != INVALID_DISTANCE && set->distances[ind] != 0) {
        // we're the first element with that hash, but the slot's taken
        // displace element with different hash from the slot, then continue inserting the displaced element

        uint8_t *e = set_elem(set, ind);
        memcpy(v, e, set->elem_size);
        dist = set->distances[ind];

        memcpy(e, value, set->elem_size);
        set->distances[ind] = 0;
        value = v;
    }

    while (set->distances[ind] != INVALID_DISTANCE) {
        // should we displace elements along the way if they have lesser distance than the current distance?
        // this could minimize the maximum distance in the table?
        ++dist;
        ind = inc_modulo(set->capacity, ind, 1);
    }

    assert(dist < set->capacity);

    uint8_t *e = set_elem(set, ind);
    memcpy(e, value, set->elem_size);
    set->distances[ind] = dist;
    ++set->size;

#if defined(SET_DEBUG)
    assert(set_check_consistency(set));
#endif

    return e;
}

void set_remove(hash_set *set, void const *elem)
{
    uint32_t index = set_index(set, elem);
    assert(set->distances[index] != INVALID_DISTANCE);

    if (set->distances[index] == 0) {
        // look for the last element rooted at the same slot & move it over the removed element
        // that makes sure the slot will contain an element that hashes to that slot
        uint32_t found_dist = INVALID_DISTANCE;
        for (uint32_t dist = 1; dist < set->capacity; ++dist) {
            uint32_t dist_ind = inc_modulo(set->capacity, index, dist);
            if (set->distances[dist_ind] == INVALID_DISTANCE)
                break;
            if (set->distances[dist_ind] == dist)
                found_dist = dist;
        }
        if (found_dist != INVALID_DISTANCE) {
            uint32_t found_ind = inc_modulo(set->capacity, index, found_dist);
            memcpy(set_elem(set, index), set_elem(set, found_ind), set->elem_size);
            index = found_ind;
        }
    }
    
    while (true) {
        // look for the last element that has a hash slot before the element being removed & move it over the removed element
        uint32_t found_dist = INVALID_DISTANCE;
        for (uint32_t dist = 1; dist < set->capacity; ++dist) {
            uint32_t dist_ind = inc_modulo(set->capacity, index, dist);
            if (set->distances[dist_ind] == INVALID_DISTANCE)
                break;
            assert(set->distances[dist_ind] != dist);
            if (set->distances[dist_ind] > dist)
                found_dist = dist;
        }
        if (found_dist == INVALID_DISTANCE)
            break;
        uint32_t found_ind = inc_modulo(set->capacity, index, found_dist);
        memcpy(set_elem(set, index), set_elem(set, found_ind), set->elem_size);
        set->distances[index] = set->distances[found_ind] - found_dist;
        index = found_ind;
    }

    // invoke deleter for element?
    set->distances[index] = INVALID_DISTANCE;
    --set->size;

#if defined(SET_DEBUG)
    assert(set_check_consistency(set));
#endif
}

void set_clear(hash_set *set)
{
    set->size = 0;
    for (uint32_t i = 0; i < set->capacity; ++i)
        set->distances[i] = INVALID_DISTANCE;
}

void set_set_capacity(hash_set *set, uint32_t capacity)
{
    if (capacity == set->capacity)
        return;

    if (capacity == 0) {
        free(set->elems);
        set->elems = NULL;
        set->distances = NULL;
        set->size = 0;
        set->capacity = 0;
        return;
    }

    assert(capacity >= set->size || capacity == 0);

    uint32_t prev_capacity = set->capacity;
    uint8_t *prev_elems = set->elems;
    uint32_t *prev_distances = set->distances;

    size_t elemsSize = round_up_sz(capacity * (size_t)set->elem_size, sizeof(uint32_t));
    set->elems = malloc(elemsSize + capacity * sizeof(uint32_t));
    set->distances = (uint32_t*)((uint8_t *)set->elems + elemsSize);
    set->capacity = capacity;

    set_clear(set);

    for (uint32_t i = 0; i < prev_capacity; ++i) {
        if (prev_distances[i] == INVALID_DISTANCE)
            continue;
        set_insert(set, prev_elems + i * (size_t)set->elem_size);
    }

    free(prev_elems);

#if defined(SET_DEBUG)
    assert(set_check_consistency(set));
#endif
}

bool set_check_consistency(hash_set *set)
{
    if (!set)
        return false;
    if (!set->hash || !set->equal)
        return false;
    if (!set->elem_size)
        return false;
    if (set->capacity < set->size)
        return false;
    if (set->capacity == 0 != !set->elems)
        return false;
    if (set->capacity == 0 != !set->distances)
        return false;
    if (set->elems && (uint8_t *)set->distances - (uint8_t *)set->elems != round_up_sz(set->capacity * (size_t)set->elem_size, sizeof(uint32_t)))
        return false;
    uint32_t elems = 0;
    for (uint32_t i = 0; i < set->capacity; ++i) {
        if (set->distances[i] == INVALID_DISTANCE)
            continue;
        ++elems;
        if (set->distances[i] >= set->capacity)
            return false;
        uint32_t h = set->hash(set_elem(set, i), set->elem_size);
        uint32_t slot = h % set->capacity;
        uint32_t distance_ind = inc_modulo(set->capacity, i, set->capacity - set->distances[i]);
        if (slot != distance_ind)
            return false;
        if (set->distances[slot] == INVALID_DISTANCE)
            return false;
        uint32_t slot_h = set->hash(set_elem(set, slot), set->elem_size);
        uint32_t slot_ind = slot_h % set->capacity;
        if (slot_ind != slot)
            return false;
        for (uint32_t j = slot; j != i; j = inc_modulo(set->capacity, j, 1)) {
            if (set->distances[j] == INVALID_DISTANCE)
                return false;
        }
    }
    if (elems != set->size)
        return false;
    return true;
}

#include "vec.h"

typedef struct elem_type {
    int key;
    int value;
} elem_type;

uint32_t hash_elem(elem_type const *elem, uint32_t elem_size)
{
    return murmur3_32((uint8_t const*)&elem->key, sizeof(elem->key), 0x12345678);
}

bool eq_elem(elem_type const *value, elem_type const *elem, uint32_t elem_size)
{
    return value->key == elem->key;
}

void set_test()
{
    VEC_TYPE(elem_type, vec_elem_type);
    vec_elem_type test_elems = { 0 };

    srand(0xdeadbeef);

    int max_test_elem = -1;

    for (int i = 0; i < 20000; ++i) {
        elem_type e = { rand(), rand() };
        max_test_elem = max(max_test_elem, e.key);
        if (VEC_FIND(&test_elems, &e, eq_elem) == -1)
            VEC_PUSH_BACK(&test_elems, &e);
    }

    hash_set hs = SET_INIT(elem_type, hash_elem, eq_elem);
    assert(set_check_consistency(&hs));

    for (uint32_t i = 0; i < test_elems.size; ++i) {
        set_insert(&hs, test_elems.arr + i);
    }

    assert(hs.size == test_elems.size);

    for (elem_type *e = NULL; e = set_next(&hs, e); ) {
        uint32_t found_ind = VEC_FIND(&test_elems, e, eq_elem);
        assert(found_ind != -1);
    }

    for (uint32_t i = 0; i < test_elems.size; ++i) {
        elem_type *e = set_find(&hs, test_elems.arr + i);
        assert(e != NULL && e->key == test_elems.arr[i].key && e->value == test_elems.arr[i].value);
    }

    elem_type inexistent = { max_test_elem + 10 };
    elem_type *not_found = set_find(&hs, &inexistent);
    assert(not_found == NULL);

    VEC_SHUFFLE(&test_elems);

    for (uint32_t i = 0; i < test_elems.size; ++i) {
        elem_type *e = set_find(&hs, test_elems.arr + i);
        set_remove(&hs, e);
    }

    assert(hs.size == 0);

    SET_FREE(&hs);

    VEC_FREE(&test_elems);
}