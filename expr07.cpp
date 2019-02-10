// shift-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

namespace cxx_compiler { namespace var_impl {
  var* lsh(var*, var*);
} } // end of namespace var_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::var_impl::lsh(var* a, var* b)
{
  var* y = a->rvalue();
  var* z = b->rvalue();
  y = y->promotion();
  z = z->promotion();
  const type* T = y->m_type;
  if ( !y->m_type->integer() || !z->m_type->integer() ){
    using namespace error::expressions::binary;
    invalid(parse::position,LSH_MK,y->m_type,z->m_type);
    T = int_type::create();
  }
  T = T->unqualified();
  var* x = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new lsh3ac(x,y,z));
  return x;
}

cxx_compiler::var* cxx_compiler::var::lsh(var* z){ return var_impl::lsh(this,z); }
cxx_compiler::var* cxx_compiler::var::lshr(constant<bool>* y){ return var_impl::lsh(y,this); }
cxx_compiler::var* cxx_compiler::var::lshr(constant<char>* y){ return var_impl::lsh(y,this); }
cxx_compiler::var* cxx_compiler::var::lshr(constant<signed char>* y){ return var_impl::lsh(y,this); }
cxx_compiler::var* cxx_compiler::var::lshr(constant<unsigned char>* y){ return var_impl::lsh(y,this); }
cxx_compiler::var* cxx_compiler::var::lshr(constant<wchar_t>* y){ return var_impl::lsh(y,this); }
cxx_compiler::var* cxx_compiler::var::lshr(constant<short int>* y){ return var_impl::lsh(y,this); }
cxx_compiler::var* cxx_compiler::var::lshr(constant<unsigned short int>* y){ return var_impl::lsh(y,this); }
cxx_compiler::var* cxx_compiler::var::lshr(constant<int>* y){ return var_impl::lsh(y,this); }
cxx_compiler::var* cxx_compiler::var::lshr(constant<unsigned int>* y){ return var_impl::lsh(y,this); }
cxx_compiler::var* cxx_compiler::var::lshr(constant<long int>* y){ return var_impl::lsh(y,this); }
cxx_compiler::var* cxx_compiler::var::lshr(constant<unsigned long int>* y){ return var_impl::lsh(y,this); }
cxx_compiler::var* cxx_compiler::var::lshr(constant<__int64>* y){ return var_impl::lsh(y,this); }
cxx_compiler::var* cxx_compiler::var::lshr(constant<unsigned __int64>* y){ return var_impl::lsh(y,this); }

namespace cxx_compiler { namespace constant_impl {
  template<class A, class B> var* lsh(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    usr::flag_t fy = y->m_flag;
    if (fy & usr::CONST_PTR)
      return var_impl::lsh(y, z);
    usr::flag_t fz = z->m_flag;
    if (fz & usr::CONST_PTR)
      return var_impl::lsh(y, z);
    usr* ret = integer::create(y->m_value << (int)z->m_value);
    if (fy & usr::SUB_CONST_LONG) {
      ret->m_type = y->m_type;
      ret->m_flag = usr::SUB_CONST_LONG;
    }
    return ret; 
  }
} } // end of namespace constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::lshr(constant<bool>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<bool>::lshr(constant<char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<bool>::lshr(constant<signed char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<bool>::lshr(constant<unsigned char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<bool>::lshr(constant<wchar_t>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<bool>::lshr(constant<short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<bool>::lshr(constant<unsigned short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<bool>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<bool>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<bool>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<bool>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<bool>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<bool>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<char>::lshr(constant<bool>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<char>::lshr(constant<char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<char>::lshr(constant<signed char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<char>::lshr(constant<unsigned char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<char>::lshr(constant<wchar_t>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<char>::lshr(constant<short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<char>::lshr(constant<unsigned short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<char>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<char>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<char>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<char>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<char>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<char>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<signed char>::lshr(constant<bool>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<signed char>::lshr(constant<char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<signed char>::lshr(constant<signed char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<signed char>::lshr(constant<unsigned char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<signed char>::lshr(constant<wchar_t>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<signed char>::lshr(constant<short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<signed char>::lshr(constant<unsigned short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<signed char>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<signed char>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<signed char>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<signed char>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<signed char>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<signed char>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned char>::lshr(constant<bool>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned char>::lshr(constant<char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned char>::lshr(constant<signed char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned char>::lshr(constant<unsigned char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned char>::lshr(constant<wchar_t>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned char>::lshr(constant<short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned char>::lshr(constant<unsigned short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned char>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned char>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned char>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned char>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned char>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned char>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<wchar_t>::lshr(constant<bool>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<wchar_t>::lshr(constant<char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<wchar_t>::lshr(constant<signed char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<wchar_t>::lshr(constant<unsigned char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<wchar_t>::lshr(constant<wchar_t>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<wchar_t>::lshr(constant<short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<wchar_t>::lshr(constant<unsigned short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<wchar_t>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<wchar_t>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<wchar_t>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<wchar_t>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<wchar_t>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<wchar_t>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<short int>::lshr(constant<bool>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<short int>::lshr(constant<char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<short int>::lshr(constant<signed char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<short int>::lshr(constant<unsigned char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<short int>::lshr(constant<wchar_t>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<short int>::lshr(constant<short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<short int>::lshr(constant<unsigned short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<short int>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<short int>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<short int>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<short int>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<short int>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<short int>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned short int>::lshr(constant<bool>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned short int>::lshr(constant<char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned short int>::lshr(constant<signed char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned short int>::lshr(constant<unsigned char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned short int>::lshr(constant<wchar_t>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned short int>::lshr(constant<short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned short int>::lshr(constant<unsigned short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned short int>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned short int>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned short int>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned short int>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned short int>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned short int>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<int>::lshr(constant<bool>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<int>::lshr(constant<char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<int>::lshr(constant<signed char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<int>::lshr(constant<unsigned char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<int>::lshr(constant<wchar_t>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<int>::lshr(constant<short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<int>::lshr(constant<unsigned short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<int>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<int>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<int>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<int>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<int>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<int>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned int>::lshr(constant<bool>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned int>::lshr(constant<char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned int>::lshr(constant<signed char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned int>::lshr(constant<unsigned char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned int>::lshr(constant<wchar_t>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned int>::lshr(constant<short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned int>::lshr(constant<unsigned short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned int>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned int>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned int>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned int>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned int>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned int>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<long int>::lshr(constant<bool>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<long int>::lshr(constant<char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<long int>::lshr(constant<signed char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<long int>::lshr(constant<unsigned char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<long int>::lshr(constant<wchar_t>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<long int>::lshr(constant<short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<long int>::lshr(constant<unsigned short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<long int>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<long int>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<long int>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<long int>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<long int>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<long int>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned long int>::lshr(constant<bool>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned long int>::lshr(constant<char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned long int>::lshr(constant<signed char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned long int>::lshr(constant<unsigned char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned long int>::lshr(constant<wchar_t>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned long int>::lshr(constant<short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned long int>::lshr(constant<unsigned short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned long int>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned long int>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned long int>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned long int>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned long int>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned long int>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<__int64>::lshr(constant<bool>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<__int64>::lshr(constant<char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<__int64>::lshr(constant<signed char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<__int64>::lshr(constant<unsigned char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<__int64>::lshr(constant<wchar_t>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<__int64>::lshr(constant<short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<__int64>::lshr(constant<unsigned short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<__int64>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<__int64>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<__int64>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<__int64>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<__int64>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<__int64>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned __int64>::lshr(constant<bool>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned __int64>::lshr(constant<char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned __int64>::lshr(constant<signed char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned __int64>::lshr(constant<unsigned char>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned __int64>::lshr(constant<wchar_t>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned __int64>::lshr(constant<short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned __int64>::lshr(constant<unsigned short int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned __int64>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned __int64>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned __int64>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned __int64>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned __int64>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  template<>
  var* constant<unsigned __int64>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }
} // end of namespace cxx_compiler

namespace cxx_compiler { namespace var_impl {
  var* rsh(var*, var*);
} } // end of namespace var_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::var_impl::rsh(var* a, var* b)
{
  var* y = a->rvalue();
  var* z = b->rvalue();
  y = y->promotion();
  z = z->promotion();
  const type* T = y->m_type;
  if ( !y->m_type->integer() || !z->m_type->integer() ){
    using namespace error::expressions::binary;
    invalid(parse::position,RSH_MK,y->m_type,z->m_type);
    T = int_type::create();
  }
  T = T->unqualified();
  var* x = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new rsh3ac(x,y,z));
  return x;
}

cxx_compiler::var* cxx_compiler::var::rsh(var* z){ return var_impl::rsh(this,z); }
cxx_compiler::var* cxx_compiler::var::rshr(constant<bool>* y){ return var_impl::rsh(y,this); }
cxx_compiler::var* cxx_compiler::var::rshr(constant<char>* y){ return var_impl::rsh(y,this); }
cxx_compiler::var* cxx_compiler::var::rshr(constant<signed char>* y){ return var_impl::rsh(y,this); }
cxx_compiler::var* cxx_compiler::var::rshr(constant<unsigned char>* y){ return var_impl::rsh(y,this); }
cxx_compiler::var* cxx_compiler::var::rshr(constant<wchar_t>* y){ return var_impl::rsh(y,this); }
cxx_compiler::var* cxx_compiler::var::rshr(constant<short int>* y){ return var_impl::rsh(y,this); }
cxx_compiler::var* cxx_compiler::var::rshr(constant<unsigned short int>* y){ return var_impl::rsh(y,this); }
cxx_compiler::var* cxx_compiler::var::rshr(constant<int>* y){ return var_impl::rsh(y,this); }
cxx_compiler::var* cxx_compiler::var::rshr(constant<unsigned int>* y){ return var_impl::rsh(y,this); }
cxx_compiler::var* cxx_compiler::var::rshr(constant<long int>* y){ return var_impl::rsh(y,this); }
cxx_compiler::var* cxx_compiler::var::rshr(constant<unsigned long int>* y){ return var_impl::rsh(y,this); }
cxx_compiler::var* cxx_compiler::var::rshr(constant<__int64>* y){ return var_impl::rsh(y,this); }
cxx_compiler::var* cxx_compiler::var::rshr(constant<unsigned __int64>* y){ return var_impl::rsh(y,this); }

namespace cxx_compiler { namespace constant_impl {
  template<class A, class B> var* rsh(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    usr::flag_t fy = y->m_flag;
    if (fy & usr::CONST_PTR)
      return var_impl::rsh(y, z);
    usr::flag_t fz = z->m_flag;
    if (fz & usr::CONST_PTR)
      return var_impl::rsh(y, z);
    usr* ret = integer::create(y->m_value >> z->m_value);
    if (fy & usr::SUB_CONST_LONG) {
      ret->m_type = y->m_type;
      ret->m_flag = usr::SUB_CONST_LONG;
    }
    return ret; 
  }
} } // end of namespace constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::rshr(constant<bool>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<bool>::rshr(constant<char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<bool>::rshr(constant<signed char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<bool>::rshr(constant<unsigned char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<bool>::rshr(constant<wchar_t>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<bool>::rshr(constant<short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<bool>::rshr(constant<unsigned short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<bool>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<bool>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<bool>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<bool>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<bool>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<bool>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<char>::rshr(constant<bool>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<char>::rshr(constant<char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<char>::rshr(constant<signed char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<char>::rshr(constant<unsigned char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<char>::rshr(constant<wchar_t>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<char>::rshr(constant<short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<char>::rshr(constant<unsigned short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<char>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<char>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<char>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<char>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<char>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<char>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<signed char>::rshr(constant<bool>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<signed char>::rshr(constant<char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<signed char>::rshr(constant<signed char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<signed char>::rshr(constant<unsigned char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<signed char>::rshr(constant<wchar_t>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<signed char>::rshr(constant<short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<signed char>::rshr(constant<unsigned short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<signed char>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<signed char>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<signed char>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<signed char>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<signed char>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<signed char>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned char>::rshr(constant<bool>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned char>::rshr(constant<char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned char>::rshr(constant<signed char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned char>::rshr(constant<unsigned char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned char>::rshr(constant<wchar_t>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned char>::rshr(constant<short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned char>::rshr(constant<unsigned short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned char>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned char>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned char>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned char>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned char>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned char>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<wchar_t>::rshr(constant<bool>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<wchar_t>::rshr(constant<char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<wchar_t>::rshr(constant<signed char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<wchar_t>::rshr(constant<unsigned char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<wchar_t>::rshr(constant<wchar_t>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<wchar_t>::rshr(constant<short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<wchar_t>::rshr(constant<unsigned short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<wchar_t>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<wchar_t>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<wchar_t>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<wchar_t>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<wchar_t>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<wchar_t>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<short int>::rshr(constant<bool>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<short int>::rshr(constant<char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<short int>::rshr(constant<signed char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<short int>::rshr(constant<unsigned char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<short int>::rshr(constant<wchar_t>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<short int>::rshr(constant<short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<short int>::rshr(constant<unsigned short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<short int>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<short int>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<short int>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<short int>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<short int>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<short int>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned short int>::rshr(constant<bool>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned short int>::rshr(constant<char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned short int>::rshr(constant<signed char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned short int>::rshr(constant<unsigned char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned short int>::rshr(constant<wchar_t>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned short int>::rshr(constant<short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned short int>::rshr(constant<unsigned short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned short int>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned short int>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned short int>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned short int>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned short int>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned short int>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<int>::rshr(constant<bool>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<int>::rshr(constant<char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<int>::rshr(constant<signed char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<int>::rshr(constant<unsigned char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<int>::rshr(constant<wchar_t>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<int>::rshr(constant<short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<int>::rshr(constant<unsigned short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<int>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<int>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<int>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<int>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<int>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<int>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned int>::rshr(constant<bool>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned int>::rshr(constant<char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned int>::rshr(constant<signed char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned int>::rshr(constant<unsigned char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned int>::rshr(constant<wchar_t>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned int>::rshr(constant<short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned int>::rshr(constant<unsigned short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned int>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned int>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned int>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned int>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned int>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned int>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<long int>::rshr(constant<bool>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<long int>::rshr(constant<char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<long int>::rshr(constant<signed char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<long int>::rshr(constant<unsigned char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<long int>::rshr(constant<wchar_t>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<long int>::rshr(constant<short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<long int>::rshr(constant<unsigned short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<long int>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<long int>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<long int>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<long int>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<long int>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<long int>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned long int>::rshr(constant<bool>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned long int>::rshr(constant<char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned long int>::rshr(constant<signed char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned long int>::rshr(constant<unsigned char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned long int>::rshr(constant<wchar_t>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned long int>::rshr(constant<short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned long int>::rshr(constant<unsigned short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned long int>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned long int>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned long int>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned long int>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned long int>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned long int>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<__int64>::rshr(constant<bool>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<__int64>::rshr(constant<char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<__int64>::rshr(constant<signed char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<__int64>::rshr(constant<unsigned char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<__int64>::rshr(constant<wchar_t>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<__int64>::rshr(constant<short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<__int64>::rshr(constant<unsigned short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<__int64>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<__int64>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<__int64>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<__int64>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<__int64>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<__int64>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned __int64>::rshr(constant<bool>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned __int64>::rshr(constant<char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned __int64>::rshr(constant<signed char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned __int64>::rshr(constant<unsigned char>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned __int64>::rshr(constant<wchar_t>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned __int64>::rshr(constant<short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned __int64>::rshr(constant<unsigned short int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned __int64>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned __int64>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned __int64>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned __int64>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned __int64>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  template<>
  var* constant<unsigned __int64>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }
} // end of namespace cxx_compiler
