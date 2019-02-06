#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

const cxx_compiler::type* cxx_compiler::declarations::declarators::pointer::action(std::vector<int>* p)
{
  using namespace std;
  auto_ptr<vector<int> > sweeper(p);
  const type* T = pointer_type::create(backpatch_type::create());
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
cxx_compiler::declarations::declarators::pointer::action(const type* pointer, const type* T)
{
  return T->patch(pointer,0);
}

const cxx_compiler::type* cxx_compiler::declarations::declarators::reference::action()
{
  return reference_type::create(backpatch_type::create());
}

const cxx_compiler::type*
cxx_compiler::declarations::declarators::function::action(const type* T,
                                                          std::vector<const type*>* pdc,
                                                          var* v,
                                                          std::vector<int>* cvr)
{
  using namespace std;
  auto_ptr<vector<const type*> > sweeper1(pdc);
  auto_ptr<vector<int> > sweeper2(cvr);
  usr* u = v ? v->usr_cast() : 0;
  if ( !u && v ){
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
  if ( pdc )
    return T->patch(func_type::create(backpatch_type::create(),*pdc),u);
  else {
    vector<const type*> param;
    param.push_back(void_type::create());
    return T->patch(func_type::create(backpatch_type::create(),param),u);
  }
}

const cxx_compiler::type*
cxx_compiler::declarations::declarators::function::parameter(specifier_seq::info_t* p,
                                                             var* v)
{
  using namespace std;
  usr* u = static_cast<usr*>(v);
  struct sweeper2 {
    ~sweeper2()
    {
      if ( cxx_compiler_char == ',' )
        cxx_compiler::declarations::specifier_seq::info_t::clear();
    }
  } sweeper2;
  auto_ptr<specifier_seq::info_t> sweepr(p);
  usr::flag_t mask = usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO);
  if ( u ){
    const type* T = u->m_type;
    typedef const pointer_type PT;
    if ( PT* pt = T->ptr_gen() )
      u->m_type = pt;
    usr::flag_t& flag = u->m_flag = p->m_flag;
    if ( flag & mask ){
      error::declarations::declarators::function::parameter::invalid_storage(parse::position,u);
      flag = usr::flag_t(flag & ~mask);
    }
    declarations::action1(u,false,false);
    return u->m_type;
  }
  else {
    if ( !p->m_type || !p->m_tmp.empty() )
      p->update();
    usr::flag_t flag = p->m_flag;
    if ( flag & mask )
      error::declarations::declarators::function::parameter::invalid_storage(parse::position,0);
    return p->m_type;
  }
}

const cxx_compiler::type*
cxx_compiler::declarations::declarators::function::parameter(specifier_seq::info_t* p,
                                                             const type* T)
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
  string name = new_name(".param");
  usr* u = new usr(name,T,usr::NONE,parse::position);
  typedef const pointer_type PT;
  if ( PT* pt = T->ptr_gen() )
    u->m_type = pt;
  usr::flag_t& flag = u->m_flag = p->m_flag;
  usr::flag_t mask = usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO);
  if ( flag & mask ){
    error::declarations::declarators::function::parameter::invalid_storage(parse::position,u);
    flag = usr::flag_t(flag & ~mask);
  }
  declarations::action1(u,false,false);
  return u->m_type;
}

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
  int x = code.size();
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
      if ( scope::current != &scope::root ){
        int y = code.size();
        if ( x == y )
          return variable_length::action(T,v,u);
        else {
          vector<tac*> tmp;
          copy(code.begin()+x,code.begin()+y,back_inserter(tmp));
          code.resize(x);
          return variable_length::action(T,v,tmp,u);
        }
      }
      using namespace error::declarations::declarators::vm;
      file_scope(u);
      v = expressions::primary::literal::integer::create(1);
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
  return T->patch(array_type::create(backpatch_type::create(),dim),u);
}

const cxx_compiler::type*
cxx_compiler::declarations::declarators::array::variable_length::action(const type* T, var* dim, usr* u)
{
  return T->patch(varray_type::create(backpatch_type::create(),dim),u);
}

const cxx_compiler::type*
cxx_compiler::declarations::declarators::array::variable_length::action(const type* T, var* dim, const std::vector<tac*>& c, usr* u)
{
  return T->patch(varray_type::create(backpatch_type::create(),dim,c),u);
}

void
cxx_compiler::declarations::declarators::array::variable_length::allocate(usr* u)
{
  if ( scope::current == &scope::root )
    return;
  assert(scope::current->m_parent != &scope::root);
  usr::flag_t& flag = u->m_flag;
  usr::flag_t mask = usr::flag_t(usr::STATIC | usr::EXTERN);
  if ( flag & mask ){
    using namespace error::declarations::declarators::array::variable_length;
    invalid_storage(u);
    flag = usr::flag_t(flag & ~mask);
  }
  statements::label::mark_vm(u);
  if ( flag & usr::TYPEDEF )
    return;
  const type* T = u->m_type;
  int n = code.size();
  var* size = T->vsize();
  int m = code.size();
  code.push_back(new alloca3ac(u,size));
}

cxx_compiler::fundef* cxx_compiler::fundef::current;

namespace cxx_compiler { namespace declarations { namespace declarators { namespace function { namespace definition {
  bool valid(const type*, usr*);
} } } } }

void
cxx_compiler::declarations::declarators::function::definition::begin(declarations::specifier_seq::info_t* p, var* v)
{
  using namespace std;
  usr* u = v->usr_cast();
  if ( !u ){
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
  auto_ptr<declarations::specifier_seq::info_t> sweeper(p);
  parse::identifier::flag = parse::identifier::look;
  declarations::action1(u,false, u != v);
  vector<scope*>& children = scope::current->m_children;
  if ( children.empty() ){
    using namespace error::declarations::declarators::function::definition;
    invalid(parse::position);
    scope* param = new scope;
    param->m_parent = scope::current;
    children.push_back(param);
  }
  scope* param = children.back();
  const vector<usr*>& order = param->m_order;
  for_each(order.begin(),order.end(),check_object);
  fundef::current = new fundef(u,param);
  {
    string name = u->m_name;
    usr::flag_t& flag = u->m_flag;
    if ( flag & usr::TYPEDEF ){
      using namespace error::declarations::declarators::function::definition;
      typedefed(parse::position);
      flag = usr::flag_t(flag & ~usr::TYPEDEF);
    }
    const type* T = u->m_type;
    if ( T->m_id != type::FUNC )
      return;
    typedef const func_type FUNC;
    FUNC* func = static_cast<FUNC*>(T);
    T = func->return_type();
    if ( !valid(T,u) ){
      using namespace error::declarations::declarators::function::definition;
      invalid_return(fundef::current->m_usr,T);
    }
    const vector<const type*>& param = func->param();
    scope* ptr = u->m_scope;
    if ( ptr->m_id == scope::BLOCK )
      ptr = &scope::root;
    KEY key(make_pair(name,ptr),&param);
    TABLE::const_iterator p = table.find(key);
    if ( p != table.end() ){
      using namespace error::declarations::declarators::function::definition;
      multiple(parse::position,p->second);
    }
    else
      table[key] = u;
  }
}

bool
cxx_compiler::declarations::declarators::function::definition::valid(const type* T, usr* func)
{
  if ( !T )
    return true;
  if ( T->compatible(void_type::create()) )
    return true;
  if ( !T->size() ){
    scope* ptr = func->m_scope;
    if ( ptr->m_id != scope::TAG )
      return false;
    tag* Tg = static_cast<tag*>(ptr);
    if ( T->m_id != type::INCOMPLETE_TAGGED )
      return false;
    typedef const incomplete_tagged_type IT;
    IT* it = static_cast<IT*>(T);
    return Tg == it->get_tag();
  }
  return true;
}

namespace cxx_compiler { namespace declarations { namespace declarators { namespace function { namespace Inline { namespace remember {
  extern void action(std::vector<tac*>&);
} } } } } } // end of namespace remember, Inline, function, declarators, declarations and cxx_compiler

namespace cxx_compiler { namespace declarations { namespace declarators { namespace function { namespace definition { namespace static_inline {
  extern void remember(std::vector<tac*>&);
} } } } } } // end of namespace static_inline, definition, function, declarators, declarations and cxx_compiler

void cxx_compiler::declarations::declarators::function::definition::action(statements::base* stmt)
{
  using namespace std;
  auto_ptr<statements::base> sweeper(stmt);
  usr* u = fundef::current->m_usr;
  using namespace parse::member_function_body;
  const map<usr*, saved>& tbl = parse::member_function_body::table;
  map<usr*, saved>::const_iterator p = tbl.find(u);
  if ( p != tbl.end() && !parse::member_function_body::g_restore.m_saved ){
    const saved& tmp = p->second;
    scope* ptr = tmp.m_param;
    ptr = ptr->m_parent;
    vector<scope*>& c = ptr->m_children;
    assert(c.size() == 1);
    c.clear();
  }
  else {
    file_t org = parse::position;
    stmt->gen();
    parse::position = org;
    statements::label::check();
    statements::label::clear();
    action(code,true);
    if ( scope::current->m_id == scope::TAG ){
      scope::current = scope::current->m_parent;
      usr::flag_t flag = u->m_flag;
      if ( !(flag & usr::INLINE) )
        destroy();
    }
    else
      destroy();
  }
  delete fundef::current;
  fundef::current = 0;
}

void cxx_compiler::declarations::declarators::function::definition::action(std::vector<tac*>& v, bool skip)
{
  using namespace std;
  if ( !Inline::after::lists.empty() ){
    Inline::remember::action(v);
    return;
  }
  if ( !error::counter )
    optimize::action(v);
  if ( skip ){
    usr* u = fundef::current->m_usr;
    usr::flag_t flag = u->m_flag;
    usr::flag_t mask = usr::flag_t(usr::INLINE | usr::STATIC);
    if ( (flag & mask) && !(flag & usr::EXTSTATIC) ){
      string name = u->m_name;
      const type* T = u->m_type;
      typedef const func_type FUNC;
      FUNC* func = static_cast<FUNC*>(T);
      const vector<const type*>& param = func->param();
      scope* ptr = u->m_scope;
      if ( ptr->m_id == scope::BLOCK )
        ptr = &scope::root;
      KEY key(make_pair(name,ptr),&param);
      map<KEY, pair<file_t,usr::flag_t> >::iterator p =
        static_inline::refed.find(key);
      if ( p != static_inline::refed.end() )
        static_inline::refed.erase(p);
      else {
        static_inline::remember(v);
        return;
      }
    }
  }
  if ( cmdline::output_medium ){
    if ( cmdline::output_optinfo )
      cout << "\nAfter optimization\n";
    usr* u = fundef::current->m_usr;
    scope* org = scope::current;
    scope::current = &scope::root;
    cout << dump::names::ref(u) << ":\n";
    scope::current = org;
    typedef vector<tac*>::const_iterator IT;
    for ( IT p = v.begin() ; p != v.end() ; ++p )
      cout << '\t', dump::tac(cout,*p), cout << '\n';
    cout << '\n';
    dump::scope();
  }
  if ( !error::counter ){
    if ( generator::generate ){
      generator::interface_t tmp = {
        &scope::root,
        fundef::current,
        &v
      };
      generator::generate(&tmp);
    }
  }
}

namespace cxx_compiler { namespace declarations { namespace declarators { namespace function { namespace Inline { namespace remember { namespace update {
  extern void action(KEY);
} } } } } } } // end of namespace update, remember, Inline, function, declarators, definitions and c_compiler

void cxx_compiler::declarations::declarators::function::definition::static_inline::remember(std::vector<tac*>& v)
{
  using namespace std;
  usr* u = fundef::current->m_usr;
  string name = u->m_name;
  const type* T = u->m_type;
  typedef const func_type FUNC;
  FUNC* func = static_cast<FUNC*>(T);
  const vector<const type*>& param = func->param();
  scope* ptr = u->m_scope;
  if ( ptr->m_id == scope::BLOCK )
    ptr = &scope::root;
  KEY key(make_pair(name,ptr),&param);
  static_inline::skipped[key] = new static_inline::info_t(fundef::current,v);
  scope::root.m_children.clear();
  v.clear();
  usr::flag_t flag = fundef::current->m_usr->m_flag;
  if ( flag & usr::INLINE )
    Inline::remember::update::action(key);
  fundef::current = 0;
}

namespace cxx_compiler { namespace declarations { namespace declarators { namespace function { namespace Inline { namespace remember { namespace update {
  extern int handler(KEY, std::vector<KEY>*);
} } } } } } } // end of namespace update, remember, Inline, function, declarators, definitions and c_compiler

void cxx_compiler::declarations::declarators::function::Inline::remember::update::action(KEY key)
{
  using namespace std;
  vector<KEY> keys;
  handler(key,&keys);
  for_each(keys.begin(),keys.end(),action);
}

namespace cxx_compiler { namespace declarations { namespace declarators { namespace function { namespace Inline { namespace remember {
  struct info_t {
    fundef* m_fundef;
    std::vector<tac*> m_code;
    std::vector<after*> m_list;
    info_t(fundef* a, const std::vector<tac*>& b, const std::vector<after*>& c)
      : m_fundef(a), m_code(b), m_list(c) {}
    ~info_t();
  };
  KEY key(info_t* info)
  {
    using namespace std;
    usr* u = info->m_fundef->m_usr;
    const type* T = u->m_type;
    typedef const func_type FUNC;
    FUNC* func = static_cast<FUNC*>(T);
    const vector<const type*>& param = func->param();
    return KEY(make_pair(u->m_name,u->m_scope),&param);
  }
  std::list<info_t*> m_todo;
  extern bool finish(info_t*, KEY);
} } } } } } // end of namespace remember, Inline, function, declarators, declarations and cxx_compiler

void cxx_compiler::declarations::declarators::function::Inline::resolve::action()
{
  using namespace std;
  vector<KEY> keys;
  transform(remember::m_todo.begin(),remember::m_todo.end(),back_inserter(keys),remember::key);
  flag = true;
  vector<KEY> dummy;
  for_each(keys.begin(),keys.end(),bind2nd(ptr_fun(remember::update::handler),&dummy));
  function::definition::static_inline::todo::action();
  for_each(garbage.begin(),garbage.end(),misc::deleter<var>());
  garbage.clear();
}

cxx_compiler::declarations::declarators::function::Inline::remember::info_t::~info_t()
{
  using namespace std;
  usr::flag_t flag = m_fundef->m_usr->m_flag;
  if ( !(flag & usr::INLINE) ){
    scope* param = m_fundef->m_param;
    delete param;
    delete m_fundef;
    for_each(m_code.begin(),m_code.end(),misc::deleter<tac>());
  }
  fundef::current = 0;
}

int
cxx_compiler::declarations::declarators::function::Inline::remember::update::handler(KEY in, std::vector<KEY>* keys)
{
  using namespace std;
  list<info_t*> tmp;
  while ( !m_todo.empty() ){
    Inline::remember::info_t* info = m_todo.front();
    m_todo.pop_front();
    KEY key = remember::key(info);
    usr::flag_t flag = info->m_fundef->m_usr->m_flag;
    if ( finish(info,in) ){
      if ( flag & usr::INLINE )
        keys->push_back(key);
    }
    else
      tmp.push_back(info);
  }
  m_todo = tmp;
  return 0;
}

bool
cxx_compiler::declarations::declarators::function::Inline::remember::finish(info_t* info, KEY key)
{
  using namespace std;
  vector<after*>& v = info->m_list;
  assert(!v.empty());
  typedef vector<after*>::iterator IT;
  for ( IT p = v.begin() ; p != v.end() ; ){
    if ( (*p)->expand(key,info->m_code) )
      p = v.erase(p);
    else
      ++p;
  }
  if ( !v.empty() )
    return false;
  scope* param = info->m_fundef->m_param;
  scope::root.m_children.push_back(param);
  fundef::current = info->m_fundef;
  function::definition::action(info->m_code,true);
  if ( !scope::root.m_children.empty() )
    scope::root.m_children.pop_back();
  delete info;
  return true;
}

void cxx_compiler::declarations::declarators::function::Inline::remember::action(std::vector<tac*>& v)
{
  optimize::remember_action(v);
  m_todo.push_back(new info_t(fundef::current,v,after::lists));
  scope::root.m_children.clear();
  v.clear();
  fundef::current = 0;
  after::lists.clear();
}

cxx_compiler::declarations::declarators::function::definition::TABLE
cxx_compiler::declarations::declarators::function::definition::table;

cxx_compiler::declarations::declarators::function::definition::static_inline::skipped_t
cxx_compiler::declarations::declarators::function::definition::static_inline::skipped;

cxx_compiler::declarations::declarators::function::definition::static_inline::info_t::~info_t()
{
  using namespace std;
  if ( m_delete ){
    scope* param = m_fundef->m_param;
    delete param;
  }
  delete m_fundef;
  for_each(m_code.begin(),m_code.end(),misc::deleter<tac>());
  for_each(m_expanded.begin(),m_expanded.end(),misc::deleter<tac>());
}

std::set<cxx_compiler::declarations::declarators::function::KEY>
cxx_compiler::declarations::declarators::function::definition::static_inline::todo::lists;

void cxx_compiler::declarations::declarators::function::definition::static_inline::todo::action()
{
  using namespace std;
  set<KEY>::iterator p = lists.begin();
  while ( p != lists.end() ){
    map<KEY,info_t*>::iterator q = skipped.find(*p);
    if ( q != skipped.end() ){
      gencode(q->second);
      set<KEY>::iterator r = p++;
      lists.erase(r);
    }
    else
      ++p;
  }
}


std::map<cxx_compiler::declarations::declarators::function::KEY, std::pair<cxx_compiler::file_t,cxx_compiler::usr::flag_t> >
cxx_compiler::declarations::declarators::function::definition::static_inline::refed;

void
cxx_compiler::declarations::declarators::function::definition::static_inline::gencode(info_t* info)
{
  scope* param = info->m_fundef->m_param;
  scope::root.m_children.push_back(param);
  fundef::current = info->m_fundef;
  action(info->m_code,false);
  scope::root.m_children.pop_back();
}

void
cxx_compiler::declarations::declarators::function::definition::static_inline::blame(const std::pair<KEY,std::pair<file_t,usr::flag_t> >& p)
{
  using namespace std;
  using namespace error::declarations::declarators::function::definition::static_inline;
  const KEY key = p.first;
  string name = key.first.first;
  const file_t& ref = p.second.first;
  usr::flag_t flag = p.second.second;
  if ( flag & usr::STATIC )
    nodef(parse::position,name,ref);
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


namespace cxx_compiler { namespace declarations { namespace declarators { namespace function { namespace definition { namespace static_inline { namespace expand {
  block* create(const scope*);
  struct arg {
    std::vector<tac*>* m_expand;
    std::vector<goto3ac*>* m_returns;
    var* m_ret;
    dup::patch* m_patch;
  };
  int conv(tac*, arg*);
} } } } } } } // expand, static_inline, definition, function, declarators, declarations and cxx_compiler

void cxx_compiler::declarations::declarators::function::definition::static_inline::expand::action(info_t* info)
{
  using namespace std;
  fundef* def = info->m_fundef;
  info->m_param = create(def->m_param);
  const type* T = def->m_usr->m_type;
  typedef const func_type FUNC;
  FUNC* func = static_cast<FUNC*>(T);
  T = func->return_type();
  if ( T && !T->compatible(void_type::create()) ){
    info->m_ret = new var(T);
    info->m_ret->m_scope = info->m_param;
    vector<scope*>& c = info->m_param->m_children;
    assert(!c.empty());
    block* b = static_cast<block*>(c[0]);
    b->m_vars.push_back(info->m_ret);
  }
  const vector<tac*>& v = info->m_code;
  vector<goto3ac*> returns;
  dup::patch patch;
  arg arg = { &info->m_expanded, &returns, info->m_ret, &patch };
  for_each(v.begin(),v.end(),bind2nd(ptr_fun(conv),&arg));
  map<goto3ac*,goto3ac*>& s = patch.m_goto;
  map<to3ac*,to3ac*>& t = patch.m_to;
  for_each(s.begin(),s.end(),bind2nd(ptr_fun(dup::spatch),&t));
  for_each(t.begin(),t.end(),bind2nd(ptr_fun(dup::tpatch),&s));
  to3ac* to = new to3ac;
  info->m_expanded.push_back(to);
  for_each(returns.begin(),returns.end(),bind2nd(ptr_fun(misc::update),to));
  copy(returns.begin(),returns.end(),back_inserter(to->m_goto));
}

namespace cxx_compiler { namespace declarations { namespace declarators { namespace function { namespace definition { namespace static_inline { namespace expand {
  usr* new_usr(usr*, scope*);
  int install(usr*);
  scope* new_block(scope*, scope*);
} } } } } } } // expand, static_inline, definition, function, declarators, declarations and cxx_compiler

cxx_compiler::block*
cxx_compiler::declarations::declarators::function::definition::static_inline::expand::create(const scope* param)
{
  using namespace std;
  symtab::table.clear();
  block* ret = new block;
  ret->m_parent = scope::current;
  const vector<usr*>& o = param->m_order;
  vector<usr*>& v = ret->m_order;
  transform(o.begin(),o.end(),back_inserter(v),bind2nd(ptr_fun(new_usr),ret));
  for_each(v.begin(),v.end(),install);
  const vector<scope*>& c = param->m_children;
  transform(c.begin(),c.end(),back_inserter(ret->m_children),bind2nd(ptr_fun(new_block),ret));
  return ret;
}

cxx_compiler::usr*
cxx_compiler::declarations::declarators::function::definition::static_inline::expand::new_usr(usr* u, scope* s)
{
  usr* ret = new usr(*u);
  ret->m_scope = s;
  symtab::table[u] = ret;
  return ret;
}

int
cxx_compiler::declarations::declarators::function::definition::static_inline::expand::install(usr* u)
{
  using namespace std;
  string name = u->m_name;
  map<string, vector<usr*> >& usrs = u->m_scope->m_usrs;
  usrs[name].push_back(u);
  return 0;
}

namespace cxx_compiler { namespace declarations { namespace declarators { namespace function { namespace definition { namespace static_inline { namespace expand {
  var* new_var(var*, scope*);
} } } } } } } // expand, static_inline, definition, function, declarators, declarations and cxx_compiler

cxx_compiler::scope*
cxx_compiler::declarations::declarators::function::definition::static_inline::expand::new_block(scope* ptr, scope* parent)
{
  using namespace std;
  block* ret = new block;
  ret->m_parent = parent;
  assert(ptr->m_id == scope::BLOCK);
  block* b = static_cast<block*>(ptr);
  const map<string, vector<usr*> >& u = b->m_usrs;
  map<string, vector<usr*> >& d = ret->m_usrs;
  typedef map<string, vector<usr*> >::const_iterator IT;
  for ( IT p = u.begin() ; p != u.end() ; ++p ){
    string name = p->first;
    const vector<usr*>& v = p->second;
    transform(v.begin(),v.end(),back_inserter(d[name]),bind2nd(ptr_fun(new_usr),ret));
  }
  const vector<var*>& v = b->m_vars;
  transform(v.begin(),v.end(),back_inserter(ret->m_vars),bind2nd(ptr_fun(new_var),ret));
  const vector<scope*>& c = b->m_children;
  transform(c.begin(),c.end(),back_inserter(ret->m_children),bind2nd(ptr_fun(new_block),ret));
  return ret;
}

cxx_compiler::var*
cxx_compiler::declarations::declarators::function::definition::static_inline::expand::new_var(var* v, scope* s)
{
  var* ret = new var(*v);
  ret->m_scope = s;
  return symtab::table[v] = ret;
}

int
cxx_compiler::declarations::declarators::function::definition::static_inline::expand::conv(tac* tac, arg* pa)
{
  using namespace std;
  if ( tac->m_id == tac::RETURN ){
    if ( var* y = tac->y ){
      map<var*,var*>::const_iterator p = symtab::table.find(y);
      if ( p != symtab::table.end() )
        y = p->second;
      pa->m_expand->push_back(new assign3ac(pa->m_ret,y));
    }
    goto3ac* go = new goto3ac;
    pa->m_returns->push_back(go);
    pa->m_expand->push_back(go);
  }
  else
    pa->m_expand->push_back(dup::filter(tac,pa->m_patch));
  return 0;
}

std::map<cxx_compiler::declarations::declarators::function::KEY, std::vector<cxx_compiler::usr*> >
cxx_compiler::declarations::declarators::function::Inline::decled;

void cxx_compiler::declarations::declarators::function::Inline::nodef(const std::pair<KEY, std::vector<usr*> >& p)
{
  using namespace std;
  const vector<usr*>& v = p.second;
  for_each(v.begin(),v.end(),error::declarations::specifier_seq::function::Inline::no_definition);
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
