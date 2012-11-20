#if 0
#!/bin/bash
gcc -o slice-test *.c -Wall -Wfatal-errors -O3
if [ "$?" = "0" ]; then
	time ./slice-test
fi
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #define NDEBUG
// #include <assert.h>

#include "slice.h"

SLICE_TYPE_DECLARE(int)

typedef char * string;

SLICE_TYPE_DECLARE(string);

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
    assert(a != d.ptr);
    assert(d.ptr[0] == 1);
    assert(d.ptr[1] == 2);
    assert(d.ptr[2] == 3);
    assert(d.ptr[3] == 4);
    free(d.ptr);
}

void test4(void)
{
    int_slice a = slice_make(int, 6);
    assert(a.ptr[0] == 0);
    assert(a.ptr[1] == 0);
    free(a.ptr);
}

void test5(void)
{
    int_slice a = slice_make(int, 6);
    int_slice b = slice(a, 0, 4);
    assert(b.len == 4);
    assert(b.cap == 6);
    free(a.ptr);
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
    free(a.ptr);
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
    }
    assert(a.len == 0);
    
    free(a.ptr);
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
    
    free(a.ptr);
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
	
	free(a.ptr);
	free(b.ptr);
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
	
	slice_append(a, b);
	
	assert(a.len == 6);
	assert(a.ptr[0] == 0);
	assert(a.ptr[1] == 1);
	assert(a.ptr[2] == 2);
	assert(a.ptr[3] == 0);
	assert(a.ptr[4] == 1);
	assert(a.ptr[5] == 2);
	assert(a.ptr[6] == 0);
	
	free(a.ptr);
	free(b.ptr);
}

int main(int argc, char *argv[])
{
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
    
    return 0;
}