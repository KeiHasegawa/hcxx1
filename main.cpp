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

namespace cxx_compiler {
  namespace declarations {
    namespace declarators {
      namespace function {
	namespace definition {
	  namespace static_inline {
	    namespace defer {
	      void last()
	      {
		using namespace std;
		for (auto& p : refs) {
		  const vector<ref_t>& v = p.second;
		  assert(!v.empty());
		  const ref_t& r = v[0];
		  error::declarations::declarators::function::definition::
		    static_inline::nodef(r.m_def, r.m_flag, r.m_name, r.m_use);
		}
	      }
	    } // end of namespace defer
	  } // end of namespace static_inline
	} // end of namespace definition
      } // end of namespace function
    } // end of namespace declarators
  } // end of namespace declarations
} // end of namespace cxx_compiler
