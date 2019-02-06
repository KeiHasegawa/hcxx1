// and-expression
// exclusive-or-expression
// inclusive-or-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

namespace cxx_compiler { namespace var_impl {
  var* bitwise(int, var*, var*);
  var* bit_and(var*, var*);
} } // end of namespace var_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::var_impl::bitwise(int op, var* a, var* b)
{
  var* y = a->rvalue();
  var* z = b->rvalue();
  const type* Ty = y->m_type;
  const type* Tz = z->m_type;
  const type* T;
  if ( Ty->integer() && Tz->integer() )
    T = conversion::arithmetic::gen(&y,&z);
  else {
    using namespace error::expressions::binary;
    invalid(parse::position,op,y->m_type,z->m_type);
    T = int_type::create();
  }
  var* x = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  switch ( op ){
  case '&': code.push_back(new and3ac(x,y,z)); break;
  case '^': code.push_back(new xor3ac(x,y,z)); break;
  case '|': code.push_back(new or3ac(x,y,z)); break;
  }
  return x;
}

cxx_compiler::var* cxx_compiler::var_impl::bit_and(var* a, var* b)
{
  return bitwise('&',a,b);
}

cxx_compiler::var* cxx_compiler::var::bit_and(var* z){ return var_impl::bit_and(this,z); }
cxx_compiler::var* cxx_compiler::var::bit_andr(constant<bool>* y){ return var_impl::bit_and(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_andr(constant<char>* y){ return var_impl::bit_and(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_andr(constant<signed char>* y){ return var_impl::bit_and(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_andr(constant<unsigned char>* y){ return var_impl::bit_and(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_andr(constant<short int>* y){ return var_impl::bit_and(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_andr(constant<unsigned short int>* y){ return var_impl::bit_and(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_andr(constant<int>* y){ return var_impl::bit_and(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_andr(constant<unsigned int>* y){ return var_impl::bit_and(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_andr(constant<long int>* y){ return var_impl::bit_and(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_andr(constant<unsigned long int>* y){ return var_impl::bit_and(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_andr(constant<__int64>* y){ return var_impl::bit_and(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_andr(constant<unsigned __int64>* y){ return var_impl::bit_and(y,this); }

namespace cxx_compiler { namespace constant_impl {
  template<class A, class B> var* bit_and(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    return integer::create(y->m_value & z->m_value);
  }
} } // end of namespace constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::bit_andr(constant<bool>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<bool>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<bool>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<bool>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<bool>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<bool>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<bool>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<bool>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<bool>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<bool>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<bool>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<bool>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<char>::bit_andr(constant<bool>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<char>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<char>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<char>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<char>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<char>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<char>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<char>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<char>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<char>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<char>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<char>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<signed char>::bit_andr(constant<bool>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<signed char>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<signed char>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<signed char>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<signed char>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<signed char>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<signed char>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<signed char>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<signed char>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<signed char>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<signed char>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<signed char>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned char>::bit_andr(constant<bool>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned char>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned char>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned char>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned char>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned char>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned char>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned char>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned char>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned char>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned char>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned char>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<short int>::bit_andr(constant<bool>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<short int>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<short int>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<short int>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<short int>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<short int>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<short int>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<short int>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<short int>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<short int>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<short int>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<short int>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned short int>::bit_andr(constant<bool>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned short int>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned short int>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned short int>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned short int>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned short int>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned short int>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned short int>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned short int>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned short int>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned short int>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned short int>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<int>::bit_andr(constant<bool>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<int>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<int>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<int>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<int>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<int>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<int>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<int>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<int>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<int>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<int>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<int>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned int>::bit_andr(constant<bool>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned int>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned int>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned int>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned int>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned int>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned int>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned int>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned int>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned int>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned int>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned int>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<long int>::bit_andr(constant<bool>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<long int>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<long int>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<long int>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<long int>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<long int>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<long int>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<long int>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<long int>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<long int>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<long int>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<long int>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned long int>::bit_andr(constant<bool>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned long int>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned long int>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned long int>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned long int>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned long int>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned long int>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned long int>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned long int>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned long int>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned long int>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned long int>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<__int64>::bit_andr(constant<bool>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<__int64>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<__int64>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<__int64>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<__int64>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<__int64>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<__int64>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<__int64>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<__int64>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<__int64>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<__int64>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<__int64>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_andr(constant<bool>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
} // end of namespace cxx_compiler

namespace cxx_compiler { namespace var_impl {
  var* bit_xor(var*, var*);
} } // end of namespace var_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::var_impl::bit_xor(var* a, var* b)
{
  return bitwise('^',a,b);
}

cxx_compiler::var* cxx_compiler::var::bit_xor(var* z){ return var_impl::bit_xor(this,z); }
cxx_compiler::var* cxx_compiler::var::bit_xorr(constant<bool>* y){ return var_impl::bit_xor(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_xorr(constant<char>* y){ return var_impl::bit_xor(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_xorr(constant<signed char>* y){ return var_impl::bit_xor(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_xorr(constant<unsigned char>* y){ return var_impl::bit_xor(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_xorr(constant<short int>* y){ return var_impl::bit_xor(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_xorr(constant<unsigned short int>* y){ return var_impl::bit_xor(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_xorr(constant<int>* y){ return var_impl::bit_xor(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_xorr(constant<unsigned int>* y){ return var_impl::bit_xor(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_xorr(constant<long int>* y){ return var_impl::bit_xor(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_xorr(constant<unsigned long int>* y){ return var_impl::bit_xor(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_xorr(constant<__int64>* y){ return var_impl::bit_xor(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_xorr(constant<unsigned __int64>* y){ return var_impl::bit_xor(y,this); }

namespace cxx_compiler { namespace constant_impl {
  template<class A, class B> var* bit_xor(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    return integer::create(y->m_value ^ z->m_value);
  }
} } // end of namespace constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::bit_xorr(constant<bool>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<bool>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<bool>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<bool>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<bool>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<bool>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<bool>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<bool>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<bool>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<bool>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<bool>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<bool>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<char>::bit_xorr(constant<bool>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<char>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<char>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<char>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<char>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<char>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<char>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<char>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<char>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<char>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<char>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<char>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<signed char>::bit_xorr(constant<bool>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<signed char>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<signed char>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<signed char>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<signed char>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<signed char>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<signed char>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<signed char>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<signed char>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<signed char>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<signed char>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<signed char>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned char>::bit_xorr(constant<bool>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned char>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned char>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned char>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned char>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned char>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned char>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned char>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned char>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned char>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned char>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned char>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<short int>::bit_xorr(constant<bool>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<short int>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<short int>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<short int>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<short int>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<short int>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<short int>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<short int>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<short int>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<short int>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<short int>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<short int>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned short int>::bit_xorr(constant<bool>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned short int>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned short int>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned short int>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned short int>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned short int>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned short int>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned short int>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned short int>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned short int>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned short int>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned short int>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<int>::bit_xorr(constant<bool>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<int>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<int>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<int>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<int>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<int>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<int>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<int>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<int>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<int>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<int>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<int>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned int>::bit_xorr(constant<bool>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned int>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned int>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned int>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned int>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned int>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned int>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned int>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned int>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned int>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned int>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned int>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<long int>::bit_xorr(constant<bool>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<long int>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<long int>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<long int>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<long int>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<long int>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<long int>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<long int>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<long int>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<long int>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<long int>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<long int>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned long int>::bit_xorr(constant<bool>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned long int>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned long int>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned long int>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned long int>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned long int>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned long int>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned long int>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned long int>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned long int>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned long int>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned long int>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<__int64>::bit_xorr(constant<bool>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<__int64>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<__int64>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<__int64>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<__int64>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<__int64>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<__int64>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<__int64>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<__int64>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<__int64>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<__int64>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<__int64>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_xorr(constant<bool>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
} // end of namespace cxx_compiler

namespace cxx_compiler { namespace var_impl {
  var* bit_or(var*, var*);
} } // end of namespace var_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::var_impl::bit_or(var* a, var* b)
{
  return bitwise('|',a,b);
}

cxx_compiler::var* cxx_compiler::var::bit_or(var* z){ return var_impl::bit_or(this,z); }
cxx_compiler::var* cxx_compiler::var::bit_orr(constant<bool>* y){ return var_impl::bit_or(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_orr(constant<char>* y){ return var_impl::bit_or(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_orr(constant<signed char>* y){ return var_impl::bit_or(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_orr(constant<unsigned char>* y){ return var_impl::bit_or(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_orr(constant<short int>* y){ return var_impl::bit_or(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_orr(constant<unsigned short int>* y){ return var_impl::bit_or(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_orr(constant<int>* y){ return var_impl::bit_or(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_orr(constant<unsigned int>* y){ return var_impl::bit_or(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_orr(constant<long int>* y){ return var_impl::bit_or(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_orr(constant<unsigned long int>* y){ return var_impl::bit_or(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_orr(constant<__int64>* y){ return var_impl::bit_or(y,this); }
cxx_compiler::var* cxx_compiler::var::bit_orr(constant<unsigned __int64>* y){ return var_impl::bit_or(y,this); }

namespace cxx_compiler { namespace constant_impl {
  template<class A, class B> var* bit_or(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    return integer::create(y->m_value | z->m_value);
  }
} } // end of namespace constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::bit_orr(constant<bool>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<bool>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<bool>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<bool>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<bool>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<bool>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<bool>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<bool>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<bool>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<bool>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<bool>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<bool>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<char>::bit_orr(constant<bool>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<char>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<char>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<char>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<char>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<char>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<char>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<char>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<char>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<char>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<char>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<char>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<signed char>::bit_orr(constant<bool>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<signed char>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<signed char>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<signed char>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<signed char>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<signed char>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<signed char>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<signed char>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<signed char>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<signed char>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<signed char>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<signed char>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned char>::bit_orr(constant<bool>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned char>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned char>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned char>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned char>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned char>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned char>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned char>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned char>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned char>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned char>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned char>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<short int>::bit_orr(constant<bool>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<short int>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<short int>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<short int>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<short int>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<short int>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<short int>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<short int>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<short int>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<short int>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<short int>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<short int>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned short int>::bit_orr(constant<bool>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned short int>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned short int>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned short int>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned short int>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned short int>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned short int>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned short int>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned short int>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned short int>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned short int>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned short int>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<int>::bit_orr(constant<bool>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<int>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<int>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<int>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<int>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<int>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<int>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<int>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<int>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<int>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<int>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<int>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned int>::bit_orr(constant<bool>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned int>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned int>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned int>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned int>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned int>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned int>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned int>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned int>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned int>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned int>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned int>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<long int>::bit_orr(constant<bool>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<long int>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<long int>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<long int>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<long int>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<long int>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<long int>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<long int>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<long int>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<long int>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<long int>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<long int>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned long int>::bit_orr(constant<bool>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned long int>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned long int>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned long int>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned long int>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned long int>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned long int>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned long int>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned long int>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned long int>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned long int>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned long int>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<__int64>::bit_orr(constant<bool>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<__int64>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<__int64>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<__int64>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<__int64>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<__int64>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<__int64>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<__int64>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<__int64>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<__int64>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<__int64>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<__int64>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_orr(constant<bool>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<>
  var* constant<unsigned __int64>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
} // end of namespace cxx_compiler
