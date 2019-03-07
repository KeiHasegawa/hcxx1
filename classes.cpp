#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

cxx_compiler::tag::kind_t cxx_compiler::classes::specifier::get(int keyword)
{
  switch ( keyword ){
  case CLASS_KW:  return tag::CLASS;
  case STRUCT_KW: return tag::STRUCT;
  case UNION_KW:  return tag::UNION;
  default:        return tag::ENUM;
  }
}

void
cxx_compiler::classes::specifier::begin(int keyword, var* v, std::vector<base*>* bases)
{
  using namespace std;
  usr* u = static_cast<usr*>(v);
  auto_ptr<usr> sweeper(u);
  tag::kind_t kind = get(keyword);
  string name = u ? u->m_name : new_name(".tag");
  const file_t& file = u ? u->m_file : parse::position;
  map<string,tag*>& tags = scope::current->m_tags;
  map<string,tag*>::const_iterator p = tags.find(name);
  if ( p != tags.end() ){
    tag* prev = p->second;
    if ( prev->m_kind != kind ){
      using namespace error::classes;
      redeclaration(parse::position,prev->m_file.back(),name);
      name = new_name(".tag");
    }
    pair<const type*, const type*> types = prev->m_types;
    if ( types.second ){
      using namespace error::classes;
      redeclaration(parse::position,prev->m_file.back(),name);
      name = new_name(".tag");
    }
    prev->m_file.push_back(file);
    scope::current = prev;
  }
  else {
    tag* ptr = new tag(kind,name,file,bases);
    ptr->m_parent = scope::current;
    ptr->m_parent->m_children.push_back(ptr);
    ptr->m_types.first = incomplete_tagged_type::create(ptr);
    tags[name] = ptr;
    scope::current = ptr;
  }
  declarations::specifier_seq::info_t::clear();
}

void
cxx_compiler::classes::specifier::begin2(int keyword, tag* ptr)
{
  using namespace std;
  string name = ptr->m_name;
  usr* tmp = new usr(name,0,usr::NONE,file_t());
  begin(keyword,tmp,0);
}

namespace cxx_compiler { namespace classes { namespace specifier {
  void member_function_definition(std::pair<usr* const,parse::member_function_body::save_t>&);
} } } // end of namespace specifier, classes and cxx_compiler

const cxx_compiler::type* cxx_compiler::classes::specifier::action()
{
  using namespace std;
  tag* ptr = static_cast<tag*>(scope::current);
  const type* ret = record_type::create(ptr);
  ptr->m_types.second = ret;
  map<usr*, parse::member_function_body::save_t>& tbl = parse::member_function_body::table;
  for_each(tbl.begin(),tbl.end(),member_function_definition);
  tbl.clear();
  scope::current = ptr->m_parent;
  class_or_namespace_name::after();
  return ret;
}

void cxx_compiler::classes::specifier::
member_function_definition(std::pair<usr* const,parse::member_function_body::save_t>& E)
{
  using namespace std;
  usr* u = E.first;
  u->m_type = u->m_type->complete_type();
  scope::current = u->m_scope;
  vector<scope*>& children = scope::current->m_children;
  scope* param = E.second.m_param;
  children.push_back(param);
  fundef::current = new fundef(u,param);
  parse::member_function_body::saved = &E.second;
  file_t org = parse::position;
  cxx_compiler_parse();
  parse::position = org;
  parse::member_function_body::saved = 0;
}

namespace cxx_compiler {
  struct constant_member : usr {
    usr* m_constant;
    constant_member(usr* u, usr* c) : usr(*u), m_constant(c) {}
    bool isconstant(bool) const { return true; }
    __int64 value() const { return m_constant->value(); }
  };
} // end of namespace cxx_compiler

void cxx_compiler::classes::members::action(var* v, expressions::base* expr)
{
  using namespace std;
  usr* cons = 0;
  if ( expr ){
    var* cexpr = expr->gen();
    if ( !cexpr->isconstant() )
      error::not_implemented();
    cons = cexpr->usr_cast();
  }
  usr* u = static_cast<usr*>(v);
  if ( cons ){
    constant_member* cm = new constant_member(u,cons);
    delete u;
    u = cm;
  }
  declarations::action1(u,false);
  vector<scope*>& children = scope::current->m_children;
  typedef vector<scope*>::iterator IT;
  for ( IT p = begin(children) ; p != end(children) ; ) {
    scope* ptr = *p;
    scope::id_t id = ptr->m_id;
    if (id == scope::PARAM)
      p = children.erase(p);
    else
      ++p;
  }
}

void cxx_compiler::classes::members::bit_field(var* v, expressions::base* expr)
{
  using namespace std;
  using namespace error::classes::bit_field;
  usr* u = static_cast<usr*>(v);
  auto_ptr<expressions::base> sweeper(expr);
  expressions::constant_flag = true;
  v = expr->gen();
  v = v->rvalue();
  expressions::constant_flag = false;
  if ( !v->m_type->integer() ){
    not_integer_bit(u);
    v->m_type = int_type::create();
  }
  int bit = 1;
  if ( !v->isconstant() )
    not_constant(u);
  else
    bit = v->value();
  if ( bit < 0 ){
    negative(u);
    bit = 1;
  }
  if ( !u )
    u = new usr("",backpatch_type::create(),usr::NONE,parse::position); 
  const type* T = u->m_type;
  if ( !T->backpatch() ){
    T = backpatch_type::create();
    u = new usr(u->m_name,T,usr::NONE,parse::position); 
  }
  u->m_type = T->patch(bit_field_type::create(bit,backpatch_type::create()),0);
  declarations::action1(u,false);
}

void cxx_compiler::class_or_namespace_name::action(scope* p)
{
  if ( !before ){
    if ( parse::identifier::mode != parse::identifier::new_obj )
      before = scope::current;
    else if (parse::peek() == COLONCOLON_MK)
      before = scope::current;
  }
  scope::current = p;
}

cxx_compiler::scope* cxx_compiler::class_or_namespace_name::before;

cxx_compiler::scope* cxx_compiler::class_or_namespace_name::last;

void cxx_compiler::class_or_namespace_name::after()
{
  if ( !class_or_namespace_name::before )
    return;
  class_or_namespace_name::last = scope::current;
  scope::current = class_or_namespace_name::before;
  class_or_namespace_name::before = 0;
}

