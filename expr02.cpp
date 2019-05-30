// unary-exrepssion
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

cxx_compiler::var* cxx_compiler::expressions::unary::ppmm::gen()
{
  var* expr = m_expr->gen();
  return expr->ppmm(m_plus,false);
}

const cxx_compiler::file_t& cxx_compiler::expressions::unary::ppmm::file() const
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
    if ( var* size = T->vsize() )
      return size;
    unsigned int n = T->size();
    if ( !n ){
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

cxx_compiler::var* cxx_compiler::expressions::unary::new_expr::gen()
{
  using namespace std;
  int n = m_T->size();
  var* sz = sizeof_impl::common(n);
  code.push_back(new param3ac(sz));
  string name = "new";
  map<string, vector<usr*> >& usrs = scope::root.m_usrs;
  map<string, vector<usr*> >::const_iterator it = usrs.find(name);
  const type* vp = pointer_type::create(void_type::create());
  usr* new_entry = 0;
  if (it != usrs.end()) {
    const vector<usr*>& v = it->second;
    if (v.size() != 1)
      error::not_implemented();
    new_entry = v.back();
  }
  else {
    vector<const type*> param;
    param.push_back(uint_type::create());
    const func_type* ft = func_type::create(vp,param);
    usr::flag_t flag = usr::flag_t(usr::FUNCTION | usr::NEW_SCALAR);
    new_entry = new usr(name,ft,flag,file_t());
    new_entry->m_scope = &scope::root;
    usrs[name].push_back(new_entry);
  }
  var* ret = new var(pointer_type::create(m_T));
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  code.push_back(new call3ac(ret,new_entry));
  const type* U = m_T->unqualified();
  if (U->m_id == type::RECORD) {
    typedef const record_type REC;
    REC* rec = static_cast<REC*>(U);
    tag* ptr = rec->get_tag();
    map<string, vector<usr*> >& usrs = ptr->m_usrs;
    string name = ptr->m_name;
    map<string, vector<usr*> >::const_iterator p = usrs.find(name);
    if (p != usrs.end()) {
      const vector<usr*>& v = p->second;
      if (v.size() != 1)
        error::not_implemented();
      usr* ctor = v.back();
      const type* T = ctor->m_type;
      assert(T->m_id == type::FUNC);
      typedef const func_type FT;
      FT* ft = static_cast<FT*>(T);
      vector<var*> arg;
      call_impl::common(ft, ctor, &arg, false, ret, false, 0);
      usr::flag_t flag = ctor->m_flag;
      if (!error::counter && !cmdline::no_inline_sub) {
        if (flag & usr::INLINE) {
          using namespace declarations::declarators::function;
          using namespace definition::static_inline::skip;
          table_t::const_iterator p = stbl.find(ctor);
          if (p != stbl.end())
            substitute(code, code.size()-1, p->second);
        }
      }
    }
  }
  return ret;
}

cxx_compiler::var* cxx_compiler::expressions::unary::delete_expr::gen()
{
  using namespace std;
  string name = "delete";
  map<string, vector<usr*> >& usrs = scope::root.m_usrs;
  map<string, vector<usr*> >::const_iterator it = usrs.find(name);
  usr* delete_entry = 0;
  if ( it != usrs.end() ){
    const vector<usr*>& v = it->second;
    if (v.size() != 1)
      error::not_implemented();
    delete_entry = v.back();
  }
  else {
    vector<const type*> param;
    const type* vp = pointer_type::create(void_type::create());
    param.push_back(vp);
    const func_type* ft = func_type::create(vp,param);
    usr::flag_t flag = usr::flag_t(usr::FUNCTION | usr::DELETE_SCALAR);
    delete_entry = new usr(name,ft,flag,file_t());
    delete_entry->m_scope = &scope::root;
    usrs[name].push_back(delete_entry);
  }
  var* v = m_expr->gen();
  code.push_back(new param3ac(v));
  var* ret = new var(void_type::create());
  garbage.push_back(ret);
  code.push_back(new call3ac(ret, delete_entry));
  return ret;
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
  if ( T->m_id != type::POINTER ){
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
  using namespace error::expressions::unary::address;
  not_lvalue(parse::position);
  return this;
}

cxx_compiler::var* cxx_compiler::usr::address()
{
  if ( !lvalue() ){
    using namespace error::expressions::unary::address;
    not_lvalue(parse::position);
  }
  typedef const pointer_type PT;
  PT* pt = pointer_type::create(m_type);
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  if ( b && !expressions::constant_flag ){
    var* x = new var(pt);
    b->m_vars.push_back(x);
    code.push_back(new addr3ac(x,this));
    return x;
  }
  else {
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
  if ( !lvalue() ){
    using namespace error::expressions::unary::address;
    not_lvalue(parse::position);
    return this;
  }
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  if ( b && !expressions::constant_flag ){
    const type* T = m_type;
    var* ret = new var(T);
    b->m_vars.push_back(ret);
    code.push_back(new addr3ac(ret,m_addrof.m_ref));
    if ( m_addrof.m_offset ){
      usr* off = expressions::primary::literal::integer::create(m_addrof.m_offset);
      var* tmp = new var(T);
      b->m_vars.push_back(tmp);
      code.push_back(new add3ac(tmp,ret,off));
      ret = tmp;
    }
    return ret;
  }
  else {
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(m_type);
    usr* u = static_cast<usr*>(m_addrof.m_ref);
    var* ret = new addrof(pt,u,m_addrof.m_offset);
    garbage.push_back(ret);
    return ret;
  }
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
  if ( !T->arithmetic() ){
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
  if ( !T->arithmetic() ){
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
  if ( var* size = m_type->vsize() )
    return size;
  else {
    const type* T = m_type->complete_type();
    int n = T->size();
    if ( !n ){
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
  if ( !T->scalar() ){
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
  if ( !T->arithmetic() || !T->integer() ){
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
