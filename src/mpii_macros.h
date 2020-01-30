#pragma once

#define CONC(a, b) a##b
#define CONCAT(a, b) CONC(a, b)
#define STRINGIFY2(a) #a
#define STRINGIFY(a) STRINGIFY2(a)


// Make a FOREACH macro
#define FE_1(WHAT, X) WHAT(X) 
#define FE_2(WHAT, X, ...) WHAT(X)FE_1(WHAT, __VA_ARGS__)
#define FE_3(WHAT, X, ...) WHAT(X)FE_2(WHAT, __VA_ARGS__)
#define FE_4(WHAT, X, ...) WHAT(X)FE_3(WHAT, __VA_ARGS__)
#define FE_5(WHAT, X, ...) WHAT(X)FE_4(WHAT, __VA_ARGS__)
//... repeat as needed

#define FE_I_1(WHAT, I, X) WHAT(I, X) 
#define FE_I_2(WHAT, I, X, ...) WHAT(I, X)FE_I_1(WHAT, I-1, __VA_ARGS__)
#define FE_I_3(WHAT, I, X, ...) WHAT(I, X)FE_I_2(WHAT, I-1, __VA_ARGS__)
#define FE_I_4(WHAT, I, X, ...) WHAT(I, X)FE_I_3(WHAT, I-1, __VA_ARGS__)
#define FE_I_5(WHAT, I, X, ...) WHAT(I, X)FE_I_4(WHAT, I-1, __VA_ARGS__)


#define GET_MACRO(_1,_2,_3,_4,_5,NAME,...) NAME 

#define FOR_EACH(action,...)  GET_MACRO(__VA_ARGS__,FE_5,FE_4,FE_3,FE_2,FE_1)(action,__VA_ARGS__)

#define FOR_EACH_I(action,...)  GET_MACRO(__VA_ARGS__,FE_I_5,FE_I_4,FE_I_3,FE_I_2,FE_I_1)(action, GET_MACRO(__VA_ARGS__,5,4,3,2,1), __VA_ARGS__)

#define __VA_NARGS__(...) GET_MACRO(__VA_ARGS__,5,4,3,2,1)


/* return the offset of the field MEMBER in a structure TYPE */
#define offset_of(TYPE, MEMBER) ((size_t) & ((TYPE*)0)->MEMBER)

/* Find the global structure's address
 * It needs :
 * - ptr: address of intern field
 * - type: type of the global structure
 * - member: name of the intern field of the global structure
 */
#define container_of(ptr, type, member) \
  ((type*)((char*)(__typeof__(&((type*)0)->member))(ptr)-offset_of(type, member)))
