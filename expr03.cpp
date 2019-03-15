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
  const type* T = m_type->unqualified();
  if (T->m_id == type::VOID) {
    var* ret = new var(T);
    garbage.push_back(ret);
    return ret;
  }
  if (!T->scalar()) {
    using namespace error::expressions::cast;
    not_scalar(parse::position);
    T = int_type::create();
  }
  T = valid(T, expr);
  if (!T) {
    using namespace error::expressions::cast;
    invalid(parse::position);
    T = int_type::create();
  }
  return expr->cast(T);
}

const cxx_compiler::file_t& cxx_compiler::expressions::cast::info_t::file() const
{
  return m_expr->file();
}

const cxx_compiler::type* cxx_compiler::expressions::cast::valid(const type* T, var* y)
{
  if (assignment::valid(T,y,0))
    return T;
  const type* Tx = T->unqualified();
  const type* Ty = y->m_type->unqualified();
  if ( Tx->m_id == type::POINTER )
    return Ty->real() ? 0 : T;
  if ( Ty->m_id == type::POINTER )
    return Tx->real() ? 0 : T;
  return 0;
}

namespace cxx_compiler {
  namespace cast_impl {
    inline bool require(const type* Tx, const type* Ty)
    {
      Tx = Tx->unqualified();
      Ty = Ty->unqualified();
      if (compatible(Tx, Ty))
        return false;
      if (Tx->m_id == type::POINTER && Ty->m_id == type::POINTER) {
        typedef const pointer_type PT;
        PT* Px = static_cast<PT*>(Tx);
        PT* Py = static_cast<PT*>(Ty);
        Tx = Px->referenced_type();
        Ty = Py->referenced_type();
        Tx = Tx->unqualified();
        Ty = Ty->unqualified();
        return !compatible(Tx, Ty);
      }
      return true;
    }
  }  // end of namespace cast_impl
}  // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::var::cast(const type* T)
{
  if ( T == m_type )
    return this;
  var* ret = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  if (!cast_impl::require(T, m_type))
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
  using namespace expressions::primary::literal;
  template<class T> var* cast(const type* Tx, constant<T>* y)
  {
    using namespace std;
    if (!cast_impl::require(Tx, y->m_type))
      return y;
    Tx = Tx->unqualified();
    switch (Tx->m_id) {
    case type::BOOL: return integer::create((bool)(y->m_value));
    case type::CHAR: return integer::create((char)(y->m_value));
    case type::SCHAR: return integer::create((signed char)(y->m_value));
    case type::UCHAR: return integer::create((unsigned char)(y->m_value));
    case type::WCHAR: return integer::create((wchar_t)(y->m_value));
    case type::SHORT: return integer::create((short int)(y->m_value));
    case type::USHORT: return integer::create((unsigned short int)(y->m_value));
    case type::INT: return integer::create((int)(y->m_value));
    case type::UINT: return integer::create((unsigned int)(y->m_value));
    case type::LONG:
      {
        typedef long int X;
        if (Tx->size() <= sizeof(X))
          return integer::create((X)(y->m_value));
        typedef long long int XX;
        assert(Tx->size() == sizeof(XX));
        usr* ret = integer::create((XX)(y->m_value));
        ret->m_type = const_type::create(long_type::create());
        ret->m_flag = usr::SUB_CONST_LONG;
        return ret;
      }
    case type::ULONG:
      {
        typedef unsigned long int X;
        if (Tx->size() <= sizeof(X))
          return integer::create((X)(y->m_value));
        typedef unsigned long long int XX;
        assert(Tx->size() == sizeof(XX));
        usr* ret = integer::create((XX)(y->m_value));
        ret->m_type = const_type::create(ulong_type::create());
        ret->m_flag = usr::SUB_CONST_LONG;
        return ret;
      }
    case type::LONGLONG: return integer::create((__int64)(y->m_value));
    case type::ULONGLONG:
      return integer::create((unsigned __int64)(y->m_value));
    case type::FLOAT: return floating::create((float)y->m_value);
    case type::DOUBLE: return floating::create((double)y->m_value);
    case type::LONG_DOUBLE: return cast_ld(y->m_value);
    case type::POINTER:
      {
        if (sizeof(void*) >= Tx->size())
          return pointer::create(Tx,(void*)y->m_value);
        else
          return pointer::create(Tx,(__int64)y->m_value);
      }
    case type::ENUM:
      {
        typedef const enum_type ET;
        ET* et = static_cast<ET*>(Tx);
        return cast(et->get_integer(),y);
      }
    default:
      return y->var::cast(Tx);
    }
  }
  template<class T> var* fcast(const type* Tx, constant<T>* y)
  {
    using namespace std;
    if (!cast_impl::require(Tx, y->m_type))
      return y;
    Tx = Tx->unqualified();    
    switch (Tx->m_id) {
    case type::BOOL: return integer::create((bool)(y->m_value));
    case type::CHAR: return integer::create((char)(y->m_value));
    case type::SCHAR: return integer::create((signed char)(y->m_value));
    case type::UCHAR: return integer::create((unsigned char)(y->m_value));
    case type::WCHAR: return integer::create((wchar_t)(y->m_value));
    case type::SHORT: return integer::create((short int)(y->m_value));
    case type::USHORT: return integer::create((unsigned short int)(y->m_value));
    case type::INT: return integer::create((int)(y->m_value));
    case type::UINT: return integer::create((unsigned int)(y->m_value));
    case type::LONG:
      {
        typedef long int X;
        if (Tx->size() <= sizeof(X))
          return integer::create((X)(y->m_value));
        typedef long long int XX;
        assert(Tx->size() == sizeof(XX));
        usr* ret = integer::create((XX)(y->m_value));
        ret->m_type = const_type::create(long_type::create());
        ret->m_flag = usr::SUB_CONST_LONG;
        return ret;
      }
    case type::ULONG:
      {
        typedef unsigned long int X;
        if (Tx->size() <= sizeof(X))
          return integer::create((X)(y->m_value));
        typedef unsigned long long int XX;
        assert(Tx->size() == sizeof(XX));
        usr* ret = integer::create((XX)(y->m_value));
        ret->m_type = const_type::create(ulong_type::create());
        ret->m_flag = usr::SUB_CONST_LONG;
        return ret;
      }
    case type::LONGLONG: return integer::create((__int64)(y->m_value));
    case type::ULONGLONG:
      return integer::create((unsigned __int64)(y->m_value));
    case type::FLOAT: return floating::create((float)y->m_value);
    case type::DOUBLE: return floating::create((double)y->m_value);
    case type::LONG_DOUBLE: return cast_ld(y->m_value);
    case type::ENUM:
      {
        typedef const enum_type ET;
        ET* et = static_cast<ET*>(Tx);
        return fcast(et->get_integer(),y);
      }
    default:
      return y->var::cast(Tx);
    }
  }
  template<class T> var* pcast(const type* Tx, constant<T>* y)
  {
    using namespace std;
    if (!cast_impl::require(Tx, y->m_type))
      return y;
    Tx = Tx->unqualified();    
    switch (Tx->m_id) {
    case type::BOOL: return integer::create((bool)(__int64)y->m_value);
    case type::CHAR: return integer::create((char)(__int64)y->m_value);
    case type::SCHAR: return integer::create((signed char)(__int64)y->m_value);
    case type::UCHAR:
      return integer::create((unsigned char)(__int64)y->m_value);
    case type::WCHAR: return integer::create((wchar_t)(__int64)y->m_value);
    case type::SHORT: return integer::create((short int)(__int64)y->m_value);
    case type::USHORT:
      return integer::create((unsigned short int)(__int64)y->m_value);
    case type::INT: return integer::create((int)(__int64)(y->m_value));
    case type::UINT:
      return integer::create((unsigned int)(__int64)(y->m_value));
    case type::LONG:
      {
        typedef long int X;
        if (Tx->size() <= sizeof(X))
          return integer::create((X)(y->m_value));
        typedef long long int XX;
        assert(Tx->size() == sizeof(XX));
        usr* ret = integer::create((XX)(y->m_value));
        ret->m_type = const_type::create(long_type::create());
        ret->m_flag = usr::SUB_CONST_LONG;
        return ret;
      }
    case type::ULONG:
      {
        typedef unsigned long int X;
        if (Tx->size() <= sizeof(X))
          return integer::create((X)(y->m_value));
        typedef unsigned long long int XX;
        assert(Tx->size() == sizeof(XX));
        usr* ret = integer::create((XX)(y->m_value));
        ret->m_type = const_type::create(ulong_type::create());
        ret->m_flag = usr::SUB_CONST_LONG;
        return ret;
      }
    case type::LONGLONG: return integer::create((__int64)(y->m_value));
    case type::ULONGLONG: return integer::create((unsigned __int64)y->m_value);
    case type::POINTER:
      {
        if (sizeof(void*) >= Tx->size())
          return pointer::create(Tx,(void*)y->m_value);
        else
          return pointer::create(Tx,(__int64)y->m_value);
      }
    case type::ENUM:
      {
        typedef const enum_type ET;
        ET* et = static_cast<ET*>(Tx);
        return pcast(et->get_integer(),y);
      }
    default:
      return y->var::cast(Tx);
    }
  }
#ifdef _MSC_VER
  template<> var* cast(const type* Tx, constant<unsigned __int64>* y)
  {
    using namespace std;
    if (!cast_impl::require(Tx, y->m_type)) 
      return y;
    Tx = Tx->unqualified();
    switch (Tx->m_id) {
    case type::BOOL: return integer::create((bool)(y->m_value));
    case type::CHAR: return integer::create((char)(y->m_value));
    case type::SCHAR: return integer::create((signed char)(y->m_value));
    case type::UCHAR: return integer::create((unsigned char)(y->m_value));
    case type::WCHAR: return integer::create((wchar_t)(y->m_value));
    case type::SHORT: return integer::create((short int)(y->m_value));
    case type::USHORT: return integer::create((unsigned short int)(y->m_value));
    case type::INT: return integer::create((int)(y->m_value));
    case type::UINT: return integer::create((unsigned int)(y->m_value));
    case type::LONG:
      {
        typedef long int X;
        if (Tx->size() <= sizeof(X))
          return integer::create((X)(y->m_value));
        typedef long long int XX;
        assert(Tx->size() == sizeof(XX));
        usr* ret = integer::create((XX)(y->m_value));
        ret->m_type = const_type::create(long_type::create());
        ret->m_flag = usr::SUB_CONST_LONG;
        return ret;
      }
    case type::ULONG:
      {
        typedef unsigned long int X;
        if (Tx->size() <= sizeof(X))
          return integer::create((X)(y->m_value));
        typedef unsigned long long int XX;
        assert(Tx->size() == sizeof(XX));
        usr* ret = integer::create((XX)(y->m_value));
        ret->m_type = const_type::create(ulong_type::create());
        ret->m_flag = usr::SUB_CONST_LONG;
        return ret;
      }
    case type::LONGLONG: return integer::create((__int64)(y->m_value));
    case type::ULONGLONG:
      return integer::create((unsigned __int64)(y->m_value));
    case type::FLOAT: return floating::create((float)(__int64)y->m_value);
    case type::DOUBLE: return floating::create((double)(__int64)y->m_value);
    case type::LONG_DOUBLE: return cast_ld((__int64)y->m_value);
    case type::POINTER:
      {
        if (sizeof(void*) >= Tx->size())
          return pointer::create(Tx,(void*)y->m_value);
        else
          return pointer::create(Tx,(__int64)y->m_value);
      }
    case type::ENUM:
      {
        typedef const enum_type ET;
        ET* et = static_cast<ET*>(Tx);
        return cast(et->get_integer(),y);
      }
    default:
        return y->var::cast(Tx);
    }
  }
#endif // _MSC_VER
} } // end of namespace constant_impl and cxx_compmiler

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
  var* constant<wchar_t>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<short int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<unsigned short int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  var* constant<int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  var* constant<unsigned int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  var* constant<long int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  var* constant<unsigned long int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  var* constant<__int64>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  var* constant<unsigned __int64>::cast(const type* type)
  { return constant_impl::cast(type,this); }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<float>::cast(const type* type)
{ return constant_impl::fcast(type,this); }
cxx_compiler::var* cxx_compiler::constant<double>::cast(const type* type)
{ return constant_impl::fcast(type,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::cast(const type* T)
{
  using namespace expressions::primary::literal;
  if ( generator::long_double ){
    T = T->unqualified();
    if (T->m_id == type::LONG_DOUBLE)
      return this;
    else {
      double d = (*generator::long_double->to_double)(b);
      usr* tmp = floating::create(d);
      return tmp->cast(T);
    }
  }
  else
    return constant_impl::fcast(T,this);
}
cxx_compiler::var* cxx_compiler::constant<void*>::cast(const type* type)
{ return constant_impl::pcast(type,this); }
