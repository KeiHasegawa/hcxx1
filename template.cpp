#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"

namespace cxx_compiler {
  namespace type_parameter {
    using namespace std;
    string param_name(var* v)
    {
      if (!v)
	return new_name(".tag");
      assert(v->usr_cast());
      usr* u = static_cast<usr*>(v);
      return u->m_name;
    }
  } // end of namespace type_parameter
} // end of namespace cxx_compiler

void cxx_compiler::type_parameter::action(var* v, const type* T)
{
  string name = param_name(v);
  vector<scope::tps_t>& tps = scope::current->m_tps;
  assert(!tps.empty());
  scope::tps_t& b = tps.back();
  map<string, scope::tps_t::value_t>& table = b.m_table;
  map<string, scope::tps_t::value_t>::const_iterator p = table.find(name);
  if (p != table.end())
    error::not_implemented();
  tag* ptr = new tag(tag::CLASS, name, parse::position, 0);
  assert(class_or_namespace_name::before.back() == ptr);
  class_or_namespace_name::before.pop_back();
  ptr->m_parent = scope::current;
  vector<scope*>& children = scope::current->m_children;
  children.push_back(ptr);
  ptr->m_types.first = template_param_type::create(ptr);
  table[name].first = ptr;
  vector<string>& order = b.m_order;
  order.push_back(name);
  if (T) {
    map<string, pair<const type*, var*> >& def = b.m_default;
    def[name] = make_pair(T, (var*)0);
    parse::identifier::mode = parse::identifier::new_obj;
  }
}

void cxx_compiler::
templ_parameter::action(pair<const type*, expressions::base*>* p)
{
  auto_ptr<pair<const type*, expressions::base*> > sweeper(p);
  expressions::base* expr = p->second;
  if (!expr)
    return;
  auto_ptr<expressions::base> sweeper2(expr);
  var* v = expr->gen();
  v = v->rvalue();
  vector<scope::tps_t>& tps = scope::current->m_tps;
  assert(!tps.empty());
  scope::tps_t& b = tps.back();
  const vector<string>& order = b.m_order;
  assert(!order.empty());
  string name = order.back();
  map<string, pair<const type*, var*> >& def = b.m_default;
  assert(def.find(name) == def.end());
  def[name] = make_pair((const type*)0, v);
}

void cxx_compiler::declarations::templ::decl_begin()
{
  const vector<scope::tps_t>& tps = scope::current->m_tps;
  assert(!tps.empty());
  const scope::tps_t& b = tps.back();
  const map<string, scope::tps_t::value_t>& table = b.m_table;
  assert(!table.empty());
  using namespace parse::templ;
  save_t::nest.push_back(new save_t);
}

namespace cxx_compiler {
  namespace declarations {
    namespace templ {
      inline instantiated_tag* get(scope* p)
      {
	if (!p)
	  return 0;
	if (p->m_id != scope::TAG)
	  return get(p->m_parent);
	tag* ptr = static_cast<tag*>(p);
	if (ptr->m_flag & tag::INSTANTIATE) {
	  typedef instantiated_tag IT;
	  IT* it = static_cast<IT*>(ptr);
	  return it;
	}
	return get(ptr->m_parent);
      }
      static usr* ins_if_res;
      inline
      void instantiate_if(template_usr* tu, const template_usr::KEY& key)
      {
	template_usr::KEY::const_iterator p =
	  find_if(begin(key), end(key), template_param);
	if (p == end(key))
	  ins_if_res = tu->instantiate(key);
      }
      inline void class_templ_member(usr* prev, template_usr* tu)
      {
	scope* ps = prev->m_scope;
	if (ps->m_id != scope::TAG)
	  return;
	tag* ptr = static_cast<tag*>(ps);
	tag::flag_t flag = ptr->m_flag;
	if (!(flag & tag::INSTANTIATE))
	  return;
	instantiated_tag* it = static_cast<instantiated_tag*>(ptr);
	template_tag* tt = it->m_src;
	map<template_tag::KEY, tag*>& table = tt->m_table;
	for (const auto& p : table)
	  instantiate_if(tu, p.first);
      }
      inline void after_instantiate(template_usr* tu)
      {
	usr* prev = tu->m_prev;
	if (!prev)
	  return;
	if (prev->m_flag & usr::FRIEND) {
	  friend_func* ff = static_cast<friend_func*>(prev);
	  assert(ff->m_org);
	  prev = ff->m_org;
	}

	if (prev->m_flag2 & usr::TEMPLATE) {
	  template_usr* tp = static_cast<template_usr*>(prev);
	  map<template_usr::KEY, usr*>& table = tp->m_table;
	  for (const auto& p : table)
	    instantiate_if(tu, p.first);
	  return;
	}

	class_templ_member(prev, tu);
      }
      inline void handle(usr* u, const parse::read_t& r)
      {
	assert(u->m_flag2 & usr::TEMPLATE);
	template_usr* tu = static_cast<template_usr*>(u);
	assert(!tu->m_decled);
	tu->m_decled = scope::current;
	tu->m_read = r;
	if (u->m_flag & usr::STATIC_DEF) {
	  scope* p = u->m_scope;
	  instantiated_tag* it = get(p);
	  template_tag* tt = it->m_src;
	  tt->m_static_def.push_back(tu);
	}
	else
	  after_instantiate(tu);
      }
      inline scope::tps_t::val2_t* create(const scope::tps_t::val2_t& x)
      {
	return new scope::tps_t::val2_t(x);
      }
      inline void dispatch(pair<template_tag::KEY, tag*> x, template_tag* tt)
      {
	const template_tag::KEY& key = x.first;
	tag* ptr = x.second;
	if (ptr->m_flag & tag::SPECIAL_VER) {
	  tt->m_table[key] = ptr;
	  return;
	}
	vector<scope::tps_t::val2_t*>* pv = new vector<scope::tps_t::val2_t*>;
	transform(begin(key), end(key), back_inserter(*pv), create);
	tt->instantiate(pv);
      }
      inline void handle(tag* ptr, const parse::read_t& r)
      {
	assert(ptr->m_flag & tag::TEMPLATE);
	template_tag* tt = static_cast<template_tag*>(ptr);
	tt->m_read = r;
	if (template_tag* prev = tt->m_prev) {
	  const template_tag::table_t& table = prev->m_table;
	  for_each(begin(table), end(table), bind2nd(ptr_fun(dispatch), tt));
	}
      }
      inline void instantiate_if2(template_usr* tu,
				  const template_tag::KEY& key)
      {
	typedef template_usr::KEY::const_iterator IT;
	IT p = find_if(begin(key), end(key), template_param);
	if (p == end(key))
	  tu->instantiate(key);
      }
    } // end of namespace templ
  } // end of namespace declarations
} // end of namespace cxx_compiler

void cxx_compiler::declarations::templ::decl_end()
{
  vector<scope::tps_t>& tps = scope::current->m_tps;
  assert(!tps.empty());
  scope::tps_t& b = tps.back();
  map<string, scope::tps_t::value_t>& table = b.m_table;
  if (table.empty()) {
    tps.pop_back();
    return;
  }

  using namespace parse::templ;
  assert(!save_t::nest.empty());
  save_t* p = save_t::nest.back();
  static usr* last;
  if (usr* u = p->m_usr)
    handle(last = u, p->m_read);
  else if (p->m_tag)
    handle(p->m_tag, p->m_read);
  else {
    assert(last);
    scope* ps = last->m_scope;
    assert(ps->m_id == scope::TAG);
    tag* ptr = static_cast<tag*>(ps);
    assert(ptr->m_flag & tag::INSTANTIATE);
    instantiated_tag* it = static_cast<instantiated_tag*>(ptr);
    template_tag* tt = it->m_src;
    const template_tag::table_t& table = tt->m_table;
    assert(last->m_flag2 & usr::TEMPLATE);
    template_usr* tu = static_cast<template_usr*>(last);
    const parse::read_t& rd = p->m_read;
    template_usr* tu2 = new template_usr(*tu, b, false);
    tu->m_outer = tu2;
    handle(tu2, rd);
    for (const auto& p : table)
      instantiate_if2(tu2, p.first);
  }
  delete p;
  save_t::nest.pop_back();

  if (save_t::nest.empty())
    last = 0;

  vector<scope*>& children = scope::current->m_children;
  for (const auto& p : table) {
    if (tag* ptr = p.second.first) {
      typedef vector<scope*>::reverse_iterator ITx;
      ITx q = find(rbegin(children), rend(children), ptr);
      assert(q != rend(children));
      typedef vector<scope*>::iterator ITy;
      ITy r = q.base() - 1;
      children.erase(r);
    }
  }

  tps.pop_back();
}

namespace cxx_compiler {
  namespace template_usr_impl {
    struct calc {
      const map<string, scope::tps_t::value_t>& m_table;
      calc(const map<string, scope::tps_t::value_t>& table) : m_table(table) {}
      bool operator()(const type* Tx, const type* Ty);
    };
    namespace calc_impl {
      typedef bool FUNC(const type*, const type*,
			const map<string, scope::tps_t::value_t>&);
      bool template_param_case(const type* Tx, const type* Ty,
			       const map<string, scope::tps_t::value_t>& table)
      {
	assert(Tx->m_id == type::TEMPLATE_PARAM);
	typedef const template_param_type TP;
	TP* tp = static_cast<TP*>(Tx);
	tag* ptr = tp->get_tag();
	Ty = Ty->unqualified();
	if (ptr->m_types.second)
	  return ptr->m_types.second == Ty;
	ptr->m_types.second = Ty;
	return true;
      }
      struct helper {
	const map<string, scope::tps_t::value_t>& m_table;
	helper(const map<string, scope::tps_t::value_t>& table)
	  : m_table(table) {}
	bool
	operator()(const scope::tps_t::val2_t& x,
		   const scope::tps_t::val2_t& y)
	{
	  if (var* v = x.second) {
	    assert(y.second);
	    assert(v->usr_cast());
	    usr* u = static_cast<usr*>(v);
	    string name = u->m_name;
	    typedef map<string, scope::tps_t::value_t>::const_iterator IT;
	    IT p = m_table.find(name);
	    assert(p != m_table.end());
	    const scope::tps_t::value_t& value = p->second;
	    value.second->second = y.second;
	    return true;
	  }
	  const type* Tx = x.first;
	  const type* Ty = y.first;
	  calc tmp(m_table);
	  return tmp(Tx, Ty);
	}
      };
      bool common(tag* xtag, tag* ytag,
		  const map<string, scope::tps_t::value_t>& table)
      {
	if (xtag == ytag)
	  return true;
	if (!(xtag->m_flag & tag::INSTANTIATE))
	  return false;
	if (!(ytag->m_flag & tag::INSTANTIATE))
	  return false;
	instantiated_tag* xit = static_cast<instantiated_tag*>(xtag);
	instantiated_tag* yit = static_cast<instantiated_tag*>(ytag);
	if (xit->m_src != yit->m_src)
	  return false;
	typedef instantiated_tag::SEED SEED;
	const SEED& xseed = xit->m_seed;
	const SEED& yseed = yit->m_seed;
	assert(xseed.size() == yseed.size());
	typedef SEED::const_iterator IT;
	pair<IT, IT> ret = mismatch(begin(xseed), end(xseed), begin(yseed),
				    helper(table));
	return ret == make_pair(end(xseed), end(yseed));
      }
      bool record_case(const type* Tx, const type* Ty,
		       const map<string, scope::tps_t::value_t>& table)
      {
	assert(Tx->m_id == type::RECORD);
	if (Ty->m_id == type::REFERENCE) {
	  typedef const reference_type RT;
	  RT* rt = static_cast<RT*>(Ty);
	  Ty = rt->referenced_type();
	}

	type::id_t id = Ty->m_id;
	if (id != type::RECORD && id != type::INCOMPLETE_TAGGED)
	  return false;

	tag* xtag = Tx->get_tag();
	tag* ytag = Ty->get_tag();
	return common(xtag, ytag, table);
      }
      bool incomplete_case(const type* Tx, const type* Ty,
		       const map<string, scope::tps_t::value_t>& table)
      {
	assert(Tx->m_id == type::INCOMPLETE_TAGGED);
	typedef const incomplete_tagged_type ITT;
	ITT* xitt = static_cast<ITT*>(Tx);
	if (Ty->m_id == type::REFERENCE) {
	  typedef const reference_type RT;
	  RT* rt = static_cast<RT*>(Ty);
	  Ty = rt->referenced_type();
	}
	type::id_t id = Ty->m_id;
	if (id != type::RECORD && id != type::INCOMPLETE_TAGGED)
	  return false;
	tag* xtag = Tx->get_tag();
	tag* ytag = Ty->get_tag();
	return common(xtag, ytag, table);
      }
      bool pointer_case(const type* Tx, const type* Ty,
			const map<string, scope::tps_t::value_t>& table)
      {
	assert(Tx->m_id == type::POINTER);
	typedef const pointer_type PT;
	PT* xpt = static_cast<PT*>(Tx);
	Tx = xpt->referenced_type();
	if (Ty->m_id != type::POINTER)
	  error::not_implemented();
	PT* ypt = static_cast<PT*>(Ty);
	Ty = ypt->referenced_type();
	calc tmp(table);
	return tmp(Tx, Ty);
      }
      bool reference_case(const type* Tx, const type* Ty,
			  const map<string, scope::tps_t::value_t>& table)
      {
	assert(Tx->m_id == type::REFERENCE);
	typedef const reference_type RT;
	RT* xrt = static_cast<RT*>(Tx);
	if (Ty->m_id == type::REFERENCE) {
	  RT* rt = static_cast<RT*>(Ty);
	  Ty = rt->referenced_type();
	}
	Tx = xrt->referenced_type();
	calc tmp(table);
	return tmp(Tx, Ty);
      }
      bool qualifier_case(const type* Tx, const type* Ty,
			  const map<string, scope::tps_t::value_t>& table)
      {
	Tx = Tx->unqualified();
	Ty = Ty->unqualified();
	calc tmp(table);
	return tmp(Tx, Ty);
      }
      struct table_t : map<type::id_t, FUNC*> {
	table_t()
	{
	  (*this)[type::TEMPLATE_PARAM] = template_param_case;
	  (*this)[type::RECORD] = record_case;
	  (*this)[type::INCOMPLETE_TAGGED] = incomplete_case;
	  (*this)[type::POINTER] = pointer_case;
	  (*this)[type::REFERENCE] = reference_case;
	  (*this)[type::CONST] =
	  (*this)[type::VOLATILE] =
	  (*this)[type::RESTRICT] = qualifier_case;
	}
      } table;
    } // end of namespace calc_impl
    bool calc::operator()(const type* Tx, const type* Ty)
    {
      calc_impl::table_t::const_iterator p =
	calc_impl::table.find(Tx->m_id);
      if (p != calc_impl::table.end())
	return (p->second)(Tx, Ty, m_table);
      return Tx == Ty;
    }
    inline bool
    not_decided(const pair<string, pair<tag*, scope::tps_t::val2_t*> >& p)
    {
      const pair<tag*, scope::tps_t::val2_t*>& x = p.second;
      tag* ptr = x.first;
      if (ptr)
	return !ptr->m_types.second;
      scope::tps_t::val2_t* y = x.second;
      assert(y);
      assert(y->first);
      return !y->second;
    }
    struct decide {
      const map<string, scope::tps_t::value_t>& m_table;
      decide(const map<string, scope::tps_t::value_t>& table)
	: m_table(table) {}
      pair<const type*, var*> operator()(string name)
      {
	typedef map<string, scope::tps_t::value_t>::const_iterator IT;
	IT p = m_table.find(name);
	assert(p != m_table.end());
	const pair<tag*, scope::tps_t::val2_t*>& x = p->second;
	if (tag* ptr = x.first) {
	  const type* T = ptr->m_types.second;
	  return make_pair(T, (usr*)0);
	}
	scope::tps_t::val2_t* y = x.second;
	assert(y);
	assert(y->first);
	return make_pair((const type*)0, y->second);
      }
    };
    struct sweeper_a {
      const map<string, scope::tps_t::value_t>& m_table;
      map<tag*, const type*> m_org1;
      map<scope::tps_t::val2_t*, var*> m_org2;
      sweeper_a(const map<string, scope::tps_t::value_t>& table)
	: m_table(table)
      {
	for (auto p : m_table) {
	  scope::tps_t::value_t& x = p.second;
	  if (tag* ptr = x.first)
	    m_org1[ptr] = ptr->m_types.second;
	  else {
	    scope::tps_t::val2_t* y = x.second;
	    m_org2[y] = y->second;
	  }
	}
      }
      ~sweeper_a()
      {
	for (auto p : m_table) {
	  scope::tps_t::value_t& x = p.second;
	  if (tag* ptr = x.first)
	    ptr->m_types.second = m_org1[ptr];
	  else {
	    scope::tps_t::val2_t* y = x.second;
	    y->second = m_org2[y];
	  }
	}
      }
    };
    struct sweeper_b {
      vector<scope::tps_t> m_tps;
      scope* m_current;
      scope* m_del;
      file_t m_file;
      fundef* m_fundef;
      templ_base* m_ptr;
      vector<var*> m_garbage;
      vector<tac*> m_code;
      stack<declarations::specifier_seq::info_t*> m_stack1;
      vector<scope*> m_before;
      scope* m_last;
      vector<parse::templ::save_t*> m_nest;
      int m_yychar;
      parse::read_t m_read;
      map<int, bool> m_retry;
      parse::member_function_body::save_t* m_saved;
      static bool block_or_param(scope* p)
      {
	scope::id_t id = p->m_id;
	return id == scope::BLOCK || id == scope::PARAM;
      }
      static scope* current_param()
      {
	if (fundef::current) {
	  scope* param = fundef::current->m_param;
	  scope* parent = param->m_parent;
	  vector<scope*>& children = parent->m_children;
	  typedef vector<scope*>::reverse_iterator IT;
	  IT p = find(rbegin(children), rend(children), param);
	  assert(p != rend(children));
	  // Once erase current function parameter scope
	  children.erase(p.base() - 1);
	  return param;
	}

	if (scope* parent = scope::current->m_parent) {
	  scope* ret = scope::current;
	  while (block_or_param(parent)) {
	    ret = parent;
	    parent = parent->m_parent;
	  }
	  if (ret->m_id == scope::PARAM) {
	    vector<scope*>& children = parent->m_children;
	    typedef vector<scope*>::reverse_iterator IT;
	    IT p = find(rbegin(children), rend(children), ret);
	    assert(p != rend(children));
	    // Once erase current function parameter scope
	    children.erase(p.base() - 1);
	    return ret;
	  }
	}

	vector<scope*>& children = scope::current->m_children;
	if (children.empty())
	  return 0;
	scope* ret = children.back();
	if (ret->m_id != scope::PARAM)
	  return 0;
	children.pop_back();
	return ret;
      }
      sweeper_b(scope* p, templ_base* q)
	: m_tps(scope::current->m_tps), m_current(scope::current),
	  m_del(current_param()), m_file(parse::position),
	  m_fundef(fundef::current), m_ptr(parse::templ::ptr),
	  m_garbage(garbage), m_code(code),
	  m_stack1(declarations::specifier_seq::info_t::s_stack),
	  m_before(class_or_namespace_name::before),
	  m_last(class_or_namespace_name::last),
	  m_nest(parse::templ::save_t::nest), m_yychar(cxx_compiler_char),
	  m_read(parse::g_read), m_retry(parse::context_t::retry),
	  m_saved(parse::member_function_body::saved)
      {
	vector<scope::tps_t>& tps = scope::current->m_tps;
	tps.clear();
	scope::current = p;
	fundef::current = 0;
	parse::templ::ptr = q;
	garbage.clear();
	code.clear();
	while (!declarations::specifier_seq::info_t::s_stack.empty())
	  declarations::specifier_seq::info_t::s_stack.pop();
	class_or_namespace_name::before.clear();
	class_or_namespace_name::last = 0;

	vector<scope*> tmp;
	while (p) {
	  tmp.push_back(p);
	  p = p->m_parent;
	}
	copy(rbegin(tmp), rend(tmp),
	     back_inserter(class_or_namespace_name::before));

	parse::templ::save_t::nest.clear();
	cxx_compiler_char = 0;
	parse::g_read.m_token.clear();
	parse::g_read.m_lval.clear();
	parse::context_t::retry.clear();
	parse::member_function_body::saved = 0;
      } 
      ~sweeper_b()
      {
	parse::member_function_body::saved = m_saved;
	parse::context_t::retry = m_retry;
	parse::g_read = m_read;
	cxx_compiler_char = m_yychar;
	parse::templ::save_t::nest = m_nest;
	class_or_namespace_name::last = m_last;
	class_or_namespace_name::before = m_before;
	declarations::specifier_seq::info_t::s_stack = m_stack1;
	code = m_code;
	garbage = m_garbage;
	parse::templ::ptr = m_ptr;
	fundef::current = m_fundef;
	parse::position = m_file;
	if (m_del) {
	  scope* parent = m_del->m_parent;
	  vector<scope*>& children = parent->m_children;
	  // Recover parameter scope which is erased at sweeper_b::sweeper_b
	  children.push_back(m_del);
	}
	scope::current = m_current;
	scope::current->m_tps = m_tps;
      }
    };
    bool comp(const scope::tps_t::val2_t& x, const scope::tps_t::val2_t& y)
    {
      if (const type* Tx = x.first) {
	const type* Ty = y.first;
	assert(Ty);
	return compatible(Tx, Ty);
      }

      var* vx = x.second;
      assert(vx);
      var* vy = y.second;
      assert(vy);
      if (vx == vy)
	return true;
      if (usr* ux = vx->usr_cast()) {
	usr::flag2_t fx = ux->m_flag2;
	if (fx & usr::TEMPL_PARAM)
	  return false;
      }
      if (usr* uy = vy->usr_cast()) {
	usr::flag2_t fy = uy->m_flag2;
	if (fy & usr::TEMPL_PARAM)
	  return false;
      }
      if (!vx->isconstant(true) || !vy->isconstant(true))
	return false;
      if (vx->isconstant()) {
	assert(vy->isconstant());
	assert(vx->usr_cast());
	assert(vy->usr_cast());
	return vx->value() == vy->value();
      }
      addrof* ax = vx->addrof_cast();
      addrof* ay = vy->addrof_cast();
      assert(ax);
      assert(!ax->m_offset);
      assert(ay);
      assert(!ay->m_offset);
      return ax->m_ref == ay->m_ref;
    }
    bool match(const templ_base::KEY& x, const templ_base::KEY& y)
    {
      assert(x.size() == y.size());
      typedef templ_base::KEY::const_iterator IT;
      pair<IT, IT> ret = mismatch(begin(x), end(x), begin(y), comp);
      return ret == make_pair(end(x), end(y));
    }
  } // end of namespace template_usr_impl
} // end of namespace cxx_compiler

namespace cxx_compiler {
  stack<template_usr::info_t> template_usr::s_stack;
} // end of namespace cxx_compiler

cxx_compiler::usr*
cxx_compiler::template_usr::instantiate(std::vector<var*>* arg, KEY* trial)
{
  if (!arg)
    error::not_implemented();
  vector<const type*> atype;
  transform(begin(*arg), end(*arg), back_inserter(atype),
	    [](var* v){ return v->result_type(); });

  assert(m_type->m_id == type::FUNC);
  typedef const func_type FT;
  FT* ft = static_cast<FT*>(m_type);
  const vector<const type*>& param = ft->param();
  if (atype.size() != param.size())
    return 0;

  const map<string, scope::tps_t::value_t>& table = m_tps.m_table;
  template_usr_impl::sweeper_a sweeper_a(table);

  typedef vector<const type*>::const_iterator ITx;
  pair<ITx, ITx> pxx = mismatch(begin(param), end(param), begin(atype),
				template_usr_impl::calc(table));
  if (pxx.first != end(param)) {
    if (trial)
      return 0;
    error::not_implemented();
  }

  typedef map<string, scope::tps_t::value_t>::const_iterator ITy;
  ITy py = find_if(begin(table), end(table),
		   template_usr_impl::not_decided);
  if (py != end(table)) {
    if (trial)
      return 0;
    error::not_implemented();
  }

  KEY key;
  const vector<string>& order = m_tps.m_order;
  transform(begin(order), end(order), back_inserter(key),
	    template_usr_impl::decide(table));
  if (trial) {
    *trial = key;
    return reinterpret_cast<usr*>(-1);
  }

  table_t::const_iterator it = m_table.find(key);
  if (it != m_table.end())
    return it->second;
  it = find_if(m_table.begin(), m_table.end(),
	       [&key](const pair<KEY, usr*>& x)
	       { return template_usr_impl::match(key, x.first); });
  if (it != m_table.end())
    return m_table[key] = it->second;

  templ_base tmp = *this;
  template_usr_impl::sweeper_b sweeper_b(m_scope, &tmp);
  s_stack.push(info_t(this, 0, info_t::NONE, key));
  tinfos.push_back(make_pair(&s_stack.top(),(template_tag::info_t*)0));
  cxx_compiler_parse();
  tinfos.pop_back();
  instantiated_usr* ret = s_stack.top().m_iu;
  s_stack.pop();
  assert(ret->m_src == this || ret->m_src == m_next);
  assert(ret->m_seed == key);
  assert(!(ret->m_flag2 & usr::EXPLICIT_INSTANTIATE));
  return m_table[key] = ret;
}

namespace cxx_compiler {
  namespace template_usr_impl {
    struct sweeper_c : sweeper_a {
      struct helper {
	const map<string, scope::tps_t::value_t>& m_table;
	helper(const map<string, scope::tps_t::value_t>& table)
	  : m_table(table) {}
	bool operator()(string name, const pair<const type*, var*>& x)
	{
	  map<string, scope::tps_t::value_t>::const_iterator p =
	    m_table.find(name);
	  assert(p != m_table.end());
	  const scope::tps_t::value_t& value = p->second;
	  if (tag* ptr = value.first) {
	    const type* T = x.first;
	    assert(T);
	    ptr->m_types.second = T;
	  }
	  else {
	    scope::tps_t::val2_t* y = value.second;
	    var* v = x.second;
	    assert(v);
	    y->second = v;
	  }
	  return true;
	}
      };
      sweeper_c(const scope::tps_t& tps, const template_usr::KEY& key)
	: sweeper_a(tps.m_table)
      {
	const map<string, scope::tps_t::value_t>& table = tps.m_table;
	const vector<string>& v = tps.m_order;
	assert(v.size() == key.size());
	mismatch(begin(v), end(v), begin(key), helper(table));
      }
    };
  } // end of namespace template_usr_impl
} // end of namespace cxx_compiler

cxx_compiler::usr*
cxx_compiler::template_usr::instantiate(const KEY& key)
{
  table_t::const_iterator p = m_table.find(key);
  if (p != m_table.end())
    return p->second;
  p = find_if(m_table.begin(), m_table.end(),
	      [&key](const pair<KEY, usr*>& x)
	       { return template_usr_impl::match(key, x.first); });
  if (p != m_table.end())
    return p->second;

  template_usr_impl::sweeper_c sweeper_c(m_tps, key);

  templ_base tmp = *this;
  template_usr_impl::sweeper_b sweeper_b(m_decled, &tmp);
  declarations::templ::ins_if_res = 0;
  s_stack.push(info_t(this, 0, info_t::NONE, key));
  tinfos.push_back(make_pair(&s_stack.top(),(template_tag::info_t*)0));
  cxx_compiler_parse();
  tinfos.pop_back();
  instantiated_usr* ret = s_stack.top().m_iu;
  s_stack.pop();
  if (!ret) {
    usr* r = declarations::templ::ins_if_res;
    declarations::templ::ins_if_res = 0;
    return r;
  }
  if (m_patch_13_2) {
    assert(m_flag & usr::INLINE);
    ret->m_flag = usr::flag_t(ret->m_flag | usr::INLINE);
  }
  assert(ret->m_src == this);
  assert(ret->m_seed == key);
  assert(!(ret->m_flag2 & usr::EXPLICIT_INSTANTIATE));
  return m_table[key] = ret;
}

namespace cxx_compiler {
  namespace declarations {
    namespace templ {
      namespace id {
	struct sweeper {
	  parse::read_t m_org;
	  sweeper()
	  {
	    m_org = parse::g_read;
	    parse::g_read.m_token.clear();
	    parse::g_read.m_lval.clear();
	  }
	  ~sweeper()
	  {
	    parse::g_read = m_org;
	  }
	};
	inline tag* tag_action(tag* ptr, vector<scope::tps_t::val2_t*>* pv)
	{
	  assert(ptr->m_flag & tag::TEMPLATE);
          template_tag* tt = static_cast<template_tag*>(ptr);
	  if (!parse::base_clause) {
	    int c = parse::peek();
	    if (c == '{' || c == ':')
	      return tt->special_ver(pv);
	    if (c == ';') {
	      if (!specialization::nest.empty()) {
		if (specialization::nest.top() == scope::current) {
		  tag* ptr = tt->special_ver(pv);
		  assert(!ptr->m_types.first);
		  assert(!ptr->m_types.second);
		  ptr->m_types.first = incomplete_tagged_type::create(ptr);
		  map<string, tag*>& tags = scope::current->m_tags;
		  string name = ptr->m_name;
		  assert(tags.find(name) == tags.end());
		  tags[name] = ptr;
		  vector<scope*>& children = scope::current->m_children;
		  assert(find(begin(children), end(children), ptr)
			 == end(children));
		  children.push_back(ptr);
		  return ptr;
		}
	      }
	    }
	  }
	  sweeper sweeper;
	  return tt->instantiate(pv);
	}
	inline usr* usr_action(usr* u, vector<scope::tps_t::val2_t*>* pv)
	{
	  assert(u->m_flag2 == usr::TEMPLATE);
	  template_usr* tu = static_cast<template_usr*>(u);
	  return tu->instantiate_explicit(pv);
	}
        pair<usr*, tag*>*
        action(pair<usr*, tag*>* x, vector<scope::tps_t::val2_t*>* pv)
	{
	  // Note that `pv' is deleted at `template_tag::common()' or
	  // `template_usr::instantiate_explicit()'
	  bool b = parse::templ::save_t::nest.empty();
	  auto_ptr<pair<usr*, tag*> > sweeper(b ? x : 0);
	  if (tag* ptr = x->second) {
	    assert(!x->first);
	    return new pair<usr*, tag*>(0, tag_action(ptr, pv));
	  }
	  else {
	    usr* u = x->first;
	    assert(u);
	    return new pair<usr*, tag*>(usr_action(u, pv), 0);
	  }
	}
      } // end of namespace id
    } // end of namespace templ
  } // end of namespace declarations
} // end of namespace cxx_compiler

namespace cxx_compiler {
  namespace template_tag_impl {
    struct sweeper {
      vector<scope::tps_t::val2_t*>* m_ptr;
      stack<declarations::specifier_seq::info_t*> m_stack;
      bool m_new;
      sweeper(vector<scope::tps_t::val2_t*>* pv, bool b)
	: m_ptr(pv), m_new(b)
      {
	stack<declarations::specifier_seq::info_t*>& x =
	  declarations::specifier_seq::info_t::s_stack;
	m_stack = x;
	while (!x.empty())
	  x.pop();
      }
      ~sweeper()
      {
	parse::identifier::mode =
	  m_new ? parse::identifier::new_obj : parse::identifier::look;
	declarations::specifier_seq::info_t::s_stack = m_stack;
	if (m_ptr) {
	  for (auto p : *m_ptr)
	    delete p;
	  delete m_ptr;
	}
      }
    };
    struct calc_key {
      const map<string, scope::tps_t::value_t>& m_table;
      calc_key(const map<string, scope::tps_t::value_t>& table)
	: m_table(table) {}
      struct update {
	const map<string, scope::tps_t::value_t>& m_table;
	update(const map<string, scope::tps_t::value_t>& table)
	  : m_table(table) {}
	scope::tps_t::val2_t* operator()(const scope::tps_t::val2_t& x)
	{
	  if (var* v = x.second)
	    return new scope::tps_t::val2_t(0, v);
	  const type* T = x.first;
	  tag* ptr = T->get_tag();
	  if (!ptr)
	    return new scope::tps_t::val2_t(T, 0);
	  if (T->m_id == type::TEMPLATE_PARAM) {
	    T = ptr->m_types.second;
	    if (!T)
	      T = ptr->m_types.first;
	    return new scope::tps_t::val2_t(T, 0);
	  }
	  if (ptr->m_flag & tag::INSTANTIATE) {
	    calc_key op(m_table);
	    T = op.resolve_templ(T);
	    return new scope::tps_t::val2_t(T, 0);
	  }
	  return new scope::tps_t::val2_t(T, 0);
	}
      };
      const type* resolve_templ(const type* T)
      {
	assert(T);
	tag* ptr = T->get_tag();
	if (!ptr)
	  return T;
	if (T->m_id == type::TEMPLATE_PARAM) {
	  string name = ptr->m_name;
	  typedef map<string, scope::tps_t::value_t>::const_iterator IT;
	  IT p = m_table.find(name);
	  if (p == m_table.end())
	    return T;
	  const scope::tps_t::value_t& value = p->second;
	  assert(!value.second);
	  ptr = value.first;
	  if (const type* T2 = ptr->m_types.second)
	    return T2;
	  return T;
	}
	if (!(ptr->m_flag & tag::INSTANTIATE))
	  return T;
	typedef instantiated_tag IT;
	IT* it = static_cast<IT*>(ptr);
	const IT::SEED& seed = it->m_seed;
	vector<scope::tps_t::val2_t*>* conv =
	  new vector<scope::tps_t::val2_t*>;
	transform(begin(seed), end(seed), back_inserter(*conv),
		  update(m_table));
	template_tag* tt = it->m_src;
	tag* res = tt->instantiate(conv);
	T = res->m_types.second;
	if (!T)
	  T = res->m_types.first;
	assert(T);
	return T;
      }
      scope::tps_t::val2_t
      operator()(scope::tps_t::val2_t* p, string name)
      {
	map<string, scope::tps_t::value_t>::const_iterator it
	  = m_table.find(name);
	assert(it != m_table.end());
	const pair<tag*, scope::tps_t::val2_t*>& x = it->second;
	if (tag* ptr = x.first) {
	  const type* T = p->first;
	  ptr->m_types.second = T = resolve_templ(T);
	  return scope::tps_t::val2_t(T, 0);
	}
	scope::tps_t::val2_t* y = x.second;
	assert(y);
	const type* T = y->first;
	assert(T);
	var* v = p->second;
	assert(v);
	bool discard = false;
	bool ctor_conv = false;
	using namespace expressions;
	if (!assignment::valid(T, v, &discard, &ctor_conv, 0))
	  error::not_implemented();
	y->second = v;
	if (v->addrof_cast()) {
	  typedef vector<var*>::reverse_iterator IT;
	  IT p = find(rbegin(garbage), rend(garbage), v);
	  assert(p != rend(garbage));
	  vector<var*>::iterator q = p.base() - 1;
	  garbage.erase(q);
	}
	return scope::tps_t::val2_t(0, v);
      }
    };
  } // end of namespace template_tag_impl

  vector<template_tag::info_t> template_tag::nest;

  special_ver_tag::special_ver_tag(string name, template_tag* src,
				   const KEY& key)
  : tag(src->m_kind, name, parse::position, 0), m_src(src), m_key(key)
  {
    m_parent = src->m_parent;
    m_flag = SPECIAL_VER;
  }

  struct partial_special_tag : template_tag {
    special_ver_tag* m_sv;
    template_tag* m_primary;
    partial_special_tag(special_ver_tag* sv, const scope::tps_t& tps,
			template_tag* primary)
      : template_tag(*sv, tps), m_sv(sv), m_primary(primary)
    {
      m_flag = flag_t(m_flag | tag::PARTIAL_SPECIAL);
    }
    string instantiated_name() const;
  };
} // end of namespace cxx_compiler

namespace cxx_compiler {
  namespace template_tag_impl {
    struct get {
      const map<string, pair<const type*, var*> >& m_default;
      get(const map<string, pair<const type*, var*> >& def) : m_default(def) {}
      scope::tps_t::val2_t* operator()(string name)
      {
	typedef map<string, pair<const type*, var*> >::const_iterator IT;
	IT p = m_default.find(name);
	if (p == m_default.end())
	  error::not_implemented();
	return new scope::tps_t::val2_t(p->second);
      }
    };
    struct cmp_helper {
      template_tag::KEY& m_key;
      map<tag*, const type*>& m_table;
      cmp_helper(template_tag::KEY& key, map<tag*, const type*>& table)
	: m_key(key), m_table(table) {}
      bool operator()(const scope::tps_t::val2_t& x,
		      const scope::tps_t::val2_t& y)
      {
	if (const type* Tx = x.first) {
	  const type* Ty = y.first;
	  assert(Tx && Ty);
	  tag* xtag = Tx->get_tag();
	  assert(xtag);
	  map<tag*, const type*>::const_iterator p = m_table.find(xtag);
	  if (p != m_table.end()) {
	    const type* Tz = p->second;
	    return Ty == Tz;
	  }
	  m_key.push_back(y);
	  return true;
	}
	else {
	  var* vx = x.second;
	  var* vy = y.second;
	  assert(vx && vy);
	  error::not_implemented();
	  return false;
	}
      }
    };
    struct cmp {
      template_tag::KEY& m_key;
      map<tag*, const type*> m_table;
      bool cmp_type(const type* Tx, tag* xtag, const type* Ty)
      {
	if (Tx->m_id == type::TEMPLATE_PARAM) {
	  map<tag*, const type*>::const_iterator p = m_table.find(xtag);
	  if (p != m_table.end()) {
	    const type* Tz = p->second;
	    return compatible(Ty, Tz);
	  }
	  m_table[xtag] = Ty;
	  m_key.push_back(scope::tps_t::val2_t(Ty, 0));
	  return true;
	}
	return false;
      }
      bool cmp_type(const type* Tx, const type* Ty)
      {
	tag* xtag = Tx->get_tag();
	if (!xtag) {
	  if (Tx->m_id == Ty->m_id) {
	    if (Tx->m_id == type::POINTER) {
	      typedef const pointer_type PT;
	      PT* ptx = static_cast<PT*>(Tx);
	      Tx = ptx->referenced_type();
	      PT* pty = static_cast<PT*>(Ty);
	      Ty = pty->referenced_type();
	      return cmp_type(Tx, Ty);
	    }
	  }
	  return false;
	}
	tag* ytag = Ty->get_tag();
	if (!ytag)
	  return cmp_type(Tx, xtag, Ty);

	if (!(xtag->m_flag & tag::INSTANTIATE))
	  return cmp_type(Tx, xtag, Ty);

	instantiated_tag* itx = static_cast<instantiated_tag*>(xtag);
	if (!(ytag->m_flag & tag::INSTANTIATE))
	  return false;
	instantiated_tag* ity = static_cast<instantiated_tag*>(ytag);
	template_tag* ttx = itx->m_src;
	template_tag* tty = ity->m_src;
	if (ttx != tty)
	  return false;
	const template_tag::KEY& xs = itx->m_seed;
	const template_tag::KEY& ys = ity->m_seed;
	typedef template_tag::KEY::const_iterator IT;
	pair<IT, IT> ret = mismatch(begin(xs), end(xs), begin(ys),
				    cmp_helper(m_key, m_table));
	return ret == make_pair(end(xs), end(ys));
      }
      cmp(template_tag::KEY& key) : m_key(key) {}
      bool
      operator()(const scope::tps_t::val2_t& x, const scope::tps_t::val2_t* y)
      {
	if (const type* Tx = x.first) {
	  const type* Ty = y->first;
	  assert(Tx && Ty);
	  return cmp_type(Tx, Ty);
	}
	else {
	  var* vx = x.second;
	  var* vy = y->second;
	  assert(vx && vy);
	  error::not_implemented();
	  return false;
	}
      }
    };
    struct match {
      vector<scope::tps_t::val2_t*>* m_pv;
      template_tag::KEY& m_key;
      match(vector<scope::tps_t::val2_t*>* pv, template_tag::KEY& key)
	: m_pv(pv), m_key(key) {}
      bool operator()(const partial_special_tag* ps)
      {
	const special_ver_tag* sv = ps->m_sv;
	const template_tag::KEY& key = sv->m_key;
	assert(key.size() == m_pv->size());
	typedef template_tag::KEY::const_iterator ITx;
	typedef vector<scope::tps_t::val2_t*>::iterator ITy;
	pair<ITx, ITy> ret = mismatch(begin(key), end(key), begin(*m_pv),
				      cmp(m_key));
	if (ret != make_pair(end(key), end(*m_pv))) {
	  m_key.clear();
	  return false;
	}
	const vector<string>& order = ps->templ_base::m_tps.m_order;
	if (order.size() != m_key.size()) {
	  m_key.clear();
	  return false;
	}
	return true;
      }
    };
    struct instantiate {
      const template_tag::KEY& m_key;
      instantiate(const template_tag::KEY& key) : m_key(key) {}
      void operator()(template_usr* tu)
      {
	vector<scope::tps_t::val2_t*>* pv = new vector<scope::tps_t::val2_t*>;
	transform(begin(m_key), end(m_key), back_inserter(*pv),
		  declarations::templ::create);
	tu->instantiate_static_def(pv);
      }
    };
    inline void modify(template_tag* tt, scope::tps_t::val2_t* p)
    {
      string pn = new_name(".pn");
      scope::tps_t& tps = tt->templ_base::m_tps;
      vector<string>& order = tps.m_order;
      order.push_back(pn);
      map<string, scope::tps_t::value_t>& table = tps.m_table;
      if (p->first) {
	tag* ptr = new tag(tag::CLASS, pn, parse::position, 0);
	assert(!class_or_namespace_name::before.empty());
	assert(class_or_namespace_name::before.back() == ptr);
	class_or_namespace_name::before.pop_back();
	table[pn] = scope::tps_t::value_t(ptr,0);
      }
      else
	error::not_implemented();
    }
    bool already(template_tag* tt)
    {
      return  find_if(begin(template_tag::nest), end(template_tag::nest),
	      [tt](const template_tag::info_t& x)
	      { return x.m_tt == tt; }) != end(template_tag::nest);
    }
  } // end of namespace template_tag_impl
} // end of namespace cxx_compiler

cxx_compiler::tag*
cxx_compiler::
template_tag::common(std::vector<scope::tps_t::val2_t*>* pv,
		     bool special_ver)
{
  template_tag_impl::sweeper sweeper(pv, true);

  if (!nest.empty()) {
    template_tag::info_t& x = nest.back();
    template_tag* tt = x.m_tt;
    flag_t flag = tt->m_flag;
    if (flag & tag::PARTIAL_SPECIAL) {
      partial_special_tag* ps = static_cast<partial_special_tag*>(tt);
      template_tag* primary = ps->m_primary;
      if (primary == this) {
	string name = ps->instantiated_name();
	x.m_it = new instantiated_tag(m_kind, name, parse::position,
				      m_bases, ps, x.m_seed);
	return x.m_it;
      }
    }
  }

  KEY res;
  typedef vector<partial_special_tag*>::reverse_iterator IT;
  IT it = find_if(rbegin(m_partial_special), rend(m_partial_special),
		  template_tag_impl:: match(pv, res));
  if (it != rend(m_partial_special)) {
    vector<scope::tps_t::val2_t*>* pv2 = new vector<scope::tps_t::val2_t*>;
    transform(begin(res), end(res), back_inserter(*pv2),
      [](scope::tps_t::val2_t& x){ return new scope::tps_t::val2_t(x); });
    return (*it)->common(pv2, special_ver);
  }

  const vector<string>& order = templ_base::m_tps.m_order;
  if (!pv) {
    if (!order.empty())
      error::not_implemented();
  }
  else {
    int n = pv->size();
    int m = order.size();
    if (n < m) {
      const map<string, pair<const type*, var*> >& def =
	templ_base::m_tps.m_default;
      transform(begin(order) + n, end(order), back_inserter(*pv),
		template_tag_impl::get(def));
    }
 
    if (order.empty()) {
      assert(m_kind == tag::TEMPL);
      if (pv) {
	for_each(begin(*pv), end(*pv),
		 bind1st(ptr_fun(template_tag_impl::modify), this));
      }
    }

    if (order.size() != pv->size())
      error::not_implemented();
  }

  const map<string, scope::tps_t::value_t>& table = templ_base::m_tps.m_table;
  template_usr_impl::sweeper_a sweeper_a(table);
  KEY key;
  if (pv) {
    transform(begin(*pv), end(*pv), begin(order), back_inserter(key),
	      template_tag_impl::calc_key(table));
  }

  table_t::const_iterator p = m_table.find(key);
  if (p != m_table.end())
    return p->second;

  p = find_if(m_table.begin(), m_table.end(),
	      [&key](const pair<KEY, tag*>& x)
	      { return template_usr_impl::match(key, x.first); });
  if (p != m_table.end())
    return m_table[key] = p->second;

  if (special_ver) {
    string name = instantiated_name();
    special_ver_tag* sv = new special_ver_tag(name, this, key);
    const vector<scope::tps_t>& tps = scope::current->m_tps;
    if (!tps.empty()) {
      const scope::tps_t& b = tps.back();
      if (!b.m_table.empty()) {
	partial_special_tag* ps = new partial_special_tag(sv, b, this);
	m_partial_special.push_back(ps);
	using namespace parse::templ;
	assert(!save_t::nest.empty());
	save_t* p = save_t::nest.back();
	p->m_tag = ps;
      }
    }
    return m_table[key] = sv;
  }

  if (template_tag_impl::already(this)) {
    string name = instantiated_name();
    if (tag* ptr = declarations::elaborated::lookup(name, scope::current))
      return ptr;
    instantiated_tag* ret =
      new instantiated_tag(m_kind, name, parse::position, m_bases, this, key);
    map<string, tag*>& tags = scope::current->m_tags;
    assert(tags.find(name) == tags.end());
    tags[name] = ret;
    scope::current->m_children.push_back(ret);
    ret->m_parent = scope::current;
    ret->m_types.first = incomplete_tagged_type::create(ret);
    using namespace class_or_namespace_name;
    assert(!before.empty());
    assert(ret == before.back());
    before.pop_back();
    return ret;
  }

  templ_base tmp = *this;
  template_usr_impl::sweeper_b sweeper_b(m_parent, &tmp);
  nest.push_back(info_t(this, (instantiated_tag*)0, key));
  tinfos.push_back(make_pair((template_usr::info_t*)0,&nest.back()));
  cxx_compiler_parse();
  tinfos.pop_back();
  instantiated_tag* ret = nest.back().m_it;
  nest.pop_back();
  if (!ret) {
    string name = instantiated_name();
    map<string, tag*>& tags = scope::current->m_tags;
    typedef map<string, tag*>::const_iterator IT;
    IT p = tags.find(name);
    if (p != tags.end())
      return p->second;
    ret = new instantiated_tag(m_kind, name, parse::position,
			       m_bases, this, key);
    tags[name] = ret;
    scope::current->m_children.push_back(ret);
    ret->m_parent = scope::current;
    ret->m_types.first = incomplete_tagged_type::create(ret);
  }
  assert(ret->m_src == this);
  m_table[key] = ret;
  for_each(begin(m_static_def), end(m_static_def),
	   template_tag_impl::instantiate(key));
  return ret;
}

namespace cxx_compiler {
  using namespace std;
  vector<pair<template_usr::info_t*, template_tag::info_t*> > tinfos;
} // end of namespace cxx_compiler

namespace cxx_compiler {
  namespace template_tag_impl {
    struct helper {
      const map<string, scope::tps_t::value_t>& m_table;
      helper(const map<string, scope::tps_t::value_t>& table)
	: m_table(table) {}
      string operator()(string name, string pn)
      {
	map<string, scope::tps_t::value_t>::const_iterator p =
	  m_table.find(pn);
	assert(p != m_table.end());
	const pair<tag*, scope::tps_t::val2_t*>& x = p->second;
	if (tag* ptr = x.first) {
	  const type* T = ptr->m_types.second;
	  ostringstream os;
	  T->decl(os, "");
	  if (T->m_id == type::TEMPLATE_PARAM)
	    os << '.' << T;
	  return name + os.str() + ',';
	}
	const scope::tps_t::val2_t* y = x.second;
	assert(y);
	assert(y->first);
	var* v = y->second;
	if (usr* u = v->usr_cast()) {
	  usr::flag2_t flag2 = u->m_flag2;
	  if (flag2 & usr::TEMPL_PARAM) {
	    ostringstream os;
	    const type* T = y->first;
	    T->decl(os, u->m_name);
	    os << '.' << u;
	    return name + os.str() + ',';
	  }
	}
	if (!v->isconstant(true))
	  error::not_implemented();
	ostringstream os;
	if (v->isconstant()) {
	  assert(v->usr_cast());
	  usr* u = static_cast<usr*>(v);
	  os << u->m_name;
	  return name + os.str() + ',';
	}
	addrof* a = v->addrof_cast();
	assert(a);
	assert(!a->m_offset);
	v = a->m_ref;
	usr* u = v->usr_cast();
	return name + u->m_name + ',';
      }
    };
  } // end of namespace template_tag_impl
} // end of namespace cxx_compiler

std::string cxx_compiler::template_tag::instantiated_name() const
{
  const map<string, scope::tps_t::value_t>& table
    = templ_base::m_tps.m_table;
  const vector<string>& order = templ_base::m_tps.m_order;
  string name = m_name;
  name += '<';
  name = accumulate(begin(order), end(order), name,
		    template_tag_impl::helper(table));
  name.erase(name.size()-1);
  name += '>';
  return name;
}

namespace cxx_compiler {
  namespace partial_special_impl {
    struct helper {
      const map<string, scope::tps_t::value_t>& m_table;
      helper(const map<string, scope::tps_t::value_t>& table)
	: m_table(table) {}
      string decl(string name, const type* T)
      {
	ostringstream os;
	T->decl(os, "");
	return name + os.str() + ',';
      }
      string type_case(string name, const type* T)
      {
	tag* ptr = T->get_tag();
	if (!ptr)
	  return decl(name, T);
	if (T->m_id == type::TEMPLATE_PARAM) {
	  template_tag_impl::helper op(m_table);
	  string pn = ptr->m_name;
	  return op(name, pn);
	}
	tag::flag_t flag = ptr->m_flag;
	if (!(flag & tag::INSTANTIATE))
	  return decl(name, T);
	instantiated_tag* it = static_cast<instantiated_tag*>(ptr);
	const instantiated_tag::SEED& seed = it->m_seed;
	string sn = it->m_src->m_name;
	sn += '<';
	sn = accumulate(begin(seed), end(seed), sn, helper(m_table));
	sn.erase(sn.size()-1);
	sn += '>';
	return name + sn + ',';
      }
      string var_case(string name, var* v)
      {
	error::not_implemented();
	return name;
      }
      string operator()(string name, const scope::tps_t::val2_t& p)
      {
	const type* T = p.first;
	return T ? type_case(name, T) : var_case(name, p.second);
      }
    };
  } // end of namespace partial_special_impl
} // end of namespace cxx_compiler

std::string cxx_compiler::partial_special_tag::instantiated_name() const
{
  template_tag* tt = m_sv->m_src;
  string name = tt->m_name;
  name += '<';
  const KEY& key = m_sv->m_key;
  const map<string, scope::tps_t::value_t>& table
    = templ_base::m_tps.m_table;
  name = accumulate(begin(key), end(key), name,
		    partial_special_impl::helper(table));
  name.erase(name.size()-1);
  name += '>';
  return name;
}

cxx_compiler::usr*
cxx_compiler::template_usr::
instantiate_common(vector<scope::tps_t::val2_t*>* pv, info_t::mode_t mode)
{
  template_tag_impl::sweeper sweeper(pv, false);
  KEY key;
  transform(begin(*pv), end(*pv), back_inserter(key),
	    [](scope::tps_t::val2_t* p){ return *p; });
  table_t::const_iterator it = m_table.find(key);
  if (it != m_table.end())
    return it->second;
  it = find_if(m_table.begin(), m_table.end(),
	       [&key](const pair<KEY, usr*>& x)
	       { return template_usr_impl::match(key, x.first); });
  if (it != m_table.end())
    return m_table[key] = it->second;

  template_usr_impl::sweeper_c sweeper_c(m_tps, key);

  templ_base tmp = *this;
  template_usr_impl::sweeper_b sweeper_b(m_scope, &tmp);
  s_stack.push(info_t(this, 0, mode, key));
  tinfos.push_back(make_pair(&s_stack.top(), (template_tag::info_t*)0));
  cxx_compiler_parse();
  tinfos.pop_back();
  assert(!s_stack.empty());
  instantiated_usr* ret = s_stack.top().m_iu;
  s_stack.pop();
  assert(ret);
  assert(ret->m_src == this);
  assert(ret->m_seed == key);
  if (mode == info_t::EXPLICIT)
    assert(ret->m_flag2 & usr::EXPLICIT_INSTANTIATE);
  else {
    assert(mode == info_t::STATIC_DEF);
    assert(ret->m_flag & usr::STATIC_DEF);
  }
  return m_table[key] = ret;
}

bool
cxx_compiler::template_usr::explicit_instantiating(KEY& key)
{
  if (s_stack.empty())
    return false;
  const info_t& info = s_stack.top();
  if (info.m_mode != info_t::EXPLICIT)
    return false;
  key = info.m_key;
  return true;
}

namespace cxx_compiler {
  namespace instance_of_impl {
    scope::tps_t::val2_t
    calc(const scope::tps_t::val2_t& x, const scope::tps_t::val2_t& y)
    {
      if (const type* Tx = x.first) {
	assert(Tx->m_id == type::TEMPLATE_PARAM);
	assert(y.first);
	return y;
      }
      var* v = x.second;
      assert(v->usr_cast());
      usr* u = static_cast<usr*>(v);
      usr::flag2_t flag = u->m_flag2;
      assert(flag & usr::TEMPL_PARAM);
      assert(y.second);
      return y;
    }
    bool none_tag_case(template_usr* tu, usr* ins, templ_base::KEY& key)
    {
      string x = tu->m_name;
      string y = ins->m_name;
      if (x != y)
	return false;
      typedef instantiated_tag IT;
      scope* xs = tu->m_scope;
      IT* xit = declarations::templ::get(xs);
      if (!xit)
	return false;
      template_tag* xsrc = xit->m_src;
      scope* ys = ins->m_scope;
      IT* yit = declarations::templ::get(ys);
      if (!yit)
	return false;
      template_tag* ysrc = yit->m_src;
      if (xsrc != ysrc)
	return false;
      const IT::SEED& xseed = xit->m_seed;
      const IT::SEED& yseed = yit->m_seed;
      assert(xseed.size() == yseed.size());
      transform(begin(xseed), end(xseed), begin(yseed), back_inserter(key),
		calc);
      return true;
    }
    bool comp(scope* x, scope* y, templ_base::KEY& key)
    {
      if (x == y)
	return true;
      if (x->m_id != scope::TAG)
	return false;
      tag* xt = static_cast<tag*>(x);
      if (y->m_id != scope::TAG)
	return false;
      tag* yt = static_cast<tag*>(y);
      tag::flag_t xf = xt->m_flag;
      if (!(xf & tag::INSTANTIATE))
	return false;
      typedef instantiated_tag IT;
      IT* xit = static_cast<IT*>(xt); 
      tag::flag_t yf = yt->m_flag;
      if (!(yf & tag::INSTANTIATE))
	return false;
      IT* yit = static_cast<IT*>(yt); 
      template_tag* xsrc = xit->m_src;
      template_tag* ysrc = yit->m_src;
      if (xsrc != ysrc)
	return false;
      const IT::SEED& xseed = xit->m_seed;
      const IT::SEED& yseed = yit->m_seed;
      assert(xseed.size() == yseed.size());
      transform(begin(xseed), end(xseed), begin(yseed), back_inserter(key),
		calc);
      return comp(x->m_parent, y->m_parent, key);
    }
    bool non_func_case(template_usr* tu, usr* ins, templ_base::KEY& key)
    {
      const type* Tt = tu->m_type;
      const type* Ti = ins->m_type;
      if (Tt->m_id == type::TEMPLATE_PARAM) {
	key.push_back(scope::tps_t::val2_t(Ti, 0));
	return true;
      }
      tag* x = Tt->get_tag();
      if (!x)
	return none_tag_case(tu, ins, key);
      tag* y = Ti->get_tag();
      if (!y)
	return false;
      tag::flag_t flag = x->m_flag;
      if (flag & tag::TYPENAMED)
	return comp(x->m_parent, y->m_parent, key);
      if (flag & tag::INSTANTIATE)
	return comp(x, y, key);
      if (x == y)
	return comp(tu->m_scope, ins->m_scope, key);
      return false;
    }
  } // end of namespace instance_of_impl
} // end of namespace cxx_compiler

bool
cxx_compiler::instance_of(template_usr* tu, usr* ins, templ_base::KEY& key)
{
  const type* Tt = tu->m_type;
  if (Tt->m_id != type::FUNC)
    return instance_of_impl::non_func_case(tu, ins, key);
  const type* Ti = ins->m_type;
  if (Ti->m_id != type::FUNC)
    return false;
  typedef const func_type FT;
  FT* ftt = static_cast<FT*>(Tt);
  FT* fti = static_cast<FT*>(Ti);
  const vector<const type*>& vt = ftt->param();
  const vector<const type*>& vi = fti->param();
  if (vt.size() != vi.size())
    return false;

  const map<string, scope::tps_t::value_t>& table = tu->m_tps.m_table;
  template_usr_impl::sweeper_a sweeper_a(table);

  typedef vector<const type*>::const_iterator ITx;
  pair<ITx, ITx> ret = mismatch(begin(vt), end(vt), begin(vi),
				template_usr_impl::calc(table));
  if (ret != make_pair(end(vt), end(vi)))
    return false;

  typedef map<string, scope::tps_t::value_t>::const_iterator ITy;
  ITy py = find_if(begin(table), end(table),
		   template_usr_impl::not_decided);
  if (py != end(table)) {
    scope* x = tu->m_scope;
    if (x->m_id == scope::TAG) {
      tag* px = static_cast<tag*>(x);
      tag::flag_t fx = px->m_flag;
      if (fx & tag::INSTANTIATE) {
	instantiated_tag* itx = static_cast<instantiated_tag*>(px);
	scope* y = ins->m_scope;
	assert(y->m_id == scope::TAG);
	tag* py = static_cast<tag*>(y);
	tag::flag_t fy = py->m_flag;
	if (fy & tag::INSTANTIATE) {
	  instantiated_tag* ity = static_cast<instantiated_tag*>(py);
	  if (itx->m_src == ity->m_src) {
	    key = ity->m_seed;
	    return true;
	  }
	}
      }
    }
    return false;
  }

  const vector<string>& order = tu->m_tps.m_order;
  transform(begin(order), end(order), back_inserter(key),
	    template_usr_impl::decide(table));
  if (key.size() == 1) {
    const type* T = void_type::create();
    if (key[0].first == T) {
      if (vi.size() == 1) {
	if (vi[0] == T) {
	  assert(vt.size() == 1);
	  if (vt[0]->m_id == type::TEMPLATE_PARAM)
	    return false;
	}
      }
    }
  }
  return true;
}

const cxx_compiler::type* cxx_compiler::typenamed::action(var* v)
{
  assert(v->usr_cast());
  usr* u = static_cast<usr*>(v);
  const type* T = u->m_type;
  usr::flag_t flag = u->m_flag;
  if (flag & usr::TYPEDEF) {
    if (tag* ptr = T->get_tag())
      ptr->m_flag = tag::flag_t(ptr->m_flag | tag::TYPENAMED);
    assert(!class_or_namespace_name::before.empty());
    scope* p = class_or_namespace_name::before.back();
    if (p->m_id != scope::TAG)
      return T;
    tag* ptr = static_cast<tag*>(p);
    if (!(ptr->m_flag & tag::TEMPLATE))
      return T;
  }
  string name = u->m_name;
  tag* ptr = new tag(tag::TYPENAME, name, parse::position, 0);
  assert(!class_or_namespace_name::before.empty());
  assert(class_or_namespace_name::before.back() == ptr);
  class_or_namespace_name::before.pop_back();
  ptr->m_types.first = incomplete_tagged_type::create(ptr);
  map<string, tag*>& tags = scope::current->m_tags;
  assert(tags.find(name) == tags.end());
  tags[name] = ptr;
  scope::current->m_children.push_back(ptr);
  ptr->m_parent = scope::current;
  return action(ptr);
}

const cxx_compiler::type* cxx_compiler::typenamed::action(tag* ptr)
{
  using namespace parse::templ;
  if (!save_t::nest.empty())
    ptr->m_flag = tag::flag_t(ptr->m_flag | tag::TYPENAMED);

  if (const type* T = ptr->m_types.second)
    return T;
  return ptr->m_types.first;
}

const cxx_compiler::type*
cxx_compiler::typenamed::action(pair<usr*, tag*>* p)
{
  auto_ptr<pair<usr*, tag*> > sweeper(p);
  assert(!p->first);
  tag* ptr = p->second;
  return action(ptr);
}

namespace cxx_compiler {
  namespace declarations {
    namespace templ {
      stack<scope*> specialization::nest;
    } // end of namespace templ
  } // end of namespace declarations
} // end of namespace cxx_compiler


