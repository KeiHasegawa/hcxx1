#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

namespace cxx_compiler {
  namespace declarations {
    namespace initializers {
      using namespace std;
      struct gendata {
        vector<tac*> m_code;
        map<int, var*> m_value;
      };
      map<usr*, gendata> table;
      usr* argument::dst;
      namespace clause {
        extern int gencode(info_t*, argument*);
      }  // end of namespace clause
      extern int expr_list(vector<expressions::base*>*, argument*);
    }  // end of namespace initializers
  }  // end of namespace declarations
}  // end of namespace cxx_compiler

void cxx_compiler::declarations::initializers::action(var* v, info_t* i)
{
  using namespace std;
  assert(v->usr_cast());
  usr* u = static_cast<usr*>(v);
  auto_ptr<info_t> sweeper(i);
  argument::dst = u;
  usr::flag_t flag = u->m_flag;
  with_initial* p = 0;
  if (flag & usr::WITH_INI)
    p = static_cast<with_initial*>(u);
  argument arg(u->m_type,p ? p->m_value : table[u].m_value,0,0,-1,-1,-1,-1);
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
    gendata& data = table[u];
    copy(code.begin()+n,code.begin()+m,back_inserter(data.m_code));
    code.resize(n);
  }
  const type* T = u->m_type;
  if ( T->m_id == type::ARRAY ){
    typedef const array_type ARRAY;
    ARRAY* array = static_cast<ARRAY*>(T);
    if (!array->dim()) {
      T = array->element_type();
      int n = arg.off_max;
      int m = T->size();
      if (m)
        u->m_type = array_type::create(T,(n + m - 1)/ m);
    }
  }
  if (p) {
    if (arg.not_constant)
      initialize_code(p);
  }
  parse::identifier::mode = parse::identifier::new_obj;
}

namespace cxx_compiler {
  namespace declarations {
    namespace initializers {
      extern void gencode(usr*);
    }  // end of namespace initializers
  }  // end of namespace declarations
}  // end of namespace cxx_compiler

void cxx_compiler::usr::initialize()
{
  declarations::initializers::gencode(this);
  if (scope::current->m_id == scope::BLOCK) {
    block* b = static_cast<block*>(scope::current);
    if ( m_type->variably_modified() ){
      if ( m_flag & usr::EXTERN ){
        using namespace error::declarations::declarators::vm;
        invalid_linkage(this);
        m_flag = usr::flag_t(m_flag & ~usr::EXTERN);
      }
    }
  }
  using namespace declarations::declarators::array;
  variable_length::allocate(this);
}

bool cxx_compiler::array_of_tor(const array_type* at, bool ctor)
{
  const type* T = at->element_type();
  if (T->m_id == type::ARRAY) {
    typedef const array_type AT;
    AT* at = static_cast<AT*>(T);
    return array_of_tor(at, ctor);
  }
  if (T->m_id != type::RECORD)
    return false;
  typedef const record_type REC;
  REC* rec = static_cast<REC*>(T);
  tag* ptr = rec->get_tag();
  string name = ptr->m_name;
  if (!ctor)
    name = '~' + name;
  const map<string, vector<usr*> >& usrs = ptr->m_usrs;
  return usrs.find(name) != usrs.end();
}

void
cxx_compiler::ctor_dtor_common(var* v, const array_type* at, void (*pf)(var*),
			       bool ctor)
{
  const type* T = at->element_type();
  int size = T->size();
  if (!size)
    return;
  int dim = at->dim();
  for (int i = 0 ; i != dim ; ++i) {
    const pointer_type* pt = pointer_type::create(T);
    int offset = ctor ? (size * i) : (dim - i - 1) * size;
    var* t0 = new ref(pt);
    if (scope::current->m_id == scope::BLOCK) {
      block* b = static_cast<block*>(scope::current);
      b->m_vars.push_back(t0);
    }
    else
      garbage.push_back(t0);
    code.push_back(new addr3ac(t0, v));
    if (offset) {
      using namespace expressions::primary::literal;
      var* off = integer::create(offset);
      var* t1 = new ref(pt);
      if (scope::current->m_id == scope::BLOCK) {
	block* b = static_cast<block*>(scope::current);
	b->m_vars.push_back(t1);
      }
      else
	garbage.push_back(t1);
      code.push_back(new add3ac(t1, t0, off));
      t0 = t1;
    }
    pf(t0);
  }
}

namespace cxx_compiler {
  namespace declarations {
    namespace initializers {
      namespace reference_impl {
        void constant_case(usr* u, var* v)
        {
          var* tmp = new var(u->m_type);
          if (scope::current->m_id == scope::BLOCK) {
            block* b = static_cast<block*>(scope::current);
            b->m_vars.push_back(tmp);
          }
          else
            garbage.push_back(tmp);
          code.push_back(new assign3ac(tmp, v));
          v = tmp;
          code.push_back(new addr3ac(u, v));
        }
        void not_constant(usr* u, var* v)
        {
          addrof* addr = v->addrof_cast();
          assert(addr);
          var* ref = addr->m_ref;
          if (int offset = addr->m_offset) {
            using namespace expressions::primary::literal;
            var* off = integer::create(offset);
            var* tmp = new var(u->m_type);
            if (scope::current->m_id == scope::BLOCK) {
              block* b = static_cast<block*>(scope::current);
              b->m_vars.push_back(tmp);
            }
            else
              garbage.push_back(tmp);
            code.push_back(new addr3ac(tmp, ref));
            code.push_back(new add3ac(u, tmp, off));
          }
          else
            code.push_back(new addr3ac(u, ref));
        }
      } // end of namespace reference_impl
      inline void reference_case(usr* u, var* v)
      {
        if (v->isconstant())
          reference_impl::constant_case(u, v);
        else
          reference_impl::not_constant(u, v);
      }
    } // end of nmeaspace initializers
  } // end of nmeaspace declarations
} // end of nmeaspace cxx_compiler

void cxx_compiler::declarations::initializers::gencode(usr* u)
{
  using namespace std;
  using namespace declarations::declarators::function::definition;
  using namespace static_inline;
  usr::flag_t flag = u->m_flag;
  if (flag & usr::TYPEDEF)
    return;

  typedef map<usr*, gendata>::iterator IT;
  IT p = table.find(u);
  if (p == table.end())
    return call_default_ctor(u);

  struct sweeper {
    IT m_it;
    sweeper(IT it) : m_it(it) {}
    ~sweeper(){ table.erase(m_it); }
  } sweeper(p);

  gendata& data = p->second;
  vector<tac*>& c = data.m_code;
  copy(c.begin(),c.end(),back_inserter(code));
  map<int,var*>& value = data.m_value;
  const type* Tx = u->m_type;
  if (value.size() != 1) {
    for_each(value.begin(),value.end(),bind1st(ptr_fun(gen_loff),u));
    return;
  }

  var* v = value[0];
  const type* Ty = v->m_type;
  if (Tx->scalar() != Ty->scalar()) {
    for_each(value.begin(),value.end(),bind1st(ptr_fun(gen_loff),u));
    return;
  }

  Tx = Tx->unqualified();
  Ty = Ty->unqualified();
  if (Tx->m_id == type::REFERENCE && Ty->m_id != type::REFERENCE)
    return reference_case(u, v);

  if (Tx->m_id == type::RECORD) {
    assert(compatible(Tx, Ty));
    tag* ptr = Tx->get_tag();
    string tgn = ptr->m_name;
    const map<string, vector<usr*> >& usrs = ptr->m_usrs;
    typedef map<string, vector<usr*> >::const_iterator IT;
    IT p = usrs.find(tgn);
    if (p != usrs.end()) {
      const vector<usr*>& vu = p->second;
      typedef vector<usr*>::const_iterator IT;
      IT q = find_if(begin(vu), end(vu),
		     bind2nd(ptr_fun(canbe_copy_ctor), ptr));
      if (q != end(vu)) {
	usr* copy_ctor = *q;
	const type* T = pointer_type::create(Tx);
	var* t0 = new var(T);
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
	code.push_back(new addr3ac(t0, u));
	code.push_back(new addr3ac(t1, v));
	code.push_back(new param3ac(t0));
	code.push_back(new param3ac(t1));
	usr::flag_t flag = copy_ctor->m_flag;	
	if (flag & usr::HAS_DEFAULT_ARG) {
	  using namespace declarations::declarators::function;
	  typedef map<usr*, vector<var*> >::const_iterator IT;
	  IT p = default_arg_table.find(copy_ctor);
	  assert(p != default_arg_table.end());
	  const vector<var*>& v = p->second;
	  assert(!v.empty());
	  for_each(begin(v)+1, end(v), [](var* v)
		   {
		     if (v)
		       code.push_back(new param3ac(v));
		   });
	}
	code.push_back(new call3ac(0, copy_ctor));
	if (!error::counter && !cmdline::no_inline_sub) {
	  if (flag & usr::INLINE) {
	    using namespace declarations::declarators::function;
	    using namespace definition::static_inline;
	    skip::table_t::const_iterator p = skip::stbl.find(copy_ctor);
	    if (p != skip::stbl.end())
	      substitute(code, code.size()-1, p->second);
	  }
	}
	return;
      }
    }
  }

  if (compatible(Tx, Ty)) {
    code.push_back(new assign3ac(u,v));
    return;
  }

  using namespace expressions::primary::literal;
  var* zero = integer::create(0);
  code.push_back(new loff3ac(u, zero, v));
}

namespace cxx_compiler {
  namespace declarations {
    namespace initializers {
      namespace clause {
        extern int assign(var*, argument*);
        extern int lsting(std::vector<element*>*, argument*);
      }  // end of namespace clause
    }  // end of namespace initializers
  }  // end of namespace declarations
}  // end of namespace cxx_compiler

int cxx_compiler::declarations::initializers::clause::
gencode(info_t* c, argument* arg)
{
  return c->m_expr ? assign(c->m_expr->gen(),arg) : lsting(c->m_list,arg);
}

int cxx_compiler::declarations::initializers::
expr_list(std::vector<expressions::base*>* exprs, argument* arg)
{
  using namespace std;
  assert(!exprs->empty());
  const type* T = arg->T;
  if (T->scalar()) {
    if (exprs->size() == 1) {
      expressions::base* expr = (*exprs)[0];
      return clause::assign(expr->gen(),arg);
    }
    using namespace error::declarations::initializers;
    exceed(argument::dst);
    return 0;
  }

  T = T->unqualified();
  if (T->m_id != type::RECORD) {
    error::not_implemented();
    return 0;
  }

  typedef const record_type REC;
  REC* rec = static_cast<REC*>(T);
  tag* ptr = rec->get_tag();
  string name = ptr->m_name;
  typedef map<string, vector<usr*> >::const_iterator IT;
  IT p = ptr->m_usrs.find(name);
  if (p == ptr->m_usrs.end()) {
    using namespace error::declarations::initializers;
    no_ctor(argument::dst);
    return 0;
  }

  vector<var*> res;
  transform(begin(*exprs), end(*exprs), back_inserter(res),
            mem_fun(&expressions::base::gen));

  const vector<usr*>& v = p->second;
  usr* ctor = v.back();
  usr::flag_t flag = ctor->m_flag;
  if (flag & usr::OVERLOAD) {
    overload* ovl = static_cast<overload*>(ctor);
    ovl->m_obj = argument::dst;
    int n = code.size();
    ovl->call(&res);
    vector<tac*>& c = table[argument::dst].m_code;
    copy(begin(code)+n, end(code), back_inserter(c));
    code.resize(n);
    return 0;
  }

  const type* T2 = ctor->m_type;
  assert(T2->m_id == type::FUNC);
  typedef const func_type FT;
  FT* ft = static_cast<FT*>(T2);
  int n = code.size();
  call_impl::wrapper(ctor, &res, argument::dst);
  vector<tac*>& c = table[argument::dst].m_code;
  copy(begin(code)+n, end(code), back_inserter(c));
  code.resize(n);

  return 0;
}

void cxx_compiler::declarations::initializers::
gen_loff(usr* dst, std::pair<int,var*> p)
{
  using namespace expressions::primary::literal;
  int offset = p.first;
  var* off = integer::create(offset);
  var* src = p.second;
  code.push_back(new loff3ac(dst,off,src));
}

namespace cxx_compiler {
  namespace declarations {
    namespace initializers {
      namespace char_array_string {
        extern int action(var*, argument*);
      }  // end of namespace char_array_string
      using namespace std;
      extern int assign_special(var*, argument*);
      extern int merge(pair<map<int,var*>*,int>, pair<int,var*>);
      extern int bit_field(var*, argument*);
    }  // end of namespace initializers
  }  // end of namespace declarations
}  // end of namespace cxx_compiler

int cxx_compiler::declarations::initializers::clause::
assign(var* y, argument* arg)
{
  using namespace std;
  const type* T = arg->T;
  const type* U = T->unqualified();
  if (U->m_id != type::REFERENCE)
    y = y->rvalue();
  if (duration::_static(argument::dst) && !y->isconstant(true)
      && U->m_id != type::REFERENCE)
    arg->not_constant = true;

  if ( int r = char_array_string::action(y,arg) )
    return r;
  if ( arg->nth >= 0 ){
    pair<int, const type*> ret = T->current(arg->nth);
    if ( ret.first < 0 ){
      using namespace error::declarations::initializers;
      exceed(argument::dst);
      return arg->off;
    }
    T = ret.second;
    if (!T->scalar() && y->m_type->scalar()) {
      bool discard = false;
      if (!expressions::assignment::valid(T, y, &discard, true))
	return assign_special(y,arg);
    }
    arg->off_max = max(arg->off_max, arg->off = ret.first);
  }

  if (U->m_id == type::REFERENCE) {
    typedef const reference_type RT;
    RT* rt = static_cast<RT*>(U);
    const type* Tx = rt->referenced_type();
    const type* Ty = y->m_type; 
    int cvr_x = 0, cvr_y = 0;
    Tx->unqualified(&cvr_x);
    Ty->unqualified(&cvr_y);
    if (!expressions::assignment::include(cvr_x, cvr_y)) {
      using namespace error::declarations::initializers;
      invalid_assign(parse::position,argument::dst,true);
      return arg->off;
    }
  }
  bool discard = false;
  T = expressions::assignment::valid(T, y, &discard, true);
  if (!T) {
    using namespace error::declarations::initializers;
    invalid_assign(parse::position,argument::dst,discard);
    return arg->off;
  }

  typedef const bit_field_type BF;
  if ( T->m_id == type::BIT_FIELD )
    return bit_field(y,arg);
  y = T->aggregate() ? aggregate_conv(T, y) : y->cast(T);
  if (y->addrof_cast()) {
    vector<var*>& v = garbage;
    vector<var*>::reverse_iterator p = find(v.rbegin(),v.rend(),y);
    if (p != v.rend())
      v.erase(p.base()-1);
  }
  if (U->m_id == type::REFERENCE) {
    typedef const reference_type RT;
    RT* rt = static_cast<RT*>(U);
    const type* R = rt->referenced_type();
    if (y->isconstant()) {
      if (scope::current->m_id == scope::BLOCK) {
        block* b = static_cast<block*>(scope::current);
        var* tmp = new var(y->m_type);
        b->m_vars.push_back(tmp);
        code.push_back(new assign3ac(tmp, y));
        y = tmp;
      }
      else {
        string name = new_name(".tmp");
        with_initial* tmp = new with_initial(name, y->m_type, parse::position);
        scope::root.m_usrs[name].push_back(tmp);
        tmp->m_value[0] = y;
        y = tmp;
      }
    }
    const type* Ty = y->m_type;
    const type* pt = pointer_type::create(Ty);
    Ty = Ty->unqualified();
    if (Ty->m_id == type::REFERENCE)
      arg->V[arg->off] = y;
    else
      arg->V[arg->off] = new addrof(pt, y, 0);
  }
  else
    arg->V[arg->off] = y;
  arg->nth_max = max(arg->nth_max,++arg->nth);
  arg->off_max = max(arg->off_max, arg->off += T->size());
  return arg->off;
}

int
cxx_compiler::declarations::initializers::clause::
lsting(std::vector<element*>* v, argument* arg)
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
  for_each(u.begin(),u.end(),
           bind1st(ptr_fun(initializers::merge), make_pair(&arg->V,arg->off)));
  arg->nth_max = max(arg->nth_max, ++arg->nth);
  arg->off_max = max(arg->off_max, arg->off += tmp.off_max);
  if ( tmp.not_constant )
    arg->not_constant = true;
  return arg->off;
}

namespace cxx_compiler {
  namespace declarations {
    namespace initializers {
      namespace char_array_string {
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
      }  // end of namespace char_array_string
    }  // end of namespace initializers
  }  // end of namespace declarations
}  // end of namespace cxx_compiler

int cxx_compiler::declarations::initializers::char_array_string::
action(var* y, argument* arg)
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
  for_each(u.begin(),u.end(),
           bind1st(ptr_fun(merge), make_pair(&arg->V,arg->off)));
  arg->nth_max = max(arg->nth_max,
                     brace ? arg->nth += tmp.nth_max : ++arg->nth);
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

namespace cxx_compiler {
  namespace declarations {
    namespace initializers {
      namespace char_array_string {
        const array_type* char_array(const type*);
      }  // end of namespace char_array_string
    }  // end of namespace initializers
  }  // end of namespace declarations
}  // end of namespace cxx_compiler

const cxx_compiler::array_type*
cxx_compiler::declarations::initializers::char_array_string::
char_array(const type* T, int nth, bool* brace)
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
cxx_compiler::declarations::initializers::char_array_string::
char_array(const type* T)
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

int cxx_compiler::declarations::initializers::char_array_string::eval::
operator()(int c)
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

namespace cxx_compiler {
  namespace declarations {
    namespace initializers {
      int order(const record_type*, int);
      int member_size(const record_type*);
    }  // end of namespace initializers
  }  // end of namespace declarations
}  // end of namespace cxx_compiler

int cxx_compiler::declarations::initializers::
assign_special(var* y, argument* arg)
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
  for_each(u.begin(),u.end(), bind1st(ptr_fun(initializers::merge),
                                      make_pair(&arg->V,ret.first)));
  if ( tmp.nth == (array ? array->dim() : member_size(rec)) )
    arg->nth_max = max(arg->nth_max,++arg->nth), tmp.off = 0;
  arg->off_max = max(arg->off_max, arg->off = arg->nth * size + tmp.off);
  if ( tmp.not_constant )
    arg->not_constant = true;
  return arg->off;
}

int
cxx_compiler::declarations::initializers::order(const record_type* rec, int y)
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

int
cxx_compiler::declarations::initializers::member_size(const record_type* rec)
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
  var* a = refbit::mask(bf->bit());
  conversion::arithmetic::gen(&y, &a);
  y = y->bit_and(a);
  int pos = rec->position(member);
  var* p = expressions::primary::literal::integer::create(pos);
  conversion::arithmetic::gen(&y, &p);
  y = y->lsh(p);
  var*& x = arg->V[arg->off];
  if ( !x ){
    x = y;
    arg->nth_max = max(arg->nth_max,++arg->nth);
    return arg->off;
  }
  var* b = refbit::mask(bf->bit(),pos);
  conversion::arithmetic::gen(&x, &b);
  x = x->bit_and(b);
  conversion::arithmetic::gen(&x, &y);
  x = x->bit_or(y);
  arg->nth_max = max(arg->nth_max,++arg->nth);
  const usr::flag_t& flag = member->m_flag;
  if ( flag & usr::MSB_FIELD )
    arg->off_max = max(arg->off_max, arg->off += T->size());
  return arg->off;
}

namespace cxx_compiler {
  namespace declarations {
    namespace initializers {
      using namespace std;
      int designation(vector<designator::info_t*>*, clause::info_t*,
                      argument*);
    }  // end of namespace initializers
  }  // end of namespace declarations
}  // end of namespace cxx_compiler

int
cxx_compiler::declarations::initializers::lst::
gencode(element* p, argument* arg)
{
  using namespace std;
  if ( vector<designator::info_t*>* d = p->m_designation )
    return designation(d,p->m_clause,arg);
  else
    return ++arg->list_pos, clause::gencode(p->m_clause,arg);
}

namespace cxx_compiler {
  namespace declarations {
    namespace initializers {
      namespace designator {
        int action(info_t*, argument*);
      }  // end of namespace designator
      const type* complete(const type*, int);
    }  // end of namespace initializers
  }  // end of namespace declarations
}  // end of namespace cxx_compiler

int cxx_compiler::declarations::initializers::
designation(std::vector<designator::info_t*>* des, clause::info_t* ini,
            argument* arg)
{
  using namespace std;

  map<int, var*> u1;
  argument tmp1(arg->T,u1,0,0,-1,-1,-1,-1);
  for_each(des->begin(),des->end(),bind2nd(ptr_fun(designator::action),&tmp1));
  u1.erase(u1.begin(),u1.lower_bound(arg->off_max));
  for_each(u1.begin(),u1.end(),
           bind1st(ptr_fun(initializers::merge),make_pair(&arg->V,0)));
  arg->nth_max = max(arg->nth_max, arg->nth = tmp1.nth + 1);
  if ( tmp1.not_constant )
    arg->not_constant = true;

  map<int, var*> u2;
  ++arg->list_pos;
  argument tmp2(tmp1.T,u2,0,0,-1,-1,arg->list_pos,arg->list_len);
  clause::gencode(ini,&tmp2);
  for_each(u2.begin(),u2.end(),
           bind1st(ptr_fun(initializers::merge),make_pair(&arg->V,tmp1.off)));
  arg->off_max = max(arg->off_max, arg->off = tmp1.off + tmp2.off);
  if ( tmp2.not_constant )
    arg->not_constant = true;
  
  map<int, var*> u3;
  const type* T = complete(arg->T,arg->off_max);
  argument tmp3(T,u3,0,0,0,0,arg->list_pos,arg->list_len);
  fill_zero(&tmp3);
  u3.erase(u3.begin(),u3.lower_bound(arg->off_max));
  for_each(u3.begin(),u3.end(),
           bind1st(ptr_fun(initializers::merge),make_pair(&arg->V,0)));
  if ( tmp3.not_constant )
    arg->not_constant = true;
  
  return arg->off;
}

const cxx_compiler::type*
cxx_compiler::declarations::initializers::complete(const type* T, int offset)
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

namespace cxx_compiler {
  namespace declarations {
    namespace initializers {
      namespace designator {
        int subscripting(expressions::base*, argument*);
        int dot(usr*, argument*);
      }  // end of namespace designator
    }  // end of namespace initializers
  }  // end of namespace declarations
}  // end of namespace cxx_compiler

int cxx_compiler::declarations::initializers::designator::
action(info_t* p, argument* arg)
{
  return p->m_expr ? subscripting(p->m_expr,arg) : dot(p->m_usr,arg);
}

int cxx_compiler::declarations::initializers::designator::
subscripting(expressions::base* expr, argument* arg)
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
  for_each(u.begin(),u.end(),
           bind1st(ptr_fun(initializers::merge),make_pair(&arg->V,arg->off)));
  arg->T = array->element_type();
  arg->off_max = max(arg->off_max, arg->off += tmp.off_max);
  arg->nth_max = max(arg->nth_max, arg->nth = tmp.nth_max);
  if ( tmp.not_constant )
    arg->not_constant = true;
  return arg->off;
}

int cxx_compiler::declarations::initializers::designator::
dot(usr* member, argument* arg)
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
  for_each(u.begin(),u.end(),
           bind1st(ptr_fun(initializers::merge),make_pair(&arg->V,arg->off)));
  arg->T = member->m_type;
  arg->off_max = max(arg->off_max, arg->off += offset);
  arg->nth_max = max(arg->nth_max, arg->nth = order(rec,offset));
  if ( tmp.not_constant )
    arg->not_constant = true;
  return arg->off;
}

int cxx_compiler::declarations::initializers::
merge(std::pair<std::map<int,var*>*,int> x, std::pair<int,var*> y)
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
      for_each(u.begin(),u.end(), bind1st(ptr_fun(initializers::merge),
                                          make_pair(&arg->V,arg->off)));
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

namespace cxx_compiler {
  namespace declarations {
    namespace initializers {
      void change_scope1(tac*, block*);
      void scalar(std::map<int, var*>::iterator, var*, block*);
      void aggregate(std::map<int, var*>::iterator, var*, block*);
      inline void for_with_initial(with_initial* x, block* body)
      {
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
      }
      void common(usr*, bool);
    }  // end of namespace initializers
  }  // end of namespace declarations
}  // end of namespace cxx_compiler

void cxx_compiler::declarations::initializers::initialize_code(with_initial* x)
{
  common(x, true);
}

void cxx_compiler::initialize_ctor_code(usr* u)
{
  declarations::initializers::common(u, true);
}

void cxx_compiler::terminate_dtor_code(usr* u)
{
  declarations::initializers::common(u, false);
}

void cxx_compiler::declarations::initializers::common(usr* u, bool ini)
{
  using namespace std;
  assert(scope::current == &scope::root);
  string name = ini ? "initialize." : "terminate.";
  name += u->m_name;
  vector<const type*> dummy;
  const func_type* ft = func_type::create(void_type::create(),dummy);
  usr::flag_t flag = usr::flag_t(usr::FUNCTION | usr::STATIC);
  usr::flag2_t flag2 =
    ini ? usr::INITIALIZE_FUNCTION : usr::TERMINATE_FUNCTION;
  flag2 = usr::flag2_t(flag2 | usr::GENED_BY_COMP);
  usr* func = new usr(name,ft,flag,file_t(),flag2);
  scope* param = new scope(scope::PARAM);
  using namespace class_or_namespace_name;
  assert(!before.empty());
  assert(before.back() == param);
  before.pop_back();
  param->m_parent = &scope::root;
  scope::root.m_children.push_back(param);
  block* body = new block;
  assert(!before.empty());
  assert(before.back() == body);
  before.pop_back();
  body->m_parent = param;
  param->m_children.push_back(body);
  fundef::current = new fundef(func,param);

  if (u->m_flag & usr::WITH_INI) {
    for_each(code.begin(),code.end(),bind2nd(ptr_fun(change_scope1),body));
    with_initial* p = static_cast<with_initial*>(u);
    for_with_initial(p, body);
  }
  else {
    assert(code.empty());
    assert(is_external_declaration(u));
    ini ? assert(must_call_default_ctor(u)) : assert(must_call_dtor(u));
    scope* org = scope::current;
    scope::current = body;
    ini ? call_default_ctor(u) : call_dtor(u);
    scope::current = org;
  }

  using namespace declarations::declarators;
  function::definition::action(fundef::current, code);
  delete fundef::current;
  fundef::current = 0;
  destroy();
}

namespace cxx_compiler {
  namespace declarations {
    namespace initializers {
      void change_scope2(var*, block*);
    }  // end of namespace initializers
  }  // end of namespace declarations
}  // end of namespace cxx_compiler

void cxx_compiler::declarations::initializers::change_scope1(tac* p, block* b)
{
  if ( var* x = p->x )
    change_scope2(x,b);
  if ( var* y = p->y )
    change_scope2(y,b);
  if ( var* z = p->z )
    change_scope2(z,b);
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

void cxx_compiler::declarations::initializers::
scalar(std::map<int, var*>::iterator it, var* x, block* body)
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
cxx_compiler::declarations::initializers::
aggregate(std::map<int, var*>::iterator it, var* x, block* body)
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
