#include "a8/vm/vm.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Simple VM program", "[vm]") {
  auto *pgm = a8_vm_program_new();
  auto *scope = a8_vm_program_get_scope(pgm);
  auto main_name = a8_vm_program_get_name(pgm, "main", 0);
  a8_vm_symref main_parts[] = {
    a8_vm_program_get_stdsym(pgm, a8_vm_stdsym_function_no_params),
    a8_vm_program_get_stdsym(pgm, a8_vm_stdsym_code_page),
    nullptr
  };

  auto s_main = a8_vm_scope_add_child(scope, main_name, main_parts);
  auto cp = (a8_vm_code_page *)a8_vm_symbol_get_value_at(s_main, 1);
  // auto printstr_name = a8_vm_program_get_name(pgm, "printstr", 0);
  // auto s_printstr = a8_vm_scope_get_child(scope, printstr_name);
  // auto s_function = a8_vm_program_get_stdsym(pgm, a8_vm_stdsym_function);
  // auto fn_printstr = a8_vm_symbol_get_value(s_printstr, s_function);
  // a8_vm_symref param_types[] = {
  //   a8_vm_program_get_stdsym(pgm, a8_vm_stdsym_cstr),
  //   nullptr
  // };
  // a8_vm_symref param_values[] = {
  //   "Hello world",
  //   nullptr
  // };
  // a8_vm_write_call(cp, fn_printstr, param_types, param_values);
  // a8_vm_write_return(cp);
  // a8_vm_program_exec(s_main, nullptr, nullptr);
}
