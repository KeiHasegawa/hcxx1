// pm-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

namespace cxx_compiler {
  usr* operator_function(const type* T, int op)
  {
    T = T->unqualified();
    if (T->m_id == type::REFERENCE) {
      typedef const reference_type RT;
      RT* rt = static_cast<RT*>(T);
      T = rt->referenced_type();
      T = T->unqualified();
    }
    if (T->m_id != type::RECORD)
      return 0;
    typedef const record_type REC;
    REC* rec = static_cast<REC*>(T);
    tag* ptr = rec->get_tag();
    string name = operator_name(op);
    parse::identifier::mode_t org = parse::identifier::mode;
    parse::identifier::mode = parse::identifier::no_err;
    int r = parse::identifier::lookup(name, ptr);
    parse::identifier::mode = org;
    if (!r)
      return 0;
    var* v = cxx_compiler_lval.m_var;
    if (genaddr* ga = v->genaddr_cast())
      v = ga->m_ref;
    assert(v->usr_cast());
    usr* op_fun = static_cast<usr*>(v);
    return op_fun;
  }
  usr* operator_function(int op)
  {
    string name = operator_name(op);
    parse::identifier::mode_t org = parse::identifier::mode;
    parse::identifier::mode = parse::identifier::no_err;
    int r = parse::identifier::lookup(name, scope::current);
    parse::identifier::mode = org;
    if (!r)
      return 0;
    var* v = cxx_compiler_lval.m_var;
    if (genaddr* ga = v->genaddr_cast())
      v = ga->m_ref;
    assert(v->usr_cast());
    usr* op_fun = static_cast<usr*>(v);
    scope* ps = op_fun->m_scope;
    if (ps->m_id == scope::TAG) {
      usr::flag_t flag = op_fun->m_flag;
      if (!(flag & usr::STATIC))
        return 0;
    }
    return op_fun;
  }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::expressions::binary::info_t::gen()
{
  var* left = m_left->gen();
  int n = code.size();
  var* right = m_right->gen();

  var* leftc = left;
  var* rightc = right;

  switch (m_op) {
  case '*': case '/': case '%': case '+': case '-':
  case LSH_MK: case RSH_MK:
  case '<': case '>':
  case LESSEQ_MK: case GREATEREQ_MK: case EQUAL_MK: case NOTEQ_MK:
  case '&': case '^': case '|':
  case ',':
  case MUL_ASSIGN_MK: case DIV_ASSIGN_MK: case MOD_ASSIGN_MK:
  case ADD_ASSIGN_MK: case SUB_ASSIGN_MK: case LSH_ASSIGN_MK:
  case RSH_ASSIGN_MK: case AND_ASSIGN_MK: case XOR_ASSIGN_MK:
  case OR_ASSIGN_MK:
    leftc = left->rvalue();
    leftc = leftc->promotion();
    rightc = right->rvalue();
    rightc = rightc->promotion();
    break;
  case '=':
    rightc = right->rvalue();
    break;
  }

  switch (m_op) {
  case '*': case '/': case '%': case '+': case '-':
  case '<': case '>':
  case LESSEQ_MK: case GREATEREQ_MK: case EQUAL_MK: case NOTEQ_MK:
  case '&': case '^': case '|':
  case MUL_ASSIGN_MK: case DIV_ASSIGN_MK: case MOD_ASSIGN_MK:
  case ADD_ASSIGN_MK: case SUB_ASSIGN_MK:
  case AND_ASSIGN_MK: case XOR_ASSIGN_MK: case OR_ASSIGN_MK:
    if (!conversion::arithmetic::gen(&leftc, &rightc)) {
      if (!operator_function(leftc->m_type, m_op) &&
          !operator_function(rightc->m_type, m_op)) {
        switch (m_op) {
        case '*': case '/': case '%': case '&': case '^': case '|':
        case MUL_ASSIGN_MK: case DIV_ASSIGN_MK: case MOD_ASSIGN_MK:
        case AND_ASSIGN_MK: case XOR_ASSIGN_MK: case OR_ASSIGN_MK:
          using namespace error::expressions::binary;
          invalid(file(), m_op, leftc->m_type, rightc->m_type);
        }
      }
    }
    break;
  }

  switch (m_op) {
  case DOTASTER_MK :   return leftc->ptr_member(rightc, true);
  case ARROWASTER_MK : return leftc->ptr_member(rightc,false);
  case '*' :           return leftc->mul(rightc);
  case '/' :           return leftc->div(rightc);
  case '%' :           return leftc->mod(rightc);
  case '+' :           return leftc->add(rightc);
  case '-' :           return leftc->sub(rightc);
  case LSH_MK :        return leftc->lsh(rightc);
  case RSH_MK :        return leftc->rsh(rightc);
  case '<' :           return leftc->lt(rightc);
  case '>' :           return leftc->gt(rightc);
  case LESSEQ_MK :     return leftc->le(rightc);
  case GREATEREQ_MK :  return leftc->ge(rightc);
  case EQUAL_MK :      return leftc->eq(rightc);
  case NOTEQ_MK:       return leftc->ne(rightc);
  case '&' :           return leftc->bit_and(rightc);
  case '^' :           return leftc->bit_xor(rightc);
  case '|' :           return leftc->bit_or(rightc);
  case ANDAND_MK :     return leftc->logic1(true,n,rightc);
  case OROR_MK :       return leftc->logic1(false,n,rightc);
  case '=' :           return left->assign(rightc);
  case MUL_ASSIGN_MK : return left->assign(leftc->mul(rightc));
  case DIV_ASSIGN_MK : return left->assign(leftc->div(rightc));
  case MOD_ASSIGN_MK : return left->assign(leftc->mod(rightc));
  case ADD_ASSIGN_MK : return left->assign(leftc->add(rightc));
  case SUB_ASSIGN_MK : return left->assign(leftc->sub(rightc));
  case LSH_ASSIGN_MK : return left->assign(leftc->lsh(rightc));
  case RSH_ASSIGN_MK : return left->assign(leftc->rsh(rightc));
  case AND_ASSIGN_MK : return left->assign(leftc->bit_and(rightc));
  case XOR_ASSIGN_MK : return left->assign(leftc->bit_xor(rightc));
  case  OR_ASSIGN_MK : return left->assign(leftc->bit_or(rightc));
  case ',' :           return leftc->comma(rightc);
  default:             return left->subscripting(right);
  }
}

const cxx_compiler::file_t& cxx_compiler::expressions::binary::info_t::file() const
{
  return m_left->file();
}
