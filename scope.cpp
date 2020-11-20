#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

cxx_compiler::scope::scope(id_t id = NONE) : m_id(id), m_parent(0)
{
  class_or_namespace_name::before.push_back(this);
}

cxx_compiler::scope cxx_compiler::scope::root;
cxx_compiler::scope* cxx_compiler::scope::current = &cxx_compiler::scope::root;

namespace cxx_compiler { namespace parameter {
  misc::pvector<tag> tags;
  tag* conv(const std::pair<std::string,tag*>& p)
  {
    tag* T = p.second;
    T->m_parent = &scope::root;
    return T;
  }
} } // end of namespace parameter and cxx_compiler

cxx_compiler::scope::~scope()
{
  using namespace std;

  for (const auto& p : m_usrs) {
    for (auto q : p.second)
      delete q;
  }

  for (auto p : m_children) {
    scope::id_t id = p->m_id;
    switch (id) {
    case scope::PARAM:
    case scope::BLOCK:
      delete p;
      break;
    case scope::TAG:
      p->m_parent = 0;
      break;
    }
  }
}

std::string cxx_compiler::tag::keyword(kind_t kind)
{
  switch (kind) {
  case CLASS:  return "class";
  case STRUCT: return "struct";
  case UNION:  return "union";
  case ENUM:   return "enum";
  case TEMPL:  return "template";
  default:     return "typename";
  }
}

cxx_compiler::block::~block()
{
  for (auto p : m_vars)
    delete p;
}

namespace cxx_compiler {
  namespace block_impl {
    map<block*, vector<var*> > dtor_tbl;
    set<usr*> tried;
  } // end of namespace block_impl
} // end of namespace cxx_compiler

std::string cxx_compiler::usr::keyword(flag_t flag)
{
  using namespace std;
  ostringstream os;
  if (flag & TYPEDEF)
    os << "typedef";
  if (flag & EXTERN)
    os << "extern";
  if (flag & STATIC)
    os << "static";
  if (flag & AUTO)
    os << "auto";
  if (flag & REGISTER)
    os << "register";
  if (flag & INLINE) {
    string s = os.str();
    if (!s.empty())
      os << ' ';
    os << "inline";
  }
  if (flag & VIRTUAL) {
    string s = os.str();
    if (!s.empty())
      os << ' ';
    os << "virtual";
  }
  if (flag & EXPLICIT) {
    string s = os.str();
    if (!s.empty())
      os << ' ';
    os << "explicit";
  }
  if (flag & FRIEND) {
    string s = os.str();
    if (!s.empty())
      os << ' ';
    os << "friend";
  }
  if (flag & NAMESPACE)
    os << "namespace";
  return os.str();
}

cxx_compiler::tag::~tag()
{
  if ( m_bases ){
    for (auto p : *m_bases)
      delete p;
    delete m_bases;
  }
}

cxx_compiler::base::base(int access, bool virt, tag* ptr)
  : m_flag(usr::NONE), m_tag(ptr)
{
  switch (access) {
  PRIVATE_KEY: m_access = usr::PRIVATE; break;
  PROTECTED_KEY: m_access = usr::PROTECTED; break;
  PUBLIC_KEY: m_access = usr::PUBLIC; break;
  }
  m_flag = virt ? usr::VIRTUAL : usr::NONE;
}

void cxx_compiler::original_namespace_definition(var* v, bool inl)
{
  using namespace std;
  assert(v->usr_cast());
  usr* u = static_cast<usr*>(v);
  auto_ptr<usr> sweeper(u);
  string name = u->m_name;
  map<string, vector<usr*> >& usrs = scope::current->m_usrs;
  name_space* ptr = new name_space(name, parse::position);
  if (inl)
    ptr->m_flag = usr::flag_t(ptr->m_flag | usr::INLINE);
  usrs[name].push_back(ptr);
  ptr->m_parent = scope::current;
  ptr->m_parent->m_children.push_back(ptr);
  scope::current = ptr;
}

void cxx_compiler::extension_namespace_definition(var* v)
{
  assert(v->usr_cast());
  usr* u = static_cast<usr*>(v);
  usr::flag_t flag = u->m_flag;
  assert(flag & usr::NAMESPACE);
  name_space* ptr = static_cast<name_space*>(u);
  scope::current = ptr;
  using namespace class_or_namespace_name;
  before.push_back(scope::current);
}

void cxx_compiler::using_directive::action(name_space* ns)
{
  scope::current->m_using.push_back(ns);
}
