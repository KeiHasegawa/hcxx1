#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

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

void cxx_compiler::type_parameter::action(var* v, const type* T, bool dots)
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
    auto& def = b.m_default;
    def[name] = make_pair(T, (expressions::base*)0);
    parse::identifier::mode = parse::identifier::new_obj;
  }
  if (dots) {
    if (b.m_dots)
      error::not_implemented();
    b.m_dots = true;
  }
}

void cxx_compiler::type_parameter::action(var* v, var* id)
{
  string name = param_name(v);
  auto& tps = scope::current->m_tps;
  assert(!tps.empty());
  auto bb = tps.back();
  tps.pop_back();
  assert(!tps.empty());
  auto& b = tps.back();
  auto& table = b.m_table;
  auto p = table.find(name);
  if (p != table.end())
    error::not_implemented();
  tag* ptr = new tag(tag::CLASS, name, parse::position, 0);
  assert(class_or_namespace_name::before.back() == ptr);
  class_or_namespace_name::before.pop_back();
  template_tag* tt = new template_tag(*ptr, bb);
  tt->m_parent = scope::current;
  vector<scope*>& children = scope::current->m_children;
  children.push_back(tt);
  tt->m_types.first = template_param_type::create(tt);
  table[name].first = tt;
  vector<string>& order = b.m_order;
  order.push_back(name);
  if (id)
    error::not_implemented();
}

void cxx_compiler::
templ_parameter::action(pair<const type*, expressions::base*>* p)
{
  auto_ptr<pair<const type*, expressions::base*> > sweeper(p);
  expressions::base* expr = p->second;
  if (!expr)
    return;
  vector<scope::tps_t>& tps = scope::current->m_tps;
  assert(!tps.empty());
  scope::tps_t& b = tps.back();
  const vector<string>& order = b.m_order;
  assert(!order.empty());
  string name = order.back();
  auto& def = b.m_default;
  assert(def.find(name) == def.end());
  def[name] = make_pair((const type*)0, expr);
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
      inline template_tag* get_tt(scope* p)
      {
        if (p->m_id == scope::TAG) {
	  tag* ptr = static_cast<tag*>(p);
	  if (ptr->m_flag & tag::TEMPLATE)
	    return static_cast<template_tag*>(ptr);
	}
	instantiated_tag* it = get(p);
	return it->m_src;
      }
      static usr* ins_if_res;
      inline
      void instantiate_if1(template_usr* tu,  const template_usr::KEY& key,
			   usr::flag2_t flag2)
      {
        template_usr::KEY::const_iterator p =
          find_if(begin(key), end(key), template_param);
        if (p == end(key))
          ins_if_res = tu->instantiate(key, flag2);
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
          instantiate_if1(tu, p.first, usr::NONE2);
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
            instantiate_if1(tu, p.first, p.second->m_flag2);
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
	usr::flag_t flag = u->m_flag;
	if (flag & usr::FUNCTION) {
	  usr::flag2_t flag2 = u->m_flag2;
	  if (!(flag2 & usr::FUNCTION_DEFINITION)) {
	    auto& children = scope::current->m_children;
	    assert(!children.empty());
	    auto p = children.back();
	    if (p->m_id == scope::PARAM) {
	      p->m_parent = 0;
	      children.pop_back();
	    }
	  }
	}
        tu->m_read = r;
        if (u->m_flag & usr::STATIC_DEF) {
          scope* p = u->m_scope;
          template_tag* tt = get_tt(p);
          tt->m_static_def.push_back(tu);
	  auto& refed = tt->m_static_refed;
	  for (auto p = begin(refed) ; p != end(refed) ; ) {
	    if (instantiate_static_def(*p))
	      p = refed.erase(p);
	    else
	      ++p;
	  }
        }
        else
          after_instantiate(tu);
      }
      inline void dispatch(pair<template_tag::KEY, tag*> x, template_tag* tt)
      {
        const template_tag::KEY& key = x.first;
        tag* ptr = x.second;
        if (ptr->m_flag & tag::SPECIAL_VER) {
          tt->m_table[key] = ptr;
          return;
        }
        auto pv = new vector<scope::tps_t::val2_t*>;
        transform(begin(key), end(key), back_inserter(*pv), create);
        tt->instantiate(pv, false);
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
          tu->instantiate(key, usr::NONE2);
      }
    } // end of namespace templ
  } // end of namespace declarations
} // end of namespace cxx_compiler

void cxx_compiler::declarations::templ::decl_end()
{
  typedef map<scope*, scope*>::iterator IT;
  IT it = copied_tps.find(scope::current);
  if (it != copied_tps.end()) {
    scope* ps = it->second;
    scope::current->m_tps.clear();
    copied_tps.erase(it);
    scope::current = ps;
  }
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
  else if (last) {
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
    bool not_constant(const scope::tps_t::val2_t& v)
    {
      if (v.first)
	return false;
      var* v2 = v.second;
      if (!v2)
	return false;
      return !v2->isconstant(true);
    }
    struct sweeper_a {
      const map<string, scope::tps_t::value_t>& m_table;
      map<tag*, const type*> m_org1;
      map<scope::tps_t::val2_t*, var*> m_org2;
      sweeper_a(const map<string, scope::tps_t::value_t>& table, bool zcl)
        : m_table(table)
      {
        for (auto p : m_table) {
          scope::tps_t::value_t& x = p.second;
          if (tag* ptr = x.first) {
            m_org1[ptr] = ptr->m_types.second;
	    if (zcl)
	      ptr->m_types.second = 0;
	  }
          else {
            scope::tps_t::val2_t* y = x.second;
            m_org2[y] = y->second;
	    if (zcl)
	      y->second = 0;
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
      bool m_param;
      int m_templ_arg;
      bool m_constant_flag;
      vector<scope*> m_base_clause;
      int m_typenaming;
      vector<block*> m_orphan;
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
          m_saved(parse::member_function_body::saved),
	  m_param(parse::templ::param), m_templ_arg(parse::templ::arg),
	  m_constant_flag(expressions::constant_flag),
	  m_base_clause(parse::base_clause),
	  m_typenaming(parse::identifier::typenaming),
	  m_orphan(declarations::declarators::function::definition::
		   static_inline::orphan)
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
	parse::templ::param = false;
	parse::templ::arg = 0;
	expressions::constant_flag = false;
	parse::base_clause.clear();
	parse::identifier::typenaming = 0;
	using namespace declarations::declarators::function::definition;
	static_inline::orphan.clear();
      } 
      ~sweeper_b()
      {
	using namespace declarations::declarators::function::definition;
	static_inline::orphan = m_orphan;
	parse::identifier::typenaming = m_typenaming;
	parse::base_clause = m_base_clause;
	expressions::constant_flag = m_constant_flag;
	parse::templ::arg = m_templ_arg;
	parse::templ::param = m_param;
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
      if (x.size() != y.size())
	return false;
      typedef templ_base::KEY::const_iterator IT;
      pair<IT, IT> ret = mismatch(begin(x), end(x), begin(y), comp);
      return ret == make_pair(end(x), end(y));
    }

    struct sweeper_f : sweeper_a {
      sweeper_f(const map<string, scope::tps_t::value_t>& table)
	: sweeper_a(table, false)
      {
	sweeper_f_impl::tables.push_back(&table);
      }
      ~sweeper_f()
      {
	sweeper_f_impl::tables.pop_back();
      }
    };
  } // end of namespace template_usr_impl
  vector<const map<string, scope::tps_t::value_t>*> sweeper_f_impl::tables;
} // end of namespace cxx_compiler

namespace cxx_compiler {
  vector<template_usr::info_t> template_usr::nest;
  template_usr::template_usr(usr& u, const scope::tps_t& tps,
			     bool patch_13_2)
    : usr(u), templ_base(tps), m_patch_13_2(patch_13_2), m_decled(0),
      m_prev(0), m_next(0), m_outer(0), m_express_type(false)
  {
    m_flag2 = usr::flag2_t(m_flag2 | usr::TEMPLATE);
    if (m_flag2 & usr::HAS_DEFAULT_ARG) {
      using namespace declarations::declarators::function;
      auto p = default_arg_table.find(&u);
      assert(p != default_arg_table.end());
      const auto & v = p->second;
      default_arg_table[this] = v;
    }
    if (m_patch_13_2)
      m_flag = usr::flag_t(m_flag | usr::INLINE);
  }
  namespace template_usr_impl {
    void install_def_arg(template_usr* tu, usr* ret)
    {
      usr::flag2_t flag2 = tu->m_flag2;
      if (!(flag2 & usr::HAS_DEFAULT_ARG))
	return;
      using namespace declarations::declarators::function;
      auto it = default_arg_table.find(tu);
      assert(it != default_arg_table.end());
      const auto& v = it->second;
      assert(ret->m_flag2 & usr::HAS_DEFAULT_ARG);
      assert(default_arg_table.find(ret) == default_arg_table.end());
      default_arg_table[ret] = v;
    }
    inline bool match(vector<const type*>& atype,
		      const vector<const type*>& param)
    {
      if (atype.size() == param.size())
	return true;
      if (!atype.empty())
	return false;
      if (param.size() != 1)
	return false;
      const type* T = param.back();
      if (T->m_id != type::VOID)
	return false;
      atype.push_back(T);
      return true;
    }
    const type* ref_genaddr(var* v, const type* T)
    {
      if (T->m_id != type::REFERENCE)
	return v->result_type();
      typedef const reference_type RT;
      RT* rt = static_cast<RT*>(T);
      T = rt->referenced_type();
      genaddr* ga = v->genaddr_cast();
      if (!ga)
	return v->result_type();
      var* ref = ga->m_ref;
      const type* Ty = ref->m_type;
      if (T->m_id == Ty->m_id)
	return Ty;
      if (T->m_id == type::VARRAY && Ty->m_id == type::ARRAY)
	return Ty;
      return v->result_type();
    }
  } // end of namespace template_usr_impl
} // end of namespace cxx_compiler

cxx_compiler::usr*
cxx_compiler::template_usr::instantiate(std::vector<var*>* arg, KEY* trial)
{
  assert(m_type->m_id == type::FUNC);
  typedef const func_type FT;
  FT* ft = static_cast<FT*>(m_type);
  const vector<const type*>& param = ft->param();
  if (!arg)
    error::not_implemented();
  vector<const type*> atype;
  int n = min(arg->size(), param.size());
  transform(begin(*arg), begin(*arg)+n, begin(param),
	    back_inserter(atype), template_usr_impl::ref_genaddr);
  transform(begin(*arg)+n, end(*arg), back_inserter(atype),
            [](var* v){ return v->result_type(); });
  if (atype.size() < param.size()) {
    if (m_flag2 & usr::HAS_DEFAULT_ARG) {
      using namespace declarations::declarators::function;
      auto it = default_arg_table.find(this);
      assert(it != default_arg_table.end());
      const auto& v = it->second;
      int n = atype.size();
      for (auto p = begin(v)+n ; p != end(v) ; ++p) {
	var* v = *p;
	if (!v)
	  break;
	if (const type* T = v->m_type)
	  atype.push_back(T);
      }
    }
  }
  if (!template_usr_impl::match(atype, param))
    return 0;

  const auto& table = m_tps.m_table;
  template_usr_impl::sweeper_f sweeper_f(table);
  auto pxx = mismatch(begin(param), end(param), begin(atype),
		      template_match);
  if (pxx.first != end(param)) {
    if (trial)
      return 0;
    if (!partial_ordering::info.empty()) {
      auto& bk = partial_ordering::info.back();
      bk.second = true;
      return 0;
    }
    error::not_implemented();
  }

  typedef map<string, scope::tps_t::value_t>::const_iterator ITy;
  ITy py = find_if(begin(table), end(table),
                   template_usr_impl::not_decided);
  if (py != end(table)) {
    if (m_tps.m_dots) {
      string name = py->first;
      const auto& order = m_tps.m_order;
      assert(!order.empty());
      string bk = order.back();
      if (name != bk) {
	if (trial)
	  return 0;
	error::not_implemented();
      }
    }
  }

  KEY key;
  const vector<string>& order = m_tps.m_order;
  transform(begin(order), end(order), back_inserter(key),
            template_usr_impl::decide(table));
  if (trial) {
    *trial = key;
    return reinterpret_cast<usr*>(-1);
  }

  auto q = find_if(begin(key), end(key), template_usr_impl::not_constant); 
  if (q != end(key)) {
    if (!parse::templ::save_t::nest.empty())
      return this;
    if (instantiate_with_template_param<template_usr>())
      return this;
    var* v = q->second;
    const type* T = v->m_type;
    if (T->m_id == type::BACKPATCH)
      return this;
    error::not_implemented();
    return this;
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
  info_t info(this, 0, info_t::NONE, key);
  nest.push_back(info);
  tinfos.push_back(make_pair(&info,(template_tag::info_t*)0));
  cxx_compiler_parse();
  tinfos.pop_back();
  instantiated_usr* ret = nest.back().m_iu;
  nest.pop_back();
  assert(ret->m_src == this || ret->m_src == m_next ||
	 (m_flag2 & usr::PARTIAL_INSTANTIATED));
  assert(ret->m_seed == key);
  assert(!(ret->m_flag2 & usr::EXPLICIT_INSTANTIATE));
  if (!partial_ordering::info.empty()) {
    if (partial_ordering::info.back().first == this)
      return ret;
  }
  template_usr_impl::install_def_arg(this, ret);
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
      sweeper_c(const scope::tps_t& tps, const template_usr::KEY& key,
		bool b) : sweeper_a(tps.m_table, true)
      {
        const map<string, scope::tps_t::value_t>& table = tps.m_table;
        const vector<string>& v = tps.m_order;
	if (b) {
	  assert(v.size() <= key.size());
	  mismatch(begin(v), end(v), begin(key), helper(table));
	}
	else {
	  assert(v.size() > key.size());
	  mismatch(begin(v), begin(v)+key.size(), begin(key), helper(table));
	}
      }
    };
    struct sweeper_d : sweeper_a {
      sweeper_d* m_prev;
      static stack<template_tag*> s_stack;
      sweeper_d(template_tag* tt)
	: sweeper_a(tt->templ_base::m_tps.m_table, true), m_prev(0)
      {
	if (template_tag* prev = tt->m_prev)
	  m_prev = new sweeper_d(prev);
	s_stack.push(tt);
      }
      ~sweeper_d()
      {
	s_stack.pop();
	delete m_prev;
      }
    };
    stack<template_tag*> sweeper_d::s_stack;
  } // end of namespace template_usr_impl
} // end of namespace cxx_compiler

cxx_compiler::usr*
cxx_compiler::template_usr::instantiate(const KEY& key, usr::flag2_t flag2)
{
  table_t::const_iterator p = m_table.find(key);
  if (p != m_table.end())
    return p->second;
  p = find_if(m_table.begin(), m_table.end(),
              [&key](const pair<KEY, usr*>& x)
               { return template_usr_impl::match(key, x.first); });
  if (p != m_table.end())
    return p->second;

  template_usr_impl::sweeper_c sweeper_c(m_tps, key, true);

  templ_base tmp = *this;
  template_usr_impl::sweeper_b sweeper_b(m_decled, &tmp);
  declarations::templ::ins_if_res = 0;
  info_t::mode_t mode = (flag2 & usr::EXPLICIT_INSTANTIATE) ?
    info_t::EXPLICIT : info_t::NONE;
  info_t info(this, 0, mode, key);
  nest.push_back(info);
  tinfos.push_back(make_pair(&info,(template_tag::info_t*)0));
  cxx_compiler_parse();
  tinfos.pop_back();
  instantiated_usr* ret = nest.back().m_iu;
  nest.pop_back();
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
  template_usr_impl::install_def_arg(this, ret);
  return m_table[key] = ret;
}

cxx_compiler::var*
cxx_compiler::partial_instantiated::call(std::vector<var*>* arg)
{
  usr* ins = instantiate(arg, 0);
  return call_impl::wrapper(ins, arg, 0);
}

cxx_compiler::usr*
cxx_compiler::partial_instantiated::instantiate(vector<var*>* arg,
						KEY* trial)
{
  template_usr_impl::sweeper_c sweeper_c(m_tps, m_key, false);
  return template_usr::instantiate(arg, trial);
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
	inline bool cmp(scope::tps_t::val2_t x, scope::tps_t::val2_t* y)
	{
	  return x.first == y->first && x.second == y->second;
	}
	inline bool change_if(vector<scope::tps_t::val2_t*>* pv)
	{
	  assert(pv);
	  assert(!pv->empty());
	  auto& bk = pv->back();
	  const type* T = bk->first;
	  if (!T)
	    return true;
	  if (T->m_id != type::TEMPLATE_PARAM)
	    return true;
	  tag* ptr = T->get_tag();
	  const type* T2 = ptr->m_types.second;
	  if (!T2)
	    return true;
	  assert(!template_tag::nest.empty());
	  const auto& info = template_tag::nest.back();
	  template_tag* tt = info.m_tt;
	  bool dots = tt->templ_base::m_tps.m_dots;
	  assert(dots);
	  const auto& order = tt->templ_base::m_tps.m_order;
	  string name = ptr->m_name;
	  assert(order.back() == name);

	  // change because type_specifier_impl::get() didn't convert
	  bk->first = T2;

	  // fill if necessary
	  const auto& key = info.m_key;
	  int n = order.size();
	  --n;
	  assert(0 <= n && n < key.size());
	  auto p = find_if(begin(key)+n, end(key),
			   bind2nd(ptr_fun(cmp), bk));
	  assert(p != end(key));
	  transform(p+1, end(key), back_inserter(*pv), create);
	  return false;
	}
        inline tag*
	tag_action(tag* ptr, vector<scope::tps_t::val2_t*>* pv, bool dots)
        {
	  if (dots)
	    dots = change_if(pv);
          assert(ptr->m_flag & tag::TEMPLATE);
          template_tag* tt = static_cast<template_tag*>(ptr);
	  if (parse::base_clause.empty()) {
            int c = parse::peek();
            if (c == '{' || c == ':') {
	      set<tag*>::iterator p =
		classes::specifier::special_ver.find(ptr);
	      if (p != classes::specifier::special_ver.end()) {
		classes::specifier::special_ver.erase(p);
		return tt->special_ver(pv, dots);
	      }
	    }
            if (c == ';') {
              if (!specialization::nest.empty()) {
                if (specialization::nest.top() == scope::current) {
                  tag* ptr = tt->special_ver(pv, dots);
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
          return tt->instantiate(pv, dots);
        }
	inline void update(scope::tps_t::val2_t* p)
	{
	  var* v = p->second;
	  if (v)
	    p->second = v->rvalue();
	}
        inline usr*
	usr_action(usr* u, vector<scope::tps_t::val2_t*>* pv, bool dots)
        {
	  for_each(begin(*pv), end(*pv), update);
	  usr::flag2_t flag2 = u->m_flag2;
	  if (flag2 & usr::TEMPLATE) {
	    template_usr* tu = static_cast<template_usr*>(u);
	    return tu->instantiate_explicit(pv);
	  }
	  assert(flag2 & usr::PARTIAL_ORDERING);
	  partial_ordering* po = static_cast<partial_ordering*>(u);
	  return new explicit_po(*po, pv);
        }
	struct dup_action {
	  static map<const instantiated_tag*, vector<tag*> > m_table;
	  template_tag* m_tt;
	  const instantiated_tag* m_it;
	  dup_action(template_tag* tt, const instantiated_tag* it)
	    : m_tt(tt), m_it(it) {}
	  void operator()(const scope::tps_t::val2_t& x)
	  {
	    vector<scope::tps_t::val2_t*>* pv =
	      new vector<scope::tps_t::val2_t*>;
	    const instantiated_tag::SEED& seed = m_it->m_seed;
	    template_tag::KEY key;
	    assert(seed.size() > 0);
	    copy(begin(seed), end(seed)-1, back_inserter(key));
	    key.push_back(x);
	    transform(begin(key), end(key), back_inserter(*pv), create);
	    tag* res = m_tt->instantiate(pv, false);
	    m_table[m_it].push_back(res);
	  }
	};
	map<const instantiated_tag*, vector<tag*> > dup_action::m_table;
        pair<usr*, tag*>*
        action(pair<usr*, tag*>* x, vector<scope::tps_t::val2_t*>* pv,
               bool dots, bool dup)
        {
          bool b = parse::templ::save_t::nest.empty();
          auto_ptr<pair<usr*, tag*> > sweeper(b ? x : 0);
          if (tag* ptr = x->second) {
            assert(!x->first);
	    if (const type* T2 = ptr->m_types.second) {
	      tag* ptr2 = T2->get_tag();
	      tag::flag_t flag = ptr2->m_flag;
	      if (flag & tag::TEMPLATE)
		ptr = ptr2;
	    }
	    tag* res = tag_action(ptr, pv, dots);
	    if (dup) {
	      if (!template_tag::nest.empty()) {
		assert(res->m_flag & tag::INSTANTIATE);
		instantiated_tag* it = static_cast<instantiated_tag*>(res);
		assert(ptr->m_flag & tag::TEMPLATE);
		template_tag* tt = static_cast<template_tag*>(ptr);
		assert(it->m_src == tt);
		const instantiated_tag::SEED& seed = it->m_seed;
		int n = seed.size();
		template_tag::info_t& info = template_tag::nest.back();
		const template_tag::KEY& key = info.m_key;
		assert(mismatch(begin(seed), end(seed), begin(key))
		       == make_pair(end(seed), begin(key)+n));
		for_each(begin(key)+n, end(key), dup_action(tt, it));
	      }
	    }
            return new pair<usr*, tag*>(0, res);
          }
          else {
            usr* u = x->first;
            assert(u);
            return new pair<usr*, tag*>(usr_action(u, pv, dots), 0);
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
	const type* U = T->unqualified();
        if (U->m_id == type::TEMPLATE_PARAM) {
          string name = ptr->m_name;
          typedef map<string, scope::tps_t::value_t>::const_iterator IT;
          IT p = m_table.find(name);
          if (p == m_table.end())
            return T;
          const scope::tps_t::value_t& value = p->second;
          assert(!value.second);
          ptr = value.first;
          if (const type* T2 = ptr->m_types.second) {
	    if (T2->m_id != type::TEMPLATE_PARAM)
	      return T2;
	  }
	  return U;
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
        tag* res = tt->instantiate(conv, false);
        T = res->m_types.second;
        if (!T)
          T = res->m_types.first;
        assert(T);
        return T;
      }
      static const type* update_bbb(const scope::tps_t::val2_t& v)
      {
	const type* T = v.first;
	if (!T)
	  return 0;
	tag* ptr = T->get_tag();
	T = ptr->m_types.second;
	if (!T)
	  return 0;
	if (T->m_id == type::TEMPLATE_PARAM)
	  return 0;
	return T;
      }
      static scope::tps_t::val2_t* update_bb(const scope::tps_t::val2_t& v)
      {
	const type* T = update_bbb(v);
	assert(T);
	return new scope::tps_t::val2_t(T,0);
      }
      static pair<instantiated_tag::SEED*, template_tag*>
      update_b4(tag* ptr)
      {
	tag::flag_t flag = ptr->m_flag;
	if (flag & tag::INSTANTIATE) {
	  auto it = static_cast<instantiated_tag*>(ptr);
	  return make_pair(&it->m_seed, it->m_src);
	}
	else {
	  assert(flag & tag::SPECIAL_VER);
	  auto sv = static_cast<special_ver_tag*>(ptr);
	  return make_pair(&sv->m_key, sv->m_src);
	}
      }
      static var* update_b(var* v)
      {
	const type* T = v->m_type;
	assert(T->m_id == type::BACKPATCH);
	scope* ps = v->m_scope;
	if (ps->m_id != scope::TAG)
	  return v;
	tag* ptr = static_cast<tag*>(ps);
	const type* T1 = ptr->m_types.first;
	if (T1->m_id == type::TEMPLATE_PARAM)
	  return v;
	tag::flag_t flag = ptr->m_flag;
	tag::flag_t mask = tag::flag_t(tag::TEMPLATE | tag::DEFERED);
	if (flag & mask)
	  return v;
	auto pr = update_b4(ptr);
	auto& seed = *pr.first;
	auto x = find_if(begin(seed), end(seed), not1(ptr_fun(update_bbb)));
	if (x != end(seed))
	  return v;
	vector<scope::tps_t::val2_t*>* pv =
	  new vector<scope::tps_t::val2_t*>;
	transform(begin(seed), end(seed), back_inserter(*pv), update_bb);
	template_tag* tt = pr.second;
	tag* res = tt->instantiate(pv, false);
	assert(v->usr_cast());
	usr* u = static_cast<usr*>(v);
	string name = u->m_name;
	const auto& usrs = res->m_usrs;
	auto p = usrs.find(name);
	if (p == usrs.end()) {
	  int r = parse::identifier::base_lookup::action(name, res);
	  if (!r)
	    return v;
	  assert(r == IDENTIFIER_LEX);
	  return cxx_compiler_lval.m_var;
	}
	const vector<usr*>& vec = p->second;
	return vec.back();
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
	T = resolve_templ(T);
        var* v = p->second;
        if (!v) {
	  const type* T2 = p->first;
	  tag* ptr = T2->get_tag();
	  if (!ptr)
	    error::not_implemented();
	  if (ptr->m_kind != tag::GUESS)
	    error::not_implemented();
	  string name = ptr->m_name;
	  usr* u = new templ_param(name, T, usr::NONE, parse::position,
				   usr::TEMPL_PARAM);
	  return scope::tps_t::val2_t(0, y->second = u);
	}
	if (v->m_type->m_id == type::BACKPATCH) {
	  v = update_b(v);
	  if (v->m_type->m_id == type::BACKPATCH)
	    return scope::tps_t::val2_t(0, v);
	}
        bool discard = false;
        bool ctor_conv = false;
        using namespace expressions;
        if (!assignment::valid(T, v, &discard, &ctor_conv, 0))
          error::not_implemented();
	v = v->cast(T);
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
    };  // end of struct calc_key
    void calc_recursive(template_tag* tt, vector<scope::tps_t::val2_t*>* pv)
    {
      if (auto prev = tt->m_prev)
	calc_recursive(prev, pv);
      template_tag::KEY key;
      const auto& order = tt->templ_base::m_tps.m_order;
      const auto& table = tt->templ_base::m_tps.m_table;
      transform(begin(*pv), end(*pv), begin(order), back_inserter(key),
		template_tag_impl::calc_key(table));
    }
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
    string encode_name() const;
  };
} // end of namespace cxx_compiler

namespace cxx_compiler {
  namespace templ_parameter {
    using namespace template_usr_impl;
    string correspond(string name, template_tag* curr)
    {
      const auto& table = curr->templ_base::m_tps.m_table;
      auto p = table.find(name);
      if (p != table.end())
	return name;
      template_tag* prev = curr->m_prev;
      if (!prev)
	return name;
      string pn = correspond(name, prev);
      const auto& po = prev->templ_base::m_tps.m_order;
      auto q = find(begin(po),end(po),pn);
      if (q == end(po))
	return name;
      int n = distance(begin(po),q);
      const auto& order = curr->templ_base::m_tps.m_order;
      assert(n < order.size());
      return order[n];
    }
    var* resolve_a(usr* u)
    {
      if (sweeper_d::s_stack.empty())
	return 0;
      template_tag* tt = sweeper_d::s_stack.top();
      const auto& table = tt->templ_base::m_tps.m_table;
      string name = u->m_name;
      name = correspond(name, tt);
      auto it = table.find(name);
      assert(it != table.end());
      const auto& val = it->second;
      auto q = val.second;
      assert(q);
      var* ret = q->second;
      assert(ret);
      return ret;
    }
    var* resolve_b(usr* u)
    {
      if (sweeper_d::s_stack.empty())
	return 0;
      template_tag* tt = sweeper_d::s_stack.top();
      const auto& table = tt->templ_base::m_tps.m_table;
      scope* ps = u->m_scope;
      if (ps->m_id != scope::TAG)
	return 0;
      tag* ptag = static_cast<tag*>(ps);
      string name = ptag->m_name;
      name = correspond(name, tt);
      auto it = table.find(name);
      if (it == table.end())
	return 0;
      const auto& val = it->second;
      tag* q = val.first;
      assert(q);
      const type* T2 = q->m_types.second;
      tag* ptr2 = T2->get_tag();
      const auto& usrs = ptr2->m_usrs;
      auto r = usrs.find(u->m_name);
      if (r == usrs.end())
	return 0;
      const auto& vec = r->second;
      var* ret = vec.back();
      return ret->rvalue();
    }
    var* resolve(usr* u)
    {
      if (const type* T = u->m_type) {
	if (T->m_id == type::BACKPATCH)
	  return template_tag_impl::calc_key::update_b(u);
      }
      usr::flag2_t flag2 = u->m_flag2;
      return (flag2 & usr::TEMPL_PARAM) ? resolve_a(u) : resolve_b(u);
    }
  } // end of namespace templ_parameter
} // end of namespace cxx_compiler

namespace cxx_compiler {
  namespace template_tag_impl {
    struct get {
      const map<string, pair<const type*, expressions::base*> >& m_default;
      get(const map<string, pair<const type*, expressions::base*> >& def)
	: m_default(def) {}
      scope::tps_t::val2_t* operator()(string name)
      {
        auto p = m_default.find(name);
        if (p == m_default.end())
          error::not_implemented();
	const auto& val = p->second;
	if (const type* T = val.first)
	  return new scope::tps_t::val2_t(T, (var*)0);
	expressions::base* expr = val.second;
	var* v = expr->gen();
	v = v->rvalue();
	return new scope::tps_t::val2_t((const type*)0, v);
      }
    };
    inline void set_key(string name, const vector<string>& order,
			template_tag::KEY& key,
			const scope::tps_t::val2_t& y)
    {
      auto p = find(begin(order), end(order), name);
      assert(p != end(order));
      int n = distance(begin(order), p);
      if (n >= key.size())
	key.resize(n+1);
      if (const type* Tx = key[n].first) {
	assert(!key[n].second);
	const type* Ty = y.first;
	assert(compatible(Tx, Ty));
	return;
      }
      if (var* vx = key[n].second) {
	assert(!key[n].first);
	var* vy = y.second;
	assert(vx == vy);
	return;
      }
      key[n] = y;
    }
    struct cmp_helper {
      template_tag::KEY& m_key;
      const template_tag* m_tt;
      map<tag*, const type*>& m_table;
      cmp_helper(template_tag::KEY& key, const template_tag* tt,
		 map<tag*, const type*>& table)
        : m_key(key), m_tt(tt), m_table(table) {}
      static pair<const type*, const type*>
      update(const type* Tx, const type* Ty)
      {
	type::id_t id = Tx->m_id;
	if (id != Ty->m_id)
	  return make_pair(Tx, Ty);
	if (id == type::REFERENCE) {
	  typedef const reference_type RT;
	  RT* rtx = static_cast<RT*>(Tx);
	  RT* rty = static_cast<RT*>(Ty);
	  Tx = rtx->referenced_type();
	  Ty = rty->referenced_type();
	  return update(Tx, Ty);
	}
	if (id == type::POINTER) {
	  typedef const pointer_type PT;
	  PT* ptx = static_cast<PT*>(Tx);
	  PT* pty = static_cast<PT*>(Ty);
	  Tx = ptx->referenced_type();
	  Ty = pty->referenced_type();
	  return update(Tx, Ty);
	}
	return make_pair(Tx, Ty);
      }
      bool template_template_parameter_case(const type* Tx, const type* Ty)
      {
	tag* xtag = Tx->get_tag();
	if (!xtag)
	  return false;
	tag::flag_t flagx = xtag->m_flag;
	if (!(flagx & tag::INSTANTIATE))
	  return false;
	auto itx = static_cast<instantiated_tag*>(xtag);
	auto ttx = itx->m_src;
	string name = ttx->m_name;
	const auto& order = m_tt->templ_base::m_tps.m_order;
	auto p = find(begin(order), end(order), name);
	if (p == end(order))
	  return false;
	const auto& tbl = m_tt->templ_base::m_tps.m_table;
	auto q = tbl.find(name);
	assert(q != tbl.end());
	const auto& val = q->second;
	tag* ptr = val.first;
	if (!ptr)
	  return false;
	if (ptr != ttx)
	  return false;
	tag* ytag = Ty->get_tag();
	tag::flag_t flagy = ytag->m_flag;
	assert(flagy & tag::INSTANTIATE);
	auto ity = static_cast<instantiated_tag*>(ytag);
	auto tty = ity->m_src;
	const type* T = tty->m_types.first;
	set_key(name, order, m_key, scope::tps_t::val2_t(T,0));
	assert(m_table.find(ttx) == m_table.end());
	m_table[ttx] = T;
	const auto& xs = itx->m_seed;
	const auto& ys = ity->m_seed;
	if (xs.size() != ys.size())
	    assert(ttx->templ_base::m_tps.m_dots);
	int m = min(xs.size(), ys.size());
	auto ret = mismatch(begin(xs), begin(xs)+m, begin(ys),
			    cmp_helper(m_key, m_tt, m_table));
	if (ret != make_pair(begin(xs)+m, begin(ys)+m))
	  return false;
	copy(begin(ys)+m,end(ys),back_inserter(m_key));
	return true;
      }
      bool operator()(const scope::tps_t::val2_t& x,
                      const scope::tps_t::val2_t& y)
      {
        if (const type* Tx = x.first) {
          const type* Ty = y.first;
          assert(Tx && Ty);
	  if (template_template_parameter_case(Tx, Ty))
	    return true;
	  pair<const type*, const type*> ret = update(Tx, Ty);
	  Tx = ret.first;
	  Ty = ret.second;
          tag* xtag = Tx->get_tag();
          assert(xtag);
          map<tag*, const type*>::const_iterator p = m_table.find(xtag);
          if (p != m_table.end()) {
            const type* Tz = p->second;
            return Ty == Tz;
          }
	  string name = xtag->m_name;
	  const auto& order = m_tt->templ_base::m_tps.m_order;
	  set_key(name, order, m_key, y);
	  assert(m_table.find(xtag) == m_table.end());
	  m_table[xtag] = Ty;
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
      const template_tag* m_tt;
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
	  string name = xtag->m_name;
	  const auto& order = m_tt->templ_base::m_tps.m_order;
	  set_key(name, order, m_key, scope::tps_t::val2_t(Ty, 0));
          return true;
        }
	if (Tx->m_id == Ty->m_id) {
	  if (Tx->m_id == type::CONST) {
	    typedef const const_type CT;
	    CT* ctx = static_cast<CT*>(Tx);
	    Tx = ctx->referenced_type();
	    CT* cty = static_cast<CT*>(Ty);
	    Ty = cty->referenced_type();
	    return cmp_type(Tx, xtag, Ty);
	  }
	  if (Tx->m_id == type::VOLATILE) {
	    typedef const volatile_type VT;
	    VT* vtx = static_cast<VT*>(Tx);
	    Tx = vtx->referenced_type();
	    VT* vty = static_cast<VT*>(Ty);
	    Ty = vty->referenced_type();
	    return cmp_type(Tx, xtag, Ty);
	  }
	  if (Tx->m_id == type::RESTRICT) {
	    typedef const restrict_type RT;
	    RT* rtx = static_cast<RT*>(Tx);
	    Tx = rtx->referenced_type();
	    RT* rty = static_cast<RT*>(Ty);
	    Ty = rty->referenced_type();
	    return cmp_type(Tx, xtag, Ty);
	  }
	}
        return false;
      }
      bool cmp_tag(const tag* cx, const tag* cy)
      {
	if (cx == cy)
	  return true;
	const type* Tx = cx->m_types.first;
	if (Tx->m_id != type::TEMPLATE_PARAM)
	  return false;
	const type* Ty = cy->m_types.second;
	if (!Ty)
	  return false;
	tag* xtag = const_cast<tag*>(cx);
	m_table[xtag] = Ty;
	string name = xtag->m_name;
	const auto& order = m_tt->templ_base::m_tps.m_order;
	set_key(name, order, m_key, scope::tps_t::val2_t(Ty, 0));
	return true;
      }
      struct helper {
	cmp* m_cmp;
	helper(cmp* c) : m_cmp(c) {}
	bool operator()(const type* Tx, const type* Ty);
      };
      int num_arg(const vector<const type*>& x, const vector<const type*>& y)
      {
	assert(!x.empty());
	assert(!y.empty());
	if (x.size() == y.size())
	  return x.size();
	if (x.size() != y.size()+1)
	  return 0;
	const type* T = x.back();
	if (T->m_id != type::ELLIPSIS)
	  return 0;
	int n = x.size()-1;
	assert(n);
	return n;
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
	      assert(Ty->m_id == type::POINTER);
              PT* pty = static_cast<PT*>(Ty);
              Ty = pty->referenced_type();
              return cmp_type(Tx, Ty);
            }
            if (Tx->m_id == type::REFERENCE) {
              typedef const reference_type RT;
              RT* rtx = static_cast<RT*>(Tx);
              Tx = rtx->referenced_type();
              RT* rty = static_cast<RT*>(Ty);
	      assert(Ty->m_id == type::REFERENCE);
              Ty = rty->referenced_type();
	      if (rtx->twice() != rty->twice())
		return false;
	      return cmp_type(Tx, Ty);
            }
	    if (Tx->m_id == type::POINTER_MEMBER) {
	      typedef const pointer_member_type PM;
	      PM* pmx = static_cast<PM*>(Tx);
	      Tx = pmx->referenced_type();
	      assert(Ty->m_id == type::POINTER_MEMBER);
	      PM* pmy = static_cast<PM*>(Ty);
	      Ty = pmy->referenced_type();
	      if (!cmp_type(Tx, Ty))
		return false;
	      const tag* cx = pmx->ctag();
	      const tag* cy = pmy->ctag();
	      return cmp_tag(cx, cy);
	    }
	    if (Tx->m_id == type::FUNC) {
	      typedef const func_type FT;
	      FT* ftx = static_cast<FT*>(Tx);
	      Tx = ftx->return_type();
	      assert(Ty->m_id == type::FUNC);
	      FT* fty = static_cast<FT*>(Ty);
	      Ty = fty->return_type();
	      if (!cmp_type(Tx, Ty))
		return false;
	      const auto& px = ftx->param();
	      const auto& py = fty->param();
	      int n = num_arg(px, py);
	      if (!n)
		return 0;
	      auto ret = mismatch(begin(px), begin(px)+n, begin(py),
				  helper(this));
	      if (ret.first != begin(px)+n)
		return false;
	      return true;
	    }
	    if (Tx->m_id == type::ARRAY) {
	      typedef const array_type AT;
	      AT* atx = static_cast<AT*>(Tx);
	      Tx = atx->element_type();
	      assert(Ty->m_id == type::ARRAY);
	      AT* aty = static_cast<AT*>(Ty);
	      Ty = aty->element_type();
	      if (!cmp_type(Tx, Ty))
		return false;
	      int dx = atx->dim();
	      int dy = aty->dim();
	      return dx == dy;
	    }
	    if (Tx->m_id == type::VARRAY) {
	      typedef const varray_type VA;
	      VA* vax = static_cast<VA*>(Tx);
	      Tx = vax->element_type();
	      assert(Ty->m_id == type::VARRAY);
	      VA* vay = static_cast<VA*>(Ty);
	      Ty = vay->element_type();
	      if (!cmp_type(Tx, Ty))
		return false;
	      var* dx = vax->dim();
	      var* dy = vay->dim();
	      return cmp_var(dx, dy);
	    }
          }
	  else {
	    if (Tx->m_id == type::VARRAY) {
	      if (Ty->m_id == type::ARRAY) {
		typedef const varray_type VA;
		VA* vax = static_cast<VA*>(Tx);
		typedef const array_type AT;
		AT* aty = static_cast<AT*>(Ty);
		Tx = vax->element_type();
		Ty = aty->element_type();
		if (!cmp_type(Tx, Ty))
		  return false;
		var* vx = vax->dim();
		using namespace expressions::primary::literal;
		var* vy = integer::create(aty->dim());
		return cmp_var(vx, vy);
	      }
	    }
	    if (Tx->m_id == type::ELLIPSIS) {
	      m_key.push_back(scope::tps_t::val2_t(Ty,0));
	      return true;
	    }
	  }
          return compatible(Tx, Ty);
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
	if (xs.size() != ys.size())
	    assert(ttx->templ_base::m_tps.m_dots);
	int m = min(xs.size(), ys.size());
	auto ret = mismatch(begin(xs), begin(xs)+m, begin(ys),
			    cmp_helper(m_key, m_tt, m_table));
	if (ret != make_pair(begin(xs)+m, begin(ys)+m))
	  return false;
	copy(begin(ys)+m,end(ys),back_inserter(m_key));
	return true;
      }
      bool cmp_var(var* vx, var* vy)
      {
	assert(vx->isconstant());
	vy = vy->rvalue();
	if (!vy->isconstant())
	  return false;
	assert(vx->usr_cast());
	usr* ux = static_cast<usr*>(vx);
	usr::flag2_t flag2 = ux->m_flag2;
	if  (flag2 & usr::TEMPL_PARAM) {
	  string name = ux->m_name;
	  const auto& order = m_tt->templ_base::m_tps.m_order;
	  set_key(name, order, m_key, scope::tps_t::val2_t(0,vy));
	  return true;
	}
	return vx->value() == vy->value();
      }
      cmp(template_tag::KEY& key, const template_tag* tt)
	: m_key(key), m_tt(tt) {}
      bool operator()(const scope::tps_t::val2_t& x,
		      const scope::tps_t::val2_t* y)
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
          return cmp_var(vx, vy);
        }
      }
    };
    bool cmp::helper::operator()(const type* Tx, const type* Ty)
    {
      return m_cmp->cmp_type(Tx, Ty);
    }
    typedef map<string, pair<const type*, expressions::base*> > cmpdef_tbl;
    struct cmpdef_a {
      const cmpdef_tbl& m_def;
      cmpdef_a(const cmpdef_tbl& def) : m_def(def) {}
      bool operator()(string pn, const scope::tps_t::val2_t& v)
      {
	auto p = m_def.find(pn);
	if (p == m_def.end())
	  return false;
	const auto& x = p->second;
	if (const type* T = x.first) {
	  return T == v.first;
	}
	expressions::base* expr = x.second;
	var* y = expr->gen();
	y = y->rvalue();
	return y == v.second;
      }
    };
    struct cmpdef_b {
      const cmpdef_tbl& m_def;
      cmpdef_b(const cmpdef_tbl& def) : m_def(def) {}
      bool operator()(string pn, const scope::tps_t::val2_t* v)
      {
	auto p = m_def.find(pn);
	if (p == m_def.end())
	  return false;
	const auto x = p->second;
	if (const type* T = x.first)
	  return T == v->first;
	expressions::base* expr = x.second;
	var* y = expr->gen();
	y = y->rvalue();
	return y == v->second;
      }
    };
    struct sweeper_e {
      int m_size;
      sweeper_e() : m_size(code.size()) {}
      ~sweeper_e()
      {
	for_each(begin(code)+m_size, end(code), [](tac* p){ delete p; });
	code.resize(m_size);
      }
    };
    struct match {
      vector<scope::tps_t::val2_t*>* m_pv;
      bool m_pv_dots;
      template_tag::KEY& m_key;
      static bool last_is_templ(vector<scope::tps_t::val2_t*>* pv,
                                const partial_special_tag* ps)
      {
	if (!pv)
	  return false;
	scope::tps_t::val2_t* bk = pv->back();

	const auto& order = ps->templ_base::m_tps.m_order;
	assert(!order.empty());
	string name = order.back();
	const auto& tbl = ps->templ_base::m_tps.m_table;
	auto p = tbl.find(name);
	assert(p != tbl.end());
	const auto& val = p->second;

	if (const type* T = bk->first) {
	  if (T->m_id != type::TEMPLATE_PARAM)
	    return false;
	  tag* x = val.first;
	  assert(x);
	  tag* y = T->get_tag();
	  return x == y;
	}
	var* v = bk->second;
	assert(v->usr_cast());
	usr* u = static_cast<usr*>(v);
	usr::flag2_t flag2 = u->m_flag2;
	if (!(flag2 & usr::TEMPL_PARAM))
	  return false;
	var* x = val.second->second;
	assert(x);
	return x == u;
      }
      static bool match_x(int* n, int m, bool dots, bool pv_dots,
			  const partial_special_tag* ps,
			  vector<scope::tps_t::val2_t*>* pv)
      {
	if (dots) {
	  assert(*n);
	  --*n;
	}
	if (pv_dots) {
	  assert(m);
	  --m;
	}
	if (*n == m)
	  return true;

	if (dots) {
	  if (pv_dots) {
	    if (!last_is_templ(pv, ps))
	      return *n <= m + 1;
	  }
	  return ++*n <= m;
	}

	if (pv_dots)
	  return *n == m + 1;

	return *n == pv->size();
      }
      struct tparam_or_cp {
	template_tag::KEY& m_key;
	tparam_or_cp(template_tag::KEY& key) : m_key(key) {}
	bool operator()(scope::tps_t::val2_t* p)
	{
	  if (template_param(*p))
	    return true;
	  m_key.push_back(*p);
	  return false;
	}
      };
      match(vector<scope::tps_t::val2_t*>* pv, bool pv_dots,
	    template_tag::KEY& key)
        : m_pv(pv), m_pv_dots(pv_dots), m_key(key) {}
      bool operator()(const partial_special_tag* ps)
      {
	if (ps->m_read.m_token.empty())
	  return false; // if `ps' is now declarated, not match
        const special_ver_tag* sv = ps->m_sv;
        const template_tag::KEY& key = sv->m_key;
	if (!m_pv)
	  return key.empty();
	bool dots = ps->templ_base::m_tps.m_dots;
	int n = key.size();
	template_tag* src = sv->m_src;
	const scope::tps_t& tps = src->templ_base::m_tps;
	const auto& def = tps.m_default;
	if (!def.empty()) {
	  const auto& order = tps.m_order;
	  sweeper_e sweeper_e;
	  auto ret = mismatch(rbegin(order), rend(order),
			      rbegin(key), cmpdef_a(def));
	  int sz = distance(rbegin(order), ret.first);
	  assert(n > sz);
	  n -= sz;
	}
	int m = m_pv->size();
	if (!def.empty()) {
	  const auto& order = tps.m_order;
	  int x = min(m, (int)order.size());
	  sweeper_e sweeper_e;
	  auto ret = mismatch(rbegin(order), rbegin(order)+x,
			      rbegin(*m_pv), cmpdef_b(def));
	  int sz = distance(rbegin(order), ret.first);
	  assert(m > sz);
	  m -= sz;
	}
	if (!match_x(&n, m, dots, m_pv_dots, ps, m_pv)) {
	  m_key.clear();
	  return false;
	}
	auto ret = mismatch(begin(key), begin(key)+n, begin(*m_pv),
			    cmp(m_key, ps));
	if (ret != make_pair(begin(key)+n, begin(*m_pv)+n)) {
	  m_key.clear();
	  return false;
	}
	if (n <= m)
	  find_if(begin(*m_pv)+n,begin(*m_pv)+m, tparam_or_cp(m_key));
	const auto& order = ps->templ_base::m_tps.m_order;
	int n2 = order.size();
	if (!match_x(&n2, m_key.size(), dots, false, 0, 0)) {
	  m_key.clear();
	  return false;
	}
        return true;
      }
    };  // end of struct match
    struct cmp_ps {
      vector<scope::tps_t::val2_t*>* m_pv;
      bool m_pv_dots;
      template_tag::KEY& m_res;
      bool* m_matched;
      cmp_ps(vector<scope::tps_t::val2_t*>* pv, bool pv_dots,
	     template_tag::KEY& res, bool* matched)
	: m_pv(pv), m_pv_dots(pv_dots), m_res(res), m_matched(matched) {}
      bool operator()(partial_special_tag* x, partial_special_tag* y)
      {
	template_tag::KEY xres;
	match opx(m_pv, m_pv_dots, xres);
	bool xx = opx(x);
	template_tag::KEY yres;
	match opy(m_pv, m_pv_dots, yres);
	bool yy = opy(y);
	if (!xx && !yy)
	  return false;

	*m_matched = true;
	if (xx && !yy) {
	  m_res = xres;
	  return true;
	}

	if (!xx && yy) {
	  m_res = yres;
	  return false;
	}

	const auto& xo = x->templ_base::m_tps.m_order;
	const auto& yo = y->templ_base::m_tps.m_order;
	int xn = xo.size();
	int yn = yo.size();
	assert(xn != yn);
	if (xn < yn) {
	  m_res = xres;
	  return true;
	}

	m_res = yres;
	return false;
      }
    };
    inline bool valid(bool special_ver,
		      vector<scope::tps_t::val2_t*>* pv,
		      bool pv_dots,
		      template_tag::KEY& res)
    {
      if (!special_ver)
	return true;
      if (!pv_dots)
	return true;
      return pv->size() == res.size();
    }
    partial_special_tag* get_ps(template_tag* tt,
				vector<scope::tps_t::val2_t*>* pv,
				bool pv_dots,
				bool special_ver,
				template_tag::KEY& res)
    {
      const auto& ps = tt->m_partial_special;
      if (!ps.empty()) {
	if (ps.size() == 1) {
	  template_tag_impl::match op(pv, pv_dots, res);
	  partial_special_tag* bk = ps.back();
	  if (op(bk))
            if (valid(special_ver, pv, pv_dots, res))
	      return bk;
	}
	else {
	  bool matched = false;
	  auto it = min_element(begin(ps), end(ps),
		template_tag_impl::cmp_ps(pv, pv_dots, res, &matched));
	  if (matched) {
	    assert(it != end(ps));
	    if (valid(special_ver, pv, pv_dots, res))
	      return *it;
	  }
	}
      }
      if (template_tag* prev = tt->m_prev)
	return get_ps(prev, pv, pv_dots, special_ver, res);
      return 0;
    }
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
    struct already_p {
      template_tag* m_tt;
      const template_tag::KEY& m_key;
      already_p(template_tag* tt, const template_tag::KEY& key)
	: m_tt(tt), m_key(key) {}
      bool operator()(const template_tag::info_t& x)
      {
	if (x.m_tt != m_tt)
	  return false;
	auto p = find_if(begin(m_key), end(m_key),template_param);
	if (p != end(m_key))
	  return true;
	return x.m_key == m_key;
      }
    };
    bool already(template_tag* tt, const template_tag::KEY& key)
    {
      const auto& v = template_tag::nest;
      return find_if(begin(v), end(v), already_p(tt,key)) != end(v);
    }
    bool out_addr(const type* T)
    {
      T = T->unqualified();
      if (T->m_id == type::TEMPLATE_PARAM)
	return true;
      if (T->m_id == type::POINTER) {
	typedef const pointer_type PT;
	PT* pt = static_cast<PT*>(T);
	T = pt->referenced_type();
	return out_addr(T);
      }
      if (T->m_id == type::REFERENCE) {
	typedef const reference_type RT;
	RT* rt = static_cast<RT*>(T);
	T = rt->referenced_type();
	return out_addr(T);
      }
      if (T->m_id == type::POINTER_MEMBER) {
	typedef const pointer_member_type PM;
	PM* pm = static_cast<PM*>(T);
	T = pm->referenced_type();
	if (out_addr(T))
	  return true;
	const tag* ptr = pm->ctag();
	if (const type* T2 = ptr->m_types.second)
	  return out_addr(T2);
	T = ptr->m_types.first;
	return out_addr(T);
      }
      tag* ptr = T->get_tag();
      if (!ptr)
	return false;
      tag::kind_t kind = ptr->m_kind;
      return kind == tag::TYPENAME;
    }
    string add_special_a(string name, const type* T)
    {
      ostringstream os;
      T->decl(os, "");
      if (out_addr(T))
	os << '.' << T;
      os << ',';
      return name + os.str();
    }
    string add_special_b(string name, var* v)
    {
      if (usr* u = v->usr_cast()) {
	usr::flag2_t flag2 = u->m_flag2;
	if (flag2 & usr::TEMPL_PARAM) {
	  ostringstream os;
	  os << u->m_name << '.' << u << ',';
	  return name + os.str();
	}
      }
      assert(v->isconstant(true));
      if (v->isconstant()) {
	ostringstream os;
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
    string add_special(string s, const scope::tps_t::val2_t* p)
    {
      if (const type* T = p->first)
	return add_special_a(s, T);
      var* v = p->second;
      return add_special_b(s, v);
    }
    string
    special_name(template_tag* tt, bool dots,
		 vector<scope::tps_t::val2_t*>* pv, bool pv_dots)
    {
      if (!dots)
	return tt->instantiated_name();
      tag::flag_t flag = tt->m_flag;
      if (flag & tag::PARTIAL_SPECIAL) {
	partial_special_tag* ps = static_cast<partial_special_tag*>(tt);
	tt = ps->m_primary;
      }
      string name = tt->m_name;
      if (!pv)
	return name + "<>";
      name += '<';
      name = accumulate(begin(*pv), end(*pv), name, add_special);
      name.erase(name.size()-1);
      name += '>';
      return name;
    }
    inline scope::tps_t::val2_t conv_dup_tbl(tag* ptr)
    {
      assert(ptr->m_flag & tag::INSTANTIATE);
      const type* T = ptr->m_types.second;
      if (!T)
	T = ptr->m_types.first;
      assert(T);
      return scope::tps_t::val2_t(T,0);
    }
    inline bool copy_dup_tbl(bool pv_dots, vector<scope::tps_t::val2_t>& key)
    {
      if (!pv_dots)
	return false;
      if (key.empty())
	return false;
      const scope::tps_t::val2_t& x = key.back();
      const type* T = x.first;
      if (!T)
	return false;
      tag* ptr = T->get_tag();
      if (!ptr)
	return false;
      tag::flag_t flag = ptr->m_flag;
      if (!(flag & tag::INSTANTIATE))
	return false;
      instantiated_tag* it = static_cast<instantiated_tag*>(ptr);
      using namespace declarations::templ::id;
      typedef map<const instantiated_tag*, vector<tag*> >::iterator IT;
      IT p = dup_action::m_table.find(it);
      if (p == dup_action::m_table.end())
	return false;
      vector<tag*>& v = p->second;
      transform(begin(v), end(v), back_inserter(key), conv_dup_tbl);
      dup_action::m_table.erase(p);
      return true;
    }
    inline bool should_calc(bool pv_dots, vector<scope::tps_t::val2_t*>* pv)
    {
      if (!pv_dots)
	return true;
      assert(pv);
      if (pv->size() != 1)
	return true;
      assert(pv->size() == 1);
      auto ptr = (*pv)[0];
      const type* T = ptr->first;
      if (!T)
	return true;
      if (T->m_id != type::TEMPLATE_PARAM)
	return true;
      if (parse::templ::save_t::nest.empty())
	return false;
      auto b = parse::templ::save_t::nest.back();
      if (b->m_tag)
	return false;
      return true;
    }
  } // end of namespace template_tag_impl
} // end of namespace cxx_compiler

cxx_compiler::tag*
cxx_compiler::
template_tag::common(std::vector<scope::tps_t::val2_t*>* pv,
                     bool special_ver, bool pv_dots)
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
	if (!x.m_it) {
	  using namespace template_tag_impl;
	  bool dots = ps->templ_base::m_tps.m_dots;
	  string name = special_name(ps, dots, pv, pv_dots);
	  instantiated_tag* ptr =
	    new instantiated_tag(m_kind, name, parse::position,
				 m_bases, ps, x.m_key);
	  assert(special_ver);
	  ptr->m_flag = tag::flag_t(ptr->m_flag | tag::SPECIAL_VER);
	  ptr->m_parent = scope::current;
	  return x.m_it = ptr;
	}
      }
    }
  }

  bool dots = templ_base::m_tps.m_dots;
  const vector<string>& order = templ_base::m_tps.m_order;
  if (!pv) {
    if (!order.empty() && !dots) {
      pv = new vector<scope::tps_t::val2_t*>;
      const auto& def =	templ_base::m_tps.m_default;
      transform(begin(order), end(order), back_inserter(*pv),
		template_tag_impl::get(def));
    }
  }
  else {
    int n = pv->size();
    int m = order.size();
    if (dots) {
      if (n < m - 1) {
	const auto& def = templ_base::m_tps.m_default;
	transform(begin(order) + n, end(order) - 1, back_inserter(*pv),
		  template_tag_impl::get(def));
      }
    }
    else {
      if (n < m) {
	KEY key;
	const auto& table = templ_base::m_tps.m_table;
	template_usr_impl::sweeper_d sweeper_d(this);
	transform(begin(*pv), end(*pv), begin(order), back_inserter(key),
		  template_tag_impl::calc_key(table));
	if (m_prev)
	  template_tag_impl::calc_recursive(m_prev, pv);
	const auto& def = templ_base::m_tps.m_default;
	transform(begin(order) + n, end(order), back_inserter(*pv),
		  template_tag_impl::get(def));
      }
    }
 
    if (order.empty()) {
      assert(m_kind == tag::TEMPL);
      if (pv) {
        for_each(begin(*pv), end(*pv),
                 bind1st(ptr_fun(template_tag_impl::modify), this));
      }
    }
    if (order.size() != pv->size() && !dots)
      error::not_implemented();
  }

  KEY res;
  if (partial_special_tag* ps =
      template_tag_impl::get_ps(this, pv, pv_dots, special_ver, res)) {
    vector<scope::tps_t::val2_t*>* pv2 = new vector<scope::tps_t::val2_t*>;
    transform(begin(res), end(res), back_inserter(*pv2),
	      declarations::templ::create);
    return ps->common(pv2, special_ver, pv_dots);
  }

  const auto& table = templ_base::m_tps.m_table;
  template_usr_impl::sweeper_a sweeper_a(table, true);
  KEY key;
  if (pv) {
    if (order.size() == pv->size()) {
      if (template_tag_impl::should_calc(pv_dots, pv)) {
	transform(begin(*pv), end(*pv), begin(order), back_inserter(key),
		  template_tag_impl::calc_key(table));
      }
    }
    else {
      assert(dots);
      if (order.size() < pv->size()) {
	int n = order.size();
	transform(begin(*pv), begin(*pv)+n, begin(order), back_inserter(key),
		  template_tag_impl::calc_key(table));
	if (!pv_dots || special_ver) {
	  transform(begin(*pv)+n, end(*pv), back_inserter(key),
		    [](scope::tps_t::val2_t* p){ return *p; });
	}
      }
      else {
	assert(order.size() - 1 == pv->size());
	transform(begin(*pv), end(*pv), begin(order), back_inserter(key),
		  template_tag_impl::calc_key(table));
      }
    }
  }

  if (template_tag_impl::copy_dup_tbl(pv_dots, key)) {
    vector<scope::tps_t::val2_t*>* pv = new vector<scope::tps_t::val2_t*>;
    transform(begin(key), end(key), back_inserter(*pv),
	      declarations::templ::create);
    if (m_flag & tag::PARTIAL_SPECIAL) {
      partial_special_tag* ps = static_cast<partial_special_tag*>(this);
      template_tag* prim = ps->m_primary;
      return prim->common(pv, special_ver, false);
    }
    return common(pv, special_ver, false);
  }

  auto q = find_if(begin(key), end(key), template_usr_impl::not_constant); 
  if (q != end(key)) {
    if (!parse::templ::save_t::nest.empty())
      return this;
    if (instantiate_with_template_param<template_tag>())
      return this;
    var* v = q->second;
    const type* T = v->m_type;
    if (T->m_id == type::BACKPATCH)
      return this;
    error::not_implemented();
    return this;
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
    string name = template_tag_impl::special_name(this, dots, pv, pv_dots);
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

  if (template_tag_impl::already(this, key)) {
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
    assert(!class_or_namespace_name::before.empty());
    assert(ret == class_or_namespace_name::before.back());
    class_or_namespace_name::before.pop_back();
    return ret;
  }

  templ_base tmp = *this;
  template_usr_impl::sweeper_b sweeper_b(m_parent, &tmp);
  info_t info(this, (instantiated_tag*)0, key);
  nest.push_back(info);
  tinfos.push_back(make_pair((template_usr::info_t*)0,&info));
  cxx_compiler_parse();
  tinfos.pop_back();
  instantiated_tag* ret = nest.back().m_it;
  const type* using_type = nest.back().m_using;
  struct x {
    ~x(){ nest.pop_back(); }
  } x;
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
    ret->m_types.second = using_type;
  }
  // ret->m_src == this is almost all true except for bellow situation:
  // template<class C> using X = S<C>;
  m_table[key] = ret;
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
	  if (!T)  // case that last variable parameter is not specified
	    return name;
	  return add_special_a(name, T);
        }
        const scope::tps_t::val2_t* y = x.second;
        assert(y);
        assert(y->first);
        var* v = y->second;
	return add_special_b(name, v);
      }
    };
    inline string helper2(string name, const scope::tps_t::val2_t& x)
    {
      if (const type* T = x.first) 
	return add_special_a(name, T);
      var* v = x.second;
      return add_special_b(name, v);
    }
    inline string fill_if(string name)
    {
      if (template_tag::nest.empty())
	return name;
      const auto& info = template_tag::nest.back();
      const auto& key = info.m_key;
      if (key.empty())
	return name;
      name = accumulate(begin(key)+1, end(key), name,
			template_tag_impl::helper2);
      return name;
    }
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
  if (templ_base::m_tps.m_dots)
    name = template_tag_impl::fill_if(name);
  int n = name.size()-1;
  if (name[n] == ',')
    name.erase(n);
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
        assert(v->usr_cast());
        usr* u = static_cast<usr*>(v);
        string s = u->m_name;
        return name + s + ',';
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
  bool dots = templ_base::m_tps.m_dots;
  if (dots) {
    assert(!key.empty());
    name = accumulate(begin(key), end(key) - 1, name,
		      partial_special_impl::helper(table));
  }
  else {
    name = accumulate(begin(key), end(key), name,
		      partial_special_impl::helper(table));
  }
  name.erase(name.size()-1);
  name += '>';
  return name;
}

namespace cxx_compiler {
  namespace partial_special_impl {
      string type_case(string name, const type* T)
      {
	// just WA
	if (T->m_id == type::CONST)
	  return name + 'c';
	return name;
      }
      string var_case(string name, var* v)
      {
        assert(v->usr_cast());
        usr* u = static_cast<usr*>(v);
        string s = u->m_name;
        return name + s;
      }
      string helper2(string name, const scope::tps_t::val2_t& p)
      {
        const type* T = p.first;
        return T ? type_case(name, T) : var_case(name, p.second);
      }
  } // end of namespace partial_special_impl
} // end of namespace cxx_compiler

std::string cxx_compiler::partial_special_tag::encode_name() const
{
  template_tag* tt = m_sv->m_src;
  string name = tt->m_name;
  const KEY& key = m_sv->m_key;
  name = accumulate(begin(key), end(key), name,
		    partial_special_impl::helper2);
  return name;
}

namespace cxx_compiler {
  namespace template_usr_impl {
    struct get {
      const map<string, pair<const type*, expressions::base*> >& m_default;
      template_usr::KEY& m_key;
      get(const map<string, pair<const type*, expressions::base*> >& def,
	  template_usr::KEY& key) : m_default(def), m_key(key) {}
      bool operator()(string name)
      {
        auto p = m_default.find(name);
        if (p == m_default.end())
          return false;
	const auto& val = p->second;
	if (const type* T = val.first) {
	  T = T->complete_type();
	  m_key.push_back(scope::tps_t::val2_t(T,(var*)0));
	  return true;
	}
	expressions::base* expr = val.second;
	var* v = expr->gen();
	v = v->rvalue();
	m_key.push_back(scope::tps_t::val2_t((const type*)0, v));
	return true;
      }
    };
  } // end of namespace template_usr_impl
} // end of namespace cxx_compiler

cxx_compiler::usr*
cxx_compiler::template_usr::
instantiate_common(vector<scope::tps_t::val2_t*>* pv, info_t::mode_t mode)
{
  template_tag_impl::sweeper sweeper(pv, false);
  KEY key;
  transform(begin(*pv), end(*pv), back_inserter(key),
            [](scope::tps_t::val2_t* p){ return *p; });

  const vector<string>& order = m_tps.m_order;
  if (order.size() > key.size()) {
    template_usr_impl::sweeper_c sweeper_c(m_tps, key, false);
    find_if(begin(order)+key.size(), end(order),
	    template_usr_impl::get(m_tps.m_default, key));
  }

  auto q = find_if(begin(key), end(key), template_usr_impl::not_constant); 
  if (q != end(key)) {
    if (!parse::templ::save_t::nest.empty())
      return this;
    if (instantiate_with_template_param<template_usr>())
      return this;
    if (instantiate_with_template_param<template_tag>())
      return this;
    var* v = q->second;
    const type* T = v->m_type;
    if (T->m_id == type::BACKPATCH)
      return this;
    error::not_implemented();
    return this;
  }

  table_t::const_iterator it = m_table.find(key);
  if (it != m_table.end())
    return it->second;
  it = find_if(m_table.begin(), m_table.end(),
               [&key](const pair<KEY, usr*>& x)
               { return template_usr_impl::match(key, x.first); });
  if (it != m_table.end())
    return m_table[key] = it->second;

  if (order.size() > key.size())
    return m_table[key] = new partial_instantiated(*this, key);

  template_usr_impl::sweeper_c sweeper_c(m_tps, key, true);

  templ_base tmp = *this;
  template_usr_impl::sweeper_b sweeper_b(m_scope, &tmp);
  info_t info(this, 0, mode, key);
  nest.push_back(info);
  tinfos.push_back(make_pair(&info, (template_tag::info_t*)0));
  cxx_compiler_parse();
  tinfos.pop_back();
  assert(!nest.empty());
  instantiated_usr* ret = nest.back().m_iu;
  nest.pop_back();
  assert(ret);
  assert(ret->m_src == this || ret->m_src == m_next ||
	 (m_flag2 & usr::PARTIAL_INSTANTIATED));
  assert(ret->m_seed == key);
  if (mode == info_t::EXPLICIT) {
    if (ret->m_src == m_next)
      m_next->instantiate(key, usr::EXPLICIT_INSTANTIATE);
  }
  else {
    assert(mode == info_t::STATIC_DEF);
    assert(ret->m_flag & usr::STATIC_DEF);
  }
  return m_table[key] = ret;
}

bool
cxx_compiler::template_usr::explicit_instantiating(KEY& key)
{
  if (nest.empty())
    return false;
  const info_t& info = nest.back();
  if (info.m_mode != info_t::EXPLICIT)
    return false;
  key = info.m_key;
  return true;
}

namespace cxx_compiler {
  namespace instance_of_impl {
    struct calc {
      templ_base::KEY& m_key; 
      const vector<string>& m_order;
      calc(templ_base::KEY& key, const vector<string>& order)
	: m_key(key), m_order(order) {}
      bool operator()(const scope::tps_t::val2_t& x,
		      const scope::tps_t::val2_t& y)
      {
	using namespace template_tag_impl;
	if (const type* Tx = x.first) {
	  assert(Tx->m_id == type::TEMPLATE_PARAM);
	  assert(y.first);
	  tag* ptr = Tx->get_tag();
	  string name = ptr->m_name;
	  set_key(name, m_order, m_key, y);
	  return true;
	}
	var* v = x.second;
	assert(v->usr_cast());
	usr* u = static_cast<usr*>(v);
	usr::flag2_t flag = u->m_flag2;
	assert(flag & usr::TEMPL_PARAM);
	assert(y.second);
	string name = u->m_name;
	set_key(name, m_order, m_key, y);
	return true;
      }
    };
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
      const auto& order = tu->m_tps.m_order;
      mismatch(begin(xseed), end(xseed), begin(yseed), calc(key, order));
      return true;
    }
    bool comp(scope* x, scope* y, templ_base::KEY& key,
	      const vector<string>& order)
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
      mismatch(begin(xseed), end(xseed), begin(yseed), calc(key, order));
      return comp(x->m_parent, y->m_parent, key, order);
    }
    bool non_func_static_def(template_usr* tu, usr* ins,
			     templ_base::KEY& key)
    {
      usr::flag_t xf = tu->m_flag;
      assert(xf & usr::STATIC_DEF);
      usr::flag_t yf = ins->m_flag;
      assert(yf & usr::STATIC_DEF);
      scope* xs = tu->m_scope;
      assert(xs->m_id == scope::TAG);
      scope* ys = ins->m_scope;
      assert(ys->m_id == scope::TAG);
      tag* px = static_cast<tag*>(xs);
      tag* py = static_cast<tag*>(ys);
      tag::flag_t xt = px->m_flag;
      tag::flag_t yt = py->m_flag;
      if (xt & tag::TEMPLATE) {
	assert(yt & tag::INSTANTIATE);
	template_tag* tt = static_cast<template_tag*>(px);
	instantiated_tag* it = static_cast<instantiated_tag*>(py);
	if (it->m_src != tt)
	  return false;
	key = it->m_seed;
	return true;
      }
      const type* Tt = tu->m_type;
      Tt = Tt->unqualified();
      const type* Ti = ins->m_type;
      Ti = Ti->unqualified();
      if (Tt->m_id == type::TEMPLATE_PARAM) {
	tag* ptr = Tt->get_tag();
	string name = ptr->m_name;
	const auto& order = tu->m_tps.m_order;
	using namespace template_tag_impl;
	set_key(name, order, key, scope::tps_t::val2_t(Ti, 0));
	return comp(tu->m_scope, ins->m_scope, key, order);
      }
      tag* x = Tt->get_tag();
      if (!x)
        return none_tag_case(tu, ins, key);
      tag* y = Ti->get_tag();
      if (!y)
        return false;
      const auto& order = tu->m_tps.m_order;
      tag::flag_t flag = x->m_flag;
      if (flag & tag::TYPENAMED)
        return comp(x->m_parent, y->m_parent, key, order);
      if (flag & tag::INSTANTIATE)
        return comp(x, y, key, order);
      if (x == y)
        return comp(tu->m_scope, ins->m_scope, key, order);
      return false;
    }
    bool non_func_static(template_usr* tu, usr* ins, templ_base::KEY& key)
    {
      usr::flag_t xf = tu->m_flag;
      assert(xf & usr::STATIC);
      usr::flag_t yf = ins->m_flag;
      assert(yf & usr::STATIC);
      scope* xs = tu->m_scope;
      assert(xs->m_id == scope::TAG);
      scope* ys = ins->m_scope;
      assert(ys->m_id == scope::TAG);
      tag* px = static_cast<tag*>(xs);
      tag* py = static_cast<tag*>(ys);
      assert(px == py);
      const type* Tt = tu->m_type;
      Tt = Tt->unqualified();
      const type* Ti = ins->m_type;
      Ti = Ti->unqualified();
      assert(Tt->m_id == type::TEMPLATE_PARAM);
      tag* ptr = Tt->get_tag();
      string name = ptr->m_name;
      const auto& order = tu->m_tps.m_order;
      using namespace template_tag_impl;
      set_key(name, order, key, scope::tps_t::val2_t(Ti, 0));
      return true;
    }
    bool non_func_case(template_usr* tu, usr* ins, templ_base::KEY& key)
    {
      usr::flag_t xf = tu->m_flag;
      if (xf & usr::STATIC_DEF)
	return non_func_static_def(tu, ins, key);
      if (xf & usr::STATIC)
	return non_func_static(tu, ins, key);
      assert(xf == usr::NONE);
      usr::flag_t yf = ins->m_flag;
      assert(yf == usr::NONE);
      scope* xs = tu->m_scope;
      assert(xs->m_id == scope::TAG);
      scope* ys = ins->m_scope;
      assert(ys->m_id == scope::TAG);
      tag* px = static_cast<tag*>(xs);
      tag* py = static_cast<tag*>(ys);
      assert(px == py);
      const type* Tt = tu->m_type;
      Tt = Tt->unqualified();
      const type* Ti = ins->m_type;
      Ti = Ti->unqualified();
      assert(Tt->m_id == type::TEMPLATE_PARAM);
      tag* ptr = Tt->get_tag();
      string name = ptr->m_name;
      const auto& order = tu->m_tps.m_order;
      using namespace template_tag_impl;
      set_key(name, order, key, scope::tps_t::val2_t(Ti, 0));
      return true;
    }
    inline bool helper(template_usr* tu, usr* ins, templ_base::KEY& key)
    {
      scope* x = tu->m_scope;
      if (x->m_id != scope::TAG)
	return false;
      tag* px = static_cast<tag*>(x);
      tag::flag_t fx = px->m_flag;
      if (!(fx & tag::INSTANTIATE))
	return false;
      instantiated_tag* itx = static_cast<instantiated_tag*>(px);
      scope* y = ins->m_scope;
      assert(y->m_id == scope::TAG);
      tag* py = static_cast<tag*>(y);
      tag::flag_t fy = py->m_flag;
      if (!(fy & tag::INSTANTIATE))
	return false;
      instantiated_tag* ity = static_cast<instantiated_tag*>(py);
      if (itx->m_src != ity->m_src)
	return false;
      key = ity->m_seed;
      return true;
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

  const auto& table = tu->m_tps.m_table;
  template_usr_impl::sweeper_f sweeper_f(table);
  auto ret = mismatch(begin(vt), end(vt), begin(vi), template_match);
  if (ret.first != end(vt))
    return false;

  auto py = find_if(begin(table), end(table),
                   template_usr_impl::not_decided);
  if (py != end(table)) {
    if (instance_of_impl::helper(tu, ins, key))
      return true;
    if (!tu->m_tps.m_dots)
      return false;
    string name = py->first;
    const auto& order = tu->m_tps.m_order;
    assert(!order.empty());
    string bk = order.back();
    if (name != bk)
      return false;
  }

  const vector<string>& order = tu->m_tps.m_order;
  transform(begin(order), end(order), back_inserter(key),
            template_usr_impl::decide(table));
  if (tu->m_tps.m_dots) {
    assert(!template_usr::nest.empty());
    const auto& info = template_usr::nest.back();
    const auto& ans = info.m_key;
    assert(key.size() <= ans.size());
    auto p = mismatch(begin(key), end(key), begin(ans));
    assert(p.first == end(key));

    // Only this case, just cheating
    copy(begin(ans)+key.size(),end(ans),back_inserter(key));
  }
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

namespace cxx_compiler {
  namespace typenamed {
    inline bool should_install(scope* p)
    {
      assert(p->m_id == scope::TAG);
      tag* ptr = static_cast<tag*>(p);
      const type* T = ptr->m_types.first;
      return T->m_id != type::TEMPLATE_PARAM;
    }
  }  // end of namespace typenamed
} // end of namespace cxx_compiler

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
  auto& tags = scope::current->m_tags;
  auto p = tags.find(name);
  if (p != tags.end()) {
    tag* ptr = p->second;
    const type* T1 = ptr->m_types.first;
    const type* T2 = ptr->m_types.second;
    return T2 ? T2 : T1;
  }
  tag* ptr = new tag(tag::TYPENAME, name, parse::position, 0);
  assert(!class_or_namespace_name::before.empty());
  assert(class_or_namespace_name::before.back() == ptr);
  class_or_namespace_name::before.pop_back();
  ptr->m_types.first = incomplete_tagged_type::create(ptr);
  if (should_install(scope::current)) {
    tags[name] = ptr;
    scope::current->m_children.push_back(ptr);
    ptr->m_parent = scope::current;
  }
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

namespace cxx_compiler {
  bool template_param(const scope::tps_t::val2_t& x)
  {
    if (const type* T = x.first) {
      T = T->unqualified();
      if (T->m_id == type::POINTER) {
	typedef const pointer_type PT;
	PT* pt = static_cast<PT*>(T);
	const type* X = pt->referenced_type();
	return template_param(scope::tps_t::val2_t(X,0));
      }
      if (T->m_id == type::REFERENCE) {
	typedef const reference_type RT;
	RT* rt = static_cast<RT*>(T);
	const type* X = rt->referenced_type();
	return template_param(scope::tps_t::val2_t(X,0));
      }
      if (T->m_id == type::POINTER_MEMBER) {
	typedef const pointer_member_type PM;
	PM* pm = static_cast<PM*>(T);
	const type* X = pm->referenced_type();
	if (template_param(scope::tps_t::val2_t(X,0)))
	  return true;
	const tag* ptr = pm->ctag();
	return record_impl::should_skip(ptr);
      }
      if (T->m_id == type::TEMPLATE_PARAM)
	return true;
      if (tag* ptr = T->get_tag()) {
	if (record_impl::should_skip(ptr))
	  return true;
      }
      return false;
    }
    var* v = x.second;
    if (!v)
      return false;
    usr* u = v->usr_cast();
    if (!u)
      return false;
    usr::flag2_t flag2 = u->m_flag2;
    return flag2 & usr::TEMPL_PARAM;
  }
} // end of namespace cxx_compiler

namespace cxx_compiler {
  namespace less_than_impl {
    struct set_key {
      const map<string, scope::tps_t::value_t>& m_table;
      set_key(const map<string, scope::tps_t::value_t>& table)
	: m_table(table) {}
      bool operator()(string name, const scope::tps_t::val2_t& val)
      {
	auto p = m_table.find(name);
	assert(p != m_table.end());
	auto& v = p->second;
	if (tag* ptr = v.first) {
	  assert(!ptr->m_types.second);
	  ptr->m_types.second = val.first;
	}
	else {
	  auto y = v.second;
	  assert(!y->second);
	  y->second = val.second;
	}
	return true;
      }
    };
    int calc(template_usr* tu, const template_usr::KEY& key,
	     vector<var*>* arg)
    {
      typedef const func_type FT;
      const auto& table = tu->m_tps.m_table;
      const auto& order = tu->m_tps.m_order;
      if (order.size() != key.size())
	return numeric_limits<int>::max();
      template_usr_impl::sweeper_f sweeper_f(table);
      mismatch(begin(order), end(order), begin(key), set_key(table));
      const type* T = tu->m_type;
      T = T->instantiate();
      assert(T->m_id == type::FUNC);
      FT* ft = static_cast<FT*>(T);
      int cost = 0;
      int n = code.size();
      (void)call_impl::common(ft, tu, arg, &cost, 0, false, 0, true);
      for_each(begin(code)+n, end(code), [](tac* p){ delete p; });
      code.resize(n);
      return cost;
    }
  } // end of namespace less_than_impl

  pair<bool, int>
  less_than(template_usr* x, const template_usr::KEY& xkey,
	    template_usr* y, const template_usr::KEY& ykey,
	    vector<var*>* arg)
  {
    int cx = less_than_impl::calc(x, xkey, arg);
    int cy = less_than_impl::calc(y, ykey, arg);
    if (cx == cy)
      return make_pair(cx != numeric_limits<int>::max(), 0);
    return make_pair(true, cx < cy ? -1 : 1);
  }
} // end of namespace cxx_compiler

// Just called from debugger command line
void debug_key(const cxx_compiler::template_usr::KEY& key)
{
  using namespace std;
  using namespace cxx_compiler;
  for (auto p : key) {
    if (const type* T = p.first)
      T->decl(cout, "");
    else {
      var* v = p.second;
      usr* u = v->usr_cast();
      if (u)
	cout << u->m_name;
    }
    cout << ',';
  }
  cout << endl;
}
