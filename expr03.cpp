// cast-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

namespace cxx_compiler { namespace expressions { namespace cast {
  const type* valid(const type*, var*);
} } } // end of namespace cast, expressions and cxx_compiler

cxx_compiler::var* cxx_compiler::expressions::cast::info_t::gen()
{
  var* expr = m_expr->gen();
  expr = expr->rvalue();
  if ( m_type->compatible(void_type::create()) ){
    var* ret = new var(void_type::create());
    garbage.push_back(ret);
    return ret;
  }
  if ( !m_type->scalar() ){
    using namespace error::expressions::cast;
    not_scalar(parse::position);
    m_type = int_type::create();
  }
  m_type = valid(m_type,expr);
  if ( !m_type ){
    using namespace error::expressions::cast;
    invalid(parse::position);
    m_type = int_type::create();
  }
  return expr->cast(m_type);
}

const cxx_compiler::file_t& cxx_compiler::expressions::cast::info_t::file() const
{
  return m_expr->file();
}

const cxx_compiler::type* cxx_compiler::expressions::cast::valid(const type* T, var* y)
{
  if ( const type* r = assignment::valid(T,y,0) )
    return r;
  const type* Tx = T->unqualified();
  const type* Ty = y->m_type->unqualified();
  if ( Tx->m_id == type::POINTER )
    return Ty->real() ? 0 : T;
  if ( Ty->m_id == type::POINTER )
    return Tx->real() ? 0 : T;
  return 0;
}

cxx_compiler::var* cxx_compiler::var::cast(const type* T)
{
  if ( T == m_type && !lvalue() )
    return this;
  var* ret = new var(T);
  if ( T->compatible(void_type::create()) ){
    garbage.push_back(ret);
    return ret;
  }
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  if ( T->compatible(m_type) )
    code.push_back(new assign3ac(ret,this));
  else
    code.push_back(new cast3ac(ret,this,T));
  return ret;
}

cxx_compiler::var* cxx_compiler::addrof::cast(const type* T)
{
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  if ( b && !expressions::constant_flag )
    return var::cast(T);
  if ( T == m_type )
    return this;
  else {
    var* ret = new addrof(T,m_ref,m_offset);
    garbage.push_back(ret);
    return ret;
  }
}

namespace cxx_compiler { namespace constant_impl {
  var* cast_ld(long double);
} } // end of namespace constant_impl and c_compiler

cxx_compiler::var* cxx_compiler::constant_impl::cast_ld(long double ld)
{
  using namespace expressions::primary::literal;
  if ( generator::long_double ){
    int sz = long_double_type::create()->size();
    unsigned char* p = new unsigned char[sz];
    (*generator::long_double->from_double)(p,ld);
    return floating::create(p);
  }
  else
    return floating::create(ld);
}

namespace cxx_compiler { namespace constant_impl {
  template<class T> var* cast(const type* type, constant<T>* y)
  {
    using namespace std;
    using namespace expressions::primary::literal;
    if ( type->compatible(y->m_type) )
      return y;
    else if ( type->compatible(char_type::create()) )
      return integer::create((char)(y->m_value));
    else if ( type->compatible(schar_type::create()) )
      return integer::create((signed char)(y->m_value));
    else if ( type->compatible(uchar_type::create()) )
      return integer::create((unsigned char)(y->m_value));
    else if ( type->compatible(wchar_type::create()) )
      return integer::create((wchar_typedef)(y->m_value));
    else if ( type->compatible(short_type::create()) )
      return integer::create((short int)(y->m_value));
    else if ( type->compatible(ushort_type::create()) )
      return integer::create((unsigned short int)(y->m_value));
    else if ( type->compatible(int_type::create()) )
      return integer::create((int)(y->m_value));
    else if ( type->compatible(uint_type::create()) )
      return integer::create((unsigned int)(y->m_value));
    else if ( type->compatible(long_type::create()) )
      return integer::create((long int)(y->m_value));
    else if ( type->compatible(ulong_type::create()) )
      return integer::create((unsigned long int)(y->m_value));
    else if ( type->compatible(long_long_type::create()) )
      return integer::create((__int64)(y->m_value));
    else if ( type->compatible(ulong_long_type::create()) )
      return integer::create((unsigned __int64)(y->m_value));
    else if ( type->compatible(float_type::create()) )
      return floating::create((float)y->m_value);
    else if ( type->compatible(double_type::create()) )
      return floating::create((double)y->m_value);
    else if ( type->compatible(long_double_type::create()) )
      return cast_ld(y->m_value);
    else if ( type->m_id == type::POINTER )
      return pointer::create(type,(void*)y->m_value);
    else
      return y->var::cast(type);
  }
  template<class T> var* fcast(const type* type, constant<T>* y)
  {
    using namespace std;
    using namespace expressions::primary::literal;
    if ( type->compatible(y->m_type) )
      return y;
    else if ( type->compatible(char_type::create()) )
      return integer::create((char)(y->m_value));
    else if ( type->compatible(schar_type::create()) )
      return integer::create((signed char)(y->m_value));
    else if ( type->compatible(uchar_type::create()) )
      return integer::create((unsigned char)(y->m_value));
    else if ( type->compatible(short_type::create()) )
      return integer::create((short int)(y->m_value));
    else if ( type->compatible(ushort_type::create()) )
      return integer::create((unsigned short int)(y->m_value));
    else if ( type->compatible(int_type::create()) )
      return integer::create((int)(y->m_value));
    else if ( type->compatible(uint_type::create()) )
      return integer::create((unsigned int)(y->m_value));
    else if ( type->compatible(long_type::create()) )
      return integer::create((long int)(y->m_value));
    else if ( type->compatible(ulong_type::create()) )
      return integer::create((unsigned long int)(y->m_value));
    else if ( type->compatible(long_long_type::create()) )
      return integer::create((__int64)(y->m_value));
    else if ( type->compatible(ulong_long_type::create()) )
      return integer::create((unsigned __int64)(y->m_value));
    else if ( type->compatible(float_type::create()) )
      return floating::create((float)y->m_value);
    else if ( type->compatible(double_type::create()) )
      return floating::create((double)y->m_value);
    else if ( type->compatible(long_double_type::create()) )
      return cast_ld(y->m_value);
    else
      return y->var::cast(type);
  }
  template<class T> var* pcast(const type* type, constant<T>* y)
  {
    using namespace std;
    using namespace expressions::primary::literal;
    if ( type->compatible(y->m_type) )
      return y;
    else if ( type->compatible(char_type::create()) )
      return integer::create((char)(__int64)(y->m_value));
    else if ( type->compatible(schar_type::create()) )
      return integer::create((signed char)(__int64)(y->m_value));
    else if ( type->compatible(uchar_type::create()) )
      return integer::create((unsigned char)(__int64)(y->m_value));
    else if ( type->compatible(short_type::create()) )
      return integer::create((short int)(__int64)(y->m_value));
    else if ( type->compatible(ushort_type::create()) )
      return integer::create((unsigned short int)(__int64)(y->m_value));
    else if ( type->compatible(int_type::create()) )
      return integer::create((__int64)(y->m_value));
    else if ( type->compatible(uint_type::create()) )
      return integer::create((unsigned int)(__int64)(y->m_value));
    else if ( type->compatible(long_type::create()) )
      return integer::create((long int)(y->m_value));
    else if ( type->compatible(ulong_type::create()) )
      return integer::create((unsigned long int)(y->m_value));
    else if ( type->compatible(long_long_type::create()) )
      return integer::create((__int64)(y->m_value));
    else if ( type->compatible(ulong_long_type::create()) )
      return integer::create((unsigned __int64)(y->m_value));
    else if ( type->m_id == type::POINTER )
      return pointer::create(type,(void*)y->m_value);
    else
      return y->var::cast(type);
  }
#ifdef _MSC_VER
  template<> var* cast(const type* type, constant<unsigned __int64>* y)
  {
    using namespace std;
    using namespace expressions::primary::literal;
    if ( type->compatible(y->m_type) )
      return y;
    else if ( type->compatible(char_type::create()) )
      return integer::create((char)(y->m_value));
    else if ( type->compatible(schar_type::create()) )
      return integer::create((signed char)(y->m_value));
    else if ( type->compatible(uchar_type::create()) )
      return integer::create((unsigned char)(y->m_value));
    else if ( type->compatible(short_type::create()) )
      return integer::create((short int)(y->m_value));
    else if ( type->compatible(ushort_type::create()) )
      return integer::create((unsigned short int)(y->m_value));
    else if ( type->compatible(int_type::create()) )
      return integer::create((int)(y->m_value));
    else if ( type->compatible(uint_type::create()) )
      return integer::create((unsigned int)(y->m_value));
    else if ( type->compatible(long_type::create()) )
      return integer::create((long int)(y->m_value));
    else if ( type->compatible(ulong_type::create()) )
      return integer::create((unsigned long int)(y->m_value));
    else if ( type->compatible(long_long_type::create()) )
      return integer::create((__int64)(y->m_value));
    else if ( type->compatible(ulong_long_type::create()) )
      return integer::create((unsigned __int64)(y->m_value));
    else if ( type->compatible(float_type::create()) )
      return floating::create((float)(__int64)y->m_value);
    else if ( type->compatible(double_type::create()) )
      return floating::create((double)(__int64)y->m_value);
    else if ( type->compatible(long_double_type::create()) )
      return cast_ld((__int64)y->m_value);
    else if ( type->m_id == type::POINTER )
      return pointer::create(type,(void*)y->m_value);
    else
      return y->var::cast(type);
  }
#endif // _MSC_VER
} } // end of namespace constant_impl and c_compmiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<char>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<signed char>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<unsigned char>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<short int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<unsigned short int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<unsigned int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<long int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<unsigned long int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<__int64>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<unsigned __int64>::cast(const type* type)
  { return constant_impl::cast(type,this); }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<float>::cast(const type* type)
{ return constant_impl::fcast(type,this); }
cxx_compiler::var* cxx_compiler::constant<double>::cast(const type* type)
{ return constant_impl::fcast(type,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::cast(const type* type)
{
  using namespace expressions::primary::literal;
  if ( generator::long_double ){
    if ( type->compatible(long_double_type::create()) )
      return this;
    else {
      double d = (*generator::long_double->to_double)(b);
      usr* tmp = floating::create(d);
      return tmp->cast(type);
    }
  }
  else
    return constant_impl::fcast(type,this);
}
cxx_compiler::var* cxx_compiler::constant<void*>::cast(const type* type)
{ return constant_impl::pcast(type,this); }
