#if 0
#!/bin/bash
clear
gcc -o test-slice test-slice.c -Wall -Wfatal-errors -O3
if [ "$?" = "0" ]; then
	time ./test-slice
fi
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #define NDEBUG
// #include <assert.h>

#include "slice.h"

slice_type(int);
slice_type(double);

typedef char * string;

slice_type(string);

#define Err() do {printf("%i Error!\n", __LINE__); exit(1);} while (0)

void test1(void)
{
    int a[] = {1,2,3,4};
    int an = sizeof(a)/sizeof(int);
    int_slice b = slice_array(a, 0, an);
    int_slice c = slice(b, 1, 2);
    assert(c.ptr == a+1);
    assert(c.len == 1);
    assert(c.cap == 3);
}

void test2(void)
{
    int a[] = {1,2,3,4};
    int_slice b = slice_array(a, 0, 2);
    
    int size = slice_itemsize(b);
    assert(size == 4);
    
    int_slice c = slice_array(a, 2, 4);
    assert(c.ptr[0] == 3);
    assert(c.ptr[1] == 4);
    int cut = slice_minlen(b, c);
    assert(cut == 2);
    
    int copied = slice_copy(b, c);
    assert(copied == 2);
    assert(a[0] == 3);
    assert(a[1] == 4);
    assert(a[2] == 3);
    assert(a[3] == 4);
}

void test3(void)
{
    int a[] = {1,2,3,4};
    int_slice b = slice_array(a, 0, 2);
    int_slice c = slice_array(a, 2, 4);
    int_slice d = slice_merge(b, c);

	assert(d.is_allocated);
	
    assert(a != d.ptr);
    assert(d.ptr[0] == 1);
    assert(d.ptr[1] == 2);
    assert(d.ptr[2] == 3);
    assert(d.ptr[3] == 4);

    slice_free(d);
}

void test4(void)
{
    int_slice a = slice_make(int, 6);
    assert(a.ptr[0] == 0);
    assert(a.ptr[1] == 0);
    slice_free(a);
}

void test5(void)
{
    int_slice a = slice_make(int, 6);
    int_slice b = slice(a, 0, 4);
    assert(b.len == 4);
    assert(b.cap == 6);
	
	slice_free(b);
    slice_free(a);
}

void test6(void)
{
    int n = 6;
    int_slice a = slice_make(int, n);
    int i;
    for (i = 0; i < n; i++) {
        int_slice b = slice(a, i, i+1);
        b.ptr[0] = i;
    }
    assert(a.ptr[0] == 0);
    assert(a.ptr[1] == 1);
    assert(a.ptr[2] == 2);
    assert(a.ptr[3] == 3);
    assert(a.ptr[4] == 4);
    assert(a.ptr[5] == 5);
    slice_free(a);
}

void test7(void)
{
    int n = 6;
    int_slice a = slice_make(int, n);
    int i;
    for (i = 0; i < n; i++) {
        slice_push(a, i);
    }
    assert(a.len == 6);
    assert(a.ptr[0] == 0);
    assert(a.ptr[1] == 1);
    assert(a.ptr[2] == 2);
    assert(a.ptr[3] == 3);
    assert(a.ptr[4] == 4);
    assert(a.ptr[5] == 5);
    
    int c;
    for (i = 0; i < n; i++) {
        c = slice_pop(a);
		assert(c == n-i-1);
    }
    assert(a.len == 0);
    
    slice_free(a);
}

void test8(void)
{
    int n = 6;
    int_slice a = slice_make(int, n);
    int i;
    for (i = 0; i < n; i++) {
        slice_push(a, i);
    }
    
    slice_cut(a, 0, 3);
    assert(a.ptr[0] == 3);
    assert(a.ptr[1] == 4);
    
    slice_delete(a, 0);
    assert(a.ptr[0] == 4);
    
    slice_clear(a);
    assert(a.ptr[0] == 0);
    
    slice_free(a);
}

void test_string_array_9(void)
{
	string arr[] = {"Hello", "this", "is", "an", "array", "of", "strings"};
	string_slice a = slice_array(arr, 0, sizeof(arr)/sizeof(string));
	
	assert(a.cap == sizeof(arr)/sizeof(string));
	assert(a.len == sizeof(arr)/sizeof(string));
	assert(strcmp(a.ptr[0], "Hello") == 0);
	assert(strcmp(a.ptr[a.len-1], "strings") == 0);
}

void test_copy_10(void)
{
	int_slice a = slice_make(int, 10);
	int_slice b = slice_make(int, 3);
	int i;
	for (i = 0; i < 3; i++) slice_push(b, i);
	
	assert(b.ptr[0] == 0);
	assert(b.ptr[1] == 1);
	assert(b.ptr[2] == 2);
	
	assert(slice_copy(a, b) == 3);
	
	assert(a.ptr[0] == 0);
	assert(a.ptr[1] == 1);
	assert(a.ptr[2] == 2);
	assert(a.ptr[3] == 0);
	
	assert(a.len == 3);
	
	slice_free(a);
	slice_free(b);
}

void test_append_11(void)
{
	int_slice a = slice_make(int, 10);
	int_slice b = slice_make(int, 3);
	int i;
	for (i = 0; i < 3; i++) {
		slice_push(a, i);
		slice_push(b, i);
	}
	
	int appened = slice_append(a, b);
	// printf("%i\n", appened);
	assert(appened == b.len);
	
	assert(a.len == 6);
	assert(a.ptr[0] == 0);
	assert(a.ptr[1] == 1);
	assert(a.ptr[2] == 2);
	assert(a.ptr[3] == 0);
	assert(a.ptr[4] == 1);
	assert(a.ptr[5] == 2);
	assert(a.ptr[6] == 0);
	
	slice_free(a);
	slice_free(b);
}

void test_insert_1(void)
{
	int_slice a = slice_make(int, 10);
	slice_insert(a, 0, 2);
	assert(a.len == 1);
	assert(a.ptr[0] == 2);
	slice_insert(a, 0, 1);
	assert(a.len == 2);
	assert(a.ptr[0] == 1);
	assert(a.ptr[1] == 2);
	slice_insert(a, 0, 0);
	assert(a.ptr[0] == 0);
	assert(a.ptr[1] == 1);
	assert(a.ptr[2] == 2);
	slice_free(a);
}

void test_put_1(void)
{
	int_slice a = slice_make(int, 10);
	slice_push(a, 1);
	slice_push(a, 2);
	slice_push(a, 3);
	int_slice b = slice_make(int, 2);
	slice_push(b, 7);
	slice_push(b, 8);
	slice_put(a, 1, b);
	assert(a.ptr[0] == 1);
	assert(a.ptr[1] == 7);
	assert(a.ptr[2] == 8);
	assert(a.ptr[3] == 2);
	assert(a.ptr[4] == 3);
	slice_cut(a, 1, 1+2);
	assert(a.ptr[0] == 1);
	assert(a.ptr[1] == 2);
	slice_put(a, 0, b);
	assert(a.ptr[0] == 7);
	assert(a.ptr[1] == 8);
	assert(a.ptr[2] == 1);
	assert(a.ptr[3] == 2);
	slice_free(b);
	slice_free(a);
}

typedef struct
{
	double_slice items;
} dummy;

void test_pop_2(void)
{
	dummy d = {.items = slice_make(double, 10)};
	slice_push(d.items, 5);
	double val = slice_pop((&d)->items);
	assert(val == 5);
	slice_free(d.items);
}

int double_Compare(const double *a, const double *b)
{
	double aVal = *a;
	double bVal = *b;
	
	if (aVal < bVal) return -1;
	if (aVal > bVal) return 1;
	
	return 0;
}

int double_slice_SortedIndex
(double_slice arr, int_slice sortedindices, double d)
{
	return slice_binarysearch_sortedindex
	(arr, sortedindices, double_Compare, d);
}

void test_search_1()
{
	double_slice a = slice_make(double, 10);
	slice_push(a, 1.0);
	slice_push(a, 2.0);
	slice_push(a, 3.0);
	int_slice b = slice_make(int, 3);
	slice_push(b, 0);
	slice_push(b, 1);
	slice_push(b, 2);
	double val = 1.0;
	assert(double_slice_SortedIndex(a, b, val) == 0);
	val = 2.0;
	assert(double_slice_SortedIndex(a, b, val) == 1);
	val = 3.0;
	assert(double_slice_SortedIndex(a, b, val) == 2);
	val = 0.0;
	assert(double_slice_SortedIndex(a, b, val) == -1);
	val = 4.0;
	assert(double_slice_SortedIndex(a, b, val) == -4);
	slice_free(b);
	slice_free(a);
}

int main(int argc, char *argv[])
{
	int i;
	int end = 1 << 0; // 23;
	for (i = 0; i < end; i++) {
		test1();
		test2();
		test3();
		test4();
		test5();
		test6();
		test7();
		test8();
		test_string_array_9();
		test_copy_10();
		test_append_11();
		test_insert_1();
		test_put_1();
		test_search_1();
		test_pop_2();
	}
    
    return 0;
}