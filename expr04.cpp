// pm-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

cxx_compiler::var* cxx_compiler::expressions::binary::info_t::gen()
{
  var* left = m_left->gen();
  int n = code.size();
  var* right = m_right->gen();
  switch ( m_op ){
  case DOTASTER_MK :   throw int();
  case ARROWASTER_MK : throw int();
  case '*' :           return left->mul(right);
  case '/' :           return left->div(right);
  case '%' :           return left->mod(right);
  case '+' :           return left->add(right);
  case '-' :           return left->sub(right);
  case LSH_MK :        return left->lsh(right);
  case RSH_MK :        return left->rsh(right);
  case '<' :           return left->lt(right);
  case '>' :           return left->gt(right);
  case LESSEQ_MK :     return left->le(right);
  case GREATEREQ_MK :  return left->ge(right);
  case EQUAL_MK :      return left->eq(right);
  case NOTEQ_MK:       return left->ne(right);
  case '&' :           return left->bit_and(right);
  case '^' :           return left->bit_xor(right);
  case '|' :           return left->bit_or(right);
  case ANDAND_MK :     return left->logic1(true,n,right);
  case OROR_MK :       return left->logic1(false,n,right);
  case '=' :           return left->assign(right);
  case MUL_ASSIGN_MK : return left->assign(left->mul(right));
  case DIV_ASSIGN_MK : return left->assign(left->div(right));
  case MOD_ASSIGN_MK : return left->assign(left->mod(right));
  case ADD_ASSIGN_MK : return left->assign(left->add(right));
  case SUB_ASSIGN_MK : return left->assign(left->sub(right));
  case RSH_ASSIGN_MK : return left->assign(left->rsh(right));
  case LSH_ASSIGN_MK : return left->assign(left->lsh(right));
  case AND_ASSIGN_MK : return left->assign(left->bit_and(right));
  case XOR_ASSIGN_MK : return left->assign(left->bit_xor(right));
  case  OR_ASSIGN_MK : return left->assign(left->bit_or(right));
  case ',' :           return left->comma(right);
  default:             return left->subscripting(right);
  }
}

const cxx_compiler::file_t& cxx_compiler::expressions::binary::info_t::file() const
{
  return m_left->file();
}
