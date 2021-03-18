#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

namespace cxx_compiler { namespace dump {
  void usrsx(std::pair<std::string, std::vector<usr*> >, int);
  void tagx(std::pair<std::string, tag*>, int);
  void varx(var*, int);
} } // end of namespace dump and cxx_compiler

void cxx_compiler::dump::scopex(scope* ptr, int ntab)
{
  using namespace std;
  const map<string, vector<usr*> >& usrs = ptr->m_usrs;
  for_each(usrs.begin(),usrs.end(),bind2nd(ptr_fun(usrsx),ntab));
  const map<string, tag*>& tags = ptr->m_tags;
  for_each(tags.begin(),tags.end(),bind2nd(ptr_fun(tagx),ntab));
  const vector<scope*>& children = ptr->m_children;
  if ( ptr->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(ptr);
    vector<var*>& vars = b->m_vars;
    for_each(vars.begin(),vars.end(),bind2nd(ptr_fun(varx),ntab));
  }

  for (auto p : children) {
    scope::id_t id = p->m_id;
    if (id != scope::TAG && id != scope::NAMESPACE)
      scopex(p, ntab+1);
  }
}

namespace cxx_compiler { namespace dump {
  void usrx(const usr*, int);
} } // end of namespace dump and cxx_compiler

void cxx_compiler::dump::
usrsx(std::pair<std::string, std::vector<usr*> > p, int ntab)
{
  using namespace std;
  const vector<usr*>& v = p.second;
  for_each(v.begin(),v.end(),bind2nd(ptr_fun(usrx),ntab));
}

namespace cxx_compiler { namespace dump {
  std::string initial(const std::pair<int, var*>&);
} } // end of namespace dump and cxx_compiler

void cxx_compiler::dump::usrx(const usr* u, int ntab)
{
  using namespace std;
  usr::flag_t flag = u->m_flag;
  if (flag & usr::OVERLOAD)
    return;
  int n = ntab;
  while ( n-- ) cout << '\t';
  if (flag) {
    string s = usr::keyword(flag);
    if ( !s.empty() )
      cout << s << ' ';
  }
  string name = names::ref(const_cast<usr*>(u));
  usr::flag2_t flag2 = u->m_flag2;
  if (flag2 & usr::TEMPLATE) {
    cout << "template " << name << '\n';
    return;
  }
  if (flag2 & usr::PARTIAL_ORDERING)
    return;
  if (flag2 & usr::ALIAS) {
    cout << "alias " << name << '\n';
    return;
  }
  const type* T = u->m_type;
  if (!T) {
    assert(flag & usr::NAMESPACE);
    cout << name << '\n';
    const name_space* ns = static_cast<const name_space*>(u);
    scope* org = scope::current;
    scope::current = const_cast<name_space*>(ns);
    dump::scopex(scope::current,ntab+1);
    scope::current = org;
    return;
  }
  T->decl(cout,name);
  if (flag & usr::WITH_INI) {
    const with_initial* p = static_cast<const with_initial*>(u);
    cout << '\t';
    const map<int, var*>& v = p->m_value;
    transform(v.begin(),v.end(),ostream_iterator<string>(cout,","),initial);
  }
  cout << '\n';
}

std::string cxx_compiler::dump::initial(const std::pair<int,cxx_compiler::var*>& p)
{
  using namespace std;
  ostringstream os;
  os << '(' << p.first << ',';
  var* v = p.second;
  addrof* addr = v->addrof_cast();
  if (addr) {
    os << "addrof(";
    v = addr->m_ref;
  }
  assert(v->usr_cast());
  usr* u = static_cast<usr*>(v);
  os << names::ref(u);
  if (addr) {
    os << ')';
    if ( int offset = addr->m_offset )
      os << '+' << offset;
  }
  os << ')';
  return os.str();
}

void cxx_compiler::dump::
tagx(std::pair<std::string, tag*> p, int ntab)
{
  using namespace std;
  tag* ptr = p.second;
  int n = ntab;
  while ( n-- ) cout << '\t';
  tag::flag_t flag = ptr->m_flag;
  if (flag & tag::TEMPLATE)
    cout << "template ";
  if (flag & tag::ALIAS) {
    string s = p.first;
    cout << s << " alias ";
  }
  cout << tag::keyword(ptr->m_kind);
  string name = ptr->m_name;
  if ( !name.empty() )
    cout << ' ' << name;
  cout << '\n';
  if (flag & tag::TEMPLATE)
    return;
  scope* org = scope::current;
  scope::current = ptr;
  dump::scopex(ptr, ntab+1);
  scope::current = org;
}

void cxx_compiler::dump::varx(var* v, int ntab)
{
  using namespace std;
  while ( ntab-- ) cout << '\t';
  string name = dump::names::ref(v);
  const type* T = v->m_type;
  T->decl(cout,name);
  cout << '\n';
}

namespace cxx_compiler { namespace dump {
  using namespace std;
  struct table_t : map<tac::id_t, void (*)(ostream&, const tac*)> {
    table_t();
  } table;
} } // end of namespace dump and cxx_compiler

void cxx_compiler::dump::tacx(std::ostream& os, const tac* ptr)
{
  table[ptr->m_id](os,ptr);
}

namespace cxx_compiler { namespace dump {
  using namespace std;
  void assign(ostream&, const tac*);
  void mul(ostream&, const tac*);
  void div(ostream&, const tac*);
  void mod(ostream&, const tac*);
  void add(ostream&, const tac*);
  void sub(ostream&, const tac*);
  void lsh(ostream&, const tac*);
  void rsh(ostream&, const tac*);
  void bit_and(ostream&, const tac*);
  void bit_or(ostream&, const tac*);
  void bit_xor(ostream&, const tac*);
  void param(ostream&, const tac*);
  void call(ostream&, const tac*);
  void ret(ostream&, const tac*);
  void addr(ostream&, const tac*);
  void invraddr(ostream&, const tac*);
  void invladdr(ostream&, const tac*);
  void uminus(ostream&, const tac*);
  void tilde(ostream&, const tac*);
  void cast(ostream&, const tac*);
  void go(ostream&, const tac*);
  void to(ostream&, const tac*);
  void loff(ostream&, const tac*);
  void roff(ostream&, const tac*);
  void _alloca_(ostream&, const tac*);
  void _asm_(ostream&, const tac*);
  void _va_start(ostream&, const tac*);
  void _va_arg(ostream&, const tac*);
  void _va_end(ostream&, const tac*);
  void alloce(ostream&, const tac*);
  void throwe(ostream&, const tac*);
  void rethrow(ostream&, const tac*);
  void try_begin(ostream&, const tac*);
  void try_end(ostream&, const tac*);
  void here(ostream&, const tac*);
  void here_reason(ostream&, const tac*);
  void here_info(ostream&, const tac*);
  void there(ostream&, const tac*);
  void unwind_resume(ostream&, const tac*);
  void catch_begin(ostream&, const tac*);
  void catch_end(ostream&, const tac*);
  void dcast(ostream&, const tac*);
} } // end of namespace dump and cxx_compiler

cxx_compiler::dump::table_t::table_t()
{
  (*this)[tac::ASSIGN] = assign;
  (*this)[tac::MUL] = mul;
  (*this)[tac::DIV] = div;
  (*this)[tac::MOD] = mod;
  (*this)[tac::ADD] = add;
  (*this)[tac::SUB] = sub;
  (*this)[tac::LSH] = lsh;
  (*this)[tac::RSH] = rsh;
  (*this)[tac::AND] = bit_and;
  (*this)[tac::OR] = bit_or;
  (*this)[tac::XOR] = bit_xor;
  (*this)[tac::PARAM] = param;
  (*this)[tac::CALL] = call;
  (*this)[tac::RETURN] = ret;
  (*this)[tac::ADDR] = addr;
  (*this)[tac::INVRADDR] = invraddr;
  (*this)[tac::INVLADDR] = invladdr;
  (*this)[tac::UMINUS] = uminus;
  (*this)[tac::TILDE] = tilde;
  (*this)[tac::CAST] = cast;
  (*this)[tac::GOTO] = go;
  (*this)[tac::TO] = to;
  (*this)[tac::LOFF] = loff;
  (*this)[tac::ROFF] = roff;
  (*this)[tac::ALLOCA] = _alloca_;
  (*this)[tac::ASM] = _asm_;
  (*this)[tac::VASTART] = _va_start;
  (*this)[tac::VAARG] = _va_arg;
  (*this)[tac::VAEND] = _va_end;
  (*this)[tac::ALLOCE] = alloce;
  (*this)[tac::THROW] = throwe;
  (*this)[tac::RETHROW] = rethrow;
  (*this)[tac::TRY_BEGIN] = try_begin;
  (*this)[tac::TRY_END] = try_end;
  (*this)[tac::HERE] = here;
  (*this)[tac::HERE_REASON] = here_reason;
  (*this)[tac::HERE_INFO] = here_info;
  (*this)[tac::THERE] = there;
  (*this)[tac::UNWIND_RESUME] = unwind_resume;
  (*this)[tac::CATCH_BEGIN] = catch_begin;
  (*this)[tac::CATCH_END] = catch_end;
  (*this)[tac::DCAST] = dcast;
}

namespace cxx_compiler { namespace dump { namespace names {
  template<class C> struct table : std::map<C, std::string> {
    int counter;
    void reset()
    {
      std::map<C, std::string>::clear();
      counter = 0;
    }
  };
  table<var*> vars;
} } } // end of namespace names, dump and cxx_compiler

std::string cxx_compiler::dump::names::ref(var* v)
{
  using namespace std;
  if (usr* u = v->usr_cast()) {
    string name = u->m_name;
    if ( name[0] != '.' )
      return names::scopey(v->m_scope) + name;
    if ( constant<int>* p = dynamic_cast<constant<int>*>(u) ){
      int n = p->m_value;
      ostringstream os;
      os << n;
      return os.str();
    }
    if ( constant<char>* p = dynamic_cast<constant<char>*>(u) ){
      char c = p->m_value;
      if (0 <= c && c <= 255) {
        if (isgraph(c) || c == ' ' || c == '\t') {
          ostringstream os;
          os << "'" << c << "'";
          return os.str();
        }
      }
    }
    return name;
  }
  if ( cmdline::simple_medium ){
    map<var*, string>::const_iterator p =
      vars.find(v);
    if ( p != vars.end() )
      return p->second;
    ostringstream os;
    os << 't' << vars.counter++;
    return vars[v] = os.str();
  }
  ostringstream os;
  os << ".var" << v;
  return os.str();
}

std::string cxx_compiler::dump::names::scopey(scope* p)
{
  if ( p == scope::current )
    return "";
  if ( p->m_id == scope::TAG ){
    tag* tg = static_cast<tag*>(p);
    return dump::names::scopey(tg->m_parent) + tg->m_name + "::";
  }
  if ( p->m_id == scope::NAMESPACE ){
    name_space* ns = static_cast<name_space*>(p);
    return dump::names::scopey(ns->m_parent) + ns->m_name + "::";
  }
  return "";
}

void cxx_compiler::dump::assign(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  os << x << " := " << y;
}

namespace cxx_compiler { namespace dump {
  void bin(std::ostream&, const tac*, std::string);
} } // end of namespace dump and cxx_compiler

void cxx_compiler::dump::mul(std::ostream& os, const tac* ptr)
{
  bin(os,ptr,"*");
}

void cxx_compiler::dump::div(std::ostream& os, const tac* ptr)
{
  bin(os,ptr,"/");
}

void cxx_compiler::dump::mod(std::ostream& os, const tac* ptr)
{
  bin(os,ptr,"%");
}

void cxx_compiler::dump::add(std::ostream& os, const tac* ptr)
{
  bin(os,ptr,"+");
}

void cxx_compiler::dump::sub(std::ostream& os, const tac* ptr)
{
  bin(os,ptr,"-");
}

void cxx_compiler::dump::lsh(std::ostream& os, const tac* ptr)
{
  bin(os,ptr,"<<");
}

void cxx_compiler::dump::rsh(std::ostream& os, const tac* ptr)
{
  bin(os,ptr,">>");
}

void cxx_compiler::dump::bit_and(std::ostream& os, const tac* ptr)
{
  bin(os,ptr,"&");
}

void cxx_compiler::dump::bit_or(std::ostream& os, const tac* ptr)
{
  bin(os,ptr,"|");
}

void cxx_compiler::dump::bit_xor(std::ostream& os, const tac* ptr)
{
  bin(os,ptr,"^");
}

void cxx_compiler::dump::bin(std::ostream& os, const tac* ptr, std::string op)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  string z = names::ref(ptr->z);
  os << x << " := " << y << ' ' << op << ' ' << z;
}

void cxx_compiler::dump::param(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string y = names::ref(ptr->y);
  os << "param " << y;
}

void cxx_compiler::dump::call(std::ostream& os, const tac* ptr)
{
  using namespace std;
  if ( ptr->x ){
    string x = names::ref(ptr->x);
    os << x << " := ";
  }
  string y = names::ref(ptr->y);
  os << "call " << y;
}

void cxx_compiler::dump::ret(std::ostream& os, const tac* ptr)
{
  using namespace std;
  os << "return";
  if ( ptr->y ){
    string y = names::ref(ptr->y);
    os << ' ' << y;
  }
}

namespace cxx_compiler { namespace dump {
  void una(std::ostream&, const tac*, std::string);
} } // end of namespace dump and cxx_compiler

void cxx_compiler::dump::addr(std::ostream& os, const tac* ptr)
{
  una(os,ptr,"&");
}

void cxx_compiler::dump::invraddr(std::ostream& os, const tac* ptr)
{
  una(os,ptr,"*");
}

void cxx_compiler::dump::invladdr(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string y = names::ref(ptr->y);
  string z = names::ref(ptr->z);
  os << '*' << y << " := " << z;
}

void cxx_compiler::dump::uminus(std::ostream& os, const tac* ptr)
{
  una(os,ptr,"-");
}

void cxx_compiler::dump::tilde(std::ostream& os, const tac* ptr)
{
  una(os,ptr,"~");
}

void cxx_compiler::dump::cast(std::ostream& os, const tac* ptr)
{
  using namespace std;
  ostringstream tmp;
  tmp << '(';
  const cast3ac* tc = static_cast<const cast3ac*>(ptr);
  const type* type = tc->m_type;
  type->decl(tmp,"");
  tmp << ')';
  una(os,ptr,tmp.str());
}

void cxx_compiler::dump::una(std::ostream& os, const tac* ptr, std::string op)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  os << x << " := " << op << y;
}

namespace cxx_compiler { namespace dump {
  std::string op2str(goto3ac::op op)
  {
    switch ( op ){
    case goto3ac::EQ: return "==";
    case goto3ac::NE: return "!=";
    case goto3ac::LE: return "<=";
    case goto3ac::GE: return ">=";
    case goto3ac::LT: return "<";
    case goto3ac::GT: return ">";
    default:          return "";
    }
  }
} } // end of namespace dump and cxx_compiler

namespace cxx_compiler { namespace dump { namespace names {
  std::string ref(to3ac*);
} } } // end of namespace names, dump and cxx_compiler

void cxx_compiler::dump::go(std::ostream& os, const tac* ptr)
{
  using namespace std;
  const goto3ac* p = static_cast<const goto3ac*>(ptr);
  string op = op2str(p->m_op);
  if ( op.empty() ){
    os << "goto " << names::ref(p->m_to);
    return;
  }
  string y = names::ref(ptr->y);
  string z = names::ref(ptr->z);
  os << "if " << y << ' ' << op << ' ' << z << " goto " << names::ref(p->m_to);
}

namespace cxx_compiler { namespace dump { namespace names {
  table<to3ac*> labels;
} } } // end of namespace names, dump and cxx_compiler

std::string cxx_compiler::dump::names::ref(to3ac* label)
{
  using namespace std;
  if ( cmdline::simple_medium ){
    map<to3ac*, string>::const_iterator p = labels.find(label);
    if ( p != labels.end() )
      return p->second;
    ostringstream os;
    os << "label" << labels.counter++;
    return labels[label] = os.str();
  }
  else {
    ostringstream os;
    os << "label" << label;
    return os.str();
  }
}

void cxx_compiler::dump::to(std::ostream& os, const tac* ptr)
{
  using namespace std;
  tac* x = const_cast<tac*>(ptr);
  to3ac* y = static_cast<to3ac*>(x);
  string label = names::ref(y);
  os << label << ':';
}

void cxx_compiler::dump::loff(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  string z = names::ref(ptr->z);
  os << x << '[' << y << ']' << " := " << z;
}

void cxx_compiler::dump::roff(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  string z = names::ref(ptr->z);
  os << x << " := " << y << '[' << z << ']';
}

void
cxx_compiler::dump::_alloca_(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  os << x << ":= alloca " << y;
}

void cxx_compiler::dump::_asm_(std::ostream& os, const tac* ptr)
{
  using namespace std;
  const asm3ac* p = static_cast<const asm3ac*>(ptr);
  os << "asm " << p->m_inst;
}

void cxx_compiler::dump::_va_start(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  os << x << " := va_start " << y;
}

void cxx_compiler::dump::_va_arg(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  os << x << " := va_arg " << y << " ,";
  const va_arg3ac* va = static_cast<const va_arg3ac*>(ptr);
  const type* type = va->m_type;
  type->decl(os,"");
}

void cxx_compiler::dump::_va_end(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string y = names::ref(ptr->y);
  os << "va_end " << y;
}

void cxx_compiler::dump::alloce(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  os << x << " := alloce " << y;
}

void cxx_compiler::dump::throwe(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string y = names::ref(ptr->y);
  os << "throwe " << y << ", ";
  os << '(';
  const throw3ac* th = static_cast<const throw3ac*>(ptr);
  const type* T = th->m_type;
  T->decl(os,"");
  os << ')';
}

void cxx_compiler::dump::rethrow(std::ostream& os, const tac* ptr)
{
  using namespace std;
  os << "rethrow";
}

void cxx_compiler::dump::try_begin(std::ostream& os, const tac* ptr)
{
  using namespace std;
  os << "try_begin";
}

void cxx_compiler::dump::try_end(std::ostream& os, const tac* ptr)
{
  using namespace std;
  os << "try_end";
}

void cxx_compiler::dump::here(std::ostream& os, const tac* ptr)
{
  using namespace std;
  os << "here";
}

void cxx_compiler::dump::here_reason(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  os << x << " := here_reason";
}

void cxx_compiler::dump::here_info(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  os << x << " := here_info";
}

void cxx_compiler::dump::there(std::ostream& os, const tac* ptr)
{
  using namespace std;
  os << "there";
}

void cxx_compiler::dump::unwind_resume(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string y = names::ref(ptr->y);
  os << "unwind_resume " << y;
}

void cxx_compiler::dump::catch_begin(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string y = names::ref(ptr->y);
  if (ptr->x) {
    string x = names::ref(ptr->x);
    os << x << " := ";
  }
  os << "catch_begin " << y;
}

void cxx_compiler::dump::catch_end(std::ostream& os, const tac* ptr)
{
  using namespace std;
  os << "catch_end";
}

void cxx_compiler::dump::dcast(std::ostream& os, const tac* ptr)
{
  using namespace std;
  una(os,ptr,"dcast ");
}

namespace cxx_compiler { namespace dump {
  int live1(const std::pair<optimize::basic_block::info_t*, std::set<var*> >&);
} } // end of namespace dump and cxx_compiler

void
cxx_compiler::dump::live(std::string name,
                         const std::map<optimize::basic_block::info_t*, std::set<var*> >& m)
{
  using namespace std;
  cout << name << '\n';
  for_each(m.begin(),m.end(),live1);
}

namespace cxx_compiler { namespace dump {
  int live2(var*);
} } // end of namespace dump and cxx_compiler

int
cxx_compiler::dump::live1(const std::pair<optimize::basic_block::info_t*, std::set<var*> >& p)
{
  using namespace std;
  optimize::basic_block::info_t* B = p.first;
  cout << '\t' << names::refb(B) << " :";
  const set<var*>& v = p.second;
  for_each(v.begin(),v.end(),live2);
  cout << '\n';
  return 0;
}

int cxx_compiler::dump::live2(var* v)
{
  using namespace std;
  cout << ' ' << dump::names::ref(v);
  return 0;
}

namespace cxx_compiler { namespace dump { namespace names {
  table<optimize::basic_block::info_t*> bbs;
} } } // end of namespace names, dump and cxx_compiler

std::string cxx_compiler::dump::names::refb(optimize::basic_block::info_t* B)
{
  using namespace std;
  if ( cmdline::simple_medium ){
    table<optimize::basic_block::info_t*>::const_iterator p = bbs.find(B);
    if ( p != bbs.end() )
      return p->second;
    ostringstream os;
    os << 'B' << bbs.counter++;
    return bbs[B] = os.str();
  }
  ostringstream os;
  os << 'B' << B;
  return os.str();
}

void cxx_compiler::dump::names::reset()
{
  vars.reset();
  labels.reset();
  bbs.reset();
}
