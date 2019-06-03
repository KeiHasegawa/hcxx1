#ifndef _CXX_IMPL_H_
#define _CXX_IMPL_H_

union YYSTYPE;

namespace cxx_compiler {

struct base {
  usr::flag_t m_flag;
  tag* m_tag;
  base(int access, bool virt, tag* ptr);
};

using namespace std;  
extern vector<tac*> code;

namespace misc {
  template<class C> class pvector : public vector<C*> {
  public:
    ~pvector()
      {
        for ( auto p : *this )
          delete p;
      }
  };
  template<class K, class V> struct pmap : std::map<K,V*> {
#ifdef _DEBUG
    ~pmap()
    {
      for ( auto p : *this )
        delete p.second;
    }
#endif // _DEBUG
  };
  inline int update(goto3ac* go, to3ac* to)
  {
    go->m_to = to;
    return 0;
  }
} // end of namespace misc

namespace ucn {
  string conv(string);
} // end of namespace ucn

namespace parse {
  extern file_t position;
  extern void delete_buffer();
  struct read_t {
    list<pair<int,file_t> > m_token;
    list<void*> m_lval;
  };
  struct context_t {
    int m_state;
    vector<short> m_stack0;
    vector<void*> m_stack1;
    int m_char;
    read_t m_read;
    context_t(int state, const vector<short>& vs, const vector<void*>& vv,
              int c)
    : m_state(state), m_stack0(vs), m_stack1(vv), m_char(c) {}
    static vector<context_t> all;
    static map<int, int> retry;
    static void clear(){ all.clear(); retry.clear(); }
  };
  extern void save(int state, short* b0, short* t0, YYSTYPE* b1, YYSTYPE* t1);
  extern void restore(int* state, short** b0, short** t0, short* a0,
                      YYSTYPE** b1, YYSTYPE** t1, YYSTYPE* a1);
  
  extern read_t g_read;
  extern int last_token;
  extern int get_token();
  extern int peek();
  extern int lex_and_save();
  namespace identifier {
    extern int lookup(std::string, scope*);
    extern int judge(string);
    enum mode_t { look, new_obj, member, peeking };
    extern mode_t mode;
    namespace base_lookup {
      extern vector<route_t> route;
      extern int action(string name, tag* ptr);
    } // end of namespace base_lookup
  } // end of namespace identifier
  extern bool is_last_decl;
  namespace parameter {
    extern void enter();
    extern void leave();
  } // end of namespace parameter
  namespace block {
    extern void enter();
    extern void leave();
  } // end of namespace block
  namespace member_function_body {
    struct save_t {
      scope* m_param;
      read_t m_read;
      save_t() : m_param(0) {}
    };
    extern map<usr*, save_t> stbl;
    extern save_t* saved;
    extern int get_token();
  } // end of namespace member_function_body
} // end of namespace parse

typedef pair<const fundef*, vector<tac*> > FUNCS_ELEMENT_TYPE;
extern vector<FUNCS_ELEMENT_TYPE> funcs;
inline scope* get_pm(FUNCS_ELEMENT_TYPE& elem)
{
  return elem.first->m_param;
}

inline bool cmp(const scope* ptr, scope::id_t id)
{
  return ptr->m_id == id;
}

const string vftbl_name = ".vftbl";
const string vfptr_name = ".vfptr";
const string vbtbl_name = ".vbtbl";
const string vbptr_name = ".vbptr";
bool match_vf(pair<int, var*>, usr*);

namespace error {
  enum LANG { jpn, other };
  extern LANG lang;
  void not_implemented();
  namespace cmdline {
    extern void open(string);
    extern void generator();
  } // end of namespace cmdline
  extern int counter;
  extern bool headered;
  extern void header(const file_t&, string);
  extern void undeclared(const file_t&, string);
  namespace declarations {
    namespace specifier_seq {
      namespace function {
        extern void not_function(const usr*);
        namespace func_spec {
          extern void main(const usr*);
          extern void static_storage(const usr*);
          extern void invalid_internal_linkage(const file_t&, const usr*);
        } // end of namespace func_spec
      } // end of namespace function
      namespace type {
        extern void multiple(const file_t&, const cxx_compiler::type*, const cxx_compiler::type*);
        extern void invalid_combination(const file_t&, const cxx_compiler::type*, string);
        extern void invalid_combination(const file_t&, string, string);
        extern void implicit_int(const usr*);
        extern void implicit_int(const file_t&);
      } // end of namespace type
    } // end of namespace specifier_seq
    extern void redeclaration(const usr*, const usr*, bool);
    extern void not_object(const usr*, const type*);
    namespace declarators {
      namespace qualifier {
        extern void invalid(const file_t&, const type*);
      } // end of namespace qualifier
      namespace function {
        extern void of_function(const file_t&, const usr*);
        extern void of_array(const file_t&, const usr*);
        extern void invalid_storage(const usr*);
        namespace definition {
          extern void multiple(const file_t&, const usr*);
          extern void invalid(const file_t&);
          extern void invalid_return(const usr*, const type*);
          extern void typedefed(const file_t&);
          namespace static_inline {
            extern void nodef(const file_t& decl, usr::flag_t flag,
                              std::string name, const file_t& use);
          } // end of namespace static_inline
        } // end of namespace definition
        namespace parameter {
          extern void invalid_storage(const file_t&, const usr*);
        } // end of namespace parameter
      } // end of namespace function
      namespace array {
        extern void of_function(const file_t&, const usr*);
        extern void not_integer(const file_t&, const usr*);
        extern void not_positive(const file_t&, const usr*);
        extern void asterisc_dimension(const file_t&, const usr*);
        namespace variable_length {
          extern void initializer(const usr*);
          extern void invalid_storage(const usr*);
        } // end of namespace variable_length
      } // end of namespace array
      namespace vm {
        extern void file_scope(const usr*);
        extern void invalid_linkage(const usr*);
      } // end of namespace vm
      namespace reference {
        extern void missing_initializer(const usr*);
      } // end of namespace reference
    } // end of namespace declarators
    namespace enumeration {
      extern void not_constant(const usr*);
      extern void not_integer(const usr*);
      extern void redeclaration(const file_t&, const file_t&, string);
    } // end of namespace enumeration
    namespace initializers {
      extern void exceed(const usr*);
      extern void not_object(const usr*);
      extern void invalid_assign(const file_t&, const usr*, bool);
      extern void with_extern(const usr*);
      extern void no_ctor(const usr*);
      namespace designator {
        extern void invalid_subscripting(const file_t&, const type*);
        extern void not_integer(const file_t&);
        extern void not_constant(const file_t&);
        extern void invalid_dot(const file_t&, const type*);
        extern void not_member(const usr*, const record_type*, const usr*);
      } // end of namespace designator
    } // end of namespace initializers
    extern void empty(const file_t&);
    namespace storage_class {
      extern void multiple(const file_t&, usr::flag_t, usr::flag_t);
    } // end of namespace storage_class
    namespace external {
      extern void invalid_storage(const file_t&);
    } // end of namespace external
  } // end of namespace declarations
  namespace classes {
    extern void redeclaration(const file_t&, const file_t&, string);
    extern void not_ordinary(const usr*);
    extern void incomplete_member(const usr*);
    namespace bit_field {
      extern void exceed(const usr*, const type*);
      extern void zero(const usr*);
      extern void not_integer_bit(const usr*);
      extern void not_constant(const usr*);
      extern void negative(const usr*);
      extern void not_integer_type(const usr*);
    } // end of namespace bit_field
  } // end of namespace classes
  namespace expressions {
    namespace primary {
      namespace literal {
        namespace integer {
          extern void too_large(const file_t&, string, const type*);
        } // end of namespace integer
        namespace character {
          extern void invalid(const file_t&, string, const type*);
        } // end of namespace character
      } // end of namespace literal
      namespace underscore_func {
        extern void outside(const file_t&);
        extern void declared(const file_t&);
      } // end of namespace underscore_func
    } // end of namespace primary
    namespace postfix {
      namespace subscripting {
        extern void not_pointer(const file_t&, const var*);
        extern void not_object(const file_t&, const type*);
        extern void not_integer(const file_t&, const var*);
      } // end of namespace subscripting
      namespace call {
        extern void not_function(const file_t&, const var*);
        extern void num_of_arg(const file_t&, const var*, int, int);
        extern void not_object(const file_t&, const var*);
        extern void mismatch_argument(const file_t&, int, bool, const var*);
        extern void overload_not_match(const usr*);
      } // end of namespace call
      namespace member {
        extern void not_record(const file_t&, const var*);
        extern void not_pointer(const file_t&, const var*);
      } // end of namespace member
      namespace fcast {
        extern void too_many_arg(const file_t&);
      } // end of namespace fcast
    } // end of namespace postfix
    namespace ppmm {
      extern void not_lvalue(const file_t&, bool, const var*);
      extern void not_scalar(const file_t&, bool, const var*);
      extern void not_modifiable(const file_t&, bool, const var*);
      extern void not_modifiable_lvalue(const file_t&, bool, const type*);
      extern void invalid_pointer(const file_t&, bool, const pointer_type*);
    } // end of namespace ppmm
    namespace unary {
      namespace address {
        extern void not_lvalue(const file_t&);
        extern void bit_field(const file_t&, const usr*);
      } // end of namespace address
      namespace indirection {
        extern void not_pointer(const file_t&);
      } // end of namespace indirection
      namespace size {
        extern void invalid(const file_t&, const type*);
        extern void bit_field(const file_t&, const usr*);
      } // end of namespace size
      extern void invalid(const file_t&, int, const type*);
    } // end of namespace unary
    namespace cast {
      extern void invalid(const file_t&);
      extern void not_scalar(const file_t&);
    } // end of namespace cast
    namespace binary {
      extern void invalid(const file_t&, int, const type*, const type*);
      extern void not_compatible(const file_t&, const pointer_type*, const pointer_type*);
    } // end of namespace binary
    namespace conditional {
      extern void not_scalar(const file_t&);
      extern void mismatch(const file_t&);
    } // end of namespace conditional
    namespace assignment {
      extern void not_lvalue(const file_t&);
      extern void not_modifiable(const file_t&, const usr*);
      extern void not_modifiable_lvalue(const file_t&, const type*);
      extern void invalid(const file_t&, const usr*, bool);
    } // end of namespace assignment
    namespace va {
      extern void not_lvalue(const file_t&);
      extern void no_size(const file_t&);
      extern void invalid(string, const file_t&, const var*);
    } // end of namespace va
  } // end of namespace expressions
  namespace statements {
    namespace label {
      extern void multiple(string, const file_t&, const file_t&);
      extern void not_defined(string, const file_t&);
    } // end of namespace label
    namespace _case {
      extern void not_constant(const file_t&, const var*);
      extern void not_integer(const file_t&, const var*);
      extern void no_switch(const file_t&);
      extern void duplicate(const file_t&, const file_t&);
    } // end of namespace _case
    namespace _default {
      extern void no_switch(const file_t&);
      extern void multiple(const file_t&, const file_t&);
    } // end of namespace _default
    namespace expression {
      extern void incomplete_type(const file_t&);
    } // end of namespace expression
    namespace if_stmt {
      extern void not_scalar(const file_t&, const var*);
    } // end of namespace if_stmt
    namespace switch_stmt {
      extern void not_integer(const file_t&, const var*);
      extern void invalid(bool, const file_t&, const usr*);
    } // end of namespace switch_stmt
    namespace while_stmt {
      extern void not_scalar(const file_t&);
    } // end of namespace while_stmt
    namespace do_stmt {
      extern void not_scalar(const file_t&);
    } // end of namespace do_stmt
    namespace for_stmt {
      extern void not_scalar(const file_t&);
      extern void invalid_storage(const usr*);
    } // end of namespace for_stmt
    namespace goto_stmt {
      extern void invalid(const file_t&, const file_t&, const usr*);
    } // end of namespace goto_stmt
    namespace break_stmt {
      extern void not_within(const file_t&);
    } // end of namespace break_stmt
    namespace continue_stmt {
      extern void not_within(const file_t&);
    } // end of namespace continue_stmt
    namespace return_stmt {
      extern void invalid(const file_t&, const type*, const type*);
    } // end of namespace return_stmt
  } // end of namespace statements
} // end of namespace error

namespace warning {
  extern int counter;
  namespace cmdline {
    extern void option(string);
    extern void input(string);
    extern void generator();
    extern void generator_option();
    extern void generator_option_option(string);
    extern void o_option();
    extern void lang_option(string);
    extern void lang_option();
    extern void optimize_option();
  }
  namespace generator {
    extern void open(string);
    extern void seed(string);
    extern void seed(string, pair<int,int>);
    extern void option(string);
    extern void option(string, int, string);
  }
  namespace declarations {
    namespace initializers {
      extern void with_extern(const usr*);
    } // end of namespace initializers
  } // end of namespace declarations
  extern void undefined_reference(const usr*);
  extern void zero_division(const file_t&);
}  // end of namespace warning

namespace cmdline {
  extern void setup(int, char**);
  extern string prog;
  extern string input;
  extern string output;
  extern bool output_medium;
  extern string generator;
  extern vector<string> generator_options;
  extern int optimize_level;
  extern bool simple_medium;
  extern bool bb_optimize;
  extern bool dag_optimize;
  extern bool output_optinfo;
  extern bool no_inline_sub;
}

namespace generator {
  extern void initialize();
  extern void (*generate)(const interface_t*);
  extern int (*close_file)();
  extern void terminate();
  extern long_double_t* long_double;
  extern type::id_t sizeof_type;
  extern type::id_t ptrdiff_type;
  namespace wchar {
    extern type::id_t id;
    extern const cxx_compiler::type* type;
  } // end of namespace wchar
  extern void (*last)(const last_interface_t*);
} // end of namespace generator

namespace dump {
  extern void tacx(ostream&, const tac*);
  extern void scopex(scope* = &scope::root, int = 0);
  namespace names {
    extern void reset();
    extern string ref(var*);
    extern string scopey(scope*);
    extern string refb(optimize::basic_block::info_t*);
  } // end of namespace names
  extern void live(string, const map<optimize::basic_block::info_t*, set<var*> >&);
} // end of namespace dump

namespace type_impl {
  extern void update(int (*)(int id));
} // end of namespace type_impl

extern int calc_offset(const record_type* drec,
		       const record_type* brec,
		       const std::vector<route_t>& route);

extern var* aggregate_conv(const type* T, var* y);

namespace record_impl {
  extern int base_vb(int n, const base* bp);
} // end of namespace record_impl

namespace expressions {
  struct base;
} // end of namespace expressions

namespace declarations {
  extern void destroy();
  struct type_specifier;
  struct specifier {
    int m_keyword;
    const type* m_type;
    usr* m_usr;
    bool m_tag;
    specifier(int n) : m_keyword(n), m_type(0), m_usr(0), m_tag(false) {}
    specifier(type_specifier*);
  };
  struct type_specifier {
    int m_keyword;
    const type* m_type;
    usr* m_usr;
    type_specifier(int);
    type_specifier(const type*);
    type_specifier(usr*);
    type_specifier(tag*);
  };
  namespace specifier_seq {
    struct info_t {
      usr::flag_t m_flag;
      const type* m_type;
      vector<int> m_tmp;
      bool m_tag;
      void update();
      static stack<info_t*> s_stack;
      static void clear();
      info_t(info_t*, specifier*);
      ~info_t()
      {
	assert(!s_stack.empty());
	assert(s_stack.top() == this);
	s_stack.pop();
      }
    };
    namespace func_spec {
      extern void check(var*);
    } // end of namespace func_spec
  } // end of namespace specifier_seq
  namespace elaborated {
    const type* action(int, var*);
  } // end of namespace elaborated
  namespace linkage {
    extern void action(var*, bool brace);
    extern vector<bool> braces;
  } // end of namespace linkage
  extern void check_object(usr*);
  extern usr* action1(var*, bool);
  namespace declarators {
    namespace pointer {
      extern const type* action(const type*, const type*);
      extern const type* action(vector<int>*, bool pm);
    } // end of namespace pointer
    namespace reference {
      extern const type* action();
    } // end of namespace reference
    namespace function {
      extern const type* action(const type*, vector<const type*>*, var*, vector<int>*);
      extern const type* parameter(specifier_seq::info_t*, var*);
      extern const type* parameter(specifier_seq::info_t*, const type*);
      namespace definition {
        extern void begin(declarations::specifier_seq::info_t*, var*);
        extern void action(statements::base*);
        extern void action(fundef* fdef, vector<tac*>&);
        typedef pair<pair<string, scope*>,const vector<const type*>*> KEY;
        typedef map<KEY,usr*> table_t;
        extern table_t dtbl;
        namespace static_inline {
          struct info_t {
            fundef* m_fundef;
            vector<tac*> m_code;
            vector<const type*> m_tmp;
            info_t(fundef* f, const vector<tac*>& c,
                   const vector<const type*>& t)
            : m_fundef(f), m_code(c), m_tmp(t) {}
            ~info_t();
          };
          void substitute(vector<tac*>& vt, int pos, info_t* info);
          namespace skip {
            struct table_t : map<usr*, info_t*> {
#ifdef _DEBUG
              ~table_t(){ for (auto p : *this) delete p.second; }
#endif // _DEBUG
            };
            extern table_t stbl;
            void add(fundef* fdef, vector<tac*>& vc, bool b);
            struct chk_t {
              int m_pos;
              bool m_wait_inline;
              fundef* m_fundef;
              chk_t(fundef* f) : m_pos(-1), m_wait_inline(false), m_fundef(f) {}
            };
            void check(tac* ptac, chk_t* arg);
          } // end of namespace skip
          extern void gencode(info_t*);
          namespace defer {
            struct ref_t {
              string m_name;
              usr::flag_t m_flag;
              file_t m_def;
              file_t m_use;
              ref_t(string name, usr::flag_t flag, const file_t& def,
                    const file_t& use)
              : m_name(name), m_flag(flag), m_def(def), m_use(use) {}
            };
            extern map<string, vector<ref_t> > refs;

            // inline function -> callers
            extern map<string, set<usr*> > callers;

            // caller -> position at caller
            extern map<usr*, vector<int> > positions;
            
            void last();
          } // end of namespace defer
        } // end of namespace static_inline
        namespace mem_initializer {
	  typedef pair<var*, tag*> PAIR;
	  typedef vector<expressions::base*> EXPRS;
          typedef vector<pair<PAIR*,EXPRS*> > VALUE;
          extern map<usr*, VALUE> mtbl;
          void action(PAIR*, EXPRS*);
        } // end of mem_initializer
      } // end of namespace definition
    } // end of namespace function
    namespace array {
      extern const type* action(const type*, expressions::base*, bool, var*);
      namespace variable_length {
        extern map<var*, vector<tac*> > dim_code;
        extern void destroy_tmp();
        extern void allocate(usr*);
      } // end of namespace variable_length
    } // end of namespace array
    namespace type_id {
      extern const type* action(const type*);
    } // end of namespace type_id
    usr* ctor();
    usr* ctor(type_specifier*);
  } // end of namespace declarators
  namespace type_specifier_seq {
    struct info_t {
      const type* m_type;
      vector<int> m_tmp;
      void update();
      static stack<info_t*> s_stack;
      info_t(type_specifier*, info_t*);
      ~info_t(){ s_stack.pop(); }
    };
  } // end of namespace type_specifier_seq
  namespace initializers {
    struct info_t;
    extern void action(var*, info_t*);
    namespace clause { struct info_t; }
    struct info_t {
      clause::info_t* m_clause;
      vector<expressions::base*>* m_exprs;
      info_t(clause::info_t* c) : m_clause(c), m_exprs(0) {}
      info_t(vector<expressions::base*>* exprs) : m_clause(0), m_exprs(exprs) {}
      ~info_t();
    };
    struct element;
    namespace clause {
      struct info_t {
        expressions::base* m_expr;
        vector<element*>* m_list;
        info_t(expressions::base* expr) : m_expr(expr), m_list(0) {}
        info_t(vector<element*>* list) : m_expr(0), m_list(list) {}
        ~info_t();
      };
    } // end of namespace clause
    namespace designator { struct info_t; }
    struct element {
      vector<designator::info_t*>* m_designation;
      clause::info_t* m_clause;
      element(vector<designator::info_t*>* d, clause::info_t* c)
        : m_designation(d), m_clause(c) {}
      ~element();
    };
    namespace designator {
      struct info_t {
        expressions::base* m_expr;
        usr* m_usr;
        info_t(expressions::base* expr, var* v)
          : m_expr(expr), m_usr(static_cast<usr*>(v)) {}
        ~info_t();
      };
    } // end of namespace designator
    struct argument {
      static usr* dst;
      const type* T;
      map<int, var*>& V;
      int off;
      int off_max;
      int nth;
      int nth_max;
      int list_pos;
      int list_len;
      bool not_constant;
      argument(const type* t, map<int,var*>& v, int o, int omax, int n, int nmax, int pos, int len)
        : T(t), V(v), off(o), off_max(omax), nth(n), nth_max(nmax), list_pos(pos), list_len(len), not_constant(false) {}
    };
    namespace lst {
      int gencode(element*, argument*);
    } // end of namespace lst
    extern int fill_zero(argument*);
    extern void gen_loff(usr*, pair<int,var*>);
    extern void initialize_code(with_initial*);
  } // end of namespace initializers
  namespace enumeration {
    extern tag* begin(var*);
    extern void definition(var*, expressions::base*);
    extern const type* end(tag*);
  } // end of namespace enumeration
  namespace duration {
    bool _static(const usr*);
  } // end of namespace duration
  extern bool internal_linkage(usr*);
  namespace asm_definition {
    struct info_t : usr {
      string m_inst;
      void initialize();
      info_t(var*);
    };
  } // end of namespace asm_definition
  namespace new_type_id {
    extern const type* action(type_specifier_seq::info_t*);
  } // end of namespace new_type_id
} // end of namespace declarations

namespace conversion {
  namespace arithmetic {
    const type* gen(var**, var**);
  } // end of namespace arithmetic
} // end of namespace conversion

namespace expressions {
  struct base {
    virtual var* gen() = 0;
    virtual const file_t& file() const = 0;
    virtual ~base(){}
  };
  namespace primary {
    struct info_t : base {
      var* m_var;
      base* m_expr;
      file_t m_file;
      vector<route_t> m_route;
      info_t();
      info_t(var* v);
      info_t(base* b) : m_var(0), m_expr(b) {}
      var* gen();
      const file_t& file() const;
      ~info_t();
    };
    namespace literal {
      namespace integer {
        usr* create(string);
        usr* create(char);
        usr* create(signed char);
        usr* create(unsigned char);
        usr* create(wchar_t);
        usr* create(short int);
        usr* create(unsigned short int);
        usr* create(int);
        usr* create(unsigned int);
        usr* create(long int);
        usr* create(unsigned long int);
        usr* create(__int64);
        usr* create(unsigned __int64);
      } // end of namespace integer
#if defined(_MSC_VER) || defined(__CYGWIN__)
      typedef unsigned short int wchar_typedef;
#endif // defined(_MSC_VER) || defined(__CYGWIN__)
#ifdef linux
      typedef long int wchar_typedef;
#endif // linux
#ifdef __APPLE__
  typedef int wchar_typedef;
#endif // __APPLE__
      namespace character {
        usr* create(string);
      } // end of namespace character
      namespace floating {
        usr* create(string);
        usr* create(float);
        usr* create(double);
        usr* create(long double);
        usr* create(unsigned char*);
      } // end of namespace floating
      namespace stringa {
        var* create(string);
        var* create(var*, var*);
      } // end of namespace stringa
      namespace pointer {
        template<class V> usr* create(const type*, V);
      } // end of namespace pointer
      namespace boolean {
        usr* create(bool);
      } // end of namespace boolean
    } // end of namespace literal
    extern var* action(var* v, const vector<route_t>& route);
    extern var* from_member(usr* u, const vector<route_t>&);
  } // end of namespace primary
  namespace postfix {
    struct call : base {
      base* m_func;
      vector<base*>* m_arg;
      call(base* func, vector<base*>* arg)
        : m_func(func), m_arg(arg) {}
      var* gen();
      const file_t& file() const;
      ~call();
    };
    namespace member {
      struct info_t : base {
        vector<tac*> m_code;
        var* m_expr;
        bool m_dot;
        scope* m_scope;
        file_t m_file;
        var* m_member;
        vector<route_t> m_route;
        var* gen();
        const file_t& file() const { return m_file; }
        info_t(const vector<tac*>& c, var* expr, bool dot, scope* s, const file_t& file)
          : m_code(c), m_expr(expr), m_dot(dot), m_scope(s), m_file(file), m_member(0) {}
      };
      extern stack<info_t*> handling;
      extern info_t* begin(base*, bool);
      extern base* end(info_t*, var*);
    } // end of namespace member
    struct ppmm : base {
      base* m_expr;
      bool m_plus;
      const file_t& file() const { return m_expr->file(); }
      var* gen();
      ppmm(base* expr, bool plus) : m_expr(expr), m_plus(plus) {}
      ~ppmm(){ delete m_expr; }
    };
    struct fcast : base {
      const type* m_type;
      vector<base*>* m_list;
      file_t m_file;
      var* gen();
      const file_t& file() const { return m_file; }
      fcast(declarations::type_specifier*, vector<base*>* list);
    };
  } // end of namespace postfix
  namespace unary {
    struct ppmm : base {
      bool m_plus;
      base* m_expr;
      ppmm(bool plus, base* expr) : m_plus(plus), m_expr(expr) {}
      var* gen();
      const file_t& file() const;
      ~ppmm(){ delete m_expr; }
    };
    struct ope : base {
      int m_op;
      base* m_expr;
      ope(int op, base* expr) : m_op(op), m_expr(expr) {}
      var* gen();
      const file_t& file() const;
      ~ope(){ delete m_expr; }
    };
    struct size_of : base {
      base* m_expr;
      const type* m_type;
      const file_t m_file;
      size_of(base* expr) : m_expr(expr), m_type(0) {}
      size_of(const type* T) : m_expr(0), m_type(T), m_file(parse::position) {}
      var* gen();
      const file_t& file() const;
      ~size_of(){ delete m_expr; }
    };
    struct new_expr : base {
      const type* m_T;
      file_t m_file;
      const file_t& file() const { return m_file; }
      new_expr(const type* T, const file_t& file) : m_T(T), m_file(file) {}
      var* gen();
    };
    struct delete_expr : base {
      base* m_expr;
      const file_t& file() const;
      delete_expr(base* expr) : m_expr(expr) {}
      var* gen();
      ~delete_expr(){ delete m_expr; }
    };
  } // end of namespace unary
  namespace cast {
    struct info_t : base {
      const type* m_type;
      base* m_expr;
      info_t(const type* type, base* expr) : m_type(type), m_expr(expr) {}
      var* gen();
      const file_t& file() const;
      ~info_t(){ delete m_expr; }
    };
  } // end of namespace cast
  namespace compound {
    struct info_t : base {
      const type* m_type;
      vector<declarations::initializers::element*>* m_list;
      file_t m_file;
      var* gen();
      const file_t& file() const { return m_file; }
      info_t(const type* type, vector<declarations::initializers::element*>* list)
        : m_type(type), m_list(list), m_file(parse::position) {}
      ~info_t();
    };
  } // end of namespace compound
  namespace _va_start {
    struct info_t : base {
      base* m_expr1;
      base* m_expr2;
      const file_t& file() const;
      var* gen();
      info_t(base* expr1, base* expr2) : m_expr1(expr1), m_expr2(expr2) {}
      ~info_t(){ delete m_expr1; delete m_expr2; }
    };
  } // end of namespace _va_start
  namespace _va_arg {
    struct info_t : base {
      base* m_expr;
      const type* m_type;
      const file_t& file() const;
      var* gen();
      info_t(base* expr, const type* type) : m_expr(expr), m_type(type) {}
      ~info_t(){ delete m_expr; }
    };
  } // end of namespace _va_arg
  namespace _va_end {
    struct info_t : base {
      base* m_expr;
      const file_t& file() const;
      var* gen();
      info_t(base* expr) : m_expr(expr) {}
      ~info_t(){ delete m_expr; }
    };
  } // end of namespace _va_arg
  namespace binary {
    struct info_t : base {
      base* m_left;
      int m_op;
      base* m_right;
      var* gen();
      const file_t& file() const;
      info_t(base* left, int op, base* right)
        : m_left(left), m_op(op), m_right(right) {}
      ~info_t(){ delete m_left; delete m_right; }
    };
  } // end of namespace binary
  namespace conditional {
    struct info_t : base {
      base* m_expr1;
      base* m_expr2;
      base* m_expr3;
      var* gen();
      const file_t& file() const;
      info_t(base* expr1, base* expr2, base* expr3)
        : m_expr1(expr1), m_expr2(expr2), m_expr3(expr3) {}
      ~info_t(){ delete m_expr1; delete m_expr2; delete m_expr3; }
    };
  } // end of namespace conditional
  namespace assignment {
    extern const type* valid(const type*, var*, bool*);
    extern bool include(int cvr_x, int cvr_y);
  } // end of namespace assignment
  extern bool constant_flag;
} // end of namespace expressions

namespace statements {
  struct base {
    virtual int gen() = 0;
    virtual ~base(){}
  };
  namespace label {
    struct info_t : base {
      usr* m_label;
      base* m_stmt;
      info_t(var* v, base* stmt);
      int gen();
      ~info_t(){ delete m_label; delete m_stmt; }
    };
    extern void check();
    extern void clear();
    extern std::vector<usr*> vm;
  } // end of namespace label
  namespace _case {
    struct info_t : base {
      expressions::base* m_expr;
      base* m_stmt;
      info_t(expressions::base* expr, base* stmt) : m_expr(expr), m_stmt(stmt) {}
      int gen();
      ~info_t(){ delete m_expr; delete m_stmt; }
    };
  } // end of namespace _case
  namespace _default {
    struct info_t : base {
      file_t* m_file;
      base* m_stmt;
      info_t(file_t* file, base* stmt) : m_file(file), m_stmt(stmt) {}
      int gen();
      ~info_t(){ delete m_file; delete m_stmt; }
    };
  } // end of namespace _default
  namespace expression {
    struct info_t : base {
      cxx_compiler::expressions::base* m_expr;
      info_t(cxx_compiler::expressions::base* expr) : m_expr(expr) {}
      int gen();
      ~info_t(){ delete m_expr; }
    };
  } // end of namespace expression
  namespace compound {
    struct info_t : base {
      vector<base*>* m_bases;
      scope* m_scope;
      info_t(vector<base*>* bases, scope* ptr) : m_bases(bases), m_scope(ptr) {}
      int gen();
      ~info_t();
    };
  } // end of namespace compound
  namespace if_stmt {
    struct info_t : base {
      expressions::base* m_expr;
      base* m_stmt1;
      base* m_stmt2;
      int gen();
      info_t(expressions::base* expr, base* stmt1, base* stmt2)
        : m_expr(expr), m_stmt1(stmt1), m_stmt2(stmt2) {}
      ~info_t(){ delete m_expr; delete m_stmt1; delete m_stmt2; }
    };
  } // end of namespace if_stmt
  namespace break_stmt {
    struct outer : vector<goto3ac*> {};
  } // end of namespace break_stmt
  namespace switch_stmt {
    struct info_t : base, break_stmt::outer {
      expressions::base* m_expr;
      base* m_stmt;
      struct case_t {
        var* m_label;
        to3ac* m_to;
        expressions::base* m_expr;
        scope* m_scope;
        case_t(var* v, to3ac* t, expressions::base* b, scope* s)
          : m_label(v), m_to(t), m_expr(b), m_scope(s) {}
        static bool cmp(case_t x, var* y)
        {
          var* tmp = x.m_label;
          conversion::arithmetic::gen(&tmp, &y);
          tmp = tmp->eq(y);
          return tmp->value() != 0;
        }
      };
      vector<case_t> m_cases;
      struct default_t {
        to3ac* m_to;
        file_t m_file;
        scope* m_scope;
        default_t() : m_to(0), m_scope(0) {}
        default_t(to3ac* t, const file_t& f, scope* s)
        : m_to(t), m_file(f), m_scope(s) {}
      };
      default_t m_default;
      int gen();
      info_t(expressions::base* expr, base* stmt)
        : m_expr(expr), m_stmt(stmt) {}
      ~info_t(){ delete m_expr; delete m_stmt; }
    };
  } // end of namespace switch_stmt
  namespace continue_stmt {
    struct outer : vector<goto3ac*> {};
  } // end of namespace continue_stmt
  namespace while_stmt {
    struct info_t : base, break_stmt::outer, continue_stmt::outer {
      expressions::base* m_expr;
      base* m_stmt;
      int gen();
      info_t(expressions::base* expr, base* stmt)
        : m_expr(expr), m_stmt(stmt) {}
      ~info_t(){ delete m_expr; delete m_stmt; }
    };
  } // end of namespace while_stmt
  namespace do_stmt {
    struct info_t : base, break_stmt::outer, continue_stmt::outer {
      base* m_stmt;
      expressions::base* m_expr;
      int gen();
      info_t(base* stmt, expressions::base* expr)
        : m_stmt(stmt), m_expr(expr) {}
      ~info_t(){ delete m_stmt; delete m_expr; }
    };
  } // end of namespace do_stmt
  namespace for_stmt {
    struct info_t : base, break_stmt::outer, continue_stmt::outer {
      scope* m_scope;
      base* m_stmt1;
      expressions::base* m_expr2;
      expressions::base* m_expr3;
      base* m_stmt;
      int gen();
      info_t(base* stmt1, expressions::base* expr2, expressions::base* expr3, base* stmt)
        : m_scope(scope::current), m_stmt1(stmt1), m_expr2(expr2), m_expr3(expr3), m_stmt(stmt) {}
      ~info_t(){ delete m_stmt1; delete m_expr2; delete m_expr3; delete m_stmt; }
    };
  } // end of namespace do_stmt
  namespace break_stmt {
    struct info_t : base {
      file_t m_file;
      int gen();
      info_t() : m_file(parse::position) {}
    };
  } // end of namespace break_stmt
  namespace continue_stmt {
    struct info_t : base {
      file_t m_file;
      int gen();
      info_t() : m_file(parse::position) {}
    };
  } // end of namespace continue_stmt
  namespace return_stmt {
    struct info_t : base {
      expressions::base* m_expr;
      file_t m_file;
      vector<usr*> m_usrs;
      info_t(expressions::base*);
      int gen();
      ~info_t(){ delete m_expr; }
    };
  } // end of namespace return_stmt
  namespace goto_stmt {
    struct info_t : base {
      usr* m_label;
      info_t(var* v) : m_label(static_cast<usr*>(v)) {}
      int gen();
      ~info_t(){ delete m_label; }
    };
  } // end of namespace goto_stmt
  namespace declaration {
    struct info_t : base {
      vector<usr*>* m_usrs;
      int gen();
      info_t(vector<usr*>*, bool);
      ~info_t(){ delete m_usrs; }
    };
  };
} // end of namespace statements

namespace classes {
  namespace specifier {
    extern void begin(int, var*, vector<base*>*);
    extern void begin2(int, tag*);
    extern const type* action();
    extern tag::kind_t get(int);
  } // end of namespace specifier
  namespace members {
    extern void action(var*, expressions::base*);
    extern void bit_field(var*, expressions::base*);
  } // end of namespace members
} // end of namespace classes

extern vector<var*> garbage;
extern string new_name(string);

struct generated : virtual var {
  const type* m_org;
  vector<tac*> m_code;
  generated(const pointer_type* G, const type* T)
    : var(G), m_org(T) {}
  generated* generated_cast(){ return this; }
  var* ppmm(bool, bool);
  var* size();
  var* assign(var*);
  ~generated()
  {
    for (auto p : m_code)
      delete p;
  }
};

namespace cast_impl {
  extern var* with_route(const type* Tx, var* src, const vector<route_t>&);
} // end of namespace cast_impl

struct genaddr : generated, addrof {
  bool m_qualified_func;
  genaddr(const pointer_type*, const type*, var*, int);
  var* rvalue();
  var* subscripting(var*);
  var* call(vector<var*>*);
  var* address();
  var* indirection();
  bool lvalue() const { return true; }
  var* offref(const type*, var*);
  genaddr* genaddr_cast(){ return this; }
};

struct ref : var {
  const type* m_result;
  ref(const pointer_type*);
  var* rvalue();
  bool lvalue() const { return true; }
  var* address();
  var* ppmm(bool, bool);
  var* size();
  var* assign(var*);
  const type* result_type() const { return m_result; }
};

struct refaddr : ref {
  addrof m_addrof;
  refaddr(const pointer_type* pt, var* v, int offset)
    : ref(pt), m_addrof(pt,v,offset) {}
  var* rvalue();
  bool lvalue() const { return m_addrof.m_ref->lvalue(); }
  var* assign(var*);
  var* address();
  var* offref(const type*, var*);
};

struct refbit : refaddr {
  usr* m_member;
  int m_position;
  int m_bit;
  bool m_dot;
  refbit(const pointer_type* pt, var* ref, int offset, usr* member, int position, int bit, bool dot)
    : refaddr(pt,ref,offset), m_member(member), m_position(position), m_bit(bit), m_dot(dot) {}
  var* rvalue();
  var* assign(var*);
  var* address();
  var* size();
  static usr* mask(int);
  static usr* mask(int, int);
};

template<class V> struct refimm : ref {
  V m_addr;
  var* common();
  refimm(const pointer_type* pt, V addr) : ref(pt), m_addr(addr) {}
  var* rvalue()
  {
    using namespace std;
    if (scope::current->m_id == scope::BLOCK) {
      vector<var*>& v = garbage;
      vector<var*>::reverse_iterator p = find(v.rbegin(),v.rend(),this);
      assert(p != v.rend());
      v.erase(p.base()-1);
      block* b = static_cast<block*>(scope::current);
      b->m_vars.push_back(this);
      code.push_back(new assign3ac(this, common()));
    }
    return ref::rvalue();
  }
  var* address()
  {
    using namespace expressions::primary::literal;
    return pointer::create(m_type, m_addr);
  }
  var* assign(var* op)
  {
    using namespace std;
    if ( scope::current->m_id == scope::BLOCK ){
      vector<var*>& v = garbage;
      vector<var*>::reverse_iterator p = find(v.rbegin(),v.rend(),this);
      assert(p != v.rend());
      v.erase(p.base()-1);
      block* b = static_cast<block*>(scope::current);
      b->m_vars.push_back(this);
      code.push_back(new assign3ac(this, common()));
    }
    return ref::assign(op);
  }
};

struct refsomewhere : ref {
  var* m_ref;
  var* m_offset;
  refsomewhere(const pointer_type* pt, var* r, var* o) : ref(pt), m_ref(r), m_offset(o) {}
  var* rvalue();
  var* address();
  var* assign(var*);
  var* offref(const type*, var*); 
};

struct enum_member : usr {
  usr* m_value;
  enum_member(const usr& u, usr* value) : usr(u), m_value(value) {}
};

struct var01 : var {
  int m_one;
  int m_zero;
  var01(const type* T) : var(T), m_one(-1), m_zero(-1) {}
  void if_code(statements::if_stmt::info_t*);
  void while_code(statements::while_stmt::info_t*, to3ac*);
  void for_code(statements::for_stmt::info_t*, to3ac*);
  void do_code(statements::do_stmt::info_t*, to3ac*);
  var* cond(int, int, var*, var*);
  void sweep();
  static void sweep(vector<tac*>::iterator);
};

struct log01 : var01 {
  int m_goto1;
  log01(const type* T, int goto1) : var01(T), m_goto1(goto1) {}
  void do_code(statements::do_stmt::info_t*, to3ac*);
};

struct member_function : var {
  var* m_obj;
  var* m_fun;
  bool m_qualified_func;
  var* m_vftbl_off;
  member_function(var* obj, usr* fun, bool qf)
    : var(0), m_obj(obj), m_fun(fun), m_qualified_func(qf), m_vftbl_off(0) {}
  member_function(var* obj, var* fun, var* vftbl_off)
    : var(0), m_obj(obj), m_fun(fun), m_qualified_func(false),
    m_vftbl_off(vftbl_off) {}
  var* call(vector<var*>*);
  var* rvalue();
};

var* fun_ptr_mem(tag* ptr, usr* fun);

struct opposite_t : map<goto3ac::op,goto3ac::op> {
  opposite_t();
};

extern opposite_t opposite;

namespace optimize {
  void action(fundef*, vector<tac*>&);
  void mark(usr*);
  namespace basic_block {
    void create(std::vector<tac*>&, std::vector<info_t*>&);
  } // end of namespace basic_block
} // end of namespace optimize

struct overload : usr {
  vector<usr*> m_candidacy;
  var* m_obj;
  overload(usr* prev, usr* curr)
    : usr(curr->m_name, 0, usr::OVERLOAD, curr->m_file), m_obj(0)
  { 
    if (prev->m_flag & usr::OVERLOAD) {
      overload* ovl = static_cast<overload*>(prev);
      m_candidacy = ovl->m_candidacy;
    }
    else
      m_candidacy.push_back(prev);
    m_candidacy.push_back(curr);
  }
  var* call(vector<var*>*);
};

namespace class_or_namespace_name {
  extern vector<scope*> before;
  extern scope* last;
  extern void after(bool set_last);
} // end of namespace class_or_namespace_name

namespace unqualified_id {
  extern var* from_nonmember(var*);
  extern var* dtor(tag*);
} // end of namespace unqualifed_id

namespace qualified_id {
  extern var* action(var*);
} // end of namespace qualified_id

namespace call_impl {
  var* common(const func_type* ft,
              var* func,
              vector<var*>* arg,
              bool trial,
              var* this_ptr,
              bool qualified_func,
              var* vftbl_off);
} // end of namespace call_impl

void original_namespace_definition(var*);

void extension_namespace_definition(var*);

inline bool compatible(const type* x, const type* y)
{
  return x->compatible(y);
}

inline const type* composite(const type* x, const type* y)
{
  return x->composite(y);
}

namespace SUB_CONST_LONG_impl {
  const type* propagation(const usr* y, const usr* z);
} // end of namespace SUB_CONST_LONG_impl
 
} // end of namespace cxx_compiler

#endif // _CXX_IMPL_H_
