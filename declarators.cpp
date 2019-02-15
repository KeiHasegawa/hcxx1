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
  typedef const pointer_type PT;
  if ( u ){
    usr::flag_t flag = u->m_flag = p->m_flag;
    const type* T = u->m_type;
    if ( PT* pt = T->ptr_gen() )
      u->m_type = pt;
    if ( flag & mask ){
      error::declarations::declarators::function::parameter::invalid_storage(parse::position,u);
      flag = u->m_flag = p->m_flag = usr::flag_t(flag & ~mask);
    }
    u = declarations::action1(u,false,false);
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
	  error::declarations::declarators::function::parameter::invalid_storage(parse::position, 0);
	  flag = p->m_flag = usr::flag_t(flag & ~mask);
	}
    const type* T = p->m_type;
    if (PT* pt = T->ptr_gen())
      T = pt;
    return T;
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
  u = declarations::action1(u,false,false);
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
  u = declarations::action1(u,false, u != v);
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
    table_t::const_iterator p = table.find(key);
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
  if (!T)
    return true;
  T = T->unqualified();
  if (T->m_id == type::VOID)
    return true;
  if (!T->size()) {
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
  using namespace std;
  extern void remember(fundef*, vector<tac*>&);
  skip::table_t skip::table;
  namespace defer {
    map<string, vector<ref_t> > refs;
    map<string, set<usr*> > callers;
    map<usr*, vector<int> > positions;
  }
} } } } } } // end of namespace static_inline, definition, function, declarators, declarations and cxx_compiler

namespace cxx_compiler {
  std::vector<FUNCS_ELEMENT_TYPE> funcs;
} // end of namespace cxx_compiler

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
    action(fundef::current, code);
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

void cxx_compiler::declarations::declarators::function::definition::action(fundef* fdef, std::vector<tac*>& vc)
{
  using namespace std;
  using namespace static_inline;
  if (!error::counter && cmdline::optimize_level >= 1)
    optimize::action(fdef, vc);
  usr::flag_t flag = fdef->m_usr->m_flag;
  usr::flag_t mask = usr::flag_t(usr::INLINE | usr::STATIC);
  if (flag & mask)
    return skip::add(fdef, vc, true);

  skip::chk_t arg(fdef);
  for_each(vc.begin(), vc.end(), bind2nd(ptr_fun(skip::check), &arg));
  if (arg.m_wait_inline)
    return skip::add(fdef, vc, false);

  if ( cmdline::output_medium ){
    if ( cmdline::output_optinfo )
      cout << "\nAfter optimization\n";
    usr* u = fdef->m_usr;
    scope* org = scope::current;
    scope::current = &scope::root;
    cout << dump::names::ref(u) << ":\n";
    scope::current = org;
    for (auto p : vc)
      cout << '\t', dump::tac(cout, p), cout << '\n';
    cout << '\n';
    dump::scope();
  }
  if ( !error::counter ){
    if ( generator::generate ){
      generator::interface_t tmp = {
        &scope::root,
        fdef,
        &vc
      };
      generator::generate(&tmp);
    }
    else if (generator::last) {
      remember(fdef, vc);
      fundef::current = 0;
    }
  }
  fdef->m_usr->m_type = fdef->m_usr->m_type->vla2a();
}

namespace cxx_compiler { namespace declarations { namespace declarators { namespace function { namespace definition { namespace static_inline {
    
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

    table_t::iterator it = table.find(u);
    if (it != table.end()) {
      info_t* info = it->second;
      table.erase(it);
      return gencode(info);
    }

    string name = u->m_name;
    it = find_if(table.begin(),table.end(),
         [name](const pair<usr*, info_t*>& p){ return p.first->m_name == name; });
    if (it != table.end()) {
      info_t* info = it->second;
      table.erase(it);
      return gencode(info);
    }

    const type* T = u->m_type;
    typedef const func_type FT;
    assert(T->m_id == type::FUNC);
    FT* ft = static_cast<FT*>(T);
    const vector<const type*>& param = ft->param();
    KEY key(make_pair(name, u->m_scope), &param);
    if (definition::table.find(key) != definition::table.end())
      return;

    using namespace defer;
    refs[name].push_back(ref_t(name, flag, u->m_file, ptac->m_file));
    if (ptac->m_id == tac::ADDR)
      return;
    assert(ptac->m_id == tac::CALL);
    if (!(flag & usr::INLINE))
      return;
    arg->m_wait_inline = true;
    usr* v = arg->m_fundef->m_usr;
    callers[name].insert(v);
    positions[v].push_back(arg->m_pos);
  }
} } } } } } // end of namespace static_inline, definition, function, declarators, declarations and cxx_compiler
namespace cxx_compiler { namespace declarations { namespace declarators { namespace function { namespace definition {
  namespace static_inline {
    using namespace std;
    using namespace defer;
    namespace skip {
      void after_substitute(usr* ucaller, pair<string, info_t*> pcallee)
      {
        map<usr*, vector<int> >::iterator p = positions.find(ucaller);
        assert(p != positions.end());
        vector<int>& vi = p->second;
        table_t::iterator q = table.find(ucaller);
        assert(q != table.end());
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
            string name = pcallee.first;
            info_t* callee = pcallee.second;
            if (fn->m_name == name) {
              int before = vc.size();
              if (!error::counter)
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
            table.erase(q);
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
        table[u] = info;
        vector<scope*>& ch = scope::root.m_children;
        assert(ch.size() == 1 && ch[0] == fdef->m_param);
        ch.clear();
        vc.clear();
        fundef::current = 0;
        if (!f)
          return;

        string callee = u->m_name;
        map<string, vector<ref_t> >::iterator p = refs.find(callee);
        if (p == refs.end())
          return;

        refs.erase(p);
        map<string, set<usr*> >::iterator q = callers.find(callee);
        if (q == callers.end()) {
          table.erase(u);
          return gencode(info);
        }

        const set<usr*>& su = q->second;
        usr::flag_t flag = u->m_flag;
        assert(flag & usr::INLINE);
        for_each(begin(su), end(su),
                 bind2nd(ptr_fun(after_substitute),make_pair(callee,info)));
        callers.erase(q);
      }
    } // end of namespace skip
  } // end of namespace static_inline
} } } } } // end of namespace definition, function, declarators, declarations and cxx_compiler

void cxx_compiler::declarations::declarators::function::definition::static_inline::remember(fundef* fdef, std::vector<tac*>& vc)
{
  funcs.push_back(make_pair(fdef, vc));
  scope::root.m_children.clear();
  vc.clear();
}

cxx_compiler::declarations::declarators::function::definition::table_t
cxx_compiler::declarations::declarators::function::definition::table;

cxx_compiler::declarations::declarators::function::definition::static_inline::info_t::~info_t()
{
  using namespace std;
  for (auto p : m_code) delete p;
  for (auto p : m_tmp) delete p;
  scope* param = m_fundef->m_param;
  delete param;
  delete m_fundef;
}

void
cxx_compiler::declarations::declarators::function::definition::static_inline::gencode(info_t* info)
{
  fundef* fdef = info->m_fundef;
  vector<tac*>& vc = info->m_code;
  skip::chk_t arg(fdef);
  for_each(vc.begin(), vc.end(), bind2nd(ptr_fun(skip::check), &arg));
  if (arg.m_wait_inline) {
    assert(skip::table.find(fdef->m_usr) == skip::table.end());
    skip::table[fdef->m_usr] = info;
    return;
  }
  scope* param = info->m_fundef->m_param;
  struct ss {
    scope* m_org;
    ss(scope* ptr) : m_org(0)
    {
      vector<scope*>& ch = scope::root.m_children;
      if (ch.empty())
        ch.push_back(ptr);
      else {
        assert(ch.size() == 1);
        m_org = ch[0];
        ch[0] = ptr;
      }
    }
    ~ss()
    {
      vector<scope*>& ch = scope::root.m_children;
      if (ch.size() == 1)
        ch.pop_back();
      else
        assert(generator::last && ch.empty());
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
      cout << '\t', dump::tac(cout, p), cout << '\n';
    cout << '\n';
    dump::scope();
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
