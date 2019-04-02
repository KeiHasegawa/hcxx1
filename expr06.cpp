// compound-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

namespace cxx_compiler {
  namespace expressions {
    namespace compound {
      using namespace std;
      struct literal : usr {
        literal(string name, const type* T, usr::flag_t flag,
                const file_t& file)
          : usr(name,T,flag,file) {}
      };
    }  // end of namespace compound
  }  // end of namespace expressions
}  // end of namespace cxx_compiler

cxx_compiler::var* cxx_compiler::expressions::compound::info_t::gen()
{
  using namespace std;
  typedef const pointer_type PT;
  typedef const array_type ARRAY;
  if ( scope::current != &scope::root ){
    string name = new_name(".compound");
    usr* ret = new literal(name,m_type,usr::NONE,parse::position);
    if ( scope::current->m_id == scope::BLOCK )
      scope::current->m_usrs[name].push_back(ret);
    else
      garbage.push_back(ret);
    map<int,var*> v;
    using namespace declarations::initializers;
    argument::dst = ret;
    argument arg(m_type,v,0,0,0,0,0,m_list->size());
    for_each(m_list->begin(),m_list->end(),
             bind2nd(ptr_fun(lst::gencode),&arg));
    fill_zero(&arg);
    for_each(v.begin(),v.end(), bind1st(ptr_fun(gen_loff),ret));
    if ( m_type->m_id == type::ARRAY ){
      ARRAY* array = static_cast<ARRAY*>(m_type);
      if ( !array->dim() ){
        const type* T = array->element_type();
        int n = T->size();
        int dim = arg.off_max / n;
        ret->m_type = m_type = array_type::create(T,dim);
      }
      PT* G = m_type->ptr_gen();
      var* gen = new genaddr(G,m_type,ret,0);
      garbage.push_back(gen);
      return gen;
    }
    else
      return ret;
  }
  else {
    string name = new_name(".compound");
    with_initial* ret = new with_initial(name,m_type,parse::position);
    using namespace declarations::initializers;
    argument::dst = ret;
    argument arg(m_type,ret->m_value,0,0,0,0,0,m_list->size());
    for_each(m_list->begin(),m_list->end(),
             bind2nd(ptr_fun(lst::gencode),&arg));
    fill_zero(&arg);
    scope::current->m_usrs[name].push_back(ret);
    if ( m_type->m_id != type::ARRAY ){
      if ( arg.not_constant )
        initialize_code(ret);
      return ret;
    }
    ARRAY* array = static_cast<ARRAY*>(m_type);
    if ( !array->dim() ){
      const type* T = array->element_type();
      int n = T->size();
      int dim = arg.off_max / n;
      ret->m_type = m_type = array_type::create(T,dim);
    }
    if ( arg.not_constant )
      initialize_code(ret);
    PT* G = m_type->ptr_gen();
    var* gen = new genaddr(G,m_type,ret,0);
    garbage.push_back(gen);
    return gen;
  }
}

cxx_compiler::expressions::compound::info_t::~info_t()
{
  for (auto p : *m_list)
    delete p;
  delete m_list;
}
