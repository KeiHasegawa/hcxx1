#pragma warning ( disable : 4503 )
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "cxx_y.h"

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
#ifdef __GNUC__
  (*this)[float128_type::create()] = sizeof(__float128);
#else // __GNUC__
  (*this)[float128_type::create()] = 64;
#endif // __GNUC__
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
  sizeof_table[float128_type::create()] = size((int)type::FLOAT128);
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

bool cxx_compiler::type::template_match(const type* that, bool) const
{
  that = that->unqualified();
  return this == that;
}

bool cxx_compiler::type::comp(const type* that, int* res) const
{
  if (this == that)
    return true;
  bool b = that->comp(this, res);
  assert(!b);
  assert(*res == 1);
  *res = -1;
  return b;
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

cxx_compiler::char16_type cxx_compiler::char16_type::obj;

void cxx_compiler::char16_type::decl(std::ostream& os, std::string name) const
{
  os << "char16_t";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::char16_type::encode(std::ostream& os) const { os << 'w'; }

int cxx_compiler::char16_type::size() const
{
  return 2;
}

const cxx_compiler::type* cxx_compiler::char16_type::promotion() const
{
  return int_type::create();
}

bool cxx_compiler::char16_type::_signed() const
{
  return true;
}

cxx_compiler::char32_type cxx_compiler::char32_type::obj;

void cxx_compiler::char32_type::decl(std::ostream& os, std::string name) const
{
  os << "char32_t";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::char32_type::encode(std::ostream& os) const { os << 'w'; }

int cxx_compiler::char32_type::size() const
{
  return 4;
}

const cxx_compiler::type* cxx_compiler::char32_type::promotion() const
{
  return int_type::create();
}

bool cxx_compiler::char32_type::_signed() const
{
  return true;
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

cxx_compiler::float128_type cxx_compiler::float128_type::obj;

int cxx_compiler::float128_type::size() const
{
  return type_impl::sizeof_table[&obj];
}

void
cxx_compiler::float128_type::decl(std::ostream& os, std::string name) const
{
  os << "__float128";
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::float128_type::encode(std::ostream& os) const
{
  os << "f128";
}

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

bool cxx_compiler::const_type::template_match(const type* Ty, bool) const
{
  Ty = Ty->unqualified();
  return m_T->template_match(Ty, true);
}

bool cxx_compiler::const_type::comp(const type* Ty, int* res) const
{
  if (Ty->m_id != type::CONST) {
    *res = 1;
    return false;
  }

  typedef const const_type CT;
  CT* ct = static_cast<CT*>(Ty);
  Ty = ct->m_T;
  return m_T->comp(Ty, res);
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

bool cxx_compiler::volatile_type::template_match(const type* Ty, bool) const
{
  Ty = Ty->unqualified();
  return m_T->template_match(Ty, true);
}

bool cxx_compiler::volatile_type::comp(const type* Ty, int* res) const
{
  if (Ty->m_id != type::VOLATILE) {
    *res = 1;
    return false;
  }

  typedef const volatile_type VT;
  VT* vt = static_cast<VT*>(Ty);
  Ty = vt->m_T;
  return m_T->comp(Ty, res);
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

bool cxx_compiler::restrict_type::template_match(const type* Ty, bool) const
{
  Ty = Ty->unqualified();
  return m_T->template_match(Ty, true);
}

bool cxx_compiler::restrict_type::comp(const type* Ty, int* res) const
{
  if (Ty->m_id != type::RESTRICT) {
    *res = 1;
    return false;
  }

  typedef const restrict_type RT;
  RT* rt = static_cast<RT*>(Ty);
  Ty = rt->m_T;
  return m_T->comp(Ty, res);
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
  else {
    if (cvr & 1)
      os << "const ";
    if (cvr & 2)
      os << "volatile ";
    if (cvr & 4)
      os << "restrict ";
  }
  T->decl(os,name);
  post(os);
}

void cxx_compiler::func_type::encode(std::ostream& os) const
{
  using namespace std;
  os << 'F';
  if (m_T)
    m_T->encode(os);
  for (auto T : m_param)
    T->encode(os);
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
  if (this == T)
    return true;
  if (T->m_id != type::FUNC)
    return false;
  typedef const func_type FT;
  FT* that = static_cast<FT*>(T);
  if (this->m_T) {
    if (!this->m_T->compatible(that->m_T))
      return false;
  }
  const vector<const type*>& u = this->m_param;
  const vector<const type*>& v = that->m_param;
  if ( u.size() != v.size() )
    return false;
  return mismatch(u.begin(),u.end(),v.begin(),cxx_compiler::compatible) == make_pair(u.end(),v.end());
}

const cxx_compiler::type*
cxx_compiler::func_type::composite(const type* T) const
{
  using namespace std;
  if ( this == T )
    return this;
  if ( T->m_id != type::FUNC )
    return 0;
  typedef const func_type FT;
  FT* that = static_cast<FT*>(T);
  if (this->m_T) {
    if (!this->m_T->compatible(that->m_T))
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

const cxx_compiler::type*
cxx_compiler::func_type::patch(const type* T, usr* u) const
{
  if ( T && m_T )
    T = m_T->patch(T,u);
  if (T && T->m_id == type::FUNC) {
    using namespace error::declarations::declarators::function;
    of_function(parse::position, u);
    if (T->backpatch())
      T = backpatch_type::create();
    else
      T = int_type::create();
  }
  if (T && T->m_id == type::ARRAY) {
    using namespace error::declarations::declarators::function;
    of_array(parse::position, u);
    if (T->backpatch())
      T = backpatch_type::create();
    else
      T = int_type::create();
  }
  if (u)
    u->m_flag = usr::flag_t(u->m_flag | usr::FUNCTION);
  return create(T, m_param);
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

const cxx_compiler::type*
cxx_compiler::func_type::instantiate() const
{
  const type* T = m_T->instantiate();
  vector<const type*> param;
  transform(begin(m_param), end(m_param), back_inserter(param),
	    mem_fun(&type::instantiate));
  return create(T, param);
}

bool
cxx_compiler::func_type::template_match(const type* Ty, bool) const
{
  if (Ty->m_id != type::FUNC)
    return false;
  typedef const func_type FT;
  FT* yft = static_cast<FT*>(Ty);
  const vector<const type*>& yparam = yft->m_param;
  if (m_param.size() != yparam.size())
    return false;
  auto ret = mismatch(begin(m_param), end(m_param),
		      begin(yparam), cxx_compiler::template_match);
  if (ret.first != end(m_param))
    return false;
  Ty = yft->m_T;
  return m_T->template_match(Ty, true);
}

bool cxx_compiler::func_type::comp(const type* Ty, int* res) const
{
  error::not_implemented();
  return false;
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
  else {
    if (cvr & 1)
      os << "const ";
    if (cvr & 2)
      os << "volatile ";
    if (cvr & 4)
      os << "restrict ";
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

bool
cxx_compiler::array_type::template_match(const type* Ty, bool) const
{
  if (Ty->m_id != type::ARRAY)
    return false;
  typedef const array_type AT;
  AT* yat = static_cast<AT*>(Ty);
  int dy = yat->m_dim;
  if (m_dim != dy)
    return false;
  Ty = yat->m_T;
  return m_T->template_match(Ty, true);
}

bool cxx_compiler::array_type::comp(const type* Ty, int* res) const
{
  if (Ty->m_id != type::ARRAY) {
    *res = 1;
    return false;
  }
  typedef const array_type AT;
  AT* yat = static_cast<AT*>(Ty);
  int dy = yat->m_dim;
  assert(m_dim == dy);
  Ty = yat->m_T;
  return m_T->comp(Ty, res);
}

const cxx_compiler::array_type*
cxx_compiler::array_type::create(const type* T, int dim)
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
  if (u)
    u->m_flag = (usr::flag_t)(u->m_flag & ~usr::FUNCTION & ~usr::VL);
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

bool
cxx_compiler::pointer_type::template_match(const type* Ty, bool) const
{
  typedef const pointer_type PT;
  if (Ty->m_id != type::POINTER)
    return false;
  PT* ypt = static_cast<PT*>(Ty);
  Ty = ypt->m_T;
  return m_T->template_match(Ty, false);
}

bool cxx_compiler::pointer_type::comp(const type* Ty, int* res) const
{
  typedef const pointer_type PT;
  if (Ty->m_id != type::POINTER) {
    *res = 1;
    return false;
  }
  PT* ypt = static_cast<PT*>(Ty);
  Ty = ypt->m_T;
  return m_T->comp(Ty, res);
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
  if ( T == m_T ) {
    tmp << '&';
    if (m_twice)
      tmp << '&';
    tmp << name;
  }
  else {
    tmp << '(' << '&';
    if (m_twice)
      tmp << '&';
    tmp << name << ')';
  }
  m_T->decl(os,tmp.str());
}

void cxx_compiler::reference_type::encode(std::ostream& os) const
{
  os << 'R';
  m_T->encode(os);
}

const cxx_compiler::type*
cxx_compiler::reference_type::patch(const type* T, usr* u) const
{
  T = m_T->patch(T,u);
  if (u)
    u->m_flag = (usr::flag_t)(u->m_flag & ~usr::FUNCTION & ~usr::VL);
  return create(T, m_twice);
}

bool cxx_compiler::reference_type::compatible(const type* T) const
{
  if (this == T)
    return true;
  if (T->m_id != type::REFERENCE)
    return false;
  typedef const reference_type REF;
  REF* that = static_cast<REF*>(T);
  if (this->m_twice != that->m_twice)
    return false;
  return this->m_T->compatible(that->m_T);
}

const cxx_compiler::type*
cxx_compiler::reference_type::composite(const type* T) const
{
  if (this == T)
    return this;
  if (T->m_id != type::REFERENCE)
    return 0;
  typedef const reference_type REF;
  REF* that = static_cast<REF*>(T);
  T = this->m_T->composite(that->m_T);
  return T ? create(T, m_twice) : 0;
}

const cxx_compiler::type* cxx_compiler::reference_type::complete_type() const
{
  return create(m_T->complete_type(), m_twice);
}

bool
cxx_compiler::reference_type::template_match(const type* Ty, bool) const
{
  typedef const reference_type RT;
  if (Ty->m_id == type::REFERENCE) {
    RT* rt = static_cast<RT*>(Ty);
    Ty = rt->m_T;
  }
  return m_T->template_match(Ty, false);
}

bool cxx_compiler::reference_type::comp(const type* Ty, int* res) const
{
  if (Ty->m_id != type::REFERENCE) {
    *res = 1;
    return false;
  }
  typedef const reference_type RT;
  RT* rt = static_cast<RT*>(Ty);
  Ty = rt->m_T;
  return m_T->comp(Ty, res);
}

const cxx_compiler::reference_type*
cxx_compiler::reference_type::create(const type* T, bool twice)
{
  table_t& table = T->tmp() ? tmp_tbl : pmt_tbl;
  pair<const type*, bool> key(T, twice);
  table_t::const_iterator p = table.find(key);
  if ( p != table.end() )
    return p->second;
  else
    return table[key] = new reference_type(T, twice);
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

void cxx_compiler::
incomplete_tagged_type::decl(std::ostream& os, std::string name) const
{
  record_impl::decl(os, name, m_tag, false);
}

void cxx_compiler::incomplete_tagged_type::encode(std::ostream& os) const
{
  record_impl::encode(os, m_tag);
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

int cxx_compiler::incomplete_tagged_type::complexity() const
{
  return record_impl::complexity(m_tag);
}

const cxx_compiler::type*
cxx_compiler::incomplete_tagged_type::instantiate() const
{
  const tag* res = record_impl::instantiate(m_tag);
  const type* T = res->m_types.second;
  if (T)
    return T;
  return res->m_types.first;
}

bool cxx_compiler::
incomplete_tagged_type::template_match(const type* Ty, bool) const
{
  if (Ty->m_id == type::REFERENCE) {
    typedef const reference_type RT;
    RT* rt = static_cast<RT*>(Ty);
    Ty = rt->referenced_type();
  }
  Ty = Ty->unqualified();
  type::id_t id = Ty->m_id;
  if (id != type::RECORD && id != type::INCOMPLETE_TAGGED) {
    tag::kind_t kind = m_tag->m_kind;
    if (kind == tag::TYPENAME)
      return true;
    return false;
  }
  tag* ytag = Ty->get_tag();
  return record_impl::template_match_impl::common(m_tag, ytag);
}

bool
cxx_compiler::incomplete_tagged_type::comp(const type* Ty, int* res) const
{
  tag* py = Ty->get_tag();
  if (!py) {
    *res = 1;
    return false;
  }
  return record_impl::comp_impl::common(m_tag, py, res);
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
    assert(ptr != ptr->m_parent);
    return ptr->m_parent ? inblock(ptr->m_parent) : false;
  }
  bool temporary(const tag* ptr)
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

void cxx_compiler::enum_type::decl(std::ostream& os, std::string name) const
{
  record_impl::decl(os, name, m_tag, false);
}

void cxx_compiler::enum_type::encode(std::ostream& os) const
{
  record_impl::encode(os, m_tag);
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

int cxx_compiler::enum_type::complexity() const
{
  return record_impl::complexity(m_tag);
}

const cxx_compiler::type*
cxx_compiler::enum_type::instantiate() const
{
  const tag* res = record_impl::instantiate(m_tag);
  const type* T = res->m_types.second;
  assert(T);
  return T;
}

bool 
cxx_compiler::enum_type::template_match(const type*, bool) const
{
  error::not_implemented();
  return false;
}

bool
cxx_compiler::enum_type::comp(const type* Ty, int* res) const
{
  tag* py = Ty->get_tag();
  if (!py) {
    *res = 1;
    return false;
  }
  return record_impl::comp_impl::common(m_tag, py, res);
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
  u->m_flag = usr::flag_t(u->m_flag | usr::BIT_FIELD);
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
  if (u)
    u->m_flag = usr::flag_t(u->m_flag | usr::VL);
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

bool cxx_compiler::varray_type::tmp() const
{
  using namespace std;
  pair<const type*, var*> key(m_T,m_dim);
  auto p = table.find(key);
  return p != table.end();
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

const cxx_compiler::type*
cxx_compiler::varray_type::vla2a() const
{
  if (!tmp())
    return this;
  return array_type::create(m_T->vla2a(), 0);
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

const cxx_compiler::type*
cxx_compiler::varray_type::instantiate() const
{
  const type* T = m_T->instantiate();
  assert(m_dim->usr_cast());
  usr* u = static_cast<usr*>(m_dim);
  string name = u->m_name;
  assert(!sweeper_f_impl::tables.empty());
  auto bk = sweeper_f_impl::tables.back();
  auto p = bk->find(name);
  assert(p != bk->end());
  auto& v = p->second;
  auto y = v.second;
  assert(y);
  var* dim = y->second;
  assert(dim);
  assert(dim->usr_cast());
  usr* u2 = static_cast<usr*>(dim);
  assert(u2->isconstant());
  int n = u2->value();
  return array_type::create(T, n);
}

namespace cxx_compiler {
  namespace varray_type_impl {
    bool array_case(const varray_type* xvat, const array_type* yat)
    {
      using namespace expressions::primary::literal;
      var* dx = xvat->dim();
      assert(dx->usr_cast());
      usr* u = static_cast<usr*>(dx);
      assert(u->m_flag2 & usr::TEMPL_PARAM);
      string name = u->m_name;
      assert(!sweeper_f_impl::tables.empty());
      auto bk = sweeper_f_impl::tables.back();
      auto p = bk->find(name);
      assert(p != bk->end());
      auto& v = p->second;
      int dy = yat->dim();
      if (v.second->second) {
	if (v.second->second != integer::create(dy))
	  return false;
      }
      else
	v.second->second = integer::create(dy);
      const type* Tx = xvat->element_type();
      const type* Ty = yat->element_type();
      return Tx->template_match(Ty, true);
    }
  } // /end of namesapce varray_type_impl
} // end of namespace cxx_compiler

bool
cxx_compiler::varray_type::template_match(const type* Ty, bool) const
{
  if (Ty->m_id == type::ARRAY) {
    typedef const array_type AT;
    AT* yat = static_cast<AT*>(Ty);
    return varray_type_impl::array_case(this, yat);
  }
  if (Ty->m_id != type::VARRAY)
    return false;
  typedef const varray_type VAT;
  VAT* yvat = static_cast<VAT*>(Ty);
  assert(m_dim->usr_cast());
  usr* ux = static_cast<usr*>(m_dim);
  assert(ux->m_flag2 & usr::TEMPL_PARAM);
  var* dy = yvat->m_dim;
  assert(dy->usr_cast());
  usr* uy = static_cast<usr*>(dy);
  assert(uy->m_flag2 & usr::TEMPL_PARAM);

  string nx = ux->m_name;
  assert(!sweeper_f_impl::tables.empty());
  auto bk = sweeper_f_impl::tables.back();
  auto p = bk->find(nx);
  assert(p != bk->end());
  auto& v = p->second;
  if (v.second->second) {
    if (v.second->second != uy)
      return false;
  }
  else
    v.second->second = uy;

  Ty = yvat->m_T;
  return m_T->template_match(Ty, true);
}

bool
cxx_compiler::varray_type::comp(const type* Ty, int* res) const
{
  error::not_implemented();
  return false;
}

const cxx_compiler::varray_type*
cxx_compiler::varray_type::create(const type* T, var* dim)
{
  using namespace std;
  pair<const type*, var*> key(T,dim);
  table_t::const_iterator p = table.find(key);
  if ( p != table.end() )
    return p->second;
  varray_type* ret = new varray_type(T,dim);
  if (usr* u = dim->usr_cast()) {
    usr::flag2_t flag2 = u->m_flag2;
    if (flag2 & usr::TEMPL_PARAM)
      return ret;
  }
  return table[key] = ret;
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

int cxx_compiler::pointer_member_type::complexity() const
{
  int n = m_T->complexity();
  if (n)
    return n + 1;
  const type* T = m_tag->m_types.first;
  int m = T->complexity();
  return m ? m + 1 : 0;
}

const cxx_compiler::type*
cxx_compiler::pointer_member_type::instantiate() const
{
  const type* T = m_tag->m_types.second;
  assert(T);
  tag* ptr = T->get_tag();
  assert(ptr);
  return create(ptr, m_T->instantiate());
}

bool
cxx_compiler::pointer_member_type::template_match(const type* Ty, bool) const
{
  Ty = Ty->unqualified();
  if (Ty->m_id != type::POINTER_MEMBER)
    return false;
  typedef const pointer_member_type PM;
  PM* pmy = static_cast<PM*>(Ty);
  Ty = pmy->m_T;
  if (!m_T->template_match(Ty, true))
    return false;
  const type* T = m_tag->m_types.first;
  const tag* ytag = pmy->m_tag;
  Ty = ytag->m_types.second;
  if (!Ty)
    Ty = ytag->m_types.first;
  return T->template_match(Ty, true);
}

bool
cxx_compiler::pointer_member_type::comp(const type* Ty, int* res) const
{
  if (Ty->m_id != type::POINTER_MEMBER) {
    *res = 1;
    return false;
  }
  typedef const pointer_member_type PM;
  PM* pmy = static_cast<PM*>(Ty);
  Ty = pmy->m_T;
  if (!m_T->comp(Ty, res))
    return false;
  return record_impl::comp_impl::common(m_tag, pmy->m_tag, res);
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
cxx_compiler::pointer_member_type::integer() const
{
  return m_T->m_id != type::FUNC;
}

bool
cxx_compiler::pointer_member_type::scalar() const
{
  return m_T->m_id != type::FUNC;
}

int
cxx_compiler::pointer_member_type::size() const
{
  int n = int_type::create()->size();
  if (m_T->m_id == type::FUNC)
    return n + type_impl::pointer_sizeof;
  return n;
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
  if (u)
    u->m_flag = (usr::flag_t)(u->m_flag & ~usr::FUNCTION & ~usr::VL);
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

void cxx_compiler::
template_param_type::decl(std::ostream& os, std::string name) const
{
  const type* T = m_tag->m_types.second;
  if (T && T != this) {
    if (T->m_id == type::TEMPLATE_PARAM)
      return record_impl::decl(os, name, T->get_tag(), true);
    return T->decl(os, name);
  }

  record_impl::decl(os, name, m_tag, false);
}

void cxx_compiler::
template_param_type::encode(std::ostream& os) const
{
  assert(0);
}

int cxx_compiler::template_param_type::size() const { return 1; }

const cxx_compiler::type*
cxx_compiler::template_param_type::complete_type() const
{
  assert(this == m_tag->m_types.first);
  const type* T = m_tag->m_types.second;
  return T ? T : this;
}

const cxx_compiler::type*
cxx_compiler::template_param_type::instantiate() const
{
  const type* T = m_tag->m_types.second;
  assert(T);
  return T;
}

bool cxx_compiler::
template_param_type::template_match(const type* Ty, bool unq) const
{
  if (unq)
    Ty = Ty->unqualified();
  if (const type* T2 = m_tag->m_types.second) {
    if (T2 == Ty)
      return true;
    type::id_t idx = T2->m_id;
    type::id_t idy = Ty->m_id;
    return idx == idy && idx == type::TEMPLATE_PARAM;
  }
  m_tag->m_types.second = Ty;
  return true;
}

namespace cxx_compiler {
  template_param_type::table_t template_param_type::table;
} // end of namespace cxx_compiler

const cxx_compiler::template_param_type*
cxx_compiler::template_param_type::create(tag* ptr)
{
  table_t::const_iterator p = table.find(ptr);
  if (p != table.end())
    return p->second;
  return table[ptr] = new template_param_type(ptr);
}

namespace cxx_compiler {
  map<vector<const type*>, brace_type*> brace_type::m_table;
}

const cxx_compiler::brace_type*
cxx_compiler::brace_type::create(const vector<const type*>& vt)
{
  auto p = m_table.find(vt);
  if (p != m_table.end())
    return p->second;
  return m_table[vt] = new brace_type(vt);
}

// Just called from debugger command line
void debug_type(const cxx_compiler::type* T)
{
  using namespace std;
  T->decl(cout, "");
  cout << endl;
}
