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
  generator::terminate();
  return error::counter;
}
