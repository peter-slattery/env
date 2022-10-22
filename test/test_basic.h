#include "utest.h"

#define ENV_DEBUG 1
#define ENV_DO_SYSTEM_INCLUDES 1
#include "../src/env.h"

static_assert(1, "foo");
//static_assert(0, "bar");

#define test(expr, ...) if (!(expr)) { printf("Test Failed\n%s:%d\n", (char*)__FILE__, __LINE__); printf(__VA_ARGS__); }

UTEST(basic, assert) {
  // Assert Testing
  assert(true);
  //assert(false);
  
  // Debug Utils Testing
  Debug_Location test_debug_loc = DEBUG_LOC;
  EXPECT_NE(test_debug_loc.file, (char*)0);
  EXPECT_NE(test_debug_loc.line, 0);
}

UTEST(basic, flags) {
  u32 flag = 0x4;
  u32 value = 0;
  
  // Value is zero, flag is missing
  EXPECT_FALSE(flag_present(value, flag));
  EXPECT_TRUE(flag_missing(value, flag));
  
  // value is not zero, but also is not flag, flag is missing
  value = 3;
  EXPECT_FALSE(flag_present(value, flag));
  EXPECT_TRUE(flag_missing(value, flag));
  
  // flag_add, flag is present
  flag_add(value, flag);
  EXPECT_TRUE(flag_present(value, flag));
  EXPECT_FALSE(flag_missing(value, flag));
  
  // flag_rem, flag is missing
  flag_rem(value, flag);
  EXPECT_FALSE(flag_present(value, flag));
  EXPECT_TRUE(flag_missing(value, flag));
  
  // flag_toggle  
  flag_toggle(value, flag);
  EXPECT_TRUE(flag_present(value, flag));
  flag_toggle(value, flag);
  EXPECT_FALSE(flag_present(value, flag));
}

typedef struct Test_Sll Test_Sll;
struct Test_Sll
{
  Test_Sll* next;
};

UTEST(basic, sll)
{
  Test_Sll* first = 0;
  Test_Sll* last = 0;
  
  // SLL Init
  // Order After: e0
  Test_Sll* e0 = (Test_Sll*)malloc(sizeof(Test_Sll));
  sll_init(first, last, e0);
  EXPECT_EQ(first, e0);
  EXPECT_EQ(last,  e0);
  EXPECT_EQ(last->next, (Test_Sll*)0);
  
  // SLL Prepend
  // Order After: e1, e0
  Test_Sll* e1 = (Test_Sll*)malloc(sizeof(Test_Sll));
  sll_prepend(first, last, e1);
  EXPECT_EQ(first,    e1);
  EXPECT_EQ(e1->next, e0);
  EXPECT_EQ(last,     e0);
  
  // SLL Append
  // Order After: e1, e0, e2
  Test_Sll* e2 = (Test_Sll*)malloc(sizeof(Test_Sll));
  sll_append(first, last, e2);
  EXPECT_EQ(first, e1);
  EXPECT_EQ(last, e2);
  EXPECT_EQ(e0->next, e2);
  EXPECT_EQ(e2->next, (Test_Sll*)0);
  
  // SLL Insert
  // Order After: e1, e0, e3, e2
  Test_Sll* e3 = (Test_Sll*)malloc(sizeof(Test_Sll));
  sll_insert(e0, e2, e3);
  EXPECT_EQ(e0->next, e3);
  EXPECT_EQ(e3->next, e2);
  
  // Order Checking
  Test_Sll* e_at = first; EXPECT_EQ(e_at, e1);
  e_at = e_at->next; EXPECT_EQ(e_at, e0);
  e_at = e_at->next; EXPECT_EQ(e_at, e3);
  e_at = e_at->next; EXPECT_EQ(e_at, e2);
  EXPECT_EQ(e_at->next, (Test_Sll*)0);
  
  Test_Sll* next = 0;
  for (Test_Sll* at = first; at != 0;) {
    next = at->next;
    free((void*)at);
    at = next;
  }
}

UTEST(basic, math)
{
  // Min and MAx
  EXPECT_EQ(max_val(5, 2), 5);
  EXPECT_EQ(max_val(2, 5), 5);
  EXPECT_EQ(min_val(5, 2), 2);
  EXPECT_EQ(min_val(2, 5), 2);
  
  // Clamp
  EXPECT_EQ(clamp_val(0, 3, 5), 3);
  EXPECT_EQ(clamp_val(0, -2, 5), 0);
  EXPECT_EQ(clamp_val(0, 7, 5), 5);
  // NOTE(PS): Technically this is invalid, but I want to 
  // codify that this is the behavior
  EXPECT_EQ(clamp_val(5, 3, 2), 5);
  EXPECT_EQ(clamp_val(5, 7, 2), 5);
  
  // Abs & Sign
  EXPECT_EQ(abs_val(5), 5);
  EXPECT_EQ(abs_val(-5), 5);
  EXPECT_EQ(sign_val(5), 1);
  EXPECT_EQ(sign_val(-5), -1);
  
  // Is Pow 2 and IsOdd
  EXPECT_TRUE(is_pow2(128));
  EXPECT_FALSE(is_pow2(129));
  EXPECT_TRUE(is_odd(3));
  EXPECT_FALSE(is_odd(4));
  
  // Lerp
  EXPECT_EQ(lerp_r32(0, .5f, 5), 2.5f);
  EXPECT_EQ(lerp_r32(0, 1.5f, 5), 7.5f);
  EXPECT_EQ(lerp_r32(0, -.5f, 5), -2.5f);
  EXPECT_EQ(lerp_r64(0, .5f, 5), 2.5f);
  EXPECT_EQ(lerp_r64(0, 1.5f, 5), 7.5f);
  EXPECT_EQ(lerp_r64(0, -.5f, 5), -2.5f);
  
  // Remap
  EXPECT_EQ(remap_r32( 0.5f, 0, 1, 2, 3), 2.5f);
  EXPECT_EQ(remap_r32( 1.5f, 0, 1, 2, 3), 3.5f);
  EXPECT_EQ(remap_r32(-0.5f, 0, 1, 2, 3), 1.5f);
  EXPECT_EQ(remap_r32( 0.5f, 0, 1, 2, 0), 1.0f);
  EXPECT_EQ(remap_r64( 0.5f, 0, 1, 2, 3), 2.5f);
  EXPECT_EQ(remap_r64( 1.5f, 0, 1, 2, 3), 3.5f);
  EXPECT_EQ(remap_r64(-0.5f, 0, 1, 2, 3), 1.5f);
  EXPECT_EQ(remap_r64( 0.5f, 0, 1, 2, 0), 1.0f);
  
  // Round Up TO
  EXPECT_EQ(round_to_u32(22, 8), (u32)24);
  EXPECT_EQ(round_to_u32(19, 7), (u32)21);
  EXPECT_EQ(round_to_u64(22, 8), (u64)24);
  EXPECT_EQ(round_to_u64(19, 7), (u64)21);
  
  // Round Up TO Pow 2
  EXPECT_EQ(round_up_pow2_u32(31), (u32)32);
  EXPECT_EQ(round_up_pow2_u32(32), (u32)32);
  EXPECT_EQ(round_up_pow2_u64(31), (u64)32);
  EXPECT_EQ(round_up_pow2_u64(32), (u64)32);
}

UTEST(basic, hash)
{
  char* str = (char*)"Hello world, this is a test\n";
  u64   str_len = cstr_len(str);
  EXPECT_EQ(str_len, 28);
  u64 str_hash = hash_u64_djb2(str, str_len);
  EXPECT_NE(str_hash, 0);
  EXPECT_EQ(str_hash, 113246580103267532); // ensure hash value stays consistent
}

UTEST(basic, random)
{
  u32 init_value = 3559;
  Random_Series rs = random_series_init(init_value);
  u32 a = random_series_next(&rs);
  EXPECT_NE(a, init_value);
  u32 b = random_series_next(&rs);
  EXPECT_NE(a, b);
  
  r32 c = random_series_next_unilateral(&rs);
  EXPECT_GE(c, 0);
  EXPECT_LE(c, 1);
  
  r32 d = random_series_next_bilateral(&rs);
  EXPECT_GE(d, -1);
  EXPECT_LE(d,  1);
}

UTEST(memory, cursor)
{
  Memory_Cursor c = ZEROED_STRUCT;
  c.base = (u8*)malloc(128);
  c.cap = 128;
  c.at = 0;
  
  // Writing
  u8 w_u8 = 135;
  u8* v = memory_cursor_write_struct(&c, w_u8);
  EXPECT_NE(v, (u8*)0);
  
  u16 w_u16 = 3349;
  memory_cursor_write_struct(&c, w_u16);
  
  u32 w_u32 = 3349999;
  memory_cursor_write_struct(&c, w_u32);
  
  u64 w_u64 = 334912399;
  memory_cursor_write_struct(&c, w_u64);
  
  u32 w_array[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  memory_cursor_write_array(&c, w_array, 10);
  
  // Reading
  c.at = 0;
  u8 r_u8 = memory_cursor_read_u8(&c);
  EXPECT_EQ(r_u8, w_u8);
  
  u16 r_u16 = memory_cursor_read_u16(&c);
  EXPECT_EQ(r_u16, w_u16);
  
  u32 r_u32 = memory_cursor_read_u32(&c);
  EXPECT_EQ(r_u32, w_u32);
  
  u64 r_u64 = memory_cursor_read_u64(&c);
  EXPECT_EQ(r_u64, w_u64);
  
  u32 r_array[10];
  memory_cursor_read_array(&c, r_array, 10);
  for (u32 i = 0; i < 10; i++) {
    EXPECT_EQ(r_array[i], w_array[i]);
  }
}

UTEST(memory, push_buffer_arena)
{
  Allocator a = ZEROED_STRUCT;
  Memory_Arena ma = push_buffer_arena_create(&a, 4, 256, (char*)"test arena", 8);
  Push_Buffer_Arena* pb = (Push_Buffer_Arena*)ma.arena_data;
  
  u8* a0 = memory_arena_alloc_array(&ma, u8, 250);
  for (u32 i = 0; i < 32; i++) a0[i] = i;
  
  // shouldn't fit on the first page, force the pb to alloc the next page
  u8* a1 = memory_arena_alloc_array(&ma, u8, 10); 
  EXPECT_GT(pb->buffers_len, 1);
}

UTEST(memory, hashtable)
{
  Allocator a = ZEROED_STRUCT;
  Memory_Arena ma = push_buffer_arena_create(&a, 1, kb(16), (char*)"hashtable arena", 8);
  
  Hashtable ht = ZEROED_STRUCT;
  ht.cap = 1024;
  ht.keys = memory_arena_alloc_array(&ma, u32, ht.cap);
  ht.values = memory_arena_alloc_array(&ma, u8*, ht.cap);
  ht.used = 0;
  
  hashtable_add(&ht, 256, (u8*)1);
  hashtable_add(&ht, 394, (u8*)2);
  hashtable_add(&ht, 81932, (u8*)3);
  
  // this should force chaining
  hashtable_add(&ht, ht.cap + 256, (u8*)4);
  
  u64 v0 = (u64)hashtable_get(&ht, 256);
  EXPECT_EQ(v0, 1);
  u64 v1 = (u64)hashtable_get(&ht, 394);
  EXPECT_EQ(v1, 2);
  u64 v2 = (u64)hashtable_get(&ht, 81932);
  EXPECT_EQ(v2, 3);
  u64 v3 = (u64)hashtable_get(&ht, ht.cap + 256);
  EXPECT_EQ(v3, 4);
  
  // getting a value that's not present
  u64 vi = (u64)hashtable_get(&ht, 3333);
  EXPECT_EQ(vi, 0);
  
  b8 r0 = hashtable_rem(&ht, 256);
  EXPECT_NE(r0, 0);
  v0 = (u64)hashtable_get(&ht, 256);
  EXPECT_EQ(v0, 0);
}

UTEST(memory, stretchy_buffer)
{
  Allocator a = ZEROED_STRUCT;
  
  // Initialization
  u32* sb = sb_init(1, u32, Sb_Defaults);
  EXPECT_TRUE(sb_is_valid(sb));
  EXPECT_EQ(sb_cap(sb), 1);
  EXPECT_EQ(sb_len(sb), 0);
  
  // Append, No Grow
  sb_append(sb, u32, 99);
  EXPECT_EQ(sb[0], 99);
  EXPECT_EQ(sb_cap(sb), 1);
  EXPECT_EQ(sb_len(sb), 1);
  
  // Append, Grow
  sb_append(sb, u32, 201);
  EXPECT_EQ(sb[0], 99);
  EXPECT_EQ(sb[1], 201);
  EXPECT_EQ(sb_cap(sb), 2);
  EXPECT_EQ(sb_len(sb), 2);
  
  // Append, Grow, ensure pow2 minimum growth
  sb_append(sb, u32, 333);
  EXPECT_EQ(sb[0], 99);
  EXPECT_EQ(sb[1], 201);
  EXPECT_EQ(sb[2], 333);
  EXPECT_EQ(sb_cap(sb), 4);
  EXPECT_EQ(sb_len(sb), 3);
  
  // Append, No Grow
  sb_append(sb, u32, 41);
  EXPECT_EQ(sb[0], 99);
  EXPECT_EQ(sb[1], 201);
  EXPECT_EQ(sb[2], 333);
  EXPECT_EQ(sb[3], 41);
  EXPECT_EQ(sb_cap(sb), 4);
  EXPECT_EQ(sb_len(sb), 4);
}

UTEST(sort, radix_u32)
{
  Allocator a = ZEROED_STRUCT;
  Memory_Arena arena = push_buffer_arena_create(&a, 1, kb(4), "arena", 8);
  
  u32 array[] = { 5, 999, 33, 0XFFFFFFFF, 21, 6, 0xF000000, 5, 0 };
  u32 array_cap = sizeof(array) / sizeof(array[0]);
  Radix_Sort_Result_U32 sorted = radix_sort_u32(array, array_cap, &arena);
  for (u32 i = 0; i < array_cap - 1; i++)
  {
    EXPECT_LE(sorted.entries[i].value, sorted.entries[i+1].value);
  }
  
  memory_arena_clear(&arena);
}

UTEST(sort, radix_u64)
{
  Allocator a = ZEROED_STRUCT;
  Memory_Arena arena = push_buffer_arena_create(&a, 1, kb(4), "arena", 8);
  
  u64 array[] = { 5, 999, 33, 0XFFFFFFFFFFFFFFFF, 21, 0xF000000000000000, 6, 5, 0 };
  u64 array_cap = sizeof(array) / sizeof(array[0]);
  Radix_Sort_Result_U64 sorted = radix_sort_u64(array, array_cap, &arena);
  for (u32 i = 0; i < array_cap - 1; i++)
  {
    EXPECT_LE(sorted.entries[i].value, sorted.entries[i+1].value);
  }
  
  memory_arena_clear(&arena);
}

UTEST(os, time)
{
  Ticks start = os_ticks_now();
  
  Seconds sleep_time;
  sleep_time.v = 1;
  os_sleep(sleep_time);
  
  Ticks end = os_ticks_now();
  Ticks   delta_ticks = ticks_elapsed(start, end);
  EXPECT_GE(delta_ticks.v, 0);
  Seconds delta_secs  = os_ticks_to_seconds(delta_ticks);
  EXPECT_GE(delta_secs.v, 0);
}

#if 0
you need to make some basic architectural decisions
- we should probably convert over utf8 as our default string
since snprintf etc all use that
- it means that string length gets complicated but it'll be fine
#endif

UTEST(basic, string)
{
  Allocator a = ZEROED_STRUCT;
  Memory_Arena pb = push_buffer_arena_create(&a, 32, kb(4), (char*)"string arena", 8);
  
  char* ascii = (char*)"Hello!\n";
  String_Utf32 ascii_utf32 = cstring_to_utf32_arena(ascii, &pb);
  EXPECT_EQ(ascii_utf32.len, 7);
  
  String_Utf8 utf32_to_utf8 = string_utf32_to_utf8_arena(ascii_utf32, &pb);
  EXPECT_EQ(utf32_to_utf8.len, 7);
  EXPECT_EQ(utf32_to_utf8.data_len, 7);
  
  String_Utf32 utf8_to_utf32 = string_utf8_to_utf32_arena(utf32_to_utf8, &pb);
  EXPECT_EQ(utf8_to_utf32.len, 7);
  
  char* cstr_ori = "Hello World!";
  char* cstr_mod = "Hello World!                                                   ";
  u64   cstr_mod_len = 12;
  u64   cstr_mod_len_actual = cstr_len(cstr_mod);
  u32* u = (u32*)cstr_mod;
  u[0] = 5;
  cstr_to_utf32_in_place(cstr_mod, cstr_mod_len);
#if 0
  for (u32 i = 0; i < cstrlen; i++)
  {
    EXPECT_EQ(cstr_ori[i], cstr_mod[i + 0]);
    EXPECT_EQ(0,           cstr_mod[i + 1]);
    EXPECT_EQ(0,           cstr_mod[i + 2]);
    EXPECT_EQ(0,           cstr_mod[i + 3]);
  }
#endif
  
  String_Utf32 exe_path_utf32 = os_get_exe_path(&pb);
  String8 exe_path_ascii = string_utf32_to_ascii_arena(exe_path_utf32, &pb);
  //printf("Exe Path: %s\n", (char*)exe_path_ascii.base);
  
  String_Utf32 test_path = cstring_to_utf32_arena((char*)"test/foobar\\file.exe", &pb);
  
  String_Utf32 t_before_last_slash = cstring_to_utf32_arena((char*)"test/foobar", &pb);
  String_Utf32 t_after_last_slash = cstring_to_utf32_arena((char*)"file.exe", &pb);
  String_Utf32 t_after_last_dot = cstring_to_utf32_arena((char*)"exe", &pb);
  
  String_Utf32 before_last_slash = string_chop_last_slash(test_path);
  EXPECT_TRUE(string_match(before_last_slash, t_before_last_slash, StringMatch_SlashInsensitive));
  
  String_Utf32 after_last_slash = string_skip_last_slash(test_path);
  EXPECT_TRUE(string_match(after_last_slash, t_after_last_slash, StringMatch_SlashInsensitive));
  
  String_Utf32 after_last_dot = string_skip_last_period(test_path);
  EXPECT_TRUE(string_match(after_last_dot, t_after_last_dot, StringMatch_SlashInsensitive));
}

UTEST(os, file)
{
  Allocator a = ZEROED_STRUCT;
  Memory_Arena pb = push_buffer_arena_create(&a, 32, kb(4), (char*)"string arena", 8);
  
  String_Utf32 pwd = os_get_pwd(&pb);
  EXPECT_NE(pwd.len, 0);
  EXPECT_NE(pwd.base, (u32*)0);
  
  String_Utf32 exe_path = os_get_exe_path(&pb);
  EXPECT_NE(exe_path.len, 0);
  EXPECT_NE(exe_path.base, (u32*)0);
  String_Utf8 exe_path8 = string_utf32_to_utf8_arena(exe_path, &pb);
  printf("Exe Path: %.*s\n", (s32)exe_path8.len, (char*)exe_path8.base);
  
  File_Handle fh = ZEROED_STRUCT;
  EXPECT_FALSE(os_file_handle_is_valid(fh));
}

Thread_Result
thread_proc(Thread* t)
{
  Seconds s = { 1 };
  os_sleep(s);
  return ThreadResult_OK;
}

UTEST(os, thread)
{
  Thread_Desc td = {
    .proc = thread_proc,
  };
  
  Thread_Handle th = os_thread_begin(td, 0);
  Thread_Status ran = ThreadStatus_Initializing;
  for (; ran != ThreadStatus_Running && ran != ThreadStatus_Complete; ran = os_thread_status(th))
  {
    Seconds s = { 0.1f };
    os_sleep(s);
  }
  Thread_Status completed = ThreadStatus_Initializing;
  for (; ran != ThreadStatus_Complete; ran = os_thread_status(th))
  {
    Seconds s = { 0.1f };
    os_sleep(s);
  }
  EXPECT_EQ(ran, ThreadStatus_Running);
  EXPECT_EQ(completed, ThreadStatus_Complete);
}