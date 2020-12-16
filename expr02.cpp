// unary-exrepssion
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "cxx_y.h"

cxx_compiler::var* cxx_compiler::expressions::unary::ppmm::gen()
{
  var* expr = m_expr->gen();
  return expr->ppmm(m_plus,false);
}

const cxx_compiler::file_t&
cxx_compiler::expressions::unary::ppmm::file() const
{
  return m_expr->file();
}

namespace cxx_compiler {
  namespace expressions {
    namespace unary {
      namespace sizeof_impl {
        var* common(int n)
        {
          using namespace primary::literal;
          switch (generator::sizeof_type) {
          case type::UINT:
            {
              typedef unsigned int X;
              return integer::create((X)n);
            }
          case type::ULONG:
            {
              typedef unsigned long int X;
              const type* XX = ulong_type::create();
              XX = const_type::create(XX);
              if (XX->size() == sizeof(X))
                return integer::create((X)n);
              typedef unsigned long long int Y;
              assert(XX->size() == sizeof(Y));
              usr* ret = integer::create((Y)n);
              ret->m_type = XX;
              ret->m_flag = usr::SUB_CONST_LONG;
              return ret;
            }
          default:
            {
              typedef unsigned long long int X;
              return integer::create((X)n);
            }
          }
        }
      } // end of namespace sizoef_impl
    } // end of namespace unary
  } // end of namespace expressions
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::expressions::unary::size_of::gen()
{
  using namespace std;
  if (m_type) {
    const type* T = m_type->complete_type();
    if (tag* ptr = T->get_tag()) {
      tag::flag_t flag = ptr->m_flag;
      if (flag & tag::TYPENAMED)
	return sizeof_impl::common(1);
    }
    if (var* size = T->vsize())
      return size;
    unsigned int n = T->size();
    if (!n) {
      using namespace error::expressions::unary::size;
      invalid(file(), T);
      n = 1;
    }
    return sizeof_impl::common(n);
  }

  int n = code.size();
  var* expr = m_expr->gen();
  int m = code.size();
  for_each(code.begin()+n,code.begin()+m,[](tac* p){ delete p; });
  code.resize(n);
  parse::position = file();
  return expr->size();
}

const cxx_compiler::file_t&
cxx_compiler::expressions::unary::size_of::file() const
{
  return m_expr ? m_expr->file() : m_file;
}

cxx_compiler::var* cxx_compiler::expressions::unary::ope::gen()
{
  var* expr = m_expr->gen();
  switch ( m_op ){
  case '*' : return expr->indirection();
  case '&' : return expr->address();
  case '+' : return expr->plus();
  case '-' : return expr->minus();
  case '!' : return expr->_not();
  default  : return expr->tilde();
  }
}

const cxx_compiler::file_t&
cxx_compiler::expressions::unary::ope::file() const
{
  return m_expr->file();
}

namespace cxx_compiler {
  const type* sizeof_type()
  {
    switch (generator::sizeof_type) {
    case type::UINT:
      return uint_type::create();
    case type::ULONG:
      return ulong_type::create();
    default:
      return ulong_long_type::create();
    }
  }
  static bool new_installed = false;
  inline bool integer_arg(const usr* u)
  {
    const type* T = u->m_type;
    assert(T->m_id == type::FUNC);
    typedef const func_type FT;
    FT* ft = static_cast<FT*>(T);
    const vector<const type*>& param = ft->param();
    assert(!param.empty());
    if (param.size() != 1)
      return false;
    T = param.back();
    return T->integer();
  }
  inline usr* install_new()
  {
    assert(!new_installed);
    string name = "new";
    map<string, vector<usr*> >& usrs = scope::root.m_usrs;
    typedef map<string, vector<usr*> >::const_iterator IT;
    IT p = usrs.find(name);
    if (p != usrs.end()) {
      const vector<usr*>& v = p->second;
      typedef vector<usr*>::const_iterator IT2;
      IT2 q = find_if(begin(v), end(v), integer_arg);
      if (q != end(v)) {
        // user defined new(size_t) exists.
        return v.back();  // if overloaded, return it
      }
    }
    vector<const type*> param;
    param.push_back(sizeof_type());
    const type* T = void_type::create();
    const pointer_type* vp = pointer_type::create(T);
    const func_type* ft = func_type::create(vp,param);
    usr::flag_t flag = usr::flag_t(usr::FUNCTION | usr::NEW_SCALAR);

    usr* new_func = new usr(name, ft, flag, parse::position,
                	    usr::GENED_BY_COMP);
    new_func->m_scope = &scope::root;
    if (p == usrs.end()) {
      usrs[name].push_back(new_func);
      return new_func;
    }
    const vector<usr*>& v = p->second;
    usr* prev = v.back();
    usrs[name].push_back(new_func);
    usr* ovl = new overload(prev, new_func);
    usrs[name].push_back(ovl);
    return ovl;
  }
  inline var* call_new(usr* new_func, vector<var*>& arg)
  {
    usr::flag_t flag = new_func->m_flag;
    if (flag & usr::OVERLOAD)
      return new_func->call(&arg);
    return call_impl::wrapper(new_func, &arg, 0);
  }
} // end of namespace cxx_compiler

namespace cxx_compiler {
  namespace expressions {
    namespace unary {
      inline usr* ctor_dtor_entry(const type* T, bool is_dtor)
      {
        const type* U = T->unqualified();
        if (U->m_id != type::RECORD)
          return 0;

        typedef const record_type REC;
        REC* rec = static_cast<REC*>(U);
        tag* ptr = rec->get_tag();
        return has_ctor_dtor(ptr, is_dtor);
      }
      inline usr* ctor_entry(const type* T)
      {
        return ctor_dtor_entry(T, false);
      }
      inline usr* dtor_entry(const type* T)
      {
        return ctor_dtor_entry(T, true);
      }
      inline usr* new_delete_entry(const type* T, string name)
      {
        if (T->m_id != type::RECORD)
          return 0;
        typedef const record_type REC;
        REC* rec = static_cast<REC*>(T);
        tag* ptr = rec->get_tag();
        const map<string, vector<usr*> >& usrs = ptr->m_usrs;
        typedef map<string, vector<usr*> >::const_iterator IT;
        IT p = usrs.find(name);
        if (p != usrs.end()) {
          const vector<usr*>& v = p->second;
          return v.back();
        }
        using namespace parse::identifier;
        int r = base_lookup::action(name, ptr);
        if (!r)
          return 0;
        assert(r == IDENTIFIER_LEX);
        var* v = cxx_compiler_lval.m_var;
        genaddr* ga = v->genaddr_cast();
        assert(ga);
        v = ga->m_ref;
        assert(v->usr_cast());
        return static_cast<usr*>(v);
      }
      inline usr* new_entry(const type* T)
      {
        return new_delete_entry(T, "new");
      }
      inline usr* delete_entry(const type* T, bool array)
      {
        string name = "delete";
        if (array)
          name += " []";
        return new_delete_entry(T, name);
      }
      inline usr* vdel_entry(const type* T)
      {
        if (T->m_id != type::RECORD)
          return 0;
        typedef const record_type REC;
        REC* rec = static_cast<REC*>(T);
        tag* ptr = rec->get_tag();
        const vector<usr*>& order = ptr->m_order;
        typedef vector<usr*>::const_iterator IT;
        IT p = find_if(begin(order), end(order),[](usr* u)
                       { return u->m_flag & usr::VDEL; });
        if (p == end(order))
          return 0;
        return *p;
      }
      const type* array_element(const type* T)
      {
        if (T->m_id == type::ARRAY) {
          typedef const array_type AT;
          AT* at = static_cast<AT*>(T);
          T = at->element_type();
          T = T->unqualified();
          return array_element(T);
        }
        if (T->m_id == type::VARRAY) {
          typedef const varray_type VT;
          VT* vt = static_cast<VT*>(T);
          T = vt->element_type();
          T = T->unqualified();
          return array_element(T);
        }
        return T;
      }
    } // end of namespace unary
  } // end of namespace expressions
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::expressions::unary::new_expr::gen()
{
  using namespace std;
  using namespace primary::literal;
  map<const type*, vector<tac*> >::iterator p =
    declarations::new_type_id::table.find(m_T);
  if (p != declarations::new_type_id::table.end()) {
    vector<tac*>& v = p->second;
    copy(begin(v), end(v), back_inserter(code));
    declarations::new_type_id::table.erase(p);
  }
  vector<var*> new_arg;
  var* whole_sz = m_T->vsize();
  if (!whole_sz) {
    int n = m_T->size();
    assert(n);
    whole_sz = sizeof_impl::common(n);
  }
  const type* ET = array_element(m_T);
  var* extra = integer::create(sizeof_type()->size());
  var* new_sz = (ET == m_T) ? whole_sz : whole_sz->add(extra);
  int n = ET->size();
  assert(n);
  var* esz = sizeof_impl::common(n);
  new_arg.push_back(new_sz);
  usr* new_func = new_entry(m_T);
  if (m_place) {
    transform(begin(*m_place), end(*m_place), back_inserter(new_arg),
              mem_fun(&base::gen));
  }
  else if (!new_func) {
    if (!new_installed) {
      new_func = install_new();
      new_installed = true;
    }
  }

  if (!new_func) {
    string name = "new";
    const map<string, vector<usr*> >& usrs = scope::root.m_usrs;
    map<string, vector<usr*> >::const_iterator it = usrs.find(name);
    if (it == usrs.end()) {
      assert(!new_installed);
      new_func = install_new();
      new_installed = true;
    }
    else {
      const vector<usr*>& v = it->second;
      new_func = v.back();
    }
  }
  var* ret = call_new(new_func, new_arg);
  if (ET != m_T) {
    var* times = whole_sz->div(esz);
    const type* T = times->m_type;
    T = pointer_type::create(T);
    var* t0 = new var(T);
    if (scope::current->m_id == scope::BLOCK) {
      block* b = static_cast<block*>(scope::current);
      b->m_vars.push_back(t0);
    }
    else
      garbage.push_back(t0);
    code.push_back(new cast3ac(t0, ret, T));
    code.push_back(new invladdr3ac(t0, times));
    code.push_back(new add3ac(ret, ret, extra));
  }

  usr* ctor = ctor_entry(ET);
  if (!ctor) {
    if (!m_exprs)
      return ret;
    if (m_exprs->size() != 1)
      error::not_implemented();
    expressions::base* expr = (*m_exprs)[0];
    var* src = expr->gen();
    bool discard = false;
    bool ctor_conv = false;
    if (!expressions::assignment::valid(ET, src, &discard, &ctor_conv, 0))
      error::not_implemented();
    code.push_back(new invladdr3ac(ret, src));
    return ret;
  }

  vector<var*> arg;
  if (m_exprs) {
    transform(begin(*m_exprs), end(*m_exprs), back_inserter(arg),
              mem_fun(&base::gen));
  }

  usr::flag_t flag = ctor->m_flag;
  if (ET == m_T) {
    if (flag & usr::OVERLOAD) {
      overload* ovl = static_cast<overload*>(ctor);
      ovl->m_obj = ret;
      ovl->call(&arg);
      return ret;
    }
    call_impl::wrapper(ctor, &arg, ret);
    return ret;
  }

  const type* T = ret->m_type;
  var* t0 = new var(T);
  var* t1 = new var(T);
  if (scope::current->m_id == scope::BLOCK) {
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(t0);
    b->m_vars.push_back(t1);
  }
  else {
    garbage.push_back(t0);
    garbage.push_back(t1);
  }
  code.push_back(new assign3ac(t0, ret));
  code.push_back(new add3ac(t1, ret, whole_sz));
  to3ac* label0 = new to3ac;
  code.push_back(label0);
  goto3ac* goto1 = new goto3ac(goto3ac::EQ, t0, t1);
  code.push_back(goto1);
  if (flag & usr::OVERLOAD) {
    overload* ovl = static_cast<overload*>(ctor);
    ovl->m_obj = t0;
    ovl->call(&arg);
  }
  else
    call_impl::wrapper(ctor, &arg, t0);
  code.push_back(new add3ac(t0, t0, esz));
  goto3ac* goto0 = new goto3ac;
  goto0->m_to = label0;
  label0->m_goto.push_back(goto0);
  code.push_back(goto0);
  to3ac* label1 = new to3ac;
  code.push_back(label1);
  goto1->m_to = label1;
  label1->m_goto.push_back(goto1);
  return ret;
}

namespace cxx_compiler {
  usr* installed_delete()
  {
    string name = "delete";
    map<string, vector<usr*> >& usrs = scope::root.m_usrs;
    map<string, vector<usr*> >::const_iterator p = usrs.find(name);
    if (p != usrs.end()) {
      const vector<usr*>& v = p->second;
      return v.back();
    }

    vector<const type*> param;
    const type* vt = void_type::create();
    const type* vp = pointer_type::create(vt);
    param.push_back(vp);
    const func_type* ft = func_type::create(vt,param);
    usr::flag_t flag = usr::flag_t(usr::FUNCTION | usr::DELETE_SCALAR);
    usr* delete_func = new usr(name, ft, flag, parse::position,
                	       usr::GENED_BY_COMP);
    delete_func->m_scope = &scope::root;
    usrs[name].push_back(delete_func);
    return delete_func;
  }
  inline void dtor_for_array(var* delete_arg, var* expr, usr* dtor)
  {
    using namespace expressions::primary::literal;
    using namespace expressions::unary;
    const type* st = sizeof_type();
    const type* pt = pointer_type::create(st);
    var* t0 = new var(pt);
    var* t1 = new var(st);
    var* t2 = new var(expr->m_type);
    if (scope::current->m_id == scope::BLOCK) {
      block* b = static_cast<block*>(scope::current);
      b->m_vars.push_back(t0);
      b->m_vars.push_back(t1);
      b->m_vars.push_back(t2);
    }
    else {
      garbage.push_back(t0);
      garbage.push_back(t1);
      garbage.push_back(t2);
    }
    code.push_back(new cast3ac(t0, delete_arg, pt));
    code.push_back(new invraddr3ac(t1, t0));
    var* one = integer::create(1);
    one = one->cast(st);
    code.push_back(new sub3ac(t1, t1, one));
    scope* p = dtor->m_scope;
    assert(p->m_id == scope::TAG);
    tag* ptr = static_cast<tag*>(p);
    const type* T = ptr->m_types.second;
    int n = T->size();
    var* sz = sizeof_impl::common(n);
    t1 = t1->mul(sz);
    code.push_back(new add3ac(t2, expr, t1));
    to3ac* label0 = new to3ac;
    code.push_back(label0);
    call_impl::wrapper(dtor, 0, t2);
    goto3ac* goto1 = new goto3ac(goto3ac::EQ, expr, t2);
    code.push_back(goto1);
    code.push_back(new sub3ac(t2, t2, sz));
    goto3ac* goto0 = new goto3ac;
    goto0->m_to = label0;
    label0->m_goto.push_back(goto0);
    code.push_back(goto0);
    to3ac* label1 = new to3ac;
    code.push_back(label1);
    goto1->m_to = label1;
    label1->m_goto.push_back(goto1);
  }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::expressions::unary::delete_expr::gen()
{
  using namespace std;
  using namespace primary::literal;
  var* expr = m_expr->gen();
  expr = expr->rvalue();
  var* delete_arg = expr;
  const type* T = expr->m_type;
  if (m_array) {
    var* extra = integer::create(sizeof_type()->size());
    var* t0 = new var(T);
    if (scope::current->m_id == scope::BLOCK) {
      block* b = static_cast<block*>(scope::current);
      b->m_vars.push_back(t0);
    }
    else
      garbage.push_back(t0);
    code.push_back(new sub3ac(t0, expr, extra));
    delete_arg = t0;
  }
  T = T->unqualified();
  if (T->m_id != type::POINTER)
    error::not_implemented();
  typedef const pointer_type PT;
  PT* pt = static_cast<PT*>(T);
  T = pt->referenced_type();
  if (!m_root) {
    if (usr* vdel = vdel_entry(T))
      return call_impl::wrapper(vdel, 0, expr);
  }
  if (usr* dtor = dtor_entry(T)) {
    if (m_array)
      dtor_for_array(delete_arg, expr, dtor);
    else
      call_impl::wrapper(dtor, 0, expr);
  }
  vector<var*> arg;
  arg.push_back(delete_arg);
  usr* delete_func = m_root ? 0 : delete_entry(T, m_array);
  if (!delete_func)
    delete_func = installed_delete();
  const type* D = delete_func->m_type;
  assert(D->m_id == type::FUNC);
  typedef const func_type FT;
  FT* ft = static_cast<FT*>(D);
  const vector<const type*>& param = ft->param();
  if (param.size() != 1) {
    assert(param.size() == 2);
    const type* P = param[1];
    int n = T->size();
    var* sz = sizeof_impl::common(n);
    sz = sz->cast(P);
    arg.push_back(sz);
  }
  return call_impl::wrapper(delete_func, &arg, 0);
}

const cxx_compiler::file_t&
cxx_compiler::expressions::unary::delete_expr::file() const
{
  return m_expr->file();
}

cxx_compiler::var* cxx_compiler::var::indirection()
{
  using namespace std;
  var* y = rvalue();
  const type* T = y->m_type;
  T = T->unqualified();
  if (T->m_id == type::TEMPLATE_PARAM)
    return y;
  if (T->m_id != type::POINTER) {
    if (usr* func = operator_function(T, '*'))
      return call_impl::wrapper(func, 0, y);
    using namespace error::expressions::unary::indirection;
    not_pointer(parse::position);
    T = pointer_type::create(T);
  }
  typedef const pointer_type PT;
  PT* pt = static_cast<PT*>(T);
  ref* x = new ref(pt);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new assign3ac(x,y));
  return x;
}

cxx_compiler::var* cxx_compiler::addrof::indirection()
{
  typedef const pointer_type PT;
  PT* pt = static_cast<PT*>(m_type);
  var* ret = new refaddr(pt,m_ref,m_offset);
  garbage.push_back(ret);
  return ret;
}

cxx_compiler::var* cxx_compiler::genaddr::indirection()
{
  const type* T = m_ref->m_type;
  if (T->m_id == type::FUNC)
    return this;
  
  return addrof::indirection();
}

namespace cxx_compiler {
  var* constant<__int64>::indirection()
  {
    if (m_flag & CONST_PTR) {
      assert(sizeof(void*) < m_type->size());
      typedef const pointer_type PT;
      PT* pt = static_cast<PT*>(m_type);
      var* ret = new refimm<__int64>(pt,m_value);
      garbage.push_back(ret);
      return ret;
    }
    return var::indirection();
  }
  var* constant<void*>::indirection()
  {
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(m_type);
    var* ret = new refimm<void*>(pt,m_value);
    garbage.push_back(ret);
    return ret;
  }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::with_initial::indirection()
{
  var* y = rvalue();
  if (y != this)
    return y->indirection();
  return usr::indirection();
}

cxx_compiler::var* cxx_compiler::var::address()
{
  var* expr = rvalue();
  const type* T = expr->m_type;
  if (usr* func = operator_function(T, '&'))
    return call_impl::wrapper(func, 0, expr);
  using namespace error::expressions::unary::address;
  not_lvalue(parse::position);
  return this;
}

cxx_compiler::var* cxx_compiler::usr::address()
{
  if (usr* func = operator_function(m_type, '&'))
    return call_impl::wrapper(func, 0, this);
  if (!lvalue()) {
    using namespace error::expressions::unary::address;
    not_lvalue(parse::position);
  }
  const type* T = m_type;
  const type* U = T->unqualified();
  if (U->m_id == type::REFERENCE) {
    typedef const reference_type RT;
    RT* rt = static_cast<RT*>(T);
    T = rt->referenced_type();
  }
  typedef const pointer_type PT;
  PT* pt = pointer_type::create(T);
  block* b = 0;
  if (scope::current->m_id == scope::BLOCK)
    b = static_cast<block*>(scope::current);
  if (b && !expressions::constant_flag) {
    var* x = new var(pt);
    b->m_vars.push_back(x);
    if (U->m_id == type::REFERENCE)
      code.push_back(new assign3ac(x,this));
    else
      code.push_back(new addr3ac(x,this));
    return x;
  }
  else {
    if (U->m_id == type::REFERENCE) {
      var* x = new var(pt);
      code.push_back(new assign3ac(x,this));
      garbage.push_back(x);
      return x;
    }
    var* ret = new addrof(pt,this,0);
    garbage.push_back(ret);
    return ret;
  }
}

namespace cxx_compiler {
  namespace genaddr_impl {
    inline var* addr_normal(var* ref, block* b)
    {
      const type* T = ref->m_type;
      T = pointer_type::create(T);
      var* ret = new var(T);
      b->m_vars.push_back(ret);
      code.push_back(new addr3ac(ret, ref));
      return ret;
    }
  } // end of namespace genaddr_impl
} // end of nmaespace cxx_compiler

cxx_compiler::var* cxx_compiler::genaddr::address()
{
  using namespace expressions::primary::literal;
  block* b = 0;
  if (scope::current->m_id == scope::BLOCK)
    b = static_cast<block*>(scope::current);
  if (!b || expressions::constant_flag) {
    const type* T = m_ref->m_type;
    const pointer_type* pt = pointer_type::create(T);
    var* ret = new addrof(pt , m_ref, 0);
    garbage.push_back(ret);
    return ret;
  }

  assert(fundef::current);
  usr* func = fundef::current->m_usr;
  scope* p = func->m_scope;
  if (p->m_id == scope::TAG)
    return genaddr_impl::addr_normal(m_ref, b);
  scope* q = m_ref->m_scope;
  if (q->m_id != scope::TAG)
    return genaddr_impl::addr_normal(m_ref, b);
  tag* ptr = static_cast<tag*>(q);
  usr* u = m_ref->usr_cast();
  if (!u)
    return genaddr_impl::addr_normal(m_ref, b);
  usr::flag_t flag = u->m_flag;
  if (flag & usr::STATIC)
    return genaddr_impl::addr_normal(m_ref, b);

  return fun_ptr_mem(ptr, u);
}

cxx_compiler::var* cxx_compiler::refbit::address()
{
  using namespace error::expressions::unary::address;
  bit_field(parse::position,m_member);
  return refaddr::address();
}

cxx_compiler::var* cxx_compiler::ref::address()
{
  var* ret = new var(m_type);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  code.push_back(new assign3ac(ret,this));
  return ret;
}

cxx_compiler::var* cxx_compiler::refaddr::address()
{
  using namespace std;
  using namespace expressions::primary::literal;
  if (!lvalue()) {
    using namespace error::expressions::unary::address;
    not_lvalue(parse::position);
    return this;
  }
  block* b = 0;
  if (scope::current->m_id == scope::BLOCK)
    b = static_cast<block*>(scope::current);
  if (b && !expressions::constant_flag) {
    const type* T = m_type;
    if (T->m_id == type::REFERENCE) {
      typedef const reference_type RT;
      RT* rt = static_cast<RT*>(T);
      const type* R = rt->referenced_type();
      T = pointer_type::create(R);
    }
    var* ret = new var(T);
    b->m_vars.push_back(ret);
    code.push_back(new addr3ac(ret,m_addrof.m_ref));
    if ( m_addrof.m_offset ){
      usr* off = integer::create(m_addrof.m_offset);
      var* tmp = new var(T);
      b->m_vars.push_back(tmp);
      code.push_back(new add3ac(tmp,ret,off));
      ret = tmp;
    }
    return ret;
  }

  typedef const pointer_type PT;
  typedef const reference_type RT;
  PT* pt = 0;
  RT* rt = 0;
  if (m_type->m_id == type::POINTER)
    pt = static_cast<PT*>(m_type);
  else {
    assert(m_type->m_id == type::REFERENCE);
    rt = static_cast<RT*>(m_type);
  }

  var* v = m_addrof.m_ref;
  assert(v->usr_cast());
  usr* u = static_cast<usr*>(v);

  var* ret = 0;
  if (m_type->m_id == type::POINTER)
    ret = new addrof(pt,u,m_addrof.m_offset);
  else
    ret = new addrof(rt,u,m_addrof.m_offset);

  garbage.push_back(ret);
  return ret;
}

cxx_compiler::var* cxx_compiler::refsomewhere::address()
{
  using namespace std;
  var* ret = new var(m_type);
  if ( scope::current->m_id == scope::BLOCK ){
    vector<var*>& v = garbage;
    vector<var*>::reverse_iterator p = find(v.rbegin(),v.rend(),this);
    assert(p != v.rend());
    v.erase(p.base()-1);
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(this);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  code.push_back(new addr3ac(this,m_ref));
  code.push_back(new add3ac(ret,this,m_offset));
  return ret;
}

cxx_compiler::var* cxx_compiler::var::plus()
{
  var* expr = rvalue();
  const type* T = expr->m_type;
  if (!T->arithmetic()) {
    if (usr* func = operator_function(T, '+'))
      return call_impl::wrapper(func, 0, expr);
    using namespace error::expressions::unary;
    invalid(parse::position,'+',T);
  }
  T = T->promotion();
  expr = expr->cast(T);
  var* ret = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  code.push_back(new assign3ac(ret,expr));
  return ret;
}

cxx_compiler::var* cxx_compiler::var::minus()
{
  var* expr = rvalue();
  const type* T = expr->m_type;
  if (!T->arithmetic()) {
    if (tag* ptr = T->get_tag()) {
      tag::flag_t flag = ptr->m_flag;
      if (flag & tag::TYPENAMED)
	return expr;
    }
    if (usr* func = operator_function(T, '-'))
      return call_impl::wrapper(func, 0, expr);
    using namespace error::expressions::unary;
    invalid(parse::position,'-',T);
  }
  T = T->promotion();
  expr = expr->cast(T);
  var* ret = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  code.push_back(new uminus3ac(ret,expr));
  return ret;
}

namespace cxx_compiler { namespace constant_impl {
  template<class T> var* minus(constant<T>* p)
  {
    using namespace expressions::primary::literal;
    usr* ret = integer::create(-p->m_value);
    usr::flag_t f = p->m_flag;
    if (f & usr::SUB_CONST_LONG) {
      ret->m_type = p->m_type;
      ret->m_flag = usr::SUB_CONST_LONG;
    }
    return ret;
  }
} } // end of constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::minus()
  { return constant_impl::minus(this); }
  template<>
  var* constant<char>::minus()
  { return constant_impl::minus(this); }
  template<>
  var* constant<signed char>::minus()
  { return constant_impl::minus(this); }
  template<>
  var* constant<unsigned char>::minus()
  { return constant_impl::minus(this); }
  template<>
  var* constant<wchar_t>::minus()
  { return constant_impl::minus(this); }
  template<>
  var* constant<short int>::minus()
  { return constant_impl::minus(this); }
  template<>
  var* constant<unsigned short int>::minus()
  { return constant_impl::minus(this); }
  var* constant<int>::minus()
  { return constant_impl::minus(this); }
  var* constant<unsigned int>::minus()
  { return constant_impl::minus(this); }
  var* constant<long int>::minus()
  { return constant_impl::minus(this); }
  var* constant<unsigned long int>::minus()
  { return constant_impl::minus(this); }
  var* constant<__int64>::minus()
  {
    if (m_flag & CONST_PTR) {
      assert(sizeof(void*) < m_type->size());
      return var::minus();
    }
    return constant_impl::minus(this);
  }
  var* constant<unsigned __int64>::minus()
  { return constant_impl::minus(this); }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<float>::minus()
{
  using namespace expressions::primary::literal;
  return floating::create(-m_value);
}

cxx_compiler::var* cxx_compiler::constant<double>::minus()
{
  using namespace expressions::primary::literal;
  return floating::create(-m_value);
}

cxx_compiler::var* cxx_compiler::constant<long double>::minus()
{
  using namespace expressions::primary::literal;
  if ( generator::long_double ){
    int sz = long_double_type::create()->size();
    unsigned char* p = new unsigned char[sz];
    (*generator::long_double->minus)(p,b);
    return floating::create(p);
  }
  else
    return floating::create(-m_value);
}

cxx_compiler::var* cxx_compiler::var::size()
{
  if (var* size = m_type->vsize())
    return size;
  else {
    const type* T = m_type->complete_type();
    int n = T->size();
    if (!n) {
      using namespace error::expressions::unary::size;
      invalid(parse::position,m_type);
      n = 1;
    }
    using namespace expressions::unary;
    return sizeof_impl::common(n);
  }
}

cxx_compiler::var* cxx_compiler::generated::size()
{
  if ( var* size = m_org->vsize() )
    return size;
  else {
    int n = m_org->size();
    if ( !n ){
      using namespace error::expressions::unary::size;
      invalid(parse::position,m_org);
      n = 1;
    }
    using namespace expressions::unary;
    return sizeof_impl::common(n);
  }
}

cxx_compiler::var* cxx_compiler::ref::size()
{
  if ( var* size = m_result->vsize() )
    return size;
  else {
    int n = m_result->size();
    if ( !n ){
      using namespace error::expressions::unary::size;
      invalid(parse::position,m_result);
      n = 1;
    }
    using namespace expressions::unary;
    return sizeof_impl::common(n);
  }
}

cxx_compiler::var* cxx_compiler::var::_not()
{
  using namespace std;
  var* expr = rvalue();
  const type* T = expr->m_type;
  if (!T->scalar()) {
    if (usr* func = operator_function(T, '!'))
      return call_impl::wrapper(func, 0, expr);
    using namespace error::expressions::unary;
    invalid(parse::position,'!',T);
  }
  expr = expr->promotion();
  usr* zero = expressions::primary::literal::integer::create(0);
  usr* one = expressions::primary::literal::integer::create(1);
  var01* ret = new var01(int_type::create());
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  var* tmp = zero->cast(expr->m_type);
  goto3ac* goto1 = new goto3ac(goto3ac::NE,expr,tmp);
  code.push_back(goto1);
  ret->m_one = code.size();
  code.push_back(new assign3ac(ret,one));
  goto3ac* goto2 = new goto3ac;
  code.push_back(goto2);
  to3ac* to1 = new to3ac;
  code.push_back(to1);
  to1->m_goto.push_back(goto1);
  goto1->m_to = to1;
  ret->m_zero = code.size();
  code.push_back(new assign3ac(ret,zero));
  to3ac* to2 = new to3ac;
  code.push_back(to2);
  to2->m_goto.push_back(goto2);
  goto2->m_to = to2;
  return ret;
}

namespace cxx_compiler {
  template<>
  var* constant<bool>::_not()
  { return zero() ? expressions::primary::literal::integer::create(1) : expressions::primary::literal::integer::create(0); }
  template<>
  var* constant<char>::_not()
  { return zero() ? expressions::primary::literal::integer::create(1) : expressions::primary::literal::integer::create(0); }
  template<>
  var* constant<signed char>::_not()
  { return zero() ? expressions::primary::literal::integer::create(1) : expressions::primary::literal::integer::create(0); }
  template<>
  var* constant<unsigned char>::_not()
  { return zero() ? expressions::primary::literal::integer::create(1) : expressions::primary::literal::integer::create(0); }
  template<>
  var* constant<wchar_t>::_not()
  { return zero() ? expressions::primary::literal::integer::create(1) : expressions::primary::literal::integer::create(0); }
  template<>
  var* constant<short int>::_not()
  { return zero() ? expressions::primary::literal::integer::create(1) : expressions::primary::literal::integer::create(0); }
  template<>
  var* constant<unsigned short int>::_not()
  { return zero() ? expressions::primary::literal::integer::create(1) : expressions::primary::literal::integer::create(0); }
  var* constant<int>::_not()
  { return zero() ? expressions::primary::literal::integer::create(1) : expressions::primary::literal::integer::create(0); }
  var* constant<unsigned int>::_not()
  { return zero() ? expressions::primary::literal::integer::create(1) : expressions::primary::literal::integer::create(0); }
  var* constant<long int>::_not()
  { return zero() ? expressions::primary::literal::integer::create(1) : expressions::primary::literal::integer::create(0); }
  var* constant<unsigned long int>::_not()
  { return zero() ? expressions::primary::literal::integer::create(1) : expressions::primary::literal::integer::create(0); }
  var* constant<__int64>::_not()
  { return zero() ? expressions::primary::literal::integer::create(1) : expressions::primary::literal::integer::create(0); }
  var* constant<unsigned __int64>::_not()
  { return zero() ? expressions::primary::literal::integer::create(1) : expressions::primary::literal::integer::create(0); }
} // end of namespace cxx_comiler

cxx_compiler::var* cxx_compiler::constant<float>::_not()
{ return zero() ? expressions::primary::literal::integer::create(1) : expressions::primary::literal::integer::create(0); }
cxx_compiler::var* cxx_compiler::constant<double>::_not()
{ return zero() ? expressions::primary::literal::integer::create(1) : expressions::primary::literal::integer::create(0); }
cxx_compiler::var* cxx_compiler::constant<long double>::_not()
{ return zero() ? expressions::primary::literal::integer::create(1) : expressions::primary::literal::integer::create(0); }
cxx_compiler::var* cxx_compiler::constant<void*>::_not()
{ return zero() ? expressions::primary::literal::integer::create(1) : expressions::primary::literal::integer::create(0); }

bool cxx_compiler::constant<long double>::zero()
{
  return generator::long_double ?
    (*generator::long_double->zero)(b) : m_value == 0.0L;
}

cxx_compiler::var* cxx_compiler::var::tilde()
{
  var* expr = rvalue();
  const type* T = expr->m_type;
  if (!T->arithmetic() || !T->integer()) {
    if (tag* ptr = T->get_tag()) {
      tag::flag_t flag = ptr->m_flag;
      if (flag & tag::TYPENAMED)
	return expr;
    }
    if (usr* func = operator_function(T, '~'))
      return call_impl::wrapper(func, 0, expr);
    using namespace error::expressions::unary;
    invalid(parse::position,'~',T);
  }
  T = T->promotion();
  expr = expr->cast(T);
  var* ret = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  code.push_back(new tilde3ac(ret,expr));
  return ret;
}

namespace cxx_compiler { namespace constant_impl {
  template<class T> var* tilde(constant<T>* y)
  {
    using namespace expressions::primary::literal;
    usr* ret = integer::create(~y->m_value);
    usr::flag_t f = y->m_flag;
    if (f & usr::SUB_CONST_LONG) {
      ret->m_type = y->m_type;
      ret->m_flag = usr::SUB_CONST_LONG;
    }
    return ret;
  }
} } // end of namespace constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::tilde()
  { return constant_impl::tilde(this); }
  template<>
  var* constant<char>::tilde()
  { return constant_impl::tilde(this); }
  template<>
  var* constant<signed char>::tilde()
  { return constant_impl::tilde(this); }
  template<>
  var* constant<unsigned char>::tilde()
  { return constant_impl::tilde(this); }
  template<>
  var* constant<wchar_t>::tilde()
  { return constant_impl::tilde(this); }
  template<>
  var* constant<short int>::tilde()
  { return constant_impl::tilde(this); }
  template<>
  var* constant<unsigned short int>::tilde()
  { return constant_impl::tilde(this); }
  var* constant<int>::tilde()
  { return constant_impl::tilde(this); }
  var* constant<unsigned int>::tilde()
  { return constant_impl::tilde(this); }
  var* constant<long int>::tilde()
  { return constant_impl::tilde(this); }
  var* constant<unsigned long int>::tilde()
  { return constant_impl::tilde(this); }
  var* constant<__int64>::tilde()
  {
    if (m_flag & CONST_PTR) {
      assert(sizeof(void*) < m_type->size());
      return var::tilde();
    }
    return constant_impl::tilde(this);
  }
  var* constant<unsigned __int64>::tilde()
  { return constant_impl::tilde(this); }
} // end of namespace cxx_compiler
