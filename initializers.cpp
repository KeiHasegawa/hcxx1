#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

namespace cxx_compiler { namespace declarations { namespace initializers {
  struct gendata {
    std::vector<tac*> m_code;
    std::map<int, var*> m_value;
  };
  std::map<usr*, gendata> m_table;
  usr* argument::dst;
  namespace clause {
    extern int gencode(info_t*, argument*);
  }
  extern int expr_list(std::vector<expressions::base*>*, argument*);
} } } // end of namespace initializers, declarations and cxx_compiler

void cxx_compiler::declarations::initializers::action(var* v, info_t* i)
{
  using namespace std;
  usr* u = static_cast<usr*>(v);
  auto_ptr<info_t> sweeper(i);
  argument::dst = u;
  with_initial* p = u->with_initial_cast();
  argument arg(u->m_type,p ? p->m_value : m_table[u].m_value,0,0,-1,-1,-1,-1);
  int n = code.size();
  i->m_clause ? clause::gencode(i->m_clause,&arg) : expr_list(i->m_exprs,&arg);
  if ( p ){
#ifdef _DEBUG
    map<int,var*>& m = p->m_value;
    typedef map<int,var*>::const_iterator IT;
    for ( IT it = m.begin() ; it != m.end() ; ++it ){
      var* v = it->second;
      if ( addrof* addrof = v->addrof_cast() ){
        static misc::pvector<var> v;
        v.push_back(addrof);
      }
    }
#endif // _DEBUG
    if ( scope::current != &scope::root )
      expressions::constant_flag = false;
  }
  else {
    int m = code.size();
    gendata& data = m_table[u];
    copy(code.begin()+n,code.begin()+m,back_inserter(data.m_code));
    code.resize(n);
  }
  const type* T = u->m_type;
  if ( T->m_id == type::ARRAY ){
    typedef const array_type ARRAY;
    ARRAY* array = static_cast<ARRAY*>(T);
    if ( !array->dim() ){
      T = array->element_type();
      int n = arg.off_max;
      int m = T->size();
	  if ( m )
        u->m_type = array_type::create(T,(n + m - 1)/ m);
    }
  }
  if ( p ){
    if ( arg.not_constant )
      initialize_code(p);
  }
  parse::identifier::mode = parse::identifier::new_obj;
}

namespace cxx_compiler { namespace declarations { namespace initializers {
  extern void gencode(usr*);
} } } // end of namespace initializers, declarations and cxx_compiler

int cxx_compiler::usr::initialize()
{
  declarations::initializers::gencode(this);
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  if ( b && m_type->variably_modified() ){
    if ( m_flag & usr::EXTERN ){
      using namespace error::declarations::declarators::vm;
      invalid_linkage(this);
      m_flag = usr::flag_t(m_flag & ~usr::EXTERN);
    }
  }
  using namespace declarations::declarators::array;
  variable_length::allocate(this);
  return 0;
}

void cxx_compiler::declarations::initializers::gencode(usr* u)
{
  using namespace std;
  using namespace declarations::declarators::function::definition::static_inline;
  map<usr*, gendata>::iterator p = m_table.find(u);
  if ( p != m_table.end() ){
    gendata& data = p->second;
    vector<tac*>& c = data.m_code;
    copy(c.begin(),c.end(),back_inserter(code));
    map<int,var*>& value = data.m_value;
    if ( value.size() == 1 && u->m_type->scalar() == value[0]->m_type->scalar() )
      code.push_back(new assign3ac(u,value[0]));
    else
      for_each(value.begin(),value.end(),bind1st(ptr_fun(gen_loff),u));
    m_table.erase(p);
  }
}

namespace cxx_compiler { namespace declarations { namespace initializers { namespace clause {
  extern int assign(var*, argument*);
  extern int lsting(std::vector<element*>*, argument*);
} } } } // end of namespace clause, initializers, declarations and cxx_compiler

int cxx_compiler::declarations::initializers::clause::gencode(info_t* c, argument* arg)
{
  return c->m_expr ? assign(c->m_expr->gen(),arg) : lsting(c->m_list,arg);
}

namespace cxx_compiler { namespace declarations { namespace initializers { namespace clause {
  extern int helper(expressions::base*, argument*);
} } } } // end of namespace clause, initializers, declarations and cxx_compiler

int cxx_compiler::declarations::initializers::expr_list(std::vector<expressions::base*>* v, argument* arg)
{
  using namespace std;
  for_each(v->begin(),v->end(),bind2nd(ptr_fun(clause::helper),arg));
  return 0;
}

int cxx_compiler::declarations::initializers::clause::helper(expressions::base* expr, argument* arg)
{
  return assign(expr->gen(),arg);
}

int cxx_compiler::declarations::initializers::gen_loff(usr* dst, std::pair<int,var*> p)
{
  int offset = p.first;
  var* off = expressions::primary::literal::integer::create(offset);
  var* src = p.second;
  code.push_back(new loff3ac(dst,off,src));
  return 0;
}

namespace cxx_compiler { namespace declarations { namespace initializers {
  namespace char_array_string {
    extern int action(var*, argument*);
  }
  extern int assign_special(var*, argument*);
  extern int merge(std::pair<std::map<int,var*>*,int>, std::pair<int,var*>);
  extern int bit_field(var*, argument*);
} } } // end of namespace initializers, declarations and cxx_compiler

int cxx_compiler::declarations::initializers::clause::assign(var* y, argument* arg)
{
  using namespace std;
  y = y->rvalue();
  if ( duration::_static(argument::dst) && !y->isconstant(true) )
    arg->not_constant = true;

  if ( int r = char_array_string::action(y,arg) )
    return r;
  const type* T = arg->T;
  if ( arg->nth >= 0 ){
    pair<int, const type*> ret = T->current(arg->nth);
    if ( ret.first < 0 ){
      using namespace error::declarations::initializers;
      exceed(argument::dst);
      return arg->off;
    }
    T = ret.second;
    if ( !T->scalar() && y->m_type->scalar() )
      return assign_special(y,arg);
    arg->off_max = max(arg->off_max, arg->off = ret.first);
  }
  bool discard = false;
  T = expressions::assignment::valid(T,y,&discard);
  if ( !T ){
    using namespace error::declarations::initializers;
    invalid_assign(parse::position,argument::dst,discard);
    return arg->off;
  }
  typedef const bit_field_type BF;
  if ( T->m_id == type::BIT_FIELD )
    return bit_field(y,arg);
  y = y->cast(T);
  if ( y->addrof_cast() ){
    vector<var*>& v = garbage;
    vector<var*>::reverse_iterator p = find(v.rbegin(),v.rend(),y);
    if (p != v.rend())
      v.erase(p.base()-1);
  }
  arg->V[arg->off] = y;
  arg->nth_max = max(arg->nth_max,++arg->nth);
  arg->off_max = max(arg->off_max, arg->off += T->size());
  return arg->off;
}

int
cxx_compiler::declarations::initializers::clause::lsting(std::vector<element*>* v, argument* arg)
{
  using namespace std;
  const type* T = arg->T;
  if ( arg->nth >= 0 ){
    pair<int, const type*> ret = T->current(arg->nth);
    if ( ret.first < 0 ){
      using namespace error::declarations::initializers;
      exceed(argument::dst);
      return arg->off;
    }
    arg->off_max = max(arg->off_max, arg->off = ret.first);
    T = ret.second;
  }
  map<int,var*> u;
  argument tmp(T,u,0,0,0,0,0,v->size());
  for_each(v->begin(),v->end(),bind2nd(ptr_fun(lst::gencode),&tmp));
  fill_zero(&tmp);
  for_each(u.begin(),u.end(),bind1st(ptr_fun(initializers::merge),make_pair(&arg->V,arg->off)));
  arg->nth_max = max(arg->nth_max, ++arg->nth);
  arg->off_max = max(arg->off_max, arg->off += tmp.off_max);
  if ( tmp.not_constant )
    arg->not_constant = true;
  return arg->off;
}

namespace cxx_compiler { namespace declarations { namespace initializers { namespace char_array_string {
  usr* is_string(var*);
  const array_type* char_array(const type*, int, bool*);
  struct eval {
    argument& arg;
    bool m_wide;
    bool m_escape;
    int m_hex_mode;
    char m_prev;
    bool m_shiftjis_state;
    int m_jis_state;
    int m_euc_state;
    eval(argument& a, bool wide)
      : arg(a), m_wide(wide), m_escape(false), m_hex_mode(0), m_prev(0),
        m_shiftjis_state(false), m_jis_state(0), m_euc_state(0) {}
    int operator()(int);
  };
} } } } // end of namespace char_array_string, initializers, declarations and cxx_compiler

int cxx_compiler::declarations::initializers::char_array_string::action(var* y, argument* arg)
{
  usr* s = is_string(y);
  if ( !s )
    return 0;
  typedef const array_type ARRAY;
  bool brace = false;
  ARRAY* array = char_array(arg->T,arg->nth,&brace);
  if ( !array )
    return 0;
  const type* T = array->element_type();
  var* zero = expressions::primary::literal::integer::create(char(0));
  zero = zero->cast(T);
  std::string name = s->m_name;
  bool wide = name[0] == 'L';
  name = name.substr(1+wide,name.length()-(2+wide));
  int dim = array->dim();
  using namespace std;
  map<int,var*> u;
  argument tmp(array,u,0,0,0,0,0,0);
  for_each(name.begin(),name.end(),eval(tmp,wide));
  if ( tmp.nth != dim || !dim ){
    u[tmp.off] = zero;
    tmp.off += wide ? sizeof(expressions::primary::literal::wchar_typedef) : 1;
    tmp.nth_max = max(tmp.nth_max,++tmp.nth);
  }
  fill_zero(&tmp);
  for_each(u.begin(),u.end(),bind1st(ptr_fun(merge),make_pair(&arg->V,arg->off)));
  arg->nth_max = max(arg->nth_max,brace ? arg->nth += tmp.nth_max : ++arg->nth);
  arg->off_max = max(arg->off_max, arg->off += tmp.off);
  if ( tmp.not_constant )
    arg->not_constant = true;
  return arg->off;
}

cxx_compiler::usr*
cxx_compiler::declarations::initializers::char_array_string::is_string(var* y)
{
  if ( genaddr* ptr = y->genaddr_cast() ){
    if ( usr* u = ptr->m_ref->usr_cast() ){
      std::string name = u->m_name;
      if ( name[0] == '"' )
        return u;
      if ( name[0] == 'L' && name.length() > 1 && name[1] == '"' )
        return u;
    }
  }
  return 0;
}

namespace cxx_compiler { namespace declarations { namespace initializers { namespace char_array_string {
  const array_type* char_array(const type*);
} } } } // end of namespace char_array_string, initializers, declarations and cxx_compiler

const cxx_compiler::array_type*
cxx_compiler::declarations::initializers::char_array_string::char_array(const type* T, int nth, bool* brace)
{
  using namespace std;
  if ( nth < 0 )
    return char_array(T);
  if ( nth == 0 ){
    if ( const array_type* r = char_array(T) ){
      *brace = true;
      return r;
    }
  }
  pair<int, const type*> ret = T->current(nth);
  if ( ret.first < 0 )
    return 0;
  return char_array(ret.second);
}

const cxx_compiler::array_type*
cxx_compiler::declarations::initializers::char_array_string::char_array(const type* T)
{
  if ( T->m_id != type::ARRAY )
    return 0;
  typedef const array_type ARRAY;
  ARRAY* array = static_cast<ARRAY*>(T);
  T = array->element_type();
  T = T->unqualified();
  if ( T != char_type::create() && T != wchar_type::create() )
    return 0;
  return array;
}

int cxx_compiler::declarations::initializers::char_array_string::eval::operator()(int c)
{
  using namespace std;
  using namespace expressions::primary::literal;
  if ( c == '\\' && !m_escape ){
    m_escape = true;
    return arg.off;
  }
  if ( m_escape && c == 'x' ){
    m_hex_mode = 1;
    m_escape = false;
    return arg.off;
  }
  if ( m_hex_mode ){
    if ( isxdigit(c) ){
      if ( ++m_hex_mode < 3 ){
        m_prev = c;
        return arg.off;
      }
    }
  }
  c = (unsigned char)c;
  if ( !m_shiftjis_state && ( 129 <= c && c <= 159 || 224 <= c && c <= 239 ) ){
    m_shiftjis_state = true;
    m_prev = c;
    return arg.off;
  }
  if ( m_jis_state == 0 && c == 0x1b ){
    m_jis_state = 1;
    return arg.off;
  }
  if ( m_jis_state == 1 && c == 0x24 ){
    m_jis_state = 2;
    return arg.off;
  }
  if ( m_jis_state == 2 && c == 0x42 ){
    m_jis_state = 3;
    return arg.off;
  }
  if ( m_jis_state == 3 ){
    if ( c == 0x1b ){
      m_jis_state = 5;
      return arg.off;
    }
    m_prev = c;
    m_jis_state = 4;
    return arg.off;
  }
  if ( m_jis_state == 4 )
    m_jis_state = 3;
  if ( m_jis_state == 5 && c == 0x28 ){
    m_jis_state = 6;
    return arg.off;
  }
  if ( m_jis_state == 6 && c == 0x42 ){
    m_jis_state = 0;
    return arg.off;
  }
  if ( m_euc_state == 0 && c == 0x8e ){
    m_euc_state = 1;
    return arg.off;
  }
  if ( !m_shiftjis_state && m_euc_state == 0 && 0xa1 <= c && c <= 0xfe ){
    m_prev = c;
    m_euc_state = 2;
    return arg.off;
  }

  usr* y;
  if ( m_escape ){
    ostringstream os;
    if ( m_wide )
      os << 'L';
    os << "'";
    os << '\\' << char(c) << "'";
    y = character::create(os.str());
    m_escape = false;
  }
  else if ( m_hex_mode ){
    int a = isdigit(m_prev) ? m_prev - '0' : m_prev - 'a' + 10;
    int b = isdigit(c) ? c - '0' : c - 'a' + 10;
    y = m_wide ? integer::create((wchar_typedef)(a << 4 | b)) : integer::create(char(a << 4 | b));
    m_hex_mode = false;
  }
  else if ( m_shiftjis_state ){
    assert(64 <= c && c <= 126 || 128 <= c && c <= 252);
    y = m_wide ? integer::create((wchar_typedef)(m_prev << 8 | c))
      : integer::create(char(m_prev << 8 | c));
    m_shiftjis_state = false;
  }
  else if ( m_jis_state == 3 )
    y = m_wide ? integer::create((wchar_typedef)(m_prev << 8 | c))
      : integer::create(char(m_prev << 8 | c));
  else if ( m_euc_state == 1 ){
    y = integer::create(char(c));
    m_euc_state = 0;
  }
  else if ( m_euc_state == 2 ){
    y = m_wide ? integer::create((wchar_typedef)(m_prev << 8 | c))
      : integer::create(char(m_prev << 8 | c));
    m_euc_state = 0;
  }
  else
    y = m_wide ? integer::create((wchar_typedef)c) : integer::create(char(c));
  arg.V[arg.off] = y;
  arg.nth_max = max(arg.nth_max,++arg.nth);
  arg.off_max = max(arg.off_max, arg.off += m_wide ? sizeof(wchar_typedef) : 1);
  return arg.off;
}

namespace cxx_compiler { namespace declarations { namespace initializers {
  int order(const record_type*, int);
  int member_size(const record_type*);
} } } // end of namespace initializers, declarations and cxx_compiler

int cxx_compiler::declarations::initializers::assign_special(var* y, argument* arg)
{
  using namespace std;
  const type* T = arg->T;
  pair<int, const type*> ret = T->current(arg->nth);
  T = ret.second;
  int size = T->size();
  if ( !size ){
    using namespace error::declarations::initializers;
    not_object(argument::dst);
    return 0;
  }
  typedef const array_type ARRAY;
  ARRAY* array = T->m_id == type::ARRAY ? static_cast<ARRAY*>(T) : 0;
  typedef const record_type REC;
  REC* rec = array ? 0 : static_cast<REC*>(T->unqualified());
  map<int,var*> u;
  int offset = arg->off % size;
  int nth = array ? offset / array->element_type()->size() : order(rec,offset);
  argument tmp(T,u,offset,offset,nth,nth,arg->list_pos,arg->list_len);
  clause::assign(y,&tmp);
  fill_zero(&tmp);
  for_each(u.begin(),u.end(),bind1st(ptr_fun(initializers::merge),make_pair(&arg->V,ret.first)));
  if ( tmp.nth == (array ? array->dim() : member_size(rec)) )
    arg->nth_max = max(arg->nth_max,++arg->nth), tmp.off = 0;
  arg->off_max = max(arg->off_max, arg->off = arg->nth * size + tmp.off);
  if ( tmp.not_constant )
    arg->not_constant = true;
  return arg->off;
}

int cxx_compiler::declarations::initializers::order(const record_type* rec, int y)
{
  using namespace std;
  tag* T = rec->get_tag();
  if ( T->m_kind == tag::UNION )
    return 0;

  int nth = 0;
  while ( 1 ){
    pair<int, const type*> ret = rec->current(nth);
    int x = ret.first;
    const type* T = ret.second;
    if ( y < x + T->size() || y <= x )
      break;
    ++nth;
  }
  return nth;
}

int cxx_compiler::declarations::initializers::member_size(const record_type* rec)
{
  using namespace std;
  tag* T = rec->get_tag();
  if ( T->m_kind == tag::UNION )
    return 1;
  return rec->member().size();
}

int cxx_compiler::declarations::initializers::bit_field(var* y, argument* arg)
{
  using namespace std;
  typedef const record_type REC;
  REC* rec =static_cast<REC*>(arg->T);
  usr* member = rec->member()[arg->nth];
  typedef const bit_field_type BF;
  BF* bf = static_cast<BF*>(member->m_type);
  const type* T = bf->integer_type();
  y = y->cast(T);
  usr* a = refbit::mask(bf->bit());
  y = y->bit_and(a);
  int pos = rec->position(member);
  var* p = expressions::primary::literal::integer::create(pos);
  y = y->lsh(p);
  var*& x = arg->V[arg->off];
  if ( !x ){
    x = y;
    arg->nth_max = max(arg->nth_max,++arg->nth);
    return arg->off;
  }
  usr* b = refbit::mask(bf->bit(),pos);
  x = x->bit_and(b);
  x = x->bit_or(y);
  arg->nth_max = max(arg->nth_max,++arg->nth);
  const usr::flag_t& flag = member->m_flag;
  if ( flag & usr::MSB_FIELD )
    arg->off_max = max(arg->off_max, arg->off += T->size());
  return arg->off;
}

namespace cxx_compiler { namespace declarations { namespace initializers {
  int designation(std::vector<designator::info_t*>*, clause::info_t*, argument*);
} } } // end of namespace list, initializers, declarations and cxx_compiler

int
cxx_compiler::declarations::initializers::lst::gencode(element* p,
                                                       argument* arg)
{
  using namespace std;
  if ( vector<designator::info_t*>* d = p->m_designation )
    return designation(d,p->m_clause,arg);
  else
    return ++arg->list_pos, clause::gencode(p->m_clause,arg);
}

namespace cxx_compiler { namespace declarations { namespace initializers {
  namespace designator {
    int action(info_t*, argument*);
  }
  const type* complete(const type*, int);
} } } // end of namespace list, initializers, declarations and cxx_compiler

int cxx_compiler::declarations::initializers::designation(std::vector<designator::info_t*>* des,
                                                          clause::info_t* ini,
                                                          argument* arg)
{
  using namespace std;

  map<int, var*> u1;
  argument tmp1(arg->T,u1,0,0,-1,-1,-1,-1);
  for_each(des->begin(),des->end(),bind2nd(ptr_fun(designator::action),&tmp1));
  u1.erase(u1.begin(),u1.lower_bound(arg->off_max));
  for_each(u1.begin(),u1.end(),bind1st(ptr_fun(initializers::merge),make_pair(&arg->V,0)));
  arg->nth_max = max(arg->nth_max, arg->nth = tmp1.nth + 1);
  if ( tmp1.not_constant )
    arg->not_constant = true;

  map<int, var*> u2;
  ++arg->list_pos;
  argument tmp2(tmp1.T,u2,0,0,-1,-1,arg->list_pos,arg->list_len);
  clause::gencode(ini,&tmp2);
  for_each(u2.begin(),u2.end(),bind1st(ptr_fun(initializers::merge),make_pair(&arg->V,tmp1.off)));
  arg->off_max = max(arg->off_max, arg->off = tmp1.off + tmp2.off);
  if ( tmp2.not_constant )
    arg->not_constant = true;
  
  map<int, var*> u3;
  argument tmp3(complete(arg->T,arg->off_max),u3,0,0,0,0,arg->list_pos,arg->list_len);
  fill_zero(&tmp3);
  u3.erase(u3.begin(),u3.lower_bound(arg->off_max));
  for_each(u3.begin(),u3.end(),bind1st(ptr_fun(initializers::merge),make_pair(&arg->V,0)));
  if ( tmp3.not_constant )
    arg->not_constant = true;
  
  return arg->off;
}

const cxx_compiler::type* cxx_compiler::declarations::initializers::complete(const type* T, int offset)
{
  if ( T->m_id != type::ARRAY )
    return T;
  typedef const array_type ARRAY;
  ARRAY* array = static_cast<ARRAY*>(T);
  if ( array->dim() )
    return T;
  T = array->element_type();
  int size = T->size();
  int dim = offset / size;
  if ( offset % size )
    ++dim;
  return array_type::create(T,dim);
}

namespace cxx_compiler { namespace declarations { namespace initializers { namespace designator {
  int subscripting(expressions::base*, argument*);
  int dot(usr*, argument*);
} } } } // end of namespace designator, initializers, declarations and cxx_compiler

int cxx_compiler::declarations::initializers::designator::action(info_t* p, argument* arg)
{
  return p->m_expr ? subscripting(p->m_expr,arg) : dot(p->m_usr,arg);
}

int cxx_compiler::declarations::initializers::designator::subscripting(expressions::base* expr, argument* arg)
{
  using namespace std;
  if ( arg->T->m_id != type::ARRAY ){
    using namespace error::declarations::initializers::designator;
    invalid_subscripting(parse::position,arg->T);
    return arg->off;
  }
  typedef const array_type ARRAY;
  ARRAY* array = static_cast<ARRAY*>(arg->T);
  var* cexpr = expr->gen();
  cexpr = cexpr->rvalue();
  if ( !cexpr->m_type->integer() ){
    using namespace error::declarations::initializers::designator;
    not_integer(parse::position);
    cexpr = cexpr->cast(int_type::create());
  }
  if ( !cexpr->isconstant() ){
    using namespace error::declarations::initializers::designator;
    not_constant(parse::position);
    return arg->off;
  }
  int nth = cexpr->value();
  map<int, var*> u;
  const type* T = array_type::create(array->element_type(),nth);
  argument tmp(T,u,0,0,0,0,0,0);
  fill_zero(&tmp);
  for_each(u.begin(),u.end(),bind1st(ptr_fun(initializers::merge),make_pair(&arg->V,arg->off)));
  arg->T = array->element_type();
  arg->off_max = max(arg->off_max, arg->off += tmp.off_max);
  arg->nth_max = max(arg->nth_max, arg->nth = tmp.nth_max);
  if ( tmp.not_constant )
    arg->not_constant = true;
  return arg->off;
}

int cxx_compiler::declarations::initializers::designator::dot(usr* member, argument* arg)
{
  using namespace std;
  arg->T = arg->T->unqualified();
  if ( arg->T->m_id != type::RECORD ){
    using namespace error::declarations::initializers::designator;
    invalid_dot(parse::position,arg->T);
    return arg->off;
  }
  typedef const record_type REC;
  REC* rec = static_cast<REC*>(arg->T);
  string name = member->m_name;
  pair<int, usr*> ret = rec->offset(name);
  int offset = ret.first;
  if ( !ret.second ){
    using namespace error::declarations::initializers::designator;
    not_member(member,rec,argument::dst);
    return arg->off;
  }
  member = ret.second;
  map<int, var*> u;
  argument tmp(rec,u,0,0,0,0,0,0);
  fill_zero(&tmp);
  u.erase(u.lower_bound(offset),u.end());
  for_each(u.begin(),u.end(),bind1st(ptr_fun(initializers::merge),make_pair(&arg->V,arg->off)));
  arg->T = member->m_type;
  arg->off_max = max(arg->off_max, arg->off += offset);
  arg->nth_max = max(arg->nth_max, arg->nth = order(rec,offset));
  if ( tmp.not_constant )
    arg->not_constant = true;
  return arg->off;
}

int cxx_compiler::declarations::initializers::merge(std::pair<std::map<int,var*>*,int> x, std::pair<int,var*> y)
{
  using namespace std;
  map<int,var*>& V = *x.first;
  int delta = x.second;
  int offset = y.first;
  var* v = y.second;
  V[delta+offset] = v;
  return 0;
}

int cxx_compiler::declarations::initializers::fill_zero(argument* arg)
{
  using namespace std;
  if ( arg->list_pos < 0 )
    return 0;
  if ( arg->list_pos != arg->list_len )
    return 0;
  if ( arg->off_max == arg->T->size() )
    return 0;
  if ( !arg->T->size() )
    return 0;

  while ( 1 ){
    pair<int, const type*> ret = arg->T->current(arg->nth_max);
    if ( ret.first < 0 )
      break;
    arg->off_max = max(arg->off_max, arg->off = ret.first);
    const type* T = ret.second;
    if ( T->m_id == type::BIT_FIELD ){
      typedef const bit_field_type BF;
      BF* bf = static_cast<BF*>(T);
      T = bf->integer_type();
    }
    if ( T->scalar() ){
      var* zero = expressions::primary::literal::integer::create(0);
      zero = zero->cast(T);
      arg->V[arg->off] = zero;
      arg->off_max = max(arg->off_max, arg->off += T->size());
    }
    else {
      map<int, var*> u;
      argument tmp(T,u,0,0,0,0,0,0);
      fill_zero(&tmp);
      for_each(u.begin(),u.end(),bind1st(ptr_fun(initializers::merge),make_pair(&arg->V,arg->off)));
      arg->off_max = max(arg->off_max, arg->off += tmp.off);
      if ( tmp.not_constant )
        arg->not_constant = true;
    }
    arg->nth = ++arg->nth_max;
  }
  return arg->off;
}

cxx_compiler::declarations::initializers::info_t::~info_t()
{
  using namespace std;
  delete m_clause;
  if ( m_exprs ){
    for (auto p : *m_exprs)
      delete p;
    delete m_exprs;
  }
}

cxx_compiler::declarations::initializers::clause::info_t::~info_t()
{
  using namespace std;
  delete m_expr;
  if ( m_list ){
    for (auto p : *m_list)
      delete p;
    delete m_list;
  }
}

cxx_compiler::declarations::initializers::element::~element()
{
  using namespace std;
  if ( m_designation ){
    for (auto p : *m_designation)
      delete p;
    delete m_designation;
  }
  delete m_clause;
}

cxx_compiler::declarations::initializers::designator::info_t::~info_t()
{
  delete m_expr;
  delete m_usr;
}

namespace cxx_compiler { namespace declarations { namespace initializers {
  int change_scope1(tac*, block*);
  void scalar(std::map<int, var*>::iterator, var*, block*);
  void aggregate(std::map<int, var*>::iterator, var*, block*);
} } } // end of namespace initializers, declarations ans cxx_compiler

void cxx_compiler::declarations::initializers::initialize_code(with_initial* x)
{
  using namespace std;
  assert(scope::current == &scope::root);
  string name = "initialize." + x->m_name;
  vector<const type*> dummy;
  const func_type* ft = func_type::create(void_type::create(),dummy);
  usr::flag_t flag = usr::flag_t(usr::FUNCTION | usr::INITIALIZE_FUNCTION);
  usr* func = new usr(name,ft,flag,file_t());
  scope* param = new scope(scope::PARAM);
  param->m_parent = &scope::root;
  scope::root.m_children.push_back(param);
  block* body = new block;
  body->m_parent = param;
  param->m_children.push_back(body);
  fundef::current = new fundef(func,param);

  for_each(code.begin(),code.end(),bind2nd(ptr_fun(change_scope1),body));

  map<int, var*>& value = x->m_value;
  typedef map<int, var*>::iterator IT;
  if ( x->m_type->scalar() ){
    if ( value.size() == 1 ){  // This holds if program is correct.
      IT it = value.find(0);
      assert(it != value.end());
      scalar(it,x,body);
    }
  }
  else {
    for ( IT it = value.begin() ; it != value.end() ; ++it )
      aggregate(it,x,body);
  }

  if ( !error::counter )
    optimize::action(fundef::current, code);

  if ( cmdline::output_medium ){
    usr* u = fundef::current->m_usr;
    scope* org = scope::current;
    scope::current = &scope::root;
    cout << dump::names::ref(u) << ":\n";
    scope::current = org;
    typedef vector<tac*>::const_iterator IT;
    for ( IT p = code.begin() ; p != code.end() ; ++p )
      cout << '\t', dump::tac(cout,*p), cout << '\n';
    cout << '\n';
    dump::scope();
  }
  if ( !error::counter ){
    if ( generator::generate ){
      generator::interface_t tmp = {
        &scope::root,
        fundef::current,
        &code
      };
      generator::generate(&tmp);
    }
  }
  delete func;
  delete fundef::current;
  fundef::current = 0;
  destroy();
}

namespace cxx_compiler { namespace declarations { namespace initializers {
  void change_scope2(var*, block*);
} } } // end of namespace initializers, declarations ans cxx_compiler

int cxx_compiler::declarations::initializers::change_scope1(tac* p, block* b)
{
  if ( var* x = p->x )
    change_scope2(x,b);
  if ( var* y = p->y )
    change_scope2(y,b);
  if ( var* z = p->z )
    change_scope2(z,b);
  return 0;
}

void cxx_compiler::declarations::initializers::change_scope2(var* x, block* b)
{
  using namespace std;
  vector<var*>& v = garbage;
  vector<var*>::iterator p = find(v.begin(),v.end(),x);
  if ( p != v.end() ){
    v.erase(p);
    x->m_scope = b;
    b->m_vars.push_back(x);
  }
}

void cxx_compiler::declarations::initializers::scalar(std::map<int, var*>::iterator it,
  var* x, block* body)
{
  using namespace std;
  var* y = it->second;
  assert(!y->isconstant(true));
  code.push_back(new assign3ac(x,y));
  const type* T = y->m_type;
  var* zero = expressions::primary::literal::integer::create(0);
  it->second = zero->cast(T);
}

void
cxx_compiler::declarations::initializers::aggregate(std::map<int, var*>::iterator it,
  var* x, block* body)
{
  using namespace std;
  var* y = it->second;
  if ( y->isconstant(true) )
    return;
  int offset = it->first;
  var* off = expressions::primary::literal::integer::create(offset);
  code.push_back(new loff3ac(x,off,y));
  const type* T = y->m_type;
  var* zero = expressions::primary::literal::integer::create(0);
  if ( T->scalar() )
    it->second = zero->cast(T);
  else
    it->second = zero->cast(char_type::create());
}
