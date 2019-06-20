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
cxx_compiler::classes::specifier::begin(int keyword, var* v,
					std::vector<base*>* bases)
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
    if (types.second) {
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

namespace cxx_compiler {
  using namespace std;
  namespace classes {
    namespace specifier {
      using namespace std;
      void member_function_definition(pair<usr* const, parse::member_function_body::save_t>&);
    } // end of namespace specifier
  } // end of namespace classes
} // end of namespace cxx_compiler

const cxx_compiler::type* cxx_compiler::classes::specifier::action()
{
  using namespace std;
  using namespace class_or_namespace_name;
  assert(scope::current->m_id == scope::TAG);
  tag* ptr = static_cast<tag*>(scope::current);
  const type* ret = record_type::create(ptr);
  ptr->m_types.second = ret;
  handle_copy_ctor(ptr);

  map<usr*, parse::member_function_body::save_t>& tbl =
    parse::member_function_body::stbl;
  if (tbl.empty()) {
    assert(!before.empty());
    assert(scope::current == before.back());
    before.pop_back();
    scope::current = ptr->m_parent;
    return ret;
  }
  for_each(tbl.begin(),tbl.end(),member_function_definition);
  tbl.clear();
  assert(!before.empty());
  assert(ptr == before.back());
  before.pop_back();
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
	using namespace declarations;
        usr* u = E.first;
        u->m_type = u->m_type->complete_type();
        scope::current = u->m_scope;
        vector<scope*>& children = scope::current->m_children;
        scope* param = E.second.m_param;
        children.push_back(param);
        fundef::current = new fundef(u,param);
	const vector<usr*>& order = param->m_order;
	for_each(order.begin(),order.end(),check_object);
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
  u = declarations::action1(u, false);
  if (cons) {
    usr::flag_t flag = u->m_flag;
    usr::flag_t mask = usr::flag_t(usr::STATIC | usr::VIRTUAL);
    if (!(flag & mask))
      error::not_implemented();
    if ((flag & usr::STATIC) && (flag & usr::VIRTUAL))
      error::not_implemented();
    if (flag & usr::STATIC) {
      with_initial* wi = new with_initial(*u);
      wi->m_value[0] = cons;
      scope* p = u->m_scope;
      string name = u->m_name;
      assert(p->m_order.back() == u);
      p->m_order.back() = wi;
      assert(p->m_usrs[name].back() == u);
      p->m_usrs[name].back() = wi;
      wi->m_flag = usr::flag_t(flag | usr::WITH_INI | usr::STATIC_DEF);
      delete u;
      u = wi;
    }
    if (flag & usr::VIRTUAL) {
      const type* T = cons->m_type;
      T = T->unqualified();
      if (T->m_id != type::INT)
        error::not_implemented();
      constant<int>* c = static_cast<constant<int>*>(cons);
      if (c->m_value != 0)
        error::not_implemented();
      u->m_flag = usr::flag_t(flag | usr::PURE_VIRT);
    }
  }
  vector<scope*>& children = scope::current->m_children;
  typedef vector<scope*>::iterator IT;
  for ( IT p = begin(children) ; p != end(children) ; ) {
    scope* ptr = *p;
    scope::id_t id = ptr->m_id;
    if (id == scope::PARAM) {
      delete ptr;
      p = children.erase(p);
    }
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

void cxx_compiler::class_or_namespace_name::after(bool set_last)
{
  assert(!before.empty());
  assert(before.back());
  if (set_last)
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
              const type* T = assignment::valid(Tx, &tmp, &discard, true);
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
            map<usr*, VALUE> mtbl;
	    inline block* get_block(tag* ptr)
	    {
              scope* param = fundef::current->m_param;
              vector<scope*>& children = param->m_children;
              if (!children.empty()) {
		scope* ps = children.back();
		assert(ps->m_id == scope::BLOCK);
		return static_cast<block*>(ps);
	      }
	      block* b = new block;
	      children.push_back(b);
	      b->m_parent = param;

	      string name = "this";
	      const type* T = ptr->m_types.second;
	      T = pointer_type::create(T);
	      usr* this_ptr = new usr(name,T,usr::NONE,file_t());
	      this_ptr->m_scope = param;
	      param->m_order.push_back(this_ptr);
	      param->m_usrs[name].push_back(this_ptr);
	      return b;
	    }
	    void id_action(var* v, EXPRS* exprs, usr* ctor)
	    {
              using namespace expressions::primary;
	      scope* tmp = ctor->m_scope;
	      assert(tmp->m_id == scope::TAG);
	      tag* ptr = static_cast<tag*>(tmp);
              const type* T = ptr->m_types.second;
              if (!T) {
                assert(scope::current->m_id == scope::PARAM);
                mtbl[ctor].push_back(make_pair(new PAIR(v,0),exprs));
                return;
              }
              assert(v->usr_cast());
              usr* u = static_cast<usr*>(v);
	      block* b = get_block(ptr);
	      assert(T->m_id == type::RECORD);
	      typedef const record_type REC;
	      REC* rec = static_cast<REC*>(T);
	      scope* param = fundef::current->m_param;
	      usr* this_ptr = *param->m_order.begin();
	      rec->ctor_code(ctor, param, this_ptr, b);
              scope* org = scope::current;
              scope::current = b;
              vector<route_t> dummy;
              var* dst = from_member(u, dummy);
              gen(dst, exprs);
              scope::current = org;
	    }
	    void gen(tag* ptr, var* this_ptr, int offset, EXPRS* exprs)
	    {
              using namespace expressions::primary::literal;
	      string name = ptr->m_name;
	      typedef map<string, vector<usr*> >::const_iterator IT;
	      IT p = ptr->m_usrs.find(name);
	      if (p == ptr->m_usrs.end())
		error::not_implemented();
	      if (offset) {
		var* off = integer::create(offset);
		const type* T = this_ptr->m_type;
		var* tmp = new var(T);
		assert(scope::current->m_id == scope::BLOCK);
		block* b = static_cast<block*>(scope::current);
		b->m_vars.push_back(tmp);
		code.push_back(new add3ac(tmp, this_ptr, off));
		this_ptr = tmp;
	      }
	      const vector<usr*>& v = p->second;
	      usr* ctor = v.back();
	      usr::flag_t flag = ctor->m_flag;
	      vector<var*> arg;
	      if (exprs)
		transform(begin(*exprs), end(*exprs), back_inserter(arg),
			  mem_fun(&expressions::base::gen));
	      if (flag & usr::OVERLOAD) {
		overload* ovl = static_cast<overload*>(ctor);
		ovl->m_obj = this_ptr;
		ovl->call(&arg);
		return;
	      }
	      call_impl::wrapper(ctor, &arg, this_ptr);
	    }
	    void tag_action(tag* btag, EXPRS* exprs, usr* ctor)
	    {
	      scope* tmp = ctor->m_scope;
	      assert(tmp->m_id == scope::TAG);
	      tag* ptr = static_cast<tag*>(tmp);
	      if (!ptr->m_bases) {
		error::not_implemented();
		return;
	      }
	      const type* T = ptr->m_types.second;
              if (!T) {
                assert(scope::current->m_id == scope::PARAM);
                mtbl[ctor].push_back(make_pair(new PAIR(0, btag),exprs));
                return;
              }
	      vector<base*>& bases = *ptr->m_bases;
	      typedef vector<base*>::const_iterator IT;
	      IT p = find_if(begin(bases), end(bases), 
			     [btag](base* bp){ return bp->m_tag == btag; });
	      if (p == end(bases)) {
		error::not_implemented();
		return;
	      }
	      base* bp = *p;
	      assert(T->m_id == type::RECORD);
	      typedef const record_type REC;
	      REC* rec = static_cast<REC*>(T);
	      const map<base*, int>& bo = rec->base_offset();
	      map<base*, int>::const_iterator q = bo.find(bp);
	      assert(q != bo.end());
	      int offset = q->second;
              scope* param = fundef::current->m_param;
	      typedef map<string, vector<usr*> >::const_iterator IT2;
	      IT2 r = param->m_usrs.find("this");
	      assert(r != param->m_usrs.end());
	      const vector<usr*>& v = r->second;
	      assert(v.size() == 1);
	      usr* this_ptr = v.back();
              vector<scope*>& c = param->m_children;
              assert(!c.empty());
              scope* ps = c.back();
              assert(ps->m_id == scope::BLOCK);
              block* b = static_cast<block*>(ps);
              scope* org = scope::current;
              scope::current = b;
              gen(bp->m_tag, this_ptr, offset, exprs);
              scope::current = org;
	    }
            void action(pair<var*, tag*>* x, EXPRS* exprs)
            {
	      auto_ptr<pair<var*, tag*> > sweeper(x);
              assert(fundef::current);
              usr* ctor = fundef::current->m_usr;
              usr::flag_t flag = ctor->m_flag;
              if (!(flag & usr::CTOR)) {
                error::not_implemented();
                return;
              }
	      var* v = x->first;
	      v ? id_action(v, exprs, ctor) : tag_action(x->second, exprs, ctor);
            }
          } // end of namespace mem_initializer
        } // end of namespace definition
      } // end of namespace function
    } // end of namespace declarators
  } // end of namespace declarations
}  // end of namespace cxx_compiler
