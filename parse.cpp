#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

cxx_compiler::file_t cxx_compiler::parse::position;

namespace cxx_compiler { namespace parse { namespace identifier {
  flag_t flag;
  bool g_peek_coloncolon;
  bool g_maybe_absdecl;
  int create(std::string, const type* = backpatch_type::create());
  int lookup(std::string, scope*);
} } } // end of namespace identifier, parse and cxx_compiler

int cxx_compiler::parse::identifier::judge(std::string name)
{
  if ( flag == peeking )
    return create(name);
  if ( g_maybe_absdecl ){
    if ( lookup(name,scope::current) == TYPEDEF_NAME_LEX )
      return TYPEDEF_NAME_LEX;
  }
  int c = peek();
  if ( c == COLONCOLON_MK ){
    g_peek_coloncolon = true;
    return lookup(name,scope::current);
  }
  if ( c == '(' ){
    if ( scope::current->m_id == scope::TAG ){
      tag* Tg = static_cast<tag*>(scope::current);
      if ( Tg->m_name == name ){
        class_or_namespace_name::before = 0;
        if ( parse::last_token == '~' ){
          cxx_compiler_lval.m_tag = Tg;
          return CLASS_NAME_LEX;
        }
        else {
          int ret = create(name);
          usr* u = cxx_compiler_lval.m_usr;
          u->m_flag = usr::flag_t(usr::CTOR | usr::FUNCTION);
          return ret;
        }
      }
    }
  }
  g_peek_coloncolon = false;
  if ( flag != new_obj )
    return lookup(name,scope::current);
  return create(name);
}

int cxx_compiler::parse::identifier::create(std::string name, const type* T)
{
  cxx_compiler_lval.m_usr = new usr(name,T,usr::NONE,parse::position);
  return IDENTIFIER_LEX;
}

namespace cxx_compiler { namespace parse { namespace identifier {
  namespace underscore_func {
    int action();
  } // end of namespace underscore_func
  struct base_arg {
    std::string m_name;
    std::vector<std::vector<usr*> >* m_res;
    base_arg(std::string name, std::vector<std::vector<usr*> >* res)
      : m_name(name), m_res(res) {}
  };
  int bases_lookup(std::string, tag*);
  int base_lookup(base*, base_arg*);
} } } // end of namespace identifier, parse and cxx_compiler

int cxx_compiler::parse::identifier::lookup(std::string name, scope* ptr)
{
  using namespace std;
  const map<string, vector<usr*> >& usrs = ptr->m_usrs;
  map<string, vector<usr*> >::const_iterator p = usrs.find(name);
  if ( p != usrs.end() ){
    const vector<usr*>& v = p->second;
    usr* u = v.back();
    cxx_compiler_lval.m_usr = u;
    if ( u->m_flag & usr::ENUM_MEMBER ){
      cxx_compiler_lval.m_usr = static_cast<enum_member*>(u)->m_value;
      return INTEGER_LITERAL_LEX;
    }
    if ( u->m_flag & usr::TYPEDEF ) {
      type_def* tdef = static_cast<type_def*>(u);
	  tdef->m_refed.push_back(parse::position);
      return TYPEDEF_NAME_LEX;
	}
    if ( u->m_flag & usr::OVERLOAD )
      return IDENTIFIER_LEX;
    if ( u->m_flag & usr::NAMESPACE ){
      cxx_compiler_lval.m_name_space = static_cast<name_space*>(u);
      return ORIGINAL_NAMESPACE_NAME_LEX;
    }
    const type* T = u->m_type;
    if ( const pointer_type* G = T->ptr_gen() )
      garbage.push_back(cxx_compiler_lval.m_var = new genaddr(G,T,u,0));
    return IDENTIFIER_LEX;
  }
  const map<string, tag*>& tags = ptr->m_tags;
  map<string, tag*>::const_iterator q = tags.find(name);
  if ( q != tags.end() ){
    tag* Tag = q->second;
    cxx_compiler_lval.m_tag = q->second;
    return Tag->m_kind == tag::ENUM ? ENUM_NAME_LEX : CLASS_NAME_LEX;
  }
  if ( flag == member ){
    assert(scope::current->m_id == scope::TAG);
    tag* Tg = static_cast<tag*>(scope::current);
    if ( int n = bases_lookup(name,Tg) )
      return n;
    error::undeclared(parse::position,name);
    return create(name,int_type::create());
  }
  else {
    if ( ptr->m_id == scope::TAG ){
      tag* Tg = static_cast<tag*>(ptr);
      if ( int n = bases_lookup(name,Tg) )
        return n;
    }
    if ( ptr->m_parent )
      return lookup(name,ptr->m_parent);
    else {
      if ( name == "__func__" )
        return underscore_func::action();
      if ( g_maybe_absdecl )
        return 0;
      if ( peek() == ':' ) // identifier ':' statement
        return create(name);
      if ( last_token == NAMESPACE_KW )
        return create(name);
      error::undeclared(parse::position,name);
      return create(name,int_type::create());
    }
  }
}

cxx_compiler::parse::read cxx_compiler::parse::g_read;

int cxx_compiler::parse::peek()
{
  using namespace std;
  if ( !g_read.m_token.empty() ){
    parse::position = g_read.m_token.front().second;
    return g_read.m_token.front().first;
  }

  if ( member_function_body::g_restore.m_saved ){
    list<pair<int, file_t> >& token = member_function_body::g_restore.m_saved->m_token;
    assert(!token.empty());
    parse::position = token.front().second;
    return token.front().first;
  }

  identifier::flag_t org = identifier::flag;
  identifier::flag = identifier::peeking;
  int r = lex_and_save();
  identifier::flag = org;
  return r;
}

int cxx_compiler::parse::lex_and_save()
{
  using namespace std;
  int n = cxx_compiler_lex();
  g_read.m_token.push_back(make_pair(n,position));
  switch ( n ){
  case IDENTIFIER_LEX:
    g_read.m_lval.push_back(cxx_compiler_lval.m_var);
    break;
  case INTEGER_LITERAL_LEX:
  case CHARACTER_LITERAL_LEX:
  case FLOATING_LITERAL_LEX:
  case TYPEDEF_NAME_LEX:
    g_read.m_lval.push_back(cxx_compiler_lval.m_usr);
    break;
  case STRING_LITERAL_LEX:
    g_read.m_lval.push_back(cxx_compiler_lval.m_var);
    break;
  case CLASS_NAME_LEX:
  case ENUM_NAME_LEX:
    g_read.m_lval.push_back(cxx_compiler_lval.m_tag);
    break;
  case DEFAULT_KW:
    g_read.m_lval.push_back(cxx_compiler_lval.m_file);
    break;
  case ORIGINAL_NAMESPACE_NAME_LEX:
  case NAMESPACE_ALIAS_LEX:
    g_read.m_lval.push_back(cxx_compiler_lval.m_name_space);
    break;
  }
  return n;
}

int cxx_compiler::parse::get_token()
{
  if ( !g_read.m_token.empty() ){
    position = g_read.m_token.front().second;
    last_token = g_read.m_token.front().first;
    g_read.m_token.pop_front();
    switch ( last_token ){
    case IDENTIFIER_LEX:
      cxx_compiler_lval.m_var = static_cast<var*>(g_read.m_lval.front());
      g_read.m_lval.pop_front();
      break;
    case INTEGER_LITERAL_LEX:
    case CHARACTER_LITERAL_LEX:
    case FLOATING_LITERAL_LEX:
    case TYPEDEF_NAME_LEX:
      cxx_compiler_lval.m_usr = static_cast<usr*>(g_read.m_lval.front());
      g_read.m_lval.pop_front();
      break;
    case STRING_LITERAL_LEX:
      cxx_compiler_lval.m_var = static_cast<var*>(g_read.m_lval.front());
      g_read.m_lval.pop_front();
      break;
    case CLASS_NAME_LEX:
    case ENUM_NAME_LEX:
      cxx_compiler_lval.m_tag = static_cast<tag*>(g_read.m_lval.front());
      g_read.m_lval.pop_front();
      break;
    case DEFAULT_KW:
      cxx_compiler_lval.m_file = static_cast<file_t*>(g_read.m_lval.front());
      g_read.m_lval.pop_front();
      break;
    case ORIGINAL_NAMESPACE_NAME_LEX:
    case NAMESPACE_ALIAS_LEX:
      cxx_compiler_lval.m_name_space = static_cast<name_space*>(g_read.m_lval.front());
      g_read.m_lval.pop_front();
      break;
    }
    if ( last_token == COLONCOLON_MK )
      identifier::flag = identifier::look;
    return last_token;
  }

  if ( member_function_body::g_restore.m_saved ){
    last_token = member_function_body::get_token();
    if ( last_token == COLONCOLON_MK )
      identifier::flag = identifier::look;
    return last_token;
  }

  last_token = cxx_compiler_lex();
  if ( last_token == COLONCOLON_MK )
    identifier::flag = identifier::look;
  return last_token;
}

namespace cxx_compiler { namespace parse { namespace identifier { namespace underscore_func {
  struct func {
    std::map<int, var*>& m_value;
    func(std::map<int, var*>& v) : m_value(v) {}
    int operator()(int, char);
  };
} } } } // end of namespace underscore_func, identifier, parse and cxx_compiler

int cxx_compiler::parse::identifier::underscore_func::action()
{
  using namespace std;
  string s;
  if ( fundef::current )
    s = fundef::current->m_usr->m_name;
  else {
    using namespace error::expressions::primary::underscore_func;
    outside(parse::position);
  }
  const type* T = char_type::create();
  T = const_type::create(T);
  T = array_type::create(T,s.length()+1);
  with_initial* u = new with_initial("__func__",T,parse::position);
  map<int, var*>& v = u->m_value;
  accumulate(s.begin(),s.end(),0,func(v));
  v[s.length()] = expressions::primary::literal::integer::create(char(0));
  scope::current->m_usrs["__func__"].push_back(u);
  const pointer_type* G = T->ptr_gen();
  cxx_compiler_lval.m_var = new genaddr(G,T,u,0);
  garbage.push_back(cxx_compiler_lval.m_var);
  return IDENTIFIER_LEX;
}

int cxx_compiler::parse::identifier::underscore_func::func::operator()(int n, char c)
{
  m_value[n] = expressions::primary::literal::integer::create(c);
  return n + 1;
}

int cxx_compiler::parse::identifier::bases_lookup(std::string name, tag* Tg)
{
  using namespace std;
  const vector<base*>* bases = Tg->m_bases;
  if ( !bases )
    return 0;
  vector<vector<usr*> > res;
  base_arg arg(name,&res);
  for_each(bases->begin(),bases->end(),bind2nd(ptr_fun(base_lookup),&arg));
  if ( res.empty() )
    return 0;
  const vector<usr*>& v = res.back();
  usr* u = v.back();
  cxx_compiler_lval.m_usr = u;
  return IDENTIFIER_LEX;
}

int cxx_compiler::parse::identifier::base_lookup(base* bp, base_arg* arg)
{
  using namespace std;
  tag* Tg = bp->m_tag;
  const map<string, vector<usr*> >& usrs = Tg->m_usrs;
  string name = arg->m_name;
  map<string, vector<usr*> >::const_iterator p = usrs.find(name);
  if ( p == usrs.end() )
    return 0;
  const vector<usr*>& v = p->second;
  arg->m_res->push_back(v);
  return 0;
}

bool cxx_compiler::parse::is_last_decl = true;

void cxx_compiler::parse::parameter::enter()
{
  using namespace std;
  ++depth;
  if ( class_or_namespace_name::last ){
    scope::current = class_or_namespace_name::last;
    class_or_namespace_name::last = 0;
  }
  vector<scope*>& children = scope::current->m_children;
  scope* param = new scope(scope::PARAM);
  param->m_parent = scope::current;
  children.push_back(param);
  scope::current = param;
  declarations::specifier_seq::info_t::clear();
}

void cxx_compiler::parse::parameter::leave()
{
  using namespace std;
  scope* org = 0;
  if ( scope::current->m_parent == &scope::root ){
    vector<scope*>& children = scope::root.m_children;
    if ( children.size() != 1 )
      org = scope::current;
  }
  else {
    scope::id_t id = scope::current->m_parent->m_id;
    if ( id != scope::TAG && id != scope::NAMESPACE )
      org = scope::current;
  }
  scope::current = scope::current->m_parent;
  --depth;
  if ( org ){
    vector<scope*>& children = org->m_parent->m_children;
    assert(children.back() == org);
    children.pop_back();
    delete org;
  }
}

int cxx_compiler::parse::parameter::depth;

namespace cxx_compiler { namespace parse { namespace parameter {
  inline void move(var *v)
  {
    using namespace std;
    vector<var*>::reverse_iterator p = find(garbage.rbegin(),garbage.rend(),v);
    if (p != garbage.rend()) {
      garbage.erase(p.base()-1);
      cxx_compiler::block* b =
	static_cast<cxx_compiler::block*>(scope::current);
      v->m_scope = b;
      b->m_vars.push_back(v);
    }
  }
  inline void move()
  {
    for (auto p : code) {
      if (p->x) move(p->x);
      if (p->y) move(p->y);
      if (p->z) move(p->z);
    }
  }
} } } // end of namespace parameter, parse and cxx_compiler

namespace cxx_compiler { namespace parse { namespace block {
  void new_block();
} } } // end of namespace block, parse and cxx_compiler

namespace cxx_compiler { namespace parse { namespace member_function_body {
  void save();
} } } // end of namespace member_function_body, parse and cxx_compiler

void cxx_compiler::parse::block::enter()
{
  using namespace std;

  if ( scope::current == &scope::root ){
    vector<scope*>& children = scope::current->m_children;
    assert(children.size() == 1);
    scope::current = children.back();
    return new_block(), parameter::move();
  }

  if ( scope::current->m_id == scope::TAG ){
    tag* tg = static_cast<tag*>(scope::current);
    const type* T = tg->m_types.second;
    vector<scope*>& children = scope::current->m_children;
    scope::current = children.back();
    if ( !T ){
      new_block(), parameter::move();
      usr::flag_t& flag = fundef::current->m_usr->m_flag;
      flag = usr::flag_t(flag | usr::INLINE);
      member_function_body::save();
      return;
    }
    if ( !(fundef::current->m_usr->m_flag & usr::STATIC) ){
      T = pointer_type::create(T);
      string name = "this";
      usr* u = new usr(name,T,usr::NONE,file_t());
      scope::current->m_usrs[name].push_back(u);
      scope::current->m_order.push_back(u);
    }
    vector<scope*>& c = scope::current->m_children;
    if ( !c.empty() ){
      assert(c.size() == 1);
      scope::current = c.back();
      return;
    }
    return new_block(), parameter::move();
  }

  if ( scope::current->m_id == scope::NAMESPACE ){
    vector<scope*>& c = scope::current->m_children;
    assert(c.size() == 1);
    scope::current = c.back();
    return new_block(), parameter::move();
  }

  new_block();
}

void cxx_compiler::parse::block::new_block()
{
  using namespace std;
  cxx_compiler::block* b = new cxx_compiler::block();
  b->m_parent = scope::current;
  vector<scope*>& children = scope::current->m_children;
  children.push_back(b);
  scope::current = b;
}

void cxx_compiler::parse::block::leave()
{
  scope::current = scope::current->m_parent;
  if ( scope::current->m_parent == &scope::root )
    scope::current = &scope::root;
  else {
    scope::id_t id = scope::current->m_parent->m_id;
    if ( id == scope::TAG || id == scope::NAMESPACE )
      scope::current = scope::current->m_parent;
  }
}

namespace cxx_compiler { namespace parse { namespace member_function_body {
  std::map<usr*, saved> table;
  restore g_restore;
  void save_brace();
} } } // end of namespace parse, member_function_body and cxx_compiler

void cxx_compiler::parse::member_function_body::save()
{
  using namespace std;
  identifier::flag = identifier::new_obj;
  usr* key = fundef::current->m_usr;
  scope* ptr = scope::current->m_parent;
  assert(ptr->m_id == scope::PARAM);
  table[key].m_param = ptr;
  table[key].m_token.push_back(make_pair('{',position));
  save_brace();
  g_read.m_token.push_front(make_pair('}',position));
  identifier::flag = identifier::look;
}

void cxx_compiler::parse::member_function_body::save_brace()
{
  using namespace std;
  usr* key = fundef::current->m_usr;
  saved& tmp = table[key];
  list<pair<int, file_t> >& token = tmp.m_token;
  list<var*>& lval = tmp.m_lval;
  while ( 1 ){
    int n;
    if ( !g_read.m_token.empty() ){
      position = g_read.m_token.front().second;
      n = g_read.m_token.front().first;
      g_read.m_token.pop_front();
    }
    else
      n = cxx_compiler_lex();
    token.push_back(make_pair(n,position));
    switch ( n ){
    case IDENTIFIER_LEX:
    case INTEGER_LITERAL_LEX:
    case CHARACTER_LITERAL_LEX:
    case FLOATING_LITERAL_LEX:
      lval.push_back(cxx_compiler_lval.m_usr);
      break;
    case STRING_LITERAL_LEX:
      lval.push_back(cxx_compiler_lval.m_var);
      break;
    }
    if ( n == '{' )
      save_brace();
    if ( n == '}' )
      break;
  }
}

int cxx_compiler::parse::member_function_body::get_token()
{
  using namespace std;
  list<pair<int, file_t> >& token = g_restore.m_saved->m_token;
  if ( token.empty() )
    return 0; // YYEOF
  position = token.front().second;
  int n = token.front().first;
  token.pop_front();
  list<var*>& lval = g_restore.m_saved->m_lval;
  switch ( n ){
  case IDENTIFIER_LEX:
    {
      assert(!lval.empty());
      var* v = lval.front();
      lval.pop_front();
      usr* u = v->usr_cast();
      if ( !u ){
        assert(v->genaddr_cast());
        cxx_compiler_lval.m_var = v;
        return IDENTIFIER_LEX;
      }
      cxx_compiler_text = const_cast<char*>(u->m_name.c_str());
      n = identifier::judge(cxx_compiler_text);
    }
    break;
  case INTEGER_LITERAL_LEX:
  case CHARACTER_LITERAL_LEX:
  case FLOATING_LITERAL_LEX:
    assert(!lval.empty());
    cxx_compiler_lval.m_usr = static_cast<usr*>(lval.front());
    lval.pop_front();
    break;
  case STRING_LITERAL_LEX:
    assert(!lval.empty());
    cxx_compiler_lval.m_var = lval.front();
    lval.pop_front();
    break;
  }
  return n;
}

int cxx_compiler::parse::last_token;

std::stack<cxx_compiler::parse::backtrack> cxx_compiler::parse::backtrack::g_stack;

std::string cxx_compiler::ucn::conv(std::string name)
{
  using namespace std;
  typedef string::size_type T;
  T end = string::npos;
  for ( T p = name.find('\\') ; p != end ; p = name.find('\\',p+1) ){
    int c = name[p+1];
    if ( c == 'u' ){
      string s = name.substr(p+2,4);
      int n = strtol(s.c_str(),0,16);
      name.replace(p,6,1,n);
    }
    else if ( c == 'U' ){
      string s = name.substr(p+2,4);
      int n = strtol(s.c_str(),0,16);
      string t = name.substr(p+6,4);
      int m = strtol(t.c_str(),0,16);
      name.replace(p,10,1,n << 16 | m);
    }
  }
  return name;
}

cxx_compiler::expressions::primary::info_t::info_t()
 : m_var(0), m_expr(0), m_file(parse::position)
{
  parse::identifier::lookup("this",scope::current);
  m_var = cxx_compiler_lval.m_usr;
}

namespace cxx_compiler { namespace parse {
  std::list<void*>::const_iterator debug_subr(std::ostream&, int, std::list<void*>::const_iterator);
} }

void cxx_compiler::parse::debug()
{
  using namespace std;
  ostringstream os;
  typedef list<pair<int, file_t> >::const_iterator IT;
  list<void*>::const_iterator q = g_read.m_lval.begin();
  for ( IT p = g_read.m_token.begin() ; p != g_read.m_token.end() ; ++p ){
    int n = p->first;
    q = debug_subr(os,n,q);
  }
  string debug = os.str();
}

std::list<void*>::const_iterator
cxx_compiler::parse::debug_subr(std::ostream& os, int n, std::list<void*>::const_iterator q)
{
  if ( 33 <= n && n <= 126 ){
    os << char(n) << ' ';
    return q;
  }
  if ( n == IDENTIFIER_LEX ){
    os << "IDENTIFIER_LEX";
    var* v = static_cast<var*>(*q);
    usr* u = v->usr_cast();
    if ( u )
      os << '(' << u->m_name << ')' << ' ';
    else {
      genaddr* ga = v->genaddr_cast();
      assert(ga);
      throw int();
    }
    return ++q;
  }
  if ( n == INTEGER_LITERAL_LEX ||
       n == CHARACTER_LITERAL_LEX ||
       n == FLOATING_LITERAL_LEX ||
       n == TYPEDEF_NAME_LEX ){
    usr* u = static_cast<usr*>(*q);
    os << u->m_name << ' ';
    return ++q;
  }
  throw int();
  /*
  STRING_LITERAL_LEX
  CLASS_NAME_LEX
  ENUM_NAME_LEX
  DEFAULT_KW
  ORIGINAL_NAMESPACE_NAME_LEX
  NAMESPACE_ALIAS_LEX
  */
}
