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
  parse::identifier::mode = parse::identifier::new_obj;
}

cxx_compiler::declarations::type_specifier::type_specifier(usr* u)
 : m_keyword(0), m_type(0), m_usr(u)
{
  parse::identifier::mode = parse::identifier::new_obj;
}

cxx_compiler::declarations::type_specifier::type_specifier(tag* ptr)
 : m_keyword(0), m_type(0), m_usr(0)
{
  parse::identifier::base_lookup::route.clear();
  m_type = ptr->m_types.second ? ptr->m_types.second : ptr->m_types.first;
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

namespace cxx_compiler { namespace declarations {
  void check_installed(usr*, specifier_seq::info_t*);
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
} } // end of namespace declarations ans cxx_compiler

cxx_compiler::usr*
cxx_compiler::declarations::action1(var* v, bool ini)
{
  using namespace std;
  using namespace error::declarations::specifier_seq::type;
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
      check_installed(u, p);
    else {
      u->m_flag = flag = usr::flag_t(flag | p->m_flag);
      u->m_type = T = T->patch(p->m_type,u);
      flag = u->m_flag;
    }
  }
  else {
    if (flag & usr::DTOR) {
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
    else {
      // Rare case. Maybe already error happened.
      if (T->backpatch())
        u->m_type = T = T->patch(int_type::create(),u);
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
      if (!(flag & usr::EXTERN)) {
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

  if (installed && u->m_scope->m_id == scope::TAG && (flag & usr::STATIC)
      && T->size()) {
    usr* tmp = new usr(*u);
    tmp->m_flag = usr::flag_t(tmp->m_flag | usr::STATIC_DEF);
    tmp->m_file = parse::position;
    map<string, vector<usr*> >& usrs = tmp->m_scope->m_usrs;
    usrs[name].push_back(tmp);
    u = tmp;
  }

  const pair<map<string, tag*>, vector<string> >& tps = scope::current->m_tps;
  if (!tps.first.empty()) {
    using namespace parse::templ;
    assert(!save_t::s_stack.empty());
    save_t* p = save_t::s_stack.top();
    assert(!p->m_usr);
    p->m_usr = u = new template_usr(*u, tps);
  }

  if (!installed)
    u = action2(u);

  if (!ini) {
    usr::flag_t mask = 
      usr::flag_t(usr::TYPEDEF |usr::FUNCTION | usr::OVERLOAD);
    if (!(flag & mask) && is_external_declaration(u) &&
	!just_static_member_decl(u)) {
      if (must_call_default_ctor(u))
	initialize_ctor_code(u);
      if (must_call_dtor(u))
	terminate_dtor_code(u);
    }
  }

  return u;
}

void cxx_compiler::declarations::check_object(usr* u)
{
  const type* T = u->m_type;
  u->m_type = T = T->complete_type();
  int size = T->size();
  if (!size) {
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
  struct friend_func : usr {
    tag* m_tag;
    friend_func(const usr& u, tag* ptr) : usr(u), m_tag(ptr) {}
  };
} } // end of namespace declarations and cxx_compiler

cxx_compiler::usr* cxx_compiler::declarations::action2(usr* curr)
{
  using namespace std;
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
      else if (!declarations::linkage::braces.empty()) {
        switch (scope::current->m_id) {
        case scope::NONE: case scope::NAMESPACE:
          {
            curr->m_flag = usr::flag_t(flag | usr::C_SYMBOL);
            bool brace = declarations::linkage::braces.back();
            if (!brace)
              curr->m_flag = usr::flag_t(curr->m_flag | usr::EXTERN);
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
    if (scope::current->m_id != scope::TAG)
      error::not_implemented();
    tag* ptr = static_cast<tag*>(scope::current);
    if (!(flag & usr::FUNCTION))
      error::not_implemented();
    curr->m_scope = &scope::root;
    curr = new friend_func(*curr, ptr);
  }

  map<string, vector<usr*> >& usrs = curr->m_scope->m_usrs;
  map<string, vector<usr*> >::const_iterator p = usrs.find(name);
  if (p != usrs.end()) {
    const vector<usr*>& v = p->second;
    usr* prev = v.back();
    assert(prev != curr);
    if (conflict(prev,curr)) {
      using namespace error::declarations;
      redeclaration(prev,curr,false);
    }
    else {
      curr = combine(prev,curr);
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
          usr::flag_t& flag = info->m_fundef->m_usr->m_flag;
          flag = usr::flag_t(flag | usr::EXTERN);
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
      if (!(flag & mask))
	block_impl::dtor_tbl[b].push_back(curr);
    }
  }
  class_or_namespace_name::last = 0;
  return curr;
}

namespace cxx_compiler { namespace declarations {
  bool conflict(usr::flag_t, usr::flag_t);
  bool conflict(const type*, const type*);
} } // end of namespace declarations and cxx_compiler

bool cxx_compiler::declarations::conflict(usr* x, usr* y)
{
  if ( conflict(x->m_flag,y->m_flag) ){
    if (!x->m_type) {
      assert(x->m_flag & usr::OVERLOAD);
      return false;
    }
    if ( x->m_type->m_id != type::FUNC )
      return true;
  }
  if ( scope::current == &scope::root ){
    if ((x->m_flag & usr::WITH_INI) && (y->m_flag & usr::WITH_INI))
      return true;
  }
  if (x->m_flag & usr::OVERLOAD)
    return false;
  return conflict(x->m_type,y->m_type);
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
  return !fp->overloadable(fc);
}

cxx_compiler::usr* cxx_compiler::declarations::combine(usr* prev, usr* curr)
{
  using namespace std;
  scope::id_t id = curr->m_scope->m_id;
  switch (id) {
  case scope::NONE: case scope::NAMESPACE:
    {
      usr::flag_t a = prev->m_flag;
      usr::flag_t b = curr->m_flag;
      if (a == usr::NONE && b == usr::NONE)
        curr->m_flag = usr::EXTERN;
      else if (a & usr::STATIC)
        curr->m_flag = usr::flag_t(b | usr::STATIC);
      else if (a & usr::INLINE)
        curr->m_flag = usr::flag_t(b | usr::INLINE);
      if (a & usr::C_SYMBOL)
        curr->m_flag = usr::flag_t(b | usr::C_SYMBOL);
    }
    break;
  case scope::TAG:
    {
      usr::flag_t a = prev->m_flag;
      usr::flag_t b = curr->m_flag;
      if (a & usr::STATIC)
	curr->m_flag = usr::flag_t(b | usr::STATIC_DEF);
    }
    break;
  }

  const type* x = prev->m_type;
  if (x) {
    const type* y = curr->m_type;
    if (const type* z = composite(x, y)) {
      curr->m_type = z;
      return curr;
    }
  }

  usr::flag2_t flag2 = prev->m_flag2;
  if (flag2 & usr::TEMPLATE) {
    curr->m_flag2 = usr::flag2_t(curr->m_flag2 | usr::INSTANTIATE);
    return curr;
  }

  string name = curr->m_name;
  scope::current->m_usrs[name].push_back(curr);
  return new overload(prev, curr);
}

namespace cxx_compiler {
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
    IT p = find_if(begin(m_candidacy), end(m_candidacy), [curr](usr* u)
		   { return compatible(u->m_type, curr->m_type); });
    if (p != end(m_candidacy))
      *p = curr;
    else
      m_candidacy.push_back(curr);
  }
} // end of namespace cxx_compiler

void
cxx_compiler::declarations::check_installed(usr* u, specifier_seq::info_t* p)
{
  usr::flag_t flag = u->m_flag;
  u->m_flag = flag = usr::flag_t(flag | p->m_flag);
  const type* Tu = u->m_type;
  if (flag & usr::OVERLOAD)
    return;
  assert(!Tu->backpatch());
  if (Tu->m_id == type::FUNC) {
    typedef const func_type FT;
    FT* ft = static_cast<FT*>(Tu);
    const type* T = ft->return_type();
    const type* Tp = p->m_type;
    if (!compatible(T, Tp))
      error::not_implemented();
  }
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
  delete org;
  return new_one;
}

namespace cxx_compiler { namespace declarations { namespace elaborated {
  tag* lookup(std::string, scope*);
} } } // end of namespace elaborated, declarations and cxx_compiler

const cxx_compiler::type*
cxx_compiler::declarations::elaborated::action(int keyword, var* v)
{
  using namespace std;
  assert(v->usr_cast());
  usr* u = static_cast<usr*>(v);
  auto_ptr<usr> sweeper(u);
  string name = u->m_name;
  tag* T = lookup(name,scope::current);
  if (T) {
    const pair<const type*, const type*>& p = T->m_types;
    return p.second ? p.second : p.first;
  }
  else {
    scope* parent = linkage::braces.empty() ? scope::current : &scope::root;
    tag::kind_t kind = classes::specifier::get(keyword);
    const file_t& file = u->m_file;
    tag* ptr = new tag(kind,name,file,0);
    parent->m_tags[name] = ptr;
    ptr->m_parent = parent;
    parent->m_children.push_back(ptr);
    using namespace class_or_namespace_name;
    assert(!before.empty());
    before.pop_back();
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
    usr* b = static_cast<usr*>(a->second);
    constant<char>* c = static_cast<constant<char>*>(b);
    if (c->m_value != 'C')
      error::not_implemented();
    map<int,var*>::const_iterator d = value.find(1);
    assert(d != value.end());
    usr* e = static_cast<usr*>(d->second);
    constant<char>* f = static_cast<constant<char>*>(e);
    assert(f->m_value == '\0');
    braces.push_back(brace);
  }
  else if (value.size() == 4) {
    // check `v' is "C++"
    map<int,var*>::const_iterator a = value.find(0);
    assert(a != value.end());
    usr* b = static_cast<usr*>(a->second);
    constant<char>* c = static_cast<constant<char>*>(b);
    if (c->m_value != 'C')
      error::not_implemented();
    map<int,var*>::const_iterator d = value.find(1);
    assert(d != value.end());
    usr* e = static_cast<usr*>(d->second);
    constant<char>* f = static_cast<constant<char>*>(e);
    if (f->m_value != '+')
      error::not_implemented();
    map<int,var*>::const_iterator g = value.find(2);
    assert(g != value.end());
    usr* h = static_cast<usr*>(g->second);
    constant<char>* i = static_cast<constant<char>*>(h);
    if (i->m_value != '+')
      error::not_implemented();
    map<int,var*>::const_iterator j = value.find(3);
    assert(j != value.end());
    usr* k = static_cast<usr*>(j->second);
    constant<char>* l = static_cast<constant<char>*>(k);
    assert(l->m_value == '\0');
  }
  else
    error::not_implemented();
}

std::vector<bool> cxx_compiler::declarations::linkage::braces;

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
  usr* u = static_cast<usr*>(v);
  using namespace expressions::primary::literal;
  prev = integer::create(0);
  auto_ptr<usr> sweeper(u);
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
    return prev;
  }
  tag::kind_t kind = tag::ENUM;
  tag* T = new tag(kind,name,file,0);
  T->m_types.first = incomplete_tagged_type::create(T);
  T->m_parent = scope::current;
  return tags[name] = T;
}

void cxx_compiler::declarations::enumeration::definition(var* v, expressions::base* expr)
{
  using namespace std;
  using namespace error::declarations::enumeration;
  usr* u = static_cast<usr*>(v);
  auto_ptr<usr> sweeper1(u);
  auto_ptr<expressions::base> sweeper2(expr);
  v = 0;
  if ( expr ){
    v = expr->gen();
    v = v->rvalue();
  }
  if ( v && !v->isconstant() ){
    not_constant(u);
    v = 0;
  }
  if ( v && !v->m_type->integer() ){
    not_integer(u);
    v = 0;
  }
  if ( !v )
    v = prev;
  u->m_type = const_type::create(int_type::create());
  u->m_flag = usr::ENUM_MEMBER;
  enum_member* member = new enum_member(*u,static_cast<usr*>(v));
  declarations::action2(member);
  using namespace expressions::primary::literal;
  var* one = integer::create(1);
  conversion::arithmetic::gen(&v, &one);
  v = v->add(one);
  prev = static_cast<usr*>(v);
}

const cxx_compiler::type*
cxx_compiler::declarations::enumeration::end(tag* ptr)
{
  using namespace class_or_namespace_name;
  assert(!before.empty());
  if (before.back() == ptr)
    before.pop_back();
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
	  error::not_implemented();
	  return T;
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
