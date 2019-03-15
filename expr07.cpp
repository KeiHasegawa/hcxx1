// shift-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

namespace cxx_compiler { namespace var_impl {
  var* lsh(var*, var*);
} } // end of namespace var_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::var_impl::lsh(var* y, var* z)
{
  const type* Ty = y->m_type;
  const type* Tz = z->m_type;
  const type* Tx = Ty->unqualified();  
  if ( !Ty->integer() || !Tz->integer() ){
    using namespace error::expressions::binary;
    invalid(parse::position,LSH_MK, Ty, Tz);
    Tx = int_type::create();
  }

  var* x = new var(Tx);
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
  var* constant<int>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<int>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<int>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<int>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<int>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<int>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }

  var* constant<unsigned int>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<unsigned int>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<unsigned int>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<unsigned int>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<unsigned int>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<unsigned int>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }

  var* constant<long int>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<long int>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<long int>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<long int>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<long int>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<long int>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }

  var* constant<unsigned long int>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<unsigned long int>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<unsigned long int>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<unsigned long int>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<unsigned long int>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<unsigned long int>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }

  var* constant<__int64>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<__int64>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<__int64>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<__int64>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<__int64>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<__int64>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }

  var* constant<unsigned __int64>::lshr(constant<int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<unsigned __int64>::lshr(constant<unsigned int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<unsigned __int64>::lshr(constant<long int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<unsigned __int64>::lshr(constant<unsigned long int>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<unsigned __int64>::lshr(constant<__int64>* y)
  { return constant_impl::lsh(y,this); }
  var* constant<unsigned __int64>::lshr(constant<unsigned __int64>* y)
  { return constant_impl::lsh(y,this); }
} // end of namespace cxx_compiler

namespace cxx_compiler { namespace var_impl {
  var* rsh(var*, var*);
} } // end of namespace var_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::var_impl::rsh(var* y, var* z)
{
  const type* Ty = y->m_type;
  const type* Tz = z->m_type;
  const type* Tx = Ty->unqualified();
  if ( !Ty->integer() || !Tz->integer()) {
    using namespace error::expressions::binary;
    invalid(parse::position,RSH_MK, Ty, Tz);
    Tx = int_type::create();
  }

  var* x = new var(Tx);
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
  var* constant<int>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<int>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<int>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<int>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<int>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<int>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }

  var* constant<unsigned int>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<unsigned int>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<unsigned int>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<unsigned int>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<unsigned int>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<unsigned int>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }

  var* constant<long int>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<long int>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<long int>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<long int>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<long int>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<long int>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }

  var* constant<unsigned long int>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<unsigned long int>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<unsigned long int>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<unsigned long int>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<unsigned long int>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<unsigned long int>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }

  var* constant<__int64>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<__int64>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<__int64>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<__int64>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<__int64>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<__int64>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }

  var* constant<unsigned __int64>::rshr(constant<int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<unsigned __int64>::rshr(constant<unsigned int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<unsigned __int64>::rshr(constant<long int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<unsigned __int64>::rshr(constant<unsigned long int>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<unsigned __int64>::rshr(constant<__int64>* y)
  { return constant_impl::rsh(y,this); }
  var* constant<unsigned __int64>::rshr(constant<unsigned __int64>* y)
  { return constant_impl::rsh(y,this); }
} // end of namespace cxx_compiler
