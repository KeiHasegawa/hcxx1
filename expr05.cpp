// multiplicative-expression
// additive-expression
// shift-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

namespace cxx_compiler { namespace var_impl {
  var* mul(var*, var*);
  var* opt_mul(var*, var*);
} } // end of namespace var_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::var_impl::mul(var* a, var* b)
{
  var* y = a->rvalue();
  var* z = b->rvalue();
  const type* T = conversion::arithmetic::gen(&y,&z);
  if ( !T ){
    using namespace error::expressions::binary;
    invalid(parse::position,'*',y->m_type,z->m_type);
    T = int_type::create();
  }
  if ( var* x = opt_mul(y,z) )
    return x;
  if ( var* x = opt_mul(z,y) )
    return x;
  var* x = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new mul3ac(x,y,z));
  return x;
}

namespace cxx_compiler { namespace var_impl {
  int log2(unsigned int);
} } // end of namespace var_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::var_impl::opt_mul(var* y, var* z)
{
  if ( !z->isconstant() )
    return 0;
  if ( z->zero() )
    return z;
  const type* T = z->m_type;
  if ( !T->integer() || T->size() > sizeof(int) )
    return 0;
  int n = z->value();
  n = log2(n);
  if ( n == -1 )
    return 0;
  var* x = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  var* pos = expressions::primary::literal::integer::create(n);
  switch ( n ){
  case 0: code.push_back(new assign3ac(x,y)); break;
  case 1: code.push_back(new add3ac(x,y,y)); break;
  default: code.push_back(new lsh3ac(x,y,pos)); break;
  }
  return x;
}

int cxx_compiler::var_impl::log2(unsigned int n)
{
  int i;
  for ( i = 0 ; n ; n >>= 1, ++i ){
    if ( n & 1 ){
      n &= ~1;
      break;
    }
  }
  return n ? -1 : i;
}


cxx_compiler::var* cxx_compiler::var::mul(var* z){ return var_impl::mul(this,z); }
cxx_compiler::var* cxx_compiler::var::mulr(constant<bool>* y){ return var_impl::mul(y,this); }
cxx_compiler::var* cxx_compiler::var::mulr(constant<char>* y){ return var_impl::mul(y,this); }
cxx_compiler::var* cxx_compiler::var::mulr(constant<signed char>* y){ return var_impl::mul(y,this); }
cxx_compiler::var* cxx_compiler::var::mulr(constant<unsigned char>* y){ return var_impl::mul(y,this); }
cxx_compiler::var* cxx_compiler::var::mulr(constant<short int>* y){ return var_impl::mul(y,this); }
cxx_compiler::var* cxx_compiler::var::mulr(constant<unsigned short int>* y){ return var_impl::mul(y,this); }
cxx_compiler::var* cxx_compiler::var::mulr(constant<int>* y){ return var_impl::mul(y,this); }
cxx_compiler::var* cxx_compiler::var::mulr(constant<unsigned int>* y){ return var_impl::mul(y,this); }
cxx_compiler::var* cxx_compiler::var::mulr(constant<long int>* y){ return var_impl::mul(y,this); }
cxx_compiler::var* cxx_compiler::var::mulr(constant<unsigned long int>* y){ return var_impl::mul(y,this); }
cxx_compiler::var* cxx_compiler::var::mulr(constant<__int64>* y){ return var_impl::mul(y,this); }
cxx_compiler::var* cxx_compiler::var::mulr(constant<unsigned __int64>* y){ return var_impl::mul(y,this); }
cxx_compiler::var* cxx_compiler::var::mulr(constant<float>* y){ return var_impl::mul(y,this); }
cxx_compiler::var* cxx_compiler::var::mulr(constant<double>* y){ return var_impl::mul(y,this); }
cxx_compiler::var* cxx_compiler::var::mulr(constant<long double>* y){ return var_impl::mul(y,this); }

namespace cxx_compiler { namespace constant_impl {
  template<class A, class B> var* mul(constant<A>* y, constant<B>* z)
  {
    return expressions::primary::literal::integer::create(y->m_value * z->m_value); 
  }
  template<class A, class B> var* fop1(constant<A>* y, constant<B>* z,
                                       void (*pf)(unsigned char*, const unsigned char*))
  {
    using namespace std;
    if ( y->m_type->compatible(long_double_type::create()) ){
      int sz = long_double_type::create()->size();
      unsigned char* p = new unsigned char[sz];
      constant<long double>* yy = reinterpret_cast<constant<long double>*>(y);
      memcpy(p,yy->b,sz);
      auto_ptr<unsigned char> q = auto_ptr<unsigned char>(new unsigned char[sz]);
#ifndef _MSC_VER
      (*generator::long_double->from_double)(q.get(),z->m_value);
#else // _MSC_VER
      (*generator::long_double->from_double)(q.get(),(__int64)z->m_value);
#endif // _MSC_VER
      (*pf)(p,q.get());
      return expressions::primary::literal::floating::create(p);
    }
    return 0;
  }
  template<class A, class B> var* fmul1(constant<A>* y, constant<B>* z)
  {
    if ( generator::long_double ){
      if ( var* v = fop1(y,z,generator::long_double->mul) )
        return v;
    }
#ifndef _MSC_VER
    return expressions::primary::literal::floating::create(y->m_value * z->m_value);
#else // _MSC_VER
    return expressions::primary::literal::floating::create(y->m_value * (__int64)z->m_value);
#endif // _MSC_VER
  }
  template<class A, class B> var* fop2(constant<A>* y, constant<B>* z,
                                       void (*pf)(unsigned char*, const unsigned char*))
  {
    if ( z->m_type->compatible(long_double_type::create()) ){
      int sz = long_double_type::create()->size();
      unsigned char* p = new unsigned char[sz];
#ifndef _MSC_VER
      (*generator::long_double->from_double)(p,y->m_value);
#else // _MSC_VER
      (*generator::long_double->from_double)(p,(__int64)y->m_value);
#endif // _MSC_VER
      constant<long double>* zz = reinterpret_cast<constant<long double>*>(z);
      (*pf)(p,zz->b);
      return expressions::primary::literal::floating::create(p);
    }
    return 0;
  }
  template<class A, class B> var* fmul2(constant<A>* y, constant<B>* z)
  {
    if ( generator::long_double ){
      if ( var* v = fop2(y,z,generator::long_double->mul) )
        return v;
    }
#ifndef _MSC_VER
    return expressions::primary::literal::floating::create(y->m_value * z->m_value);
#else // _MSC_VER
    return expressions::primary::literal::floating::create((__int64)y->m_value * z->m_value);
#endif // _MSC_VER
  }
  template<class A, class B> var* fop3(constant<A>* y, constant<B>* z,
                                       void (*pf)(unsigned char*, const unsigned char*))
  {
    using namespace std;
    int sz = long_double_type::create()->size();
    if ( y->m_type->compatible(long_double_type::create()) ){
      unsigned char* p = new unsigned char[sz];
      constant<long double>* yy = reinterpret_cast<constant<long double>*>(y);
      memcpy(p,yy->b,sz);
      if ( z->m_type->compatible(long_double_type::create()) ){
        constant<long double>* zz = reinterpret_cast<constant<long double>*>(z);
        (*pf)(p,zz->b);
        return expressions::primary::literal::floating::create(p);
      }
      auto_ptr<unsigned char> q = auto_ptr<unsigned char>(new unsigned char[sz]);
      (*generator::long_double->from_double)(q.get(),z->m_value);
      (*pf)(p,q.get());
      return expressions::primary::literal::floating::create(p);
    }
    if ( z->m_type->compatible(long_double_type::create()) ){
      unsigned char* p = new unsigned char[sz];
      (*generator::long_double->from_double)(p,y->m_value);
      constant<long double>* zz = reinterpret_cast<constant<long double>*>(z);
      (*pf)(p,zz->b);
      return expressions::primary::literal::floating::create(p);
    }
    return 0;
  }
  template<class A, class B> var* fmul3(constant<A>* y, constant<B>* z)
  {
    if ( generator::long_double ){
      if ( var* v = fop3(y,z,generator::long_double->mul) )
        return v;
    }
    return expressions::primary::literal::floating::create(y->m_value * z->m_value);
  }
} } // end of namespace constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::mulr(constant<bool>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<bool>::mulr(constant<char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<bool>::mulr(constant<signed char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<bool>::mulr(constant<unsigned char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<bool>::mulr(constant<short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<bool>::mulr(constant<unsigned short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<bool>::mulr(constant<int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<bool>::mulr(constant<unsigned int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<bool>::mulr(constant<long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<bool>::mulr(constant<unsigned long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<bool>::mulr(constant<__int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<bool>::mulr(constant<unsigned __int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<bool>::mulr(constant<float>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<bool>::mulr(constant<double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<bool>::mulr(constant<long double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<char>::mulr(constant<bool>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<char>::mulr(constant<char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<char>::mulr(constant<signed char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<char>::mulr(constant<unsigned char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<char>::mulr(constant<short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<char>::mulr(constant<unsigned short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<char>::mulr(constant<int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<char>::mulr(constant<unsigned int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<char>::mulr(constant<long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<char>::mulr(constant<unsigned long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<char>::mulr(constant<__int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<char>::mulr(constant<unsigned __int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<char>::mulr(constant<float>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<char>::mulr(constant<double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<char>::mulr(constant<long double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<signed char>::mulr(constant<bool>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<signed char>::mulr(constant<char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<signed char>::mulr(constant<signed char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<signed char>::mulr(constant<unsigned char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<signed char>::mulr(constant<short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<signed char>::mulr(constant<unsigned short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<signed char>::mulr(constant<int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<signed char>::mulr(constant<unsigned int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<signed char>::mulr(constant<long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<signed char>::mulr(constant<unsigned long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<signed char>::mulr(constant<__int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<signed char>::mulr(constant<unsigned __int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<signed char>::mulr(constant<float>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<signed char>::mulr(constant<double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<signed char>::mulr(constant<long double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<unsigned char>::mulr(constant<bool>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned char>::mulr(constant<char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned char>::mulr(constant<signed char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned char>::mulr(constant<unsigned char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned char>::mulr(constant<short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned char>::mulr(constant<unsigned short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned char>::mulr(constant<int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned char>::mulr(constant<unsigned int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned char>::mulr(constant<long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned char>::mulr(constant<unsigned long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned char>::mulr(constant<__int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned char>::mulr(constant<unsigned __int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned char>::mulr(constant<float>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<unsigned char>::mulr(constant<double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<unsigned char>::mulr(constant<long double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<short int>::mulr(constant<bool>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<short int>::mulr(constant<char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<short int>::mulr(constant<signed char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<short int>::mulr(constant<unsigned char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<short int>::mulr(constant<short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<short int>::mulr(constant<unsigned short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<short int>::mulr(constant<int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<short int>::mulr(constant<unsigned int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<short int>::mulr(constant<long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<short int>::mulr(constant<unsigned long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<short int>::mulr(constant<__int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<short int>::mulr(constant<unsigned __int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<short int>::mulr(constant<float>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<short int>::mulr(constant<double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<short int>::mulr(constant<long double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<unsigned short int>::mulr(constant<bool>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned short int>::mulr(constant<char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned short int>::mulr(constant<signed char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned short int>::mulr(constant<unsigned char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned short int>::mulr(constant<short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned short int>::mulr(constant<unsigned short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned short int>::mulr(constant<int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned short int>::mulr(constant<unsigned int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned short int>::mulr(constant<long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned short int>::mulr(constant<unsigned long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned short int>::mulr(constant<__int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned short int>::mulr(constant<unsigned __int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned short int>::mulr(constant<float>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<unsigned short int>::mulr(constant<double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<unsigned short int>::mulr(constant<long double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<int>::mulr(constant<bool>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<int>::mulr(constant<char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<int>::mulr(constant<signed char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<int>::mulr(constant<unsigned char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<int>::mulr(constant<short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<int>::mulr(constant<unsigned short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<int>::mulr(constant<int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<int>::mulr(constant<unsigned int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<int>::mulr(constant<long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<int>::mulr(constant<unsigned long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<int>::mulr(constant<__int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<int>::mulr(constant<unsigned __int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<int>::mulr(constant<float>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<int>::mulr(constant<double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<int>::mulr(constant<long double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<unsigned int>::mulr(constant<bool>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned int>::mulr(constant<char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned int>::mulr(constant<signed char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned int>::mulr(constant<unsigned char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned int>::mulr(constant<short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned int>::mulr(constant<unsigned short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned int>::mulr(constant<int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned int>::mulr(constant<unsigned int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned int>::mulr(constant<long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned int>::mulr(constant<unsigned long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned int>::mulr(constant<__int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned int>::mulr(constant<unsigned __int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned int>::mulr(constant<float>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<unsigned int>::mulr(constant<double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<unsigned int>::mulr(constant<long double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<long int>::mulr(constant<bool>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<long int>::mulr(constant<char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<long int>::mulr(constant<signed char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<long int>::mulr(constant<unsigned char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<long int>::mulr(constant<short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<long int>::mulr(constant<unsigned short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<long int>::mulr(constant<int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<long int>::mulr(constant<unsigned int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<long int>::mulr(constant<long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<long int>::mulr(constant<unsigned long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<long int>::mulr(constant<__int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<long int>::mulr(constant<unsigned __int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<long int>::mulr(constant<float>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<long int>::mulr(constant<double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<long int>::mulr(constant<long double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<unsigned long int>::mulr(constant<bool>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned long int>::mulr(constant<char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned long int>::mulr(constant<signed char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned long int>::mulr(constant<unsigned char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned long int>::mulr(constant<short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned long int>::mulr(constant<unsigned short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned long int>::mulr(constant<int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned long int>::mulr(constant<unsigned int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned long int>::mulr(constant<long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned long int>::mulr(constant<unsigned long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned long int>::mulr(constant<__int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned long int>::mulr(constant<unsigned __int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned long int>::mulr(constant<float>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<unsigned long int>::mulr(constant<double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<unsigned long int>::mulr(constant<long double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<__int64>::mulr(constant<bool>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<__int64>::mulr(constant<char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<__int64>::mulr(constant<signed char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<__int64>::mulr(constant<unsigned char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<__int64>::mulr(constant<short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<__int64>::mulr(constant<unsigned short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<__int64>::mulr(constant<int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<__int64>::mulr(constant<unsigned int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<__int64>::mulr(constant<long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<__int64>::mulr(constant<unsigned long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<__int64>::mulr(constant<__int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<__int64>::mulr(constant<unsigned __int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<__int64>::mulr(constant<float>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<__int64>::mulr(constant<double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<__int64>::mulr(constant<long double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<unsigned __int64>::mulr(constant<bool>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned __int64>::mulr(constant<char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned __int64>::mulr(constant<signed char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned __int64>::mulr(constant<unsigned char>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned __int64>::mulr(constant<short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned __int64>::mulr(constant<unsigned short int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned __int64>::mulr(constant<int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned __int64>::mulr(constant<unsigned int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned __int64>::mulr(constant<long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned __int64>::mulr(constant<unsigned long int>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned __int64>::mulr(constant<__int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned __int64>::mulr(constant<unsigned __int64>* y)
  { return constant_impl::mul(y,this); }
  template<>
  var* constant<unsigned __int64>::mulr(constant<float>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<unsigned __int64>::mulr(constant<double>* y)
  { return constant_impl::fmul1(y,this); }
  template<>
  var* constant<unsigned __int64>::mulr(constant<long double>* y)
  { return constant_impl::fmul1(y,this); }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<float>::mul(var* z){ return z->mulr(this); }
cxx_compiler::var* cxx_compiler::constant<float>::mulr(constant<bool>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::mulr(constant<char>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::mulr(constant<signed char>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::mulr(constant<unsigned char>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::mulr(constant<short int>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::mulr(constant<unsigned short int>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::mulr(constant<int>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::mulr(constant<unsigned int>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::mulr(constant<long int>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::mulr(constant<unsigned long int>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::mulr(constant<__int64>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::mulr(constant<unsigned __int64>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::mulr(constant<float>* y)
{ return constant_impl::fmul3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::mulr(constant<double>* y)
{ return constant_impl::fmul3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::mulr(constant<long double>* y)
{ return constant_impl::fmul3(y,this); }

cxx_compiler::var* cxx_compiler::constant<double>::mul(var* z){ return z->mulr(this); }
cxx_compiler::var* cxx_compiler::constant<double>::mulr(constant<bool>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::mulr(constant<char>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::mulr(constant<signed char>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::mulr(constant<unsigned char>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::mulr(constant<short int>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::mulr(constant<unsigned short int>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::mulr(constant<int>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::mulr(constant<unsigned int>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::mulr(constant<long int>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::mulr(constant<unsigned long int>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::mulr(constant<__int64>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::mulr(constant<unsigned __int64>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::mulr(constant<float>* y)
{ return constant_impl::fmul3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::mulr(constant<double>* y)
{ return constant_impl::fmul3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::mulr(constant<long double>* y)
{ return constant_impl::fmul3(y,this); }

cxx_compiler::var* cxx_compiler::constant<long double>::mul(var* z){ return z->mulr(this); }
cxx_compiler::var* cxx_compiler::constant<long double>::mulr(constant<bool>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::mulr(constant<char>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::mulr(constant<signed char>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::mulr(constant<unsigned char>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::mulr(constant<short int>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::mulr(constant<unsigned short int>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::mulr(constant<int>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::mulr(constant<unsigned int>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::mulr(constant<long int>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::mulr(constant<unsigned long int>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::mulr(constant<__int64>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::mulr(constant<unsigned __int64>* y)
{ return constant_impl::fmul2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::mulr(constant<float>* y)
{ return constant_impl::fmul3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::mulr(constant<double>* y)
{ return constant_impl::fmul3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::mulr(constant<long double>* y)
{ return constant_impl::fmul3(y,this); }

namespace cxx_compiler { namespace var_impl {
  var* div(var*, var*);
  var* opt_div(var*, var*);
} } // end of namespace var_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::var_impl::div(var* a, var* b)
{
  var* y = a->rvalue();
  var* z = b->rvalue();
  const type* T = conversion::arithmetic::gen(&y,&z);
  if ( !T ){
    using namespace error::expressions::binary;
    invalid(parse::position,'/',y->m_type,z->m_type);
    T = int_type::create();
  }
  if ( var* x = opt_div(y,z) )
    return x;
  var* x = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new div3ac(x,y,z));
  return x;
}

cxx_compiler::var* cxx_compiler::var_impl::opt_div(var* y, var* z)
{
  if ( !z->isconstant() )
    return 0;
  const type* T = z->m_type;
  if ( z->zero() ){
    if ( T->integer() ){
      using namespace warning;
      zero_division(parse::position);
    }
    return 0;
  }
  if ( !T->integer() || T->size() > sizeof(int) )
    return 0;
  int n = z->value();
  n = log2(n);
  if ( n == -1 )
    return 0;
  var* x = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  var* pos = expressions::primary::literal::integer::create(n);
  switch ( n ){
  case 0: code.push_back(new assign3ac(x,y)); break;
  default: code.push_back(new rsh3ac(x,y,pos)); break;
  }
  return x;
}

cxx_compiler::var* cxx_compiler::var::div(var* z){ return var_impl::div(this,z); }
cxx_compiler::var* cxx_compiler::var::divr(constant<bool>* y){ return var_impl::div(y,this); }
cxx_compiler::var* cxx_compiler::var::divr(constant<char>* y){ return var_impl::div(y,this); }
cxx_compiler::var* cxx_compiler::var::divr(constant<signed char>* y){ return var_impl::div(y,this); }
cxx_compiler::var* cxx_compiler::var::divr(constant<unsigned char>* y){ return var_impl::div(y,this); }
cxx_compiler::var* cxx_compiler::var::divr(constant<short int>* y){ return var_impl::div(y,this); }
cxx_compiler::var* cxx_compiler::var::divr(constant<unsigned short int>* y){ return var_impl::div(y,this); }
cxx_compiler::var* cxx_compiler::var::divr(constant<int>* y){ return var_impl::div(y,this); }
cxx_compiler::var* cxx_compiler::var::divr(constant<unsigned int>* y){ return var_impl::div(y,this); }
cxx_compiler::var* cxx_compiler::var::divr(constant<long int>* y){ return var_impl::div(y,this); }
cxx_compiler::var* cxx_compiler::var::divr(constant<unsigned long int>* y){ return var_impl::div(y,this); }
cxx_compiler::var* cxx_compiler::var::divr(constant<__int64>* y){ return var_impl::div(y,this); }
cxx_compiler::var* cxx_compiler::var::divr(constant<unsigned __int64>* y){ return var_impl::div(y,this); }
cxx_compiler::var* cxx_compiler::var::divr(constant<float>* y){ return var_impl::div(y,this); }
cxx_compiler::var* cxx_compiler::var::divr(constant<double>* y){ return var_impl::div(y,this); }
cxx_compiler::var* cxx_compiler::var::divr(constant<long double>* y){ return var_impl::div(y,this); }

namespace cxx_compiler { namespace constant_impl {
  template<class A, class B> var* div(constant<A>* y, constant<B>* z)
  {
    if ( z->m_value )
      return expressions::primary::literal::integer::create(y->m_value / z->m_value);
    else
      return var_impl::div(y,z);
  }
  template<class A, class B> var* fdiv1(constant<A>* y, constant<B>* z)
  {
    if ( generator::long_double ){
      if ( var* v = fop1(y,z,generator::long_double->div) )
        return v;
    }
#ifndef _MSC_VER
    return expressions::primary::literal::floating::create(y->m_value / z->m_value);
#else // _MSC_VER
    return expressions::primary::literal::floating::create(y->m_value / (__int64)z->m_value);
#endif // _MSC_VER
  }
  template<class A, class B> var* fdiv2(constant<A>* y, constant<B>* z)
  {
    if ( generator::long_double ){
      if ( var* v = fop2(y,z,generator::long_double->div) )
        return v;
    }
#ifndef _MSC_VER
    return expressions::primary::literal::floating::create(y->m_value / z->m_value);
#else // _MSC_VER
    return expressions::primary::literal::floating::create((__int64)y->m_value / z->m_value);
#endif // _MSC_VER
  }
  template<class A, class B> var* fdiv3(constant<A>* y, constant<B>* z)
  {
    if ( generator::long_double ){
      if ( var* v = fop3(y,z,generator::long_double->div) )
        return v;
    }
    return expressions::primary::literal::floating::create(y->m_value / z->m_value);
  }
} } // end of namespace constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::divr(constant<bool>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<bool>::divr(constant<char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<bool>::divr(constant<signed char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<bool>::divr(constant<unsigned char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<bool>::divr(constant<short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<bool>::divr(constant<unsigned short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<bool>::divr(constant<int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<bool>::divr(constant<unsigned int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<bool>::divr(constant<long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<bool>::divr(constant<unsigned long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<bool>::divr(constant<__int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<bool>::divr(constant<unsigned __int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<bool>::divr(constant<float>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<bool>::divr(constant<double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<bool>::divr(constant<long double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<char>::divr(constant<bool>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<char>::divr(constant<char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<char>::divr(constant<signed char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<char>::divr(constant<unsigned char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<char>::divr(constant<short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<char>::divr(constant<unsigned short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<char>::divr(constant<int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<char>::divr(constant<unsigned int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<char>::divr(constant<long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<char>::divr(constant<unsigned long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<char>::divr(constant<__int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<char>::divr(constant<unsigned __int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<char>::divr(constant<float>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<char>::divr(constant<double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<char>::divr(constant<long double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<signed char>::divr(constant<bool>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<signed char>::divr(constant<char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<signed char>::divr(constant<signed char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<signed char>::divr(constant<unsigned char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<signed char>::divr(constant<short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<signed char>::divr(constant<unsigned short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<signed char>::divr(constant<int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<signed char>::divr(constant<unsigned int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<signed char>::divr(constant<long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<signed char>::divr(constant<unsigned long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<signed char>::divr(constant<__int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<signed char>::divr(constant<unsigned __int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<signed char>::divr(constant<float>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<signed char>::divr(constant<double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<signed char>::divr(constant<long double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<unsigned char>::divr(constant<bool>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned char>::divr(constant<char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned char>::divr(constant<signed char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned char>::divr(constant<unsigned char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned char>::divr(constant<short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned char>::divr(constant<unsigned short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned char>::divr(constant<int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned char>::divr(constant<unsigned int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned char>::divr(constant<long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned char>::divr(constant<unsigned long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned char>::divr(constant<__int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned char>::divr(constant<unsigned __int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned char>::divr(constant<float>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<unsigned char>::divr(constant<double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<unsigned char>::divr(constant<long double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<short int>::divr(constant<bool>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<short int>::divr(constant<char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<short int>::divr(constant<signed char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<short int>::divr(constant<unsigned char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<short int>::divr(constant<short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<short int>::divr(constant<unsigned short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<short int>::divr(constant<int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<short int>::divr(constant<unsigned int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<short int>::divr(constant<long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<short int>::divr(constant<unsigned long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<short int>::divr(constant<__int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<short int>::divr(constant<unsigned __int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<short int>::divr(constant<float>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<short int>::divr(constant<double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<short int>::divr(constant<long double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<unsigned short int>::divr(constant<bool>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned short int>::divr(constant<char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned short int>::divr(constant<signed char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned short int>::divr(constant<unsigned char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned short int>::divr(constant<short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned short int>::divr(constant<unsigned short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned short int>::divr(constant<int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned short int>::divr(constant<unsigned int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned short int>::divr(constant<long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned short int>::divr(constant<unsigned long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned short int>::divr(constant<__int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned short int>::divr(constant<unsigned __int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned short int>::divr(constant<float>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<unsigned short int>::divr(constant<double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<unsigned short int>::divr(constant<long double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<int>::divr(constant<bool>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<int>::divr(constant<char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<int>::divr(constant<signed char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<int>::divr(constant<unsigned char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<int>::divr(constant<short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<int>::divr(constant<unsigned short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<int>::divr(constant<int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<int>::divr(constant<unsigned int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<int>::divr(constant<long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<int>::divr(constant<unsigned long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<int>::divr(constant<__int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<int>::divr(constant<unsigned __int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<int>::divr(constant<float>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<int>::divr(constant<double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<int>::divr(constant<long double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<unsigned int>::divr(constant<bool>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned int>::divr(constant<char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned int>::divr(constant<signed char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned int>::divr(constant<unsigned char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned int>::divr(constant<short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned int>::divr(constant<unsigned short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned int>::divr(constant<int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned int>::divr(constant<unsigned int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned int>::divr(constant<long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned int>::divr(constant<unsigned long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned int>::divr(constant<__int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned int>::divr(constant<unsigned __int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned int>::divr(constant<float>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<unsigned int>::divr(constant<double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<unsigned int>::divr(constant<long double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<long int>::divr(constant<bool>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<long int>::divr(constant<char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<long int>::divr(constant<signed char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<long int>::divr(constant<unsigned char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<long int>::divr(constant<short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<long int>::divr(constant<unsigned short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<long int>::divr(constant<int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<long int>::divr(constant<unsigned int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<long int>::divr(constant<long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<long int>::divr(constant<unsigned long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<long int>::divr(constant<__int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<long int>::divr(constant<unsigned __int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<long int>::divr(constant<float>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<long int>::divr(constant<double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<long int>::divr(constant<long double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<unsigned long int>::divr(constant<bool>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned long int>::divr(constant<char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned long int>::divr(constant<signed char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned long int>::divr(constant<unsigned char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned long int>::divr(constant<short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned long int>::divr(constant<unsigned short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned long int>::divr(constant<int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned long int>::divr(constant<unsigned int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned long int>::divr(constant<long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned long int>::divr(constant<unsigned long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned long int>::divr(constant<__int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned long int>::divr(constant<unsigned __int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned long int>::divr(constant<float>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<unsigned long int>::divr(constant<double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<unsigned long int>::divr(constant<long double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<__int64>::divr(constant<bool>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<__int64>::divr(constant<char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<__int64>::divr(constant<signed char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<__int64>::divr(constant<unsigned char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<__int64>::divr(constant<short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<__int64>::divr(constant<unsigned short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<__int64>::divr(constant<int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<__int64>::divr(constant<unsigned int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<__int64>::divr(constant<long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<__int64>::divr(constant<unsigned long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<__int64>::divr(constant<__int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<__int64>::divr(constant<unsigned __int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<__int64>::divr(constant<float>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<__int64>::divr(constant<double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<__int64>::divr(constant<long double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<unsigned __int64>::divr(constant<bool>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned __int64>::divr(constant<char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned __int64>::divr(constant<signed char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned __int64>::divr(constant<unsigned char>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned __int64>::divr(constant<short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned __int64>::divr(constant<unsigned short int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned __int64>::divr(constant<int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned __int64>::divr(constant<unsigned int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned __int64>::divr(constant<long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned __int64>::divr(constant<unsigned long int>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned __int64>::divr(constant<__int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned __int64>::divr(constant<unsigned __int64>* y)
  { return constant_impl::div(y,this); }
  template<>
  var* constant<unsigned __int64>::divr(constant<float>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<unsigned __int64>::divr(constant<double>* y)
  { return constant_impl::fdiv1(y,this); }
  template<>
  var* constant<unsigned __int64>::divr(constant<long double>* y)
  { return constant_impl::fdiv1(y,this); }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<float>::div(var* z){ return z->divr(this); }
cxx_compiler::var* cxx_compiler::constant<float>::divr(constant<bool>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::divr(constant<char>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::divr(constant<signed char>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::divr(constant<unsigned char>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::divr(constant<short int>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::divr(constant<unsigned short int>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::divr(constant<int>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::divr(constant<unsigned int>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::divr(constant<long int>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::divr(constant<unsigned long int>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::divr(constant<__int64>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::divr(constant<unsigned __int64>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::divr(constant<float>* y)
{ return constant_impl::fdiv3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::divr(constant<double>* y)
{ return constant_impl::fdiv3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::divr(constant<long double>* y)
{ return constant_impl::fdiv3(y,this); }

cxx_compiler::var* cxx_compiler::constant<double>::div(var* z){ return z->divr(this); }
cxx_compiler::var* cxx_compiler::constant<double>::divr(constant<bool>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::divr(constant<char>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::divr(constant<signed char>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::divr(constant<unsigned char>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::divr(constant<short int>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::divr(constant<unsigned short int>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::divr(constant<int>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::divr(constant<unsigned int>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::divr(constant<long int>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::divr(constant<unsigned long int>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::divr(constant<__int64>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::divr(constant<unsigned __int64>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::divr(constant<float>* y)
{ return constant_impl::fdiv3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::divr(constant<double>* y)
{ return constant_impl::fdiv3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::divr(constant<long double>* y)
{ return constant_impl::fdiv3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::div(var* z){ return z->divr(this); }
cxx_compiler::var* cxx_compiler::constant<long double>::divr(constant<bool>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::divr(constant<char>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::divr(constant<signed char>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::divr(constant<unsigned char>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::divr(constant<short int>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::divr(constant<unsigned short int>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::divr(constant<int>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::divr(constant<unsigned int>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::divr(constant<long int>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::divr(constant<unsigned long int>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::divr(constant<__int64>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::divr(constant<unsigned __int64>* y)
{ return constant_impl::fdiv2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::divr(constant<float>* y)
{ return constant_impl::fdiv3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::divr(constant<double>* y)
{ return constant_impl::fdiv3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::divr(constant<long double>* y)
{ return constant_impl::fdiv3(y,this); }

namespace cxx_compiler { namespace var_impl {
  var* mod(var*, var*);
  var* opt_mod(var*, var*);
} } // end of namespace var_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::var_impl::mod(var* a, var* b)
{
  var* y = a->rvalue();
  var* z = b->rvalue();
  const type* T = conversion::arithmetic::gen(&y,&z);
  if ( !T || !T->integer() ){
    using namespace error::expressions::binary;
    invalid(parse::position,'%',y->m_type,z->m_type);
    T = int_type::create();
  }
  if ( var* x = opt_mod(y,z) )
    return x;
  var* x = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new mod3ac(x,y,z));
  return x;
}

cxx_compiler::var* cxx_compiler::var_impl::opt_mod(var* y, var* z)
{
  if ( !z->isconstant() )
    return 0;
  const type* T = z->m_type;
  if ( z->zero() ){
    if ( T->integer() ){
      using namespace warning;
      zero_division(parse::position);
    }
    return 0;
  }
  if ( !T->integer() || T->size() > sizeof(int) )
    return 0;
  int n = z->value();
  n = log2(n);
  if ( n == -1 )
    return 0;
  var* x = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  if ( n ){
    var* mask = expressions::primary::literal::integer::create((1 << n) - 1);
    code.push_back(new and3ac(x,y,mask));
  }
  else {
    var* zero = expressions::primary::literal::integer::create(0);
    code.push_back(new assign3ac(x,zero));
  }
  return x;
}

cxx_compiler::var* cxx_compiler::var::mod(var* z){ return var_impl::mod(this,z); }
cxx_compiler::var* cxx_compiler::var::modr(constant<bool>* y){ return var_impl::mod(y,this); }
cxx_compiler::var* cxx_compiler::var::modr(constant<char>* y){ return var_impl::mod(y,this); }
cxx_compiler::var* cxx_compiler::var::modr(constant<signed char>* y){ return var_impl::mod(y,this); }
cxx_compiler::var* cxx_compiler::var::modr(constant<unsigned char>* y){ return var_impl::mod(y,this); }
cxx_compiler::var* cxx_compiler::var::modr(constant<short int>* y){ return var_impl::mod(y,this); }
cxx_compiler::var* cxx_compiler::var::modr(constant<unsigned short int>* y){ return var_impl::mod(y,this); }
cxx_compiler::var* cxx_compiler::var::modr(constant<int>* y){ return var_impl::mod(y,this); }
cxx_compiler::var* cxx_compiler::var::modr(constant<unsigned int>* y){ return var_impl::mod(y,this); }
cxx_compiler::var* cxx_compiler::var::modr(constant<long int>* y){ return var_impl::mod(y,this); }
cxx_compiler::var* cxx_compiler::var::modr(constant<unsigned long int>* y){ return var_impl::mod(y,this); }
cxx_compiler::var* cxx_compiler::var::modr(constant<__int64>* y){ return var_impl::mod(y,this); }
cxx_compiler::var* cxx_compiler::var::modr(constant<unsigned __int64>* y){ return var_impl::mod(y,this); }

namespace cxx_compiler { namespace constant_impl {
  template<class A, class B> var* mod(constant<A>* y, constant<B>* z)
  {
    if ( z->m_value )
      return expressions::primary::literal::integer::create(y->m_value % z->m_value); 
    else
      return var_impl::mod(y,z);
  }
} } // end of namespace constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::modr(constant<bool>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<bool>::modr(constant<char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<bool>::modr(constant<signed char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<bool>::modr(constant<unsigned char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<bool>::modr(constant<short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<bool>::modr(constant<unsigned short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<bool>::modr(constant<int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<bool>::modr(constant<unsigned int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<bool>::modr(constant<long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<bool>::modr(constant<unsigned long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<bool>::modr(constant<__int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<bool>::modr(constant<unsigned __int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<char>::modr(constant<bool>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<char>::modr(constant<char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<char>::modr(constant<signed char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<char>::modr(constant<unsigned char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<char>::modr(constant<short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<char>::modr(constant<unsigned short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<char>::modr(constant<int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<char>::modr(constant<unsigned int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<char>::modr(constant<long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<char>::modr(constant<unsigned long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<char>::modr(constant<__int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<char>::modr(constant<unsigned __int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<signed char>::modr(constant<bool>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<signed char>::modr(constant<char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<signed char>::modr(constant<signed char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<signed char>::modr(constant<unsigned char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<signed char>::modr(constant<short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<signed char>::modr(constant<unsigned short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<signed char>::modr(constant<int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<signed char>::modr(constant<unsigned int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<signed char>::modr(constant<long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<signed char>::modr(constant<unsigned long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<signed char>::modr(constant<__int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<signed char>::modr(constant<unsigned __int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned char>::modr(constant<bool>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned char>::modr(constant<char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned char>::modr(constant<signed char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned char>::modr(constant<unsigned char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned char>::modr(constant<short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned char>::modr(constant<unsigned short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned char>::modr(constant<int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned char>::modr(constant<unsigned int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned char>::modr(constant<long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned char>::modr(constant<unsigned long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned char>::modr(constant<__int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned char>::modr(constant<unsigned __int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<short int>::modr(constant<bool>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<short int>::modr(constant<char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<short int>::modr(constant<signed char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<short int>::modr(constant<unsigned char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<short int>::modr(constant<short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<short int>::modr(constant<unsigned short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<short int>::modr(constant<int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<short int>::modr(constant<unsigned int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<short int>::modr(constant<long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<short int>::modr(constant<unsigned long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<short int>::modr(constant<__int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<short int>::modr(constant<unsigned __int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned short int>::modr(constant<bool>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned short int>::modr(constant<char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned short int>::modr(constant<signed char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned short int>::modr(constant<unsigned char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned short int>::modr(constant<short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned short int>::modr(constant<unsigned short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned short int>::modr(constant<int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned short int>::modr(constant<unsigned int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned short int>::modr(constant<long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned short int>::modr(constant<unsigned long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned short int>::modr(constant<__int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned short int>::modr(constant<unsigned __int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<int>::modr(constant<bool>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<int>::modr(constant<char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<int>::modr(constant<signed char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<int>::modr(constant<unsigned char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<int>::modr(constant<short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<int>::modr(constant<unsigned short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<int>::modr(constant<int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<int>::modr(constant<unsigned int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<int>::modr(constant<long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<int>::modr(constant<unsigned long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<int>::modr(constant<__int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<int>::modr(constant<unsigned __int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned int>::modr(constant<bool>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned int>::modr(constant<char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned int>::modr(constant<signed char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned int>::modr(constant<unsigned char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned int>::modr(constant<short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned int>::modr(constant<unsigned short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned int>::modr(constant<int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned int>::modr(constant<unsigned int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned int>::modr(constant<long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned int>::modr(constant<unsigned long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned int>::modr(constant<__int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned int>::modr(constant<unsigned __int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<long int>::modr(constant<bool>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<long int>::modr(constant<char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<long int>::modr(constant<signed char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<long int>::modr(constant<unsigned char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<long int>::modr(constant<short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<long int>::modr(constant<unsigned short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<long int>::modr(constant<int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<long int>::modr(constant<unsigned int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<long int>::modr(constant<long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<long int>::modr(constant<unsigned long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<long int>::modr(constant<__int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<long int>::modr(constant<unsigned __int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned long int>::modr(constant<bool>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned long int>::modr(constant<char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned long int>::modr(constant<signed char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned long int>::modr(constant<unsigned char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned long int>::modr(constant<short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned long int>::modr(constant<unsigned short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned long int>::modr(constant<int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned long int>::modr(constant<unsigned int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned long int>::modr(constant<long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned long int>::modr(constant<unsigned long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned long int>::modr(constant<__int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned long int>::modr(constant<unsigned __int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<__int64>::modr(constant<bool>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<__int64>::modr(constant<char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<__int64>::modr(constant<signed char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<__int64>::modr(constant<unsigned char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<__int64>::modr(constant<short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<__int64>::modr(constant<unsigned short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<__int64>::modr(constant<int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<__int64>::modr(constant<unsigned int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<__int64>::modr(constant<long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<__int64>::modr(constant<unsigned long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<__int64>::modr(constant<__int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<__int64>::modr(constant<unsigned __int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned __int64>::modr(constant<bool>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned __int64>::modr(constant<char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned __int64>::modr(constant<signed char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned __int64>::modr(constant<unsigned char>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned __int64>::modr(constant<short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned __int64>::modr(constant<unsigned short int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned __int64>::modr(constant<int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned __int64>::modr(constant<unsigned int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned __int64>::modr(constant<long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned __int64>::modr(constant<unsigned long int>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned __int64>::modr(constant<__int64>* y)
  { return constant_impl::mod(y,this); }
  template<>
  var* constant<unsigned __int64>::modr(constant<unsigned __int64>* y)
  { return constant_impl::mod(y,this); }
} // end of namespace cxx_compiler

namespace cxx_compiler { namespace var_impl {
  var* add(var*, var*);
  var* pointer_integer(int, var* ,var*);
  var* opt_add(var*, var*);
} } // end of namespace var_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::var_impl::add(var* a, var* b)
{
  var* y = a->rvalue();
  var* z = b->rvalue();
  if ( var* r = pointer_integer('+',y,z) )
    return r;
  if ( var* r = pointer_integer('+',z,y) )
    return r;
  const type* T = conversion::arithmetic::gen(&y,&z);
  if ( !T ){
    using namespace error::expressions::binary;
    invalid(parse::position,'+',y->m_type,z->m_type);
    T = int_type::create();
  }
  if ( var* x = opt_add(y,z) )
    return x;
  if ( var* x = opt_add(z,y) )
    return x;
  var* x = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new add3ac(x,y,z));
  return x;
}

cxx_compiler::var* cxx_compiler::var_impl::pointer_integer(int op, var* y, var* z)
{
  using namespace std;
  const type* Ty = y->m_type;
  Ty = Ty->unqualified();
  if ( Ty->m_id != type::POINTER )
    return 0;
  typedef const pointer_type PT;
  PT* pt = static_cast<PT*>(Ty);
  const type* Tz = z->m_type;
  if ( !Tz->integer() )
    return 0;
  const type* T = pt->referenced_type();
  T = T->complete_type();
  var* size = T->vsize();
  if ( !size ){
    int n = T->size();
    if ( !n ){
      using namespace error::expressions::binary;
      invalid(parse::position,op,pt,Tz);
      n = 1;
    }
    size = expressions::primary::literal::integer::create(n);
  }
  z = z->mul(size);
  if ( var* x = opt_add(y,z) )
    return x;
  var* x = new var(Ty);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  if ( op == '+' )
    code.push_back(new add3ac(x,y,z));
  else
    code.push_back(new sub3ac(x,y,z));
  return x;
}

cxx_compiler::var* cxx_compiler::var_impl::opt_add(var* y, var* z)
{
  if ( !z->isconstant() )
    return 0;
  if ( !z->zero() )
    return 0;
  const type* T = y->m_type;
  var* x = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new assign3ac(x,y));
  return x;
}

cxx_compiler::var* cxx_compiler::var::add(var* z){ return var_impl::add(this,z); }
cxx_compiler::var* cxx_compiler::var::addr(constant<bool>* y){ return var_impl::add(y,this); }
cxx_compiler::var* cxx_compiler::var::addr(constant<char>* y){ return var_impl::add(y,this); }
cxx_compiler::var* cxx_compiler::var::addr(constant<signed char>* y){ return var_impl::add(y,this); }
cxx_compiler::var* cxx_compiler::var::addr(constant<unsigned char>* y){ return var_impl::add(y,this); }
cxx_compiler::var* cxx_compiler::var::addr(constant<short int>* y){ return var_impl::add(y,this); }
cxx_compiler::var* cxx_compiler::var::addr(constant<unsigned short int>* y){ return var_impl::add(y,this); }
cxx_compiler::var* cxx_compiler::var::addr(constant<int>* y){ return var_impl::add(y,this); }
cxx_compiler::var* cxx_compiler::var::addr(constant<unsigned int>* y){ return var_impl::add(y,this); }
cxx_compiler::var* cxx_compiler::var::addr(constant<long int>* y){ return var_impl::add(y,this); }
cxx_compiler::var* cxx_compiler::var::addr(constant<unsigned long int>* y){ return var_impl::add(y,this); }
cxx_compiler::var* cxx_compiler::var::addr(constant<__int64>* y){ return var_impl::add(y,this); }
cxx_compiler::var* cxx_compiler::var::addr(constant<unsigned __int64>* y){ return var_impl::add(y,this); }
cxx_compiler::var* cxx_compiler::var::addr(constant<float>* y){ return var_impl::add(y,this); }
cxx_compiler::var* cxx_compiler::var::addr(constant<double>* y){ return var_impl::add(y,this); }
cxx_compiler::var* cxx_compiler::var::addr(constant<long double>* y){ return var_impl::add(y,this); }
cxx_compiler::var* cxx_compiler::var::addr(constant<void*>* y){ return var_impl::add(y,this); }
cxx_compiler::var* cxx_compiler::var::addr(addrof* y){ return var_impl::add(y,this); }

namespace cxx_compiler { namespace constant_impl {
  template<class A, class B> var* add(constant<A>* y, constant<B>* z)
  {
    return expressions::primary::literal::integer::create(y->m_value + z->m_value); 
  }
  template<class A, class B> var* fadd1(constant<A>* y, constant<B>* z)
  {
    if ( generator::long_double ){
      if ( var* v = fop1(y,z,generator::long_double->add) )
        return v;
    }
#ifndef _MSC_VER
    return expressions::primary::literal::floating::create(y->m_value + z->m_value); 
#else // _MSC_VER
    return expressions::primary::literal::floating::create(y->m_value + (__int64)z->m_value); 
#endif // _MSC_VER
  }
  template<class A, class B> var* fadd2(constant<A>* y, constant<B>* z)
  {
    if ( generator::long_double ){
      if ( var* v = fop2(y,z,generator::long_double->add) )
        return v;
    }
#ifndef _MSC_VER
    return expressions::primary::literal::floating::create(y->m_value + z->m_value);
#else // _MSC_VER
    return expressions::primary::literal::floating::create((__int64)y->m_value + z->m_value); 
#endif // _MSC_VER
  }
  template<class A, class B> var* fadd3(constant<A>* y, constant<B>* z)
  {
    if ( generator::long_double ){
      if ( var* v = fop3(y,z,generator::long_double->add) )
        return v;
    }
    return expressions::primary::literal::floating::create(y->m_value + z->m_value); 
  }
  template<class C> var* padd(constant<void*>* y, constant<C>* z)
  {
    char* p = reinterpret_cast<char*>(y->m_value);
    int n = z->m_value;
    const type* T = y->m_type;
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(T);
    T = pt->referenced_type();
    int size = T->size();
    if ( !size )
      return var_impl::add(y,z);
    p += size * n;
    return expressions::primary::literal::pointer::create(pt,(void*)p);
  }
} } // end of namespace constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::addr(constant<bool>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<bool>::addr(constant<char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<bool>::addr(constant<signed char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<bool>::addr(constant<unsigned char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<bool>::addr(constant<short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<bool>::addr(constant<unsigned short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<bool>::addr(constant<int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<bool>::addr(constant<unsigned int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<bool>::addr(constant<long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<bool>::addr(constant<unsigned long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<bool>::addr(constant<__int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<bool>::addr(constant<unsigned __int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<bool>::addr(constant<float>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<bool>::addr(constant<double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<bool>::addr(constant<long double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<bool>::addr(constant<void*>* y)
  { return constant_impl::padd(y,this); }
  template<>
  var* constant<char>::addr(constant<bool>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<char>::addr(constant<char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<char>::addr(constant<signed char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<char>::addr(constant<unsigned char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<char>::addr(constant<short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<char>::addr(constant<unsigned short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<char>::addr(constant<int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<char>::addr(constant<unsigned int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<char>::addr(constant<long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<char>::addr(constant<unsigned long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<char>::addr(constant<__int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<char>::addr(constant<unsigned __int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<char>::addr(constant<float>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<char>::addr(constant<double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<char>::addr(constant<long double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<char>::addr(constant<void*>* y)
  { return constant_impl::padd(y,this); }
  template<>
  var* constant<signed char>::addr(constant<bool>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<signed char>::addr(constant<char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<signed char>::addr(constant<signed char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<signed char>::addr(constant<unsigned char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<signed char>::addr(constant<short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<signed char>::addr(constant<unsigned short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<signed char>::addr(constant<int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<signed char>::addr(constant<unsigned int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<signed char>::addr(constant<long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<signed char>::addr(constant<unsigned long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<signed char>::addr(constant<__int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<signed char>::addr(constant<unsigned __int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<signed char>::addr(constant<float>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<signed char>::addr(constant<double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<signed char>::addr(constant<long double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<signed char>::addr(constant<void*>* y)
  { return constant_impl::padd(y,this); }
  template<>
  var* constant<unsigned char>::addr(constant<bool>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned char>::addr(constant<char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned char>::addr(constant<signed char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned char>::addr(constant<unsigned char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned char>::addr(constant<short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned char>::addr(constant<unsigned short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned char>::addr(constant<int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned char>::addr(constant<unsigned int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned char>::addr(constant<long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned char>::addr(constant<unsigned long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned char>::addr(constant<__int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned char>::addr(constant<unsigned __int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned char>::addr(constant<float>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<unsigned char>::addr(constant<double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<unsigned char>::addr(constant<long double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<unsigned char>::addr(constant<void*>* y)
  { return constant_impl::padd(y,this); }
  template<>
  var* constant<short int>::addr(constant<bool>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<short int>::addr(constant<char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<short int>::addr(constant<signed char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<short int>::addr(constant<unsigned char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<short int>::addr(constant<short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<short int>::addr(constant<unsigned short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<short int>::addr(constant<int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<short int>::addr(constant<unsigned int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<short int>::addr(constant<long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<short int>::addr(constant<unsigned long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<short int>::addr(constant<__int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<short int>::addr(constant<unsigned __int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<short int>::addr(constant<float>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<short int>::addr(constant<double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<short int>::addr(constant<long double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<short int>::addr(constant<void*>* y)
  { return constant_impl::padd(y,this); }
  template<>
  var* constant<unsigned short int>::addr(constant<bool>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned short int>::addr(constant<char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned short int>::addr(constant<signed char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned short int>::addr(constant<unsigned char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned short int>::addr(constant<short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned short int>::addr(constant<unsigned short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned short int>::addr(constant<int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned short int>::addr(constant<unsigned int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned short int>::addr(constant<long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned short int>::addr(constant<unsigned long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned short int>::addr(constant<__int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned short int>::addr(constant<unsigned __int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned short int>::addr(constant<float>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<unsigned short int>::addr(constant<double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<unsigned short int>::addr(constant<long double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<unsigned short int>::addr(constant<void*>* y)
  { return constant_impl::padd(y,this); }
  template<>
  var* constant<int>::addr(constant<bool>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<int>::addr(constant<char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<int>::addr(constant<signed char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<int>::addr(constant<unsigned char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<int>::addr(constant<short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<int>::addr(constant<unsigned short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<int>::addr(constant<int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<int>::addr(constant<unsigned int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<int>::addr(constant<long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<int>::addr(constant<unsigned long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<int>::addr(constant<__int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<int>::addr(constant<unsigned __int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<int>::addr(constant<float>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<int>::addr(constant<double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<int>::addr(constant<long double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<int>::addr(constant<void*>* y)
  { return constant_impl::padd(y,this); }
  template<>
  var* constant<unsigned int>::addr(constant<bool>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned int>::addr(constant<char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned int>::addr(constant<signed char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned int>::addr(constant<unsigned char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned int>::addr(constant<short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned int>::addr(constant<unsigned short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned int>::addr(constant<int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned int>::addr(constant<unsigned int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned int>::addr(constant<long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned int>::addr(constant<unsigned long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned int>::addr(constant<__int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned int>::addr(constant<unsigned __int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned int>::addr(constant<float>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<unsigned int>::addr(constant<double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<unsigned int>::addr(constant<long double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<unsigned int>::addr(constant<void*>* y)
  { return constant_impl::padd(y,this); }
  template<>
  var* constant<long int>::addr(constant<bool>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<long int>::addr(constant<char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<long int>::addr(constant<signed char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<long int>::addr(constant<unsigned char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<long int>::addr(constant<short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<long int>::addr(constant<unsigned short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<long int>::addr(constant<int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<long int>::addr(constant<unsigned int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<long int>::addr(constant<long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<long int>::addr(constant<unsigned long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<long int>::addr(constant<__int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<long int>::addr(constant<unsigned __int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<long int>::addr(constant<float>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<long int>::addr(constant<double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<long int>::addr(constant<long double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<long int>::addr(constant<void*>* y)
  { return constant_impl::padd(y,this); }
  template<>
  var* constant<unsigned long int>::addr(constant<bool>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned long int>::addr(constant<char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned long int>::addr(constant<signed char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned long int>::addr(constant<unsigned char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned long int>::addr(constant<short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned long int>::addr(constant<unsigned short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned long int>::addr(constant<int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned long int>::addr(constant<unsigned int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned long int>::addr(constant<long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned long int>::addr(constant<unsigned long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned long int>::addr(constant<__int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned long int>::addr(constant<unsigned __int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned long int>::addr(constant<float>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<unsigned long int>::addr(constant<double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<unsigned long int>::addr(constant<long double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<unsigned long int>::addr(constant<void*>* y)
  { return constant_impl::padd(y,this); }
  template<>
  var* constant<__int64>::addr(constant<bool>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<__int64>::addr(constant<char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<__int64>::addr(constant<signed char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<__int64>::addr(constant<unsigned char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<__int64>::addr(constant<short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<__int64>::addr(constant<unsigned short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<__int64>::addr(constant<int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<__int64>::addr(constant<unsigned int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<__int64>::addr(constant<long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<__int64>::addr(constant<unsigned long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<__int64>::addr(constant<__int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<__int64>::addr(constant<unsigned __int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<__int64>::addr(constant<float>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<__int64>::addr(constant<double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<__int64>::addr(constant<long double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<__int64>::addr(constant<void*>* y)
  { return constant_impl::padd(y,this); }
  template<>
  var* constant<unsigned __int64>::addr(constant<bool>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned __int64>::addr(constant<char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned __int64>::addr(constant<signed char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned __int64>::addr(constant<unsigned char>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned __int64>::addr(constant<short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned __int64>::addr(constant<unsigned short int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned __int64>::addr(constant<int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned __int64>::addr(constant<unsigned int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned __int64>::addr(constant<long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned __int64>::addr(constant<unsigned long int>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned __int64>::addr(constant<__int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned __int64>::addr(constant<unsigned __int64>* y)
  { return constant_impl::add(y,this); }
  template<>
  var* constant<unsigned __int64>::addr(constant<float>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<unsigned __int64>::addr(constant<double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<unsigned __int64>::addr(constant<long double>* y)
  { return constant_impl::fadd1(y,this); }
  template<>
  var* constant<unsigned __int64>::addr(constant<void*>* y)
  { return constant_impl::padd(y,this); }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<float>::add(var* z){ return z->addr(this); }
cxx_compiler::var* cxx_compiler::constant<float>::addr(constant<bool>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::addr(constant<char>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::addr(constant<signed char>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::addr(constant<unsigned char>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::addr(constant<short int>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::addr(constant<unsigned short int>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::addr(constant<int>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::addr(constant<unsigned int>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::addr(constant<long int>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::addr(constant<unsigned long int>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::addr(constant<__int64>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::addr(constant<unsigned __int64>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::addr(constant<float>* y)
{ return constant_impl::fadd3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::addr(constant<double>* y)
{ return constant_impl::fadd3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::addr(constant<long double>* y)
{ return constant_impl::fadd3(y,this); }

cxx_compiler::var* cxx_compiler::constant<double>::add(var* z){ return z->addr(this); }
cxx_compiler::var* cxx_compiler::constant<double>::addr(constant<bool>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::addr(constant<char>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::addr(constant<signed char>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::addr(constant<unsigned char>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::addr(constant<short int>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::addr(constant<unsigned short int>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::addr(constant<int>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::addr(constant<unsigned int>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::addr(constant<long int>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::addr(constant<unsigned long int>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::addr(constant<__int64>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::addr(constant<unsigned __int64>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::addr(constant<float>* y)
{ return constant_impl::fadd3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::addr(constant<double>* y)
{ return constant_impl::fadd3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::addr(constant<long double>* y)
{ return constant_impl::fadd3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::add(var* z){ return z->addr(this); }
cxx_compiler::var* cxx_compiler::constant<long double>::addr(constant<bool>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::addr(constant<char>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::addr(constant<signed char>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::addr(constant<unsigned char>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::addr(constant<short int>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::addr(constant<unsigned short int>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::addr(constant<int>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::addr(constant<unsigned int>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::addr(constant<long int>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::addr(constant<unsigned long int>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::addr(constant<__int64>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::addr(constant<unsigned __int64>* y)
{ return constant_impl::fadd2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::addr(constant<float>* y)
{ return constant_impl::fadd3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::addr(constant<double>* y)
{ return constant_impl::fadd3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::addr(constant<long double>* y)
{ return constant_impl::fadd3(y,this); }

cxx_compiler::var* cxx_compiler::constant<void*>::addr(constant<bool>* y)
{ return constant_impl::padd(this,y); }
cxx_compiler::var* cxx_compiler::constant<void*>::addr(constant<char>* y)
{ return constant_impl::padd(this,y); }
cxx_compiler::var* cxx_compiler::constant<void*>::addr(constant<signed char>* y)
{ return constant_impl::padd(this,y); }
cxx_compiler::var* cxx_compiler::constant<void*>::addr(constant<unsigned char>* y)
{ return constant_impl::padd(this,y); }
cxx_compiler::var* cxx_compiler::constant<void*>::addr(constant<short int>* y)
{ return constant_impl::padd(this,y); }
cxx_compiler::var* cxx_compiler::constant<void*>::addr(constant<unsigned short int>* y)
{ return constant_impl::padd(this,y); }
cxx_compiler::var* cxx_compiler::constant<void*>::addr(constant<int>* y)
{ return constant_impl::padd(this,y); }
cxx_compiler::var* cxx_compiler::constant<void*>::addr(constant<unsigned int>* y)
{ return constant_impl::padd(this,y); }
cxx_compiler::var* cxx_compiler::constant<void*>::addr(constant<long int>* y)
{ return constant_impl::padd(this,y); }
cxx_compiler::var* cxx_compiler::constant<void*>::addr(constant<unsigned long int>* y)
{ return constant_impl::padd(this,y); }
cxx_compiler::var* cxx_compiler::constant<void*>::addr(constant<__int64>* y)
{ return constant_impl::padd(this,y); }
cxx_compiler::var* cxx_compiler::constant<void*>::addr(constant<unsigned __int64>* y)
{ return constant_impl::padd(this,y); }

namespace cxx_compiler { namespace addrof_impl {
  template<class T> var* add(addrof* y, constant<T>* z)
  {
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(y->m_type);
    const type* type = pt->referenced_type();
    int n = type->size();
    if ( !n )
      return var_impl::add(y,z);
    int offset = y->m_offset + n * z->m_value;
    var* ret = new addrof(pt,y->m_ref,offset);
    garbage.push_back(ret);
    return ret;
  }
} } // end of namespace addrof_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::addrof::add(var* z){ return z->addr(this); }

namespace cxx_compiler {
  template<>
  var* constant<bool>::addr(addrof* y){ return addrof_impl::add(y,this); }
  template<>
  var* constant<char>::addr(addrof* y){ return addrof_impl::add(y,this); }
  template<>
  var* constant<signed char>::addr(addrof* y){ return addrof_impl::add(y,this); }
  template<>
  var* constant<unsigned char>::addr(addrof* y){ return addrof_impl::add(y,this); }
  template<>
  var* constant<short int>::addr(addrof* y){ return addrof_impl::add(y,this); }
  template<>
  var* constant<unsigned short int>::addr(addrof* y){ return addrof_impl::add(y,this); }
  template<>
  var* constant<int>::addr(addrof* y){ return addrof_impl::add(y,this); }
  template<>
  var* constant<unsigned int>::addr(addrof* y){ return addrof_impl::add(y,this); }
  template<>
  var* constant<long int>::addr(addrof* y){ return addrof_impl::add(y,this); }
  template<>
  var* constant<unsigned long int>::addr(addrof* y){ return addrof_impl::add(y,this); }
  template<>
  var* constant<__int64>::addr(addrof* y){ return addrof_impl::add(y,this); }
  template<>
  var* constant<unsigned __int64>::addr(addrof* y){ return addrof_impl::add(y,this); }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::addrof::addr(constant<bool>* y){ return addrof_impl::add(this,y); }
cxx_compiler::var* cxx_compiler::addrof::addr(constant<char>* y){ return addrof_impl::add(this,y); }
cxx_compiler::var* cxx_compiler::addrof::addr(constant<signed char>* y){ return addrof_impl::add(this,y); }
cxx_compiler::var* cxx_compiler::addrof::addr(constant<unsigned char>* y){ return addrof_impl::add(this,y); }
cxx_compiler::var* cxx_compiler::addrof::addr(constant<short int>* y){ return addrof_impl::add(this,y); }
cxx_compiler::var* cxx_compiler::addrof::addr(constant<unsigned short int>* y){ return addrof_impl::add(this,y); }
cxx_compiler::var* cxx_compiler::addrof::addr(constant<int>* y){ return addrof_impl::add(this,y); }
cxx_compiler::var* cxx_compiler::addrof::addr(constant<unsigned int>* y){ return addrof_impl::add(this,y); }
cxx_compiler::var* cxx_compiler::addrof::addr(constant<long int>* y){ return addrof_impl::add(this,y); }
cxx_compiler::var* cxx_compiler::addrof::addr(constant<unsigned long int>* y){ return addrof_impl::add(this,y); }
cxx_compiler::var* cxx_compiler::addrof::addr(constant<__int64>* y){ return addrof_impl::add(this,y); }
cxx_compiler::var* cxx_compiler::addrof::addr(constant<unsigned __int64>* y){ return addrof_impl::add(this,y); }

namespace cxx_compiler { namespace var_impl {
  var* sub(var*, var*);
  var* pointer_pointer(var*, var*);
  var* opt_sub(var*, var*);
} } // end of namespace var_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::var_impl::sub(var* a, var* b)
{
  var* y = a->rvalue();
  var* z = b->rvalue();
  if ( var* r = pointer_pointer(y,z) )
    return r;
  if ( var* r = pointer_integer('-',y,z) )
    return r;
  const type* T = conversion::arithmetic::gen(&y,&z);
  if ( !T ){
    using namespace error::expressions::binary;
    invalid(parse::position,'-',y->m_type,z->m_type);
    T = int_type::create();
  }
  if ( var* x = opt_sub(y,z) )
    return x;
  var* x = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new sub3ac(x,y,z));
  return x;
}

cxx_compiler::var* cxx_compiler::var_impl::pointer_pointer(var* y, var* z)
{
  using namespace std;
  const type* Ty = y->m_type;
  Ty = Ty->unqualified();
  if ( Ty->m_id != type::POINTER )
    return 0;
  typedef const pointer_type PT;
  PT* py = static_cast<PT*>(Ty);
  const type* Tz = z->m_type;
  Tz = Tz->unqualified();
  if ( Tz->m_id != type::POINTER )
    return 0;
  PT* pz = static_cast<PT*>(Tz);
  if ( !py->compatible(pz) ){
    using namespace error::expressions::binary;
    not_compatible(parse::position,py,pz);
  }
  const type* T = py->referenced_type();
  var* s = T->vsize();
  if ( !s ){
    int n = T->size();
    if ( !n ){
      using namespace error::expressions::binary;
      invalid(parse::position,'-',py,pz);
      n = 1;
    }
    s = expressions::primary::literal::integer::create(n);
  }
  var* x = new var(long_type::create());
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new sub3ac(x,y,z));
  code.push_back(new div3ac(x,x,s));
  return x;
}

cxx_compiler::var* cxx_compiler::var_impl::opt_sub(var* y, var* z)
{
  if ( var* x = opt_add(y,z) )
    return x;
  if ( !y->isconstant() )
    return 0;
  if ( !y->zero() )
    return 0;
  const type* T = y->m_type;
  var* x = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new uminus3ac(x,z));
  return x;
}

cxx_compiler::var* cxx_compiler::var::sub(var* z){ return var_impl::sub(this,z); }
cxx_compiler::var* cxx_compiler::var::subr(constant<bool>* y){ return var_impl::sub(y,this); }
cxx_compiler::var* cxx_compiler::var::subr(constant<char>* y){ return var_impl::sub(y,this); }
cxx_compiler::var* cxx_compiler::var::subr(constant<signed char>* y){ return var_impl::sub(y,this); }
cxx_compiler::var* cxx_compiler::var::subr(constant<unsigned char>* y){ return var_impl::sub(y,this); }
cxx_compiler::var* cxx_compiler::var::subr(constant<short int>* y){ return var_impl::sub(y,this); }
cxx_compiler::var* cxx_compiler::var::subr(constant<unsigned short int>* y){ return var_impl::sub(y,this); }
cxx_compiler::var* cxx_compiler::var::subr(constant<int>* y){ return var_impl::sub(y,this); }
cxx_compiler::var* cxx_compiler::var::subr(constant<unsigned int>* y){ return var_impl::sub(y,this); }
cxx_compiler::var* cxx_compiler::var::subr(constant<long int>* y){ return var_impl::sub(y,this); }
cxx_compiler::var* cxx_compiler::var::subr(constant<unsigned long int>* y){ return var_impl::sub(y,this); }
cxx_compiler::var* cxx_compiler::var::subr(constant<__int64>* y){ return var_impl::sub(y,this); }
cxx_compiler::var* cxx_compiler::var::subr(constant<unsigned __int64>* y){ return var_impl::sub(y,this); }
cxx_compiler::var* cxx_compiler::var::subr(constant<float>* y){ return var_impl::sub(y,this); }
cxx_compiler::var* cxx_compiler::var::subr(constant<double>* y){ return var_impl::sub(y,this); }
cxx_compiler::var* cxx_compiler::var::subr(constant<long double>* y){ return var_impl::sub(y,this); }
cxx_compiler::var* cxx_compiler::var::subr(constant<void*>* y){ return var_impl::sub(y,this); }
cxx_compiler::var* cxx_compiler::var::subr(addrof* y){ return var_impl::sub(y,this); }

cxx_compiler::var* cxx_compiler::addrof::sub(var* z)
{
  return z->subr(this);
}

cxx_compiler::var* cxx_compiler::addrof::subr(addrof* y)
{
  using namespace std;
  if ( y->m_ref == m_ref ){
    const type* T = m_type;
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(T);
    T = pt->referenced_type();
    long int n = T->size();
    if ( !n ){
      using namespace error::expressions::binary;
      invalid(parse::position,'-',T,T);
      n = 1;
    }
    n = (y->m_offset - m_offset)/n;
    return expressions::primary::literal::integer::create(n);
  }
  else
    return var_impl::sub(y,this);
}

namespace cxx_compiler { namespace addrof_impl {
  template<class T> var* sub(addrof* y, constant<T>* z)
  {
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(y->m_type);
    const type* type = pt->referenced_type();
    int n = type->size();
    if ( !n )
      return var_impl::sub(y,z);
    int offset = y->m_offset - n * z->m_value;
    var* ret = new addrof(pt,y->m_ref,offset);
    garbage.push_back(ret);
    return ret;
  }
} } // end of namespace addrof_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::subr(addrof* y){ return addrof_impl::sub(y,this); }
  template<>
  var* constant<char>::subr(addrof* y){ return addrof_impl::sub(y,this); }
  template<>
  var* constant<signed char>::subr(addrof* y){ return addrof_impl::sub(y,this); }
  template<>
  var* constant<unsigned char>::subr(addrof* y){ return addrof_impl::sub(y,this); }
  template<>
  var* constant<short int>::subr(addrof* y){ return addrof_impl::sub(y,this); }
  template<>
  var* constant<unsigned short int>::subr(addrof* y){ return addrof_impl::sub(y,this); }
  template<>
  var* constant<int>::subr(addrof* y){ return addrof_impl::sub(y,this); }
  template<>
  var* constant<unsigned int>::subr(addrof* y){ return addrof_impl::sub(y,this); }
  template<>
  var* constant<long int>::subr(addrof* y){ return addrof_impl::sub(y,this); }
  template<>
  var* constant<unsigned long int>::subr(addrof* y){ return addrof_impl::sub(y,this); }
  template<>
  var* constant<__int64>::subr(addrof* y){ return addrof_impl::sub(y,this); }
  template<>
  var* constant<unsigned __int64>::subr(addrof* y){ return addrof_impl::sub(y,this); }
} // end of namespace cxx_compiler

namespace cxx_compiler { namespace constant_impl {
  template<class A, class B> var* sub(constant<A>* y, constant<B>* z)
  {
    return expressions::primary::literal::integer::create(y->m_value - z->m_value); 
  }
  template<class A, class B> var* fsub1(constant<A>* y, constant<B>* z)
  {
    if ( generator::long_double ){
      if ( var* v = fop1(y,z,generator::long_double->sub) )
        return v;
    }
#ifndef _MSC_VER
    return expressions::primary::literal::floating::create(y->m_value - z->m_value);
#else // _MSC_VER
    return expressions::primary::literal::floating::create(y->m_value - (__int64)z->m_value);
#endif // _MSC_VER
  }
  template<class A, class B> var* fsub2(constant<A>* y, constant<B>* z)
  {
    if ( generator::long_double ){
      if ( var* v = fop2(y,z,generator::long_double->sub) )
        return v;
    }
#ifndef _MSC_VER
    return expressions::primary::literal::floating::create(y->m_value - z->m_value);
#else // _MSC_VER
    return expressions::primary::literal::floating::create((__int64)y->m_value - z->m_value);
#endif // _MSC_VER
  }
  template<class A, class B> var* fsub3(constant<A>* y, constant<B>* z)
  {
    if ( generator::long_double ){
      if ( var* v = fop3(y,z,generator::long_double->sub) )
        return v;
    }
    return expressions::primary::literal::floating::create(y->m_value - z->m_value);
  }
  template<class C> var* psub(constant<void*>* y, constant<C>* z)
  {
    char* p = reinterpret_cast<char*>(y->m_value);
    int n = z->m_value;
    const type* T = y->m_type;
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(T);
    T = pt->referenced_type();
    int size = T->size();
    if ( !size )
      return var_impl::sub(y,z);
    p -= size * n;
    return expressions::primary::literal::pointer::create(pt,(void*)p);
  }
} } // end of namespace constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::subr(constant<bool>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<bool>::subr(constant<char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<bool>::subr(constant<signed char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<bool>::subr(constant<unsigned char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<bool>::subr(constant<short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<bool>::subr(constant<unsigned short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<bool>::subr(constant<int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<bool>::subr(constant<unsigned int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<bool>::subr(constant<long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<bool>::subr(constant<unsigned long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<bool>::subr(constant<__int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<bool>::subr(constant<unsigned __int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<bool>::subr(constant<float>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<bool>::subr(constant<double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<bool>::subr(constant<long double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<bool>::subr(constant<void*>* y)
  { return constant_impl::psub(y,this); }
  template<>
  var* constant<char>::subr(constant<bool>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<char>::subr(constant<char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<char>::subr(constant<signed char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<char>::subr(constant<unsigned char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<char>::subr(constant<short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<char>::subr(constant<unsigned short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<char>::subr(constant<int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<char>::subr(constant<unsigned int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<char>::subr(constant<long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<char>::subr(constant<unsigned long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<char>::subr(constant<__int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<char>::subr(constant<unsigned __int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<char>::subr(constant<float>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<char>::subr(constant<double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<char>::subr(constant<long double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<char>::subr(constant<void*>* y)
  { return constant_impl::psub(y,this); }
  template<>
  var* constant<signed char>::subr(constant<bool>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<signed char>::subr(constant<char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<signed char>::subr(constant<signed char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<signed char>::subr(constant<unsigned char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<signed char>::subr(constant<short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<signed char>::subr(constant<unsigned short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<signed char>::subr(constant<int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<signed char>::subr(constant<unsigned int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<signed char>::subr(constant<long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<signed char>::subr(constant<unsigned long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<signed char>::subr(constant<__int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<signed char>::subr(constant<unsigned __int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<signed char>::subr(constant<float>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<signed char>::subr(constant<double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<signed char>::subr(constant<long double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<signed char>::subr(constant<void*>* y)
  { return constant_impl::psub(y,this); }
  template<>
  var* constant<unsigned char>::subr(constant<bool>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned char>::subr(constant<char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned char>::subr(constant<signed char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned char>::subr(constant<unsigned char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned char>::subr(constant<short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned char>::subr(constant<unsigned short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned char>::subr(constant<int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned char>::subr(constant<unsigned int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned char>::subr(constant<long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned char>::subr(constant<unsigned long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned char>::subr(constant<__int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned char>::subr(constant<unsigned __int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned char>::subr(constant<float>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<unsigned char>::subr(constant<double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<unsigned char>::subr(constant<long double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<unsigned char>::subr(constant<void*>* y)
  { return constant_impl::psub(y,this); }
  template<>
  var* constant<short int>::subr(constant<bool>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<short int>::subr(constant<char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<short int>::subr(constant<signed char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<short int>::subr(constant<unsigned char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<short int>::subr(constant<short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<short int>::subr(constant<unsigned short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<short int>::subr(constant<int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<short int>::subr(constant<unsigned int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<short int>::subr(constant<long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<short int>::subr(constant<unsigned long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<short int>::subr(constant<__int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<short int>::subr(constant<unsigned __int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<short int>::subr(constant<float>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<short int>::subr(constant<double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<short int>::subr(constant<long double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<short int>::subr(constant<void*>* y)
  { return constant_impl::psub(y,this); }
  template<>
  var* constant<unsigned short int>::subr(constant<bool>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned short int>::subr(constant<char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned short int>::subr(constant<signed char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned short int>::subr(constant<unsigned char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned short int>::subr(constant<short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned short int>::subr(constant<unsigned short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned short int>::subr(constant<int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned short int>::subr(constant<unsigned int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned short int>::subr(constant<long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned short int>::subr(constant<unsigned long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned short int>::subr(constant<__int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned short int>::subr(constant<unsigned __int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned short int>::subr(constant<float>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<unsigned short int>::subr(constant<double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<unsigned short int>::subr(constant<long double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<unsigned short int>::subr(constant<void*>* y)
  { return constant_impl::psub(y,this); }
  template<>
  var* constant<int>::subr(constant<bool>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<int>::subr(constant<char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<int>::subr(constant<signed char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<int>::subr(constant<unsigned char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<int>::subr(constant<short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<int>::subr(constant<unsigned short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<int>::subr(constant<int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<int>::subr(constant<unsigned int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<int>::subr(constant<long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<int>::subr(constant<unsigned long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<int>::subr(constant<__int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<int>::subr(constant<unsigned __int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<int>::subr(constant<float>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<int>::subr(constant<double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<int>::subr(constant<long double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<int>::subr(constant<void*>* y)
  { return constant_impl::psub(y,this); }
  template<>
  var* constant<unsigned int>::subr(constant<bool>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned int>::subr(constant<char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned int>::subr(constant<signed char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned int>::subr(constant<unsigned char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned int>::subr(constant<short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned int>::subr(constant<unsigned short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned int>::subr(constant<int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned int>::subr(constant<unsigned int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned int>::subr(constant<long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned int>::subr(constant<unsigned long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned int>::subr(constant<__int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned int>::subr(constant<unsigned __int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned int>::subr(constant<float>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<unsigned int>::subr(constant<double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<unsigned int>::subr(constant<long double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<unsigned int>::subr(constant<void*>* y)
  { return constant_impl::psub(y,this); }
  template<>
  var* constant<long int>::subr(constant<bool>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<long int>::subr(constant<char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<long int>::subr(constant<signed char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<long int>::subr(constant<unsigned char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<long int>::subr(constant<short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<long int>::subr(constant<unsigned short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<long int>::subr(constant<int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<long int>::subr(constant<unsigned int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<long int>::subr(constant<long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<long int>::subr(constant<unsigned long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<long int>::subr(constant<__int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<long int>::subr(constant<unsigned __int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<long int>::subr(constant<float>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<long int>::subr(constant<double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<long int>::subr(constant<long double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<long int>::subr(constant<void*>* y)
  { return constant_impl::psub(y,this); }
  template<>
  var* constant<unsigned long int>::subr(constant<bool>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned long int>::subr(constant<char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned long int>::subr(constant<signed char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned long int>::subr(constant<unsigned char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned long int>::subr(constant<short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned long int>::subr(constant<unsigned short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned long int>::subr(constant<int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned long int>::subr(constant<unsigned int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned long int>::subr(constant<long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned long int>::subr(constant<unsigned long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned long int>::subr(constant<__int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned long int>::subr(constant<unsigned __int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned long int>::subr(constant<float>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<unsigned long int>::subr(constant<double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<unsigned long int>::subr(constant<long double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<unsigned long int>::subr(constant<void*>* y)
  { return constant_impl::psub(y,this); }
  template<>
  var* constant<__int64>::subr(constant<bool>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<__int64>::subr(constant<char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<__int64>::subr(constant<signed char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<__int64>::subr(constant<unsigned char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<__int64>::subr(constant<short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<__int64>::subr(constant<unsigned short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<__int64>::subr(constant<int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<__int64>::subr(constant<unsigned int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<__int64>::subr(constant<long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<__int64>::subr(constant<unsigned long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<__int64>::subr(constant<__int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<__int64>::subr(constant<unsigned __int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<__int64>::subr(constant<float>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<__int64>::subr(constant<double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<__int64>::subr(constant<long double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<__int64>::subr(constant<void*>* y)
  { return constant_impl::psub(y,this); }
  template<>
  var* constant<unsigned __int64>::subr(constant<bool>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned __int64>::subr(constant<char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned __int64>::subr(constant<signed char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned __int64>::subr(constant<unsigned char>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned __int64>::subr(constant<short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned __int64>::subr(constant<unsigned short int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned __int64>::subr(constant<int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned __int64>::subr(constant<unsigned int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned __int64>::subr(constant<long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned __int64>::subr(constant<unsigned long int>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned __int64>::subr(constant<__int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned __int64>::subr(constant<unsigned __int64>* y)
  { return constant_impl::sub(y,this); }
  template<>
  var* constant<unsigned __int64>::subr(constant<float>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<unsigned __int64>::subr(constant<double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<unsigned __int64>::subr(constant<long double>* y)
  { return constant_impl::fsub1(y,this); }
  template<>
  var* constant<unsigned __int64>::subr(constant<void*>* y)
  { return constant_impl::psub(y,this); }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<float>::sub(var* z){ return z->subr(this); }
cxx_compiler::var* cxx_compiler::constant<float>::subr(constant<bool>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::subr(constant<char>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::subr(constant<signed char>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::subr(constant<unsigned char>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::subr(constant<short int>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::subr(constant<unsigned short int>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::subr(constant<int>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::subr(constant<unsigned int>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::subr(constant<long int>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::subr(constant<unsigned long int>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::subr(constant<__int64>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::subr(constant<unsigned __int64>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::subr(constant<float>* y)
{ return constant_impl::fsub3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::subr(constant<double>* y)
{ return constant_impl::fsub3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::subr(constant<long double>* y)
{ return constant_impl::fsub3(y,this); }

cxx_compiler::var* cxx_compiler::constant<double>::sub(var* z){ return z->subr(this); }
cxx_compiler::var* cxx_compiler::constant<double>::subr(constant<bool>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::subr(constant<char>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::subr(constant<signed char>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::subr(constant<unsigned char>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::subr(constant<short int>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::subr(constant<unsigned short int>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::subr(constant<int>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::subr(constant<unsigned int>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::subr(constant<long int>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::subr(constant<unsigned long int>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::subr(constant<__int64>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::subr(constant<unsigned __int64>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::subr(constant<float>* y)
{ return constant_impl::fsub3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::subr(constant<double>* y)
{ return constant_impl::fsub3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::subr(constant<long double>* y)
{ return constant_impl::fsub3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::sub(var* z){ return z->subr(this); }
cxx_compiler::var* cxx_compiler::constant<long double>::subr(constant<bool>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::subr(constant<char>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::subr(constant<signed char>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::subr(constant<unsigned char>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::subr(constant<short int>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::subr(constant<unsigned short int>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::subr(constant<int>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::subr(constant<unsigned int>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::subr(constant<long int>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::subr(constant<unsigned long int>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::subr(constant<__int64>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::subr(constant<unsigned __int64>* y)
{ return constant_impl::fsub2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::subr(constant<float>* y)
{ return constant_impl::fsub3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::subr(constant<double>* y)
{ return constant_impl::fsub3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::subr(constant<long double>* y)
{ return constant_impl::fsub3(y,this); }

cxx_compiler::var* cxx_compiler::constant<void*>::subr(constant<void*>* that)
{
  const type* Ty = that->m_type;
  const type* Tz = this->m_type;
  if ( !Ty->compatible(Tz) )
    return var_impl::sub(that,this);
  typedef const pointer_type PT;
  PT* pt = static_cast<PT*>(Ty);
  const type* T = pt->referenced_type();
  int size = T->size();
  if ( !size )
    return var_impl::sub(that,this);
  int y = reinterpret_cast<__int64>(that->m_value);
  int z = reinterpret_cast<__int64>(this->m_value);
  return expressions::primary::literal::integer::create((long int)((y - z)/size));
}
