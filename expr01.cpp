// postfix-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "cxx_y.h"
#include "patch.03.q"

namespace cxx_compiler { namespace subscript_impl {
  var* size(const type*);
} } // end of namespace subscript_impl and cxx_compiler

cxx_compiler::var* cxx_compiler::var::subscripting(var* y)
{
  using namespace std;
  var* array = rvalue();
  var* index = y->rvalue();
  if ( !index->m_type->integer() )
    swap(array,index);
  if ( !index->m_type->integer() ){
    using namespace error::expressions::postfix::subscripting;
    not_integer(parse::position,index);
    return array;
  }
  const type* T = array->m_type;
  T = T->unqualified();
  if ( T->m_id != type::POINTER ){
    using namespace error::expressions::postfix::subscripting;
    not_pointer(parse::position,array);
    return array;
  }
  typedef const pointer_type PT;
  PT* pt = static_cast<PT*>(T);
  pt = static_cast<PT*>(pt->complete_type());
  T = pt->referenced_type();
  var* size = subscript_impl::size(T);
  if ( !size ){
    using namespace error::expressions::postfix::subscripting;
    not_object(parse::position,T);
    return array;
  }
  conversion::arithmetic::gen(&size, &index);
  var* offset = size->mul(index);
  assert(offset->m_type->integer());
  return array->offref(T,offset);
}

cxx_compiler::var* cxx_compiler::subscript_impl::size(const type* T)
{
  using namespace std;
  using namespace expressions::primary::literal;
  if ( int size = T->size() )
    return integer::create(size);
  else
    return T->vsize();
}

cxx_compiler::var* cxx_compiler::genaddr::subscripting(var* y)
{
  var* index = y->rvalue();
  if ( !index->m_type->integer() ){
    using namespace error::expressions::postfix::subscripting;
    not_integer(parse::position,index);
    return this;
  }
  const type* T = m_ref->m_type;
  switch ( T->m_id ){
  case type::ARRAY:
    {
      typedef const array_type ARRAY;
      ARRAY* array = static_cast<ARRAY*>(T);
      T = array->element_type();
      break;
    }
  case type::VARRAY:
    {
      typedef const varray_type VARRAY;
      VARRAY* varray = static_cast<VARRAY*>(T);
      T = varray->element_type();
      break;
    }
  default:
    using namespace error::expressions::postfix::subscripting;
    not_object(parse::position,T);
    return this;
  }
  var* size = subscript_impl::size(T);
  if ( !size ){
    using namespace error::expressions::postfix::subscripting;
    not_object(parse::position,T);
    return this;
  }
  conversion::arithmetic::gen(&size, &index);
  var* offset = size->mul(index);
  assert(offset->m_type->integer());
  return offref(T,offset);
}

cxx_compiler::var* cxx_compiler::expressions::postfix::call::gen()
{
  using namespace std;
  var* func = m_func->gen();
  vector<var*> arg;
  if ( m_arg )
    transform(m_arg->begin(),m_arg->end(),back_inserter(arg),mem_fun(&base::gen));
  return func->call(&arg);
}

const cxx_compiler::file_t&
cxx_compiler::expressions::postfix::call::file() const
{
  return m_func->file();
}

cxx_compiler::expressions::postfix::call::~call()
{
  using namespace std;
  delete m_func;
  if ( m_arg ) {
    for (auto p : *m_arg)
      delete p;
  }
  delete m_arg;
}

cxx_compiler::var* cxx_compiler::var::call(std::vector<var*>* arg)
{
  using namespace std;
  var* func = rvalue();
  const type* T = func->m_type;
  T = T->unqualified();
  if ( T->m_id == type::POINTER ){
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(T);
    T = pt->referenced_type();
  }
  if ( T->m_id != type::FUNC ){
    using namespace error::expressions::postfix::call;
    not_function(parse::position,func);
    return func;
  }
  typedef const func_type FUNC;
  FUNC* ft = static_cast<FUNC*>(T);
  return call_impl::common(ft, func, arg, false, 0, false);
}

cxx_compiler::var*
cxx_compiler::genaddr::call(std::vector<var*>* arg)
{
  using namespace std;
  const type* T = m_ref->m_type;
  if ( T->m_id != type::FUNC ){
    using namespace error::expressions::postfix::call;
    not_function(parse::position,m_ref);
    return rvalue();
  }
  typedef const func_type FUNC;
  FUNC* ft = static_cast<FUNC*>(T);
  assert(m_ref->usr_cast());
  usr* u = static_cast<usr*>(m_ref);
  usr::flag_t flag = u->m_flag;
  scope* fun_scope = u->m_scope;
  var* this_ptr = 0;
  if (fun_scope->m_id == scope::TAG) {
    if (!(flag & usr::STATIC)) {
      int r = parse::identifier::lookup("this", scope::current);
      if (!r)
        error::not_implemented();
      assert(r == IDENTIFIER_LEX);
      this_ptr = cxx_compiler_lval.m_var;
    }
    if (this_ptr) {
      scope* this_parent = this_ptr->m_scope->m_parent;
      if (fun_scope != this_parent) {
        tag* b = static_cast<tag*>(fun_scope);
        const type* Tb = b->m_types.second;
        assert(Tb);
        const type* pTb = pointer_type::create(Tb);
        this_ptr = this_ptr->cast(pTb);
      }
    }
  }
  var* ret = call_impl::common(ft, u, arg, false, this_ptr, m_qualified_func);
  if (!error::counter && !cmdline::no_inline_sub) {
    if (flag & usr::INLINE) {
      using namespace declarations::declarators::function;
      using namespace definition::static_inline::skip;
      table_t::const_iterator p = stbl.find(u);
      if (p != stbl.end())
        substitute(code, code.size()-1, p->second);
    }
  }
  return ret;
}

cxx_compiler::var*
cxx_compiler::member_function::call(std::vector<var*>* arg)
{
  using namespace std;
  auto_ptr<member_function> sweeper(this);
  typedef const func_type FUNC;
  FUNC* ft = static_cast<FUNC*>(m_fun->m_type);
  var* ret = call_impl::common(ft,m_fun,arg,false,m_obj, m_qualified_func);
  usr::flag_t flag = m_fun->m_flag;
  if (!error::counter && !cmdline::no_inline_sub) {
    if (flag & usr::INLINE) {
      using namespace declarations::declarators::function;
      using namespace definition::static_inline::skip;
      table_t::const_iterator p = stbl.find(m_fun);
      if (p != stbl.end())
        substitute(code, code.size()-1, p->second);
    }
  }
  return ret;
}

namespace cxx_compiler {
  namespace overload_impl {
    struct result {
      var* m_var;
      result(var* v) : m_var(v) {}
      bool NG(){ return m_var == 0; }
    };
    result* do_trial(usr* u, std::vector<var*>* arg, var* obj)
    {
      using namespace std;
      const type* T = u->m_type;
      typedef const func_type FUNC;
      FUNC* ft = static_cast<FUNC*>(T);
      var* tmp = call_impl::common(ft, u, arg, true, obj, false);
      if (tmp) {
	if (!error::counter && !cmdline::no_inline_sub) {
	  usr::flag_t flag = u->m_flag;
	  if (flag & usr::INLINE) {
	    using namespace declarations::declarators::function;
	    using namespace definition::static_inline::skip;
	    table_t::const_iterator p = stbl.find(u);
	    if (p != stbl.end())
	      substitute(code, code.size()-1, p->second);
	  }
	}
      }
      return new result(tmp);
    }
  } // end of namespace overload_impl
} // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::overload::call(std::vector<var*>* arg)
{
  using namespace std;
  using namespace overload_impl;
  const vector<usr*>& cand = m_candidacy;
  var* obj = m_obj;
  misc::pvector<result> res;
  transform(begin(cand), end(cand), back_inserter(res),
	    [arg, obj](usr* u){ return do_trial(u, arg, obj); });
  vector<result*>::iterator p = begin(res);
  while ( p != end(res) ){
    p = find_if(p, end(res), mem_fun(&result::NG));
    if ( p != res.end() ){
      delete *p;
      p = res.erase(p);
    }
  }
  if (res.size() == 1)
    return res[0]->m_var;
  if (res.empty()) {
    using namespace error::expressions::postfix::call;
    overload_not_match(this);
    var* ret = new var(int_type::create());
    if (scope::current->m_id == scope::BLOCK) {
      block* b = static_cast<block*>(scope::current);
      b->m_vars.push_back(ret);
    }
    else
      garbage.push_back(ret);
    return ret;
  }
  error::not_implemented();
}

namespace cxx_compiler {
  namespace call_impl {
    using namespace std;
    pair<int,int> num_of_range(const vector<const type*>&);
    struct convert {
      const vector<const type*>& m_param;
      var* m_func;
      bool m_trial;
      int m_counter;
      convert(const vector<const type*>& param, var* func, bool trial)
        : m_param(param), m_func(func), m_counter(-1), m_trial(trial) {}
      var* operator()(var*);
    };
    tac* gen_param(var*);
    var* ref_vftbl(usr* vf, var* vp);
  } // end of namespace call_impl
 } // end of namespace cxx_compiler

cxx_compiler::var*
cxx_compiler::call_impl::common(const func_type* ft,
                                var* func,
                                std::vector<var*>* arg,
                                bool trial,
                                var* obj,
				bool qualified_func)
{
  using namespace std;
  const vector<const type*>& param = ft->param();
  int n = arg ? arg->size() : 0;
  pair<int,int> m = call_impl::num_of_range(param);
  if ( n < m.first ){
    if ( trial )
      return 0;
    using namespace error::expressions::postfix::call;
    num_of_arg(parse::position,func,n,m.first);
  }
  else if ( m.second < n ){
    if ( trial )
      return 0;
    using namespace error::expressions::postfix::call;
    num_of_arg(parse::position,func,n,m.second);
    n = m.second;
  }
  vector<var*> conved;
  if ( arg ){
    const vector<var*>& v = *arg;
    transform(v.begin(),v.begin()+n,back_inserter(conved),
              call_impl::convert(param,func,trial));
    if ( trial && find(conved.begin(),conved.end(),(var*)0) != conved.end() )
      return 0;
  }
  if (obj) {
    const type* T = obj->m_type;
    if (T->scalar()) {
      assert(T->m_id == type::POINTER);
      usr* u = func->usr_cast();
      usr::flag_t flag = u->m_flag;
      if ((flag & usr::VIRTUAL) && !qualified_func)
	func = ref_vftbl(u,obj);
      code.push_back(new param3ac(obj));
    }
    else {
      T = pointer_type::create(T);
      var* tmp = new var(T);
      if ( scope::current->m_id == scope::BLOCK ){
        block* b = static_cast<block*>(scope::current);
        b->m_vars.push_back(tmp);
      }
      else
        garbage.push_back(tmp);
      code.push_back(new addr3ac(tmp,obj));
      code.push_back(new param3ac(tmp));
    }
  }
  transform(conved.begin(),conved.end(),back_inserter(code),call_impl::gen_param);
  
  const type* T = ft->return_type();
  if ( T )
    T = T->complete_type();
  var* x = new var(T);
  if (!T || T->m_id == type::VOID){
    code.push_back(new call3ac(0,func));
    garbage.push_back(x);
    return x;
  }
  if ( !T->size() ){
    using namespace error::expressions::postfix::call;
    not_object(parse::position,func);
    x->m_type = int_type::create();
  }
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new call3ac(x,func));
  return x;
}

cxx_compiler::tac* cxx_compiler::call_impl::gen_param(var* y)
{
  return new param3ac(y);
}

std::pair<int,int> cxx_compiler::call_impl::num_of_range(const std::vector<const type*>& param)
{
  using namespace std;
  const type* T = param.back();
  if (T->m_id == type::ELLIPSIS)
    return make_pair(param.size()-1,INT_MAX);
  if (T->m_id == type::VOID)
    return make_pair(0,0);
  else
    return make_pair(param.size(),param.size());
}

cxx_compiler::var* cxx_compiler::call_impl::convert::operator()(var* arg)
{
  using namespace std;
  if ( ++m_counter == m_param.size() )
    --m_counter;
  const type* T = m_param[m_counter];
  T = T->complete_type();
  const type* U = T->unqualified();
  if (U->m_id != type::REFERENCE)
    arg = arg->rvalue();
  if ( T->m_id == type::ELLIPSIS ){
    const type* T2 = arg->m_type;
    if ( T2->compatible(T2->varg()) )
      return arg;
    T = T2->varg();
  }
  T = T->unqualified();
  bool discard = false;
  T = expressions::assignment::valid(T,arg,&discard);
  if (!T) {
    if ( m_trial )
      return 0;
    using namespace error::expressions::postfix::call;
    mismatch_argument(parse::position,m_counter,discard,m_func);
    return arg;
  }
  arg = arg->cast(T);
  if (U->m_id == type::REFERENCE) {
    typedef const reference_type RT;
    RT* rt = static_cast<RT*>(U);
    const type* R = rt->referenced_type();
    R = R->unqualified();
    if (R == T) {
      var* tmp = new var(U);
      if (scope::current->m_id == scope::BLOCK) {
        block* b = static_cast<block*>(scope::current);
        b->m_vars.push_back(tmp);
      }
      else
        garbage.push_back(tmp);
      if (arg->isconstant()) {
        var* tmp2 = new var(R);
        if (scope::current->m_id == scope::BLOCK) {
          block* b = static_cast<block*>(scope::current);
          b->m_vars.push_back(tmp2);
        }
        else
          garbage.push_back(tmp2);
        code.push_back(new assign3ac(tmp2, arg));
        arg = tmp2;
      }
      code.push_back(new addr3ac(tmp, arg));
      arg = tmp;
    }
  }
  return arg;
}

cxx_compiler::var* cxx_compiler::call_impl::ref_vftbl(usr* vf, var* vp)
{
  using namespace std;
  scope* ptr = vf->m_scope;
  assert(ptr->m_id == scope::TAG);
  tag* ptag = static_cast<tag*>(ptr);
  const type* T = ptag->m_types.second;
  assert(T->m_id == type::RECORD);
  typedef const record_type REC;
  REC* rec = static_cast<REC*>(T);

  block* b = ( scope::current->m_id == scope::BLOCK ) ?
    static_cast<block*>(scope::current) : 0;

  T = vf->m_type;
  assert(T->m_id == type::FUNC);
  const func_type* ft = static_cast<const func_type*>(T);
  const pointer_type* pt = pointer_type::create(ft);
  var* tmp = 0;
  vector<tag*> dummy;
  pair<int, usr*> off = rec->offset(vfptr_name, dummy);
  int vfptr_offset = off.first;
  assert(vfptr_offset >= 0);
  if (!vfptr_offset)  {
    var* t0 = new var(pt);
    if ( b )
      b->m_vars.push_back(t0);
    else
      garbage.push_back(t0);
    code.push_back(new invraddr3ac(t0,vp));
    tmp = t0;
  }
  else {
    var* t0 = new var(pt);
    var* t1 = new var(pt);
    if ( b ){
      b->m_vars.push_back(t0);
      b->m_vars.push_back(t1);
    }
    else {
      garbage.push_back(t0);
      garbage.push_back(t1);
    }
    using namespace expressions::primary::literal;
    var* off = integer::create(vfptr_offset);
    code.push_back(new add3ac(t0,vp,off));
    code.push_back(new invraddr3ac(t1,t0));
    tmp = t1;
  }
  var* res = new var(pt);
  if ( b )
    b->m_vars.push_back(res);
  else
    garbage.push_back(res);
  const map<string, vector<usr*> >& usrs = ptag->m_usrs;
  typedef map<string, vector<usr*> >::const_iterator IT;
  IT p = usrs.find(vftbl_name);
  assert(p != usrs.end());
  const vector<usr*>& v = p->second;
  assert(v.size() == 1);
  usr* u = v.back();
  assert(u->m_flag & usr::WITH_INI);
  with_initial* vftbl = static_cast<with_initial*>(u);
  map<int, var*>::const_iterator q =
    find_if(begin(vftbl->m_value), end(vftbl->m_value),
            bind2nd(ptr_fun(match_vf),vf));
  assert(q != end(vftbl->m_value));
  int offset = q->first;
  if (offset){
    var* t2 = new var(pt);
    if ( b )
      b->m_vars.push_back(t2);
    else
      garbage.push_back(t2);
    using namespace expressions::primary::literal;
    var* off = integer::create(offset);
    code.push_back(new add3ac(t2,tmp,off));
    tmp = t2;
  }
  code.push_back(new invraddr3ac(res,tmp));
  return res;
}

namespace cxx_compiler {
  namespace declarations {
    namespace declarators {
      namespace function {
        namespace definition {
          namespace static_inline {
            using namespace std;      
            namespace substitute_impl {
              map<var*, var*> symtab;
              usr* new_usr(usr* u, scope* s)
              {
                usr::flag_t flag = u->m_flag;
                usr* ret = 0;
                if (flag & usr::WITH_INI) {
                  with_initial* p = static_cast<with_initial*>(u);
                  ret = new with_initial(*p);
                }
                else if (flag & usr::ENUM_MEMBER) {
                  enum_member* p = static_cast<enum_member*>(u);
                  ret = new enum_member(*p);
                }
                else
                  ret = new usr(*u);
                ret->m_scope = s;
                symtab[u] = ret;
                return ret;
              }
              var* new_var(var* v, scope* s)
              {
                var* ret = new var(*v);
                ret->m_scope = s;
                return symtab[v] = ret;
              }
              scope* new_block(scope* ptr, scope* parent)
              {
                block* ret = new block;
                {
                  using namespace class_or_namespace_name;
                  assert(before.back() == ret);
                  before.pop_back();
                }
                ret->m_parent = parent;
                if (ptr->m_id != scope::BLOCK) {
                  assert(ptr->m_id == scope::TAG);
                  return ret;
                }
                block* b = static_cast<block*>(ptr);
                const map<string, vector<usr*> >& u = b->m_usrs;
                map<string, vector<usr*> >& d = ret->m_usrs;
                typedef map<string, vector<usr*> >::const_iterator IT;
                for (auto& p : u) {
                  string name = p.first;
                  const vector<usr*>& v = p.second;
                  transform(v.begin(),v.end(),back_inserter(d[name]),
                            bind2nd(ptr_fun(new_usr),ret));
                }
                const vector<var*>& v = b->m_vars;
                transform(v.begin(),v.end(),back_inserter(ret->m_vars),
                          bind2nd(ptr_fun(new_var),ret));
                const vector<scope*>& c = b->m_children;
                transform(c.begin(),c.end(),back_inserter(ret->m_children),
                          bind2nd(ptr_fun(new_block),ret));
                return ret;
              }
              block* create(const scope* param)
              {
                symtab.clear();
                block* ret = new block;
                {
                  using namespace class_or_namespace_name;
                  assert(before.back() == ret);
                  before.pop_back();
                }
                ret->m_parent = scope::current;
                scope::current->m_children.push_back(ret);
                const vector<usr*>& o = param->m_order;
                vector<var*>& v = ret->m_vars;
                transform(o.begin(),o.end(),back_inserter(v),
                          bind2nd(ptr_fun(new_usr),ret));
                const vector<scope*>& c = param->m_children;
                transform(c.begin(),c.end(),back_inserter(ret->m_children),
                          bind2nd(ptr_fun(new_block),ret));
                return ret;
              }
              tac* param2assign(tac* param, var* x)
              {
                assert(param->m_id == tac::PARAM);
                var* y = param->y;
                delete param;
                return new assign3ac(x, y);
              }
              namespace dup {
                struct patch_t {
                  map<goto3ac*,goto3ac*> m_goto;
                  map<to3ac*,to3ac*> m_to;
                };
                tac* filter(tac* ptac, patch_t* patch)
                {
                  tac* ret = ptac->new3ac();
                  if (ret->x) {
                    map<var*, var*>::const_iterator p = symtab.find(ret->x);
                    if (p != symtab.end())
                      ret->x = p->second;
                  }
                  if (ret->y) {
                    map<var*, var*>::const_iterator p = symtab.find(ret->y);
                    if (p != symtab.end())
                      ret->y = p->second;
                  }
                  if (ret->z) {
                    map<var*, var*>::const_iterator p = symtab.find(ret->z);
                    if (p != symtab.end())
                      ret->z = p->second;
                  }

                  tac::id_t id = ptac->m_id;
                  switch (id) {
                  case tac::GOTO:
                    {
                      goto3ac* go = static_cast<goto3ac*>(ptac);
                      return patch->m_goto[go] = static_cast<goto3ac*>(ret);
                    }
                  case tac::TO:
                    {
                      to3ac* to = static_cast<to3ac*>(ptac);
                      return patch->m_to[to] = static_cast<to3ac*>(ret);
                    }
                  default:
                    return ret;
                  }
                }
                void spatch(pair<goto3ac*,goto3ac*> x, map<to3ac*,to3ac*>* y)
                {
                  goto3ac* org = x.first;
                  map<to3ac*,to3ac*>::const_iterator p = y->find(org->m_to);
                  assert(p != y->end());
                  goto3ac* _new = x.second;
                  _new->m_to = p->second;
                }
                goto3ac* helper(goto3ac* x, map<goto3ac*,goto3ac*>* y)
                {
                  map<goto3ac*,goto3ac*>::const_iterator p = y->find(x);
                  assert(p != y->end());
                  return p->second;
                }
                void tpatch(pair<to3ac*,to3ac*> x, map<goto3ac*,goto3ac*>* y)
                {
                  to3ac* org = x.first;
                  const vector<goto3ac*>& u = org->m_goto;
                  to3ac* _new = x.second;
                  vector<goto3ac*>& v = _new->m_goto;
                  transform(u.begin(),u.end(),v.begin(),
                            bind2nd(ptr_fun(helper),y));
                }
              } // end of namespace dup
              struct arg_t {
                vector<tac*>* m_result;
                vector<goto3ac*>* m_returns;
                var* m_ret;
                dup::patch_t* m_patch;
              };
              void conv(tac* ptac, arg_t* pa)
              {
                if (ptac->m_id == tac::RETURN) {
                  if (var* y = ptac->y) {
                    map<var*,var*>::const_iterator p = symtab.find(y);
                    if (p != symtab.end())
                      y = p->second;
                    pa->m_result->push_back(new assign3ac(pa->m_ret,y));
                  }
                  goto3ac* go = new goto3ac;
                  pa->m_returns->push_back(go);
                  pa->m_result->push_back(go);
                }
                else
                  pa->m_result->push_back(dup::filter(ptac,pa->m_patch));
              }
            }  // end of namespace substitute_impl

            void substitute(vector<tac*>& vt, int pos, info_t* info)
            {
              using namespace call_impl;
              using namespace substitute_impl;
              tac* call = vt[pos];
              assert(call->m_id == tac::CALL);
              fundef* fdef = info->m_fundef;
              usr* func = fdef->m_usr;
              var* y = call->y;
              usr* u = y->usr_cast();
              assert(u && u->m_name == func->m_name);
              typedef const func_type FT;
              const type* T = func->m_type;
              assert(T->m_id == type::FUNC);
              FT* ft = static_cast<FT*>(T);
              pair<int,int> p = num_of_range(ft->param());
              if (p.first != p.second)  // take variable number of arguments
                return;
              block* pb = create(fdef->m_param);
              int n = p.first;
              if (func->m_scope->m_id == scope::TAG &&
                  !(func->m_flag & usr::STATIC))
                ++n;
              const vector<var*>& vars = pb->m_vars;
              assert(vars.size() == n);
              assert(pos >= n);
              assert(pos < vt.size());
              transform(&vt[pos-n],&vt[pos],vars.begin(),&vt[pos-n],
                        param2assign);
              const vector<tac*>& v = info->m_code;
              vector<tac*> result;
              vector<goto3ac*> returns;
              dup::patch_t patch;
              arg_t arg = { &result, &returns, call->x, &patch };
              for_each(v.begin(),v.end(),bind2nd(ptr_fun(conv),&arg));
              map<goto3ac*,goto3ac*>& s = patch.m_goto;
              map<to3ac*,to3ac*>& t = patch.m_to;
              for_each(s.begin(),s.end(),bind2nd(ptr_fun(dup::spatch),&t));
              for_each(t.begin(),t.end(),bind2nd(ptr_fun(dup::tpatch),&s));
              to3ac* to = new to3ac;
              result.push_back(to);
              for_each(returns.begin(),returns.end(),
                       bind2nd(ptr_fun(misc::update),to));
              copy(returns.begin(),returns.end(),back_inserter(to->m_goto));
              vt.erase(vt.begin()+pos);
              delete call;
              for_each(result.begin(), result.end(), [&vt, &pos](tac* ptr)
                       { vt.insert(vt.begin()+pos++, ptr); });
            }

            namespace defer {
              void last()
              {
                using namespace std;
                for (auto& p : refs) {
                  const vector<ref_t>& v = p.second;
                  assert(!v.empty());
                  const ref_t& r = v[0];
                  error::declarations::declarators::function::definition::
                    static_inline::nodef(r.m_def, r.m_flag, r.m_name, r.m_use);
                }
              }
            } // end of namespace defer
          }  // end of namespace static_inline
        }  // end of namespace definition
      }  // end of namespace function
    }  // end of namespace declarators
  }  // end of namespace declarations
}  // end of namespace cxx_compiler

std::stack<cxx_compiler::expressions::postfix::member::info_t*>
cxx_compiler::expressions::postfix::member::handling;

cxx_compiler::expressions::postfix::member::info_t*
cxx_compiler::expressions::postfix::member::begin(base* expr, bool dot)
{
  using namespace std;
  parse::identifier::base_lookup::route.clear();
  auto_ptr<base> sweeper(expr);
  int n = code.size();
  var* v = expr->gen();
  int m = code.size();
  vector<tac*> nm;
  copy(code.begin()+n,code.begin()+m,back_inserter(nm));
  code.resize(n);
  const type* T = v->result_type();
  if ( !dot ){
    T = T->unqualified();
    if ( T->m_id != type::POINTER ){
      using namespace error::expressions::postfix::member;
      not_pointer(parse::position,v);
      return new info_t(nm,v,dot,scope::current,expr->file());
    }
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(T);
    T = pt->referenced_type();
  }
  info_t* ret = new info_t(nm,v,dot,scope::current,expr->file());
  tag* ptr = T->get_tag();
  if ( ptr && ptr->m_kind != tag::ENUM ){
    scope::current = ptr;
    parse::identifier::mode = parse::identifier::member;
  }
  else {
    using namespace error::expressions::postfix::member;
    not_record(parse::position,v);
  }
  handling.push(ret);
  return ret;
}

cxx_compiler::expressions::base*
cxx_compiler::expressions::postfix::member::end(info_t* info, var* member)
{
  info->m_member = member;
  info->m_route = parse::identifier::base_lookup::route;
  parse::identifier::base_lookup::route.clear();
  scope::current = info->m_scope;
  parse::identifier::mode = parse::identifier::look;
  if ( !handling.empty() )
    handling.pop();
  return info;
}

cxx_compiler::var* cxx_compiler::expressions::postfix::member::info_t::gen()
{
  using namespace std;
  using namespace declarations::declarators::function::definition::static_inline;
  copy(m_code.begin(),m_code.end(),back_inserter(code));
  return m_expr->member(m_member,m_dot,m_route);
}

cxx_compiler::var*
cxx_compiler::var::member(var* expr, bool dot, const std::vector<tag*>& route)
{
  using namespace std;
  using namespace expressions::primary::literal;
  genaddr* ga = expr->genaddr_cast();
  if (ga)
    expr = ga->m_ref;
  const type* T = result_type();
  int cvr = 0;
  T = T->unqualified(dot ? &cvr : 0);
  typedef const pointer_type PT;
  if (!dot) {
    if (T->m_id != type::POINTER)
      return this;
    PT* pt = static_cast<PT*>(T);
    T = pt->referenced_type();
    T = T->unqualified(&cvr);
  }
  T = T->complete_type();
  if (T->m_id != type::RECORD)
    return this;
  typedef const record_type REC;
  REC* rec = static_cast<REC*>(T);
  usr* member = expr->usr_cast();
  if (!member)
    return expr;
  usr::flag_t flag = member->m_flag;
  if (flag & usr::STATIC)
    return member;
  if (flag & usr::ENUM_MEMBER) {
    enum_member* p = static_cast<enum_member*>(member);
    return p->m_value;
  }
  const type* Mt = member->m_type;
  if (!Mt) {
    assert(flag & usr::OVERLOAD);
    overload* ovl = static_cast<overload*>(member);
    ovl->m_obj = this;
    return ovl;
  }

  scope* msp = member->m_scope;
  assert(msp->m_id == scope::TAG);
  tag* ptr = static_cast<tag*>(msp);
  const type* Tm = ptr->m_types.second;
  assert(Tm->m_id == type::RECORD);
  REC* mrec = static_cast<REC*>(Tm);

  if (Mt->m_id == type::FUNC) {
    assert(ga);
    bool qualified_func = ga->m_qualified_func;
    if (rec == mrec)
      return new member_function(this, member, qualified_func);
    const type* T = pointer_type::create(mrec);
    if (dot) {
      var* tmp = new var(T);
      if (scope::current->m_id == scope::BLOCK) {
	block* b = static_cast<block*>(scope::current);
	b->m_vars.push_back(tmp);
      }
      else
	garbage.push_back(tmp);
      code.push_back(new addr3ac(tmp, this));
      bool direct_virt = false;
      int offset = rec->base_offset(mrec, route, &direct_virt);
      assert(offset >= 0);
      if (offset) {
	var* off = integer::create(offset);
	code.push_back(new add3ac(tmp, tmp, off));
      }
      return new member_function(tmp, member, qualified_func);
    }
    var* tmp = cast(T);
    return new member_function(tmp, member, qualified_func);
  }

  if (rec == mrec) {
    pair<int, usr*> off = rec->offset(member->m_name, route);
    int offset = off.first;
    if (offset < 0)
      return this;
    if (flag & usr::BIT_FIELD) {
      int pos = rec->position(member);
      typedef const bit_field_type BF;
      BF* bf = static_cast<BF*>(Mt);
      T = bf->integer_type();
      PT* pt = pointer_type::create(T);
      int bit = bf->bit();
      var* ret = new refbit(pt,this,offset,member,pos,bit,dot);
      garbage.push_back(ret);
      return ret;
    }
    Mt = Mt->qualified(cvr);
    var* O = integer::create(offset);
    if (dot)
      return offref(Mt, O);
    var* rv = rvalue();
    return rv->offref(Mt, O);
  }

  bool direct_virtual = false;
  int base_offset = rec->base_offset(mrec, route, &direct_virtual);
  assert(base_offset >= 0);
  vector<tag*> dummy;
  pair<int, usr*> off = mrec->offset(member->m_name, dummy);
  int offset = off.first;
  assert(offset >= 0);

  if (flag & usr::BIT_FIELD)
    error::not_implemented();

  if (dot) {
    var* O = integer::create(base_offset + offset);
    return offref(Mt, O);
  }
  var* rv = rvalue();
  T = pointer_type::create(mrec);
  var* tmp = cast_impl::with_route(rv, T, route);
  var* O = integer::create(offset);
  return tmp->offref(Mt, O);
}

cxx_compiler::var* cxx_compiler::expressions::postfix::ppmm::gen()
{
  var* expr = m_expr->gen();
  return expr->ppmm(m_plus,true);
}

cxx_compiler::var* cxx_compiler::var::offref(const type* T, var* offset)
{
  using namespace std;
  if ( m_type->scalar() ){
    ref* ret = new ref(pointer_type::create(T));
    if ( scope::current->m_id == scope::BLOCK ){
      block* b = static_cast<block*>(scope::current);
      b->m_vars.push_back(ret);
    }
    else
      garbage.push_back(ret);
    if ( offset->isconstant() && !offset->value() )
      code.push_back(new assign3ac(ret,this));
    else
      code.push_back(new add3ac(ret,this,offset));
    return ret;
  }
  if ( offset->isconstant() ){
    int off = offset->value();
    var* ret = new refaddr(pointer_type::create(T),this,off);
    garbage.push_back(ret);
    return ret;
  }
  var* ret = new refsomewhere(pointer_type::create(T),this,offset);
  garbage.push_back(ret);
  return ret;
}

cxx_compiler::var* cxx_compiler::refaddr::offref(const type* T, var* offset)
{
  if ( offset->isconstant() ){
    int off = m_addrof.m_offset + offset->value();
    var* ret = new refaddr(pointer_type::create(T),m_addrof.m_ref,off);
    garbage.push_back(ret);
    return ret;
  }
  if ( int n = m_addrof.m_offset ) {
    var* tmp = expressions::primary::literal::integer::create(n);
    conversion::arithmetic::gen(&offset, &tmp);
    offset = offset->add(tmp);
  }
  var* ret = new refsomewhere(pointer_type::create(T),m_addrof.m_ref,offset);
  garbage.push_back(ret);
  return ret;
}

cxx_compiler::var* cxx_compiler::refsomewhere::offref(const type* T, var* offset)
{
  conversion::arithmetic::gen(&offset, &m_offset);
  offset = offset->add(m_offset);
  var* ret = new refsomewhere(pointer_type::create(T),m_ref,offset);
  garbage.push_back(ret);
  return ret;
}

cxx_compiler::var* cxx_compiler::genaddr::offref(const type* T, var* offset)
{
  typedef const pointer_type PT;
  PT* pt = pointer_type::create(T);
  if ( offset->isconstant() ){
    int off = m_offset + offset->value();
    var* ret = new refaddr(pt,m_ref,off);
    garbage.push_back(ret);
    return ret;
  }
  if ( m_offset ) {
    var* tmp = expressions::primary::literal::integer::create(m_offset);
    conversion::arithmetic::gen(&offset, &tmp);
    offset = offset->add(tmp);
  }
  var* ret = new refsomewhere(pt,m_ref,offset);
  garbage.push_back(ret);
  return ret;
}

cxx_compiler::var* cxx_compiler::addrof::offref(const type* T, var* offset)
{
  if ( offset->isconstant() ){
    int off = m_offset + offset->value();
    var* ret;
    if ( const pointer_type* G = T->ptr_gen() )
      ret = new genaddr(G,T,m_ref,off);
    else
      ret = new refaddr(pointer_type::create(T),m_ref,off);
    garbage.push_back(ret);
    return ret;
  }
  if ( m_offset ) {
    var* tmp = expressions::primary::literal::integer::create(m_offset);
    conversion::arithmetic::gen(&offset, &tmp);
    offset = offset->add(tmp);
  }
  if ( const pointer_type* G = T->ptr_gen() ){
    var* ret = new generated(G,T);
    block* b = (scope::current->m_id == scope::BLOCK) ? static_cast<block*>(scope::current) : 0;
    if ( b )
      b->m_vars.push_back(ret);
    else
      garbage.push_back(ret);
    code.push_back(new addr3ac(ret,m_ref));
    code.push_back(new add3ac(ret,ret,offset));
    return ret;
  }
  var* ret = new refsomewhere(pointer_type::create(T),m_ref,offset);
  garbage.push_back(ret);
  return ret;
}

namespace cxx_compiler {
  var* constant<__int64>::offref(const type* T, var* offset)
  {
    if (m_flag & CONST_PTR) {
      assert(sizeof(void*) < m_type->size());
      if (offset->isconstant()) {
        int off = offset->value();
        void* p = reinterpret_cast<void*>(m_value + off);
        const pointer_type* pt = pointer_type::create(T);
        var* ret = new refimm<void*>(pt, p);
        garbage.push_back(ret);
        return ret;
      }
    }
    return var::offref(T, offset);
  }
  var* constant<void*>::offref(const type* T, var* offset)
  {
    if ( offset->isconstant() ){
      int off = offset->value();
      unsigned char* p = reinterpret_cast<unsigned char*>(m_value);
      p += off;
      void* q = reinterpret_cast<void*>(p);
      var* ret = new refimm<void*>(pointer_type::create(T),q);
      garbage.push_back(ret);
      return ret;
    }
    return var::offref(T,offset);
  }
} // end of namespace cxx_compiler


std::vector<cxx_compiler::tac*> cxx_compiler::code;

std::vector<cxx_compiler::var*> cxx_compiler::garbage;

std::string cxx_compiler::new_name(std::string head)
{
  using namespace std;
  ostringstream os;
  static int cnt;
  os << head << cnt++;
  return os.str();
}

namespace cxx_compiler {
  namespace expressions {
    namespace assignment {
      struct table_t : public set<pair<int, int> > {
        table_t()
        {
          insert(make_pair(0, 0));
          insert(make_pair(1, 0));
          insert(make_pair(1, 1));
          insert(make_pair(2, 0));
          insert(make_pair(2, 2));
          insert(make_pair(3, 0));
          insert(make_pair(3, 1));
          insert(make_pair(3, 2));
          insert(make_pair(3, 3));
          insert(make_pair(4, 0));
          insert(make_pair(4, 4));
          insert(make_pair(5, 0));
          insert(make_pair(5, 1));
          insert(make_pair(5, 4));
          insert(make_pair(5, 5));
          insert(make_pair(6, 0));
          insert(make_pair(6, 2));
          insert(make_pair(6, 4));
          insert(make_pair(6, 6));
          for (int i = 0; i != 8 ; ++i)
            insert(make_pair(7, i));
        }
      } table;
      bool include(int x, int y)
      {
        return table.find(make_pair(x, y)) != table.end();
      }
    } // end of namespace assignment
  } // end of namespace expressions
} // end of namespace cxx_compiler

const cxx_compiler::type*
cxx_compiler::expressions::
assignment::valid(const type* T, var* src, bool* discard)
{
  const type* xx = T;
  const type* yy = src->m_type;
  xx = xx->unqualified();
  yy = yy->unqualified();
  if ( xx->arithmetic() && yy->arithmetic() )
    return xx;

  if (xx->m_id == type::RECORD) {
    if (compatible(xx, yy))
      return xx;
    return 0;
  }

  typedef const pointer_type PT;
  if ( xx->m_id == type::POINTER ){
    PT* px = static_cast<PT*>(xx);
    if ( yy->m_id == type::POINTER ){
      PT* py = static_cast<PT*>(yy);
      const type* Tx = px->referenced_type();
      const type* Ty = py->referenced_type();
      int cvr_x = 0, cvr_y = 0;
      Tx = Tx->unqualified(&cvr_x);
      Ty = Ty->unqualified(&cvr_y);
      if (compatible(Tx, Ty)){
        if (!discard || include(cvr_x, cvr_y))
          return px;
        else {
          *discard = true;
          return 0;
        }
      }
      const type* v = void_type::create();
      if (compatible(Tx, v)){
        if ( include(cvr_x, cvr_y))
          return px;
        else {
          if (discard)
            *discard = true;
          return 0;
        }
      }
      if (compatible(Ty, v)) {
        if (include(cvr_x, cvr_y))
          return px;
        else {
          if (discard)
            *discard = true;
          return 0;
        }
      }
      if (Tx->m_id == type::RECORD && Ty->m_id == type::RECORD) {
        typedef const record_type REC;
        REC* rx = static_cast<REC*>(Tx);
        REC* ry = static_cast<REC*>(Ty);
        vector<tag*> dummy;
        bool direct_virtual = false;
        if (ry->base_offset(rx, dummy, &direct_virtual) >= 0) {
          if (include(cvr_x, cvr_y))
            return px;
          else {
            if (discard)
              *discard = true;
            return 0;
          }
        }
      }
    }
    if (yy->integer() && src->zero())
      return xx;
    return 0;
  }

  if (xx->m_id == type::REFERENCE) {
    typedef const reference_type REF;
    REF* ref = static_cast<REF*>(xx);
    const type* T = ref->referenced_type();
    const type* X = src->m_type;
    if (T == X)
      return T;
    if (!T->modifiable() || !X->modifiable())
      return valid(T,src,discard);
    return 0;
  }

  if (xx->m_id == type::POINTER_MEMBER) {
    typedef const pointer_member_type PMT;
    PMT* px = static_cast<PMT*>(xx);
    if (yy->m_id == type::POINTER_MEMBER) {
      PMT* py = static_cast<PMT*>(yy);
      if (px->ctag() == py->ctag()) { 
        const type* Tx = px->referenced_type();
        const type* Ty = py->referenced_type();
        int cvr_x = 0, cvr_y = 0;
        Tx = Tx->unqualified(&cvr_x);
        Ty = Ty->unqualified(&cvr_y);
        if (compatible(Tx, Ty)){
          if (!discard || include(cvr_x, cvr_y))
            return px;
          else {
            *discard = true;
            return 0;
          }
        }
      }
    }
    return 0;
  }
  return 0;
}

bool cxx_compiler::expressions::constant_flag;

cxx_compiler::var* cxx_compiler::var::ppmm(bool plus, bool post)
{
  using namespace std;
  if ( !lvalue() ){
    using namespace error::expressions::ppmm;
    not_lvalue(parse::position,plus,this);
  }
  const type* T = m_type;
  T = T->promotion();
  if ( !T->scalar() ){
    using namespace error::expressions::ppmm;
    not_scalar(parse::position,plus,this);
    return this;
  }
  if ( !T->modifiable() ){
    using namespace error::expressions::ppmm;
    not_modifiable(parse::position,plus,this);
    return this;
  }
  var* one = expressions::primary::literal::integer::create(1);
  if ( T->arithmetic() )
    one = one->cast(T);
  else {
    const type* TT = T->unqualified();
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(TT);
    TT = pt->referenced_type();
    TT = TT->complete_type();
    if ( !(one = TT->vsize()) ){
      int n = TT->size();
      if ( !n ){
        using namespace error::expressions::ppmm;
        invalid_pointer(parse::position,plus,pt);
        n = 1;
      }
      one = expressions::primary::literal::integer::create(n);
    }
  }
  var* ret = new var(T);
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  b ? b->m_vars.push_back(ret) : garbage.push_back(ret);
  if ( post ){
    if (T == m_type) {
      code.push_back(new assign3ac(ret,this));
      if ( plus )
        code.push_back(new add3ac(this,this,one));
      else
        code.push_back(new sub3ac(this,this,one));
    }
    else {
      code.push_back(new cast3ac(ret,this,T));
      var* tmp = new var(T);
      b ? b->m_vars.push_back(tmp) : garbage.push_back(tmp);
      code.push_back(new assign3ac(tmp,ret));
      if ( plus )
        code.push_back(new add3ac(tmp,tmp,one));
      else
        code.push_back(new sub3ac(tmp,tmp,one));
      code.push_back(new cast3ac(this,tmp,m_type));
    }
  }
  else {
    if (T == m_type) {
      if ( plus )
        code.push_back(new add3ac(this,this,one));
      else
        code.push_back(new sub3ac(this,this,one));
      code.push_back(new assign3ac(ret,this));
    }
    else {
      code.push_back(new cast3ac(ret,this,T));
      if ( plus )
        code.push_back(new add3ac(ret,ret,one));
      else
        code.push_back(new sub3ac(ret,ret,one));
      code.push_back(new cast3ac(this,ret,m_type));
    }
  }
  return ret;
}

cxx_compiler::var* cxx_compiler::ref::ppmm(bool plus, bool post)
{
  using namespace std;
  const type* T = m_result;
  T = T->promotion();
  if ( !T->scalar() ){
    using namespace error::expressions::ppmm;
    not_scalar(parse::position,plus,this);
    return this;
  }
  if ( !T->modifiable() ){
    using namespace error::expressions::ppmm;
    not_modifiable(parse::position,plus,this);
    return this;
  }
  var* one = expressions::primary::literal::integer::create(1);
  if ( T->arithmetic() )
    one = one->cast(T);
  else {
    const type* TT = T->unqualified();
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(TT);
    TT = pt->referenced_type();
    int n = TT->size();
    if ( !n ){
      using namespace error::expressions::ppmm;
      invalid_pointer(parse::position,plus,pt);
      n = 1;
    }
    one = expressions::primary::literal::integer::create(n);
  }
  var* ret = rvalue();
  ret = ret->promotion();
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  if ( post ){
    var* tmp = new var(T);
    b ? b->m_vars.push_back(tmp) : garbage.push_back(tmp);
    if ( plus )
      code.push_back(new add3ac(tmp,ret,one));
    else
      code.push_back(new sub3ac(tmp,ret,one));
    assign(tmp);
  }
  else {
    if ( plus )
      code.push_back(new add3ac(ret,ret,one));
    else
      code.push_back(new sub3ac(ret,ret,one));
    assign(ret);
  }
  return ret;
}

cxx_compiler::var* cxx_compiler::generated::ppmm(bool plus, bool post)
{
  using namespace error::expressions::ppmm;
  not_modifiable_lvalue(parse::position,plus,m_org);
  return this;
}

cxx_compiler::expressions::postfix::
fcast::fcast(declarations::type_specifier* ptr, std::vector<base*>* list)
  : m_list(list), m_file(parse::position)
{
  using namespace parse;
  using namespace declarations;
  if (m_list) {
    if (context_t::retry[DECL_FCAST_CONFLICT_STATE]) {
      // Note that `ptr' is already deleted.
      assert(!specifier_seq::info_t::s_stack.empty());
      specifier_seq::info_t* p = specifier_seq::info_t::s_stack.top();
      auto_ptr<specifier_seq::info_t> sweeper(p);
      p->update();
      m_type = p->m_type;
      return;
    }
  }

  assert(!context_t::retry[DECL_FCAST_CONFLICT_STATE]);
  specifier* spec = new specifier(ptr);
  specifier_seq::info_t info(0, spec);
  info.update();
  m_type = info.m_type;
}

cxx_compiler::var* cxx_compiler::expressions::postfix::fcast::gen()
{
  using namespace std;
  vector<var*> arg;
  if ( m_list ) {
    transform(m_list->begin(),m_list->end(),back_inserter(arg),
              mem_fun(&base::gen));
  }

  if (m_type->scalar()) {
    switch (arg.size()) {
    case 0:
      {
        var* zero = primary::literal::integer::create(0);
        return zero->cast(m_type);
      }
    case 1:
      return arg.back()->rvalue()->cast(m_type);
    default:
      {
        error::expressions::postfix::fcast::too_many_arg(m_file);
        var* ret = new var(m_type);
        if (scope::current->m_id == scope::BLOCK) {
          block* b = static_cast<block*>(scope::current);
          b->m_vars.push_back(ret);
        }
        else
          garbage.push_back(ret);
        return ret;
      }
    }
  }

  var* ret = new var(m_type);
  if (scope::current->m_id == scope::BLOCK) {
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  return ret;
}

namespace cxx_compiler {
  using namespace expressions::primary::literal;
  template<> var* refimm<void*>::common()
  {
    if (sizeof(void*) == sizeof(int)) {
      int i = (int)(__int64)m_addr;
      return integer::create(i);
    }
    return integer::create((__int64)m_addr);
  }
  template<> var* refimm<__int64>::common()
  {
    return integer::create(m_addr);
  }
} // end of namespace cxx_compiler
