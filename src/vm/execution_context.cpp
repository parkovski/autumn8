#include "a8/vm/execution_context.h"
#include "a8/vm/data.h"
#include "a8/vm/internal/symbol_table.h"
#include "a8/vm/internal/string_map.h"
#include "a8/vm/internal/ibuf.h"

//using namespace a8::vm;

using name_id_pair = const std::pair<const char *const, const a8_vm_symid>;

#define PAIR2(name, id) {#name, a8_vm_symid_##id}
#define PAIR1(name) PAIR2(name, name)
#define for_each_pair(p, pairs) \
  for (auto *p = pairs; p != pairs + sizeof(pairs) / sizeof(pairs[0]); ++p)

static name_id_pair s_root_names[] = {
  PAIR1(meta),
  PAIR1(data),
  PAIR1(type),
  PAIR1(exec),
  PAIR1(env),
};

static name_id_pair s_meta_names[] = {
  PAIR1(undef),
  PAIR1(empty),
  PAIR1(unknown),
  PAIR1(never),
  PAIR1(symid),
  PAIR1(strid),
  PAIR1(symbol),
  PAIR2(entry, meta_entry),
  PAIR2(object, meta_object),
  PAIR2(ctor, meta_ctor),
  PAIR2(dtor, meta_dtor),
};

static name_id_pair s_data_names[] = {
  PAIR1(bit),
  PAIR1(byte),
  PAIR1(word),
  PAIR1(string),
  PAIR1(address),
  PAIR1(bool),
  PAIR1(true),
  PAIR1(false),
  PAIR1(num),
  PAIR1(int),
  PAIR1(sign2c),
  PAIR1(lossy),
  PAIR1(ieee_float),
};

static name_id_pair s_exec_names[] = {
  PAIR1(text),
  PAIR1(abi),
  PAIR1(function),
  PAIR1(memo),
};

static name_id_pair s_env_names[] = {
  PAIR2(host, env_host),
  PAIR2(meta, env_meta),
  PAIR2(control, env_control),
};

#define INIT_SCOPE(name) \
  for_each_pair(_p, s_##name##_names) \
    symtab.set_child(a8_vm_symid_##name, smap->get(_p->first), _p->second)

void a8_vm_execution_context_symbol_table_init(a8_vm_execution_context *ctx) {
  auto &symtab = *ctx->symtab;
  auto *smap = new a8_vm_string_map;

  // empty
  auto *s_empty = symtab.add<a8_vm_data>(a8_vm_symid_data).second->get();
  s_empty->size = 0;
  s_empty->flags = a8_vm_data_flag_const_size;

  // unknown
  symtab.add(a8_vm_symid_empty, 0);

  // never
  symtab.add(a8_vm_symid_empty, 0);

  symtab.add(a8_vm_symid_undef, 0);
  INIT_SCOPE(root);

  symtab.add(a8_vm_symid_undef, 0);
  INIT_SCOPE(meta);

  symtab.add(a8_vm_symid_undef, 0);
  INIT_SCOPE(data);

  symtab.add(a8_vm_symid_undef, 0);
  INIT_SCOPE(env);
}

void a8_vm_execution_context_run(a8_vm_execution_context *ctx) {
}
