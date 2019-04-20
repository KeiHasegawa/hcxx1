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
    class_or_namespace_name::before.push_back(prev);
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
  assert(scope::current->m_id == scope::TAG);
  tag* ptr = static_cast<tag*>(scope::current);
  const type* ret = record_type::create(ptr);
  ptr->m_types.second = ret;
  map<usr*, parse::member_function_body::save_t>& tbl =
    parse::member_function_body::table;
  if (tbl.empty()) {
    using namespace class_or_namespace_name;
    assert(!before.empty());
    assert(scope::current == before.back());
    before.pop_back();
    scope::current = ptr->m_parent;
    return ret;
  }
  for_each(tbl.begin(),tbl.end(),member_function_definition);
  tbl.clear();
  return ret;
}

namespace cxx_compiler {
  namespace classes {
    namespace specifier {
      using namespace std;
      void
      member_function_definition(pair<usr* const,
                                      parse::member_function_body::save_t>& E)
      {
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
    } // end of namespace specifier
  } // end of namespace classes
} // end of namespace cxx_compiler

void cxx_compiler::classes::members::action(var* v, expressions::base* expr)
{
  using namespace std;
  usr* cons = 0;
  if ( expr ){
    var* cexpr = expr->gen();
    cexpr = cexpr->rvalue();
    if ( !cexpr->isconstant() )
      error::not_implemented();
    cons = cexpr->usr_cast();
  }
  assert(v->usr_cast());
  usr* u = static_cast<usr*>(v);
  if (cons) {
    with_initial* p = new with_initial(*u);
    p->m_value[0] = cons;
    delete u;
    u = p;
  }
  u = declarations::action1(u, false);
  if (cons) {
    usr::flag_t flag = u->m_flag;
    usr::flag_t mask = usr::flag_t(usr::STATIC | usr::VIRTUAL);
    if (!(flag & mask))
      error::not_implemented();
    if (flag & usr::STATIC)
      u->m_flag = usr::flag_t(flag | usr::WITH_INI | usr::STATIC_DEF);
    if (flag & usr::VIRTUAL)
      error::not_implemented();
  }
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

namespace cxx_compiler {
  namespace class_or_namespace_name {
    using namespace std;
    vector<scope*> before;
    scope* last;
  } // end of namespace class_or_namespace_name
} // end of namespace cxx_compiler

void cxx_compiler::class_or_namespace_name::after()
{
  assert(!before.empty());
  assert(before.back());
  last = scope::current;
  scope::current = before.back();
}

namespace cxx_compiler {
  namespace declarations {
    namespace declarators {
      namespace function {
        namespace definition {
          namespace mem_initializer {
            using namespace std;
            void for_scalar(var* x, vector<expressions::base*>* p)
            {
              using namespace expressions;
              if (p->size() != 1)
                error::not_implemented();
              const type* Tx = x->result_type();
              const type* Ux = Tx->unqualified();
              expressions::base* expr = (*p)[0];
              var* y = expr->gen();
              if (Ux->m_id != type::REFERENCE)
                y = y->rvalue();
              const type* Ty = y->result_type();
              var tmp(Ty);
              bool discard = false;
              const type* T = assignment::valid(Tx, &tmp, &discard);
              if (!T)
                error::not_implemented();
              code.push_back(new invladdr3ac(x,y));
            }
            void for_aggregate(var* dst, vector<expressions::base*>* p)
            {
              error::not_implemented();
            }
            struct sweeper {
              vector<expressions::base*>* m_expr;
              sweeper(vector<expressions::base*>* p) : m_expr(p) {}
              ~sweeper()
              {
                for (auto p : *m_expr)
                  delete p;
                delete m_expr;
              }
            };
            void gen(var* dst, vector<expressions::base*>* p)
            {
              sweeper sweeper(p);
              const type* T = dst->result_type();
              T->scalar() ? for_scalar(dst, p) : for_aggregate(dst, p);
            }
            map<usr*, VALUE> table;
            void action(var* v, vector<expressions::base*>* p)
            {
              using namespace expressions::primary;
              assert(fundef::current);
              usr* fun = fundef::current->m_usr;
              usr::flag_t flag = fun->m_flag;
              if (!(flag & usr::CTOR)) {
                error::not_implemented();
                return;
              }
              tag* ptr = 0;
              if (scope::current->m_id == scope::PARAM) {
                scope* parent = scope::current->m_parent;
                assert(parent->m_id == scope::TAG);
                ptr = static_cast<tag*>(parent);
              }
              else {
                assert(scope::current->m_id == scope::TAG);
                ptr = static_cast<tag*>(scope::current);
              }
              const type* T = ptr->m_types.second;
              if (!T) {
                assert(scope::current->m_id == scope::PARAM);
                table[fun].push_back(make_pair(v,p));
                return;
              }
              assert(v->usr_cast());
              usr* u = static_cast<usr*>(v);
              scope* param = fundef::current->m_param;
              vector<scope*>& c = param->m_children;
              assert(!c.empty());
              scope* ps = c.back();
              assert(ps->m_id == scope::BLOCK);
              block* b = static_cast<block*>(ps);
              scope* org = scope::current;
              scope::current = b;
              vector<tag*> dummy;
              var* dst = from_member(u, dummy);
              gen(dst, p);
              scope::current = org;
            }
          } // end of namespace mem_initializer
        } // end of namespace definition
      } // end of namespace function
    } // end of namespace declarators
  } // end of namespace declarations
}  // end of namespace cxx_compiler
