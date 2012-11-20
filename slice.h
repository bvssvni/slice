/*
 slice - Slices in C
 BSD license.
 by Sven Nilsen, 2012
 http://www.cutoutpro.com
 
 Version: 0.002 in angular degrees version notation
 http://isprogrammingeasy.blogspot.no/2012/08/angular-degrees-versioning-notation.html
 
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
 
 Slices are a convenient way of representing a part of an array.
 They are so small in size that they can be passed by values to functions.
 The point with slices is to abstract a little from raw 
 
 It has 3 members:
    ptr - the pointer to the location in array.
    len - the number of items in the slice.
    cap - the capacity of the slice.
 
 Declare slices of type 'int' and 'double'.
 
    SLICE_TYPE_DECLARE(int)
    SLICE_TYPE_DECLARE(double)
 
 This implementation of slices does not use garbage collection.
 The best practice is to use a main slice that cover the whole array,
 and keep track of slice until there are no more slices referring to it.
 
    int_slice a = slice_make(int, 10);
    ...
    free(a.ptr);
 
 'make' initializes the length of the slice to 0.
 This way you can use slices as expandable arrays up to a limit.
 
 You can push and pop items onto a slice with boundary checks.
 
    slice_push(a, 2);
 
    item = slice_pop(a);
 
 Pop has a tail assertion that fails if used in expression.
 
    if (slice_pop(a) == 0) {  // WRONG
        ...
    }
 
    item = slice_pop(a);    // RIGHT
    if (item == 0) {
        ...
    }
 
 The only member that should be modified is the 'len' member.
 
 When appending two slices, you can either use 'copy' or 'merge'.
 
    int copy(a, b) - Puts items from _b_ into _a_ up to min(a.len, b.len).
    Returns number of copied elements. If _a_ contains more items than
	copied, the operation can be thought of as overwriting.
 
	int append(a, b) - Appends items from _b_ to _a_.
	Returns the number of appened items.
 
    c = merge(a, b) - Creates a new array filled with content of two slices.
    The type of slice returned is the same as the first parameter.
    The array is dynamically allocated and must be released with free(c.ptr).
 
 TYPICAL ERRORS
 
	warning: data definition has no type or storage class
 
		This will appear if you type SLICE_TYPE_DECLARE wrong.
 
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SLICE_GUARD
#define SLICE_GUARD

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
#define SLICE_TYPE_DECLARE(type) \
typedef struct type##_slice {type * ptr; int len; int cap;} type##_slice;

	// Returns part of another slice.
#define slice(a, start, end) ({\
__typeof__(a) *_a = &(a); int _start = (start); int _end = (end);\
(__typeof__(a)){.ptr = _a->ptr + _start, .len = _end - _start, \
.cap = _a->cap - _start};\
})

    // Returns a part of an array.
    // This range determine the capacity of the slice.
#define slice_array(a, start, end) \
{.ptr = (a)+(start), .len = (end)-(start), .cap = (end)-(start)}
    
    // Declares a new array with a given capacity.
    // The length is set to 0.
#define slice_make(type, capacity) \
(type##_slice){.ptr = memset(malloc(sizeof(type) * capacity), 0, sizeof(type) * capacity), \
.cap = capacity}
    
    // Push items at end of slice.
#define slice_push(a, item) do {__typeof__(a) *_a = &(a);\
assert(_a->len < _a->cap); _a->ptr[_a->len++] = item;} while (0)
    
    // Pops item from end of slice.
#define slice_pop(a) \
({__typeof__(a) *_a = &(a); assert(_a->len > 0);\
(--_a->len >= 0 ? _a->ptr[_a->len] : (typeof(*_a->ptr)){0}); \
})
	
    // Returns the item size in bytes of a slice.
#define slice_itemsize(a) \
((unsigned long)((a).ptr + 1) - (unsigned long)((a).ptr))

    // Returns the number of items in _b_ that fits in _a_.
#define slice_minlen(a, b) \
({__typeof__(a) *_a = &(a); __typeof__(b) *_b = &(b);\
(_a->cap < _b->len ? _a->cap : _b->len);})

#define slice_mincap(a, b) \
((a).cap-(a).len < (b).len ? (a).cap-(a).len : (b).len)
	
    // Copies content from _b_ into _a_ and returns number of items copied.
	// The entries in _a_ in that range will be overwritten.
#define slice_copy(a, b) \
(memcpy((a).ptr, (b).ptr, slice_minlen(a,b) * slice_itemsize(b)) ? \
(a).len = ((a).len >= (b).len ? (a).len : \
(a).cap < (b).len ? (a).cap : (b).len) : 0)
    
	// Appends content from _b_ to _a_ and returns number of items appened.
#define slice_append(a, b) \
({__typeof__(a) *_a = &(a); __typeof__(b) *_b = &(b);\
(memcpy(_a->ptr+_a->len, _b->ptr, slice_mincap(*_a,*_b) * slice_itemsize(*_b)) ? \
slice_mincap(*_a,*_b)+((_a->len += slice_mincap(*_a,*_b))&&0) : 0);})
	
    // Cuts a range from slice.
#define slice_cut(a, start, end) \
do {int _start = (start);\
int _end = (end);\
__typeof__(a) *_a = &(a);\
assert(_end >= _start); memcpy(_a->ptr + _start, _a->ptr + _end, \
slice_itemsize(a) * (_a->cap - _end)); _a->len -= _end-_start;} while (0)
    
    // Deletes item at index, decrementing the indices of the rest.
#define slice_delete(a, index) \
do {__typeof__(a) *_a = &(a); int _index = (index);\
memcpy(_a->ptr + _index, _a->ptr + _index + 1, \
slice_itemsize(*_a) * (_a->cap - _index - 1)); _a->len--;} while (0)

    // Clears all items in a slice.
#define slice_clear(a) \
do {__typeof__(a) *_a = &(a);\
memset(_a->ptr, 0, slice_itemsize(*_a) * _a->len);} while (0)
    
    // Appends two slices that must be released when no longer needed.
#define slice_merge(a, b) \
({__typeof__(a) *_a = &(a); __typeof__(b) *_b = &(b);\
(typeof(a)){.ptr = \
(typeof(_a->ptr))memcpy(_a->len + \
(typeof(_a->ptr))memcpy(malloc(slice_itemsize(*_a) * \
(_a->len + _b->len)), _a->ptr, _a->len * slice_itemsize(*_a)), \
_b->ptr, _b->len * slice_itemsize(*_b)) - _a->len, \
.len = _a->len + _b->len, .cap = _a->len + _b->len};})
    
#endif

#ifdef __cplusplus
}
#endif
