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
  using namespace parameter;

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
    default:
      p->m_parent = 0;
      break;
    }
  }
}

std::string cxx_compiler::tag::keyword(kind_t kind)
{
  switch ( kind ){
  case CLASS: return "class";
  case STRUCT: return "struct";
  case UNION:  return "union";
  default: return "enum";
  }
}

cxx_compiler::block::~block()
{
  for (auto p : m_vars)
    delete p;
}

std::string cxx_compiler::usr::keyword(flag_t f)
{
  using namespace std;
  ostringstream os;
  if ( f & TYPEDEF )
    os << "typedef";
  else if ( f & EXTERN ){
    os << "extern";
    if ( f & INLINE )
      os << " inline";
  }
  else if ( f & STATIC ){
    os << "static";
    if ( f & INLINE )
      os << " inline";
  }
  else if ( f & AUTO )
    os << "auto";
  else if ( f & REGISTER )
    os << "register";
  else if ( f & INLINE )
    os << "inline";
  else if ( f & VIRTUAL )
    os << "virtual";
  else if ( f & NAMESPACE )
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

void cxx_compiler::original_namespace_definition(var* v)
{
  using namespace std;
  assert(v->usr_cast());
  usr* u = static_cast<usr*>(v);
  auto_ptr<usr> sweeper(u);
  string name = u->m_name;
  map<string, vector<usr*> >& usrs = scope::current->m_usrs;
  name_space* ptr = new name_space(name,parse::position);
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
