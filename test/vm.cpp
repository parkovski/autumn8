#include "a8/vm/internal/program.h"

#include <string>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("String map", "[vm]") {
  a8_vm_string_map smap;
  REQUIRE(smap.size() == 1);
  REQUIRE(smap._strings[0]->data()[0] == 0);
  REQUIRE(smap._stringsIndex[""] == 0);

  const char *s_123 = "one,two,three";
  const char *s_one = "one", *s_two = "two", *s_three = "three";
  a8_vm_strid id_123, id_one, id_two, id_three;

  const char *s_456 = "four,five,six";
  const char *s_4567 = "four,five,six,seven";
  const char *s_four = "four", *s_five = "five", *s_six = "six";
  a8_vm_strid id_456, id_four, id_five, id_six;

  std::string_view sv_123{s_123}, sv_one{s_one}, sv_two{s_two}, sv_three{s_three};
  std::string_view sv_456{s_456}, sv_four{s_four}, sv_five{s_five}, sv_six{s_six};

  // Null terminated
  {
    // Insert using memory that will be freed when we look it up.
    std::string str_123;
    str_123 += s_one;
    str_123 += ",";
    str_123 += s_two;
    str_123 += ",";
    str_123 += s_three;
    id_123 = a8_vm_string_map_get_id(&smap, str_123.c_str(), 0);
    id_one = a8_vm_string_map_get_id(&smap, str_123.substr(0, 3).c_str(), 0);
    id_two = a8_vm_string_map_get_id(&smap, str_123.substr(4, 3).c_str(), 0);
    id_three = a8_vm_string_map_get_id(&smap, str_123.substr(8, 5).c_str(), 0);

    REQUIRE(smap.size() == 5);
  }

  // Non null terminated
  {
    id_456 = a8_vm_string_map_get_id(&smap, s_4567, 13);
    id_four = a8_vm_string_map_get_id(&smap, s_4567, 4);
    id_five = a8_vm_string_map_get_id(&smap, s_4567 + 5, 4);
    id_six = a8_vm_string_map_get_id(&smap, s_4567 + 10, 3);

    auto vs_456 = a8_vm_string_map_get_string(&smap, s_4567, 13);
    REQUIRE(vs_456.len == 13);
    REQUIRE(vs_456.str[13] == 0);
    REQUIRE(s_4567 != vs_456.str);
    REQUIRE(smap.size() == 9);
  }

  // Lookup by ID.
  {
    auto vs_123 = a8_vm_string_map_get_string_at(&smap, id_123);
    auto vs_one = a8_vm_string_map_get_string_at(&smap, id_one);
    auto vs_two = a8_vm_string_map_get_string_at(&smap, id_two);
    auto vs_three = a8_vm_string_map_get_string_at(&smap, id_three);
    auto vs_456 = a8_vm_string_map_get_string_at(&smap, id_456);
    auto vs_four = a8_vm_string_map_get_string_at(&smap, id_four);
    auto vs_five = a8_vm_string_map_get_string_at(&smap, id_five);
    auto vs_six = a8_vm_string_map_get_string_at(&smap, id_six);

    // Construct by specified length.
    REQUIRE(std::string_view{vs_123.str, vs_123.len} == sv_123);
    REQUIRE(std::string_view{vs_one.str, vs_one.len} == sv_one);
    REQUIRE(std::string_view{vs_two.str, vs_two.len} == sv_two);
    REQUIRE(std::string_view{vs_three.str, vs_three.len} == sv_three);
    REQUIRE(std::string_view{vs_456.str, vs_456.len} == sv_456);
    REQUIRE(std::string_view{vs_four.str, vs_four.len} == sv_four);
    REQUIRE(std::string_view{vs_five.str, vs_five.len} == sv_five);
    REQUIRE(std::string_view{vs_six.str, vs_six.len} == sv_six);
  }

  // Lookup by string.
  {
    auto vs_123 = a8_vm_string_map_get_string(&smap, s_123, 0);
    auto vs_one = a8_vm_string_map_get_string(&smap, s_123, 3);
    auto vs_two = a8_vm_string_map_get_string(&smap, s_123 + 4, 3);
    auto vs_three = a8_vm_string_map_get_string(&smap, s_123 + 8, 5);
    auto vs_456 = a8_vm_string_map_get_string(&smap, s_456, 0);
    auto vs_four = a8_vm_string_map_get_string(&smap, s_456, 4);
    auto vs_five = a8_vm_string_map_get_string(&smap, s_456 + 5, 4);
    auto vs_six = a8_vm_string_map_get_string(&smap, s_456 + 10, 3);

    REQUIRE(s_123 != vs_123.str);

    // Construct by null termination.
    REQUIRE(std::string_view{vs_123.str} == sv_123);
    REQUIRE(std::string_view{vs_one.str} == sv_one);
    REQUIRE(std::string_view{vs_two.str} == sv_two);
    REQUIRE(std::string_view{vs_three.str} == sv_three);
    REQUIRE(std::string_view{vs_456.str} == sv_456);
    REQUIRE(std::string_view{vs_four.str} == sv_four);
    REQUIRE(std::string_view{vs_five.str} == sv_five);
    REQUIRE(std::string_view{vs_six.str} == sv_six);
  }
}

TEST_CASE("Stack", "[vm]") {
  a8_vm_stack stack{0x100};
  stack.reserve(13);
  memcpy(stack.pointer(), "Hello world!", 13);
  auto ofs_hello_world = stack.offset();
  stack.pushoff();
  REQUIRE((stack.offset() & 0x3) == 0);
  // The types are mismatched on purpose - the stack should not enforce strict
  // aliasing.
  stack.reserve<float>();
  *stack.pointer<uint32_t>() = 0x3F800000 /* 1.f */;
  stack.reserve<int32_t>();
  *stack.pointer<float>() = -2.f;

  REQUIRE(*stack.pointer<uint32_t>() == 0xC0000000);

  stack.popoff(2 * sizeof(uint32_t));
  REQUIRE(stack.offset() == ofs_hello_world);
  REQUIRE(stack.pointer<char>() == std::string_view{"Hello world!"});

  // Underflow
  auto offset = stack.offset();
  REQUIRE_FALSE(stack.reserve(0x100));
  REQUIRE(stack.offset() == offset);

  REQUIRE_FALSE(stack.release(0x100));
  REQUIRE(stack.offset() == 0x100);
}

TEST_CASE("Symbol table", "[vm]") {
  a8_vm_symbol_table symtab;
}

TEST_CASE("Simple VM program", "[vm]") {
}
