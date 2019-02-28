#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

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
  for (auto p : m_children)
    delete p;
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
  usr* u = static_cast<usr*>(v);
  auto_ptr<usr> sweeper(u);
  string name = u->m_name;
  map<string, vector<usr*> >& usrs = scope::current->m_usrs;
  name_space* ptr = new name_space(name,parse::position);
  usrs[name].push_back(ptr);
  ptr->m_parent = scope::current;
  scope::current = ptr;
}
