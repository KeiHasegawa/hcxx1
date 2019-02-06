// logical-and-expression
// logical-or-expression
// conditional-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

cxx_compiler::var* cxx_compiler::var::logic1(bool ANDAND, int n, var* expr)
{
  using namespace std;
  vector<tac*> tmp;
  copy(code.begin()+n,code.end(),back_inserter(tmp));
  code.resize(n);
  var* y = rvalue();
  y = y->promotion();
  const type* Ty = y->m_type;
  usr* zero = expressions::primary::literal::integer::create(0);
  usr* one = expressions::primary::literal::integer::create(1);
  goto3ac::op op = ANDAND ? goto3ac::EQ : goto3ac::NE;
  goto3ac* goto1 = new goto3ac(op,y,zero->cast(Ty));
  int pos = code.size();
  code.push_back(goto1);
  copy(tmp.begin(),tmp.end(),back_inserter(code));
  var* z = expr->rvalue();
  z = z->promotion();
  const type* Tz = z->m_type;
  if ( !Ty->scalar() || !Tz->scalar() ){
    using namespace error::expressions::binary;
    invalid(parse::position,ANDAND ? ANDAND_MK : OROR_MK,Ty,Tz);
  }
  goto3ac* goto2 = new goto3ac(op,z,zero->cast(Tz));
  code.push_back(goto2);
  log01* ret = new log01(int_type::create(),pos);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  var* u = ANDAND ? one : zero;
  (ANDAND ? ret->m_one : ret->m_zero) = code.size();
  code.push_back(new assign3ac(ret,u));
  goto3ac* goto3 = new goto3ac;
  code.push_back(goto3);
  to3ac* to = new to3ac;
  code.push_back(to);
  to->m_goto.push_back(goto1);
  to->m_goto.push_back(goto2);
  goto1->m_to = goto2->m_to = to;
  var* c = ANDAND ? zero : one;
  (ANDAND ? ret->m_zero : ret->m_one) = code.size();
  code.push_back(new assign3ac(ret,c));
  to3ac* to3 = new to3ac;
  code.push_back(to3);
  to3->m_goto.push_back(goto3);
  goto3->m_to = to3;
  return ret;
}

cxx_compiler::var* cxx_compiler::var::logic2(bool ANDAND, const type* Ty)
{
  var* z = rvalue();
  z = z->promotion();
  const type* Tz = z->m_type;
  if ( !Tz->scalar() ){
    using namespace error::expressions::binary;
    invalid(parse::position,ANDAND ? ANDAND_MK : OROR_MK,Ty,Tz);
  }
  usr* zero = expressions::primary::literal::integer::create(0);
  usr* one = expressions::primary::literal::integer::create(1);
  goto3ac* goto1 = new goto3ac(goto3ac::NE,z,zero->cast(Tz));
  code.push_back(goto1);
  var01* ret = new var01(int_type::create());
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  ret->m_zero = code.size();
  code.push_back(new assign3ac(ret,zero));
  goto3ac* goto2 = new goto3ac;
  code.push_back(goto2);
  to3ac* to1 = new to3ac;
  code.push_back(to1);
  to1->m_goto.push_back(goto1);
  goto1->m_to = to1;
  ret->m_one = code.size();
  code.push_back(new assign3ac(ret,one));
  to3ac* to2 = new to3ac;
  code.push_back(to2);
  to2->m_goto.push_back(goto2);
  goto2->m_to = to2;
  return ret;
}

namespace cxx_compiler { namespace constant_impl {
  template<class T> var* logic1(bool ANDAND, int n, constant<T>* y, var* z)
  {
    using namespace std;
    if ( ANDAND ){
      if ( y->zero() ){
        z = z->rvalue();
        const type* Tz = z->m_type;
        if ( !Tz->scalar() ){
          using namespace error::expressions;
          binary::invalid(parse::position,
			  ANDAND ? ANDAND_MK : OROR_MK,y->m_type,Tz);
        }
        for_each(code.begin()+n,code.end(),misc::deleter<tac>());
        code.resize(n);
        return expressions::primary::literal::integer::create(0);
      }
      else
        return z->logic2(ANDAND,y->m_type);
    }
    else {
      if ( !y->zero() ){
        z = z->rvalue();
        const type* Tz = z->m_type;
        if ( !Tz->scalar() ){
          using namespace error::expressions;
          binary::invalid(parse::position,
			  ANDAND ? ANDAND_MK : OROR_MK,y->m_type,Tz);
        }
        for_each(code.begin()+n,code.end(),misc::deleter<tac>());
        code.resize(n);
        return expressions::primary::literal::integer::create(1);
      }
      else
        return z->logic2(ANDAND,y->m_type);
    }
  }
} } // end of namespace constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::logic1(bool ANDAND, int n, var* z)
  { return constant_impl::logic1(ANDAND,n,this,z); }
  template<>
  var* constant<bool>::logic2(bool, const type*)
  {
    return zero() ? expressions::primary::literal::integer::create(0) : expressions::primary::literal::integer::create(1);
  }

  template<>
  var* constant<char>::logic1(bool ANDAND, int n, var* z)
  { return constant_impl::logic1(ANDAND,n,this,z); }
  template<>
  var* constant<char>::logic2(bool, const type*)
  {
    return zero() ? expressions::primary::literal::integer::create(0) : expressions::primary::literal::integer::create(1);
  }
  
  template<>
  var* constant<signed char>::logic1(bool ANDAND, int n, var* z)
  { return constant_impl::logic1(ANDAND,n,this,z); }
  template<>
  var* constant<signed char>::logic2(bool, const type*)
  {
    return zero() ? expressions::primary::literal::integer::create(0) : expressions::primary::literal::integer::create(1);
  }
  
  template<>
  var* constant<unsigned char>::logic1(bool ANDAND, int n, var* z)
  { return constant_impl::logic1(ANDAND,n,this,z); }
  template<>
  var* constant<unsigned char>::logic2(bool, const type*)
  {
    return zero() ? expressions::primary::literal::integer::create(0) : expressions::primary::literal::integer::create(1);
  }
  
  template<>
  var* constant<short int>::logic1(bool ANDAND, int n, var* z)
  { return constant_impl::logic1(ANDAND,n,this,z); }
  template<>
  var* constant<short int>::logic2(bool, const type*)
  {
    return zero() ? expressions::primary::literal::integer::create(0) : expressions::primary::literal::integer::create(1);
  }
  
  template<>
  var* constant<unsigned short int>::logic1(bool ANDAND, int n, var* z)
  { return constant_impl::logic1(ANDAND,n,this,z); }
  template<>
  var* constant<unsigned short int>::logic2(bool, const type*)
  {
    return zero() ? expressions::primary::literal::integer::create(0) : expressions::primary::literal::integer::create(1);
  }
  
  template<>
  var* constant<int>::logic1(bool ANDAND, int n, var* z)
  { return constant_impl::logic1(ANDAND,n,this,z); }
  template<>
  var* constant<int>::logic2(bool, const type*)
  {
    return zero() ? expressions::primary::literal::integer::create(0) : expressions::primary::literal::integer::create(1);
  }
  
  template<>
  var* constant<unsigned int>::logic1(bool ANDAND, int n, var* z)
  { return constant_impl::logic1(ANDAND,n,this,z); }
  template<>
  var* constant<unsigned int>::logic2(bool, const type*)
  {
    return zero() ? expressions::primary::literal::integer::create(0) : expressions::primary::literal::integer::create(1);
  }
  
  template<>
  var* constant<long int>::logic1(bool ANDAND, int n, var* z)
  { return constant_impl::logic1(ANDAND,n,this,z); }
  template<>
  var* constant<long int>::logic2(bool, const type*)
  {
    return zero() ? expressions::primary::literal::integer::create(0) : expressions::primary::literal::integer::create(1);
  }
  
  template<>
  var* constant<unsigned long int>::logic1(bool ANDAND, int n, var* z)
  { return constant_impl::logic1(ANDAND,n,this,z); }
  template<>
  var* constant<unsigned long int>::logic2(bool, const type*)
  {
    return zero() ? expressions::primary::literal::integer::create(0) : expressions::primary::literal::integer::create(1);
  }
  
  template<>
  var* constant<__int64>::logic1(bool ANDAND, int n, var* z)
  { return constant_impl::logic1(ANDAND,n,this,z); }
  template<>
  var* constant<__int64>::logic2(bool, const type*)
  {
    return zero() ? expressions::primary::literal::integer::create(0) : expressions::primary::literal::integer::create(1);
  }
  
  template<>
  var* constant<unsigned __int64>::logic1(bool ANDAND, int n, var* z)
  { return constant_impl::logic1(ANDAND,n,this,z); }
  template<>
  var* constant<unsigned __int64>::logic2(bool, const type*)
  {
    return zero() ? expressions::primary::literal::integer::create(0) : expressions::primary::literal::integer::create(1);
  }
} // end of namespace cxx_compiler


cxx_compiler::var* cxx_compiler::constant<float>::logic1(bool ANDAND, int n, var* z)
{ return constant_impl::logic1(ANDAND,n,this,z); }
cxx_compiler::var* cxx_compiler::constant<float>::logic2(bool, const type*)
{
  return zero() ? expressions::primary::literal::integer::create(0) : expressions::primary::literal::integer::create(1);
}

cxx_compiler::var* cxx_compiler::constant<double>::logic1(bool ANDAND, int n, var* z)
{ return constant_impl::logic1(ANDAND,n,this,z); }
cxx_compiler::var* cxx_compiler::constant<double>::logic2(bool, const type*)
{
  return zero() ? expressions::primary::literal::integer::create(0) : expressions::primary::literal::integer::create(1);
}

cxx_compiler::var* cxx_compiler::constant<long double>::logic1(bool ANDAND, int n, var* z)
{ return constant_impl::logic1(ANDAND,n,this,z); }
cxx_compiler::var* cxx_compiler::constant<long double>::logic2(bool, const type*)
{
  return zero() ? expressions::primary::literal::integer::create(0) : expressions::primary::literal::integer::create(1);
}

cxx_compiler::var* cxx_compiler::constant<void*>::logic1(bool ANDAND, int n, var* z)
{ return constant_impl::logic1(ANDAND,n,this,z); }
cxx_compiler::var* cxx_compiler::constant<void*>::logic2(bool, const type*)
{
  return zero() ? expressions::primary::literal::integer::create(0) : expressions::primary::literal::integer::create(1);
}

cxx_compiler::var* cxx_compiler::expressions::conditional::info_t::gen()
{
  var* expr1 = m_expr1->gen();
  int n = code.size();
  var* expr2 = m_expr2->gen();
  int m = code.size();
  var* expr3 = m_expr3->gen();
  return expr1->cond(n,m,expr2,expr3);
}

const cxx_compiler::file_t& cxx_compiler::expressions::conditional::info_t::file() const
{
  return m_expr1->file();
}

namespace cxx_compiler { namespace cond_impl {
  struct sweeper {
    int n;
    sweeper() : n(cxx_compiler::code.size()) {}
    ~sweeper()
    {
      using namespace std;
      using namespace cxx_compiler;
      for_each(code.begin()+n,code.end(),misc::deleter<tac>());
      code.resize(n);
    }
  };
  const type* valid(var*, var*);
} } // end of namespace cond_impl and cxx_compiler

const cxx_compiler::type*
cxx_compiler::cond_impl::valid(var* expr2, var* expr3)
{
  using namespace std;
  const type* T2 = expr2->m_type;
  const type* T3 = expr3->m_type;
  if ( T2->arithmetic() && T3->arithmetic() ){
    var* v2 = new var(T2); auto_ptr<var> sweeper2(v2);
    var* v3 = new var(T3); auto_ptr<var> sweeper3(v3);
    sweeper obj;
    return conversion::arithmetic::gen(&v2,&v3);
  }
  T2 = T2->complete_type();
  if ( T2->compatible(T3) )
    return T2->composite(T3);
  typedef const pointer_type PT;
  const type* pv = pointer_type::create(void_type::create());
  T2 = T2->unqualified();
  if ( T2->m_id == type::POINTER ){
    PT* pt = static_cast<PT*>(T2);
    if ( T3->integer() && expr3->zero() )
      return pt;
    if ( T3->compatible(pv) )
      return pt;
  }
  T3 = T3->unqualified();
  if ( T3->m_id == type::POINTER ){
    PT* pt = static_cast<PT*>(T3);
    if ( T2->integer() && expr2->zero() )
      return pt;
    if ( T2->compatible(pv) )
      return pt;
  }
  return 0;
}

namespace cxx_compiler { namespace var_impl {
  var* cond(var*, int, var*, int, var*);
} } // end of namespace var_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::var_impl::cond(var* expr1, int y, var* expr2, int x, var* expr3)
{
  using namespace std;
  vector<tac*> code3;
  copy(code.begin()+x,code.end(),back_inserter(code3));
  code.resize(x);
  vector<tac*> code2;
  copy(code.begin()+y,code.end(),back_inserter(code2));
  code.resize(y);
  expr1 = expr1->rvalue();
  if ( !expr1->m_type->scalar() ){
    using namespace error::expressions::conditional;
    not_scalar(parse::position);
    expr1->m_type = int_type::create();
  }
  var* zero = expressions::primary::literal::integer::create(0);
  zero = zero->cast(expr1->m_type);
  goto3ac* goto1 = new goto3ac(goto3ac::EQ,expr1,zero);
  code.push_back(goto1);
  copy(code2.begin(),code2.end(),back_inserter(code));
  expr2 = expr2->rvalue();
  if ( expr2->m_type->backpatch() ){
    using namespace error;
    usr* u = static_cast<usr*>(expr2);
    string name = u->m_name;
    file_t file = u->m_file;
    undeclared(file,name);
    expr2->m_type = int_type::create();
    scope::current->m_usrs[name].push_back(u);
  }
  int z = code.size();
  expr3 = expr3->rvalue();
  copy(code.begin()+z,code.end(),back_inserter(code3));
  code.resize(z);
  const type* T = cond_impl::valid(expr2,expr3);
  if ( !T ){
    using namespace error::expressions::conditional;
    mismatch(parse::position);
    T = int_type::create();
  }
  var* ret = new var(T);
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  bool v = T->compatible(void_type::create());
  if ( b && !v )
    b->m_vars.push_back(ret);
  else
    garbage.push_back(ret);
  if ( T->scalar() )
    expr2 = expr2->cast(T);
  if ( !v )
    code.push_back(new assign3ac(ret,expr2));
  goto3ac* goto2 = new goto3ac;
  code.push_back(goto2);
  to3ac* to1 = new to3ac;
  code.push_back(to1);
  to1->m_goto.push_back(goto1);
  goto1->m_to = to1;
  copy(code3.begin(),code3.end(),back_inserter(code));
  if ( T->scalar() )
    expr3 = expr3->cast(T);
  if ( !v )
    code.push_back(new assign3ac(ret,expr3));
  to3ac* to2 = new to3ac;
  code.push_back(to2);
  to2->m_goto.push_back(goto2);
  goto2->m_to = to2;
  return ret;
}

cxx_compiler::var* cxx_compiler::var::cond(int n, int m, var* expr2, var* expr3)
{ return var_impl::cond(this,n,expr2,m,expr3); }

namespace cxx_compiler { namespace constant_impl {
  template<class C> var* cond(constant<C>* x, int n, var* y, int m, var* z)
  {
    using namespace std;
    const type* T = cond_impl::valid(y,z);
    if ( T ){
      if ( x->zero() ){
        for_each(code.begin()+n,code.begin()+m,misc::deleter<tac>());
        vector<tac*>::iterator p = code.begin() + n;
        vector<tac*>::iterator q = code.begin() + m;
        code.erase(p,q);
        return z->cast(T);
      }
      else {
        for_each(code.begin()+m,code.end(),misc::deleter<tac>());
        code.resize(m);
        return y->cast(T);
      }
    }
    else
      return var_impl::cond(x,n,y,m,z);
  }
} } // end of namespace constant_impl

namespace cxx_compiler {
  template<>
  var* constant<bool>::cond(int n, int m, var* expr2, var* expr3)
  { return constant_impl::cond(this,n,expr2,m,expr3); }

  template<>
  var* constant<char>::cond(int n, int m, var* expr2, var* expr3)
  { return constant_impl::cond(this,n,expr2,m,expr3); }
  
  template<>
  var* constant<signed char>::cond(int n, int m, var* expr2, var* expr3)
  { return constant_impl::cond(this,n,expr2,m,expr3); }
  
  template<>
  var* constant<unsigned char>::cond(int n, int m, var* expr2, var* expr3)
  { return constant_impl::cond(this,n,expr2,m,expr3); }
  
  template<>
  var* constant<short int>::cond(int n, int m, var* expr2, var* expr3)
  { return constant_impl::cond(this,n,expr2,m,expr3); }
  
  template<>
  var* constant<unsigned short int>::cond(int n, int m, var* expr2, var* expr3)
  { return constant_impl::cond(this,n,expr2,m,expr3); }
  
  template<>
  var* constant<int>::cond(int n, int m, var* expr2, var* expr3)
  { return constant_impl::cond(this,n,expr2,m,expr3); }
  
  template<>
  var* constant<unsigned int>::cond(int n, int m, var* expr2, var* expr3)
  { return constant_impl::cond(this,n,expr2,m,expr3); }
  
  template<>
  var* constant<long int>::cond(int n, int m, var* expr2, var* expr3)
  { return constant_impl::cond(this,n,expr2,m,expr3); }
  
  template<>
  var* constant<unsigned long int>::cond(int n, int m, var* expr2, var* expr3)
  { return constant_impl::cond(this,n,expr2,m,expr3); }
  
  template<>
  var* constant<__int64>::cond(int n, int m, var* expr2, var* expr3)
  { return constant_impl::cond(this,n,expr2,m,expr3); }
  
  template<>
  var* constant<unsigned __int64>::cond(int n, int m, var* expr2, var* expr3)
  { return constant_impl::cond(this,n,expr2,m,expr3); }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<float>::cond(int n, int m, var* expr2, var* expr3)
{ return constant_impl::cond(this,n,expr2,m,expr3); }

cxx_compiler::var* cxx_compiler::constant<double>::cond(int n, int m, var* expr2, var* expr3)
{ return constant_impl::cond(this,n,expr2,m,expr3); }

cxx_compiler::var* cxx_compiler::constant<long double>::cond(int n, int m, var* expr2, var* expr3)
{ return constant_impl::cond(this,n,expr2,m,expr3); }

cxx_compiler::var* cxx_compiler::constant<void*>::cond(int n, int m, var* expr2, var* expr3)
{ return constant_impl::cond(this,n,expr2,m,expr3); }

cxx_compiler::var* cxx_compiler::var01::cond(int y, int x, var* expr2, var* expr3)
{
  using namespace std;
  if ( scope::current->m_id == scope::BLOCK )
    sweep();
  vector<tac*> code3;
  copy(code.begin()+x,code.end(),back_inserter(code3));
  code.resize(x);
  vector<tac*> code2;
  copy(code.begin()+y,code.end(),back_inserter(code2));
  code.resize(y);
  {
    int n = code.size();
    expr2 = expr2->rvalue();
    int m = code.size();
    copy(code.begin()+n,code.begin()+m,back_inserter(code2));
    code.resize(n);
  }
  {
    int n = code.size();
    expr3 = expr3->rvalue();
    int m = code.size();
    copy(code.begin()+n,code.begin()+m,back_inserter(code3));
    code.resize(n);
  }
  if ( expr2->m_type->backpatch() ){
    using namespace error;
    usr* u = static_cast<usr*>(expr2);
    string name = u->m_name;
    file_t file = u->m_file;
    undeclared(file,name);
    expr2->m_type = int_type::create();
    scope::current->m_usrs[name].push_back(u);
  }
  const type* T = cond_impl::valid(expr2,expr3);
  if ( !T ){
    using namespace error::expressions::conditional;
    mismatch(parse::position);
    T = int_type::create();
  }
  var* ret = new var(T);
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  bool v = T->compatible(void_type::create());
  if ( b && !v )
    b->m_vars.push_back(ret);
  else
    garbage.push_back(ret);
  if ( T->scalar() ){
    {
      int n = code.size();
      expr2 = expr2->cast(T);
      int m = code.size();
      copy(code.begin()+n,code.begin()+m,back_inserter(code2));
      code.resize(n);
    }
    {
      int n = code.size();
      expr3 = expr3->cast(T);
      int m = code.size();
      copy(code.begin()+n,code.begin()+m,back_inserter(code3));
      code.resize(n);
    }
  }
  if ( !v ){
    code2.push_back(new assign3ac(ret,expr2));
    code3.push_back(new assign3ac(ret,expr3));
  }
  if ( m_one < m_zero ){
    vector<tac*>::iterator p = code.begin() + m_one;
    delete *p;
    p = code.erase(p);
    code.insert(p,code2.begin(),code2.end());
    vector<tac*>::iterator q = code.begin() + m_zero + code2.size() - 1;
    delete *q;
    q = code.erase(q);
    code.insert(q,code3.begin(),code3.end());
  }
  else {
    vector<tac*>::iterator p = code.begin() + m_zero;
    delete *p;
    p = code.erase(p);
    code.insert(p,code3.begin(),code3.end());
    vector<tac*>::iterator q = code.begin() + m_one + code3.size() - 1;
    delete *q;
    q = code.erase(q);
    code.insert(q,code2.begin(),code2.end());
  }
  return ret;
}
