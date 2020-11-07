#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

namespace cxx_compiler { namespace statements { namespace label {
  struct define {
    to3ac* m_to;
    file_t m_file;
    scope* m_scope;
    define() : m_to(0), m_scope(0) {}
    define(to3ac* a, const file_t& b, scope* c) : m_to(a), m_file(b), m_scope(c) {}
  };
  std::map<std::string, define> defined;
  struct use {
    goto3ac* m_goto;
    file_t m_file;
    scope* m_scope;
    use(goto3ac* a, const file_t& b, scope* c)
      : m_goto(a), m_file(b), m_scope(c) {}
  };
  std::map<std::string, std::vector<use> > used;
  std::vector<usr*> vm;
  extern int decide(use,to3ac*);
} } } // end of namespace label, statements and cxx_compiler

cxx_compiler::statements::label::info_t::info_t(var* v, base* stmt) : m_stmt(stmt)
{
  genaddr* a = v->genaddr_cast();
  if (a)
    v = a->m_ref;
  usr* u = v->usr_cast();
  if (u->m_type->m_id == type::BACKPATCH)
    m_label = u;
  else {
    m_label = new usr(*u);
    m_label->m_file = parse::position;
  }
}

int cxx_compiler::statements::label::info_t::gen()
{
  using namespace std;
  string name = m_label->m_name;
  file_t org = parse::position;
  parse::position = m_label->m_file;
  {
    map<string, define>::const_iterator p = defined.find(name);
    if ( p != defined.end() ){
      using namespace error::statements::label;
      multiple(name,p->second.m_file,parse::position);
    }
  }
  to3ac* to = new to3ac;
  code.push_back(to);
  defined[name] = define(to,parse::position,scope::current);
  map<string, vector<use> >::iterator p = used.find(name);
  if ( p != used.end() ){
    const vector<use>& v = p->second;
    for_each(v.begin(),v.end(),bind2nd(ptr_fun(decide),to));
    used.erase(p);
  }
  m_stmt->gen();
  parse::position = org;
  return 0;
}

namespace cxx_compiler { namespace statements { namespace goto_stmt {
  bool outside(const scope*, const scope*);
  bool cmp(const usr*, const scope*);
} } } // end of namespace goto_stmt, statements and cxx_compiler

int cxx_compiler::statements::label::decide(use u, to3ac* to)
{
  using namespace std;
  using namespace goto_stmt;
  if ( outside(u.m_scope,scope::current) ){
    const vector<usr*>& v = label::vm;
    vector<usr*>::const_iterator p =
      find_if(v.begin(),v.end(),bind2nd(ptr_fun(cmp),scope::current));
    if ( p != v.end() ){
      using namespace error::statements::goto_stmt;
      invalid(u.m_file,parse::position,*p);
    }
  }
  goto3ac* go = u.m_goto;
  go->m_to = to;
  to->m_goto.push_back(go);
  return 0;
}

namespace cxx_compiler { namespace statements { namespace label {
  extern void blame(const std::pair<std::string, std::vector<use> >&);
  extern int blame2(std::string, use);
} } } // end of namespace label, statements and cxx_compiler

void cxx_compiler::statements::label::check()
{
  using namespace std;
  for_each(used.begin(),used.end(),blame);
  used.clear();
}

void cxx_compiler::statements::label::blame(const std::pair<std::string, std::vector<label::use> >& p)
{
  using namespace std;
  string name = p.first;
  const vector<label::use>& v = p.second;
  for_each(v.begin(),v.end(),bind1st(ptr_fun(blame2),name));
}

int cxx_compiler::statements::label::blame2(std::string name, use u)
{
  const file_t& file = u.m_file;
  using namespace error::statements::label;
  not_defined(name,file);
  return 0;
}


void cxx_compiler::statements::label::clear()
{
  defined.clear();
  used.clear();
  vm.clear();
}

namespace cxx_compiler { namespace statements { namespace switch_stmt {
  std::stack<info_t*> m_stack;
} } } // end of namespace switch_stmt, statements and cxx_compiler

int cxx_compiler::statements::_case::info_t::gen()
{
  using namespace std;
  expressions::constant_flag = true;
  var* expr = m_expr->gen();
  expr = expr->rvalue();
  expressions::constant_flag = false;
  const type* T = expr->m_type;
  if ( T->backpatch() ){
    using namespace error;
    assert(expr->usr_cast());
    usr* u = static_cast<usr*>(expr);
    string name = u->m_name;
    undeclared(u->m_file,name);
    map<string, vector<usr*> >& usrs = scope::current->m_usrs;
    usrs[name].push_back(u);
    u->m_type = int_type::create();
    return 0;
  }
  if ( !expr->isconstant() ){
    using namespace error::statements::_case;
    not_constant(m_expr->file(),expr);
    return 0;
  }
  if ( !T->integer() ){
    using namespace error::statements::_case;
    not_integer(m_expr->file(),expr);
    return 0;
  }
  if ( switch_stmt::m_stack.empty() ){
    using namespace error::statements::_case;
    no_switch(m_expr->file());
  }
  else {
    switch_stmt::info_t* top = switch_stmt::m_stack.top();
    typedef switch_stmt::info_t::case_t T;
    vector<T>& v = top->m_cases;
    vector<T>::iterator p =
      find_if(v.begin(),v.end(),bind2nd(ptr_fun(T::cmp),expr));
    if ( p != v.end() ){
      using namespace error::statements::_case;
      duplicate(m_expr->file(),p->m_expr->file());
    }
    else {
      to3ac* to = new to3ac;
      code.push_back(to);
      v.push_back(T(expr,to,m_expr,scope::current));
    }
  }
  return m_stmt->gen();
}

int cxx_compiler::statements::_default::info_t::gen()
{
  if ( switch_stmt::m_stack.empty() ){
    using namespace error::statements::_default;
    no_switch(*m_file);
  }
  else {
    switch_stmt::info_t* top = switch_stmt::m_stack.top();
    if ( !top->m_default.m_to ){
      to3ac* to = new to3ac;
      code.push_back(to);
      top->m_default = switch_stmt::info_t::default_t(to,*m_file,scope::current);
    }
    else {
      using namespace error::statements::_default;
      multiple(*m_file,top->m_default.m_file);
    }
  }
  return m_stmt->gen();
}

int cxx_compiler::statements::expression::info_t::gen()
{
  parse::position = m_expr->file();
  var* expr = m_expr->gen();
  expr->rvalue();
  const type* T = expr->m_type;
  if (!T)
    return 0;
  tag* ptr = T->get_tag();
  if (!ptr)
    return 0;
  tag::flag_t flag = ptr->m_flag;
  if (flag & tag::TYPENAMED)
    return 0;
  if (instantiate_with_template_param<template_usr>())
    return 0;
  const type* T1 = ptr->m_types.first;
  if (T1->m_id == type::TEMPLATE_PARAM)
    return 0;
  const type* T2 = ptr->m_types.second;
  if (!T2) {
    using namespace error::statements::expression;
    incomplete_type(parse::position);
  }
  return 0;
}

namespace cxx_compiler {
  namespace statements {
    namespace compound {
      void call_dtor2(var* v)
      {
        usr* u = v->usr_cast();
        if (!u)
          return call_dtor(v);
        set<usr*>::iterator p = block_impl::tried.find(u);
        if (p == block_impl::tried.end())
          return call_dtor(v);
        block_impl::tried.erase(p);
        code.push_back(new try_end3ac);
        here3ac* here = new here3ac;
        here->m_for_dest = true;
        code.push_back(here);
        const type* vp = void_type::create();
        vp = pointer_type::create(vp);
        var* t0 = new var(vp);
        assert(scope::current->m_id == scope::BLOCK);
        block* b = static_cast<block*>(scope::current);
        b->m_vars.push_back(t0);
        code.push_back(new here_info3ac(t0));
        call_dtor(v);
        code.push_back(new unwind_resume3ac(t0));
      }
    } // end of namespace compound
  } // end of namespace statements
} // end of namespace cxx_compiler

int cxx_compiler::statements::compound::info_t::gen()
{
  using namespace std;
  scope* org = scope::current;
  scope::current = m_scope;
  if (m_bases) {
    const vector<base*>& v = *m_bases;
    for_each(v.begin(),v.end(),mem_fun(&base::gen));
  }
  assert(scope::current->m_id == scope::BLOCK);
  block* b = static_cast<block*>(scope::current);
  typedef map<block*, vector<var*> >::iterator IT;
  IT p = block_impl::dtor_tbl.find(b);
  if (p != block_impl::dtor_tbl.end()) {
    const vector<var*>& v = p->second;
    for_each(rbegin(v), rend(v), call_dtor);
    goto3ac* go = new goto3ac;
    to3ac* to = new to3ac;
    go->m_to = to;
    to->m_goto.push_back(go);
    code.push_back(go);
    for_each(rbegin(v), rend(v), call_dtor2);
    if (b->m_parent->m_id == scope::PARAM)
      assert(block_impl::tried.empty());
    code.push_back(to);
    block_impl::dtor_tbl.erase(p);
  }
  scope* parent = scope::current->m_parent;
  if (parent->m_id == scope::PARAM) {
    const vector<usr*>& order = parent->m_order;
    for_each(rbegin(order), rend(order), call_dtor);
  }
  scope::current = org;
  return 0;
}

void cxx_compiler::call_dtor(var* v)
{
  using namespace std;
  const type* T = v->result_type();
  if (!T) {
    assert(v->usr_cast());
    usr* u = static_cast<usr*>(v);
    assert(u->m_flag & usr::OVERLOAD);
    return;
  }
  if (T->m_id == type::ARRAY) {
    typedef const array_type AT;
    AT* at = static_cast<AT*>(T);
    if (array_of_tor(at, false))
      ctor_dtor_common(v, at, call_dtor, false);
    return;
  }
  T = T->unqualified();
  if ( T->m_id != type::RECORD )
    return;
  typedef const record_type REC;
  REC* rec = static_cast<REC*>(T);
  tag* ptr = rec->get_tag();
  string name = '~' + tor_name(ptr); 
  const map<string, vector<usr*> >& usrs = ptr->m_usrs;
  map<string, vector<usr*> >::const_iterator p = usrs.find(name);
  if ( p == usrs.end() )
    return;
  const vector<usr*>& vu = p->second;
  usr* dtor = vu.back();
  dtor = instantiate_if(dtor);
  call_impl::wrapper(dtor, 0, v);
}

cxx_compiler::statements::compound::info_t::~info_t()
{
  using namespace std;
  if ( m_bases ){
    for (auto p : *m_bases)
      delete p;
    delete m_bases;
  }
}

int cxx_compiler::statements::if_stmt::info_t::gen()
{
  file_t org = parse::position;
  parse::position = m_expr->file();
  var* expr = m_expr->gen();
  expr->if_code(this);
  parse::position = org;
  return 0;
}

void cxx_compiler::var::if_code(statements::if_stmt::info_t* info)
{
  var* expr = rvalue();
  const type* T = expr->m_type;
  if (!T->scalar()) {
    if (usr* conv = conversion_function(T)) {
      expr = call_impl::wrapper(conv, 0, expr);
      T = expr->m_type;
      assert(T->scalar());
    }
    else {
      using namespace error::statements::if_stmt;
      not_scalar(parse::position,expr);
      T = int_type::create();
    }
  }
  T = T->promotion();
  expr = expr->cast(T);
  var* zero = expressions::primary::literal::integer::create(0);
  zero = zero->cast(T);
  goto3ac* goto1 = new goto3ac(goto3ac::EQ,expr,zero);
  code.push_back(goto1);
  info->m_stmt1->gen();
  if ( info->m_stmt2 ){
    goto3ac* goto2 = new goto3ac;
    code.push_back(goto2);
    to3ac* to1 = new to3ac;
    code.push_back(to1);
    to1->m_goto.push_back(goto1);
    goto1->m_to = to1;
    info->m_stmt2->gen();
    to3ac* to2 = new to3ac;
    code.push_back(to2);
    to2->m_goto.push_back(goto2);
    goto2->m_to = to2;
  }
  else {
    to3ac* to1 = new to3ac;
    code.push_back(to1);
    to1->m_goto.push_back(goto1);
    goto1->m_to = to1;
  }
}

namespace cxx_compiler { namespace statements { namespace if_stmt {
  template<class T> void common(constant<T>* ptr, info_t* info)
  {
    using namespace std;
    if ( ptr->zero() ){
      int n = code.size();
      info->m_stmt1->gen();
      for_each(code.begin()+n,code.end(),[](tac* p){ delete p; });
      code.resize(n);
      if ( info->m_stmt2 )
        info->m_stmt2->gen();
    }
    else {
      info->m_stmt1->gen();
      if ( info->m_stmt2 ){
        int n = code.size();
        info->m_stmt2->gen();
        for_each(code.begin()+n,code.end(),[](tac* p){ delete p; });
        code.resize(n);
      }
    }
  }
} } } // end of namespace if_stmt, statements and cxx_compiler

namespace cxx_compiler {
  template<>
  void constant<bool>::if_code(statements::if_stmt::info_t* info){ statements::if_stmt::common(this,info); }
  template<>
  void constant<char>::if_code(statements::if_stmt::info_t* info){ statements::if_stmt::common(this,info); }
  template<>
  void constant<signed char>::if_code(statements::if_stmt::info_t* info){ statements::if_stmt::common(this,info); }
  template<>
  void constant<unsigned char>::if_code(statements::if_stmt::info_t* info){ statements::if_stmt::common(this,info); }
  template<>
  void constant<wchar_t>::if_code(statements::if_stmt::info_t* info){ statements::if_stmt::common(this,info); }
  template<>
  void constant<short int>::if_code(statements::if_stmt::info_t* info){ statements::if_stmt::common(this,info); }
  template<>
  void constant<unsigned short int>::if_code(statements::if_stmt::info_t* info){ statements::if_stmt::common(this,info); }
  void constant<int>::if_code(statements::if_stmt::info_t* info){ statements::if_stmt::common(this,info); }
  void constant<unsigned int>::if_code(statements::if_stmt::info_t* info){ statements::if_stmt::common(this,info); }
  void constant<long int>::if_code(statements::if_stmt::info_t* info){ statements::if_stmt::common(this,info); }
  void constant<unsigned long int>::if_code(statements::if_stmt::info_t* info){ statements::if_stmt::common(this,info); }
  void constant<__int64>::if_code(statements::if_stmt::info_t* info){ statements::if_stmt::common(this,info); }
  void constant<unsigned __int64>::if_code(statements::if_stmt::info_t* info){ statements::if_stmt::common(this,info); }
} // end of namespace cxx_compiler

void cxx_compiler::constant<float>::if_code(statements::if_stmt::info_t* info){ statements::if_stmt::common(this,info); }
void cxx_compiler::constant<double>::if_code(statements::if_stmt::info_t* info){ statements::if_stmt::common(this,info); }
void cxx_compiler::constant<long double>::if_code(statements::if_stmt::info_t* info){ statements::if_stmt::common(this,info); }
void cxx_compiler::constant<void*>::if_code(statements::if_stmt::info_t* info){ statements::if_stmt::common(this,info); }

void cxx_compiler::var01::sweep(std::vector<tac*>::iterator it)
{
  it -= 2;
  delete *it;
  it = code.erase(it);
  ++it;
  delete *it;
  code.erase(it);
}

void cxx_compiler::var01::sweep()
{
  using namespace std;
  block* b = static_cast<block*>(m_scope);
  vector<var*>& v = b->m_vars;
  vector<var*>::reverse_iterator q = find(v.rbegin(),v.rend(),this);
  assert(q != v.rend());
  v.erase(q.base()-1);
  garbage.push_back(this);
}

void cxx_compiler::var01::if_code(statements::if_stmt::info_t* info)
{
  using namespace std;
  sweep();
  vector<tac*> stmt1;
  {
    int n = code.size();
    info->m_stmt1->gen();
    int m = code.size();
    copy(code.begin()+n,code.begin()+m,back_inserter(stmt1));
    code.resize(n);
  }
  vector<tac*> stmt2;
  if ( info->m_stmt2 ){
    int n = code.size();
    info->m_stmt2->gen();
    int m = code.size();
    copy(code.begin()+n,code.begin()+m,back_inserter(stmt2));
    code.resize(n);
  }
  if ( m_one < m_zero ){
    vector<tac*>::iterator p = code.begin() + m_one;
    delete *p;
    p = code.erase(p);
    code.insert(p,stmt1.begin(),stmt1.end());
    vector<tac*>::iterator q = code.begin() + m_zero + stmt1.size() - 1;
    delete *q;
    q = code.erase(q);
    if ( stmt2.empty() )
      sweep(q);
    else
      code.insert(q,stmt2.begin(),stmt2.end());
  }
  else {
    vector<tac*>::iterator p = code.begin() + m_zero;
    delete *p;
    p = code.erase(p);
    code.insert(p,stmt2.begin(),stmt2.end());
    vector<tac*>::iterator q = code.begin() + m_one + stmt2.size() - 1;
    delete *q;
    q = code.erase(q);
    code.insert(q,stmt1.begin(),stmt1.end());
  }
}

namespace cxx_compiler { namespace statements { namespace break_stmt {
  std::stack<outer*> m_stack;
} } } // end of namespace break_stmt, statements and cxx_compiler

namespace cxx_compiler { namespace statements { namespace switch_stmt {
  int gencode(var*, info_t::case_t);
} } } // end of namespace switch_stmt, statements and cxx_compiler

int cxx_compiler::statements::switch_stmt::info_t::gen()
{
  using namespace std;
  var* expr = m_expr->gen();
  expr = expr->rvalue();
  const type* T = expr->m_type;
  if ( !T->integer() ){
    using namespace error::statements::switch_stmt;
    not_integer(m_expr->file(),expr);
    expr->m_type = int_type::create();
  }
  goto3ac* start = new goto3ac;
  code.push_back(start);
  m_stack.push(this);
  break_stmt::m_stack.push(this);
  m_stmt->gen();
  break_stmt::m_stack.pop();
  m_stack.pop();
  goto3ac* last = new goto3ac;
  code.push_back(last);
  to3ac* to1 = new to3ac;
  code.push_back(to1);
  to1->m_goto.push_back(start);
  start->m_to = to1;
  for_each(m_cases.begin(),m_cases.end(),bind1st(ptr_fun(gencode),expr));
  if ( to3ac* to = m_default.m_to ){
    using namespace goto_stmt;
    if ( outside(scope::current,m_default.m_scope) ){
      const vector<usr*>& v = label::vm;
      vector<usr*>::const_iterator p =
        find_if(v.begin(),v.end(),bind2nd(ptr_fun(cmp),m_default.m_scope));
      if ( p != v.end() ){
        using namespace error::statements::switch_stmt;
        invalid(false,m_default.m_file,*p);
      }
    }
    goto3ac* go = new goto3ac;
    code.push_back(go);
    to->m_goto.push_back(go);
    go->m_to = to;
  }
  to3ac* to2 = new to3ac;
  code.push_back(to2);
  for_each(begin(),end(),bind2nd(ptr_fun(misc::update),to2));
  copy(begin(),end(),back_inserter(to2->m_goto));
  to2->m_goto.push_back(last);
  last->m_to = to2;
  return 0;
}

int cxx_compiler::statements::switch_stmt::gencode(var* x, info_t::case_t _case)
{
  using namespace std;
  using namespace goto_stmt;
  if ( outside(scope::current,_case.m_scope) ){
    const vector<usr*>& v = label::vm;
    vector<usr*>::const_iterator p =
      find_if(v.begin(),v.end(),bind2nd(ptr_fun(cmp),_case.m_scope));
    if ( p != v.end() ){
      using namespace error::statements::switch_stmt;
      invalid(true,_case.m_expr->file(),*p);
    }
  }
  var* y = _case.m_label;
  conversion::arithmetic::gen(&x,&y);
  to3ac* to = _case.m_to;
  goto3ac* go = new goto3ac(goto3ac::EQ,x,y);
  code.push_back(go);
  to->m_goto.push_back(go);
  go->m_to = to;
  return 0;
}

namespace cxx_compiler { namespace statements { namespace continue_stmt {
  std::stack<outer*> m_stack;
} } } // end of namespace continue_stmt, statements and cxx_compiler

int cxx_compiler::statements::while_stmt::info_t::gen()
{
  to3ac* begin = new to3ac;
  code.push_back(begin);
  var* expr = m_expr->gen();
  expr->while_code(this,begin);
  return 0;
}

void cxx_compiler::var::while_code(statements::while_stmt::info_t* info, to3ac* begin)
{  
  using namespace std;
  using namespace statements;
  var* expr = rvalue();
  expr = expr->promotion();
  const type* T = expr->m_type;
  if ( !T->scalar() ){
    using namespace error::statements::while_stmt;
    not_scalar(info->m_expr->file());
    T = expr->m_type = int_type::create();
  }
  var* zero = expressions::primary::literal::integer::create(0);
  zero = zero->cast(T);
  goto3ac* goto2 = new goto3ac(goto3ac::EQ,expr,zero);
  code.push_back(goto2);
  break_stmt::m_stack.push(info);
  continue_stmt::m_stack.push(info);
  info->m_stmt->gen();
  continue_stmt::m_stack.pop();
  break_stmt::m_stack.pop();
  continue_stmt::outer& c = *info;
  for_each(c.begin(),c.end(),bind2nd(ptr_fun(misc::update),begin));
  copy(c.begin(),c.end(),back_inserter(begin->m_goto));
  goto3ac* goto1 = new goto3ac;
  code.push_back(goto1);
  begin->m_goto.push_back(goto1);
  goto1->m_to = begin;
  to3ac* end = new to3ac;
  code.push_back(end);
  end->m_goto.push_back(goto2);
  goto2->m_to = end;
  break_stmt::outer& b = *info;
  for_each(b.begin(),b.end(),bind2nd(ptr_fun(misc::update),end));
  copy(b.begin(),b.end(),back_inserter(end->m_goto));
}

namespace cxx_compiler { namespace statements { namespace while_stmt {
  template<class T> void common(constant<T>* ptr, info_t* info, to3ac* begin)
  {
    using namespace std;
    if ( !ptr->zero() ){
      break_stmt::m_stack.push(info);
      continue_stmt::m_stack.push(info);
      info->m_stmt->gen();
      continue_stmt::m_stack.pop();
      break_stmt::m_stack.pop();
      continue_stmt::outer& c = *info;
      for_each(c.begin(),c.end(),bind2nd(ptr_fun(misc::update),begin));
      copy(c.begin(),c.end(),back_inserter(begin->m_goto));
      goto3ac* go = new goto3ac;
      code.push_back(go);
      go->m_to = begin;
      begin->m_goto.push_back(go);
      to3ac* end = new to3ac;
      code.push_back(end);
      break_stmt::outer& b = *info;
      for_each(b.begin(),b.end(),bind2nd(ptr_fun(misc::update),end));
      copy(b.begin(),b.end(),back_inserter(end->m_goto));
    }
    else {
      int n = code.size();
      info->m_stmt->gen();
      for_each(code.begin()+n,code.end(),[](tac* p){ delete p; });
      code.resize(n);
    }
  }
} } } // end of namespace while_stmt, statements and cxx_compiler

namespace cxx_compiler {
  template<>
  void constant<bool>::while_code(statements::while_stmt::info_t* info, to3ac* begin){ statements::while_stmt::common(this,info,begin); }
  template<>
  void constant<char>::while_code(statements::while_stmt::info_t* info, to3ac* begin){ statements::while_stmt::common(this,info,begin); }
  template<>
  void constant<signed char>::while_code(statements::while_stmt::info_t* info, to3ac* begin){ statements::while_stmt::common(this,info,begin); }
  template<>
  void constant<unsigned char>::while_code(statements::while_stmt::info_t* info, to3ac* begin){ statements::while_stmt::common(this,info,begin); }
  template<>
  void constant<wchar_t>::while_code(statements::while_stmt::info_t* info, to3ac* begin){ statements::while_stmt::common(this,info,begin); }
  template<>
  void constant<short int>::while_code(statements::while_stmt::info_t* info, to3ac* begin){ statements::while_stmt::common(this,info,begin); }
  template<>
  void constant<unsigned short int>::while_code(statements::while_stmt::info_t* info, to3ac* begin){ statements::while_stmt::common(this,info,begin); }
  void constant<int>::while_code(statements::while_stmt::info_t* info, to3ac* begin){ statements::while_stmt::common(this,info,begin); }
  void constant<unsigned int>::while_code(statements::while_stmt::info_t* info, to3ac* begin){ statements::while_stmt::common(this,info,begin); }
  void constant<long int>::while_code(statements::while_stmt::info_t* info, to3ac* begin){ statements::while_stmt::common(this,info,begin); }
  void constant<unsigned long int>::while_code(statements::while_stmt::info_t* info, to3ac* begin){ statements::while_stmt::common(this,info,begin); }
  void constant<__int64>::while_code(statements::while_stmt::info_t* info, to3ac* begin){ statements::while_stmt::common(this,info,begin); }
  void constant<unsigned __int64>::while_code(statements::while_stmt::info_t* info, to3ac* begin){ statements::while_stmt::common(this,info,begin); }
} // end of namespace cxx_compiler

void cxx_compiler::constant<float>::while_code(statements::while_stmt::info_t* info, to3ac* begin){ statements::while_stmt::common(this,info,begin); }
void cxx_compiler::constant<double>::while_code(statements::while_stmt::info_t* info, to3ac* begin){ statements::while_stmt::common(this,info,begin); }
void cxx_compiler::constant<long double>::while_code(statements::while_stmt::info_t* info, to3ac* begin){ statements::while_stmt::common(this,info,begin); }
void cxx_compiler::constant<void*>::while_code(statements::while_stmt::info_t* info, to3ac* begin){ statements::while_stmt::common(this,info,begin); }

void cxx_compiler::var01::while_code(statements::while_stmt::info_t* info, to3ac* begin)
{
  using namespace std;
  using namespace statements;
  sweep();
  break_stmt::m_stack.push(info);
  continue_stmt::m_stack.push(info);
  int n = code.size();
  info->m_stmt->gen();
  int m = code.size();
  vector<tac*> stmt;
  copy(code.begin()+n,code.begin()+m,back_inserter(stmt));
  code.resize(n);
  if ( m_one < m_zero ){
    goto3ac* goto1 = new goto3ac;
    stmt.push_back(goto1);
    goto1->m_to = begin;
    begin->m_goto.push_back(goto1);
    vector<tac*>::iterator p = code.begin() + m_one;
    delete *p;
    p = code.erase(p);
    code.insert(p,stmt.begin(),stmt.end());
    vector<tac*>::iterator q = code.begin() + stmt.size() + m_zero - 1;
    delete *q;
    q = code.erase(q);
    sweep(q);
  }
  else {
    vector<tac*>::iterator p = code.begin() + m_zero;
    delete *p;
    p = code.erase(p);
    goto3ac* goto1 = new goto3ac;
    stmt.push_back(goto1);
    goto1->m_to = begin;
    begin->m_goto.push_back(goto1);
    vector<tac*>::iterator q = code.begin() + m_one - 1;
    delete *q;
    q = code.erase(q);
    code.insert(q,stmt.begin(),stmt.end());
  }
  continue_stmt::m_stack.pop();
  break_stmt::m_stack.pop();
  continue_stmt::outer& c = *info;
  for_each(c.begin(),c.end(),bind2nd(ptr_fun(misc::update),begin));
  copy(c.begin(),c.end(),back_inserter(begin->m_goto));
  to3ac* end = static_cast<to3ac*>(code.back());
  break_stmt::outer& b = *info;
  for_each(b.begin(),b.end(),bind2nd(ptr_fun(misc::update),end));
  copy(b.begin(),b.end(),back_inserter(end->m_goto));
}

int cxx_compiler::statements::do_stmt::info_t::gen()
{
  using namespace std;
  to3ac* begin = new to3ac;
  code.push_back(begin);
  break_stmt::m_stack.push(this);
  continue_stmt::m_stack.push(this);
  m_stmt->gen();
  continue_stmt::m_stack.pop();
  break_stmt::m_stack.pop();
  to3ac* to2 = new to3ac;
  code.push_back(to2);
  continue_stmt::outer& c = *this;
  for_each(c.begin(),c.end(),bind2nd(ptr_fun(misc::update),to2));
  copy(c.begin(),c.end(),back_inserter(to2->m_goto));
  var* expr = m_expr->gen();
  expr->do_code(this,begin);
  return 0;
}

void cxx_compiler::var::do_code(statements::do_stmt::info_t* info, to3ac* begin)
{
  using namespace std;
  using namespace statements;
  var* expr = rvalue();
  expr = expr->promotion();
  const type* T = expr->m_type;
  if ( !T->scalar() ){
    using namespace error::statements::do_stmt;
    not_scalar(info->m_expr->file());
    T = expr->m_type = int_type::create();
  }
  var* zero = expressions::primary::literal::integer::create(0);
  zero = zero->cast(T);
  goto3ac* go = new goto3ac(goto3ac::NE,expr,zero);
  code.push_back(go);
  begin->m_goto.push_back(go);
  go->m_to = begin;
  to3ac* end = new to3ac;
  code.push_back(end);
  break_stmt::outer& b = *info;
  for_each(b.begin(),b.end(),bind2nd(ptr_fun(misc::update),end));
  copy(b.begin(),b.end(),back_inserter(end->m_goto));
}

namespace cxx_compiler { namespace statements { namespace do_stmt {
  template<class T> void common(constant<T>* ptr, info_t* info, to3ac* begin)
  {
    using namespace std;
    if ( !ptr->zero() ){
      goto3ac* go = new goto3ac;
      go->m_to = begin;
      begin->m_goto.push_back(go);
      code.push_back(go);
    }
    to3ac* end = new to3ac;
    code.push_back(end);
    break_stmt::outer& b = *info;
    for_each(b.begin(),b.end(),bind2nd(ptr_fun(misc::update),end));
    copy(b.begin(),b.end(),back_inserter(end->m_goto));
  }
} } } // end of namespace do_stmt, statements and cxx_compiler

namespace cxx_compiler {
  template<>
  void constant<bool>::do_code(statements::do_stmt::info_t* info, to3ac* begin){ statements::do_stmt::common(this,info,begin); }
  template<>
  void constant<char>::do_code(statements::do_stmt::info_t* info, to3ac* begin){ statements::do_stmt::common(this,info,begin); }
  template<>
  void constant<signed char>::do_code(statements::do_stmt::info_t* info, to3ac* begin){ statements::do_stmt::common(this,info,begin); }
  template<>
  void constant<unsigned char>::do_code(statements::do_stmt::info_t* info, to3ac* begin){ statements::do_stmt::common(this,info,begin); }
  template<>
  void constant<wchar_t>::do_code(statements::do_stmt::info_t* info, to3ac* begin){ statements::do_stmt::common(this,info,begin); }
  template<>
  void constant<short int>::do_code(statements::do_stmt::info_t* info, to3ac* begin){ statements::do_stmt::common(this,info,begin); }
  template<>
  void constant<unsigned short int>::do_code(statements::do_stmt::info_t* info, to3ac* begin){ statements::do_stmt::common(this,info,begin); }
  void constant<int>::do_code(statements::do_stmt::info_t* info, to3ac* begin){ statements::do_stmt::common(this,info,begin); }
  void constant<unsigned int>::do_code(statements::do_stmt::info_t* info, to3ac* begin){ statements::do_stmt::common(this,info,begin); }
  void constant<long int>::do_code(statements::do_stmt::info_t* info, to3ac* begin){ statements::do_stmt::common(this,info,begin); }
  void constant<unsigned long int>::do_code(statements::do_stmt::info_t* info, to3ac* begin){ statements::do_stmt::common(this,info,begin); }
  void constant<__int64>::do_code(statements::do_stmt::info_t* info, to3ac* begin){ statements::do_stmt::common(this,info,begin); }
  void constant<unsigned __int64>::do_code(statements::do_stmt::info_t* info, to3ac* begin){ statements::do_stmt::common(this,info,begin); }
} // end of namespace cxx_compiler

void cxx_compiler::constant<float>::do_code(statements::do_stmt::info_t* info, to3ac* begin){ statements::do_stmt::common(this,info,begin); }
void cxx_compiler::constant<double>::do_code(statements::do_stmt::info_t* info, to3ac* begin){ statements::do_stmt::common(this,info,begin); }
void cxx_compiler::constant<long double>::do_code(statements::do_stmt::info_t* info, to3ac* begin){ statements::do_stmt::common(this,info,begin); }
void cxx_compiler::constant<void*>::do_code(statements::do_stmt::info_t* info, to3ac* begin){ statements::do_stmt::common(this,info,begin); }

void cxx_compiler::var01::do_code(statements::do_stmt::info_t* info, to3ac* begin)
{
  using namespace std;
  using namespace statements;
  sweep();
  vector<tac*>::iterator p = code.begin() + m_one - 1;
  goto3ac* go = static_cast<goto3ac*>(*p);
  go->m_op = opposite[go->m_op];
  go->m_to = begin;
  begin->m_goto.push_back(go);
  delete *++p;  // destroy res := 0
  p = code.erase(p);
  // `p' points to `goto end'
  delete *p;
  p = code.erase(p);
  // `p' points to `label'
  delete *p;
  p = code.erase(p);
  // `p' points to `res := 1'
  delete *p;
  p = code.erase(p);
  // `p' points to `end:'
  to3ac* end = static_cast<to3ac*>(*p);
  end->m_goto.clear();
  break_stmt::outer& b = *info;
  for_each(b.begin(),b.end(),bind2nd(ptr_fun(misc::update),end));
  copy(b.begin(),b.end(),back_inserter(end->m_goto));
}

void cxx_compiler::log01::do_code(statements::do_stmt::info_t* info, to3ac* begin)
{
  using namespace std;
  using namespace statements;
  sweep();
  to3ac* end;
  if ( m_one < m_zero ){
    vector<tac*>::iterator p = code.begin() + m_one;
    delete *p;
    p = code.erase(p);
    goto3ac* go = static_cast<goto3ac*>(*p);
    go->m_to = begin;
    begin->m_goto.push_back(go);
    end = static_cast<to3ac*>(*++p);
    vector<tac*>::iterator q = code.begin() + m_zero - 1;
    delete *q;
    q = code.erase(q);
    delete *q;
    code.erase(q);
  }
  else {
    vector<tac*>::iterator p = code.begin() + m_goto1;
    goto3ac* goto1 = static_cast<goto3ac*>(*p);
    goto1->m_to = begin;
    begin->m_goto.push_back(goto1);
    p = code.begin() + m_zero - 1;
    goto3ac* goto2 = static_cast<goto3ac*>(*p);
    goto2->m_to = begin;
    begin->m_goto.push_back(goto2);
    delete *++p;
    p = code.erase(p);
    vector<tac*>::iterator q = code.begin() + m_one - 1;
    delete *q;
    q = code.erase(q);
    sweep(q);
    // `static_cast<to3ac*>(*p)' is runtime error in Visual Studio 2017.
    end = static_cast<to3ac*>(code[m_zero]);
    end->m_goto.clear();
  }
  break_stmt::outer& b = *info;
  for_each(b.begin(),b.end(),bind2nd(ptr_fun(misc::update),end));
  copy(b.begin(),b.end(),back_inserter(end->m_goto));
}

int cxx_compiler::statements::for_stmt::info_t::gen()
{
  using namespace std;
  scope* org = scope::current;
  scope::current = m_scope;
  if (m_stmt1)
    m_stmt1->gen();
  to3ac* begin = new to3ac;
  code.push_back(begin);
  var unused(0);
  var* expr = m_expr2 ? m_expr2->gen() : &unused;
  expr->for_code(this, begin);

  assert(m_child->m_id == scope::BLOCK);
  block* b = static_cast<block*>(m_child);
  typedef map<block*, vector<var*> >::iterator IT;
  IT p = block_impl::dtor_tbl.find(b);
  if (p != block_impl::dtor_tbl.end()) {
    const vector<var*>& v = p->second;
    for_each(v.rbegin(), v.rend(), call_dtor);
    block_impl::dtor_tbl.erase(p);
  }
  scope::current = org;
  return 0;
}

void cxx_compiler::var::for_code(statements::for_stmt::info_t* info,
                		 to3ac* begin)
{
  using namespace std;
  using namespace statements;
  goto3ac* goto2 = 0;
  if ( m_type ){
    var* expr2 = rvalue();
    expr2 = expr2->promotion();
    const type* T = expr2->m_type;
    if ( !T->scalar() ){
      using namespace error::statements::for_stmt;
      not_scalar(info->m_expr2->file());
      T = expr2->m_type = int_type::create();
    }
    var* zero = expressions::primary::literal::integer::create(0);
    zero = zero->cast(T);
    goto2 = new goto3ac(goto3ac::EQ,expr2,zero);
    code.push_back(goto2);
  }
  break_stmt::m_stack.push(info);
  continue_stmt::m_stack.push(info);
  info->m_stmt->gen();
  continue_stmt::m_stack.pop();
  break_stmt::m_stack.pop();
  to3ac* to3 = new to3ac;
  code.push_back(to3);
  continue_stmt::outer& c = *info;
  for_each(c.begin(),c.end(),bind2nd(ptr_fun(misc::update),to3));
  copy(c.begin(),c.end(),back_inserter(to3->m_goto));
  if ( info->m_expr3 ){
    var* expr3 = info->m_expr3->gen();
    expr3->rvalue();
  }
  goto3ac* goto1 = new goto3ac;
  code.push_back(goto1);
  begin->m_goto.push_back(goto1);
  goto1->m_to = begin;
  to3ac* to2 = new to3ac;
  code.push_back(to2);
  break_stmt::outer& b = *info;
  for_each(b.begin(),b.end(),bind2nd(ptr_fun(misc::update),to2));
  copy(b.begin(),b.end(),back_inserter(to2->m_goto));
  if ( goto2 ){
    to2->m_goto.push_back(goto2);
    goto2->m_to = to2;
  }
}

namespace cxx_compiler { namespace statements { namespace for_stmt {
  template<class T> void common(constant<T>* ptr, info_t* info, to3ac* begin)
  {
    using namespace std;
    if ( !ptr->zero() ){
      var unused(0);
      unused.for_code(info,begin);
    }
    else {
      int n = code.size();
      info->m_stmt->gen();
      for_each(code.begin()+n,code.end(),[](tac* p){ delete p; });
      code.resize(n);
    }
  }
} } } // end of namespace for_stmt, statements and cxx_compiler

namespace cxx_compiler {
  template<>
  void constant<bool>::for_code(statements::for_stmt::info_t* info, to3ac* begin){ statements::for_stmt::common(this,info,begin); }
  template<>
  void constant<char>::for_code(statements::for_stmt::info_t* info, to3ac* begin){ statements::for_stmt::common(this,info,begin); }
  template<>
  void constant<signed char>::for_code(statements::for_stmt::info_t* info, to3ac* begin){ statements::for_stmt::common(this,info,begin); }
  template<>
  void constant<unsigned char>::for_code(statements::for_stmt::info_t* info, to3ac* begin){ statements::for_stmt::common(this,info,begin); }
  template<>
  void constant<wchar_t>::for_code(statements::for_stmt::info_t* info, to3ac* begin){ statements::for_stmt::common(this,info,begin); }
  template<>
  void constant<short int>::for_code(statements::for_stmt::info_t* info, to3ac* begin){ statements::for_stmt::common(this,info,begin); }
  template<>
  void constant<unsigned short int>::for_code(statements::for_stmt::info_t* info, to3ac* begin){ statements::for_stmt::common(this,info,begin); }
  void constant<int>::for_code(statements::for_stmt::info_t* info, to3ac* begin){ statements::for_stmt::common(this,info,begin); }
  void constant<unsigned int>::for_code(statements::for_stmt::info_t* info, to3ac* begin){ statements::for_stmt::common(this,info,begin); }
  void constant<long int>::for_code(statements::for_stmt::info_t* info, to3ac* begin){ statements::for_stmt::common(this,info,begin); }
  void constant<unsigned long int>::for_code(statements::for_stmt::info_t* info, to3ac* begin){ statements::for_stmt::common(this,info,begin); }
  void constant<__int64>::for_code(statements::for_stmt::info_t* info, to3ac* begin){ statements::for_stmt::common(this,info,begin); }
  void constant<unsigned __int64>::for_code(statements::for_stmt::info_t* info, to3ac* begin){ statements::for_stmt::common(this,info,begin); }
} // end of namespace cxx_compiler

void cxx_compiler::constant<float>::for_code(statements::for_stmt::info_t* info, to3ac* begin){ statements::for_stmt::common(this,info,begin); }
void cxx_compiler::constant<double>::for_code(statements::for_stmt::info_t* info, to3ac* begin){ statements::for_stmt::common(this,info,begin); }
void cxx_compiler::constant<long double>::for_code(statements::for_stmt::info_t* info, to3ac* begin){ statements::for_stmt::common(this,info,begin); }
void cxx_compiler::constant<void*>::for_code(statements::for_stmt::info_t* info, to3ac* begin){ statements::for_stmt::common(this,info,begin); }

void cxx_compiler::var01::for_code(statements::for_stmt::info_t* info, to3ac* begin)
{
  using namespace std;
  using namespace statements;
  sweep();
  break_stmt::m_stack.push(info);
  continue_stmt::m_stack.push(info);
  int n = code.size();
  info->m_stmt->gen();
  int m = code.size();
  continue_stmt::m_stack.pop();
  break_stmt::m_stack.pop();
  vector<tac*> stmt;
  copy(code.begin()+n,code.begin()+m,back_inserter(stmt));
  code.resize(n);
  to3ac* to_continue = new to3ac;
  stmt.push_back(to_continue);
  if ( info->m_expr3 ){
    int n = code.size();
    var* expr3 = info->m_expr3->gen();
    expr3->rvalue();
    int m = code.size();
    copy(code.begin()+n,code.begin()+m,back_inserter(stmt));
    code.resize(n);
  }
  goto3ac* goto1 = new goto3ac;
  stmt.push_back(goto1);
  goto1->m_to = begin;
  begin->m_goto.push_back(goto1);
  if ( m_one < m_zero ){
    vector<tac*>::iterator p = code.begin() + m_one;
    delete *p;
    p = code.erase(p);
    code.insert(p,stmt.begin(),stmt.end());
    vector<tac*>::iterator q = code.begin() + stmt.size() + m_zero - 1;
    delete *q;
    q = code.erase(q);
    sweep(q);
  }
  else {
    vector<tac*>::iterator p = code.begin() + m_zero;
    delete *p;
    p = code.erase(p);
    vector<tac*>::iterator q = code.begin() + m_one - 1;
    delete *q;
    q = code.erase(q);
    code.insert(q,stmt.begin(),stmt.end());
  }
  continue_stmt::outer& c = *info;
  for_each(c.begin(),c.end(),bind2nd(ptr_fun(misc::update),to_continue));
  copy(c.begin(),c.end(),back_inserter(to_continue->m_goto));
  to3ac* end = static_cast<to3ac*>(code.back());
  break_stmt::outer& b = *info;
  for_each(b.begin(),b.end(),bind2nd(ptr_fun(misc::update),end));
  copy(b.begin(),b.end(),back_inserter(end->m_goto));
}

int cxx_compiler::statements::break_stmt::info_t::gen()
{
  if ( m_stack.empty() ){
    using namespace error::statements::break_stmt;
    not_within(m_file);
    return 0;
  }
  outer* top = m_stack.top();
  goto3ac* go = new goto3ac;
  top->push_back(go);
  code.push_back(go);
  return 0;
}

int cxx_compiler::statements::continue_stmt::info_t::gen()
{
  if ( m_stack.empty() ){
    using namespace error::statements::continue_stmt;
    not_within(m_file);
    return 0;
  }
  outer* top = m_stack.top();
  goto3ac* go = new goto3ac;
  top->push_back(go);
  code.push_back(go);
  return 0;
}

namespace cxx_compiler {
  namespace statements {
    namespace return_stmt {
      extern void gather(scope*, std::vector<var*>&);
      inline var* copy_ctor(const type* T, var* expr)
      {
        usr* ctor = get_copy_ctor(T);
        if (!ctor)
          return expr;
        usr::flag2_t flag2 = ctor->m_flag2;
        if (flag2 & usr::GENED_BY_COMP)
          return expr;
        const type* Tc = ctor->m_type;
        assert(Tc->m_id == type::FUNC);
        typedef const func_type FT;
        FT* ft = static_cast<FT*>(Tc);
        const vector<const type*>& param = ft->param();
        assert(!param.empty());
        const type* Tp = param[0];
        var* t0 = new var(Tp);
        var* t1 = new var(T);
        if (scope::current->m_id == scope::BLOCK) {
          block* b = static_cast<block*>(scope::current);
          b->m_vars.push_back(t0);
          b->m_vars.push_back(t1);
        }
        else {
          garbage.push_back(t0);
          garbage.push_back(t1);
        }
        code.push_back(new addr3ac(t0, expr));
        vector<var*> arg;
        arg.push_back(t0);
        call_impl::wrapper(ctor, &arg, t1);
        return t1;
      }
    } // end of namespace return_stmt
  } // end of namespace statements
} // end of namespace cxx_compiler

cxx_compiler::statements::return_stmt::info_t::info_t(expressions::base* b)
  : m_expr(b), m_file(parse::position)
{
  gather(scope::current, m_vars);
}

void cxx_compiler::statements::
return_stmt::gather(scope* ptr, std::vector<var*>& res)
{
  using namespace std;
  scope::id_t id = ptr->m_id;
  if (id == scope::BLOCK) {
    block* b = static_cast<block*>(ptr);
    typedef map<block*, vector<var*> >::iterator IT;
    IT p = block_impl::dtor_tbl.find(b);
    if (p != block_impl::dtor_tbl.end()) {
      const vector<var*>& v = p->second;
      copy(rbegin(v), rend(v), back_inserter(res));
    }
  }
  else if (id == scope::PARAM) {
    vector<usr*>& order = ptr->m_order;
    copy(order.rbegin(),order.rend(),back_inserter(res));
  }
  else
    return;

  if (ptr->m_parent)
    gather(ptr->m_parent, res);
}

int cxx_compiler::statements::return_stmt::info_t::gen()
{
  using namespace std;
  var* expr = 0;
  if (m_expr) {
    parse::position = m_expr->file();
    expr = m_expr->gen();
  }
  const type* T = fundef::current->m_usr->m_type;
  if (T->m_id != type::FUNC)
    return 0;
  typedef const func_type FT;
  FT* ft = static_cast<FT*>(T);
  T = ft->return_type();
  T = T->unqualified();
  if (T->m_id != type::REFERENCE && expr)
    expr = expr->rvalue();
  if (expr) {
    if (T->m_id == type::VOID) {
      if (expr->m_type->m_id == type::VOID)
        return 0;
    }
    bool discard = false;
    bool ctor_conv = false;
    const type* res =
      expressions::assignment::valid(T, expr, &discard, &ctor_conv, 0);
    if (!res) {
      if (instantiate_with_template_param<template_usr>())
	return 0;
      if (instantiate_with_template_param<template_tag>())
	return 0;
      using namespace error::statements::return_stmt;
      const type* from = expr->m_type;
      const type* to = ft->return_type();
      invalid(m_file,from,to);
      return 0;
    }
    expr = T->aggregate() ?
      aggregate_conv(T, expr, ctor_conv, 0) : expr->cast(T);
    if (T->m_id == type::REFERENCE && res->m_id != type::REFERENCE)
      expr = expr->address();
    expr = copy_ctor(T, expr);
  }
  else {
    if (T->m_id != type::VOID) {
      using namespace error::statements::return_stmt;
      invalid(m_file,void_type::create(),T);
    }
  }
  for_each(begin(m_vars), end(m_vars), call_dtor);
  code.push_back(new return3ac(expr));
  return 0;
}

int cxx_compiler::statements::goto_stmt::info_t::gen()
{
  using namespace std;
  string name = m_label->m_name;
  map<string,label::define>::const_iterator p = label::defined.find(name);
  if ( p != label::defined.end() ){
    const label::define& d = p->second;
    if ( outside(scope::current,d.m_scope) ){
      const vector<usr*>& v = label::vm;
      vector<usr*>::const_iterator p = find_if(v.begin(),v.end(),bind2nd(ptr_fun(cmp),d.m_scope));
      if ( p != v.end() ){
        using namespace error::statements::goto_stmt;
        invalid(m_label->m_file,d.m_file,*p);
      }
    }
    to3ac* to = d.m_to;
    goto3ac* go = new goto3ac;
    code.push_back(go);
    to->m_goto.push_back(go);
    go->m_to = to;
  }
  else {
    goto3ac* go = new goto3ac;
    label::used[name].push_back(label::use(go,m_label->m_file,scope::current));
    code.push_back(go);
  }
  return 0;
}

bool cxx_compiler::statements::goto_stmt::outside(const scope* from, const scope* to)
{
  if ( const scope* parent = to->m_parent ){
    if ( from == parent )
      return true;
    else
      return outside(from,parent);
  }
  else
    return false;
}

bool cxx_compiler::statements::goto_stmt::cmp(const usr* u, const scope* s)
{
  return u->m_scope == s;
}

namespace cxx_compiler { namespace statements { namespace declaration {
  extern void check_storage(usr*);
} } } // end of namespace declaration, statements and cxx_compiler

cxx_compiler::statements::declaration::info_t::
info_t(std::vector<usr*>* usrs, bool for_stmt)
  : m_usrs(usrs)
{
  using namespace std;
  if (m_usrs) {
    if (for_stmt) {
      for (auto u : *m_usrs)
        check_storage(u);
    }
  }
}

void cxx_compiler::statements::declaration::check_storage(usr* u)
{
  usr::flag_t& flag = u->m_flag;
  usr::flag_t mask = usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::STATIC);
  if (flag & mask) {
    using namespace error::statements::for_stmt;
    invalid_storage(u);
    flag = usr::flag_t(flag & ~mask);
  }
}

int cxx_compiler::statements::declaration::info_t::gen()
{
  using namespace std;
  if (m_usrs)
    for_each(m_usrs->begin(),m_usrs->end(),mem_fun(&usr::initialize));
  return 0;
}

int cxx_compiler::statements::try_block::info_t::gen()
{
  exception::try_block::action(m_stmt, m_handlers);
  return 0;
}

cxx_compiler::statements::try_block::info_t::~info_t()
{
  delete m_stmt;
  for (auto p : *m_handlers) {
    delete p->second;
    delete p;
  }
  delete m_handlers;
}

namespace cxx_compiler {
  namespace statements {
    namespace condition {
      expressions::base* action(declarations::type_specifier_seq::info_t* p,
                		var* v, expressions::base* right)
      {
        typedef declarations::type_specifier_seq::info_t X;
        auto_ptr<X> sweeper(p);
        p->update();
        const type* T = p->m_type;
        using namespace declarations;
        type_specifier* ts = new type_specifier(T);
        parse::identifier::mode = parse::identifier::look;
        specifier* sp = new specifier(ts);
        typedef specifier_seq::info_t Y;
        Y* q = new specifier_seq::info_t(0, sp);
        auto_ptr<Y> sweeper2(q);
        declarations::action1(v, false);
        expressions::base* left = new expressions::primary::info_t(v);
        return new expressions::binary::info_t(left, '=', right);
      }
    } // end of  namespace condition
  } // end of namespace statements
} // end of namespace cxx_compiler
