/*
 slice - Slices in C
 BSD license.
 by Sven Nilsen, 2012
 http://www.cutoutpro.com
 
 Version: 0.001 in angular degrees version notation
 http://isprogrammingeasy.blogspot.no/2012/08/angular-degrees-versioning-notation.html
 
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
 
    slice_int a = slice_make(int, 10);
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
 
    int copy(a, b) - Puts items from b into a up to min(a.len, b.len).
    Returns number of copied elements.
 
    c = merge(a, b) - Creates a new array filled with content of two slices.
    The type of slice returned is the same as the first parameter.
    The array is dynamically allocated and must be released with free(c.ptr).
 
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
typedef struct {type * ptr; int len; int cap;} slice_##type;

    // Returns a part of another slice.
#define slice(a, start, end) \
(typeof(a)){.ptr = (a).ptr + (start), .len = (end) - (start),\
.cap = (a).cap - (start)}

    // Returns a part of an array.
    // This range determine the capacity of the slice.
#define slice_array(a, start, end) \
{.ptr = (a)+(start), .len = (end)-(start), .cap = (end)-(start)}
    
    // Declares a new array with a given capacity.
    // The length is set to 0.
#define slice_make(type, capacity) \
{.ptr = memset(malloc(sizeof(type) * capacity), 0, sizeof(type) * capacity), \
.cap = capacity}
    
    // Push items at end of slice.
#define slice_push(a, item) \
do {assert((a).len < (a).cap); (a).ptr[(a).len++] = item;} while (0)
    
    // Pops item from end of slice.
#define slice_pop(a) \
(--(a).len >= 0 ? (a).ptr[(a).len] : (typeof(*(a).ptr)){0}); \
assert((a).len >= 0)
    
    // Returns the item size in bytes of a slice.
#define slice_itemsize(a) \
((unsigned long)((a).ptr + 1) - (unsigned long)(a.ptr))

    // Returns the number of items in _b_ that fits in _a_.
#define slice_minlen(a, b) \
((a).cap < (b).len ? (a).cap : (b).len)

    // Copies content from _b_ into _a_ and returns number of items copied.
#define slice_copy(a, b) \
(memcpy((a).ptr, (b).ptr, slice_minlen(a,b) * slice_itemsize(b)) ? \
slice_minlen(a, b) : 0)
    
    // Cuts a range from slice.
#define slice_cut(a, start, end) \
do {assert((end) >= (start)); memcpy((a).ptr + (start), (a).ptr + (end), \
slice_itemsize(a) * ((a).cap - (end))); (a).len -= (end)-(start);} while (0)
    
    // Deletes item at index, decrementing the indices of the rest.
#define slice_delete(a, index) \
do {memcpy((a).ptr + (index), (a).ptr + (index) + 1, \
slice_itemsize(a) * ((a).cap - (index) - 1)); (a).len--;} while (0)

    // Clears all items in a slice.
#define slice_clear(a) \
do {memset((a).ptr, 0, slice_itemsize(a) * (a).len);} while (0)
    
    // Appends two slices that must be released when no longer needed.
#define slice_merge(a, b) \
(typeof(a)){.ptr = \
(typeof((a).ptr))memcpy((a).len + \
(typeof((a).ptr))memcpy(malloc(slice_itemsize(a) * \
((a).len + (b).len)), (a).ptr, (a).len * slice_itemsize(a)), \
(b).ptr, (b).len * slice_itemsize(b)) - (a).len, \
.len = (a).len + (b).len, .cap = (a).len + (b).len}
    
#endif

#ifdef __cplusplus
}
#endif
