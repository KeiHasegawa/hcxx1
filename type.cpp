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
    sizeof(__int64);
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

int cxx_compiler::type::align() const
{
  switch (size()) {
  case 1: return 1;
  case 2: return 2;
  case 3:
  case 4: return 4;
  case 5: case 6: case 7: case 8: return 8;
  case 9: case 10: case 11: case 12: case 13:
  case 14: case 15: case 16:
  default: return 16;
  }
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

void cxx_compiler::type::destroy_tmp()
{
  const_type::destroy_tmp();
  volatile_type::destroy_tmp();
  restrict_type::destroy_tmp();
  func_type::destroy_tmp();
  array_type::destroy_tmp();
  pointer_type::destroy_tmp();
  incomplete_tagged_type::destroy_tmp();
  record_type::destroy_tmp();
  enum_type::destroy_tmp();
  varray_type::destroy_tmp();
  pointer_member_type::destroy_tmp();
}

void cxx_compiler::type::collect_tmp(std::vector<const type*>& vt)
{
  const_type::collect_tmp(vt);
  volatile_type::collect_tmp(vt);
  restrict_type::collect_tmp(vt);
  func_type::collect_tmp(vt);
  array_type::collect_tmp(vt);
  pointer_type::collect_tmp(vt);
  incomplete_tagged_type::collect_tmp(vt);
  record_type::collect_tmp(vt);
  enum_type::collect_tmp(vt);
  varray_type::collect_tmp(vt);
  pointer_member_type::collect_tmp(vt);
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

cxx_compiler::wchar_type cxx_compiler::wchar_type::obj;

void cxx_compiler::wchar_type::decl(std::ostream& os, std::string name) const
{
  os << "wchar_t";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::wchar_type::encode(std::ostream& os) const { os << 'w'; }

int cxx_compiler::wchar_type::size() const
{
  return generator::wchar::type->size();
}

const cxx_compiler::type* cxx_compiler::wchar_type::promotion() const
{
  return int_type::create();
}

bool cxx_compiler::wchar_type::_signed() const
{
  return generator::wchar::type->_signed();
}

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

cxx_compiler::int_type cxx_compiler::int_type::obj;

int cxx_compiler::int_type::size() const { return type_impl::sizeof_table[&obj]; }

void cxx_compiler::int_type::decl(std::ostream& os, std::string name) const
{
  os << "int";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::int_type::encode(std::ostream& os) const { os << 'i'; }

cxx_compiler::uint_type cxx_compiler::uint_type::obj;

int cxx_compiler::uint_type::size() const { return type_impl::sizeof_table[&obj]; }

void cxx_compiler::uint_type::decl(std::ostream& os, std::string name) const
{
  os << "unsigned int";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::uint_type::encode(std::ostream& os) const { os << 'j'; }

cxx_compiler::long_type cxx_compiler::long_type::obj;

int cxx_compiler::long_type::size() const { return type_impl::sizeof_table[&obj]; }

void cxx_compiler::long_type::decl(std::ostream& os, std::string name) const
{
  os << "long int";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::long_type::encode(std::ostream& os) const { os << 'l'; }

cxx_compiler::ulong_type cxx_compiler::ulong_type::obj;

int cxx_compiler::ulong_type::size() const { return type_impl::sizeof_table[&obj]; }

void cxx_compiler::ulong_type::decl(std::ostream& os, std::string name) const
{
  os << "unsigned long int";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::ulong_type::encode(std::ostream& os) const { os << 'm'; }

cxx_compiler::long_long_type cxx_compiler::long_long_type::obj;

int cxx_compiler::long_long_type::size() const { return type_impl::sizeof_table[&obj]; }

void cxx_compiler::long_long_type::decl(std::ostream& os, std::string name) const
{
  os << "long long int";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::long_long_type::encode(std::ostream& os) const { os << 'x'; }

cxx_compiler::ulong_long_type cxx_compiler::ulong_long_type::obj;

int cxx_compiler::ulong_long_type::size() const { return type_impl::sizeof_table[&obj]; }

void cxx_compiler::ulong_long_type::decl(std::ostream& os, std::string name) const
{
  os << "unsigned long long int";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::ulong_long_type::encode(std::ostream& os) const { os << 'y'; }

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

namespace cxx_compiler {
  const_type::table_t const_type::tmp_tbl;
  const_type::table_t const_type::pmt_tbl;
} // end of namespace cxx_compiler

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

bool cxx_compiler::const_type::compatible(const type* T) const
{
  if (this == T)
    return true;
  if (T->m_id != CONST)
    return false;
  typedef const const_type CT;
  CT* that = static_cast<CT*>(T);
  return this->m_T->compatible(that->m_T);
}

const cxx_compiler::type*
cxx_compiler::const_type::composite(const type* T) const
{
  if (this == T)
    return this;
  if (T->m_id != CONST)
    return 0;
  typedef const const_type CT;
  CT* that = static_cast<CT*>(T);
  T = this->m_T->composite(that->m_T);
  return T ? create(T) : 0;
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

const cxx_compiler::type* cxx_compiler::const_type::create(const type* T)
{
  if (T->m_id == CONST)
    return T;

  table_t& table = T->tmp() ? tmp_tbl : pmt_tbl;
  table_t::const_iterator p = table.find(T);
  if ( p != table.end() )
      return p->second;
  return table[T] = new const_type(T);
}

void cxx_compiler::const_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p.second;
  tmp_tbl.clear();
}

void cxx_compiler::const_type::collect_tmp(std::vector<const type*>& vs)
{
  for (auto p : tmp_tbl)
    vs.push_back(p.second);
  tmp_tbl.clear();
}

namespace cxx_compiler {
  volatile_type::table_t volatile_type::tmp_tbl;
  volatile_type::table_t volatile_type::pmt_tbl;
} // end of namespace cxx_compiler

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

bool cxx_compiler::volatile_type::compatible(const type* T) const
{
  if (this == T)
    return true;
  if (T->m_id != VOLATILE)
    return false;
  typedef const volatile_type VT;
  VT* that = static_cast<VT*>(T);
  return this->m_T->compatible(that->m_T);
}

const cxx_compiler::type*
cxx_compiler::volatile_type::composite(const type* T) const
{
  if (this == T)
    return this;
  if (T->m_id != VOLATILE)
    return 0;
  typedef const volatile_type VT;
  VT* that = static_cast<VT*>(T);
  T = this->m_T->composite(that->m_T);
  return T ? create(T) : 0;
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

const cxx_compiler::type* cxx_compiler::volatile_type::create(const type* T)
{
  if (T->m_id == VOLATILE)
    return T;

  table_t& table = T->tmp() ? tmp_tbl : pmt_tbl;
  table_t::const_iterator p = table.find(T);
  if (p != table.end())
      return p->second;
  
  if (T->m_id == CONST) {
    typedef const const_type CT;
    CT* ct = static_cast<CT*>(T);
    volatile_type* vt = new volatile_type(ct->m_T);
    table[ct->m_T] = vt;
    return const_type::create(vt);
  }

  volatile_type* ret = new volatile_type(T);
  return table[T] = ret;
}

void cxx_compiler::volatile_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p.second;
  tmp_tbl.clear();
}

void cxx_compiler::volatile_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : tmp_tbl)
    vt.push_back(p.second);
  tmp_tbl.clear();
}

namespace cxx_compiler {
  restrict_type::table_t restrict_type::tmp_tbl;
  restrict_type::table_t restrict_type::pmt_tbl;
} // end of namespace cxx_compiler

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

bool cxx_compiler::restrict_type::compatible(const type* T) const
{
  if (this == T)
    return true;
  if (T->m_id != RESTRICT)
    return false;
  typedef const restrict_type RT;
  RT* that = static_cast<RT*>(T);
  return this->m_T->compatible(that->m_T);
}

const cxx_compiler::type*
cxx_compiler::restrict_type::composite(const type* T) const
{
  if (this == T)
    return this;
  if (T->m_id != RESTRICT)
    return 0;
  typedef const restrict_type RT;
  RT* that = static_cast<RT*>(T);
  T = this->m_T->composite(that->m_T);
  return T ? create(T) : 0;
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

const cxx_compiler::type* cxx_compiler::restrict_type::create(const type* T)
{
  if (T->m_id == RESTRICT)
    return T;

  table_t& table = T->tmp() ? tmp_tbl : pmt_tbl;
  table_t::const_iterator p = table.find(T);
  if (p != table.end())
    return p->second;

  typedef const const_type CT;  
  typedef const volatile_type VT;
  
  if (T->m_id == CONST) {
    CT* ct = static_cast<CT*>(T);
    const type* T2 = ct->m_T;
    if (T2->m_id == VOLATILE) {
      VT* vt = static_cast<VT*>(T2);
      restrict_type* rt = new restrict_type(vt->m_T);
      table[vt->m_T] = rt;
      return const_type::create(volatile_type::create(rt));
    }
    restrict_type* rt = new restrict_type(T2);
    table[T2] = rt;
    return const_type::create(rt);
  }

  if (T->m_id == VOLATILE) {
    VT* vt = static_cast<VT*>(T);
    restrict_type* rt = new restrict_type(vt->m_T);
    table[vt->m_T] = rt;
    return volatile_type::create(rt);
  }

  restrict_type* ret = new restrict_type(T);  
  return table[T] = ret;
}

void cxx_compiler::restrict_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p.second;
  tmp_tbl.clear();
}

void cxx_compiler::restrict_type::collect_tmp(std::vector<const type*>& vs)
{
  for (auto p : tmp_tbl)
    vs.push_back(p.second);
  tmp_tbl.clear();
}

namespace cxx_compiler {
  using namespace std;
  struct func_type::table_t
  : map<pair<const type*, vector<const type*> >, const func_type*>
  {};
  func_type::table_t func_type::tmp_tbl;
  func_type::table_t func_type::pmt_tbl;
} // end of namespace cxx_compiler

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

const cxx_compiler::type* cxx_compiler::func_type::patch(const type* T, usr* u) const
{
  if ( T && m_T )
    T = m_T->patch(T,u);
  if ( T && T->m_id == type::FUNC ){
    using namespace error::declarations::declarators::function;
    of_function(parse::position,u);
    if (T->backpatch())
      T = backpatch_type::create();
    else
      T = int_type::create();
  }
  if ( T && T->m_id == type::ARRAY ){
    using namespace error::declarations::declarators::function;
    of_array(parse::position,u);
    if (T->backpatch())
      T = backpatch_type::create();
    else
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

bool cxx_compiler::func_type::tmp() const
{
  using namespace std;
  return m_T && m_T->tmp() || find_if(m_param.begin(),m_param.end(),mem_fun(&type::tmp)) != m_param.end();
}


bool cxx_compiler::func_type::variably_modified() const
{
  using namespace std;
  return m_T->variably_modified() || find_if(m_param.begin(), m_param.end(), mem_fun(&type::variably_modified)) != m_param.end();
}

const cxx_compiler::type* cxx_compiler::func_type::vla2a() const
{
  using namespace std;
  const type* T = m_T ? m_T->vla2a() : 0;
  vector<const type*> param;
  transform(m_param.begin(), m_param.end(), back_inserter(param), mem_fun(&type::vla2a));
  return create(T, param);
}

void cxx_compiler::func_type::decide_dim() const
{
  using namespace std;
  m_T->decide_dim();
  for_each(m_param.begin(), m_param.end(), mem_fun(&type::decide_dim));
}

bool cxx_compiler::func_type::overloadable(const func_type* that) const
{
  using namespace std;
  const vector<const type*>& u = this->m_param;
  const vector<const type*>& v = that->m_param;
  if ( u.size() != v.size() )
    return true;
  return mismatch(u.begin(),u.end(),v.begin(),cxx_compiler::compatible)
    != make_pair(u.end(),v.end());
}

const cxx_compiler::func_type*
cxx_compiler::func_type::create(const type* T,
                                const std::vector<const type*>& param)
{
  using namespace std;
  table_t& table = (T && T->tmp() || find_if(param.begin(), param.end(), mem_fun(&type::tmp)) != param.end()) ? tmp_tbl : pmt_tbl;
  pair<const type*, vector<const type*> > key(T,param);  
  table_t::const_iterator p = table.find(key);
  if ( p != table.end() )
    return p->second;
  else
    return table[key] = new func_type(T,param);
}

void cxx_compiler::func_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p.second;
  tmp_tbl.clear();
}

void cxx_compiler::func_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : tmp_tbl)
    vt.push_back(p.second);
  tmp_tbl.clear();
}

namespace cxx_compiler {
  using namespace std;
  struct array_type::table_t
    : map<pair<const type*,int>, const array_type*> {};
  array_type::table_t array_type::tmp_tbl;
  array_type::table_t array_type::pmt_tbl;
} // end of namespace cxx_compiler

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
      if ( cvr & 4 )
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
  using namespace expressions::primary::literal;
  var* dim = integer::create(m_dim);
  conversion::arithmetic::gen(&size, &dim);
  return size->mul(dim);
}

const cxx_compiler::array_type* cxx_compiler::array_type::create(const type* T, int dim)
{
  using namespace std;
  table_t& table = T->tmp() ? tmp_tbl : pmt_tbl;
  pair<const type*, int> key(T,dim);
  table_t::const_iterator p = table.find(key);
  if ( p != table.end() )
    return p->second;
  else
    return table[key] = new array_type(T,dim);
}


void cxx_compiler::array_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p.second;
  tmp_tbl.clear();
}

void cxx_compiler::array_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : tmp_tbl)
    vt.push_back(p.second);
  tmp_tbl.clear();
}

namespace cxx_compiler {
  pointer_type::table_t pointer_type::tmp_tbl;
  pointer_type::table_t pointer_type::pmt_tbl;
} // end of namespace cxx_compiler

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

int cxx_compiler::pointer_type::size() const
{
  return type_impl::pointer_sizeof;
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

const cxx_compiler::type* cxx_compiler::pointer_type::complete_type() const
{
  return create(m_T->complete_type());
}

const cxx_compiler::pointer_type* cxx_compiler::pointer_type::create(const type* T)
{
  table_t& table = T->tmp() ? tmp_tbl : pmt_tbl;
  table_t::const_iterator p = table.find(T);
  if ( p != table.end() )
    return p->second;
  else
    return table[T] = new pointer_type(T);
}

void cxx_compiler::pointer_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p.second;
  tmp_tbl.clear();
}

void cxx_compiler::pointer_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : tmp_tbl)
    vt.push_back(p.second);
  tmp_tbl.clear();
}

namespace cxx_compiler {
  reference_type::table_t reference_type::tmp_tbl;
  reference_type::table_t reference_type::pmt_tbl;
} // end of namespace cxx_compiler

int cxx_compiler::reference_type::size() const
{
  return type_impl::pointer_sizeof;
}

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

const cxx_compiler::type* cxx_compiler::reference_type::complete_type() const
{
  return create(m_T->complete_type());
}

const cxx_compiler::reference_type* cxx_compiler::reference_type::create(const type* T)
{
  table_t& table = T->tmp() ? tmp_tbl : pmt_tbl;
  table_t::const_iterator p = table.find(T);
  if ( p != table.end() )
    return p->second;
  else
    return table[T] = new reference_type(T);
}

void cxx_compiler::reference_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p.second;
  tmp_tbl.clear();
}

void cxx_compiler::reference_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : tmp_tbl)
    vt.push_back(p.second);
  tmp_tbl.clear();
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

namespace cxx_compiler {
  incomplete_tagged_type::table_t incomplete_tagged_type::tmp_tbl;
} // end of namespace cxx_compiler

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

bool cxx_compiler::incomplete_tagged_type::tmp() const
{
  return tmp_tbl.find(this) != tmp_tbl.end();
}


void cxx_compiler::incomplete_tagged_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p;
  tmp_tbl.clear();
}

void cxx_compiler::incomplete_tagged_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : tmp_tbl)
    vt.push_back(p);
  tmp_tbl.clear();
}

namespace cxx_compiler {
  bool inblock(const scope* ptr)
  {
    if (ptr->m_id == scope::BLOCK)
      return true;
    return ptr->m_parent ? inblock(ptr->m_parent) : false;
  }
  inline bool temporary(const tag* ptr)
  {
    return inblock(ptr);
  }
} // end of namespace cxx_compiler

const cxx_compiler::incomplete_tagged_type* cxx_compiler::incomplete_tagged_type::create(tag* ptr)
{
  typedef incomplete_tagged_type ITT;
  ITT* ret = new ITT(ptr);
  if (temporary(ptr))
    tmp_tbl.insert(ret);
  return ret;
}

namespace cxx_compiler {
  using namespace std;
  bool match_vf(pair<int, var*> p, usr* y)
  {
    var* v = p.second;
    addrof* a = v->addrof_cast();
    assert(a);
    v = a->m_ref;
    assert(v->usr_cast());
    usr* x = static_cast<usr*>(v);
    if (x->m_name != y->m_name)
      return false;
    return compatible(x->m_type, y->m_type);
  }
  namespace record_impl {
    struct layouter {
      insert_iterator<map<string, pair<int, usr*> > > X;
      insert_iterator<map<usr*, int> > Y;
      usr* m_last;
      struct current {
        usr* m_member;
        const type* m_integer;
        int m_position;
        current(usr* member = 0, const type* T = 0)
          : m_member(member), m_integer(T), m_position(0) {}
      } m_current;
      int operator()(int, usr*);
      layouter(insert_iterator<map<string, pair<int, usr*> > > XX,
               insert_iterator<map<usr*, int> > YY,
               usr* last)
        : X(XX), Y(YY), m_last(last) {}
    };
    struct grounder {
      insert_iterator<map<usr*, int> > Y;
      pair<string, pair<int, usr*> > operator()(usr*);
      grounder(insert_iterator<map<usr*, int> > YY) : Y(YY) {}
    };
    inline int add_size(int n, const type* T)
    {
      int m = T->size();
      assert(m);
      return n + m;
    }
    struct gather {
      vector<const record_type*>& m_common;
      const record_type* m_rec;
      gather(vector<const record_type*>& common, const record_type* rec)
	: m_common(common), m_rec(rec) {}
      void operator()(base* bp)
      {
	tag* ptr = bp->m_tag;
	const type* T = ptr->m_types.second;
	assert(T);
	assert(T->m_id == type::RECORD);
	typedef const record_type REC;
	REC* rec = static_cast<REC*>(T);
	if (rec == m_rec)
	  return;
	const vector<REC*>& x =   rec->virt_ancestor();
	const vector<REC*>& y = m_rec->virt_ancestor();
	vector<REC*> t;
	set_union(begin(x), end(x), begin(y), end(y), back_inserter(t));
	vector<REC*> t2;
	set_union(begin(m_common), end(m_common),
		  begin(t), end(t), back_inserter(t2));
	m_common = t2;
      }
    };
    struct base_layouter {
      map<base*, int>& m_base_offset;
      const vector<base*>& m_bases;
      vector<const record_type*>& m_common;
      base_layouter(map<base*, int>& bo, const vector<base*>& bases,
		    vector<const record_type*>& common)
	: m_base_offset(bo), m_bases(bases), m_common(common) {}
      int operator()(int n, base* b)
      {
        tag* ptr = b->m_tag;
        pair<const type*, const type*> x = ptr->m_types;
        const type* T = x.second;
        if (!T)
          error::not_implemented();
	assert(T->m_id == type::RECORD);
	typedef const record_type REC;
	REC* rec = static_cast<REC*>(T);
	vector<const record_type*> tmp;
	for_each(begin(m_bases), end(m_bases), gather(tmp, rec));
        int m = T->size();
        assert(m);
	m -= accumulate(begin(tmp), end(tmp), 0, add_size);
        assert(m > 0);
	m_base_offset[b] = n;
	vector<const record_type*> tmp2;
	set_union(begin(m_common), end(m_common),
		  begin(tmp), end(tmp), back_inserter(tmp2));
	m_common = tmp2;
        return n + m;
      }
    };
    struct add_common {
      map<const record_type*, int>& m_common_offset;
      add_common(map<const record_type*, int>& common_offset)
	: m_common_offset(common_offset) {}
      int operator()(int n, const record_type* rec)
      {
	m_common_offset[rec] = n;
	return add_size(n, rec);
      }
    };
    inline int base_vcommon(int n, const base* b, string vtbl_name)
    {
      tag* ptr = b->m_tag;
      const map<string, vector<usr*> >& usrs = ptr->m_usrs;
      typedef map<string, vector<usr*> >::const_iterator IT;
      IT p = usrs.find(vtbl_name);
      if (p == usrs.end())
        return n;
      const vector<usr*>& v = p->second;
      assert(v.size() == 1);
      usr* u = v.back();
      assert(u->m_flag & usr::WITH_INI);
      with_initial* vtbl = static_cast<with_initial*>(u);
      return n + vtbl->m_value.size();;
    }
    inline int base_vf(int n, const base* b)
    {
      return base_vcommon(n, b, vftbl_name);
    }
    struct base_vb {
      vector<const record_type*>& m_virt_ancestor;
      base_vb(vector<const record_type*>& v) : m_virt_ancestor(v) {}
      int operator()(int n, const base* bp)
      {
	tag* ptr = bp->m_tag;
	const type* T = ptr->m_types.second;
	assert(T);
	assert(T->m_id == type::RECORD);
	typedef const record_type REC;
	REC* rec = static_cast<REC*>(T);
	const vector<const record_type*>& va = rec->virt_ancestor();
	vector<const record_type*> tmp;
	set_union(begin(m_virt_ancestor), end(m_virt_ancestor), 
		  begin(va), end(va), back_inserter(tmp));
	m_virt_ancestor = tmp;
	return base_vcommon(n, bp, vbtbl_name);
      }
    };
    struct direct_virt {
      vector<const record_type*>& m_virt_ancestor;
      direct_virt(vector<const record_type*>& v) : m_virt_ancestor(v) {}
      bool operator()(const base* bp)
      {
	if (!bp->m_virtual)
	  return false;
	tag* ptr = bp->m_tag;
	const type* T = ptr->m_types.second;
	assert(T);
	assert(T->m_id == type::RECORD);
	typedef const record_type REC;
	REC* rec = static_cast<REC*>(T);
	m_virt_ancestor.push_back(rec);
	return true;
      }
    };
    struct copy_base_vf {
      map<int, var*>& m_value;
      map<base*, int>& m_vftbl_offset;
      copy_base_vf(map<int, var*>& value, map<base*, int>& vftbl_offset)
        : m_value(value), m_vftbl_offset(vftbl_offset) {}
      int operator()(int offset, base* b)
      {
        tag* ptr = b->m_tag;
        const map<string, vector<usr*> >& usrs = ptr->m_usrs;
        typedef map<string, vector<usr*> >::const_iterator IT;
        IT p = usrs.find(vftbl_name);
        if (p == usrs.end())
          return offset;
        m_vftbl_offset[b] = offset;
        const vector<usr*>& v = p->second;
        assert(v.size() == 1);
        usr* u = v.back();
        assert(u->m_flag & usr::WITH_INI);
        with_initial* w = static_cast<with_initial*>(u);
        const map<int, var*>& value = w->m_value;
        map<int, var*>& result = m_value;
        offset = accumulate(begin(value), end(value), offset,
                            [&result](int off, pair<int, var*> p)
                            {
			      var* v = p.second;
                              result[off] = v;
			      const type* T = void_type::create();
			      T = pointer_type::create(T);
                              return off + T->size();
                            });
        return offset;
      }
    };
    struct override_vf {
      map<int, var*>& m_value;
      override_vf(map<int, var*>& value) : m_value(value) {}
      void operator()(usr* u)
      {
        usr::flag_t flag = u->m_flag;
        if (!(flag & usr::FUNCTION))
          return;
        typedef map<int, var*>::iterator IT;
        IT p = find_if(begin(m_value), end(m_value),
                       bind2nd(ptr_fun(match_vf), u));
        if (p != end(m_value)) {
	  const type* T = u->m_type;
	  T = pointer_type::create(T);
          p->second = new addrof(T, u, 0);
	}
      }
    };
    struct own_vf {
      map<int, var*>& m_value;
      own_vf(map<int, var*>& value) : m_value(value) {}
      int operator()(int offset, usr* u)
      {
        if (!(u->m_flag & usr::VIRTUAL))
          return offset;
        assert(u->m_flag & usr::FUNCTION);
        const type* T = pointer_type::create(u->m_type);	
        m_value[offset] = new addrof(T, u, 0);
        return offset + T->size();
      }
    };
    bool comp_size(usr*, usr*);
    bool comp_align(usr*, usr*);
    bool member_modifiable(usr*);
    bool base_modifiable(base*);
    struct set_org_vbtbl_subr {
      map<int, var*>& m_value;
      int m_base_offset;
      map<const record_type*, int>& m_common_offset;
      set_org_vbtbl_subr(map<int, var*>& value,
			 int base_offset,
			 map<const record_type*, int>& common_offset)
	: m_value(value), m_base_offset(base_offset),
	  m_common_offset(common_offset) {}
      int operator()(int offset, const record_type* rec)
      {
        using namespace expressions::primary::literal;
	typedef map<const record_type*, int>::const_iterator IT;
	IT p = m_common_offset.find(rec);
	if (p == m_common_offset.end())
	  return offset;
	int common_offset = p->second;
	int n = common_offset - m_base_offset;
	assert(n > 0);
        var* v = integer::create(n);
	assert(m_value.find(offset) == m_value.end());
        m_value[offset] = v;
	return offset + v->m_type->size();
      }
    };
    struct set_org_vbtbl {
      map<int, var*>& m_value;
      const map<base*, int>& m_base_offset;
      map<const record_type*, int>& m_common_offset;
      map<base*, int>& m_vbtbl_offset;
      set_org_vbtbl(map<int, var*>& value,
		    const map<base*, int>& base_offset,
		    map<const record_type*, int>& common_offset,
		    map<base*, int>& vbtbl_offset)
	: m_value(value), m_base_offset(base_offset),
	  m_common_offset(common_offset), m_vbtbl_offset(vbtbl_offset) {}
      int operator()(int offset, base* bp)
      {
	typedef map<base*, int>::const_iterator IT;
	IT p = m_base_offset.find(bp);
	assert(p != m_base_offset.end());
	int base_offset = p->second;
	tag* ptr = bp->m_tag;
	const type* T = ptr->m_types.second;
	assert(T);
	assert(T->m_id == type::RECORD);
	typedef const record_type REC;
	REC* rec = static_cast<REC*>(T);
	const vector<REC*>& va = rec->virt_ancestor();
	if (va.empty())
	  return offset;
	m_vbtbl_offset[bp] = offset;
	return accumulate(begin(va), end(va), offset,
			  set_org_vbtbl_subr(m_value, base_offset,
					     m_common_offset));
      }
    };
    struct set_own_vbtbl {
      int m_offset;
      map<int, var*>& m_value;
      const map<base*, int>& m_base_offset;
      set_own_vbtbl(int offset,
		    map<int, var*>& value,
		    const map<base*, int>& base_offset)
        : m_offset(offset), m_value(value), m_base_offset(base_offset) {}
      void operator()(base* b)
      {
        using namespace expressions::primary::literal;
        if (!b->m_virtual)
          return;
        typedef map<base*, int>::const_iterator IT; 
        IT p = m_base_offset.find(b);
        assert(p != m_base_offset.end());
        int base_offset = p->second;
        var* v = integer::create(base_offset);
	assert(m_value.find(m_offset) == m_value.end());
        m_value[m_offset] = v;
        m_offset += v->m_type->size();
      }
    };
    inline usr* get_virt(tag* ptr, string virt_name)
    {
      const map<string, vector<usr*> >& usrs = ptr->m_usrs;
      typedef map<string, vector<usr*> >::const_iterator IT;
      IT p = usrs.find(virt_name);
      if (p == usrs.end())
	return 0;
      const vector<usr*>& v = p->second;
      assert(v.size() == 1);
      usr* u = v.back();
      return u;
    }
    inline usr* get_vfptr(tag* ptr)
    {
      return get_virt(ptr, vfptr_name);
    }
    inline usr* get_vbptr(tag* ptr)
    {
      return get_virt(ptr, vbptr_name);
    }
    inline usr* get_vftbl(tag* ptr)
    {
      return get_virt(ptr, vftbl_name);
    }
    inline usr* get_vbtbl(tag* ptr)
    {
      return get_virt(ptr, vbtbl_name);
    }
    struct call_ctor {
      const map<base*, int>& m_base_offset;
      usr* m_this;
      block* m_block;
      call_ctor(const map<base*, int>& base_offset, usr* this_ptr, block* b)
	: m_base_offset(base_offset), m_this(this_ptr), m_block(b) {}
      void operator()(base* pb)
      {
	tag* ptr = pb->m_tag;
	string tgn = ptr->m_name;
	map<string, vector<usr*> >& usrs = ptr->m_usrs;
	map<string, vector<usr*> >::const_iterator p = usrs.find(tgn);
	if (p == usrs.end())
	  return;
	const vector<usr*>& v = p->second;
	if (v.size() != 1)
	  error::not_implemented();
	usr* ctor = v.back();
	const type* T = ctor->m_type;
	assert(T->m_id == type::FUNC);
	typedef const func_type FT;
	FT* ft = static_cast<FT*>(T);
	const vector<const type*>& param = ft->param();
	if (param.size() != 1)
	  error::not_implemented();
	T = param.back();
	if (T->m_id != type::VOID)
	  error::not_implemented();
	T = ptr->m_types.second;
	assert(T->m_id == type::RECORD);
	T = pointer_type::create(T);
	var* tmp = new var(T);
	m_block->m_vars.push_back(tmp);
	code.push_back(new cast3ac(tmp, m_this, T));
	typedef map<base*, int>::const_iterator IT;
	IT q = m_base_offset.find(pb);
	assert(q != m_base_offset.end());
	if (int offset = q->second) {
	  using namespace expressions::primary::literal;
	  var* off = integer::create(offset);
	  code.push_back(new add3ac(tmp, tmp, off));
	}
	vector<var*> arg;
	call_impl::common(ft, ctor, &arg, false, tmp);
	usr::flag_t flag = ctor->m_flag;
        if (!error::counter && !cmdline::no_inline_sub) {
          if (flag & usr::INLINE) {
            using namespace declarations::declarators::function::definition;
            using namespace static_inline;
            skip::table_t::const_iterator p = skip::table.find(ctor);
            if (p != skip::table.end())
              substitute(code, code.size()-1, p->second);
          }
        }
      }
    };
    inline bool has_ctor(base* bp)
    {
      tag* ptr = bp->m_tag;
      string tgn = ptr->m_name;
      const map<string, vector<usr*> >& usrs = ptr->m_usrs;
      return usrs.find(tgn) != usrs.end();
    }
    inline bool bases_have_ctor(tag* ptr)
    {
      if (!ptr->m_bases)
	return false;
      vector<base*>& bases = *ptr->m_bases;
      typedef vector<base*>::const_iterator IT;
      IT p = find_if(begin(bases), end(bases), has_ctor);
      return p != end(bases);
    }
    struct update_vptr {
      const map<base*, int>& m_base_offset;
      const map<base*, int>& m_vtbl_offset;
      usr* m_this;
      block* m_block;
      usr* m_vtbl;
      string m_vptr_name;
      update_vptr(const map<base*, int>& base_offset,
		  const map<base*, int>& vtbl_offset,
		  usr* this_ptr, block* b, usr* vtbl,
		  string vptr_name)
	: m_base_offset(base_offset), m_vtbl_offset(vtbl_offset),
	  m_this(this_ptr), m_block(b), m_vtbl(vtbl), m_vptr_name(vptr_name) {}
      void operator()(base* bp)
      {
	using namespace expressions::primary::literal;
	tag* ptr = bp->m_tag;
	usr* vptr = get_virt(ptr, m_vptr_name);
	if (!vptr)
	  return;
	const type* T = ptr->m_types.second;
	assert(T->m_id == type::RECORD);
	typedef const record_type REC;
	REC* rec = static_cast<REC*>(T);
	vector<tag*> dummy;
	pair<int, usr*> off = rec->offset(m_vptr_name, dummy);
	int vptr_off = off.first;
	assert(vptr_off >= 0);
	map<base*, int>::const_iterator p = m_base_offset.find(bp);
	assert(p != m_base_offset.end());
	vptr_off += p->second;
	T = vptr->m_type;
	var* t0 = new var(T);
	m_block->m_vars.push_back(t0);
	code.push_back(new addr3ac(t0, m_vtbl));
	map<base*, int>::const_iterator q = m_vtbl_offset.find(bp);
	assert(q != m_vtbl_offset.end());
	int vtbl_off = q->second;
	if (vtbl_off) {
	  var* vo = integer::create(vtbl_off);
	  code.push_back(new add3ac(t0, t0, vo));
	}
	T = pointer_type::create(T);
	var* t1 = new var(T);
	m_block->m_vars.push_back(t1);
	code.push_back(new cast3ac(t1, m_this, T));
	if (vptr_off) {
	  var* vo = integer::create(vptr_off);
	  code.push_back(new add3ac(t1, t1, vo));
	}
	code.push_back(new invladdr3ac(t1, t0));
      }
    };
    void add_ctor(tag* ptr,
		  const map<string, pair<int, usr*> >& layout,
		  const map<base*, int>& base_offset,
		  const map<base*, int>& vbtbl_offset,
                  const map<base*, int>& vftbl_offset)
    {
      if (!bases_have_ctor(ptr) && !get_vbptr(ptr) && !get_vfptr(ptr))
	return;

      usr* ctor = 0;
      string tgn = ptr->m_name;
      map<string, vector<usr*> >& usrs = ptr->m_usrs;
      map<string, vector<usr*> >::const_iterator p = usrs.find(tgn);
      if (p == usrs.end()) {
	vector<const type*> param;
	param.push_back(void_type::create());
	const type* T = func_type::create(0,param);
	usr::flag_t flag =
	  usr::flag_t(usr::CTOR | usr::FUNCTION | usr::INLINE);
	ctor = new usr(tgn,T,flag,file_t());
	ptr->m_usrs[tgn].push_back(ctor);
      }
      else {
	const vector<usr*>& v = p->second;
	if (v.size() != 1)
	  error::not_implemented();
	ctor = v.back();
      }

      scope* param = new scope(scope::PARAM);
      using namespace class_or_namespace_name;
      assert(!before.empty());
      assert(before.back() == param);
      before.pop_back();
      param->m_parent = ptr;
      ptr->m_children.push_back(param);
      string name = "this";
      const type* T = ptr->m_types.first;
      T = pointer_type::create(T);
      usr* this_ptr = new usr(name,T,usr::NONE,file_t());
      this_ptr->m_scope = param;
      param->m_order.push_back(this_ptr);
      param->m_usrs[name].push_back(this_ptr);

      block* b = new block;
      assert(!before.empty());
      assert(before.back() == b);
      before.pop_back();
      b->m_parent = param;
      param->m_children.push_back(b);

      assert(code.empty());
      if (ptr->m_bases) {
	vector<base*>& bases = *ptr->m_bases;
	scope* org = scope::current;
	scope::current = b;
	for_each(begin(bases), end(bases),
		 call_ctor(base_offset, this_ptr, b));
	scope::current = org;
	if (usr* vbtbl = get_vbtbl(ptr)) {
	  for_each(begin(bases), end(bases),
		   update_vptr(base_offset, vbtbl_offset, this_ptr, b, vbtbl,
			       vbptr_name));
	}
	if (usr* vftbl = get_vftbl(ptr)) {
	  for_each(begin(bases), end(bases),
		   update_vptr(base_offset, vftbl_offset, this_ptr, b, vftbl,
			       vfptr_name));
	}
      }
      if (usr* vbptr = get_vbptr(ptr)) {
	const type* T = vbptr->m_type;
	var* t0 = new var(T);
	b->m_vars.push_back(t0);
	T = pointer_type::create(T);
	var* t1 = new var(T);
	b->m_vars.push_back(t1);
	usr* vbtbl = get_vbtbl(ptr);
	code.push_back(new addr3ac(t0, vbtbl));
	code.push_back(new cast3ac(t1, this_ptr, T));
	typedef map<string, pair<int, usr*> >::const_iterator IT;
	IT p = layout.find(vbptr_name);
	assert(p != layout.end());
	pair<int, usr*> off = p->second;
	if (int offset = off.first) {
	  using namespace expressions::primary::literal;
	  var* off = integer::create(offset);
	  code.push_back(new add3ac(t1,t1,off));
	}
	code.push_back(new invladdr3ac(t1, t0));
      }
      if (usr* vfptr = get_vfptr(ptr)) {
	const type* T = vfptr->m_type;
	var* t0 = new var(T);
	b->m_vars.push_back(t0);
	T = pointer_type::create(T);
	var* t1 = new var(T);
	b->m_vars.push_back(t1);
	usr* vftbl = get_vftbl(ptr);
	code.push_back(new addr3ac(t0, vftbl));
	code.push_back(new cast3ac(t1, this_ptr, T));
	typedef map<string, pair<int, usr*> >::const_iterator IT;
	IT p = layout.find(vfptr_name);
	assert(p != layout.end());
	pair<int, usr*> off = p->second;
	if (int offset = off.first) {
	  using namespace expressions::primary::literal;
	  var* off = integer::create(offset);
	  code.push_back(new add3ac(t1,t1,off));
	}
	code.push_back(new invladdr3ac(t1, t0));
      }
      fundef* fdef = new fundef(ctor,param);
      declarations::declarators::function::definition::action(fdef, code);
    }
    const usr::flag_t vtbl_flag
    = usr::flag_t(usr::WITH_INI| usr::STATIC | usr::STATIC_DEF);
    struct has {
      string m_name;
      const vector<tag*>& m_route;
      has(string name, const vector<tag*>& route)
	: m_name(name), m_route(route) {}
      bool operator()(base* bp)
      {
	tag* ptr = bp->m_tag;
	if (m_route.size() >= 2)
	  return ptr == m_route[1];
	return ptr->m_usrs.find(m_name) != ptr->m_usrs.end();
      }
    };
  } // end of namespace record_imp
} // end of namespace cxx_compiler

cxx_compiler::record_type::record_type(tag* ptr)
  : type(RECORD), m_size(0), m_modifiable(true), m_tag(ptr)
{
  using namespace std;
  using namespace record_impl;
  const vector<base*>* bases = m_tag->m_bases;
  int nbvf = 0;
  with_initial* vbtbl = 0;
  if (bases) {
    int nbvb = accumulate(begin(*bases), end(*bases), 0,
			  base_vb(m_virt_ancestor));
    int nvb = count_if(begin(*bases), end(*bases),
		       direct_virt(m_virt_ancestor));
    if (nbvb + nvb) {
      const type* T = int_type::create();
      T = const_type::create(T);
      T = array_type::create(T, nbvb + nvb);
      vbtbl = new with_initial(vbtbl_name,T,file_t());
      vbtbl->m_flag = vtbl_flag;
      map<string, vector<usr*> >& usrs = m_tag->m_usrs;
      usrs[vbtbl_name].push_back(vbtbl);
      if (nvb) {
	T = pointer_type::create(T);
	usr* vbptr = new usr(vbptr_name,T,usr::NONE,file_t());
	usrs[vbptr_name].push_back(vbptr);
	m_member.push_back(vbptr);
	m_layout[vbptr_name] = make_pair(m_size, vbptr);
	m_position[vbptr] = 0;
	m_size += T->size();
      }
    }
    nbvf = accumulate(begin(*bases), end(*bases), 0, base_vf);
  }
  const vector<usr*>& order = m_tag->m_order;
  int nvf = count_if(begin(order),end(order),
                     [](usr* u){ return u->m_flag & usr::VIRTUAL; });
  map<base*, int> vftbl_offset;
  if (nbvf + nvf) {
    const type* T = void_type::create();
    T = pointer_type::create(T);
    T = const_type::create(T);
    T = array_type::create(T, nbvf + nvf);
    with_initial* vftbl = new with_initial(vftbl_name, T, file_t());
    vftbl->m_flag = vtbl_flag;
    m_tag->m_usrs[vftbl_name].push_back(vftbl);
    if (nvf) {
      T = pointer_type::create(T);
      usr* vfptr = new usr(vfptr_name,T,usr::NONE,file_t());
      m_tag->m_usrs[vfptr_name].push_back(vfptr);
      m_member.push_back(vfptr);
      m_layout[vfptr_name] = make_pair(m_size, vfptr);
      m_position[vfptr] = m_size ? 1 : 0;
      m_size += T->size();
    }
    int offset = 0;
    if (bases) {
      offset = accumulate(begin(*bases), end(*bases), offset,
                          copy_base_vf(vftbl->m_value, vftbl_offset));
      for_each(begin(order), end(order), override_vf(vftbl->m_value));
    }
    accumulate(begin(order), end(order), offset, own_vf(vftbl->m_value));
  }
  map<base*, int> vbtbl_offset;
  if (bases) {
    map<const record_type*, int> common_offset;
    if (m_tag->m_kind != tag::UNION) {
      vector<const record_type*> common;
      m_size = accumulate(begin(*bases), end(*bases), m_size,
                          base_layouter(m_base_offset, *bases, common));
      m_size = accumulate(begin(common), end(common), m_size,
			  add_common(common_offset));
    }
    else
      error::not_implemented();
    if (vbtbl) {
      map<int, var*>& value = vbtbl->m_value;
      int offset = accumulate(begin(*bases), end(*bases), 0,
			      set_org_vbtbl(value, m_base_offset,
					    common_offset, vbtbl_offset));
      for_each(begin(*bases), end(*bases),
	       set_own_vbtbl(offset, value, m_base_offset));
    }
  }
  copy_if(begin(order),end(order),back_inserter(m_member),
          [](usr* u) {
            usr::flag_t flag = u->m_flag;
            usr::flag_t mask = usr::flag_t(usr::FUNCTION | usr::STATIC);
            return !(flag & mask);
          });
  if (!bases && m_member.empty()) {
    string name = ".dummy";
    usr* u = new usr(name, char_type::create(),usr::NONE,file_t());
    m_member.push_back(u);
    m_tag->m_usrs[name].push_back(u);
  }
  if (m_tag->m_kind != tag::UNION) {
    usr* last = m_member.empty() ? 0 : *m_member.rbegin();
    typedef vector<usr*>::iterator IT;
    IT beg = begin(m_member);
    if (beg != end(m_member) && (*beg)->m_name == vbptr_name)
      ++beg;
    if (beg != end(m_member) && (*beg)->m_name == vfptr_name)
      ++beg;
    m_size = accumulate(beg, end(m_member), m_size,
                        layouter(inserter(m_layout,m_layout.begin()),
                                 inserter(m_position,m_position.begin()),
                                 last));
    if (!m_member.empty()) {
      const type* T = m_member[0]->m_type;
      typedef const bit_field_type BF;
      if ( T->m_id == type::BIT_FIELD ){
        BF* bf = static_cast<BF*>(T);
        T = bf->integer_type();
      }
      T = m_member.back()->m_type;
      if ( T->m_id == type::BIT_FIELD ){
        BF* bf = static_cast<BF*>(T);
        T = bf->integer_type();
        m_size += T->size();
        usr::flag_t& flag = m_member.back()->m_flag;
        flag = usr::flag_t(flag | usr::MSB_FIELD);
      }
    }
  }
  else {
    if (!m_member.empty()) {
      transform(begin(m_member), end(m_member),
                inserter(m_layout,m_layout.begin()),
                grounder(inserter(m_position,m_position.begin())));
      {
        vector<usr*>::const_iterator p =
          max_element(begin(m_member),end(m_member),comp_size);
        assert(p != m_member.end());
        const type* T = (*p)->m_type;
        if ( T->m_id == type::BIT_FIELD ){
          typedef const bit_field_type BF;
          BF* bf = static_cast<BF*>(T);
          T = bf->integer_type();
        }
        m_size = T->size();
      }
      {
        vector<usr*>::const_iterator p =
          max_element(begin(m_member),end(m_member),comp_align);
        assert(p != m_member.end());
        const type* T = (*p)->m_type;
        if ( T->m_id == type::BIT_FIELD ){
          typedef const bit_field_type BF;
          BF* bf = static_cast<BF*>(T);
          T = bf->integer_type();
        }
      }
    }
  }
  if (bases) {
    typedef vector<base*>::const_iterator IT;
    IT p =
      find_if(begin(*bases), end(*bases), not1(ptr_fun(base_modifiable)));
    if (p != end(*bases))
      m_modifiable = false;
  }
  if (m_modifiable) {
    typedef vector<usr*>::const_iterator IT;
    IT p = find_if(begin(m_member), end(m_member),
                   not1(ptr_fun(member_modifiable)));
    if (p != end(m_member))
      m_modifiable = false;
  }
  int al = align();
  if ( int n = m_size % al ) {
    m_size += al - n;
  }
  add_ctor(m_tag, m_layout, m_base_offset, vbtbl_offset, vftbl_offset);
}

int cxx_compiler::record_impl::layouter::operator()(int offset, usr* member)
{
  using namespace std;
  if ( member->m_flag & usr::BIT_FIELD ){
    const type* T = member->m_type;
    assert(T->m_id == type::BIT_FIELD);
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
    if (T->variably_modified()){
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
      for_each(code.begin()+n,code.end(),[](tac* p){ delete p; });
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

bool cxx_compiler::record_impl::member_modifiable(usr* u)
{
  const type* T = u->m_type;
  return T->modifiable();
}

bool cxx_compiler::record_impl::base_modifiable(base* b)
{
  tag* ptr = b->m_tag;
  const type* T = ptr->m_types.second;
  assert(T);
  return T->modifiable();
}

namespace cxx_compiler {
  record_type::table_t record_type::tmp_tbl;
} // end of namespace cxx_compiler

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

std::pair<int, cxx_compiler::usr*> cxx_compiler::
record_type::offset(std::string name, const std::vector<tag*>& route) const
{
  using namespace std;
  using namespace record_impl;
  if (!route.empty()) {
    tag* ptr = route[0];
    assert(ptr == m_tag);
  }
  map<string, pair<int, usr*> >::const_iterator p = m_layout.find(name);
  if (p != m_layout.end())
    return p->second;
  vector<base*>* bases = m_tag->m_bases;
  if (!bases)
    return make_pair(-1,static_cast<usr*>(0));    

  typedef vector<base*>::const_iterator ITx;
  ITx q = find_if(begin(*bases), end(*bases), has(name, route));
  if (q == end(*bases))
    return make_pair(-1,static_cast<usr*>(0));    

  base* b = *q;
  typedef map<base*, int>::const_iterator ITy;
  ITy it = m_base_offset.find(b);
  assert(it != m_base_offset.end());
  int bo = it->second;
  tag* ptr = b->m_tag;
  const type* T = ptr->m_types.second;
  assert(T->m_id == type::RECORD);
  typedef const record_type REC;
  REC* rec = static_cast<REC*>(T);
  vector<tag*> route2;
  if (!route.empty())
    copy(begin(route)+1, end(route), back_inserter(route2));
  pair<int, usr*> ret = rec->offset(name, route2);
  assert(ret.first >= 0);
  ret.first += bo;
  return ret;
}

namespace cxx_compiler {
  namespace record_impl {
    using namespace std;
    struct cmp_base {
      tag* m_tag;
      const vector<tag*>& m_route;
      cmp_base(tag* ptr, const vector<tag*>& route)
	: m_tag(ptr), m_route(route) {}
      bool operator()(base* pb)
      {
	tag* xtag = pb->m_tag;
	if (m_route.size() >= 2)
	  return xtag == m_route[1];
	if (xtag == m_tag)
	  return true;
	const type* Tx = xtag->m_types.second;
	assert(Tx->m_id == type::RECORD);
	typedef const record_type REC;
	REC* Rx = static_cast<REC*>(Tx);
	const type* Ty = m_tag->m_types.second;
	assert(Ty->m_id == type::RECORD);
	REC* Ry = static_cast<REC*>(Ty);
	vector<tag*> dummy;
	int offset = Rx->base_offset(Ry, dummy);
	return offset >= 0;
      }
    };
  }  // end of namespace record_impl
}  // end of namespace cxx_compiler

int
cxx_compiler::record_type::base_offset(const record_type* that,
				       const std::vector<tag*>& route) const
{
  using namespace record_impl;
  if (this == that)
    return 0;
  tag* xtag = this->m_tag;
  tag* ytag = that->m_tag;
  if (!xtag->m_bases)
    return -1;
  const vector<base*>& bases = *xtag->m_bases;
  typedef vector<base*>::const_iterator IT;
  IT p = find_if(begin(bases), end(bases), cmp_base(ytag, route));
  if (p == end(bases))
    return -1;
  base* b = *p;
  map<base*, int>::const_iterator q = m_base_offset.find(b);
  assert(q != m_base_offset.end());
  tag* btag = b->m_tag;
  const type* Tb = btag->m_types.second;
  assert(Tb->m_id == type::RECORD);
  typedef const record_type REC;
  REC* Rb = static_cast<REC*>(Tb);
  vector<tag*> route2;
  if (!route.empty())
    copy(begin(route)+1, end(route), back_inserter(route2));
  return Rb->base_offset(that, route2) + q->second;
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
  tag* ptr = get_tag();
  if ( ptr->m_kind == tag::UNION && nth >= 1 )
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

bool cxx_compiler::record_type::tmp() const
{
  return tmp_tbl.find(this) != tmp_tbl.end();
}

const cxx_compiler::record_type*
cxx_compiler::record_type::create(tag* ptr)
{
  record_type* ret = new record_type(ptr);
  if (temporary(ptr))
    tmp_tbl.insert(ret);
  return ret;
}

void cxx_compiler::record_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p;
  tmp_tbl.clear();
}

void cxx_compiler::record_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : tmp_tbl)
    vt.push_back(p);
  tmp_tbl.clear();
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

namespace cxx_compiler {
  enum_type::table_t enum_type::tmp_tbl;
} // end of namespace cxx_compiler

const cxx_compiler::enum_type* cxx_compiler::enum_type::create(tag* ptr, const type* integer)
{
  enum_type* ret = new enum_type(ptr, integer);
  if (temporary(ptr))
    tmp_tbl.insert(ret);
  return ret;
}

bool cxx_compiler::enum_type::tmp() const
{
  return tmp_tbl.find(this) != tmp_tbl.end();
}

void cxx_compiler::enum_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p;
  tmp_tbl.clear();
}

void cxx_compiler::enum_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : tmp_tbl)
    vt.push_back(p);
  tmp_tbl.clear();
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

namespace cxx_compiler {
  using namespace std;
  struct varray_type::table_t
    : map<pair<const type*, var*>, const varray_type*> {};
  varray_type::table_t varray_type::table;
} // end of namespace cxx_compiler

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

void cxx_compiler::varray_type::post(std::ostream& os) const
{
  os << '[';
  os << ']';
  m_T->post(os);
}

const cxx_compiler::type* cxx_compiler::varray_type::patch(const type* T, usr* u) const
{
  T = m_T->patch(T,u);
  if (T->m_id == type::FUNC) {
    using namespace error::declarations::declarators::array;
    of_function(parse::position,u);
    T = backpatch_type::create();
  }
  T = T->complete_type();
  if (u) {
    usr::flag_t& flag = u->m_flag;
    flag = usr::flag_t(flag | usr::VL);
  }
  return create(T,m_dim);
}

const cxx_compiler::type* cxx_compiler::varray_type::qualified(int cvr) const
{
  return create(m_T->qualified(cvr),m_dim);
}

const cxx_compiler::type* cxx_compiler::varray_type::complete_type() const
{
  return create(m_T->complete_type(),m_dim);
}

const cxx_compiler::pointer_type* cxx_compiler::varray_type::ptr_gen() const
{
  return pointer_type::create(m_T);
}

cxx_compiler::var* cxx_compiler::varray_type::vsize() const
{
  using namespace std;
  const type* T = m_T->complete_type();
  var* dim = m_dim;
  if ( var* vs = T->vsize() ) {
    conversion::arithmetic::gen(&dim, &vs);
    return dim->mul(vs);
  }

  int n = T->size();
  var* size = expressions::primary::literal::integer::create(n);
  conversion::arithmetic::gen(&dim, &size);
  return dim->mul(size);
}

void cxx_compiler::varray_type::decide_dim() const
{
  using namespace std;
  using namespace declarations::declarators::array;
  map<var*, vector<tac*> >::iterator p =
    variable_length::dim_code.find(m_dim);
  if (p != variable_length::dim_code.end()) {
    vector<tac*>& vc = p->second;
    copy(vc.begin(), vc.end(), back_inserter(code));
    variable_length::dim_code.erase(p);
  }
  m_T->decide_dim();
}

const cxx_compiler::varray_type* cxx_compiler::varray_type::create(const type* T, var* dim)
{
  using namespace std;
  pair<const type*, var*> key(T,dim);
  table_t::const_iterator p = table.find(key);
  if ( p != table.end() )
    return p->second;
  else
    return table[key] = new varray_type(T,dim);
}

void cxx_compiler::varray_type::destroy_tmp()
{
  for (auto p : table)
    delete p.second;
  table.clear();
}

void cxx_compiler::varray_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : table)
    vt.push_back(p.second);
  table.clear();
}

namespace cxx_compiler {
  pointer_member_type::table_t pointer_member_type::tmp_tbl;
  pointer_member_type::table_t pointer_member_type::pmt_tbl;
} // end of namespace cxx_compiler

const cxx_compiler::pointer_member_type*
cxx_compiler::pointer_member_type::create(const tag* ptr, const type* T)
{
  using namespace std;
  pair<const tag*, const type*> key(ptr,T);  
  table_t& table = (temporary(ptr) || T->tmp()) ? tmp_tbl : pmt_tbl;
  table_t::const_iterator p = table.find(key);
  if ( p != table.end() )
      return p->second;
  return table[key] = new pointer_member_type(ptr, T);
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

const cxx_compiler::type*
cxx_compiler::pointer_member_type::patch(const type* T, usr* u) const
{
  T = m_T->patch(T, u);
  return create(m_tag, T);
}

void cxx_compiler::pointer_member_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p.second;
  tmp_tbl.clear();
}

void
cxx_compiler::pointer_member_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : tmp_tbl)
    vt.push_back(p.second);
  tmp_tbl.clear();
}
