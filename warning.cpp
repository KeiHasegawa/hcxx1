#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

int cxx_compiler::warning::counter;

void cxx_compiler::warning::cmdline::option(std::string option)
{
  using namespace std;
  using namespace error;
  string prog = cxx_compiler::cmdline::prog;
  switch ( lang ){
  case jpn:
    cerr << prog << ": `" << option << "' �͖����ȃI�v�V�����ł�.\n";
    break;
  default:
    cerr << prog << ": unknown option `" << option << "'.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::warning::cmdline::input(std::string fn)
{
  using namespace std;
  using namespace error;
  string prog = cxx_compiler::cmdline::prog;
  switch ( lang ){
  case jpn:
    cerr << prog << ": �����̓��̓t�@�C�����w�肳��Ă��܂�. `" << fn << "' �͖�������܂�.\n";
    break;
  default:
    cerr << prog << ": multiple input files `" << fn << "' specified.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::warning::cmdline::optimize_option()
{
  using namespace std;
  using namespace error;
  string prog = cxx_compiler::cmdline::prog;
  switch ( lang ){
  case jpn:
    cerr << prog << ": --optimize �I�v�V�����ɂ� 0 �� 1 �̈������K�v�ł�.\n";
    break;
  default:
    cerr << prog << ": --optimize option requires argument 0 or 1.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::warning::generator::seed(std::string fn)
{
  using namespace std;
  using namespace error;
  string prog = cxx_compiler::cmdline::prog;
  switch ( lang ){
  case jpn:
    cerr << prog << ": `generator_seed' �� " << fn << " ���猩����܂���.\n";
    break;
  default:
    cerr << prog << ": cannot find symbol `generator_seed' from " << fn << ".\n";
    break;
  }
  ++counter;
}

void cxx_compiler::warning::generator::seed(std::string fn, std::pair<int,int> p)
{
  using namespace std;
  using namespace error;
  string prog = cxx_compiler::cmdline::prog;
  switch ( lang ){
  case jpn:
    cerr << prog << ": " << fn << " �� `generator_seed' ���� ";
    cerr << p.first << " ���Ԃ�̂����҂���܂���, " << p.second << " ���Ԃ�܂���.\n";
    break;
  default:
    cerr << prog << ": unexpected return value " << p.second << "from `generator_seed' of " << fn << ".\n";
    cerr << prog << ": expected return value " << p.first << ".\n";
    break;
  }
  ++counter;
}

void cxx_compiler::warning::cmdline::generator_option()
{
  using namespace std;
  using namespace error;
  string prog = cxx_compiler::cmdline::prog;
  switch ( lang ){
  case jpn:
    cerr << prog << ": --generator �I�v�V�����͈������K�v�ł�.\n";
    break;
  default:
    cerr << prog << ": --generator option require argument" << '\n';
    break;
  }
  ++counter;
}

void cxx_compiler::warning::cmdline::generator_option_option(std::string parenthsis)
{
  using namespace std;
  using namespace error;
  string prog = cxx_compiler::cmdline::prog;
  switch ( lang ){
  case jpn:
    if ( parenthsis == "(" )
      cerr << prog << ": --generator_option �I�v�V�����ɂ� '(' ���K�v�ł�.\n";
    else
      cerr << prog << ": --generator_option �I�v�V������ ')' ������܂���.\n";
    break;
  default:
    if ( parenthsis == "(" )
      cerr << prog << ": --generator_option require '('.\n";
    else
      cerr << prog << ": no ')' in --generator_option.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::warning::cmdline::o_option()
{
  using namespace std;
  string prog = cxx_compiler::cmdline::prog;
  cerr << prog << ": --o_option require argument.\n";
  ++counter;
}

void cxx_compiler::warning::generator::open(std::string fn)
{
  using namespace std;
  using namespace error;
  string prog = cxx_compiler::cmdline::prog;
  struct stat buff;
  switch ( lang ){
  case jpn:
    if ( stat(fn.c_str(),&buff) )
      cerr << prog << ": `" << fn << "' ���J���܂���.\n";
    else
      cerr << prog << ": `" << fn << "' ���_�C�i�~�b�N�����N���C�u�����Ƃ��ĊJ���܂���.\n";
#ifdef unix
    cerr << dlerror() << '\n';
#endif // unix
    break;
  default:
    if ( stat(fn.c_str(),&buff) )
      cerr << prog << ": cannot open `" << fn << "'.\n";
    else
      cerr << prog << ": cannot open `" << fn << "' as a dynamic link library.\n";
#ifdef unix
    cerr << dlerror() << '\n';
#endif // unix
    break;
  }
  ++counter;
}

void cxx_compiler::warning::generator::option(std::string fn)
{
  using namespace std;
  using namespace error;
  string prog = cxx_compiler::cmdline::prog;
  switch ( lang ){
  case jpn:
    cerr << prog << ": `generator_option' �� " << fn << " ���猩����܂���.\n";
    break;
  default:
    cerr << prog << ": cannot find symbol `generator_option' from " << fn << ".\n";
    break;
  }
  ++counter;
}

void cxx_compiler::warning::generator::option(std::string option, int n, std::string fn)
{
  using namespace std;
  using namespace error;
  string prog = cxx_compiler::cmdline::prog;
  switch ( lang ){
  case jpn:
    cerr << prog << ": `" << option << "' �� " << fn << " �ŃG���[�ɂȂ�܂���. �G���[�R�[�h " << n << ".\n";
    break;
  default:
    cerr << prog << ": `" << option << "' option is error code " << n << " in " << fn << ".\n";
    break;
  }
  ++counter;
}

void cxx_compiler::warning::generator::open_file(std::string fn)
{
  using namespace std;
  using namespace error;
  string prog = cxx_compiler::cmdline::prog;
  switch ( lang ){
  case jpn:
    cerr << prog << ": `generator_open_file' �� " << fn << " ���猩����܂���.\n";
    break;
  default:
    cerr << prog << ": cannot find symbol `generator_open_file' from " << fn << ".\n";
    break;
  }
  ++counter;
}

void cxx_compiler::warning::generator::generate(std::string fn)
{
  using namespace std;
  using namespace error;
  string prog = cxx_compiler::cmdline::prog;
  switch ( lang ){
  case jpn:
    cerr << prog << ": `generator_generate' �� " << fn << " ���猩����܂���.\n";
    break;
  default:
    cerr << prog << ": cannot find symbol `generator_generate' from " << fn << ".\n";
    break;
  }
  ++counter;
}

void cxx_compiler::warning::generator::close_file(std::string fn)
{
  using namespace std;
  using namespace error;
  string prog = cxx_compiler::cmdline::prog;
  switch ( lang ){
  case jpn:
    cerr << prog << ": `generator_close_file' �� " << fn << " ���猩����܂���.\n";
    break;
  default:
    cerr << prog << ": cannot find symbol `generator_close_file' from " << fn << ".\n";
    break;
  }
  ++counter;
}

void cxx_compiler::warning::cmdline::lang_option(std::string arg)
{
  using namespace std;
  using namespace error;
  string prog = cxx_compiler::cmdline::prog;
  switch ( lang ){
  case jpn:
    cerr << prog << ": --lang �� `" << arg << "' ���w�肳��܂�����, �T�|�[�g���Ă��܂���.\n";
    break;
  default:
    cerr << prog << ": for --lang, specified `" << arg << "', but not supported.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::warning::cmdline::lang_option()
{
  using namespace std;
  using namespace error;
  string prog = cxx_compiler::cmdline::prog;
  switch ( lang ){
  case jpn:
    cerr << prog << ": --lang �I�v�V�����ɂ͈������K�v�ł�.\n";
    break;
  default:
    cerr << prog << ": --lang option require argument.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::warning::declarations::initializers::with_extern(const usr* u)
{
  using namespace std;
  using namespace error;
  string name = u->m_name;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�x��");
    cerr << "`extern' �t���� `" << name << "' ������������Ă��܂�.\n";
    break;
  default:
    header(u->m_file,"warning");
    cerr << "`" << name << "' has both `extern' and initializer.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::warning::zero_division(const file_t& file)
{
  using namespace std;
  using namespace error;
  switch ( lang ){
  case jpn:
    header(file,"�x��");
    cerr << "�[���ɂ�鏜�Z�����n����܂���.\n";
    break;
  default:
    header(file,"warning");
    cerr << "zero division detected.\n";
    break;
  }
  ++counter;
}

void cxx_compiler::warning::undefined_reference(const usr* u)
{
  using namespace std;
  using namespace error;
  string name = u->m_name;
  switch ( lang ){
  case jpn:
    header(u->m_file,"�x��");
    cerr << "�s��l `" << name << "' ���Q�Ƃ���Ă��܂�.\n";
    break;
  default:
    header(u->m_file,"warning");
    cerr << "Undefined value `" << name << "' is referenced.\n";
    break;
  }
  ++counter;
}
