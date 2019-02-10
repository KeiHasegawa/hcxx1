// relational-expression
// equality-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

namespace cxx_compiler { namespace expressions { namespace cmp_impl {
  var* gen(goto3ac::op, var*, var*);
  bool valid_pointer(goto3ac::op, var*, var*);
} } } // end of namespace cmp_impl, expressions and cxx_compiler

cxx_compiler::opposite_t::opposite_t()
{
  (*this)[goto3ac::EQ] = goto3ac::NE;
  (*this)[goto3ac::NE] = goto3ac::EQ;
  (*this)[goto3ac::LE] = goto3ac::GT;
  (*this)[goto3ac::GE] = goto3ac::LT;
  (*this)[goto3ac::LT] = goto3ac::GE;
  (*this)[goto3ac::GT] = goto3ac::LE;
}

cxx_compiler::opposite_t cxx_compiler::opposite;

cxx_compiler::var*
cxx_compiler::expressions::cmp_impl::gen(goto3ac::op op, var* a, var* b)
{
  using namespace std;
  var* y = a->rvalue();
  var* z = b->rvalue();
  const type* Ty = y->m_type;
  const type* Tz = z->m_type;
  if ( Ty->arithmetic() && Tz->arithmetic() )
    conversion::arithmetic::gen(&y,&z);
  else if ( !cmp_impl::valid_pointer(op,y,z) ){
    using namespace error::expressions::binary;
    switch ( op ){
    case goto3ac::LT: invalid(parse::position,'<',Ty,Tz); break;
    case goto3ac::GT: invalid(parse::position,'>',Ty,Tz); break;
    case goto3ac::LE: invalid(parse::position,LESSEQ_MK,Ty,Tz); break;
    case goto3ac::GE: invalid(parse::position,GREATEREQ_MK,Ty,Tz); break;
    case goto3ac::EQ: invalid(parse::position,EQUAL_MK,Ty,Tz); break;
    case goto3ac::NE: invalid(parse::position,NOTEQ_MK,Ty,Tz); break;
    }
  }
  usr* zero = primary::literal::integer::create(0);
  usr* one = primary::literal::integer::create(1);
  var01* ret = new var01(int_type::create());
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  goto3ac* goto1 = new goto3ac(opposite[op],y,z);
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

bool
cxx_compiler::expressions::cmp_impl::valid_pointer(goto3ac::op op,
                                                   var* y, var* z)
{
  const type* Ty = y->m_type;
  const type* Tz = z->m_type;
  Ty = Ty->unqualified();
  Tz = Tz->unqualified();
  typedef const pointer_type PT;
  PT* py = Ty->m_id == type::POINTER ? static_cast<PT*>(Ty) : 0;
  PT* pz = Tz->m_id == type::POINTER ? static_cast<PT*>(Tz) : 0;
  const type* Ry = py ? py->referenced_type()->unqualified() : 0;
  const type* Rz = pz ? pz->referenced_type()->unqualified() : 0;
  if (py && pz && compatible(Ry, Rz))
    return true;
  if ( op != goto3ac::EQ && op != goto3ac::NE )
    return false;
  const void_type* v = void_type::create();
  if ( py && pz && compatible(Rz, v) )
    return true;
  if ( pz && py && compatible(Ry, v) )
    return true;
  if ( py && z->isconstant() && Tz->integer() && z->value() == 0 )
    return true;
  if ( pz && y->isconstant() && Ty->integer() && y->value() == 0 )
    return true;
  return false;
}

cxx_compiler::var* cxx_compiler::var::lt(var* z)
{ return expressions::cmp_impl::gen(goto3ac::LT,this,z); }
cxx_compiler::var* cxx_compiler::var::ltr(constant<bool>* y)
{ return expressions::cmp_impl::gen(goto3ac::LT,y,this); }
cxx_compiler::var* cxx_compiler::var::ltr(constant<char>* y)
{ return expressions::cmp_impl::gen(goto3ac::LT,y,this); }
cxx_compiler::var* cxx_compiler::var::ltr(constant<signed char>* y)
{ return expressions::cmp_impl::gen(goto3ac::LT,y,this); }
cxx_compiler::var* cxx_compiler::var::ltr(constant<unsigned char>* y)
{ return expressions::cmp_impl::gen(goto3ac::LT,y,this); }
cxx_compiler::var* cxx_compiler::var::ltr(constant<wchar_t>* y)
{ return expressions::cmp_impl::gen(goto3ac::LT,y,this); }
cxx_compiler::var* cxx_compiler::var::ltr(constant<short int>* y)
{ return expressions::cmp_impl::gen(goto3ac::LT,y,this); }
cxx_compiler::var* cxx_compiler::var::ltr(constant<unsigned short int>* y)
{ return expressions::cmp_impl::gen(goto3ac::LT,y,this); }
cxx_compiler::var* cxx_compiler::var::ltr(constant<int>* y)
{ return expressions::cmp_impl::gen(goto3ac::LT,y,this); }
cxx_compiler::var* cxx_compiler::var::ltr(constant<unsigned int>* y)
{ return expressions::cmp_impl::gen(goto3ac::LT,y,this); }
cxx_compiler::var* cxx_compiler::var::ltr(constant<long int>* y)
{ return expressions::cmp_impl::gen(goto3ac::LT,y,this); }
cxx_compiler::var* cxx_compiler::var::ltr(constant<unsigned long int>* y)
{ return expressions::cmp_impl::gen(goto3ac::LT,y,this); }
cxx_compiler::var* cxx_compiler::var::ltr(constant<__int64>* y)
{ return expressions::cmp_impl::gen(goto3ac::LT,y,this); }
cxx_compiler::var* cxx_compiler::var::ltr(constant<unsigned __int64>* y)
{ return expressions::cmp_impl::gen(goto3ac::LT,y,this); }
cxx_compiler::var* cxx_compiler::var::ltr(constant<float>* y)
{ return expressions::cmp_impl::gen(goto3ac::LT,y,this); }
cxx_compiler::var* cxx_compiler::var::ltr(constant<double>* y)
{ return expressions::cmp_impl::gen(goto3ac::LT,y,this); }
cxx_compiler::var* cxx_compiler::var::ltr(constant<long double>* y)
{ return expressions::cmp_impl::gen(goto3ac::LT,y,this); }
cxx_compiler::var* cxx_compiler::var::ltr(constant<void*>* y)
{ return expressions::cmp_impl::gen(goto3ac::LT,y,this); }
cxx_compiler::var* cxx_compiler::var::ltr(addrof* y)
{ return expressions::cmp_impl::gen(goto3ac::LT,y,this); }

cxx_compiler::var* cxx_compiler::var::gt(var* z)
{ return expressions::cmp_impl::gen(goto3ac::GT,this,z); }
cxx_compiler::var* cxx_compiler::var::gtr(constant<bool>* y)
{ return expressions::cmp_impl::gen(goto3ac::GT,y,this); }
cxx_compiler::var* cxx_compiler::var::gtr(constant<char>* y)
{ return expressions::cmp_impl::gen(goto3ac::GT,y,this); }
cxx_compiler::var* cxx_compiler::var::gtr(constant<signed char>* y)
{ return expressions::cmp_impl::gen(goto3ac::GT,y,this); }
cxx_compiler::var* cxx_compiler::var::gtr(constant<unsigned char>* y)
{ return expressions::cmp_impl::gen(goto3ac::GT,y,this); }
cxx_compiler::var* cxx_compiler::var::gtr(constant<wchar_t>* y)
{ return expressions::cmp_impl::gen(goto3ac::GT,y,this); }
cxx_compiler::var* cxx_compiler::var::gtr(constant<short int>* y)
{ return expressions::cmp_impl::gen(goto3ac::GT,y,this); }
cxx_compiler::var* cxx_compiler::var::gtr(constant<unsigned short int>* y)
{ return expressions::cmp_impl::gen(goto3ac::GT,y,this); }
cxx_compiler::var* cxx_compiler::var::gtr(constant<int>* y)
{ return expressions::cmp_impl::gen(goto3ac::GT,y,this); }
cxx_compiler::var* cxx_compiler::var::gtr(constant<unsigned int>* y)
{ return expressions::cmp_impl::gen(goto3ac::GT,y,this); }
cxx_compiler::var* cxx_compiler::var::gtr(constant<long int>* y)
{ return expressions::cmp_impl::gen(goto3ac::GT,y,this); }
cxx_compiler::var* cxx_compiler::var::gtr(constant<unsigned long int>* y)
{ return expressions::cmp_impl::gen(goto3ac::GT,y,this); }
cxx_compiler::var* cxx_compiler::var::gtr(constant<__int64>* y)
{ return expressions::cmp_impl::gen(goto3ac::GT,y,this); }
cxx_compiler::var* cxx_compiler::var::gtr(constant<unsigned __int64>* y)
{ return expressions::cmp_impl::gen(goto3ac::GT,y,this); }
cxx_compiler::var* cxx_compiler::var::gtr(constant<float>* y)
{ return expressions::cmp_impl::gen(goto3ac::GT,y,this); }
cxx_compiler::var* cxx_compiler::var::gtr(constant<double>* y)
{ return expressions::cmp_impl::gen(goto3ac::GT,y,this); }
cxx_compiler::var* cxx_compiler::var::gtr(constant<long double>* y)
{ return expressions::cmp_impl::gen(goto3ac::GT,y,this); }
cxx_compiler::var* cxx_compiler::var::gtr(constant<void*>* y)
{ return expressions::cmp_impl::gen(goto3ac::GT,y,this); }
cxx_compiler::var* cxx_compiler::var::gtr(addrof* y)
{ return expressions::cmp_impl::gen(goto3ac::GT,y,this); }

cxx_compiler::var* cxx_compiler::var::le(var* z)
{ return expressions::cmp_impl::gen(goto3ac::LE,this,z); }
cxx_compiler::var* cxx_compiler::var::ler(constant<bool>* y)
{ return expressions::cmp_impl::gen(goto3ac::LE,y,this); }
cxx_compiler::var* cxx_compiler::var::ler(constant<char>* y)
{ return expressions::cmp_impl::gen(goto3ac::LE,y,this); }
cxx_compiler::var* cxx_compiler::var::ler(constant<signed char>* y)
{ return expressions::cmp_impl::gen(goto3ac::LE,y,this); }
cxx_compiler::var* cxx_compiler::var::ler(constant<unsigned char>* y)
{ return expressions::cmp_impl::gen(goto3ac::LE,y,this); }
cxx_compiler::var* cxx_compiler::var::ler(constant<wchar_t>* y)
{ return expressions::cmp_impl::gen(goto3ac::LE,y,this); }
cxx_compiler::var* cxx_compiler::var::ler(constant<short int>* y)
{ return expressions::cmp_impl::gen(goto3ac::LE,y,this); }
cxx_compiler::var* cxx_compiler::var::ler(constant<unsigned short int>* y)
{ return expressions::cmp_impl::gen(goto3ac::LE,y,this); }
cxx_compiler::var* cxx_compiler::var::ler(constant<int>* y)
{ return expressions::cmp_impl::gen(goto3ac::LE,y,this); }
cxx_compiler::var* cxx_compiler::var::ler(constant<unsigned int>* y)
{ return expressions::cmp_impl::gen(goto3ac::LE,y,this); }
cxx_compiler::var* cxx_compiler::var::ler(constant<long int>* y)
{ return expressions::cmp_impl::gen(goto3ac::LE,y,this); }
cxx_compiler::var* cxx_compiler::var::ler(constant<unsigned long int>* y)
{ return expressions::cmp_impl::gen(goto3ac::LE,y,this); }
cxx_compiler::var* cxx_compiler::var::ler(constant<__int64>* y)
{ return expressions::cmp_impl::gen(goto3ac::LE,y,this); }
cxx_compiler::var* cxx_compiler::var::ler(constant<unsigned __int64>* y)
{ return expressions::cmp_impl::gen(goto3ac::LE,y,this); }
cxx_compiler::var* cxx_compiler::var::ler(constant<float>* y)
{ return expressions::cmp_impl::gen(goto3ac::LE,y,this); }
cxx_compiler::var* cxx_compiler::var::ler(constant<double>* y)
{ return expressions::cmp_impl::gen(goto3ac::LE,y,this); }
cxx_compiler::var* cxx_compiler::var::ler(constant<long double>* y)
{ return expressions::cmp_impl::gen(goto3ac::LE,y,this); }
cxx_compiler::var* cxx_compiler::var::ler(constant<void*>* y)
{ return expressions::cmp_impl::gen(goto3ac::LE,y,this); }
cxx_compiler::var* cxx_compiler::var::ler(addrof* y)
{ return expressions::cmp_impl::gen(goto3ac::LE,y,this); }

cxx_compiler::var* cxx_compiler::var::ge(var* z)
{ return expressions::cmp_impl::gen(goto3ac::GE,this,z); }
cxx_compiler::var* cxx_compiler::var::ger(constant<bool>* y)
{ return expressions::cmp_impl::gen(goto3ac::GE,y,this); }
cxx_compiler::var* cxx_compiler::var::ger(constant<char>* y)
{ return expressions::cmp_impl::gen(goto3ac::GE,y,this); }
cxx_compiler::var* cxx_compiler::var::ger(constant<signed char>* y)
{ return expressions::cmp_impl::gen(goto3ac::GE,y,this); }
cxx_compiler::var* cxx_compiler::var::ger(constant<unsigned char>* y)
{ return expressions::cmp_impl::gen(goto3ac::GE,y,this); }
cxx_compiler::var* cxx_compiler::var::ger(constant<wchar_t>* y)
{ return expressions::cmp_impl::gen(goto3ac::GE,y,this); }
cxx_compiler::var* cxx_compiler::var::ger(constant<short int>* y)
{ return expressions::cmp_impl::gen(goto3ac::GE,y,this); }
cxx_compiler::var* cxx_compiler::var::ger(constant<unsigned short int>* y)
{ return expressions::cmp_impl::gen(goto3ac::GE,y,this); }
cxx_compiler::var* cxx_compiler::var::ger(constant<int>* y)
{ return expressions::cmp_impl::gen(goto3ac::GE,y,this); }
cxx_compiler::var* cxx_compiler::var::ger(constant<unsigned int>* y)
{ return expressions::cmp_impl::gen(goto3ac::GE,y,this); }
cxx_compiler::var* cxx_compiler::var::ger(constant<long int>* y)
{ return expressions::cmp_impl::gen(goto3ac::GE,y,this); }
cxx_compiler::var* cxx_compiler::var::ger(constant<unsigned long int>* y)
{ return expressions::cmp_impl::gen(goto3ac::GE,y,this); }
cxx_compiler::var* cxx_compiler::var::ger(constant<__int64>* y)
{ return expressions::cmp_impl::gen(goto3ac::GE,y,this); }
cxx_compiler::var* cxx_compiler::var::ger(constant<unsigned __int64>* y)
{ return expressions::cmp_impl::gen(goto3ac::GE,y,this); }
cxx_compiler::var* cxx_compiler::var::ger(constant<float>* y)
{ return expressions::cmp_impl::gen(goto3ac::GE,y,this); }
cxx_compiler::var* cxx_compiler::var::ger(constant<double>* y)
{ return expressions::cmp_impl::gen(goto3ac::GE,y,this); }
cxx_compiler::var* cxx_compiler::var::ger(constant<long double>* y)
{ return expressions::cmp_impl::gen(goto3ac::GE,y,this); }
cxx_compiler::var* cxx_compiler::var::ger(constant<void*>* y)
{ return expressions::cmp_impl::gen(goto3ac::GE,y,this); }
cxx_compiler::var* cxx_compiler::var::ger(addrof* y)
{ return expressions::cmp_impl::gen(goto3ac::GE,y,this); }

cxx_compiler::var* cxx_compiler::var::eq(var* z)
{ return expressions::cmp_impl::gen(goto3ac::EQ,this,z); }
cxx_compiler::var* cxx_compiler::var::eqr(constant<bool>* y)
{ return expressions::cmp_impl::gen(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::var::eqr(constant<char>* y)
{ return expressions::cmp_impl::gen(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::var::eqr(constant<signed char>* y)
{ return expressions::cmp_impl::gen(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::var::eqr(constant<unsigned char>* y)
{ return expressions::cmp_impl::gen(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::var::eqr(constant<wchar_t>* y)
{ return expressions::cmp_impl::gen(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::var::eqr(constant<short int>* y)
{ return expressions::cmp_impl::gen(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::var::eqr(constant<unsigned short int>* y)
{ return expressions::cmp_impl::gen(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::var::eqr(constant<int>* y)
{ return expressions::cmp_impl::gen(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::var::eqr(constant<unsigned int>* y)
{ return expressions::cmp_impl::gen(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::var::eqr(constant<long int>* y)
{ return expressions::cmp_impl::gen(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::var::eqr(constant<unsigned long int>* y)
{ return expressions::cmp_impl::gen(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::var::eqr(constant<__int64>* y)
{ return expressions::cmp_impl::gen(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::var::eqr(constant<unsigned __int64>* y)
{ return expressions::cmp_impl::gen(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::var::eqr(constant<float>* y)
{ return expressions::cmp_impl::gen(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::var::eqr(constant<double>* y)
{ return expressions::cmp_impl::gen(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::var::eqr(constant<long double>* y)
{ return expressions::cmp_impl::gen(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::var::eqr(constant<void*>* y)
{ return expressions::cmp_impl::gen(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::var::eqr(addrof* y)
{ return expressions::cmp_impl::gen(goto3ac::EQ,y,this); }

cxx_compiler::var* cxx_compiler::var::ne(var* z)
{ return expressions::cmp_impl::gen(goto3ac::NE,this,z); }
cxx_compiler::var* cxx_compiler::var::ner(constant<bool>* y)
{ return expressions::cmp_impl::gen(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::var::ner(constant<char>* y)
{ return expressions::cmp_impl::gen(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::var::ner(constant<signed char>* y)
{ return expressions::cmp_impl::gen(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::var::ner(constant<unsigned char>* y)
{ return expressions::cmp_impl::gen(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::var::ner(constant<wchar_t>* y)
{ return expressions::cmp_impl::gen(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::var::ner(constant<short int>* y)
{ return expressions::cmp_impl::gen(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::var::ner(constant<unsigned short int>* y)
{ return expressions::cmp_impl::gen(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::var::ner(constant<int>* y)
{ return expressions::cmp_impl::gen(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::var::ner(constant<unsigned int>* y)
{ return expressions::cmp_impl::gen(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::var::ner(constant<long int>* y)
{ return expressions::cmp_impl::gen(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::var::ner(constant<unsigned long int>* y)
{ return expressions::cmp_impl::gen(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::var::ner(constant<__int64>* y)
{ return expressions::cmp_impl::gen(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::var::ner(constant<unsigned __int64>* y)
{ return expressions::cmp_impl::gen(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::var::ner(constant<float>* y)
{ return expressions::cmp_impl::gen(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::var::ner(constant<double>* y)
{ return expressions::cmp_impl::gen(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::var::ner(constant<long double>* y)
{ return expressions::cmp_impl::gen(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::var::ner(constant<void*>* y)
{ return expressions::cmp_impl::gen(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::var::ner(addrof* y)
{ return expressions::cmp_impl::gen(goto3ac::NE,y,this); }

namespace cxx_compiler { namespace constant_impl {
  template<class A, class B> var* lt(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if (expressions::cmp_impl::valid_pointer(goto3ac::LT, y, z)) {
      constant<__int64>* yy = reinterpret_cast<constant<__int64>*>(y);
      constant<__int64>* zz = reinterpret_cast<constant<__int64>*>(z);
      return integer::create(yy->m_value < zz->m_value); 
    }
    usr::flag_t fy = y->m_flag;
    if (fy & usr::CONST_PTR)
      return expressions::cmp_impl::gen(goto3ac::LT, y,z);
    usr::flag_t fz = z->m_flag;
    if (fz & usr::CONST_PTR)
      return expressions::cmp_impl::gen(goto3ac::LT, y,z);
    return integer::create(y->m_value < z->m_value); 
  }
  template<class A, class B> var* fop1(constant<A>* y, constant<B>* z, goto3ac::op op)
  {
    using namespace std;
    using namespace expressions::primary::literal;
    const type* Ty = y->m_type;
    Ty = Ty->unqualified();
    if (Ty->m_id == type::LONG_DOUBLE) {
      int sz = long_double_type::create()->size();
      auto_ptr<unsigned char> p =
        auto_ptr<unsigned char>(new unsigned char[sz]);
#ifndef _MSC_VER
      (*generator::long_double->from_double)(p.get(),z->m_value);
#else // _MSC_VER
      (*generator::long_double->from_double)(p.get(),(__int64)z->m_value);
#endif // _MSC_VER
      constant<long double>* yy = dynamic_cast<constant<long double>*>(y);
      bool b = (*generator::long_double->cmp)(op,yy->b,p.get());
      return b ? integer::create(1) : integer::create(0);
    }
    return 0;
  }
  template<class A, class B> var* flt1(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if ( generator::long_double ){
      if ( var* v = fop1(y,z,goto3ac::LT) )
        return v;
    }
#ifndef _MSC_VER
    return integer::create(y->m_value < z->m_value); 
#else // _MSC_VER
    return integer::create(y->m_value < (__int64)z->m_value); 
#endif // _MSC_VER
  }
  template<class A, class B> var* fop2(constant<A>* y, constant<B>* z, goto3ac::op op)
  {
    using namespace std;
    using namespace expressions::primary::literal;
    const type* Tz = z->m_type;
    Tz = Tz->unqualified();
    if (Tz->m_id == type::LONG_DOUBLE) {
      int sz = long_double_type::create()->size();
      auto_ptr<unsigned char> p = auto_ptr<unsigned char>(new unsigned char[sz]);
#ifndef _MSC_VER
      (*generator::long_double->from_double)(p.get(),y->m_value);
#else // _MSC_VER
      (*generator::long_double->from_double)(p.get(),(__int64)y->m_value);
#endif // _MSC_VER
      constant<long double>* zz = dynamic_cast<constant<long double>*>(z);
      bool b = (*generator::long_double->cmp)(op,p.get(),zz->b);
      return b ? integer::create(1) : integer::create(0);
    }
    return 0;
  }
  template<class A, class B> var* flt2(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if ( generator::long_double ){
      if ( var* v = fop2(y,z,goto3ac::LT) )
        return v;
    }
#ifndef _MSC_VER
    return integer::create(y->m_value < z->m_value); 
#else // _MSC_VER
    return integer::create((__int64)y->m_value < z->m_value); 
#endif // _MSC_VER
  }
  template<class A, class B> var* fop3(constant<A>* y, constant<B>* z, goto3ac::op op)
  {
    using namespace std;
    using namespace expressions::primary::literal;
    const type* Ty = y->m_type;
    const type* Tz = z->m_type;
    Ty = Ty->unqualified();
    Tz = Tz->unqualified();
    int sz = long_double_type::create()->size();
    if (Ty->m_id == type::LONG_DOUBLE) {
      constant<long double>* yy = reinterpret_cast<constant<long double>*>(y);
      if (Tz->m_id == type::LONG_DOUBLE) {
        constant<long double>* zz = reinterpret_cast<constant<long double>*>(z);
        bool b = (*generator::long_double->cmp)(op,yy->b,zz->b);
        return b ? integer::create(1) : integer::create(0);
      }
      auto_ptr<unsigned char> p =
        auto_ptr<unsigned char>(new unsigned char[sz]);
      (*generator::long_double->from_double)(p.get(),z->m_value);
      bool b = (*generator::long_double->cmp)(op,yy->b,p.get());
      return b ? integer::create(1) : integer::create(0);
    }
    if (Tz->m_id == type::LONG_DOUBLE) {
      auto_ptr<unsigned char> p =
        auto_ptr<unsigned char>(new unsigned char[sz]);
      (*generator::long_double->from_double)(p.get(),y->m_value);
      constant<long double>* zz = reinterpret_cast<constant<long double>*>(z);
      bool b = (*generator::long_double->cmp)(op,p.get(),zz->b);
      return b ? integer::create(1) : integer::create(0);
    }
    return 0;
  }
  template<class A, class B> var* flt3(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if ( generator::long_double ){
      if ( var* v = fop3(y,z,goto3ac::LT) )
        return v;
    }
    return integer::create(y->m_value < z->m_value); 
  }
} } // end of namespace constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::ltr(constant<bool>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<bool>::ltr(constant<char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<bool>::ltr(constant<signed char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<bool>::ltr(constant<unsigned char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<bool>::ltr(constant<wchar_t>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<bool>::ltr(constant<short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<bool>::ltr(constant<unsigned short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<bool>::ltr(constant<int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<bool>::ltr(constant<unsigned int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<bool>::ltr(constant<long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<bool>::ltr(constant<unsigned long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<bool>::ltr(constant<__int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<bool>::ltr(constant<unsigned __int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<bool>::ltr(constant<float>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<bool>::ltr(constant<double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<bool>::ltr(constant<long double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<char>::ltr(constant<bool>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<char>::ltr(constant<char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<char>::ltr(constant<signed char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<char>::ltr(constant<unsigned char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<char>::ltr(constant<wchar_t>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<char>::ltr(constant<short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<char>::ltr(constant<unsigned short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<char>::ltr(constant<int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<char>::ltr(constant<unsigned int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<char>::ltr(constant<long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<char>::ltr(constant<unsigned long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<char>::ltr(constant<__int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<char>::ltr(constant<unsigned __int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<char>::ltr(constant<float>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<char>::ltr(constant<double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<char>::ltr(constant<long double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<signed char>::ltr(constant<bool>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<signed char>::ltr(constant<char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<signed char>::ltr(constant<signed char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<signed char>::ltr(constant<unsigned char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<signed char>::ltr(constant<wchar_t>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<signed char>::ltr(constant<short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<signed char>::ltr(constant<unsigned short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<signed char>::ltr(constant<int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<signed char>::ltr(constant<unsigned int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<signed char>::ltr(constant<long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<signed char>::ltr(constant<unsigned long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<signed char>::ltr(constant<__int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<signed char>::ltr(constant<unsigned __int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<signed char>::ltr(constant<float>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<signed char>::ltr(constant<double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<signed char>::ltr(constant<long double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<unsigned char>::ltr(constant<bool>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned char>::ltr(constant<char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned char>::ltr(constant<signed char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned char>::ltr(constant<unsigned char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned char>::ltr(constant<wchar_t>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned char>::ltr(constant<short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned char>::ltr(constant<unsigned short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned char>::ltr(constant<int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned char>::ltr(constant<unsigned int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned char>::ltr(constant<long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned char>::ltr(constant<unsigned long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned char>::ltr(constant<__int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned char>::ltr(constant<unsigned __int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned char>::ltr(constant<float>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<unsigned char>::ltr(constant<double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<unsigned char>::ltr(constant<long double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<wchar_t>::ltr(constant<bool>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<wchar_t>::ltr(constant<char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<wchar_t>::ltr(constant<signed char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<wchar_t>::ltr(constant<unsigned char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<wchar_t>::ltr(constant<wchar_t>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<wchar_t>::ltr(constant<short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<wchar_t>::ltr(constant<unsigned short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<wchar_t>::ltr(constant<int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<wchar_t>::ltr(constant<unsigned int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<wchar_t>::ltr(constant<long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<wchar_t>::ltr(constant<unsigned long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<wchar_t>::ltr(constant<__int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<wchar_t>::ltr(constant<unsigned __int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<wchar_t>::ltr(constant<float>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<wchar_t>::ltr(constant<double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<wchar_t>::ltr(constant<long double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<short int>::ltr(constant<bool>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<short int>::ltr(constant<char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<short int>::ltr(constant<signed char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<short int>::ltr(constant<unsigned char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<short int>::ltr(constant<wchar_t>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<short int>::ltr(constant<short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<short int>::ltr(constant<unsigned short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<short int>::ltr(constant<int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<short int>::ltr(constant<unsigned int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<short int>::ltr(constant<long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<short int>::ltr(constant<unsigned long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<short int>::ltr(constant<__int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<short int>::ltr(constant<unsigned __int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<short int>::ltr(constant<float>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<short int>::ltr(constant<double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<short int>::ltr(constant<long double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<unsigned short int>::ltr(constant<bool>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned short int>::ltr(constant<char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned short int>::ltr(constant<signed char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned short int>::ltr(constant<unsigned char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned short int>::ltr(constant<wchar_t>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned short int>::ltr(constant<short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned short int>::ltr(constant<unsigned short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned short int>::ltr(constant<int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned short int>::ltr(constant<unsigned int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned short int>::ltr(constant<long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned short int>::ltr(constant<unsigned long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned short int>::ltr(constant<__int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned short int>::ltr(constant<unsigned __int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned short int>::ltr(constant<float>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<unsigned short int>::ltr(constant<double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<unsigned short int>::ltr(constant<long double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<int>::ltr(constant<bool>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<int>::ltr(constant<char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<int>::ltr(constant<signed char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<int>::ltr(constant<unsigned char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<int>::ltr(constant<wchar_t>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<int>::ltr(constant<short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<int>::ltr(constant<unsigned short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<int>::ltr(constant<int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<int>::ltr(constant<unsigned int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<int>::ltr(constant<long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<int>::ltr(constant<unsigned long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<int>::ltr(constant<__int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<int>::ltr(constant<unsigned __int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<int>::ltr(constant<float>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<int>::ltr(constant<double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<int>::ltr(constant<long double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<unsigned int>::ltr(constant<bool>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned int>::ltr(constant<char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned int>::ltr(constant<signed char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned int>::ltr(constant<unsigned char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned int>::ltr(constant<wchar_t>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned int>::ltr(constant<short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned int>::ltr(constant<unsigned short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned int>::ltr(constant<int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned int>::ltr(constant<unsigned int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned int>::ltr(constant<long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned int>::ltr(constant<unsigned long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned int>::ltr(constant<__int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned int>::ltr(constant<unsigned __int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned int>::ltr(constant<float>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<unsigned int>::ltr(constant<double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<unsigned int>::ltr(constant<long double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<long int>::ltr(constant<bool>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<long int>::ltr(constant<char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<long int>::ltr(constant<signed char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<long int>::ltr(constant<unsigned char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<long int>::ltr(constant<wchar_t>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<long int>::ltr(constant<short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<long int>::ltr(constant<unsigned short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<long int>::ltr(constant<int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<long int>::ltr(constant<unsigned int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<long int>::ltr(constant<long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<long int>::ltr(constant<unsigned long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<long int>::ltr(constant<__int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<long int>::ltr(constant<unsigned __int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<long int>::ltr(constant<float>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<long int>::ltr(constant<double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<long int>::ltr(constant<long double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<unsigned long int>::ltr(constant<bool>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned long int>::ltr(constant<char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned long int>::ltr(constant<signed char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned long int>::ltr(constant<unsigned char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned long int>::ltr(constant<wchar_t>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned long int>::ltr(constant<short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned long int>::ltr(constant<unsigned short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned long int>::ltr(constant<int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned long int>::ltr(constant<unsigned int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned long int>::ltr(constant<long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned long int>::ltr(constant<unsigned long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned long int>::ltr(constant<__int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned long int>::ltr(constant<unsigned __int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned long int>::ltr(constant<float>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<unsigned long int>::ltr(constant<double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<unsigned long int>::ltr(constant<long double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<__int64>::ltr(constant<bool>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<__int64>::ltr(constant<char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<__int64>::ltr(constant<signed char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<__int64>::ltr(constant<unsigned char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<__int64>::ltr(constant<wchar_t>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<__int64>::ltr(constant<short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<__int64>::ltr(constant<unsigned short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<__int64>::ltr(constant<int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<__int64>::ltr(constant<unsigned int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<__int64>::ltr(constant<long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<__int64>::ltr(constant<unsigned long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<__int64>::ltr(constant<__int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<__int64>::ltr(constant<unsigned __int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<__int64>::ltr(constant<float>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<__int64>::ltr(constant<double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<__int64>::ltr(constant<long double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<unsigned __int64>::ltr(constant<bool>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned __int64>::ltr(constant<char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned __int64>::ltr(constant<signed char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned __int64>::ltr(constant<unsigned char>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned __int64>::ltr(constant<wchar_t>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned __int64>::ltr(constant<short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned __int64>::ltr(constant<unsigned short int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned __int64>::ltr(constant<int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned __int64>::ltr(constant<unsigned int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned __int64>::ltr(constant<long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned __int64>::ltr(constant<unsigned long int>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned __int64>::ltr(constant<__int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned __int64>::ltr(constant<unsigned __int64>* y)
  { return constant_impl::lt(y,this); }
  template<>
  var* constant<unsigned __int64>::ltr(constant<float>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<unsigned __int64>::ltr(constant<double>* y)
  { return constant_impl::flt1(y,this); }
  template<>
  var* constant<unsigned __int64>::ltr(constant<long double>* y)
  { return constant_impl::flt1(y,this); }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<float>::lt(var* y){ return y->ltr(this); }
cxx_compiler::var* cxx_compiler::constant<float>::ltr(constant<bool>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ltr(constant<char>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ltr(constant<signed char>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ltr(constant<unsigned char>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ltr(constant<wchar_t>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ltr(constant<short int>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ltr(constant<unsigned short int>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ltr(constant<int>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ltr(constant<unsigned int>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ltr(constant<long int>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ltr(constant<unsigned long int>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ltr(constant<__int64>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ltr(constant<unsigned __int64>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ltr(constant<float>* y)
{ return constant_impl::flt3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ltr(constant<double>* y)
{ return constant_impl::flt3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ltr(constant<long double>* y)
{ return constant_impl::flt3(y,this); }

cxx_compiler::var* cxx_compiler::constant<double>::lt(var* y){ return y->ltr(this); }
cxx_compiler::var* cxx_compiler::constant<double>::ltr(constant<bool>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ltr(constant<char>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ltr(constant<signed char>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ltr(constant<unsigned char>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ltr(constant<wchar_t>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ltr(constant<short int>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ltr(constant<unsigned short int>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ltr(constant<int>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ltr(constant<unsigned int>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ltr(constant<long int>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ltr(constant<unsigned long int>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ltr(constant<__int64>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ltr(constant<unsigned __int64>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ltr(constant<float>* y)
{ return constant_impl::flt3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ltr(constant<double>* y)
{ return constant_impl::flt3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ltr(constant<long double>* y)
{ return constant_impl::flt3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::lt(var* y){ return y->ltr(this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ltr(constant<bool>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ltr(constant<char>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ltr(constant<signed char>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ltr(constant<unsigned char>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ltr(constant<wchar_t>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ltr(constant<short int>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ltr(constant<unsigned short int>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ltr(constant<int>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ltr(constant<unsigned int>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ltr(constant<long int>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ltr(constant<unsigned long int>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ltr(constant<__int64>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ltr(constant<unsigned __int64>* y)
{ return constant_impl::flt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ltr(constant<float>* y)
{ return constant_impl::flt3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ltr(constant<double>* y)
{ return constant_impl::flt3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ltr(constant<long double>* y)
{ return constant_impl::flt3(y,this); }

namespace cxx_compiler { namespace constant_impl {
  template<class A, class B> var* pcmp(goto3ac::op op, constant<A>* a, constant<B>* b)
  {
    using namespace expressions::primary::literal;
    if (expressions::cmp_impl::valid_pointer(op,a,b)){
      void* x = (void*)a->m_value;
      void* y = (void*)b->m_value;
      switch ( op ){
      case goto3ac::LT: return integer::create(x <  y);
      case goto3ac::GT: return integer::create(x >  y);
      case goto3ac::LE: return integer::create(x <= y);
      case goto3ac::GE: return integer::create(x >= y);
      case goto3ac::EQ: return integer::create(x == y);
      case goto3ac::NE: return integer::create(x != y);
      }
    }
    return expressions::cmp_impl::gen(op,a,b);
  }
} } // end of namespace constant_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::constant<void*>::lt(var* z){ return z->ltr(this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ltr(constant<void*>* y){ return constant_impl::pcmp(goto3ac::LT,y,this); }

namespace cxx_compiler { namespace addrof_impl {
  var* pcmp(goto3ac::op, addrof*, addrof*);
} } // end of namespace addrof_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::addrof_impl::pcmp(goto3ac::op op, addrof* a, addrof* b)
{
  using namespace expressions::primary::literal;
  if ( a->m_ref == b->m_ref ){
    int x = a->m_offset;
    int y = b->m_offset;
    switch ( op ){
    case goto3ac::LT: return integer::create(x <  y);
    case goto3ac::GT: return integer::create(x >  y);
    case goto3ac::LE: return integer::create(x <= y);
    case goto3ac::GE: return integer::create(x >= y);
    case goto3ac::EQ: return integer::create(x == y);
    case goto3ac::NE: return integer::create(x != y);
    }
  }
  return expressions::cmp_impl::gen(op,a,b);
}

cxx_compiler::var* cxx_compiler::addrof::lt(var* z){ return z->ltr(this); }
cxx_compiler::var* cxx_compiler::addrof::ltr(addrof* y){ return addrof_impl::pcmp(goto3ac::LT,y,this); }

namespace cxx_compiler { namespace constant_impl {
  template<class A, class B> var* gt(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if (expressions::cmp_impl::valid_pointer(goto3ac::GT, y, z)) {
      constant<__int64>* yy = reinterpret_cast<constant<__int64>*>(y);
      constant<__int64>* zz = reinterpret_cast<constant<__int64>*>(z);
      return integer::create(yy->m_value > zz->m_value); 
    }
    usr::flag_t fy = y->m_flag;
    if (fy & usr::CONST_PTR)
      return expressions::cmp_impl::gen(goto3ac::GT, y,z);
    usr::flag_t fz = z->m_flag;
    if (fz & usr::CONST_PTR)
      return expressions::cmp_impl::gen(goto3ac::GT, y,z);
    return integer::create(y->m_value > z->m_value); 
  }
  template<class A, class B> var* fgt1(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if ( generator::long_double ){
      if ( var* v = fop1(y,z,goto3ac::GT) )
        return v;
    }
#ifndef _MSC_VER
    return integer::create(y->m_value > z->m_value);
#else // _MSC_VER
    return integer::create(y->m_value > (__int64)z->m_value);
#endif // _MSC_VER
  }
  template<class A, class B> var* fgt2(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if ( generator::long_double ){
      if ( var* v = fop2(y,z,goto3ac::GT) )
              return v;
    }
#ifndef _MSC_VER
    return integer::create(y->m_value > z->m_value);
#else // _MSC_VER
    return integer::create((__int64)y->m_value > z->m_value);
#endif // _MSC_VER
  }
  template<class A, class B> var* fgt3(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if ( generator::long_double ){
      if ( var* v = fop3(y,z,goto3ac::GT) )
        return v;
    }
    return integer::create(y->m_value > z->m_value);
  }
} } // end of namespace constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::gtr(constant<bool>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<bool>::gtr(constant<char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<bool>::gtr(constant<signed char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<bool>::gtr(constant<unsigned char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<bool>::gtr(constant<wchar_t>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<bool>::gtr(constant<short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<bool>::gtr(constant<unsigned short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<bool>::gtr(constant<int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<bool>::gtr(constant<unsigned int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<bool>::gtr(constant<long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<bool>::gtr(constant<unsigned long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<bool>::gtr(constant<__int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<bool>::gtr(constant<unsigned __int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<bool>::gtr(constant<float>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<bool>::gtr(constant<double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<bool>::gtr(constant<long double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<char>::gtr(constant<bool>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<char>::gtr(constant<char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<char>::gtr(constant<signed char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<char>::gtr(constant<unsigned char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<char>::gtr(constant<wchar_t>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<char>::gtr(constant<short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<char>::gtr(constant<unsigned short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<char>::gtr(constant<int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<char>::gtr(constant<unsigned int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<char>::gtr(constant<long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<char>::gtr(constant<unsigned long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<char>::gtr(constant<__int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<char>::gtr(constant<unsigned __int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<char>::gtr(constant<float>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<char>::gtr(constant<double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<char>::gtr(constant<long double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<signed char>::gtr(constant<bool>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<signed char>::gtr(constant<char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<signed char>::gtr(constant<signed char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<signed char>::gtr(constant<unsigned char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<signed char>::gtr(constant<wchar_t>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<signed char>::gtr(constant<short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<signed char>::gtr(constant<unsigned short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<signed char>::gtr(constant<int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<signed char>::gtr(constant<unsigned int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<signed char>::gtr(constant<long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<signed char>::gtr(constant<unsigned long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<signed char>::gtr(constant<__int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<signed char>::gtr(constant<unsigned __int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<signed char>::gtr(constant<float>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<signed char>::gtr(constant<double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<signed char>::gtr(constant<long double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<unsigned char>::gtr(constant<bool>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned char>::gtr(constant<char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned char>::gtr(constant<signed char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned char>::gtr(constant<unsigned char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned char>::gtr(constant<wchar_t>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned char>::gtr(constant<short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned char>::gtr(constant<unsigned short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned char>::gtr(constant<int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned char>::gtr(constant<unsigned int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned char>::gtr(constant<long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned char>::gtr(constant<unsigned long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned char>::gtr(constant<__int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned char>::gtr(constant<unsigned __int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned char>::gtr(constant<float>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<unsigned char>::gtr(constant<double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<unsigned char>::gtr(constant<long double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<wchar_t>::gtr(constant<bool>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<wchar_t>::gtr(constant<char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<wchar_t>::gtr(constant<signed char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<wchar_t>::gtr(constant<unsigned char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<wchar_t>::gtr(constant<wchar_t>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<wchar_t>::gtr(constant<short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<wchar_t>::gtr(constant<unsigned short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<wchar_t>::gtr(constant<int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<wchar_t>::gtr(constant<unsigned int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<wchar_t>::gtr(constant<long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<wchar_t>::gtr(constant<unsigned long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<wchar_t>::gtr(constant<__int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<wchar_t>::gtr(constant<unsigned __int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<wchar_t>::gtr(constant<float>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<wchar_t>::gtr(constant<double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<wchar_t>::gtr(constant<long double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<short int>::gtr(constant<bool>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<short int>::gtr(constant<char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<short int>::gtr(constant<signed char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<short int>::gtr(constant<unsigned char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<short int>::gtr(constant<wchar_t>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<short int>::gtr(constant<short int>* y)
  { return constant_impl::gt(y,this); }
  
  template<>
  var* constant<short int>::gtr(constant<unsigned short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<short int>::gtr(constant<int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<short int>::gtr(constant<unsigned int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<short int>::gtr(constant<long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<short int>::gtr(constant<unsigned long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<short int>::gtr(constant<__int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<short int>::gtr(constant<unsigned __int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<short int>::gtr(constant<float>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<short int>::gtr(constant<double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<short int>::gtr(constant<long double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<unsigned short int>::gtr(constant<bool>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned short int>::gtr(constant<char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned short int>::gtr(constant<signed char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned short int>::gtr(constant<unsigned char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned short int>::gtr(constant<wchar_t>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned short int>::gtr(constant<short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned short int>::gtr(constant<unsigned short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned short int>::gtr(constant<int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned short int>::gtr(constant<unsigned int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned short int>::gtr(constant<long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned short int>::gtr(constant<unsigned long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned short int>::gtr(constant<__int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned short int>::gtr(constant<unsigned __int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned short int>::gtr(constant<float>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<unsigned short int>::gtr(constant<double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<unsigned short int>::gtr(constant<long double>* y)
  { return constant_impl::fgt1(y,this); }
  
  template<>
  var* constant<int>::gtr(constant<bool>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<int>::gtr(constant<char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<int>::gtr(constant<signed char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<int>::gtr(constant<unsigned char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<int>::gtr(constant<wchar_t>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<int>::gtr(constant<short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<int>::gtr(constant<unsigned short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<int>::gtr(constant<int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<int>::gtr(constant<unsigned int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<int>::gtr(constant<long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<int>::gtr(constant<unsigned long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<int>::gtr(constant<__int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<int>::gtr(constant<unsigned __int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<int>::gtr(constant<float>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<int>::gtr(constant<double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<int>::gtr(constant<long double>* y)
  { return constant_impl::fgt1(y,this); }
  
  template<>
  var* constant<unsigned int>::gtr(constant<bool>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned int>::gtr(constant<char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned int>::gtr(constant<signed char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned int>::gtr(constant<unsigned char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned int>::gtr(constant<wchar_t>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned int>::gtr(constant<short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned int>::gtr(constant<unsigned short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned int>::gtr(constant<int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned int>::gtr(constant<unsigned int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned int>::gtr(constant<long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned int>::gtr(constant<unsigned long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned int>::gtr(constant<__int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned int>::gtr(constant<unsigned __int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned int>::gtr(constant<float>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<unsigned int>::gtr(constant<double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<unsigned int>::gtr(constant<long double>* y)
  { return constant_impl::fgt1(y,this); }
  
  template<>
  var* constant<long int>::gtr(constant<bool>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<long int>::gtr(constant<char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<long int>::gtr(constant<signed char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<long int>::gtr(constant<unsigned char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<long int>::gtr(constant<wchar_t>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<long int>::gtr(constant<short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<long int>::gtr(constant<unsigned short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<long int>::gtr(constant<int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<long int>::gtr(constant<unsigned int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<long int>::gtr(constant<long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<long int>::gtr(constant<unsigned long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<long int>::gtr(constant<__int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<long int>::gtr(constant<unsigned __int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<long int>::gtr(constant<float>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<long int>::gtr(constant<double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<long int>::gtr(constant<long double>* y)
  { return constant_impl::fgt1(y,this); }
  
  template<>
  var* constant<unsigned long int>::gtr(constant<bool>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned long int>::gtr(constant<char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned long int>::gtr(constant<signed char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned long int>::gtr(constant<unsigned char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned long int>::gtr(constant<wchar_t>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned long int>::gtr(constant<short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned long int>::gtr(constant<unsigned short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned long int>::gtr(constant<int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned long int>::gtr(constant<unsigned int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned long int>::gtr(constant<long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned long int>::gtr(constant<unsigned long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned long int>::gtr(constant<__int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned long int>::gtr(constant<unsigned __int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned long int>::gtr(constant<float>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<unsigned long int>::gtr(constant<double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<unsigned long int>::gtr(constant<long double>* y)
  { return constant_impl::fgt1(y,this); }
  
  template<>
  var* constant<__int64>::gtr(constant<bool>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<__int64>::gtr(constant<char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<__int64>::gtr(constant<signed char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<__int64>::gtr(constant<unsigned char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<__int64>::gtr(constant<wchar_t>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<__int64>::gtr(constant<short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<__int64>::gtr(constant<unsigned short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<__int64>::gtr(constant<int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<__int64>::gtr(constant<unsigned int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<__int64>::gtr(constant<long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<__int64>::gtr(constant<unsigned long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<__int64>::gtr(constant<__int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<__int64>::gtr(constant<unsigned __int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<__int64>::gtr(constant<float>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<__int64>::gtr(constant<double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<__int64>::gtr(constant<long double>* y)
  { return constant_impl::fgt1(y,this); }
  
  template<>
  var* constant<unsigned __int64>::gtr(constant<bool>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned __int64>::gtr(constant<char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned __int64>::gtr(constant<signed char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned __int64>::gtr(constant<unsigned char>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned __int64>::gtr(constant<wchar_t>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned __int64>::gtr(constant<short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned __int64>::gtr(constant<unsigned short int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned __int64>::gtr(constant<int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned __int64>::gtr(constant<unsigned int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned __int64>::gtr(constant<long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned __int64>::gtr(constant<unsigned long int>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned __int64>::gtr(constant<__int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned __int64>::gtr(constant<unsigned __int64>* y)
  { return constant_impl::gt(y,this); }
  template<>
  var* constant<unsigned __int64>::gtr(constant<float>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<unsigned __int64>::gtr(constant<double>* y)
  { return constant_impl::fgt1(y,this); }
  template<>
  var* constant<unsigned __int64>::gtr(constant<long double>* y)
  { return constant_impl::fgt1(y,this); }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<float>::gt(var* y){ return y->gtr(this); }
cxx_compiler::var* cxx_compiler::constant<float>::gtr(constant<bool>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::gtr(constant<char>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::gtr(constant<signed char>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::gtr(constant<unsigned char>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::gtr(constant<wchar_t>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::gtr(constant<short int>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::gtr(constant<unsigned short int>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::gtr(constant<int>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::gtr(constant<unsigned int>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::gtr(constant<long int>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::gtr(constant<unsigned long int>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::gtr(constant<__int64>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::gtr(constant<unsigned __int64>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::gtr(constant<float>* y)
{ return constant_impl::fgt3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::gtr(constant<double>* y)
{ return constant_impl::fgt3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::gtr(constant<long double>* y)
{ return constant_impl::fgt3(y,this); }

cxx_compiler::var* cxx_compiler::constant<double>::gt(var* y){ return y->gtr(this); }
cxx_compiler::var* cxx_compiler::constant<double>::gtr(constant<bool>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::gtr(constant<char>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::gtr(constant<signed char>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::gtr(constant<unsigned char>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::gtr(constant<wchar_t>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::gtr(constant<short int>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::gtr(constant<unsigned short int>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::gtr(constant<int>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::gtr(constant<unsigned int>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::gtr(constant<long int>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::gtr(constant<unsigned long int>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::gtr(constant<__int64>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::gtr(constant<unsigned __int64>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::gtr(constant<float>* y)
{ return constant_impl::fgt3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::gtr(constant<double>* y)
{ return constant_impl::fgt3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::gtr(constant<long double>* y)
{ return constant_impl::fgt3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::gt(var* z)
{ return z->gtr(this); }
cxx_compiler::var* cxx_compiler::constant<long double>::gtr(constant<bool>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::gtr(constant<char>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::gtr(constant<signed char>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::gtr(constant<unsigned char>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::gtr(constant<wchar_t>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::gtr(constant<short int>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::gtr(constant<unsigned short int>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::gtr(constant<int>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::gtr(constant<unsigned int>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::gtr(constant<long int>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::gtr(constant<unsigned long int>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::gtr(constant<__int64>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::gtr(constant<unsigned __int64>* y)
{ return constant_impl::fgt2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::gtr(constant<float>* y)
{ return constant_impl::fgt3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::gtr(constant<double>* y)
{ return constant_impl::fgt3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::gtr(constant<long double>* y)
{ return constant_impl::fgt3(y,this); }

cxx_compiler::var* cxx_compiler::constant<void*>::gt(var* z){ return z->gtr(this); }
cxx_compiler::var* cxx_compiler::constant<void*>::gtr(constant<void*>* y){ return constant_impl::pcmp(goto3ac::GT,y,this); }

cxx_compiler::var* cxx_compiler::addrof::gt(var* z){ return z->gtr(this); }
cxx_compiler::var* cxx_compiler::addrof::gtr(addrof* y){ return addrof_impl::pcmp(goto3ac::GT,y,this); }

namespace cxx_compiler { namespace constant_impl {
  template<class A, class B> var* le(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if (expressions::cmp_impl::valid_pointer(goto3ac::LE, y, z)) {
      constant<__int64>* yy = reinterpret_cast<constant<__int64>*>(y);
      constant<__int64>* zz = reinterpret_cast<constant<__int64>*>(z);
      return integer::create(yy->m_value <= zz->m_value); 
    }
    usr::flag_t fy = y->m_flag;
    if (fy & usr::CONST_PTR)
      return expressions::cmp_impl::gen(goto3ac::LE, y,z);
    usr::flag_t fz = z->m_flag;
    if (fz & usr::CONST_PTR)
      return expressions::cmp_impl::gen(goto3ac::LE, y,z);
    return integer::create(y->m_value <= z->m_value); 
  }
  template<class A, class B> var* fle1(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if ( generator::long_double ){
      if ( var* v = fop1(y,z,goto3ac::LE) )
        return v;
    }
#ifndef _MSC_VER
    return integer::create(y->m_value <= z->m_value);
#else // _MSC_VER
    return integer::create(y->m_value <= (__int64)z->m_value);
#endif // _MSC_VER
  }
  template<class A, class B> var* fle2(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if ( generator::long_double ){
      if ( var* v = fop2(y,z,goto3ac::LE) )
        return v;
    }
#ifndef _MSC_VER
    return integer::create(y->m_value <= z->m_value);
#else // _MSC_VER
    return integer::create((__int64)y->m_value <= z->m_value);
#endif // _MSC_VER
  }
  template<class A, class B> var* fle3(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if ( generator::long_double ){
      if ( var* v = fop3(y,z,goto3ac::LE) )
        return v;
    }
    return integer::create(y->m_value <= z->m_value);
  }
} } // end of namespace constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::ler(constant<bool>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<bool>::ler(constant<char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<bool>::ler(constant<signed char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<bool>::ler(constant<unsigned char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<bool>::ler(constant<wchar_t>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<bool>::ler(constant<short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<bool>::ler(constant<unsigned short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<bool>::ler(constant<int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<bool>::ler(constant<unsigned int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<bool>::ler(constant<long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<bool>::ler(constant<unsigned long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<bool>::ler(constant<__int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<bool>::ler(constant<unsigned __int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<bool>::ler(constant<float>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<bool>::ler(constant<double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<bool>::ler(constant<long double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<char>::ler(constant<bool>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<char>::ler(constant<char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<char>::ler(constant<signed char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<char>::ler(constant<unsigned char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<char>::ler(constant<wchar_t>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<char>::ler(constant<short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<char>::ler(constant<unsigned short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<char>::ler(constant<int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<char>::ler(constant<unsigned int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<char>::ler(constant<long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<char>::ler(constant<unsigned long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<char>::ler(constant<__int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<char>::ler(constant<unsigned __int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<char>::ler(constant<float>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<char>::ler(constant<double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<char>::ler(constant<long double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<signed char>::ler(constant<bool>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<signed char>::ler(constant<char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<signed char>::ler(constant<signed char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<signed char>::ler(constant<unsigned char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<signed char>::ler(constant<wchar_t>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<signed char>::ler(constant<short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<signed char>::ler(constant<unsigned short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<signed char>::ler(constant<int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<signed char>::ler(constant<unsigned int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<signed char>::ler(constant<long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<signed char>::ler(constant<unsigned long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<signed char>::ler(constant<__int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<signed char>::ler(constant<unsigned __int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<signed char>::ler(constant<float>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<signed char>::ler(constant<double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<signed char>::ler(constant<long double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<unsigned char>::ler(constant<bool>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned char>::ler(constant<char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned char>::ler(constant<signed char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned char>::ler(constant<unsigned char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned char>::ler(constant<wchar_t>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned char>::ler(constant<short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned char>::ler(constant<unsigned short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned char>::ler(constant<int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned char>::ler(constant<unsigned int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned char>::ler(constant<long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned char>::ler(constant<unsigned long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned char>::ler(constant<__int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned char>::ler(constant<unsigned __int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned char>::ler(constant<float>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<unsigned char>::ler(constant<double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<unsigned char>::ler(constant<long double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<wchar_t>::ler(constant<bool>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<wchar_t>::ler(constant<char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<wchar_t>::ler(constant<signed char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<wchar_t>::ler(constant<unsigned char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<wchar_t>::ler(constant<wchar_t>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<wchar_t>::ler(constant<short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<wchar_t>::ler(constant<unsigned short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<wchar_t>::ler(constant<int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<wchar_t>::ler(constant<unsigned int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<wchar_t>::ler(constant<long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<wchar_t>::ler(constant<unsigned long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<wchar_t>::ler(constant<__int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<wchar_t>::ler(constant<unsigned __int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<wchar_t>::ler(constant<float>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<wchar_t>::ler(constant<double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<wchar_t>::ler(constant<long double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<short int>::ler(constant<bool>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<short int>::ler(constant<char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<short int>::ler(constant<signed char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<short int>::ler(constant<unsigned char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<short int>::ler(constant<wchar_t>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<short int>::ler(constant<short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<short int>::ler(constant<unsigned short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<short int>::ler(constant<int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<short int>::ler(constant<unsigned int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<short int>::ler(constant<long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<short int>::ler(constant<unsigned long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<short int>::ler(constant<__int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<short int>::ler(constant<unsigned __int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<short int>::ler(constant<float>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<short int>::ler(constant<double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<short int>::ler(constant<long double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<unsigned short int>::ler(constant<bool>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned short int>::ler(constant<char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned short int>::ler(constant<signed char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned short int>::ler(constant<unsigned char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned short int>::ler(constant<wchar_t>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned short int>::ler(constant<short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned short int>::ler(constant<unsigned short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned short int>::ler(constant<int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned short int>::ler(constant<unsigned int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned short int>::ler(constant<long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned short int>::ler(constant<unsigned long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned short int>::ler(constant<__int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned short int>::ler(constant<unsigned __int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned short int>::ler(constant<float>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<unsigned short int>::ler(constant<double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<unsigned short int>::ler(constant<long double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<int>::ler(constant<bool>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<int>::ler(constant<char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<int>::ler(constant<signed char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<int>::ler(constant<unsigned char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<int>::ler(constant<wchar_t>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<int>::ler(constant<short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<int>::ler(constant<unsigned short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<int>::ler(constant<int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<int>::ler(constant<unsigned int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<int>::ler(constant<long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<int>::ler(constant<unsigned long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<int>::ler(constant<__int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<int>::ler(constant<unsigned __int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<int>::ler(constant<float>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<int>::ler(constant<double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<int>::ler(constant<long double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<unsigned int>::ler(constant<bool>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned int>::ler(constant<char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned int>::ler(constant<signed char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned int>::ler(constant<unsigned char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned int>::ler(constant<wchar_t>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned int>::ler(constant<short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned int>::ler(constant<unsigned short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned int>::ler(constant<int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned int>::ler(constant<unsigned int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned int>::ler(constant<long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned int>::ler(constant<unsigned long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned int>::ler(constant<__int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned int>::ler(constant<unsigned __int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned int>::ler(constant<float>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<unsigned int>::ler(constant<double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<unsigned int>::ler(constant<long double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<long int>::ler(constant<bool>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<long int>::ler(constant<char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<long int>::ler(constant<signed char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<long int>::ler(constant<unsigned char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<long int>::ler(constant<wchar_t>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<long int>::ler(constant<short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<long int>::ler(constant<unsigned short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<long int>::ler(constant<int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<long int>::ler(constant<unsigned int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<long int>::ler(constant<long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<long int>::ler(constant<unsigned long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<long int>::ler(constant<__int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<long int>::ler(constant<unsigned __int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<long int>::ler(constant<float>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<long int>::ler(constant<double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<long int>::ler(constant<long double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<unsigned long int>::ler(constant<bool>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned long int>::ler(constant<char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned long int>::ler(constant<signed char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned long int>::ler(constant<unsigned char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned long int>::ler(constant<wchar_t>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned long int>::ler(constant<short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned long int>::ler(constant<unsigned short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned long int>::ler(constant<int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned long int>::ler(constant<unsigned int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned long int>::ler(constant<long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned long int>::ler(constant<unsigned long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned long int>::ler(constant<__int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned long int>::ler(constant<unsigned __int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned long int>::ler(constant<float>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<unsigned long int>::ler(constant<double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<unsigned long int>::ler(constant<long double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<__int64>::ler(constant<bool>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<__int64>::ler(constant<char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<__int64>::ler(constant<signed char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<__int64>::ler(constant<unsigned char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<__int64>::ler(constant<wchar_t>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<__int64>::ler(constant<short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<__int64>::ler(constant<unsigned short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<__int64>::ler(constant<int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<__int64>::ler(constant<unsigned int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<__int64>::ler(constant<long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<__int64>::ler(constant<unsigned long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<__int64>::ler(constant<__int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<__int64>::ler(constant<unsigned __int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<__int64>::ler(constant<float>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<__int64>::ler(constant<double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<__int64>::ler(constant<long double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<unsigned __int64>::ler(constant<bool>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned __int64>::ler(constant<char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned __int64>::ler(constant<signed char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned __int64>::ler(constant<unsigned char>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned __int64>::ler(constant<wchar_t>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned __int64>::ler(constant<short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned __int64>::ler(constant<unsigned short int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned __int64>::ler(constant<int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned __int64>::ler(constant<unsigned int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned __int64>::ler(constant<long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned __int64>::ler(constant<unsigned long int>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned __int64>::ler(constant<__int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned __int64>::ler(constant<unsigned __int64>* y)
  { return constant_impl::le(y,this); }
  template<>
  var* constant<unsigned __int64>::ler(constant<float>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<unsigned __int64>::ler(constant<double>* y)
  { return constant_impl::fle1(y,this); }
  template<>
  var* constant<unsigned __int64>::ler(constant<long double>* y)
  { return constant_impl::fle1(y,this); }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<float>::le(var* y){ return y->ler(this); }
cxx_compiler::var* cxx_compiler::constant<float>::ler(constant<bool>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ler(constant<char>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ler(constant<signed char>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ler(constant<unsigned char>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ler(constant<wchar_t>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ler(constant<short int>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ler(constant<unsigned short int>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ler(constant<int>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ler(constant<unsigned int>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ler(constant<long int>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ler(constant<unsigned long int>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ler(constant<__int64>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ler(constant<unsigned __int64>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ler(constant<float>* y)
{ return constant_impl::fle3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ler(constant<double>* y)
{ return constant_impl::fle3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ler(constant<long double>* y)
{ return constant_impl::fle3(y,this); }

cxx_compiler::var* cxx_compiler::constant<double>::le(var* y){ return y->ler(this); }
cxx_compiler::var* cxx_compiler::constant<double>::ler(constant<bool>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ler(constant<char>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ler(constant<signed char>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ler(constant<unsigned char>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ler(constant<wchar_t>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ler(constant<short int>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ler(constant<unsigned short int>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ler(constant<int>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ler(constant<unsigned int>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ler(constant<long int>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ler(constant<unsigned long int>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ler(constant<__int64>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ler(constant<unsigned __int64>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ler(constant<float>* y)
{ return constant_impl::fle3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ler(constant<double>* y)
{ return constant_impl::fle3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ler(constant<long double>* y)
{ return constant_impl::fle3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::le(var* y){ return y->ler(this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ler(constant<bool>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ler(constant<char>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ler(constant<signed char>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ler(constant<unsigned char>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ler(constant<wchar_t>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ler(constant<short int>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ler(constant<unsigned short int>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ler(constant<int>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ler(constant<unsigned int>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ler(constant<long int>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ler(constant<unsigned long int>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ler(constant<__int64>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ler(constant<unsigned __int64>* y)
{ return constant_impl::fle2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ler(constant<float>* y)
{ return constant_impl::fle3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ler(constant<double>* y)
{ return constant_impl::fle3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ler(constant<long double>* y)
{ return constant_impl::fle3(y,this); }

cxx_compiler::var* cxx_compiler::constant<void*>::le(var* z){ return z->ler(this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ler(constant<void*>* y){ return constant_impl::pcmp(goto3ac::LE,y,this); }

cxx_compiler::var* cxx_compiler::addrof::le(var* z){ return z->ler(this); }
cxx_compiler::var* cxx_compiler::addrof::ler(addrof* y){ return addrof_impl::pcmp(goto3ac::LE,y,this); }

namespace cxx_compiler { namespace constant_impl {
  template<class A, class B> var* ge(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if (expressions::cmp_impl::valid_pointer(goto3ac::GE, y, z)) {
      constant<__int64>* yy = reinterpret_cast<constant<__int64>*>(y);
      constant<__int64>* zz = reinterpret_cast<constant<__int64>*>(z);
      return integer::create(yy->m_value >= zz->m_value); 
    }
    usr::flag_t fy = y->m_flag;
    if (fy & usr::CONST_PTR)
      return expressions::cmp_impl::gen(goto3ac::GE, y,z);
    usr::flag_t fz = z->m_flag;
    if (fz & usr::CONST_PTR)
      return expressions::cmp_impl::gen(goto3ac::GE, y,z);
    return integer::create(y->m_value >= z->m_value);
  }
  template<class A, class B> var* fge1(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if ( generator::long_double ){
      if ( var* v = fop1(y,z,goto3ac::GE) )
        return v;
    }
#ifndef _MSC_VER
    return integer::create(y->m_value >= z->m_value);
#else // _MSC_VER
    return integer::create(y->m_value >= (__int64)z->m_value);
#endif // _MSC_VER
  }
  template<class A, class B> var* fge2(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if ( generator::long_double ){
      if ( var* v = fop2(y,z,goto3ac::GE) )
        return v;
    }
#ifndef _MSC_VER
    return integer::create(y->m_value >= z->m_value);
#else // _MSC_VER
    return integer::create((__int64)y->m_value >= z->m_value);
#endif // _MSC_VER
  }
  template<class A, class B> var* fge3(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if ( generator::long_double ){
      if ( var* v = fop3(y,z,goto3ac::GE) )
        return v;
    }
#ifndef _MSC_VER
    return integer::create(y->m_value >= z->m_value);
#else // _MSC_VER
    return integer::create((__int64)y->m_value >= z->m_value);
#endif // _MSC_VER
  }
} } // end of namespace constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::ger(constant<bool>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<bool>::ger(constant<char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<bool>::ger(constant<signed char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<bool>::ger(constant<unsigned char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<bool>::ger(constant<wchar_t>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<bool>::ger(constant<short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<bool>::ger(constant<unsigned short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<bool>::ger(constant<int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<bool>::ger(constant<unsigned int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<bool>::ger(constant<long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<bool>::ger(constant<unsigned long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<bool>::ger(constant<__int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<bool>::ger(constant<unsigned __int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<bool>::ger(constant<float>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<bool>::ger(constant<double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<bool>::ger(constant<long double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<char>::ger(constant<bool>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<char>::ger(constant<char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<char>::ger(constant<signed char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<char>::ger(constant<unsigned char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<char>::ger(constant<wchar_t>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<char>::ger(constant<short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<char>::ger(constant<unsigned short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<char>::ger(constant<int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<char>::ger(constant<unsigned int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<char>::ger(constant<long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<char>::ger(constant<unsigned long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<char>::ger(constant<__int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<char>::ger(constant<unsigned __int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<char>::ger(constant<float>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<char>::ger(constant<double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<char>::ger(constant<long double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<signed char>::ger(constant<bool>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<signed char>::ger(constant<char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<signed char>::ger(constant<signed char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<signed char>::ger(constant<unsigned char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<signed char>::ger(constant<wchar_t>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<signed char>::ger(constant<short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<signed char>::ger(constant<unsigned short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<signed char>::ger(constant<int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<signed char>::ger(constant<unsigned int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<signed char>::ger(constant<long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<signed char>::ger(constant<unsigned long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<signed char>::ger(constant<__int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<signed char>::ger(constant<unsigned __int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<signed char>::ger(constant<float>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<signed char>::ger(constant<double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<signed char>::ger(constant<long double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<unsigned char>::ger(constant<bool>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned char>::ger(constant<char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned char>::ger(constant<signed char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned char>::ger(constant<unsigned char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned char>::ger(constant<wchar_t>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned char>::ger(constant<short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned char>::ger(constant<unsigned short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned char>::ger(constant<int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned char>::ger(constant<unsigned int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned char>::ger(constant<long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned char>::ger(constant<unsigned long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned char>::ger(constant<__int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned char>::ger(constant<unsigned __int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned char>::ger(constant<float>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<unsigned char>::ger(constant<double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<unsigned char>::ger(constant<long double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<wchar_t>::ger(constant<bool>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<wchar_t>::ger(constant<char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<wchar_t>::ger(constant<signed char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<wchar_t>::ger(constant<unsigned char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<wchar_t>::ger(constant<wchar_t>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<wchar_t>::ger(constant<short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<wchar_t>::ger(constant<unsigned short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<wchar_t>::ger(constant<int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<wchar_t>::ger(constant<unsigned int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<wchar_t>::ger(constant<long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<wchar_t>::ger(constant<unsigned long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<wchar_t>::ger(constant<__int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<wchar_t>::ger(constant<unsigned __int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<wchar_t>::ger(constant<float>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<wchar_t>::ger(constant<double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<wchar_t>::ger(constant<long double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<short int>::ger(constant<bool>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<short int>::ger(constant<char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<short int>::ger(constant<signed char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<short int>::ger(constant<unsigned char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<short int>::ger(constant<wchar_t>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<short int>::ger(constant<short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<short int>::ger(constant<unsigned short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<short int>::ger(constant<int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<short int>::ger(constant<unsigned int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<short int>::ger(constant<long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<short int>::ger(constant<unsigned long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<short int>::ger(constant<__int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<short int>::ger(constant<unsigned __int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<short int>::ger(constant<float>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<short int>::ger(constant<double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<short int>::ger(constant<long double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<unsigned short int>::ger(constant<bool>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned short int>::ger(constant<char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned short int>::ger(constant<signed char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned short int>::ger(constant<unsigned char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned short int>::ger(constant<wchar_t>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned short int>::ger(constant<short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned short int>::ger(constant<unsigned short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned short int>::ger(constant<int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned short int>::ger(constant<unsigned int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned short int>::ger(constant<long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned short int>::ger(constant<unsigned long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned short int>::ger(constant<__int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned short int>::ger(constant<unsigned __int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned short int>::ger(constant<float>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<unsigned short int>::ger(constant<double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<unsigned short int>::ger(constant<long double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<int>::ger(constant<bool>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<int>::ger(constant<char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<int>::ger(constant<signed char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<int>::ger(constant<unsigned char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<int>::ger(constant<wchar_t>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<int>::ger(constant<short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<int>::ger(constant<unsigned short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<int>::ger(constant<int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<int>::ger(constant<unsigned int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<int>::ger(constant<long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<int>::ger(constant<unsigned long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<int>::ger(constant<__int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<int>::ger(constant<unsigned __int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<int>::ger(constant<float>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<int>::ger(constant<double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<int>::ger(constant<long double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<unsigned int>::ger(constant<bool>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned int>::ger(constant<char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned int>::ger(constant<signed char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned int>::ger(constant<unsigned char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned int>::ger(constant<wchar_t>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned int>::ger(constant<short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned int>::ger(constant<unsigned short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned int>::ger(constant<int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned int>::ger(constant<unsigned int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned int>::ger(constant<long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned int>::ger(constant<unsigned long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned int>::ger(constant<__int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned int>::ger(constant<unsigned __int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned int>::ger(constant<float>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<unsigned int>::ger(constant<double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<unsigned int>::ger(constant<long double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<long int>::ger(constant<bool>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<long int>::ger(constant<char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<long int>::ger(constant<signed char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<long int>::ger(constant<unsigned char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<long int>::ger(constant<wchar_t>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<long int>::ger(constant<short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<long int>::ger(constant<unsigned short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<long int>::ger(constant<int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<long int>::ger(constant<unsigned int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<long int>::ger(constant<long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<long int>::ger(constant<unsigned long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<long int>::ger(constant<__int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<long int>::ger(constant<unsigned __int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<long int>::ger(constant<float>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<long int>::ger(constant<double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<long int>::ger(constant<long double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<unsigned long int>::ger(constant<bool>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned long int>::ger(constant<char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned long int>::ger(constant<signed char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned long int>::ger(constant<unsigned char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned long int>::ger(constant<wchar_t>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned long int>::ger(constant<short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned long int>::ger(constant<unsigned short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned long int>::ger(constant<int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned long int>::ger(constant<unsigned int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned long int>::ger(constant<long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned long int>::ger(constant<unsigned long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned long int>::ger(constant<__int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned long int>::ger(constant<unsigned __int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned long int>::ger(constant<float>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<unsigned long int>::ger(constant<double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<unsigned long int>::ger(constant<long double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<__int64>::ger(constant<bool>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<__int64>::ger(constant<char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<__int64>::ger(constant<signed char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<__int64>::ger(constant<unsigned char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<__int64>::ger(constant<wchar_t>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<__int64>::ger(constant<short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<__int64>::ger(constant<unsigned short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<__int64>::ger(constant<int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<__int64>::ger(constant<unsigned int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<__int64>::ger(constant<long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<__int64>::ger(constant<unsigned long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<__int64>::ger(constant<__int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<__int64>::ger(constant<unsigned __int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<__int64>::ger(constant<float>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<__int64>::ger(constant<double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<__int64>::ger(constant<long double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<unsigned __int64>::ger(constant<bool>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned __int64>::ger(constant<char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned __int64>::ger(constant<signed char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned __int64>::ger(constant<unsigned char>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned __int64>::ger(constant<wchar_t>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned __int64>::ger(constant<short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned __int64>::ger(constant<unsigned short int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned __int64>::ger(constant<int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned __int64>::ger(constant<unsigned int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned __int64>::ger(constant<long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned __int64>::ger(constant<unsigned long int>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned __int64>::ger(constant<__int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned __int64>::ger(constant<unsigned __int64>* y)
  { return constant_impl::ge(y,this); }
  template<>
  var* constant<unsigned __int64>::ger(constant<float>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<unsigned __int64>::ger(constant<double>* y)
  { return constant_impl::fge1(y,this); }
  template<>
  var* constant<unsigned __int64>::ger(constant<long double>* y)
  { return constant_impl::fge1(y,this); }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<float>::ge(var* y){ return y->ger(this); }
cxx_compiler::var* cxx_compiler::constant<float>::ger(constant<bool>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ger(constant<char>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ger(constant<signed char>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ger(constant<unsigned char>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ger(constant<wchar_t>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ger(constant<short int>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ger(constant<unsigned short int>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ger(constant<int>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ger(constant<unsigned int>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ger(constant<long int>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ger(constant<unsigned long int>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ger(constant<__int64>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ger(constant<unsigned __int64>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ger(constant<float>* y)
{ return constant_impl::fge3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ger(constant<double>* y)
{ return constant_impl::fge3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ger(constant<long double>* y)
{ return constant_impl::fge3(y,this); }

cxx_compiler::var* cxx_compiler::constant<double>::ge(var* y){ return y->ger(this); }
cxx_compiler::var* cxx_compiler::constant<double>::ger(constant<bool>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ger(constant<char>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ger(constant<signed char>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ger(constant<unsigned char>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ger(constant<wchar_t>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ger(constant<short int>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ger(constant<unsigned short int>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ger(constant<int>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ger(constant<unsigned int>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ger(constant<long int>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ger(constant<unsigned long int>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ger(constant<__int64>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ger(constant<unsigned __int64>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ger(constant<float>* y)
{ return constant_impl::fge3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ger(constant<double>* y)
{ return constant_impl::fge3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ger(constant<long double>* y)
{ return constant_impl::fge3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ge(var* y){ return y->ger(this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ger(constant<bool>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ger(constant<char>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ger(constant<signed char>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ger(constant<unsigned char>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ger(constant<wchar_t>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ger(constant<short int>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ger(constant<unsigned short int>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ger(constant<int>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ger(constant<unsigned int>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ger(constant<long int>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ger(constant<unsigned long int>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ger(constant<__int64>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ger(constant<unsigned __int64>* y)
{ return constant_impl::fge2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ger(constant<float>* y)
{ return constant_impl::fge3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ger(constant<double>* y)
{ return constant_impl::fge3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ger(constant<long double>* y)
{ return constant_impl::fge3(y,this); }

cxx_compiler::var* cxx_compiler::constant<void*>::ge(var* z){ return z->ger(this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ger(constant<void*>* y){ return constant_impl::pcmp(goto3ac::GE,y,this); }

cxx_compiler::var* cxx_compiler::addrof::ge(var* z){ return z->ger(this); }
cxx_compiler::var* cxx_compiler::addrof::ger(addrof* y){ return addrof_impl::pcmp(goto3ac::GE,y,this); }

namespace cxx_compiler { namespace constant_impl {
  using namespace expressions::primary::literal;
  template<class A, class B> var* ptr_equality(goto3ac::op op,
                                               constant<A>* y, constant<B>* z)
  {
    assert(op == goto3ac::EQ || op == goto3ac::NE);
    usr::flag_t fy = y->m_flag;
    usr::flag_t fz = z->m_flag;
    constant<__int64>* yy = 0;
    constant<__int64>* zz = 0;
    if (fy & usr::CONST_PTR)
      yy = reinterpret_cast<constant<__int64>*>(y);
    if (fz & usr::CONST_PTR)
      zz = reinterpret_cast<constant<__int64>*>(z);
    if (yy && zz) {
      if (op == goto3ac::EQ)
        return integer::create(yy->m_value == zz->m_value);
      else
        return integer::create(yy->m_value != zz->m_value);
    }
    if (yy) {
      assert(z->m_type->integer() && z->zero());
      if (op == goto3ac::EQ)
        return integer::create(yy->m_value == 0);
      else
        return integer::create(yy->m_value != 0);
    }

    assert(zz && y->m_type->integer() && y->zero());
    if (op == goto3ac::EQ)
      return integer::create(0 == zz->m_value);
    else
      return integer::create(0 != zz->m_value);
  }
  template<class A, class B> var* eq(constant<A>* y, constant<B>* z)
  {
    if (expressions::cmp_impl::valid_pointer(goto3ac::EQ, y, z))
      return ptr_equality(goto3ac::EQ, y, z);

    usr::flag_t fy = y->m_flag;
    if (fy & usr::CONST_PTR)
      return expressions::cmp_impl::gen(goto3ac::EQ, y, z);
    usr::flag_t fz = z->m_flag;
    if (fz & usr::CONST_PTR)
      return expressions::cmp_impl::gen(goto3ac::EQ, y, z);
    return integer::create(y->m_value == z->m_value);
  }
  template<class A, class B> var* feq1(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if ( generator::long_double ){
      if ( var* v = fop1(y,z,goto3ac::EQ) )
        return v;
    }
#ifndef _MSC_VER
    return integer::create(y->m_value == z->m_value);
#else // _MSC_VER
    return integer::create(y->m_value == (__int64)z->m_value);
#endif // _MSC_VER
  }
  template<class A, class B> var* feq2(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if ( generator::long_double ){
      if ( var* v = fop2(y,z,goto3ac::EQ) )
        return v;
    }
#ifndef _MSC_VER
    return integer::create(y->m_value == z->m_value);
#else // _MSC_VER
    return integer::create((__int64)y->m_value == z->m_value);
#endif // _MSC_VER
  }
  template<class A, class B> var* feq3(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if ( generator::long_double ){
      if ( var* v = fop3(y,z,goto3ac::EQ) )
        return v;
    }
    return integer::create(y->m_value == z->m_value);
  }
} } // end of namespace constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::eqr(constant<bool>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<bool>::eqr(constant<char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<bool>::eqr(constant<signed char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<bool>::eqr(constant<unsigned char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<bool>::eqr(constant<wchar_t>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<bool>::eqr(constant<short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<bool>::eqr(constant<unsigned short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<bool>::eqr(constant<int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<bool>::eqr(constant<unsigned int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<bool>::eqr(constant<long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<bool>::eqr(constant<unsigned long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<bool>::eqr(constant<__int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<bool>::eqr(constant<unsigned __int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<bool>::eqr(constant<float>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<bool>::eqr(constant<double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<bool>::eqr(constant<long double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<bool>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }
  template<>
  var* constant<char>::eqr(constant<bool>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<char>::eqr(constant<char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<char>::eqr(constant<signed char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<char>::eqr(constant<unsigned char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<char>::eqr(constant<wchar_t>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<char>::eqr(constant<short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<char>::eqr(constant<unsigned short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<char>::eqr(constant<int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<char>::eqr(constant<unsigned int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<char>::eqr(constant<long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<char>::eqr(constant<unsigned long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<char>::eqr(constant<__int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<char>::eqr(constant<unsigned __int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<char>::eqr(constant<float>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<char>::eqr(constant<double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<char>::eqr(constant<long double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<char>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }
  template<>
  var* constant<signed char>::eqr(constant<bool>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<signed char>::eqr(constant<char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<signed char>::eqr(constant<signed char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<signed char>::eqr(constant<unsigned char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<signed char>::eqr(constant<wchar_t>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<signed char>::eqr(constant<short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<signed char>::eqr(constant<unsigned short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<signed char>::eqr(constant<int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<signed char>::eqr(constant<unsigned int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<signed char>::eqr(constant<long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<signed char>::eqr(constant<unsigned long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<signed char>::eqr(constant<__int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<signed char>::eqr(constant<unsigned __int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<signed char>::eqr(constant<float>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<signed char>::eqr(constant<double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<signed char>::eqr(constant<long double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<signed char>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }
  template<>
  var* constant<unsigned char>::eqr(constant<bool>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned char>::eqr(constant<char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned char>::eqr(constant<signed char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned char>::eqr(constant<unsigned char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned char>::eqr(constant<wchar_t>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned char>::eqr(constant<short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned char>::eqr(constant<unsigned short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned char>::eqr(constant<int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned char>::eqr(constant<unsigned int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned char>::eqr(constant<long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned char>::eqr(constant<unsigned long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned char>::eqr(constant<__int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned char>::eqr(constant<unsigned __int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned char>::eqr(constant<float>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<unsigned char>::eqr(constant<double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<unsigned char>::eqr(constant<long double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<unsigned char>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }
  template<>
  var* constant<wchar_t>::eqr(constant<bool>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<wchar_t>::eqr(constant<char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<wchar_t>::eqr(constant<signed char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<wchar_t>::eqr(constant<unsigned char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<wchar_t>::eqr(constant<wchar_t>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<wchar_t>::eqr(constant<short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<wchar_t>::eqr(constant<unsigned short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<wchar_t>::eqr(constant<int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<wchar_t>::eqr(constant<unsigned int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<wchar_t>::eqr(constant<long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<wchar_t>::eqr(constant<unsigned long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<wchar_t>::eqr(constant<__int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<wchar_t>::eqr(constant<unsigned __int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<wchar_t>::eqr(constant<float>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<wchar_t>::eqr(constant<double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<wchar_t>::eqr(constant<long double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<wchar_t>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }
  template<>
  var* constant<short int>::eqr(constant<bool>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<short int>::eqr(constant<char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<short int>::eqr(constant<signed char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<short int>::eqr(constant<unsigned char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<short int>::eqr(constant<wchar_t>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<short int>::eqr(constant<short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<short int>::eqr(constant<unsigned short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<short int>::eqr(constant<int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<short int>::eqr(constant<unsigned int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<short int>::eqr(constant<long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<short int>::eqr(constant<unsigned long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<short int>::eqr(constant<__int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<short int>::eqr(constant<unsigned __int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<short int>::eqr(constant<float>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<short int>::eqr(constant<double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<short int>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }
  template<>
  var* constant<short int>::eqr(constant<long double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<unsigned short int>::eqr(constant<bool>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned short int>::eqr(constant<char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned short int>::eqr(constant<signed char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned short int>::eqr(constant<unsigned char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned short int>::eqr(constant<wchar_t>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned short int>::eqr(constant<short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned short int>::eqr(constant<unsigned short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned short int>::eqr(constant<int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned short int>::eqr(constant<unsigned int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned short int>::eqr(constant<long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned short int>::eqr(constant<unsigned long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned short int>::eqr(constant<__int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned short int>::eqr(constant<unsigned __int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned short int>::eqr(constant<float>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<unsigned short int>::eqr(constant<double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<unsigned short int>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }
  template<>
  var* constant<unsigned short int>::eqr(constant<long double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<int>::eqr(constant<bool>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<int>::eqr(constant<char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<int>::eqr(constant<signed char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<int>::eqr(constant<unsigned char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<int>::eqr(constant<wchar_t>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<int>::eqr(constant<short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<int>::eqr(constant<unsigned short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<int>::eqr(constant<int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<int>::eqr(constant<unsigned int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<int>::eqr(constant<long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<int>::eqr(constant<unsigned long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<int>::eqr(constant<__int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<int>::eqr(constant<unsigned __int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<int>::eqr(constant<float>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<int>::eqr(constant<double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<int>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }
  template<>
  var* constant<int>::eqr(constant<long double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<unsigned int>::eqr(constant<bool>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned int>::eqr(constant<char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned int>::eqr(constant<signed char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned int>::eqr(constant<unsigned char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned int>::eqr(constant<wchar_t>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned int>::eqr(constant<short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned int>::eqr(constant<unsigned short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned int>::eqr(constant<int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned int>::eqr(constant<unsigned int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned int>::eqr(constant<long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned int>::eqr(constant<unsigned long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned int>::eqr(constant<__int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned int>::eqr(constant<unsigned __int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned int>::eqr(constant<float>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<unsigned int>::eqr(constant<double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<unsigned int>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }
  template<>
  var* constant<unsigned int>::eqr(constant<long double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<long int>::eqr(constant<bool>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<long int>::eqr(constant<char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<long int>::eqr(constant<signed char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<long int>::eqr(constant<unsigned char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<long int>::eqr(constant<wchar_t>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<long int>::eqr(constant<short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<long int>::eqr(constant<unsigned short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<long int>::eqr(constant<int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<long int>::eqr(constant<unsigned int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<long int>::eqr(constant<long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<long int>::eqr(constant<unsigned long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<long int>::eqr(constant<__int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<long int>::eqr(constant<unsigned __int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<long int>::eqr(constant<float>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<long int>::eqr(constant<double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<long int>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }
  template<>
  var* constant<long int>::eqr(constant<long double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<unsigned long int>::eqr(constant<bool>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned long int>::eqr(constant<char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned long int>::eqr(constant<signed char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned long int>::eqr(constant<unsigned char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned long int>::eqr(constant<wchar_t>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned long int>::eqr(constant<short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned long int>::eqr(constant<unsigned short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned long int>::eqr(constant<int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned long int>::eqr(constant<unsigned int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned long int>::eqr(constant<long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned long int>::eqr(constant<unsigned long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned long int>::eqr(constant<__int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned long int>::eqr(constant<unsigned __int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned long int>::eqr(constant<float>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<unsigned long int>::eqr(constant<double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<unsigned long int>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }
  template<>
  var* constant<unsigned long int>::eqr(constant<long double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<__int64>::eqr(constant<bool>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<__int64>::eqr(constant<char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<__int64>::eqr(constant<signed char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<__int64>::eqr(constant<unsigned char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<__int64>::eqr(constant<wchar_t>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<__int64>::eqr(constant<short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<__int64>::eqr(constant<unsigned short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<__int64>::eqr(constant<int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<__int64>::eqr(constant<unsigned int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<__int64>::eqr(constant<long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<__int64>::eqr(constant<unsigned long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<__int64>::eqr(constant<__int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<__int64>::eqr(constant<unsigned __int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<__int64>::eqr(constant<float>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<__int64>::eqr(constant<double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<__int64>::eqr(constant<long double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<__int64>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }
  template<>
  var* constant<unsigned __int64>::eqr(constant<bool>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned __int64>::eqr(constant<char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned __int64>::eqr(constant<signed char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned __int64>::eqr(constant<unsigned char>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned __int64>::eqr(constant<wchar_t>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned __int64>::eqr(constant<short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned __int64>::eqr(constant<unsigned short int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned __int64>::eqr(constant<int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned __int64>::eqr(constant<unsigned int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned __int64>::eqr(constant<long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned __int64>::eqr(constant<unsigned long int>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned __int64>::eqr(constant<__int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned __int64>::eqr(constant<unsigned __int64>* y)
  { return constant_impl::eq(y,this); }
  template<>
  var* constant<unsigned __int64>::eqr(constant<float>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<unsigned __int64>::eqr(constant<double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<unsigned __int64>::eqr(constant<long double>* y)
  { return constant_impl::feq1(y,this); }
  template<>
  var* constant<unsigned __int64>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }
} // end of namaespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<float>::eq(var* y){ return y->eqr(this); }
cxx_compiler::var* cxx_compiler::constant<float>::eqr(constant<bool>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::eqr(constant<char>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::eqr(constant<signed char>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::eqr(constant<unsigned char>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::eqr(constant<wchar_t>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::eqr(constant<short int>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::eqr(constant<unsigned short int>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::eqr(constant<int>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::eqr(constant<unsigned int>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::eqr(constant<long int>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::eqr(constant<unsigned long int>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::eqr(constant<__int64>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::eqr(constant<unsigned __int64>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::eqr(constant<float>* y)
{ return constant_impl::feq3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::eqr(constant<double>* y)
{ return constant_impl::feq3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::eqr(constant<long double>* y)
{ return constant_impl::feq3(y,this); }

cxx_compiler::var* cxx_compiler::constant<double>::eq(var* y){ return y->eqr(this); }
cxx_compiler::var* cxx_compiler::constant<double>::eqr(constant<bool>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::eqr(constant<char>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::eqr(constant<signed char>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::eqr(constant<unsigned char>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::eqr(constant<wchar_t>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::eqr(constant<short int>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::eqr(constant<unsigned short int>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::eqr(constant<int>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::eqr(constant<unsigned int>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::eqr(constant<long int>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::eqr(constant<unsigned long int>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::eqr(constant<__int64>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::eqr(constant<unsigned __int64>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::eqr(constant<float>* y)
{ return constant_impl::feq3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::eqr(constant<double>* y)
{ return constant_impl::feq3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::eqr(constant<long double>* y)
{ return constant_impl::feq3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::eq(var* y){ return y->eqr(this); }
cxx_compiler::var* cxx_compiler::constant<long double>::eqr(constant<bool>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::eqr(constant<char>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::eqr(constant<signed char>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::eqr(constant<unsigned char>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::eqr(constant<wchar_t>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::eqr(constant<short int>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::eqr(constant<unsigned short int>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::eqr(constant<int>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::eqr(constant<unsigned int>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::eqr(constant<long int>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::eqr(constant<unsigned long int>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::eqr(constant<__int64>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::eqr(constant<unsigned __int64>* y)
{ return constant_impl::feq2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::eqr(constant<float>* y)
{ return constant_impl::feq3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::eqr(constant<double>* y)
{ return constant_impl::feq3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::eqr(constant<long double>* y)
{ return constant_impl::feq3(y,this); }

cxx_compiler::var* cxx_compiler::constant<void*>::eq(var* z){ return z->eqr(this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<bool>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<char>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<signed char>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<unsigned char>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<wchar_t>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<short int>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<unsigned short int>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<int>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<unsigned int>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<long int>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<unsigned long int>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<__int64>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<unsigned __int64>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<void*>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }

cxx_compiler::var* cxx_compiler::addrof::eq(var* z){ return z->eqr(this); }
cxx_compiler::var* cxx_compiler::addrof::eqr(addrof* y){ return addrof_impl::pcmp(goto3ac::EQ,y,this); }

namespace cxx_compiler { namespace constant_impl {
  template<class A, class B> var* ne(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if (expressions::cmp_impl::valid_pointer(goto3ac::NE, y, z))
      return ptr_equality(goto3ac::NE, y, z);
    usr::flag_t fy = y->m_flag;
    if (fy & usr::CONST_PTR)
      return expressions::cmp_impl::gen(goto3ac::NE, y, z);
    usr::flag_t fz = z->m_flag;
    if (fz & usr::CONST_PTR)
      return expressions::cmp_impl::gen(goto3ac::NE, y, z);
    return integer::create(y->m_value != z->m_value);
  }
  template<class A, class B> var* fne1(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if ( generator::long_double ){
      if ( var* v = fop1(y,z,goto3ac::NE) )
        return v;
    }
#ifndef _MSC_VER
    return integer::create(y->m_value != z->m_value);
#else // _MSC_VER
    return integer::create(y->m_value != (__int64)z->m_value);
#endif // _MSC_VER
  }
  template<class A, class B> var* fne2(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if ( generator::long_double ){
      if ( var* v = fop2(y,z,goto3ac::NE) )
        return v;
    }
#ifndef _MSC_VER
    return integer::create(y->m_value != z->m_value);
#else // _MSC_VER
    return integer::create((__int64)y->m_value != z->m_value);
#endif // _MSC_VER
  }
  template<class A, class B> var* fne3(constant<A>* y, constant<B>* z)
  {
    using namespace expressions::primary::literal;
    if ( generator::long_double ){
      if ( var* v = fop3(y,z,goto3ac::NE) )
        return v;
    }
    return integer::create(y->m_value != z->m_value);
  }
} } // end of namespace constant_impl and cxx_compiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::ner(constant<bool>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<bool>::ner(constant<char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<bool>::ner(constant<signed char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<bool>::ner(constant<unsigned char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<bool>::ner(constant<wchar_t>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<bool>::ner(constant<short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<bool>::ner(constant<unsigned short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<bool>::ner(constant<int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<bool>::ner(constant<unsigned int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<bool>::ner(constant<long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<bool>::ner(constant<unsigned long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<bool>::ner(constant<__int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<bool>::ner(constant<unsigned __int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<bool>::ner(constant<float>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<bool>::ner(constant<double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<bool>::ner(constant<long double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<bool>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }
  template<>
  var* constant<char>::ner(constant<bool>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<char>::ner(constant<char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<char>::ner(constant<signed char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<char>::ner(constant<unsigned char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<char>::ner(constant<wchar_t>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<char>::ner(constant<short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<char>::ner(constant<unsigned short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<char>::ner(constant<int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<char>::ner(constant<unsigned int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<char>::ner(constant<long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<char>::ner(constant<unsigned long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<char>::ner(constant<__int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<char>::ner(constant<unsigned __int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<char>::ner(constant<float>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<char>::ner(constant<double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<char>::ner(constant<long double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<char>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }
  template<>
  var* constant<signed char>::ner(constant<bool>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<signed char>::ner(constant<char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<signed char>::ner(constant<signed char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<signed char>::ner(constant<unsigned char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<signed char>::ner(constant<wchar_t>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<signed char>::ner(constant<short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<signed char>::ner(constant<unsigned short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<signed char>::ner(constant<int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<signed char>::ner(constant<unsigned int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<signed char>::ner(constant<long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<signed char>::ner(constant<unsigned long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<signed char>::ner(constant<__int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<signed char>::ner(constant<unsigned __int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<signed char>::ner(constant<float>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<signed char>::ner(constant<double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<signed char>::ner(constant<long double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<signed char>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }
  template<>
  var* constant<unsigned char>::ner(constant<bool>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned char>::ner(constant<char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned char>::ner(constant<signed char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned char>::ner(constant<unsigned char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned char>::ner(constant<wchar_t>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned char>::ner(constant<short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned char>::ner(constant<unsigned short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned char>::ner(constant<int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned char>::ner(constant<unsigned int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned char>::ner(constant<long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned char>::ner(constant<unsigned long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned char>::ner(constant<__int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned char>::ner(constant<unsigned __int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned char>::ner(constant<float>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<unsigned char>::ner(constant<double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<unsigned char>::ner(constant<long double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<unsigned char>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }
  template<>
  var* constant<wchar_t>::ner(constant<bool>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<wchar_t>::ner(constant<char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<wchar_t>::ner(constant<signed char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<wchar_t>::ner(constant<unsigned char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<wchar_t>::ner(constant<wchar_t>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<wchar_t>::ner(constant<short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<wchar_t>::ner(constant<unsigned short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<wchar_t>::ner(constant<int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<wchar_t>::ner(constant<unsigned int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<wchar_t>::ner(constant<long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<wchar_t>::ner(constant<unsigned long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<wchar_t>::ner(constant<__int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<wchar_t>::ner(constant<unsigned __int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<wchar_t>::ner(constant<float>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<wchar_t>::ner(constant<double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<wchar_t>::ner(constant<long double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<wchar_t>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }
  template<>
  var* constant<short int>::ner(constant<bool>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<short int>::ner(constant<char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<short int>::ner(constant<signed char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<short int>::ner(constant<unsigned char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<short int>::ner(constant<wchar_t>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<short int>::ner(constant<short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<short int>::ner(constant<unsigned short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<short int>::ner(constant<int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<short int>::ner(constant<unsigned int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<short int>::ner(constant<long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<short int>::ner(constant<unsigned long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<short int>::ner(constant<__int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<short int>::ner(constant<unsigned __int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<short int>::ner(constant<float>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<short int>::ner(constant<double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<short int>::ner(constant<long double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<short int>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }
  template<>
  var* constant<unsigned short int>::ner(constant<bool>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned short int>::ner(constant<char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned short int>::ner(constant<signed char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned short int>::ner(constant<unsigned char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned short int>::ner(constant<wchar_t>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned short int>::ner(constant<short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned short int>::ner(constant<unsigned short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned short int>::ner(constant<int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned short int>::ner(constant<unsigned int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned short int>::ner(constant<long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned short int>::ner(constant<unsigned long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned short int>::ner(constant<__int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned short int>::ner(constant<unsigned __int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned short int>::ner(constant<float>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<unsigned short int>::ner(constant<double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<unsigned short int>::ner(constant<long double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<unsigned short int>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }
  template<>
  var* constant<int>::ner(constant<bool>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<int>::ner(constant<char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<int>::ner(constant<signed char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<int>::ner(constant<unsigned char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<int>::ner(constant<wchar_t>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<int>::ner(constant<short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<int>::ner(constant<unsigned short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<int>::ner(constant<int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<int>::ner(constant<unsigned int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<int>::ner(constant<long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<int>::ner(constant<unsigned long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<int>::ner(constant<__int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<int>::ner(constant<unsigned __int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<int>::ner(constant<float>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<int>::ner(constant<double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<int>::ner(constant<long double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<int>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }
  template<>
  var* constant<unsigned int>::ner(constant<bool>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned int>::ner(constant<char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned int>::ner(constant<signed char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned int>::ner(constant<unsigned char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned int>::ner(constant<wchar_t>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned int>::ner(constant<short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned int>::ner(constant<unsigned short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned int>::ner(constant<int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned int>::ner(constant<unsigned int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned int>::ner(constant<long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned int>::ner(constant<unsigned long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned int>::ner(constant<__int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned int>::ner(constant<unsigned __int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned int>::ner(constant<float>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<unsigned int>::ner(constant<double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<unsigned int>::ner(constant<long double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<unsigned int>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }
  template<>
  var* constant<long int>::ner(constant<bool>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<long int>::ner(constant<char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<long int>::ner(constant<signed char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<long int>::ner(constant<unsigned char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<long int>::ner(constant<wchar_t>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<long int>::ner(constant<short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<long int>::ner(constant<unsigned short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<long int>::ner(constant<int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<long int>::ner(constant<unsigned int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<long int>::ner(constant<long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<long int>::ner(constant<unsigned long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<long int>::ner(constant<__int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<long int>::ner(constant<unsigned __int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<long int>::ner(constant<float>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<long int>::ner(constant<double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<long int>::ner(constant<long double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<long int>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }
  template<>
  var* constant<unsigned long int>::ner(constant<bool>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned long int>::ner(constant<char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned long int>::ner(constant<signed char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned long int>::ner(constant<unsigned char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned long int>::ner(constant<wchar_t>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned long int>::ner(constant<short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned long int>::ner(constant<unsigned short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned long int>::ner(constant<int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned long int>::ner(constant<unsigned int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned long int>::ner(constant<long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned long int>::ner(constant<unsigned long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned long int>::ner(constant<__int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned long int>::ner(constant<unsigned __int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned long int>::ner(constant<float>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<unsigned long int>::ner(constant<double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<unsigned long int>::ner(constant<long double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<unsigned long int>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }
  template<>
  var* constant<__int64>::ner(constant<bool>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<__int64>::ner(constant<char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<__int64>::ner(constant<signed char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<__int64>::ner(constant<unsigned char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<__int64>::ner(constant<wchar_t>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<__int64>::ner(constant<short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<__int64>::ner(constant<unsigned short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<__int64>::ner(constant<int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<__int64>::ner(constant<unsigned int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<__int64>::ner(constant<long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<__int64>::ner(constant<unsigned long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<__int64>::ner(constant<__int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<__int64>::ner(constant<unsigned __int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<__int64>::ner(constant<float>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<__int64>::ner(constant<double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<__int64>::ner(constant<long double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<__int64>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }
  template<>
  var* constant<unsigned __int64>::ner(constant<bool>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned __int64>::ner(constant<char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned __int64>::ner(constant<signed char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned __int64>::ner(constant<unsigned char>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned __int64>::ner(constant<wchar_t>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned __int64>::ner(constant<short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned __int64>::ner(constant<unsigned short int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned __int64>::ner(constant<int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned __int64>::ner(constant<unsigned int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned __int64>::ner(constant<long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned __int64>::ner(constant<unsigned long int>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned __int64>::ner(constant<__int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned __int64>::ner(constant<unsigned __int64>* y)
  { return constant_impl::ne(y,this); }
  template<>
  var* constant<unsigned __int64>::ner(constant<float>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<unsigned __int64>::ner(constant<double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<unsigned __int64>::ner(constant<long double>* y)
  { return constant_impl::fne1(y,this); }
  template<>
  var* constant<unsigned __int64>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<float>::ne(var* y){ return y->ner(this); }
cxx_compiler::var* cxx_compiler::constant<float>::ner(constant<bool>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ner(constant<char>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ner(constant<signed char>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ner(constant<unsigned char>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ner(constant<wchar_t>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ner(constant<short int>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ner(constant<unsigned short int>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ner(constant<int>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ner(constant<unsigned int>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ner(constant<long int>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ner(constant<unsigned long int>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ner(constant<__int64>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ner(constant<unsigned __int64>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ner(constant<float>* y)
{ return constant_impl::fne3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ner(constant<double>* y)
{ return constant_impl::fne3(y,this); }
cxx_compiler::var* cxx_compiler::constant<float>::ner(constant<long double>* y)
{ return constant_impl::fne3(y,this); }

cxx_compiler::var* cxx_compiler::constant<double>::ne(var* y){ return y->ner(this); }
cxx_compiler::var* cxx_compiler::constant<double>::ner(constant<bool>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ner(constant<char>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ner(constant<signed char>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ner(constant<unsigned char>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ner(constant<wchar_t>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ner(constant<short int>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ner(constant<unsigned short int>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ner(constant<int>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ner(constant<unsigned int>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ner(constant<long int>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ner(constant<unsigned long int>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ner(constant<__int64>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ner(constant<unsigned __int64>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ner(constant<float>* y)
{ return constant_impl::fne3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ner(constant<double>* y)
{ return constant_impl::fne3(y,this); }
cxx_compiler::var* cxx_compiler::constant<double>::ner(constant<long double>* y)
{ return constant_impl::fne3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ne(var* y){ return y->ner(this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ner(constant<bool>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ner(constant<char>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ner(constant<signed char>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ner(constant<unsigned char>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ner(constant<wchar_t>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ner(constant<short int>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ner(constant<unsigned short int>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ner(constant<int>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ner(constant<unsigned int>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ner(constant<long int>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ner(constant<unsigned long int>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ner(constant<__int64>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ner(constant<unsigned __int64>* y)
{ return constant_impl::fne2(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ner(constant<float>* y)
{ return constant_impl::fne3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ner(constant<double>* y)
{ return constant_impl::fne3(y,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::ner(constant<long double>* y)
{ return constant_impl::fne3(y,this); }

cxx_compiler::var* cxx_compiler::constant<void*>::ne(var* z){ return z->ner(this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<bool>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<char>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<signed char>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<unsigned char>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<wchar_t>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<short int>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<unsigned short int>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<int>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<unsigned int>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<long int>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<unsigned long int>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<__int64>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<unsigned __int64>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<void*>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }

cxx_compiler::var* cxx_compiler::addrof::ne(var* z){ return z->ner(this); }
cxx_compiler::var* cxx_compiler::addrof::ner(addrof* y){ return addrof_impl::pcmp(goto3ac::NE,y,this); }


