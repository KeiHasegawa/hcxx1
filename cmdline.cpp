#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"

#pragma warning ( disable : 4244)

namespace cxx_compiler {
  using namespace std;
  namespace cmdline {
    struct table_t : map<string,int (*)(char**)> {
      table_t();
    } table;
    bool m_no_generator;
    string prog;
    string input;
    string output;
    bool output_medium;
    bool simple_medium;    
    void setup(int argc, char** argv)
    {
      using namespace std;
#ifdef _MSC_VER
      int separator = '\\';
#else // _MSC_VER
      int separator = '/';
#endif // _MSC_VER
      prog = *argv;
      while ( *++argv ){
	if ( **argv == '-' ){
	  table_t::const_iterator p = table.find(*argv);
	  if ( p != table.end() )
	    argv += (p->second)(argv);
	  else
	    warning::cmdline::option(*argv);
	}
	else {
	  if ( input.empty() ){
	    input = *argv;
	    if (output.empty()) {
	      output = input;
	      string::size_type pos = output.find_last_of(separator);
	      if ( pos != string::npos )
		output.erase(0,pos+1);
	      pos = output.find_last_of('.');
	      if ( pos != string::npos )
		output.erase(pos);
	      output += ".s";
	    }
	  }
	  else
	    warning::cmdline::input(*argv);
	}
      }

      if ( generator.empty() && !m_no_generator ){
	if ( char* p = getenv("CXX1GENERATOR") )
	  generator = p;
	else
	  error::cmdline::generator();
      }
    }
    int no_generator_option(char** argv)
    {
      using namespace std;
      if ( !argv ){
	cerr << " : not specify generator";
	return 0;
      }

      m_no_generator = true;
      return 0;
    }
    int output_medium_option(char** argv)
    {
      using namespace std;
      if ( !argv ){
	cerr << " : dump 3 address code and symbol table";
	return 0;
      }

      output_medium = true;
      return 0;
    }
    int simple_medium_option(char** argv)
    {
      using namespace std;
      if ( !argv ){
	cerr << " : name compiler medium variables simply when dump";
	return 0;
      }

      simple_medium = true;
      return 0;
    }
    
    string generator;
    int generator_option(char** argv)
    {
      using namespace std;
      if ( !argv ){
	cerr << " GENERATOR : specify generator";
	return 0;
      }

      if ( *++argv && **argv != '-' ){
	generator = *argv;
	return 1;
      }
      else {
	warning::cmdline::generator_option();
	return 0;
      }
    }

    vector<string> generator_options;
    int generator_option_option(char** argv)
    {
      using namespace std;
      string left = "(";
      using namespace std;
      if ( !argv ){
	cerr << " ( GENERATOR-OPTION ) : pass option to generator";
	return 0;
      }

      if ( !*++argv || *argv != left ){
	warning::cmdline::generator_option_option(left);
	return 0;
      }

      int n = 1;
      string right = ")";
      for ( ; *++argv ; ++n ){
	if ( right == *argv )
	  return ++n;
	generator_options.push_back(*argv);
      }

      warning::cmdline::generator_option_option(right);
      return n;
    }

    int o_option(char** argv)
    {
      using namespace std;
      if ( !argv ){
	cerr << " OUTPUTFILE : specify output file";
	return 0;
      }

      if ( *++argv && **argv != '-' ){
	output = *argv;
	return 1;
      }
      else {
	warning::cmdline::o_option();
	return 0;
      }
    }
    
    int lang_option(char**);
    int version_option(char**);
    int optimize_option(char**);
    bool bb_optimize = true;
    int nobb_optimize_option(char** argv)
    {
      using namespace std;
      if ( !argv ){
	cerr << " : not apply basic block optimization";
	return 0;
      }

      bb_optimize = false;
      return 0;
    }
    bool dag_optimize = true;
    int nodag_optimize_option(char** argv)
    {
      using namespace std;
      if ( !argv ){
	cerr << " : not apply dag optimization";
	return 0;
      }

      dag_optimize = false;
      return 0;
    }
    bool output_optinfo = false;
    int output_optinfo_option(char** argv)
    {
      using namespace std;
      if ( !argv ){
	cerr << " : output optimization infomation";
	return 0;
      }

      output_optinfo = true;
      return 0;
    }
#ifdef YYDEBUG    
    int cxx_compiler_debug_option(char** argv)
    {
      using namespace std;
      if ( !argv ){
	cerr << " : set cxx_compiler_debug = 1";
	return 0;
      }

      cxx_compiler_debug = 1;
      return 0;
    }
#endif // YYDEBUG
    bool no_inline_sub;
    int no_inline_sub_option(char** argv)
    {
      using namespace std;
      if ( !argv ){
	cerr << " : no inline substitution";
	return 0;
      }

      no_inline_sub = true;
      return 0;
    }
    
    int help_option(char**);
    table_t::table_t()
    {
      (*this)["--no-generator"] = no_generator_option;
      (*this)["--output-medium"] = output_medium_option;
      (*this)["--simple-medium"] = simple_medium_option;
      (*this)["--generator"] = generator_option;
      (*this)["--generator-option"] = generator_option_option;
      (*this)["-o"] = o_option;
      (*this)["--lang"] = lang_option;
      (*this)["--version"] = version_option;
      (*this)["--optimize"] = optimize_option;
      (*this)["--no-basic-block-optimize"] = nobb_optimize_option;
      (*this)["--no-dag-optimize"] = nodag_optimize_option;
      (*this)["--output-optinfo"] = output_optinfo_option;
#ifdef YYDEBUG
      (*this)["--cxx-compiler-debug"] = cxx_compiler_debug_option;
#endif // YYDEBUG      
      (*this)["--no-inline-sub"] = no_inline_sub_option;
      (*this)["-h"] = help_option;
    }
  } // end of namespace cmdline
} // end of namespace cxx_compiler




namespace cxx_compiler { namespace cmdline { namespace lang_impl {
  struct table : std::map<std::string, error::LANG> {
    table();
  } m_table;
} } } // end of namespace lang_impl, cmdline and cxx_compiler

int cxx_compiler::cmdline::lang_option(char** argv)
{
  using namespace std;
  if ( !argv ){
    cerr << " LANG : specify error message language";
    return 0;
  }

  if ( *++argv && **argv != '-' ){
    map<string, error::LANG>::const_iterator p
      = lang_impl::m_table.find(*argv);
    if ( p != lang_impl::m_table.end() )
      error::lang = p->second;
    else
      warning::cmdline::lang_option(*argv);
    return 1;
  }
  else {
    warning::cmdline::lang_option();
    return 0;
  }
}

cxx_compiler::cmdline::lang_impl::table::table()
{
  (*this)["jp"] = error::jpn;
  (*this)["eng"] = error::other;
}

int cxx_compiler::cmdline::version_option(char** argv)
{
  using namespace std;
  if ( !argv ){
    cerr << " : output version";
    return 0;
  }

  using namespace std;
  cerr << prog << ": version " << "1.0" << '\n';
  return 0;
}

int cxx_compiler::cmdline::optimize_level = 1;

int cxx_compiler::cmdline::optimize_option(char** argv)
{
  using namespace std;
  if ( !argv ){
    cerr << " LEVEL : specify optimize level";
    return 0;
  }

  if ( *++argv && '0' <= **argv && **argv <= '1' ){
    optimize_level = **argv - '0';
    return 1;
  }
  else {
    warning::cmdline::optimize_option();
    return 0;
  }
}

namespace cxx_compiler { namespace cmdline {
  void help(const std::pair<std::string, int (*)(char**)>& p)
  {
    using namespace std;
    cerr << '\t' << p.first;
    p.second(0);
    cerr << '\n';
  }
} } // end of namespace cmdline

int cxx_compiler::cmdline::help_option(char** argv)
{
  using namespace std;
  if ( !argv ){
    cerr << " : output this message";
    return 0;
  }

  for_each(table.begin(),table.end(),help);
  return 0;
}
