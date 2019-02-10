#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

cxx_compiler::addrof::addrof(const type* T, var* ref, int offset)
  : var(T), m_ref(ref), m_offset(offset) {}

cxx_compiler::genaddr::genaddr(const pointer_type* G, const type* T, var* ref, int offset)
  : var(G), generated(G,T), addrof(G,ref,offset)
{
  using namespace std;
  m_code.push_back(new addr3ac(this,m_ref));
  if ( offset ){
    var* off = expressions::primary::literal::integer::create(offset);
    m_code.push_back(new add3ac(this,this,off));
  }
}

cxx_compiler::var* cxx_compiler::genaddr::rvalue()
{
  using namespace std;
  block* b = m_scope->m_id == scope::BLOCK ? static_cast<block*>(m_scope) : 0;
  if ( b && !expressions::constant_flag ){
    if ( !m_code.empty() ){
      vector<var*>& v = garbage;
      vector<var*>::reverse_iterator p = find(v.rbegin(),v.rend(),this);
      assert(p != v.rend());
      v.erase(p.base()-1);
      b->m_vars.push_back(this);
    }
  }
  if ( !expressions::constant_flag ){
    copy(m_code.begin(),m_code.end(),back_inserter(code));
    m_code.clear();
  }
  return this;
}

cxx_compiler::ref::ref(const pointer_type* pt)
  : var(pt), m_result(pt->referenced_type()) {}

cxx_compiler::var* cxx_compiler::ref::rvalue()
{
  using namespace std;
  const type* T = m_result;
  T = T->complete_type();
  if ( const pointer_type* G = T->ptr_gen() ){
    generated* ret = new generated(G,T);
    if ( scope::current->m_id == scope::BLOCK ){
      block* b = static_cast<block*>(scope::current);
      b->m_vars.push_back(ret);
    }
    else
      garbage.push_back(ret);
    code.push_back(new cast3ac(ret,this,G));
    return ret;
  }
  var* ret = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  code.push_back(new invraddr3ac(ret,this));
  return ret;
}

cxx_compiler::var * cxx_compiler::refaddr::rvalue()
{
  const type* T = m_result;
  if ( const pointer_type* G = T->ptr_gen() ){
    genaddr* tmp = new genaddr(G,T,m_addrof.m_ref,m_addrof.m_offset);
    garbage.push_back(tmp);
    return tmp->rvalue();
  }
  var* ret = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  var* off = expressions::primary::literal::integer::create(m_addrof.m_offset);
  code.push_back(new roff3ac(ret,m_addrof.m_ref,off));
  return ret;
}

cxx_compiler::usr* cxx_compiler::refbit::mask(int n)
{
  return expressions::primary::literal::integer::create(~(~0 << n));
}

cxx_compiler::usr* cxx_compiler::refbit::mask(int n, int pos)
{
  using namespace std;
  n = ~(~0 << n);
  n <<= pos;
  n = ~n;
  return expressions::primary::literal::integer::create(n);
}

cxx_compiler::var* cxx_compiler::refsomewhere::rvalue()
{
  block* b = ( scope::current->m_id == scope::BLOCK ) ? static_cast<block*>(scope::current) : 0;
  const type* T = m_result;
  if ( const pointer_type* G = T->ptr_gen() ){
    var* tmp = new var(G);
    var* ret = new generated(G,T);
    if ( b ) {
      b->m_vars.push_back(tmp);
      b->m_vars.push_back(ret);
    }
    else {
      garbage.push_back(tmp);
      garbage.push_back(ret);
    }
    code.push_back(new addr3ac(tmp,m_ref));
    code.push_back(new add3ac(ret,tmp,m_offset));
    return ret;
  }
  var* ret = new var(T);
  if ( b )
    b->m_vars.push_back(ret);
  else
    garbage.push_back(ret);
  code.push_back(new roff3ac(ret,m_ref,m_offset));
  return ret;
}

cxx_compiler::var* cxx_compiler::addrof::rvalue()
{
  if ( scope::current == &scope::root || expressions::constant_flag )
    return this;
  else {
    var* ret = new var(m_type);
    block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
    b ? b->m_vars.push_back(ret) : garbage.push_back(ret);
    code.push_back(new addr3ac(ret,m_ref));
    if ( m_offset ){
      var* off = expressions::primary::literal::integer::create(m_offset);
      code.push_back(new add3ac(ret,ret,off));
    }
    return ret;
  }
}

namespace cxx_compiler {
  const type* unsigned_wchar()
  {
    switch(generator::wchar::id) {
    case type::INT: return uint_type::create();
    case type::LONG: return ulong_type::create();
    case type::LONGLONG: return ulong_long_type::create();
    case type::SHORT:      
    default:
      return ushort_type::create();
    }
  }
  const type* unsigned_type(const type* T)
  {
    T = T->unqualified();
    switch (T->m_id) {
    case type::ENUM:
      {
	typedef const enum_type ET;
	ET* et = static_cast<ET*>(T);
	T = et->get_integer();
	return unsigned_type(T);
      }
    case type::CHAR: return uchar_type::create();
    case type::SCHAR: return uchar_type::create();
    case type::WCHAR: return unsigned_wchar();
    case type::SHORT: return ushort_type::create();
    case type::INT: return uint_type::create();
    case type::LONG: return ulong_type::create();
    default:
      assert(T->m_id == type::LONGLONG);
      return ulong_long_type::create();
    }
  }
}  // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::refbit::rvalue()
{
  using namespace std;
  const type* T = m_result;
  var* ret = new var(T);
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  assert(b);
  b->m_vars.push_back(ret);
  if ( m_dot ){
    var* offset = expressions::primary::literal::integer::create(m_addrof.m_offset);
    code.push_back(new roff3ac(ret,m_addrof.m_ref,offset));
  }
  else {
    var* ptr = m_addrof.m_ref;
    if ( m_addrof.m_offset ){
      ptr = new var(m_type);
      b->m_vars.push_back(ptr);
      usr* off = expressions::primary::literal::integer::create(m_addrof.m_offset);
      code.push_back(new add3ac(ptr,m_addrof.m_ref,off));
    }
    code.push_back(new invraddr3ac(ret,ptr));
  }
  if ( m_position ){
    if (T->_signed()) {
      const type* U = unsigned_type(T);      
      var* tmp = new var(U);
      b->m_vars.push_back(tmp);
      code.push_back(new cast3ac(tmp,ret,U));
      ret = tmp;
    }
    usr* pos = expressions::primary::literal::integer::create(m_position);
    var* tmp = new var(T);
    b->m_vars.push_back(tmp);
    code.push_back(new rsh3ac(tmp,ret,pos));
    ret = tmp;
  }
  usr* m = mask(m_bit);
  var* tmp = new var(T);
  b->m_vars.push_back(tmp);
  code.push_back(new and3ac(tmp,ret,m));
  ret = tmp;
  if (T->_signed()) {
    usr* m = expressions::primary::literal::integer::create(1 << (m_bit-1));
    var* tmp = new var(T);
    b->m_vars.push_back(tmp);
    code.push_back(new and3ac(tmp,ret,m));
    usr* zero = expressions::primary::literal::integer::create(0);
    goto3ac* go = new goto3ac(goto3ac::EQ,tmp,zero);
    code.push_back(go);
    {
      usr* ext = expressions::primary::literal::integer::create((~0 << m_bit));
      code.push_back(new or3ac(ret,ret,ext));
    }
    to3ac* to = new to3ac;
    code.push_back(to);
    to->m_goto.push_back(go);
    go->m_to = to;
  }
  return ret;
}

cxx_compiler::var* cxx_compiler::refbit::size()
{
  using namespace error::expressions::unary::size;
  bit_field(parse::position,m_member);
  return refaddr::size();
}
