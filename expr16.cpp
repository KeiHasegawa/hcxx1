// assignmet-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

cxx_compiler::var* cxx_compiler::var::assign(var*)
{
  using namespace error::expressions::assignment;
  if (m_type->m_id != type::TEMPLATE_PARAM)
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
  var* ctor_code(const type* Tx, var* u, var* v)
  {
    if (scope::current->m_id != scope::BLOCK)
      return v;
    int cvr = 0;
    const type* Ty = v->result_type();
    Ty = Ty->unqualified(&cvr);
    usr* copy_ctor = get_copy_ctor(Tx->qualified(cvr));
    if (!copy_ctor)
      return v;
    usr::flag2_t flag2 = copy_ctor->m_flag2;
    if (flag2 & usr::DEFAULT) {
      code.push_back(new assign3ac(u, v));
      return v;
    }
    const type* T = pointer_type::create(Tx);
    var* t0 = new var(T);
    var* t1 = new var(T);
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(t0);
    b->m_vars.push_back(t1);
    code.push_back(new addr3ac(t0, u));
    code.push_back(new addr3ac(t1, v));
    code.push_back(new param3ac(t0));
    code.push_back(new param3ac(t1));
    if (flag2 & usr::HAS_DEFAULT_ARG) {
      using namespace declarations::declarators::function;
      typedef map<usr*, vector<var*> >::const_iterator IT;
      IT p = default_arg_table.find(copy_ctor);
      assert(p != default_arg_table.end());
      const vector<var*>& v = p->second;
      assert(!v.empty());
      for_each(begin(v)+1, end(v), [](var* v)
               {
                 if (v)
                   code.push_back(new param3ac(v));
               });
    }
    copy_ctor = instantiate_if(copy_ctor);
    code.push_back(new call3ac(0, copy_ctor));
    if (!error::counter && !cmdline::no_inline_sub) {
      usr::flag_t flag = copy_ctor->m_flag;
      if (flag & usr::INLINE) {
        using namespace declarations::declarators::function;
        using namespace definition::static_inline;
        skip::table_t::const_iterator p = skip::stbl.find(copy_ctor);
        if (p != skip::stbl.end()) {
          using definition::static_inline::info_t;
          if (info_t* info = p->second)
            substitute(code, code.size()-1, info);
        }
      }
    }
    return v;
  }

  var* aggregate_conv(const type* T, var* y, bool ctor_conv, var* res)
  {
    using namespace expressions::primary::literal;
    const type* Tx = T->unqualified();
    const type* Ty = y->result_type();
    Ty = Ty->unqualified();
    if (compatible(Tx, Ty)) {
      if (res)
        return ctor_code(Tx, res, y);
      return y;
    }

    if (tag* ptr = Tx->get_tag()) {
      if (ptr->m_flag & tag::TYPENAMED) {
        if (Tx->m_id == type::INCOMPLETE_TAGGED)
          return y;
      }
    }

    assert(Tx->m_id == type::RECORD);
    typedef const record_type REC;
    REC* xrec = static_cast<REC*>(Tx);
    if (ctor_conv)
      return expressions::assignment::ctor_conv_common(xrec, y, false, 0, res);
    if (Ty->m_id != type::RECORD)
      return y;
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
      if (res)
        return ctor_code(xrec, res, x);
      return x;
    }
    usr* fun = cast_impl::conversion_function(yrec, xrec, true);
    assert(fun);
    return call_impl::wrapper(fun, 0, y);
  }
  namespace operator_assign {
    inline bool require(const type* T, var* y)
    {
      T = T->unqualified();
      if (usr* op_fun = operator_function(T, '=')) {
	usr::flag2_t flag2 = op_fun->m_flag2;
	if (flag2 & usr::DEFAULT)
	  return false;
        if (y) {
          const type* T2 = op_fun->m_type;
          assert(T2->m_id == type::FUNC);
          typedef const func_type FT;
          FT* ft = static_cast<FT*>(T2);
          vector<var*> arg;
          arg.push_back(y);
          int trial_cost = 0;
          var tmp(T);
          int n = code.size();
          var* ret = call_impl::common(ft, op_fun, &arg, &trial_cost, &tmp,
                		       false, 0);
          for_each(begin(code)+n, end(code), [](tac* p){ delete p; });
          code.resize(n);
          return ret;
        }
        return true;
      }
      if (T->m_id != type::RECORD)
        return false;
      typedef const record_type REC;
      REC* rec = static_cast<REC*>(T);
      const vector<usr*>& member = rec->member();
      typedef vector<usr*>::const_iterator IT;
      IT p = find_if(begin(member), end(member),
                     [](usr* u){ return require(u->m_type, 0); });
      return p != end(member);
    }
    void gen(var* px, var* y, const record_type* rec);
    struct recursive {
      var* px;
      var* py;
      const record_type* m_rec;
      recursive(var* x, var* y, const record_type* rec)
        : px(x), py(y), m_rec(rec) {}
      void operator()(usr* member)
      {
        using namespace expressions::primary::literal;
        string name = member->m_name;
        pair<int, usr*> off = m_rec->offset(name);
        int offset = off.first;
        assert(offset >= 0);
        const type* T = member->m_type;
        const reference_type* rt = reference_type::create(T);
        var* src = new ref(rt);
        const type* pt = pointer_type::create(T);
        var* dst = new var(pt);
        if (scope::current->m_id == scope::BLOCK) {
          block* b = static_cast<block*>(scope::current);
          b->m_vars.push_back(src);
          b->m_vars.push_back(dst);
        }
        else {
          garbage.push_back(src);
          garbage.push_back(dst);
        }
        code.push_back(new cast3ac(src, py, rt));
        code.push_back(new cast3ac(dst, px, pt));
        if (offset) {
          var* off = integer::create(offset);
          var* t0 = new ref(rt);
          var* t1 = new var(pt);
          if (scope::current->m_id == scope::BLOCK) {
            block* b = static_cast<block*>(scope::current);
            b->m_vars.push_back(t0);
            b->m_vars.push_back(t1);
          }
          else {
            garbage.push_back(t0);
            garbage.push_back(t1);
          }
          code.push_back(new add3ac(t0, src, off));
          code.push_back(new add3ac(t1, dst, off));
          src = t0;
          dst = t1;
        }
        var* tmp = new var(T);
        if (scope::current->m_id == scope::BLOCK) {
          block* b = static_cast<block*>(scope::current);
          b->m_vars.push_back(tmp);
        }
        else
          garbage.push_back(tmp);
        if (require(T, src)) {
          assert(T->m_id == type::RECORD);
          typedef const record_type REC;
          REC* rec = static_cast<REC*>(T);
          gen(dst, src, rec);
        }
        else {
          code.push_back(new invraddr3ac(tmp, src));
          code.push_back(new invladdr3ac(dst, tmp));
        }
      }
    };
    void gen(var* px, var* y, const record_type* rec)
    {
      if (usr* op_fun = operator_function(rec, '=')) {
        vector<var*> arg;
        arg.push_back(y);
        call_impl::wrapper(op_fun, &arg, px);
      }
      else {
        const vector<usr*>& member = rec->member();
        var* py = new var(px->m_type);
        if (scope::current->m_id == scope::BLOCK) {
          block* b = static_cast<block*>(scope::current);
          b->m_vars.push_back(py);
        }
        else
          garbage.push_back(py);
        for_each(begin(member), end(member), recursive(px, py, rec));
      }
    }
  } // end of namespace operator_assign
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
  bool ctor_conv = false;
  if (T->m_id == type::REFERENCE) {
    typedef const reference_type RT;
    RT* rt = static_cast<RT*>(T);
    T = rt->referenced_type();
    const type* Ty = y->m_type;
    int cvr = 0;
    Ty->unqualified(&cvr);
    if (cvr & 1)
      T = const_type::create(T);
    if (cvr & 2)
      T = volatile_type::create(T);
    if (cvr & 4)
      T = restrict_type::create(T);
    T = reference_type::create(T);
  }
  T = expressions::assignment::valid(T, y, &discard, &ctor_conv, 0);
  if (!T) {
    invalid(parse::position,this,discard);
    T = int_type::create();
  }
  if (m_type->m_id == type::REFERENCE)
    code.push_back(new invladdr3ac(this,y));
  else {
    y = T->aggregate() ? aggregate_conv(T, y, ctor_conv, 0) : y->cast(T);
    const type* Ty = y->m_type;
    Ty = Ty->unqualified();
    var tmp(Ty);
    if (operator_assign::require(T, &tmp)) {
      const type* pt = pointer_type::create(T);
      var* px = new var(pt);
      if (scope::current->m_id == scope::BLOCK) {
        block* b = static_cast<block*>(scope::current);
        b->m_vars.push_back(px);
      }
      else
        garbage.push_back(px);
      code.push_back(new addr3ac(px, this));
      assert(T->m_id == type::RECORD);
      typedef const record_type REC;
      REC* rec = static_cast<REC*>(T);
      operator_assign::gen(px, y, rec);
    }
    else
      code.push_back(new assign3ac(this, y));
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
  bool ctor_conv = false;
  T = expressions::assignment::valid(T, y, &discard, &ctor_conv, 0);
  if (!T) {
    using namespace error::expressions::assignment;
    invalid(parse::position,0,discard);
    T = int_type::create();
  }
  y = T->aggregate() ? aggregate_conv(T, y, ctor_conv, 0) : y->cast(T);
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
  bool ctor_conv = false;
  const type* res =
    expressions::assignment::valid(T, z, &discard, &ctor_conv, 0);
  if (!res) {
    using namespace error::expressions::assignment;
    invalid(parse::position,0,discard);
    res = int_type::create();
  }
  z = res->aggregate() ? aggregate_conv(res, z, ctor_conv, 0) : z->cast(res);
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
    else {
      const type* Tx = x->m_type;
      Tx = Tx->complete_type();
      if (Tx->aggregate())
        code.push_back(new loff3ac(x,y,z));
      else {
        assert(!offset);
        code.push_back(new invladdr3ac(x,z));
      }
    }
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
  bool ctor_conv = false;
  T = expressions::assignment::valid(T, op, &discard, &ctor_conv, 0);
  if ( !T ){
    using namespace error::expressions::assignment;
    invalid(parse::position,0,discard);
    T = int_type::create();
  }
  op = T->aggregate() ? aggregate_conv(T, op, ctor_conv, 0) : op->cast(T);
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
