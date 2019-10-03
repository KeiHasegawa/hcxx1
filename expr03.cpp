// cast-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "cxx_y.h"

namespace cxx_compiler { namespace expressions { namespace cast {
  const type* valid(const type*, var*);
} } } // end of namespace cast, expressions and cxx_compiler

cxx_compiler::var* cxx_compiler::expressions::cast::info_t::gen()
{
  var* expr = m_expr->gen();
  const type* T = m_type->unqualified();
  if (T->m_id == type::REFERENCE) {
    const type* Ty = expr->m_type;
    if (Ty->m_id == type::REFERENCE) {
      const type* res = valid(T, expr);
      return expr->cast(T);
    }
  }
  expr = expr->rvalue();
  if (T->m_id == type::VOID) {
    var* ret = new var(T);
    garbage.push_back(ret);
    return ret;
  }
  const type* res = valid(T, expr);
  if (!res) {
    if (!T->scalar()) {
      using namespace error::expressions::cast;
      not_scalar(parse::position);
      res = int_type::create();
    }
    else {
      using namespace error::expressions::cast;
      invalid(parse::position);
      res = int_type::create();
    }
  }
  return res->aggregate() ? aggregate_conv(res, expr) : expr->cast(res);
}

const cxx_compiler::file_t& cxx_compiler::expressions::cast::info_t::file() const
{
  return m_expr->file();
}

namespace cxx_compiler {
  namespace cast_impl {
    inline bool require(const type* Tx, const type* Ty)
    {
      Tx = Tx->unqualified();
      Ty = Ty->unqualified();
      if (compatible(Tx, Ty))
        return false;
      if (Tx->m_id == type::POINTER && Ty->m_id == type::POINTER) {
        typedef const pointer_type PT;
        PT* Px = static_cast<PT*>(Tx);
        PT* Py = static_cast<PT*>(Ty);
        Tx = Px->referenced_type();
        Ty = Py->referenced_type();
        Tx = Tx->unqualified();
        Ty = Ty->unqualified();
        return !compatible(Tx, Ty);
      }
      return true;
    }
    inline bool virt_base(const base* bp, tag* ptr)
    {
      return (bp->m_flag & usr::VIRTUAL) && bp->m_tag == ptr;
    }
    inline int nth_virt_base(tag* ptr, const base* bp)
    {
      vector<base*>& bases = *ptr->m_bases;
      int n = accumulate(begin(bases), end(bases), 0, record_impl::base_vb);
      vector<base*> tmp;
      copy_if(begin(bases), end(bases), back_inserter(tmp),
              [](base* bp){ return bp->m_flag & usr::VIRTUAL; });
      typedef vector<base*>::iterator IT;
      IT p = find(begin(tmp), end(tmp), bp);
      assert(p != end(tmp));
      return n + distance(begin(tmp), p);
    }
    inline var* ref_vbtbl(const record_type* rec, const base* bp, var* src)
    {
      using namespace expressions::primary::literal;
      tag* ptr = rec->get_tag();
      map<string, vector<usr*> >& usrs = ptr->m_usrs;
      assert(usrs.find(vbptr_name) != usrs.end());
      pair<int, usr*> off = rec->offset(vbptr_name);
      int offset = off.first;
      assert(offset >= 0);
      usr* u = off.second;
      const type* T = u->m_type;
      assert(T->m_id == type::POINTER);
      typedef const pointer_type PT;
      PT* pt = static_cast<PT*>(T);
      T = pt->referenced_type();
      assert(T->m_id == type::ARRAY);
      typedef const array_type AT;
      AT* at = static_cast<AT*>(T);
      const type* T2 = at->element_type();
      T2 = T2->unqualified();
      const type* T1 = pointer_type::create(T2);
      const type* T0 = pointer_type::create(T1);
      var* t0 = new var(T0);
      if (scope::current->m_id == scope::BLOCK) {
        block*b = static_cast<block*>(scope::current);
        b->m_vars.push_back(t0);
      }
      else
        garbage.push_back(t0);
      code.push_back(new cast3ac(t0, src, T0));
      if (offset) {
        var* off = integer::create(offset);
        code.push_back(new add3ac(t0, t0, off));
      }
      var* t1 = new var(T1);
      if (scope::current->m_id == scope::BLOCK) {
        block*b = static_cast<block*>(scope::current);
        b->m_vars.push_back(t1);
      }
      else
        garbage.push_back(t1);
      code.push_back(new invraddr3ac(t1, t0));
      if (int n = nth_virt_base(ptr, bp)) {
        var* off = integer::create(n * T2->size());
        code.push_back(new add3ac(t1, t1, off));
      }
      var* t2 = new var(T2);
      if (scope::current->m_id == scope::BLOCK) {
        block*b = static_cast<block*>(scope::current);
        b->m_vars.push_back(t2);
      }
      else
        garbage.push_back(t2);
      code.push_back(new invraddr3ac(t2, t1));
      return t2;
    }
    inline const type* referenced_type(const type* T)
    {
      if (T->m_id == type::POINTER) {
	typedef const pointer_type PT;
	PT* pt = static_cast<PT*>(T);
	return pt->referenced_type();
      }
      if (T->m_id == type::REFERENCE) {
	typedef const reference_type RT;
	RT* rt = static_cast<RT*>(T);
	return rt->referenced_type();
      }
      return 0;
    }
    inline var*
    base_ptr_offset(const type* Tx, var* src, const vector<route_t>& route)
    {
      using namespace expressions::primary::literal;
      Tx = Tx->unqualified();
      Tx = referenced_type(Tx);
      if (!Tx)
        return 0;
      Tx = Tx->unqualified();
      if (Tx->m_id != type::RECORD)
        return 0;
      typedef const record_type REC;
      REC* Rx = static_cast<REC*>(Tx);
      const type* Ty = src->m_type;
      Ty = Ty->unqualified();
      Ty = referenced_type(Ty);
      if (!Ty)
        return 0;
      Ty = Ty->unqualified();
      if (Ty->m_id != type::RECORD) 
	return 0;
      REC* Ry = static_cast<REC*>(Ty);
      tag* xtag = Rx->get_tag();
      tag* ytag = Ry->get_tag();
      if (ytag->m_bases) {
        const vector<base*>& bases = *ytag->m_bases;
        typedef vector<base*>::const_iterator IT;
        IT p = find_if(begin(bases), end(bases),
                       bind2nd(ptr_fun(virt_base), xtag));
        if (p != end(bases))
          return ref_vbtbl(Ry, *p, src);
      }
      int offset = calc_offset(Ry, Rx, route, 0);
      if (offset <= 0)
        return 0;
      return integer::create(offset);
    }
    var* with_route(const type* Tx, var* y, const vector<route_t>& route)
    {
      const type* Ty = y->m_type;
      if ( Tx == Ty )
        return y;
      var* x = new var(Tx);
      if (scope::current->m_id == scope::BLOCK) {
        block* b = static_cast<block*>(scope::current);
        b->m_vars.push_back(x);
      }
      else
        garbage.push_back(x);
      if (!cast_impl::require(Tx, Ty))
        code.push_back(new assign3ac(x, y));
      else {
        code.push_back(new cast3ac(x, y, Tx));
        if (var* off = cast_impl::base_ptr_offset(Tx, y, route))
          code.push_back(new add3ac(x, x, off));
        else if (var* off = cast_impl::base_ptr_offset(Ty, x, route))
          code.push_back(new sub3ac(x, x, off));
      }
      return x;
    }
    inline usr* other_conv_fun(const record_type* rec, const type* T)
    {
      if (!T->arithmetic())
	return 0;
      const type* Ta[] = {
	bool_type::create(),
	schar_type::create(),
	uchar_type::create(),
	short_type::create(),
	ushort_type::create(),
	wchar_type::create(),
	int_type::create(),
	uint_type::create(),
	long_type::create(),
	ulong_type::create(),
	long_long_type::create(),
	ulong_long_type::create(),
	float_type::create(),
	double_type::create(),
	long_double_type::create()
      };
      int n = sizeof Ta/sizeof Ta[0];
      usr* tmp = 0;
      auto cf = [rec, T, &tmp](const type* O) {
	if (T == O)
	  return (usr*)0;
	tmp = conversion_function(rec, O, false);
	return tmp;
      };
      const type** p = find_if(&Ta[0], &Ta[n], cf);
      if (p == &Ta[n])
	return 0;
      usr* ret = tmp;
      const type** q = find_if(p+1, &Ta[n], cf);
      if (q != &Ta[n])
	error::not_implemented();
      return ret;
    }
    usr* conversion_function(const record_type* rec, const type* T, bool other)
    {
      ostringstream os;
      T->decl(os, "");
      string name = os.str();
      tag* ptr = rec->get_tag();
      parse::identifier::mode_t org = parse::identifier::mode;
      parse::identifier::mode = parse::identifier::no_err;
      int r = parse::identifier::lookup(name, ptr);
      parse::identifier::mode = org;
      if (!r)
	return other ? other_conv_fun(rec, T) : 0;
      assert(r == IDENTIFIER_LEX);
      var* v = cxx_compiler_lval.m_var;
      genaddr* ga = v->genaddr_cast();
      assert(ga);
      v = ga->m_ref;
      assert(v->usr_cast());
      usr* op = static_cast<usr*>(v);
      return op;
    }
    inline var* conversion(const record_type* rec, var* src, const type* T)
    {
      usr* op = conversion_function(rec, T, true);
      if (!op) {
	// already handled error
	var* ret = new var(T);
	if (scope::current->m_id == scope::BLOCK) {
	  block* b = static_cast<block*>(scope::current);
	  b->m_vars.push_back(ret);
	}
	else
	  garbage.push_back(ret);
	code.push_back(new cast3ac(ret, src, T));
	return ret;
      }
      usr::flag_t flag = op->m_flag;
      if (flag & usr::VIRTUAL) {
	const pointer_type* pt = pointer_type::create(rec);
	var* ptr = new var(pt);
	if (scope::current->m_id == scope::BLOCK) {
	  block* b = static_cast<block*>(scope::current);
	  b->m_vars.push_back(ptr);
	}
	else
	  garbage.push_back(ptr);
	code.push_back(new addr3ac(ptr, src));
	var* func = call_impl::ref_vftbl(op, ptr);
	const type* T = op->m_type;
	assert(T->m_id == type::FUNC);
	typedef const func_type FT;
	FT* ft = static_cast<FT*>(T);
	return call_impl::common(ft, func, 0, 0, src, false, 0);
      }
      var* ret = call_impl::wrapper(op, 0, src);
      return ret->cast(T);
    }
  }  // end of namespace cast_impl
}  // end of namespace cxx_compiler

const cxx_compiler::type*
cxx_compiler::expressions::cast::valid(const type* T, var* y)
{
  if (assignment::valid(T, y, 0, true, 0))
    return T;
  const type* Tx = T->unqualified();
  const type* Ty = y->m_type->unqualified();
  if ( Tx->m_id == type::POINTER )
    return Ty->real() ? 0 : T;
  if ( Ty->m_id == type::POINTER )
    return Tx->real() ? 0 : T;
  if (Ty->m_id == type::RECORD) {
    typedef const record_type REC;
    REC* rec = static_cast<REC*>(Ty);
    return cast_impl::conversion_function(rec, T, true) ? T : 0;
  }
  return 0;
}

namespace cxx_compiler {
  namespace cast_impl {
    var* refaddr_case(const type* T, var* src)
    {
      if (T->m_id != type::REFERENCE)
	return 0;
      const type* Ty = src->m_type;
      if (Ty->m_id == type::REFERENCE)
	return 0;
      const type* R = src->result_type();
      if (Ty != R)
	return src;
      typedef const reference_type RT;
      RT* xrt = static_cast<RT*>(T);
      const type* Rx = xrt->referenced_type();
      Rx = Rx->unqualified();
      int offset = 0;
      if (Rx->m_id == type::RECORD) {
	typedef const record_type REC;
	REC* xrec = static_cast<REC*>(Rx);
	if (Ty->m_id != type::RECORD)
	  return 0;
	REC* yrec = static_cast<REC*>(Ty);
	vector<route_t> dummy;
	offset = calc_offset(yrec, xrec, dummy, 0);
	assert(offset >= 0);
      }
      var* ret = new refaddr(xrt, src, offset);
      garbage.push_back(ret);
      return ret;
    }
  } // end of namespace cast_impl
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::var::cast(const type* T)
{
  if (var* r = cast_impl::refaddr_case(T, this))
    return r;

  const type* U = m_type->unqualified();
  if (U->m_id == type::RECORD) {
    typedef const record_type REC;
    REC* rec = static_cast<REC*>(U);
    return cast_impl::conversion(rec, this, T);
  }

  vector<route_t> dummy;
  return cast_impl::with_route(T, this, dummy);
}

cxx_compiler::var* cxx_compiler::addrof::cast(const type* T)
{
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  if ( b && !expressions::constant_flag )
    return var::cast(T);
  if ( T == m_type )
    return this;
  else {
    var* ret = new addrof(T,m_ref,m_offset);
    garbage.push_back(ret);
    return ret;
  }
}

namespace cxx_compiler { namespace constant_impl {
  var* cast_ld(long double);
} } // end of namespace constant_impl and c_compiler

cxx_compiler::var* cxx_compiler::constant_impl::cast_ld(long double ld)
{
  using namespace expressions::primary::literal;
  if ( generator::long_double ){
    int sz = long_double_type::create()->size();
    unsigned char* p = new unsigned char[sz];
    (*generator::long_double->from_double)(p,ld);
    return floating::create(p);
  }
  else
    return floating::create(ld);
}

namespace cxx_compiler { namespace constant_impl {
  using namespace expressions::primary::literal;
  template<class T> var* cast(const type* Tx, constant<T>* y)
  {
    using namespace std;
    if (!cast_impl::require(Tx, y->m_type))
      return y;
    Tx = Tx->unqualified();
    switch (Tx->m_id) {
    case type::BOOL: return integer::create((bool)(y->m_value));
    case type::CHAR: return integer::create((char)(y->m_value));
    case type::SCHAR: return integer::create((signed char)(y->m_value));
    case type::UCHAR: return integer::create((unsigned char)(y->m_value));
    case type::WCHAR: return integer::create((wchar_t)(y->m_value));
    case type::SHORT: return integer::create((short int)(y->m_value));
    case type::USHORT: return integer::create((unsigned short int)(y->m_value));
    case type::INT: return integer::create((int)(y->m_value));
    case type::UINT: return integer::create((unsigned int)(y->m_value));
    case type::LONG:
      {
        typedef long int X;
        if (Tx->size() <= sizeof(X))
          return integer::create((X)(y->m_value));
        typedef long long int XX;
        assert(Tx->size() == sizeof(XX));
        usr* ret = integer::create((XX)(y->m_value));
        ret->m_type = const_type::create(long_type::create());
        ret->m_flag = usr::SUB_CONST_LONG;
        return ret;
      }
    case type::ULONG:
      {
        typedef unsigned long int X;
        if (Tx->size() <= sizeof(X))
          return integer::create((X)(y->m_value));
        typedef unsigned long long int XX;
        assert(Tx->size() == sizeof(XX));
        usr* ret = integer::create((XX)(y->m_value));
        ret->m_type = const_type::create(ulong_type::create());
        ret->m_flag = usr::SUB_CONST_LONG;
        return ret;
      }
    case type::LONGLONG: return integer::create((__int64)(y->m_value));
    case type::ULONGLONG:
      return integer::create((unsigned __int64)(y->m_value));
    case type::FLOAT: return floating::create((float)y->m_value);
    case type::DOUBLE: return floating::create((double)y->m_value);
    case type::LONG_DOUBLE: return cast_ld(y->m_value);
    case type::POINTER:
      {
        if (sizeof(void*) >= Tx->size())
          return pointer::create(Tx,(void*)y->m_value);
        else
          return pointer::create(Tx,(__int64)y->m_value);
      }
    case type::ENUM:
      {
        typedef const enum_type ET;
        ET* et = static_cast<ET*>(Tx);
        return cast(et->get_integer(),y);
      }
    default:
      return Tx->aggregate() ? aggregate_conv(Tx, y) : y->var::cast(Tx);
    }
  }
  template<class T> var* fcast(const type* Tx, constant<T>* y)
  {
    using namespace std;
    if (!cast_impl::require(Tx, y->m_type))
      return y;
    Tx = Tx->unqualified();    
    switch (Tx->m_id) {
    case type::BOOL: return integer::create((bool)(y->m_value));
    case type::CHAR: return integer::create((char)(y->m_value));
    case type::SCHAR: return integer::create((signed char)(y->m_value));
    case type::UCHAR: return integer::create((unsigned char)(y->m_value));
    case type::WCHAR: return integer::create((wchar_t)(y->m_value));
    case type::SHORT: return integer::create((short int)(y->m_value));
    case type::USHORT: return integer::create((unsigned short int)(y->m_value));
    case type::INT: return integer::create((int)(y->m_value));
    case type::UINT: return integer::create((unsigned int)(y->m_value));
    case type::LONG:
      {
        typedef long int X;
        if (Tx->size() <= sizeof(X))
          return integer::create((X)(y->m_value));
        typedef long long int XX;
        assert(Tx->size() == sizeof(XX));
        usr* ret = integer::create((XX)(y->m_value));
        ret->m_type = const_type::create(long_type::create());
        ret->m_flag = usr::SUB_CONST_LONG;
        return ret;
      }
    case type::ULONG:
      {
        typedef unsigned long int X;
        if (Tx->size() <= sizeof(X))
          return integer::create((X)(y->m_value));
        typedef unsigned long long int XX;
        assert(Tx->size() == sizeof(XX));
        usr* ret = integer::create((XX)(y->m_value));
        ret->m_type = const_type::create(ulong_type::create());
        ret->m_flag = usr::SUB_CONST_LONG;
        return ret;
      }
    case type::LONGLONG: return integer::create((__int64)(y->m_value));
    case type::ULONGLONG:
      return integer::create((unsigned __int64)(y->m_value));
    case type::FLOAT: return floating::create((float)y->m_value);
    case type::DOUBLE: return floating::create((double)y->m_value);
    case type::LONG_DOUBLE: return cast_ld(y->m_value);
    case type::ENUM:
      {
        typedef const enum_type ET;
        ET* et = static_cast<ET*>(Tx);
        return fcast(et->get_integer(),y);
      }
    default:
      return Tx->aggregate() ? aggregate_conv(Tx, y) : y->var::cast(Tx);
    }
  }
  template<class T> var* pcast(const type* Tx, constant<T>* y)
  {
    using namespace std;
    if (!cast_impl::require(Tx, y->m_type))
      return y;
    Tx = Tx->unqualified();    
    switch (Tx->m_id) {
    case type::BOOL: return integer::create((bool)(__int64)y->m_value);
    case type::CHAR: return integer::create((char)(__int64)y->m_value);
    case type::SCHAR: return integer::create((signed char)(__int64)y->m_value);
    case type::UCHAR:
      return integer::create((unsigned char)(__int64)y->m_value);
    case type::WCHAR: return integer::create((wchar_t)(__int64)y->m_value);
    case type::SHORT: return integer::create((short int)(__int64)y->m_value);
    case type::USHORT:
      return integer::create((unsigned short int)(__int64)y->m_value);
    case type::INT: return integer::create((int)(__int64)(y->m_value));
    case type::UINT:
      return integer::create((unsigned int)(__int64)(y->m_value));
    case type::LONG:
      {
        typedef long int X;
        if (Tx->size() <= sizeof(X))
          return integer::create((X)(y->m_value));
        typedef long long int XX;
        assert(Tx->size() == sizeof(XX));
        usr* ret = integer::create((XX)(y->m_value));
        ret->m_type = const_type::create(long_type::create());
        ret->m_flag = usr::SUB_CONST_LONG;
        return ret;
      }
    case type::ULONG:
      {
        typedef unsigned long int X;
        if (Tx->size() <= sizeof(X))
          return integer::create((X)(y->m_value));
        typedef unsigned long long int XX;
        assert(Tx->size() == sizeof(XX));
        usr* ret = integer::create((XX)(y->m_value));
        ret->m_type = const_type::create(ulong_type::create());
        ret->m_flag = usr::SUB_CONST_LONG;
        return ret;
      }
    case type::LONGLONG: return integer::create((__int64)(y->m_value));
    case type::ULONGLONG: return integer::create((unsigned __int64)y->m_value);
    case type::POINTER:
      {
        if (sizeof(void*) >= Tx->size())
          return pointer::create(Tx,(void*)y->m_value);
        else
          return pointer::create(Tx,(__int64)y->m_value);
      }
    case type::ENUM:
      {
        typedef const enum_type ET;
        ET* et = static_cast<ET*>(Tx);
        return pcast(et->get_integer(),y);
      }
    default:
      return y->var::cast(Tx);
    }
  }
#ifdef _MSC_VER
  template<> var* cast(const type* Tx, constant<unsigned __int64>* y)
  {
    using namespace std;
    if (!cast_impl::require(Tx, y->m_type)) 
      return y;
    Tx = Tx->unqualified();
    switch (Tx->m_id) {
    case type::BOOL: return integer::create((bool)(y->m_value));
    case type::CHAR: return integer::create((char)(y->m_value));
    case type::SCHAR: return integer::create((signed char)(y->m_value));
    case type::UCHAR: return integer::create((unsigned char)(y->m_value));
    case type::WCHAR: return integer::create((wchar_t)(y->m_value));
    case type::SHORT: return integer::create((short int)(y->m_value));
    case type::USHORT: return integer::create((unsigned short int)(y->m_value));
    case type::INT: return integer::create((int)(y->m_value));
    case type::UINT: return integer::create((unsigned int)(y->m_value));
    case type::LONG:
      {
        typedef long int X;
        if (Tx->size() <= sizeof(X))
          return integer::create((X)(y->m_value));
        typedef long long int XX;
        assert(Tx->size() == sizeof(XX));
        usr* ret = integer::create((XX)(y->m_value));
        ret->m_type = const_type::create(long_type::create());
        ret->m_flag = usr::SUB_CONST_LONG;
        return ret;
      }
    case type::ULONG:
      {
        typedef unsigned long int X;
        if (Tx->size() <= sizeof(X))
          return integer::create((X)(y->m_value));
        typedef unsigned long long int XX;
        assert(Tx->size() == sizeof(XX));
        usr* ret = integer::create((XX)(y->m_value));
        ret->m_type = const_type::create(ulong_type::create());
        ret->m_flag = usr::SUB_CONST_LONG;
        return ret;
      }
    case type::LONGLONG: return integer::create((__int64)(y->m_value));
    case type::ULONGLONG:
      return integer::create((unsigned __int64)(y->m_value));
    case type::FLOAT: return floating::create((float)(__int64)y->m_value);
    case type::DOUBLE: return floating::create((double)(__int64)y->m_value);
    case type::LONG_DOUBLE: return cast_ld((__int64)y->m_value);
    case type::POINTER:
      {
        if (sizeof(void*) >= Tx->size())
          return pointer::create(Tx,(void*)y->m_value);
        else
          return pointer::create(Tx,(__int64)y->m_value);
      }
    case type::ENUM:
      {
        typedef const enum_type ET;
        ET* et = static_cast<ET*>(Tx);
        return cast(et->get_integer(),y);
      }
    default:
        return y->var::cast(Tx);
    }
  }
#endif // _MSC_VER
} } // end of namespace constant_impl and cxx_compmiler

namespace cxx_compiler {
  template<>
  var* constant<bool>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<char>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<signed char>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<unsigned char>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<wchar_t>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<short int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<>
  var* constant<unsigned short int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  var* constant<int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  var* constant<unsigned int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  var* constant<long int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  var* constant<unsigned long int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  var* constant<__int64>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  var* constant<unsigned __int64>::cast(const type* type)
  { return constant_impl::cast(type,this); }
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::constant<float>::cast(const type* type)
{ return constant_impl::fcast(type,this); }
cxx_compiler::var* cxx_compiler::constant<double>::cast(const type* type)
{ return constant_impl::fcast(type,this); }
cxx_compiler::var* cxx_compiler::constant<long double>::cast(const type* T)
{
  using namespace expressions::primary::literal;
  if ( generator::long_double ){
    T = T->unqualified();
    if (T->m_id == type::LONG_DOUBLE)
      return this;
    else {
      double d = (*generator::long_double->to_double)(b);
      usr* tmp = floating::create(d);
      return T->aggregate() ? aggregate_conv(T, tmp) : tmp->cast(T);
    }
  }
  else
    return constant_impl::fcast(T,this);
}
cxx_compiler::var* cxx_compiler::constant<void*>::cast(const type* type)
{ return constant_impl::pcast(type,this); }
