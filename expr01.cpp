// postfix-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "cxx_y.h"
#include "patch.03.q"

namespace cxx_compiler {
  namespace subscript_impl {
    var* size(const type*);
  }
} // end of namespace subscript_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::var::subscripting(var* y)
{
  using namespace std;

  if (var* ret = var_impl::operator_code('[', this, y))
    return ret;

  var* array = rvalue();
  var* index = y->rvalue();
  if (!index->m_type->integer())
    swap(array,index);
  if (!index->m_type->integer()) {
    using namespace error::expressions::postfix::subscripting;
    not_integer(parse::position,index);
    return array;
  }
  const type* T = array->m_type;
  T = T->unqualified();
  if (T->m_id != type::POINTER) {
    using namespace error::expressions::postfix::subscripting;
    not_pointer(parse::position,array);
    return array;
  }
  typedef const pointer_type PT;
  PT* pt = static_cast<PT*>(T);
  pt = static_cast<PT*>(pt->complete_type());
  T = pt->referenced_type();
  var* size = subscript_impl::size(T);
  if ( !size ){
    using namespace error::expressions::postfix::subscripting;
    not_object(parse::position,T);
    return array;
  }
  conversion::arithmetic::gen(&size, &index);
  var* offset = size->mul(index);
  assert(offset->m_type->integer());
  return array->offref(T,offset);
}

cxx_compiler::var* cxx_compiler::subscript_impl::size(const type* T)
{
  using namespace std;
  using namespace expressions::primary::literal;
  if ( int size = T->size() )
    return integer::create(size);
  else
    return T->vsize();
}

cxx_compiler::var* cxx_compiler::genaddr::subscripting(var* y)
{
  var* index = y->rvalue();
  if ( !index->m_type->integer() ){
    using namespace error::expressions::postfix::subscripting;
    not_integer(parse::position,index);
    return this;
  }
  const type* T = m_ref->m_type;
  switch ( T->m_id ){
  case type::ARRAY:
    {
      typedef const array_type ARRAY;
      ARRAY* array = static_cast<ARRAY*>(T);
      T = array->element_type();
      break;
    }
  case type::VARRAY:
    {
      typedef const varray_type VARRAY;
      VARRAY* varray = static_cast<VARRAY*>(T);
      T = varray->element_type();
      break;
    }
  default:
    using namespace error::expressions::postfix::subscripting;
    not_object(parse::position,T);
    return this;
  }
  var* size = subscript_impl::size(T);
  if ( !size ){
    using namespace error::expressions::postfix::subscripting;
    not_object(parse::position,T);
    return this;
  }
  conversion::arithmetic::gen(&size, &index);
  var* offset = size->mul(index);
  assert(offset->m_type->integer());
  return offref(T,offset);
}

namespace cxx_compiler {
  namespace expressions {
    var* dup(scope::tps_t::val2_t x, base* b)
    {
      assert(!x.second);
      const type* T = x.first;
      assert(T);

      assert(!template_usr::nest.empty());
      auto& info = template_usr::nest.back();
      template_usr* tu = info.m_tu;
      assert(tu->m_tps.m_dots);
      const auto& order = tu->m_tps.m_order;
      assert(!order.empty());
      string name = order.back();
      const auto& table = tu->m_tps.m_table;
      auto p = table.find(name);
      assert(p != table.end());
      const auto& v = p->second;
      assert(!v.second);
      tag* ptr = v.first;
      const type* org = ptr->m_types.second;
      ptr->m_types.second = T;
      var* ret = b->gen();
      ptr->m_types.second = org;
      return ret;
    }
    void gen_arg(const expr_list* exprs, vector<var*>& arg)
    {
      if (!exprs->m_dots) {
	transform(begin(*exprs), end(*exprs), back_inserter(arg),
		  mem_fun(&base::gen));
	return;
      }
      assert(!exprs->empty());
      transform(begin(*exprs), end(*exprs)-1, back_inserter(arg),
		mem_fun(&base::gen));
      if (template_usr::nest.empty())
	return;
      auto& info = template_usr::nest.back();
      template_usr* tu = info.m_tu;
      assert(tu->m_tps.m_dots);
      const auto& order = tu->m_tps.m_order;
      int n = order.size();
      const auto& key = info.m_key;
      assert(n <= key.size());
      const auto& x = key[n-1];
      assert(!x.second);
      const type* T = x.first;
      if (!T)
	return;
      base* b = exprs->back();
      var* v = b->gen();
      arg.push_back(v);
      if (n == key.size())
	return;
      transform(begin(key)+n, end(key), back_inserter(arg),
		bind2nd(ptr_fun(dup), b));
    }
    bool match(scope::tps_t::val2_t x, tag* ptr)
    {
      assert(!x.second);
      const type* T = x.first;
      assert(T);
      return T->get_tag() == ptr;
    }
    scope::tps_t::val2_t* conv(scope::tps_t::val2_t x, tag* ptr)
    {
      assert(!x.second);
      const type* T = x.first;
      if (T->get_tag() == ptr) {
	T = ptr->m_types.second;
	assert(T);
      }
      return new scope::tps_t::val2_t(T, (var*)0);
    }
    var* change_if(var* func)
    {
      usr* u = func->usr_cast();
      if (!u)
	return func;
      usr::flag2_t flag2 = u->m_flag2;
      if (!(flag2 & usr::INSTANTIATE))
	return func;
      auto iu = static_cast<instantiated_usr*>(u);
      const auto& seed = iu->m_seed;
      if (template_usr::nest.empty())
	return func;
      auto& info = template_usr::nest.back();
      template_usr* tu = info.m_tu;
      if (!tu->m_tps.m_dots)
	return func;
      const auto& order = tu->m_tps.m_order;
      assert(!order.empty());
      string name = order.back();
      const auto& table = tu->m_tps.m_table;
      auto p = table.find(name);
      assert(p != table.end());
      const auto& v = p->second;
      assert(!v.second);
      tag* ptr = v.first;
      assert(ptr);
      auto q = find_if(begin(seed), end(seed), bind2nd(ptr_fun(match),ptr));
      if (q == end(seed))
	return func;
      template_usr* src = iu->m_src;
      auto pv = new vector<scope::tps_t::val2_t*>;
      transform(begin(seed), end(seed), back_inserter(*pv),
		bind2nd(ptr_fun(conv),ptr));
      return src->instantiate_explicit(pv);
    }
  } // end of namespace expressions
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::expressions::postfix::call::gen()
{
  using namespace std;
  var* func = m_func->gen();
  func = change_if(func);
  vector<var*> arg;
  if (m_arg)
    gen_arg(m_arg, arg);
  return func->call(&arg);
}

const cxx_compiler::file_t&
cxx_compiler::expressions::postfix::call::file() const
{
  return m_func->file();
}

cxx_compiler::expressions::postfix::call::~call()
{
  using namespace std;
  delete m_func;
  if ( m_arg ) {
    for (auto p : *m_arg)
      delete p;
  }
  delete m_arg;
}

namespace cxx_compiler {
  namespace var_impl {
    var* funop_code(var* fun, vector<var*>* arg)
    {
      const type* T = fun->result_type();
      if (!T)
        return 0;
      usr* op_fun = operator_function(T, '(');
      if (!op_fun)
        return 0;
      usr::flag2_t flag2 = op_fun->m_flag2;
      if (flag2 & usr::TEMPLATE) {
        template_usr* templ = static_cast<template_usr*>(op_fun);
        op_fun = templ->instantiate(arg, 0);
      }
      return call_impl::wrapper(op_fun, arg, fun);
    }
  } // end of namespace var_impl
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::var::call(std::vector<var*>* arg)
{
  using namespace std;

  if (var* ret = var_impl::funop_code(this, arg))
    return ret;

  var* func = rvalue();
  const type* T = func->m_type;
  T = T->unqualified();
  if (T->m_id == type::POINTER) {
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(T);
    T = pt->referenced_type();
  }
  if (T->m_id == type::TEMPLATE_PARAM)
    return this;
  if (T->m_id == type::FUNC) {
    typedef const func_type FT;
    FT* ft = static_cast<FT*>(T);
    return call_impl::common(ft, func, arg, 0, 0, false, 0);
  }
  using namespace error::expressions::postfix::call;
  not_function(parse::position,func);
  return func;
}

cxx_compiler::var*
cxx_compiler::genaddr::call(std::vector<var*>* arg)
{
  using namespace std;
  assert(m_ref->usr_cast());
  usr* u = static_cast<usr*>(m_ref);
  usr::flag2_t flag2 = u->m_flag2;
  if (flag2 & usr::TEMPLATE) {
    template_usr* tu = static_cast<template_usr*>(u);
    u = tu->instantiate(arg, 0);
    (void)instantiate_if(u);
  }
  else
    (void)instantiate_if(u);
  const type* T = u->m_type;
  if (T->m_id != type::FUNC) {
    using namespace error::expressions::postfix::call;
    not_function(parse::position,m_ref);
    return rvalue();
  }
  typedef const func_type FT;
  FT* ft = static_cast<FT*>(T);
  usr::flag_t flag = u->m_flag;
  scope* fun_scope = u->m_scope;
  var* this_ptr = 0;
  if (fun_scope->m_id == scope::TAG) {
    if (!(flag & usr::STATIC)) {
      int r = parse::identifier::lookup(this_name, scope::current);
      if (!r)
        error::not_implemented();
      assert(r == IDENTIFIER_LEX);
      this_ptr = cxx_compiler_lval.m_var;
    }
    if (this_ptr) {
      scope* this_parent = this_ptr->m_scope->m_parent;
      if (fun_scope != this_parent) {
        tag* b = static_cast<tag*>(fun_scope);
        const type* Tb = b->m_types.second;
        assert(Tb);
        const type* pTb = pointer_type::create(Tb);
        this_ptr = this_ptr->cast(pTb);
      }
    }
  }
  return call_impl::common(ft, u, arg, 0, this_ptr, m_qualified_func, 0);
}

cxx_compiler::var*
cxx_compiler::member_function::call(std::vector<var*>* arg)
{
  using namespace std;
  var* fun = m_fun;
  auto_ptr<member_function> sweeper(this);
  const type* T = fun->m_type;
  if (T->m_id == type::POINTER) {
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(T);
    T = pt->referenced_type();
  }
  assert(T->m_id == type::FUNC); 
  typedef const func_type FT;
  FT* ft = static_cast<FT*>(T);
  if (usr* u = fun->usr_cast()) {
    fun = instantiate_if(u);
    if (fun == u) {
      usr::flag2_t flag2 = u->m_flag2;
      if (flag2 & usr::TEMPLATE) {
        template_usr* tu = static_cast<template_usr*>(u);
        fun = tu->instantiate(arg, 0);
        if (template_usr* next = tu->m_next) {
          assert(fun->usr_cast());
          usr* u = static_cast<usr*>(fun);
          assert(u->m_flag2 & usr::INSTANTIATE);
          instantiated_usr* iu = static_cast<instantiated_usr*>(u);
          const instantiated_usr::SEED& seed = iu->m_seed;
          next->instantiate(seed, usr::NONE2);
        }
        T = fun->m_type;
        assert(T->m_id == type::FUNC); 
        ft = static_cast<FT*>(T);
      }
    }
  }
  return call_impl::common(ft, fun, arg, 0, m_obj,
			   m_qualified_func, m_vftbl_off);
}

namespace cxx_compiler {
  var* fun_ptr_mem(tag* ptr, usr* fun)
  {
    using namespace expressions::primary::literal;
    const type* T = fun->m_type;
    const pointer_type* pt = pointer_type::create(T);
    var* tmp = new var(pt);
    T = pointer_member_type::create(ptr, T);
    var* ret = new var(T);
    if (scope::current->m_id == scope::BLOCK) {
      block* b = static_cast<block*>(scope::current);
      b->m_vars.push_back(tmp);
      b->m_vars.push_back(ret);
    }
    else {
      garbage.push_back(tmp);
      garbage.push_back(ret);
    }
    code.push_back(new addr3ac(tmp, fun));
    var* idx = integer::create(0);
    code.push_back(new loff3ac(ret, idx, tmp));
    idx = integer::create(pt->size());
    if (fun->m_flag & usr::VIRTUAL) {
      const map<string, vector<usr*> >& usrs = ptr->m_usrs;
      typedef map<string, vector<usr*> >::const_iterator IT;
      IT p = usrs.find(vftbl_name);
      assert(p != usrs.end());
      const vector<usr*>& v = p->second;
      assert(v.size() == 1);
      usr* u = v.back();
      assert(u->m_flag & usr::WITH_INI);
      with_initial* vftbl = static_cast<with_initial*>(u);
      map<int, var*>::const_iterator q =
        find_if(begin(vftbl->m_value), end(vftbl->m_value),
                bind2nd(ptr_fun(match_vf),fun));
      assert(q != end(vftbl->m_value));
      int offset = q->first;
      var* off = integer::create(offset);
      code.push_back(new loff3ac(ret, idx, off));
    }
    else {
      var* off = integer::create(-1);
      code.push_back(new loff3ac(ret, idx, off));
    }
    return ret;
  }
} // end of namespace cxx_compiler

cxx_compiler::var*
cxx_compiler::member_function::rvalue()
{
  scope* p = m_fun->m_scope;
  assert(p->m_id == scope::TAG);
  tag* ptr = static_cast<tag*>(p);
  assert(m_fun->usr_cast());
  usr* fun = static_cast<usr*>(m_fun);
  fun = instantiate_if(fun);
  return fun_ptr_mem(ptr, fun);
}

namespace cxx_compiler {
  namespace member_function_impl {
    inline tag* decled_tag(tag* ptr,  usr* fun)
    {
      const map<string, vector<usr*> >& usrs = ptr->m_usrs;
      string fn = fun->m_name;
      map<string, vector<usr*> >::const_iterator p = usrs.find(fn);
      if (p != usrs.end())
        return ptr;
      scope* fs = fun->m_scope;
      assert(fs->m_id == scope::TAG);
      tag* fst = static_cast<tag*>(fs);
      const map<string, tag*>& tags = ptr->m_tags;
      string tn = fst->m_name;
      map<string, tag*>::const_iterator q = tags.find(tn);
      if (q != tags.end()) {
        ptr = q->second;
        return decled_tag(ptr, fun);
      }
      return 0;
    }
    inline bool helper(usr* u, usr* fun)
    {
      if (!(u->m_flag2 & usr::TEMPLATE))
        return false;
      template_usr* tu = static_cast<template_usr*>(u);
      templ_base::KEY key;
      return instance_of(tu, fun, key);
    }
    template_usr* has_templ(const pair<template_tag::KEY, tag*>& x, usr* fun)
    {
      const template_tag::KEY& key = x.first;
      template_tag::KEY::const_iterator p =
        find_if(begin(key), end(key), not1(ptr_fun(template_param)));
      if (p != end(key))
        return 0;
      tag* ptr = decled_tag(x.second, fun);
      if (!ptr)
        return 0;
      map<string, vector<usr*> >& usrs = ptr->m_usrs;
      string name = fun->m_name;
      map<string, vector<usr*> >::const_iterator q = usrs.find(name);
      assert(q != usrs.end());
      const vector<usr*>& v = q->second;
      usr* u = v.back();
      usr::flag2_t flag2 = u->m_flag2;
      if (flag2 & usr::TEMPLATE)
        return static_cast<template_usr*>(u);
      usr::flag_t flag = u->m_flag;
      if (flag & usr::OVERLOAD) {
        overload* ovl = static_cast<overload*>(u);
        vector<usr*>& c = ovl->m_candidacy;
        typedef vector<usr*>::const_iterator IT;
        IT p = find_if(begin(c), end(c), bind2nd(ptr_fun(helper), fun));
        if (p != end(c)) {
          usr* u = *p;
          return static_cast<template_usr*>(u);
        }
      }
      return 0;
    }
  } // end of namespace member_function_impl
  instantiated_tag* get_it(tag* ptr)
  {
    if (ptr->m_flag & tag::INSTANTIATE)
      return static_cast<instantiated_tag*>(ptr);
    scope* parent = ptr->m_parent;
    if (!(parent->m_id & scope::TAG))
      return 0;
    ptr = static_cast<tag*>(parent);
    return get_it(ptr);
  }
} // end of namespace cxx_compiler

cxx_compiler::var*
cxx_compiler::instantiated_usr::call(std::vector<var*>* arg)
{
  assert(!declarations::templ::nested_gened);
  (void)instantiate_if(this);
  if (auto tu = declarations::templ::nested_gened) {
    tu->instantiate(m_seed, usr::EXPLICIT_INSTANTIATE);
    declarations::templ::nested_gened = 0;
  }
  return usr::call(arg);
}

cxx_compiler::usr* cxx_compiler::instantiate_if(usr* fun)
{
  assert(fun);
  scope* ps = fun->m_scope;
  if (ps->m_id != scope::TAG)
    return fun;
  tag* ptr = static_cast<tag*>(ps);
  auto it = get_it(ptr);
  if (!it)
    return fun;
  template_tag* tt = it->m_src;
  const template_tag::table_t& tbl = tt->m_table;
  typedef template_tag::table_t::const_iterator IT;
  using namespace member_function_impl;
  template_usr* tu = 0;
  IT p = find_if(begin(tbl), end(tbl),
                 [fun, &tu](const pair<template_tag::KEY, tag*>& x)
                 { return tu = has_templ(x, fun); } );
  if (p == end(tbl))
    return fun;
  if (template_usr* outer = tu->m_outer) {
    usr* ret = outer->instantiate(it->m_seed, usr::NONE2);
    return ret ? ret : fun;
  }
  return tu->instantiate(it->m_seed, usr::NONE2);
}

namespace cxx_compiler {
  namespace overload_impl {
    using namespace std;
    var* do_trial(usr* u, vector<var*>* arg, var* obj,
                  vector<vector<tac*> >& tmp, vector<int>& cost)
    {
      using namespace std;
      usr::flag2_t flag2 = u->m_flag2;
      if (flag2 & usr::TEMPLATE) {
        template_usr* tu = static_cast<template_usr*>(u);
        template_usr::KEY key;
        if (tu->instantiate(arg, &key))
  	  u = tu->instantiate(key, usr::NONE2);
      }
      const type* T = u->m_type;
      assert(T->m_id == type::FUNC);
      typedef const func_type FT;
      FT* ft = static_cast<FT*>(T);
      int n = code.size();
      int m = 0;
      var* ret = call_impl::common(ft, u, arg, &m, obj, false, 0);
      cost.push_back(m);
      tmp.resize(tmp.size()+1);
      copy(begin(code)+n, end(code), back_inserter(tmp.back()));
      code.resize(n);
      return ret;
    }
    int chose(const vector<var*>& res, const vector<int>& cost)
    {
      auto ok = [](var* v){ return v; };
      typedef vector<var*>::const_iterator ITx;
      ITx p = find_if(begin(res), end(res), ok);
      if (p == end(res))
        return -1;
      ITx q = find_if(p+1, end(res), ok);
      if (q == end(res))
        return p - begin(res);

      typedef vector<int>::const_iterator ITy;
      ITy r = min_element(begin(cost), end(cost));
      assert(r != end(cost));
      int min_cost = *r;
      int n = count_if(begin(cost), end(cost),
                       [min_cost](int c){ return c == min_cost; });
      if (n != 1) {
	if (parse::templ::save_t::nest.empty())
	  error::not_implemented();
      }
      return r - begin(cost);
    }
  } // end of namespace overload_impl
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::overload::call(std::vector<var*>* arg)
{
  return call(arg, 0);
}

cxx_compiler::var* cxx_compiler::overload::call(std::vector<var*>* arg,
                				int* ind)
{
  using namespace std;
  using namespace overload_impl;
  const vector<usr*>& cand = m_candidacy;
  var* obj = m_obj;
  vector<var*> res;
  vector<vector<tac*> > tmp;
  struct sweeper {
    vector<vector<tac*> >& m_code;
    sweeper(vector<vector<tac*> >& v) : m_code(v) {}
    ~sweeper()
    {
      for (auto &v : m_code)
        for (auto p : v)
          delete p;
    }
  } sweeper(tmp);
  vector<int> cost;
  transform(begin(cand), end(cand), back_inserter(res),
  [arg, obj, &tmp, &cost](usr* u){ return do_trial(u, arg, obj, tmp, cost); });
  int m = chose(res, cost);
  if (m < 0) {
    using namespace error::expressions::postfix::call;
    overload_not_match(this);
    var* ret = new var(int_type::create());
    if (scope::current->m_id == scope::BLOCK) {
      block* b = static_cast<block*>(scope::current);
      b->m_vars.push_back(ret);
    }
    else
      garbage.push_back(ret);
    return ret;
  }
  if (ind)
    *ind = m;
  var* ret = res[m];
  assert(ret);
  vector<tac*>& v = tmp[m];
  usr* u = cand[m];
  u = instantiate_if(u);
  usr::flag_t flag = u->m_flag;
  if ((flag & usr::STATIC) && m_obj) {
    assert(v.size() >= 2);
    tac* p0 = v[0];
    assert(p0->m_id == tac::ADDR);
    assert(p0->y == obj);
    tac* p1 = v[1];
    assert(p1->m_id == tac::PARAM);
    assert(p1->y == p0->x);
    v.erase(begin(v), begin(v)+2);
  }
  copy(begin(v), end(v), back_inserter(code));
  v.clear();
  return ret;
}

namespace cxx_compiler {
  namespace partial_ordering_impl {
    struct help {
      int* m_res;
      help(int* res) : m_res(res) {}
      bool none_tag(const type* Tx, const type* Ty)
      {
	if (Tx == Ty)
	  return true;
	type::id_t idx = Tx->m_id;
	type::id_t idy = Ty->m_id;
	if (idx == type::POINTER && idy != type::POINTER) {
	  *m_res = 1;
	  return false;
	}
	if (idx != type::POINTER && idy == type::POINTER) {
	  *m_res = -1;
	  return false;
	}
	if (idx == type::POINTER && idy == type::POINTER) {
	  typedef const pointer_type PT;
	  PT* ptx = static_cast<PT*>(Tx);
	  PT* pty = static_cast<PT*>(Ty);
	  Tx = ptx->referenced_type();
	  Ty = pty->referenced_type();
	  return subr(Tx, Ty);
	}
	if (idx == type::REFERENCE && idy != type::REFERENCE) {
	  *m_res = 1;
	  return false;
	}
	if (idx != type::REFERENCE && idy == type::REFERENCE) {
	  *m_res = -1;
	  return false;
	}
	if (idx == type::REFERENCE && idy == type::REFERENCE) {
	  typedef const reference_type RT;
	  RT* rtx = static_cast<RT*>(Tx);
	  RT* rty = static_cast<RT*>(Ty);
	  Tx = rtx->referenced_type();
	  Ty = rty->referenced_type();
	  return subr(Tx, Ty);
	}
	if (idx == type::CONST && idy != type::CONST) {
	  *m_res = 1;
	  return false;
	}
	if (idx != type::CONST && idy == type::CONST) {
	  *m_res = -1;
	  return false;
	}
	if (idx == type::CONST && idy == type::CONST) {
	  typedef const const_type CT;
	  CT* ctx = static_cast<CT*>(Tx);
	  CT* cty = static_cast<CT*>(Ty);
	  Tx = ctx->referenced_type();
	  Ty = cty->referenced_type();
	  return subr(Tx, Ty);
	}
	if (idx == type::VOLATILE && idy != type::VOLATILE) {
	  *m_res = 1;
	  return false;
	}
	if (idx != type::VOLATILE && idy == type::VOLATILE) {
	  *m_res = -1;
	  return false;
	}
	if (idx == type::VOLATILE && idy == type::VOLATILE) {
	  typedef const volatile_type VT;
	  VT* vtx = static_cast<VT*>(Tx);
	  VT* vty = static_cast<VT*>(Ty);
	  Tx = vtx->referenced_type();
	  Ty = vty->referenced_type();
	  return subr(Tx, Ty);
	}
	if (idx == type::ARRAY && idy != type::ARRAY) {
	  *m_res = 1;
	  return false;
	}
	if (idx != type::ARRAY && idy == type::ARRAY) {
	  *m_res = -1;
	  return false;
	}
	if (idx == type::ARRAY && idy == type::ARRAY) {
	  typedef const array_type AT;
	  AT* atx = static_cast<AT*>(Tx);
	  AT* aty = static_cast<AT*>(Ty);
	  int dx = atx->dim();
	  int dy = aty->dim();
	  assert(dx == dy);
	  Tx = atx->element_type();
	  Ty = aty->element_type();
	  return subr(Tx, Ty);
	}
	error::not_implemented();
	return true;
      }
      bool subr(const type* Tx, const type* Ty)
      {
	tag* px = Tx->get_tag();
	tag* py = Ty->get_tag();
	if (!px) {
	  if (!py)
	    return none_tag(Tx, Ty);
	  tag::flag_t yflag = py->m_flag;
	  if (yflag & tag::INSTANTIATE) {
	    *m_res = -1;
	    return false;
	  }
	  return true;
	}
	tag::flag_t xflag = px->m_flag;
	if (!py) {
	  if (xflag & tag::INSTANTIATE) {
	    *m_res = 1;
	    return false;
	  }
	  return true;
	}
	tag::flag_t yflag = py->m_flag;
	if (xflag & tag::INSTANTIATE) {
	  if (!(yflag & tag::INSTANTIATE)) {
	    *m_res = 1;
	    return false;
	  }
	  return true;
	}
	if (yflag & tag::INSTANTIATE) {
	  *m_res = -1;
	  return false;
	}
	return true;
      }
      bool subr(var*, var*)
      {
	error::not_implemented();
	return true;
      }
      typedef scope::tps_t::val2_t V;
      bool operator()(const V& x, const V& y)
      {
	if (const type* Tx = x.first) {
	  const type* Ty = y.first;
	  assert(Ty);
	  return subr(Tx, Ty);
	}
	else {
	  var* vx = x.second;
	  var* vy = y.second;
	  assert(vx && vy);
	  return subr(vx, vy);
	}
      }
    };
    struct comp {
      vector<var*>* m_arg;
      comp(vector<var*>* arg) : m_arg(arg) {}
      bool operator()(template_usr* x, template_usr* y)
      {
        template_usr::KEY xkey;
        usr* ux = x->instantiate(m_arg, &xkey);
        if (!ux)
          return false;
        template_usr::KEY ykey;
        usr* uy = y->instantiate(m_arg, &ykey);
        if (!uy)
          return true;
        if (xkey.size() < ykey.size())
          return true;
        if (xkey.size() > ykey.size())
          return false;
	typedef template_usr::KEY::iterator IT;
	int n = 0;
	pair<IT, IT> p = mismatch(begin(xkey), end(xkey), begin(ykey),
				  help(&n));
	if (p != make_pair(end(xkey), end(ykey))) {
	  assert(n == -1 || n == 1);
	  return n < 0;
	}

	partial_ordering::info.push_back(make_pair(x, false));
	usr* insx = x->instantiate(m_arg, 0);
	bool bx = partial_ordering::info.back().second;
	partial_ordering::info.pop_back();

	partial_ordering::info.push_back(make_pair(y, false));
	usr* insy = y->instantiate(m_arg, 0);
	bool by = partial_ordering::info.back().second;
	partial_ordering::info.pop_back();

	if (bx && !by)
	  return false;

	if (!bx && by)
	  return true;

	error::not_implemented();
	return true;
      }
    };
  } // end of namespace partial_ordering_impl
} // end of namespace cxx_compiler

std::vector<std::pair<cxx_compiler::template_usr*, bool> >
cxx_compiler::partial_ordering::info;

cxx_compiler::var*
cxx_compiler::partial_ordering::call(std::vector<var*>* arg)
{
  typedef vector<template_usr*>::const_iterator IT;
  IT p = min_element(begin(m_candidacy), end(m_candidacy),
                     partial_ordering_impl::comp(arg));
  assert(p != end(m_candidacy));
  template_usr* tu = *p;
  usr* ins = tu->instantiate(arg, 0);

  if (template_usr* next = tu->m_next) {
    assert(ins->m_flag2 & usr::INSTANTIATE);
    instantiated_usr* iu = static_cast<instantiated_usr*>(ins);
    next->instantiate(iu->m_seed, usr::NONE2);
  }
  scope* ps = ins->m_scope;
  if (ps->m_id == scope::TAG) {
    usr::flag_t flag = ins->m_flag;
    if (!(flag & usr::STATIC)) {
      assert(m_obj);
      member_function* mf = new member_function(m_obj, ins, false);
      return mf->call(arg);
    }
  }

  return call_impl::wrapper(ins, arg, 0);
}

cxx_compiler::var*
cxx_compiler::alias_usr::call(std::vector<var*>* arg)
{
  if (m_org)
    return m_org->call(arg);
  return usr::call(arg);
}

namespace cxx_compiler {
  namespace call_impl {
    using namespace std;
    pair<int,int> num_of_range(const vector<const type*>&);
    struct convert {
      const vector<const type*>& m_param;
      var* m_func;
      int* m_trial_cost;
      int m_counter;
      convert(const vector<const type*>& param, var* func, int* tc)
        : m_param(param), m_func(func), m_counter(-1), m_trial_cost(tc) {}
      var* operator()(var*);
    };
    tac* gen_param(var*);
    var* ref_vftbl(var* vp, var* vftbl_off, const func_type* ft);
    inline bool common_default_arg(var* func, int n, int m, bool just_query)
    {
      using namespace declarations::declarators::function;
      usr* u = func->usr_cast();
      if (!u)
        return false;
      usr::flag2_t flag2 = u->m_flag2;
      if (!(flag2 & usr::HAS_DEFAULT_ARG))
        return false;

      typedef map<usr*, vector<var*> >::const_iterator ITx;
      ITx p = default_arg_table.find(u);
      assert(p != default_arg_table.end());
      const vector<var*>& v = p->second;
      typedef vector<var*>::const_iterator ITy;
      ITy beg = begin(v) + n;
      ITy end = begin(v) + m;
      ITy q = find(beg, end, (var*)0);
      if (just_query)
        return q == end;

      if (q == end)
        for_each(beg, end, [](var* v){ code.push_back(new param3ac(v)); }); 
      return q == end;
    }
    inline bool has_default_arg(var* func, int n, int m)
    {
      return common_default_arg(func, n, m, true);
    }
    inline void gen_default_arg(var* func, int n, int m)
    {
      common_default_arg(func, n, m, false);
    }
    var* wrapper(usr* fun, vector<var*>* arg, var* this_ptr)
    {
      const type* T = fun->m_type;
      assert(T->m_id == type::FUNC);
      typedef const func_type FT;
      FT* ft = static_cast<FT*>(T);
      return call_impl::common(ft, fun, arg, 0, this_ptr, false, 0);
    }
    inline var* via_obj(var* obj, var* func, bool qualified_func,
                	var* vftbl_off, const func_type* ft)
    {
      const type* T = obj->result_type();
      if (T->scalar()) {
        type::id_t id = T->m_id;
        assert(id == type::POINTER || id == type::REFERENCE);
        if (usr* u = func->usr_cast()) {
          usr::flag_t flag = u->m_flag;
          if ((flag & usr::VIRTUAL) && !qualified_func)
            func = ref_vftbl(u,obj);
        }
        else if (vftbl_off) {
          using namespace expressions::primary::literal;
          var* zero = integer::create(0);
          goto3ac* go = new goto3ac(goto3ac::LT, vftbl_off, zero);
          code.push_back(go);
          if (var* vf = ref_vftbl(obj, vftbl_off, ft))
            code.push_back(new assign3ac(func, vf));
          to3ac* to = new to3ac;
          code.push_back(to);
          go->m_to = to;
          to->m_goto.push_back(go);
        }
        code.push_back(new param3ac(obj));
      }
      else {
        if (obj->m_type != T) {
          var* tmp = obj->address();
          code.push_back(new param3ac(tmp));
        }
        else {
          T = pointer_type::create(T);
          var* tmp = new var(T);
          if ( scope::current->m_id == scope::BLOCK ){
            block* b = static_cast<block*>(scope::current);
            b->m_vars.push_back(tmp);
          }
          else
            garbage.push_back(tmp);
          code.push_back(new addr3ac(tmp,obj));
          code.push_back(new param3ac(tmp));
        }
      }
      return func;
    }
    inline var* call_copy_ctor(var* y)
    {
      const type* T = y->m_type;
      usr* copy_ctor = get_copy_ctor(T);
      if (!copy_ctor)
        return y;
      usr::flag2_t flag2 = copy_ctor->m_flag2;
      if (flag2 & usr::GENED_BY_COMP)
        return y;
      var* t0 = new var(T);
      if (scope::current->m_id == scope::BLOCK) {
        block* b = static_cast<block*>(scope::current);
        b->m_vars.push_back(t0);
      }
      else
        garbage.push_back(t0);
      const type* Tc = copy_ctor->m_type;
      assert(Tc->m_id == type::FUNC);
      typedef const func_type FT;
      FT* ft = static_cast<FT*>(Tc);
      const vector<const type*>& param = ft->param();
      assert(!param.empty());
      const type* Tp = param[0];
      var* t1 = new var(Tp);
      if (scope::current->m_id == scope::BLOCK) {
        block* b = static_cast<block*>(scope::current);
        b->m_vars.push_back(t1);
      }
      else
        garbage.push_back(t1);
      code.push_back(new addr3ac(t1, y));
      vector<var*> arg;
      arg.push_back(t1);
      copy_ctor = instantiate_if(copy_ctor);
      call_impl::wrapper(copy_ctor, &arg, t0);
      return t0;
    }
    var* except_inline(const func_type* ft,
		       var* func,
		       std::vector<var*>* arg,
		       int* trial_cost,
		       var* obj,
		       bool qualified_func,
		       var* vftbl_off);
    var* common(const func_type* ft,
		var* func,
		std::vector<var*>* arg,
		int* trial_cost,
		var* obj,
		bool qualified_func,
		var* vftbl_off)
    {
      int n = code.size();
      var* ret = except_inline(ft, func, arg, trial_cost, obj,
			       qualified_func, vftbl_off);
      if (error::counter)
	return ret;
      if (cmdline::no_inline_sub)
	return ret;
      if (trial_cost) {
	if (*trial_cost == numeric_limits<int>::max())
	  return ret;
      }
      usr* u = func->usr_cast();
      if (!u)
	return ret;
      usr::flag_t flag = u->m_flag;
      if (!(flag & usr::INLINE))
	return ret;
      using namespace declarations::declarators::function;
      using namespace definition::static_inline;
      skip::table_t::const_iterator p = skip::stbl.find(u);
      if (p == skip::stbl.end())
	return ret;
      info_t* info = p->second;
      if (!info)
	return ret;
      substitute(code, code.size()-1, info);
      if (!(flag & usr::CONSTEXPR))
	return ret;
      vector<tac*> tmp;
      copy(begin(code)+n, end(code), back_inserter(tmp));
      code.resize(n);
      if (scope::current->m_id != scope::BLOCK) {
	assert(!orphan.empty());
	block* pb = orphan.back();
	using namespace declarations::initializers;
	change_scope(tmp, pb);
	pb->m_parent = scope::current;
      }
      scope* org = ret->m_scope;
      ret->m_scope = &scope::root;  // make `ret' live variable
      fundef* fdef = fundef::current;
      optimize::action(fdef, tmp, true);
      ret->m_scope = org;
      if (tmp.size() != 1)
	error::not_implemented();
      tac* ptr = tmp.back();
      assert(ptr->m_id == tac::ASSIGN);
      assert(ptr->x == ret);
      if (scope::current->m_id ==scope::BLOCK) {
	block* b = static_cast<block*>(scope::current);
	auto& vars = b->m_vars;
	auto p = find(begin(vars), end(vars), ret);
	assert(p != end(vars));
	vars.erase(p);
      }
      ret = ptr->y;
      delete ptr;
      if (!ret->isconstant())
	error::not_implemented();
      return ret;
    }
  } // end of namespace call_impl
} // end of namespace cxx_compiler

cxx_compiler::var*
cxx_compiler::call_impl::except_inline(const func_type* ft,
				       var* func,
				       std::vector<var*>* arg,
				       int* trial_cost,
				       var* obj,
				       bool qualified_func,
                                var* vftbl_off)
{
  using namespace std;
  const vector<const type*>& param = ft->param();
  int n = arg ? arg->size() : 0;
  pair<int,int> m = call_impl::num_of_range(param);
  if (n < m.first) {
    if (!has_default_arg(func, n, m.first)) {
      if (trial_cost) {
        *trial_cost = numeric_limits<int>::max();
        return 0;
      }
      using namespace error::expressions::postfix::call;
      num_of_arg(parse::position,func,n,m.first);
    }
  }
  else if (m.second < n) {
    if (trial_cost) {
      *trial_cost = numeric_limits<int>::max();
      return 0;
    }
    using namespace error::expressions::postfix::call;
    num_of_arg(parse::position,func,n,m.second);
    n = m.second;
  }
  vector<var*> conved;
  if (arg) {
    const vector<var*>& v = *arg;
    transform(v.begin(),v.begin()+n,back_inserter(conved),
              call_impl::convert(param,func,trial_cost));
    if (trial_cost
        && find(begin(conved),end(conved),(var*)0) != end(conved)) {
      *trial_cost = numeric_limits<int>::max();
      return 0;
    }
  }
  transform(begin(conved), begin(conved)+min(n, m.first), begin(conved),
            call_copy_ctor);
  if (obj)
    func = via_obj(obj, func, qualified_func, vftbl_off, ft);
  transform(conved.begin(),conved.end(),back_inserter(code),
            call_impl::gen_param);
  if (n < m.first)
    gen_default_arg(func, n, m.first);
  const type* T = ft->return_type();
  if (T)
    T = T->complete_type();
  var* x = (!T || T->m_id != type::REFERENCE) ? new var(T) :
    new ref(static_cast<const reference_type*>(T));
  if (!T || T->m_id == type::VOID){
    code.push_back(new call3ac(0,func));
    garbage.push_back(x);
    return x;
  }
  if (!T->size()) {
    if (tag* ptr = T->get_tag()) {
      tag::kind_t kind = ptr->m_kind;
      if (kind == tag::TYPENAME) {
	code.push_back(new call3ac(0,func));
	return x;
      }
    }
    using namespace error::expressions::postfix::call;
    not_object(parse::position,func);
    x->m_type = int_type::create();
  }
  if (scope::current->m_id == scope::BLOCK) {
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
    if (must_call_dtor(T))
      block_impl::dtor_tbl[b].push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new call3ac(x,func));
  return x;
}

cxx_compiler::tac* cxx_compiler::call_impl::gen_param(var* y)
{
  return new param3ac(y);
}

std::pair<int,int> cxx_compiler::call_impl::num_of_range(const std::vector<const type*>& param)
{
  using namespace std;
  const type* T = param.back();
  if (T->m_id == type::ELLIPSIS)
    return make_pair(param.size()-1,INT_MAX);
  if (T->m_id == type::VOID)
    return make_pair(0,0);
  else
    return make_pair(param.size(),param.size());
}

namespace cxx_compiler {
  namespace call_impl {
    using namespace std;
    inline pair<const type*, int>
    addr_case(const reference_type* rt, const type* T)
    {
      T = T->unqualified();
      const type* R = rt->referenced_type();
      R = R->unqualified();
      if (compatible(R, T))
        return make_pair(R,0);
      pair<const type*, int> zero;
      if (R->m_id != type::RECORD)
        return zero;
      if (T->m_id != type::RECORD)
        return zero;
      typedef const record_type REC;
      REC* x = static_cast<REC*>(R);
      REC* y = static_cast<REC*>(T);
      vector<route_t> dummy;
      bool ambiguous = false;
      int offset = calc_offset(y, x, dummy, &ambiguous);
      if (ambiguous)
        error::not_implemented();
      return offset >= 0 ? make_pair(R,offset) : zero;
    }
  } // end of namepsace call_impl
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::call_impl::convert::operator()(var* arg)
{
  using namespace std;
  if ( ++m_counter == m_param.size() )
    --m_counter;
  const type* T = m_param[m_counter];
  T = T->complete_type();
  const type* U = T->unqualified();
  if (U->m_id != type::REFERENCE)
    arg = arg->rvalue();
  if (T->m_id == type::ELLIPSIS) {
    if (m_trial_cost)
      *m_trial_cost += 2;
    const type* Ta = arg->m_type;
    const type* Tav = Ta->varg();
    if ( Ta->compatible(Tav) )
      return arg;
    T = Tav;
  }
  const type* Ta = arg->m_type;
  if (Ta->m_id == type::BACKPATCH) {
    assert(!parse::templ::save_t::nest.empty());
    return arg;
  }
  T = T->unqualified();
  bool discard = false;
  bool ctor_conv = false;
  bool* tmp = !m_trial_cost ? &ctor_conv : 0;
  T = expressions::assignment::valid(T, arg, &discard, tmp, 0);
  if (!T) {
    if (m_trial_cost) {
      *m_trial_cost = numeric_limits<int>::max();
      return 0;
    }
    using namespace error::expressions::postfix::call;
    mismatch_argument(parse::position,m_counter,discard,m_func);
    return arg;
  }
  if (T->m_id == type::TEMPLATE_PARAM)
    return arg;
  if (T->scalar()) {
    const type* Ty = arg->m_type;
    Ty = Ty->unqualified();
    if (Ty->scalar()) {
      if (T->m_id != type::REFERENCE && Ty->m_id == type::REFERENCE) {
        var* tmp = new var(T);
        if (scope::current->m_id == scope::BLOCK) {
          block* b = static_cast<block*>(scope::current);
          b->m_vars.push_back(tmp);
        }
        else
          garbage.push_back(tmp);
        code.push_back(new invraddr3ac(tmp, arg));
        if (m_trial_cost)
          ++*m_trial_cost;
        arg = tmp;
      }
      else {
        var* org = arg;
        arg = arg->cast(T);
        if (org == arg && U->m_id == type::REFERENCE && 
	    (!arg->lvalue() || arg->genaddr_cast())) {
	  var* tmp = new var(T);
	  if (scope::current->m_id == scope::BLOCK) {
	    block* b = static_cast<block*>(scope::current);
	    b->m_vars.push_back(tmp);
	  }
	  else
	    garbage.push_back(tmp);
	  code.push_back(new assign3ac(tmp, arg));
	  arg = tmp;
	}
        if (org != arg && m_trial_cost)
          ++*m_trial_cost;
      }
    }
    else {
      if (Ty->m_id == type::BRACE) {
	ini_list& il = dynamic_cast<ini_list&>(*arg);
	arg = il.cast(T);
      }
      else {
	assert(Ty->m_id == type::RECORD);
	typedef const record_type REC;
	REC* rec = static_cast<REC*>(Ty);
	if (usr* fun = cast_impl::conversion_function(rec, T, true)) {
	  arg = call_impl::wrapper(fun, 0, arg);
	  if (m_trial_cost)
	    ++*m_trial_cost;
	}
      }
    }
  }
  else {
    arg = aggregate_conv(T, arg, ctor_conv, 0);
    if (arg->m_type->m_id == type::BRACE) {
      ini_list& il = dynamic_cast<ini_list&>(*arg);
      var* tmp = new var(T);
      if (scope::current->m_id == scope::BLOCK) {
	block* b = static_cast<block*>(scope::current);
	b->m_vars.push_back(tmp);
      }
      else
	garbage.push_back(tmp);
      il.fill(tmp);
      arg = tmp;
    }
    if (m_trial_cost) {
      T = T->unqualified();
      const type* Ty = arg->result_type();
      Ty = Ty->unqualified();
      if (!compatible(T, Ty))
        ++*m_trial_cost;
      if (tag* ptr = T->get_tag()) {
        if (ptr->m_flag & tag::TYPENAMED) {
          if (T->m_id == type::INCOMPLETE_TAGGED)
            ++*m_trial_cost;
        }
      }
    }
  }
  if (U->m_id == type::REFERENCE) {
    typedef const reference_type RT;
    RT* rt = static_cast<RT*>(U);
    pair<const type*, int> res = call_impl::addr_case(rt, arg->m_type);
    const type* R = res.first;
    int offset = res.second;
    if (R) {
      if (arg->isconstant()) {
        var* tmp = new var(R);
        if (scope::current->m_id == scope::BLOCK) {
          block* b = static_cast<block*>(scope::current);
          b->m_vars.push_back(tmp);
        }
        else
          garbage.push_back(tmp);
        code.push_back(new assign3ac(tmp, arg));
        arg = tmp;
      }
      if (arg->lvalue())
        arg = arg->address();
      else {
        var* tmp = new var(U);
        if (scope::current->m_id == scope::BLOCK) {
          block* b = static_cast<block*>(scope::current);
          b->m_vars.push_back(tmp);
        }
        else
          garbage.push_back(tmp);
        code.push_back(new addr3ac(tmp, arg));
        arg = tmp;
      }
      if (offset) {
        using namespace expressions::primary::literal;
        var* off = integer::create(offset);
        var* tmp = new var(U);
        if (scope::current->m_id == scope::BLOCK) {
          block* b = static_cast<block*>(scope::current);
          b->m_vars.push_back(tmp);
        }
        else
          garbage.push_back(tmp);
        code.push_back(new add3ac(tmp, arg, off));
        arg = tmp;
      }
    }
  }
  return arg;
}

cxx_compiler::var* cxx_compiler::call_impl::ref_vftbl(usr* vf, var* vp)
{
  using namespace std;
  scope* ptr = vf->m_scope;
  assert(ptr->m_id == scope::TAG);
  tag* ptag = static_cast<tag*>(ptr);
  const type* T = ptag->m_types.second;
  assert(T->m_id == type::RECORD);
  typedef const record_type REC;
  REC* rec = static_cast<REC*>(T);

  block* b = 0;
  if (scope::current->m_id == scope::BLOCK)
    b = static_cast<block*>(scope::current);

  T = vf->m_type;
  assert(T->m_id == type::FUNC);
  const func_type* ft = static_cast<const func_type*>(T);
  const pointer_type* pt = pointer_type::create(ft);
  var* tmp = 0;
  pair<int, usr*> off = rec->offset(vfptr_name);
  int vfptr_offset = off.first;
  assert(vfptr_offset >= 0);
  if (!vfptr_offset)  {
    var* t0 = new var(pt);
    if ( b )
      b->m_vars.push_back(t0);
    else
      garbage.push_back(t0);
    code.push_back(new invraddr3ac(t0,vp));
    tmp = t0;
  }
  else {
    var* t0 = new var(pt);
    var* t1 = new var(pt);
    if ( b ){
      b->m_vars.push_back(t0);
      b->m_vars.push_back(t1);
    }
    else {
      garbage.push_back(t0);
      garbage.push_back(t1);
    }
    using namespace expressions::primary::literal;
    var* off = integer::create(vfptr_offset);
    code.push_back(new add3ac(t0,vp,off));
    code.push_back(new invraddr3ac(t1,t0));
    tmp = t1;
  }
  var* res = new var(pt);
  if ( b )
    b->m_vars.push_back(res);
  else
    garbage.push_back(res);
  const map<string, vector<usr*> >& usrs = ptag->m_usrs;
  typedef map<string, vector<usr*> >::const_iterator IT;
  IT p = usrs.find(vftbl_name);
  assert(p != usrs.end());
  const vector<usr*>& v = p->second;
  assert(v.size() == 1);
  usr* u = v.back();
  assert(u->m_flag & usr::WITH_INI);
  with_initial* vftbl = static_cast<with_initial*>(u);
  map<int, var*>::const_iterator q =
    find_if(begin(vftbl->m_value), end(vftbl->m_value),
            bind2nd(ptr_fun(match_vf),vf));
  assert(q != end(vftbl->m_value));
  int offset = q->first;
  if (offset){
    var* t2 = new var(pt);
    if ( b )
      b->m_vars.push_back(t2);
    else
      garbage.push_back(t2);
    using namespace expressions::primary::literal;
    var* off = integer::create(offset);
    code.push_back(new add3ac(t2,tmp,off));
    tmp = t2;
  }
  code.push_back(new invraddr3ac(res,tmp));
  return res;
}

cxx_compiler::var*
cxx_compiler::call_impl::ref_vftbl(var* vp, var* vftbl_off,
                                   const func_type* ft)
{
  const type* T = vp->m_type;
  T = T->unqualified();
  if (T->m_id == type::POINTER) {
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(T);
    T = pt->referenced_type();
  }
  else {
    assert(T->m_id == type::REFERENCE);
    typedef const reference_type RT;
    RT* rt = static_cast<RT*>(T);
    T = rt->referenced_type();
  }
  T = T->unqualified();
  T = T->complete_type();
  assert(T->m_id == type::RECORD);
  typedef const record_type REC;
  REC* rec = static_cast<REC*>(T);
  pair<int, usr*> p = rec->offset(vfptr_name);
  int vfptr_offset = p.first;
  if (vfptr_offset < 0)
    return 0;
  const type* pt = pointer_type::create(ft);

  block* b = 0;
  if (scope::current->m_id == scope::BLOCK)
    b = static_cast<block*>(scope::current);
  var* tmp = 0;
  if (!vfptr_offset)  {
    var* t0 = new var(pt);
    if ( b )
      b->m_vars.push_back(t0);
    else
      garbage.push_back(t0);
    code.push_back(new invraddr3ac(t0,vp));
    tmp = t0;
  }
  else {
    var* t0 = new var(pt);
    var* t1 = new var(pt);
    if ( b ){
      b->m_vars.push_back(t0);
      b->m_vars.push_back(t1);
    }
    else {
      garbage.push_back(t0);
      garbage.push_back(t1);
    }
    using namespace expressions::primary::literal;
    var* off = integer::create(vfptr_offset);
    code.push_back(new add3ac(t0,vp,off));
    code.push_back(new invraddr3ac(t1,t0));
    tmp = t1;
  }

  var* t2 = new var(pt);
  var* res = new var(pt);
  if (b) {
    b->m_vars.push_back(t2);
    b->m_vars.push_back(res);
  }
  else {
    garbage.push_back(t2);
    garbage.push_back(res);
  }
  code.push_back(new add3ac(t2, tmp, vftbl_off));
  code.push_back(new invraddr3ac(res,t2));
  return res;
}

namespace cxx_compiler {
  namespace declarations {
    namespace declarators {
      namespace function {
        namespace definition {
          namespace static_inline {
            using namespace std;      
            namespace substitute_impl {
              map<var*, var*> symtab;
              usr* new_usr(usr* u, scope* s)
              {
                usr::flag_t flag = u->m_flag;
                usr* ret = 0;
                if (flag & usr::WITH_INI) {
                  with_initial* p = static_cast<with_initial*>(u);
                  ret = new with_initial(*p);
                }
                else if (flag & usr::ENUM_MEMBER) {
                  enum_member* p = static_cast<enum_member*>(u);
                  ret = new enum_member(*p);
                }
                else
                  ret = new usr(*u);
                ret->m_scope = s;
                symtab[u] = ret;
                return ret;
              }
              var* new_var(var* v, scope* s)
              {
                var* ret = new var(*v);
                ret->m_scope = s;
                return symtab[v] = ret;
              }
              scope* new_block(scope* ptr, scope* parent)
              {
                block* ret = new block;
                {
                  assert(class_or_namespace_name::before.back() == ret);
		  class_or_namespace_name::before.pop_back();
                }
                ret->m_parent = parent;
                if (ptr->m_id != scope::BLOCK) {
                  assert(ptr->m_id == scope::TAG);
                  return ret;
                }
                block* b = static_cast<block*>(ptr);
                const map<string, vector<usr*> >& u = b->m_usrs;
                map<string, vector<usr*> >& d = ret->m_usrs;
                typedef map<string, vector<usr*> >::const_iterator IT;
                for (auto& p : u) {
                  string name = p.first;
                  const vector<usr*>& v = p.second;
                  transform(v.begin(),v.end(),back_inserter(d[name]),
                            bind2nd(ptr_fun(new_usr),ret));
                }
                const vector<var*>& v = b->m_vars;
                transform(v.begin(),v.end(),back_inserter(ret->m_vars),
                          bind2nd(ptr_fun(new_var),ret));
                const vector<scope*>& c = b->m_children;
                transform(c.begin(),c.end(),back_inserter(ret->m_children),
                          bind2nd(ptr_fun(new_block),ret));
                return ret;
              }
              block* create(scope* param)
              {
                symtab.clear();
                block* ret = new block;
                {
                  assert(class_or_namespace_name::before.back() == ret);
		  class_or_namespace_name::before.pop_back();
                }
		if (scope::current->m_id == scope::BLOCK) {
		  ret->m_parent = scope::current;
		  scope::current->m_children.push_back(ret);
		}
		else
		  orphan.push_back(ret);
                const vector<usr*>& o = param->m_order;
                vector<var*>& v = ret->m_vars;
                transform(o.begin(),o.end(),back_inserter(v),
                          bind2nd(ptr_fun(new_usr),ret));
                const vector<scope*>& c = param->m_children;
                transform(c.begin(),c.end(),back_inserter(ret->m_children),
                          bind2nd(ptr_fun(new_block),ret));
                return ret;
              }
              tac* param2assign(tac* param, var* x)
              {
                assert(param->m_id == tac::PARAM);
                var* y = param->y;
                delete param;
                return new assign3ac(x, y);
              }
              namespace dup {
                struct patch_t {
                  map<goto3ac*,goto3ac*> m_goto;
                  map<to3ac*,to3ac*> m_to;
                };
                tac* filter(tac* ptac, patch_t* patch)
                {
                  tac* ret = ptac->new3ac();
                  if (ret->x) {
                    map<var*, var*>::const_iterator p = symtab.find(ret->x);
                    if (p != symtab.end())
                      ret->x = p->second;
                  }
                  if (ret->y) {
                    map<var*, var*>::const_iterator p = symtab.find(ret->y);
                    if (p != symtab.end())
                      ret->y = p->second;
                  }
                  if (ret->z) {
                    map<var*, var*>::const_iterator p = symtab.find(ret->z);
                    if (p != symtab.end())
                      ret->z = p->second;
                  }

                  tac::id_t id = ptac->m_id;
                  switch (id) {
                  case tac::GOTO:
                    {
                      goto3ac* go = static_cast<goto3ac*>(ptac);
                      return patch->m_goto[go] = static_cast<goto3ac*>(ret);
                    }
                  case tac::TO:
                    {
                      to3ac* to = static_cast<to3ac*>(ptac);
                      return patch->m_to[to] = static_cast<to3ac*>(ret);
                    }
                  default:
                    return ret;
                  }
                }
                void spatch(pair<goto3ac*,goto3ac*> x, map<to3ac*,to3ac*>* y)
                {
                  goto3ac* org = x.first;
                  map<to3ac*,to3ac*>::const_iterator p = y->find(org->m_to);
                  assert(p != y->end());
                  goto3ac* _new = x.second;
                  _new->m_to = p->second;
                }
                goto3ac* helper(goto3ac* x, map<goto3ac*,goto3ac*>* y)
                {
                  map<goto3ac*,goto3ac*>::const_iterator p = y->find(x);
                  assert(p != y->end());
                  return p->second;
                }
                void tpatch(pair<to3ac*,to3ac*> x, map<goto3ac*,goto3ac*>* y)
                {
                  to3ac* org = x.first;
                  const vector<goto3ac*>& u = org->m_goto;
                  to3ac* _new = x.second;
                  vector<goto3ac*>& v = _new->m_goto;
                  transform(u.begin(),u.end(),v.begin(),
                            bind2nd(ptr_fun(helper),y));
                }
              } // end of namespace dup
              struct arg_t {
                vector<tac*>* m_result;
                vector<goto3ac*>* m_returns;
                var* m_ret;
                dup::patch_t* m_patch;
              };
              void conv(tac* ptac, arg_t* pa)
              {
                if (ptac->m_id == tac::RETURN) {
                  if (var* y = ptac->y) {
                    map<var*,var*>::const_iterator p = symtab.find(y);
                    if (p != symtab.end())
                      y = p->second;
                    pa->m_result->push_back(new assign3ac(pa->m_ret,y));
                  }
                  goto3ac* go = new goto3ac;
                  pa->m_returns->push_back(go);
                  pa->m_result->push_back(go);
                }
                else
                  pa->m_result->push_back(dup::filter(ptac,pa->m_patch));
              }
            }  // end of namespace substitute_impl
	    vector<block*> orphan;
            void substitute(vector<tac*>& vt, int pos, info_t* info)
            {
              using namespace call_impl;
              using namespace substitute_impl;
              assert(pos < vt.size());
              tac* call = vt[pos];
              assert(call->m_id == tac::CALL);
              fundef* fdef = info->m_fundef;
              usr* func = fdef->m_usr;
              var* y = call->y;
              usr* u = y->usr_cast();
              assert(u && u->m_name == func->m_name);
              typedef const func_type FT;
              const type* T = func->m_type;
              assert(T->m_id == type::FUNC);
              FT* ft = static_cast<FT*>(T);
              pair<int,int> p = num_of_range(ft->param());
              if (p.first != p.second)  // take variable number of arguments
                return;
              block* pb = create(fdef->m_param);
              int n = p.first;
              if (func->m_scope->m_id == scope::TAG &&
                  !(func->m_flag & usr::STATIC))
                ++n;
              const vector<var*>& vars = pb->m_vars;
              assert(vars.size() == n);
              assert(pos >= n);
              transform(&vt[pos-n],&vt[pos],vars.begin(),&vt[pos-n],
                        param2assign);
              const vector<tac*>& v = info->m_code;
              vector<tac*> result;
              vector<goto3ac*> returns;
              dup::patch_t patch;
              arg_t arg = { &result, &returns, call->x, &patch };
              for_each(v.begin(),v.end(),bind2nd(ptr_fun(conv),&arg));
              map<goto3ac*,goto3ac*>& s = patch.m_goto;
              map<to3ac*,to3ac*>& t = patch.m_to;
              for_each(s.begin(),s.end(),bind2nd(ptr_fun(dup::spatch),&t));
              for_each(t.begin(),t.end(),bind2nd(ptr_fun(dup::tpatch),&s));
              to3ac* to = new to3ac;
              result.push_back(to);
              for_each(returns.begin(),returns.end(),
                       bind2nd(ptr_fun(misc::update),to));
              copy(returns.begin(),returns.end(),back_inserter(to->m_goto));
              vt.erase(vt.begin()+pos);
              delete call;
              for_each(result.begin(), result.end(), [&vt, &pos](tac* ptr)
                       { vt.insert(vt.begin()+pos++, ptr); });
            }

            namespace defer {
              void last()
              {
                using namespace std;
                for (auto& p : refs) {
                  const vector<ref_t>& v = p.second;
                  assert(!v.empty());
                  const ref_t& r = v[0];
                  error::declarations::declarators::function::definition::
                    static_inline::nodef(r.m_def, r.m_flag, r.m_name, r.m_use);
                }
              }
            } // end of namespace defer
          }  // end of namespace static_inline
        }  // end of namespace definition
      }  // end of namespace function
    }  // end of namespace declarators
  }  // end of namespace declarations
}  // end of namespace cxx_compiler

std::stack<cxx_compiler::expressions::postfix::member::info_t*>
cxx_compiler::expressions::postfix::member::handling;

cxx_compiler::expressions::postfix::member::info_t*
cxx_compiler::expressions::postfix::member::begin(base* expr, bool dot)
{
  using namespace std;
  parse::identifier::base_lookup::route.clear();
  auto_ptr<base> sweeper(expr);
  if (parse::templ::func()) {
    if (parse::peek() != '~')
      parse::identifier::mode = parse::identifier::new_obj;
    return 0;
  }
  int n = code.size();
  var* v = expr->gen();
  int m = code.size();
  vector<tac*> nm;
  copy(code.begin()+n,code.begin()+m,back_inserter(nm));
  code.resize(n);
  const type* T = v->result_type();
  if (dot) {
    T = T->unqualified();
    if (T->m_id == type::REFERENCE) {
      typedef const reference_type RT;
      RT* rt = static_cast<RT*>(T);
      T = rt->referenced_type();
    }
  }
  else {
    T = T->unqualified();
    if ( T->m_id != type::POINTER ){
      using namespace error::expressions::postfix::member;
      not_pointer(parse::position,v);
      return new info_t(nm,v,dot,scope::current,expr->file());
    }
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(T);
    T = pt->referenced_type();
  }
  info_t* ret = new info_t(nm,v,dot,scope::current,expr->file());
  tag* ptr = T->get_tag();
  if ( ptr && ptr->m_kind != tag::ENUM ){
    scope::current = ptr;
    parse::identifier::mode = parse::identifier::member;
  }
  handling.push(ret);
  return ret;
}

cxx_compiler::expressions::base*
cxx_compiler::expressions::postfix::member::end(info_t* info, var* member)
{
  if (!info) {
    assert(parse::templ::func());
    parse::identifier::mode = parse::identifier::look;
    return 0;
  }
  info->m_member = member;
  info->m_route = parse::identifier::base_lookup::route;
  parse::identifier::base_lookup::route.clear();
  scope::current = info->m_scope;
  parse::identifier::mode = parse::identifier::look;
  if ( !handling.empty() )
    handling.pop();
  return info;
}

cxx_compiler::expressions::base*
cxx_compiler::expressions::postfix::
member::end(info_t* info, pair<declarations::type_specifier*, bool>* x)
{
  auto_ptr<pair<declarations::type_specifier*, bool> > sweeper(x);
  if (!info) {
    assert(parse::templ::func());
    parse::identifier::mode = parse::identifier::look;
    return 0;
  }
  info->m_qualified = x->second;
  declarations::type_specifier* spec = x->first;
  auto_ptr<declarations::type_specifier> sweeper2(spec);
  if (const type* T = spec->m_type) {
    tag* ptr = T->get_tag();
    const map<string, vector<usr*> >& usrs = ptr->m_usrs;
    string name = ptr->m_name;
    name = '~' + name;
    typedef map<string, vector<usr*> >::const_iterator IT;
    IT p = usrs.find(name);
    if (p == usrs.end()) {
      info->m_type = T;
      return end(info, (var*)0);
    }
    const vector<usr*>& v = p->second;
    assert(v.size() == 1);
    usr* dtor = v.back();
    const type* DT = dtor->m_type;
    const pointer_type* pt = pointer_type::create(DT);
    genaddr* ga = new genaddr(pt, DT, dtor, 0);
    ga->m_qualified_func = x->second;
    return end(info, ga);
  }
  usr* u = spec->m_usr;
  assert(u);
  usr::flag_t flag = u->m_flag;
  assert(flag & usr::TYPEDEF);
  info->m_type = u->m_type;
  return end(info, (var*)0);
}

namespace cxx_compiler {
  struct pseudo_destructor : var {
    var* m_ptr;
    bool m_qualified;
    pseudo_destructor(const type* T, var* v, bool qualified)
      : var(T), m_ptr(v), m_qualified(qualified) {}
    static var* no_effect()
    {
      var* ret = new var(void_type::create());
      garbage.push_back(ret);
      return ret;
    }
    var* call(vector<var*>* arg)
    {
      if (!arg->empty())
        error::not_implemented();
      const type* T = m_type;
      T = T->unqualified();
      if (T->m_id != type::RECORD)
        return no_effect();
      typedef const record_type REC;
      REC* rec = static_cast<REC*>(T);
      tag* ptr = rec->get_tag();
      usr* dtor = has_ctor_dtor(ptr, true);
      if (!dtor)
        return no_effect();
      usr::flag_t flag = dtor->m_flag;
      if (m_qualified)
        dtor->m_flag = usr::flag_t(flag & ~usr::VIRTUAL);
      var* ret = call_impl::wrapper(dtor, 0, m_ptr);
      dtor->m_flag = flag;
      return ret;
    }
  };
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::expressions::postfix::member::info_t::gen()
{
  using namespace std;
  copy(m_code.begin(),m_code.end(),back_inserter(code));
  if (m_member)
    return m_expr->member(m_member,m_dot,m_route);
  assert(m_type);
  var* v = m_expr->rvalue();
  const type* T = v->m_type;
  T = T->unqualified();
  if (!m_dot) {
    assert(T->m_id == type::POINTER);
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(T);
    T = pt->referenced_type();
  }
  if (!compatible(T, m_type))
    error::not_implemented();
  return new pseudo_destructor(m_type, v, m_qualified);
}

cxx_compiler::var*
cxx_compiler::var::member(var* expr, bool dot,
                          const std::vector<route_t>& route)
{
  using namespace std;
  using namespace expressions::primary::literal;
  genaddr* ga = expr->genaddr_cast();
  if (ga)
    expr = ga->m_ref;
  const type* T = result_type();
  int cvr = 0;
  T = T->unqualified(dot ? &cvr : 0);
  typedef const pointer_type PT;
  if (dot) {
    if (T->m_id == type::REFERENCE) {
      typedef const reference_type RT;
      RT* rt = static_cast<RT*>(T);
      T = rt->referenced_type();
      T = T->unqualified(&cvr);
    }
  }
  else {
    if (T->m_id != type::POINTER)
      return this;
    PT* pt = static_cast<PT*>(T);
    T = pt->referenced_type();
    T = T->unqualified(&cvr);
  }
  T = T->complete_type();
  if (T->m_id != type::RECORD) {
    using namespace error::expressions::postfix::member;
    not_record(parse::position, this);
    return this;
  }
  typedef const record_type REC;
  REC* rec = static_cast<REC*>(T);
  usr* member = expr->usr_cast();
  if (!member)
    return expr;
  usr::flag_t flag = member->m_flag;
  if (flag & usr::STATIC) {
    instantiate_static_def(member);
    return member;
  }
  if (flag & usr::ENUM_MEMBER) {
    enum_member* p = static_cast<enum_member*>(member);
    return p->m_value;
  }
  const type* Mt = member->m_type;
  if (!Mt) {
    if (member->m_flag2 & usr::PARTIAL_ORDERING) {
      partial_ordering* po = static_cast<partial_ordering*>(member);
      po->m_obj = this;
      return po;
    }
    assert(flag & usr::OVERLOAD);
    overload* ovl = static_cast<overload*>(member);
    ovl->m_obj = this;
    return ovl;
  }

  scope* msp = member->m_scope;
  assert(msp->m_id == scope::TAG);
  tag* ptr = static_cast<tag*>(msp);
  const type* Tm = ptr->m_types.second;
  assert(Tm->m_id == type::RECORD);
  REC* mrec = static_cast<REC*>(Tm);

  if (Mt->m_id == type::FUNC) {
    assert(ga);
    bool qualified_func = ga->m_qualified_func;
    if (rec == mrec) {
      var* tmp = dot ? this : rvalue();
      return new member_function(tmp, member, qualified_func);
    }
    const type* T = pointer_type::create(mrec);
    if (dot) {
      var* tmp = new var(T);
      if (scope::current->m_id == scope::BLOCK) {
        block* b = static_cast<block*>(scope::current);
        b->m_vars.push_back(tmp);
      }
      else
        garbage.push_back(tmp);
      code.push_back(new addr3ac(tmp, this));
      int offset = calc_offset(rec, mrec, route, 0);
      assert(offset >= 0);
      if (offset) {
        var* off = integer::create(offset);
        code.push_back(new add3ac(tmp, tmp, off));
      }
      return new member_function(tmp, member, qualified_func);
    }
    var* tmp = cast(T);
    return new member_function(tmp, member, qualified_func);
  }

  if (rec == mrec) {
    pair<int, usr*> off = rec->offset(member->m_name);
    int offset = off.first;
    if (offset < 0)
      return this;
    if (flag & usr::BIT_FIELD) {
      int pos = rec->position(member);
      typedef const bit_field_type BF;
      BF* bf = static_cast<BF*>(Mt);
      T = bf->integer_type();
      PT* pt = pointer_type::create(T);
      int bit = bf->bit();
      var* ret = new refbit(pt,this,offset,member,pos,bit,dot);
      garbage.push_back(ret);
      return ret;
    }
    Mt = Mt->qualified(cvr);
    var* O = integer::create(offset);
    if (dot)
      return offref(Mt, O);
    var* rv = rvalue();
    return rv->offref(Mt, O);
  }

  pair<int, usr*> off = mrec->offset(member->m_name);
  int offset = off.first;
  assert(offset >= 0);

  if (flag & usr::BIT_FIELD)
    error::not_implemented();

  if (dot) {
    int base_offset = calc_offset(rec, mrec, route, 0);
    assert(base_offset >= 0);
    var* O = integer::create(base_offset + offset);
    return offref(Mt, O);
  }
  var* rv = rvalue();
  T = pointer_type::create(mrec);
  var* tmp = cast_impl::with_route(T, rv, route);
  var* O = integer::create(offset);
  return tmp->offref(Mt, O);
}

cxx_compiler::var*
cxx_compiler::var::ptr_member(var* expr, bool dot)
{
  using namespace std;
  using namespace expressions::primary::literal;
  const type* T = result_type();
  int cvr = 0;
  T = T->unqualified(dot ? &cvr : 0);
  typedef const pointer_type PT;
  if (dot) {
    if (T->m_id == type::REFERENCE) {
      typedef const reference_type RT;
      RT* rt = static_cast<RT*>(T);
      T = rt->referenced_type();
    }
  }
  else {
    if (T->m_id != type::POINTER)
      return this;
    PT* pt = static_cast<PT*>(T);
    T = pt->referenced_type();
    T = T->unqualified(&cvr);
  }
  T = T->complete_type();
  if (T->m_id != type::RECORD)
    return this;
  typedef const record_type REC;
  REC* rec = static_cast<REC*>(T);
  expr = expr->rvalue();
  T = expr->m_type;
  if (T->m_id != type::POINTER_MEMBER) {
    error::not_implemented();
    return this;
  }
  typedef const pointer_member_type PM;
  PM* pm = static_cast<PM*>(T);
  if (pm->scalar()) {
    T = pm->referenced_type();
    const tag* ptr = pm->ctag();
    const type* X = ptr->m_types.second;
    assert(X->m_id == type::RECORD);
    REC* xrec = static_cast<REC*>(X);
    vector<route_t> dummy;
    bool ambiguous = false;
    int offset = calc_offset(rec, xrec, dummy, &ambiguous);
    if (ambiguous)
      error::not_implemented();
    if (offset < 0)
      error::not_implemented();
    if (offset) {
      var* off = integer::create(offset);
      var* tmp = new var(T);
      if (scope::current->m_id == scope::BLOCK) {
        block* b = static_cast<block*>(scope::current);
        b->m_vars.push_back(tmp);
      }
      else
        garbage.push_back(tmp);
      code.push_back(new add3ac(tmp, expr, off));
      expr = tmp;
    }
    if (dot)
      return offref(T, expr);
    var* rv = rvalue();
    return rv->offref(T, expr);
  }

  T = pm->referenced_type();
  T = pointer_type::create(T);
  var* tmp = new var(T);
  var* vftbl_off = new var(int_type::create());
  if (scope::current->m_id == scope::BLOCK) {
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(tmp);
    b->m_vars.push_back(vftbl_off);
  }
  else {
    garbage.push_back(tmp);
    garbage.push_back(vftbl_off);
  }
  var* idx = integer::create(0);
  code.push_back(new roff3ac(tmp, expr, idx));
  idx = integer::create(T->size());
  code.push_back(new roff3ac(vftbl_off, expr, idx));
  return new member_function(this, tmp, vftbl_off);
}

cxx_compiler::var* cxx_compiler::expressions::postfix::ppmm::gen()
{
  var* expr = m_expr->gen();
  return expr->ppmm(m_plus,true);
}

cxx_compiler::var* cxx_compiler::var::offref(const type* T, var* offset)
{
  using namespace std;
  if ( m_type->scalar() ){
    ref* ret = new ref(pointer_type::create(T));
    if ( scope::current->m_id == scope::BLOCK ){
      block* b = static_cast<block*>(scope::current);
      b->m_vars.push_back(ret);
    }
    else
      garbage.push_back(ret);
    if ( offset->isconstant() && !offset->value() )
      code.push_back(new assign3ac(ret,this));
    else
      code.push_back(new add3ac(ret,this,offset));
    return ret;
  }
  if ( offset->isconstant() ){
    int off = offset->value();
    var* ret = new refaddr(pointer_type::create(T),this,off);
    garbage.push_back(ret);
    return ret;
  }
  var* ret = new refsomewhere(pointer_type::create(T),this,offset);
  garbage.push_back(ret);
  return ret;
}

cxx_compiler::var* cxx_compiler::refaddr::offref(const type* T, var* offset)
{
  if ( offset->isconstant() ){
    int off = m_addrof.m_offset + offset->value();
    var* ret = new refaddr(pointer_type::create(T),m_addrof.m_ref,off);
    garbage.push_back(ret);
    return ret;
  }
  if ( int n = m_addrof.m_offset ) {
    var* tmp = expressions::primary::literal::integer::create(n);
    conversion::arithmetic::gen(&offset, &tmp);
    offset = offset->add(tmp);
  }
  var* ret = new refsomewhere(pointer_type::create(T),m_addrof.m_ref,offset);
  garbage.push_back(ret);
  return ret;
}

cxx_compiler::var* cxx_compiler::refsomewhere::offref(const type* T, var* offset)
{
  conversion::arithmetic::gen(&m_offset, &offset);
  offset = m_offset->add(offset);
  const pointer_type* pt = pointer_type::create(T);
  var* ret = new refsomewhere(pt, m_ref, offset);
  garbage.push_back(ret);
  return ret;
}

cxx_compiler::var* cxx_compiler::genaddr::offref(const type* T, var* offset)
{
  typedef const pointer_type PT;
  PT* pt = pointer_type::create(T);
  if ( offset->isconstant() ){
    int off = m_offset + offset->value();
    var* ret = new refaddr(pt,m_ref,off);
    garbage.push_back(ret);
    return ret;
  }
  if ( m_offset ) {
    using namespace expressions::primary::literal;
    var* tmp = integer::create(m_offset);
    conversion::arithmetic::gen(&offset, &tmp);
    offset = offset->add(tmp);
  }
  var* ret = new refsomewhere(pt,m_ref,offset);
  garbage.push_back(ret);
  return ret;
}

cxx_compiler::var* cxx_compiler::addrof::offref(const type* T, var* offset)
{
  if ( offset->isconstant() ){
    int off = m_offset + offset->value();
    var* ret;
    if ( const pointer_type* G = T->ptr_gen() )
      ret = new genaddr(G,T,m_ref,off);
    else
      ret = new refaddr(pointer_type::create(T),m_ref,off);
    garbage.push_back(ret);
    return ret;
  }
  if ( m_offset ) {
    var* tmp = expressions::primary::literal::integer::create(m_offset);
    conversion::arithmetic::gen(&offset, &tmp);
    offset = offset->add(tmp);
  }
  if ( const pointer_type* G = T->ptr_gen() ){
    var* ret = new generated(G,T);
    block* b = (scope::current->m_id == scope::BLOCK) ? static_cast<block*>(scope::current) : 0;
    if ( b )
      b->m_vars.push_back(ret);
    else
      garbage.push_back(ret);
    code.push_back(new addr3ac(ret,m_ref));
    code.push_back(new add3ac(ret,ret,offset));
    return ret;
  }
  var* ret = new refsomewhere(pointer_type::create(T),m_ref,offset);
  garbage.push_back(ret);
  return ret;
}

namespace cxx_compiler {
  var* constant<__int64>::offref(const type* T, var* offset)
  {
    if (m_flag & CONST_PTR) {
      assert(sizeof(void*) < m_type->size());
      if (offset->isconstant()) {
        int off = offset->value();
        void* p = reinterpret_cast<void*>(m_value + off);
        const pointer_type* pt = pointer_type::create(T);
        var* ret = new refimm<void*>(pt, p);
        garbage.push_back(ret);
        return ret;
      }
    }
    return var::offref(T, offset);
  }
  var* constant<void*>::offref(const type* T, var* offset)
  {
    if ( offset->isconstant() ){
      int off = offset->value();
      unsigned char* p = reinterpret_cast<unsigned char*>(m_value);
      p += off;
      void* q = reinterpret_cast<void*>(p);
      var* ret = new refimm<void*>(pointer_type::create(T),q);
      garbage.push_back(ret);
      return ret;
    }
    return var::offref(T,offset);
  }
} // end of namespace cxx_compiler


std::vector<cxx_compiler::tac*> cxx_compiler::code;

std::vector<cxx_compiler::var*> cxx_compiler::garbage;

std::string cxx_compiler::new_name(std::string head)
{
  using namespace std;
  ostringstream os;
  static int cnt;
  os << head << cnt++;
  return os.str();
}

namespace cxx_compiler {
  namespace expressions {
    namespace assignment {
      struct table_t : public set<pair<int, int> > {
        table_t()
        {
          insert(make_pair(0, 0));
          insert(make_pair(1, 0));
          insert(make_pair(1, 1));
          insert(make_pair(2, 0));
          insert(make_pair(2, 2));
          insert(make_pair(3, 0));
          insert(make_pair(3, 1));
          insert(make_pair(3, 2));
          insert(make_pair(3, 3));
          insert(make_pair(4, 0));
          insert(make_pair(4, 4));
          insert(make_pair(5, 0));
          insert(make_pair(5, 1));
          insert(make_pair(5, 4));
          insert(make_pair(5, 5));
          insert(make_pair(6, 0));
          insert(make_pair(6, 2));
          insert(make_pair(6, 4));
          insert(make_pair(6, 6));
          for (int i = 0; i != 8 ; ++i)
            insert(make_pair(7, i));
        }
      } table;
      bool include(int x, int y)
      {
        return table.find(make_pair(x, y)) != table.end();
      }
      var* ctor_conv_common(const record_type* xx, var* src, bool trial,
                	    usr** exp_ctor, var* obj)
      {
        tag* ptr = xx->get_tag();
        usr* ctor = has_ctor_dtor(ptr, false);
        if (!ctor)
          return 0;
        usr::flag_t flag = ctor->m_flag;
        if (flag & usr::EXPLICIT) {
          if (exp_ctor)
            *exp_ctor = ctor;
        }
        if (!obj) {
          obj = new var(xx);
          if (scope::current->m_id == scope::BLOCK) {
            block* b = static_cast<block*>(scope::current);
            b->m_vars.push_back(obj);
          }
          else
            garbage.push_back(obj);
        }
        vector<var*> arg;
        arg.push_back(src);
        if (flag & usr::OVERLOAD) {
          overload* ovl = static_cast<overload*>(ctor);
          ovl->m_obj = obj;
          int n = code.size();
          vector<usr::flag_t> org;
          if (scope::current->m_id != scope::BLOCK) {
            const vector<usr*>& v = ovl->m_candidacy;
            for_each(begin(v), end(v), [&org](usr* u)
                     {
                       usr::flag_t flag = u->m_flag;
                       org.push_back(flag);
                       u->m_flag = usr::flag_t(flag & ~usr::INLINE);
                     });
          }
          int ind = -1;
          var* ret = ovl->call(&arg, &ind);
          if (scope::current->m_id != scope::BLOCK) {
            vector<usr*>& v = ovl->m_candidacy;
            for (int i = 0 ; i != v.size() ; ++i )
              v[i]->m_flag = org[i];
          }
          if (ret) {
            assert(ind >= 0);
            const vector<usr*>& v = ovl->m_candidacy;
            usr* u = v[ind];
            usr::flag_t flag = u->m_flag;
            if (flag & usr::EXPLICIT) {
              if (exp_ctor)
                *exp_ctor = u;
            }
          }

          if (trial) {
            for_each(begin(code)+n, end(code), [](tac* p){ delete p; });
            code.resize(n);
          }
          assert(trial || ret);
          return ret ? obj : 0;
        }
        const type* T = ctor->m_type;
        assert(T->m_id == type::FUNC);
        typedef const func_type FT;
        FT* ft = static_cast<FT*>(T);
        int trial_cost = 0;
        int* pi = trial ? &trial_cost : 0;
        int n = code.size();
        if (scope::current->m_id != scope::BLOCK)
          flag = usr::flag_t(flag & ~usr::INLINE);
        var* ret = call_impl::common(ft, ctor, &arg, pi, obj, false, 0);
        if (trial) {
          for_each(begin(code)+n, end(code), [](tac* p){ delete p; });
          code.resize(n);
        }
        assert(trial || ret);
        return ret ? obj : 0;
      }
    } // end of namespace assignment
  } // end of namespace expressions
} // end of namespace cxx_compiler

const cxx_compiler::type*
cxx_compiler::expressions::
assignment::valid(const type* T, var* src, bool* discard, bool* ctor_conv,
                  usr** exp_ctor)
{
  const type* xx = T;
  const type* yy = src->result_type();
  xx = xx->unqualified();
  yy = yy->unqualified();
  xx = xx->complete_type();
  yy = yy->complete_type();

  if (tag* ptr = xx->get_tag()) {
    if (ptr->m_flag & tag::TYPENAMED) {
      if (xx->m_id == type::INCOMPLETE_TAGGED)
        return T;
    }
  }
  if (xx->m_id == type::TEMPLATE_PARAM) {
    if (yy->m_id == type::INCOMPLETE_TAGGED) {
      if (tag* ptr = yy->get_tag()) {
	tag::kind_t kind = ptr->m_kind;
	return kind == tag::TYPENAME ? xx : 0;
      }
    }
  }

  if ( xx->arithmetic() && yy->arithmetic() )
    return xx;

  if (xx->m_id == type::RECORD) {
    if (compatible(xx, yy))
      return xx;
    typedef const record_type REC;
    REC* xrec = static_cast<REC*>(xx);
    if (yy->m_id == type::RECORD) {
      REC* yrec = static_cast<REC*>(yy);
      vector<route_t> dummy;
      bool ambiguous = false;
      int offset = calc_offset(yrec, xrec, dummy, &ambiguous);
      if (ambiguous)
        error::not_implemented();
      if (offset >= 0)
        return xx;
    }
    if (ctor_conv) {
      if (ctor_conv_common(xrec, src, true, exp_ctor, 0)) {
        *ctor_conv = true;
        return xx;
      }
    }
    if (yy->m_id == type::BRACE)
      return xx;
    return 0;
  }

  typedef const pointer_type PT;
  if (xx->m_id == type::POINTER) {
    PT* px = static_cast<PT*>(xx);
    if ( yy->m_id == type::POINTER ){
      PT* py = static_cast<PT*>(yy);
      const type* Tx = px->referenced_type();
      const type* Ty = py->referenced_type();
      int cvr_x = 0, cvr_y = 0;
      Tx = Tx->unqualified(&cvr_x);
      Ty = Ty->unqualified(&cvr_y);
      if (compatible(Tx, Ty)){
        if (!discard || include(cvr_x, cvr_y))
          return px;
        else {
          *discard = true;
          return 0;
        }
      }
      const type* v = void_type::create();
      if (compatible(Tx, v)){
        if ( include(cvr_x, cvr_y))
          return px;
        else {
          if (discard)
            *discard = true;
          return 0;
        }
      }
      if (compatible(Ty, v)) {
        if (include(cvr_x, cvr_y))
          return px;
        else {
          if (discard)
            *discard = true;
          return 0;
        }
      }
      if (Tx->m_id == type::RECORD && Ty->m_id == type::RECORD) {
        typedef const record_type REC;
        REC* rx = static_cast<REC*>(Tx);
        REC* ry = static_cast<REC*>(Ty);
        vector<route_t> dummy;
        bool ambiguous = false;
        int offset = calc_offset(ry, rx, dummy, &ambiguous);
        if (ambiguous)
          error::ambiguous(parse::position, ry, rx);
        if (offset >= 0) {
          if (include(cvr_x, cvr_y))
            return px;
          else {
            if (discard)
              *discard = true;
            return 0;
          }
        }
      }
    }
    if (yy->integer() && src->zero())
      return xx;
  }

  if (xx->m_id == type::REFERENCE) {
    if (compatible(xx, yy))
      return xx;
    typedef const reference_type REF;
    REF* ref = static_cast<REF*>(xx);
    const type* T = ref->referenced_type();
    const type* X = src->result_type();
    if (compatible(T, X))
      return T;
    if (!T->modifiable() || !X->modifiable()) {
      if (const type* r = valid(T, src, discard, ctor_conv, exp_ctor)) {
        if (src->isconstant())
          return r;
        int cvr1 = 0, cvr2 = 0;
        T = T->unqualified(&cvr1);
        X = X->unqualified(&cvr2);
        if (include(cvr1, cvr2))
          return r;
        if (discard) 
          *discard = true;
        return 0;
      }
    }
    if (X->m_id == type::REFERENCE) {
      REF* ref = static_cast<REF*>(X);
      X = ref->referenced_type();
    }
    int cvr1 = 0, cvr2 = 0;
    T = T->unqualified(&cvr1);
    X = X->unqualified(&cvr2);
    if (T->m_id == type::RECORD && X->m_id == type::RECORD) {
      T = pointer_type::create(T);
      X = pointer_type::create(X);
      var tmp(X);
      if (valid(T, &tmp, discard, ctor_conv, exp_ctor)) {
        if (include(cvr1, cvr2))
          return xx;
        if (discard)
          *discard = true;
        return 0;
      }
    }
    type::id_t idx = X->m_id;
    type::id_t idy = T->m_id;
    if (idx == idy && idx == type::TEMPLATE_PARAM)
      return xx;
  }

  if (yy->m_id == type::REFERENCE) {
    typedef const reference_type REF;
    REF* ref = static_cast<REF*>(yy);
    const type* T = ref->referenced_type();
    var tmp(T);
    return valid(xx, &tmp, discard, ctor_conv, exp_ctor);
  }

  if (xx->m_id == type::POINTER_MEMBER) {
    typedef const pointer_member_type PMT;
    PMT* px = static_cast<PMT*>(xx);
    if (yy->m_id == type::POINTER_MEMBER) {
      PMT* py = static_cast<PMT*>(yy);
      if (px->ctag() == py->ctag()) { 
        const type* Tx = px->referenced_type();
        const type* Ty = py->referenced_type();
        int cvr_x = 0, cvr_y = 0;
        Tx = Tx->unqualified(&cvr_x);
        Ty = Ty->unqualified(&cvr_y);
        if (compatible(Tx, Ty)){
          if (!discard || include(cvr_x, cvr_y))
            return px;
          else {
            *discard = true;
            return 0;
          }
        }
      }
    }
    return 0;
  }
  if (yy->m_id == type::RECORD) {
    typedef const record_type REC;
    REC* rec = static_cast<REC*>(yy);
    if (xx->m_id == type::REFERENCE) {
      typedef const reference_type RT;
      RT* rt = static_cast<RT*>(xx);
      xx = rt->referenced_type();
      xx = xx->unqualified();
    }
    return cast_impl::conversion_function(rec, xx, true) ? xx : 0;
  }

  if (xx->m_id == type::BOOL) {
    if (yy->scalar())
      return xx;
  }

  if (yy->m_id == type::BRACE) {
    typedef const brace_type BT;
    BT* bt = static_cast<BT*>(yy);
    const vector<const type*>& vt = bt->types();
    ini_list& il = dynamic_cast<ini_list&>(*src);
    if (vt.empty())
      return xx;
    if (vt.size() == 1) {
      vector<var*>& v = il.m_vars;
      assert(v.size() == 1);
      return valid(xx, v[0], discard, ctor_conv, exp_ctor);
    }
    error::not_implemented();
  }
  return 0;
}

bool cxx_compiler::expressions::constant_flag;

cxx_compiler::var* cxx_compiler::var::ppmm(bool plus, bool post)
{
  using namespace std;
  if (!lvalue()) {
    using namespace error::expressions::ppmm;
    not_lvalue(parse::position,plus,this);
  }
  const type* T = m_type;
  T = T->promotion();
  if (tag* ptr = T->get_tag()) {
    tag::flag_t flag = ptr->m_flag;
    if (flag & tag::TYPENAMED)
      return this;
  }
  if (instantiate_with_template_param<template_usr>())
    return this;
  if (!T->scalar()) {
    using namespace error::expressions::ppmm;
    not_scalar(parse::position,plus,this);
    return this;
  }
  if (!T->modifiable()) {
    using namespace error::expressions::ppmm;
    not_modifiable(parse::position,plus,this);
    return this;
  }
  var* one = expressions::primary::literal::integer::create(1);
  if (T->arithmetic())
    one = one->cast(T);
  else {
    const type* TT = T->unqualified();
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(TT);
    TT = pt->referenced_type();
    TT = TT->complete_type();
    if ( !(one = TT->vsize()) ){
      int n = TT->size();
      if ( !n ){
        using namespace error::expressions::ppmm;
        invalid_pointer(parse::position,plus,pt);
        n = 1;
      }
      one = expressions::primary::literal::integer::create(n);
    }
  }
  var* ret = new var(T);
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  b ? b->m_vars.push_back(ret) : garbage.push_back(ret);
  if ( post ){
    if (T == m_type) {
      code.push_back(new assign3ac(ret,this));
      if ( plus )
        code.push_back(new add3ac(this,this,one));
      else
        code.push_back(new sub3ac(this,this,one));
    }
    else {
      code.push_back(new cast3ac(ret,this,T));
      var* tmp = new var(T);
      b ? b->m_vars.push_back(tmp) : garbage.push_back(tmp);
      code.push_back(new assign3ac(tmp,ret));
      if ( plus )
        code.push_back(new add3ac(tmp,tmp,one));
      else
        code.push_back(new sub3ac(tmp,tmp,one));
      code.push_back(new cast3ac(this,tmp,m_type));
    }
  }
  else {
    if (T == m_type) {
      if ( plus )
        code.push_back(new add3ac(this,this,one));
      else
        code.push_back(new sub3ac(this,this,one));
      code.push_back(new assign3ac(ret,this));
    }
    else {
      code.push_back(new cast3ac(ret,this,T));
      if ( plus )
        code.push_back(new add3ac(ret,ret,one));
      else
        code.push_back(new sub3ac(ret,ret,one));
      code.push_back(new cast3ac(this,ret,m_type));
    }
  }
  return ret;
}

cxx_compiler::var* cxx_compiler::ref::ppmm(bool plus, bool post)
{
  using namespace std;
  const type* T = m_result;
  T = T->promotion();
  if ( !T->scalar() ){
    using namespace error::expressions::ppmm;
    not_scalar(parse::position,plus,this);
    return this;
  }
  if ( !T->modifiable() ){
    using namespace error::expressions::ppmm;
    not_modifiable(parse::position,plus,this);
    return this;
  }
  var* one = expressions::primary::literal::integer::create(1);
  if ( T->arithmetic() )
    one = one->cast(T);
  else {
    const type* TT = T->unqualified();
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(TT);
    TT = pt->referenced_type();
    int n = TT->size();
    if ( !n ){
      using namespace error::expressions::ppmm;
      invalid_pointer(parse::position,plus,pt);
      n = 1;
    }
    one = expressions::primary::literal::integer::create(n);
  }
  var* ret = rvalue();
  ret = ret->promotion();
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  if ( post ){
    var* tmp = new var(T);
    b ? b->m_vars.push_back(tmp) : garbage.push_back(tmp);
    if ( plus )
      code.push_back(new add3ac(tmp,ret,one));
    else
      code.push_back(new sub3ac(tmp,ret,one));
    assign(tmp);
  }
  else {
    if ( plus )
      code.push_back(new add3ac(ret,ret,one));
    else
      code.push_back(new sub3ac(ret,ret,one));
    assign(ret);
  }
  return ret;
}

cxx_compiler::var* cxx_compiler::generated::ppmm(bool plus, bool post)
{
  using namespace error::expressions::ppmm;
  not_modifiable_lvalue(parse::position,plus,m_org);
  return this;
}

cxx_compiler::expressions::postfix::
fcast::fcast(declarations::type_specifier* ptr, std::vector<base*>* list,
	     bool brace)
  : m_list(list), m_file(parse::position), m_brace(brace)
{
  using namespace parse;
  using namespace declarations;
  if (m_list) {
    if (context_t::retry[DECL_FCAST_CONFLICT_STATE]) {
      // Note that `ptr' is already deleted.
      context_t::retry[DECL_FCAST_CONFLICT_STATE] = false;
      if (!specifier_seq::info_t::s_stack.empty()) {
	specifier_seq::info_t* p = specifier_seq::info_t::s_stack.top();
	auto_ptr<specifier_seq::info_t> sweeper(p);
	p->update();
	m_type = p->m_type;
	return;
      }
      assert(!type_specifier_seq::info_t::s_stack.empty());
      type_specifier_seq::info_t* p =
	type_specifier_seq::info_t::s_stack.top();
      auto_ptr<type_specifier_seq::info_t> sweeper(p);
      p->update();
      m_type= p->m_type;
      return;
    }
  }

  if (context_t::retry[DECL_FCAST_CONFLICT_STATE]) {
    context_t::retry[DECL_FCAST_CONFLICT_STATE] = false;
    if (specifier_seq::info_t::s_stack.empty()) {
      // `ptr' is aready deleted when
      // type-id : type-specifier abstract-declarator
      // is reduced.
      const type* T = reinterpret_cast<const type*>(ptr);
      assert(T->m_id == type::FUNC);
      typedef const func_type FT;
      FT* ft = static_cast<FT*>(T);
      m_type = ft->return_type();
      return;
    }
    specifier_seq::info_t::s_stack.pop();
  }

  specifier* spec = new specifier(ptr);
  specifier_seq::info_t info(0, spec);
  info.update();
  m_type = info.m_type;
}

cxx_compiler::expressions::postfix::
fcast::fcast(tag* ptr, std::vector<base*>* list, bool brace)
  : m_list(list), m_file(parse::position), m_brace(brace)
{
  m_type = ptr->m_types.second;
  if (!m_type)
    m_type = ptr->m_types.first;
}

cxx_compiler::expressions::postfix::
fcast::fcast(usr* u, std::vector<base*>* list, bool brace)
  : m_list(list), m_brace(brace), m_file(parse::position)
{
  usr::flag_t flag = u->m_flag;
  assert(flag & usr::TYPEDEF);
  m_type = u->m_type;
}

cxx_compiler::expressions::postfix::
fcast::fcast(var* v, std::vector<base*>* list, bool brace)
  : m_list(list), m_brace(brace), m_file(parse::position)
{
  assert(v->usr_cast());
  const type* T = v->m_type;
  assert(T->m_id == type::BACKPATCH);
  m_type = T;
}

namespace cxx_compiler {
  namespace expressions {
    namespace postfix {
      namespace fcast_impl {
        var* try_call(usr* fun, vector<var*>* arg, var* this_ptr)
        {
          const type* T = fun->m_type;
          assert(T->m_id == type::FUNC);
          typedef const func_type FT;
          FT* ft = static_cast<FT*>(T);
          int trial_cost = 0;
          int* pi = (arg->size() == 1) ? &trial_cost : 0;
          return call_impl::common(ft, fun, arg, pi, this_ptr, false, 0);
        }
        bool copy_code(var* x, var* y)
        {
          const type* Tx = x->m_type;
          assert(Tx->m_id == type::RECORD);
          const type* Ty = y->m_type;
          Ty = Ty->unqualified();
          bool b = (Ty->m_id == type::REFERENCE);
          if (b) {
            typedef const reference_type REF;
            REF* ref = static_cast<REF*>(Ty);
            Ty = ref->referenced_type();
            Ty = Ty->unqualified();
          }
          if (Ty->m_id != type::RECORD)
            return false;
          if (!compatible(Tx, Ty))
            return false;
          if (b)
            code.push_back(new invraddr3ac(x, y));
          else
            code.push_back(new assign3ac(x, y));
          return true;
        }
        var* operator_code(const type* Tx, var* y)
        {
          const type* Ty = y->m_type;
          Ty = Ty->unqualified();
          if (Ty->m_id == type::REFERENCE) {
            typedef const reference_type REF;
            REF* ref = static_cast<REF*>(Ty);
            Ty = ref->referenced_type();
            Ty = Ty->unqualified();
          }
          if (Ty->m_id != type::RECORD)
            error::not_implemented();
          typedef const record_type REC;
          REC* recy = static_cast<REC*>(Ty);
          tag* ptr = recy->get_tag();
          const map<string, vector<usr*> >& usrs = ptr->m_usrs;
          string name = conversion_name(Tx);
          typedef map<string, vector<usr*> >::const_iterator IT;
          IT p = usrs.find(name);
          if (p == usrs.end())
            error::not_implemented();
          const vector<usr*>& v = p->second;
          assert(v.size() == 1);
          usr* op_fun = v.back();
          return call_impl::wrapper(op_fun, 0, y);
        }
	struct brace_subr {
	  var* m_ret;
	  brace_subr(var* ret) : m_ret(ret) {}
	  int operator()(int n, var* v)
	  {
	    using namespace expressions::primary::literal;
	    const type* T = m_ret->m_type;
	    pair<int, const type*> curr = T->current(n);
	    int off = curr.first;
	    if (off < 0)
	      error::not_implemented();
	    var* offset = integer::create(off);
	    const type* Tx = curr.second;
	    bool discard = false;
	    bool ctor_conv = true;
	    if (!assignment::valid(Tx, v, &discard, &ctor_conv, 0))
	      error::not_implemented();
	    v = v->cast(Tx);
	    assert(T->aggregate());
	    code.push_back(new loff3ac(m_ret, offset, v));
	    return n + 1;
	  }
	};
        var* brace_code(const vector<var*>& arg, var* ret)
	{
	  using namespace expressions::primary::literal;
	  brace_subr op(ret);
	  int n = accumulate(begin(arg), end(arg), 0, op);
	  const type* T = ret->m_type;
	  pair<int, const type*> curr;
	  while (curr = T->current(n), curr.first >= 0)
	    n = op(n, integer::create(0));
	  return ret;
	}
      } // end of namespace fcast_impl
    } // end of namespace postfix
  } // end of namespace expressions
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::expressions::postfix::fcast::gen()
{
  using namespace std;
  vector<var*> arg;
  if (m_list) {
    transform(m_list->begin(),m_list->end(),back_inserter(arg),
              mem_fun(&base::gen));
  }

  if (m_type->scalar()) {
    switch (arg.size()) {
    case 0:
      {
        var* zero = primary::literal::integer::create(0);
        return zero->cast(m_type);
      }
    case 1:
      return arg.back()->rvalue()->cast(m_type);
    default:
      {
        error::expressions::postfix::fcast::too_many_arg(m_file);
        var* ret = new var(m_type);
        if (scope::current->m_id == scope::BLOCK) {
          block* b = static_cast<block*>(scope::current);
          b->m_vars.push_back(ret);
        }
        else
          garbage.push_back(ret);
        return ret;
      }
    }
  }

  var* ret = new var(m_type);
  if (scope::current->m_id == scope::BLOCK) {
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
    if (must_call_dtor(m_type))
      block_impl::dtor_tbl[b].push_back(ret);
  }
  else
    garbage.push_back(ret);

  if (m_type->m_id == type::BACKPATCH)
    return ret;

  tag* ptr = m_type->get_tag();
  if (ptr->m_flag & tag::TYPENAMED) {
    if (m_type->m_id == type::INCOMPLETE_TAGGED)
      return ret;
  }

  usr* ctor = has_ctor_dtor(ptr, false);
  if (ctor) {
    usr::flag_t flag = ctor->m_flag;
    if (flag & usr::OVERLOAD) {
      overload* ovl = static_cast<overload*>(ctor);
      ovl->m_obj = ret;
      ctor->call(&arg);
      return ret;
    }

    if (fcast_impl::try_call(ctor, &arg, ret))
      return ret;
  }
  else if (m_brace)
    return fcast_impl::brace_code(arg, ret);

  if (arg.size() != 1)
    return ret;  // already error handled. just return.

  var* y = arg.back();

  if (!ctor && fcast_impl::copy_code(ret, y))
    return ret;

  return fcast_impl::operator_code(m_type, y);
}

namespace cxx_compiler {
  namespace typeid_impl {
    const type* get_type_info()
    {
      typedef map<string, vector<usr*> >::const_iterator ITx;
      ITx p = scope::root.m_usrs.find("std");
      if (p == scope::root.m_usrs.end())
	error::not_implemented();
      const vector<usr*>& v = p->second;
      usr* u = v.back();
      usr::flag_t flag = u->m_flag;
      if (!(flag & usr::NAMESPACE))
	error::not_implemented();
      name_space* ns = static_cast<name_space*>(u);
      map<string, tag*>& tags = ns->m_tags;
      typedef map<string, tag*>::const_iterator ITy;
      ITy q = tags.find("type_info");
      if (q == tags.end())
	error::not_implemented();
      tag* ptr = q->second;
      const type* T = ptr->m_types.second;
      if (!T)
	error::not_implemented();
      return T;
    }
    const type* get_type(expressions::base* expr)
    {
      int n = code.size();
      var* v = expr->gen();
      for_each(begin(code)+n, end(code), [](tac* p){ delete p; });
      code.resize(n);
      return v->result_type();
    }
    struct result : var {
      result(const type* T) : var(T) {}
      bool lvalue() const { return true; }
      var* address()
      {
	const type* T = pointer_type::create(m_type);
	var* ret = new var(T);
	if (scope::current->m_id == scope::BLOCK) {
	  block* b = static_cast<block*>(scope::current);
	  b->m_vars.push_back(ret);
	}
	else
	  garbage.push_back(ret);
	code.push_back(new addr3ac(ret, this));
	return ret;
      }
    };
  } // end of namespace typeid_impl
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::expressions::postfix::type_ident::gen()
{
  using namespace expressions::primary::literal;
  const type* TI = typeid_impl::get_type_info();
  TI = const_type::create(TI);
  var* ret = new typeid_impl::result(TI);
  if (scope::current->m_id == scope::BLOCK) {
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  tag* ptr = TI->get_tag();
  if (!ptr)
    error::not_implemented();
  usr* ctor = has_ctor_dtor(ptr, false);
  if (!ctor)
    error::not_implemented();
  usr::flag_t flag = ctor->m_flag;
  assert(flag & usr::OVERLOAD);
  overload* ovl = static_cast<overload*>(ctor);
  ovl->m_obj = ret;
  const type* T = m_expr ? typeid_impl::get_type(m_expr) : m_type;
  T = T->unqualified();
  if (T->m_id == type::REFERENCE) {
    typedef const reference_type RT;
    RT* rt = static_cast<RT*>(T);
    T = rt->referenced_type();
    T = T->unqualified();
  }
  ostringstream os;
  T->decl(os, "");
  string name = os.str();
  name = '"' + name + '"';
  vector<var*> arg;
  arg.push_back(stringa::create(name));
  ovl->call(&arg);
  return ret;
}

namespace cxx_compiler {
  namespace expressions {
    bool unjudge(const type* T)
    {
      tag* ptr = T->get_tag();
      if (!ptr)
	return template_param(scope::tps_t::val2_t(T,0));
      if (const type* T2 = ptr->m_types.second)
	return template_param(scope::tps_t::val2_t(T2,0));
      return template_param(scope::tps_t::val2_t(T,0));
    }
    usr* later()
    {
      string name = new_name(".param");
      const type* T = int_type::create();
      usr* u = new templ_param(name, T, usr::NONE, parse::position,
			       usr::TEMPL_PARAM);
      return u;
    }
  } // end of namespace expressions
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::expressions::postfix::is_kind::gen()
{
  using namespace primary::literal;
  if (unjudge(m_type))
    return later();
  const type* T = m_type->complete_type();
  tag* ptr = T->get_tag();
  if (!ptr)
    return integer::create(0);
  tag::kind_t kind = ptr->m_kind;
  if (kind == tag::STRUCT) {
    int n = m_kind == tag::CLASS ? 1 : 0;
    return integer::create(n);
  }
  int n = kind == m_kind ? 1 : 0;
  return integer::create(n);
}

cxx_compiler::var* cxx_compiler::expressions::postfix::is_common2::gen()
{
  using namespace primary::literal;
  if (unjudge(m_Tx) || unjudge(m_Ty))
    return later();
  const type* Tx = m_Tx->complete_type();
  const type* Ty = m_Ty->complete_type();
  switch (m_kind) {
  case same:
    int n = compatible(Tx, Ty) ? 1 : 0;
    return integer::create(n);
  }
  assert(m_kind == ass || m_kind == triv_ass);
  using namespace expressions;

  // inspired by usr::assign(var*)
  if (Tx->m_id == type::REFERENCE) {
    typedef const reference_type RT;
    RT* rtx = static_cast<RT*>(Tx);
    Tx = rtx->referenced_type();
    if (Ty->m_id == type::REFERENCE) {
      RT* rty = static_cast<RT*>(Ty);
      Ty = rty->referenced_type();
    }
    int cvr = 0;
    Ty->unqualified(&cvr);
    if (cvr & 1)
      Tx = const_type::create(Tx);
    if (cvr & 2)
      Tx = volatile_type::create(Tx);
    if (cvr & 4)
      Tx = restrict_type::create(Tx);
    Tx = reference_type::create(Tx, false);
  }
  var tmp(Ty);
  bool discard = false;
  bool ctor_conv = false;
  const type* T = assignment::valid(Tx, &tmp, &discard, &ctor_conv, 0);
  int n = T ? 1 : 0;
  return integer::create(n);
}

cxx_compiler::var* cxx_compiler::expressions::postfix::is_common::gen()
{
  if (unjudge(m_type))
    return later();
  using namespace primary::literal;
  const type* T = m_type->complete_type();
  tag* ptr = T->get_tag();
  if (!ptr)
    return integer::create(1);
  usr* ctor = has_ctor_dtor(ptr, false);
  if (!ctor)
    return integer::create(1);
  usr::flag_t flag = ctor->m_flag;
  assert(flag & usr::OVERLOAD);
  error::not_implemented();
  overload* ovl = static_cast<overload*>(ctor);
  const vector<usr*>& v = ovl->m_candidacy;
  typedef vector<usr*>::const_iterator IT;
  IT p = find_if(begin(v), end(v),
		 [](usr* u){ return !(u->m_flag2 & usr::GENED_BY_COMP); });
  int n = p != end(v) ? 0 : 1;
  return integer::create(n);
}

cxx_compiler::var*
cxx_compiler::expressions::postfix::is_constructible::gen()
{
  using namespace primary::literal;
  auto_ptr<vector<const type*>> sweeper(m_types);
  return integer::create(1);
}

cxx_compiler::var* cxx_compiler::expressions::postfix::no_except::gen()
{
  using namespace primary::literal;
  return integer::create(1);
}

cxx_compiler::var* cxx_compiler::expressions::postfix::align_of::gen()
{
  using namespace primary::literal;
  if (unjudge(m_type))
    return later();
  const type* T = m_type->complete_type();
  int n = T->align();
  return integer::create(n);
}

namespace cxx_compiler {
  using namespace expressions::primary::literal;
  template<> var* refimm<void*>::common()
  {
    if (sizeof(void*) == sizeof(int)) {
      int i = (int)(__int64)m_addr;
      return integer::create(i);
    }
    return integer::create((__int64)m_addr);
  }
  template<> var* refimm<__int64>::common()
  {
    return integer::create(m_addr);
  }
} // end of namespace cxx_compiler
