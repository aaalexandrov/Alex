#include "vec.h"

#include <memory.h>
#include <malloc.h>

void arr_shuffle(void const *arr, size_t elem_size, size_t size)
{
	uint8_t *tmp = alloca(elem_size);
	for (size_t i = 1; i < size; ++i) {
		size_t ind = rand() % (size - i + 1) + i - 1;
		assert(i - 1 <= ind && ind < size);
		uint8_t *e = (uint8_t *)arr + (i - 1) * elem_size;
		uint8_t *s = (uint8_t *)arr + ind * elem_size;
		memcpy(tmp, e, elem_size);
		memcpy(e, s, elem_size);
		memcpy(s, tmp, elem_size);
	}
}

bool arr_is_sorted(void const *arr, size_t elem_size, size_t size, vec_void_cmp cmp)
{
	uint8_t *end = (uint8_t *)arr + size * elem_size;
	for (uint8_t *e = (uint8_t*)arr + elem_size; e < end; e += elem_size) {
		if (cmp(e - elem_size, e) > 0)
			return false;
	}
	return true;
}

void arr_insert_sort(void *arr, size_t elem_size, size_t size, vec_void_cmp cmp)
{
	uint8_t *tmp = alloca(elem_size);
	for (size_t i = 1; i < size; ++i) {
		uint8_t *e_i = (uint8_t *)arr + i * elem_size;
		memcpy(tmp, e_i, elem_size);
		for (size_t j = i; j > 0; --j) {
			uint8_t *e_j = (uint8_t *)arr + j * elem_size;
			int rel = cmp(e_j - elem_size, tmp);
			if (rel <= 0) {
				if (j != i)
					memcpy(e_j, tmp, elem_size);
				break;
			}
			memcpy(e_j, e_j - elem_size, elem_size);
		}
	}
}

void arr_qsort(void *arr, size_t elem_size, size_t size, vec_void_cmp cmp)
{
	if (size <= 16) {
		arr_insert_sort(arr, elem_size, size, cmp);
		return;
	}
	//if (size <= 1)
	//	return;
	uint8_t *tmp = alloca(elem_size * 2);
	uint8_t *pivot = tmp + elem_size;
	memcpy(pivot, (uint8_t *)arr + (size / 2) * elem_size, elem_size);
	uint8_t *l = (uint8_t*)arr - elem_size;
	uint8_t *r = (uint8_t*)arr + size * elem_size;
	while (true) {
		do {
			l += elem_size;
		} while (cmp(l, pivot) < 0);
		do {
			r -= elem_size;
		} while (cmp(pivot, r) < 0);
		if (l >= r)
			break;
		memcpy(tmp, l, elem_size);
		memcpy(l, r, elem_size);
		memcpy(r, tmp, elem_size);
	}
	// the following are necessary so the algorithm doesn't go into an infinite loop for arrays of size 2
	if (cmp(l, pivot) == 0)
		l += elem_size;
	if (cmp(pivot, r) == 0 && r > (uint8_t *)arr)
		r -= elem_size;
	arr_qsort(arr, elem_size, (r - (uint8_t *)arr) / elem_size + 1, cmp);
	arr_qsort(l, elem_size, size - (l - (uint8_t *)arr) / elem_size, cmp);
}

