#pragma warning ( disable : 4503 )
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

namespace cxx_compiler { namespace type_impl {
    struct sizeof_table : std::map<const type*, int> {
      sizeof_table();
    } sizeof_table;
    int pointer_sizeof = sizeof(void*);
} } // end of namespace type_impl and cxx_compiler

cxx_compiler::type_impl::sizeof_table::sizeof_table()
{
  (*this)[short_type::create()] = (*this)[ushort_type::create()] = sizeof(short);
  (*this)[int_type::create()] = (*this)[uint_type::create()] = sizeof(int);
  (*this)[long_type::create()] = (*this)[ulong_type::create()] = sizeof(long);
  (*this)[long_long_type::create()] = (*this)[ulong_long_type::create()] =
#ifdef WIN32
    sizeof(__int64);
#else  // WIN32
    sizeof(long long);
#endif  // WIN32
  (*this)[float_type::create()] = sizeof(float);
  (*this)[double_type::create()] = sizeof(double);
  (*this)[long_double_type::create()] = sizeof(long double);
}

void cxx_compiler::type_impl::update(int (*size)(int id))
{
  if (!size)
    return;
  sizeof_table[short_type::create()] =
  sizeof_table[ushort_type::create()] = size((int)type::SHORT);
  sizeof_table[int_type::create()] =
  sizeof_table[uint_type::create()] = size((int)type::INT);
  sizeof_table[long_type::create()] =
  sizeof_table[ulong_type::create()] = size((int)type::LONG);
  sizeof_table[long_long_type::create()] =
  sizeof_table[ulong_long_type::create()] = size((int)type::LONGLONG);
  sizeof_table[float_type::create()] = size((int)type::FLOAT);
  sizeof_table[double_type::create()] = size((int)type::DOUBLE);
  sizeof_table[long_double_type::create()] = size((int)type::LONG_DOUBLE);
  pointer_sizeof = size((int)type::POINTER);
}

bool cxx_compiler::type::compatible(const type* that) const
{
  return this == that;
}

const cxx_compiler::type* cxx_compiler::type::composite(const type* that) const
{
  return this == that ? this : 0;
}

bool cxx_compiler::type::include_cvr(const type* that) const
{
  int cvr = 0;
  that->unqualified(&cvr);
  return !cvr;
}

std::pair<int, const cxx_compiler::type*> cxx_compiler::type::current(int nth) const
{
  using namespace std;
  return nth ? make_pair(-1,static_cast<const type*>(0)) : make_pair(0,this);
}

const cxx_compiler::type* cxx_compiler::type::qualified(int cvr) const
{
  const type* T = this;
  if ( cvr & 1 )
    T = const_type::create(T);
  if ( cvr & 2 )
    T = volatile_type::create(T);
  if ( cvr & 4 ){
    if ( T->m_id == type::POINTER )
      T = restrict_type::create(T);
    else {
      using namespace error::declarations::declarators::qualifier;
      invalid(parse::position,T);
    }
  }
  return T;
}

namespace cxx_compiler { namespace type_impl {
  std::vector<const type*> temp1;
#ifdef _DEBUG
  misc::pvector<const type> temp2;
#endif // _DEBUG
} } // end of namespace type_impl and cxx_compiler

void cxx_compiler::type::destroy_temporary()
{
  using namespace std;
  vector<const type*>& v = type_impl::temp1;
  for_each(v.begin(),v.end(),misc::deleter<const type>());
  v.clear();
}

cxx_compiler::void_type cxx_compiler::void_type::obj;

void cxx_compiler::void_type::decl(std::ostream& os, std::string name) const
{
  os << "void";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::void_type::encode(std::ostream& os) const { os << 'v'; }

cxx_compiler::char_type cxx_compiler::char_type::obj;

void cxx_compiler::char_type::decl(std::ostream& os, std::string name) const
{
  os << "char";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::char_type::encode(std::ostream& os) const { os << 'c'; }

const cxx_compiler::type* cxx_compiler::char_type::promotion() const
{
  return int_type::create();
}

const cxx_compiler::type* cxx_compiler::char_type::_unsigned() const { return uchar_type::create(); }

cxx_compiler::schar_type cxx_compiler::schar_type::obj;

void cxx_compiler::schar_type::decl(std::ostream& os, std::string name) const
{
  os << "signed char";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::schar_type::encode(std::ostream& os) const { os << 'a'; }

const cxx_compiler::type* cxx_compiler::schar_type::promotion() const
{
  return int_type::create();
}

const cxx_compiler::type* cxx_compiler::schar_type::_unsigned() const { return uchar_type::create(); }

cxx_compiler::uchar_type cxx_compiler::uchar_type::obj;

void cxx_compiler::uchar_type::decl(std::ostream& os, std::string name) const
{
  os << "unsigned char";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::uchar_type::encode(std::ostream& os) const { os << 'h'; }

const cxx_compiler::type* cxx_compiler::uchar_type::promotion() const
{
  return int_type::create();
}

const cxx_compiler::type* cxx_compiler::uchar_type::_signed() const { return char_type::create(); }

cxx_compiler::wchar_type cxx_compiler::wchar_type::obj;

void cxx_compiler::wchar_type::decl(std::ostream& os, std::string name) const
{
  os << "wchar_t";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::wchar_type::encode(std::ostream& os) const { os << 'w'; }

const cxx_compiler::type* cxx_compiler::wchar_type::promotion() const
{
  return int_type::create();
}

const cxx_compiler::type* cxx_compiler::wchar_type::_signed() const { return short_type::create(); }

cxx_compiler::bool_type cxx_compiler::bool_type::obj;

void cxx_compiler::bool_type::decl(std::ostream& os, std::string name) const
{
  os << "bool";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::bool_type::encode(std::ostream& os) const { os << 'b'; }

const cxx_compiler::type* cxx_compiler::bool_type::promotion() const
{
  return int_type::create();
}

cxx_compiler::short_type cxx_compiler::short_type::obj;

int cxx_compiler::short_type::size() const { return type_impl::sizeof_table[&obj]; }

void cxx_compiler::short_type::decl(std::ostream& os, std::string name) const
{
  os << "short int";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::short_type::encode(std::ostream& os) const { os << 's'; }

const cxx_compiler::type* cxx_compiler::short_type::promotion() const
{
  return int_type::create();
}

const cxx_compiler::type* cxx_compiler::short_type::_unsigned() const { return ushort_type::create(); }

cxx_compiler::ushort_type cxx_compiler::ushort_type::obj;

int cxx_compiler::ushort_type::size() const { return type_impl::sizeof_table[&obj]; }

void cxx_compiler::ushort_type::decl(std::ostream& os, std::string name) const
{
  os << "unsigned short int";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::ushort_type::encode(std::ostream& os) const { os << 't'; }

const cxx_compiler::type* cxx_compiler::ushort_type::promotion() const
{
  return int_type::create();
}

const cxx_compiler::type* cxx_compiler::ushort_type::_signed() const { return short_type::create(); }

cxx_compiler::int_type cxx_compiler::int_type::obj;

int cxx_compiler::int_type::size() const { return type_impl::sizeof_table[&obj]; }

void cxx_compiler::int_type::decl(std::ostream& os, std::string name) const
{
  os << "int";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::int_type::encode(std::ostream& os) const { os << 'i'; }

const cxx_compiler::type* cxx_compiler::int_type::_unsigned() const { return uint_type::create(); }

cxx_compiler::uint_type cxx_compiler::uint_type::obj;

int cxx_compiler::uint_type::size() const { return type_impl::sizeof_table[&obj]; }

void cxx_compiler::uint_type::decl(std::ostream& os, std::string name) const
{
  os << "unsigned int";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::uint_type::encode(std::ostream& os) const { os << 'j'; }

const cxx_compiler::type* cxx_compiler::uint_type::_signed() const { return int_type::create(); }

cxx_compiler::long_type cxx_compiler::long_type::obj;

int cxx_compiler::long_type::size() const { return type_impl::sizeof_table[&obj]; }

void cxx_compiler::long_type::decl(std::ostream& os, std::string name) const
{
  os << "long int";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::long_type::encode(std::ostream& os) const { os << 'l'; }

const cxx_compiler::type* cxx_compiler::long_type::_unsigned() const { return ulong_type::create(); }

cxx_compiler::ulong_type cxx_compiler::ulong_type::obj;

int cxx_compiler::ulong_type::size() const { return type_impl::sizeof_table[&obj]; }

void cxx_compiler::ulong_type::decl(std::ostream& os, std::string name) const
{
  os << "unsigned long int";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::ulong_type::encode(std::ostream& os) const { os << 'm'; }

const cxx_compiler::type* cxx_compiler::ulong_type::_signed() const { return long_type::create(); }

cxx_compiler::long_long_type cxx_compiler::long_long_type::obj;

int cxx_compiler::long_long_type::size() const { return type_impl::sizeof_table[&obj]; }

void cxx_compiler::long_long_type::decl(std::ostream& os, std::string name) const
{
  os << "long long int";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::long_long_type::encode(std::ostream& os) const { os << 'x'; }

const cxx_compiler::type* cxx_compiler::long_long_type::_unsigned() const { return ulong_long_type::create(); }

cxx_compiler::ulong_long_type cxx_compiler::ulong_long_type::obj;

int cxx_compiler::ulong_long_type::size() const { return type_impl::sizeof_table[&obj]; }

void cxx_compiler::ulong_long_type::decl(std::ostream& os, std::string name) const
{
  os << "unsigned long long int";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::ulong_long_type::encode(std::ostream& os) const { os << 'y'; }

const cxx_compiler::type* cxx_compiler::ulong_long_type::_signed() const { return long_long_type::create(); }

cxx_compiler::float_type cxx_compiler::float_type::obj;

int cxx_compiler::float_type::size() const { return type_impl::sizeof_table[&obj]; }

void cxx_compiler::float_type::decl(std::ostream& os, std::string name) const
{
  os << "float";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::float_type::encode(std::ostream& os) const { os << 'f'; }

const cxx_compiler::type* cxx_compiler::float_type::varg() const { return double_type::create(); }

cxx_compiler::double_type cxx_compiler::double_type::obj;

int cxx_compiler::double_type::size() const { return type_impl::sizeof_table[&obj]; }

void cxx_compiler::double_type::decl(std::ostream& os, std::string name) const
{
  os << "double";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::double_type::encode(std::ostream& os) const { os << 'd'; }

cxx_compiler::long_double_type cxx_compiler::long_double_type::obj;

int cxx_compiler::long_double_type::size() const { return type_impl::sizeof_table[&obj]; }

void cxx_compiler::long_double_type::decl(std::ostream& os, std::string name) const
{
  os << "long double";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::long_double_type::encode(std::ostream& os) const { os << 'e'; }

cxx_compiler::backpatch_type cxx_compiler::backpatch_type::obj;

struct cxx_compiler::const_type::table_t : cxx_compiler::misc::pmap<const type*, const const_type> {};

void cxx_compiler::const_type::decl(std::ostream& os, std::string name) const
{
  typedef const pointer_type PT;
  if ( m_T->m_id == type::POINTER ){
    name = "const " + name;
    m_T->decl(os,name);
  }
  else {
    os << "const ";
    m_T->decl(os,name);
  }
}

void cxx_compiler::const_type::encode(std::ostream& os) const
{
  os << 'K';
  m_T->encode(os);
}

bool cxx_compiler::const_type::include_cvr(const type* that) const
{
  int x = 0;
  const type* a = this->unqualified(&x);
  int y = 0;
  const type* b = that->unqualified(&y);
  if ( ~x & y )
    return false;
  return a->include_cvr(b);
}

const cxx_compiler::type* cxx_compiler::const_type::patch(const type* T, usr* u) const
{
  return create(m_T->patch(T,u));
}

const cxx_compiler::type* cxx_compiler::const_type::qualified(int cvr) const
{
  cvr |= 1;
  return m_T->qualified(cvr);
}

const cxx_compiler::const_type* cxx_compiler::const_type::create(const type* T)
{
  if ( T->temporary(false) ){
    const const_type* ret = new const_type(T);
    if ( scope::current->m_id == scope::BLOCK )
      type_impl::temp1.push_back(ret);
#ifdef _DEBUG
    else
      type_impl::temp2.push_back(ret);
#endif // _DEBUG
    return ret;
  }
  table_t::const_iterator p = table.find(T);
  if ( p != table.end() )
    return p->second;
  else
    return table[T] = new const_type(T);
}

struct cxx_compiler::volatile_type::table_t : cxx_compiler::misc::pmap<const type*, const volatile_type> {};

cxx_compiler::volatile_type::table_t cxx_compiler::volatile_type::table;

void cxx_compiler::volatile_type::decl(std::ostream& os, std::string name) const
{
  typedef const pointer_type PT;
  if ( m_T->m_id == type::POINTER ){
    name = "volatile " + name;
    m_T->decl(os,name);
  }
  else {
    os << "volatile ";
    m_T->decl(os,name);
  }
}

void cxx_compiler::volatile_type::encode(std::ostream& os) const
{
  os << 'V';
  m_T->encode(os);
}

bool cxx_compiler::volatile_type::include_cvr(const type* that) const
{
  int x = 0;
  const type* a = this->unqualified(&x);
  int y = 0;
  const type* b = that->unqualified(&y);
  if ( ~x & y )
    return false;
  return a->include_cvr(b);
}

const cxx_compiler::type* cxx_compiler::volatile_type::patch(const type* T, usr* u) const
{
  return create(m_T->patch(T,u));
}

const cxx_compiler::type* cxx_compiler::volatile_type::qualified(int cvr) const
{
  cvr |= 2;
  return m_T->qualified(cvr);
}

const cxx_compiler::volatile_type* cxx_compiler::volatile_type::create(const type* T)
{
  if ( T->temporary(false) ){
    const volatile_type* ret = new volatile_type(T);
    if ( scope::current->m_id == scope::BLOCK )
      type_impl::temp1.push_back(ret);
#ifdef _DEBUG
    else
      type_impl::temp2.push_back(ret);
#endif // _DEBUG
    return ret;
  }
  table_t::const_iterator p = table.find(T);
  if ( p != table.end() )
    return p->second;
  else
    return table[T] = new volatile_type(T);
}

struct cxx_compiler::restrict_type::table_t : cxx_compiler::misc::pmap<const type*, const restrict_type> {};

cxx_compiler::restrict_type::table_t cxx_compiler::restrict_type::table;

void cxx_compiler::restrict_type::decl(std::ostream& os, std::string name) const
{
  typedef const pointer_type PT;
  if ( m_T->m_id == type::POINTER ){
    name = "restrict " + name;
    m_T->decl(os,name);
  }
  else {
    os << "restrict ";
    m_T->decl(os,name);
  }
}

void cxx_compiler::restrict_type::encode(std::ostream& os) const
{
  os << 'R';
  m_T->encode(os);
}

bool cxx_compiler::restrict_type::include_cvr(const type* that) const
{
  int x = 0;
  const type* a = this->unqualified(&x);
  int y = 0;
  const type* b = that->unqualified(&y);
  if ( ~x & y )
    return false;
  return a->include_cvr(b);
}

const cxx_compiler::type* cxx_compiler::restrict_type::patch(const type* T, usr* u) const
{
  return create(m_T->patch(T,u));
}

const cxx_compiler::type* cxx_compiler::restrict_type::qualified(int cvr) const
{
  cvr |= 4;
  return m_T->qualified(cvr);
}

const cxx_compiler::restrict_type* cxx_compiler::restrict_type::create(const type* T)
{
  if ( T->temporary(false) ){
    const restrict_type* ret = new restrict_type(T);
    if ( scope::current->m_id == scope::BLOCK )
      type_impl::temp1.push_back(ret);
#ifdef _DEBUG
    else
      type_impl::temp2.push_back(ret);
#endif // _DEBUG
    return ret;
  }
  table_t::const_iterator p = table.find(T);
  if ( p != table.end() )
    return p->second;
  else
    return table[T] = new restrict_type(T);
}

struct cxx_compiler::func_type::table_t : cxx_compiler::misc::pmap<std::pair<const type*, std::vector<const type*> >, const func_type> {};

cxx_compiler::func_type::table_t cxx_compiler::func_type::table;

void cxx_compiler::func_type::decl(std::ostream& os, std::string name) const
{
  using namespace std;
  if ( !m_T ){
    os << name;
    post(os);
    return;
  }
  const type* T = m_T->prev();
  typedef const pointer_type PT;
  int cvr = 0;
  T = T->unqualified(&cvr);
  if ( T->m_id == type::POINTER ){
    PT* pt = static_cast<PT*>(T);
    const type* x = pt->referenced_type();
    const type* y = x->prev();
    if ( x != y ){
      ostringstream tmp;
      tmp << '(' << '*';
      if ( cvr & 1 )
        tmp << "const ";
      if ( cvr & 2 )
        tmp << "volatile ";
      if ( cvr & 2 )
        tmp << "restrict ";
      tmp << name;
      y->decl(os,tmp.str());
      post(os);
      os << ')';
      x->post(os);
      return;
    }
    T = T->qualified(cvr);
  }
  T->decl(os,name);
  post(os);
}

void cxx_compiler::func_type::encode(std::ostream& os) const
{
  using namespace std;
  os << 'F';
  if ( m_T )
    m_T->encode(os);
  typedef vector<const type*>::const_iterator IT;
  for ( IT p = m_param.begin() ; p != m_param.end() ; ++p )
    (*p)->encode(os);
  os << 'E';
}

void cxx_compiler::func_type::post(std::ostream& os) const
{
  using namespace std;
  os << '(';
  typedef vector<const type*>::const_iterator IT;
  for ( IT p = m_param.begin() ; p != m_param.end() ; ++p ){
    if ( p != m_param.begin() )
      os << ',';
    (*p)->decl(os,"");
  }
  os << ')';
}

bool cxx_compiler::func_type::compatible(const type* T) const
{
  using namespace std;
  if ( this == T )
    return true;
  if ( T->m_id != type::FUNC )
    return false;
  typedef const func_type FUNC;
  FUNC* that = static_cast<FUNC*>(T);
  if ( this->m_T ){
    if ( !this->m_T->compatible(that->m_T) )
      return false;
  }
  const vector<const type*>& u = this->m_param;
  const vector<const type*>& v = that->m_param;
  if ( u.size() != v.size() )
    return false;
  return mismatch(u.begin(),u.end(),v.begin(),cxx_compiler::compatible) == make_pair(u.end(),v.end());
}

const cxx_compiler::type* cxx_compiler::func_type::composite(const type* T) const
{
  using namespace std;
  if ( this == T )
    return this;
  if ( T->m_id != type::FUNC )
    return 0;
  typedef const func_type FUNC;
  FUNC* that = static_cast<FUNC*>(T);
  if ( this->m_T ){
    if ( !this->m_T->compatible(that->m_T) )
      return 0;
  }
  const vector<const type*>& u = this->m_param;
  const vector<const type*>& v = that->m_param;
  if ( u.size() != v.size() )
    return 0;
  if ( mismatch(u.begin(),u.end(),v.begin(),cxx_compiler::compatible) != make_pair(u.end(),v.end()) )
    return 0;
  vector<const type*> param;
  transform(u.begin(),u.end(),v.begin(),back_inserter(param),cxx_compiler::composite);
  if ( !m_T )
    return create(0,param);
  return create(this->m_T->composite(that->m_T),param);
}

namespace cxx_compiler { namespace func_impl {
  bool include_cvr(const type* x, const type* y){ return x->include_cvr(y); }
} } // end of namespace func_impl and c_compier

bool cxx_compiler::func_type::include_cvr(const type* T) const
{
  using namespace std;
  if ( this == T )
    return true;
  if ( T->m_id != type::FUNC )
    return false;
  typedef const func_type FUNC;
  FUNC* that = static_cast<FUNC*>(T);
  if ( this->m_T ){
    if ( !this->m_T->include_cvr(that->m_T) )
      return false;
  }
  const vector<const type*>& u = this->m_param;
  const vector<const type*>& v = that->m_param;
  if ( u.size() != v.size() )
    return false;
  return mismatch(u.begin(),u.end(),v.begin(),func_impl::include_cvr) == make_pair(u.end(),v.end());
}

const cxx_compiler::type* cxx_compiler::func_type::patch(const type* T, usr* u) const
{
  if ( T && m_T )
    T = m_T->patch(T,u);
  if ( T && T->m_id == type::FUNC ){
    using namespace error::declarations::declarators::function;
    of_function(parse::position,u);
    T = int_type::create();
  }
  if ( T && T->m_id == type::ARRAY ){
    using namespace error::declarations::declarators::function;
    of_array(parse::position,u);
    T = int_type::create();
  }
  if ( u ){
    usr::flag_t& flag = u->m_flag;
    flag = usr::flag_t(flag | usr::FUNCTION);
  }
  return create(T,m_param);
}

const cxx_compiler::type* cxx_compiler::func_type::qualified(int cvr) const
{
  return create(m_T->qualified(cvr),m_param);
}

const cxx_compiler::type* cxx_compiler::func_type::complete_type() const
{
  using namespace std;
  const type* T = m_T ? m_T->complete_type() : 0;
  vector<const type*> param;
  transform(m_param.begin(),m_param.end(),back_inserter(param),mem_fun(&type::complete_type));

  return create(T,param);
}

const cxx_compiler::pointer_type* cxx_compiler::func_type::ptr_gen() const
{
  return pointer_type::create(this);
}

bool cxx_compiler::func_type::temporary(bool b) const
{
  using namespace std;
  return m_T && m_T->temporary(b) || find_if(m_param.begin(),m_param.end(),bind2nd(mem_fun(&type::temporary),b)) != m_param.end();
}

bool cxx_compiler::func_type::overloadable(const func_type* that) const
{
  using namespace std;
  const vector<const type*>& u = this->m_param;
  const vector<const type*>& v = that->m_param;
  if ( u.size() != v.size() )
    return true;
  return mismatch(u.begin(),u.end(),v.begin(),cxx_compiler::compatible) != make_pair(u.end(),v.end());
}

const cxx_compiler::func_type* cxx_compiler::func_type::create(const type* T,
                                                           const std::vector<const type*>& param)
{
  using namespace std;
  if ( T && T->temporary(false) || find_if(param.begin(),param.end(),bind2nd(mem_fun(&type::temporary),false)) != param.end() ){
    const func_type* ret = new func_type(T,param);
    if ( scope::current->m_id == scope::BLOCK )
      type_impl::temp1.push_back(ret);
#ifdef _DEBUG
    else
      type_impl::temp2.push_back(ret);
#endif // _DEBUG
    return ret;
  }
  pair<const type*, vector<const type*> > key(T,param);
  table_t::const_iterator p = table.find(key);
  if ( p != table.end() )
    return p->second;
  else
    return table[key] = new func_type(T,param);
}

struct cxx_compiler::array_type::table_t : cxx_compiler::misc::pmap<std::pair<const type*,int>, const array_type> {};

cxx_compiler::array_type::table_t cxx_compiler::array_type::table;

void cxx_compiler::array_type::decl(std::ostream& os, std::string name) const
{
  using namespace std;
  const type* T = m_T->prev();
  typedef const pointer_type PT;
  int cvr = 0;
  T = T->unqualified(&cvr);
  if ( T->m_id == type::POINTER ){
    PT* pt = static_cast<PT*>(T);
    const type* x = pt->referenced_type();
    const type* y = x->prev();
    if ( x != y ){
      ostringstream tmp;
      tmp << '(' << '*';
      if ( cvr & 1 )
        tmp << "const ";
      if ( cvr & 2 )
        tmp << "volatile ";
      if ( cvr & 2 )
        tmp << "restrict ";
      tmp << name;
      y->decl(os,tmp.str());
      post(os);
      os << ')';
      x->post(os);
      return;
    }
    T = T->qualified(cvr);
  }
  T->decl(os,name);
  post(os);
}

void cxx_compiler::array_type::encode(std::ostream& os) const
{
  os << 'A';
  os << m_dim;
  os << '_';
  m_T->encode(os);
}

bool cxx_compiler::array_type::compatible(const type* T) const
{
  if ( this == T )
    return true;
  if ( T->m_id == type::ARRAY ){
    typedef const array_type ARRAY;
    ARRAY* that = static_cast<ARRAY*>(T);
    if ( this->m_dim && that->m_dim && this->m_dim != that->m_dim )
      return false;
    return this->m_T->compatible(that->m_T);
  }
  else if ( T->m_id == type::VARRAY ){
    typedef const varray_type VARRAY;
    VARRAY* that = static_cast<VARRAY*>(T);
    return this->m_T->compatible(that->element_type());
  }
  else
    return false;
}

const cxx_compiler::type* cxx_compiler::array_type::composite(const type* T) const
{
  if ( this == T )
    return this;
  if ( T->m_id == type::ARRAY ){
    typedef const array_type ARRAY;
    ARRAY* that = static_cast<ARRAY*>(T);
    if ( this->m_dim && that->m_dim && this->m_dim != that->m_dim )
      return 0;
    if ( !this->m_T->compatible(that->m_T) )
      return 0;
    return this->m_dim ? this : that;
  }
  else if ( T->m_id == type::VARRAY ){
    typedef const varray_type VARRAY;
    VARRAY* that = static_cast<VARRAY*>(T);
    return this->m_T->compatible(that->element_type()) ? that : 0;
  }
  else
    return 0;
}

bool cxx_compiler::array_type::include_cvr(const type* T) const
{
  if ( this == T )
    return true;
  if ( T->m_id == type::ARRAY ){
    typedef const array_type ARRAY;
    ARRAY* that = static_cast<ARRAY*>(T);
    return this->m_T->include_cvr(that->m_T);
  }
  else if ( T->m_id == type::VARRAY ){
    typedef const varray_type VARRAY;
    VARRAY* that = static_cast<VARRAY*>(T);
    return this->m_T->include_cvr(that->element_type());
  }
  else
    return false;
}

void cxx_compiler::array_type::post(std::ostream& os) const
{
  os << '[';
  if ( m_dim )
    os << m_dim;
  os << ']';
  m_T->post(os);
}

const cxx_compiler::type* cxx_compiler::array_type::patch(const type* T, usr* u) const
{
  T = m_T->patch(T,u);
  if ( T->m_id == type::FUNC ){
    using namespace error::declarations::declarators::array;
    of_function(parse::position,u);
    T = backpatch_type::create();
  }
  T = T->complete_type();
  return create(T,m_dim);
}

const cxx_compiler::type* cxx_compiler::array_type::qualified(int cvr) const
{
  return create(m_T->qualified(cvr),m_dim);
}

const cxx_compiler::type* cxx_compiler::array_type::complete_type() const
{
  return create(m_T->complete_type(),m_dim);
}

const cxx_compiler::pointer_type* cxx_compiler::array_type::ptr_gen() const
{
  return pointer_type::create(m_T);
}

std::pair<int, const cxx_compiler::type*> cxx_compiler::array_type::current(int nth) const
{
  using namespace std;
  return ( m_dim && m_dim <= nth ) ? make_pair(-1,static_cast<const type*>(0))
    : make_pair(m_T->size() * nth, m_T);
}

cxx_compiler::var* cxx_compiler::array_type::vsize() const
{
  using namespace std;
  var* size = m_T->vsize();
  if ( !size )
    return 0;
  usr* dim = expressions::primary::literal::integer::create(m_dim);
  return size->mul(dim);
}

const cxx_compiler::array_type* cxx_compiler::array_type::create(const type* T, int dim)
{
  using namespace std;
  if ( T->temporary(false) ){
    const array_type* ret = new array_type(T,dim);
    if ( scope::current->m_id == scope::BLOCK )
      type_impl::temp1.push_back(ret);
#ifdef _DEBUG
    else
      type_impl::temp2.push_back(ret);
#endif // _DEBUG
    return ret;
  }
  pair<const type*, int> key(T,dim);
  table_t::const_iterator p = table.find(key);
  if ( p != table.end() )
    return p->second;
  else
    return table[key] = new array_type(T,dim);
}

struct cxx_compiler::pointer_type::table_t : cxx_compiler::misc::pmap<const type*, const pointer_type> {};

int cxx_compiler::pointer_type::size() const { return type_impl::pointer_sizeof; }

cxx_compiler::pointer_type::table_t cxx_compiler::pointer_type::table;

void cxx_compiler::pointer_type::decl(std::ostream& os, std::string name) const
{
  using namespace std;
  const type* T = m_T->prev();
  ostringstream tmp;
  if ( T == m_T )
    tmp << '*' << name;
  else
    tmp << '(' << '*' << name << ')';
  m_T->decl(os,tmp.str());
}

void cxx_compiler::pointer_type::encode(std::ostream& os) const
{
  os << 'P';
  m_T->encode(os);
}

const cxx_compiler::type* cxx_compiler::pointer_type::patch(const type* T, usr* u) const
{
  T = m_T->patch(T,u);
  if ( u ){
    usr::flag_t& flag = u->m_flag;
    flag = (usr::flag_t)(flag & ~usr::FUNCTION & ~usr::VL);
  }
  return create(T);
}

bool cxx_compiler::pointer_type::compatible(const type* T) const
{
  if ( this == T )
    return true;
  if ( T->m_id != type::POINTER )
    return false;
  typedef const pointer_type PT;
  PT* that = static_cast<PT*>(T);
  return this->m_T->compatible(that->m_T);
}

const cxx_compiler::type* cxx_compiler::pointer_type::composite(const type* T) const
{
  if (this == T)
    return this;
  if (T->m_id != type::POINTER)
    return 0;
  typedef const pointer_type PT;
  PT* that = static_cast<PT*>(T);
  T = this->m_T->composite(that->m_T);
  return T ? create(T) : 0;
}

bool cxx_compiler::pointer_type::include_cvr(const type* T) const
{
  if ( T->m_id != type::POINTER )
    return false;
  typedef const pointer_type PT;
  PT* that = static_cast<PT*>(T);
  const type* a = this->m_T;
  const type* b = that->m_T;
  int c = 0;
  int d = 0;
  const type* e = a->unqualified(&c);
  const type* f = b->unqualified(&d);
  if ( ~c & d )
    return false;
  if ( e->m_id != type::POINTER )
    return a->include_cvr(b);
  PT* g = static_cast<PT*>(e);
  PT* h = static_cast<PT*>(f);
  if ( !g->include_cvr(h) )
    return false;
  const type* i = g->m_T;
  const type* j = h->m_T;
  int k = 0;
  int l = 0;
  i->unqualified(&k);
  j->unqualified(&l);
  if ( ~k & l )
    return false;
  if ( (k&4) && !(l&4) && !(c&1) )
    return false;
  if ( (k&2) && !(l&2) && !(c&1) )
    return false;
  if ( (k&1) && !(l&1) && !(c&1) )
    return false;
  return true;
}

const cxx_compiler::type* cxx_compiler::pointer_type::complete_type() const
{
  return create(m_T->complete_type());
}

const cxx_compiler::pointer_type* cxx_compiler::pointer_type::create(const type* T)
{
  if ( T->temporary(false) ){
    const pointer_type* ret = new pointer_type(T);
    if ( scope::current->m_id == scope::BLOCK )
      type_impl::temp1.push_back(ret);
#ifdef _DEBUG
    else
      type_impl::temp2.push_back(ret);
#endif // _DEBUG
    return ret;
  }
  table_t::const_iterator p = table.find(T);
  if ( p != table.end() )
    return p->second;
  else
    return table[T] = new pointer_type(T);
}

struct cxx_compiler::reference_type::table_t : cxx_compiler::misc::pmap<const type*, const reference_type> {};

int cxx_compiler::reference_type::size() const { return type_impl::pointer_sizeof; }

cxx_compiler::reference_type::table_t cxx_compiler::reference_type::table;

void cxx_compiler::reference_type::decl(std::ostream& os, std::string name) const
{
  using namespace std;
  const type* T = m_T->prev();
  ostringstream tmp;
  if ( T == m_T )
    tmp << '&' << name;
  else
    tmp << '(' << '&' << name << ')';
  m_T->decl(os,tmp.str());
}

void cxx_compiler::reference_type::encode(std::ostream& os) const
{
  os << 'R';
  m_T->encode(os);
}

const cxx_compiler::type* cxx_compiler::reference_type::patch(const type* T, usr* u) const
{
  T = m_T->patch(T,u);
  if ( u ){
    usr::flag_t& flag = u->m_flag;
    flag = (usr::flag_t)(flag & ~usr::FUNCTION & ~usr::VL);
  }
  return create(T);
}

bool cxx_compiler::reference_type::compatible(const type* T) const
{
  if (this == T)
    return true;
  if (T->m_id != type::REFERENCE)
    return false;
  typedef const reference_type REF;
  REF* that = static_cast<REF*>(T);
  return this->m_T->compatible(that->m_T);
}

const cxx_compiler::type* cxx_compiler::reference_type::composite(const type* T) const
{
  if (this == T)
    return this;
  if (T->m_id != type::REFERENCE)
    return 0;
  typedef const reference_type REF;
  REF* that = static_cast<REF*>(T);
  T = this->m_T->composite(that->m_T);
  return T ? create(T) : 0;
}

bool cxx_compiler::reference_type::include_cvr(const type* T) const
{
  if ( T->m_id != type::REFERENCE )
    return false;
  typedef const reference_type REF;
  REF* that = static_cast<REF*>(T);
  const type* a = this->m_T;
  const type* b = that->m_T;
  int c = 0;
  int d = 0;
  const type* e = a->unqualified(&c);
  const type* f = b->unqualified(&d);
  if ( ~c & d )
    return false;
  if ( e->m_id != type::REFERENCE )
    return a->include_cvr(b);
  REF* g = static_cast<REF*>(e);
  REF* h = static_cast<REF*>(f);
  if ( !g->include_cvr(h) )
    return false;
  const type* i = g->m_T;
  const type* j = h->m_T;
  int k = 0;
  int l = 0;
  i->unqualified(&k);
  j->unqualified(&l);
  if ( ~k & l )
    return false;
  if ( (k&4) && !(l&4) && !(c&1) )
    return false;
  if ( (k&2) && !(l&2) && !(c&1) )
    return false;
  if ( (k&1) && !(l&1) && !(c&1) )
    return false;
  return true;
}

const cxx_compiler::type* cxx_compiler::reference_type::complete_type() const
{
  return create(m_T->complete_type());
}

const cxx_compiler::reference_type* cxx_compiler::reference_type::create(const type* T)
{
  if ( T->temporary(false) ){
    const reference_type* ret = new reference_type(T);
    if ( scope::current->m_id == scope::BLOCK )
      type_impl::temp1.push_back(ret);
#ifdef _DEBUG
    else
      type_impl::temp2.push_back(ret);
#endif // _DEBUG
    return ret;
  }
  table_t::const_iterator p = table.find(T);
  if ( p != table.end() )
    return p->second;
  else
    return table[T] = new reference_type(T);
}

void cxx_compiler::ellipsis_type::decl(std::ostream& os, std::string name) const
{
  os << "...";
  assert(name.empty());
}

void cxx_compiler::ellipsis_type::encode(std::ostream& os) const { os << 'z'; }

cxx_compiler::ellipsis_type cxx_compiler::ellipsis_type::m_obj;

const cxx_compiler::ellipsis_type* cxx_compiler::ellipsis_type::create()
{
  return &m_obj;
}

void cxx_compiler::incomplete_tagged_type::decl(std::ostream& os, std::string name) const
{
  os << tag::keyword(m_tag->m_kind) << ' ' << m_tag->m_name;
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::incomplete_tagged_type::encode(std::ostream& os) const
{
  os << 1;
  os << m_tag->m_name;
}

bool cxx_compiler::incomplete_tagged_type::compatible(const type* T) const
{
  if (this == T)
    return true;
  if (T->m_id == type::RECORD) {
    typedef const record_type REC;
    REC* rec = static_cast<REC*>(T);
    return m_tag == rec->get_tag();
  }
  if (T->m_id == type::ENUM) {
    typedef const enum_type ENUM;
    ENUM* et = static_cast<ENUM*>(T);
    return m_tag == et->get_tag();
  }
  return false;
}

const cxx_compiler::type* cxx_compiler::incomplete_tagged_type::composite(const type* T) const
{
  if (this == T)
    return this;
  if (T->m_id == type::RECORD) {
    typedef const record_type REC;
    REC* rec = static_cast<REC*>(T);
    return m_tag == rec->get_tag() ? T : 0;
  }
  if ( T->m_id == type::ENUM ){
    typedef const enum_type ENUM;
    ENUM* et = static_cast<ENUM*>(T);
    return m_tag == et->get_tag() ? T : 0;
  }
  return 0;
}

const cxx_compiler::type* cxx_compiler::incomplete_tagged_type::complete_type() const
{
  return m_tag->m_types.second ? m_tag->m_types.second : this;
}

namespace cxx_compiler {
  bool temporary_helper(tag* Tag)
  {
    scope* parent = Tag->m_parent;
    if ( parent->m_id == scope::TAG ){
      tag* T = static_cast<tag*>(parent);
      return temporary_helper(T);
    }
    else
      return parent != &scope::root;
  }
} // end of namespace cxx_compiler

bool cxx_compiler::incomplete_tagged_type::temporary(bool vm) const
{
  if ( vm )
    return false;
  else
    return temporary_helper(m_tag);
}

const cxx_compiler::incomplete_tagged_type* cxx_compiler::incomplete_tagged_type::create(tag* tag)
{
  return new incomplete_tagged_type(tag);
}

namespace cxx_compiler { namespace record_impl {
  struct layouter {
    std::insert_iterator<std::map<std::string, std::pair<int, usr*> > > X;
    std::insert_iterator<std::map<usr*, int> > Y;
    usr* m_last;
    struct current {
      usr* m_member;
      const type* m_integer;
      int m_position;
      current(usr* member = 0, const type* T = 0)
        : m_member(member), m_integer(T), m_position(0) {}
    } m_current;
    int operator()(int, usr*);
    layouter(std::insert_iterator<std::map<std::string, std::pair<int, usr*> > > XX,
             std::insert_iterator<std::map<usr*, int> > YY,
             usr* last)
             : X(XX), Y(YY), m_last(last) {}
  };
  struct grounder {
    std::insert_iterator<std::map<usr*, int> > Y;
    std::pair<std::string, std::pair<int, usr*> > operator()(usr*);
    grounder(std::insert_iterator<std::map<usr*, int> > YY) : Y(YY) {}
  };
  bool comp_size(usr*, usr*);
  bool comp_align(usr*, usr*);
  bool member_modifiable(usr*);
  struct info {
    std::vector<usr*>* m_member;
    std::vector<usr*>* m_vb_member;
    int m_virtual_base;
    usr* m_vbtbl;
    int m_virtual_func;
    usr* m_vftbl;
    info(std::vector<usr*>* m, std::vector<usr*>* v)
      : m_member(m), m_vb_member(v),  m_virtual_base(0), m_vbtbl(0), m_virtual_func(0), m_vftbl(0) {}
  };
  int base_handler(base*, info*);
  int member_handler(usr*, info*);
  bool is_virtual_func(usr*);
  struct set_vftbl {
    int m_offset;
    set_vftbl() : m_offset(0) {}
    std::pair<int, var*> operator()(usr*);
  };
  bool is_virtual_base(base*);
  struct set_vbtbl {
    int m_offset;
    int m_delta;
    set_vbtbl() : m_offset(0), m_delta(0) {}
    std::pair<int, var*> operator()(base*);
  };
  void add_ctor(tag*, const info&);
} } // end of namespace record_impl and cxx_compiler

cxx_compiler::record_type::record_type(tag* Tg)
  : type(RECORD), m_tag(Tg), m_vbtbl(0), m_vftbl(0)
{
  using namespace std;
  using namespace record_impl;
  const vector<base*>* bases = m_tag->m_bases;
  vector<usr*> vb_member;
  info arg(&m_member,&vb_member);
  if ( bases )
    for_each(bases->begin(),bases->end(),bind2nd(ptr_fun(base_handler),&arg));
  const vector<usr*>& member = m_tag->m_order;
  for_each(member.begin(),member.end(),bind2nd(ptr_fun(member_handler),&arg));
  if ( int n = arg.m_virtual_func ){
    const type* T = void_type::create();
    T = pointer_type::create(T);
    T = array_type::create(T,n);
    m_vftbl = new with_initial(".vftbl",T,file_t());
    m_vftbl->m_flag = usr::STATIC_DEF;
    T = pointer_type::create(T);
    usr* vfptr = new usr("",T,usr::NONE,file_t());
    m_member.insert(m_member.begin(),vfptr);
    remove_copy_if(member.begin(),member.end(),back_inserter(m_vftbl_contents),
      not1(ptr_fun(is_virtual_func)));
    map<int, var*>& value = m_vftbl->m_value;
    transform(m_vftbl_contents.begin(),m_vftbl_contents.end(),inserter(value,value.begin()),set_vftbl());
  }
  if ( int n = arg.m_virtual_base ){
    const type* T = int_type::create();
    T = array_type::create(T,n);
    m_vbtbl = new with_initial(".vbtbl",T,file_t());
    m_vbtbl->m_flag = usr::STATIC_DEF;
    T = pointer_type::create(T);
    usr* vbptr = new usr("",T,usr::NONE,file_t());
    m_member.insert(m_member.begin(),vbptr);
    remove_copy_if(bases->begin(),bases->end(),back_inserter(m_vbtbl_contents),
      not1(ptr_fun(is_virtual_base)));
    map<int, var*>& value = m_vbtbl->m_value;
    transform(m_vbtbl_contents.begin(),m_vbtbl_contents.end(),inserter(value,value.begin()),set_vbtbl());
  }
  add_ctor(m_tag,arg);
  if ( m_member.empty() ){
    usr* u = new usr("",char_type::create(),usr::NONE,file_t());
    m_member.push_back(u);
  }
  if ( m_tag->m_kind == tag::STRUCT ){
    usr* last = *m_member.rbegin();
    m_size = accumulate(m_member.begin(),m_member.end(),0,
                        layouter(inserter(m_layout,m_layout.begin()),inserter(m_position,m_position.begin()),last));
    const type* T = m_member[0]->m_type;
    typedef const bit_field_type BF;
    if ( T->m_id == type::BIT_FIELD ){
      BF* bf = static_cast<BF*>(T);
      T = bf->integer_type();
    }
    m_align = T->align();
    T = m_member.back()->m_type;
    if ( T->m_id == type::BIT_FIELD ){
      BF* bf = static_cast<BF*>(T);
      T = bf->integer_type();
      m_size += T->size();
      usr::flag_t& flag = m_member.back()->m_flag;
      flag = usr::flag_t(flag | usr::MSB_FIELD);
    }
    if ( int n = m_size % m_align )
      m_size += m_align - n;
  }
  else {
    transform(m_member.begin(),m_member.end(),inserter(m_layout,m_layout.begin()),
      grounder(inserter(m_position,m_position.begin())));
    {
      vector<usr*>::const_iterator p = max_element(m_member.begin(),m_member.end(),comp_size);
      const type* T = (*p)->m_type;
      if ( T->m_id == type::BIT_FIELD ){
        typedef const bit_field_type BF;
        BF* bf = static_cast<BF*>(T);
        T = bf->integer_type();
      }
      m_size = T->size();
    }
    {
      vector<usr*>::const_iterator p = max_element(m_member.begin(),m_member.end(),comp_align);
      const type* T = (*p)->m_type;
      if ( T->m_id == type::BIT_FIELD ){
        typedef const bit_field_type BF;
        BF* bf = static_cast<BF*>(T);
        T = bf->integer_type();
      }
      m_align = T->align();
    }
  }
  m_modifiable =
    find_if(m_member.begin(),m_member.end(),not1(ptr_fun(member_modifiable))) == m_member.end();
}

int cxx_compiler::record_impl::layouter::operator()(int offset, usr* member)
{
  using namespace std;
  if ( member->m_flag & usr::BIT_FIELD ){
    const type* T = member->m_type;
    typedef const bit_field_type BF;
    BF* bf = static_cast<BF*>(T);
    T = bf->integer_type();
    bool update = false;
    if ( const type* C = m_current.m_integer ){
      if ( C != T ){
        usr::flag_t& flag = m_current.m_member->m_flag;
        flag = usr::flag_t(flag | usr::MSB_FIELD);
        offset += C->size();
        m_current = current(member,T);
      }
      else if ( m_current.m_position + bf->bit() > T->size() * 8 ){
        offset += C->size();
        usr::flag_t& flag = m_current.m_member->m_flag;
        flag = usr::flag_t(flag | usr::MSB_FIELD);
        m_current = current(member,T);
      }
      else {
        m_current.m_member = member;
        update = true;
      }
    }
    else {
      m_current = current(member,T);
      update = true;
    }
    int align = T->align();
    if ( int n = offset % align )
      offset += align - n;
    string name = member->m_name;
    *X++ = make_pair(name,make_pair(offset,member));
    *Y++ = make_pair(member,m_current.m_position);
    if ( update )
      m_current.m_position += bf->bit();
    return offset;
  }
  else {
    if ( const type* C = m_current.m_integer ){
      usr::flag_t& flag = m_current.m_member->m_flag;
      flag = usr::flag_t(flag | usr::MSB_FIELD);
      offset += C->size();
    }
    m_current = current();
    string name = member->m_name;
    const type* T = member->m_type;
    if ( T->temporary(true) ){
      using namespace error::classes;
      not_ordinary(member);
      T = member->m_type = int_type::create();
    }
    if ( !T->size() ){
      int n = code.size();
      typedef const array_type ARRAY;
      ARRAY* array = T->m_id == type::ARRAY ? static_cast<ARRAY*>(T) : 0;
      if ( member == m_last && array && !array->vsize() ){
        T = array->element_type();
        int align = T->align();
        assert(align);
        if ( int n = offset % align )
          offset += align - n;
        *X++ = make_pair(name,make_pair(offset,member));
        return offset;
      }
      for_each(code.begin()+n,code.end(),misc::deleter<tac>());
      code.resize(n);
      using namespace error::classes;
      incomplete_member(member);
      T = member->m_type = int_type::create();
    }
    int align = T->align();
    assert(align);
    if ( int n = offset % align )
      offset += align - n;
    *X++ = make_pair(name,make_pair(offset,member));
    return offset + T->size();
  }
}

std::pair<std::string, std::pair<int, cxx_compiler::usr*> >
cxx_compiler::record_impl::grounder::operator()(usr* member)
{
  using namespace std;
  const type* T = member->m_type;
  if ( !T->size() ){
    using namespace error::classes;
    incomplete_member(member);
    T = member->m_type = int_type::create();
  }
  if ( T->m_id == type::BIT_FIELD ){
    typedef const bit_field_type BF;
    BF* bf = static_cast<BF*>(T);
    *Y++ = make_pair(member,0);
  }
  string name = member->m_name;
  return make_pair(name,make_pair(0,member));
}

bool cxx_compiler::record_impl::comp_size(usr* x, usr* y)
{
  typedef const bit_field_type BF;
  const type* xx = x->m_type;
  if ( xx->m_id == type::BIT_FIELD ){
    BF* bf = static_cast<BF*>(xx);
    xx = bf->integer_type();
  }
  const type* yy = y->m_type;
  if ( yy->m_id == type::BIT_FIELD ){
    BF* bf = static_cast<BF*>(yy);
    yy = bf->integer_type();
  }
  return xx->size() < yy->size();
}

bool cxx_compiler::record_impl::comp_align(usr* x, usr* y)
{
  typedef const bit_field_type BF;
  typedef const bit_field_type BF;
  const type* xx = x->m_type;
  if ( xx->m_id == type::BIT_FIELD ){
    BF* bf = static_cast<BF*>(xx);
    xx = bf->integer_type();
  }
  const type* yy = y->m_type;
  if ( yy->m_id == type::BIT_FIELD ){
    BF* bf = static_cast<BF*>(yy);
    yy = bf->integer_type();
  }
  return xx->align() < yy->align();
}

bool cxx_compiler::record_impl::member_modifiable(usr* member)
{
  return member->m_type->modifiable();
}

int cxx_compiler::record_impl::base_handler(base* p, info* arg)
{
  using namespace std;
  bool vb = is_virtual_base(p);
  if ( vb )
    ++arg->m_virtual_base;
  tag* Tg = p->m_tag;
  const vector<usr*>& member = Tg->m_order;
  info tmp(vb && arg->m_vb_member ? arg->m_vb_member : arg->m_member, 0);
  for_each(member.begin(),member.end(),bind2nd(ptr_fun(member_handler),&tmp));
  return 0;
}

int cxx_compiler::record_impl::member_handler(usr* u, info* arg)
{
  using namespace std;
  if ( is_virtual_func(u) ){
    ++arg->m_virtual_func;
    return 0;
  }
  usr::flag_t flag = u->m_flag;
  if ( flag & usr::FUNCTION )
    return 0;
  if ( flag & usr::STATIC )
    return 0;
  arg->m_member->push_back(u);
  return 0;
}

bool cxx_compiler::record_impl::is_virtual_func(usr* u)
{
  usr::flag_t flag = u->m_flag;
  return flag & usr::VIRTUAL;
}

std::pair<int, cxx_compiler::var*>
cxx_compiler::record_impl::set_vftbl::operator()(usr* vf)
{
  using namespace std;
  int offset = m_offset;
  const type* T = vf->m_type;
  T = pointer_type::create(T);
  int delta = T->size();
  m_offset += delta;
  return make_pair(offset, new addrof(T,vf,0));
}

bool cxx_compiler::record_impl::is_virtual_base(cxx_compiler::base* b)
{
  return b->m_virtual;
}

std::pair<int, cxx_compiler::var*>
cxx_compiler::record_impl::set_vbtbl::operator()(base* vb)
{
  using namespace std;
  int offset = m_offset;
  m_offset += int_type::create()->size();
  tag* Tg = vb->m_tag;
  const type* T = Tg->m_types.second;
  m_delta += T->size();
  var* delta = expressions::primary::literal::integer::create(m_delta);
  return make_pair(offset, delta);
}

void cxx_compiler::record_impl::add_ctor(tag* Tg, const info& arg)
{
  using namespace std;

  if ( !arg.m_virtual_base && !arg.m_virtual_func )
    return;

  usr* ctor = 0;
  string tgn = Tg->m_name;
  map<string, vector<usr*> >::const_iterator p = Tg->m_usrs.find(tgn);
  if ( p == Tg->m_usrs.end() ){
    vector<const type*> param;
    param.push_back(void_type::create());
    const type* T = func_type::create(0,param);
    usr::flag_t flag = usr::flag_t(usr::FUNCTION | usr::INLINE);
    ctor = new usr(tgn,T,flag,file_t());
    Tg->m_usrs[tgn].push_back(ctor);
  }
  else {
    const vector<usr*>& v = p->second;
    ctor = v.back();
  }

  scope* param = new scope(scope::PARAM);
  param->m_parent = Tg;
  Tg->m_children.push_back(param);
  string name = "this";
  const type* T = Tg->m_types.first;
  T = pointer_type::create(T);
  usr* This = new usr(name,T,usr::NONE,file_t());
  param->m_order.push_back(This);
  param->m_usrs[name].push_back(This);

  block* b = new block;
  b->m_parent = param;
  param->m_children.push_back(b);

  assert(code.empty());
  if ( arg.m_virtual_base ){
    const type* T = arg.m_vbtbl->m_type;
    T = pointer_type::create(T);
    var* tmp = new var(T);
    b->m_vars.push_back(tmp);
    code.push_back(new addr3ac(tmp,arg.m_vbtbl));
    code.push_back(new invladdr3ac(This,tmp));
  }
  if ( arg.m_virtual_func ){
    const type* T = arg.m_vftbl->m_type;
    T = pointer_type::create(T);
    var* tmp = new var(T);
    b->m_vars.push_back(tmp);
    code.push_back(new addr3ac(tmp,arg.m_vftbl));
    if ( arg.m_virtual_base ){
      var* ptr = new var(pointer_type::create(T));
      var* size = expressions::primary::literal::integer::create(T->size());
      code.push_back(new add3ac(ptr,This,size));
      code.push_back(new invladdr3ac(ptr,tmp));
    }
    else
      code.push_back(new invladdr3ac(This,tmp));
  }
  fundef::current = new fundef(ctor,param);
  declarations::declarators::function::definition::action(code,true);
}

void cxx_compiler::record_type::decl(std::ostream& os, std::string name) const
{
  os << tag::keyword(m_tag->m_kind) << ' ' << m_tag->m_name;
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::record_type::encode(std::ostream& os) const
{
  os << 1;
  os << m_tag->m_name;
}

bool cxx_compiler::record_type::compatible(const type* T) const
{
  if (this == T)
    return true;

  if (T->m_id != type::INCOMPLETE_TAGGED)
    return false;
  
  typedef const incomplete_tagged_type ITT;
  ITT* that = static_cast<ITT*>(T);
  return m_tag == that->get_tag();
}

const cxx_compiler::type* cxx_compiler::record_type::composite(const type* T) const
{
  if (this == T)
    return this;
  if (T->m_id != type::INCOMPLETE_TAGGED)
    return 0;
  typedef const incomplete_tagged_type ITT;
  ITT* that = static_cast<ITT*>(T);
  return m_tag == that->get_tag() ? this : 0;
}

std::pair<int, cxx_compiler::usr*> cxx_compiler::record_type::offset(std::string name) const
{
  using namespace std;
  map<string, pair<int, usr*> >::const_iterator p = m_layout.find(name);
  if ( p != m_layout.end() )
    return p->second;
  else
    return make_pair(-1,static_cast<usr*>(0));
}

int cxx_compiler::record_type::position(usr* member) const
{
  using namespace std;
  map<usr*, int>::const_iterator p = m_position.find(member);
  assert(p != m_position.end());
  return p->second;
}

std::pair<int, const cxx_compiler::type*> cxx_compiler::record_type::current(int nth) const
{
  using namespace std;
  tag* tg = get_tag();
  if ( tg->m_kind == tag::UNION && nth >= 1 )
    return make_pair(-1,static_cast<const type*>(0));
  if ( m_member.size() <= nth )
    return make_pair(-1,static_cast<const type*>(0));
  usr* u = m_member[nth];
  const type* T = u->m_type;
  string name = u->m_name;
  map<string, pair<int, usr*> >::const_iterator p = m_layout.find(name);
  assert(p != m_layout.end());
  int offset = p->second.first;
  return make_pair(offset,T);
}

bool cxx_compiler::record_type::temporary(bool vm) const
{
  if ( vm )
    return false;
  else
    return temporary_helper(m_tag);
}

const cxx_compiler::record_type*
cxx_compiler::record_type::create(tag* Tg)
{
  return new record_type(Tg);
}

void cxx_compiler::enum_type::decl(std::ostream& os, std::string name) const
{
  os << tag::keyword(m_tag->m_kind) << ' ' << m_tag->m_name;
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::enum_type::encode(std::ostream& os) const
{
  os << 1;
  os << m_tag->m_name;
}

bool cxx_compiler::enum_type::compatible(const type* T) const
{
  if (this == T)
    return true;

  if (T->m_id != type::INCOMPLETE_TAGGED)
    return false;

  typedef const incomplete_tagged_type ITT;
  ITT* that = static_cast<ITT*>(T);
  return m_tag == that->get_tag();
}

const cxx_compiler::type* cxx_compiler::enum_type::composite(const type* T) const
{
  if (this == T)
    return this;

  if (T->m_id != type::INCOMPLETE_TAGGED)
    return 0;

  typedef const incomplete_tagged_type ITT;
  ITT* that = static_cast<ITT*>(T);
  return m_tag == that->get_tag() ? this : 0;
}

bool cxx_compiler::enum_type::temporary(bool vm) const
{
  if ( vm )
    return false;
  else
    return temporary_helper(m_tag);
}

const cxx_compiler::enum_type* cxx_compiler::enum_type::create(tag* tag, const type* integer)
{
  return new enum_type(tag,integer);
}

struct cxx_compiler::bit_field_type::table_t : cxx_compiler::misc::pmap<std::pair<int, const type*>, const bit_field_type> {};

cxx_compiler::bit_field_type::table_t cxx_compiler::bit_field_type::table;

void cxx_compiler::bit_field_type::decl(std::ostream& os, std::string name) const
{
  m_integer->decl(os,name);
  os << ':' << m_bit;
}

const cxx_compiler::type* cxx_compiler::bit_field_type::patch(const type* T, usr* u) const
{
  using namespace error::classes::bit_field;
  if ( !T->integer() ){
    not_integer_type(u);
    T = int_type::create();
  }
  usr::flag_t& flag = u->m_flag;
  flag = usr::flag_t(flag | usr::BIT_FIELD);
  int n = T->size();
  n <<= 3;
  int bit = m_bit;
  if ( bit > n ){
    exceed(u,T);
    bit = 1;
  }
  if ( !bit && !u->m_name.empty() ){
    zero(u);
    bit = 1;
  }
  return create(bit,m_integer->patch(T,u));
}

const cxx_compiler::bit_field_type* cxx_compiler::bit_field_type::create(int bit, const type* integer)
{
  using namespace std;
  table_t::const_iterator p = table.find(make_pair(bit,integer));
  if ( p != table.end() )
    return p->second;
  else
    return table[make_pair(bit,integer)] = new bit_field_type(bit,integer);
}

cxx_compiler::varray_type::varray_type(const type* T, var* dim)
  : type(VARRAY), m_T(T), m_decided(false)
{
  m_dim.first = dim;
  m_dim.second = new var(dim->m_type);
  m_dim.second->m_scope = 0;
}

cxx_compiler::varray_type::varray_type(const type* T,
                                       var* dim,
                                       const std::vector<tac*>& c)
  : type(VARRAY), m_T(T), m_decided(false), m_code(c)
{
  m_dim.first = dim;
  m_dim.second = new var(dim->m_type);
  m_dim.second->m_scope = 0;
}

cxx_compiler::varray_type::~varray_type()
{
  using namespace std;
  if ( !m_decided )
    delete m_dim.second;
  for_each(m_code.begin(),m_code.end(),misc::deleter<tac>());
}

void cxx_compiler::varray_type::decl(std::ostream& os, std::string name) const
{
  using namespace std;
  const type* T = m_T->prev();
  int cvr = 0;
  T = T->unqualified(&cvr);
  if ( T->m_id == type::POINTER ){
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(T);
    const type* x = pt->referenced_type();
    const type* y = x->prev();
    if ( x != y ){
      ostringstream tmp;
      tmp << '(' << '*';
      if ( cvr & 1 )
        tmp << "const ";
      if ( cvr & 2 )
        tmp << "volatile ";
      if ( cvr & 2 )
        tmp << "restrict ";
      tmp << name;
      y->decl(os,tmp.str());
      post(os);
      os << ')';
      x->post(os);
      return;
    }
    T = T->qualified(cvr);
  }
  T->decl(os,name);
  post(os);
}

void cxx_compiler::varray_type::encode(std::ostream& os) const
{
  os << "VA";
  m_T->encode(os);
}

bool cxx_compiler::varray_type::compatible(const type* T) const
{
  if ( this == T )
    return true;

  if (T->m_id == type::VARRAY) {
    typedef const varray_type VARRAY;
    VARRAY* that = static_cast<VARRAY*>(T);
    return this->m_T->compatible(that->m_T);
  }

  if (T->m_id == type::ARRAY) {
    typedef const array_type ARRAY;
    ARRAY* that = static_cast<ARRAY*>(T);
    return this->m_T->compatible(that->element_type());
  }

  return false;
}

const cxx_compiler::type* cxx_compiler::varray_type::composite(const type* T) const
{
  if ( this == T )
    return this;
  
  if (T->m_id == type::VARRAY) {
    typedef const varray_type VARRAY;
    VARRAY* that = static_cast<VARRAY*>(T);
    return this->m_T->compatible(that->m_T) ? this : 0;
  }
  
  if (T->m_id == type::ARRAY) {
    typedef const array_type ARRAY;
    ARRAY* that = static_cast<ARRAY*>(T);
    return this->m_T->compatible(that->element_type()) ? this : 0;
  }

  return 0;
}

bool cxx_compiler::varray_type::include_cvr(const type* T) const
{
  if ( this == T )
    return true;
  if ( T->m_id == type::VARRAY ){
    typedef const varray_type VARRAY;
    VARRAY* that = static_cast<VARRAY*>(T);
    return this->m_T->include_cvr(that->m_T);
  }
  else if ( T->m_id == type::ARRAY ){
    typedef const array_type ARRAY;
    ARRAY* that = static_cast<ARRAY*>(T);
    return this->m_T->include_cvr(that->element_type());
  }
  else
    return false;
}

void cxx_compiler::varray_type::post(std::ostream& os) const
{
  os << '[';
  os << ']';
  m_T->post(os);
}

const cxx_compiler::type* cxx_compiler::varray_type::patch(const type* T, usr* u) const
{
  T = m_T->patch(T,u);
  if ( T->m_id == type::FUNC ){
    using namespace error::declarations::declarators::array;
    of_function(parse::position,u);
    T = backpatch_type::create();
  }
  T = T->complete_type();
  if ( u ){
    usr::flag_t& flag = u->m_flag;
    flag = usr::flag_t(flag | usr::VL);
  }
  const type* ret = m_code.empty() ? create(T,m_dim.first) : create(T,m_dim.first,m_code);
  m_code.clear();
  return ret;
}

const cxx_compiler::type* cxx_compiler::varray_type::qualified(int cvr) const
{
  return create(m_T->qualified(cvr),m_dim.first);
}

const cxx_compiler::type* cxx_compiler::varray_type::complete_type() const
{
  m_T = m_T->complete_type();
  return this;
}

const cxx_compiler::pointer_type* cxx_compiler::varray_type::ptr_gen() const
{
  return pointer_type::create(m_T);
}

namespace cxx_compiler { namespace varray_impl {
  void move1(tac*);
  void move2(var*);
} } // end of namespace varray_impl and cxx_compiler

void cxx_compiler::varray_impl::move1(tac* ptr)
{
  if ( ptr->x ) move2(ptr->x);
  if ( ptr->y ) move2(ptr->y);
  if ( ptr->z ) move2(ptr->z);
}

void cxx_compiler::varray_impl::move2(var* v)
{
  using namespace std;
  vector<var*>::reverse_iterator p = find(garbage.rbegin(),garbage.rend(),v);
  if ( p != garbage.rend() ){
    garbage.erase(p.base()-1);
    block* b = static_cast<block*>(scope::current);
    v->m_scope = b;
    b->m_vars.push_back(v);
  }
}

void cxx_compiler::varray_type::decide() const
{
  using namespace std;
  if ( m_decided )
    return;
  m_T->decide();
  for_each(m_code.begin(),m_code.end(),varray_impl::move1);
  copy(m_code.begin(),m_code.end(),back_inserter(code));
  m_code.clear();
  block* b = static_cast<block*>(scope::current);
  m_dim.second->m_scope = b;
  b->m_vars.push_back(m_dim.second);
  code.push_back(new assign3ac(m_dim.second,m_dim.first));
  m_decided = true;
}

cxx_compiler::var* cxx_compiler::varray_type::vsize() const
{
  using namespace std;
  const type* T = m_T;
  T = T->complete_type();
  if ( var* vs = T->vsize() )
    return m_dim.second->mul(vs);
  else {
    int n = T->size();
    usr* size = expressions::primary::literal::integer::create(n);
    return m_dim.second->mul(size);
  }
}

const cxx_compiler::varray_type* cxx_compiler::varray_type::create(const type* T, var* dim)
{
  using namespace std;
  const varray_type* ret = new varray_type(T,dim);
  if ( scope::current->m_id == scope::BLOCK )
    type_impl::temp1.push_back(ret);
#ifdef _DEBUG
  else
    type_impl::temp2.push_back(ret);
#endif // _DEBUG
  return ret;
}

const cxx_compiler::varray_type* cxx_compiler::varray_type::create(const type* T, var* dim, const std::vector<tac*>& c)
{
  using namespace std;
  const varray_type* ret = new varray_type(T,dim,c);
  if ( scope::current->m_id == scope::BLOCK )
    type_impl::temp1.push_back(ret);
#ifdef _DEBUG
  else
    type_impl::temp2.push_back(ret);
#endif // _DEBUG
  return ret;
}

struct cxx_compiler::pointer_member_type::table_t : cxx_compiler::misc::pmap<pointer_member_type::KEY, const pointer_member_type> {};

cxx_compiler::pointer_member_type::table_t cxx_compiler::pointer_member_type::table;

const cxx_compiler::pointer_member_type*
cxx_compiler::pointer_member_type::create(const tag* tg, const type* T)
{
  throw int();
  using namespace std;
  KEY key(tg,T);
  table_t::const_iterator p = table.find(key);
  if ( p != table.end() )
    return p->second;
  else
    return table[key] = new pointer_member_type(tg,T);
}

void
cxx_compiler::pointer_member_type::decl(std::ostream& os, std::string name) const
{
  name = m_tag->m_name + "::*" + name;
  m_T->decl(os,name);
}

void
cxx_compiler::pointer_member_type::encode(std::ostream& os) const
{
  os << 'M';
  const type* T = m_tag->m_types.first;
  T->encode(os);
  m_T->encode(os);
}

bool
cxx_compiler::pointer_member_type::compatible(const type* T) const
{
  if (this == T)
    return true;
  if (T->m_id != type::POINTER_MEMBER)
    return false;
  typedef const pointer_member_type PM;
  PM* that = static_cast<PM*>(T);
  if ( this->m_tag != that->m_tag )
    return false;
  return this->m_T->compatible(that->m_T);
}

const cxx_compiler::type*
cxx_compiler::pointer_member_type::composite(const type* T) const
{
  if (this == T)
    return this;
  if (T->m_id != type::POINTER_MEMBER)
    return 0;
  typedef const pointer_member_type PM;
  PM* that = static_cast<PM*>(T);
  if ( this->m_tag != that->m_tag )
    return 0;
  T = this->m_T->composite(that->m_T);
  return T ? create(m_tag, T) : 0;
}
