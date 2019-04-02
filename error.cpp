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
  case jpn: cerr << prog << " : " << fn << " を開けません.\n"; break;
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
    cerr << prog << ": ターゲットジェネレータが指定されていません.\n";
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
    error::header(parse::position,"エラー");
    if (*cxx_compiler_text) {
      cerr << '`';
      istringstream ist(cxx_compiler_text); string s; ist >> s; cerr << s;
      cerr << "' で";
    }
    cerr << "構文エラーです.";
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
        cerr << file.m_name << ": 函数 `" << name << "' :\n";
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
    header(file,"エラー");
    cerr << '`' << name << "' は宣言されていません.\n";
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
    header(file,"エラー");
    cerr << '`';
    x->decl(cerr,"");
    cerr << "' と `";
    y->decl(cerr,"");
    cerr << "' が指定されています.\n";
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
    header(file,"エラー");
    cerr << '`';
    T->decl(cerr,"");
    cerr << "' に `" << spec << "' が指定されています.\n";
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
    header(file,"エラー");
    cerr << '`' << x << "' と `" << y << "' が指定されています.\n";
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
    error::header(u->m_file,"エラー");
    cerr << "`" << u->m_name << "' の宣言で `int' が仮定されました.\n";
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
    error::header(file,"エラー");
    cerr << "仮引数の宣言で `int' が仮定されました.\n";
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
    header(file,"エラー");
    cerr << '`';
    T->decl(cerr,"");
    cerr << "' が `restrict' によって修飾されています.\n";
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
      header(u->m_file,"エラー");
      string name = u->m_name;
      cerr << '`' << name << "' が函数を返す函数と宣言されています.\n";
    }
    else {
      header(file,"エラー");
      cerr << "函数を返す函数が宣言されています.\n";
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
      header(u->m_file,"エラー");
      string name = u->m_name;
      cerr << '`' << name << "' が配列を返す函数と宣言されています.\n";
    }
    else {
      header(file,"error");
      cerr << "配列を返す函数が宣言されています.\n";
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
      header(u->m_file,"エラー");
      string name = u->m_name;
      cerr << '`' << name << "' が函数の配列と宣言されています.\n";
    }
    else {
      header(file,"error");
      cerr << "函数の配列が宣言されています.\n";
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
      header(u->m_file,"エラー");
      string name = u->m_name;
      cerr << "配列 `" << name << "' の次元が整数型ではありません.\n";
    }
    else {
      header(file,"error");
      cerr << "配列の次元が整数型ではありません.\n";
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
      header(u->m_file,"エラー");
      string name = u->m_name;
      cerr << "配列 `" << name << "' の次元が正数ではありません.\n";
    }
    else {
      header(file,"error");
      cerr << "配列の次元が正数ではありません.\n";
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
      header(u->m_file,"エラー");
      string name = u->m_name;
      cerr << '`' << name << "' の次元に `*' が指定されています.\n";
    }
    else {
      header(file,"error");
      cerr << "函数の次元に `*' が指定されています.\n";
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
    header(u->m_file,"エラー");
    cerr << "variably modified な `";
    cerr << name;
    cerr << "' がファイルスコープで宣言されています.\n";
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
    header(u->m_file,"エラー");
    cerr << "`";
    cerr << name;
    cerr << "' の初期化指定子がありません.\n";
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
    header(u->m_file,"エラー");
    cerr << "ビットフィールド `" << u->m_name << "' のビット数にゼロが指定されています.\n";
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
    header(u->m_file,"エラー");
    cerr << "ビットフィールド";
    if ( !u->m_name.empty() )
      cerr << " `" << u->m_name << "' ";
    cerr << "のビット数が ";
    T->decl(cerr,"");
    cerr << " のサイズを超えています.\n";
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
    header(u->m_file,"エラー");
    T->decl(cerr,name);
    cerr << " がメンバとして宣言されています.\n";
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
    header(u->m_file,"エラー");
    cerr << "variably modified な `";
    T->decl(cerr,name);
    cerr << "' がメンバとして宣言されています.\n";
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
    header(file,"エラー");
    cerr << "`" << name << "' が `";
    T->decl(cerr,"");
    cerr << "' の範囲を超えています.\n";
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
    header(file,"エラー");
    cerr << "函数ではない";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    else
      cerr << "もの";
    cerr << "を呼び出しました.\n";
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
    header(file,"エラー");
    cerr << "函数呼び出し";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    cerr << "の引数は " << n << " 個が指定されていますが " << m << " 個が必要です.\n";
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
    header(file,"エラー");
    cerr << "函数";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    cerr << "の戻り値はオブジェクトではありません.\n";
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
    header(file,"エラー");
    cerr << "函数";
    if ( u )
      cerr << " `" << u->m_name << "' の";
    cerr << "呼び出しの " << n + 1 << " 番目の引数がマッチしません.";
    if ( discard )
      cerr << " 修飾子が失われます.";
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
    header(file,"エラー");
    cerr << "函数" << " `" << u->m_name << "' の";
    cerr << "オーバーロード呼び出しがいずれもマッチしません.";
    cerr << '\n';
    cerr << "候補:" << '\n';
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
    header(file,"エラー");
    cerr << "二項演算子 `" << m_table[op] << "' が型 `";
    y->decl(cerr,"");
    cerr << "' と型 `";
    z->decl(cerr,"");
    cerr << "' に適用されました.\n";
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
    header(file,"エラー");
    cerr << "代入演算子の左のオペランドが左辺値を持ちません.\n";
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
    header(file,"エラー");
    cerr << "代入演算子の左オペランド";
    if ( !name.empty() )
      cerr << " `" << name << "' ";
    cerr << "は変更できません.\n";
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
    header(file,"エラー");
    if ( u ){
      string name = u->m_name;
      cerr << "`" << name << "' に対する";
    }
    cerr << "代入演算子が不正です.";
    if ( discard )
      cerr << " 修飾子が失われます.";
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
    header(file,"エラー");
    cerr << "代入演算子が型 `";
    T->decl(cerr,"");
    cerr << "' に適用されています.\n";
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
    header(file,"エラー");
    cerr << "case ラベル";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    cerr << "が定数ではありません.\n";
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
    header(file,"エラー");
    cerr << "case ラベル";
    if ( !name.empty() )
      cerr << " `" << name << "' ";
    cerr << "が整数ではありません.\n";
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
    header(file,"エラー");
    cerr << "case ラベル が switch の外で使用されています.\n";
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
    header(curr,"エラー");
    cerr << "case ラベル が重複して指定されています.\n";
    header(prev,"エラー");
    cerr << "この case ラベルと重複しています.\n";
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
    header(file,"エラー");
    cerr << "default が switch の外で使用されています.\n";
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
    header(curr,"エラー");
    cerr << "default が 2 つあります.\n";
    header(prev,"エラー");
    cerr << "ここに default がありました.\n";
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
    header(file,"エラー");
    cerr << "if の式";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    cerr << "がスカラー型ではありません.\n";
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
    header(file,"エラー");
    cerr << "switch の式";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    cerr << "が整数ではありません.\n";
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
    header(file,"エラー");
    cerr << "break 文がループの中でも switch 文の中でもないところに出現しました.\n";
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
    header(file,"エラー");
    cerr << "continue 文がループの中でないところに出現しました.\n";
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
    header(file,"エラー");
    cerr << "while 式がスカラー型でありません.\n";
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
    header(file,"エラー");
    cerr << "`for' の第 2 式がスカラー型でありません.\n";
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
    header(u->m_file,"エラー");
    cerr << "列挙のメンバ `" << u->m_name << "' の値が定数ではありません.\n";
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
    header(u->m_file,"エラー");
    cerr << "列挙のメンバ `" << u->m_name << "' の値が整数ではありません.\n";
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
    header(curr,"エラー");
    cerr << "ラベル `" << label << "' が多重に定義されています.\n";
    header(prev,"エラー");
    cerr << "ここで定義されていました.\n";
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
    header(file,"エラー");
    cerr << "単項の `&' 演算子が左辺値でない式に適用されています.\n";
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
    header(file,"エラー");
    cerr << "単項の & 演算子がビットフィールド `" << u->m_name << "' に対して指定されています.\n";
    header(u->m_file,"エラー");
    cerr << '`' << u->m_name << "' はここで宣言されています.\n";
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
    header(file,"エラー");
    cerr << "単項の * がポインタでない型の式に指定されています.\n";
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
    header(file,"エラー");
    cerr << "単項の `" << char(op) << "' が型 `";
    T->decl(cerr,"");
    cerr << "' に適用されています.\n";
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
    header(file,"エラー");
    cerr << "3 項演算子の第 1 式の型がスカラーではありません.\n";
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
    header(file,"エラー");
    cerr << "3 項演算子の第 2 式と第 3 式の型が不正です.\n";
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
    header(file,"エラー");
    cerr << "添字演算子を配列でもポインタでもない";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    else
      cerr << "もの";
    cerr << "に使用しています.\n";
    if ( u ){
      header(u->m_file,"エラー");
      cerr << '`' << u->m_name << "' はここで宣言されています.\n";
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
    header(file,"エラー");
    cerr << "添字参照された値が ";
    pointer_type::create(T)->decl(cerr,"");
    cerr << " です.\n";
    if ( tag* tag = T->get_tag() ){
      header(tag->m_file.back(),"エラー");
      cerr << '`' << tag::keyword(tag->m_kind) << ' ' << tag->m_name << "'";
      cerr << " はここで宣言されています.\n";
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
    header(file,"エラー");
    cerr << "添字の値";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    cerr << "は整数型でありません.\n";
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
    header(curr->m_file,"エラー");
    if ( param )
      cerr << "仮引数 ";
    cerr << "`" << curr->m_name << "' が再宣言されています.\n";
    header(prev->m_file,"エラー");
    cerr << "ここで宣言されていました.\n";
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
    header(entry->m_file,"エラー");
    if ( tag* tag = T->get_tag() ){
      cerr << '`' << name << "' が不完全型の ";
      cerr << '`' << tag::keyword(tag->m_kind) << ' ' << tag->m_name << "'";
      cerr << " として宣言されています.\n";
      header(tag->m_file.back(),"エラー");
      cerr << '`' << tag::keyword(tag->m_kind) << ' ' << tag->m_name << "'" << " はここで宣言されています.\n";
    }
    else {
      cerr << '`' << name << "' が ";
      T->decl(cerr,"");
      cerr << " として宣言されています.\n";
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
    header(file,"エラー");
    cerr << "メンバ指定子が型 `";
    T->decl(cerr,"");
    cerr << "' に適用されました.\n";
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
    header(id->m_file,"エラー");
    if ( u ){
      cerr << '`' << u->m_name << "' の型は ";
      u->m_type->decl(cerr,"");
      cerr << " ですが ";
    }
    cerr << tag::keyword(T->m_kind) << ' ' << T->m_name;
    cerr << " には `" << id->m_name << "' というメンバーはありません.\n";
    header(T->m_file.back(),"エラー");
    cerr << tag::keyword(T->m_kind) << ' ' << T->m_name;
    cerr << " はここで宣言されています.\n";
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
    header(u->m_file,"エラー");
    if ( name[0] != '.' )
      cerr << '`' << name << "' の";
    cerr << "初期化指定子が要素数を超えて指定されています.\n";
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
    header(u->m_file,"エラー");
    cerr << "オブジェクトでない";
    if ( name[0] == '.' )
      cerr << "もの";
    else
      cerr << " `" << name << "' ";
    cerr << "に初期化指定子が指定されています.\n";
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
    header(file,"エラー");
    if ( u ){
      string name = u->m_name;
      cerr << "`" << name << "' に対する";
    }
    cerr << "初期化指定子が不正です.";
    if ( discard )
      cerr << " 修飾子が失われます.";
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
    header(file,"エラー");
    cerr << "ラベル `" << label << "' が使用されていますが定義されていません.\n";
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
    header(curr,"エラー");
    cerr << "`" << name << "' が再宣言されています.\n";
    header(prev,"エラー");
    cerr << "ここで宣言されていました.\n";
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
    header(curr,"エラー");
    cerr << '`' << name << "' の函数定義が重複しています.\n";
    header(prev,"エラー");
    cerr << "ここに函数定義がありました.\n";
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
    header(decl,"エラー");
    if (flag & usr::STATIC)
      cerr << "static ";
    if (flag & usr::INLINE)
      cerr << "inline ";
    cerr << "な函数 `" << name << "' の函数定義がありません.\n";
    header(use,"エラー");
    cerr << "ここで参照されています.\n";
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
    header(u->m_file,"エラー");
    cerr << "variably modified な `";
    T->decl(cerr,name);
    cerr << "' が外部リンケージを持っています.\n";
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
    header(u->m_file,"エラー");
    cerr << "variable length array `" << u->m_name << "' が初期化されています.\n";
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
    header(file,"エラー");
    cerr << "variable length array ";
    if ( name.empty() )
      cerr << '`' << name << "' ";
    cerr << "の記憶クラスが不正です.\n";
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
    header(u->m_file,"エラー");
    cerr << "ビットフィールド";
    if ( !u->m_name.empty() )
      cerr << " `" << u->m_name << "' ";
    cerr << "のビット指定が整数型ではありません.\n";
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
    header(u->m_file,"エラー");
    cerr << "ビットフィールド";
    if ( !u->m_name.empty() )
      cerr << " `" << u->m_name << "' ";
    cerr << "のビット指定が定数ではありません.\n";
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
    header(u->m_file,"エラー");
    cerr << "ビットフィールド";
    if ( !u->m_name.empty() )
      cerr << " `" << u->m_name << "' ";
    cerr << "のビット指定が負です.\n";
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
    header(u->m_file,"エラー");
    cerr << "整数型でないビットフィールド";
    if ( !u->m_name.empty() )
      cerr << " `" << u->m_name << "' ";
    cerr << "が宣言されています.\n";
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
    header(file,"エラー");
    cerr << "sizeof 演算子がビットフィールド `" << u->m_name << "' に対して指定されています.\n";
    header(u->m_file,"エラー");
    cerr << '`' << u->m_name << "' はここで宣言されています.\n";
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
    header(u->m_file,"エラー");
    cerr << "`extern' 付きの `" << name << "' が初期化されています.\n";
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
    header(u->m_file,"エラー");
    cerr << '`' << name << "' に対するコンストラクタがありません.\n";
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
    header(file,"エラー");
    cerr << "文字 `" << name << "` は型 `";
    T->decl(cerr,"");
    cerr << "' で表現できません.\n";
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
    header(file,"エラー");
    cerr << "`__func__' が函数の外側で参照されています.\n";
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
    header(file,"エラー");
    cerr << "予約識別子 `__func__' が宣言されています.\n";
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
    header(file,"エラー");
    cerr << "添字指定子が型 `";
    T->decl(cerr,"");
    cerr << "' に適用されました.\n";
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
    header(file,"エラー");
    cerr << "添字が整定数ではありません.\n";
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
    header(file,"エラー");
    cerr << "添字が定数ではありません.\n";
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
    header(file,"エラー");
    cerr << "va_start の第二引数が左辺値を持ちません.\n";
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
    header(file,"エラー");
    cerr << "va_start の第二引数のサイズが 0 です.\n";
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
    header(file,"エラー");
    cerr << op << " の第一引数が変更可能なポインタではありません.\n";
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
    header(file,"エラー");
    cerr << "構造体でない";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    else
      cerr << "もの";
    cerr << "に対してメンバーが参照されています.\n";
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
    header(file,"エラー");
    cerr << op << " 演算子が左辺値でない";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    else
      cerr << "式";
    cerr << "に適用されています.\n";
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
    header(file,"エラー");
    cerr << op << " 演算子が変更できない";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    else
      cerr << "式";
    cerr << "に適用されています.\n";
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
    header(file,"エラー");
    cerr << op << " 演算子が型 `";
    T->decl(cerr,"");
    cerr << " 'に適用されています.\n";
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
    header(file,"エラー");
    cerr << op << " 演算子がスカラー型でない";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    else
      cerr << "式";
    cerr << "に適用されています.\n";
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
    header(file,"エラー");
    cerr << op << " 演算子がオブジェクトでない型 `";
    T->decl(cerr,"");
    cerr << "' へのポインタに適用されています.\n";
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
    header(file,"エラー");
    cerr << "`sizeof' が型 `";
    T->decl(cerr,"");
    cerr << "' に適用されています.\n";
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
    header(file,"エラー");
    cerr << "式文の式の型が不完全型です.\n";
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
    header(file,"エラー");
    cerr << "`";
    from->decl(cerr,"");
    cerr << "' から `";
    to->decl(cerr,"");
    cerr << "' へ変換できません.\n";
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
    header(file,"エラー");
    cerr << "-> がポインタでない";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    else
      cerr << "もの";
    cerr << "に対して指定されています.\n";
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
    header(file,"エラー");
    cerr << "函数スタイルのキャストに対して引数が多すぎます.\n";
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
    header(file,"エラー");
    cerr << "キャストの型がスカラーではありません.\n";
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
    header(file,"エラー");
    cerr << "不正なキャストです.\n";
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
    error::header(file,"エラー");
    cerr << "空の宣言です.\n";
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
    header(file,"エラー");
    cerr << "記憶クラス `";
    cerr << usr::keyword(x);
    cerr << "' と `";
    cerr << usr::keyword(y);
    cerr << "' が指定されています.\n";
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
    header(file,"エラー");
    cerr << "パラメータスコープのない関数定義です.\n";
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
    header(u->m_file,"エラー");
    cerr << "ブロックスコープの函数 `" << name << "' が `extern' 以外の記憶クラスを伴って宣言されました.\n";
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
    header(curr,"エラー");
    cerr << "`" << name << "' が再宣言されています.\n";
    header(prev,"エラー");
    cerr << "ここで宣言されていました.\n";
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
    header(u->m_file,"エラー");
    cerr << "`inline' が型 `";
    T->decl(cerr,"");
    cerr << "' の `" << name << "' に指定されています.\n";
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
    header(u->m_file,"エラー");
    cerr << "`inline' が `main' に指定されています.\n";
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
    header(u->m_file,"エラー");
    cerr << "static storage duration を持つ `" << u->m_name << "' が inline 函数 `" << func;
    cerr << "' で定義されています.\n";
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
    header(file,"エラー");
    cerr << "内部リンケージを持つ `" << u->m_name << "' が inline 函数 `" << func;
    cerr << "' で参照されています.\n";
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
    header(file,"エラー");
    cerr << "パラメータ";
    if ( u )
      cerr << " `" << u->m_name << "' ";
    cerr << "の宣言に register 以外の記憶クラスが指定されています.\n";
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
    header(go,"エラー");
    cerr << "goto 文で.\n";
    header(lab,"エラー");
    cerr << "ここにジャンプしますが,\n";
    header(u->m_file,"エラー");
    cerr << "`" << u->m_name << "' がここで宣言されています.\n";
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
    header(file,"エラー");
    cerr << s << " ラベルへのジャンプ先で\n";
    header(u->m_file,"エラー");
    cerr << "`" << u->m_name << "' が宣言されています.\n";
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
    header(file,"エラー");
    cerr << "`for' における `" << name << "' の宣言で auto, register 以外の記憶クラスが指定されています.\n";
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
    header(file,"エラー");
    cerr << "ファイルスコープで記憶クラスが auto, register の宣言です.\n";
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
    header(file,"エラー");
    cerr << '`' << name << "' の函数定義で戻り値の型が `";
    T->decl(cerr,"");
    cerr << "' です.\n";
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
    header(file,"エラー");
    cerr << "函数定義が typedef されています.\n";
    break;
  default:
    header(file,"error");
    cerr << "function definition is typedefed.\n";
    break;
  }
  ++counter;
}
