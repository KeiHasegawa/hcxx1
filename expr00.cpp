// primary-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

cxx_compiler::var* cxx_compiler::expressions::primary::info_t::gen()
{
  if ( m_var )
    return m_var;
  else
    return m_expr->gen();
}

const cxx_compiler::file_t& cxx_compiler::expressions::primary::info_t::file() const
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

namespace cxx_compiler { namespace expressions { namespace primary { namespace literal { namespace integer {
  usr* new_obj(std::string);
} } } } } // end of namespace integer, literal, primary, expressions and cxx_compiler

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::create(std::string name)
{
  using namespace std;
  map<std::string, vector<usr*> >& usrs = scope::root.m_usrs;
  map<std::string, vector<usr*> >::const_iterator p =
    usrs.find(name);
  if ( p != usrs.end() )
    return p->second.back();
  usr* u = new_obj(name);
  u->m_scope = &scope::root;
  usrs[name].push_back(u);
  return u;
}

namespace cxx_compiler { namespace expressions { namespace primary { namespace literal { namespace integer {
  usr* int_(std::string, int, std::string);
  usr* uint_(std::string, unsigned int, std::string);
  usr* long_long_(std::string, __int64, std::string);
  usr* ulong_long_(std::string, unsigned __int64, std::string);
} } } } } // end of namespace integer, literal, primary, expressions and cxx_compiler

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::new_obj(std::string name)
{
  errno = 0;
  char* end;
  int i = strtol(name.c_str(),&end,0);
  if ( errno != ERANGE ){
    std::string suffix = end;
    return integer::int_(name,i,suffix);
  }
  if ( name[0] == '0' || *end == 'u' || *end == 'U' ){
    errno = 0;
    unsigned int ui = strtoul(name.c_str(),&end,0);
    if ( errno != ERANGE ){
      std::string suffix = end;
      return integer::uint_(name,ui,suffix);
    }
  }
  errno = 0;
  __int64 ll = strtoll(name.c_str(),&end,0);
  if ( errno != ERANGE ){
    std::string suffix = end;
    return integer::long_long_(name,ll,suffix);
  }
  errno = 0;
  if ( name[0] == '0' || *end == 'u' || *end == 'U' || name[0] == '-' ){
    unsigned __int64 ull = strtoull(name.c_str(),&end,0);
    if ( errno != ERANGE ){
      std::string suffix = end;
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

namespace cxx_compiler { namespace expressions { namespace primary { namespace literal { namespace integer {
  template<class C> usr* common_int(std::string name, C value, const type* T, std::string suffix)
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
  template<class C> usr* common_long_long(std::string name, C value, const type* T, std::string suffix)
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

template<class T> std::map<T, cxx_compiler::constant<T>*> cxx_compiler::constant<T>::table;

namespace cxx_compiler { namespace expressions { namespace primary { namespace literal { namespace integer {
  template<class T> usr* common(T v, const type* (*pf)())
  {
    using namespace std;
#ifndef __GNUC__
    typedef map<T, constant<T>*>::const_iterator IT;
    IT p = constant<T>::table.find(v);
    if ( p != constant<T>::table.end() )
      return p->second;
#else // __GNUC__
    if ( constant<T>::table.find(v) != constant<T>::table.end() )
      return constant<T>::table.find(v)->second;
#endif // __GNUC__
    std::string name = new_name(".integer");
    const type* type = (*pf)();
    type = const_type::create(type);
    constant<T>* c = new constant<T>(name,type,usr::NONE,parse::position);
    c->m_scope = &scope::root;
    c->m_value = v;
    constant<T>::table[v] = c;
    map<std::string, vector<usr*> >& usrs = scope::root.m_usrs;
    usrs[name].push_back(c);
    return c;
  }
} } } } } // end of namespace integer, literal, primary, expressions and cxx_compiler

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::create(char v){ return common(v,(const type* (*)())char_type::create); }
cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::create(signed char v){ return common(v,(const type* (*)())schar_type::create); }
cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::create(unsigned char v){ return common(v,(const type* (*)())uchar_type::create); }
cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::create(wchar_t v){ return common(v,(const type* (*)())wchar_type::create); }
cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::create(short int v){ return common(v,(const type* (*)())short_type::create); }
cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::create(unsigned short int v){ return common(v,(const type* (*)())ushort_type::create); }
cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::create(int v){ return common(v,(const type* (*)())int_type::create); }
cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::create(unsigned int v){ return common(v,(const type* (*)())uint_type::create); }

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
cxx_compiler::expressions::primary::literal::integer::create(unsigned long int v)
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

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::create(__int64 v){ return common(v,(const type* (*)())long_long_type::create); }
cxx_compiler::usr* cxx_compiler::expressions::primary::literal::integer::create(unsigned __int64 v){ return common(v,(const type* (*)())ulong_long_type::create); }

namespace cxx_compiler { namespace expressions { namespace primary { namespace literal { namespace character {
  usr* new_obj(std::string);
} } } } } // end of namespace character, literal, primary, expressions and cxx_compiler

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::character::create(std::string name)
{
  using namespace std;
  map<std::string, vector<usr*> >& usrs = scope::root.m_usrs;
  map<std::string, vector<usr*> >::const_iterator p =
    usrs.find(name);
  if ( p != usrs.end() )
    return p->second.back();
  usr* u = character::new_obj(name);
  u->m_scope = &scope::root;
  usrs[name].push_back(u);
  return u;
}

namespace cxx_compiler { namespace expressions { namespace primary { namespace literal { namespace character {
  struct simple_escape : misc::pmap<std::string, usr> {
    bool m_initialized;
    simple_escape() : m_initialized(false) {}
    void helper(std::string, std::string, int);
    void initialize();
  } m_simple_escape;
  usr* escape(std::string);
  namespace wide {
    usr* get(std::string);
  }
  usr* universal(std::string);
  usr* normal(std::string);
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
  constant<wchar_typedef>* b = new constant<wchar_typedef>(y,v,usr::NONE,file_t());
  (*this)[y] = b;
  a->m_value = b->m_value = z;
}

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::character::escape(std::string name)
{
  using namespace std;
  std::string s = name;
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
    std::string t = "\'";
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
    typedef int K;
    K k = *reinterpret_cast<K*>(&x);
    constant<X>::table[k] = c;
    return c;
  }
  else if ( suffix == 'l' || suffix == 'L' ){
    typedef long double X;
    volatile X x = 0; x = value<X>()(name);
    const type* T = long_double_type::create();
    T = const_type::create(T);
    constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
    c->m_value = x;
    typedef pair<__int64,__int64> K;
    K k;
    if ( generator::long_double ){
      int sz = long_double_type::create()->size();
      c->b = new unsigned char[sz];
      (*generator::long_double->bit)(c->b,name.c_str());
      memcpy(&k,c->b,sz);
    }
    else
      memcpy(&k,(const void*)&x,sizeof x);
    map<K,constant<X>*>::const_iterator p = constant<X>::table.find(k);
    if ( p != constant<X>::table.end() ){
      delete[] c->b;
      c->b = p->second->b;
    }
    constant<X>::table[k] = c;
    return c;
  }
  else {
    typedef double X;
    X x = value<X>()(name);
    const type* T = double_type::create();
    T = const_type::create(T);
    constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
    c->m_value = x;
    typedef __int64 K;
    K k = *reinterpret_cast<K*>(&x);
    constant<X>::table[k] = c;
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
  if ( fundef::current ){
    usr* u = fundef::current->m_usr;
    if ( u->m_flag & usr::INLINE )
      ret->m_flag = usr::flag_t(ret->m_flag | usr::INLINE_REFED);
  }
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

std::pair<std::map<std::pair<const cxx_compiler::type*, void*>, cxx_compiler::constant<void*>*>,std::map<std::pair<const cxx_compiler::type*, void*>, cxx_compiler::constant<void*>*> >
cxx_compiler::constant<void*>::table;

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

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::floating::create(float x)
{
  using namespace std;
  typedef float X;
  typedef int K;
  K k;
  memcpy(&k,&x,sizeof x);
  map<K, constant<X>*>::const_iterator p = constant<X>::table.find(k);
  if ( p != constant<X>::table.end() )
    return p->second;
  std::string name = new_name(".float");
  const type* T = float_type::create();
  T = const_type::create(T);
  constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
  c->m_scope = &scope::root;
  c->m_value = x;
  constant<X>::table[k] = c;
  map<std::string, vector<usr*> >& usrs = scope::root.m_usrs;
  usrs[name].push_back(c);
  return c;
}

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::floating::create(double x)
{
  using namespace std;
  typedef double X;
  typedef __int64 K;
  K k;
  memcpy(&k,&x,sizeof x);
  map<K, constant<X>*>::const_iterator p = constant<X>::table.find(k);
  if ( p != constant<X>::table.end() )
    return p->second;
  std::string name = new_name(".double");
  const type* T = double_type::create();
  T = const_type::create(T);
  constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
  c->m_scope = &scope::root;
  c->m_value = x;
  constant<X>::table[k] = c;
  map<std::string, vector<usr*> >& usrs = scope::root.m_usrs;
  usrs[name].push_back(c);
  return c;
}

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::floating::create(long double x)
{
  using namespace std;
  typedef long double X;
  typedef pair<__int64,__int64> K;
  K k;
  memcpy(&k,&x,sizeof x);
  map<K, constant<X>*>::const_iterator p = constant<X>::table.find(k);
  if ( p != constant<X>::table.end() )
    return p->second;
  std::string name = new_name(".long double");
  const type* T = long_double_type::create();
  T = const_type::create(T);
  constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
  c->m_scope = &scope::root;
  c->m_value = x;
  constant<X>::table[k] = c;
  map<std::string, vector<usr*> >& usrs = scope::root.m_usrs;
  usrs[name].push_back(c);
  return c;
}

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::floating::create(unsigned char* b)
{
  using namespace std;
  typedef long double X;
  typedef pair<__int64,__int64> K;
  K k;
  int sz = long_double_type::create()->size();
  memcpy(&k,b,sz);
  map<K, constant<X>*>::const_iterator p = constant<X>::table.find(k);
  if ( p != constant<X>::table.end() ){
    delete[] b;
    return p->second;
  }
  std::string name = new_name(".long double");
  const type* T = long_double_type::create();
  T = const_type::create(T);
  constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
  c->m_scope = &scope::root;
  c->b = b;
  constant<X>::table[k] = c;
  map<std::string, vector<usr*> >& usrs = scope::root.m_usrs;
  usrs[name].push_back(c);
  return c;
}

cxx_compiler::usr* cxx_compiler::expressions::primary::literal::boolean::create(bool x)
{
  using namespace std;
  typedef bool X;
  map<X, constant<X>*>::const_iterator p = constant<X>::table.find(x);
  if ( p != constant<X>::table.end() )
    return p->second;
  std::string name = x ? "true" : "false";
  const type* T = bool_type::create();
  constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
  c->m_scope = &scope::root;
  c->m_value = x;
  constant<X>::table[x] = c;
  map<std::string, vector<usr*> >& usrs = scope::root.m_usrs;
  usrs[name].push_back(c);
  return c;
}

namespace cxx_compiler { namespace unqualified_id {
  struct search_info_t {
    tag* m_tag;
    std::vector<base*> m_route;
    search_info_t(tag* Tg) : m_tag(Tg) {}
  };
  int base_search(base*, search_info_t*);
  block* get_block();
} } // end of namespace unqualified_id and cxx_compiler

cxx_compiler::var* cxx_compiler::unqualified_id::action(var* v)
{
  using namespace std;
  usr* u = v->usr_cast();
  if ( !u )
    return v;
  if ( !fundef::current )
    return u;
  usr* func = fundef::current->m_usr;
  scope* p = func->m_scope;
  if ( p->m_id != scope::TAG )
    return u;
  tag* tp = static_cast<tag*>(p);
  scope* q = u->m_scope;
  if ( q->m_id != scope::TAG )
    return u;
  tag* tq = static_cast<tag*>(q);
  if ( tp != tq ){
    vector<base*>* bases = tp->m_bases;
    if ( !bases )
      return u;
    search_info_t info(tq);
    for_each(bases->begin(),bases->end(),bind2nd(ptr_fun(base_search),&info));
    if ( info.m_route.empty() )
      return u;
  }
  const type* T = tp->m_types.second;
  assert(T->m_id == type::RECORD);
  typedef const record_type REC;
  REC* rec = static_cast<REC*>(T);
  pair<int,usr*> ret = rec->offset(u->m_name);
  int offset = ret.first;
  assert(offset >= 0);
  T = u->m_type;
  var* res = new ref(pointer_type::create(T));
  block* b = get_block();
  if ( b )
    b->m_vars.push_back(res);
  else
    garbage.push_back(res);
  scope* s = fundef::current->m_param;
  map<string, vector<usr*> >::const_iterator it = s->m_usrs.find("this");
  assert(it != s->m_usrs.end());
  const vector<usr*>& t = it->second;
  usr* This = t.back();
  code.push_back(new assign3ac(res,This));
  return res;
}

int cxx_compiler::unqualified_id::base_search(base* bp, search_info_t* info)
{
  using namespace std;
  if ( bp->m_tag == info->m_tag ){
    info->m_route.push_back(bp);
    return 0;
  }
  vector<base*>* bases = bp->m_tag->m_bases;
  if ( !bases )
    return 0;
  search_info_t tmp(info->m_tag);
  for_each(bases->begin(),bases->end(),bind2nd(ptr_fun(base_search),&tmp));
  const vector<base*>& u = tmp.m_route;
  vector<base*>& v = info->m_route;
  copy(u.begin(),u.end(),back_inserter(v));
  return 0;
}

cxx_compiler::block* cxx_compiler::unqualified_id::get_block()
{
  using namespace std;
  if ( parse::identifier::mode == parse::identifier::member ){
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
  if ( scope::current->m_id == scope::BLOCK )
    return static_cast<block*>(scope::current);
  return 0;
}

cxx_compiler::var* cxx_compiler::unqualified_id::dtor(tag* Tg)
{
  using namespace std;
  string name = "~" + Tg->m_name;
  const type* T = backpatch_type::create();
  var* ret = new usr(name,T,usr::DTOR,parse::position);
  return ret;
}
