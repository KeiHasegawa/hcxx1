#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"

void cxx_compiler::type_parameter::action(var* v)
{
  assert(v->usr_cast());
  usr* u = static_cast<usr*>(v);
  string name = u->m_name;
  scope::TPSF& tpsf = scope::current->m_tps.first;
  scope::TPSF::const_iterator p = tpsf.find(name);
  if (p != tpsf.end())
    error::not_implemented();
  tag* ptr = new tag(tag::CLASS, name, parse::position, 0);
  assert(class_or_namespace_name::before.back() == ptr);
  class_or_namespace_name::before.pop_back();
  ptr->m_parent = scope::current;
  vector<scope*>& children = scope::current->m_children;
  children.push_back(ptr);
  const type* T = template_param_type::create(ptr);
  ptr->m_types.first = T;
  tpsf[name].first = ptr;
  vector<string>& tpss = scope::current->m_tps.second;
  tpss.push_back(name);
}

void cxx_compiler::declarations::templ::decl_begin()
{
  const scope::TPSF& tpsf = scope::current->m_tps.first;
  if (!tpsf.empty()) {
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
  scope::TPSF& tpsf = scope::current->m_tps.first;
  if (tpsf.empty())
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
  for (const auto& p : tpsf) {
    if (tag* ptr = p.second.first) {
      typedef vector<scope*>::reverse_iterator ITx;
      ITx q = find(rbegin(children), rend(children), ptr);
      assert(q != rend(children));
      typedef vector<scope*>::iterator ITy;
      ITy r = q.base() - 1;
      children.erase(r);
    }
  }

  tpsf.clear();
  vector<string>& tpss = scope::current->m_tps.second;
  tpss.clear();
}

namespace cxx_compiler {
  namespace template_usr_impl {
    struct calc {
      const scope::TPSF& m_tpsf;
      calc(const scope::TPSF& tpsf) : m_tpsf(tpsf) {}
      bool template_param_case(var* arg, const template_param_type* tp)
      {
	const type* Ta = arg->result_type();
	tag* ptr = tp->get_tag();
	typedef scope::TPSF::const_iterator IT;
	IT p = find_if(begin(m_tpsf), end(m_tpsf),
		       [ptr](const pair<string, scope::TPSFV>& x)
		       { return x.second.first == ptr; });
	assert(p != end(m_tpsf));
	if (ptr->m_types.second)
	  return ptr->m_types.second == Ta;
	ptr->m_types.second = Ta;
	return true;
      }
      bool operator()(var* arg, const type* T)
      {
	if (T->m_id == type::TEMPLATE_PARAM) {
	  typedef const template_param_type TP;
	  TP* tp = static_cast<TP*>(T);
	  return template_param_case(arg, tp);
	}
	error::not_implemented();
	return false;
      }
    };
    inline bool not_decided(const pair<string, pair<tag*, scope::TPSFVS*> >& p)
    {
      const pair<tag*, scope::TPSFVS*>& x = p.second;
      tag* ptr = x.first;
      if (ptr)
	return !ptr->m_types.second;
      scope::TPSFVS* y = x.second;
      assert(y);
      assert(y->first);
      return !y->second;
    }
    struct decide {
      const scope::TPSF& m_tpsf;
      decide(const scope::TPSF& tpsf) : m_tpsf(tpsf) {}
      pair<const type*, var*> operator()(string name)
      {
	typedef scope::TPSF::const_iterator IT;
	IT p = m_tpsf.find(name);
	assert(p != m_tpsf.end());
	const pair<tag*, scope::TPSFVS*>& x = p->second;
	if (tag* ptr = x.first) {
	  const type* T = ptr->m_types.second;
	  return make_pair(T, (usr*)0);
	}
	scope::TPSFVS* y = x.second;
	assert(y);
	assert(y->first);
	return make_pair((const type*)0, y->second);
      }
    };
    struct sweeper_a {
      const scope::TPSF& m_tpsf;
      sweeper_a(const scope::TPSF& tpsf) : m_tpsf(tpsf) {}
      ~sweeper_a()
      {
	for (auto p : m_tpsf) {
	  scope::TPSFV& x = p.second;
	  if (tag* ptr = x.first)
	    ptr->m_types.second = 0;
	  else {
	    scope::TPSFVS* y = x.second;
	    y->second = 0;
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
      vector<var*> m_garbage;
      vector<tac*> m_code;
      vector<scope*> m_before;
      static bool block_or_param(scope* p)
      {
	scope::id_t id = p->m_id;
	return id == scope::BLOCK || id == scope::PARAM;
      }
      sweeper_b(scope* p, templ_base* q)
	: m_current(scope::current), m_parent(scope::current->m_parent),
	  m_del(scope::current), m_file(parse::position),
	  m_fundef(fundef::current), m_garbage(garbage), m_code(code),
	  m_before(class_or_namespace_name::before)
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
      } 
      ~sweeper_b()
      {
	class_or_namespace_name::before = m_before;
	code = m_code;
	garbage = m_garbage;
	fundef::current = m_fundef;
	parse::position = m_file;
	if (m_parent) {
	  vector<scope*>& children = m_parent->m_children;
	  children.push_back(m_del);
	}
	scope::current = m_current;
	parse::templ::ptr = 0;
      }
    };
    bool comp(const scope::TPSFVS& x, const scope::TPSFVS& y)
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

cxx_compiler::usr*
cxx_compiler::template_usr::instantiate(std::vector<var*>* arg)
{
  if (!arg)
    error::not_implemented();
  assert(m_type->m_id == type::FUNC);
  typedef const func_type FT;
  FT* ft = static_cast<FT*>(m_type);
  const vector<const type*>& param = ft->param();
  int a = arg->size();
  int b = param.size();
  int n = min(a, b);

  typedef vector<var*>::const_iterator ITx;
  typedef vector<const type*>::const_iterator ITy;
  ITx b1 = begin(*arg);
  ITx e1 = begin(*arg) + n;

  pair<ITx, ITy> ret = mismatch(b1, e1, begin(param),
				template_usr_impl::calc(m_tps.first));
  if (ret.first != e1)
    error::not_implemented();

  typedef scope::TPSF::const_iterator ITz;
  ITz pz = find_if(begin(m_tps.first), end(m_tps.first),
		   template_usr_impl::not_decided);
  if (pz != end(m_tps.first))
    error::not_implemented();

  template_usr_impl::sweeper_a sweeper_a(m_tps.first);

  KEY key;
  transform(begin(m_tps.second), end(m_tps.second), back_inserter(key),
	    template_usr_impl::decide(m_tps.first));
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
  cxx_compiler_parse();

  map<string, vector<usr*> >& usrs = scope::current->m_usrs;
  typedef map<string, vector<usr*> >::iterator ITw;
  ITw pw = usrs.find(m_name);
  assert(pw != usrs.end());
  vector<usr*>& v = pw->second;
  usr* ins = v.back();
  v.pop_back();
  assert(!v.empty());
  usr* templ = v.back();
  assert(templ->m_flag2 & usr::TEMPLATE);
  v.pop_back();
  v.push_back(ins);
  v.push_back(templ);
  return m_table[key] = ins;
}

namespace cxx_compiler {
  namespace declarations {
    namespace templ {
      namespace id {
	tag* action(tag* ptr, vector<pair<var*, const type*>*>* pv)
	{
	  assert(ptr->m_kind2 == tag::TEMPLATE);
          template_tag* tt = static_cast<template_tag*>(ptr);
	  return tt->instantiate(pv);
	}
      } // end of namespace id
    } // end of namespace templ
  } // end of namespace declarations
} // end of namespace cxx_compiler

namespace cxx_compiler {
  namespace template_tag_impl {
    struct sweeper {
      vector<pair<var*, const type*>*>* m_ptr;
      stack<declarations::specifier_seq::info_t*> m_stack;
      sweeper(vector<pair<var*, const type*>*>* pv) : m_ptr(pv)
      {
	stack<declarations::specifier_seq::info_t*>& x =
	  declarations::specifier_seq::info_t::s_stack;
	m_stack = x;
	while (!x.empty())
	  x.pop();
      }
      ~sweeper()
      {
	parse::identifier::mode = parse::identifier::new_obj;
	declarations::specifier_seq::info_t::s_stack = m_stack;
	if (m_ptr) {
	  for (auto p : *m_ptr)
	    delete p;
	  delete m_ptr;
	}
      }
    };
    struct calc_key {
      const scope::TPSF& m_tpsf;
      calc_key(const scope::TPSF& tpsf) : m_tpsf(tpsf) {}
      pair<const type*, var*>
      operator()(pair<var*, const type*>* p, string name)
      {
	scope::TPSF::const_iterator it = m_tpsf.find(name);
	assert(it != m_tpsf.end());
	const pair<tag*, scope::TPSFVS*>& x = it->second;
	if (tag* ptr = x.first) {
	  const type* T = p->second;
	  assert(T);
	  ptr->m_types.second = T;
	  return make_pair(T, (usr*)0);
	}
	scope::TPSFVS* y = x.second;
	assert(y);
	const type* T = y->first;
	assert(T);
	var* v = p->first;
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
	return make_pair((const type*)0, v);
      }
    };
  } // end of namespace template_tag_impl
  instantiated_tag* template_tag::result;
  template_tag* template_tag::instantiating;
} // end of namespace cxx_compiler

cxx_compiler::instantiated_tag*
cxx_compiler::
template_tag::instantiate(std::vector<std::pair<var*, const type*>*>* pv)
{
  template_tag_impl::sweeper sweeper(pv);
  const vector<string>& tpss = templ_base::m_tps.second;
  if (!pv) {
    if (!tpss.empty())
      error::not_implemented();
  }
  else {
    if (tpss.size() != pv->size())
      error::not_implemented();
  }

  KEY key;
  const scope::TPSF& tpsf = templ_base::m_tps.first;
  if (pv) {
    transform(begin(*pv), end(*pv), begin(tpss), back_inserter(key),
	      template_tag_impl::calc_key(tpsf));
  }

  template_usr_impl::sweeper_a sweeper_a(tpsf);

  table_t::const_iterator p = m_table.find(key);
  if (p != m_table.end())
    return p->second;
  p = find_if(m_table.begin(), m_table.end(),
	      [key](const pair<KEY, tag*>& x)
	      { return template_usr_impl::match(key, x.first); });
  if (p != m_table.end())
    return m_table[key] = p->second;

  templ_base tmp = *this;
  template_usr_impl::sweeper_b sweeper_b(m_parent, &tmp);
  assert(!template_tag::result);
  assert(!template_tag::instantiating);
  template_tag::instantiating = this;
  cxx_compiler_parse();
  instantiated_tag* ret = template_tag::result;
  assert(ret->m_src == this);
  ret->m_seed = key;
  template_tag::result = 0;
  template_tag::instantiating = 0;
  return m_table[key] = ret;
}
