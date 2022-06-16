#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "vec.h"
#include "hash.h"

VEC_TYPE(int, vec_int);

void print_vec(vec_int *vec)
{
	printf("vec capacity: %d", vec->capacity);
	for (uint32_t i = 0; i < vec->size; ++i) {
		printf(", vec[%d] = %d", i, vec->arr[i]);
	}
	printf("\n");
}

int cmp_int(int const *a, int const *b)
{
	return *a - *b;
}

int main(const int argc, char const *argv[])
{
	printf("Hello, hi, ho!\n");
	printf("Executable: %s\n", argv[0]);

	vec_int v = { 0 };
	VEC_RESIZE(&v, 5);
	for (uint32_t i = 0; i < v.size; ++i) {
		v.arr[i] = rand();
	}

	print_vec(&v);

	VEC_RESIZE(&v, 9);
	print_vec(&v);

	VEC_SET_CAPACITY(&v, 3);
	print_vec(&v);

	VEC_FREE(&v);
	assert(v.arr == NULL);


	clock_t start = clock();
	for (int j = 0; j < 1000; ++j) {
		VEC_RESIZE(&v, 0);
		for (int i = 0; i < 10000; ++i) {
			VEC_PUSH_BACK(&v, &i);
		}
		VEC_SHUFFLE(&v);
		//arr_insert_sort(v.arr, sizeof(*v.arr), v.size, cmp_int);
		arr_qsort(v.arr, sizeof(*v.arr), v.size, cmp_int);
		//qsort(v.arr, v.size, sizeof(*v.arr), cmp_int);

		assert(arr_is_sorted(v.arr, sizeof(*v.arr), v.size, cmp_int));
	}
	clock_t end = clock();
	double dur = (double)(end - start) / CLOCKS_PER_SEC;
	printf("Sorting duration: %.03g", dur);

	set_test();

	return EXIT_SUCCESS;
}