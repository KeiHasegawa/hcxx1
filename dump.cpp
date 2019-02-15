#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

namespace cxx_compiler { namespace dump {
  int usrsx(std::pair<std::string, std::vector<usr*> >, int);
  int tagx(std::pair<std::string, tag*>, int);
  int varx(cxx_compiler::var*, int);
} } // end of namespace dump and cxx_compiler

int cxx_compiler::dump::scope(cxx_compiler::scope* ptr, int ntab)
{
  using namespace std;
  const map<string, vector<usr*> >& usrs = ptr->m_usrs;
  for_each(usrs.begin(),usrs.end(),bind2nd(ptr_fun(dump::usrsx),ntab));
  const map<string, cxx_compiler::tag*>& tags = ptr->m_tags;
  for_each(tags.begin(),tags.end(),bind2nd(ptr_fun(dump::tagx),ntab));
  const vector<cxx_compiler::scope*>& children = ptr->m_children;
  if ( ptr->m_id == cxx_compiler::scope::BLOCK ){
    block* b = static_cast<block*>(ptr);
    vector<cxx_compiler::var*>& vars = b->m_vars;
    for_each(vars.begin(),vars.end(),bind2nd(ptr_fun(dump::varx),ntab));
  }
  for_each(children.begin(),children.end(),bind2nd(ptr_fun(dump::scope),ntab+1));
  return 0;
}

namespace cxx_compiler { namespace dump {
  int usrx(const usr*, int);
} } // end of namespace dump and cxx_compiler

int cxx_compiler::dump::usrsx(std::pair<std::string, std::vector<cxx_compiler::usr*> > p, int ntab)
{
  using namespace std;
  const vector<cxx_compiler::usr*>& v = p.second;
  for_each(v.begin(),v.end(),bind2nd(ptr_fun(dump::usrx),ntab));
  return 0;
}

namespace cxx_compiler { namespace dump {
  std::string initial(const std::pair<int,cxx_compiler::var*>&);
} } // end of namespace dump and cxx_compiler

int cxx_compiler::dump::usrx(const cxx_compiler::usr* u, int ntab)
{
  using namespace std;
  int n = ntab;
  while ( n-- ) cout << '\t';
  if ( u->m_flag ){
    string s = cxx_compiler::usr::keyword(u->m_flag);
    if ( !s.empty() )
      cout << s << ' ';
  }
  string name = names::ref(const_cast<cxx_compiler::usr*>(u));
  const type* T = u->m_type;
  if ( !T ){
    assert(u->m_flag & cxx_compiler::usr::NAMESPACE);
    cout << name << '\n';
    const name_space* ns = static_cast<const name_space*>(u);
    cxx_compiler::scope* org = cxx_compiler::scope::current;
    cxx_compiler::scope::current = const_cast<name_space*>(ns);
    dump::scope(cxx_compiler::scope::current,ntab+1);
    cxx_compiler::scope::current = org;
    return 0;
  }
  T->decl(cout,name);
  if ( with_initial* p = ((cxx_compiler::usr*)u)->with_initial_cast() ){
    cout << '\t';
    const map<int,cxx_compiler::var*>& v = p->m_value;
    transform(v.begin(),v.end(),ostream_iterator<string>(cout,","),initial);
  }
  cout << '\n';
  return 0;
}

std::string cxx_compiler::dump::initial(const std::pair<int,cxx_compiler::var*>& p)
{
  using namespace std;
  ostringstream os;
  os << '(' << p.first << ',';
  cxx_compiler::var* v = p.second;
  addrof* addr = dynamic_cast<addrof*>(v);
  if ( addr ){
    os << "addrof(";
    v = addr->m_ref;
  }
  cxx_compiler::usr* u = static_cast<cxx_compiler::usr*>(v);
  os << names::ref(u);
  if ( addr ){
    os << ')';
    if ( int offset = addr->m_offset )
      os << '+' << offset;
  }
  os << ')';
  return os.str();
}

int cxx_compiler::dump::tagx(std::pair<std::string, cxx_compiler::tag*> p, int ntab)
{
  using namespace std;
  int n = ntab;
  while ( n-- ) cout << '\t';
  cxx_compiler::tag* T = p.second;
  cout << cxx_compiler::tag::keyword(T->m_kind);
  string name = T->m_name;
  if ( !name.empty() )
    cout << ' ' << name;
  cout << '\n';
  cxx_compiler::scope* org = cxx_compiler::scope::current;
  cxx_compiler::scope::current = T;
  dump::scope(T,ntab+1);
  cxx_compiler::scope::current = org;
  return 0;
}

int cxx_compiler::dump::varx(cxx_compiler::var* v, int ntab)
{
  using namespace std;
  while ( ntab-- ) cout << '\t';
  string name = dump::names::ref(v);
  const type* T = v->m_type;
  T->decl(cout,name);
  cout << '\n';
  return 0;
}

namespace cxx_compiler { namespace dump {
  struct table : std::map<tac::id_t, void (*)(std::ostream&, const cxx_compiler::tac*)> {
    table();
  } m_table;
} } // end of namespace dump and cxx_compiler

void cxx_compiler::dump::tac(std::ostream& os, const cxx_compiler::tac* ptr)
{
  m_table[ptr->m_id](os,ptr);
}

namespace cxx_compiler { namespace dump {
  void assign(std::ostream&, const cxx_compiler::tac*);
  void mul(std::ostream&, const cxx_compiler::tac*);
  void div(std::ostream&, const cxx_compiler::tac*);
  void mod(std::ostream&, const cxx_compiler::tac*);
  void add(std::ostream&, const cxx_compiler::tac*);
  void sub(std::ostream&, const cxx_compiler::tac*);
  void lsh(std::ostream&, const cxx_compiler::tac*);
  void rsh(std::ostream&, const cxx_compiler::tac*);
  void bit_and(std::ostream&, const cxx_compiler::tac*);
  void bit_or(std::ostream&, const cxx_compiler::tac*);
  void bit_xor(std::ostream&, const cxx_compiler::tac*);
  void param(std::ostream&, const cxx_compiler::tac*);
  void call(std::ostream&, const cxx_compiler::tac*);
  void ret(std::ostream&, const cxx_compiler::tac*);
  void addr(std::ostream&, const cxx_compiler::tac*);
  void invraddr(std::ostream&, const cxx_compiler::tac*);
  void invladdr(std::ostream&, const cxx_compiler::tac*);
  void uminus(std::ostream&, const cxx_compiler::tac*);
  void tilde(std::ostream&, const cxx_compiler::tac*);
  void cast(std::ostream&, const cxx_compiler::tac*);
  void go(std::ostream&, const cxx_compiler::tac*);
  void to(std::ostream&, const cxx_compiler::tac*);
  void loff(std::ostream&, const cxx_compiler::tac*);
  void roff(std::ostream&, const cxx_compiler::tac*);
  void _alloca_(std::ostream&, const cxx_compiler::tac*);
  void _asm_(std::ostream&, const cxx_compiler::tac*);
  void _va_start(std::ostream&, const cxx_compiler::tac*);
  void _va_arg(std::ostream&, const cxx_compiler::tac*);
  void _va_end(std::ostream&, const cxx_compiler::tac*);
} } // end of namespace dump and cxx_compiler

cxx_compiler::dump::table::table()
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
  table<cxx_compiler::var*> vars;
} } } // end of namespace names, dump and cxx_compiler

std::string cxx_compiler::dump::names::ref(cxx_compiler::var* v)
{
  using namespace std;
  if ( cxx_compiler::usr* u = v->usr_cast() ){
    string name = u->m_name;
    if ( name[0] != '.' )
      return names::scope(v->m_scope) + name;
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
    map<cxx_compiler::var*, string>::const_iterator p =
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

std::string cxx_compiler::dump::names::scope(cxx_compiler::scope* p)
{
  if ( p == cxx_compiler::scope::current )
    return "";
  if ( p->m_id == cxx_compiler::scope::TAG ){
    cxx_compiler::tag* tg = static_cast<cxx_compiler::tag*>(p);
    return dump::names::scope(tg->m_parent) + tg->m_name + "::";
  }
  if ( p->m_id == cxx_compiler::scope::NAMESPACE ){
    name_space* ns = static_cast<name_space*>(p);
    return dump::names::scope(ns->m_parent) + ns->m_name + "::";
  }
  return "";
}

void cxx_compiler::dump::assign(std::ostream& os, const cxx_compiler::tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  os << x << " := " << y;
}

namespace cxx_compiler { namespace dump {
  void bin(std::ostream&, const cxx_compiler::tac*, std::string);
} } // end of namespace dump and cxx_compiler

void cxx_compiler::dump::mul(std::ostream& os, const cxx_compiler::tac* ptr)
{
  bin(os,ptr,"*");
}

void cxx_compiler::dump::div(std::ostream& os, const cxx_compiler::tac* ptr)
{
  bin(os,ptr,"/");
}

void cxx_compiler::dump::mod(std::ostream& os, const cxx_compiler::tac* ptr)
{
  bin(os,ptr,"%");
}

void cxx_compiler::dump::add(std::ostream& os, const cxx_compiler::tac* ptr)
{
  bin(os,ptr,"+");
}

void cxx_compiler::dump::sub(std::ostream& os, const cxx_compiler::tac* ptr)
{
  bin(os,ptr,"-");
}

void cxx_compiler::dump::lsh(std::ostream& os, const cxx_compiler::tac* ptr)
{
  bin(os,ptr,"<<");
}

void cxx_compiler::dump::rsh(std::ostream& os, const cxx_compiler::tac* ptr)
{
  bin(os,ptr,">>");
}

void cxx_compiler::dump::bit_and(std::ostream& os, const cxx_compiler::tac* ptr)
{
  bin(os,ptr,"&");
}

void cxx_compiler::dump::bit_or(std::ostream& os, const cxx_compiler::tac* ptr)
{
  bin(os,ptr,"|");
}

void cxx_compiler::dump::bit_xor(std::ostream& os, const cxx_compiler::tac* ptr)
{
  bin(os,ptr,"^");
}

void cxx_compiler::dump::bin(std::ostream& os, const cxx_compiler::tac* ptr, std::string op)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  string z = names::ref(ptr->z);
  os << x << " := " << y << ' ' << op << ' ' << z;
}

void cxx_compiler::dump::param(std::ostream& os, const cxx_compiler::tac* ptr)
{
  using namespace std;
  string y = names::ref(ptr->y);
  os << "param " << y;
}

void cxx_compiler::dump::call(std::ostream& os, const cxx_compiler::tac* ptr)
{
  using namespace std;
  if ( ptr->x ){
    string x = names::ref(ptr->x);
    os << x << " := ";
  }
  string y = names::ref(ptr->y);
  os << "call " << y;
}

void cxx_compiler::dump::ret(std::ostream& os, const cxx_compiler::tac* ptr)
{
  using namespace std;
  os << "return";
  if ( ptr->y ){
    string y = names::ref(ptr->y);
    os << ' ' << y;
  }
}

namespace cxx_compiler { namespace dump {
  void una(std::ostream&, const cxx_compiler::tac*, std::string);
} } // end of namespace dump and cxx_compiler

void cxx_compiler::dump::addr(std::ostream& os, const cxx_compiler::tac* ptr)
{
  una(os,ptr,"&");
}

void cxx_compiler::dump::invraddr(std::ostream& os, const cxx_compiler::tac* ptr)
{
  una(os,ptr,"*");
}

void cxx_compiler::dump::invladdr(std::ostream& os, const cxx_compiler::tac* ptr)
{
  using namespace std;
  string y = names::ref(ptr->y);
  string z = names::ref(ptr->z);
  os << '*' << y << " := " << z;
}

void cxx_compiler::dump::uminus(std::ostream& os, const cxx_compiler::tac* ptr)
{
  una(os,ptr,"-");
}

void cxx_compiler::dump::tilde(std::ostream& os, const cxx_compiler::tac* ptr)
{
  una(os,ptr,"~");
}

void cxx_compiler::dump::cast(std::ostream& os, const cxx_compiler::tac* ptr)
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

void cxx_compiler::dump::una(std::ostream& os, const cxx_compiler::tac* ptr, std::string op)
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

void cxx_compiler::dump::go(std::ostream& os, const cxx_compiler::tac* ptr)
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

void cxx_compiler::dump::to(std::ostream& os, const cxx_compiler::tac* ptr)
{
  using namespace std;
  cxx_compiler::tac* x = const_cast<cxx_compiler::tac*>(ptr);
  to3ac* y = static_cast<to3ac*>(x);
  string label = names::ref(y);
  os << label << ':';
}

void cxx_compiler::dump::loff(std::ostream& os, const cxx_compiler::tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  string z = names::ref(ptr->z);
  os << x << '[' << y << ']' << " := " << z;
}

void cxx_compiler::dump::roff(std::ostream& os, const cxx_compiler::tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  string z = names::ref(ptr->z);
  os << x << " := " << y << '[' << z << ']';
}

void
cxx_compiler::dump::_alloca_(std::ostream& os, const cxx_compiler::tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  os << "alloca " << x << ", " << y;
}

void cxx_compiler::dump::_asm_(std::ostream& os, const cxx_compiler::tac* ptr)
{
  using namespace std;
  const asm3ac* p = static_cast<const asm3ac*>(ptr);
  os << "asm " << p->m_inst;
}

void cxx_compiler::dump::_va_start(std::ostream& os, const cxx_compiler::tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  os << x << " := va_start " << y;
}

void cxx_compiler::dump::_va_arg(std::ostream& os, const cxx_compiler::tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  os << x << " := va_arg " << y << " ,";
  const va_arg3ac* va = static_cast<const va_arg3ac*>(ptr);
  const type* type = va->m_type;
  type->decl(os,"");
}

void cxx_compiler::dump::_va_end(std::ostream& os, const cxx_compiler::tac* ptr)
{
  using namespace std;
  string y = names::ref(ptr->y);
  os << "va_end " << y;
}

namespace cxx_compiler { namespace dump {
  int live1(const std::pair<optimize::basic_block::info_t*, std::set<cxx_compiler::var*> >&);
} } // end of namespace dump and cxx_compiler

void
cxx_compiler::dump::live(std::string name,
                         const std::map<optimize::basic_block::info_t*, std::set<cxx_compiler::var*> >& m)
{
  using namespace std;
  cout << name << '\n';
  for_each(m.begin(),m.end(),live1);
}

namespace cxx_compiler { namespace dump {
  int live2(cxx_compiler::var*);
} } // end of namespace dump and cxx_compiler

int
cxx_compiler::dump::live1(const std::pair<optimize::basic_block::info_t*, std::set<cxx_compiler::var*> >& p)
{
  using namespace std;
  optimize::basic_block::info_t* B = p.first;
  cout << '\t' << names::refb(B) << " :";
  const set<cxx_compiler::var*>& v = p.second;
  for_each(v.begin(),v.end(),live2);
  cout << '\n';
  return 0;
}

int cxx_compiler::dump::live2(cxx_compiler::var* v)
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
