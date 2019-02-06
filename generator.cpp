#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

namespace cxx_compiler {
  namespace generator {
    void (*generate)(const interface_t*);
    long_double_t* long_double;
    bool m_require_align = true;
    int (*m_close_file)();
    void* m_module;
  } // end of namespace generator
} // end of namespace cxx_compiler

#ifdef _MSC_VER
extern "C" void* __stdcall LoadLibraryA(const char*);
extern "C" void* __stdcall GetProcAddress(void*, const char*);
extern "C" void __stdcall FreeLibrary(void*);
#endif // _MSC_VER

void cxx_compiler::generator::initialize()
{
  using namespace std;
  string fn = cmdline::generator;
  if ( fn.empty() )
    return;

#ifdef _MSC_VER
  m_module = LoadLibraryA(fn.c_str());
#else // _MSC_VER
  m_module = dlopen(fn.c_str(),RTLD_LAZY);
#endif // _MSC_VER
  if ( !m_module ){
    warning::generator::open(fn);
    return;
  }

#ifdef _MSC_VER
#define dlsym GetProcAddress
#endif // _MSC_VER

  int (*seed)() = (int (*)())dlsym(m_module,"generator_seed");
  if ( !seed ){
    warning::generator::seed(fn);
    return;
  }
#ifdef _MSC_VER
  int hcxx1_seed = _MSC_VER;
  hcxx1_seed += 20000000;
#endif // _MSC_VER
#ifdef __GNUC__
  int hcxx1_seed = (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__);
  hcxx1_seed += 40000000;
#endif // __GNUC__
  int gen_seed = (*seed)();
  if ( hcxx1_seed != gen_seed ){
    warning::generator::seed(fn,make_pair(hcxx1_seed,gen_seed));
    return;
  }

  const vector<string>& option = cmdline::generator_options;
  if ( !option.empty() ){
    void (*op)(int, const char**, int*) = (void (*)(int, const char**, int*))dlsym(m_module,"generator_option");
    if ( !op ){
      warning::generator::option(fn);
      return;
    }
    vector<const char*> argv;
    argv.push_back(fn.c_str());
    for ( int i = 0 ; i < option.size() ; ++i )
      argv.push_back(option[i].c_str());
    vector<int> error(argv.size());
    (*op)(argv.size(),&argv[0],&error[0]);
    vector<int>::iterator p =
      find_if(error.begin(),error.end(),bind2nd(not_equal_to<int>(),0));
    if ( p != error.end() ){
      int n = distance(error.begin(),p);
      warning::generator::option(option[n],*p,fn);
      return;
    }
  }

  int (*of)(const char*) = (int (*)(const char*))dlsym(m_module,"generator_open_file");
  if ( !of ){
    warning::generator::open_file(fn);
    return;
  }
  else {
    if ( !cmdline::output.empty() )
      if ( (*of)(cmdline::output.c_str()) )
        return;
  }

  void (*spell)(void*) = (void (*)(void*))dlsym(m_module,"generator_spell");
  if ( spell ){
    void* magic[] = {
      (void*)&dump::tac,
    };
    (*spell)(&magic[0]);
  }

  generate = (void (*)(const interface_t*))dlsym(m_module,"generator_generate");
  if ( !generate ){
    warning::generator::generate(fn);
    return;
  }

  int (*size)(int) = (int (*)(int))dlsym(m_module,"generator_sizeof");
  type_impl::update(size);
  bool (*require_align)() = (bool (*)())dlsym(m_module,"generator_require_align");
  if ( require_align )
    m_require_align = require_align();

  if ( long_double_t* (*pf)() = (long_double_t* (*)())dlsym(m_module,"generator_long_double") )
    long_double = (*pf)();

  m_close_file = (int (*)())dlsym(m_module,"generator_close_file");
  if ( !m_close_file ){
    warning::generator::close_file(fn);
    return;
  }
}

void cxx_compiler::generator::terminate()
{
  if ( !cmdline::generator.empty() ){
    if ( generator::m_close_file )
      generator::m_close_file();
    if ( error::counter )
      unlink(cmdline::output.c_str());
#ifdef WIN32
    FreeLibrary(m_module);
#endif // WIN32
#ifdef unix
    dlclose(m_module);
#endif // unix
  }
}

