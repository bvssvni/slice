/*
 slice - Slices in C
 BSD license.
 by Sven Nilsen, 2012
 http://www.cutoutpro.com
 
 Version: 0.004 in angular degrees version notation
 http://isprogrammingeasy.blogspot.no/2012/08/angular-degrees-versioning-notation.html
 
 004	Added 'insert' and 'put' for faster insertion.
		Replaced memcpy with memmove.
 003	Added standard headers for easier compiling.
		Added 'check' macro that expands slice if items does not fit.
		Added 'is_allocated' flag and 'slice_free'.
 002	Changed type 'slice_<type>' to '<type>_slice' to avoid naming collision.
 001	Made slice mutable for easier programming.
 
 */
/*
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.
 */

/*
 
 slice - Slices in C.
 
 Slices makes it easier to work with arrays in C.
 The macroes are written in a way to behave like function calls.
 
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SLICE_GUARD
#define SLICE_GUARD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
	
    // Create a simple assert if not declared for boundary checking.
    // If you want to remove boundary checks, put following at top your code:
    // #define NDEBUG
    // #include <assert.h>
#ifndef assert(cond)
#define assert(cond) do {if (!(cond)) { \
printf("Assertion failed: (%s), function %s, file %s, line %i.\n", \
#cond, __FUNCTION__, __FILE__, __LINE__); exit(1);}} while (0)
#endif
    
			// Declare type.
#define		slice_type(type) \
\
typedef struct type##_slice {\
	int is_allocated;\
	type * ptr;\
	int len;\
	int cap;\
} type##_slice

			// Returns part of another slice.
#define		slice(a, start, end) ({\
\
__typeof__(a) *_a = &(a); int _start = (start); int _end = (end);\
(__typeof__(a)){.ptr = _a->ptr + _start, .len = _end - _start, \
.cap = _a->cap - _start};\
})

			// Returns a part of an array.
			// This range determine the capacity of the slice.
#define		slice_array(a, start, end) \
\
{.ptr = (a)+(start), .len = (end)-(start), .cap = (end)-(start)}
    
			// Declares a new array with a given capacity.
			// The length is set to 0.
#define		slice_make(type, capacity) \
\
(type##_slice){\
.is_allocated = 1,\
.ptr = memset(malloc(sizeof(type) * capacity), 0, sizeof(type) * capacity),\
.cap = capacity}
    
			// Push items at end of slice.
#define		slice_push(a, item) \
\
do {__typeof__(a) *_a = &(a);\
assert(_a->len < _a->cap); _a->ptr[_a->len++] = item;} while (0)
	
			// Inserts item in slice at index.
#define		slice_insert(a, index, item) \
\
do{__typeof__(a) *_a = &(a);\
int _index = (index);\
assert(_a->len < _a->cap);\
memmove(_a->ptr + (_index+1),_a->ptr + _index, (_a->len-_index)*slice_itemsize(*_a));\
_a->ptr[_index] = (item);\
_a->len++;\
} while (0)
	
			// Puts a slice into another slice at index, opposite of _cut_.
			// This pushes the other items toward end.
#define		slice_put(a, index, b) \
\
do{\
__typeof__(a) *_a = &(a);\
__typeof__(b) *_b = &(b);\
int _index = (index);\
assert(_a->len+_b->len <= _a->cap);\
memmove(_a->ptr + (_index+_b->len),_a->ptr + _index, \
(_a->len-_index)*slice_itemsize(*_a));\
memmove(_a->ptr + _index, _b->ptr, _b->len * slice_itemsize(*_b));\
_a->len += _b->len;\
} while (0)

			// Pops item from end of slice.
#define		slice_pop(a) \
\
({__typeof__(a) *_a = &(a);\
assert(_a->len > 0);\
_a->ptr[--_a->len]; \
})
	
			// Sets item at index, expands the length if necessary.
#define		slice_set(a, index, item) \
do {\
__typeof__(a) *_a = &(a);\
int _index = (index);\
__typeof__(item) _item = (item);\
assert(_index < _a->cap);\
_a->ptr[_index] = _item;\
_a->len = _a->len <= _index ? _index+1 : _a->len;\
} while (0)
	
			// Returns the item size in bytes of a slice.
#define		slice_itemsize(a) \
\
((unsigned long)((a).ptr + 1) - (unsigned long)((a).ptr))

			// Returns the number of items in _b_ that fits in _a_.
#define		slice_minlen(a, b) \
\
({__typeof__(a) *_a = &(a); __typeof__(b) *_b = &(b);\
(_a->cap < _b->len ? _a->cap : _b->len);})

			// Used internally.
#define		slice_mincap(a, b) \
\
((a).cap-(a).len < (b).len ? (a).cap-(a).len : (b).len)
	
			// Copies content from _b_ into _a_ and returns number of items copied.
			// The entries in _a_ in that range will be overwritten.
#define		slice_copy(a, b) \
\
(memmove((a).ptr, (b).ptr, slice_minlen(a,b) * slice_itemsize(b)) ? \
(a).len = ((a).len >= (b).len ? (a).len : \
(a).cap < (b).len ? (a).cap : (b).len) : 0)
    
			// Appends content from _b_ to _a_ and returns number of items appened.
#define		slice_append(a, b) \
\
({__typeof__(a) *_a = &(a); __typeof__(b) *_b = &(b);\
(memmove(_a->ptr+_a->len, _b->ptr, slice_mincap(*_a,*_b) * slice_itemsize(*_b)) ? \
slice_mincap(*_a,*_b)+((_a->len += slice_mincap(*_a,*_b))&&0) : 0);})
	
			// Expand the slice with block if items do not fit.
#define		slice_check(type, buffer, items, block) \
\
do { \
	__typeof__(buffer) *_buffer = &(buffer); \
	int _items = (items); \
	int _block = (block); \
	if (_buffer->len + _items <= _buffer->cap) break; \
	type##_slice _newbuffer = slice_make(type, _buffer->len + _block); \
	slice_append(_newbuffer, *_buffer); \
	slice_free(*_buffer); \
	*_buffer = _newbuffer; \
} while (0)
	
			// Cuts a range from slice.
			// This is the opposite from _put_.
#define slice_cut(a, start, end) \
\
do {int _start = (start);\
int _end = (end);\
__typeof__(a) *_a = &(a);\
assert(_end >= _start); memmove(_a->ptr + _start, _a->ptr + _end, \
slice_itemsize(a) * (_a->cap - _end)); _a->len -= _end-_start;} while (0)
    
			// Deletes item at index, decrementing the indices of the rest.
#define		slice_delete(a, index) \
\
do {__typeof__(a) *_a = &(a); int _index = (index);\
memmove(_a->ptr + _index, _a->ptr + _index + 1, \
slice_itemsize(*_a) * (_a->cap - _index - 1)); _a->len--;} while (0)

			// Clears all items in a slice.
#define		slice_clear(a) \
\
do {__typeof__(a) *_a = &(a);\
memset(_a->ptr, 0, slice_itemsize(*_a) * _a->len);} while (0)
    
			// Appends two slices that must be released when no longer needed.
#define		slice_merge(a, b) \
\
({__typeof__(a) *_a = &(a); __typeof__(b) *_b = &(b);\
(__typeof__(a)){.ptr = \
(__typeof__(_a->ptr))memmove(_a->len + \
(__typeof__(_a->ptr))memmove(malloc(slice_itemsize(*_a) * \
(_a->len + _b->len)), _a->ptr, _a->len * slice_itemsize(*_a)), \
_b->ptr, _b->len * slice_itemsize(*_b)) - _a->len, \
.len = _a->len + _b->len, .cap = _a->len + _b->len, .is_allocated = 1};})

			// Releases a slice if it is allocated.
#define		slice_free(a) \
\
do {__typeof__(a) *_a = &(a);\
if (_a->is_allocated && _a->ptr != NULL) {\
free(_a->ptr); *_a = (__typeof__(a)){};}} while (0)
	
			// Returns the sorted index by binary search mapping to a slice.
			// This is the common way of sorting data, which is faster than
			// moving the items around themselves.
			// Returns a positive index if the item is found.
			// Returns a negative index minus one of the smallest greater
			// index if the item does not exists.
			// The returned value can be used to insert new items.
			// Requires 'int_slice'.
			// The _f_ parameter is a function that compares two items.
			// The arguments to this function should be pointers.
			// int compare(<type> *a, <type> *b);
#define		slice_binarysearch_sortedindex(arr, sortedindices, f, item) \
	({\
	__typeof__(item) *_item = &(item);\
	__typeof__(arr) *_arr = &(arr);\
	int_slice *_sortedindices = &(sortedindices);\
	int _start = 0;\
	int _end = _arr->len;\
	int _mid;\
	while (_end - _start >= 1) {\
		_mid = _start + ((_end-_start)>>1);\
		int _index = _sortedindices->ptr[_mid];\
		int _compare = f(_item, &_arr->ptr[_index]);\
		if (_compare > 0) {\
			_start = _mid + 1;\
		} else if (_compare < 0) {\
			_end = _mid;\
		} else {\
			break;\
		}\
	}\
	if (_end-_start == 0) _mid = -(_end+1);\
	_mid;\
	})
	
#endif
	
#ifndef FUNC_GUARD
#define FUNC_GUARD
#define func(ns, n) static __typeof__(ns##_##n) * const n = ns##_##n
#endif

#ifdef __cplusplus
}
#endif
