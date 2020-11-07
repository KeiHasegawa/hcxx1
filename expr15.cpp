// va_arg
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

cxx_compiler::var* cxx_compiler::expressions::_va_start::info_t::gen()
{
  using namespace error::expressions::va;
  var* x = m_expr1->gen();
  x = x->rvalue();
  if ( !x->lvalue() )
    invalid("va_start", parse::position, x);
  const type* Tx = x->m_type;
  if ( !Tx->modifiable() )
    invalid("va_start",parse::position,x);
  Tx = Tx->unqualified();
  if ( Tx->m_id != type::POINTER )
    invalid("va_start",parse::position,x);
  var* y = m_expr2->gen();
  y = y->rvalue();
  if ( !y->lvalue() )
    not_lvalue(parse::position);
  const type* Ty = y->m_type;
  int n = Ty->size();
  if ( !n )
    no_size(parse::position);
  code.push_back(new va_start3ac(x,y));
  return x;
}

const cxx_compiler::file_t&
cxx_compiler::expressions::_va_start::info_t::file() const
{
  return m_expr1->file();
}

cxx_compiler::var* cxx_compiler::expressions::_va_arg::info_t::gen()
{
  var* expr = m_expr->gen();
  expr = expr->rvalue();
  {
    using namespace error::expressions::va;
    const type* T = expr->m_type;
    if ( !T->modifiable() )
      invalid("va_arg",parse::position,expr);
    T = T->unqualified();
    if ( T->m_id != type::POINTER )
      invalid("va_arg",parse::position,expr);
  }
  var* ret = new var(m_type);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  code.push_back(new va_arg3ac(ret,expr,m_type));
  return ret;
}

const
cxx_compiler::file_t& cxx_compiler::expressions::_va_arg::info_t::file() const
{
  return m_expr->file();
}

cxx_compiler::var* cxx_compiler::expressions::_va_end::info_t::gen()
{
  var* expr = m_expr->gen();
  expr = expr->rvalue();
  code.push_back(new va_end3ac(expr));
  return expr;
}

const cxx_compiler::file_t&
cxx_compiler::expressions::_va_end::info_t::file() const
{
  return m_expr->file();
}

cxx_compiler::var* cxx_compiler::expressions::address_of::info_t::gen()
{
  var* expr = m_expr->gen();
  return expr->address();
}

const cxx_compiler::file_t&
cxx_compiler::expressions::address_of::info_t::file() const
{
  return m_expr->file();
}

cxx_compiler::var* cxx_compiler::expressions::is_base_of::info_t::gen()
{
  using namespace cxx_compiler::expressions::primary::literal;
  usr* ret = boolean::create(false);
  m_Tx = m_Tx->complete_type();
  m_Ty = m_Ty->complete_type();
  m_Tx = m_Tx->unqualified();
  m_Ty = m_Ty->unqualified();
  if (m_Tx->m_id != type::RECORD)
    return ret;
  if (m_Ty->m_id != type::RECORD)
    return ret;
  typedef const record_type REC;
  REC* Rx = static_cast<REC*>(m_Tx);
  REC* Ry = static_cast<REC*>(m_Ty);
  vector<route_t> dummy;
  int offset = calc_offset(Ry, Rx, dummy, 0);
  if (offset < 0)
    return ret;
  return boolean::create(true);
}

const cxx_compiler::file_t&
cxx_compiler::expressions::constant_p::info_t::file() const
{
  return m_expr->file();
}

cxx_compiler::var* cxx_compiler::expressions::constant_p::info_t::gen()
{
  using namespace cxx_compiler::expressions::primary::literal;
  var* expr = m_expr->gen();
  expr = expr->rvalue();
  return boolean::create(expr->isconstant());
}
