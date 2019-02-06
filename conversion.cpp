#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

namespace cxx_compiler { namespace conversion { namespace arithmetic {
  const type* match(const type*, var**, var**);
  const type* match(var**, var**);
} } } // end of namespace arithmetic, conversion and cxx_compiler

const cxx_compiler::type* cxx_compiler::conversion::arithmetic::gen(var** y, var** z)
{
  const type* Ty = (*y)->m_type;
  const type* Tz = (*z)->m_type;
  if ( !Ty->arithmetic() || !Tz->arithmetic() )
    return 0;
  *y = (*y)->promotion();
  *z = (*z)->promotion();
  { const type* Tx = long_double_type::create(); if ( match(Tx,y,z) ) return Tx; }
  { const type* Tx = double_type::create();      if ( match(Tx,y,z) ) return Tx; }
  { const type* Tx = float_type::create();       if ( match(Tx,y,z) ) return Tx; }
  { const type* Tx = ulong_long_type::create();  if ( match(Tx,y,z) ) return Tx; }
  { const type* Tx = long_long_type::create();   if ( match(Tx,y,z) ) return Tx; }
  { const type* Tx = ulong_type::create();       if ( match(Tx,y,z) ) return Tx; }
  if ( const type* Tx = match(y,z) ) return Tx;
  { const type* Tx = long_type::create();        if ( match(Tx,y,z) ) return Tx; }
  { const type* Tx = uint_type::create();        if ( match(Tx,y,z) ) return Tx; }
  return int_type::create();
}

const cxx_compiler::type* cxx_compiler::conversion::arithmetic::match(const type* Tx, var** y, var** z)
{
  const type* Ty = (*y)->m_type;
  const type* Tz = (*z)->m_type;
  if ( Tx->compatible(Ty) ){
    *z = (*z)->cast(Tx);
    return Tx;
  }
  if ( Tx->compatible(Tz) ){
    *y = (*y)->cast(Tx);
    return Tx;
  }
  return 0;
}

const cxx_compiler::type* cxx_compiler::conversion::arithmetic::match(var** y, var** z)
{
  const type* Ty = (*y)->m_type;
  const type* Tz = (*z)->m_type;
  const type* Ta = long_type::create();
  const type* Tb = uint_type::create();
  if ( Ta->compatible(Ty) && Tb->compatible(Tz) ){
    if ( Ta->size() >= 2 * Tb->size() ){
      *z = (*z)->cast(Ta);
      return Ta;
    }
    else {
      const type* Tx = ulong_type::create();
      *y = (*y)->cast(Tx);
      *z = (*z)->cast(Tx);
      return Tx;
    }
  }
  if ( Ta->compatible(Tz) && Tb->compatible(Ty) ){
    if ( Ta->size() >= 2 * Tb->size() ){
      *y = (*y)->cast(Ta);
      return Ta;
    }
    else {
      const type* Tx = ulong_type::create();
      *y = (*y)->cast(Tx);
      *z = (*z)->cast(Tx);
      return Tx;
    }
  }
  return 0;
}

cxx_compiler::var* cxx_compiler::var::promotion()
{
  const type* T = m_type;
  const type* P = T->promotion();
  return P != T ? cast(P) : this;
}
