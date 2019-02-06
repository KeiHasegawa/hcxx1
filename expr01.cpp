// postfix-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

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
  var* offset = size->mul(index);
  assert(offset->m_type->integer());
  return array->offref(T,offset);
}

cxx_compiler::var* cxx_compiler::subscript_impl::size(const type* T)
{
  using namespace std;
  if ( int size = T->size() )
    return expressions::primary::literal::integer::create(size);
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

const cxx_compiler::file_t& cxx_compiler::expressions::postfix::call::file() const
{
  return m_func->file();
}

cxx_compiler::expressions::postfix::call::~call()
{
  using namespace std;
  delete m_func;
  if ( m_arg )
    for_each(m_arg->begin(),m_arg->end(),misc::deleter<base>());
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
  return call_impl::common(ft,func,arg);
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
  FUNC* func = static_cast<FUNC*>(T);
  mark();
  usr* u = static_cast<usr*>(m_ref);
  usr::flag_t flag = u->m_flag;
  if ( flag & usr::INLINE ){
    using namespace declarations::declarators::function;
    string name = u->m_name;
    const vector<const type*>& param = func->param();
    scope* ptr = u->m_scope;
    if ( ptr->m_id == scope::BLOCK )
      ptr = &scope::root;
    KEY key(make_pair(name,ptr),&param);
    map<KEY,definition::static_inline::info_t*>::const_iterator p =
      definition::static_inline::skipped.find(key);
    if ( p != definition::static_inline::skipped.end() )
      return call_impl::common(func,m_ref,arg,p->second);
    else if ( !Inline::resolve::flag ){
      int n = code.size();
      var* v = call_impl::common(func,u,arg);
      if ( error::counter )
        return v;
      int m = code.size();
      for_each(code.begin()+n,code.begin()+m,misc::deleter<tac>());
      code.resize(n);
      const type* T = func->return_type();
      vector<var*> tmp;
      tac* point = new assign3ac(0,0);
      Inline::after* ret = new Inline::after(T,u,arg ? *arg : tmp,point);
      code.push_back(point);
      block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
      if ( b && !T->compatible(void_type::create()) )
        b->m_vars.push_back(ret);
      return ret;
    }
    else
      definition::static_inline::todo::lists.insert(key);
  }
  return call_impl::common(func,m_ref,arg);
}

cxx_compiler::var*
cxx_compiler::member_function::call(std::vector<var*>* arg)
{
  using namespace std;
  using namespace declarations::declarators::function;
  using namespace declarations::declarators::function::definition::static_inline;
  auto_ptr<member_function> sweeper(this);
  typedef const func_type FUNC;
  FUNC* ft = static_cast<FUNC*>(m_fun->m_type);
  info_t* inline_info = 0;
  if ( m_fun->m_flag & usr::INLINE ){
    string name = m_fun->m_name;
    const vector<const type*>& param = ft->param();
    KEY key(make_pair(name,m_fun->m_scope),&param);
    map<KEY,info_t*>::const_iterator p = skipped.find(key);
    assert(p != skipped.end());
    inline_info = p->second;
  }
  return call_impl::common(ft,m_fun,arg,inline_info,false,m_obj);
}

bool cxx_compiler::declarations::declarators::function::Inline::after::expand(KEY key, std::vector<tac*>& dst)
{
  using namespace std;
  typedef const func_type FUNC;
  FUNC* ft = static_cast<FUNC*>(m_func->m_type);
  const vector<const type*>& param = ft->param();
  KEY tmp(make_pair(m_func->m_name,m_func->m_scope),&param);
  if ( tmp != key )
    return false;
  scope* org = scope::current;
  scope::current = m_scope;
  genaddr genaddr(0,0,m_func,0);
  var* ret = genaddr.call(&m_arg);
  scope::current = org;
  bool b = m_type->compatible(void_type::create());
  if ( !b )
    code.push_back(new assign3ac(this,ret));
  vector<tac*>::iterator p = find(dst.begin(),dst.end(),m_point);
  assert(p != dst.end());
  delete *p;
  p = dst.erase(p);
  dst.insert(p,code.begin(),code.end());
  code.clear();
  if ( b )
    delete this;
  return true;
}

cxx_compiler::declarations::declarators::function::Inline::after::after(const type* T, usr* func, const std::vector<var*>& arg, tac* point)
  : var(T), m_func(func), m_arg(arg), m_scope(scope::current), m_point(point)
{
  lists.push_back(this);
}

std::vector<cxx_compiler::declarations::declarators::function::Inline::after*>
cxx_compiler::declarations::declarators::function::Inline::after::lists;

bool cxx_compiler::declarations::declarators::function::Inline::resolve::flag;

namespace cxx_compiler { namespace overload_impl {
  struct result {
    var* m_var;
    bool NG(){ return m_var == 0; }
    result(var* v) : m_var(v) {}
  };
  result* trial(usr*, std::vector<cxx_compiler::var*>*);
} } // end of namespace overload_impl and cxx_compiler

cxx_compiler::var*
cxx_compiler::overload::call(std::vector<cxx_compiler::var*>* arg)
{
  using namespace std;
  using namespace overload_impl;
  const vector<usr*>& u = m_candidacy;
  misc::pvector<result> v;
  transform(u.begin(),u.end(),back_inserter(v),bind2nd(ptr_fun(trial),arg));
  vector<result*>::iterator p = v.begin();
  while ( p != v.end() ){
    p = find_if(p,v.end(),mem_fun(&result::NG));
    if ( p != v.end() ){
      delete *p;
      p = v.erase(p);
    }
  }
  if ( v.size() == 1 )
    return v[0]->m_var;
  throw int();
}

cxx_compiler::overload_impl::result*
cxx_compiler::overload_impl::trial(usr* u, std::vector<cxx_compiler::var*>* arg)
{
  using namespace std;
  const type* T = u->m_type;
  typedef const func_type FUNC;
  FUNC* func = static_cast<FUNC*>(T);
  usr::flag_t flag = u->m_flag;
  if ( flag & usr::INLINE ){
    using namespace declarations::declarators::function;
    string name = u->m_name;
    const vector<const type*>& param = func->param();
    scope* ptr = u->m_scope;
    if ( ptr->m_id == scope::BLOCK )
      ptr = &scope::root;
    KEY key(make_pair(name,ptr),&param);
    map<KEY,definition::static_inline::info_t*>::const_iterator p =
      definition::static_inline::skipped.find(key);
    if ( p != definition::static_inline::skipped.end() )
      return new result(call_impl::common(func,u,arg,p->second,true));
  }
  return new result(call_impl::common(func,u,arg,0,true));
}

namespace cxx_compiler { namespace call_impl {
  std::pair<int,int> num_of_range(const std::vector<const type*>&);
  struct convert {
    const std::vector<const type*>& m_param;
    var* m_func;
    bool m_trial;
    int m_counter;
    convert(const std::vector<const type*>& param, var* func, bool trial)
      : m_param(param), m_func(func), m_counter(-1), m_trial(trial) {}
    var* operator()(var*);
  };
  tac* gen_param(var*);
  namespace inlined {
    tac* assign_param(var*, var*);
  } // end of namespace inlined
  var* ref_vftbl(usr* vf, var* vp);
} } // end of namespace call_impl and cxx_compiler

cxx_compiler::var*
cxx_compiler::call_impl::common(const func_type* ft,
                                var* func,
                                std::vector<var*>* arg,
                                declarations::declarators::function::definition::static_inline::info_t* info,
                                bool trial,
                                var* obj)
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
    info = 0;
  }
  else if ( m.second < n ){
    if ( trial )
      return 0;
    using namespace error::expressions::postfix::call;
    num_of_arg(parse::position,func,n,m.second);
    n = m.second;
    info = 0;
  }
  vector<var*> conved;
  if ( arg ){
    const vector<var*>& v = *arg;
    transform(v.begin(),v.begin()+n,back_inserter(conved),
              call_impl::convert(param,func,trial));
    if ( trial && find(conved.begin(),conved.end(),(var*)0) != conved.end() )
      return 0;
  }
  if ( info ){
    using namespace declarations::declarators::function::definition::static_inline;
    expand::action(info);
    if ( scope::current->m_id == scope::BLOCK ){
      block* b = static_cast<block*>(scope::current);
      vector<scope*>& c = b->m_children;
      c.push_back(info->m_param);
    }
    const vector<usr*>& order = info->m_param->m_order;
    if ( obj ){
      const type* T = obj->m_type;
      if ( T->scalar() )
        conved.insert(conved.begin(),obj);
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
        conved.insert(conved.begin(),tmp);
      }
    }
    transform(order.begin(),order.end(),conved.begin(),back_inserter(code),inlined::assign_param);
    vector<tac*>& v = info->m_expanded;
    copy(v.begin(),v.end(),back_inserter(code));
    v.clear();
  }
  else {
    if ( obj ){
      const type* T = obj->m_type;
      if ( T->scalar() ){
        usr* u = func->usr_cast();
        usr::flag_t flag = u->m_flag;
        if ( flag & usr::VIRTUAL )
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
  }
  const type* T = ft->return_type();
  if ( T )
    T = T->complete_type();
  var* x = new var(T);
  if ( !T || T->compatible(void_type::create()) ){
    if ( !info )
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
  if ( info )
    code.push_back(new assign3ac(x,info->m_ret));
  else
    code.push_back(new call3ac(x,func));
  return x;
}

void cxx_compiler::genaddr::mark()
{
  using namespace std;
  using namespace declarations::declarators::function;
  usr* u = static_cast<usr*>(m_ref);
  usr::flag_t flag = u->m_flag;
  if ( !(flag & usr::INLINE) && (flag & usr::STATIC)
       || u->m_scope->m_id == scope::BLOCK ){
    string name = u->m_name;
    const type* T = u->m_type;
    typedef const func_type FUNC;
    FUNC* func = static_cast<FUNC*>(T);
    const vector<const type*>& param = func->param();
    scope* ptr = u->m_scope;
    if ( ptr->m_id == scope::BLOCK )
      ptr = &scope::root;
    KEY key(make_pair(name,ptr),&param);
    if ( definition::table.find(key) != definition::table.end() ){
      if ( definition::static_inline::skipped.find(key) != definition::static_inline::skipped.end() )
        definition::static_inline::todo::lists.insert(key);
    }
    else
      definition::static_inline::refed[key] = make_pair(parse::position,flag);
  }
}

cxx_compiler::tac* cxx_compiler::call_impl::gen_param(var* y)
{
  return new param3ac(y);
}

cxx_compiler::tac* cxx_compiler::call_impl::inlined::assign_param(var* x, var* y)
{
  return new assign3ac(x,y);
}

#ifdef _MSC_VER
#undef max
#endif // _MSC_VER

std::pair<int,int> cxx_compiler::call_impl::num_of_range(const std::vector<const type*>& param)
{
  using namespace std;
  const type* T = param.back();
  if ( T->m_id == type::ELLIPSIS )
    return make_pair(param.size()-1,INT_MAX);
  if ( T->compatible(void_type::create()) )
    return make_pair(0,0);
  else
    return make_pair(param.size(),param.size());
}

cxx_compiler::var* cxx_compiler::call_impl::convert::operator()(var* arg)
{
  using namespace std;
  arg = arg->rvalue();
  if ( ++m_counter == m_param.size() )
    --m_counter;
  const type* T = m_param[m_counter];
  T = T->complete_type();
  if ( T->m_id == type::ELLIPSIS ){
    const type* type = arg->m_type;
    if ( type->compatible(type->varg()) )
      return arg;
    T = type->varg();
  }
  T = T->unqualified();
  bool discard = false;
  T = expressions::assignment::valid(T,arg,&discard);
  if ( !T ){
    if ( m_trial )
      return 0;
    using namespace error::expressions::postfix::call;
    mismatch_argument(parse::position,m_counter,discard,m_func);
    return arg;
  }
  return arg->cast(T);
}

cxx_compiler::var* cxx_compiler::call_impl::ref_vftbl(usr* vf, var* vp)
{
  using namespace std;
  scope* ptr = vf->m_scope;
  assert(ptr->m_id == scope::TAG);
  tag* Tg = static_cast<tag*>(ptr);
  const type* T = Tg->m_types.second;
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
  if ( !rec->vbtbl() ){
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
    var* delta = expressions::primary::literal::integer::create(pt->size());
    code.push_back(new add3ac(t0,vp,delta));
    code.push_back(new invraddr3ac(t1,t0));
    tmp = t1;
  }
  var* res = new var(pt);
  if ( b )
    b->m_vars.push_back(res);
  else
    garbage.push_back(res);
  usr* vftbl = rec->vftbl();
  assert(vftbl);
  const vector<usr*>& contents = rec->vftbl_contents();
  vector<usr*>::const_iterator p = find(contents.begin(),contents.end(),vf);
  assert(p != contents.end());
  int index = distance(contents.begin(),p);
  if ( index ){
    var* t2 = new var(pt);
    if ( b )
      b->m_vars.push_back(t2);
    else
      garbage.push_back(t2);
    var* delta = expressions::primary::literal::integer::create(pt->size());
    code.push_back(new add3ac(t2,tmp,delta));
    tmp = t2;
  }
  code.push_back(new invraddr3ac(res,tmp));
  return res;
}

std::stack<cxx_compiler::expressions::postfix::member::info_t*>
cxx_compiler::expressions::postfix::member::handling;

cxx_compiler::expressions::postfix::member::info_t*
cxx_compiler::expressions::postfix::member::begin(base* expr, bool dot)
{
  using namespace std;
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
  tag* Tag = T->get_tag();
  if ( Tag && Tag->m_kind != tag::ENUM ){
    scope::current = Tag;
    parse::identifier::flag = parse::identifier::member;
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
  scope::current = info->m_scope;
  parse::identifier::flag = parse::identifier::look;
  if ( !handling.empty() )
    handling.pop();
  return info;
}

cxx_compiler::var* cxx_compiler::expressions::postfix::member::info_t::gen()
{
  using namespace std;
  using namespace declarations::declarators::function::definition::static_inline;
  copy(m_code.begin(),m_code.end(),back_inserter(code));
  return m_expr->member(m_member,m_dot);
}

cxx_compiler::var* cxx_compiler::var::member(var* expr, bool dot)
{
  using namespace std;
  const type* T = result_type();
  int cvr = 0;
  T = T->unqualified(dot ? &cvr : 0);
  typedef const pointer_type PT;
  if ( !dot ){
    if ( T->m_id == type::POINTER ){
      PT* pt = static_cast<PT*>(T);
      T = pt->referenced_type();
      T = T->unqualified(&cvr);
    }
    else
      return this;
  }
  T = T->complete_type();
  if ( T->m_id != type::RECORD )
    return this;
  typedef const record_type REC;
  REC* rec = static_cast<REC*>(T);
  if ( genaddr* addr = expr->genaddr_cast() )
    expr = addr->m_ref;
  usr* member = expr->usr_cast();
  if ( !member )
    return expr;
  T = member->m_type;
  if ( T->m_id == type::FUNC )
    return new member_function(this,member);
  pair<int, usr*> off = rec->offset(member->m_name);
  int offset = off.first;
  if ( offset < 0 )
    return this;
  if ( member->m_flag & usr::BIT_FIELD ){
    int pos = rec->position(member);
    typedef const bit_field_type BF;
    BF* bf = static_cast<BF*>(T);
    T = bf->integer_type();
    PT* pt = pointer_type::create(T);
    int bit = bf->bit();
    var* ret = new refbit(pt,this,offset,member,pos,bit,dot);
    garbage.push_back(ret);
    return ret;
  }
  T = T->qualified(cvr);
  return offref(T,expressions::primary::literal::integer::create(offset));
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
  if ( int n = m_addrof.m_offset )
    offset = offset->add(expressions::primary::literal::integer::create(n));
  var* ret = new refsomewhere(pointer_type::create(T),m_addrof.m_ref,offset);
  garbage.push_back(ret);
  return ret;
}

cxx_compiler::var* cxx_compiler::refsomewhere::offref(const type* T, var* offset)
{
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
  if ( m_offset )
    offset = offset->add(expressions::primary::literal::integer::create(m_offset));
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
  if ( m_offset )
    offset = offset->add(expressions::primary::literal::integer::create(m_offset));
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

cxx_compiler::var* cxx_compiler::constant<void*>::offref(const type* T, var* offset)
{
  if ( offset->isconstant() ){
    int off = offset->value();
    unsigned char* p = reinterpret_cast<unsigned char*>(m_value);
    p += off;
    var* ret = new refimm(pointer_type::create(T),reinterpret_cast<void*>(p));
    garbage.push_back(ret);
    return ret;
  }
  return var::offref(T,offset);
}

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


const cxx_compiler::type* cxx_compiler::expressions::assignment::valid(const type* T, var* src, bool* discard)
{
  const type* xx = T;
  const type* yy = src->m_type;
  xx = xx->unqualified();
  yy = yy->unqualified();
  if ( xx->arithmetic() && yy->arithmetic() )
    return xx;

  if ( xx->m_id == type::RECORD ){
    typedef const record_type REC;
    REC* rec = static_cast<REC*>(xx);
    if ( rec->compatible(yy) )
      return rec;
    return 0;
  }

  typedef const pointer_type PT;
  if ( xx->m_id == type::POINTER ){
    PT* lp = static_cast<PT*>(xx);
    if ( yy->m_id == type::POINTER ){
      PT* rp = static_cast<PT*>(yy);
      if ( lp->compatible(rp) ){
        if ( !discard || lp->include_cvr(rp) )
          return lp;
        else {
          *discard = true;
          return 0;
        }
      }
      const type* vp = pointer_type::create(void_type::create());
      if ( vp->compatible(lp) ){
        if ( lp->include_cvr(rp) )
          return lp;
        else {
          *discard = true;
          return 0;
        }
      }
      if ( vp->compatible(rp) ){
        if ( lp->include_cvr(rp) )
          return lp;
        else {
          *discard = true;
          return 0;
        }
      }
    }
  }

  if ( xx->m_id == type::POINTER ){
    if ( yy->integer() && src->zero() )
      return xx;
  }

  if ( xx->m_id == type::REFERENCE ){
    typedef const reference_type REF;
    REF* ref = static_cast<REF*>(xx);
    const type* T = ref->referenced_type();
    return valid(T,src,discard);
  }
  return 0;
}

bool cxx_compiler::expressions::constant_flag;

std::map<__int64,cxx_compiler::constant<double>*> cxx_compiler::constant<double>::table;
std::map<int,cxx_compiler::constant<float>*> cxx_compiler::constant<float>::table;

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
    if ( T->compatible(m_type) ){
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
    if ( T->compatible(m_type) ){
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

cxx_compiler::var* cxx_compiler::expressions::postfix::fcast::gen()
{
  using namespace std;
  using namespace declarations;
  specifier_seq::info_t tmp(0,new specifier(m_type_specifier));
  tmp.update();
  const type* T = tmp.m_type;
  vector<var*> arg;
  if ( m_list )
    transform(m_list->begin(),m_list->end(),back_inserter(arg),mem_fun(&base::gen));
  if ( arg.empty() ){
    var* zero = primary::literal::integer::create(0);
    return zero->cast(T);
  }
  else
    return arg.back()->cast(T);
}
