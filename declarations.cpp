#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

void cxx_compiler::declarations::destroy()
{
  using namespace std;
  for (auto p : code)
    delete p;
  code.clear();
  for (auto p : garbage)
    delete p;
  garbage.clear();
  vector<scope*>& children = scope::current->m_children;
  typedef vector<scope*>::iterator IT;
  for (IT p = children.begin() ; p != children.end() ; ) {
    scope::id_t id = (*p)->m_id;
    switch (id) {
    case scope::NONE: case scope::PARAM: case scope::BLOCK:
      delete *p;
      p = children.erase(p);
      break;
    case scope::TAG: case scope::NAMESPACE:
      ++p;
      break;
    default:
      assert(0);
      break;
    }
  }
  error::headered = false;
  if (!generator::last)
    type::destroy_tmp();
  declarators::array::variable_length::destroy_tmp();
  if ( cmdline::simple_medium )
    dump::names::reset();
  class_or_namespace_name::after(false);
  class_or_namespace_name::last = 0;
  parse::context_t::clear();
}

cxx_compiler::declarations::specifier::specifier(type_specifier* spec)
{
  using namespace std;
  auto_ptr<type_specifier> sweeper(spec);
  if ( int n = spec->m_keyword ){
    m_keyword = n;
    m_type = 0;
    m_usr = 0;
    m_tag = false;
  }
  else if ( spec->m_type ){
    m_keyword = 0;
    m_type = spec->m_type;
    m_usr = 0;
    m_tag = true;
  }
  else {
    m_keyword = 0;
    m_type = 0;
    m_usr = spec->m_usr;
    m_tag = false;
  }
}

cxx_compiler::declarations::type_specifier::type_specifier(int n)
 : m_keyword(n), m_type(0), m_usr(0)
{
  if ( VOID_KW <= m_keyword && m_keyword <= UNSIGNED_KW )
    parse::identifier::mode = parse::identifier::new_obj;
}

cxx_compiler::declarations::type_specifier::type_specifier(const type* T)
 : m_keyword(0), m_type(T), m_usr(0)
{
  assert(m_type);
  parse::identifier::mode = parse::identifier::new_obj;
}

cxx_compiler::declarations::type_specifier::type_specifier(usr* u)
 : m_keyword(0), m_type(0), m_usr(u)
{
  assert(m_usr);
  parse::identifier::mode = parse::identifier::new_obj;
}

cxx_compiler::declarations::type_specifier::type_specifier(tag* ptr)
 : m_keyword(0), m_type(0), m_usr(0)
{
  parse::identifier::base_lookup::route.clear();
  m_type = ptr->m_types.second ? ptr->m_types.second : ptr->m_types.first;
  assert(m_type);
  parse::identifier::mode = parse::identifier::new_obj;
}

namespace cxx_compiler { namespace declarations { namespace specifier_seq { namespace flag {
  struct table : std::map<int,usr::flag_t> {
    table();
  } m_table;
  extern usr::flag_t merge(usr::flag_t, usr::flag_t);
} } } } // end of namespace flag, specifier_seq, declarations and cxx_compiler

cxx_compiler::declarations::specifier_seq::
info_t::info_t(info_t* prev, specifier* spec)
{
  using namespace std;
  auto_ptr<specifier> sweeper(spec);
  if ( int keyword = spec->m_keyword ){
    if ( prev ){
      m_type = prev->m_type;
      m_tag = prev->m_tag;
    }
    else {
      m_type = 0;
      m_tag = false;
    }
    flag::table::const_iterator p = flag::m_table.find(keyword);
    if ( p != flag::m_table.end() )
      m_flag = prev ? flag::merge(prev->m_flag,p->second) : p->second;
    else {
      m_flag = prev ? prev->m_flag : usr::NONE;
      if ( prev )
        m_tmp = prev->m_tmp;
      m_tmp.push_back(keyword);
    }
  }
  else if ( const type* T = spec->m_type ){
    if ( prev && prev->m_type ){
      using namespace error::declarations::specifier_seq::type;
      multiple(parse::position,prev->m_type,T);
    }
    m_type = T;
    m_tag = spec->m_tag;
    if ( prev ){
      m_flag = prev->m_flag;
      m_tmp = prev->m_tmp;
    }
    else
      m_flag = usr::NONE;
  }
  else {
    m_type = spec->m_usr->m_type;
    m_flag = usr::flag_t(spec->m_usr->m_flag & ~usr::TYPEDEF);
    m_tag = false;
    if ( prev ){
      m_flag = usr::flag_t(m_flag | prev->m_flag);
      m_tmp = prev->m_tmp;
    }
  }
  delete prev;
  if ( s_stack.empty() || s_stack.top() )
    s_stack.push(this);
  else
    s_stack.top() = this;
}

cxx_compiler::declarations::specifier_seq::info_t::~info_t()
{
  assert(!s_stack.empty());
  assert(s_stack.top() == this);
  s_stack.pop();
}

void cxx_compiler::declarations::specifier_seq::info_t::clear()
{
  parse::identifier::mode = parse::identifier::look;
  s_stack.push(0);
}

cxx_compiler::declarations::specifier_seq::flag::table::table()
{
  (*this)[TYPEDEF_KW] = usr::TYPEDEF;
  (*this)[EXTERN_KW] = usr::EXTERN;
  (*this)[STATIC_KW] = usr::STATIC;
  (*this)[AUTO_KW] = usr::AUTO;
  (*this)[REGISTER_KW] = usr::REGISTER;
  (*this)[INLINE_KW] = usr::INLINE;
  (*this)[VIRTUAL_KW] = usr::VIRTUAL;
  (*this)[EXPLICIT_KW] = usr::EXPLICIT;
  (*this)[FRIEND_KW] = usr::FRIEND;
  (*this)[CONSTEXPR_KW] = usr::CONSTEXPR;
}

cxx_compiler::usr::flag_t
cxx_compiler::declarations::specifier_seq::flag::merge(usr::flag_t x, usr::flag_t y)
{
  usr::flag_t mask = usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO | usr::REGISTER);
  if ( (x & mask) && (y & mask) ){
    using namespace error::declarations::storage_class;
    multiple(parse::position,x,y);
    return x;
  }
  else
    return usr::flag_t(x | y);
}

namespace cxx_compiler { namespace declarations { namespace specifier_seq {
  const type* updator(const type*, int);
  namespace type {
    struct table : std::map<int,const cxx_compiler::type* (*)(const cxx_compiler::type*)> {
      table();
    } m_table;
  } // end of namespace type
} } } // end of namespace specifier_seq, declarations and cxx_compiler

const cxx_compiler::type*
cxx_compiler::declarations::specifier_seq::updator(const cxx_compiler::type* T, int keyword)
{
  type::table::const_iterator p = type::m_table.find(keyword);
  assert(p != type::m_table.end());
  return p->second(T);
}

namespace cxx_compiler { namespace declarations { namespace specifier_seq { namespace type {
  const cxx_compiler::type* char_handler(const cxx_compiler::type*);
  const cxx_compiler::type* wchar_handler(const cxx_compiler::type*);
  const cxx_compiler::type* bool_handler(const cxx_compiler::type*);
  const cxx_compiler::type* short_handler(const cxx_compiler::type*);
  const cxx_compiler::type* int_handler(const cxx_compiler::type*);
  const cxx_compiler::type* long_handler(const cxx_compiler::type*);
  const cxx_compiler::type* signed_handler(const cxx_compiler::type*);
  const cxx_compiler::type* unsigned_handler(const cxx_compiler::type*);
  const cxx_compiler::type* float_handler(const cxx_compiler::type*);
  const cxx_compiler::type* float128_handler(const cxx_compiler::type*);
  const cxx_compiler::type* double_handler(const cxx_compiler::type*);
  const cxx_compiler::type* void_handler(const cxx_compiler::type*);
  const cxx_compiler::type* const_handler(const cxx_compiler::type*);
  const cxx_compiler::type* volatile_handler(const cxx_compiler::type*);
  const cxx_compiler::type* restrict_handler(const cxx_compiler::type*);
} } } } // end of namespace type, specifier_seq, declarations and cxx_compiler

cxx_compiler::declarations::specifier_seq::type::table::table()
{
  (*this)[CHAR_KW] = char_handler;
  (*this)[WCHAR_T_KW] = wchar_handler;
  (*this)[BOOL_KW] = bool_handler;
  (*this)[SHORT_KW] = short_handler;
  (*this)[INT_KW] = int_handler;
  (*this)[LONG_KW] = long_handler;
  (*this)[SIGNED_KW] = signed_handler;
  (*this)[UNSIGNED_KW] = unsigned_handler;
  (*this)[FLOAT_KW] = float_handler;
  (*this)[FLOAT128_KW] = float128_handler;
  (*this)[DOUBLE_KW] = double_handler;
  (*this)[VOID_KW] = void_handler;
  (*this)[CONST_KW] = const_handler;
  (*this)[VOLATILE_KW] = volatile_handler;
  (*this)[RESTRICT_KW] = restrict_handler;
}

const cxx_compiler::type*
cxx_compiler::declarations::specifier_seq::type::char_handler(const cxx_compiler::type* T)
{
  if ( T ){
    using namespace error::declarations::specifier_seq::type;
    multiple(parse::position,T,char_type::create());
  }
  return char_type::create();
}

const cxx_compiler::type*
cxx_compiler::declarations::specifier_seq::type::wchar_handler(const cxx_compiler::type* T)
{
  if ( T ){
    using namespace error::declarations::specifier_seq::type;
    multiple(parse::position,T,wchar_type::create());
  }
  return wchar_type::create();
}

const cxx_compiler::type*
cxx_compiler::declarations::specifier_seq::type::bool_handler(const cxx_compiler::type* T)
{
  if ( T ){
    using namespace error::declarations::specifier_seq::type;
    multiple(parse::position,T,bool_type::create());
  }
  return bool_type::create();
}

namespace cxx_compiler { namespace declarations { namespace specifier_seq { namespace type {
  bool m_short;
  namespace short_impl {
    struct table : std::map<const cxx_compiler::type*, const cxx_compiler::type*> {
      table();
    } m_table;
  } // end of namespace short_impl
} } } } // end of namespace type, specifier_seq, declarations and cxx_compiler

const cxx_compiler::type*
cxx_compiler::declarations::specifier_seq::type::short_handler(const cxx_compiler::type* T)
{
  m_short = true;
  short_impl::table::const_iterator p =
    short_impl::m_table.find(T);
  if ( p != short_impl::m_table.end() )
    return p->second;
  else {
    using namespace error::declarations::specifier_seq::type;
    invalid_combination(parse::position,T,"short");
    return short_type::create();
  }
}

cxx_compiler::declarations::specifier_seq::type::short_impl::table::table()
{
  (*this)[0] = short_type::create();
  (*this)[int_type::create()] = short_type::create();
}

const cxx_compiler::type*
cxx_compiler::declarations::specifier_seq::type::int_handler(const cxx_compiler::type* T)
{
  if ( T ){
    using namespace error::declarations::specifier_seq::type;
    multiple(parse::position,T,int_type::create());
  }
  return int_type::create();
}

namespace cxx_compiler { namespace declarations { namespace specifier_seq { namespace type {
  namespace long_impl {
    struct table : std::map<const cxx_compiler::type*, const cxx_compiler::type*> {
      table();
    } m_table;
  } // end of namespace long_impl
} } } } // end of namespace type, specifier_seq, declarations and cxx_compiler

const cxx_compiler::type*
cxx_compiler::declarations::specifier_seq::type::long_handler(const cxx_compiler::type* T)
{
  long_impl::table::const_iterator p =
    long_impl::m_table.find(T);
  if ( p != long_impl::m_table.end() )
    return p->second;
  else {
    using namespace error::declarations::specifier_seq::type;
    invalid_combination(parse::position,T,"long");
    return long_type::create();
  }
}

cxx_compiler::declarations::specifier_seq::type::long_impl::table::table()
{
  (*this)[0] = long_type::create();
  (*this)[int_type::create()] = long_type::create();
  (*this)[long_type::create()] = long_long_type::create();
  (*this)[double_type::create()] = long_double_type::create();
}

namespace cxx_compiler { namespace declarations { namespace specifier_seq { namespace type {
  bool m_signed;
  bool m_unsigned;
  namespace signed_impl {
    struct table : std::map<const cxx_compiler::type*, const cxx_compiler::type*> {
      table();
    } m_table;
  } // end of namespace signed_impl
} } } } // end of namespace type, specifier_seq, declarations and cxx_compiler

const cxx_compiler::type*
cxx_compiler::declarations::specifier_seq::type::signed_handler(const cxx_compiler::type* T)
{
  using namespace error::declarations::specifier_seq::type;
  if ( type::m_signed )
    invalid_combination(parse::position,"signed","signed");
  if ( type::m_unsigned )
    invalid_combination(parse::position,"unsigned","signed");
  type::m_signed = true;
  signed_impl::table::const_iterator p =
    signed_impl::m_table.find(T);
  if ( p != signed_impl::m_table.end() )
    return p->second;
  else {
    invalid_combination(parse::position,T,"signed");
    return short_type::create();
  }
}

cxx_compiler::declarations::specifier_seq::type::signed_impl::table::table()
{
  (*this)[0] = int_type::create();
  (*this)[char_type::create()] = schar_type::create();
  (*this)[short_type::create()] = short_type::create();
  (*this)[int_type::create()] = int_type::create();
  (*this)[long_type::create()] = long_type::create();
  (*this)[long_long_type::create()] = long_long_type::create();
}

namespace cxx_compiler { namespace declarations { namespace specifier_seq { namespace type {
  namespace unsigned_impl {
    struct table : std::map<const cxx_compiler::type*, const cxx_compiler::type*> {
      table();
    } m_table;
  } // end of namespace unsigned_impl
} } } } // end of namespace type, specifier_seq, declarations and cxx_compiler

const cxx_compiler::type*
cxx_compiler::declarations::specifier_seq::type::unsigned_handler(const cxx_compiler::type* T)
{
  using namespace error::declarations::specifier_seq::type;
  if ( type::m_signed )
    invalid_combination(parse::position,"signed","unsigned");
  if ( type::m_unsigned )
    invalid_combination(parse::position,"unsigned","unsigned");
  type::m_unsigned = true;
  unsigned_impl::table::const_iterator p =
    unsigned_impl::m_table.find(T);
  if ( p != unsigned_impl::m_table.end() )
    return p->second;
  else {
    invalid_combination(parse::position,T,"unsigned");
    return uint_type::create();
  }
}

cxx_compiler::declarations::specifier_seq::type::unsigned_impl::table::table()
{
  (*this)[0] = uint_type::create();
  (*this)[char_type::create()] = uchar_type::create();
  (*this)[short_type::create()] = ushort_type::create();
  (*this)[int_type::create()] = uint_type::create();
  (*this)[long_type::create()] = ulong_type::create();
  (*this)[long_long_type::create()] = ulong_long_type::create();
}

const cxx_compiler::type*
cxx_compiler::declarations::specifier_seq::type::float_handler(const cxx_compiler::type* T)
{
  if ( T ){
    using namespace error::declarations::specifier_seq::type;
    multiple(parse::position,T,float_type::create());
  }
  return float_type::create();
}

const cxx_compiler::type*
cxx_compiler::declarations::specifier_seq::type::float128_handler(const cxx_compiler::type* T)
{
  if ( T ){
    using namespace error::declarations::specifier_seq::type;
    multiple(parse::position,T,float128_type::create());
  }
  return float128_type::create();
}

const cxx_compiler::type*
cxx_compiler::declarations::specifier_seq::type::double_handler(const cxx_compiler::type* T)
{
  if ( T ){
    using namespace error::declarations::specifier_seq::type;
    multiple(parse::position,T,double_type::create());
  }
  return double_type::create();
}

const cxx_compiler::type*
cxx_compiler::declarations::specifier_seq::type::void_handler(const cxx_compiler::type* T)
{
  if ( T ){
    using namespace error::declarations::specifier_seq::type;
    multiple(parse::position,T,void_type::create());
  }
  return void_type::create();
}

namespace cxx_compiler { namespace declarations { namespace specifier_seq { namespace type {
  usr* g_usr;
  const cxx_compiler::type* qualifier(const cxx_compiler::type*, int);
} } } } // end of namespace type, specifier_seq, declarations and cxx_compiler

const cxx_compiler::type*
cxx_compiler::declarations::specifier_seq::type::const_handler(const cxx_compiler::type* T)
{
  return qualifier(T, 1 << 0);
}

const cxx_compiler::type*
cxx_compiler::declarations::specifier_seq::type::volatile_handler(const cxx_compiler::type* T)
{
  return qualifier(T, 1 << 1);
}

const cxx_compiler::type*
cxx_compiler::declarations::specifier_seq::type::restrict_handler(const cxx_compiler::type* T)
{
  return qualifier(T, 1 << 2);
}

const cxx_compiler::type*
cxx_compiler::declarations::specifier_seq::type::qualifier(const cxx_compiler::type* T, int cvr)
{
  using namespace std;
  if ( !T ){
    using namespace error::declarations::specifier_seq::type;
    type::g_usr ? implicit_int(type::g_usr) : implicit_int(parse::position);
    T = int_type::create();
  }
  return T->qualified(cvr);
}

void cxx_compiler::declarations::specifier_seq::info_t::update()
{
  using namespace std;
  sort(m_tmp.begin(),m_tmp.end());
  type::m_short = type::m_signed = type::m_unsigned = false;
  m_type = accumulate(m_tmp.begin(),m_tmp.end(),m_type,updator);
  m_tmp.clear();
}

std::stack<cxx_compiler::declarations::specifier_seq::info_t*>
cxx_compiler::declarations::specifier_seq::info_t::s_stack;

namespace cxx_compiler {
  void debug_stack()
  {
    using namespace declarations::specifier_seq;
    stack<info_t*> tmp;
    while (!info_t::s_stack.empty()) {
      info_t* p = info_t::s_stack.top();
      tmp.push(p);
      info_t::s_stack.pop();
    }

    while (!tmp.empty()) {
      info_t* p = tmp.top();
      info_t::s_stack.push(p);
      tmp.pop();
    }
  }
} // end of namespace cxx_compiler

namespace cxx_compiler {
  namespace declarations {
    usr* check_installed(usr*, specifier_seq::info_t*, bool*);
    usr* exchange(bool installed, usr* new_one, usr* org);
    usr* action2(usr*);
    inline bool just_static_member_decl(usr* u)
    {
      scope* p = u->m_scope;
      if (p->m_id != scope::TAG)
        return false;
      usr::flag_t flag = u->m_flag;
      if (!(flag & usr::STATIC))
        return false;
      return !(flag & usr::STATIC_DEF);
    }
  } // end of namespace declarations
} // end of namespace cxx_compiler

cxx_compiler::usr*
cxx_compiler::declarations::action1(var* v, bool ini)
{
  using namespace std;
  using namespace error::declarations::specifier_seq::type;
  if (ini) {
    using namespace parse;
    if (last_token != '=' && last_token != '(') {
      if (!context_t::all.empty()) {
        // Not declaration. This situation causes retry.
        return 0;
      }
    }
  }
  if (genaddr* ga = v->genaddr_cast())
    v = ga->m_ref;
  assert(v->usr_cast());
  usr* u = static_cast<usr*>(v);
  const type* T = u->m_type;
  usr::flag_t flag = u->m_flag;
  bool installed = (flag & usr::OVERLOAD) ? true : !T->backpatch();
  if ( specifier_seq::info_t::s_stack.empty() ){
    usr::flag_t mask = usr::flag_t(usr::CTOR | usr::DTOR);
    if (flag & mask) {
      assert(T->m_id == type::FUNC);
      if (installed) {
        typedef const func_type FT;
        FT* ft = static_cast<FT*>(T);
        assert(!ft->return_type());
      }
      else {
        assert(T->backpatch());
        u->m_type = T = T->patch(0,u);
      }
    }
    else if (!(flag & usr::OVERLOAD)) {
      implicit_int(u);
      u->m_type = T = int_type::create();
    }
  }
  else if (specifier_seq::info_t* p = specifier_seq::info_t::s_stack.top()) {
    usr::flag_t mask = usr::flag_t(usr::CTOR | usr::DTOR);
    if (flag & mask) {
      assert(!p->m_type);
      assert(p->m_tmp.empty());
    }
    if (!p->m_type || !p->m_tmp.empty()) {
      declarations::specifier_seq::type::g_usr = u;
      p->update();
    }
    if (!p->m_type && !(flag & mask)) {
      implicit_int(u);
      p->m_type = int_type::create();
    }
    if (installed)
      u = check_installed(u, p, &installed);
    else {
      u->m_flag = flag = usr::flag_t(flag | p->m_flag);
      u->m_type = T = T->patch(p->m_type,u);
      flag = u->m_flag;
    }
  }
  else {
    usr::flag_t mask = usr::flag_t(usr::CTOR | usr::DTOR);
    if (flag & mask) {
      assert(T->m_id == type::FUNC);
      if (installed) {
        typedef const func_type FT;
        FT* ft = static_cast<FT*>(T);
        assert(!ft->return_type());
      }
      else {
        assert(T->backpatch());
        u->m_type = T = T->patch(0, u);
      }
    }
  }
  if (flag & usr::TYPEDEF) {
    type_def* tmp = new type_def(*u);
    u = exchange(installed, tmp, u);
  }
  if (ini) {
    parse::identifier::mode = parse::identifier::look;
    if (duration::_static(u)) {
      with_initial* tmp = new with_initial(*u);
      u = exchange(installed, tmp, u);
      if (scope::current != &scope::root)
        expressions::constant_flag = true;
    }
    else {
      const type* T = u->m_type;
      int cvr = 0;
      T->unqualified(&cvr);
      if (cvr & 1) {
	if (T->scalar()) {
	  const_usr* cu = new const_usr(*u);
	  u = cu;
	}
      }
    }
  }
  T = u->m_type;
  flag = u->m_flag;
  string name = u->m_name;
  block* b = 0;
  if (scope::current->m_id == scope::BLOCK)
    b = static_cast<block*>(scope::current);
  usr::flag_t mask =
    usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::FUNCTION | usr::VL);
  if (!(flag & mask)) {
    if (b || scope::current == &scope::root) {
      typedef const array_type ARRAY;
      ARRAY* array = T->m_id == type::ARRAY ? static_cast<ARRAY*>(T) : 0;
      if ( !array || array->dim() || !ini ) {
        check_object(u);
        T = u->m_type;
      }
    }
  }
  else if (flag & usr::EXTERN) {
    if (ini) {
      if (scope::current == &scope::root) {
        using namespace warning::declarations::initializers;
        with_extern(u);
      }
      else {
        using namespace error::declarations::initializers;
        with_extern(u);
      }
      u->m_flag = flag = usr::flag_t(flag & ~usr::EXTERN);
    }
  }
  else if (flag & usr::FUNCTION) {
    usr::flag_t mask = usr::flag_t(usr::STATIC | usr::AUTO | usr::REGISTER);
    if ( flag & mask ){
      if ( b ){
        using namespace error::declarations::declarators::function;
        invalid_storage(u);
        flag = usr::flag_t(flag & ~mask);
      }
    }
    check_abstract_func(u);
  }
  if ((flag & usr::VL) && ini) {
    using namespace error::declarations::declarators::array;
    variable_length::initializer(u);
  }
  if (b) {
    usr::flag_t mask = usr::flag_t(usr::STATIC | usr::EXTERN);
    if (flag & mask) {
      if (fundef::current->m_usr->m_flag & usr::INLINE) {
        using namespace error::declarations::specifier_seq::function;
        func_spec::static_storage(u);
        fundef::current->m_usr->m_flag =
          usr::flag_t(fundef::current->m_usr->m_flag & ~usr::INLINE);
      }
    }
  }
  if (flag & usr::INLINE) {
    using namespace error::declarations::specifier_seq::function;
    if ( !(flag & usr::FUNCTION) )
      not_function(u);
    else {
      string name = u->m_name;
      if ( u->m_name == "main" )
        func_spec::main(u);
    }
  }
  if (b) {
    scope* param = b->m_parent;
    if ( param->m_parent == &scope::root ){
      const map<string, vector<usr*> >& usrs = param->m_usrs;
      map<string, vector<usr*> >::const_iterator p =
        usrs.find(name);
      if (p != usrs.end()) {
        using namespace error::declarations;
        usr* prev = p->second.back();
        redeclaration(prev,u,true);
      }
    }
  }
  else if (scope::current == &scope::root) {
    usr::flag_t mask = usr::flag_t(usr::AUTO | usr::REGISTER);
    if ( flag & mask ){
      using namespace error::declarations::external;
      invalid_storage(parse::position);
      flag = usr::flag_t(flag & ~mask);
    }
    u->m_type = T = T->vla2a();
  }
  if (T) {
    const type* U = T->unqualified();
    if (U->m_id == type::REFERENCE) {
      usr::flag2_t flag2 = u->m_flag2;
      if (!(flag & usr::EXTERN) && !(flag2 & usr::DECLED_HANDLER)) {
        switch (scope::current->m_id) {
        case scope::PARAM:
        case scope::TAG:
          break;
        default:
          if (!ini) {
            using namespace error::declarations::declarators;
            reference::missing_initializer(u);
          }
          break;
        }
      }
    }
  }

  if (!parse::templ::param) {
    assert(!class_or_namespace_name::before.empty());
    scope* ptr = class_or_namespace_name::before.back();
    if (scope::current->m_id == scope::TAG) {
      tag* tmp = static_cast<tag*>(scope::current);
      if (tmp->m_bases) {
	tag::flag_t flag = tmp->m_flag;
	tag::flag_t mask = tag::flag_t(tag::INSTANTIATE | tag::SPECIAL_VER);
	if (flag & mask)
	  ptr = scope::current;
      }
    }
    const vector<scope::tps_t>& tps = ptr->m_tps;
    if (!tps.empty()) {
      const scope::tps_t& b = tps.back();
      if (!b.m_table.empty()) {
        using namespace parse::templ;
        if (!save_t::nest.empty()) {
	  save_t* p = save_t::nest.back();
	  assert(!p->m_usr);
	  p->m_usr = u = new template_usr(*u, b, p->m_patch_13_2);
	}
      }
    }
  }

  if (!installed) {
    if (parse::templ::param) {
      vector<scope::tps_t>& tps = scope::current->m_tps;
      if (!tps.empty()) {
        scope::tps_t& b = tps.back();
        map<string, scope::tps_t::value_t>& table = b.m_table;
        map<string, scope::tps_t::value_t>::const_iterator p =
          table.find(name);
        if (p != table.end())
          error::not_implemented();
        table[name].second = new scope::tps_t::val2_t(T, 0);
        vector<string>& order = b.m_order;
        order.push_back(name);
      }
    }
    else
      u = action2(u);
  }

  if (!ini) {
    usr::flag_t mask = 
      usr::flag_t(usr::TYPEDEF |usr::FUNCTION | usr::OVERLOAD);
    usr::flag2_t flag2 = u->m_flag2;
    usr::flag2_t mask2 = usr::TEMPLATE;
    if (!(flag & mask) && is_external_declaration(u) &&
        !just_static_member_decl(u) && !(flag2 & mask2)) {
      if (must_call_default_ctor(u))
        initialize_ctor_code(u);
      if (must_call_dtor(u->m_type))
        terminate_dtor_code(u);
    }
  }

  return u;
}

void cxx_compiler::declarations::check_object(usr* u)
{
  const type* T = u->m_type;
  u->m_type = T = T->complete_type();
  if (tag* ptr = T->get_tag()) {
    tag::flag_t flag = ptr->m_flag;
    if (flag & tag::TYPENAMED)
      return;
    if (ptr->m_kind == tag::TYPENAME) {
      using namespace parse::templ;
      if (!save_t::nest.empty())
        return;
      if (instantiate_with_template_param<template_usr>())
	return;
    }
  }
  int size = T->size();
  if (!size) {
    if (tag* x = T->get_tag()) {
      scope* parent = x->m_parent;
      if (parent->m_id == scope::TAG) {
        tag* y = static_cast<tag*>(parent);
        tag::flag_t flag = y->m_flag;
        if (flag & tag::INSTANTIATE) {
          string name = x->m_name;
          int r = parse::identifier::lookup(name, parent);
          assert(r == TYPEDEF_NAME_LEX);
          usr* v = cxx_compiler_lval.m_usr;
          const type* T = v->m_type;
          u->m_type = T;
          size = T->size();
        }
      }
    }
  }
  if (!size) {
    if (tag* ptr = T->get_tag()) {
      tag::kind_t kind = ptr->m_kind;
      if (kind == tag::TYPENAME)
	return;
    }
    using namespace error::declarations;
    not_object(u,T);
    u->m_type = int_type::create();
  }
  check_abstract_obj(u);
}

namespace cxx_compiler { namespace declarations {
  bool conflict(usr*, usr*);
  usr* combine(usr*, usr*);
  inline bool new_or_delete(string name)
  {
    if (name == operator_name(NEW_KW))
      return true;
    if (name == operator_name(DELETE_KW))
      return true;
    if (name == operator_name(NEW_ARRAY_LEX))
      return true;
    return name == operator_name(DELETE_ARRAY_LEX);
  }
  tag* friend_tag()
  {
    if (scope::current->m_id == scope::TAG)
      return static_cast<tag*>(scope::current);
    if (class_or_namespace_name::before.empty())
      error::not_implemented();
    scope* ps = class_or_namespace_name::before.back();
    assert(ps->m_id == scope::TAG);
    return static_cast<tag*>(ps);
  }
} } // end of namespace declarations and cxx_compiler

cxx_compiler::usr* cxx_compiler::declarations::action2(usr* curr)
{
  using namespace std;
  if (!(curr->m_flag2 & usr::NESTED_MEMBER))
    curr->m_scope = scope::current;
  string name = curr->m_name;
  if ( name == "__func__" ){
    using namespace error::expressions::primary::underscore_func;
    declared(parse::position);
  }
  else {
    usr::flag_t flag = curr->m_flag;
    if ( !(flag & usr::TYPEDEF ) ){
      if (name == "main")
        curr->m_flag = usr::flag_t(flag | usr::C_SYMBOL);
      else if (!declarations::linkage::infos.empty()) {
        using namespace declarations::linkage;
        const info_t& info = infos.back();
        switch (scope::current->m_id) {
        case scope::NONE: case scope::NAMESPACE:
          {
            if (info.m_kind == info_t::C)
              curr->m_flag = usr::flag_t(flag | usr::C_SYMBOL);
          }
        }
      }
    }
  }

  if (new_or_delete(name)) {
    if (scope::current->m_id == scope::TAG)
      curr->m_flag = usr::flag_t(curr->m_flag | usr::STATIC);
  }

  usr::flag_t flag = curr->m_flag;
  if (flag & usr::FRIEND) {
    tag* ptr = friend_tag();
    if (!(flag & usr::FUNCTION))
      error::not_implemented();
    tag::flag_t flag = ptr->m_flag;
    tag::flag_t mask = tag::flag_t(tag::TEMPLATE | tag::INSTANTIATE);
    if (flag & mask)
      return curr;
    if (curr->m_scope == ptr)
      curr->m_scope = ptr->m_parent;
    curr->m_flag = usr::flag_t(curr->m_flag & ~usr::FRIEND);
    curr = new friend_func(curr, ptr);
  }

  map<string, vector<usr*> >& usrs = curr->m_scope->m_usrs;
  map<string, vector<usr*> >::const_iterator p = usrs.find(name);
  if (p != usrs.end()) {
    const vector<usr*>& v = p->second;
    usr* prev = v.back();
    assert(prev != curr);
    if (conflict(prev, curr)) {
      using namespace error::declarations;
      redeclaration(prev,curr,false);
    }
    else {
      curr = combine(prev, curr);
      usr::flag_t flag = curr->m_flag;
      if (flag & usr::OVERLOAD) {
        overload* ovl = static_cast<overload*>(curr);
        usr* tmp = ovl->m_candidacy.back();
        flag = tmp->m_flag;
      }
      if ((flag & usr::FUNCTION) && (flag & usr::EXTERN)){
        using namespace declarators::function::definition;
        using namespace static_inline;
        skip::table_t::iterator r = skip::stbl.find(prev);
        if (r != skip::stbl.end()) {
          info_t* info = r->second;
          usr* u = info->m_fundef->m_usr;
          u->m_flag = usr::flag_t(u->m_flag | usr::EXTERN);
          skip::stbl.erase(r);
          gencode(info);
        }
      }
    }
  }

  usrs[name].push_back(curr);
  if (curr->m_scope->m_id != scope::PARAM ||
      !(curr->m_flag & usr::ENUM_MEMBER)) {
    curr->m_scope->m_order.push_back(curr);
    if (curr->m_scope->m_id == scope::BLOCK) {
      block* b = static_cast<block*>(curr->m_scope);
      usr::flag_t mask =
        usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::FUNCTION);
      if (!(flag & mask)) {
        const type* T = curr->m_type;
        if (must_call_dtor(T))
          block_impl::dtor_tbl[b].push_back(curr);
      }
    }
  }
  class_or_namespace_name::last = 0;

  if (curr->m_flag2 & usr::INSTANTIATE) {
    typedef map<string, vector<usr*> >::iterator IT;
    IT p = usrs.find(name);
    assert(p != usrs.end());
    vector<usr*>& v = p->second;
    usr* ins = v.back();
    assert(ins == curr);
    v.pop_back();
    usr* prev = v.back();
    usr::flag2_t mask2 = usr::flag2_t(usr::TEMPLATE | usr::PARTIAL_ORDERING);
    if (prev->m_flag2 & mask2) {
      v.pop_back();
      v.push_back(ins);
      v.push_back(prev);
    }
    else if (prev->m_flag & usr::OVERLOAD) {
      overload* ovl = static_cast<overload*>(prev);
      const vector<usr*>& cand = ovl->m_candidacy;
      typedef vector<usr*>::const_iterator IT;
      IT p = find_if(begin(cand), end(cand),
                  [](usr* u){ return u->m_flag2 & usr::TEMPLATE; });
      if (p != end(cand)) {
        usr* templ = *p;
        instantiated_usr* iu = static_cast<instantiated_usr*>(curr);
        assert(iu->m_src == templ);
        v.pop_back();
        v.push_back(ins);
        v.push_back(prev);
      }
      else {
        // instantiated via `template_usr::instantiate(const KEY&)'
        assert(ins->m_scope->m_id == scope::TAG);
        v.push_back(ins);
      }
    }
    else {
      // instantiated via `template_usr::instantiate(const KEY&)'
      assert(ins->m_scope->m_id == scope::TAG);
      v.push_back(ins);
    }
  }

  usr::flag2_t flag2 = curr->m_flag2;
  if ((flag2 & usr::TEMPLATE) && !(flag2 & usr::INSTANTIATE)){
    vector<usr*>& v = usrs[name];
    int n = v.size();
    if (n >= 2) {
      assert(v[n-1] == curr);
      usr* prev = v[n-2];
      usr::flag2_t flag2 = prev->m_flag2;
      usr::flag2_t mask =
	usr::flag2_t(usr::PARTIAL_ORDERING | usr::FUNCTION_DEFINITION);
      if (flag2 & mask)
        swap(v[n-1], v[n-2]);
    }
  }

  return curr;
}

namespace cxx_compiler {
  namespace declarations {
    bool conflict(usr::flag_t, usr::flag_t);
    bool conflict(const type*, const type*);
    bool conflict_tu(template_usr*, template_usr*);
  } // end of namespace declarations
} // end of namespace cxx_compiler

bool cxx_compiler::declarations::conflict(usr* x, usr* y)
{
  if (conflict(x->m_flag,y->m_flag)) {
    if (!x->m_type) {
      usr::flag_t flag = x->m_flag;
      usr::flag2_t flag2 = x->m_flag2;
      assert((flag & usr::OVERLOAD) || (flag2 & usr::PARTIAL_ORDERING));
      return false;
    }
    if (x->m_type->m_id != type::FUNC)
      return true;
  }
  if (scope::current == &scope::root) {
    if ((x->m_flag & usr::WITH_INI) && (y->m_flag & usr::WITH_INI))
      return true;
  }
  usr::flag_t flag = x->m_flag;
  if (flag & usr::OVERLOAD)
    return false;
  usr::flag2_t flag2 = x->m_flag2;
  if (flag2 & usr::TEMPLATE) {
    if (!(y->m_flag2 & usr::TEMPLATE))
      return false;
    if (flag & usr::FRIEND) {
      friend_func* ff = static_cast<friend_func*>(x);
      x = ff->m_org;
    }
    template_usr* xt = static_cast<template_usr*>(x);
    template_usr* yt = static_cast<template_usr*>(y);
    return conflict_tu(xt, yt);
  }
  if (flag2 & usr::PARTIAL_ORDERING)
    return false;
  return conflict(x->m_type, y->m_type);
}

namespace cxx_compiler {
  namespace declarations {
    using namespace std;
    struct table_t {
      map<pair<usr::flag_t, usr::flag_t>,bool> m_root;
      map<pair<usr::flag_t, usr::flag_t>,bool> m_tag;
      map<pair<usr::flag_t, usr::flag_t>,bool> m_other;
      table_t();
    } ctbl;
  } // end of namespace declarations
} // end of namespace cxx_compiler

bool cxx_compiler::declarations::conflict(usr::flag_t x, usr::flag_t y)
{
  using namespace std;
  if ((x & usr::ENUM_MEMBER) || (y & usr::ENUM_MEMBER))
    return true;
  usr::flag_t mask =
    usr::flag_t(usr::TYPEDEF|usr::EXTERN|usr::STATIC|usr::AUTO|usr::REGISTER);
  x = usr::flag_t(x & mask);
  y = usr::flag_t(y & mask);
  pair<usr::flag_t, usr::flag_t> key(x,y);
  scope::id_t id = scope::current->m_id;
  switch (id) {
  case scope::NONE: case scope::NAMESPACE: return ctbl.m_root[key];
  case scope::TAG: return ctbl.m_tag[key];
  default: return ctbl.m_other[key];
  }
}

cxx_compiler::declarations::table_t::table_t()
{
  using namespace std;
  m_other[make_pair(usr::NONE,usr::NONE)] = true;
  m_tag[make_pair(usr::NONE,usr::NONE)] = true;

  m_root[make_pair(usr::TYPEDEF,usr::NONE)] = true;
  m_root[make_pair(usr::NONE,usr::TYPEDEF)] = true;
  m_other[make_pair(usr::TYPEDEF,usr::NONE)] = true;
  m_other[make_pair(usr::NONE,usr::TYPEDEF)] = true;
  m_root[make_pair(usr::NONE,usr::STATIC)] =
  m_other[make_pair(usr::NONE,usr::STATIC)] = true;
  m_root[make_pair(usr::STATIC,usr::NONE)] =
  m_other[make_pair(usr::STATIC,usr::NONE)]  = true;
  m_root[make_pair(usr::TYPEDEF,usr::TYPEDEF)] =
  m_other[make_pair(usr::TYPEDEF,usr::TYPEDEF)] = true;
}

bool cxx_compiler::declarations::conflict(const type* prev, const type* curr)
{
  if (compatible(prev, curr))
    return false;
  if ( prev->m_id != type::FUNC )
    return true;
  if ( curr->m_id != type::FUNC )
    return true;
  typedef const func_type FT;
  FT* fp = static_cast<FT*>(prev);
  FT* fc = static_cast<FT*>(curr);
  if (fp->overloadable(fc))
    return false;
  const type* rp = fp->return_type();
  const type* rc = fc->return_type();
  if (!rp || !rc)
    return true;
  tag* tp = rp->get_tag();
  tag* tc = rc->get_tag();
  if (!tp && !tc)
    return true;
  if (!tp) {
    tag::flag_t flagc = tc->m_flag;
    return !(flagc && tag::TYPENAMED);
  }
  if (!tc) {
    tag::flag_t flagp = tp->m_flag;
    return !(flagp & tag::TYPENAMED);
  }
  tag::flag_t flagc = tc->m_flag;
  tag::flag_t flagp = tp->m_flag;
  if (!(flagp & tag::TYPENAMED))
    return true;
  if (!(flagc && tag::TYPENAMED))
    return true;
  return false;
}

namespace cxx_compiler {
  namespace declarations {
    namespace conflict_impl {
      struct comp {
        const scope::tps_t& m_xtps;
        const scope::tps_t& m_ytps;
        comp(const scope::tps_t& xtps, const scope::tps_t& ytps)
          : m_xtps(xtps), m_ytps(ytps) {}
        bool operator()(string x, string y)
        {
          typedef scope::tps_t::value_t V;
          typedef map<string, V>::const_iterator IT;
          const map<string, V>& xtbl = m_xtps.m_table;
          const map<string, V>& ytbl = m_ytps.m_table;
          IT px = xtbl.find(x);
          assert(px != xtbl.end());
          const V& vx = px->second;
          IT py = ytbl.find(y);
          assert(py != ytbl.end());
          const V& vy = py->second;
          if (vx.first)
            return vy.first;
          assert(vx.second);
          return vy.second;
        }
      };
      struct templ_arg {
        const scope::tps_t& m_tps;
        templ_arg(const scope::tps_t& tps) : m_tps(tps) {}
        scope::tps_t::val2_t operator()(string name)
        {
          typedef scope::tps_t::value_t V;
          typedef map<string, V>::const_iterator IT;
          const map<string, V>& tbl = m_tps.m_table;
          IT p = tbl.find(name);
          assert(p != tbl.end());
          const V& v = p->second;
          if (tag* ptr = v.first) {
            const type* T = ptr->m_types.first;
            return scope::tps_t::val2_t(T, (var*)0);
          }
	  scope::tps_t::val2_t* pv = v.second;
	  const type* T = pv->first;
	  usr* u = new templ_param(name, T, usr::NONE, parse::position,
				   usr::TEMPL_PARAM);
          return scope::tps_t::val2_t((const type*)0, u);
        }
      };
      const type* tu_common(template_usr* x, template_usr* y)
      {
        using namespace conflict_impl;
        assert(x->m_name == y->m_name);
        const scope::tps_t& xtps = x->m_tps;
        const scope::tps_t& ytps = y->m_tps;
        const vector<string>& xo = xtps.m_order;
        const vector<string>& yo = ytps.m_order;
        if (xo.size() != yo.size())
          return 0;
        typedef vector<string>::const_iterator IT;
        pair<IT, IT> ret = mismatch(begin(xo), end(xo), begin(yo),
                                    comp(xtps, ytps));
        if (ret != make_pair(end(xo), end(yo)))
          return 0;
        template_usr::KEY key;
        transform(begin(yo), end(yo), back_inserter(key), templ_arg(ytps));
        typedef template_usr::info_t X;
        typedef template_tag::info_t Y;
        tinfos.push_back(make_pair((X*)0, (Y*)1));
        usr* xi = x->instantiate(key);
        tinfos.pop_back();
        scope* ps = xi->m_scope;
        map<string, vector<usr*> >& usrs = ps->m_usrs;
        map<string, vector<usr*> >::iterator p = usrs.find(xi->m_name);
        vector<usr*>& v = p->second;
        if (v.size() >= 2) {
          usr* b = v.back();
          usr::flag_t flag = b->m_flag;
          if (!(flag & usr::OVERLOAD)) {
            if (flag & usr::FRIEND) {
              friend_func* ff = static_cast<friend_func*>(b);
              assert(ff->m_org == x);
            }
            else {
              usr::flag2_t flag2 = b->m_flag2;
              if (flag2 & usr::PARTIAL_ORDERING) {
                partial_ordering* po = static_cast<partial_ordering*>(b);
                const vector<template_usr*>& c = po->m_candidacy;
                assert(!c.empty());
                assert(find(begin(c), end(c), x) != end(c));
              }
              else
                assert(b == x);
            }
            v.pop_back();
            if (v.back() == xi) {
              v.pop_back();
              v.push_back(b);
            }
            else
              v.push_back(b);
          }
        }
        else {
          assert(v.size() == 1);
          usr* b = v.back();
          if (b->m_flag & usr::FRIEND) {
            friend_func* ff = static_cast<friend_func*>(b);
            assert(ff->m_org == x);
          }
          else
            assert(b == x);
        }
        const type* Tx = xi->m_type;
        return Tx;
      }
    } // end of namespace conflict_impl
  } // end of namespace declarations
} // end of namespace cxx_compiler

bool
cxx_compiler::declarations::conflict_tu(template_usr* x, template_usr* y)
{
  using namespace conflict_impl;
  const type* Tx = tu_common(x, y);
  if (!Tx)
    return false;
  const type* Ty = y->m_type;
  return conflict(Tx, Ty);
}

namespace cxx_compiler {
  namespace declarations {
    const type* composite_tu(template_usr* x, template_usr* y)
    {
      using namespace conflict_impl;
      const type* Tx = tu_common(x, y);
      if (!Tx)
        return 0;
      const type* Ty = y->m_type;
      return composite(Tx, Ty);
    }
    usr::flag_t combine(scope::id_t id, usr::flag_t prev, usr::flag_t curr)
    {
      switch (id) {
      case scope::NONE: case scope::NAMESPACE:
        {
          if (prev == usr::NONE && curr == usr::NONE)
            return usr::EXTERN;
          else if (prev & usr::STATIC)
            return usr::flag_t(curr | usr::STATIC);
          else if (prev & usr::INLINE)
            return usr::flag_t(curr | usr::INLINE);
          if (prev & usr::C_SYMBOL)
            return usr::flag_t(curr | usr::C_SYMBOL);
          return curr;
        }
        break;
      case scope::TAG:
        {
          if (prev & usr::STATIC) {
            if (prev & usr::FUNCTION) {
              assert(curr & usr::FUNCTION);
              return usr::flag_t(curr | usr::STATIC);
            }
            else {
              assert(!(curr & usr::FUNCTION));
              return usr::flag_t(curr | usr::STATIC_DEF);
            }
          }
        }
        return curr;
        break;
      default:
        return curr;
      }
    }
  } // end of namespace declarations
} // end of namespace cxx_compiler

cxx_compiler::usr* cxx_compiler::declarations::combine(usr* prev, usr* curr)
{
  using namespace std;
  const type* Tp = prev->m_type;
  const type* Tc = curr->m_type;
  if (Tp && Tc && compatible(Tp, Tc)) {
    scope::id_t id = curr->m_scope->m_id;
    curr->m_flag = combine(id, prev->m_flag, curr->m_flag);
  }

  usr::flag_t flag = prev->m_flag;
  usr* old_prev = prev;
  if (flag & usr::OVERLOAD) {
    overload* ovl = static_cast<overload*>(prev);
    const vector<usr*>& cand = ovl->m_candidacy;
    if (parse::templ::ptr) {
      typedef vector<usr*>::const_iterator IT;
      IT p = find_if(begin(cand), end(cand),
                  [](usr* u){ return u->m_flag2 & usr::TEMPLATE; });
      if (p != end(cand))
        prev = *p;
      else if (!template_usr::nest.empty()) {
        template_usr::info_t& info = template_usr::nest.back();
        template_usr* tu = info.m_tu;
	if (tu->m_name == prev->m_name)
	  prev = tu;
      }
    }
    else {
      usr::flag2_t flag2 = curr->m_flag2;
      if ((flag2 & usr::TEMPLATE) || !templ::specialization::nest.empty()) {
        typedef vector<usr*>::const_iterator IT;
        IT p = find_if(begin(cand), end(cand),
                       [](usr* u){ return u->m_flag2 & usr::TEMPLATE; });
        if (p != end(cand))
          prev = *p;
      }
    }
  }

  const type* x = prev->m_type;
  if (x) {
    const type* y = curr->m_type;
    if (const type* z = composite(x, y)) {
      curr->m_type = z;
      if (parse::templ::ptr) {
        assert(!template_usr::nest.empty());
        template_usr::info_t& info = template_usr::nest.back();
        template_usr* tu = info.m_tu;
        templ_base::KEY key;
        if (!instance_of(tu, curr, key))
          error::not_implemented();
        curr = info.m_iu = new instantiated_usr(*curr, tu, key);
        if (info.m_mode == template_usr::info_t::EXPLICIT) {
          curr->m_flag2 =
            usr::flag2_t(curr->m_flag2 | usr::EXPLICIT_INSTANTIATE);
        }
        return curr;
      }
      if (curr->m_flag2 & usr::TEMPLATE) {
        template_usr* ctu = static_cast<template_usr*>(curr);
        ctu->m_prev = prev;
        if (prev->m_flag2 & usr::TEMPLATE) {
          template_usr* ptu = static_cast<template_usr*>(prev);
          ptu->m_next = ctu;
        }
      }
      return curr;
    }
  }

  usr::flag2_t flag2 = prev->m_flag2;
  if (flag2 & usr::TEMPLATE) {
    if (flag & usr::FRIEND) {
      friend_func* ff = static_cast<friend_func*>(prev);
      prev = ff->m_org;
    }
    template_usr* ptu = static_cast<template_usr*>(prev);
    if (curr->m_flag2 & usr::TEMPLATE) {
      template_usr* ctu = static_cast<template_usr*>(curr);
      if (const type* T = composite_tu(ptu, ctu)) {
        ctu->m_type = T;
        ctu->m_prev = ptu;
        ptu->m_next = ctu;
        if (old_prev->m_flag & usr::OVERLOAD) {
          string name = curr->m_name;
          scope::current->m_usrs[name].push_back(curr);
          return new overload(old_prev, curr);
        }
        return ctu;
      }
      string name = curr->m_name;
      scope::current->m_usrs[name].push_back(curr);
      return new partial_ordering(ptu, ctu);
    }
    templ_base::KEY key;
    if (instance_of(ptu, curr, key) ||
        template_usr::explicit_instantiating(key)) {
      scope::id_t id = curr->m_scope->m_id;
      curr->m_flag = combine(id, prev->m_flag, curr->m_flag);
      instantiated_usr* ret = new instantiated_usr(*curr, ptu, key);
      if (parse::templ::ptr) {
        if (!template_usr::nest.empty()) {
          template_usr::info_t& info = template_usr::nest.back();
	  assert(ptu == info.m_tu || ptu->m_prev == info.m_tu ||
		 (info.m_tu->m_flag2 & usr::PARTIAL_INSTANTIATED));
          assert(key == info.m_key);
          info.m_iu = ret;
          if (info.m_mode == template_usr::info_t::EXPLICIT)
            ret->m_flag2 =
              usr::flag2_t(ret->m_flag2 | usr::EXPLICIT_INSTANTIATE);
          return ret;
        }
      }
      ret->m_flag2 = usr::flag2_t(ret->m_flag2 | usr::SPECIAL_VER);
      ptu->m_table[key] = ret;
      return ret;
    }
  }

  if (flag2 & usr::PARTIAL_ORDERING) {
    partial_ordering* po = static_cast<partial_ordering*>(prev);
    if (!(curr->m_flag2 & usr::TEMPLATE)) {
      assert(parse::templ::ptr);
      assert(!template_usr::nest.empty());
      template_usr::info_t& info = template_usr::nest.back();
      template_usr* tu = info.m_tu;
      const vector<template_usr*>& c = po->m_candidacy;
      assert((find(begin(c), end(c), tu) != end(c)) ||
             (find_if(begin(c), end(c), [tu](template_usr* ptu)
                      { return composite_tu(ptu, tu); }) != end(c)));
      templ_base::KEY key;
      bool b = instance_of(tu, curr, key);
      assert(b);
      instantiated_usr* ret = new instantiated_usr(*curr, tu, key);
      info.m_iu = ret;
      return ret;
    }

    template_usr* ctu = static_cast<template_usr*>(curr);
    const vector<template_usr*>& c = po->m_candidacy;
    typedef vector<template_usr*>::const_iterator IT;
    IT p = find_if(begin(c), end(c), [ctu](template_usr* ptu)
                   { return composite_tu(ptu, ctu); });
    if (p != end(c)) {
      template_usr* ptu = *p;
      ctu->m_prev = ptu;
      ptu->m_next = ctu;
      return ctu;
    }

    string name = curr->m_name;
    scope::current->m_usrs[name].push_back(curr);
    return new partial_ordering(po, ctu);
  }

  string name = curr->m_name;
  scope::current->m_usrs[name].push_back(curr);
  return new overload(prev, curr);
}

namespace cxx_compiler {
  namespace overload_impl {
    bool match(usr* prev, usr* curr)
    {
      if (compatible(prev->m_type, curr->m_type))
        return true;
      usr::flag2_t pf = prev->m_flag2;
      if (!(pf & usr::TEMPLATE))
        return false;
      usr::flag2_t cf = curr->m_flag2;
      if (!(cf & usr::TEMPLATE))
        return false;
      return true;
    }
  } // end of namespace overload_impl
  overload::overload(usr* prev, usr* curr)
    : usr(curr->m_name, 0, usr::OVERLOAD, curr->m_file, usr::NONE2), m_obj(0)
  { 
    usr::flag_t flag = prev->m_flag;
    if (!(flag & usr::OVERLOAD)) {
      m_candidacy.push_back(prev);
      m_candidacy.push_back(curr);
      return;
    }

    overload* ovl = static_cast<overload*>(prev);
    m_candidacy = ovl->m_candidacy;
    typedef vector<usr*>::iterator IT;
    IT p = find_if(begin(m_candidacy), end(m_candidacy),
                   bind2nd(ptr_fun(overload_impl::match), curr));
    if (p != end(m_candidacy))
      *p = curr;
    else
      m_candidacy.push_back(curr);
  }
  partial_ordering::partial_ordering(template_usr* prev, template_usr* curr)
    : usr(curr->m_name, 0, usr::NONE, curr->m_file, usr::PARTIAL_ORDERING),
      m_obj(0)
  {
    m_candidacy.push_back(prev);
    m_candidacy.push_back(curr);
  }
  partial_ordering::partial_ordering(partial_ordering* prev,
                                     template_usr* curr)
    : usr(curr->m_name, 0, usr::NONE, curr->m_file, usr::PARTIAL_ORDERING),
      m_obj(0)
  {
    m_candidacy = prev->m_candidacy;
    m_candidacy.push_back(curr);
  }
} // end of namespace cxx_compiler

cxx_compiler::usr*
cxx_compiler::declarations::
check_installed(usr* u, specifier_seq::info_t* p, bool* installed)
{
  usr::flag_t flag = u->m_flag;
  if ((flag & usr::EXTERN) && !(p->m_flag & usr::EXTERN)) {
    usr* ret = new usr(*u);
    ret->m_flag = usr::flag_t((flag & ~usr::EXTERN) | p->m_flag);
    ret->m_file = parse::position;
    *installed = false;
    return ret;
  }

  scope* ps = u->m_scope;
  const type* Tu = u->m_type;
  if (Tu)
    Tu = Tu->complete_type();
  if (ps->m_id == scope::TAG && (flag & usr::STATIC) && Tu && Tu->size()) {
    usr* ret = new usr(*u);
    ret->m_flag = usr::flag_t(flag | p->m_flag | usr::STATIC_DEF);
    ret->m_flag = usr::flag_t(ret->m_flag & ~usr::WITH_INI);
    ret->m_file = parse::position;
    map<string, vector<usr*> >& usrs = ps->m_usrs;
    string name = u->m_name;
    if (parse::templ::ptr) {
      assert(!template_usr::nest.empty());
      template_usr::info_t& info = template_usr::nest.back();
      template_usr* tu = info.m_tu;
      templ_base::KEY key;
      if (!instance_of(tu, ret, key))
        error::not_implemented();
      ret = info.m_iu = new instantiated_usr(*ret, tu, key);
      assert(info.m_mode == template_usr::info_t::STATIC_DEF);
    }
    usrs[name].push_back(ret);
    return ret;
  }

  u->m_flag = flag = usr::flag_t(flag | p->m_flag);
  if (u->m_flag2 & usr::TEMPLATE)
    u->m_flag = flag = usr::flag_t(flag & ~usr::FRIEND);
  if (flag & usr::OVERLOAD)
    return u;
  assert(!Tu->backpatch());
  if (Tu->m_id == type::FUNC) {
    typedef const func_type FT;
    FT* ft = static_cast<FT*>(Tu);
    const type* T = ft->return_type();
    const type* Tp = p->m_type;
    if (T && Tp) {
      if (!compatible(T, Tp))
        error::not_implemented();
    }
    else if (T || Tp)
      error::not_implemented();
  }
  if (flag & usr::FRIEND) {
    if (!(p->m_flag & usr::FRIEND))
      return u;
  }
  usr* ret = new usr(*u);
  ret->m_file = parse::position;
  *installed = false;
  return ret;
}

cxx_compiler::usr*
cxx_compiler::declarations::exchange(bool installed, usr* new_one, usr* org)
{
  using namespace std;
  if (installed) {
    string name = org->m_name;
    map<string, vector<usr*> >& usrs = org->m_scope->m_usrs;
    vector<usr*>& v = usrs[name];
    assert(v.back() == org);
    v.back() = new_one;
  }

  if (!template_usr::nest.empty()) {
    const template_usr::info_t& info = template_usr::nest.back();
    if (info.m_iu == org)
      return new_one;
  }

  delete org;
  using namespace parse::templ;
  for (auto p : save_t::nest) {
    list<void*>& lv = p->m_read.m_lval;
    replace(begin(lv), end(lv), org, new_one);
  }

  return new_one;
}

namespace cxx_compiler {
  inline bool del()
  {
    const vector<scope::tps_t>& tps = scope::current->m_tps;
    if (tps.empty())
      return true;
    const scope::tps_t& b = tps.back();
    const map<string, scope::tps_t::value_t>& table = b.m_table;
    return table.empty();
  }
} // end of namespace cxx_compiler

const cxx_compiler::type*
cxx_compiler::declarations::elaborated::action(int keyword, var* v)
{
  using namespace std;
  assert(v->usr_cast());
  usr* u = static_cast<usr*>(v);
  auto_ptr<usr> sweeper(del() ? u : 0);
  string name = u->m_name;
  tag* T = lookup(name, scope::current);
  if (T) {
    const pair<const type*, const type*>& p = T->m_types;
    return p.second ? p.second : p.first;
  }
  else {
    scope* parent = scope::current;
    if (!linkage::infos.empty()) {
      linkage::info_t& info = linkage::infos.back();
      if (info.m_kind == linkage::info_t::C)
	parent = &scope::root;
    }
    tag::kind_t kind = classes::specifier::get(keyword);
    const file_t& file = u->m_file;
    tag* ptr = new tag(kind,name,file,0);
    const vector<scope::tps_t>& tps = scope::current->m_tps;
    if (!tps.empty()) {
      const scope::tps_t& b = tps.back();
      if (!b.m_table.empty()) {
        using namespace parse::templ;
        assert(!save_t::nest.empty());
        save_t* p = save_t::nest.back();
        assert(!p->m_tag);
        assert(!class_or_namespace_name::before.empty());
        assert(class_or_namespace_name::before.back() == ptr);
        p->m_tag = ptr = new template_tag(*ptr, b);
        class_or_namespace_name::before.back() = ptr;
      }
    }

    parent->m_tags[name] = ptr;
    ptr->m_parent = parent;
    parent->m_children.push_back(ptr);
    assert(!class_or_namespace_name::before.empty());
    class_or_namespace_name::before.pop_back();
    return ptr->m_types.first = incomplete_tagged_type::create(ptr);
  }
}

cxx_compiler::tag*
cxx_compiler::declarations::elaborated::lookup(std::string name, scope* ptr)
{
  using namespace std;
  map<string, tag*>& tags = ptr->m_tags;
  map<string, tag*>::const_iterator p = tags.find(name);
  if ( p != tags.end() )
    return p->second;
  if ( ptr->m_parent )
    return lookup(name,ptr->m_parent);
  else
    return 0;
}

const cxx_compiler::type*
cxx_compiler::declarations::
elaborated::action(int keyword, std::pair<usr*, tag*>* p)
{
  auto_ptr<pair<usr*, tag*> > sweeper(p);
  if (p->first)
    error::not_implemented();
  tag* ptr = p->second;
  tag::kind_t x = classes::specifier::get(keyword);
  tag::kind_t y = ptr->m_kind;
  if (x != y) 
    error::not_implemented();
  const type* T = ptr->m_types.second;
  if (T)
    return T;
  T = ptr->m_types.first;
  assert(T);
  return T;
}

const cxx_compiler::type*
cxx_compiler::declarations::
elaborated::action(int keyword, tag* ptr)
{
  tag::kind_t x = classes::specifier::get(keyword);
  tag::kind_t y = ptr->m_kind;
  if (x != y) 
    error::not_implemented();
  const type* T = ptr->m_types.second;
  if (T)
    return T;
  T = ptr->m_types.first;
  assert(T);
  return T;
}

void cxx_compiler::declarations::linkage::action(var* v, bool brace)
{
  using namespace std;
  // check if `v' is "C" or "C++"
  genaddr* p = v->genaddr_cast();
  assert(p);
  with_initial* q = static_cast<with_initial*>(p->m_ref);
  const map<int,var*>& value = q->m_value;
  if (value.size() == 2) {
    // check `v' is "C"
    map<int,var*>::const_iterator a = value.find(0);
    assert(a != value.end());
    assert(a->second->usr_cast());
    usr* b = static_cast<usr*>(a->second);
    constant<char>* c = static_cast<constant<char>*>(b);
    if (c->m_value != 'C')
      error::not_implemented();
    map<int,var*>::const_iterator d = value.find(1);
    assert(d != value.end());
    assert(d->second->usr_cast());
    usr* e = static_cast<usr*>(d->second);
    constant<char>* f = static_cast<constant<char>*>(e);
    assert(f->m_value == '\0');
    infos.push_back(info_t(info_t::C, brace));
  }
  else if (value.size() == 4) {
    // check `v' is "C++"
    map<int,var*>::const_iterator a = value.find(0);
    assert(a != value.end());
    assert(a->second->usr_cast());
    usr* b = static_cast<usr*>(a->second);
    constant<char>* c = static_cast<constant<char>*>(b);
    if (c->m_value != 'C')
      error::not_implemented();
    map<int,var*>::const_iterator d = value.find(1);
    assert(d != value.end());
    assert(d->second->usr_cast());
    usr* e = static_cast<usr*>(d->second);
    constant<char>* f = static_cast<constant<char>*>(e);
    if (f->m_value != '+')
      error::not_implemented();
    map<int,var*>::const_iterator g = value.find(2);
    assert(g != value.end());
    assert(g->second->usr_cast());
    usr* h = static_cast<usr*>(g->second);
    constant<char>* i = static_cast<constant<char>*>(h);
    if (i->m_value != '+')
      error::not_implemented();
    map<int,var*>::const_iterator j = value.find(3);
    assert(j != value.end());
    assert(j->second->usr_cast());
    usr* k = static_cast<usr*>(j->second);
    constant<char>* l = static_cast<constant<char>*>(k);
    assert(l->m_value == '\0');
    infos.push_back(info_t(info_t::CXX, brace));
  }
  else
    error::not_implemented();
}

namespace cxx_compiler {
  namespace declarations {
    using namespace std;
    vector<linkage::info_t> linkage::infos;
  }  // end of namespace declarations
}  // end of namespace cxx_compiler

cxx_compiler::declarations::type_specifier_seq::info_t::info_t(type_specifier* specifier, info_t* follow)
{
  if ( int n = specifier->m_keyword ){
    if ( follow ){
      m_tmp = follow->m_tmp;
      m_type = follow->m_type;
    }
    else
      m_type = 0;
    m_tmp.push_back(n);
  }
  else if ( specifier->m_type ){
    m_type = specifier->m_type;
  }
  else
    m_type = specifier->m_usr->m_type;
  delete specifier;
  delete follow;
  s_stack.push(this);
}

void cxx_compiler::declarations::type_specifier_seq::info_t::update()
{
  using namespace std;
  sort(m_tmp.begin(),m_tmp.end());
  specifier_seq::type::m_short = specifier_seq::type::m_signed = specifier_seq::type::m_unsigned = false;
  m_type = accumulate(m_tmp.begin(),m_tmp.end(),m_type,specifier_seq::updator);
  m_tmp.clear();
}

std::stack<cxx_compiler::declarations::type_specifier_seq::info_t*>
cxx_compiler::declarations::type_specifier_seq::info_t::s_stack;

namespace cxx_compiler { namespace declarations { namespace enumeration {
  usr* prev;
} } } // end of namespace enumeration, declaratoins and cxx_compiler

cxx_compiler::tag* cxx_compiler::declarations::enumeration::begin(var* v)
{
  using namespace std;
  assert(!v || v->usr_cast());
  usr* u = static_cast<usr*>(v);
  using namespace expressions::primary::literal;
  prev = integer::create(0);
  using namespace parse::templ;
  auto_ptr<usr> sweeper(save_t::nest.empty() ? u : 0);
  string name = u ? u->m_name : new_name(".tag");
  const file_t& file = u ? u->m_file : parse::position;
  map<string, tag*>& tags = scope::current->m_tags;
  map<string, tag*>::const_iterator p = tags.find(name);
  if ( p != tags.end() ){
    tag* prev = p->second;
    pair<const type*, const type*> types = prev->m_types;
    if ( types.second ){
      using namespace error::declarations::enumeration;
      redeclaration(parse::position,prev->m_file.back(),name);
    }
    else
      prev->m_file.push_back(file);
    class_or_namespace_name::before.push_back(prev);
    return prev;
  }
  tag::kind_t kind = tag::ENUM;
  tag* T = new tag(kind,name,file,0);
  T->m_types.first = incomplete_tagged_type::create(T);
  T->m_parent = scope::current;
  return tags[name] = T;
}

void cxx_compiler::declarations::
enumeration::definition(var* v, expressions::base* expr)
{
  using namespace std;
  using namespace error::declarations::enumeration;
  assert(v->usr_cast());
  usr* u = static_cast<usr*>(v);
  auto_ptr<usr> sweeper1(parse::templ::save_t::nest.empty() ? u : 0);
  auto_ptr<expressions::base> sweeper2(expr);
  v = 0;
  if (expr) {
    v = expr->gen();
    v = v->rvalue();
  }
  if (v && !v->isconstant()) {
    if (parse::templ::save_t::nest.empty())
      not_constant(u);
    v = 0;
  }
  if (v && !v->m_type->integer()) {
    not_integer(u);
    v = 0;
  }
  if (!v)
    v = prev;
  u->m_type = const_type::create(int_type::create());
  u->m_flag = usr::ENUM_MEMBER;
  assert(v->usr_cast());
  enum_member* member = new enum_member(*u, static_cast<usr*>(v));
  declarations::action2(member);
  using namespace expressions::primary::literal;
  var* one = integer::create(1);
  conversion::arithmetic::gen(&v, &one);
  v = v->add(one);
  assert(v->usr_cast());
  prev = static_cast<usr*>(v);
}

const cxx_compiler::type*
cxx_compiler::declarations::enumeration::end(tag* ptr)
{
  assert(!class_or_namespace_name::before.empty());
  assert(scope::current == ptr->m_parent);
  assert(class_or_namespace_name::before.back() == ptr);
  class_or_namespace_name::before.pop_back();
  return ptr->m_types.second = enum_type::create(ptr, prev->m_type);
}

bool cxx_compiler::declarations::duration::_static(const usr* u)
{
  return u && (u->m_scope == &scope::root || u->m_flag & usr::STATIC);
}

cxx_compiler::tac::tac(id_t id, var* xx, var* yy, var* zz) : m_id(id), x(xx), y(yy), z(zz), m_file(parse::position)
{
  if ( fundef::current ){
    usr::flag_t flag = fundef::current->m_usr->m_flag;
    if ( (flag & usr::INLINE) && !(flag & usr::STATIC) ){
      using namespace declarations::specifier_seq::func_spec;
      if ( x ) check(x);
      if ( y ) check(y);
      if ( z ) check(z);
    }
  }
}

bool cxx_compiler::declarations::internal_linkage(usr* u)
{
  if (u->m_scope != &scope::root)
    return false;
  usr::flag_t flag = u->m_flag;
  if (flag & usr::FUNCTION)
    return false;
  if (!(flag & usr::STATIC))
    return false;
  string name = u->m_name;
  if (name[name.length() - 1] == '"')
    return false;
  return true;
}

void cxx_compiler::declarations::specifier_seq::func_spec::check(var* v)
{
  if ( v ){
    if ( usr* u = v->usr_cast() ){
      if ( internal_linkage(u) ){
        using namespace error::declarations::specifier_seq::function::func_spec;
        invalid_internal_linkage(parse::position,u);
        usr::flag_t& flag = fundef::current->m_usr->m_flag;
        flag = usr::flag_t(flag & ~usr::INLINE);
      }
    }
  }
}

cxx_compiler::declarations::asm_definition::info_t::info_t(var* v)
 : usr("",0,usr::NONE,parse::position,usr::NONE2)
{
  genaddr* p = v->genaddr_cast();
  usr* u = p->m_ref->usr_cast();
  m_inst = u->m_name;
}

void cxx_compiler::declarations::asm_definition::info_t::initialize()
{
  using namespace std;
  string inst = m_inst.substr(1,m_inst.length()-2);
  code.push_back(new asm3ac(inst));
  delete this;
}

namespace cxx_compiler {
  namespace declarations {
    namespace new_type_id {
      struct sweeper {
        LIST* m_list;
        sweeper(LIST* p) : m_list(p) {}
        ~sweeper()
        {
          if (!m_list)
            return;
          for (auto p : *m_list) {
            vector<expressions::base*>* q = p.second;
            if (q) {
              for (auto x : *q)
                delete x;
              delete q;
            }
          }
          delete m_list;
        }
      };
      const type* calc2(const type* T, expressions::base* expr)
      {
        var* v = expr->gen();
        v = v->rvalue();
        if (!v->isconstant())
          return varray_type::create(T, v);
        __int64 dim = v->value();
        return array_type::create(T, dim);
      }
      const type* calc(const type* T, const LIST_ELEMENT& elem)
      {
        const type* Tx = elem.first;
        if (Tx) {
          assert(Tx->backpatch());
          return Tx->patch(T, 0);
        }
        vector<expressions::base*>* exprs = elem.second;
        return accumulate(begin(*exprs), end(*exprs), T, calc2);
      }
      map<const type*, vector<tac*> > table;
    } // end of namespace new_type_id
  } // end of namespace declarations
} // end of namespace cxx_compiler

const cxx_compiler::type* cxx_compiler::declarations::new_type_id::
action(type_specifier_seq::info_t* p, LIST* q)
{
  using namespace std;
  sweeper sweeper(q);
  p->update();
  const type* T = p->m_type;
  if (!q)
    return T;
  int n = code.size();
  T = accumulate(begin(*q), end(*q), T, calc);
  int m = code.size();
  if (n != m)
    copy(begin(code)+n, end(code), back_inserter(table[T]));
  code.resize(n);
  return T;
}

namespace cxx_compiler {
  namespace declarations {
    type_specifier* decl_type(expressions::base* expr)
    {
      int n = code.size();
      var* v = expr->gen();
      for_each(begin(code) + n, end(code), [](tac* ptr){ delete ptr; });
      code.resize(n);
      const type* T = v->result_type();
      return new type_specifier(T);
    }
  } // end of namespace declarations
} // end of namespace cxx_compiler

namespace cxx_compiler {
  namespace declarations {
    namespace use {
      void action(var* v)
      {
        if (genaddr* ga = v->genaddr_cast()) {
          v = ga->m_ref;
        }
        assert(v->usr_cast());
        usr* u = static_cast<usr*>(v);
        string name = u->m_name;
        map<string, vector<usr*> >& usrs = scope::current->m_usrs;
        typedef map<string, vector<usr*> >::const_iterator IT;
        IT p = usrs.find(name);
        if (p != usrs.end()) {
	  const vector<usr*>& v = p->second;
	  usr* prev = v.back();
	  usr::flag2_t flag2 = prev->m_flag2;
	  if (flag2 & usr::ALIAS)
	    return;
	}
        alias_usr* al = new alias_usr(u);
        usrs[name].push_back(al);
      }
      void action(tag* ptr)
      {
	string name = ptr->m_name;
	map<string, tag*>& tags = scope::current->m_tags;
        typedef map<string, tag*>::const_iterator IT;
	IT p = tags.find(name);
	if (p != tags.end())
          error::not_implemented();
        alias_tag* al = new alias_tag(ptr);
        tags[name] = al;
      }
      void action(var* v, type_specifier* ts)
      {
	auto_ptr<type_specifier> sweeper(ts);
	assert(v->usr_cast());
	usr* ident = static_cast<usr*>(v);
	assert(ident->m_type->m_id == type::BACKPATCH);
	string name = ident->m_name;
	if (const type* T = ts->m_type) {
	  map<string, tag*>& tags = scope::current->m_tags;
	  typedef map<string, tag*>::const_iterator IT;
	  IT p = tags.find(name);
	  if (p != tags.end()) {
	    tag* ptr = p->second;
	    tag::flag_t flag = ptr->m_flag;
	    if (!(flag & tag::TEMPLATE))
	      error::not_implemented();
	    template_tag* tt = static_cast<template_tag*>(ptr);
	    assert(!template_tag::nest.empty());
	    template_tag::info_t& info = template_tag::nest.back();
	    assert(info.m_tt == tt);
	    assert(!info.m_it);
	    tag::kind_t kind = tt->m_kind;
	    info.m_it = new instantiated_tag(kind, name, parse::position,
					     0, tt, info.m_key);
	    tag* tmp = info.m_it;
	    tmp->m_types = make_pair(incomplete_tagged_type::create(tmp), T);
	    return;
	  }
	  tag* ptr = T->get_tag();
	  assert(ptr);
	  const vector<scope::tps_t>& tps = scope::current->m_tps;
	  if (!tps.empty()) {
	    const scope::tps_t& b = tps.back();
	    if (!b.m_table.empty()) {
	      using namespace parse::templ;
	      assert(!save_t::nest.empty());
	      save_t* p = save_t::nest.back();
	      assert(!p->m_tag);
	      template_tag* tt = new template_tag(*ptr, b);
	      p->m_tag = tt;
	      tags[name] = tt;
	      return;
	    }
	  }
	  alias_tag* al = new alias_tag(ptr);
	  tags[name] = al;
	  return;
	}
	usr* tdef = ts->m_usr;
	assert(tdef->m_flag & usr::TYPEDEF);
        map<string, vector<usr*> >& usrs = scope::current->m_usrs;
        typedef map<string, vector<usr*> >::const_iterator IT;
        IT p = usrs.find(name);
	if (p != usrs.end()) {
	  const vector<usr*>& v = p->second;
	  usr* uu = v.back();
	  usr::flag2_t flag2 = uu->m_flag2;
	  if (!(flag2 & usr::TEMPLATE))
	    error::not_implemented();
	  template_usr* tu = static_cast<template_usr*>(uu);
	  assert(!template_usr::nest.empty());
	  template_usr::info_t& info = template_usr::nest.back();
	  assert(info.m_tu == tu);
	  assert(!info.m_iu);
	  info.m_iu = new instantiated_usr(*tdef, tu, info.m_key);
	  return;
	}
	const vector<scope::tps_t>& tps = scope::current->m_tps;
	if (!tps.empty()) {
	  const scope::tps_t& b = tps.back();
	  if (!b.m_table.empty()) {
	    using namespace parse::templ;
	    assert(!save_t::nest.empty());
	    save_t* p = save_t::nest.back();
	    assert(!p->m_usr);
	    template_usr* tu = new template_usr(*tdef, b, false);
	    tu->m_flag = usr::flag_t(tu->m_flag & ~usr::TYPEDEF);
	    tu->m_scope = scope::current;
	    p->m_usr = tu;
	    usrs[name].push_back(tu);
	    return;
	  }
	}
        alias_usr* al = new alias_usr(tdef);
        usrs[name].push_back(al);
      }
    } // end of namespace declarations
  } // end of namespace declarations
} // end of namespace cxx_compiler
