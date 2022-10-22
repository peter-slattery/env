/* date = August 17th 2022 6:37 pm */

/*

- No includes in this file, but write out errors if things
  aren't present that should be

- Intrinsics - cmp and swap, incrememnt, decrement

- Memory
  x Allocators
    - Allocation Tracking

 x Time
- Thread

- Data Structures
  x Stretchy Buffers
  x Hash Table

 x Basic String Manipulation
   x Ascii, Utf8 & Utf32
  - snprintf, with ability to replace with stb_snprintf

 x File I/O
  - 

  - Sorting
- 

Multiple OSes represented in one file
**NO GRAPHICS, all the other stuff though**

*/

// TODO(PS): 
// - is there a way to determine if compiling to WASM without Emscripten?

////////////////////////////////////////////////////////
// CONTEXT 
// NOTE: credit here goes to the dion team. A lot of this was
// inspired/pulled from metadesk.h

#define ENV_COMPILER_CLANG 1

#define ENV_OS_MAC 0
#define ENV_OS_LINUX 0
#define ENV_OS_WINDOWS 0
#define ENV_OS_WASM 0
#define ENV_OS_EMSCRIPTEN 0
#define ENV_OS_ANDROID 0

#define ENV_ARCH_X64 0
#define ENV_ARCH_X86 0
#define ENV_ARCH_ARM64 0
#define ENV_ARCH_ARM32 0

#define ENV_LANG_CPP 0
#define ENV_LANG_C 0

// Compiler ID
#if defined(__clang__)
#  define ENV_COMPILER_CLANG 1

// Clang Platform ID
#  if defined(__APPLE__) && defined(__MACH__)
#    undef  ENV_OS_MAC
#    define ENV_OS_MAC 1
#  elif defined(__gnu_linux__) || defined(__linux__)
#    undef  ENV_OS_LINUX
#    define ENV_OS_LINUX 1
#  elif defined(_WIN32) || defined(_WIN64)
#    undef  ENV_OS_WINDOWS
#    define ENV_OS_WINDOWS 1
#  elif defined(__EMSCRIPTEN__)
#    undef  ENV_OS_EMSCRIPTEN
#    undef  ENV_OS_WASM
#    define ENV_OS_EMSCRIPTEN 1
#    define ENV_OS_WASM 1
#  elif defined(__ANDROID__)
#    undef  ENV_OS_ANDROID
#    define ENV_OS_ANDROID 1
#  else
#    error Clang does not recognize this platform
#  endif

// Clang Architecture ID
#  if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#    undef  ENV_ARCH_X64
#    define ENV_ARCH_X64 1
#  elif defined(i386) || defined(__i386) || defined(__i386__)
#    undef  ENV_ARCH_X86
#    define ENV_ARCH_X86 1
#  elif defined(__aarch64__)
#    undef  ENV_ARCH_ARM64
#    define ENV_ARCH_ARM64 1
#  elif defined(__arm__)
#    undef  ENV_ARCH_ARM32
#    define ENV_ARCH_ARM32 1
#  else
#    error architecture not supported yet
#  endif 

#elif defined(_MSC_VER)
#  error The MSVC compiler is not supported yet

#elif defined(__GNUC__) || defined(__GNUG__)
#  error The GCC compiler is not supported yet

#endif // Compiler ID

// Language ID
#if defined(__cplusplus)
#  undef  ENV_LANG_CPP
#  define ENV_LANG_CPP 1
#else
#  undef  ENV_LANG_C
#  define ENV_LANG_C   1
#endif

////////////////////////////////////////////////////////
// DO INCLUDES FOR ME

#if !defined(ENV_DONT_DO_SYSTEM_INCLUDES)
// Common
#  include <stdint.h>
#  include <stdio.h>
#  include <stdlib.h>
#  include <errno.h>
#  include <math.h>
#  include <time.h>
#  include <unistd.h>

// Per-OS Includes
#  if ENV_OS_WINDOWS
#    include <windows.h>

#  elif ENV_OS_MAC
#    include <mach/mach_time.h>
#    include <mach-o/dyld.h>
#    include <sys/syslimits.h>
#    include <sys/stat.h>
#    include <libkern/OSAtomic.h>
#    include <pthread.h>

#  elif ENV_OS_LINUX
#    include <sys/stat.h>
#    include <linux/limits.h>
#    include <pthread.h>

#  else
#    error This OS does not do includes for you

#  endif

#endif

////////////////////////////////////////////////////////
// MACRO UTILITIES

// This can be overriden to make all the functions in this
// file be static or to add boilerplate to each function
#if !defined(ENV_FUNCTION)
#  define ENV_FUNCTION
#endif

// Static has many meanings. These macros provide semantically
// relevant keywords for each use
#define internal      static
#define local_persist static
#define global        static
#define local_const   static const
#define global_const  static const

#define external      extern "C"

// Glue two strings together within a macro
// ie. macro_glue(foo, _bar) would produce foo_bar
#define macro_glue_(a,b) a##b
#define macro_glue(a,b) macro_glue_(a,b)

// Turns an identifier into a CString
// ie. macro_stringify(foobar) -> "foobar"
#define macro_stringify_(a) #a
#define macro_stringify(a) macro_stringify_(a)

#define macro_statement(s) do { s } while(0)

// A macro utility that just expands to invalid code
// that will crash the compiler. Good for picking up 
// where you left off
#define dont_compile ImAfraidICantDoThat

// good for tagging #if blocks that you create during
// development, but might need to remember to reenable
// later
#define temp_omit false

#if ENV_LANG_CPP

#  define ZEROED_STRUCT {}
#  define ZERO_STRUCT(type) type{}

#  define LINK_BEGIN extern "C" {
#  define LINK_END   }

#else

#  define ZEROED_STRUCT {0}
#  define ZERO_STRUCT(type) (type){0}

#  define LINK_BEGIN
#  define LINK_END

#endif

// Overridable error printing behavior
#if !defined(error_printf)
#  define error_printf(...) macro_statement(printf("Error: "); printf(__VA_ARGS__); )
#endif

////////////////////////////////////////////////////////
// ASSERT
//
#if !defined(ENV_NO_ASSERTS)

// Run Time Assert

#if !ENV_OS_WASM
#  define assert_always (*((volatile int*)0) = 0xFFFFFFFF)
#else
#  define macro_statement(printf("Assert Failed: %s:%d - %s\n", __FILE__, __LINE__, __FUNCTION__); abort();) 
#endif // !ENV_OS_EMSCRIPTEN

#  if !defined(assert)
#    if defined(ENV_DEBUG)

// Stops execution of the program if the expr evaluates to false
#      define assert(expr) if(!(expr)) { assert_always; }

// asserts always, but semantically indicates that a codepath
// should never be reached
#      define invalid_code_path assert_always

// asserts any time the default case of a switch statement
// is evaluated. 
#      define invalid_default_case default: { assert_always; } break;
#    else
#      define assert(expr)
#      define invalid_code_path
#      define invalid_default_case
#    endif
#  endif // !defined(assert)

// Static Assert
// Asserts at compile time
#  if !defined(static_assert)
#    if ENV_COMPILER_CLANG
#      define static_assert(value, error) _Static_assert((value), error)
#    endif
#  endif // !defined(assert)

#endif // !defined(ENV_NO_ASSERTS)

////////////////////////////////////////////////////////
// TYPES

// Signed Integers
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
static_assert(sizeof(s8)  == 1, "Incorrect size for s8");
static_assert(sizeof(s16) == 2, "Incorrect size for s16");
static_assert(sizeof(s32) == 4, "Incorrect size for s32");
static_assert(sizeof(s64) == 8, "Incorrect size for s64");

// Unsigned Integers
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

static_assert(sizeof(u8)  == 1, "Incorrect size for u8");
static_assert(sizeof(u16) == 2, "Incorrect size for u16");
static_assert(sizeof(u32) == 4, "Incorrect size for u32");
static_assert(sizeof(u64) == 8, "Incorrect size for u64");

// Boolean & Flag Types
typedef s8  b8;
typedef s32 b32;
typedef s64 b64;

// True & False
#if !ENV_LANG_CPP
#  if !defined(true)
#    define true 1
#  endif
#  if  !defined(false)
#    define false 0
#  endif
#endif // !ENV_LANG_CPP

// Floats
typedef float  r32;
typedef double r64;

static_assert(sizeof(r32) == 4, "Incorrect size for r32");
static_assert(sizeof(r64) == 8, "Incorrect size for r64");

// Boundary values of basic types
global_const u8  max_u8  = 0xFF;
global_const u16 max_u16 = 0xFFFF;
global_const u32 max_u32 = 0xFFFFFFFF;
global_const u64 max_u64 = 0xFFFFFFFFFFFFFFFF;

global_const s8  max_s8  = 127;
global_const s16 max_s16 = 32767;
global_const s32 max_s32 = 2147483647;
global_const s64 max_s64 = 9223372036854775807;

global_const s8  min_s8  = -127 - 1;
global_const s16 min_s16 = -32767 - 1;
global_const s32 min_s32 = -2147483647 - 1;
global_const s64 min_s64 = -9223372036854775807 - 1;

global_const r32 max_r32  = 3.402823466e+38f;
global_const r32 min_r32  = -max_r32;
global_const r32 smallest_positive_r32 = 1.1754943508e-38f;
global_const r32 epsilon_r32 = 5.96046448e-8f;

global_const r32 pi_r32 = 3.14159265359f;
global_const r32 half_pi_r32 = 1.5707963267f;
global_const r32 tau_r32 = 6.28318530717f;

global_const r64 max_r64 = 1.79769313486231e+308;
global_const r64 min_r64 = -max_r64;
global_const r64 smallest_positive_r64 = 4.94065645841247e-324;
global_const r64 epsilon_r64 = 1.11022302462515650e-16;

global_const r64 pi_r64 = 3.14159265359f;
global_const r64 half_pi_r64 = 1.5707963267f;
global_const r64 tau_r64 = 6.28318530717f;

global_const r64 nanos_to_seconds_r64 = 1 / 10000000.0;
global_const r64 seconds_to_nanos_r64 = 10000000.0;

global_const u64 nanos_per_usec_u64 = 1000ULL;
global_const u64 nanos_per_msec_u64 = 1000ULL * nanos_per_usec_u64;
global_const u64 nanos_per_sec_u64  = 1000ULL * nanos_per_msec_u64;

r32 deg_to_rad_r32(r32 deg) { return (deg * (pi_r32 / 180.0f)); }
r32 rad_to_deg_r32(r32 rad) { return (rad * (180.0f / pi_r32)); }
r32 deg_to_rad_r64(r64 deg) { return (deg * (pi_r64 / 180.0f)); }
r32 rad_to_deg_r64(r64 rad) { return (rad * (180.0f / pi_r64)); }

#define bytes(x) (x)
#define kb(x) ((x) << 10)
#define mb(x) ((x) << 20)
#define gb(x) ((x) << 30)
#define tb(x) ((u64)(x) << 40)

#define flag_present(b, flag) (((b) & (flag)) != 0)
#define flag_missing(b, flag) (((b) & (flag)) == 0)
#define flag_add(b, flag) (b) = ((b) | (flag))
#define flag_rem(b, flag) (b) = ((b) & (~(flag)))
#define flag_toggle(b, flag) (b) = flag_present((b), (flag)) ? ((b) & (~(flag))) : ((b) | (flag))

////////////////////////////////////////////////////////
// DEBUG UTILS

typedef struct Debug_Location Debug_Location;
struct Debug_Location
{
  char* file;
  char* func;
  u32   line;
};

#if ENV_LANG_CPP
#  define DEBUG_LOC (Debug_Location){ (char*)__FILE__, (char*)__FUNCTION__, (u32)__LINE__ }
#else
#  define DEBUG_LOC (Debug_Location){ (char*)__FILE__, (char*)__FUNCTION__, (u32)__LINE__ }
#endif // ENV_LANG_CPP

////////////////////////////////////////////////////////
// Math

// Fract
ENV_FUNCTION r32 fract_r32(r32 v) { return (r32)modf((r64)v, 0); }
ENV_FUNCTION r64 fract_r64(r64 v) { return (r64)modf(     v, 0); }

// Min & Max
#define max_val(a,b) (((a) > (b)) ? (a) : (b))
#define min_val(a,b) (((a) > (b)) ? (b) : (a))

// Clamp
#define clamp_val(l,v,h) (max_val((l),min_val((h),(v))))
#define clamp_val_01(v) clamp_val(0,(v),1)

// Abs & Sign
#define abs_val(v)  (((v) < 0) ? (-v) : (v))
#define sign_val(v) (((v) < 0) ? (-1) : (1))

ENV_FUNCTION b8 is_pow2(u64 v) { return ((v & (v - 1)) == 0); }
ENV_FUNCTION b8 is_odd(u64 v)  { return ((v & 1) != 0); }

// Lerp
ENV_FUNCTION r32 lerp_r32(r32 l, r32 t, r32 h) { return l + ((h - l) * t); }
ENV_FUNCTION r64 lerp_r64(r64 l, r64 t, r64 h) { return l + ((h - l) * t); }

// Remap
ENV_FUNCTION r32 remap_r32(r32 v, r32 lo, r32 ho, r32 ln, r32 hn) {
  return (((v - lo) / (ho - lo)) * (hn - ln)) + ln;
}
ENV_FUNCTION r64 remap_r64(r64 v, r64 lo, r64 ho, r64 ln, r64 hn) {
  return (((v - lo) / (ho - lo)) * (hn - ln)) + ln;
}

// Round To
ENV_FUNCTION u32
round_to_u32(u32 v, u32 align)
{
  u32 result  = v + (align - 1);
  result -= result % align;
  return result;
}

ENV_FUNCTION u64
round_to_u64(u64 v, u64 align)
{
  u64 result  = v + (align - 1);
  result -= result % align;
  return result;
}

// Round Up to Pow2
// NOTE: These round to the nearest power of 2. If v
// is already a power of two, then v will not be changed
ENV_FUNCTION u32 
round_up_pow2_u32(u32 v) 
{ 
  if (is_pow2((u64)v)) return v;
  v--; 
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}

ENV_FUNCTION u64
round_up_pow2_u64(u64 v) 
{ 
  if (is_pow2(v)) return v;
  v--; 
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v |= v >> 24;
  v |= v >> 32;
  v |= v >> 40;
  v |= v >> 48;
  v++;
  return v;
}

// Smoothstep
ENV_FUNCTION r32 smoothstep_r32(r32 t) { return t*t*(3-(2*t)); }
ENV_FUNCTION r64 smoothstep_r64(r64 t) { return t*t*(3-(2*t)); }

ENV_FUNCTION r32
ease_inout_cubic_r32(r32 v)
{
  assert(v >= 0 && v <= 1);
  // Equation:
  // [0,0.5) = 4 * v^3
  // [0.5,1] = 1 - (-2 * v + 2)^3 / 2
  if (v < 0.5f) {
    return 4 * v * v * v;
  } else {
    r32 a = -2 * v + 2;
    r32 a3 = a * a * a;
    return 1 - a3 / 2;
  }
}

////////////////////////////////////////////////////////
// C Strings

ENV_FUNCTION u64
cstr_len(char* v)
{
  u64 result = 0;
  while (v[result] != 0) result++;
  return result;
}

ENV_FUNCTION b8
cstr_equals_len(char* a, char* b, u64 len)
{
  for (u64 i = 0; i < len; i++) {
    if (a[i] == 0 || b[i] == 0) {
      if (a[i] != b[i]) {
        return false; // one string did not end here
      }
      return true; // both strings end, short of len
    }
    if (a[i] != b[i]) return false; // strings not equal
  }
  return true;
}

ENV_FUNCTION b8
cstr_equals(char* a, char* b)
{
  u64 a_len = cstr_len(a);
  return cstr_equals_len(a, b, a_len);
}

ENV_FUNCTION void
cstr_copy_len(char* from, char* to, u64 len)
{
  for (u32 i = 0; i < len; i++) to[i] = from[i];
}

ENV_FUNCTION void
cstr_copy(char* from, char* to)
{
  u64 from_len = cstr_len(from);
  cstr_copy_len(from, to, from_len);
}


////////////////////////////////////////////////////////
// SLL & DLL

// Utilities for Singly Linked Lists
#define sll_init(first, last, value)    macro_statement((first) = (value); (last) = (value); (value)->next = 0; )
#define sll_prepend_unsafe(first, last, value) macro_statement((value)->next = (first); (first) = (value);)
#define sll_append_unsafe(first, last, value)  macro_statement((last)->next = (value); (last) = (value); (value)->next = 0;)
#define sll_prepend(first, last, value) macro_statement(\
if (!(first)) { sll_init((first), (last), (value)); } \
else { sll_prepend_unsafe((first), (last), (value)); } \
)
#define sll_append(first, last, value) macro_statement(\
if (!(first)) { sll_init((first), (last), (value)); } \
else { sll_append_unsafe((first), (last), (value)); } \
)
#define sll_insert(before, after, value) macro_statement((before)->next = (value); (value)->next = (after);)

// TODO(PS): Doubly Linked Lists

////////////////////////////////////////////////////////
// Stretchy Buffers

////////////////////////////////////////////////////////
// Hash Table
// Uses robin hood probing

#define HASHTABLE_TOMBSTONE (u32)(1 << 31)
#define HASHTABLE_ENTRY_IS_DELETED(v) (((v) & HASHTABLE_TOMBSTONE) != 0)
#define HASHTABLE_VALIDATE_KEY(k) (!HASHTABLE_ENTRY_IS_DELETED(k))
#define HASHTABLE_MAKE_KEY_VALID(k) ((k) & ~HASHTABLE_TOMBSTONE)

ENV_FUNCTION u32
ht_desired_pos(u32 ht_cap, u32 key)
{
  assert(is_pow2(ht_cap));
  u32 cap_mask = ht_cap - 1;
  u32 result = key % cap_mask;
  if (result == 0) result = 1;
  return result;
}

ENV_FUNCTION u32
ht_probe_distance(u32 ht_cap, u32 key, u32 pos)
{
  return pos - ht_desired_pos(ht_cap, key);
}

ENV_FUNCTION void
ht_insert(u32* ht_keys, u8** ht_values, u32 ht_cap, u32 key, u8* value, u32 index)
{
  assert(index > 0 && index < ht_cap);
  ht_keys[index] = key;
  ht_values[index] = value;
}

ENV_FUNCTION void
ht_add(u32* ht_keys, u8** ht_values, u32 ht_cap, u32 key, u8* value)
{
  assert(ht_cap > 0 && is_pow2(ht_cap));
  assert(HASHTABLE_VALIDATE_KEY(key));
  
  u32 active_key = HASHTABLE_MAKE_KEY_VALID(key);
  u8* active_value = value;
  u32 index = ht_desired_pos(ht_cap, active_key);
  u32 dist = 0;
  for (;;) 
  {
    if (ht_keys[index] == 0)
    {
      ht_insert(ht_keys, ht_values, ht_cap, active_key, active_value, index);
      break;
    }
    
    u32 existing_key = ht_keys[index];
    if (HASHTABLE_ENTRY_IS_DELETED(existing_key))
    {
      ht_insert(ht_keys, ht_values, ht_cap, active_key, active_value, index);
      break;
    }
    
    u32 existing_dist = ht_probe_distance(ht_cap, existing_key, index);
    if (existing_dist < dist)
    {
      u8* existing_value = ht_values[index];
      ht_values[index] = active_value;
      ht_keys[index]   = active_key;
      active_key = existing_key;
      active_value = existing_value;
      dist = existing_dist;
    }
    
    index = (index + 1) & (ht_cap - 1);
    dist += 1;
  }
}

ENV_FUNCTION u32
ht_lookup_index(u32* ht_keys, u32 ht_cap, u32 key, b8* is_empty)
{
  assert(is_pow2(ht_cap));
  assert(HASHTABLE_VALIDATE_KEY(key));
  
  if (!ht_keys || ht_cap == 0) {
    if (is_empty) *is_empty = true;
    return 0;
  }
  
  key = HASHTABLE_MAKE_KEY_VALID(key);
  u32 index = ht_desired_pos(ht_cap, key);
  while (ht_keys[index] != 0 && ht_keys[index] != key) {
    index = (index + 1) & (ht_cap - 1);
  }
  
  u32 fkey = ht_keys[index];
  if (fkey == 0 || HASHTABLE_ENTRY_IS_DELETED(fkey)) 
  {
    index = 0;
    if (is_empty) *is_empty = true;
  }
  
  return index;
}

ENV_FUNCTION b8
ht_rem(u32* ht_keys, u8** ht_values, u32 ht_cap, u32 key)
{
  b8 is_empty = false;
  u32 index = ht_lookup_index(ht_keys, ht_cap, key, &is_empty);
  if (is_empty) return false;
  ht_keys[index] = HASHTABLE_TOMBSTONE;
  ht_values[index] = 0;
  return true;
}

ENV_FUNCTION u8*
ht_get(u32* ht_keys, u8** ht_values, u32 ht_cap, u32 key)
{
  b8 is_empty = false;
  u32 index = ht_lookup_index(ht_keys, ht_cap, key, &is_empty);
  if (is_empty) return 0;
  return ht_values[index];
}

typedef struct Hashtable Hashtable;
struct Hashtable
{
  u32* keys;
  u8** values;
  u32 cap;
  u32 used;
};

ENV_FUNCTION void
hashtable_add(Hashtable* ht, u32 key, u8* value)
{
  ht_add(ht->keys, ht->values, ht->cap, key, value);
  ht->used += 1;
}

ENV_FUNCTION u32
hashtable_lookup_index(Hashtable* ht, u32 key, b8* is_empty)
{
  return ht_lookup_index(ht->keys, ht->cap, key, is_empty);
}

ENV_FUNCTION b8
hashtable_rem(Hashtable* ht, u32 key)
{
  return ht_rem(ht->keys, ht->values, ht->cap, key);
}

ENV_FUNCTION u8*
hashtable_get(Hashtable* ht, u32 key)
{
  return ht_get(ht->keys, ht->values, ht->cap, key);
}


////////////////////////////////////////////////////////
// String Hashing

// DJB2 Hash
// High Speed, High Quality, Non Cryptographic hash
// Source: http://www.cse.yorku.ca/~oz/hash.html

ENV_FUNCTION u64
hash_u64_djb2_append(u64 hash, char* base, u64 len)
{
  for (u64 i = 0; i < len; i++) {
    hash = ((hash << 5) + hash) + (u8)base[i];
  }
  return hash;
}

ENV_FUNCTION u64
hash_u64_djb2(char* base, u64 len)
{
  u64 hash = 5381;
  return hash_u64_djb2_append(hash, base, len);
}

////////////////////////////////////////////////////////
// Random Number Generator

// Get a random number in the range from 0 to max_u32
ENV_FUNCTION u32
random_next(u32 last)
{
  u32 result = last;
  result ^= result << 13;
  result ^= result >> 17;
  result ^= result << 5;
  return result;
}

// Get a random floating point number in the range from 0 to 1
ENV_FUNCTION r32
random_next_unilateral(u32 last)
{
  u32 next = random_next(last);
  r32 result = (r32)next / (r32)max_u32;
  return result;
}

// Get a random floating point number in the range from -1 to 1
ENV_FUNCTION r32
random_next_bilateral(u32 last)
{
  u32 next = random_next(last);
  r32 result = (r32)next / max_r32;
  result = (result * 2) - 1;
  return result;
}

typedef struct Random_Series Random_Series;
struct Random_Series
{
  u32 last;
};

ENV_FUNCTION Random_Series
random_series_init(u32 seed)
{
  Random_Series result = ZEROED_STRUCT;
  result.last = seed;
  return result;
}

ENV_FUNCTION u32
random_series_next(Random_Series* rs)
{
  rs->last = random_next(rs->last);
  return rs->last;
}

ENV_FUNCTION u32
random_series_next_unilateral(Random_Series* rs)
{
  u32 next = random_series_next(rs);
  r32 result = (r32)rs->last / (r32)max_u32;
  return result;
}

ENV_FUNCTION u32
random_series_next_bilateral(Random_Series* rs)
{
  u32 next = random_series_next(rs);
  r32 result = (r32)rs->last / (r32)max_u32;
  result = (result * 2) - 1;
  return result;
}

////////////////////////////////////////////////////////
// MEMORY

#define zero_struct(s) mem_zero((u8*)s, sizeof(*s))
ENV_FUNCTION void
mem_zero(u8* base, u64 len)
{
  for (u32 i = 0; i < len; i++) base[i] = 0;
}

ENV_FUNCTION void 
mem_copy(u8* from, u8* to, u64 len) 
{
  for (u32 i = 0; i < len; i++) to[i] = from[i];
}

// MEMORY CURSOR
// 
// A structure for writing to a buffer of memory

typedef struct Memory_Cursor Memory_Cursor;
struct Memory_Cursor
{
  u8* base;
  u64 cap;
  u64 at;
};

// Advances the at pointer of a memory cursor without writing
// any data, returning the original pointer at offset cursor.at
ENV_FUNCTION u8*
memory_cursor_push_size(Memory_Cursor* c, u64 size)
{
  assert((c->at + size) <= c->cap);
  u8* result = c->base + c->at;
  c->at += size;
  return result;
}

#define memory_cursor_push_struct(c, type) (type*)memory_cursor_push_size((c), sizeof(type))
#define memory_cursor_push_array(c, type, len) (type*)memory_cursor_push_size((c), sizeof(type) * (len))


ENV_FUNCTION u8*
memory_cursor_push_size_zeroed(Memory_Cursor* c, u64 size)
{
  u8* result = memory_cursor_push_size(c, size);
  for (u64 i = 0; i < size; i++) result[i] = 0;
  return result;
}

#define memory_cursor_push_struct_zeroed(c, type) (type*)memory_cursor_push_size_zeroed((c), sizeof(type))
#define memory_cursor_push_array_zeroed(c, type, len) (type*)memory_cursor_push_size_zeroed((c), sizeof(type) * (len))

// Returns the base address of the written struct
ENV_FUNCTION u8*
memory_cursor_write(Memory_Cursor* c, u8* data, u64 size)
{
  assert(c->at < c->cap);
  u8* result = c->base + c->at;
  for (u64 i = 0; i < size; i++) c->base[c->at++] = data[i];
  return result;
}

#define memory_cursor_write_struct(cursor, value) memory_cursor_write((cursor), (u8*)&(value), sizeof(value))
#define memory_cursor_write_array(cursor, base, len) memory_cursor_write((cursor), (u8*)(base), sizeof((base)[0])* (len))

ENV_FUNCTION u8
memory_cursor_read_u8(Memory_Cursor* c)
{
  assert((c->at + 1) <= c->cap);
  u8 result = c->base[c->at++];
  return result;
}

ENV_FUNCTION u16
memory_cursor_read_u16(Memory_Cursor* c)
{
  assert((c->at + 2) <= c->cap);
  u16 result = (
    ((u16)c->base[c->at++] & 0xFF) |
    ((u16)c->base[c->at++] & 0xFF) << 8
  );
  return result;
}

ENV_FUNCTION u32
memory_cursor_read_u32(Memory_Cursor* c)
{
  assert((c->at + 4) <= c->cap);
  u32 result = (
    ((u32)c->base[c->at++] & 0xFF) << 0  |
    ((u32)c->base[c->at++] & 0xFF) << 8  |
    ((u32)c->base[c->at++] & 0xFF) << 16 |
    ((u32)c->base[c->at++] & 0xFF) << 24
  );
  return result;
}

ENV_FUNCTION u64
memory_cursor_read_u64(Memory_Cursor* c)
{
  assert((c->at + 8) <= c->cap);
  u64 result = (
    ((u64)c->base[c->at++] & 0xFF) << 0  |
    ((u64)c->base[c->at++] & 0xFF) << 8  |
    ((u64)c->base[c->at++] & 0xFF) << 16 |
    ((u64)c->base[c->at++] & 0xFF) << 24 |
    ((u64)c->base[c->at++] & 0xFF) << 32 |
    ((u64)c->base[c->at++] & 0xFF) << 40 |
    ((u64)c->base[c->at++] & 0xFF) << 48 |
    ((u64)c->base[c->at++] & 0xFF) << 56
  );
  return result;
}

ENV_FUNCTION void
memory_cursor_read_to(Memory_Cursor* c, u8* dest, u64 size)
{
  assert((c->at + size) <= c->cap);
  for (u64 i = 0; i < size; i++)
  {
    dest[i] = c->base[c->at++];
  }
}

#define memory_cursor_read_struct(c, dest_ptr) memory_cursor_read_to((c), (u8*)(dest_ptr), sizeof(dest_ptr[0]))
#define memory_cursor_read_array(c, dest_ptr, len) memory_cursor_read_to((c), (u8*)(dest_ptr), sizeof(dest_ptr[0]) * (len))

// OS MEMORY INTERFACE
//
// Provides a unified interface to the necessary memory
// operations for a program to run

ENV_FUNCTION u8*  os_memory_reserve(u64 size);
ENV_FUNCTION b8   os_memory_commit(u8* base, u64 size);
ENV_FUNCTION void os_memory_decommit(u8* base, u64 size);
ENV_FUNCTION void os_memory_release(u8* base, u64 size);

#if ENV_OS_MAC || ENV_OS_LINUX || ENV_OS_WASM

ENV_FUNCTION u8*  
os_memory_reserve(u64 size)
{
  return (u8*)malloc(size);
}

ENV_FUNCTION b8
os_memory_commit(u8* base, u64 size)
{
  return (base != 0);
}

ENV_FUNCTION void 
os_memory_decommit(u8* base, u64 size)
{
  return;
}

ENV_FUNCTION void
os_memory_release(u8* base, u64 size)
{
  free((void*)base);
}

#elif ENV_OS_WINDOWS

ENV_FUNCTION u8*  
os_memory_reserve(u64 size)
{
  return (u8*)VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
}

ENV_FUNCTION u8*  
os_memory_commit(u8* base, u64 size)
{
  return (VirtualAlloc((void*)base, size, MEM_COMMIT, PAGE_READWRITE) != 0);
}

ENV_FUNCTION void 
os_memory_decommit(u8* base, u64 size)
{
  VirtualFree((void*)base, size, MEM_DECOMMIT);
}

ENV_FUNCTION void
os_memory_release(u8* base, u64 size)
{
  VirtualFree((void*)base, 0, MEM_RELEASE);
}

#elif ENV_OS_ANDROID
# error Memory interface for Android not implemented yet
#endif

// ALLOCATOR
//
// There really should only be one allocator per program
// All an allocator does is provide an interface to the low
// level memory allocation functions exposed by the operating
// system. 
// 
// However, there are two layers of functions, one which takes
// an allocator parameter, the other which assumes a global 
// Allocator, which is exposed within this file

typedef struct Allocator_Allocation Allocator_Allocation;
struct Allocator_Allocation
{
  u64 addr;
  u64 size;
  char usage[32];
  Debug_Location alloc_loc;
  Debug_Location free_loc;
};

// @Maintainence: Flags
enum 
{
  AllocatorFlag_None = 0,
  AllocatorFlag_DontTrackAllocations = 1
};

typedef struct Allocator Allocator;
struct Allocator
{
  u8 flags;
  
  Allocator_Allocation* allocations;
  u32                   allocations_len;
  u32                   allocations_cap;
};

global Allocator allocator_g = ZEROED_STRUCT;

ENV_FUNCTION void
allocator_track_alloc(Allocator* a, u8* base, u64 size, char* usage, Debug_Location loc)
{
  if (a->allocations_len >= a->allocations_cap) 
  {
    // Get New Size
    u32 new_cap = a->allocations_cap * 2;
    if (new_cap == 0) new_cap = 1024;
    u64 new_size = new_cap * sizeof(Allocator_Allocation);
    
    // Allocate and Copy Over old values
    Allocator_Allocation* new_mem = (Allocator_Allocation*)os_memory_reserve(new_size);
    os_memory_commit((u8*)new_mem, new_size);
    for (u32 i = 0; i < a->allocations_len; i++) new_mem[i] = a->allocations[i];
    
    // Free Old Memory
    u64 old_size = a->allocations_cap * sizeof(Allocator_Allocation);
    os_memory_decommit((u8*)a->allocations, old_size);
    os_memory_release((u8*)a->allocations, old_size);
    
    a->allocations = new_mem;
  }
  
  Allocator_Allocation aa = ZEROED_STRUCT;
  aa.addr = (u64)base;
  aa.size = size;
  cstr_copy_len(usage, aa.usage, 32);
  aa.alloc_loc = loc;
  
  a->allocations[a->allocations_len++] = aa;
}

ENV_FUNCTION void
allocator_track_free(Allocator* a, u8* base, u64 size, Debug_Location loc)
{
  // TODO(PS): 
}

ENV_FUNCTION u8*
mem_alloc_allocator(Allocator* a, u64 size, char* usage, Debug_Location loc)
{
  u8* result = os_memory_reserve(size);
  if (!os_memory_commit(result, size)) {
    error_printf("unable to commit\n");
    invalid_code_path;
  }
  allocator_track_alloc(a, result, size, usage, loc);
  return result;
}

ENV_FUNCTION u8*
mem_alloc(u64 size, char* usage, Debug_Location loc)
{
  return mem_alloc_allocator(&allocator_g, size, usage, loc);
}

ENV_FUNCTION void
mem_free_allocator(Allocator* a, u8* base, u64 size, Debug_Location loc)
{
  allocator_track_free(a, base, size, loc);
  os_memory_decommit(base, size);
  os_memory_release(base, size);
}

ENV_FUNCTION void
mem_free(u8* base, u64 size, Debug_Location loc)
{
  mem_free_allocator(&allocator_g, base, size, loc);
}

// ARENA
//
// Arenas are how a program doles out memory according to different
// needs. Arena's track large blocks of memory and can implement
// different interfaces to access that memory.
// ie. a Push Buffer Arena vs a Free List Arena vs etc...

typedef struct Memory_Arena Memory_Arena;

typedef u8*  Memory_Arena_Alloc(Memory_Arena* arena, u64 size, Debug_Location loc);
typedef u8*  Memory_Arena_Realloc(Memory_Arena* arena, u64 old_size, u64 new_size, Debug_Location loc);
typedef void Memory_Arena_Free(Memory_Arena* arena, u64 size, Debug_Location loc);
typedef void Memory_Arena_Clear(Memory_Arena* arena);

struct Memory_Arena
{
  char* name;
  Memory_Arena_Alloc* alloc;
  Memory_Arena_Realloc* realloc;
  Memory_Arena_Free* free;
  Memory_Arena_Clear* clear;
  u8* arena_data;
  u8 alignment;
};

ENV_FUNCTION u8*
memory_arena_alloc_(Memory_Arena* arena, u64 size, Debug_Location loc)
{
  if (arena->alloc) return arena->alloc(arena, size, loc);
  return 0;
}

ENV_FUNCTION u8*
memory_arena_realloc_(Memory_Arena* arena, u64 old_size, u64 new_size, Debug_Location loc)
{
  if (arena->realloc) return arena->realloc(arena, old_size, new_size, loc);
  return 0;
}

ENV_FUNCTION void
memory_arena_free_(Memory_Arena* arena, u64 size, Debug_Location loc)
{
  if (arena->free) return arena->free(arena, size, loc);
}

ENV_FUNCTION void
memory_arena_clear(Memory_Arena* arena)
{
  if (arena->clear) return arena->clear(arena);
}

#define memory_arena_alloc(arena, size)                   memory_arena_alloc_((arena), (size), DEBUG_LOC)
#define memory_arena_alloc_struct(arena, type)     (type*)memory_arena_alloc_((arena), sizeof(type), DEBUG_LOC)
#define memory_arena_alloc_array(arena, type, cap) (type*)memory_arena_alloc_((arena), sizeof(type) * (cap), DEBUG_LOC)

#define memory_arena_realloc(arena, old_size, new_Size)                  memory_arena_realloc_((arena), (old_size), (new_size), DEBUG_LOC)
#define memory_arena_realloc_array(arena, type, old_cap, new_cap) (type*)memory_arena_realloc_((arena), sizeof(type) * (old_cap), sizeof(type) * (new_cap), DEBUG_LOC)

#define memory_arena_free(arena, size)            memory_arena_free_((arena), (size), DEBUG_LOC)
#define memory_arena_free_struct(arena, type)     memory_arena_free_((arena), sizeof(type), DEBUG_LOC
#define memory_arena_free_array(arena, type, cap) memory_arena_free_((arena), sizeof(type) * (cap), DEBUG_LOC)

// Push Buffer Arena

#define ENV_PUSH_BUFFER_START_CANARY 121
#define ENV_PUSH_BUFFER_END_CANARY   212

typedef struct Push_Buffer_Arena Push_Buffer_Arena;
struct Push_Buffer_Arena
{
  u8 start_canary__;
  u8** buffers;
  u16  buffers_cap;
  u16  buffers_len;
  
  u64  buffer_cap; // amount of memory in each buffer
  u64  cur_buffer_at; // how much memory in the current buffer has been allocated out  
  
  // TODO(PS): ideally this serves more purposes than a bool
  b8 zero_on_clear;
  
  u8 end_canary__;
};

ENV_FUNCTION void
push_buffer_valid__(Memory_Arena* a)
{
  Push_Buffer_Arena* pb = (Push_Buffer_Arena*)a->arena_data;
  assert(pb->start_canary__ == ENV_PUSH_BUFFER_START_CANARY);
  assert(pb->end_canary__ == ENV_PUSH_BUFFER_END_CANARY);
  assert(pb->buffers_len > 0 && pb->buffers_len <= pb->buffers_cap);
}

#if defined(ENV_DEBUG)
#  define PUSH_BUFFER_VALID(a) push_buffer_valid__(a)
#else
#  define PUSH_BUFFER_VALID(a)
#endif

ENV_FUNCTION u8*
push_buffer_alloc(Memory_Arena* arena, u64 size, Debug_Location loc)
{
  PUSH_BUFFER_VALID(arena);
  Push_Buffer_Arena* pb = (Push_Buffer_Arena*)arena->arena_data;
  
  // Is there enough room in the current buffer?
  if ((pb->cur_buffer_at + size) > pb->buffer_cap)
  {
    pb->cur_buffer_at = 0;
    pb->buffers_len += 1;
    assert(pb->buffers_len <= pb->buffers_cap); // out of pages
  }
  
  // HAs the current buffer been allocated?
  u16 buffer_at_i = pb->buffers_len - 1;
  u8* buffer_at = pb->buffers[buffer_at_i];
  if (buffer_at == 0) 
  {
    u64 size_needed = max_val(size, pb->buffer_cap);
    buffer_at = os_memory_reserve(size_needed);
    os_memory_commit(buffer_at, size_needed);
    pb->buffers[buffer_at_i] = buffer_at;
  }
  
  u8* result = buffer_at + pb->cur_buffer_at;
  
  // Align cur_buffer_at for the next allocation
  pb->cur_buffer_at += size;
  pb->cur_buffer_at = round_to_u64(pb->cur_buffer_at, arena->alignment);
  if (pb->cur_buffer_at > pb->buffer_cap)
  {
    pb->cur_buffer_at = 0;
    pb->buffers_len += 1;
    assert(pb->buffers_len <= pb->buffers_cap); // out of pages
  }
  
  return result;
}

ENV_FUNCTION void
push_buffer_clear(Memory_Arena* arena)
{
  PUSH_BUFFER_VALID(arena);
  Push_Buffer_Arena* pb = (Push_Buffer_Arena*)arena->arena_data;
  pb->buffers_len = 1;
  pb->cur_buffer_at = 0;
  
  // Optionally zero all cleared memory
  if (pb->zero_on_clear)
  {
    for (u16 i = 0; i < pb->buffers_cap; i++)
    {
      u8* b = pb->buffers[i];
      if (!b) break;
      for (u64 j = 0; j < pb->buffer_cap; j++) b[j] = 0;
    }
  }
}

ENV_FUNCTION Memory_Arena
push_buffer_arena_create(Allocator* allocator, u16 buffers_cap, u64 buffer_cap, char* name, u8 alignment)
{
  Memory_Arena result = ZEROED_STRUCT;  
  result.alloc = push_buffer_alloc;
  result.clear = push_buffer_clear;
  result.alignment = alignment;
  
  // Obtain Memory Required
  u64 buffers_size = sizeof(u8*) * buffers_cap;
  u64 data_size = sizeof(Push_Buffer_Arena);
  u64 name_size = cstr_len(name);  
  Memory_Cursor cur = ZEROED_STRUCT;
  cur.cap = buffers_size + data_size + name_size;
  cur.base = mem_alloc_allocator(allocator, cur.cap, name, DEBUG_LOC);
  
  // Store Name
  result.name = (char*)memory_cursor_write_array(&cur, name, name_size);
  
  // Prep Push Buffer Data
  Push_Buffer_Arena* pb = memory_cursor_push_struct(&cur, Push_Buffer_Arena);
  pb->start_canary__ = ENV_PUSH_BUFFER_START_CANARY;
  pb->buffers = memory_cursor_push_array_zeroed(&cur, u8*, buffers_cap);
  pb->buffers_cap = buffers_cap;
  pb->buffers_len = 1;
  pb->buffer_cap = buffer_cap;
  pb->cur_buffer_at = 0;
  pb->end_canary__   = ENV_PUSH_BUFFER_END_CANARY;
  result.arena_data = (u8*)pb;
  
  return result;
}

////////////////////////////////////////////////////////
// Stretchy Buffers

#define SB_SENTINEL 0xA0B1C2D3

typedef u32 Stretchy_Buffer_Flags;
enum {
  // Defaults are:
  // MaintainOrderOnRemove
  Sb_Defaults = 0,
  
  Sb_MaintainOrderOnRemove = 1,
  
};

typedef struct Stretchy_Buffer_Header Stretchy_Buffer_Header;
struct Stretchy_Buffer_Header
{
  u32 cap;
  u32 len;
  u32 ele_size;
  u32 flags;
  u32 sentinel;
};

#define sb_get_header(base) (((Stretchy_Buffer_Header*)base) - 1)
#define sb_is_valid(base) (sb_get_header(base)->sentinel == SB_SENTINEL)
#if defined(ENV_DEBUG)
#  define sb_debug_validate(base) assert(sb_is_valid(base))
#else
#  define sb_debug_validate(base)
#endif
#define sb_mem_size(header) ((header->cap * header->ele_size) + sizeof(Stretchy_Buffer_Header))

#define sb_init(cap_init, type, flags) (type*)sb_init_((cap_init), sizeof(type), (flags))
#define sb_len(base) (sb_get_header(base)->len)
#define sb_cap(base) (sb_get_header(base)->cap)

ENV_FUNCTION u8*
sb_init__(u32 cap, u32 len, u32 ele_size, Stretchy_Buffer_Flags flags)
{
  u32 size_needed = (cap * ele_size) + sizeof(Stretchy_Buffer_Header);
  
  u8* buffer = os_memory_reserve(size_needed);
  os_memory_commit(buffer, size_needed);
  
  if (flags == Sb_Defaults)
  {
    flags = (Sb_MaintainOrderOnRemove);
  }
  
  Stretchy_Buffer_Header* header = (Stretchy_Buffer_Header*)buffer;
  header->cap = cap;
  header->len = len;
  header->ele_size = ele_size;
  header->flags = flags;
  header->sentinel = 0xA0B1C2D3;
  
  u8* base = (u8*)(header + 1);
  sb_debug_validate(base);
  return base;
}

ENV_FUNCTION u8*
sb_init_(u32 cap_init, u32 ele_size, Stretchy_Buffer_Flags flags)
{
  return sb_init__(cap_init, 0, ele_size, flags);
}

ENV_FUNCTION void
sb_free(u8* base)
{
  sb_debug_validate(base);
  Stretchy_Buffer_Header* header = sb_get_header(base);
  os_memory_decommit((u8*)header, sb_mem_size(header));
  os_memory_release((u8*)header, sb_mem_size(header));
}

ENV_FUNCTION u8*
sb_maybe_grow(u8* base, u32 room_needed)
{
  sb_debug_validate(base);
  
  Stretchy_Buffer_Header* header = sb_get_header(base);
  if (header->len + room_needed <= header->cap) return base;
  
  u32 new_cap = max_val(header->cap * 2, header->cap + room_needed);
  u8* new_mem = sb_init__(new_cap, header->len, header->ele_size, header->flags);
  u32 old_size = header->len * header->ele_size;
  mem_copy(base, new_mem, old_size);
  
  sb_free(base);
  
  return new_mem;
}

ENV_FUNCTION void
sb_shift(u8* base, s32 ele_min, s32 ele_max, s32 ele_offset)
{
  Stretchy_Buffer_Header* header = sb_get_header(base);
  assert((s32)header->len + ele_offset < (s32)header->cap);
  if (ele_offset > 0) 
  {
    // shift arr[i + x] = arr[i]
    s32 offset_size = ele_offset * header->ele_size;
    u8* first = base + (ele_max * ele_offset);
    u8* last  = base + (ele_min * ele_offset);
    for (u8* at = first; at >= last; at--)
    {
      *(at + offset_size) = *at;
    }
  }
  else if (ele_offset < 0)
  {
    // shift arr[i - x] = arr[i]
  }
}

#define sb_append(base, type, ele) ((base) = (type*)sb_maybe_grow((u8*)(base), 1),  \
(base)[sb_get_header(base)->len++] = (ele))
#define sb_prepend(base, type, ele) ((base) = (type*)sb_maybe_grow((u8*)(base), 1), \
sb_shift((base), 0, sb_len(base), 1),          \
(base)[0] = (ele),                             \
sb_get_header(base)->len++)
#define sb_insert(base, type, ele, index) ((base) = (type*)sb_maybe_grow((u8*)(base), 1), \
sb_shift((base), (index), sb_len(base), 1),    \
(base)[(index)] = (ele),                       \
sb_get_header(base)->len++)
#define sb_remove_last(base) ((base)[sb_get_header(base)->len--])
#define sb_remove(base, index)

////////////////////////////////////////////////////////
// BASIC DATA STRUCTURES

typedef struct Data32 Data32;
struct Data32
{
  u8* base;
  u32 size;
};

typedef struct Data64 Data64;
struct Data64
{
  u8* base;
  u64 size;
};

////////////////////////////////////////////////////////
// SORTING

typedef struct Radix_Sort_Entry_U32 
{ 
  u32 value; 
  u32 original_index; 
} Radix_Sort_Entry_U32;

typedef struct Radix_Sort_Result_U32
{ 
  Radix_Sort_Entry_U32* entries;
  u32 cap;
} Radix_Sort_Result_U32;

ENV_FUNCTION void
radix_sort_in_place_u32_(Radix_Sort_Entry_U32* entries, u32 first, u32 opl, u32 iterations)
{
  u32 mask = 1 << iterations;
  u32 boundary_0 = first;
  u32 boundary_1 = opl - 1;
  for (u32 i = first; i < opl; i++)
  {
    Radix_Sort_Entry_U32 entry = entries[boundary_0];
    u32 place = (entry.value & mask) != 0;
    if (place) 
    {
      Radix_Sort_Entry_U32 evicted = entries[boundary_1];
      entries[boundary_1] = entry;
      entries[boundary_0] = evicted;
      boundary_1 -= 1;
    }
    else
    {
      boundary_0 += 1;
    }
  }
  
  if (iterations > 0) 
  {
    iterations -= 1;
    if (first < boundary_0) radix_sort_in_place_u32_(entries, first,      boundary_0, iterations);
    if (boundary_0 < opl)   radix_sort_in_place_u32_(entries, boundary_0, opl,        iterations);
  }
}

ENV_FUNCTION void 
radix_sort_in_place_u32(Radix_Sort_Entry_U32* entries, u32 entries_cap)
{
  u32 value_hi = 0xFFFFFF;
  for (u32 i = 0; i < entries_cap; i++) {
    value_hi = max_val(value_hi, entries[i].value);
  }
  u32 iterations = 0;
  for (; value_hi > 1; iterations++) { value_hi = value_hi >> 1; }
  
  radix_sort_in_place_u32_(entries, 0, entries_cap, iterations);
}

ENV_FUNCTION Radix_Sort_Result_U32
radix_sort_u32(u32* values, u32 values_cap, Memory_Arena* arena)
{
  Radix_Sort_Result_U32 result = ZEROED_STRUCT;
  result.entries = memory_arena_alloc_array(arena, Radix_Sort_Entry_U32, values_cap);
  result.cap = values_cap;
  
  for (u32 i = 0; i < values_cap; i++)
  {
    Radix_Sort_Entry_U32 entry;
    entry.value = values[i];
    entry.original_index = i;
    result.entries[i] = entry;
  }
  
  radix_sort_in_place_u32(result.entries, result.cap);
  
  return result;
}

typedef struct Radix_Sort_Entry_U64 
{ 
  u64 value; 
  u32 original_index; 
} Radix_Sort_Entry_U64;

typedef struct Radix_Sort_Result_U64
{ 
  Radix_Sort_Entry_U64* entries;
  u32 cap;
} Radix_Sort_Result_U64;

ENV_FUNCTION void
radix_sort_in_place_u64_(Radix_Sort_Entry_U64* entries, u32 first, u32 opl, u32 iterations)
{
  u64 mask = 1 << iterations;
  u32 boundary_0 = first;
  u32 boundary_1 = opl - 1;
  for (u32 i = first; i < opl; i++)
  {
    Radix_Sort_Entry_U64 entry = entries[boundary_0];
    u64 place = (entry.value & mask) != 0;
    if (place) 
    {
      Radix_Sort_Entry_U64 evicted = entries[boundary_1];
      entries[boundary_1] = entry;
      entries[boundary_0] = evicted;
      boundary_1 -= 1;
    }
    else
    {
      boundary_0 += 1;
    }
  }
  
  if (iterations > 0) 
  {
    iterations -= 1;
    if (first < boundary_0) radix_sort_in_place_u64_(entries, first,      boundary_0, iterations);
    if (boundary_0 < opl)   radix_sort_in_place_u64_(entries, boundary_0, opl,        iterations);
  }
}

ENV_FUNCTION void 
radix_sort_in_place_u64(Radix_Sort_Entry_U64* entries, u32 entries_cap)
{
  u64 value_hi = 0xFFFFFF;
  for (u32 i = 0; i < entries_cap; i++) {
    value_hi = max_val(value_hi, entries[i].value);
  }
  u32 iterations = 0;
  for (; value_hi > 1; iterations++) { value_hi = value_hi >> 1; }
  
  radix_sort_in_place_u64_(entries, 0, entries_cap, iterations);
}

ENV_FUNCTION Radix_Sort_Result_U64
radix_sort_u64(u64* values, u32 values_cap, Memory_Arena* arena)
{
  Radix_Sort_Result_U64 result = ZEROED_STRUCT;
  result.entries = memory_arena_alloc_array(arena, Radix_Sort_Entry_U64, values_cap);
  result.cap = values_cap;
  
  for (u32 i = 0; i < values_cap; i++)
  {
    Radix_Sort_Entry_U64 entry;
    entry.value = values[i];
    entry.original_index = i;
    result.entries[i] = entry;
  }
  
  radix_sort_in_place_u64(result.entries, result.cap);
  
  return result;
}

////////////////////////////////////////////////////////
// STRINGS

// All Strings here are considered to be immutable
// Functions like Substring return memory that points into the source
// string. THis is considered safe since the source string is also assumed
// to be immutable.
//
// There are functions for converting between ASCII, UTF8, and UTF32
// However, for brevity and speed, all string functions operate on UTF32
// strings.

typedef struct Rune32 { u32 v; } Rune32;
typedef struct String8 { char* base; u64 len; } String8;
typedef struct String_Utf32 { u32* base; u64 len; } String_Utf32;
typedef struct String_Utf8  { u8* base;  u64 data_len; u64 len; } String_Utf8;

// Useful for expanding a String8 in a printf context
// ie. printf("%.*s\n", s8_varg(my_string))
#define s8_varg(s) (int)(s).len, (s).base

// Expands a string with arguments in reverse order from s8_varg
#define s8_expand(s) (s).base, (s).len

typedef u32 String_Match_Flags;
enum {
  StringMatch_FindLast = 1,
  StringMatch_CaseInsensitive = 2,
  StringMatch_SlashInsensitive = 4,
};

ENV_FUNCTION String8 
cstr_to_string8(char* str)
{
  String8 result = ZEROED_STRUCT;
  result.base = str;
  result.len = cstr_len(str);
  return result;
}

ENV_FUNCTION void
cstring_to_utf32(char* str, u64 str_len, String_Utf32* dest)
{
  u64 len = min_val(str_len, dest->len);
  for (u64 i = 0; i < len; i++)
  {
    dest->base[i] = (u32)str[i];
  }
}

ENV_FUNCTION String_Utf32
cstring_to_utf32_arena(char* str, Memory_Arena* storage)
{
  u64 len = cstr_len(str);
  String_Utf32 result = ZEROED_STRUCT;
  result.base = memory_arena_alloc_array(storage, u32, len);
  result.len = len;
  cstring_to_utf32(str, len, &result);
  return result;
}

#define ENV_UTF8_ONE_BYTE_MASK     0b10000000
#define ENV_UTF8_ONE_BYTE_VALUE    0b00000000
#define ENV_UTF8_ONE_BYTE_REV_MASK 0b01111111

#define ENV_UTF8_LOWER_BYTE_MASK   0b00111111

#define ENV_UTF8_TWO_BYTE_MASK     0b11100000
#define ENV_UTF8_TWO_BYTE_VALUE    0b11000000
#define ENV_UTF8_TWO_BYTE_REV_MASK 0b00011111

#define ENV_UTF8_THREE_BYTE_MASK     0b11110000
#define ENV_UTF8_THREE_BYTE_VALUE    0b11100000
#define ENV_UTF8_THREE_BYTE_REV_MASK 0b00001111

ENV_FUNCTION u8
utf8_codepoint_size(u8 cp)
{
  u8 result = 0;
  if ((cp & ENV_UTF8_ONE_BYTE_MASK) == ENV_UTF8_ONE_BYTE_VALUE) 
  {
    result = 1;
  }
  else if ((cp & ENV_UTF8_TWO_BYTE_MASK) == ENV_UTF8_TWO_BYTE_VALUE)
  {
    result = 2;
  }
  else if ((cp & ENV_UTF8_THREE_BYTE_MASK) == ENV_UTF8_THREE_BYTE_VALUE)
  {
    result = 3;
  }
  else
  {
    result = 4;
  }
  return result;
}

ENV_FUNCTION u64
string_utf32_to_ascii_buffer(String_Utf32 src, char* dst, u64 dst_cap)
{
  u64 len = min_val(src.len, dst_cap);
  for (u64 i = 0; i < len; i++)
  {
    dst[i] = (u8)(src.base[i] & 0xFF);
  }
  dst[len] = 0;
  return len;
}

ENV_FUNCTION void
string_utf32_to_ascii(String_Utf32 src, String8* dst)
{
  dst->len = string_utf32_to_ascii_buffer(src, (char*)dst->base, dst->len);
}

ENV_FUNCTION String8
string_utf32_to_ascii_arena(String_Utf32 str, Memory_Arena* storage)
{
  String8 result = ZEROED_STRUCT;
  result.base = memory_arena_alloc_array(storage, char, str.len + 1);
  result.len = str.len;
  string_utf32_to_ascii(str, &result);
  return result;
}

// TODO: We can do this without branching
// see here: https://nullprogram.com/blog/2017/10/06/
ENV_FUNCTION void
string_utf8_to_utf32(String_Utf8 src, String_Utf32* dest)
{
  u64 len = min_val(src.len, dest->len);
  u8* src_at = src.base;
  for (u64 i = 0; i < dest->len; i++)
  {
    u8 cp_size = utf8_codepoint_size(src_at[0]);
    switch (cp_size) {
      case 1: { dest->base[i] = (u32)(src_at[0] & ENV_UTF8_ONE_BYTE_REV_MASK); } break;
      
      case 2: {
        dest->base[i] = (u32)(
          (src_at[0] & ENV_UTF8_TWO_BYTE_REV_MASK) << 6 |
          (src_at[1] & ENV_UTF8_LOWER_BYTE_MASK)
        );
      } break;
      
      case 3: {
        // error handling - we don't know that src_at + 1 exists
        dest->base[i] = (u32)(
          (src_at[0] & ENV_UTF8_TWO_BYTE_REV_MASK) << 12 |
          (src_at[1] & ENV_UTF8_LOWER_BYTE_MASK)   << 6 |
          (src_at[2] & ENV_UTF8_LOWER_BYTE_MASK)
        );
      } break;
      
      case 4: {
        dest->base[i] = (u32)(
          (src_at[0] & ENV_UTF8_TWO_BYTE_REV_MASK) << 18 |
          (src_at[1] & ENV_UTF8_LOWER_BYTE_MASK)   << 12 |
          (src_at[2] & ENV_UTF8_LOWER_BYTE_MASK)   << 6 |
          (src_at[3] & ENV_UTF8_LOWER_BYTE_MASK)
        );
      } break;
      
      invalid_default_case;
    }
    
    src_at += cp_size;
  }
}

ENV_FUNCTION String_Utf32
string_utf8_to_utf32_arena(String_Utf8 str, Memory_Arena* storage)
{
  String_Utf32 result = ZEROED_STRUCT;
  result.base = memory_arena_alloc_array(storage, u32, str.len);
  result.len = str.len;
  string_utf8_to_utf32(str, &result);
  return result;
}

ENV_FUNCTION u32
utf32_codepoint_to_utf8(u32 utf32_cp, u8* utf8_cp_len)
{
  u32 result = 0;
  if (utf32_cp < 0x80) 
  { 
    result = utf32_cp;
    *utf8_cp_len = 1;
  }
  else if (utf32_cp < 0x800)
  {
    result = (
      (0b11000000 | (utf32_cp >> 6)   ) << 8 |
      (0b10000000 | (utf32_cp & 0x3f) )
    );
    *utf8_cp_len = 2;
  }
  else if (utf32_cp < 0x10000)
  {
    result = (
      (0b11100000 | (utf32_cp >> 12)       ) << 16 |
      (0b11000000 | (utf32_cp >> 6 ) & 0x3f) << 8 |
      (0b10000000 | (utf32_cp      ) & 0x3f)
    );
    *utf8_cp_len = 3;
  }
  else if (utf32_cp < 0x200000)
  {
    result = (
      (0b11110000 | (utf32_cp >> 18)       ) << 24 |
      (0b11100000 | (utf32_cp >> 12) & 0x3f) << 16 |
      (0b11000000 | (utf32_cp >> 6 ) & 0x3f) << 8 |
      (0b10000000 | (utf32_cp      ) & 0x3f)
    );
    *utf8_cp_len = 4;
  }
  else
  {
    // error case
    *utf8_cp_len = 0;
  }
  return result;
}

ENV_FUNCTION u64
string_utf32_to_utf8_bytes_needed(String_Utf32 src)
{
  u64 result = 0;
  for (u64 i = 0; i < src.len; i++) {
    u8 cp_len = 0;
    u32 cp = utf32_codepoint_to_utf8(src.base[i], &cp_len);
    result += cp_len;
  }
  return result;
}

ENV_FUNCTION void
string_utf32_to_utf8(String_Utf32 src, String_Utf8* dst)
{
  u64 src_i = 0;
  u64 dst_i = 0;
  for(;;) 
  {
    u32 src_cp = src.base[src_i++];
    u8  dst_cp_len = 0;
    u32 dst_cp = utf32_codepoint_to_utf8(src_cp, &dst_cp_len);
    
    if (dst_i + dst_cp_len > dst->data_len) break; // src is longer than dst
    
    u32 shift_amount = 4 - dst_cp_len;
    u8* dst_cp_bytes = (u8*)&dst_cp;
    for (u8 i = 0; i < dst_cp_len; i++) {
      dst->base[dst_i++] = dst_cp_bytes[i + shift_amount];
    }    
  }
  dst->len = src_i - 1;
}

ENV_FUNCTION String_Utf8
string_utf32_to_utf8_arena(String_Utf32 str, Memory_Arena* storage)
{
  String_Utf8 result = ZEROED_STRUCT;
  result.data_len = string_utf32_to_utf8_bytes_needed(str);
  result.base = memory_arena_alloc_array(storage, u8, result.data_len);
  result.len = 0;
  
  string_utf32_to_utf8(str, &result);
  
  return result;
}

ENV_FUNCTION b8 utf32_is_space(u32 cp)        { return ((cp == (u32)' ') || (cp == (u32)'\t')); }
ENV_FUNCTION b8 utf32_is_newline(u32 cp)      { return ((cp == (u32)'\n') || (cp == (u32)'\r')); }
ENV_FUNCTION b8 utf32_is_whitespace(u32 cp)   { return utf32_is_space(cp) || utf32_is_newline(cp); } 
ENV_FUNCTION b8 utf32_is_upper(u32 cp) { return ((cp >= (u32)'A') && (cp <= (u32)'Z')); }
ENV_FUNCTION b8 utf32_is_lower(u32 cp) { return ((cp >= (u32)'a') && (cp <= (u32)'z')); }
ENV_FUNCTION b8 utf32_is_alpha(u32 cp)        { return utf32_is_lower(cp) || utf32_is_upper(cp); }
ENV_FUNCTION b8 utf32_is_numeric(u32 cp)      { return ((cp >= (u32)'0') && (cp <= (u32)'9')); }
ENV_FUNCTION b8 utf32_is_alphanumeric(u32 cp) { return utf32_is_alpha(cp) || utf32_is_numeric(cp) || (cp == (u32)'_'); }
ENV_FUNCTION b8 utf32_is_symbol(u32 cp) {
  return (
    (cp == (u32)'!') || (cp == (u32)'@') || (cp == (u32)'#') ||
    (cp == (u32)'$') || (cp == (u32)'%') || (cp == (u32)'^') ||
    (cp == (u32)'&') || (cp == (u32)'*') || (cp == (u32)'(') ||
    (cp == (u32)')') || (cp == (u32)'-') || (cp == (u32)'_') ||
    (cp == (u32)'=') || (cp == (u32)'+') || (cp == (u32)'[') ||
    (cp == (u32)'{') || (cp == (u32)'}') || (cp == (u32)']') ||
    (cp == (u32)'\\') || (cp == (u32)'|') || (cp == (u32)';') ||
    (cp == (u32)':') || (cp == (u32)'\'') || (cp == (u32)'"') ||
    (cp == (u32)',') || (cp == (u32)'<') || (cp == (u32)'.') ||
    (cp == (u32)'>') || (cp == (u32)'/') || (cp == (u32)'?') ||
    (cp == (u32)'`') || (cp == (u32)'~')
  );
}
ENV_FUNCTION b8 utf32_is_slash(u32 cp) { return ((cp == (u32)'\\') || (cp == '/')); }

ENV_FUNCTION u32 utf32_to_lower(u32 cp) { 
  if (!utf32_is_alpha(cp)) { return cp; }
  if (!utf32_is_upper(cp)) { return cp; }
  return (cp - (u32)'A') + (u32)'a';
}

ENV_FUNCTION u32 utf32_to_upper(u32 cp) { 
  if (!utf32_is_alpha(cp)) { return cp; }
  if (!utf32_is_lower(cp)) { return cp; }
  return (cp - (u32)'a') + (u32)'A';
}

ENV_FUNCTION u32 utf32_to_forward_slash(u32 cp) { 
  if (!utf32_is_slash(cp)) { return cp; }
  return '/';
}

ENV_FUNCTION String_Utf32 
string_substring(String_Utf32 str, u64 start, u64 len)
{
  String_Utf32 result = ZEROED_STRUCT;
  start = min_val(start, str.len);
  len = min_val(start + len, str.len) - start;
  result.base = str.base + start;
  result.len = len;
  return result;
}

// get all codepoints after first
// "Skips the first characters"
ENV_FUNCTION String_Utf32 
string_skip(String_Utf32 str, u64 first)
{
  return string_substring(str, first, str.len - first);
}

// Get all but the last nmax characters
// "Chops off the end nmax characters"
ENV_FUNCTION String_Utf32 
string_chop(String_Utf32 str, u64 nmax)
{
  return string_substring(str, 0, str.len - nmax);
}

// Get the first len characters
ENV_FUNCTION String_Utf32 
string_get_prefix(String_Utf32 str, u64 len)
{
  return string_substring(str, 0, len);
}

// Get the last len characters
ENV_FUNCTION String_Utf32 
string_get_suffix(String_Utf32 str, u64 len)
{
  u64 first = str.len - len;
  if (first > str.len) first = str.len;
  return string_substring(str, str.len - len, len);
}

ENV_FUNCTION b8 
string_match(String_Utf32 a, String_Utf32 b, String_Match_Flags flags)
{
  b8 result = false;
  if (a.len == b.len)
  {
    result = true;
    for (u64 i = 0; i < a.len; i++)
    {
      b8 match = (a.base[i] == b.base[i]);
      if(flags & StringMatch_CaseInsensitive)
      {
        match |= (utf32_to_lower(a.base[i]) == utf32_to_lower(b.base[i]));
      }
      if(flags & StringMatch_SlashInsensitive)
      {
        match |= (utf32_to_forward_slash(a.base[i]) == utf32_to_forward_slash(b.base[i]));
      }
      if(match == 0)
      {
        result = 0;
        break;
      }
    }
  }
  return result;
}

ENV_FUNCTION u64 
string_find_substring(String_Utf32 str, String_Utf32 substr, u64 start, String_Match_Flags flags)
{
  bool found = false;
  u64 found_i = str.len;
  for (u64 i = start; i < str.len; i++) {
    if (i + substr.len < str.len) {
      String_Utf32 at = string_substring(str, i, substr.len);
      if (string_match(at, substr, flags)) {
        found = true;
        found_i = i;
        if (!flag_present(flags, StringMatch_FindLast)) break;
      }
    }
  }
  return found_i;
}

ENV_FUNCTION u64 
string_find_codepoint(String_Utf32 str, u32 codepoint, u64 start, String_Match_Flags flags)
{
  String_Utf32 substr = ZEROED_STRUCT;
  substr.len = 1;
  substr.base = &codepoint;
  return string_find_substring(str, substr, start, flags);
}

// Get everything after the last period
// good for getting the extension of a file
ENV_FUNCTION String_Utf32 
string_chop_last_period(String_Utf32 str)
{
  String_Utf32 result = ZEROED_STRUCT;
  u64 last_period = string_find_codepoint(str, (u32)'.', 0, StringMatch_FindLast);
  if (last_period < str.len) {
    result = string_substring(str, 0, last_period);
  }
  return result;
}

// Get everything up to the last period.
// good for getting a filename
ENV_FUNCTION String_Utf32 
string_skip_last_period(String_Utf32 str)
{
  String_Utf32 result = ZEROED_STRUCT;
  u64 last_period = string_find_codepoint(str, (u32)'.', 0, StringMatch_FindLast);
  if (last_period < str.len) {
    result = string_substring(str, last_period + 1, str.len - last_period + 1);
  }
  return result;
}

// Get everything after the last slash
// good for getting the filename + extension
ENV_FUNCTION String_Utf32
string_skip_last_slash(String_Utf32 str)
{
  String_Utf32 result = ZEROED_STRUCT;
  u64 last_slash = string_find_codepoint(str, (u32)'/', 0, StringMatch_SlashInsensitive | StringMatch_FindLast);
  if (last_slash < str.len) {
    result = string_substring(str, last_slash + 1, str.len - last_slash + 1);
  }
  return result;
}

// Get everything before the last slash
// Good for getting the path leading to a file
ENV_FUNCTION String_Utf32
string_chop_last_slash(String_Utf32 str)
{
  String_Utf32 result = ZEROED_STRUCT;
  u64 last_slash = string_find_codepoint(str, (u32)'/', 0, StringMatch_SlashInsensitive | StringMatch_FindLast);
  if (last_slash < str.len) {
    result = string_substring(str, 0, last_slash);
  }
  return result;
}

// Takes in a c string buffer of known length and converts it
// to utf32 in place. Assumes that cstr_buffer is a block of memory
// of size cstr_buffer_len * sizeof(u32) and that cstr_buffer contains
// a cstring (ie. there is unused space on the end of the buffer
ENV_FUNCTION void
cstr_to_utf32_in_place(char* cstr_buffer, u64 cstr_buffer_len)
{
  // Expand codepoints in place - convert from 8bit codepoints
  // to utf32 codepoints. Because they start off as 8bit and tightly
  // packed, shifting each codepoint to the back of the buffer
  // in reverse order means they will never overlap in the interim
  u32* as_utf32 = (u32*)cstr_buffer;
  for (u64 i = cstr_buffer_len - 1; i < cstr_buffer_len; i--)
  {
    u32 cp = (u32)cstr_buffer[i];
    printf("%llu: %c -> %u\n", i, cstr_buffer[i], cp);
    as_utf32[i] = cp;
  }
}

ENV_FUNCTION String_Utf32 os_get_exe_path(Memory_Arena* storage);

#if ENV_OS_MAC

ENV_FUNCTION String_Utf32 
os_get_exe_path(Memory_Arena* storage)
{
  String_Utf32 result = ZEROED_STRUCT;
  u32 needed = 0;
  s32 r0 = _NSGetExecutablePath(0, &needed);
  
  result.len = (u64)needed;
  result.base = memory_arena_alloc_array(storage, u32, result.len);
  s32 r1 = _NSGetExecutablePath((char*)result.base, &needed);
  printf("%.*s\n", needed, (char*)result.base);
  
  cstr_to_utf32_in_place((char*)result.base, result.len);
  printf("%.*s\n", needed * 4, (char*)result.base);
  return result;
}

#elif ENV_OS_LINUX

ENV_FUNCTION String_Utf32
os_get_exe_path(Memory_Arena* storage)
{
  String_Utf32 result = ZEROED_STRUCT;
  
  // get the size of the file name
  const char* filename = "/proc/self/exe";
  struct stat sb = ZEROED_STRUCT;
  if (lstat(filename, &sb) != -1) 
  {
    u64 buf_size = sb.st_size + 1;
    if (buf_size == 1) buf_size = PATH_MAX;
    
    // allocate the final string
    result.len  = buf_size;
    result.base = memory_arena_alloc_array(storage, u32, result.len);
    
    // read the file name into the buffer
    if (readlink(filename, (char*)result.base, result.len) != -1)
    {
      cstr_to_utf32_in_place((char*)result.base, result.len);
    }
    else
    {
      int err = errno;
      error_printf("readlink error: %d\n", err);
    }
  } 
  else 
  {
    int err = errno;
    error_printf("lstat error: %d\n", err);
  }
  
  return result;
}

#elif ENV_OS_WINDOWS

ENV_FUNCTION String_Utf32
os_get_exe_path(Memory_Arena* storage)
{
  String_Utf32 result = ZEROED_STRUCT;
  result.len = (u64)MAX_PATH;
  result.base = memory_arena_alloc_array(storage, u32, result.len);
  int r = GetModuleFileName(NULL, (char*)result.base, result.len);
  if (r != 0)
  {
    cstr_to_utf32_in_place((char*)result.base, result.len);
  }
  else
  {
    int err = GetLastError();
    error_printf("GetModuleFileName returned %d. Error Code: %d\n", r, err);
  }
  return result;
}

#else
#  error Platform doesn't support os_get_exe_path yet
#endif

////////////////////////////////////////////////////////
// TIME

typedef struct Ticks { u64 v; } Ticks;
typedef struct Seconds { r64 v; } Seconds;

ENV_FUNCTION Ticks   os_ticks_now();
ENV_FUNCTION Seconds os_ticks_to_seconds(Ticks delta);
ENV_FUNCTION void    os_sleep(Seconds seconds);

ENV_FUNCTION Ticks   
ticks_elapsed(Ticks start, Ticks end)
{
  assert(start.v <= end.v);
  Ticks result = ZEROED_STRUCT;
  result.v = end.v - start.v;
  return result;
}

#if ENV_OS_MAC

global u64 env_osx_start_time_abs = 0;
global mach_timebase_info_data_t env_osx_mach_time_info = ZEROED_STRUCT;
global r64 env_osx_ticks_per_second = 0;

ENV_FUNCTION void
os_osx_time_init()
{
  env_osx_start_time_abs = mach_absolute_time();
  mach_timebase_info(&env_osx_mach_time_info);
  env_osx_ticks_per_second = ((r64)env_osx_mach_time_info.denom * 1e9) / (r64)(env_osx_mach_time_info.numer);
}

ENV_FUNCTION Ticks
os_ticks_now()
{
  if (env_osx_start_time_abs == 0) os_osx_time_init();  
  Ticks result = ZEROED_STRUCT;
  result.v = mach_absolute_time() - env_osx_start_time_abs;
  return result;
}

ENV_FUNCTION Seconds 
os_ticks_to_seconds(Ticks delta)
{
  if (env_osx_start_time_abs == 0) os_osx_time_init();
  Seconds result = ZEROED_STRUCT;
  result.v = (r64)delta.v / env_osx_ticks_per_second;
  return result;
}

ENV_FUNCTION void
os_sleep(Seconds seconds)
{
  struct timespec elapsed, tv;
  elapsed.tv_sec = (u32)seconds.v;
  elapsed.tv_nsec = ((u32)(seconds.v * 1000) % 1000) * 100000;
  
  int was_error = 0;
  do {
    errno = 0;
    tv.tv_sec = elapsed.tv_sec;
    tv.tv_nsec = elapsed.tv_nsec;
    was_error = nanosleep(&tv, &elapsed);
  } while (was_error && (errno == EINTR));
}

#elif ENV_OS_LINUX

ENV_FUNCTION Ticks
os_ticks_now()
{
  struct timespec ts = {};
  if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts)) { invalid_code_path; }
  s64 nanos = (s64)ts.tv_sec * NANOS_PER_SECOND;
  nanos += (s64)ts.tv_nsec;
  Ticks result = ZEROED_STRUCT;
  result.v = nanos;
  return result;
}

ENV_FUNCTION Seconds 
os_ticks_to_seconds(Ticks delta)
{
  Seconds result = ZEROED_STRUCT;
  result.v = (r64)delta.v / NANOS_PER_SECOND;
  return result;
}

ENV_FUNCTION void
os_sleep(Seconds seconds)
{
  u32 usec = seconds.v * 1000000);
usleep(usec);
}

#elif ENV_OS_WINDOWS

global r64 win32_performance_counter_freq_r64 = 0;

ENV_FUNCTION Ticks
os_ticks_now()
{
  Ticks result = ZEROED_STRUCT;
  LARGE_INTEGER time;
  if (!QueryPerformanceCounter(&time))
  {
    win32_get_last_error();
    // TODO(Peter): I'm waiting to see an error actually occur here
    // to know what it could possibly be.
    invalid_code_path;
  }
  result.v = (u64)time.QuadPart;
  return result;
}

ENV_FUNCTION Seconds
os_ticks_to_seconds(Ticks delta)
{
  // init if necessary
  if (win32_performance_counter_freq_r64 == 0) 
  {
    LARGE_INTEGER freq;
    if (!QueryPerformanceCounter(&freq)) { invalid_code_path; }
    win32_performance_counter_freq_r64 = (r64)freq.QuadPart;
  }
  
  Seconds result = ZEROED_STRUCT;
  result.v = (r64)delta.v / win32_performance_counter_freq_r64;
  return result;
}

ENV_FUNCTION void
os_sleep(Seconds seconds)
{
  u32 ms = (u32)(seconds.v * 1000.0f);
  Sleep(ms);
}

#else
#  error This platform does not support Time functionality yet
#endif

////////////////////////////////////////////////////////
// FILE I/O

typedef struct File_Handle File_Handle;
struct File_Handle
{
  u64 value;
};

typedef struct File_Info File_Info;
struct File_Info
{
  char* absolute_path;
  u64 size;
  u64 time_created;
  u64 time_last_write;  
};

typedef u32 File_Access;
enum {
  FileAccess_Read,
  FileAccess_Write,
  FileAccess_ReadWrite,
};

typedef u32 File_Create_Mode;
enum {
  FileCreate_Default = 0, // OpenOrCreate
  
  FileCreate_AlwaysOverwrite,
  FileCreate_NewOnly,
  FileCreate_OpenOrCreate,
  FileCreate_OpenOnly,
  FileCreate_OpenAndClear,
};

typedef u8 File_Seek_Basis;
enum {
  FileSeekBasis_Beginning,
  FileSeekBasis_Current,
  FileSeekBasis_End,
};

typedef u32 File_Error;
enum {
  FileError_None = 0,
  
  FileError_BadFileHandle, // EBADFD
  
  // open
  FileError_Open_InvalidAccessMode, // EINVAL
  
  // close
  FileError_Close_FileNotClosable, // EBADF
  
  // seek
  FileError_Seek_InvalidBasis, // EINVAL
  FileError_Seek_FileNotSeekable, // ESPIPE
  
  // tell
  FileError_Tell_PositionTooLargeForS32, // EOVERFLOW
  FileError_Tell_FileNotTellable, // ESPIPE
  
  // read
  FileError_Read_NotEnoughRoom, 
  
  FileError_Unknown,
};

#define SET_FILE_ERROR(dest, err) if (dest != 0) { *(dest) = (err); }

// OS Specific Interface
ENV_FUNCTION b8           os_file_handle_is_valid(File_Handle handle);
ENV_FUNCTION File_Handle  os_file_open(char* path, File_Access access, File_Create_Mode create, File_Error* errout);
ENV_FUNCTION void         os_file_close(File_Handle* handle, File_Error* errout);
ENV_FUNCTION File_Info    os_file_get_info(File_Handle* handle, Memory_Arena* storage, File_Error* errout);
ENV_FUNCTION b8           os_file_seek(File_Handle* handle, File_Seek_Basis basis, u64 offset, File_Error* errout);
ENV_FUNCTION u64          os_file_tell(File_Handle* handle, File_Error* errout);
ENV_FUNCTION b8           os_file_read(File_Handle* handle, Data64* dest, u64 read_size, File_Error* errout);
ENV_FUNCTION u64          os_file_write(File_Handle* handle, u8* data, u64 size, File_Error* errout);
ENV_FUNCTION b8           os_set_pwd(String_Utf32 path);
ENV_FUNCTION String_Utf32 os_get_pwd(Memory_Arena* storage);
// TODO: Directory Iteration

// OS Agnositc Operations
ENV_FUNCTION Data64
file_read_all(File_Handle* handle, Memory_Arena* storage, File_Error* errout)
{
  Data64 result = ZEROED_STRUCT;
  
  // Get File Size without allocating for get_info
  if (!os_file_seek(handle, FileSeekBasis_End, 0, errout)) return result;
  result.size = os_file_tell(handle, errout);
  if (*errout != FileError_None) return result;
  if (!os_file_seek(handle, FileSeekBasis_Beginning, 0, errout)) return result;
  
  // Allocate Buffer & Read Into It
  result.base = memory_arena_alloc(storage, result.size);
  os_file_read(handle, &result, result.size, errout);
  return result;
}

ENV_FUNCTION u64
file_write_all(File_Handle* handle, Data64 data, File_Error* errout)
{
  // Get File Size without allocating for get_info
  if (!os_file_seek(handle, FileSeekBasis_Beginning, 0, errout)) return 0;
  u64 written = os_file_write(handle, data.base, data.size, errout);
  return written;
}

#if ENV_OS_MAC | ENV_OS_LINUX

global int fseek_translate[] = {
  SEEK_SET, // FileSeekBasis_Beginning
  SEEK_CUR, // FileSeekBasis_Current,
  SEEK_END, // FileSeekBasis_End
};

ENV_FUNCTION b8          
os_file_handle_is_valid(File_Handle handle)
{
  return handle.value != 0;
}

ENV_FUNCTION File_Handle
os_file_open(char* path, File_Access access, File_Create_Mode create, File_Error* errout)
{
  File_Handle result = ZEROED_STRUCT;
  
  char* access_str = 0;
  switch (access) 
  { 
    case FileAccess_Read:             { access_str = (char*)"rb"; } break;
    case FileAccess_Write:            { access_str = (char*)"wb"; } break;
    case FileAccess_ReadWrite:        { invalid_code_path; } break;    
    invalid_default_case;
  }
  assert(access_str != 0);
  
  FILE* fh = fopen(path, (const char*)access_str);
  if (fh != 0) 
  {
    result.value = (u64)fh;
    assert(os_file_handle_is_valid(result));
  }
  else if (errout)
  {
    switch (errno) {
      case EINVAL: { *errout = FileError_Open_InvalidAccessMode; } break;
      default:     { *errout = FileError_Unknown; } break;
    }
  }
  
  return result;
}

ENV_FUNCTION void        
os_file_close(File_Handle* handle, File_Error* errout)
{
  assert(os_file_handle_is_valid(*handle));
  FILE* fh = (FILE*)handle->value;
  if (fclose(fh)) {
    if (errout) {
      switch (errno) {
        case EBADF: { *errout = FileError_Close_FileNotClosable; } break;
        default:    { *errout = FileError_Unknown; } break;
      }
    }
  }
  handle->value = 0;
}

ENV_FUNCTION File_Info   
os_file_get_info(File_Handle* handle, Memory_Arena* storage, File_Error* errout)
{
  assert(os_file_handle_is_valid(*handle));
  File_Info result = ZEROED_STRUCT;
  
  // Get File Size
  if (!os_file_seek(handle, FileSeekBasis_End, 0, errout)) return result;
  result.size = os_file_tell(handle, errout);
  if (result.size == max_u64) return result;
  if (!os_file_seek(handle, FileSeekBasis_Beginning, 0, errout)) return result;
  
  // Get Time stamps  
  
  // Path Aquisition
  
  
  return result;
}

ENV_FUNCTION b8
os_file_seek(File_Handle* handle, File_Seek_Basis basis, u64 offset, File_Error* errout)
{
  assert(os_file_handle_is_valid(*handle));
  b8 result = true;
  FILE* fh = (FILE*)handle->value;
  if (fseek(fh, (int)offset, fseek_translate[basis])) {
    if (errout) {
      switch (errno) {
        case EINVAL: { *errout = FileError_Seek_InvalidBasis; } break;
        case ESPIPE: { *errout = FileError_Seek_FileNotSeekable; } break;
        default:     { *errout = FileError_Unknown; } break;
      }
    }
    result = false;
  }
  return result;
}

ENV_FUNCTION u64         
os_file_tell(File_Handle* handle, File_Error* errout)
{
  assert(os_file_handle_is_valid(*handle));
  FILE* fh = (FILE*)handle->value;
  long int result = ftell(fh);
  if (result == -1L && errout) {
    switch (errno) {
      case EOVERFLOW: { *errout = FileError_Tell_PositionTooLargeForS32; } break;
      case ESPIPE:    { *errout = FileError_Tell_FileNotTellable; } break;
      default:        { *errout = FileError_Unknown; } break;
    }
    result = max_u64;
  }
  return (u64)result;
}

ENV_FUNCTION b8
os_file_read(File_Handle* handle, Data64* dest, u64 read_size, File_Error* errout)
{
  // is this a valid file?
  if (!os_file_handle_is_valid(*handle))
  {
    *errout = FileError_BadFileHandle;
    return false;
  }
  
  // is there enough room?
  if (read_size < dest->size) {
    *errout = FileError_Read_NotEnoughRoom;
    return false;
  }
  
  b8 result = true;
  FILE* fh = (FILE*)handle->value;
  u64 items_read = (u64)fread((void*)dest->base, dest->size, 1, fh);
  if (items_read != 1) 
  {
    s32 err = errno;
    error_printf("fread: %d", err);
    result = false;
  }
  
  return result;
}

ENV_FUNCTION u64         
os_file_write(File_Handle* handle, u8* data, u64 size, File_Error* errout)
{
  // is this a valid file?
  if (!os_file_handle_is_valid(*handle))
  {
    *errout = FileError_BadFileHandle;
    return 0;
  }
  if (size == 0) return 0;
  
  u64 size_written = 0;
  FILE* fh = (FILE*)handle->value;
  u64 items_written = (u64)fwrite((void*)data, size, 1, fh);
  if (items_written == 1)
  {
    size_written = size;
  }
  else
  {
    s32 err = errno;
    error_printf("fwrite: %d", err);
  }
  return size_written;
}

ENV_FUNCTION b8
os_set_pwd(String_Utf32 path_utf32)
{
  char path[PATH_MAX + 1];
  string_utf32_to_ascii_buffer(path_utf32, (char*)path, PATH_MAX + 1);
  if (chdir(path) == -1) {
    int err = errno;
    error_printf("chdir: %d\n", err);
    return false;
  }
  return true;
}

ENV_FUNCTION String_Utf32 
os_get_pwd(Memory_Arena* storage)
{
  String_Utf32 result = ZEROED_STRUCT;
  result.len = PATH_MAX;
  result.base = memory_arena_alloc_array(storage, u32, result.len);
  
  if (getcwd((char*)result.base, (size_t)result.len) != 0)
  {
    cstr_to_utf32_in_place((char*)result.base, result.len);
  }
  else
  {
    int err = errno;
    error_printf("getcwd: %d\n", err);
  }
  
  return result;
}

#elif ENV_OS_WINDOWS

ENV_FUNCTION b8          
os_file_handle_is_valid(File_Handle handle)
{
  return handle.value != 0 && (HANDLE)handle.value != INVALID_HANDLE_VALUE;
}

ENV_FUNCTION File_Handle
os_file_open(char* path, File_Access access, File_Create_Mode create, File_Error* errout)
{
  File_Handle result = ZEROED_STRUCT;
  
  DWORD flags_access = 0;
  switch (access) 
  { 
    case FileAccess_Read:      { flag_add(flags_access, GENERIC_READ);  } break;
    case FileAccess_Write:     { flag_add(flags_access, GENERIC_WRITE); } break;
    case FileAccess_ReadWrite: { 
      flag_add(flags_access, GENERIC_READ);
      flag_add(flags_access, GENERIC_WRITE);
    } break;    
    invalid_default_case;
  }
  
  DWORD flags_create = 0;
  switch (create)
  {
    case FileCreate_Default:         { flag_add(flags_create, OPEN_ALWAYS);   } break;
    case FileCreate_AlwaysOverwrite: { flag_add(flags_create, CREATE_ALWAYS); } break;
    case FileCreate_NewOnly:         { flag_add(flags_create, CREATE_NEW);    } break;
    case FileCreate_OpenOrCreate:    { flag_add(flags_create, OPEN_ALWAYS);   } break;
    case FileCreate_OpenOnly:        { flag_add(flags_create, OPEN_EXISTING); } break;
    case FileCreate_OpenAndClear:    { flag_add(flags_create, TRUNCATE_EXISTING); } break;
    invalid_default_case;
  }
  
  DWORD share_mode = 0;
  LPSECURITY_ATTRIBUTES sec_attr = NULL;
  HANDLE tf = NULL;
  HANDLE fh = CreateFileA(path, flags_access, share_mode, sec_attr, flags_create, FILE_ATTRIBUTE_NORMAL, tf);
  if (fh != INVALID_HANDLE_VALUE)
  {
    result.value = (u64)HANDLE;
  }
  else
  {
    
  }
  
  return result;
}

ENV_FUNCTION void        
os_file_close(File_Handle* handle, File_Error* errout)
{
  
}

ENV_FUNCTION File_Info   
os_file_get_info(File_Handle* handle, Memory_Arena* storage, File_Error* errout)
{
  
}

ENV_FUNCTION b8
os_file_seek(File_Handle* handle, File_Seek_Basis basis, u64 offset, File_Error* errout)
{
  
}

ENV_FUNCTION u64         
os_file_tell(File_Handle* handle, File_Error* errout)
{
  
}

ENV_FUNCTION b8
os_file_read(File_Handle* handle, Data64* dest, u64 read_size, File_Error* errout)
{
  
}

ENV_FUNCTION u64         
os_file_write(File_Handle* handle, u8* data, u64 size, File_Error* errout)
{
  
}

ENV_FUNCTION b8
os_set_pwd(String_Utf32 path_utf32)
{
  
}

ENV_FUNCTION String_Utf32 
os_get_pwd(Memory_Arena* storage)
{
  
}

#elif ENV_OS_ANDROID || ENV_OS_WASM
# error Memory interface for Android and WASM not implemented yet
#endif

////////////////////////////////////////////////////////
// THREADS

typedef struct Thread_Handle { u64 value; } Thread_Handle;

typedef u32 Thread_Result;
enum {
  ThreadResult_OK = 0,
  ThreadResult_Count,
};

typedef u32 Thread_Status;
enum {
  ThreadStatus_Initializing,
  ThreadStatus_Running,
  ThreadStatus_Complete,
  ThreadStatus_Error,
  ThreadStatus_Count,
};

typedef struct Thread Thread;

typedef Thread_Result Thread_Proc(Thread* thread);

typedef struct Thread_Desc Thread_Desc;
struct Thread_Desc
{
  Thread_Proc*  proc;
  Allocator*    memory;
  Data64        data;
};

struct Thread
{
  u32 id;
  u64 os_handle;
  Thread_Desc desc;
  Thread_Status status;
};

ENV_FUNCTION Thread_Handle os_thread_begin(Thread_Desc desc, Thread** thread_out);
ENV_FUNCTION void          os_thread_end(Thread_Handle handle);
ENV_FUNCTION Thread_Status os_thread_status(Thread_Handle handle);
ENV_FUNCTION u32           os_thread_get_id();

// Atomics
ENV_FUNCTION u32 os_interlocked_increment_u32(volatile u32* value);
ENV_FUNCTION s32 os_interlocked_increment_s32(volatile s32* value);
ENV_FUNCTION u32 os_interlocked_compare_exchange_u32(volatile u32* value);
ENV_FUNCTION s32 os_interlocked_compare_exchange_s32(volatile s32* value);

// Thread Tracking Structures

#define ENV_MAX_THREADS 8
global Thread env_threads_[ENV_MAX_THREADS];

ENV_FUNCTION Thread*
env_thread_handle_to_thread(Thread_Handle th)
{
  assert(th.value < ENV_MAX_THREADS + 1);
  assert(th.value != 0);
  return env_threads_ + (th.value - 1);
}

ENV_FUNCTION Thread_Handle
env_threads_find_empty(Thread** thread)
{
  Thread_Handle result = ZEROED_STRUCT;
  for (u32 i = 0; i < ENV_MAX_THREADS; i++) {
    if (env_threads_[i].os_handle == 0) {
      result.value = i + 1;
      break;
    }
  }
  assert(result.value != 0);
  *thread = env_thread_handle_to_thread(result);
  return result;
}

ENV_FUNCTION Thread_Status 
os_thread_status(Thread_Handle handle)
{
  Thread* thread = env_thread_handle_to_thread(handle);
  return thread->status;
}

#if ENV_OS_MAC

static_assert(sizeof(pthread_t) <= sizeof(u64), "Cannot store pthread_t in u64");

ENV_FUNCTION void*
os_thread_proc_cleanup(void* arg)
{
  Thread* thread = (Thread*)arg;
  zero_struct(thread);
  thread->status = ThreadStatus_Complete;
  return 0;
}

ENV_FUNCTION void* 
env_os_thread_proc_wrapper_(void* arg)
{
  Thread* thread = (Thread*)arg;
  thread->status = ThreadStatus_Running;
  
  pthread_cleanup_push((void(*)(void*))os_thread_proc_cleanup, arg);
  thread->desc.proc(thread);
  pthread_cleanup_pop(true);
  
  pthread_exit(0);
}

ENV_FUNCTION Thread_Handle 
os_thread_begin(Thread_Desc desc, Thread** thread_out)
{
  Thread* thread = 0;
  Thread_Handle result = env_threads_find_empty(&thread);
  thread->desc = desc;
  thread->status = ThreadStatus_Initializing;
  
  pthread_t th;
  s32 create_err = pthread_create(&th, NULL, (void * _Nullable (* _Nonnull)(void * _Nullable))env_os_thread_proc_wrapper_, (u8*)thread);
  if (!create_err) 
  {
    thread->os_handle = (u64)th;
  } 
  else 
  {
    switch (create_err) {
      case EAGAIN: {
      } break;
      
      case EINVAL: {
      } break;
      
      //      case ELEMULTITHREADFORK: {
      //      } break;
      
      case ENOMEM: {
      } break;
    }
  }
  
  if (thread_out != 0) { *thread_out = thread; }
  return result;
}

ENV_FUNCTION void
os_thread_end(Thread_Handle handle)
{
  Thread* thread = env_thread_handle_to_thread(handle);
  s32 err = pthread_kill((pthread_t)thread->os_handle, 1);
  if (err) 
  {
    error_printf("pthread_kill: %d", err);
  }
}

#if 0
// Atomics
ENV_FUNCTION u32 os_interlocked_increment_u32(volatile u32* value);
ENV_FUNCTION s32 os_interlocked_increment_s32(volatile s32* value);
ENV_FUNCTION u32 os_interlocked_compare_exchange_u32(volatile u32* value);
ENV_FUNCTION s32 os_interlocked_compare_exchange_s32(volatile s32* value);
#endif

#else
#  error "Thread/Atomics not supported for this platform yet."
#endif