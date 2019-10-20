#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

const cxx_compiler::type* cxx_compiler::declarations::declarators::pointer::
action(std::vector<int>* p, bool pm)
{
  using namespace std;
  auto_ptr<vector<int> > sweeper(p);
  const type* T = backpatch_type::create();
  if (pm) {
    if (scope::current->m_id != scope::TAG)
      error::not_implemented();
    tag* ptr = static_cast<tag*>(scope::current);
    T = pointer_member_type::create(ptr, T);
  }
  else
    T = pointer_type::create(T);

  if ( p ){
    const vector<int>& v = *p;
    if ( find(v.begin(),v.end(),CONST_KW) != v.end() )
      T = const_type::create(T);
    if ( find(v.begin(),v.end(),VOLATILE_KW) != v.end() )
      T = volatile_type::create(T);
    if ( find(v.begin(),v.end(),RESTRICT_KW) != v.end() )
      T = restrict_type::create(T);
  }
  return T;
}

const cxx_compiler::type*
cxx_compiler::declarations::declarators::pointer::action(const type* X,
							 const type* T)
{
  if (T->backpatch())
    return T->patch(X, 0);

  if (specifier_seq::info_t::s_stack.empty())
    return T;
  specifier_seq::info_t* p = specifier_seq::info_t::s_stack.top();
  p->update();
  assert(X->backpatch());
  p->m_type = X->patch(p->m_type, 0);
  return T;
}

const cxx_compiler::type* cxx_compiler::declarations::declarators::reference::action()
{
  return reference_type::create(backpatch_type::create());
}

const cxx_compiler::type* cxx_compiler::declarations::declarators::
function::action(const type* T,
                 std::vector<std::pair<const type*, expressions::base*>*>* pdc,
                 var* v,
                 std::vector<int>* cvr)
{
  using namespace std;
  if (pdc)
    parse::context_t::clear();  
  auto_ptr<vector<pair<const type*, expressions::base*>*> > sweeper1(pdc);
  auto_ptr<vector<int> > sweeper2(cvr);
  usr* u = v ? v->usr_cast() : 0;
  if (u) {
    usr::flag_t flag = u->m_flag;
    if (flag & usr::OVERLOAD)
      return T;
  }

  if (!u && v) {
    /*
     Definition like
     struct S { void f(); };
     void S::f(){ ... }
     where, `f' is lookuped
     */
    genaddr* ga = v->genaddr_cast();
    v = ga->m_ref;
    u = v->usr_cast();
    return u->m_type;
  }
  if (pdc) {
    vector<const type*> param;
    transform(begin(*pdc), end(*pdc), back_inserter(param),
	      [](pair<const type*, expressions::base*>* p)
	      { return p->first; });
    const type* ret =
      T->patch(func_type::create(backpatch_type::create(),param),u);
    vector<var*> default_arg;
    transform(begin(*pdc), end(*pdc), back_inserter(default_arg),
	      [](pair<const type*, expressions::base*>* p)
	      {
		expressions::base* bp = p->second;
		auto_ptr<expressions::base> sweeper(bp);
		if (!bp)
		  return (var*)0;
		var* v = bp->gen();
		const type* T = p->first;
		return v->cast(T);
	      });
    typedef vector<var*>::const_iterator IT;
    IT p = find_if(begin(default_arg), end(default_arg),
		   [](var* v){ return v; });
    if (p != end(default_arg)) {
      if (u) {
	u->m_flag = usr::flag_t(u->m_flag | usr::HAS_DEFAULT_ARG);
	default_arg_table[u] = default_arg;
      }
      else
	error::not_implemented();
    }
    return ret;
  }
  else {
    vector<const type*> param;
    param.push_back(void_type::create());
    return T->patch(func_type::create(backpatch_type::create(),param),u);
  }
}

namespace cxx_compiler {
  namespace declarations {
    namespace declarators {
      namespace function {
	using namespace std;
	map<usr*, vector<var*> > default_arg_table;
      } // end of namespace function
    } // end of namespace declarators
  } // end of namespace declarations
} // end of namespace cxx_compiler

const cxx_compiler::type* cxx_compiler::declarations::declarators::
function::parameter(specifier_seq::info_t* p, var* v)
{
  using namespace std;
  assert(!v || v->usr_cast());
  usr* u = static_cast<usr*>(v);
  struct sweeper2 {
    ~sweeper2()
    {
      if ( cxx_compiler_char == ',' )
        cxx_compiler::declarations::specifier_seq::info_t::clear();
    }
  } sweeper2;
  auto_ptr<specifier_seq::info_t> sweepr(p);
  parse::context_t::clear();
  usr::flag_t mask =
    usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO);
  typedef const pointer_type PT;
  if (u) {
    usr::flag_t flag = u->m_flag = p->m_flag;
    const type* T = u->m_type;
    if (PT* pt = T->ptr_gen())
      u->m_type = pt;
    if (flag & mask) {
      using namespace error::declarations::declarators::function;
      parameter::invalid_storage(parse::position,u);
      flag = u->m_flag = p->m_flag = usr::flag_t(flag & ~mask);
    }
    u = declarations::action1(u, false);
    T = u->m_type;
    if (PT* pt = T->ptr_gen()) {
      T = u->m_type = pt;
      u->m_flag = usr::flag_t(u->m_flag & ~(usr::FUNCTION | usr::VL));
    }
    return T;
  }
  else {
    if ( !p->m_type || !p->m_tmp.empty() )
      p->update();
    usr::flag_t flag = p->m_flag;
    if (flag & mask) {
      using namespace error::declarations::declarators::function;
      parameter::invalid_storage(parse::position, 0);
      flag = p->m_flag = usr::flag_t(flag & ~mask);
    }
    const type* T = p->m_type;
    if (PT* pt = T->ptr_gen())
      T = pt;
    if (T->size()) {
      string name = new_name(".param");
      usr* u = new usr(name, T, flag, file_t(), usr::NONE2);
      scope::current->m_usrs[name].push_back(u);
      scope::current->m_order.push_back(u);
    }
    return T;
  }
}

const cxx_compiler::type* cxx_compiler::declarations::declarators::
function::parameter(specifier_seq::info_t* p, const type* T)
{
  using namespace std;
  struct sweeper2 {
    ~sweeper2()
    {
      if ( cxx_compiler_char == ',' )
        cxx_compiler::declarations::specifier_seq::info_t::clear();
    }
  } sweeper2;
  auto_ptr<specifier_seq::info_t> sweeper(p);
  parse::context_t::clear();
  string name = new_name(".param");
  usr* u = new usr(name,T,usr::NONE,parse::position,usr::NONE2);
  typedef const pointer_type PT;
  if ( PT* pt = T->ptr_gen() )
    u->m_type = pt;
  usr::flag_t& flag = u->m_flag = p->m_flag;
  usr::flag_t mask =
    usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO);
  if (flag & mask) {
    using namespace error::declarations::declarators::function;
    parameter::invalid_storage(parse::position,u);
    flag = usr::flag_t(flag & ~mask);
  }
  u = declarations::action1(u,false);
  T = u->m_type;
  if (PT* pt = T->ptr_gen()) {
    T = u->m_type = pt;
    u->m_flag = usr::flag_t(u->m_flag & ~(usr::FUNCTION | usr::VL));
  }
  return T;
}

namespace cxx_compiler {
  namespace declarations {
    namespace declarators {
      using namespace std;
      namespace array {
        namespace variable_length {
          map<var*, vector<tac*> > dim_code;
          void destroy_tmp()
          {
            for (auto p : dim_code)
              for (auto q : p.second)
                delete q;
            dim_code.clear();
          }
        } // end of namespace variable_length
      } // end of namespace array 
      namespace array_impl {
        bool inblock(scope* ptr)
        {
          assert(ptr);
          switch (ptr->m_id) {
          case scope::NONE:
            return false;
          case scope::BLOCK:
            return true;
          default:
            assert(ptr->m_id == scope::PARAM);
            return inblock(ptr->m_parent);
          }
        }
        bool inblock_param(scope* ptr)
        {
          assert(ptr);
          if (ptr->m_id == scope::PARAM)
            return inblock(ptr->m_parent);
          else
            return false;
        }
      } // end of namespace array_impl
    } // end of namespace declarators
  } // end of namespace declarations
} // end of namespace cxx_compiler

const cxx_compiler::type*
cxx_compiler::declarations::declarators::array::action(const type* T,
                                                       expressions::base* expr,
                                                       bool asterisc,
                                                       var* v)
{
  using namespace std;
  using namespace error::declarations::declarators::array;
  usr* u = static_cast<usr*>(v);
  auto_ptr<expressions::base> sweepr(expr);
  const type* bt = backpatch_type::create();
  int n = code.size();
  v = 0;
  if ( expr )
    v = expr->gen();

  int dim = 0;
  if ( v ){
    v = v->rvalue();
    if ( !v->m_type->integer() ){
      not_integer(parse::position,u);
      v = v->cast(int_type::create());
    }
    if ( !v->isconstant() ){
      if (scope::current != &scope::root) {
        var* tmp = new var(v->m_type);
        if (v->lvalue()) {
          if (scope::current->m_id == scope::BLOCK) {
            block* b = static_cast<block*>(scope::current);
            tmp->m_scope = b;
            b->m_vars.push_back(tmp);
          }
          else
            garbage.push_back(tmp);
          code.push_back(new assign3ac(tmp, v));
          v = tmp;
        }
        if (array_impl::inblock_param(scope::current)) {
          for_each(code.begin()+n, code.end(), [](tac* p){ delete p; });
          code.resize(n);
          return T->patch( array_type::create(bt, 0), u);
        }
        copy(code.begin()+n, code.end(),
             back_inserter(variable_length::dim_code[v]));
        code.resize(n);
        return T->patch(varray_type::create(bt, v), u);
      }
      using namespace error::declarations::declarators::vm;
      file_scope(u);
      using namespace expressions::primary::literal;
      v = integer::create(1);
    }
    dim = v->value();
    if ( dim <= 0 ){
      not_positive(parse::position,u);
      dim = 1;
    }
  }
  else if ( asterisc ){
    if ( scope::current->m_id == scope::BLOCK || scope::current == &scope::root ){
      using namespace error::declarations::declarators::array;
      asterisc_dimension(parse::position,u);
    }
  }
  return T->patch(array_type::create(bt,dim),u);
}

void
cxx_compiler::declarations::declarators::array::variable_length::allocate(usr* u)
{
  using namespace std;
  if ( scope::current == &scope::root )
    return;
  u->m_type->decide_dim();
  usr::flag_t flag = u->m_flag;
  if (!(flag & usr::VL))
    return;
  usr::flag_t mask = usr::flag_t(usr::STATIC | usr::EXTERN);
  if (flag & mask) {
    using namespace error::declarations::declarators::array::variable_length;
    invalid_storage(u);
    flag = u->m_flag = usr::flag_t(flag & ~mask);
  }
  statements::label::vm.push_back(u);
  if (flag & usr::TYPEDEF)
    return;
  const type* T = u->m_type;
  var* size = T->vsize();
  code.push_back(new alloca3ac(u,size));
}

cxx_compiler::fundef* cxx_compiler::fundef::current;

namespace cxx_compiler {
  namespace declarations {
    namespace declarators {
      namespace function {
	namespace definition {
	  bool valid(const type*, usr*);
	  inline bool include_this(const vector<usr*>& order)
	  {
	    if (order.empty())
	      return false;
	    usr* u = order[0];
	    return u->m_name == this_name;
	  }
	  bool match(const usr* u, const vector<usr*>& order)
	  {
	    const type* T = u->m_type;
	    assert(T->m_id == type::FUNC);
	    typedef const func_type FT;
	    FT* ft = static_cast<FT*>(T);
	    const vector<const type*>& param = ft->param();
	    bool inc = include_this(order);
	    int n = inc ? 1 : 0;
	    if (order.empty() || inc && order.size() == 1) {
	      if (param.size() != 1)
		return false;
	      const type* T = param.back();
	      return T->m_id == type::VOID;
	    }
	    if (param.size() != order.size() - n)
	      return false;
	    typedef vector<const type*>::const_iterator ITx;
	    typedef vector<usr*>::const_iterator ITy;
	    pair<ITx, ITy> ret = 
	      mismatch(begin(param), end(param), begin(order) + n,
		       [](const type* T, const usr* u)
		       { return compatible(T, u->m_type); });
	    return ret == make_pair(end(param), end(order));
	  }
	  inline bool check_now(usr* fun)
	  {
	    scope* p = fun->m_scope;
	    if (p->m_id != scope::TAG)
	      return true;
	    tag* ptr = static_cast<tag*>(p);
	    return ptr->m_types.second;
	  }
	} // end of namespace definition
      } // end of namespace function
    } // end of namespace declarators
  } // end of namespace declarations
} // end of namespace cxx_compiler

void
cxx_compiler::declarations::declarators::
function::definition::begin(declarations::specifier_seq::info_t* p, var* v)
{
  using namespace std;
  usr* u = v->usr_cast();
  if (!u) {
    /*
     Definition like
     struct S { void f(); };
     void S::f(){ ... }
     where, `f' is lookuped
     */
    genaddr* ga = v->genaddr_cast();
    var* ref = ga->m_ref;
    u = ref->usr_cast();
  }
  usr::flag2_t flag2 = u->m_flag2;
  if (flag2 & usr::CONV_OPE)
    p = declarations::specifier_seq::info_t::s_stack.top();
  parse::identifier::mode = parse::identifier::look;
  u = declarations::action1(u,false);
  usr::flag_t flag = u->m_flag;
  auto_ptr<declarations::specifier_seq::info_t>
    sweeper((flag & usr::CTOR) ? 0 : p);
  vector<scope*>& children = scope::current->m_children;
  if (children.empty()) {
    using namespace error::declarations::declarators::function::definition;
    invalid(parse::position);
    scope* param = new scope(scope::PARAM);
    using namespace class_or_namespace_name;
    assert(!before.empty());
    assert(before.back() == param);
    before.pop_back();
    param->m_parent = scope::current;
    children.push_back(param);
  }
  scope* param = children.back();
  const vector<usr*>& order = param->m_order;
  if (check_now(u))
    for_each(order.begin(),order.end(),check_object);
  if (flag & usr::OVERLOAD) {
    overload* ovl = static_cast<overload*>(u);
    const vector<usr*>& c = ovl->m_candidacy;
    typedef vector<usr*>::const_iterator IT;
    IT p = find_if(begin(c), end(c),
		   [order](const usr* u){ return match(u, order); });
    assert(p != end(c));
    u = *p;
  }
  fundef::current = new fundef(u,param);
  {
    string name = u->m_name;
    if (flag & usr::TYPEDEF) {
      using namespace error::declarations::declarators::function::definition;
      typedefed(parse::position);
      u->m_flag = flag = usr::flag_t(flag & ~usr::TYPEDEF);
    }
    const type* T = u->m_type;
    if (T->m_id != type::FUNC)
      return;
    typedef const func_type FT;
    FT* ft = static_cast<FT*>(T);
    T = ft->return_type();
    if (!valid(T,u)) {
      using namespace error::declarations::declarators::function::definition;
      invalid_return(fundef::current->m_usr,T);
    }
    const vector<const type*>& param = ft->param();
    scope* ptr = u->m_scope;
    if (ptr->m_id == scope::BLOCK)
      ptr = &scope::root;
    key_t key(name, ptr, &param, get_seed(u));
    table_t::const_iterator p = dtbl.find(key);
    if (p != dtbl.end()) {
      using namespace error::declarations::declarators::function::definition;
      multiple(parse::position,p->second);
    }
    else
      dtbl[key] = u;
  }
}

bool
cxx_compiler::declarations::declarators::function::definition::valid(const type* T, usr* func)
{
  if (!T)
    return true;
  T = T->unqualified();
  if (T->m_id == type::VOID)
    return true;
  if (!T->size()) {
    scope* ptr = func->m_scope;
    if ( ptr->m_id != scope::TAG )
      return false;
    tag* ptag = static_cast<tag*>(ptr);
    if ( T->m_id != type::INCOMPLETE_TAGGED )
      return false;
    typedef const incomplete_tagged_type IT;
    IT* it = static_cast<IT*>(T);
    return ptag == it->get_tag();
  }
  return true;
}

namespace cxx_compiler {
  namespace class_or_namespace_name {
    inline void pop(scope* param)
    {
      if (!before.empty()) {
	if (before.back() == param)
	  before.pop_back();
      }
    }
  } // end of namespace class_or_namespace_name
  namespace declarations {
    namespace declarators {
      namespace function {
        namespace definition {
          inline void erase(scope* param)
          {
            assert(param->m_id == scope::PARAM);
            vector<scope*>& children = param->m_parent->m_children;
            typedef vector<scope*>::reverse_iterator IT;
            IT p = find(rbegin(children), rend(children), param);
            assert(p != rend(children));
            vector<scope*>::iterator q = p.base() - 1;
            children.erase(q);
	    class_or_namespace_name::pop(param);
          }
          namespace static_inline {
            using namespace std;
            extern void remember(fundef*, vector<tac*>&);
            skip::table_t skip::stbl;
            namespace defer {
              map<key_t, vector<ref_t> > refs;
              map<key_t, set<usr*> > callers;
              map<usr*, vector<int> > positions;
            }  // end of namespace defer
          }  // end of namespace static_inline
        }  // end of namespace definition
      }  // end of namespace function
    }  // end of namespace declarators
  }  // end of namespace declarations
}  // end of namespace cxx_compiler

namespace cxx_compiler {
  std::vector<FUNCS_ELEMENT_TYPE> funcs;
} // end of namespace cxx_compiler

namespace cxx_compiler {
  inline usr* get_this(scope* param, const record_type* rec)
  {
    map<string, vector<usr*> >& usrs = param->m_usrs;
    typedef map<string, vector<usr*> >::const_iterator IT;
    IT p = usrs.find(this_name);
    assert(p != usrs.end());
    const vector<usr*>& v = p->second;
    assert(v.size() == 1);
    return v.back();
  }
  inline void add_tor_code(fundef* fdef, bool is_dtor)
  {
    usr* u = fdef->m_usr;
    scope* param = fdef->m_param;
    scope* p = u->m_scope;
    assert(p->m_id == scope::TAG);
    tag* ptr = static_cast<tag*>(p);
    const type* T = ptr->m_types.second;
    assert(T->m_id == type::RECORD);
    typedef const record_type REC;
    REC* rec = static_cast<REC*>(T); 
    usr* this_ptr = get_this(param, rec);
    const vector<scope*>& c = param->m_children;
    assert(c.size() == 1);
    scope* q = c.back();
    assert(q->m_id == scope::BLOCK);
    block* b = static_cast<block*>(q);
    scope* org = scope::current;
    scope::current = b;
    vector<REC*> dummy;
    rec->tor_code(u, param, this_ptr, b, is_dtor, dummy);
    scope::current = org;
  }
  inline bool tor_of_vbase(usr* func)
  {
    usr::flag_t flag = func->m_flag;
    usr::flag_t mask = usr::flag_t(usr::CTOR | usr::DTOR);
    if (!(flag & mask))
      return false;
    scope* p = func->m_scope;
    assert(p->m_id == scope::TAG);
    tag* ptr = static_cast<tag*>(p);
    vector<base*>* bases = ptr->m_bases;
    if (!bases)
      return false;
    typedef vector<base*>::const_iterator IT;
    IT it = find_if(begin(*bases), end(*bases),
		    [](const base* bp){ return bp->m_flag & usr::VIRTUAL; });
    return it != end(*bases);
  }
  inline void copy_usrs(const map<string, vector<usr*> >& src,
			map<string, vector<usr*> >& dst, map<var*, var*>& tbl)
  {
    for (const auto& p : src) {
      for (auto x : p.second) {
	usr* y = new usr(*x);
	dst[y->m_name].push_back(y);
	tbl[x] = y;
      }
    }
  }
  inline void copy_order(const vector<usr*>& src,
			 vector<usr*>& dst, const map<var*, var*>& tbl)
  {
    for (auto x : src) {
      map<var*, var*>::const_iterator p = tbl.find(x);
      assert(p != tbl.end());
      var* v = p->second;
      assert(v->usr_cast());
      usr* y = static_cast<usr*>(v);
      dst.push_back(y);
    }
  }
  inline void copy_vars(const vector<var*>& src,
			vector<var*>& dst, map<var*, var*>& tbl)
  {
    for (auto x : src) {
      var* y = new var(*x);
      dst.push_back(y);
      tbl[x] = y;
    }
  }
  struct copy_recursively {
    scope* m_dst;
    map<var*, var*>& m_tbl;
    copy_recursively(scope* dst, map<var*, var*>& tbl)
      : m_dst(dst), m_tbl(tbl) {}
    void operator()(scope* p)
    {
      assert(p->m_id == scope::BLOCK);
      using namespace class_or_namespace_name;
      block* bp = new block;
      assert(before.back() == bp);
      before.pop_back();
      m_dst->m_children.push_back(bp);
      bp->m_parent = m_dst;
      copy_scope(p, bp, m_tbl);
    }
  };
  void copy_scope(const scope* src, scope* dst, map<var*, var*>& tbl)
  {
    const map<string, vector<usr*> >& x = src->m_usrs;
    map<string, vector<usr*> >& y = dst->m_usrs;
    copy_usrs(x, y, tbl);
    const vector<usr*>& xx = src->m_order;
    vector<usr*>& yy = dst->m_order;
    copy_order(xx, yy, tbl);
    if (src->m_id == scope::BLOCK) {
      const block* bx = static_cast<const block*>(src);
      assert(dst->m_id == scope::BLOCK);
      block* by = static_cast<block*>(dst);
      const vector<var*>& x = bx->m_vars;
      vector<var*>& y = by->m_vars;
      copy_vars(x, y, tbl);
    }
    const vector<scope*>& children = src->m_children;
    for_each(begin(children), end(children), copy_recursively(dst, tbl));
  }
  inline void save_tor_body(fundef* fdef, int n)
  {
    usr* func = fdef->m_usr;
    string name = func->m_name + dot_body;

    const type* T = func->m_type;
    assert(T->m_id == type::FUNC);
    typedef const func_type FT;
    FT* ft = static_cast<FT*>(T);

    scope* p = func->m_scope;
    assert(p->m_id == scope::TAG);
    tag* ptr = static_cast<tag*>(p);
    usr::flag2_t flag2 = usr::flag2_t(usr::GENED_BY_COMP | usr::TOR_BODY);
    usr* u = new usr(name, ft, func->m_flag, parse::position, flag2);
    map<string, vector<usr*> >& pusrs = ptr->m_usrs;
    typedef map<string, vector<usr*> >::const_iterator ITx;
    ITx itx = pusrs.find(name);
    if (itx == pusrs.end())
      pusrs[name].push_back(u);
    else {
      const vector<usr*>& v = itx->second;
      usr* prev = v.back();
      overload* ovl = new overload(prev, u);
      pusrs[name].push_back(u);
      pusrs[name].push_back(ovl);
    }

    using namespace declarations::declarators::function::definition;
    const vector<const type*>& parameter = ft->param();
    key_t key(name, ptr, &parameter, get_seed(func));
    dtbl[key] = u;

    using namespace class_or_namespace_name;
    scope* param = new scope(scope::PARAM);
    assert(before.back() == param);
    before.pop_back();
    ptr->m_children.push_back(param);
    param->m_parent = ptr;

    map<var*, var*> tbl;
    copy_scope(fdef->m_param, param, tbl);

    vector<tac*> tmp;
    transform(begin(code)+n, end(code), back_inserter(tmp), new3ac(tbl));

    fundef* org = fundef::current;
    fundef::current = new fundef(u, param);
    declarations::declarators::
      function::definition::action(fundef::current, tmp);
    delete fundef::current;
    fundef::current = org;
  }
} // end of namespace cxx_compiler

void cxx_compiler::declarations::declarators::function::definition::
action(statements::base* stmt)
{
  using namespace std;
  auto_ptr<statements::base> sweeper(stmt);
  usr* u = fundef::current->m_usr;
  using namespace parse::member_function_body;
  const map<usr*, save_t>& tbl = parse::member_function_body::stbl;
  map<usr*, save_t>::const_iterator p = tbl.find(u);
  if (p != tbl.end() && !parse::member_function_body::saved) {
    // member function body is already saved
    const save_t& tmp = p->second;
    scope* param = tmp.m_param;
    scope* ptr = param->m_parent;
    assert(ptr->m_id == scope::TAG);
    vector<scope*>& children = ptr->m_children;
    typedef vector<scope*>::iterator IT;
    IT it = find(begin(children), end(children), param);
    assert(it != end(children));
    children.erase(it);
  }
  else {
    using namespace mem_initializer;
    typedef map<usr*, VALUE>::iterator IT;
    IT p = for_parse.find(u);
    if (p != for_parse.end()) {
      VALUE& v = p->second;
      for_each(begin(v), end(v), [](const pair<PAIR*, EXPRS*>& x)
               { mem_initializer::action(x.first, x.second); });
      for_parse.erase(p);
    }
    file_t org = parse::position;
    usr::flag_t flag = u->m_flag;
    if (flag & usr::CTOR)
      add_tor_code(fundef::current, false);
    int n = code.size();
    stmt->gen();
    if (tor_of_vbase(u))
      save_tor_body(fundef::current, n);
    if (flag & usr::DTOR)
      add_tor_code(fundef::current, true);
    parse::position = org;
    statements::label::check();
    statements::label::clear();
    action(fundef::current, code);
    if (scope::current->m_id == scope::TAG) {
      scope::current = scope::current->m_parent;
      if (!(flag & usr::INLINE))
        destroy();
    }
    else
      destroy();
  }
  delete fundef::current;
  fundef::current = 0;
}

void cxx_compiler::declarations::declarators::
function::definition::action(fundef* fdef, std::vector<tac*>& vc)
{
  using namespace std;
  using namespace static_inline;
  usr* u = fdef->m_usr;
  usr::flag2_t flag2 = u->m_flag2;
  if (flag2 & usr::TEMPLATE) {
    assert(fundef::current == fdef);
    using namespace parse::templ;
    assert(!save_t::s_stack.empty());
    save_t* p = save_t::s_stack.top();
    assert(p->m_usr == u);
    scope* param = fdef->m_param;
    vector<scope*>& children = param->m_parent->m_children;
    assert(children.back() == param);
    children.pop_back();
    class_or_namespace_name::pop(param);
    return;
  }
  if (!error::counter && cmdline::optimize_level >= 1)
    optimize::action(fdef, vc);
  usr::flag_t flag = u->m_flag;
  usr::flag_t mask = usr::flag_t(usr::VIRTUAL | usr::OVERRIDE);
  if ((flag & usr::INLINE) && !(flag & mask))
    return skip::add(fdef, vc, true);
  if (flag & usr::STATIC) {
    usr::flag2_t flag2 = u->m_flag2;
    usr::flag2_t mask2 =
      usr::flag2_t(usr::INITIALIZE_FUNCTION | usr::TERMINATE_FUNCTION);
    if (!(flag2 & mask2)) {
      if (u->m_scope->m_id != scope::TAG)
	return skip::add(fdef, vc, true);
    }
  }

  skip::chk_t arg(fdef);
  for_each(vc.begin(), vc.end(), bind2nd(ptr_fun(skip::check), &arg));
  if (arg.m_wait_inline)
    return skip::add(fdef, vc, false);

  if ( cmdline::output_medium ){
    if ( cmdline::output_optinfo )
      cout << '\n' << "After optimization" << '\n';
    scope* org = scope::current;
    scope::current = &scope::root;
    cout << dump::names::ref(u) << ":\n";
    scope::current = org;
    for (auto p : vc)
      cout << '\t', dump::tacx(cout, p), cout << '\n';
    cout << '\n';
    dump::scopex();
  }

  using namespace mem_initializer;
  if (!error::counter) {
    if (generator::generate) {
      generator::interface_t tmp = {
        &scope::root,
        fdef,
        &vc
      };
      generator::generate(&tmp);
      erase(fdef->m_param);
      if (mtbl.find(fdef->m_usr) == mtbl.end())
	delete fdef->m_param;
      for (tac* p : vc)
	delete p;
      vc.clear();
    }
    else if (generator::last) {
      remember(fdef, vc);
      class_or_namespace_name::pop(fdef->m_param);
      fundef::current = 0;
    }
    else {
      erase(fdef->m_param);
      if (mtbl.find(fdef->m_usr) == mtbl.end())
	delete fdef->m_param;
      for (tac* p : vc)
	delete p;
      vc.clear();
    }
  }
  u->m_type = u->m_type->vla2a();
}

namespace cxx_compiler {
  namespace declarations {
    namespace declarators {
      namespace function {
	namespace definition {
	  namespace static_inline {
	    inline scope* get(usr* u)
	    {
	      scope* ps = u->m_scope;
	      scope::id_t id = ps->m_id;
	      assert(id != scope::PARAM);
	      if (id == scope::BLOCK)
		return &scope::root;
	      if (id == scope::NAMESPACE) {
		usr::flag_t flag = u->m_flag;
		if ((flag & usr::C_SYMBOL) && (flag & usr::INLINE))
		  return &scope::root;
	      }
	      return ps;
	    }
	    void skip::check(tac* ptac, chk_t* arg)
	    {
	      using namespace std;
	      ++arg->m_pos;
	      var* y = ptac->y;
	      if (!y)
		return;
	      usr* u = y->usr_cast();
	      if (!u)
		return;
	      usr::flag_t flag = u->m_flag;
	      if (!(flag & usr::FUNCTION))
		return;
	      usr::flag_t mask = usr::flag_t(usr::STATIC | usr::INLINE);
	      if (!(flag & mask))
		return;

	      usr::flag_t exclude = usr::flag_t(usr::VIRTUAL | usr::OVERRIDE);
	      if (flag & exclude)
		return;

	      if ((flag & usr::STATIC) && !(flag & usr::INLINE)) {
		if (u->m_scope->m_id == scope::TAG)
		  return;
	      }

	      table_t::iterator it = stbl.find(u);
	      if (it != stbl.end()) {
		info_t* info = it->second;
		if (info) {
		  it->second = 0;
		  gencode(info);
		}
		return;
	      }

	      string name = u->m_name;
	      const type* T = u->m_type;
	      it = find_if(begin(stbl), end(stbl),
			   [name, T](const pair<usr*, info_t*>& p){
			     usr* u = p.first;
			     if (u->m_name != name)
			       return false;
			     return  compatible(u->m_type, T); });
	      if (it != stbl.end()) {
		info_t* info = it->second;
		if (info) {
		  it->second = 0;
		  gencode(info);
		}
		return;
	      }

	      typedef const func_type FT;
	      assert(T->m_id == type::FUNC);
	      FT* ft = static_cast<FT*>(T);
	      scope* ps = get(u);
	      const vector<const type*>& param = ft->param();
	      key_t key(name, ps, &param, get_seed(u));
	      if (definition::dtbl.find(key) != definition::dtbl.end())
		return;

	      using namespace defer;
	      pair<string, scope*> ns(name, ps);
	      refs[key].push_back(ref_t(name, flag, u->m_file, ptac->m_file));
	      if (ptac->m_id == tac::ADDR)
		return;
	      assert(ptac->m_id == tac::CALL);
	      if (!(flag & usr::INLINE))
		return;
	      arg->m_wait_inline = true;
	      usr* v = arg->m_fundef->m_usr;
	      callers[key].insert(v);
	      positions[v].push_back(arg->m_pos);
	    }
	  } // end of namespace static_inline
	} // end of namespace definition
      } // end of namespace function
    } // end of namespace declarators
  } // end of namespace declarations
} // end of namespace cxx_compiler

namespace cxx_compiler { namespace declarations { namespace declarators { namespace function { namespace definition {
  namespace static_inline {
    using namespace std;
    using namespace defer;
    namespace skip {
      void after_substitute(usr* ucaller,
			    pair<key_t, info_t*> pcallee)
      {
        map<usr*, vector<int> >::iterator p = positions.find(ucaller);
        assert(p != positions.end());
        vector<int>& vi = p->second;
        table_t::iterator q = stbl.find(ucaller);
        assert(q != stbl.end());
        info_t* caller = q->second;
        {
          vector<tac*>& vc = caller->m_code;
          vector<scope*>& ch = caller->m_fundef->m_param->m_children;
          assert(!ch.empty());
          struct sweeper {
            scope* m_org;
            sweeper(scope* org) : m_org(scope::current)
            { scope::current = org; }
            ~sweeper(){ scope::current = m_org; }
          } sweeper(ch[0]);
          int delta = 0;
          typedef vector<int>::iterator IT;
          for (IT r = vi.begin() ; r != vi.end() ; ) {
            *r += delta;
            int n = *r;
            tac* ptac = vc[n];
            assert(ptac->m_id == tac::CALL);
            var* y = ptac->y;
            usr* fn = y->usr_cast();
            assert(fn);
            key_t key = pcallee.first;
            info_t* callee = pcallee.second;
	    scope* ps = get(fn);
	    const type* T = fn->m_type;
	    assert(T->m_id == type::FUNC);
	    typedef const func_type FT;
	    FT* ft = static_cast<FT*>(T);
	    const vector<const type*>& param = ft->param();
	    instantiated_usr::SEED seed = get_seed(fn);
            if (fn->m_name == key.m_name && ps == key.m_scope &&
		&param == key.m_param && seed == key.m_seed) {
              int before = vc.size();
              if (!error::counter && !cmdline::no_inline_sub)
                substitute(vc, n, callee);
              int after = vc.size();
              delta += after - before;
              r = vi.erase(r);
            }
            else
              ++r;
          }
        }
        if (vi.empty()) {
          positions.erase(p);
          if (cmdline::optimize_level >= 1)
            optimize::action(caller->m_fundef, caller->m_code);
          usr::flag_t flag = ucaller->m_flag;
          if (!(flag & (usr::STATIC|usr::INLINE))) {
            stbl.erase(q);
            gencode(caller);
          }
        }
      }
      void add(fundef* fdef, vector<tac*>& vc, bool f)
      {
        usr* u = fdef->m_usr;
        u->m_type = u->m_type->vla2a();
        vector<const type*> vt;
        type::collect_tmp(vt);
        info_t* info = new info_t(fdef,vc,vt);
        stbl[u] = info;
        vector<scope*>& ch = u->m_scope->m_children;
        assert(!ch.empty());
        assert(ch.back() == fdef->m_param);
        ch.pop_back();
        vc.clear();
        fundef::current = 0;
        if (!f)
          return;

	const type* T = u->m_type;
	assert(T->m_id == type::FUNC);
	typedef const func_type FT;
	FT* ft = static_cast<FT*>(T);
	const vector<const type*>& param = ft->param();
	key_t key(u->m_name, get(u), &param, get_seed(u));
        map<key_t, vector<ref_t> >::iterator p = refs.find(key);
        if (p == refs.end())
          return;

        refs.erase(p);
        map<key_t, set<usr*> >::iterator q = callers.find(key);
        if (q == callers.end()) {
          stbl.erase(u);
          return gencode(info);
        }

        const set<usr*>& su = q->second;
        usr::flag_t flag = u->m_flag;
        assert(flag & usr::INLINE);
        for_each(begin(su), end(su),
                 bind2nd(ptr_fun(after_substitute),make_pair(key,info)));
        callers.erase(q);
      }
    } // end of namespace skip
    void remember(fundef* fdef, std::vector<tac*>& vc)
    {
      funcs.push_back(make_pair(fdef, vc));
      scope* param = fdef->m_param;
      vector<scope*>& children = param->m_parent->m_children;
      typedef vector<scope*>::reverse_iterator IT;
      IT p = find(rbegin(children), rend(children), param);
      if (p != rend(children)) {
        vector<scope*>::iterator q = p.base() - 1;
        children.erase(q);
      }
      vc.clear();
    }
  } // end of namespace static_inline
} } } } } // end of namespace definition, function, declarators, declarations and cxx_compiler

cxx_compiler::declarations::declarators::function::definition::table_t
cxx_compiler::declarations::declarators::function::definition::dtbl;

cxx_compiler::declarations::declarators::function::definition::
static_inline::info_t::~info_t()
{
  using namespace std;
  for (auto p : m_code) delete p;
  for (auto p : m_tmp) delete p;
  scope* param = m_fundef->m_param;
  delete param;
  delete m_fundef;
}

void cxx_compiler::declarations::declarators::
function::definition::static_inline::gencode(info_t* info)
{
  fundef* fdef = info->m_fundef;
  vector<tac*>& vc = info->m_code;
  skip::chk_t arg(fdef);
  for_each(vc.begin(), vc.end(), bind2nd(ptr_fun(skip::check), &arg));
  if (arg.m_wait_inline) {
    assert(skip::stbl.find(fdef->m_usr) == skip::stbl.end());
    skip::stbl[fdef->m_usr] = info;
    return;
  }
  scope* param = info->m_fundef->m_param;
  struct ss {
    scope* m_org;
    ss(scope* ptr) : m_org(0)
    {
      vector<scope*>& ch = scope::root.m_children;
      typedef vector<scope*>::reverse_iterator IT;
      IT p = find_if(rbegin(ch), rend(ch),
		     bind2nd(ptr_fun(cmp), scope::PARAM));
      if (p == rend(ch))
        ch.push_back(ptr);
      else {
	vector<scope*>::iterator q = p.base() - 1;
        assert(q+1 == end(ch));
        m_org = *p;
        *p = ptr;
      }
    }
    ~ss()
    {
      vector<scope*>& ch = scope::root.m_children;
      if (!ch.empty())
        ch.pop_back();
      else
        assert(generator::last);
      if (m_org)
        ch.push_back(m_org);
    }
  } ss(param);
  if (cmdline::output_medium) {
    if (cmdline::output_optinfo)
      cout << "\nAfter optimization\n";
    usr* u = fdef->m_usr;
    scope* org = scope::current;
    scope::current = &scope::root;
    cout << dump::names::ref(u) << ":\n";
    scope::current = org;
    for (auto p : vc)
      cout << '\t', dump::tacx(cout, p), cout << '\n';
    cout << '\n';
    dump::scopex();
  }
  if (!error::counter) {
    if (generator::generate) {
      generator::interface_t tmp = {
        &scope::root,
        fdef,
        &vc
      };
      generator::generate(&tmp);
    }
    else if (generator::last) {
      remember(fdef, vc);
      info = 0;
    }
  }
  delete info;
}

namespace cxx_compiler { namespace declarations { namespace declarators { namespace function { namespace definition { namespace static_inline { namespace symtab {
  std::map<var*, var*> table;
} } } } } } } // symtab, static_inline, definition, function, declarators, declarations and cxx_compiler

namespace cxx_compiler { namespace declarations { namespace declarators { namespace function { namespace definition { namespace static_inline { namespace dup {
  struct patch {
    std::map<goto3ac*,goto3ac*> m_goto;
    std::map<to3ac*,to3ac*> m_to;
  };
  tac* filter(tac*, patch*);
  int spatch(std::pair<goto3ac*,goto3ac*> x, std::map<to3ac*,to3ac*>* y)
  {
    using namespace std;
    goto3ac* org = x.first;
    map<to3ac*,to3ac*>::const_iterator p = y->find(org->m_to);
    assert(p != y->end());
    goto3ac* _new = x.second;
    _new->m_to = p->second;
    return 0;
  }
  goto3ac* helper(goto3ac* x, std::map<goto3ac*,goto3ac*>* y)
  {
    using namespace std;
    map<goto3ac*,goto3ac*>::const_iterator p = y->find(x);
    assert(p != y->end());
    return p->second;
  }
  int tpatch(std::pair<to3ac*,to3ac*> x, std::map<goto3ac*,goto3ac*>* y)
  {
    using namespace std;
    to3ac* org = x.first;
    const vector<goto3ac*>& u = org->m_goto;
    to3ac* _new = x.second;
    vector<goto3ac*>& v = _new->m_goto;
    transform(u.begin(),u.end(),v.begin(),bind2nd(ptr_fun(helper),y));
    return 0;
  }
} } } } } } } // dup, static_inline, definition, function, declarators, declarations and cxx_compiler

cxx_compiler::tac*
cxx_compiler::declarations::declarators::function::definition::static_inline::dup::filter(tac* ptr, patch* patch)
{
  using namespace std;
  tac* ret = ptr->new3ac();
  if ( ret->x ){
    map<var*, var*>::const_iterator p = symtab::table.find(ret->x);
    if ( p != symtab::table.end() )
      ret->x = p->second;
  }
  if ( ret->y ){
    map<var*, var*>::const_iterator p = symtab::table.find(ret->y);
    if ( p != symtab::table.end() )
      ret->y = p->second;
  }
  if ( ret->z ){
    map<var*, var*>::const_iterator p = symtab::table.find(ret->z);
    if ( p != symtab::table.end() )
      ret->z = p->second;
  }

  tac::id_t id = ptr->m_id;
  switch ( id ){
  case tac::GOTO:
    {
      goto3ac* go = static_cast<goto3ac*>(ptr);
      return patch->m_goto[go] = static_cast<goto3ac*>(ret);
    }
  case tac::TO:
    {
      to3ac* to = static_cast<to3ac*>(ptr);
      return patch->m_to[to] = static_cast<to3ac*>(ret);
    }
  default:
    return ret;
  }
}

const cxx_compiler::type*
cxx_compiler::declarations::declarators::type_id::action(const type* T)
{
  using namespace std;
  assert(!type_specifier_seq::info_t::s_stack.empty());
  type_specifier_seq::info_t* p = type_specifier_seq::info_t::s_stack.top();
  if ( !p->m_type || !p->m_tmp.empty() )
    p->update();
  return T ? T->patch(p->m_type,0) : p->m_type;
}

namespace cxx_compiler {
  namespace declarations {
    namespace declarators {
      usr* ctor()
      {
        assert(!specifier_seq::info_t::s_stack.empty());
        specifier_seq::info_t* p = specifier_seq::info_t::s_stack.top();
        auto_ptr<specifier_seq::info_t> sweeper(p);
        assert(p->m_tag);
        assert(p->m_type);
	usr::flag_t flag = usr::flag_t(p->m_flag | usr::CTOR);
        const type* T = p->m_type;
#if 0
        assert(T->m_id == type::INCOMPLETE_TAGGED);
        typedef const incomplete_tagged_type ITT;
        ITT* itt = static_cast<ITT*>(T);
        tag* ptr = itt->get_tag();
#else
	tag* ptr = T->get_tag();
	assert(ptr);
#endif
	string name = tor_name(ptr);
        T = backpatch_type::create();
        return new usr(name, T, flag, parse::position, usr::NONE2);
      }
      usr* ctor(type_specifier* spec)
      {
        using namespace std;
        auto_ptr<type_specifier> sweeper(spec);
        const type* T = spec->m_type;
        assert(T);
        assert(T->m_id == type::RECORD);
        typedef const record_type REC;
        REC* rec = static_cast<REC*>(T);
        tag* ptr = rec->get_tag();
	string name = tor_name(ptr);
        T = backpatch_type::create();
        return new usr(name, T, usr::CTOR, parse::position, usr::NONE2);
      }
    } // end of namespace declarators
  } // end of namespace declarations
} // end of namespace cxx_compiler
