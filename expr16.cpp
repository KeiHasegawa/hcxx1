// assignmet-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

cxx_compiler::var* cxx_compiler::var::assign(var*)
{
  using namespace error::expressions::assignment;
  not_lvalue(parse::position);
  return this;
}

namespace cxx_compiler {
  struct set_va {
    var* x;
    var* y;
    const map<const record_type*, int>& xvco;
    const map<const record_type*, int>& yvco;
    set_va(var* xx, var* yy, const map<const record_type*, int>& xv,
	   const map<const record_type*, int>& yv)
      : x(xx), y(yy), xvco(xv), yvco(yv) {}
    void operator()(const record_type* rec)
    {
      using namespace expressions::primary::literal;
      typedef map<const record_type*, int>::const_iterator IT;
      IT py = yvco.find(rec);
      assert(py != yvco.end());
      int yvc_off = py->second;
      assert(yvc_off);
      var* yoff = integer::create(yvc_off);
      var* tmp = new var(rec);
      if (scope::current->m_id == scope::BLOCK) {
	block* b = static_cast<block*>(scope::current);
	b->m_vars.push_back(tmp);
      }
      else
	garbage.push_back(tmp);
      code.push_back(new roff3ac(tmp, y, yoff));
      IT px = xvco.find(rec);
      assert(px != xvco.end());
      int xvc_off = px->second;
      assert(xvc_off);
      var* xoff = integer::create(xvc_off);
      code.push_back(new loff3ac(x, xoff, tmp));
    }
  };
  var* aggregate_conv(const type* T, var* y)
  {
    using namespace expressions::primary::literal;
    const type* Tx = T->unqualified();
    const type* Ty = y->result_type();
    Ty = Ty->unqualified();
    if (compatible(Tx, Ty))
      return y;
    assert(Tx->m_id == type::RECORD);
    typedef const record_type REC;
    REC* xrec = static_cast<REC*>(Tx);
    if (Ty->m_id != type::RECORD)
      return expressions::assignment::ctor_conv_common(xrec, y, false);
    REC* yrec = static_cast<REC*>(Ty);
    vector<route_t> dummy;
    bool ambiguous = false;
    int offset = calc_offset(yrec, xrec, dummy, &ambiguous);
    if (ambiguous)
      error::not_implemented();
    if (offset >= 0) {
      var* x = new var(xrec);
      if (scope::current->m_id == scope::BLOCK) {
	block* b = static_cast<block*>(scope::current);
	b->m_vars.push_back(x);
      }
      else
	garbage.push_back(x);
      var* off = integer::create(offset);
      code.push_back(new roff3ac(x, y, off));
      const vector<REC*>& va = xrec->virt_ancestor();
      const map<REC*, int>& xvco = xrec->virt_common_offset();
      const map<REC*, int>& yvco = yrec->virt_common_offset();
      for_each(begin(va), end(va), set_va(x, y, xvco, yvco));
      return x;
    }
    usr* fun = cast_impl::conversion_function(yrec, xrec);
    assert(fun);
    return call_impl::wrapper(fun, 0, y);
  }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::usr::assign(var* op)
{
  using namespace error::expressions::assignment;
  m_type = m_type->complete_type();
  const type* T = m_type;
  if ( !T->modifiable() )
    not_modifiable(parse::position,this);
  var* y = op->rvalue();
  y->m_type = y->m_type->complete_type();
  bool discard = false;
  T = expressions::assignment::valid(T, y, &discard, true);
  if (!T) {
    invalid(parse::position,this,discard);
    T = int_type::create();
  }
  if (m_type->m_id == type::REFERENCE)
    code.push_back(new invladdr3ac(this,y));
  else {
    y = T->aggregate() ? aggregate_conv(T, y) : y->cast(T);
    code.push_back(new assign3ac(this,y));
  }
  if ( !y->isconstant() )
    return y;
  var* x = new var(T);
  if (scope::current->m_id == scope::BLOCK) {
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new assign3ac(x,y));
  return x;
}

cxx_compiler::var* cxx_compiler::generated::assign(var*)
{
  using namespace error::expressions::assignment;
  not_modifiable_lvalue(parse::position,m_org);
  return this;
}

cxx_compiler::var* cxx_compiler::ref::assign(var* op)
{
  m_result = m_result->complete_type();
  const type* T = m_result;
  if ( !T->modifiable() ){
    using namespace error::expressions::assignment;
    not_modifiable(parse::position,0);
  }
  bool discard = false;
  var* y = op->rvalue();
  T = expressions::assignment::valid(T, y, &discard, true);
  if (!T) {
    using namespace error::expressions::assignment;
    invalid(parse::position,0,discard);
    T = int_type::create();
  }
  y = T->aggregate() ? aggregate_conv(T, y) : y->cast(T);
  code.push_back(new invladdr3ac(this,y));
  if ( !y->isconstant() )
    return y;
  var* x = new var(T);
  if (scope::current->m_id == scope::BLOCK) {
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new assign3ac(x,y));
  return x;
}

cxx_compiler::var* cxx_compiler::refaddr::assign(var* op)
{
  using namespace std;
  if (!lvalue()) {
    using namespace error::expressions::assignment;
    not_lvalue(parse::position);
    return this;
  }
  m_result = m_result->complete_type();
  const type* T = m_result;
  if (!T->modifiable()) {
    using namespace error::expressions::assignment;
    not_modifiable(parse::position,0);
  }
  bool discard = false;
  var* z = op->rvalue();
  const type* res = expressions::assignment::valid(T, z, &discard, true);
  if (!res) {
    using namespace error::expressions::assignment;
    invalid(parse::position,0,discard);
    res = int_type::create();
  }
  z = res->aggregate() ? aggregate_conv(res, z) : z->cast(res);
  using namespace expressions::primary::literal;
  int offset = m_addrof.m_offset;
  var* y = integer::create(offset);
  var* x = m_addrof.m_ref;
  vector<route_t> dummy;
  var* xx = expressions::primary::action(x, dummy);
  if (x != xx) {
    if (offset) {
      var* tmp = new var(xx->m_type);
      if (scope::current->m_id == scope::BLOCK) {
        block* b = static_cast<block*>(scope::current);
        b->m_vars.push_back(tmp);
      }
      else
        garbage.push_back(tmp);
      code.push_back(new add3ac(tmp, xx, y));
      xx = tmp;
    }
    code.push_back(new invladdr3ac(xx,z));
  }
  else {
    if (T->m_id == type::REFERENCE) {
      var* r = new var(T);
      if ( scope::current->m_id == scope::BLOCK ){
	block* b = static_cast<block*>(scope::current);
	b->m_vars.push_back(r);
      }
      else
	garbage.push_back(r);
      code.push_back(new roff3ac(r,x,y));
      code.push_back(new invladdr3ac(r,z));
    }
    else
      code.push_back(new loff3ac(x,y,z));
  }
  if (!z->isconstant())
    return z;
  var* ret = new var(res);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  code.push_back(new assign3ac(ret,z));
  return ret;
}

cxx_compiler::var* cxx_compiler::refbit::assign(var* op)
{
  using namespace std;
  const type* T = m_result;
  var* x = new var(T);
  var* y = op->rvalue();
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  assert(b);
  b->m_vars.push_back(x);
  usr* u = mask(m_bit);
  code.push_back(new and3ac(x,y,u));
  var* t0 = 0;
  if ( m_position ){
    t0 = new var(T);
    b->m_vars.push_back(t0);
    usr* pos = expressions::primary::literal::integer::create(m_position);
    code.push_back(new lsh3ac(t0,x,pos));
  }
  var* t1 = new var(T);
  b->m_vars.push_back(t1);
  if ( m_dot ){
    var* off = expressions::primary::literal::integer::create(m_addrof.m_offset);
    code.push_back(new roff3ac(t1,m_addrof.m_ref,off));
  }
  else {
    var* ptr = m_addrof.m_ref;
    if ( m_addrof.m_offset ){
      ptr = new var(m_type);
      b->m_vars.push_back(ptr);
      usr* off = expressions::primary::literal::integer::create(m_addrof.m_offset);
      code.push_back(new add3ac(ptr,m_addrof.m_ref,off));
    }
    code.push_back(new invraddr3ac(t1,ptr));
  }
  usr* v = mask(m_bit,m_position);
  code.push_back(new and3ac(t1,t1,v));
  code.push_back(new or3ac(t1,t1,t0 ? t0 : x));
  if ( m_dot ){
    var* off = expressions::primary::literal::integer::create(m_addrof.m_offset);
    code.push_back(new loff3ac(m_addrof.m_ref,off,t1));
  }
  else {
    var* ptr = m_addrof.m_ref;
    if ( m_addrof.m_offset ){
      ptr = new var(m_type);
      b->m_vars.push_back(ptr);
      usr* off = expressions::primary::literal::integer::create(m_addrof.m_offset);
      code.push_back(new add3ac(ptr,m_addrof.m_ref,off));
    }
    code.push_back(new invladdr3ac(ptr,t1));
  }
  return x;
}


cxx_compiler::var* cxx_compiler::refsomewhere::assign(var* op)
{
  op = op->rvalue();
  const type* T = m_result;
  bool discard = false;
  T = expressions::assignment::valid(T, op, &discard, true);
  if ( !T ){
    using namespace error::expressions::assignment;
    invalid(parse::position,0,discard);
    T = int_type::create();
  }
  op = T->aggregate() ? aggregate_conv(T, op) : op->cast(T);
  vector<route_t> dummy;
  var* x = expressions::primary::action(m_ref, dummy);
  if (x != m_ref) {
    var* tmp = new var(x->m_type);
    if (scope::current->m_id == scope::BLOCK) {
      block* b = static_cast<block*>(scope::current);
      b->m_vars.push_back(tmp);
    }
    else
      garbage.push_back(tmp);
    code.push_back(new add3ac(tmp, x, m_offset));
    code.push_back(new invladdr3ac(tmp,op));
  }
  else
    code.push_back(new loff3ac(m_ref,m_offset,op));
  if ( !op->isconstant() )
    return op;
  var* ret = new var(m_result);
  block* b = ( scope::current->m_id == scope::BLOCK ) ? static_cast<block*>(scope::current) : 0;
  if ( b )
    b->m_vars.push_back(ret);
  else
    garbage.push_back(ret);
  code.push_back(new assign3ac(ret,op));
  return ret;
}

// comma-expression
cxx_compiler::var* cxx_compiler::var::comma(var* right)
{
  rvalue();
  return right->rvalue();
}
