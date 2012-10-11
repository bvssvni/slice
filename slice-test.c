
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #define NDEBUG
// #include <assert.h>

#include "slice.h"

SLICE_TYPE_DECLARE(int)

#define Err() do {printf("%i Error!\n", __LINE__); exit(1);} while (0)

void test1(void)
{
    int a[] = {1,2,3,4};
    int an = sizeof(a)/sizeof(int);
    slice_int b = slice_array(a, 0, an);
    slice_int c = slice(b, 1, 2);
    assert(c.ptr == a+1);
    assert(c.len == 1);
    assert(c.cap == 3);
}

void test2(void)
{
    int a[] = {1,2,3,4};
    slice_int b = slice_array(a, 0, 2);
    
    int size = slice_itemsize(b);
    assert(size == 4);
    
    slice_int c = slice_array(a, 2, 4);
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
    slice_int b = slice_array(a, 0, 2);
    slice_int c = slice_array(a, 2, 4);
    slice_int d = slice_merge(b, c);
    assert(a != d.ptr);
    assert(d.ptr[0] == 1);
    assert(d.ptr[1] == 2);
    assert(d.ptr[2] == 3);
    assert(d.ptr[3] == 4);
    free(d.ptr);
}

void test4(void)
{
    slice_int a = slice_make(int, 6);
    assert(a.ptr[0] == 0);
    assert(a.ptr[1] == 0);
    free(a.ptr);
}

void test5(void)
{
    slice_int a = slice_make(int, 6);
    slice_int b = slice(a, 0, 4);
    assert(b.len == 4);
    assert(b.cap == 6);
    free(a.ptr);
}

void test6(void)
{
    int n = 6;
    slice_int a = slice_make(int, n);
    int i;
    for (i = 0; i < n; i++) {
        slice_int b = slice(a, i, i+1);
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
    slice_int a = slice_make(int, n);
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
    slice_int a = slice_make(int, n);
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
    
    return 0;
}