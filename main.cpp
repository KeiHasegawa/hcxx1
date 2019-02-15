#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"

int main(int argc, char** argv)
{
  using namespace std;
  using namespace cxx_compiler;
  cmdline::setup(argc,argv);
  if ( !cmdline::input.empty() ){
    cxx_compiler_in = fopen(cmdline::input.c_str(),"r");
    if ( !cxx_compiler_in ){
      error::cmdline::open(cmdline::input);
      exit(1);
    }
    parse::position = file_t(cmdline::input,1);
  }
  generator::initialize();
  cxx_compiler_parse();
#ifdef _DEBUG
  parse::delete_buffer();
#endif // _DEBUG
  declarations::declarators::function::definition::static_inline::defer::last();

  if ( parse::is_last_decl ){
    if ( cmdline::output_medium ){
      cout << '\n';
      dump::scope();
    }
    if ( !error::counter ){
      if ( generator::generate ){
        generator::interface_t tmp = {
          &scope::root,
        };
        generator::generate(&tmp);
      }
    }
  }

  if (!error::counter) {
    if (generator::last) {
      transform(funcs.begin(), funcs.end(), back_inserter(scope::root.m_children), get_pm);
      generator::last_interface_t tmp = {
        &scope::root,
        &funcs
      };
      generator::last(&tmp);
    }
  }
  
  generator::terminate();
  return error::counter;
}
