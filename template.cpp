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
      bool operator()(const type* Tx, const type* Ty);
    };
    namespace calc_impl {
      typedef bool FUNC(const type*, const type*, const scope::TPSF&);
      bool template_param_case(const type* Tx, const type* Ty,
			       const scope::TPSF& tpsf)
      {
	assert(Tx->m_id == type::TEMPLATE_PARAM);
	typedef const template_param_type TP;
	TP* tp = static_cast<TP*>(Tx);
	tag* ptr = tp->get_tag();
	typedef scope::TPSF::const_iterator IT;
	IT p = find_if(begin(tpsf), end(tpsf),
		       [ptr](const pair<string, scope::TPSFV>& x)
		       { return x.second.first == ptr; });
	assert(p != end(tpsf));
	if (ptr->m_types.second)
	  return ptr->m_types.second == Ty;
	ptr->m_types.second = Ty;
	return true;
      }
      struct helper {
	const scope::TPSF& m_tpsf;
	helper(const scope::TPSF& tpsf) : m_tpsf(tpsf) {}
	bool operator()(const scope::TPSFVS& x, const scope::TPSFVS& y)
	{
	  if (x.second) {
	    assert(y.second);
	    return true;
	  }
	  const type* Tx = x.first;
	  const type* Ty = y.first;
	  calc tmp(m_tpsf);
	  return tmp(Tx, Ty);
	}
      };
      bool record_case(const type* Tx, const type* Ty,
		       const scope::TPSF& tpsf)
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
				    helper(tpsf));
	return ret == make_pair(end(xseed), end(yseed));
      }
      bool pointer_case(const type* Tx, const type* Ty,
			const scope::TPSF& tpsf)
      {
	assert(Tx->m_id == type::POINTER);
	typedef const pointer_type PT;
	PT* xpt = static_cast<PT*>(Tx);
	Tx = xpt->referenced_type();
	if (Ty->m_id != type::POINTER)
	  error::not_implemented();
	PT* ypt = static_cast<PT*>(Ty);
	Ty = ypt->referenced_type();
	calc tmp(tpsf);
	return tmp(Tx, Ty);
      }
      bool reference_case(const type* Tx, const type* Ty,
			  const scope::TPSF& tpsf)
      {
	assert(Tx->m_id == type::REFERENCE);
	typedef const reference_type RT;
	RT* xrt = static_cast<RT*>(Tx);
	if (Ty->m_id == type::REFERENCE) {
	  RT* rt = static_cast<RT*>(Ty);
	  Ty = rt->referenced_type();
	}
	Tx = xrt->referenced_type();
	calc tmp(tpsf);
	return tmp(Tx, Ty);
      }
      struct table_t : map<type::id_t, FUNC*> {
	table_t()
	{
	  (*this)[type::TEMPLATE_PARAM] = template_param_case;
	  (*this)[type::RECORD] = record_case;
	  (*this)[type::POINTER] = pointer_case;
	  (*this)[type::REFERENCE] = reference_case;
	}
      } table;
    } // end of namespace calc_impl
    bool calc::operator()(const type* Tx, const type* Ty)
    {
      calc_impl::table_t::const_iterator p =
	calc_impl::table.find(Tx->m_id);
      assert(p != calc_impl::table.end());
      return (p->second)(Tx, Ty, m_tpsf);
    }
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

namespace cxx_compiler {
  namespace templ_impl {
    usr* install(map<string, vector<usr*> >& usrs, string name,
		 const templ_base::KEY& key)
    {
      typedef map<string, vector<usr*> >::iterator ITw;
      ITw pw = usrs.find(name);
      assert(pw != usrs.end());
      vector<usr*>& v = pw->second;
      usr* ins = v.back();
      usr::flag2_t flag2 = ins->m_flag2;
      assert((flag2 & usr::INSTANTIATE) || (flag2 & usr::SPECIAL_VER));
      v.pop_back();
      assert(!v.empty());
      usr* templ = v.back();
      assert(templ->m_flag2 & usr::TEMPLATE);
      template_usr* tu = static_cast<template_usr*>(templ);
      template_usr::table_t& table = tu->m_table;
      v.pop_back();
      v.push_back(ins);
      v.push_back(templ);
      return table[key] = ins;
    }
  } // end of namespace templ_impl
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

  const scope::TPSF& tpsf = m_tps.first;
  template_usr_impl::sweeper_a sweeper_a(tpsf);

  typedef vector<const type*>::const_iterator ITx;
  ITx bn = begin(param);
  ITx ed = bn + n;
  pair<ITx, ITx> ret = mismatch(bn, ed, begin(atype),
				template_usr_impl::calc(tpsf));
  if (ret.first != ed)
    error::not_implemented();

  typedef scope::TPSF::const_iterator ITy;
  ITy py = find_if(begin(tpsf), end(tpsf),
		   template_usr_impl::not_decided);
  if (py != end(tpsf))
    error::not_implemented();

  KEY key;
  const scope::TPSS& tpss = m_tps.second;
  transform(begin(tpss), end(tpss), back_inserter(key),
	    template_usr_impl::decide(tpsf));

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
  return templ_impl::install(usrs, m_name, key);
}

void cxx_compiler::template_usr::mark(instantiated_tag* it)
{
  marked.push_back(make_pair(this, it));
}

namespace cxx_compiler {
  namespace template_usr_impl {
    struct sweeper_c : sweeper_a {
      struct helper {
	const scope::TPSF& m_tpsf;
	helper(const scope::TPSF& tpsf) : m_tpsf(tpsf) {}
	bool operator()(string name, const pair<const type*, var*>& x)
	{
	  scope::TPSF::const_iterator p = m_tpsf.find(name);
	  assert(p != m_tpsf.end());
	  const scope::TPSFV& tpsfv = p->second;
	  if (tag* ptr = tpsfv.first) {
	    const type* T = x.first;
	    assert(T);
	    ptr->m_types.second = T;
	  }
	  else {
	    scope::TPSFVS* y = tpsfv.second;
	    var* v = x.second;
	    assert(v);
	    y->second = v;
	  }
	  return true;
	}
      };
      sweeper_c(const scope::TPS& tps, const template_usr::KEY& key)
	: sweeper_a(tps.first)
      {
	const scope::TPSF& tpsf = tps.first;
	const vector<string>& v = tps.second;
	assert(v.size() == key.size());
	mismatch(begin(v), end(v), begin(key), helper(tpsf));
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
  cxx_compiler_parse();

  const map<string, vector<usr*> >& usrs = it->m_usrs;
  map<string, vector<usr*> >::const_iterator q = usrs.find(m_name);
  assert(q != usrs.end());
  const vector<usr*>& v = q->second;
  usr* ins = v.back();
  assert(ins->m_flag2 & usr::INSTANTIATE);
  return m_table[key] = ins;
}

namespace cxx_compiler {
  using namespace std;
  vector<pair<template_usr*, instantiated_tag*> > template_usr::marked; 
  void template_usr::gen()
  {
    for_each(begin(marked), end(marked),
	     [](const pair<template_usr*, instantiated_tag*>& x)
	     { x.first->instantiate_mem_fun(x.second); });
    marked.clear();
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
	inline tag* tag_action(tag* ptr, vector<pair<var*, const type*>*>* pv)
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
	inline usr* usr_action(usr* u, vector<pair<var*, const type*>*>* pv)
	{
	  assert(u->m_flag2 == usr::TEMPLATE);
	  template_usr* tu = static_cast<template_usr*>(u);
	  return tu->instantiate_explicit(pv);
	}
        pair<usr*, tag*>*
        action(pair<usr*, tag*>* x, vector<pair<var*, const type*>*>* pv)
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
  stack<pair<template_tag*, instantiated_tag*> > template_tag::s_stack;

  string instantiated_name::operator()(string name, string pn)
  {
    scope::TPSF::const_iterator p = m_tpsf.find(pn);
    assert(p != m_tpsf.end());
    const pair<tag*, scope::TPSFVS*>& x = p->second;
    if (tag* ptr = x.first) {
      const type* T = ptr->m_types.second;
      ostringstream os;
      T->decl(os, "");
      return name + os.str() + ',';
    }
    const scope::TPSFVS* y = x.second;
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

  struct special_ver_tag : tag {
    template_tag* m_src;
    template_tag::KEY m_key;
    special_ver_tag(string name, template_tag* src,
		    const template_tag::KEY& key)
      : tag(src->m_kind, name, parse::position, 0), m_src(src), m_key(key)
    { m_kind2 = SPECIAL_VER; }
  };

} // end of namespace cxx_compiler

cxx_compiler::tag*
cxx_compiler::
template_tag::common(std::vector<std::pair<var*, const type*>*>* pv,
		     bool special_ver)
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

  const scope::TPSF& tpsf = templ_base::m_tps.first;
  template_usr_impl::sweeper_a sweeper_a(tpsf);
  KEY key;
  if (pv) {
    transform(begin(*pv), end(*pv), begin(tpss), back_inserter(key),
	      template_tag_impl::calc_key(tpsf));
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
    const scope::TPSF& tpsf = templ_base::m_tps.first;
    const scope::TPSS& tpss = templ_base::m_tps.second;
    string name = m_name;
    name += '<';
    name = accumulate(begin(tpss), end(tpss), name, instantiated_name(tpsf));
    name.erase(name.size()-1);
    name += '>';
    return m_table[key] = new special_ver_tag(name, this, key);
  }

  templ_base tmp = *this;
  template_usr_impl::sweeper_b sweeper_b(m_parent, &tmp);
  s_stack.push(make_pair(this, (instantiated_tag*)0));
  cxx_compiler_parse();
  instantiated_tag* ret = s_stack.top().second;
  s_stack.pop();
  if (!ret) {
    ret = new instantiated_tag(m_kind, m_name, parse::position, m_bases, this);
    ret->m_types.first = incomplete_tagged_type::create(ret);
  }
  assert(ret->m_src == this);
  ret->m_seed = key;
  return m_table[key] = ret;
}

cxx_compiler::usr*
cxx_compiler::template_usr::
instantiate_explicit(vector<pair<var*, const type*>*>* pv)
{
  template_tag_impl::sweeper sweeper(pv);
  KEY key;
  transform(begin(*pv), end(*pv), back_inserter(key),
	    [](pair<var*, const type*>* p)
	    { return make_pair(p->second, p->first); });
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
  cxx_compiler_parse();

  map<string, vector<usr*> >& usrs = scope::current->m_usrs;
  return templ_impl::install(usrs, m_name, key);
}

bool cxx_compiler::instance_of(usr* templ, usr* ins, templ_base::KEY& key)
{
  assert(templ->m_flag2 & usr::TEMPLATE);
  template_usr* tu = static_cast<template_usr*>(templ);
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

  const scope::TPSF& tpsf = tu->m_tps.first;
  template_usr_impl::sweeper_a sweeper_a(tpsf);

  typedef vector<const type*>::const_iterator ITx;
  pair<ITx, ITx> ret = mismatch(begin(vt), end(vt), begin(vi),
				template_usr_impl::calc(tpsf));
  if (ret != make_pair(end(vt), end(vi)))
    return false;

  typedef scope::TPSF::const_iterator ITy;
  ITy py = find_if(begin(tpsf), end(tpsf),
		   template_usr_impl::not_decided);
  if (py != end(tpsf))
    return false;

  const scope::TPSS& tpss = tu->m_tps.second;
  transform(begin(tpss), end(tpss), back_inserter(key),
	    template_usr_impl::decide(tpsf));
  return true;
}

