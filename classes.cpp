#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

cxx_compiler::tag::kind_t cxx_compiler::classes::specifier::get(int keyword)
{
  switch (keyword) {
  case CLASS_KW:  return tag::CLASS;
  case STRUCT_KW: return tag::STRUCT;
  case UNION_KW:  return tag::UNION;
  default:        return tag::ENUM;
  }
}

namespace cxx_compiler {
  namespace classes {
    namespace specifier {
      struct templ_name {
	const scope::TPSF& m_tpsf;
	templ_name(const scope::TPSF& tpsf) : m_tpsf(tpsf) {}
	string operator()(string name, string pn)
	{
	  scope::TPSF::const_iterator p = m_tpsf.find(pn);
	  assert(p != m_tpsf.end());
	  tag* ptr = p->second.first;
	  const type* T = ptr->m_types.second;
	  ostringstream os;
	  T->decl(os, "");
	  return name + os.str() + ',';
	}
      };
    } // end of namespace specifier
  } // end of namespace classes
} // end of namespace cxx_compiler

void
cxx_compiler::classes::specifier::begin(int keyword, var* v,
					std::vector<base*>* bases)
{
  using namespace std;
  usr* u = static_cast<usr*>(v);
  const scope::TPSF& tpsf = scope::current->m_tps.first;
  usr* uu = tpsf.empty() ? u : 0;
  auto_ptr<usr> sweeper(uu);
  tag::kind_t kind = get(keyword);
  string name = u ? u->m_name : new_name(".tag");
  const file_t& file = u ? u->m_file : parse::position;
  map<string,tag*>& tags = scope::current->m_tags;
  map<string,tag*>::const_iterator p = tags.find(name);
  template_tag* tt = 0;
  if (p != tags.end()) {
    tag* prev = p->second;
    if (prev->m_kind2 != tag::TEMPLATE) {
      if (prev->m_kind != kind) {
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
      declarations::specifier_seq::info_t::clear();
      return;
    }
    tt = static_cast<template_tag*>(prev);
    assert(tt == template_tag::instantiating);
    const scope::TPSF& tpsf = tt->templ_base::m_tps.first;
    const scope::TPSS& tpss = tt->templ_base::m_tps.second;
    name += '<';
    name = accumulate(begin(tpss), end(tpss), name, templ_name(tpsf));
    name.erase(name.size()-1);
    name += '>';
  }

  tag* ptr;
  if (tt) {
    ptr = template_tag::result
      = new instantiated_tag(kind, name, file, bases, tt);
  }
  else
    ptr = new tag(kind, name, file, bases);

  const scope::TPS& tps = scope::current->m_tps;
  if (!tps.first.empty()) {
    using namespace parse::templ;
    assert(!save_t::s_stack.empty());
    save_t* p = save_t::s_stack.top();
    assert(!p->m_tag);
    p->m_tag = ptr = new template_tag(*ptr, tps);
  }

  ptr->m_parent = scope::current;
  ptr->m_parent->m_children.push_back(ptr);
  ptr->m_types.first = incomplete_tagged_type::create(ptr);
  tags[name] = ptr;
  scope::current = ptr;
  declarations::specifier_seq::info_t::clear();
}

void
cxx_compiler::classes::specifier::begin2(int keyword, tag* ptr)
{
  using namespace std;
  string name = ptr->m_name;
  usr* tmp = new usr(name,0,usr::NONE,file_t(),usr::NONE2);
  begin(keyword,tmp,0);
}

const cxx_compiler::type* cxx_compiler::classes::specifier::action()
{
  using namespace std;
  using namespace class_or_namespace_name;
  assert(scope::current->m_id == scope::TAG);
  tag* ptr = static_cast<tag*>(scope::current);
  scope* ps = ptr->m_parent;
  const scope::TPSF& tpsf = ps->m_tps.first;
  if (!tpsf.empty()) {
    assert(ptr->m_kind2 == tag::TEMPLATE);
    template_tag* tt = static_cast<template_tag*>(ptr);
    assert(!tt->m_specified);
    tt->m_specified = true;
    scope::current = ptr->m_parent;
    return ptr->m_types.first;
  }

  const type* ret = record_type::create(ptr);
  ptr->m_types.second = ret;
  handle_copy_ctor(ptr);

  map<usr*, parse::member_function_body::save_t>& tbl =
    parse::member_function_body::stbl;
  if (tbl.empty()) {
    handle_vdel(ptr);
    assert(!before.empty());
    assert(scope::current == before.back());
    before.pop_back();
    scope::current = ptr->m_parent;
    return ret;
  }
  scope* org = scope::current;
  for_each(tbl.begin(),tbl.end(),member_function_definition);
  tbl.clear();
  scope::current = org;
  handle_vdel(ptr);
  scope::current = org->m_parent;
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
	usr::flag_t flag = u->m_flag;
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
  if (u->m_flag2 & usr::CONV_OPE) {
    using namespace declarations::specifier_seq;
    assert(!info_t::s_stack.empty());
    delete info_t::s_stack.top();
  }

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
    u = new usr("",backpatch_type::create(),usr::NONE,parse::position,
		usr::NONE2); 
  const type* T = u->m_type;
  if ( !T->backpatch() ){
    T = backpatch_type::create();
    u = new usr(u->m_name,T,usr::NONE,parse::position,usr::NONE2); 
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
	      using namespace expressions::primary::literal;
              const type* Tx = x->result_type();
	      if (!p) {
		var* zero = integer::create(0);
		zero = zero->cast(Tx);
		code.push_back(new invladdr3ac(x, zero));
		return;
	      }
              if (p->size() != 1)
                error::not_implemented();
              const type* Ux = Tx->unqualified();
              expressions::base* expr = (*p)[0];
              var* y = expr->gen();
              if (Ux->m_id != type::REFERENCE)
                y = y->rvalue();
              const type* Ty = y->result_type();
              var tmp(Ty);
              bool discard = false;
              const type* T = assignment::valid(Tx, &tmp, &discard, true, 0);
              if (!T)
                error::not_implemented();
	      y = y->cast(Tx);
              code.push_back(new invladdr3ac(x,y));
            }
            void for_aggregate(var* dst, vector<expressions::base*>* p)
            {
	      vector<var*> arg;
	      if (p)
		transform(begin(*p), end(*p), back_inserter(arg),
			  mem_fun(&expressions::base::gen));
	      const type* T = dst->result_type();
	      assert(T->m_id == type::RECORD);
	      typedef const record_type REC;
	      REC* rec = static_cast<REC*>(T);
	      tag* ptr = rec->get_tag();
	      usr* ctor = has_ctor_dtor(ptr, false);
	      usr::flag_t flag = ctor->m_flag;
	      if (flag & usr::OVERLOAD) {
		overload* ovl = static_cast<overload*>(ctor);
		ovl->m_obj = dst;
		ovl->call(&arg);
		return;
	      }
	      call_impl::wrapper(ctor, &arg, dst);
            }
            struct sweeper {
              vector<expressions::base*>* m_expr;
              sweeper(vector<expressions::base*>* p) : m_expr(p) {}
              ~sweeper()
              {
		if (!m_expr) 
		  return;
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
            map<usr*, VALUE> for_parse;
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
	      map<string, vector<usr*> >& usrs = param->m_usrs;
	      typedef map<string, vector<usr*> >::const_iterator IT;
	      IT p = usrs.find(this_name);
	      assert(p != usrs.end());
	      return b;
	    }
	    map<usr*, map<usr*, pbc> > mtbl;
	    void id_action(usr* u, EXPRS* exprs, usr* ctor)
	    {
              using namespace expressions::primary;
	      scope* ps = ctor->m_scope;
	      assert(ps->m_id == scope::TAG);
	      tag* ptr = static_cast<tag*>(ps);
              const type* T = ptr->m_types.second;
              if (!T) {
                assert(scope::current->m_id == scope::PARAM);
                for_parse[ctor].push_back(make_pair(new PAIR(u,0),exprs));
                return;
              }
	      block* b = get_block(ptr);
              scope* org = scope::current;
              scope::current = b;
	      int n = code.size();
              vector<route_t> dummy;
              var* dst = from_member(u, dummy);
              gen(dst, exprs);
              scope::current = org;
	      vector<tac*> tmp;
	      copy(begin(code) + n, end(code), back_inserter(tmp));
	      mtbl[ctor][u] = pbc(b->m_parent, b, tmp);
	      code.resize(n);
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
		const vector<usr*>& cand = ovl->m_candidacy;
		vector<usr::flag_t> org;
		for_each(begin(cand), end(cand), [&org](usr* u) {
		    usr::flag_t flag = u->m_flag;
		    org.push_back(flag);
		    u->m_flag = usr::flag_t(flag & ~usr::INLINE);
 		  });             // (*1)
		ovl->call(&arg);
		int i = 0;
		for (auto u : cand)
		  u->m_flag = org[i++];
		return;
	      }

	      // (*1) and bellow are coded for replacing special ctor version
	      // See record_type.cpp base_ctor_dtor::operator()
	      ctor->m_flag = usr::flag_t(ctor->m_flag & ~usr::INLINE);
	      call_impl::wrapper(ctor, &arg, this_ptr);
	      ctor->m_flag = flag;
	    }
	    inline usr* get_this(scope* param, const record_type* rec)
	    {
	      vector<scope*>& children = param->m_children;
	      if (children.empty()) {
		block* bp = new block;
		using namespace class_or_namespace_name;
		assert(before.back() == bp);
		before.pop_back();
		param->m_children.push_back(bp);
		bp->m_parent = param;
	      }
	      map<string, vector<usr*> >& usrs = param->m_usrs;
	      typedef map<string, vector<usr*> >::const_iterator IT;
	      IT p = usrs.find(this_name);
	      if (p != usrs.end()) {
		const vector<usr*>& v = p->second;
		assert(v.size() == 1);
		usr* this_ptr = v.back();
		return this_ptr;
	      }
	      const type* T = pointer_type::create(rec);
	      usr* this_ptr = new usr(this_name, T, usr::NONE,
				      parse::position, usr::NONE2);
	      usrs[this_name].push_back(this_ptr);
	      vector<usr*>& order = param->m_order;
	      vector<usr*> tmp = order;
	      order.clear();
	      order.push_back(this_ptr);
	      copy(begin(tmp), end(tmp), back_inserter(order));
	      return this_ptr;
	    }
	    map<usr*, map<tag*, pbc> > btbl;
	    inline void direct_base(base* bp, const record_type* rec,
				    tag* btag, EXPRS* exprs, usr* ctor)
	    {
	      const map<base*, int>& bo = rec->base_offset();
	      map<base*, int>::const_iterator q = bo.find(bp);
	      assert(q != bo.end());
	      int offset = q->second;
              scope* param = fundef::current->m_param;
	      usr* this_ptr = get_this(param, rec);
              vector<scope*>& c = param->m_children;
              assert(!c.empty());
              scope* ps = c.back();
              assert(ps->m_id == scope::BLOCK);
              block* b = static_cast<block*>(ps);
              scope* org = scope::current;
              scope::current = b;
	      assert(code.empty());
              gen(bp->m_tag, this_ptr, offset, exprs);
              scope::current = org;
	      btbl[ctor][btag] = pbc(param, b, code);
	      code.clear();
	    }
	    inline void common_case(const record_type* x,
				    const record_type* rec,
				    tag* btag, EXPRS* exprs, usr* ctor)
	    {
	      typedef const record_type REC;
	      const map<REC*, int>& vco = rec->virt_common_offset();
	      typedef map<REC*, int>::const_iterator IT;
	      IT p = vco.find(x);
	      assert(p != vco.end());
	      int offset = p->second;
              scope* param = fundef::current->m_param;
	      usr* this_ptr = get_this(param, rec);
              vector<scope*>& c = param->m_children;
              assert(!c.empty());
              scope* ps = c.back();
              assert(ps->m_id == scope::BLOCK);
              block* b = static_cast<block*>(ps);
              scope* org = scope::current;
              scope::current = b;
	      assert(code.empty());
              gen(x->get_tag(), this_ptr, offset, exprs);
              scope::current = org;
	      btbl[ctor][btag] = pbc(param, b, code);
	      code.clear();
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
                for_parse[ctor].push_back(make_pair(new PAIR(0, btag),exprs));
                return;
              }
	      assert(T->m_id == type::RECORD);
	      typedef const record_type REC;
	      REC* rec = static_cast<REC*>(T);
	      vector<base*>& bases = *ptr->m_bases;
	      typedef vector<base*>::const_iterator ITx;
	      ITx p = find_if(begin(bases), end(bases), 
			      [btag](base* bp){ return bp->m_tag == btag; });
	      if (p != end(bases))
		return direct_base(*p, rec, btag, exprs, ctor);
	      const vector<REC*>& common = rec->common();
	      typedef vector<REC*>::const_iterator ITy;
	      ITy q = find_if(begin(common), end(common),
			      [btag](REC* x){ return x->get_tag() == btag; });
	      if (q != end(common))
		return common_case(*q, rec, btag, exprs, ctor);
	      error::not_implemented();
	    }
            void action(pair<usr*, tag*>* x, EXPRS* exprs)
            {
	      auto_ptr<pair<usr*, tag*> > sweeper(x);
              assert(fundef::current);
              usr* ctor = fundef::current->m_usr;
              usr::flag_t flag = ctor->m_flag;
              if (!(flag & usr::CTOR)) {
                error::not_implemented();
                return;
              }
	      usr* u = x->first;
	      tag* ptr = x->second;
	      u ? id_action(u, exprs, ctor) : tag_action(ptr, exprs, ctor);
            }
          } // end of namespace mem_initializer
        } // end of namespace definition
      } // end of namespace function
    } // end of namespace declarators
  } // end of namespace declarations
}  // end of namespace cxx_compiler
