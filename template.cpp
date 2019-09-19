#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"

void cxx_compiler::type_parameter::action(var* v)
{
  assert(v->usr_cast());
  usr* u = static_cast<usr*>(v);
  string name = u->m_name;
  map<string, tag*>& tpsf = scope::current->m_tps.first;
  typedef map<string, tag*>::const_iterator IT;
  IT p = tpsf.find(name);
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
  tpsf[name] = ptr;
  vector<string>& tpss = scope::current->m_tps.second;
  tpss.push_back(name);
}

void cxx_compiler::declarations::templ::decl_begin()
{
  const map<string, tag*>& tpsf = scope::current->m_tps.first;
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
	assert(ptr->m_template);
	template_tag* tt = static_cast<template_tag*>(ptr);
	tt->m_read = r;
      }
    } // end of namespace templ
  } // end of namespace declarations
} // end of namespace cxx_compiler

void cxx_compiler::declarations::templ::decl_end()
{
  map<string, tag*>& tpsf = scope::current->m_tps.first;
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
    tag* ptr = p.second;
    typedef vector<scope*>::reverse_iterator ITx;
    ITx q = find(rbegin(children), rend(children), ptr);
    assert(q != rend(children));
    typedef vector<scope*>::iterator ITy;
    ITy r = q.base() - 1;
    children.erase(r);
  }

  tpsf.clear();
  vector<string>& tpss = scope::current->m_tps.second;
  tpss.clear();
}

namespace cxx_compiler {
  namespace template_usr_impl {
    struct calc {
      const map<string, tag*>& m_tpsf;
      calc(const map<string, tag*>& tpsf) : m_tpsf(tpsf) {}
      bool template_param_case(var* arg, const template_param_type* tp)
      {
	const type* Ta = arg->result_type();
	tag* ptr = tp->get_tag();
	typedef map<string, tag*>::const_iterator IT;
	IT p = find_if(begin(m_tpsf), end(m_tpsf),
		       [ptr](pair<string, tag*> x){ return x.second == ptr; });
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
    inline bool not_decided(pair<string, tag*> p)
    {
      tag* ptr = p.second;
      return !ptr->m_types.second;
    }
    struct decide {
      const map<string, tag*>& m_tpsf;
      decide(const map<string, tag*>& tpsf) : m_tpsf(tpsf) {}
      const type* operator()(string name)
      {
	typedef map<string, tag*>::const_iterator IT;
	IT p = m_tpsf.find(name);
	assert(p != m_tpsf.end());
	tag* ptr = p->second;
	return ptr->m_types.second;
      }
    };
    struct sweeper {
      scope* m_current;
      scope* m_parent;
      scope* m_del;
      file_t m_file;
      fundef* m_fundef;
      vector<var*> m_garbage;
      vector<tac*> m_code;
      vector<scope*> m_before;
      const map<string, tag*>& m_tpsf;
      static bool block_or_param(scope* p)
      {
	scope::id_t id = p->m_id;
	return id == scope::BLOCK || id == scope::PARAM;
      }
      sweeper(scope* p, templ_base* q, const map<string, tag*>& tpsf)
	: m_current(scope::current), m_parent(scope::current->m_parent),
	  m_del(scope::current), m_file(parse::position),
	  m_fundef(fundef::current), m_garbage(garbage), m_code(code),
	  m_before(class_or_namespace_name::before), m_tpsf(tpsf)
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
      ~sweeper()
      {
	for (auto p : m_tpsf) {
	  tag* ptr = p.second;
	  ptr->m_types.second = 0;
	}
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

  typedef map<string, tag*>::const_iterator ITz;
  ITz pz = find_if(begin(m_tps.first), end(m_tps.first),
		   template_usr_impl::not_decided);
  if (pz != end(m_tps.first))
    error::not_implemented();

  vector<const type*> key;
  transform(begin(m_tps.second), end(m_tps.second), back_inserter(key),
	    template_usr_impl::decide(m_tps.first));
  table_t::const_iterator it = m_table.find(key);
  if (it != m_table.end())
    return it->second;

  templ_base tmp = *this;
  template_usr_impl::sweeper sweeper(m_scope, &tmp, m_tps.first);
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
	  assert(ptr->m_template);
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
	declarations::specifier_seq::info_t::s_stack = m_stack;
	if (m_ptr) {
	  for (auto p : *m_ptr)
	    delete p;
	  delete m_ptr;
	}
      }
    };
  } // end of namespace template_tag_impl
  tag* template_tag::result;
  template_tag* template_tag::instantiating;
} // end of namespace cxx_compiler

cxx_compiler::tag*
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

  const map<string, tag*>& tpsf = templ_base::m_tps.first;
  if (pv) {
    int n = pv->size();
    for (int i = 0 ; i != n ; ++i) {
      map<string, tag*>::const_iterator p = tpsf.find(tpss[i]);
      assert(p != tpsf.end());
      tag* ptr = p->second;
      const type* T = (*pv)[i]->second;
      if (!T)
	error::not_implemented();
      ptr->m_types.second = T;
    }
  }

  templ_base tmp = *this;
  template_usr_impl::sweeper sweeper2(m_parent, &tmp, tpsf);
  assert(!template_tag::result);
  assert(!template_tag::instantiating);
  template_tag::instantiating = this;
  cxx_compiler_parse();
  tag* ret = template_tag::result;
  assert(ret->m_src == this);
  template_tag::result = 0;
  template_tag::instantiating = 0;
  parse::identifier::mode = parse::identifier::new_obj;
  return ret;
}
