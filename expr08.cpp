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
cxx_compiler::expressions::cmp_impl::gen(goto3ac::op op, var* y, var* z)
{
  const type* Ty = y->m_type;
  const type* Tz = z->m_type;
  if ( !Ty->arithmetic() || !Tz->arithmetic() ) {
    if ( !cmp_impl::valid_pointer(op,y,z) ){
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
  var* constant<int>::ltr(constant<int>* y)
  { return constant_impl::lt(y,this); }
  var* constant<unsigned int>::ltr(constant<unsigned int>* y)
  { return constant_impl::lt(y,this); }
  var* constant<long int>::ltr(constant<long int>* y)
  { return constant_impl::lt(y,this); }
  var* constant<unsigned long int>::ltr(constant<unsigned long int>* y)
  { return constant_impl::lt(y,this); }
  var* constant<__int64>::ltr(constant<__int64>* y)
  { return constant_impl::lt(y,this); }
  var* constant<unsigned __int64>::ltr(constant<unsigned __int64>* y)
  { return constant_impl::lt(y,this); }
  var* constant<float>::ltr(constant<float>* y)
  { return constant_impl::flt3(y,this); }
  var* constant<double>::ltr(constant<double>* y)
  { return constant_impl::flt3(y,this); }
  var* constant<long double>::ltr(constant<long double>* y)
  { return constant_impl::flt3(y,this); }
} // end of namespace cxx_compiler

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
  var* constant<int>::gtr(constant<int>* y)
  { return constant_impl::gt(y,this); }
  var* constant<unsigned int>::gtr(constant<unsigned int>* y)
  { return constant_impl::gt(y,this); }
  var* constant<long int>::gtr(constant<long int>* y)
  { return constant_impl::gt(y,this); }
  var* constant<unsigned long int>::gtr(constant<unsigned long int>* y)
  { return constant_impl::gt(y,this); }
  var* constant<__int64>::gtr(constant<__int64>* y)
  { return constant_impl::gt(y,this); }
  var* constant<unsigned __int64>::gtr(constant<unsigned __int64>* y)
  { return constant_impl::gt(y,this); }
  var* constant<float>::gtr(constant<float>* y)
  { return constant_impl::fgt3(y,this); }
  var* constant<double>::gtr(constant<double>* y)
  { return constant_impl::fgt3(y,this); }
  var* constant<long double>::gtr(constant<long double>* y)
  { return constant_impl::fgt3(y,this); }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<void*>::gtr(constant<void*>* y){ return constant_impl::pcmp(goto3ac::GT,y,this); }

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
  var* constant<int>::ler(constant<int>* y)
  { return constant_impl::le(y,this); }
  var* constant<unsigned int>::ler(constant<unsigned int>* y)
  { return constant_impl::le(y,this); }
  var* constant<long int>::ler(constant<long int>* y)
  { return constant_impl::le(y,this); }
  var* constant<unsigned long int>::ler(constant<unsigned long int>* y)
  { return constant_impl::le(y,this); }
  var* constant<__int64>::ler(constant<__int64>* y)
  { return constant_impl::le(y,this); }
  var* constant<unsigned __int64>::ler(constant<unsigned __int64>* y)
  { return constant_impl::le(y,this); }
  var* constant<float>::ler(constant<float>* y)
  { return constant_impl::fle3(y,this); }
  var* constant<double>::ler(constant<double>* y)
  { return constant_impl::fle3(y,this); }
  var* constant<long double>::ler(constant<long double>* y)
  { return constant_impl::fle3(y,this); }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<void*>::ler(constant<void*>* y){ return constant_impl::pcmp(goto3ac::LE,y,this); }

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
  var* constant<int>::ger(constant<int>* y)
  { return constant_impl::ge(y,this); }
  var* constant<unsigned int>::ger(constant<unsigned int>* y)
  { return constant_impl::ge(y,this); }
  var* constant<long int>::ger(constant<long int>* y)
  { return constant_impl::ge(y,this); }
  var* constant<unsigned long int>::ger(constant<unsigned long int>* y)
  { return constant_impl::ge(y,this); }
  var* constant<__int64>::ger(constant<__int64>* y)
  { return constant_impl::ge(y,this); }
  var* constant<unsigned __int64>::ger(constant<unsigned __int64>* y)
  { return constant_impl::ge(y,this); }
  var* constant<float>::ger(constant<float>* y)
  { return constant_impl::fge3(y,this); }
  var* constant<double>::ger(constant<double>* y)
  { return constant_impl::fge3(y,this); }
  var* constant<long double>::ger(constant<long double>* y)
  { return constant_impl::fge3(y,this); }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<void*>::ger(constant<void*>* y){ return constant_impl::pcmp(goto3ac::GE,y,this); }

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
  var* constant<int>::eqr(constant<int>* y)
  { return constant_impl::eq(y,this); }
  var* constant<int>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }

  var* constant<unsigned int>::eqr(constant<unsigned int>* y)
  { return constant_impl::eq(y,this); }
  var* constant<unsigned int>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }

  var* constant<long int>::eqr(constant<long int>* y)
  { return constant_impl::eq(y,this); }
  var* constant<long int>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }

  var* constant<unsigned long int>::eqr(constant<unsigned long int>* y)
  { return constant_impl::eq(y,this); }
  var* constant<unsigned long int>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }

  var* constant<__int64>::eqr(constant<__int64>* y)
  { return constant_impl::eq(y,this); }
  var* constant<__int64>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }

  var* constant<unsigned __int64>::eqr(constant<unsigned __int64>* y)
  { return constant_impl::eq(y,this); }
  var* constant<unsigned __int64>::eqr(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::EQ,y,this); }

  var* constant<float>::eqr(constant<float>* y)
  { return constant_impl::feq3(y,this); }
  var* constant<double>::eqr(constant<double>* y)
  { return constant_impl::feq3(y,this); }
  var* constant<long double>::eqr(constant<long double>* y)
  { return constant_impl::feq3(y,this); }
} // end of namaespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<int>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<unsigned int>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<long int>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<unsigned long int>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<__int64>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<unsigned __int64>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::eqr(constant<void*>* y){ return constant_impl::pcmp(goto3ac::EQ,y,this); }

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
  var* constant<int>::ner(constant<int>* y)
  { return constant_impl::ne(y,this); }
  var* constant<int>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }

  var* constant<unsigned int>::ner(constant<unsigned int>* y)
  { return constant_impl::ne(y,this); }
  var* constant<unsigned int>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }

  var* constant<long int >::ner(constant<long int>* y)
  { return constant_impl::ne(y,this); }
  var* constant<long int>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }

  var* constant<unsigned long int>::ner(constant<unsigned long int>* y)
  { return constant_impl::ne(y,this); }
  var* constant<unsigned long int>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }

  var* constant<__int64>::ner(constant<__int64>* y)
  { return constant_impl::ne(y,this); }
  var* constant<__int64>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }

  var* constant<unsigned __int64>::ner(constant<unsigned __int64>* y)
  { return constant_impl::ne(y,this); }
  var* constant<unsigned __int64>::ner(constant<void*>* y)
  { return constant_impl::pcmp(goto3ac::NE,y,this); }

  var* constant<float>::ner(constant<float>* y)
  { return constant_impl::fne3(y,this); }
  var* constant<double>::ner(constant<double>* y)
  { return constant_impl::fne3(y,this); }
  var* constant<long double>::ner(constant<long double>* y)
  { return constant_impl::fne3(y,this); }
} // end of namespace cxx_compiler


cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<int>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<unsigned int>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<long int>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<unsigned long int>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<__int64>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<unsigned __int64>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }
cxx_compiler::var* cxx_compiler::constant<void*>::ner(constant<void*>* y){ return constant_impl::pcmp(goto3ac::NE,y,this); }

cxx_compiler::var* cxx_compiler::addrof::ner(addrof* y){ return addrof_impl::pcmp(goto3ac::NE,y,this); }


