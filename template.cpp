#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"

void cxx_compiler::type_parameter::action(var* v, const type* T)
{
  assert(v->usr_cast());
  usr* u = static_cast<usr*>(v);
  string name = u->m_name;
  map<string, scope::tps_t::value_t>& table = scope::current->m_tps.m_table;
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
  vector<string>& order = scope::current->m_tps.m_order;
  order.push_back(name);
  if (T) {
    map<string, const type*>& def = scope::current->m_tps.m_default;
    def[name] = T;
    parse::identifier::mode = parse::identifier::new_obj;
  }
}

void cxx_compiler::declarations::templ::decl_begin()
{
  const map<string, scope::tps_t::value_t>& table =
    scope::current->m_tps.m_table;
  if (!table.empty()) {
    using namespace parse::templ;
    save_t::s_stack.push(new save_t);
  }
}

namespace cxx_compiler {
  namespace declarations {
    namespace templ {
      inline void handle(usr* u, const parse::read_t& r)
      {
	assert(u->m_flag2 & usr::TEMPLATE);
	template_usr* tu = static_cast<template_usr*>(u);
	tu->m_read = r;
      }
      inline void handle(tag* ptr, const parse::read_t& r)
      {
	assert(ptr->m_kind2 == tag::TEMPLATE);
	template_tag* tt = static_cast<template_tag*>(ptr);
	tt->m_read = r;
      }
    } // end of namespace templ
  } // end of namespace declarations
} // end of namespace cxx_compiler

void cxx_compiler::declarations::templ::decl_end()
{
  map<string, scope::tps_t::value_t>& table = scope::current->m_tps.m_table;
  if (table.empty())
    return;

  using namespace parse::templ;
  assert(!save_t::s_stack.empty());
  save_t* p = save_t::s_stack.top();
  if (usr* u = p->m_usr)
    handle(u, p->m_read);
  else
    handle(p->m_tag, p->m_read);
  delete p;
  save_t::s_stack.pop();

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

  table.clear();
  vector<string>& order = scope::current->m_tps.m_order;
  order.clear();
  map<string, const type*>& def = scope::current->m_tps.m_default;
  def.clear();
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
	typedef map<string, scope::tps_t::value_t>::const_iterator IT;
	IT p = find_if(begin(table), end(table),
		       [ptr](const pair<string, scope::tps_t::value_t>& x)
		       { return x.second.first == ptr; });
	assert(p != end(table));
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
	  if (x.second) {
	    assert(y.second);
	    return true;
	  }
	  const type* Tx = x.first;
	  const type* Ty = y.first;
	  calc tmp(m_table);
	  return tmp(Tx, Ty);
	}
      };
      bool record_case(const type* Tx, const type* Ty,
		       const map<string, scope::tps_t::value_t>& table)
      {
	assert(Tx->m_id == type::RECORD);
	typedef const record_type REC;
	REC* xrec = static_cast<REC*>(Tx);
	if (Ty->m_id == type::REFERENCE) {
	  typedef const reference_type RT;
	  RT* rt = static_cast<RT*>(Ty);
	  Ty = rt->referenced_type();
	}
	if (Ty->m_id != type::RECORD)
	  return false;

	typedef const record_type REC;
	REC* yrec = static_cast<REC*>(Ty);
	tag* xtag = xrec->get_tag();
	tag* ytag = yrec->get_tag();
	if (xtag->m_kind2 != tag::INSTANTIATE)
	  return false;
	if (ytag->m_kind2 != tag::INSTANTIATE)
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
      scope* m_current;
      scope* m_parent;
      scope* m_del;
      file_t m_file;
      fundef* m_fundef;
      templ_base* m_ptr;
      vector<var*> m_garbage;
      vector<tac*> m_code;
      vector<scope*> m_before;
      stack<parse::templ::save_t*> m_stack;
      int m_yychar;
      static bool block_or_param(scope* p)
      {
	scope::id_t id = p->m_id;
	return id == scope::BLOCK || id == scope::PARAM;
      }
      sweeper_b(scope* p, templ_base* q)
	: m_current(scope::current), m_parent(scope::current->m_parent),
	  m_del(scope::current), m_file(parse::position),
	  m_fundef(fundef::current), m_ptr(parse::templ::ptr),
	  m_garbage(garbage), m_code(code),
	  m_before(class_or_namespace_name::before),
	  m_stack(parse::templ::save_t::s_stack), m_yychar(cxx_compiler_char)
      {
	scope::current = p;
	fundef::current = 0;
	parse::templ::ptr = q;
	if (m_parent) {
	  while (block_or_param(m_parent)) {
	    m_del = m_parent;
	    m_parent = m_parent->m_parent;
	  }
	  vector<scope*>& children = m_parent->m_children;
	  assert(children.back() == m_del);
	  children.pop_back();
	}
	garbage.clear();
	code.clear();
	class_or_namespace_name::before.clear();

	vector<scope*> tmp;
	while (p) {
	  tmp.push_back(p);
	  p = p->m_parent;
	}
	copy(rbegin(tmp), rend(tmp),
	     back_inserter(class_or_namespace_name::before));

	while (!parse::templ::save_t::s_stack.empty())
	  parse::templ::save_t::s_stack.pop();
	cxx_compiler_char = 0;
      } 
      ~sweeper_b()
      {
	cxx_compiler_char = m_yychar;
	parse::templ::save_t::s_stack = m_stack;
	class_or_namespace_name::before = m_before;
	code = m_code;
	garbage = m_garbage;
	parse::templ::ptr = m_ptr;
	fundef::current = m_fundef;
	parse::position = m_file;
	if (m_parent) {
	  vector<scope*>& children = m_parent->m_children;
	  children.push_back(m_del);
	}
	scope::current = m_current;
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
cxx_compiler::template_usr::instantiate(std::vector<var*>* arg)
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
  int a = atype.size();
  int b = param.size();
  int n = min(a, b);

  const map<string, scope::tps_t::value_t>& table = m_tps.m_table;
  template_usr_impl::sweeper_a sweeper_a(table);

  typedef vector<const type*>::const_iterator ITx;
  ITx bn = begin(param);
  ITx ed = bn + n;
  pair<ITx, ITx> pxx = mismatch(bn, ed, begin(atype),
				template_usr_impl::calc(table));
  if (pxx.first != ed)
    error::not_implemented();

  typedef map<string, scope::tps_t::value_t>::const_iterator ITy;
  ITy py = find_if(begin(table), end(table),
		   template_usr_impl::not_decided);
  if (py != end(table))
    error::not_implemented();

  KEY key;
  const vector<string>& order = m_tps.m_order;
  transform(begin(order), end(order), back_inserter(key),
	    template_usr_impl::decide(table));

  table_t::const_iterator it = m_table.find(key);
  if (it != m_table.end())
    return it->second;
  it = find_if(m_table.begin(), m_table.end(),
	       [key](const pair<KEY, usr*>& x)
	       { return template_usr_impl::match(key, x.first); });
  if (it != m_table.end())
    return m_table[key] = it->second;

  templ_base tmp = *this;
  template_usr_impl::sweeper_b sweeper_b(m_scope, &tmp);
  s_stack.push(info_t(this, 0, false));
  cxx_compiler_parse();
  instantiated_usr* ret = s_stack.top().m_iu;
  s_stack.pop();
  assert(ret->m_src == this);
  assert(ret->m_seed == key);
  assert(!(ret->m_flag2 & usr::EXPLICIT_INSTANTIATE));
  return m_table[key] = ret;
}

void cxx_compiler::template_usr::mark(instantiated_tag* it)
{
  s_marked.push_back(make_pair(this, it));
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
cxx_compiler::template_usr::instantiate_mem_fun(instantiated_tag* it)
{
  const KEY& key = it->m_seed;
  table_t::const_iterator p = m_table.find(key);
  if (p != m_table.end())
    return p->second;
  p = find_if(m_table.begin(), m_table.end(),
	      [key](const pair<KEY, usr*>& x)
	       { return template_usr_impl::match(key, x.first); });
  if (p != m_table.end())
    return p->second;

  template_usr_impl::sweeper_c sweeper_c(m_tps, key);

  templ_base tmp = *this;
  template_usr_impl::sweeper_b sweeper_b(scope::current, &tmp);
  s_stack.push(info_t(this, 0, false));
  cxx_compiler_parse();
  instantiated_usr* ret = s_stack.top().m_iu;
  s_stack.pop();
  assert(ret->m_src == this);
  assert(ret->m_seed == key);
  assert(!(ret->m_flag2 & usr::EXPLICIT_INSTANTIATE));
  return m_table[key] = ret;
}

namespace cxx_compiler {
  using namespace std;
  vector<pair<template_usr*, instantiated_tag*> > template_usr::s_marked; 
  void template_usr::gen()
  {
    for_each(begin(s_marked), end(s_marked),
	     [](const pair<template_usr*, instantiated_tag*>& x)
	     { x.first->instantiate_mem_fun(x.second); });
    s_marked.clear();
  }
} // end of namepsace cxx_compiler

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
	  if (ptr->m_kind2 != tag::TEMPLATE)
	    return ptr;
          template_tag* tt = static_cast<template_tag*>(ptr);
	  if (!parse::base_clause) {
	    int c = parse::peek();
	    if (c == '{' || c == ':')
	      return tt->special_ver(pv);
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
	  bool b = parse::templ::save_t::s_stack.empty();
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
	    assert(T);
	    return new scope::tps_t::val2_t(T, 0);
	  }
	  if (ptr->m_kind2 == tag::INSTANTIATE) {
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
	if (ptr->m_kind2 != tag::INSTANTIATE)
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
	using namespace expressions;
	if (!assignment::valid(T, v, &discard, true, 0))
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
  stack<pair<template_tag*, instantiated_tag*> > template_tag::s_stack;

  struct special_ver_tag : tag {
    template_tag* m_src;
    template_tag::KEY m_key;
    special_ver_tag(string name, template_tag* src,
		    const template_tag::KEY& key)
      : tag(src->m_kind, name, parse::position, 0), m_src(src), m_key(key)
    { m_kind2 = SPECIAL_VER; }
  };

} // end of namespace cxx_compiler

namespace cxx_compiler {
  namespace template_tag_impl {
    struct get {
      const map<string, const type*>& m_default;
      get(const map<string, const type*>& def) : m_default(def) {}
      scope::tps_t::val2_t* operator()(string name)
      {
	map<string, const type*>::const_iterator p = m_default.find(name);
	if (p == m_default.end()) {
	  for (auto x : m_default) {
	    pair<string, const type*> debug = x;
	    string s = debug.first;
	    const type* t = debug.second;
	  }
	  error::not_implemented();
	}
	const type* T = p->second;
	return new scope::tps_t::val2_t(T, 0);
      }
    };
  } // end of namespace template_tag_impl
} // end of namespace cxx_compiler

cxx_compiler::tag*
cxx_compiler::
template_tag::common(std::vector<scope::tps_t::val2_t*>* pv,
		     bool special_ver)
{
  template_tag_impl::sweeper sweeper(pv, true);
  const vector<string>& order = templ_base::m_tps.m_order;
  if (!pv) {
    if (!order.empty())
      error::not_implemented();
  }
  else {
    int n = pv->size();
    int m = order.size();
    if (n < m) {
      const map<string, const type*>& def = templ_base::m_tps.m_default;
      transform(begin(order) + n, end(order), back_inserter(*pv),
		template_tag_impl::get(def));
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
	      [key](const pair<KEY, tag*>& x)
	      { return template_usr_impl::match(key, x.first); });
  if (p != m_table.end())
    return m_table[key] = p->second;

  if (special_ver) {
    string name = instantiated_name();
    return m_table[key] = new special_ver_tag(name, this, key);
  }

  templ_base tmp = *this;
  template_usr_impl::sweeper_b sweeper_b(m_parent, &tmp);
  s_stack.push(make_pair(this, (instantiated_tag*)0));
  cxx_compiler_parse();
  instantiated_tag* ret = s_stack.top().second;
  s_stack.pop();
  if (!ret) {
    string name = instantiated_name();
    ret = new instantiated_tag(m_kind, name, parse::position, m_bases, this);
    map<string, tag*>& tags = scope::current->m_tags;
    assert(tags.find(name) == tags.end());
    tags[name] = ret;
    scope::current->m_children.push_back(ret);
    ret->m_parent = scope::current;
    ret->m_types.first = incomplete_tagged_type::create(ret);
  }
  assert(ret->m_src == this);
  ret->m_seed = key;
  return m_table[key] = ret;
}

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
	if (!v->isconstant(true))
	  error::not_implemented();
	ostringstream os;
	if (v->isconstant()) {
	  assert(v->usr_cast());
	  os << v->value();
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

cxx_compiler::usr*
cxx_compiler::template_usr::
instantiate_explicit(vector<scope::tps_t::val2_t*>* pv)
{
  template_tag_impl::sweeper sweeper(pv, false);
  KEY key;
  transform(begin(*pv), end(*pv), back_inserter(key),
	    [](scope::tps_t::val2_t* p){ return *p; });
  table_t::const_iterator it = m_table.find(key);
  if (it != m_table.end())
    return it->second;
  it = find_if(m_table.begin(), m_table.end(),
	       [key](const pair<KEY, usr*>& x)
	       { return template_usr_impl::match(key, x.first); });
  if (it != m_table.end())
    return m_table[key] = it->second;

  template_usr_impl::sweeper_c sweeper_c(m_tps, key);

  templ_base tmp = *this;
  template_usr_impl::sweeper_b sweeper_b(m_scope, &tmp);
  s_stack.push(info_t(this, 0, true));
  cxx_compiler_parse();
  instantiated_usr* ret = s_stack.top().m_iu;
  s_stack.pop();
  assert(ret->m_src == this);
  assert(ret->m_seed == key);
  assert(ret->m_flag2 & usr::EXPLICIT_INSTANTIATE);
  return m_table[key] = ret;
}

bool
cxx_compiler::instance_of(template_usr* tu, usr* ins, templ_base::KEY& key)
{
  const type* Tt = tu->m_type;
  if (Tt->m_id != type::FUNC)
    return false;
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
  if (py != end(table))
    return false;

  const vector<string>& order = tu->m_tps.m_order;
  transform(begin(order), end(order), back_inserter(key),
	    template_usr_impl::decide(table));
  return true;
}

