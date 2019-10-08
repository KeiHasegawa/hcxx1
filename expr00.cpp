// primary-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "cxx_y.h"

namespace cxx_compiler {
  namespace expressions {
    namespace primary {
      using namespace std;
      block* get_block();
      var* action(var* v, const vector<route_t>& route)
      {
        using namespace std;
        usr* u = v->usr_cast();
        if (!u)
          return v;
        if (!fundef::current)
          return u;
        usr* func = fundef::current->m_usr;
        scope* p = func->m_scope;
        scope::id_t id = p->m_id;
        if (id != scope::TAG)
          return u;
        return from_member(u, route);
      }
    } // end of namespace primary
  } // end of namespace expressions
} // end of namespace cxx_compiler


cxx_compiler::expressions::primary::info_t::info_t()
 : m_var(0), m_expr(0), m_file(parse::position)
{
  parse::identifier::lookup(this_name, scope::current);
  m_var = cxx_compiler_lval.m_usr;
}

cxx_compiler::expressions::primary::info_t::info_t(var* v)
 : m_var(v), m_expr(0), m_file(parse::position)
{
  m_route = parse::identifier::base_lookup::route;
  parse::identifier::base_lookup::route.clear();
  if (usr* u = m_var->usr_cast()) {
    usr::flag_t flag = u->m_flag;
    if (flag & usr::ENUM_MEMBER) {
      enum_member* p = static_cast<enum_member*>(u);
      m_var = p->m_value;
    }
  }
}

cxx_compiler::var* cxx_compiler::expressions::primary::info_t::gen()
{
  if ( m_var )
    return action(m_var, m_route);
  else
    return m_expr->gen();
}

const cxx_compiler::file_t&
cxx_compiler::expressions::primary::info_t::file() const
{
  if ( m_var )
    return m_file;
  else
    return m_expr->file();
}

cxx_compiler::expressions::primary::info_t::~info_t()
{
  if ( m_expr )
    delete m_expr;
}

namespace cxx_compiler {
  namespace expressions {
    namespace primary {
      namespace literal {
        namespace integer {
          usr* new_obj(string);
        } // end of namespace integer
      } // end of namespace literal
    } // end of namespace primary
  } // end of namespace expressions
} // end of namespace cxx_compiler

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::
integer::create(std::string name)
{
  using namespace std;
  map<string, vector<usr*> >& usrs = scope::root.m_usrs;
  map<string, vector<usr*> >::const_iterator p =
    usrs.find(name);
  if ( p != usrs.end() )
    return p->second.back();
  usr* u = new_obj(name);
  u->m_scope = &scope::root;
  usrs[name].push_back(u);
  return u;
}

namespace cxx_compiler { namespace expressions { namespace primary { namespace literal { namespace integer {
  usr* int_(string, int, string);
  usr* uint_(string, unsigned int, string);
  usr* long_long_(string, __int64, string);
  usr* ulong_long_(string, unsigned __int64, string);
  int expressible_int(const char* name, char** end);
  unsigned int expressible_uint(const char* name, char** end);
} } } } } // end of namespace integer, literal, primary, expressions and cxx_compiler

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::new_obj(std::string name)
{
  errno = 0;
  char* end;
  const char* tmp = name.c_str();
  int i = integer::expressible_int(tmp,&end);
  if ( errno != ERANGE ){
    string suffix = end;
    return integer::int_(name,i,suffix);
  }
  if ( name[0] == '0' || *end == 'u' || *end == 'U' ){
    errno = 0;
    unsigned int ui = integer::expressible_uint(tmp,&end);
    if ( errno != ERANGE ){
      string suffix = end;
      return integer::uint_(name,ui,suffix);
    }
  }
  errno = 0;
  __int64 ll = strtoll(name.c_str(),&end,0);
  if ( errno != ERANGE ){
    string suffix = end;
    return integer::long_long_(name,ll,suffix);
  }
  errno = 0;
  if ( name[0] == '0' || *end == 'u' || *end == 'U' || name[0] == '-' ){
    unsigned __int64 ull = strtoull(name.c_str(),&end,0);
    if ( errno != ERANGE ){
      string suffix = end;
      return integer::ulong_long_(name,ull,suffix);
    }
  }
  if ( name[0] == '0' || *end == 'u' || *end == 'U' || name[0] == '-' ){
    typedef unsigned __int64 X;
    const type* T = ulong_long_type::create();
    using namespace error::expressions::primary::literal::integer;
    too_large(parse::position,name,T);
    T = const_type::create(T);
    constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
    c->m_value = ll;
    return c;
  }
  else {
    typedef __int64 X;
    const type* T = long_long_type::create();
    using namespace error::expressions::primary::literal::integer;
    too_large(parse::position,name,T);
    T = const_type::create(T);
    constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
    c->m_value = ll;
    return c;
  }
}


int cxx_compiler::expressions::primary::literal::integer::
expressible_int(const char* name, char** end)
{
  long int x = strtol(name,end,0);  
  if (sizeof(long) == sizeof(int))
    return x;
  assert(sizeof(long) == sizeof(long long) && sizeof(int) == 4);
  if (x & (~0L << 31))
    errno = ERANGE;
  return x;
}

unsigned int cxx_compiler::expressions::primary::literal::integer::
expressible_uint(const char* name, char** end)
{
  unsigned long int x = strtoul(name,end,0);  
  if (sizeof(long) == sizeof(int))
    return x;
  assert(sizeof(long) == sizeof(long long));
  unsigned int y = x;
  if (x != y)
    errno = ERANGE;
  return y;
}

namespace cxx_compiler { namespace expressions { namespace primary { namespace literal { namespace integer {
  template<class C> usr* common_int(string name, C value, const type* T, string suffix)
  {
    if ( suffix.empty() ){
      constant<C>* c = new constant<C>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
    else if ( suffix == "u" || suffix == "U" ){
      typedef unsigned int X;
      const type* T = uint_type::create();
      T = const_type::create(T);
      constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
    else if ( suffix == "l" || suffix == "L" ){
      typedef long int X;
      const type* T = long_type::create();
      T = const_type::create(T);
      if (T->size() <= sizeof(X)) {
        constant<X>* c = new constant<X>(name, T, usr::NONE, parse::position);
        c->m_value = value;
        return c;
      }
      typedef __int64 XX;
      assert(T->size() == sizeof(XX));
      constant<XX>* cc = new constant<XX>(name, T, usr::SUB_CONST_LONG, parse::position);
      cc->m_value = value;
      return cc;
    }
    else if ( suffix == "ul" || suffix == "uL" || suffix == "Ul" || suffix == "UL" ||
              suffix == "lu" || suffix == "lU" || suffix == "Lu" || suffix == "LU" ){
      typedef unsigned long int X;
      const type* T = ulong_type::create();
      T = const_type::create(T);
      if (T->size() <= sizeof(X)) {
        constant<X>* c = new constant<X>(name, T, usr::NONE, parse::position);
        c->m_value = value;
        return c;
      }
      typedef unsigned __int64 XX;
      assert(T->size() == sizeof(XX));
      constant<XX>* cc = new constant<XX>(name, T, usr::SUB_CONST_LONG, parse::position);
      cc->m_value = value;
      return cc;
    }
    else if ( suffix == "ll" || suffix == "LL" ){
      typedef __int64 X;
      const type* T = long_long_type::create();
      T = const_type::create(T);
      constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
    else {
      assert(suffix == "ull" || suffix == "uLL" || suffix == "Ull" || suffix == "ULL"
        ||   suffix == "llu" || suffix == "LLu" || suffix == "llU" || suffix == "LLU");
      typedef unsigned __int64 X;
      const type* T = ulong_long_type::create();
      T = const_type::create(T);
      constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
  }
} } } } } // end of namespace integer, literal, primary, expressions and cxx_compiler

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::int_(std::string name, int value, std::string suffix)
{
  const type* T = int_type::create();
  T = const_type::create(T);
  return common_int(name,value,T,suffix);
}

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::uint_(std::string name, unsigned int value, std::string suffix)
{
  const type* T = uint_type::create();
  T = const_type::create(T);
  return common_int(name,value,T,suffix);
}

namespace cxx_compiler { namespace expressions { namespace primary { namespace literal { namespace integer {
  template<class C> usr* common_long_long(string name, C value, const type* T, string suffix)
  {
    if ( suffix.empty() ){
      constant<C>* c = new constant<C>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
    else if ( suffix == "u" || suffix == "U" ){
      typedef unsigned __int64 X;
      const type* T = ulong_long_type::create();
      T = const_type::create(T);
      constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
    else if ( suffix == "l" || suffix == "L" ){
      constant<C>* c = new constant<C>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
    else if ( suffix == "ul" || suffix == "uL" || suffix == "Ul" || suffix == "UL" ||
              suffix == "lu" || suffix == "lU" || suffix == "Lu" || suffix == "LU" ){
      typedef unsigned __int64 X;
      const type* T = ulong_long_type::create();
      T = const_type::create(T);
      constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
    else if ( suffix == "ll" || suffix == "LL" ){
      constant<C>* c = new constant<C>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
    else {
      assert(suffix == "ull" || suffix == "uLL" || suffix == "Ull" || suffix == "ULL"
        ||   suffix == "llu" || suffix == "LLu" || suffix == "llU" || suffix == "LLU");
      typedef unsigned __int64 X;
      const type* T = ulong_long_type::create();
      T = const_type::create(T);
      constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
  }
} } } } } // end of namespace integer, literal, primary, expressions and cxx_compiler

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::long_long_(std::string name, __int64 value, std::string suffix)
{
  const type* T = long_long_type::create();
  T = const_type::create(T);
  return common_long_long(name,value,T,suffix);
}

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::ulong_long_(std::string name, unsigned __int64 value, std::string suffix)
{
  const type* T = ulong_long_type::create();
  T = const_type::create(T);
  return common_long_long(name,value,T,suffix);
}

namespace cxx_compiler { namespace expressions { namespace primary { namespace literal { namespace integer {
  template<class T> usr* common(T v, const type* (*pf)())
  {
    using namespace std;
    string name = new_name(".integer");
    const type* type = (*pf)();
    type = const_type::create(type);
    constant<T>* c = new constant<T>(name,type,usr::NONE,parse::position);
    c->m_scope = &scope::root;
    c->m_value = v;
    map<string, vector<usr*> >& usrs = scope::root.m_usrs;
    usrs[name].push_back(c);
    return c;
  }
  template<class T> usr* common_wrapper(T v, const type* (*pf)())
  {
    using namespace std;
    static map<T, usr*> table;
    typename map<T, usr*>::const_iterator p = table.find(v);
    if (p != table.end())
      return p->second;
    return table[v] = common<T>(v,pf);
  }
} } } } } // end of namespace integer, literal, primary, expressions and cxx_compiler

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::
integer::create(char v)
{ return common_wrapper(v,(const type* (*)())char_type::create); }
cxx_compiler::usr* cxx_compiler::expressions::primary::literal::
integer::create(signed char v)
{ return common_wrapper(v,(const type* (*)())schar_type::create); }
cxx_compiler::usr* cxx_compiler::expressions::primary::literal::
integer::create(unsigned char v)
{ return common_wrapper(v,(const type* (*)())uchar_type::create); }
cxx_compiler::usr* cxx_compiler::expressions::primary::literal::
integer::create(wchar_t v)
{ return common_wrapper(v,(const type* (*)())wchar_type::create); }
cxx_compiler::usr* cxx_compiler::expressions::primary::literal::
integer::create(short int v)
{ return common_wrapper(v,(const type* (*)())short_type::create); }
cxx_compiler::usr* cxx_compiler::expressions::primary::literal::
integer::create(unsigned short int v)
{ return common_wrapper(v,(const type* (*)())ushort_type::create); }
cxx_compiler::usr* cxx_compiler::expressions::primary::literal::
integer::create(int v)
{ return common_wrapper(v,(const type* (*)())int_type::create); }
cxx_compiler::usr* cxx_compiler::expressions::primary::literal::
integer::create(unsigned int v)
{ return common_wrapper(v,(const type* (*)())uint_type::create); }

cxx_compiler::usr*
cxx_compiler::expressions::primary::literal::integer::create(long int v)
{
  using namespace std;
  typedef long int T;
  const type* Tc = const_type::create(long_type::create());
  if (Tc->size() <= sizeof(T)) {
    static map<T, usr*> table;
    map<T, usr*>::const_iterator p = table.find(v);
    if (p != table.end())
      return p->second;
    return table[v] = common(v,(const type* (*)())long_type::create);
  }
  typedef __int64 T2;
  assert(Tc->size() == sizeof(T2));
  static map<T2, usr*> table;
  map<T2, usr*>::const_iterator p = table.find(v);
  if (p != table.end())
    return p->second;
  usr* u = create((T2)v);
  u->m_type = Tc;
  u->m_flag = usr::SUB_CONST_LONG;
  return table[v] = u;
}

cxx_compiler::usr*
cxx_compiler::expressions::primary::literal::
integer::create(unsigned long int v)
{
  using namespace std;  
  typedef unsigned long int T;
  const type* Tc = const_type::create(ulong_type::create());
  if (Tc->size() <= sizeof(T)) {
    static map<T, usr*> table;
    map<T, usr*>::const_iterator p = table.find(v);
    if (p != table.end())
      return p->second;
    return table[v] = common(v,(const type* (*)())ulong_type::create);
  }
  typedef unsigned __int64 T2;
  assert(Tc->size() == sizeof(T2));
  static map<T2, usr*> table;
  map<T2, usr*>::const_iterator p = table.find(v);
  if (p != table.end())
    return p->second;
  usr* u = create((T2)v);
  u->m_type = Tc;
  u->m_flag = usr::SUB_CONST_LONG;
  return table[v] = u;
}

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::create(__int64 v){ return common_wrapper(v,(const type* (*)())long_long_type::create); }
cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::create(unsigned __int64 v){ return common_wrapper(v,(const type* (*)())ulong_long_type::create); }

namespace cxx_compiler { namespace expressions { namespace primary { namespace literal { namespace character {
  usr* new_obj(string);
} } } } } // end of namespace character, literal, primary, expressions and cxx_compiler

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::character::create(std::string name)
{
  using namespace std;
  map<string, vector<usr*> >& usrs = scope::root.m_usrs;
  map<string, vector<usr*> >::const_iterator p =
    usrs.find(name);
  if ( p != usrs.end() )
    return p->second.back();
  usr* u = character::new_obj(name);
  u->m_scope = &scope::root;
  usrs[name].push_back(u);
  return u;
}

namespace cxx_compiler { namespace expressions { namespace primary { namespace literal { namespace character {
  struct simple_escape : misc::pmap<string, usr> {
    bool m_initialized;
    simple_escape() : m_initialized(false) {}
    void helper(string, string, int);
    void initialize();
  } m_simple_escape;
  usr* escape(string);
  namespace wide {
    usr* get(string);
  }
  usr* universal(string);
  usr* normal(string);
} } } } } // end of namespace character, literal, primary, expressions and cxx_compiler

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::character::new_obj(std::string name)
{
  using namespace std;
  m_simple_escape.initialize();
  simple_escape::iterator p = m_simple_escape.find(name);
  if ( p != m_simple_escape.end() ){
    usr* ret = p->second;
    m_simple_escape.erase(p);
    return ret;
  }
  else if ( usr* u = escape(name) )
    return u;
  else if ( usr* u = universal(name) )
    return u;
  else if ( usr* u = wide::get(name) )
    return u;
  else
    return normal(name);
}

void
cxx_compiler::expressions::primary::literal::character::simple_escape::initialize()
{
  if ( m_initialized )
    return;
  helper("'\\\''","L'\\\''",'\'');
  helper("'\\\"'","L'\\\"'",'\"');
  helper("'\\?'", "L'\\?'",'\?');
  helper("'\\\\'","L'\\\\'",'\\');
  helper("'\\a'", "L'\\a'",'\a');
  helper("'\\b'", "L'\\b'",'\b');
  helper("'\\f'", "L'\\f'",'\f');
  helper("'\\n'", "L'\\n'",'\n');
  helper("'\\r'", "L'\\r'",'\r');
  helper("'\\t'", "L'\\t'",'\t');
  helper("'\\v'", "L'\\v'",'\v');
  m_initialized = true;
}

void cxx_compiler::expressions::primary::literal::character::simple_escape::helper(std::string x, std::string y, int z)
{
  using namespace literal;
  const type* u = char_type::create();
  u = const_type::create(u);
  constant<char>* a = new constant<char>(x,u,usr::NONE,file_t());
  (*this)[x] = a;
  const type* v = wchar_type::create();
  v = const_type::create(v);
  constant<wchar_typedef>* b
    = new constant<wchar_typedef>(y,v,usr::NONE,file_t());
  (*this)[y] = b;
  a->m_value = b->m_value = z;
}

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::character::escape(std::string name)
{
  using namespace std;
  string s = name;
  if ( s[0] == 'L' )
    s.erase(0,1);
  assert(s[0] == '\'');
  assert(s.length() > 1);
  if ( s[1] != '\\' )
    return 0;
  s.erase(0,2);
  bool x = false;
  if ( s[0] == 'x' ){
    s = s.substr(1,s.length()-1);
    x = true;
  }
  char* end;
  unsigned int n = x ? strtoul(s.c_str(),&end,16) : strtoul(s.c_str(),&end,8);
  if ( end == &s[0] )
    return 0;
  if ( *end != '\'' ){
    string t = "\'";
    t += end;
    usr* u = character::create(t);
    constant<char>* c = static_cast<constant<char>*>(u);
    n <<= 8;
    n |= (unsigned int)c->m_value;
  }
  if ( name[0] == 'L' ){
    typedef literal::wchar_typedef X;
    const type* T = wchar_type::create();
    T = const_type::create(T);
    constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
    c->m_value = n;
    return c;
  }
  else {
    typedef char X;
    const type* T = char_type::create();
    T = const_type::create(T);
    constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
    c->m_value = n;
    return c;
  }
}

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::character::universal(std::string name)
{
  using namespace literal;
  if ( name[1] != '\\' )
    return 0;
  if ( name[2] != 'u' && name[2] != 'U' )
    return 0;
  unsigned int n = strtoul(&name[3],0,16);
  if ( name[2] == 'u' ){
    typedef wchar_typedef X;
    const type* T = ushort_type::create();
    T = const_type::create(T);
    constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
    c->m_value = n;
    return c;
  }
  else {
    typedef unsigned int X;
    const type* T = uint_type::create();
    T = const_type::create(T);
    constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
    c->m_value = n;
    return c;
  }
}

namespace c_compiler { namespace character_impl { namespace wide_impl {
  int value(std::string);
} } } // end of namespace wide_impl, character_impl and c_compiler

namespace cxx_compiler { namespace expressions { namespace primary { namespace literal { namespace character { namespace wide {
  int value(std::string);
} } } } } } // end of namespace wide, character, literal, primary, expressions and cxx_compiler

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::character::wide::get(std::string name)
{
  using namespace std;
  using namespace literal;
  if ( name[0] != 'L' )
    return 0;
  int v = value(name);
  typedef wchar_typedef X;
  const type* T = ushort_type::create();
  T = const_type::create(T);
  constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
  c->m_value = v;
  return c;
}

namespace cxx_compiler { namespace expressions { namespace primary { namespace literal { namespace character { namespace wide {
  bool jis(std::string);
} } } } } } // end of namespace wide, character, literal, primary, expressions and cxx_compiler

int cxx_compiler::expressions::primary::literal::character::wide::value(std::string name)
{
  using namespace std;
  assert(name[0] == 'L');
  assert(name[1] == '\'');
  pair<unsigned char, unsigned char> v;
  if ( name[4] == '\'' ){ // L' normal normal '
    v.first = name[2];
    v.second = name[3];
  }
  else if ( name[3] == '\'' ) // L' normal '
    v.second = name[2];
  else if ( name[2] != '\\' ){
    if ( jis(name) ){
      v.first = name[5];
      v.second = name[6];
    }
    else { // L' normal escape '
      v.first = name[2];
      name = '\'' + name.substr(3);
      usr* u = character::create(name);
      constant<char>* c = static_cast<constant<char>*>(u);
      v.second = c->m_value;
    }
  }
  else { // L' simple-escape something '
    std::string x = name.substr(1,3);
    x += "'";
    usr* y = character::create(x);
    constant<char>* a = static_cast<constant<char>*>(y);
    v.first = a->m_value;
    std::string w = "'" + name.substr(4);
    usr* z = character::create(w);
    constant<char>* b = static_cast<constant<char>*>(z);
    v.second = b->m_value;
  }
  return v.first << 8 | v.second;
}

bool cxx_compiler::expressions::primary::literal::character::wide::jis(std::string name)
{
  assert(name[0] == 'L');
  assert(name[1] == '\'');
  if ( name.length() == 11 ){
    return name[2] == 0x1b
        && name[3] == 0x24
        && name[4] == 0x42
        && name[7] == 0x1b
        && name[8] == 0x28
        && name[9] == 0x42
        && name[10] == '\'';
  }
  else
    return false;
}

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::character::normal(std::string name)
{
  assert(name[0] == '\'');
  if ( name[2] != '\'' ){
    using namespace error::expressions::primary::literal::character;
    invalid(parse::position,name,char_type::create());
  }
  typedef char X;
  const type* T = char_type::create();
  T = const_type::create(T);
  constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
  c->m_value = name[1];
  return c;
}

namespace cxx_compiler { namespace expressions { namespace primary { namespace literal { namespace floating {
  usr* new_obj(std::string);
} } } } } // end of namespace floating, literal, primary, expressions and cxx_compiler

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::floating::create(std::string name)
{
  using namespace std;
  map<std::string, vector<usr*> >& usrs = scope::root.m_usrs;
  map<std::string, vector<usr*> >::const_iterator p =
    usrs.find(name);
  if ( p != usrs.end() )
    return p->second.back();
  usr* u = floating::new_obj(name);
  u->m_scope = &scope::root;
  usrs[name].push_back(u);
  return u;
}

namespace cxx_compiler { namespace expressions { namespace primary { namespace literal { namespace floating {
  template<class T> struct value {
    T dec(std::string);
    T hex(std::string);
    T operator()(std::string);
  };
} } } } } // end of namespace floating, literal, primary, expressions and cxx_compiler

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::floating::new_obj(std::string name)
{
  using namespace std;
  char suffix = *name.rbegin();
  if ( suffix == 'f' || suffix == 'F' ){
    typedef float X;
    X x = value<X>()(name);
    const type* T = float_type::create();
    T = const_type::create(T);
    constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
    c->m_value = x;
    return c;
  }
  else if ( suffix == 'l' || suffix == 'L' ){
    typedef long double X;
    X x = value<X>()(name);
    const type* T = long_double_type::create();
    T = const_type::create(T);
    constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
    c->m_value = x;
    if ( generator::long_double ){
      int sz = long_double_type::create()->size();
      c->b = new unsigned char[sz];
      (*generator::long_double->bit)(c->b,name.c_str());
    }
    return c;
  }
  else {
    typedef double X;
    X x = value<X>()(name);
    const type* T = double_type::create();
    T = const_type::create(T);
    constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
    c->m_value = x;
    return c;
  }
}

template<class T> T cxx_compiler::expressions::primary::literal::floating::value<T>::operator()(std::string name)
{
  return name[0] == '0' && (name[1] == 'x' || name[1] == 'X') ? hex(name) : dec(name);
}

template<class T> T cxx_compiler::expressions::primary::literal::floating::value<T>::dec(std::string name)
{
  if ( sizeof(T) <= sizeof(double) )
    return strtod(name.c_str(),0);
  else
    return strtold(name.c_str(),0);
}

template<class T> T cxx_compiler::expressions::primary::literal::floating::value<T>::hex(std::string name)
{
  // name = 0 [xX] alpha . beta [pP] [+-]? gamma
  assert(name[0] == '0');
  assert(name[1] == 'x' || name[1] == 'X');
  char* p = const_cast<char*>(name.c_str());
  p += 2;
  T res = 0;
  if ( name[2] != '.' ){
    __int64 alpha = strtoll(p,&p,16);
    res = alpha;
  }
  if ( *p == '.' ){
    ++p;
    if ( isxdigit(*p) ){
      char* q;
      __int64 beta = strtoll(p,&q,16);
      T t = beta;
      for ( ; p != q ; ++p )
        t /= 16;
      res += t;
    }
  }
  assert(*p == 'p' || *p == 'P');
  ++p;
  int gamma = strtol(p,0,0);
  while ( gamma ){
    if ( gamma < 0 ){
      res /= 2;
      ++gamma;
    }
    else {
      res *= 2;
      --gamma;
    }
  }
  return res;
}

namespace cxx_compiler { namespace expressions { namespace primary { namespace literal { namespace stringa {
  usr* new_obj(std::string);
} } } } } // end of namespace stringa, literal, primary, expressions and c_compiler

cxx_compiler::var*
cxx_compiler::expressions::primary::literal::stringa::create(std::string name)
{
  using namespace std;
  map<std::string, vector<usr*> >& usrs = scope::root.m_usrs;
  map<std::string, vector<usr*> >::const_iterator p =
    usrs.find(name);
  usr* u;
  if ( p != usrs.end() )
    u = p->second.back();
  else {
    u = new_obj(name);
    u->m_scope = &scope::root;
    usrs[name].push_back(u);
  }
  const type* T = u->m_type;
  const pointer_type* G = T->ptr_gen();
  genaddr* ret = new genaddr(G,T,u,0);
  garbage.push_back(ret);
  return ret;
}

namespace cxx_compiler { namespace expressions { namespace primary { namespace literal { namespace stringa {
  class calc {
    std::map<int,var*>& m_value;
    bool m_wide;
    bool m_escape;
    bool m_hex_mode;
    bool m_oct_mode;
    bool m_shiftjis_state;
    int m_jis_state;
    int m_euc_state;
    char m_prev;
  public:
    struct acc_t {
      unsigned int m_hex;
      unsigned int m_oct;
      acc_t() : m_hex(0), m_oct(0) {}
    };
  private:
    acc_t* m_acc;
  public:
    calc(std::map<int,var*>& value, bool wide, acc_t* acc)
      : m_value(value), m_wide(wide), m_escape(false), m_hex_mode(false), m_oct_mode(false),
        m_shiftjis_state(false), m_acc(acc), m_jis_state(0), m_euc_state(0), m_prev(0) {}
    int operator()(int n, int c);
  };
} } } } } // end of namespace stringa, literal, primary, expressions and cxx_compiler

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::stringa::new_obj(std::string name)
{
  using namespace std;
  with_initial* ret = new with_initial(name,0,parse::position);
  map<int,var*>& value = ret->m_value;
  bool wide = name[0] == '"' ? false : true;
  if ( wide ){
    ret->m_type = wchar_type::create();
    name = name.substr(2,name.length()-3);
  }
  else {
    ret->m_type = char_type::create();
    name = name.substr(1,name.length()-2);
  }
  calc::acc_t acc;
  int size = accumulate(name.begin(),name.end(),0,calc(value,wide,&acc));
  if ( acc.m_hex ){
    int offset = wide ? size * sizeof(wchar_typedef) : size;
    usr* u = wide ? integer::create((wchar_typedef)acc.m_hex) : integer::create((char)acc.m_hex);
    value.insert(make_pair(offset,u));
    ++size;
  }
  if ( acc.m_oct ){
    int offset = wide ? size * sizeof(wchar_typedef) : size;
    usr* u = wide ? integer::create((wchar_typedef)acc.m_oct) : integer::create((char)acc.m_oct);
    value.insert(make_pair(offset,u));
    ++size;
  }
  int offset = wide ? size * sizeof(wchar_typedef) : size;
  usr* u = wide ? integer::create((wchar_typedef)0) : integer::create((char)0);
  value.insert(make_pair(offset,u));
  ret->m_type = array_type::create(ret->m_type,++size);
  if (parse::templ::save_t::s_stack.empty())
    optimize::mark(ret);
  return ret;
}

int cxx_compiler::expressions::primary::literal::stringa::calc::operator()(int n, int c)
{
  using namespace std;
  if ( c == '\\' && !m_escape ){
    m_escape = true;
    return n;
  }
  if ( m_escape ){
    if ( c == 'x' && !m_hex_mode && !m_oct_mode ){
      m_hex_mode = true;
      return n;
    }
    else if ( isdigit(c) && !m_hex_mode && !m_oct_mode ){
      m_oct_mode = true;
      m_acc->m_oct = c - '0';
      return n;
    }
  }
  if ( m_hex_mode ){
    if ( isxdigit(c) ){
      m_acc->m_hex <<= 4;
      if ( isdigit(c) )
        m_acc->m_hex += c - '0';
      else if ( isupper(c) )
        m_acc->m_hex += c - 'A';
      else
        m_acc->m_hex += c - 'a';
      return n;
    }
    else {
      int offset = m_wide ? n * sizeof(wchar_typedef) : n;
      usr* u = m_wide ? integer::create((wchar_typedef)m_acc->m_hex)
        : integer::create((char)m_acc->m_hex);
      m_value.insert(make_pair(offset,u));
      ++n;
      m_acc->m_hex = 0;
      m_hex_mode = m_escape = false;
    }
  }
  if ( m_oct_mode ){
    if ( isdigit(c) && c != '9' ){
      m_acc->m_oct <<= 3;
      m_acc->m_oct += c - '0';
      return n;
    }
    else {
      int offset = m_wide ? n * sizeof(wchar_typedef) : n;
      usr* u = m_wide ? integer::create((wchar_typedef)m_acc->m_oct)
        : integer::create((char)m_acc->m_oct);
      m_value.insert(make_pair(offset,u));
      ++n;
      m_acc->m_oct = 0;
      m_oct_mode = m_escape = false;
    }
  }
  if ( m_wide ){
    c = (unsigned char)c;
    if ( !m_shiftjis_state && ( 129 <= c && c <= 159 || 224 <= c && c <= 239 ) ){
      m_shiftjis_state = true;
      m_prev = c;
      return n;
    }
    if ( m_shiftjis_state ){
      assert(64 <= c && c <= 126 || 128 <= c && c <= 252);
      m_shiftjis_state = false;
      usr* u = integer::create((wchar_typedef)(m_prev << 8 | c));
      m_value.insert(make_pair(n * sizeof(wchar_typedef),u));
      return n + 1;
    }
    if ( m_jis_state == 0 && c == 0x1b ){
      m_jis_state = 1;
      return n;
    }
    if ( m_jis_state == 1 && c == 0x24 ){
      m_jis_state = 2;
      return n;
    }
    if ( m_jis_state == 2 && c == 0x42 ){
      m_jis_state = 3;
      return n;
    }
    if ( m_jis_state == 3 ){
      if ( c == 0x1b ){
        m_jis_state = 5;
        return n;
      }
      m_prev = c;
      m_jis_state = 4;
      return n;
    }
    if ( m_jis_state == 4 ){
      m_jis_state = 3;
      usr* u = integer::create((wchar_typedef)(m_prev << 8 | c));
      m_value.insert(make_pair(n * sizeof(wchar_typedef),u));
      return n + 1;
    }
    if ( m_jis_state == 5 && c == 0x28 ){
      m_jis_state = 6;
      return n;
    }
    if ( m_jis_state == 6 && c == 0x42 ){
      m_jis_state = 0;
      return n;
    }
    if ( m_euc_state == 0 && c == 0x8e ){
      m_euc_state = 1;
      return n;
    }
    if ( !m_shiftjis_state && m_euc_state == 0 && 0xa1 <= c && c <= 0xfe ){
      m_euc_state = 2;
      m_prev = c;
      return n;
    }
    if ( m_euc_state == 1 )
      m_euc_state = 0;
    if ( m_euc_state == 2 ){
      m_euc_state = 0;
      usr* u = integer::create((wchar_typedef)(m_prev << 8 | c));
      m_value.insert(make_pair(n * sizeof(wchar_typedef),u));
      return n + 1;
    }
  }
  int offset = m_wide ? n * sizeof(wchar_typedef) : n;
  if ( m_escape ){
    ostringstream os;
    if ( m_wide )
      os << 'L';
    os << "'\\" << char(c) << "'";
    usr* u = character::create(os.str());
    m_value.insert(make_pair(offset,u));
    m_escape = false;
  }
  else {
    usr* u = m_wide ? integer::create((wchar_typedef)c) : integer::create(char(c));
    m_value.insert(make_pair(offset,u));
  }
  return n + 1;
}

cxx_compiler::var*
cxx_compiler::expressions::primary::literal::stringa::create(var* x, var* y)
{
  using namespace std;
  genaddr* xx = x->genaddr_cast();
  with_initial* xxx = static_cast<with_initial*>(xx->m_ref);
  std::string s = xxx->m_name;
  std::string::size_type u = s[0] == 'L' ? 2 : 1;
  s = s.substr(u,s.length()-u-1);
  genaddr* yy = y->genaddr_cast();
  with_initial* yyy = static_cast<with_initial*>(yy->m_ref);
  std::string t = yyy->m_name;
  std::string::size_type v = t[0] == 'L' ? 2 : 1;
  t = t.substr(v,t.length()-v-1);
  ostringstream os;
  if ( u == 2 || v == 2 )
    os << 'L';
  os << '"' << s << t << '"';
  return create(os.str());
}

namespace cxx_compiler {
  namespace expressions {
    namespace primary {
      namespace literal {
        namespace pointer {
          template<> usr* create<void*>(const type* T, void* v)
          {
            using namespace std;
            typedef void* X;
            bool temp = T->tmp();
            typedef pair<const type*, X> KEY;
            KEY key(T, v);
            typedef map<KEY, constant<X>*> table_t;
            static table_t table;
            if (!temp) {
              table_t::const_iterator p = table.find(key);
              if (p != table.end())
                return p->second;
            }
            string name = new_name(".pointer");
            constant<X>* c = new constant<X>(name, T, usr::CONST_PTR, parse::position);
            if (!temp)
              table[key] = c;
            c->m_value = v;
            if (temp) {
              map<string, vector<usr*> >& usrs = scope::current->m_usrs;
              usrs[name].push_back(c);
            }
            else {
              c->m_scope = &scope::root;
              map<string, vector<usr*> >& usrs = scope::root.m_usrs;
              usrs[name].push_back(c);
            }
            return c;
          }
          template<> usr* create<__int64>(const type* T, __int64 v)
          {
            using namespace std;
            typedef __int64 X;
            bool temp = T->tmp();
            typedef pair<const type*,X> KEY;
            KEY key(T, v);
            typedef map<KEY, constant<X>*> table_t;
            static table_t table;
            if (!temp) {
              table_t::const_iterator p = table.find(key);
              if (p != table.end())
                return p->second;
            }
            string name = new_name(".pointer");
            constant<X>* c = new constant<X>(name, T, usr::CONST_PTR, parse::position);
            if (!temp)
              table[key] = c;
            c->m_value = v;
            if (temp) {
              map<string, vector<usr*> >& usrs = scope::current->m_usrs;
              usrs[name].push_back(c);
            }
            else {
              c->m_scope = &scope::root;
              map<string, vector<usr*> >& usrs = scope::root.m_usrs;
              usrs[name].push_back(c);
            }
            return c;
          }
        } // end of namespace pointer
      } // end of namespace literal
    } // end of namespace primary
  } // end of namespace expressions
} // end of namespace cxx_compiler

namespace cxx_compiler {
  namespace expressions {
    namespace primary {
      namespace literal {
        namespace floating {
          using namespace std;
          template<class T> map<T, constant<T>* > table;
        } // end of namespace floating      
      } // end of namespace literal      
    } // end of namespace primary    
  } // end of namespace expressions
} // end of namespace cxx_compiler

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::floating::create(float x)
{
  using namespace std;
  typedef float X;
  map<X, constant<X>*>::const_iterator p = table<X>.find(x);
  if ( p != table<X>.end() )
    return p->second;
  string name = new_name(".float");
  const type* T = float_type::create();
  T = const_type::create(T);
  constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
  c->m_scope = &scope::root;
  c->m_value = x;
  table<X>[x] = c;
  map<string, vector<usr*> >& usrs = scope::root.m_usrs;
  usrs[name].push_back(c);
  return c;
}

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::floating::create(double x)
{
  using namespace std;
  typedef double X;
  map<X, constant<X>*>::const_iterator p = table<X>.find(x);
  if ( p != table<X>.end() )
    return p->second;
  string name = new_name(".double");
  const type* T = double_type::create();
  T = const_type::create(T);
  constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
  c->m_scope = &scope::root;
  c->m_value = x;
  table<X>[x] = c;
  map<string, vector<usr*> >& usrs = scope::root.m_usrs;
  usrs[name].push_back(c);
  return c;
}

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::floating::create(long double x)
{
  using namespace std;
  typedef long double X;
  map<X, constant<X>*>::const_iterator p = table<X>.find(x);
  if ( p != table<X>.end() )
    return p->second;
  string name = new_name(".long double");
  const type* T = long_double_type::create();
  T = const_type::create(T);
  constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
  c->m_scope = &scope::root;
  c->m_value = x;
  table<X>[x] = c;
  map<string, vector<usr*> >& usrs = scope::root.m_usrs;
  usrs[name].push_back(c);
  return c;
}

cxx_compiler::usr*
cxx_compiler::expressions::primary::literal::floating::create(unsigned char* b)
{
  using namespace std;
  int sz = long_double_type::create()->size();
  string name = new_name(".long double");
  const type* T = long_double_type::create();
  T = const_type::create(T);
  constant<long double>* c = new constant<long double>(name,T,usr::NONE,parse::position);
  c->m_scope = &scope::root;
  c->b = b;
  map<string, vector<usr*> >& usrs = scope::root.m_usrs;
  usrs[name].push_back(c);
  return c;
}

cxx_compiler::usr*
cxx_compiler::expressions::primary::literal::boolean::create(bool x)
{
  using namespace std;
  typedef bool X;
  static map<X, constant<X>*> table;
  map<X, constant<X>*>::const_iterator p = table.find(x);
  if ( p != table.end() )
    return p->second;
  string name = x ? "true" : "false";
  const type* T = bool_type::create();
  constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
  c->m_scope = &scope::root;
  c->m_value = x;
  table[x] = c;
  map<string, vector<usr*> >& usrs = scope::root.m_usrs;
  usrs[name].push_back(c);
  return c;
}

cxx_compiler::var*
cxx_compiler::expressions::primary::
from_member(usr* u, const std::vector<route_t>& route)
{
  using namespace expressions::primary::literal;
  usr::flag_t flag = u->m_flag;
  if (flag & usr::STATIC)
    return u;
  usr* func = fundef::current->m_usr;
  assert(func);
  scope* p = func->m_scope;
  assert(p->m_id == scope::TAG);
  tag* tp = static_cast<tag*>(p);
  scope* q = u->m_scope;
  if (q->m_id != scope::TAG)
    return u;
  tag* tq = static_cast<tag*>(q);
  const type* Tp = tp->m_types.second;
  if (!Tp)
    return u;
  assert(Tp->m_id == type::RECORD);
  typedef const record_type REC;
  REC* rec = static_cast<REC*>(Tp);
  const type* Tq = tq->m_types.second;
  if (!Tq)
    return u;
  assert(Tq->m_id == type::RECORD);
  REC* mrec = static_cast<REC*>(Tq);

  scope* s = fundef::current->m_param;
  const map<string, vector<usr*> >& usrs = s->m_usrs;
  map<string, vector<usr*> >::const_iterator it = usrs.find(this_name);
  assert(it != usrs.end());
  const vector<usr*>& v = it->second;
  assert(v.size() == 1);
  usr* this_ptr = v.back();
  const type* pmrec = pointer_type::create(mrec);
  var* tmp = cast_impl::with_route(pmrec, this_ptr, route);

  pair<int, usr*> off = mrec->offset(u->m_name);
  int offset = off.first;
  assert(offset >= 0);
  const type* T = u->m_type;
  const pointer_type* pt = pointer_type::create(T);
  var* res = new ref(pt);
  block* b = get_block();
  if (b)
    b->m_vars.push_back(res);
  else
    garbage.push_back(res);
  code.push_back(new cast3ac(res, tmp, pt));
  if (offset) {
    var* off = integer::create(offset);
    code.push_back(new add3ac(res, res, off));
  }
  return res;
}

cxx_compiler::block* cxx_compiler::expressions::primary::get_block()
{
  using namespace std;
  if (parse::identifier::mode == parse::identifier::member) {
    const stack<expressions::postfix::member::info_t*>& s =
      expressions::postfix::member::handling;
    assert(!s.empty());
    expressions::postfix::member::info_t* p = s.top();
    scope* q = p->m_scope;
    if ( q->m_id == scope::BLOCK )
      return static_cast<block*>(q);
    else
      return 0;
  }
  if (!class_or_namespace_name::before.empty()) {
    scope* ptr = class_or_namespace_name::before.back();
    if (ptr->m_id == scope::BLOCK)
      return static_cast<block*>(ptr);
  }
  if ( scope::current->m_id == scope::BLOCK )
    return static_cast<block*>(scope::current);
  return 0;
}

cxx_compiler::var* cxx_compiler::unqualified_id::dtor(tag* ptr)
{
  using namespace std;
  if (ptr->m_kind2 == tag::INSTANTIATE) {
    instantiated_tag* it = static_cast<instantiated_tag*>(ptr);
    ptr = it->m_src;
  }
  string name = "~" + ptr->m_name;
  const type* T = backpatch_type::create();
  return new usr(name, T, usr::DTOR, parse::position, usr::NONE2);
}

namespace cxx_compiler {
  namespace unqualified_id {
    struct nonstatic_member_ref : usr {
      nonstatic_member_ref(usr* u) : usr(*u) {}
      var* rvalue()
      {
        error::not_implemented();
        return 0;
      }
      var* address()
      {
        using namespace expressions::primary::literal;
        assert(m_scope->m_id == scope::TAG);
        tag* ptr = static_cast<tag*>(m_scope);
        const type* T = pointer_member_type::create(ptr, m_type);
        var* ret = new var(T);
        if (scope::current->m_id == scope::BLOCK) {
          block* b = static_cast<block*>(scope::current);
          b->m_vars.push_back(ret);
        }
        else
          garbage.push_back(ret);
        assert(m_type->m_id != type::FUNC);
        T = ptr->m_types.second;
        assert(T->m_id == type::RECORD);
        typedef const record_type REC;
        REC* rec = static_cast<REC*>(T);
        pair<int, usr*> p = rec->offset(m_name);
        int offset = p.first;
        assert(offset >= 0);
        var* off = integer::create(offset);
        code.push_back(new assign3ac(ret, off));
        return ret;
      }
    };
  } // end of namespace unqualified_id
} // end of namespace cxx_compiler

cxx_compiler::var*
cxx_compiler::unqualified_id::from_nonmember(var* v)
{
  using namespace parse;
  if (identifier::mode == identifier::member)
    return v;
  using namespace class_or_namespace_name;
  assert(!before.empty());
  scope* ptr = before.back();
  if (ptr == scope::current)
    return v;
  scope* q = v->m_scope;
  if (q->m_id != scope::TAG)
    return v;
  usr* u = v->usr_cast();
  if (!u)
    return v;
  usr::flag_t flag = u->m_flag;
  if (flag & usr::STATIC)
    return u;
  if (!fundef::current)
    return u;
  usr* func = fundef::current->m_usr;
  assert(func);
  if (func->m_scope == u->m_scope) {
    usr::flag_t flag = func->m_flag;
    if (!(flag & usr::STATIC))
      return u;
  }
  var* ret = new nonstatic_member_ref(u);
  garbage.push_back(ret);
  return ret;
}

namespace cxx_compiler {
  string operator_name(int op)
  {
    switch (op) {
    case NEW_KW: return "new";
    case DELETE_KW: return "delete";
    case NEW_ARRAY_LEX: return "new []";
    case DELETE_ARRAY_LEX: return "delete []";
    case '+': return "+";
    case '-': return "-";
    case '*': return "*";
    case '/': return "/";
    case '%': return "%";
    case '^': return "^";
    case '&': return "&";
    case '|': return "|";
    case '~': return "~";
    case '!': return "!";
    case '=': return "=";
    case '<': return "<";
    case '>': return ">";
    case ADD_ASSIGN_MK: return "+=";
    case SUB_ASSIGN_MK: return "-=";
    case MUL_ASSIGN_MK: return "*=";
    case DIV_ASSIGN_MK: return "/=";
    case MOD_ASSIGN_MK: return "%=";
    case XOR_ASSIGN_MK: return "^=";
    case AND_ASSIGN_MK: return "&=";
    case OR_ASSIGN_MK: return "|=";
    case LSH_MK: return "<<";
    case RSH_MK: return ">>";
    case LSH_ASSIGN_MK: return "<<=";
    case RSH_ASSIGN_MK: return ">>=";
    case EQUAL_MK: return "==";
    case NOTEQ_MK: return "!=";
    case LESSEQ_MK: return "<=";
    case GREATEREQ_MK: return ">=";
    case ANDAND_MK: return "&&";
    case OROR_MK: return "||";
    case PLUSPLUS_MK: return "++";
    case MINUSMINUS_MK: return "--";
    case ',': return ",";
    case ARROWASTER_MK: return "->*";
    case ARROW_MK: return "->";
    case '(': return "()";
    default : assert(op == '['); return "[]";
    }
  }
} // end of namespace cxx_compiler

cxx_compiler::var*
cxx_compiler::unqualified_id::operator_function_id(int op)
{
  typedef declarations::specifier_seq::info_t INFO;
  const stack<INFO*>& s = INFO::s_stack;
  string opn = operator_name(op);
  if (!s.empty() && s.top()) {
    assert(!class_or_namespace_name::before.empty());
    if (scope::current == class_or_namespace_name::before.back()) {
      const type* bp = backpatch_type::create();
      return new usr(opn, bp, usr::NONE, parse::position, usr::OPERATOR);
    }
  }
  parse::identifier::mode_t morg = parse::identifier::mode;
  parse::identifier::mode = parse::identifier::no_err;
  int r = parse::identifier::lookup(opn, scope::current);
  parse::identifier::mode = morg;
  if (r)
    return cxx_compiler_lval.m_var;
  if (op != '=')
    error::not_implemented();

  assert(scope::current->m_id == scope::TAG);
  tag* ptr = static_cast<tag*>(scope::current);
  const type* T = ptr->m_types.second;
  assert(T);
  const type* rt = reference_type::create(T);
  const type* rtc = reference_type::create(const_type::create(T));
  vector<const type*> tmp;
  tmp.push_back(rtc);
  const func_type* ft = func_type::create(rt, tmp);
  usr::flag_t flag = usr::flag_t(usr::FUNCTION | usr::INLINE);
  usr::flag2_t flag2 = usr::flag2_t(usr::OPERATOR | usr::GENED_BY_COMP);
  usr* u = new usr(opn, ft, flag, parse::position, flag2);
  u->m_scope = scope::current;
  map<string, vector<usr*> >& usrs = ptr->m_usrs;
  usrs[opn].push_back(u);

  using namespace declarations::declarators::function::definition;
  const vector<const type*>& parameter = ft->param();
  KEY key(make_pair(opn, ptr), &parameter);
  dtbl[key] = u;

  using namespace class_or_namespace_name;
  scope* param = new scope(scope::PARAM);
  assert(before.back() == param);
  before.pop_back();
  ptr->m_children.push_back(param);
  param->m_parent = ptr;
  const type* pt = pointer_type::create(T);
  usr* this_ptr = new usr(this_name, pt, usr::NONE, parse::position,
			  usr::NONE2);
  string frn = new_name(".param");
  usr* first = new usr(frn, rtc, usr::NONE, parse::position, usr::NONE2);
  map<string, vector<usr*> >& pusrs = param->m_usrs;
  assert(pusrs.find(this_name) == pusrs.end());
  pusrs[this_name].push_back(this_ptr);
  pusrs[frn].push_back(first);
  vector<usr*>& order = param->m_order;
  order.push_back(this_ptr);
  order.push_back(first);

  block* body = new block;
  assert(!before.empty());
  assert(before.back() == body);
  before.pop_back();
  body->m_parent = param;
  param->m_children.push_back(body);

  vector<tac*> vc;
  var* t0 = new var(T);
  body->m_vars.push_back(t0);
  vc.push_back(new invraddr3ac(t0, first));
  vc.push_back(new invladdr3ac(this_ptr, t0));
  vc.push_back(new return3ac(this_ptr));
  fundef* forg = fundef::current;
  fundef::current = new fundef(u, param);
  declarations::declarators::
    function::definition::action(fundef::current, vc);
  delete fundef::current;
  fundef::current = forg;

  const pointer_type* G = ft->ptr_gen();
  return new genaddr(G, ft, u, 0);
}

namespace cxx_compiler {
  string conversion_name(const type* T)
  {
    using namespace std;
    ostringstream os;
    T->decl(os, "");
    return os.str();
  }
} // end of namespace cxx_compiler

cxx_compiler::var*
cxx_compiler::unqualified_id::conversion_function_id(const type* X)
{
  using namespace declarations;
  type_specifier* ts = new type_specifier(X);
  specifier* spec = new specifier(ts);
  if (specifier_seq::info_t::s_stack.empty())
    new specifier_seq::info_t(0, spec);
  else {
    assert(specifier_seq::info_t::s_stack.size() == 1);
    specifier_seq::info_t* prev = specifier_seq::info_t::s_stack.top();
    new specifier_seq::info_t(prev, spec);
    assert(specifier_seq::info_t::s_stack.size() == 1);
  }
  const type* T = backpatch_type::create();
  string name = conversion_name(X);
  return new usr(name,T,usr::NONE,parse::position,usr::CONV_OPE);
}

cxx_compiler::var* cxx_compiler::qualified_id::action(var* v)
{
  genaddr* ga = v->genaddr_cast();
  if (!ga)
    return v;
  var* r = ga->m_ref;
  usr* u = r->usr_cast();
  if (!u)
    return v;
  const type* T = u->m_type;
  if (T->m_id != type::FUNC)
    return v;
  ga->m_qualified_func = true;
  return ga;
}
