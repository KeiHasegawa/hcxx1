#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

int cxx_compiler::error::counter;
bool cxx_compiler::error::headered;

cxx_compiler::error::LANG cxx_compiler::error::lang;

namespace cxx_compiler { namespace error {
  struct init_lang {
    init_lang();
  } obj;
} } // end of namespace error and cxx_compiler

cxx_compiler::error::init_lang::init_lang()
{
  using namespace std;
  char* p = getenv("LANG");
  if ( !p )
    return;
  string s = p;
  if ( s == "en_US.UTF-8" )
    lang = other;
}

void cxx_compiler::error::not_implemented()
{
  string prog = cxx_compiler::cmdline::prog;
  cerr << prog  << " : " << "not implemented" << '\n';
  exit(1);
}

void cxx_compiler::error::cmdline::open(std::string fn)
{
  using namespace std;
  string prog = cxx_compiler::cmdline::prog;
  switch ( lang ){
  case jpn: cerr << prog << " : " << fn << " ���J���܂���.\n"; break;
  default:  cerr << prog << " : " << "cannot open `" << fn << '\n'; break;
  }
}

void cxx_compiler::error::cmdline::generator()
{
  using namespace std;
  using namespace error;
  string prog = cxx_compiler::cmdline::prog;
  switch ( lang ){
  case jpn:
    cerr << prog << ": �^�[�Q�b�g�W�F�l���[�^���w�肳��Ă��܂���.\n";
    break;
  default:
    cerr << prog << ": generator is not specified.\n";
    break;
  }
  ++counter;
}

void cxx_compiler_error(const char*)
{
  using namespace std;
  using namespace cxx_compiler;

  switch ( error::lang ){
#ifndef __GNUC__  // for garbled characters gcc version 7.3.0
  case error::jpn:
    error::header(parse::position,"�G���[");
    if (*cxx_compiler_text) {
      cerr << '`';
      istringstream ist(cxx_compiler_text); string s; ist >> s; cerr << s;
      cerr << "' ��";
    }
    cerr << "�\���G���[�ł�.";
    break;
#endif // __GNUC__ for garbled characters gcc version 7.3.0
  default:
    error::header(parse::position,"error");
    cerr << "syntax error";
    if (*cxx_compiler_text) {
      cerr << " before `";
      istringstream ist(cxx_compiler_text); string s; ist >> s; cerr << s;
      cerr << "' token.";
    }
    break;
  }
  cerr << '\n';
  ++error::counter;
}

void cxx_compiler::error::header(const file_t& file, std::string msg)
{
  using namespace std;
  if ( !headered ){
    if ( fundef* func = fundef::current ){
      string name = func->m_usr->m_name;
      switch ( lang ){
      case jpn:
        cerr << file.m_name << ": ���� `" << name << "' :\n";
        break;
      default:
        cerr << file.m_name << ": In function `" << name << "' :\n";
        break;
      }
      headered = true;
    }
  }
  cerr << file.m_name << ':' << file.m_lineno << ": " << msg << ": ";
}

void cxx_compiler::error::undeclared(const file_t& file, std::string name)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << '`' << name << "' �͐錾����Ă��܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "undeclared variable `" << name << "'.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::specifier_seq::type::multiple(const file_t& file, const cxx_compiler::type* x, const cxx_compiler::type* y)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << '`';
    x->decl(cerr,"");
    cerr << "' �� `";
    y->decl(cerr,"");
    cerr << "' ���w�肳��Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << '`';
    x->decl(cerr,"");
    cerr << "' and `";
    y->decl(cerr,"");
    cerr << "' specified.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::specifier_seq::type::invalid_combination(const file_t& file, const cxx_compiler::type* T, std::string spec)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << '`';
    T->decl(cerr,"");
    cerr << "' �� `" << spec << "' ���w�肳��Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << '`';
    T->decl(cerr,"");
    cerr << "' is specified with `" << spec << "' .\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::specifier_seq::type::invalid_combination(const file_t& file, std::string x, std::string y)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << '`' << x << "' �� `" << y << "' ���w�肳��Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "both `" << x << "' and `" << y << "' are specified.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::specifier_seq::type::implicit_int(const usr* u)
{
  using namespace std;
  using namespace error;
  switch ( lang ){
  case jpn:
    error::header(u->m_file,"�G���[");
    cerr << "`" << u->m_name << "' �̐錾�� `int' �����肳��܂���.\n";
    break;
  default:
    error::header(u->m_file,"error");
    cerr << "type defaults to `int' in declaration of `" << u->m_name << "'.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::specifier_seq::type::implicit_int(const file_t& file)
{
  using namespace std;
  using namespace error;
  switch ( lang ){
  case jpn:
    error::header(file,"�G���[");
    cerr << "�������̐錾�� `int' �����肳��܂���.\n";
    break;
  default:
    error::header(file,"error");
    cerr << "type defaults to `int' in declaration of parameter.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::declarators::qualifier::invalid(const file_t& file, const type* T)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << '`';
    T->decl(cerr,"");
    cerr << "' �� `restrict' �ɂ���ďC������Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << '`';
    T->decl(cerr,"");
    cerr << "' is qualified by `restrict'.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::declarators::function::of_function(const file_t& file, const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    if ( u ){
      header(u->m_file,"�G���[");
      string name = u->m_name;
      cerr << '`' << name << "' ��������Ԃ������Ɛ錾����Ă��܂�.\n";
    }
    else {
      header(file,"�G���[");
      cerr << "������Ԃ��������錾����Ă��܂�.\n";
    }
    break;
  default:
    if ( u ){
      header(u->m_file,"error");
      string name = u->m_name;
      cerr << '`' << name << "' declared as function returning function.\n";
    }
    else {
      header(file,"error");
      cerr << "declare function returning function.\n";
    }
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::declarators::function::of_array(const file_t& file, const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    if ( u ){
      header(u->m_file,"�G���[");
      string name = u->m_name;
      cerr << '`' << name << "' ���z���Ԃ������Ɛ錾����Ă��܂�.\n";
    }
    else {
      header(file,"error");
      cerr << "�z���Ԃ��������錾����Ă��܂�.\n";
    }
    break;
  default:
    if ( u ){
      header(u->m_file,"error");
      string name = u->m_name;
      cerr << '`' << name << "' declared as function returning array.\n";
    }
    else {
      header(file,"error");
      cerr << "declare function returning array.\n";
    }
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::declarators::array::of_function(const file_t& file, const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    if ( u ){
      header(u->m_file,"�G���[");
      string name = u->m_name;
      cerr << '`' << name << "' �������̔z��Ɛ錾����Ă��܂�.\n";
    }
    else {
      header(file,"error");
      cerr << "�����̔z�񂪐錾����Ă��܂�.\n";
    }
    break;
  default:
    if ( u ){
      header(u->m_file,"error");
      string name = u->m_name;
      cerr << '`' << name << "' declared as array of function.\n";
    }
    else {
      header(file,"error");
      cerr << "declare array of function.\n";
    }
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::declarators::array::not_integer(const file_t& file, const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    if ( u ){
      header(u->m_file,"�G���[");
      string name = u->m_name;
      cerr << "�z�� `" << name << "' �̎����������^�ł͂���܂���.\n";
    }
    else {
      header(file,"error");
      cerr << "�z��̎����������^�ł͂���܂���.\n";
    }
    break;
  default:
    if ( u ){
      header(u->m_file,"error");
      string name = u->m_name;
      cerr << "array `" << name << "' dimension is not integer.\n";
    }
    else {
      header(file,"error");
      cerr << "array dimension is not integer.\n";
    }
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::declarators::array::not_positive(const file_t& file, const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    if ( u ){
      header(u->m_file,"�G���[");
      string name = u->m_name;
      cerr << "�z�� `" << name << "' �̎����������ł͂���܂���.\n";
    }
    else {
      header(file,"error");
      cerr << "�z��̎����������ł͂���܂���.\n";
    }
    break;
  default:
    if ( u ){
      header(u->m_file,"error");
      string name = u->m_name;
      cerr << "array `" << name << "' dimension is not positive.\n";
    }
    else {
      header(file,"error");
      cerr << "array dimension is not positive.\n";
    }
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::declarators::array::asterisc_dimension(const file_t& file, const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    if ( u ){
      header(u->m_file,"�G���[");
      string name = u->m_name;
      cerr << '`' << name << "' �̎����� `*' ���w�肳��Ă��܂�.\n";
    }
    else {
      header(file,"error");
      cerr << "�����̎����� `*' ���w�肳��Ă��܂�.\n";
    }
    break;
  default:
    if ( u ){
      header(u->m_file,"error");
      string name = u->m_name;
      cerr << "array `" << name << "' dimension is specified by `*'.\n";
    }
    else {
      header(file,"error");
      cerr << "array dimension is specified by `*'.\n";
    }
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::declarators::vm::file_scope(const usr* u)
{
  using namespace std;
  string name = u->m_name;
  const type* T = u->m_type;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "variably modified �� `";
    cerr << name;
    cerr << "' ���t�@�C���X�R�[�v�Ő錾����Ă��܂�.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "variably modified type `";
    cerr << name;
    cerr << "' is declared in file scope.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::declarators::reference::missing_initializer(const usr* u)
{
  using namespace std;
  string name = u->m_name;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "`";
    cerr << name;
    cerr << "' �̏������w��q������܂���.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "missng initializer for `";
    cerr << name;
    cerr << "'.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::classes::bit_field::zero(const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "�r�b�g�t�B�[���h `" << u->m_name << "' �̃r�b�g���Ƀ[�����w�肳��Ă��܂�.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "specified bit of `" << u->m_name << "' is zero.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::classes::bit_field::exceed(const usr* u, const type* T)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "�r�b�g�t�B�[���h";
    if ( !u->m_name.empty() )
      cerr << " `" << u->m_name << "' ";
    cerr << "�̃r�b�g���� ";
    T->decl(cerr,"");
    cerr << " �̃T�C�Y�𒴂��Ă��܂�.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "specified bit ";
    if ( u->m_name.empty() )
      cerr << "of `" << u->m_name << "' ";
    cerr << "is exceed ";
    T->decl(cerr,"");
    cerr << " .\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::classes::incomplete_member(const usr* u)
{
  using namespace std;
  string name = u->m_name;
  const type* T = u->m_type;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    T->decl(cerr,name);
    cerr << " �������o�Ƃ��Đ錾����Ă��܂�.\n";
    break;
  default:
    header(u->m_file,"error");
    T->decl(cerr,name);
    cerr << " is declared as member.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::classes::not_ordinary(const usr* u)
{
  using namespace std;
  string name = u->m_name;
  const type* T = u->m_type;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "variably modified �� `";
    T->decl(cerr,name);
    cerr << "' �������o�Ƃ��Đ錾����Ă��܂�.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "variably modified type `";
    T->decl(cerr,name);
    cerr << "' is declared as member.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::primary::literal::integer::too_large(const file_t& file, std::string name, const type* T)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "`" << name << "' �� `";
    T->decl(cerr,"");
    cerr << "' �͈̔͂𒴂��Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "integer constant `" << name << "' is too large for `";
    T->decl(cerr,"");
    cerr << "'.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::postfix::call::not_function(const file_t& file, const var* v)
{
  using namespace std;
  const usr* u = dynamic_cast<const usr*>(v);
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�����ł͂Ȃ�";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    else
      cerr << "����";
    cerr << "���Ăяo���܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "called object";
    if ( u )
      cerr << " `" << u->m_name << "'";
    cerr << " is not a function.\n";
  }
  ++counter;
}

void cxx_compiler::error::expressions::postfix::call::num_of_arg(const file_t& file, const var* v, int n, int m)
{
  using namespace std;
  const usr* u = dynamic_cast<const usr*>(v);
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�����Ăяo��";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    cerr << "�̈����� " << n << " ���w�肳��Ă��܂��� " << m << " ���K�v�ł�.\n";
    break;
  default:
    header(file,"error");
    cerr << "number of arguments function";
    if ( u )
      cerr << " `" << u->m_name << "'";
    cerr << " call is specified " << n << ", but required " << m << ".\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::postfix::call::not_object(const file_t& file, const var* v)
{
  using namespace std;
  const usr* u = dynamic_cast<const usr*>(v);
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "����";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    cerr << "�̖߂�l�̓I�u�W�F�N�g�ł͂���܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "function ";
    if ( u )
      cerr << " `" << u->m_name << "'";
    cerr << " return value is not object.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::postfix::call::mismatch_argument(const file_t& file, int n, bool discard, const var* v)
{
  using namespace std;
  const usr* u = dynamic_cast<const usr*>(v);
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "����";
    if ( u )
      cerr << " `" << u->m_name << "' ��";
    cerr << "�Ăяo���� " << n + 1 << " �Ԗڂ̈������}�b�`���܂���.";
    if ( discard )
      cerr << " �C���q�������܂�.";
    cerr << '\n';
    break;
  default:
    header(file,"error");
    cerr << "function call ";
    if ( u )
      cerr << '`' << u->m_name << "' ";
    switch ( n ){
    case 0: cerr << "1st"; break;
    case 1: cerr << "2nd"; break;
    case 2: cerr << "3rd"; break;
    default:
      cerr << n-1 << "th"; break;
    }
    cerr << " argument mismatch.";
    if ( discard )
      cerr << " qualifier discarded.";
    cerr << '\n';
    break;
  }
  ++counter;
}

namespace cxx_compiler {
  namespace error {
    namespace expressions {
      namespace postfix {
        namespace call {
          void overload_candidacy(usr* c)
          {
            const file_t& file = c->m_file;
            string name = c->m_name;
            switch (lang) {
            case jpn:
            default:
              header(file,"error");
              cerr << "`" << name << "'" << '\n';
              break;
            }
          }
        } // end of namespace call
      } // end of namespace postfix
    } // end of namespace expressions
  } // end of namespace error
} // end of namespace cxx_compiler

void
cxx_compiler::error::expressions::postfix::call::overload_not_match(const usr* u)
{
  const file_t& file = u->m_file;
  assert(u->m_flag & usr::OVERLOAD);
  const overload* o = static_cast<const overload*>(u);
  const vector<usr*>& candidacy = o->m_candidacy;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "����" << " `" << u->m_name << "' ��";
    cerr << "�I�[�o�[���[�h�Ăяo������������}�b�`���܂���.";
    cerr << '\n';
    cerr << "���:" << '\n';
    for ( auto c : candidacy )
      overload_candidacy(c);
    break;
  default:
    header(file,"error");
    cerr << "function overload call ";
    cerr << '`' << u->m_name << "' ";
    cerr << " doesn't mismatch to any.";
    cerr << '\n';
    cerr << "Candidacy:" << '\n';
    for (auto c : candidacy)
      overload_candidacy(c);
    break;
  }
  ++counter;
}

namespace cxx_compiler { namespace error { namespace expressions { namespace binary {
  struct table : std::map<int, std::string> {
    table();
  } m_table;
} } } } // end of namespace binary, expressions, error and cxx_compiler

void cxx_compiler::error::expressions::binary::invalid(const file_t& file, int op, const type* y, const type* z)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�񍀉��Z�q `" << m_table[op] << "' ���^ `";
    y->decl(cerr,"");
    cerr << "' �ƌ^ `";
    z->decl(cerr,"");
    cerr << "' �ɓK�p����܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "binary operator `" << m_table[op] << "' is specified `";
    y->decl(cerr,"");
    cerr << "' and `";
    z->decl(cerr,"");
    cerr << "'.\n";
    break;
  }
  ++counter;
}

cxx_compiler::error::expressions::binary::table::table()
{
  (*this)['*'] = "*";
  (*this)['/'] = "/";
  (*this)['%'] = "%";
  (*this)['+'] = "+";
  (*this)['-'] = "-";
  (*this)[LSH_MK] = "<<";
  (*this)[RSH_MK] = ">>";
  (*this)['<'] = "<";
  (*this)['>'] = ">";
  (*this)[LESSEQ_MK] = "<=";
  (*this)[GREATEREQ_MK] = ">=";
  (*this)[EQUAL_MK] = "==";
  (*this)[NOTEQ_MK] = "!=";
  (*this)['&'] = "&";
  (*this)['|'] = "|";
  (*this)['^'] = "^";
  (*this)[ANDAND_MK] = "&&";
  (*this)[OROR_MK] = "||";
  (*this)[MUL_ASSIGN_MK] = "*";
  (*this)[DIV_ASSIGN_MK] = "/";
  (*this)[MOD_ASSIGN_MK] = "%";
  (*this)[AND_ASSIGN_MK] = "&";
  (*this)[XOR_ASSIGN_MK] = "^";
  (*this)[OR_ASSIGN_MK]  = "|";
}

void cxx_compiler::error::expressions::assignment::not_lvalue(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "������Z�q�̍��̃I�y�����h�����Ӓl�������܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "left operand is not lvalue in assignment.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::assignment::not_modifiable(const file_t& file, const usr* u)
{
  using namespace std;
  string name;
  if ( u && u->m_name[0] != '.' )
    name = u->m_name;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "������Z�q�̍��I�y�����h";
    if ( !name.empty() )
      cerr << " `" << name << "' ";
    cerr << "�͕ύX�ł��܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "left operand ";
    if ( !name.empty() )
      cerr << '`' << name << "' ";
    cerr << "is not modifiable.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::assignment::invalid(const file_t& file, const usr* u, bool discard)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    if ( u ){
      string name = u->m_name;
      cerr << "`" << name << "' �ɑ΂���";
    }
    cerr << "������Z�q���s���ł�.";
    if ( discard )
      cerr << " �C���q�������܂�.";
    break;
  case other:
    header(file,"error");
    cerr << "invalid assignment";
    if ( u ){
      string name = u->m_name;
      cerr << " for `" << name << "'";
    }
    cerr << '.';
    if ( discard )
      cerr << " qualifier discarded.";
    break;
  }
  cerr << '\n';
  ++counter;
}

void cxx_compiler::error::expressions::assignment::not_modifiable_lvalue(const file_t& file, const type* T)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "������Z�q���^ `";
    T->decl(cerr,"");
    cerr << "' �ɓK�p����Ă��܂�.\n";
    break;
  case other:
    header(file,"error");
    cerr << "assignment operator specified for `";
    T->decl(cerr,"");
    cerr << ".\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::statements::_case::not_constant(const file_t& file, const var* v)
{
  using namespace std;
  const usr* u = dynamic_cast<const usr*>(v);
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "case ���x��";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    cerr << "���萔�ł͂���܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "case label";
    if ( u )
      cerr << " `" << u->m_name << "'";
    cerr << " is not constant.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::statements::_case::not_integer(const file_t& file, const var* v)
{
  using namespace std;
  const usr* u = dynamic_cast<const usr*>(v);
  string name = u->m_name;
  if ( name[0] == '.' && !isdigit(name[1]) )
    name.erase();
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "case ���x��";
    if ( !name.empty() )
      cerr << " `" << name << "' ";
    cerr << "�������ł͂���܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "case label";
    if ( !name.empty() )
      cerr << " `" << name << "' ";
    cerr << "is not integer.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::statements::_case::no_switch(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "case ���x�� �� switch �̊O�Ŏg�p����Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "case label used outof switch.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::statements::_case::duplicate(const file_t& curr, const file_t& prev)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(curr,"�G���[");
    cerr << "case ���x�� ���d�����Ďw�肳��Ă��܂�.\n";
    header(prev,"�G���[");
    cerr << "���� case ���x���Əd�����Ă��܂�.\n";
    break;
  default:
    header(curr,"error");
    cerr << "duplicate case label.\n";
    header(prev,"error");
    cerr << "with here.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::statements::_default::no_switch(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "default �� switch �̊O�Ŏg�p����Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "default used outof switch.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::statements::_default::multiple(const file_t& curr, const file_t& prev)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(curr,"�G���[");
    cerr << "default �� 2 ����܂�.\n";
    header(prev,"�G���[");
    cerr << "������ default ������܂���.\n";
    break;
  default:
    header(curr,"error");
    cerr << "multiple default.\n";
    header(prev,"error");
    cerr << "default was here.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::statements::if_stmt::not_scalar(const file_t& file, const var* v)
{
  using namespace std;
  const usr* u = dynamic_cast<const usr*>(v);
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "if �̎�";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    cerr << "���X�J���[�^�ł͂���܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "if expression";
    if ( u )
      cerr << " `" << u->m_name << "'";
    cerr << " is not scalar type.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::statements::switch_stmt::not_integer(const file_t& file, const var* v)
{
  using namespace std;
  const usr* u = dynamic_cast<const usr*>(v);
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "switch �̎�";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    cerr << "�������ł͂���܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "case label";
    if ( u )
      cerr << " `" << u->m_name << "'";
    cerr << " is not constant.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::statements::break_stmt::not_within(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "break �������[�v�̒��ł� switch ���̒��ł��Ȃ��Ƃ���ɏo�����܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "break statement not within loop or switch.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::statements::continue_stmt::not_within(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "continue �������[�v�̒��łȂ��Ƃ���ɏo�����܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "continue statement not within a loop.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::statements::while_stmt::not_scalar(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "while �����X�J���[�^�ł���܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "while expression is not scalar.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::statements::do_stmt::not_scalar(const file_t& file)
{
  using namespace std;
  while_stmt::not_scalar(file);
}

void cxx_compiler::error::statements::for_stmt::not_scalar(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "`for' �̑� 2 �����X�J���[�^�ł���܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "2nd expression of `for' is not scalar.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::enumeration::not_constant(const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "�񋓂̃����o `" << u->m_name << "' �̒l���萔�ł͂���܂���.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "value of enumerator member `" << u->m_name << "' is not constant.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::enumeration::not_integer(const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "�񋓂̃����o `" << u->m_name << "' �̒l�������ł͂���܂���.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "value of enumerator member `" << u->m_name << "' is not integer.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::binary::not_compatible(const file_t& file, const pointer_type* y, const pointer_type* z)
{
  invalid(file,'-',y,z);
}

void cxx_compiler::error::statements::label::multiple(std::string label, const file_t& prev, const file_t& curr)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(curr,"�G���[");
    cerr << "���x�� `" << label << "' �����d�ɒ�`����Ă��܂�.\n";
    header(prev,"�G���[");
    cerr << "�����Œ�`����Ă��܂���.\n";
    break;
  default:
    header(curr,"error");
    cerr << "label `" << label << "' is redefined.\n";
    header(prev,"error");
    cerr << "previous definition is here.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::unary::address::not_lvalue(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�P���� `&' ���Z�q�����Ӓl�łȂ����ɓK�p����Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "operator `&' specified for not lvalue.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::unary::address::bit_field(const file_t& file, const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�P���� & ���Z�q���r�b�g�t�B�[���h `" << u->m_name << "' �ɑ΂��Ďw�肳��Ă��܂�.\n";
    header(u->m_file,"�G���[");
    cerr << '`' << u->m_name << "' �͂����Ő錾����Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "operator `&' specified for bit field `" << u->m_name << "'.\n";
    header(u->m_file,"error");
    cerr << '`' << u->m_name << "' declared here.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::unary::indirection::not_pointer(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�P���� * ���|�C���^�łȂ��^�̎��Ɏw�肳��Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "operator `*' specified for non-pointer type expression.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::unary::invalid(const file_t& file, int op, const type* T)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�P���� `" << char(op) << "' ���^ `";
    T->decl(cerr,"");
    cerr << "' �ɓK�p����Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "operator `" << char(op) << "' specified for `";
    T->decl(cerr,"");
    cerr << "'.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::conditional::not_scalar(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "3 �����Z�q�̑� 1 ���̌^���X�J���[�ł͂���܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "type of 1st expression is not scalar.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::conditional::mismatch(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "3 �����Z�q�̑� 2 ���Ƒ� 3 ���̌^���s���ł�.\n";
    break;
  default:
    header(file,"error");
    cerr << "type mismatch 2nd and 3rd expression.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::postfix::subscripting::not_pointer(const file_t& file, const var* v)
{
  using namespace std;
  const usr* u = dynamic_cast<const usr*>(v);
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�Y�����Z�q��z��ł��|�C���^�ł��Ȃ�";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    else
      cerr << "����";
    cerr << "�Ɏg�p���Ă��܂�.\n";
    if ( u ){
      header(u->m_file,"�G���[");
      cerr << '`' << u->m_name << "' �͂����Ő錾����Ă��܂�.\n";
    }
    break;
  default:
    header(file,"error");
    cerr << "subscripted value";
    if ( u )
      cerr << " `" << u->m_name << "'";
    cerr << " is neither array nor pointer\n";
    if ( u ){
      header(u->m_file,"error");
      cerr << '`' << u->m_name << "' is declared here.\n";
    }
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::postfix::subscripting::not_object(const file_t& file, const type* T)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�Y���Q�Ƃ��ꂽ�l�� ";
    pointer_type::create(T)->decl(cerr,"");
    cerr << " �ł�.\n";
    if ( tag* tag = T->get_tag() ){
      header(tag->m_file.back(),"�G���[");
      cerr << '`' << tag::keyword(tag->m_kind) << ' ' << tag->m_name << "'";
      cerr << " �͂����Ő錾����Ă��܂�.\n";
    }
    break;
  default:
    header(file,"error");
    cerr << "subscripted value is ";
    pointer_type::create(T)->decl(cerr,"");
    cerr << ".\n";
    if ( tag* tag = T->get_tag() ){
      header(tag->m_file.back(),"error");
      cerr << '`' << tag::keyword(tag->m_kind) << ' ' << tag->m_name << "'";
      cerr << " declared here.\n";
    }
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::postfix::subscripting::not_integer(const file_t& file, const var* v)
{
  using namespace std;
  const usr* u = dynamic_cast<const usr*>(v);
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�Y���̒l";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    cerr << "�͐����^�ł���܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "subscript value ";
    if ( u )
      cerr << '`' << u->m_name << "' ";
    cerr << "is not integer.\n";
  }
  ++counter;
}

void cxx_compiler::error::declarations::redeclaration(const usr* prev, const usr* curr, bool param)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(curr->m_file,"�G���[");
    if ( param )
      cerr << "������ ";
    cerr << "`" << curr->m_name << "' ���Đ錾����Ă��܂�.\n";
    header(prev->m_file,"�G���[");
    cerr << "�����Ő錾����Ă��܂���.\n";
    break;
  default:
    header(curr->m_file,"error");
    if ( param )
      cerr << "parameter ";
    cerr << "redeclaration of `" << curr->m_name << "'\n";
    header(prev->m_file,"error");
    cerr << "previous declaration is here.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::not_object(const usr* entry, const type* T)
{
  using namespace std;
  string name = entry->m_name;
  switch ( lang ){
  case jpn:
    header(entry->m_file,"�G���[");
    if ( tag* tag = T->get_tag() ){
      cerr << '`' << name << "' ���s���S�^�� ";
      cerr << '`' << tag::keyword(tag->m_kind) << ' ' << tag->m_name << "'";
      cerr << " �Ƃ��Đ錾����Ă��܂�.\n";
      header(tag->m_file.back(),"�G���[");
      cerr << '`' << tag::keyword(tag->m_kind) << ' ' << tag->m_name << "'" << " �͂����Ő錾����Ă��܂�.\n";
    }
    else {
      cerr << '`' << name << "' �� ";
      T->decl(cerr,"");
      cerr << " �Ƃ��Đ錾����Ă��܂�.\n";
    }
    break;
  default:
    header(entry->m_file,"error");
    if ( tag* tag = T->get_tag() ){
      cerr << '`' << name << "' declared as incomplete ";
      cerr << '`' << tag::keyword(tag->m_kind) << ' ' << tag->m_name << "'";
      cerr << '\n';
      header(tag->m_file.back(),"error");
      cerr << '`' << tag::keyword(tag->m_kind) << ' ' << tag->m_name << "'" << " declared here.\n";
    }
    else {
      cerr << '`' << name << "' declared as ";
      T->decl(cerr,"");
      cerr << ".\n";
    }
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::initializers::designator::invalid_dot(const file_t& file, const type* T)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�����o�w��q���^ `";
    T->decl(cerr,"");
    cerr << "' �ɓK�p����܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "member designator specified for `";
    T->decl(cerr,"");
    cerr << "'\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::initializers::designator::not_member(const usr* id, const record_type* rec, const usr* u)
{
  using namespace std;
  tag* T = rec->get_tag();
  switch ( lang ){
  case jpn:
    header(id->m_file,"�G���[");
    if ( u ){
      cerr << '`' << u->m_name << "' �̌^�� ";
      u->m_type->decl(cerr,"");
      cerr << " �ł��� ";
    }
    cerr << tag::keyword(T->m_kind) << ' ' << T->m_name;
    cerr << " �ɂ� `" << id->m_name << "' �Ƃ��������o�[�͂���܂���.\n";
    header(T->m_file.back(),"�G���[");
    cerr << tag::keyword(T->m_kind) << ' ' << T->m_name;
    cerr << " �͂����Ő錾����Ă��܂�.\n";
    break;
  default:
    header(id->m_file,"error");
    if ( u ){
      cerr << '`' << u->m_name << "' is ";
      u->m_type->decl(cerr,"");
      cerr << ", but ";
    }
    cerr << tag::keyword(T->m_kind) << ' ' << T->m_name;
    cerr << " has not member `" << id->m_name << "'.\n";
    header(T->m_file.back(),"error");
    cerr << tag::keyword(T->m_kind) << ' ' << T->m_name;
    cerr << " is declared here.\n";
  }
  ++counter;
}

void cxx_compiler::error::declarations::initializers::exceed(const usr* u)
{
  using namespace std;
  string name = u->m_name;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    if ( name[0] != '.' )
      cerr << '`' << name << "' ��";
    cerr << "�������w��q���v�f���𒴂��Ďw�肳��Ă��܂�.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "initializer";
    if ( name[0] != '.' )
      cerr << " of `" << name << "'";
    cerr << " exceeds elements.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::initializers::not_object(const usr* u)
{
  using namespace std;
  string name = u->m_name;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "�I�u�W�F�N�g�łȂ�";
    if ( name[0] == '.' )
      cerr << "����";
    else
      cerr << " `" << name << "' ";
    cerr << "�ɏ������w��q���w�肳��Ă��܂�.\n";
    break;
  default:
    header(u->m_file,"error");
    if ( name[0] == '.' )
      cerr << "initializer specified for none object type.\n";
    else
      cerr << " `" << name << "' doesn't have object type, which is specified initializer.\n";
      break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::initializers::invalid_assign(const file_t& file, const usr* u, bool discard)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    if ( u ){
      string name = u->m_name;
      cerr << "`" << name << "' �ɑ΂���";
    }
    cerr << "�������w��q���s���ł�.";
    if ( discard )
      cerr << " �C���q�������܂�.";
    cerr << '\n';
    break;
  default:
    header(file,"error");
    cerr << "invalid initializer";
    if ( u ){
      string name = u->m_name;
      cerr << " for `" << name << "'.";
    }
    if ( discard )
      cerr << " qualifier discarded.";
    cerr << '\n';
    break;
  }
  ++counter;
}

void cxx_compiler::error::statements::label::not_defined(std::string label, const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "���x�� `" << label << "' ���g�p����Ă��܂�����`����Ă��܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "label `" << label << "' is used but not defined.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::classes::redeclaration(const file_t& curr, const file_t& prev, std::string name)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(curr,"�G���[");
    cerr << "`" << name << "' ���Đ錾����Ă��܂�.\n";
    header(prev,"�G���[");
    cerr << "�����Ő錾����Ă��܂���.\n";
    break;
  default:
    header(curr,"error");
    cerr << "redeclaration of `" << name << "'\n";
    header(prev,"error");
    cerr << "previous declaration is here.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::declarators::function::definition::multiple(const file_t& curr, const usr* u)
{
  using namespace std;
  const file_t& prev = u->m_file;
  string name = u->m_name;
  switch ( lang ){
  case jpn:
    header(curr,"�G���[");
    cerr << '`' << name << "' �̔�����`���d�����Ă��܂�.\n";
    header(prev,"�G���[");
    cerr << "�����ɔ�����`������܂���.\n";
    break;
  default:
    header(curr,"error");
    cerr << "function definition `" << name << "' is redefined.\n";
    header(prev,"error");
    cerr << "previous function definition was here.\n";
    break;
  }
  ++counter;
}

void
cxx_compiler::error::declarations::declarators::function::definition::static_inline::nodef(const file_t& decl, usr::flag_t flag, std::string name, const file_t& use)
{
  using namespace std;
  switch (lang) {
  case jpn:
    header(decl,"�G���[");
    if (flag & usr::STATIC)
      cerr << "static ";
    if (flag & usr::INLINE)
      cerr << "inline ";
    cerr << "�Ȕ��� `" << name << "' �̔�����`������܂���.\n";
    header(use,"�G���[");
    cerr << "�����ŎQ�Ƃ���Ă��܂�.\n";
    break;
  default:
    header(decl,"error");
    cerr << "no function definition of static function `" << name << "'.\n";
    header(use,"error");
    cerr << "referenced here.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::declarators::vm::invalid_linkage(const usr* u)
{
  using namespace std;
  string name = u->m_name;
  const type* T = u->m_type;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "variably modified �� `";
    T->decl(cerr,name);
    cerr << "' ���O�������P�[�W�������Ă��܂�.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "variably modified type `";
    T->decl(cerr,name);
    cerr << "' has external linkage.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::declarators::array::variable_length::initializer(const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "variable length array `" << u->m_name << "' ������������Ă��܂�.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "variable length array `" << u->m_name << "' is initialized.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::declarators::array::variable_length::invalid_storage(const usr* u)
{
  using namespace std;
  string name = u->m_name;
  const file_t& file = u->m_file;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "variable length array ";
    if ( name.empty() )
      cerr << '`' << name << "' ";
    cerr << "�̋L���N���X���s���ł�.\n";
    break;
  default:
    header(file,"error");
    cerr << "variable length array ";
    if ( name.empty() )
      cerr << '`' << name << "' ";
    cerr << "is declared with invalid storage class.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::classes::bit_field::not_integer_bit(const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "�r�b�g�t�B�[���h";
    if ( !u->m_name.empty() )
      cerr << " `" << u->m_name << "' ";
    cerr << "�̃r�b�g�w�肪�����^�ł͂���܂���.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "specified bit ";
    if ( u->m_name.empty() )
      cerr << "of `" << u->m_name << "' ";
    cerr << "is not integer.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::classes::bit_field::not_constant(const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "�r�b�g�t�B�[���h";
    if ( !u->m_name.empty() )
      cerr << " `" << u->m_name << "' ";
    cerr << "�̃r�b�g�w�肪�萔�ł͂���܂���.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "specified bit ";
    if ( u->m_name.empty() )
      cerr << "of `" << u->m_name << "' ";
    cerr << "is not constant.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::classes::bit_field::negative(const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "�r�b�g�t�B�[���h";
    if ( !u->m_name.empty() )
      cerr << " `" << u->m_name << "' ";
    cerr << "�̃r�b�g�w�肪���ł�.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "specified bit ";
    if ( u->m_name.empty() )
      cerr << "of `" << u->m_name << "' ";
    cerr << "is negative.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::classes::bit_field::not_integer_type(const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "�����^�łȂ��r�b�g�t�B�[���h";
    if ( !u->m_name.empty() )
      cerr << " `" << u->m_name << "' ";
    cerr << "���錾����Ă��܂�.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "type of bit field";
    if ( !u->m_name.empty() )
      cerr << " `" << u->m_name << "' ";
    cerr << " is not integer.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::unary::size::bit_field(const file_t& file, const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "sizeof ���Z�q���r�b�g�t�B�[���h `" << u->m_name << "' �ɑ΂��Ďw�肳��Ă��܂�.\n";
    header(u->m_file,"�G���[");
    cerr << '`' << u->m_name << "' �͂����Ő錾����Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "sizeof operator specified for bit field `" << u->m_name << "'.\n";
    header(u->m_file,"error");
    cerr << '`' << u->m_name << "' declared here.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::initializers::with_extern(const usr* u)
{
  using namespace std;
  string name = u->m_name;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "`extern' �t���� `" << name << "' ������������Ă��܂�.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "`" << name << "' has both `extern' and initializer.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::initializers::no_ctor(const usr* u)
{
  using namespace std;
  string name = u->m_name;
  switch (lang) {
  case jpn:
    header(u->m_file,"�G���[");
    cerr << '`' << name << "' �ɑ΂���R���X�g���N�^������܂���.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "no constructor for `" << name << ".\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::primary::literal::character::invalid(const file_t& file, std::string name, const type* T)
{
  using namespace std;
  switch ( lang ){
#ifndef __GNUC__  // for garbled characters gcc version 7.3.0
  case jpn:
    header(file,"�G���[");
    cerr << "���� `" << name << "` �͌^ `";
    T->decl(cerr,"");
    cerr << "' �ŕ\���ł��܂���.\n";
    break;
#endif // __GNUC__ for garbled characters gcc version 7.3.0
  default:
    header(file,"error");
    cerr << "character constant `" << name << "` is invalid for `";
    T->decl(cerr,"");
    cerr << "'.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::primary::underscore_func::outside(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "`__func__' �������̊O���ŎQ�Ƃ���Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "`__func__' is reference outside of function definition.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::primary::underscore_func::declared(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
#ifndef __GNUC__
    header(file,"�G���[");
    cerr << "�\�񎯕ʎq `__func__' ���錾����Ă��܂�.\n";
    break;
#endif // __GNUC__
  default:
    header(file,"error");
    cerr << "reserved identifier `__func__' is declared.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::initializers::designator::invalid_subscripting(const file_t& file, const type* T)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�Y���w��q���^ `";
    T->decl(cerr,"");
    cerr << "' �ɓK�p����܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "subscripting designator specified for `";
    T->decl(cerr,"");
    cerr << "'\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::initializers::designator::not_integer(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�Y�������萔�ł͂���܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "subscripting designator is not integer.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::initializers::designator::not_constant(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�Y�����萔�ł͂���܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "subscripting designator is not constant.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::va::not_lvalue(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "va_start �̑����������Ӓl�������܂���.\n";
    break;
  case other:
    header(file,"error");
    cerr << "2nd expression of va_start is not lvalue.\n";
    break;
  }
}

void cxx_compiler::error::expressions::va::no_size(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "va_start �̑������̃T�C�Y�� 0 �ł�.\n";
    break;
  case other:
    header(file,"error");
    cerr << "sizeof 2nd expression is zero.\n";
    break;
  }
}

void cxx_compiler::error::expressions::va::invalid(std::string op, const file_t& file, const var* v)
{
  using namespace std;
  const usr* u = dynamic_cast<const usr*>(v);
  switch ( lang ){
#ifndef __GNUC__
  case jpn:
    header(file,"�G���[");
    cerr << op << " �̑��������ύX�\�ȃ|�C���^�ł͂���܂���.\n";
    break;
#endif // __GNUC__
  default:
    header(file,"error");
    cerr << "1st expression of " << op << " is not modifiable pointer.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::postfix::member::not_record(const file_t& file, const var* v)
{
  using namespace std;
  const usr* u = dynamic_cast<const usr*>(v);
  switch ( lang ){
  case jpn:
#ifndef __GNUC__
    header(file,"�G���[");
    cerr << "�\���̂łȂ�";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    else
      cerr << "����";
    cerr << "�ɑ΂��ă����o�[���Q�Ƃ���Ă��܂�.\n";
    break;
#endif // __GNUC__
  default:
    header(file,"error");
    cerr << "member is required for ";
    if ( u )
      cerr << '`' << u->m_name << "' ";
    cerr << "not structure or union.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::ppmm::not_lvalue(const file_t& file, bool plus, const var* v)
{
  using namespace std;
  string op = plus ? "`++'" : "`--'";
  const usr* u = dynamic_cast<const usr*>(v);
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << op << " ���Z�q�����Ӓl�łȂ�";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    else
      cerr << "��";
    cerr << "�ɓK�p����Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "operator " << op << " is specified for not lvalue ";
    if ( u )
      cerr << '`' << u->m_name << "'";
    else
      cerr << "expression";
    cerr << ".\n";
  }
  ++counter;
}

void cxx_compiler::error::expressions::ppmm::not_modifiable(const file_t& file, bool plus, const var* v)
{
  using namespace std;
  string op = plus ? "`++'" : "`--'";
  const usr* u = dynamic_cast<const usr*>(v);
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << op << " ���Z�q���ύX�ł��Ȃ�";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    else
      cerr << "��";
    cerr << "�ɓK�p����Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "operator " << op << " is specified for not modifiable ";
    if ( u )
      cerr << '`' << u->m_name << "'";
    else
      cerr << "expression";
    cerr << ".\n";
  }
  ++counter;
}

void cxx_compiler::error::expressions::ppmm::not_modifiable_lvalue(const file_t& file, bool plus, const type* T)
{
  using namespace std;
  string op = plus ? "`++'" : "`--'";
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << op << " ���Z�q���^ `";
    T->decl(cerr,"");
    cerr << " '�ɓK�p����Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "operator " << op << " is specified for `";
    T->decl(cerr,"");
    cerr << "'\n";
  }
  ++counter;
}

void cxx_compiler::error::expressions::ppmm::not_scalar(const file_t& file, bool plus, const var* v)
{
  using namespace std;
  string op = plus ? "`++'" : "`--'";
  const usr* u = dynamic_cast<const usr*>(v);
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << op << " ���Z�q���X�J���[�^�łȂ�";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    else
      cerr << "��";
    cerr << "�ɓK�p����Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "operator " << op << " is specified for not scalar ";
    if ( u )
      cerr << '`' << u->m_name << "'";
    else
      cerr << "expression";
    cerr << ".\n";
  }
  ++counter;
}

void cxx_compiler::error::expressions::ppmm::invalid_pointer(const file_t& file, bool plus, const pointer_type* pt)
{
  using namespace std;
  string op = plus ? "`++'" : "`--'";
  const type* T = pt->referenced_type();
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << op << " ���Z�q���I�u�W�F�N�g�łȂ��^ `";
    T->decl(cerr,"");
    cerr << "' �ւ̃|�C���^�ɓK�p����Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "operator " << op << " is specified for pointer to none-object type `";
    T->decl(cerr,"");
    cerr << "'.\n";
  }
  ++counter;
}

void cxx_compiler::error::expressions::unary::size::invalid(const file_t& file, const type* T)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "`sizeof' ���^ `";
    T->decl(cerr,"");
    cerr << "' �ɓK�p����Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "`sizeof' specified for `";
    T->decl(cerr,"");
    cerr << "'.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::statements::expression::incomplete_type(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�����̎��̌^���s���S�^�ł�.\n";
    break;
  default:
    header(file,"error");
    cerr << "expression statement has incomplete type.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::statements::return_stmt::invalid(const file_t& file, const type* from, const type* to)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "`";
    from->decl(cerr,"");
    cerr << "' ���� `";
    to->decl(cerr,"");
    cerr << "' �֕ϊ��ł��܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "invalid conversion from `";
    from->decl(cerr,"");
    cerr << "' to `";
    to->decl(cerr,"");
    cerr << "'.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::postfix::member::not_pointer(const file_t& file, const var* v)
{
  using namespace std;
  const usr* u = dynamic_cast<const usr*>(v);
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "-> ���|�C���^�łȂ�";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    else
      cerr << "����";
    cerr << "�ɑ΂��Ďw�肳��Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "-> specified for non-pointer type";
    if ( u )
      cerr << " for `" << u->m_name << "'";
    cerr << ".\n";
    break;
  }
  ++counter;
}

void
cxx_compiler::error::expressions::postfix::fcast::too_many_arg(const file_t& file)
{
  using namespace std;
  switch (lang) {
  case jpn:
    header(file,"�G���[");
    cerr << "�����X�^�C���̃L���X�g�ɑ΂��Ĉ������������܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "Too many arguments for function style cast.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::cast::not_scalar(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�L���X�g�̌^���X�J���[�ł͂���܂���.\n";
    break;
  default:
    header(file,"error");
    cerr << "cast type is not scalar.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::expressions::cast::invalid(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�s���ȃL���X�g�ł�.\n";
    break;
  default:
    header(file,"error");
    cerr << "invalid cast expression.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::empty(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    error::header(file,"�G���[");
    cerr << "��̐錾�ł�.\n";
    break;
  default:
    error::header(file,"error");
    cerr << "empty declaration.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::storage_class::multiple(const file_t& file, usr::flag_t x, usr::flag_t y)
{
  using namespace std;
  x = usr::flag_t(x & ~usr::INLINE);
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�L���N���X `";
    cerr << usr::keyword(x);
    cerr << "' �� `";
    cerr << usr::keyword(y);
    cerr << "' ���w�肳��Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "multiple storage class specifier `";
    cerr << usr::keyword(x);
    cerr << "' and `";
    cerr << usr::keyword(y);
    cerr << "' specified.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::declarators::function::definition::invalid(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�p�����[�^�X�R�[�v�̂Ȃ��֐���`�ł�.\n";
    break;
  default:
    header(file,"error");
    cerr << "function definition without parameter scope.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::declarators::function::invalid_storage(const usr* u)
{
  using namespace std;
  string name = u->m_name;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "�u���b�N�X�R�[�v�̔��� `" << name << "' �� `extern' �ȊO�̋L���N���X�𔺂��Đ錾����܂���.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "block scope function `" << name << "' is declared with storage class other than `extern'.\n";
    break;
  }
  ++counter;
}

void
cxx_compiler::error::declarations::enumeration::redeclaration(const file_t& curr, const file_t& prev, std::string name)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(curr,"�G���[");
    cerr << "`" << name << "' ���Đ錾����Ă��܂�.\n";
    header(prev,"�G���[");
    cerr << "�����Ő錾����Ă��܂���.\n";
    break;
  default:
    header(curr,"error");
    cerr << "redeclaration of `" << name << "'\n";
    header(prev,"error");
    cerr << "previous declaration is here.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::specifier_seq::function::not_function(const usr* u)
{
  using namespace std;
  string name = u->m_name;
  const cxx_compiler::type* T = u->m_type;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "`inline' ���^ `";
    T->decl(cerr,"");
    cerr << "' �� `" << name << "' �Ɏw�肳��Ă��܂�.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "`inline' specified for `" << name << "' whose type is `";
    T->decl(cerr,"");
    cerr << ".\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::specifier_seq::function::func_spec::main(const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "`inline' �� `main' �Ɏw�肳��Ă��܂�.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "`inline' specified for `main'.\n";
    cerr << ".\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::specifier_seq::function::func_spec::static_storage(const usr* u)
{
  using namespace std;
  string func = fundef::current->m_usr->m_name;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�G���[");
    cerr << "static storage duration ������ `" << u->m_name << "' �� inline ���� `" << func;
    cerr << "' �Œ�`����Ă��܂�.\n";
    break;
  default:
    header(u->m_file,"error");
    cerr << "`" << u->m_name << "' is defined in inline function `" << func;
    cerr << "', which has static storage duration.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::specifier_seq::function::func_spec::internal_linkage(const file_t& file, const usr* u)
{
  using namespace std;
  string func = fundef::current->m_usr->m_name;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "���������P�[�W������ `" << u->m_name << "' �� inline ���� `" << func;
    cerr << "' �ŎQ�Ƃ���Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "`" << u->m_name << "' is referenced in inline function `" << func;
    cerr << "', which has internal linkage.\n";
    break;
  }
  ++counter;
}

void
cxx_compiler::error::declarations::declarators::function::parameter::invalid_storage(const file_t& file, const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�p�����[�^";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    cerr << "�̐錾�� register �ȊO�̋L���N���X���w�肳��Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "parameter ";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    cerr << "is specified storage class specifier except for register.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::statements::goto_stmt::invalid(const file_t& go, const file_t& lab, const usr* u)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(go,"�G���[");
    cerr << "goto ����.\n";
    header(lab,"�G���[");
    cerr << "�����ɃW�����v���܂���,\n";
    header(u->m_file,"�G���[");
    cerr << "`" << u->m_name << "' �������Ő錾����Ă��܂�.\n";
    break;
  default:
    header(go,"error");
    cerr << "goto statement jumps to\n";
    header(lab,"error");
    cerr << "here,\n";
    header(u->m_file,"error");
    cerr << "but `" << u->m_name << "' is declared here.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::statements::switch_stmt::invalid(bool _case, const file_t& file, const usr* u)
{
  using namespace std;
  string s = _case ? "case" : "default";
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << s << " ���x���ւ̃W�����v���\n";
    header(u->m_file,"�G���[");
    cerr << "`" << u->m_name << "' ���錾����Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "jump to " << s << " label in the block, where\n";
    header(u->m_file,"error");
    cerr << "`" << u->m_name << "' is declared.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::statements::for_stmt::invalid_storage(const usr* u)
{
  using namespace std;
  const file_t& file = u->m_file;
  string name = u->m_name;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "`for' �ɂ����� `" << name << "' �̐錾�� auto, register �ȊO�̋L���N���X���w�肳��Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "declaration of `" << name << "' in `for' part has storage class specifier except for auto or register.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::external::invalid_storage(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "�t�@�C���X�R�[�v�ŋL���N���X�� auto, register �̐錾�ł�.\n";
    break;
  default:
    header(file,"error");
    cerr << "invalid storage class specifier declaration in file scope.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::declarators::function::definition::invalid_return(const usr* u, const type* T)
{
  using namespace std;
  const file_t& file = u->m_file;
  string name = u->m_name;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << '`' << name << "' �̔�����`�Ŗ߂�l�̌^�� `";
    T->decl(cerr,"");
    cerr << "' �ł�.\n";
    break;
  default:
    header(file,"error");
    cerr << "In function definition `" << name << "', return type is `";
    T->decl(cerr,"");
    cerr << "'.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::error::declarations::declarators::function::definition::typedefed(const file_t& file)
{
  using namespace std;
  switch ( lang ){
  case jpn:
    header(file,"�G���[");
    cerr << "������`�� typedef ����Ă��܂�.\n";
    break;
  default:
    header(file,"error");
    cerr << "function definition is typedefed.\n";
    break;
  }
  ++counter;
}
