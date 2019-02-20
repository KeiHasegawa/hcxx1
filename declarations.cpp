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
  for (auto p : children)
    delete p;
  children.clear();
  error::headered = false;
  if (!generator::last)
    type::destroy_tmp();
  constant<void*>::destroy_temporary();
  declarators::array::variable_length::destroy_tmp();
  if ( cmdline::simple_medium )
    dump::names::reset();
  class_or_namespace_name::after();
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
	if (parse::position.m_lineno == 1187) {
		int debug = 1;
	}
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

cxx_compiler::declarations::type_specifier::type_specifier(tag* Tag)
 : m_keyword(0), m_type(0), m_usr(0)
{
  m_type = Tag->m_types.second ? Tag->m_types.second : Tag->m_types.first;
  parse::identifier::mode = parse::identifier::new_obj;
}

namespace cxx_compiler { namespace declarations { namespace specifier_seq { namespace flag {
  struct table : std::map<int,usr::flag_t> {
    table();
  } m_table;
  extern usr::flag_t merge(usr::flag_t, usr::flag_t);
} } } } // end of namespace flag, specifier_seq, declarations and cxx_compiler

cxx_compiler::declarations::specifier_seq::info_t::info_t(info_t* prev, specifier* spec)
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

void cxx_compiler::declarations::specifier_seq::info_t::clear()
{
  parse::identifier::mode = parse::identifier::look; s_stack.push(0);
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
  usr* m_usr;
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
    type::m_usr ? implicit_int(type::m_usr) : implicit_int(parse::position);
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
  extern usr* action2(usr*, bool);
} } // end of namespace declarations ans cxx_compiler

cxx_compiler::usr*
cxx_compiler::declarations::action1(var* v, bool ini, bool lookuped)
{
  using namespace std;
  using namespace error::declarations::specifier_seq::type;
  usr* u = v->usr_cast();
  if ( !u ){
    /*
     Definition like
     struct S { void f(); };
     void S::f(){ ... }
     where, `f' is lookuped
     */
    genaddr* ga = v->genaddr_cast();
    v = ga->m_ref;
    u = v->usr_cast();
  }
  if ( specifier_seq::info_t::s_stack.empty() ){
    if ( u->m_flag & usr::CTOR ){
      assert(u->m_type->m_id == type::FUNC);
      u->m_type = u->m_type->patch(0,u);
    }
    else {
      implicit_int(u);
      u->m_type = int_type::create();
    }
  }
  else if ( specifier_seq::info_t* p = specifier_seq::info_t::s_stack.top() ){
    if ( !(u->m_flag & usr::CTOR) ){
      if ( !p->m_type || !p->m_tmp.empty() ){
        declarations::specifier_seq::type::m_usr = u;
        p->update();
      }
      if ( !p->m_type ){
        implicit_int(u);
        p->m_type = int_type::create();
      }
      u->m_flag = p->m_flag;
      u->m_type = u->m_type->patch(p->m_type,u);
    }
  }
  else {
    usr::flag_t mask = usr::flag_t(usr::CTOR | usr::DTOR);
    assert(u->m_flag & mask);
    assert(u->m_type->m_id == type::FUNC);
    u->m_type = u->m_type->patch(0,u);
  }
  if (u->m_flag & usr::TYPEDEF) {
    type_def* tmp = new type_def(*u);
    delete u;
    u = tmp;
  }
  if ( ini ){
    parse::identifier::mode = parse::identifier::look;
    if ( duration::_static(u) ){
      with_initial* p = new with_initial(*u);
      delete u;
      u = p;
      if ( scope::current != &scope::root )
        expressions::constant_flag = true;
    }
  }
  const type* T = u->m_type;
  usr::flag_t& flag = u->m_flag;
  string name = u->m_name;
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  usr::flag_t mask = usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::FUNCTION | usr::VL);
  if ( !(flag & mask) ){
    if ( b || scope::current == &scope::root ){
      typedef const array_type ARRAY;
      ARRAY* array = T->m_id == type::ARRAY ? static_cast<ARRAY*>(T) : 0;
      if ( !array || array->dim() || !ini )
        check_object(u);
    }
  }
  else if ( flag & usr::EXTERN ){
    if ( ini ){
      if ( scope::current == &scope::root ){
        using namespace warning::declarations::initializers;
        with_extern(u);
      }
      else {
        using namespace error::declarations::initializers;
        with_extern(u);
      }
      flag = usr::flag_t(flag & ~usr::EXTERN);
    }
  }
  else if ( flag & usr::FUNCTION ){
    usr::flag_t mask = usr::flag_t(usr::STATIC | usr::AUTO | usr::REGISTER);
    if ( flag & mask ){
      if ( b ){
        using namespace error::declarations::declarators::function;
        invalid_storage(u);
        flag = usr::flag_t(flag & ~mask);
      }
    }
  }
  if ( (flag & usr::VL) && ini ){
    using namespace error::declarations::declarators::array;
    variable_length::initializer(u);
  }
  if ( b ){
    usr::flag_t mask = usr::flag_t(usr::STATIC | usr::EXTERN);
    if ( flag & mask ){
      if ( fundef::current->m_usr->m_flag & usr::INLINE ){
        using namespace error::declarations::specifier_seq::function;
        func_spec::static_storage(u);
        fundef::current->m_usr->m_flag = usr::flag_t(fundef::current->m_usr->m_flag & ~usr::INLINE);
      }
    }
  }
  if ( flag & usr::INLINE ){
    using namespace error::declarations::specifier_seq::function;
    if ( !(flag & usr::FUNCTION) )
      not_function(u);
    else {
      string name = u->m_name;
      if ( u->m_name == "main" )
        func_spec::main(u);
    }
  }
  if ( b ){
    scope* param = b->m_parent;
    if ( param->m_parent == &scope::root ){
      const map<string, vector<usr*> >& usrs = param->m_usrs;
      map<string, vector<usr*> >::const_iterator p =
        usrs.find(name);
      if ( p != usrs.end() ){
        using namespace error::declarations;
        usr* prev = p->second.back();
        redeclaration(prev,u,true);
      }
    }
  }
  else if ( scope::current == &scope::root ){
    usr::flag_t mask = usr::flag_t(usr::AUTO | usr::REGISTER);
    if ( flag & mask ){
      using namespace error::declarations::external;
      invalid_storage(parse::position);
      flag = usr::flag_t(flag & ~mask);
    }
    u->m_type = u->m_type->vla2a();
  }
  if ( b && u->m_type->m_id == type::RECORD ){
    typedef const record_type REC;
    REC* rec = static_cast<REC*>(u->m_type);
    tag* ptr = rec->get_tag();
    string name = ptr->m_name;
    const map<string, vector<usr*> >& usrs = ptr->m_usrs;
    map<string, vector<usr*> >::const_iterator p = usrs.find(name);
    if ( p != usrs.end() ){
      const vector<usr*>& v = p->second;
      usr* ctor = v.back();
      const func_type* ft = static_cast<const func_type*>(ctor->m_type);
      vector<var*> arg;
      call_impl::common(ft,ctor,&arg,false,u);
      usr::flag_t flag = ctor->m_flag;
      if (!error::counter) {
        if (flag & usr::INLINE) {
          using namespace declarations::declarators::function::definition::static_inline::skip;
          table_t::const_iterator p = table.find(ctor);
          if (p != table.end())
            substitute(code, code.size()-1, p->second);
        }
      }
    }
  }
  return action2(u,lookuped);
}

void cxx_compiler::declarations::check_object(usr* u)
{
  const type* T = u->m_type;
  int size = T->size();
  if ( !size ){
    using namespace error::declarations;
    not_object(u,T);
    size = int_type::create()->size();
    u->m_type = int_type::create();
  }
}

namespace cxx_compiler { namespace declarations {
  bool conflict(usr*, usr*);
  usr* combine(usr*, usr*);
} } // end of namespace declarations and cxx_compiler

cxx_compiler::usr* cxx_compiler::declarations::action2(usr* curr, bool lookuped)
{
  using namespace std;
  string name = curr->m_name;
  curr->m_scope = scope::current;
  if ( name == "__func__" ){
    using namespace error::expressions::primary::underscore_func;
    declared(parse::position);
  }
  else {
    usr::flag_t flag = curr->m_flag;
    if ( !(flag & usr::TYPEDEF ) ){
      if ( declarations::linkage::depth || name == "main" )
        curr->m_flag = usr::flag_t(curr->m_flag | usr::C_SYMBOL);
    }
  }
  map<string, vector<usr*> >& usrs = curr->m_scope->m_usrs;
  map<string, vector<usr*> >::const_iterator p = usrs.find(name);
  if ( p != usrs.end() ){
    usr* prev = p->second.back();
    if ( prev == curr )
      return curr;
    if ( conflict(prev,curr) ){
      using namespace error::declarations;
      redeclaration(prev,curr,false);
    }
    else {
      curr = combine(prev,curr);
      usr::flag_t flag = curr->m_flag;
      if ((flag & usr::FUNCTION) && (flag & usr::EXTERN)){
        using namespace declarators::function::definition;
        using namespace static_inline;
        skip::table_t::iterator r = skip::table.find(prev);
        if (r != skip::table.end()) {
          info_t* info = r->second;
          usr::flag_t& flag = info->m_fundef->m_usr->m_flag;
          flag = usr::flag_t(flag | usr::EXTERN);
          skip::table.erase(r);
          gencode(info);
        }
      }
    }
  }
  if ( lookuped )
    return curr;
  usrs[name].push_back(curr);
  if (curr->m_scope->m_id != scope::PARAM || !(curr->m_flag & usr::ENUM_MEMBER))
    curr->m_scope->m_order.push_back(curr);
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
    if ( x->m_type->m_id != type::FUNC )
      return true;
  }
  if ( scope::current == &scope::root ){
    if ( x->with_initial_cast() && y->with_initial_cast() )
      return true;
  }
  return conflict(x->m_type,y->m_type);
}

namespace cxx_compiler { namespace declarations {
  struct table {
    std::map<std::pair<usr::flag_t, usr::flag_t>,bool> m_root;
    std::map<std::pair<usr::flag_t, usr::flag_t>,bool> m_tag;
    std::map<std::pair<usr::flag_t, usr::flag_t>,bool> m_other;
    table();
  } m_table;
} } // end of namespace declarations and cxx_compiler

bool cxx_compiler::declarations::conflict(usr::flag_t x, usr::flag_t y)
{
  using namespace std;
  if ((x & usr::ENUM_MEMBER) || (y & usr::ENUM_MEMBER))
    return true;
  usr::flag_t mask = usr::flag_t(usr::TYPEDEF|usr::EXTERN|usr::STATIC|usr::AUTO|usr::REGISTER);
  x = usr::flag_t(x & mask);
  y = usr::flag_t(y & mask);
  pair<usr::flag_t, usr::flag_t> key(x,y);
  if ( scope::current == &scope::root )
    return m_table.m_root[key];
  if ( scope::current->m_id == scope::NAMESPACE )
    return m_table.m_root[key];
  if ( scope::current->m_id == scope::TAG )
    return m_table.m_tag[key];
  return m_table.m_other[key];
}

cxx_compiler::declarations::table::table()
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
  typedef const func_type FUNC;
  FUNC* fp = static_cast<FUNC*>(prev);
  FUNC* fc = static_cast<FUNC*>(curr);
  return !fp->overloadable(fc);
}

cxx_compiler::usr* cxx_compiler::declarations::combine(usr* prev, usr* curr)
{
  using namespace std;

  if ( scope::current == &scope::root || scope::current->m_id == scope::NAMESPACE ){
    usr::flag_t a = prev->m_flag;
    usr::flag_t& b = curr->m_flag;
    if ( a == usr::NONE && b == usr::NONE )
      b = usr::EXTERN;
    else if ( a & usr::STATIC )
      b = usr::flag_t(b | usr::STATIC);
    else if ( a & usr::INLINE )
      b = usr::flag_t(b | usr::INLINE);
  }
  else if ( scope::current->m_id == scope::TAG ){
    usr::flag_t a = prev->m_flag;
    usr::flag_t& b = curr->m_flag;
    if ( a & usr::STATIC )
      b = usr::flag_t(b | usr::STATIC_DEF);
  }
  const type* x = prev->m_type;
  const type* y = curr->m_type;
  const type* z = x->composite(y);
  if ( z ){
    curr->m_type = z;
    return curr;
  }
  else {
    string name = curr->m_name;
    scope::current->m_usrs[name].push_back(curr);
    return new overload(prev,curr);
  }
}

namespace cxx_compiler { namespace declarations { namespace elaborated {
  tag* lookup(std::string, scope*);
} } } // end of namespace elaborated, declarations and cxx_compiler

const cxx_compiler::type* cxx_compiler::declarations::elaborated::action(int keyword, var* v)
{
  using namespace std;
  usr* u = static_cast<usr*>(v);
  auto_ptr<usr> sweeper(u);
  string name = u->m_name;
  tag* T = lookup(name,scope::current);
  if ( T ){
    const pair<const type*, const type*>& p = T->m_types;
    return p.second ? p.second : p.first;
  }
  else {
    scope* ptr = linkage::depth ? &scope::root : scope::current;
    tag::kind_t kind = classes::specifier::get(keyword);
    const file_t& file = u->m_file;
    tag* T = new tag(kind,name,file,0);
    ptr->m_tags[name] = T;
    T->m_parent = ptr;
    return T->m_types.first = incomplete_tagged_type::create(T);
  }
}

cxx_compiler::tag* cxx_compiler::declarations::elaborated::lookup(std::string name, scope* ptr)
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

void cxx_compiler::declarations::linkage::action(var* v)
{
  using namespace std;
  // check if `v' is "C"
  genaddr* p = v->genaddr_cast();
  with_initial* q = static_cast<with_initial*>(p->m_ref);
  const map<int,var*>& value = q->m_value;
  assert(value.size() == 2);
  map<int,var*>::const_iterator a = value.find(0);
  assert(a != value.end());
  usr* b = static_cast<usr*>(a->second);
  constant<char>* c = static_cast<constant<char>*>(b);
  assert(c->m_value == 'C');
  map<int,var*>::const_iterator d = value.find(1);
  assert(d != value.end());
  usr* e = static_cast<usr*>(d->second);
  constant<char>* f = static_cast<constant<char>*>(e);
  assert(f->m_value == '\0');
  ++depth;
}

int cxx_compiler::declarations::linkage::depth;

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
  prev = expressions::primary::literal::integer::create(0);
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
  declarations::action2(member,false);
  v = v->add(expressions::primary::literal::integer::create(1));
  prev = static_cast<usr*>(v);
}

const cxx_compiler::type* cxx_compiler::declarations::enumeration::end(tag* Tag)
{
  return Tag->m_types.second = enum_type::create(Tag,prev->m_type);
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
      using namespace declarations::specifier_seq::function::Inline;
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

void cxx_compiler::declarations::specifier_seq::function::Inline::check(var* v)
{
  if ( v ){
    if ( usr* u = v->usr_cast() ){
      if ( internal_linkage(u) ){
        error::declarations::specifier_seq::function::func_spec::internal_linkage(parse::position,u);
        usr::flag_t& flag = fundef::current->m_usr->m_flag;
        flag = usr::flag_t(flag & ~usr::INLINE);
      }
    }
  }
}

cxx_compiler::declarations::asm_definition::info_t::info_t(var* v) : usr("",0,usr::NONE,parse::position)
{
  genaddr* p = v->genaddr_cast();
  usr* u = p->m_ref->usr_cast();
  m_inst = u->m_name;
}

int cxx_compiler::declarations::asm_definition::info_t::initialize()
{
  using namespace std;
  string inst = m_inst.substr(1,m_inst.length()-2);
  code.push_back(new asm3ac(inst));
  delete this;
  return 0;
}

const cxx_compiler::type* cxx_compiler::declarations::new_type_id::action(type_specifier_seq::info_t* p)
{
  using namespace std;
  p->update();
  const type* T = p->m_type;
  return T;
}
