#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

namespace cxx_compiler { namespace optimize {
  void goto_to(vector<tac*>&);
  void empty_to3ac(vector<tac*>&);
  namespace basic_block { extern void action(fundef*, std::vector<tac*>&); }
} } // end of namespace optimize and cxx_compiler

void cxx_compiler::optimize::action(fundef* fdef, std::vector<tac*>& v)
{
  goto_to(v);
  empty_to3ac(v);
  if (cmdline::bb_optimize)
    basic_block::action(fdef, v);
}

void cxx_compiler::optimize::goto_to(std::vector<tac*>& vt)
{
  using namespace std;
  typedef vector<tac*>::iterator IT;
  for ( IT p = vt.begin() ; p != vt.end() ; ){
    if ((*p)->m_id != tac::GOTO) {
      ++p;
      continue;
    }
    goto3ac* go = static_cast<goto3ac*>(*p);
    IT q = p + 1;
    if (q == vt.end())
      break;
    if ((*q)->m_id != tac::TO) {
      p = q;
      continue;
    }
    to3ac* to = static_cast<to3ac*>(*q);
    if (go->m_to != to) {
      p = q + 1;
      continue;
    }
    delete go;
    vector<goto3ac*>& vg = to->m_goto;
    vector<goto3ac*>::iterator r = find(vg.begin(),vg.end(),go);
    assert(r != vg.end());
    vg.erase(r);
    if (vg.empty()) {
      delete to;
      vt.erase(q);
    }
    p = vt.erase(p);
  }
}

void cxx_compiler::optimize::empty_to3ac(std::vector<tac*>& v)
{
  using namespace std;
  typedef vector<tac*>::iterator IT;
  for ( IT p = v.begin() ; p != v.end() ; ){
    if ((*p)->m_id == tac::TO) {
      to3ac* to = static_cast<to3ac*>(*p);
      if ( to->m_goto.empty() ){
        delete *p;
        p = v.erase(p);
      }
      else
        ++p;
    }
    else
      ++p;
  }
}

namespace cxx_compiler { namespace optimize { namespace basic_block {
  namespace dag {
    using namespace std;
    struct action_t {
      vector<tac*> conv;
      set<var*> addr;
    };
    extern void action(info_t*, action_t*);
  } // end of namespace dag
} } } // end of namespace basic_block, optimize and cxx_compiler


namespace cxx_compiler { namespace optimize { namespace live_var {
  extern void analize(const std::vector<basic_block::info_t*>&);
} } } // end of namespace live_var, optimize and cxx_compiler

namespace cxx_compiler { namespace optimize { namespace symtab {
  extern int simplify(scope*, std::vector<tac*>*);
  namespace literal {
    void simplify(const vector<tac*>&);
    void just_clear();
  } // end of namespace literal
} } } // end of namespace symtab, optimize and cxx_compiler

void
cxx_compiler::optimize::basic_block::action(fundef* fdef, std::vector<tac*>& v)
{
  using namespace std;
  usr* u = fdef->m_usr;
  if ( cmdline::output_medium && cmdline::output_optinfo ){
    cout << "Before optimization\n";
    scope* org = scope::current;
    scope::current = &scope::root;
    cout << dump::names::ref(u) << ":\n";
    scope::current = org;
    for (auto p : v)
      cout << '\t', dump::tacx(cout, p), cout << '\n';
    cout << '\n';
    dump::scopex();
  }
  vector<info_t*> bbs;
  create(v,bbs);
  live_var::analize(bbs);
  if (cmdline::dag_optimize) {
    dag::action_t seed;
    for_each(bbs.begin(),bbs.end(),bind2nd(ptr_fun(dag::action),&seed));
    v = seed.conv;
  }
  for (auto p : bbs)
    delete p;
  {
    scope* p = fdef->m_param;
    vector<scope*>& children = p->m_children;
    assert(!children.empty());
    // children.size() == 1 is almost true.
    // Rare case like:
    // void f(struct S { ... } s, union U { ... } u) { ... }
    p = children.back();
    assert(p->m_id == scope::BLOCK);
    symtab::simplify(p,&v);
  }

  usr::flag_t flag = u->m_flag;
  scope* ptr = u->m_scope;
  if ((flag & usr::INLINE) && ptr->m_id == scope::TAG)
    symtab::literal::just_clear();
  else
    symtab::literal::simplify(v);
}

namespace cxx_compiler { namespace optimize { namespace basic_block {
  extern void devide(std::set<tac**>&, const std::vector<tac*>&);
  extern info_t* new_obj(tac**);
  extern void build(std::vector<info_t*>::iterator, std::vector<info_t*>::iterator, tac**);
} } } // end of namespace basic_block, optimize and cxx_compiler

void cxx_compiler::optimize::basic_block::create(std::vector<tac*>& v, std::vector<info_t*>& bbs)
{
  using namespace std;
  if ( v.empty() )
    return;
  set<tac**> leader;
  devide(leader,v);
  transform(leader.begin(),leader.end(),back_inserter(bbs),new_obj);
  tac** tmp = &v[0] + v.size();
  build(bbs.begin(),bbs.end(),tmp);
}

void cxx_compiler::optimize::basic_block::devide(std::set<tac**>& leader, const std::vector<tac*>& v)
{
  using namespace std;
  assert(!v.empty());
  typedef vector<tac*>::const_iterator IT;
  IT p = v.begin();
  leader.insert(const_cast<tac**>(&*p));
  while ( p != v.end() ){
    tac::id_t id = (*p)->m_id;
    switch ( id ){
    case tac::TO:
      leader.insert(const_cast<tac**>(&*p));
      ++p;
      break;
    case tac::GOTO:
      if ( ++p != v.end() )
        leader.insert(const_cast<tac**>(&*p));
      break;
    default:
      ++p;
      break;
    }
  }
}

cxx_compiler::optimize::basic_block::info_t*
cxx_compiler::optimize::basic_block::new_obj(tac** leader)
{
  return new info_t(leader);
}

namespace cxx_compiler { namespace optimize { namespace basic_block {
  bool destination(info_t*, tac*);
} } } // end of namespace basic_block, optimize and cxx_compiler

void cxx_compiler::optimize::basic_block::build(std::vector<info_t*>::iterator begin,
                                              std::vector<info_t*>::iterator end,
                                              tac** end3ac)
{
  std::vector<tac*> vv;
  using namespace std;
  typedef vector<info_t*>::const_iterator IT;
  for ( IT p = begin ; p != end ; ){
    basic_block::info_t* curr = *p;
    ++p;
    basic_block::info_t* next = p != end ? *p : 0;
    curr->m_size = next ? next->m_leader - curr->m_leader : end3ac - curr->m_leader;
    tac* last = next ? *(next->m_leader - 1) : *(end3ac-1);
    if ( last->m_id == tac::GOTO ){
      goto3ac* go = static_cast<goto3ac*>(last);
      tac* to = go->m_to;
      IT q = find_if(begin,end,bind2nd(ptr_fun(destination),to));
      assert(q != end);
      basic_block::info_t* follow = *q;
      curr->m_follow.push_back(follow);
      follow->m_preceed.push_back(curr);
      if ( go->m_op && next )
        curr->m_follow.push_back(next), next->m_preceed.push_back(curr);
    }
    else if ( next )
      curr->m_follow.push_back(next), next->m_preceed.push_back(curr);
  }
}

bool cxx_compiler::optimize::basic_block::destination(info_t* B, tac* to)
{
  return *B->m_leader == to;
}

namespace cxx_compiler { namespace optimize { namespace basic_block { namespace dag {
  struct info_t;
  struct mknode_t {
    std::map<var*, dag::info_t*>* node;
    std::map<dag::info_t*, var*>* result;
    basic_block::info_t* B;
    action_t* pa;
    bool ret;
    mknode_t(std::map<var*, dag::info_t*>* n, std::map<dag::info_t*, var*>* r, basic_block::info_t* b, action_t* a)
      : node(n), result(r), B(b), pa(a), ret(false) {}
  };
  void mknode(tac**, mknode_t*);
  namespace generate {
    struct action_t {
      mknode_t* mt;
      tac** pos;
      action_t(mknode_t* m, tac** p) : mt(m), pos(p) {}
    };
    void action(action_t*);
  } // end of namespace generate
} } } } // end of namespace dag, basic_block, optimize and c_compiler

void cxx_compiler::optimize::
basic_block::dag::action(basic_block::info_t* B, action_t* pa)
{
  using namespace std;
  tac** begin = B->m_leader;
  tac** end = begin + B->m_size;
  map<var*, dag::info_t*> node;
  map<dag::info_t*, var*> result;
  mknode_t mt(&node,&result,B,pa);
  for ( tac** p = begin ; p != end ; ++p )
    mknode(p,&mt);
  generate::action_t act(&mt,end-1);
  generate::action(&act);
}

namespace cxx_compiler { namespace optimize { namespace basic_block { namespace dag {
  using namespace std;
  struct info_t {
    tac* m_tac;
    vector<var*> m_vars;
    vector<info_t*> m_parents;
    info_t* m_left;
    info_t* m_right;
    info_t* m_extra;
    static std::vector<info_t*> all;
    info_t()
      : m_tac(0), m_left(0), m_right(0), m_extra(0) { all.push_back(this); }
  };
  vector<dag::info_t*> info_t::all;
  info_t* get(tac*, map<var*, dag::info_t*>*, bool*);
} } } } // end of namespace dag, basic_block, optimize and cxx_compiler

void cxx_compiler::optimize::basic_block::dag::mknode(tac** pp, mknode_t* mt)
{
  using namespace std;
  tac* ptr = *pp;
  tac::id_t id = ptr->m_id;
  if (mt->ret) {
    if (id != tac::TO && id != tac::GOTO) {
      delete ptr;
      return;
    }
    mt->ret = false;
  }
  if (id == tac::RETURN)
    mt->ret = true;
  var* y = ptr->y;
  if (!y) {
    // Special case. return3ac, goto3ac, to3ac or asm3ac.
    dag::info_t* p = new dag::info_t;
    p->m_tac = ptr;
    return;
  }
  map<var*, dag::info_t*>* node = mt->node;
  if (node->find(y) == node->end()) {
    dag::info_t* leaf = new dag::info_t;
    leaf->m_vars.push_back(y);
    (*node)[y] = leaf;
  }
  if (var* z = ptr->z) {
    if (node->find(z) == node->end()) {
      dag::info_t* leaf = new dag::info_t;
      leaf->m_vars.push_back(z);
      (*node)[z] = leaf;
    }
  }
  bool found = false;
  dag::info_t* n = get(ptr,node,&found);
  if (id == tac::CALL || id == tac::INVLADDR) {
    if (var* x = ptr->x) {
      vector<var*>& v = n->m_vars;
      v.push_back(x);
      (*node)[x] = n;
    }
    generate::action_t act(mt,pp);
    generate::action(&act);
    node->clear();
    mt->result->clear();
    return;
  }
  var* x = ptr->x;
  if (!x)
    return;
  auto_ptr<tac> sweeper(found ? ptr : 0);
  vector<var*>& v = n->m_vars;
  v.push_back(x);
  (*node)[x] = n;
  if (ptr->m_id == tac::ADDR)
    mt->pa->addr.insert(y);
}

namespace cxx_compiler { namespace optimize { namespace basic_block { namespace dag {
  bool match(info_t*, std::pair<tac*, std::map<var*, dag::info_t*>*>);
  bool roff_match_loff(info_t*, std::pair<tac*, std::map<var*, dag::info_t*>*>);
  void extra(info_t* info, var* x)
  {
    tac* ptr = info->m_tac;
    if (!ptr)
      return;
    tac::id_t id = ptr->m_id;
    if (id != tac::LOFF)
      return;
    info_t* extra = info->m_extra;
    if (!extra)
      return;
    vector<var*>& v = extra->m_vars;
    v.push_back(x);
  }
} } } } // end of namespace dag, basic_block, optimize and cxx_compiler

cxx_compiler::optimize::basic_block::dag::info_t*
cxx_compiler::optimize::basic_block::dag::get(tac* ptr, std::map<var*, dag::info_t*>* node, bool* found)
{
  using namespace std;
  tac::id_t id = ptr->m_id;
  switch (id) {
  case tac::ASSIGN:
    {
      var* y = ptr->y;
      map<var*, info_t*>::const_iterator p = node->find(y);
      assert(p != node->end());
      *found = true;
      info_t* info = p->second;
      var* x = ptr->x;
      extra(info, x);
      return info;
    }
  case tac::CALL: case tac::VAARG:
    break;
  default:
    if (ptr->x) {
      const vector<dag::info_t*>& v = dag::info_t::all;
      vector<dag::info_t*>::const_iterator p =
        find_if(v.begin(),v.end(),bind2nd(ptr_fun(match),make_pair(ptr,node)));
      if ( p != v.end() ){
        *found = true;
        return *p;
      }
      if ( id == tac::ROFF ){
        p = find_if(v.begin(),v.end(),bind2nd(ptr_fun(roff_match_loff),make_pair(ptr,node)));
        if ( p != v.end() ){
          *found = true;
          return (*p)->m_right;
        }
      }
    }
    break;
  }
  info_t* ret = new info_t;
  ret->m_tac = ptr;
  map<var*, info_t*>::const_iterator q = node->find(ptr->y);
  assert(q != node->end());
  info_t* i = q->second;
  i->m_parents.push_back(ret);
  ret->m_left = i;
  if (var* z = ptr->z) {
    map<var*, info_t*>::const_iterator q = node->find(z);
    assert(q != node->end());
    info_t* i = q->second;
    i->m_parents.push_back(ret);
    ret->m_right = i;
  }
  if (ptr->m_id == tac::LOFF) {
    var* x = ptr->x;
    map<var*, info_t*>::const_iterator q = node->find(x);
    if (q != node->end()) {
      info_t* i = q->second;
      i->m_parents.push_back(ret);
      ret->m_extra = i;
      vector<var*>& v = i->m_vars;
      vector<var*>::const_iterator p = find(begin(v), end(v), x);
      assert(p != end(v));
      v.erase(begin(v), p);
    }
  }
  return ret;
}

bool cxx_compiler::optimize::basic_block::
dag::match(info_t* xinfo, std::pair<tac*, std::map<var*, dag::info_t*>*> arg)
{
  using namespace std;
  tac* xtac = xinfo->m_tac;
  tac* ytac = arg.first;
  if (!xtac)
    return false;
  if (xtac->m_id != ytac->m_id)
    return false;
  map<var*, dag::info_t*>* node = arg.second;
  if (dag::info_t* left = xinfo->m_left) {
    var* y = ytac->y;
    map<var*, dag::info_t*>::const_iterator p = node->find(ytac->y);
    assert(p != node->end());
    dag::info_t* i = p->second;
    if (left != i)
      return false;
  }
  if (dag::info_t* right = xinfo->m_right) {
    map<var*, dag::info_t*>::const_iterator p = node->find(ytac->z);
    assert(p != node->end());
    dag::info_t* i = p->second;
    if (right != i)
      return false;
  }
  tac::id_t id = xtac->m_id;
  switch (id) {
  case tac::ROFF:
  case tac::INVRADDR:
    {
      const type* x = xtac->x->m_type;
      const type* y = ytac->x->m_type;
      x = x->unqualified();
      y = y->unqualified();
      return compatible(x, y);
    }
  case tac::CAST:
    {
      cast3ac* x = static_cast<cast3ac*>(xtac);
      cast3ac* y = static_cast<cast3ac*>(ytac);
      const type* Tx = x->m_type;
      const type* Ty = y->m_type;
      Tx = Tx->unqualified();
      Ty = Ty->unqualified();
      return compatible(Tx, Ty);
    }
  case tac::LOFF:
  case tac::ALLOCA:
    return xtac->x == ytac->x;
  default:
    return true;
  }
}

bool
cxx_compiler::optimize::basic_block::dag::
roff_match_loff(info_t* xinfo,
		std::pair<tac*, std::map<var*, dag::info_t*>*> arg)
{
  using namespace std;
  tac* roff = arg.first;
  assert(roff->m_id == tac::ROFF);
  tac* loff = xinfo->m_tac;
  if (!loff)
    return false;
  if (loff->m_id != tac::LOFF)
    return false;
  map<var*, dag::info_t*>* node = arg.second;
  var* a = roff->y;
  map<var*, dag::info_t*>::const_iterator p = node->find(a);
  assert(p != node->end());
  if (xinfo != p->second)
    return false;
  var* i = roff->z;
  map<var*, dag::info_t*>::const_iterator q = node->find(i);
  assert(q != node->end());
  if (xinfo->m_left != q->second)
    return false;
  const type* x = roff->x->m_type;
  const type* y = loff->z->m_type;
  x = x->unqualified();
  y = y->unqualified();
  return compatible(x, y);
}

namespace cxx_compiler { namespace optimize { namespace live_var {
  std::map<basic_block::info_t*, std::set<var*> > in, out, def, use;
  int prepare(basic_block::info_t*);
  int add_global1(basic_block::info_t*);
  int calc(int, basic_block::info_t*);
  void msg(int);
} } } // end of namespace live_var, optimize and cxx_compiler

namespace cxx_compiler { namespace optimize { namespace basic_block {
  std::map<var*, std::vector<std::pair<tac**,bool> > > live;
} } } // end of namespace basic_block, optimize and cxx_compiler

void cxx_compiler::optimize::live_var::analize(const std::vector<basic_block::info_t*>& bbs)
{
  using namespace std;
  in.clear(); out.clear(); def.clear(); use.clear();
  basic_block::live.clear();
  for_each(bbs.rbegin(),bbs.rend(),prepare);
  bool b = cmdline::output_medium && cmdline::output_optinfo;
  if ( b ){
    for_each(bbs.begin(),bbs.end(),dump::names::refb);
    dump::live("def",def);
    dump::live("use",use);
  }
  for_each(bbs.begin(),bbs.end(),add_global1);
  int i = 0;
  if ( b ) msg(i);
  while ( accumulate(bbs.begin(),bbs.end(),0,calc) )
    if ( b ) msg(++i);
  if ( b )
    dump::live("out",out);
}

void cxx_compiler::optimize::live_var::msg(int i)
{
  using namespace std;
  cout << "step " << i << '\n';
}

namespace cxx_compiler { namespace optimize { namespace live_var {
  void setup(tac**, basic_block::info_t*);
} } } // end of namespace live_var, optimize and cxx_compiler

int cxx_compiler::optimize::live_var::prepare(basic_block::info_t* B)
{
  using namespace std;
  tac** rend = B->m_leader - 1;
  tac** rbegin = rend + B->m_size;
  for ( tac** p = rbegin ; p != rend ; --p )
    setup(p,B);
  return 0;
}

void cxx_compiler::optimize::live_var::setup(tac** pp, basic_block::info_t* B)
{
  using namespace std;
  tac* p = *pp;
  typedef map<basic_block::info_t*, set<var*> >::iterator IT;
  if ( var* x = p->x ){
    def[B].insert(x);
    IT q = use.find(B);
    if ( q != use.end() )
      q->second.erase(x);
    tac::id_t id = p->m_id;
    if (id == tac::LOFF)
      basic_block::live[x].push_back(make_pair(pp,true));
    else
      basic_block::live[x].push_back(make_pair(pp,false));
  }
  if ( var* y = p->y ){
    use[B].insert(y);
    IT q = def.find(B);
    if ( q != def.end() )
      q->second.erase(y);
    basic_block::live[y].push_back(make_pair(pp,true));
  }
  if ( var* z = p->z ){
    use[B].insert(z);
    IT q = def.find(B);
    if ( q != def.end() )
      q->second.erase(z);
    basic_block::live[z].push_back(make_pair(pp,true));
  }
}

namespace cxx_compiler { namespace optimize { namespace live_var {
  int setout(basic_block::info_t*, basic_block::info_t*);
} } } // end of namespace live_var, optimize and cxx_compiler

int cxx_compiler::optimize::live_var::calc(int n, basic_block::info_t* B)
{
  using namespace std;
  // out[B] = U in[F], where F follows B
  const vector<basic_block::info_t*>& v = B->m_follow;
  for_each(v.begin(),v.end(),bind1st(ptr_fun(setout),B));
  bool flag = cmdline::output_medium && cmdline::output_optinfo;
  if ( flag ){
    cout << "\tout[" << dump::names::refb(B) << "] ";
    const set<var*>& o = out[B];
    transform(o.begin(),o.end(),ostream_iterator<string>(cout," "),dump::names::ref);
  }

  // in[B] = ( out[B] - def[B] ) U use[B]
  set<var*>& i = in[B];
  int org = i.size();
  i = out[B];
  typedef set<var*>::const_iterator IT;
  const set<var*>& d = def[B];
  for ( IT p = d.begin() ; p != d.end() ; ++p )
    i.erase(*p);
  const set<var*>& u = use[B];
  copy(u.begin(),u.end(),inserter(i,i.begin()));
  if ( flag ){
    cout << ", in[" << dump::names::refb(B) << "] ";
    transform(i.begin(),i.end(),ostream_iterator<string>(cout," "),dump::names::ref);
    cout << '\n';
  }
  return org != i.size() ? n + 1 : n;
}

int cxx_compiler::optimize::live_var::setout(basic_block::info_t* B, basic_block::info_t* S)
{
  using namespace std;
  const set<var*>& a = in[S];
  set<var*>& b = out[B];
  copy(a.begin(),a.end(),inserter(b,b.begin()));
  return 0;
}

namespace cxx_compiler { namespace optimize { namespace live_var {
  void add_global2(const std::set<var*>&, basic_block::info_t*);
  bool local(var*);
} } } // end of namespace live_var, optimize and cxx_compiler

int cxx_compiler::optimize::live_var::add_global1(basic_block::info_t* B)
{
  using namespace std;
  typedef map<basic_block::info_t*, set<var*> >::const_iterator IT;
  {
    for ( IT p = def.begin() ; p != def.end() ; ++p )
      add_global2(p->second,B);
  }
  {
    for ( IT p = use.begin() ; p != use.end() ; ++p )
      add_global2(p->second,B);
  }
  return 0;
}

void cxx_compiler::optimize::live_var::add_global2(const std::set<var*>& i, basic_block::info_t* B)
{
  using namespace std;
  set<var*>& o = out[B];
  remove_copy_if(i.begin(),i.end(),inserter(o,o.begin()),local);
}

bool cxx_compiler::optimize::live_var::local(var* x)
{
  if ( !x->m_scope->m_parent )
    return false;
  if ( x->m_scope->m_id == scope::NAMESPACE )
    return false;
  usr* u = x->usr_cast();
  if ( !u )
    return true;
  usr::flag_t flag = u->m_flag;
  usr::flag_t mask = usr::flag_t(usr::EXTERN | usr::STATIC);
  return !(flag & mask);
}

namespace cxx_compiler { namespace optimize { namespace basic_block { namespace dag { namespace generate {
  var* inorder(info_t*, action_t*);
} } } } } // end of namespace generate, dag, basic_block, optimize and cxx_compiler

void
cxx_compiler::optimize::basic_block::dag::generate::action(action_t* act)
{
  using namespace std;
  vector<info_t*>& v = info_t::all;
  for (auto p : v) inorder(p, act);
  for (auto p : v) delete p;
  v.clear();
}

namespace cxx_compiler { namespace optimize { namespace basic_block { namespace dag { namespace generate {
  var* assigns(dag::info_t*, action_t*);
  bool liveout(var*, basic_block::info_t*);
  bool use_after(var*, action_t*);
  bool resident(var*, std::pair<dag::info_t*, std::map<var*, dag::info_t*>*>);
} } } } } // end of namespace generate, dag, basic_block, optimize and cxx_compiler

cxx_compiler::var*
cxx_compiler::optimize::basic_block::dag::generate::inorder(dag::info_t* d, action_t* act)
{
  using namespace std;
  map<dag::info_t*, var*>& result = *act->mt->result;
  tac* ptr = d->m_tac;
  if (!ptr)
    return result[d] = assigns(d,act);
  dag::action_t* pa = act->mt->pa;
  vector<tac*>& conv = pa->conv;
  if (dag::info_t* i = d->m_left) {
    map<dag::info_t*, var*>::const_iterator p = result.find(i);
    assert(p != result.end());
    ptr->y = p->second;
  }
  if (dag::info_t* i = d->m_right) {
    map<dag::info_t*, var*>::const_iterator p = result.find(i);
    assert(p != result.end());
    ptr->z = p->second;
  }
  int n = conv.size();
  conv.push_back(ptr);
  var* x = ptr->x = assigns(d,act);
  if (!x || !d->m_parents.empty())
    return result[d] = x;
  basic_block::info_t* B = act->mt->B;
  if (liveout(x,B))
    return result[d] = x;
  bool b = use_after(x,act);
  if (b && resident(x,make_pair(d,act->mt->node)))
    return result[d] = x;
  const set<var*>& addr = act->mt->pa->addr;
  if (addr.find(x) != addr.end())
    return result[d] = x;
  tac::id_t id = ptr->m_id;
  switch (id) {
  case tac::CALL:
    if ( !b ){
      ptr->x = 0;
      return result[d] = 0;
    }
  case tac::ALLOCA:
    return result[d] = x;
  }
  conv.erase(conv.begin()+n);
  delete ptr;
  return result[d] = 0;
}

namespace cxx_compiler { namespace optimize { namespace basic_block { namespace dag { namespace generate {
  var* special1(dag::info_t*, action_t*);
  struct assign_t {
    action_t* act;
    var* y;
    var* x;
    info_t* d;
    var* addrrefed;
    assign_t(action_t* a, var* yy, info_t* dd) : act(a), y(yy), x(yy), d(dd), addrrefed(0) {}
  };
  int assign(var*, assign_t*);
  typedef vector<var*>::iterator IT;
  IT chose(vector<var*>& v, dag::info_t* d, action_t* act)
  {
    if (!d->m_left)
      return v.begin();
    mknode_t* mt = act->mt;
    basic_block::info_t* B = mt->B;
    IT p = find_if(v.begin(),v.end(),bind2nd(ptr_fun(liveout),B));
    if (p != v.end())
      return p;
    p = find_if(v.begin(),v.end(),bind2nd(ptr_fun(use_after),act));
    if (p != v.end())
      return p;
    p = find_if(v.begin(),v.end(),bind2nd(ptr_fun(resident),
					  make_pair(d,mt->node)));
    if (p != v.end())
      return p;
    p = find_if(v.begin(),v.end(),
		[mt](var* v){
		  set<var*>& addr = mt->pa->addr;
		  return addr.find(v) != addr.end();
		});
    if (p != v.end())
      return p;
    return v.begin();
  }
} } } } } // end of namespace generate, dag, basic_block, optimize and cxx_compiler

cxx_compiler::var*
cxx_compiler::optimize::basic_block::dag::generate::assigns(dag::info_t* d, action_t* act)
{
  using namespace std;
  vector<var*>& v = d->m_vars;
  if (v.empty())
    return 0;
  if (var* ret = special1(d,act))
    return ret;
  vector<var*>::iterator p = chose(v, d, act);
  var* y = *p;
  assign_t arg(act,y,d);
  for_each(p+1,v.end(),bind2nd(ptr_fun(assign),&arg));
  if (var* ret = arg.addrrefed)
    return ret;
  return y;
}

cxx_compiler::var*
cxx_compiler::optimize::basic_block::dag::generate::special1(dag::info_t* d, action_t* act)
{
  using namespace std;
  mknode_t* mt = act->mt;
  vector<var*>& v = d->m_vars;
  vector<var*>::iterator p =
    find_if(v.begin(),v.end(),bind2nd(ptr_fun(resident),make_pair(d,mt->node)));
  if ( p == v.end() )
    return 0;
  if ( p == v.begin() )
    return 0;
  if ( d->m_parents.empty() )
    return 0;
  basic_block::info_t* B = mt->B;
  vector<var*>::iterator q =
    find_if(p,v.end(),bind2nd(ptr_fun(liveout),B));
  if ( q != v.end() && resident(*q,make_pair(d,mt->node)) )
    p = q;
  if ( !d->m_left ){
    mt->pa->conv.push_back(new assign3ac(*p,*v.begin()));
    assign_t arg(act,*p,d);
    for_each(p+1,v.end(),bind2nd(ptr_fun(assign),&arg));
  }
  return *p;
}

bool cxx_compiler::optimize::basic_block::dag::generate::resident(var* v,
  std::pair<dag::info_t*, std::map<var*, dag::info_t*>*> p)
{
  using namespace std;
  dag::info_t* d = p.first;
  map<var*, dag::info_t*>* node = p.second;
  map<var*, dag::info_t*>::const_iterator it = node->find(v);
  assert(it != node->end());
  return it->second == d;
}

namespace cxx_compiler { namespace optimize { namespace basic_block { namespace dag { namespace generate {
  tac* replace_dst(tac*, var*);
} } } } } // end of namespace generate, dag, basic_block, optimize and cxx_compiler

cxx_compiler::tac*
cxx_compiler::optimize::basic_block::dag::generate::replace_dst(tac* ptr, var* x)
{
  ptr->x = x;
  return ptr;
}

bool cxx_compiler::optimize::basic_block::dag::generate::liveout(var* x, basic_block::info_t* B)
{
  using namespace std;
  const set<var*>& o = live_var::out[B];
  return o.find(x) != o.end();
}

bool cxx_compiler::optimize::basic_block::dag::generate::use_after(var* x, action_t* act)
{
  using namespace std;
  tac** pos = act->pos;
  map<var*, vector<pair<tac**, bool> > >::const_iterator q =
    basic_block::live.find(x);
  assert(q != basic_block::live.end());
  const vector<pair<tac**, bool> >& v = q->second;
  vector<pair<tac**,bool> >::const_reverse_iterator r;
  for ( r = v.rbegin() ; r != v.rend() ; ++r ){
    if ( r->first > pos )
      break;
  }
  if ( r == v.rend() )
    return false;
  return r->second;
}

namespace cxx_compiler { namespace optimize { namespace basic_block { namespace dag { namespace generate {
  bool addrrefs(var*, assign_t*);
  int gen_assign(var*, assign_t*);
} } } } } // end of namespace generate, dag, basic_block, optimize and cxx_compiler

int
cxx_compiler::optimize::basic_block::dag::generate::assign(var* x, assign_t* arg)
{
  using namespace std;
  var* y = arg->y;
  usr* uy = y->usr_cast();
  if ( addrrefs(x,arg) ){
    arg->addrrefed = x;
    if ( arg->d->m_left )
      return 0;
    return gen_assign(x,arg);
  }
  if ( !resident(x,make_pair(arg->d,arg->act->mt->node)) )
    return 0;
  basic_block::info_t* B = arg->act->mt->B;
  if ( liveout(x,B) )
    return gen_assign(x,arg);
  set<var*>& addr = arg->act->mt->pa->addr;
  if ( addr.find(x) != addr.end() )
    return gen_assign(x,arg);
  if ( use_after(x,arg->act) )
    return gen_assign(x,arg);
  return 0;
}

namespace cxx_compiler { namespace optimize { namespace basic_block { namespace dag { namespace generate {
  bool addrref(var*, dag::info_t*);
} } } } } // end of namespace generate, dag, basic_block, optimize and cxx_compiler

bool
cxx_compiler::optimize::basic_block::dag::generate::addrrefs(var* x, assign_t* arg)
{
  using namespace std;
  dag::info_t* d = arg->d;
  vector<dag::info_t*> v = d->m_parents;
  return find_if(v.begin(),v.end(),bind1st(ptr_fun(addrref),x)) != v.end();
}

bool
cxx_compiler::optimize::basic_block::dag::generate::addrref(var* x, dag::info_t* parent)
{
  tac* ptr = parent->m_tac;
  if ( !ptr )
    return false;
  return ptr->m_id == tac::ADDR && ptr->y == x;
}

int
cxx_compiler::optimize::basic_block::dag::generate::gen_assign(var* x, assign_t* arg)
{
  using namespace std;
  vector<tac*>& conv = arg->act->mt->pa->conv;
  var* y = arg->y;
  conv.push_back(new assign3ac(x,y));
  arg->x = x;
  return 0;
}

namespace cxx_compiler { namespace optimize { namespace symtab {
  extern bool not_referenced(var*, std::vector<tac*>*);
} } } // end of namespace symtab, optimize and cxx_compiler

int
cxx_compiler::optimize::symtab::simplify(scope* ptr, std::vector<tac*>* res)
{
  using namespace std;
  map<string, vector<usr*> >& usrs = ptr->m_usrs;
  typedef map<string, vector<usr*> >::iterator X;
  for ( X x = usrs.begin() ; x != usrs.end() ; ){
    vector<usr*>& v = x->second;
    typedef vector<usr*>::iterator IT;
    for ( IT p = v.begin() ; p != v.end() ; ){
      if ( not_referenced(*p,res) ){
        delete *p;
        p = v.erase(p);
      }
      else
        ++p;
    }
    if ( v.empty() ){
      X xx = x++;
      usrs.erase(xx);
    }
    else
      ++x;
  }
  if ( ptr->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(ptr);
    vector<var*>& v = b->m_vars;
    typedef vector<var*>::iterator IT;
    for ( IT p = v.begin() ; p != v.end() ; ){
      if ( not_referenced(*p,res) ){
        delete *p;
        p = v.erase(p);
      }
      else
        ++p;
    }
  }
  vector<scope*>& c = ptr->m_children;
  for_each(c.begin(),c.end(),bind2nd(ptr_fun(simplify),res));
  return 0;
}

namespace cxx_compiler { namespace optimize { namespace symtab {
  extern bool refs(tac*, var*);
} } } // end of namespace symtab, optimize and cxx_compiler

bool cxx_compiler::optimize::symtab::not_referenced(var* v, std::vector<tac*>* res)
{
  using namespace std;
  vector<tac*>::const_iterator p =
    find_if(res->begin(),res->end(),bind2nd(ptr_fun(refs),v));
  return p == res->end();
}

bool cxx_compiler::optimize::symtab::refs(tac* ptr, var* v)
{
  if ( var* u = ptr->x ){
    if ( u == v )
      return true;
  }
  if ( var* u = ptr->y ){
    if ( u == v )
      return true;
  }
  if ( var* u = ptr->z ){
    if ( u == v )
      return true;
  }
  return false;
}

namespace cxx_compiler { namespace optimize { namespace symtab { namespace literal {
        using namespace std;
        set<usr*> canbe_erased;
        void mark1(tac*);
        void mark2(scope*);
        void mark3(const pair<string, vector<usr*> >&);
        void mark4(usr*);
        void mark5(const pair<int, var*>&);
        void erase(usr*);
} } } } // end of namespace literal, symtab, optimize and cxx_compiler

void cxx_compiler::optimize::mark(usr* u)
{
  symtab::literal::canbe_erased.insert(u);
}

void cxx_compiler::optimize::symtab::literal::simplify(const std::vector<tac*>& vc)
{
  using namespace std;
  for_each(vc.begin(), vc.end(), mark1);
  mark2(&scope::root);
  for_each(canbe_erased.begin(), canbe_erased.end(), erase);
  canbe_erased.clear();
}

void cxx_compiler::optimize::symtab::literal::just_clear()
{
  canbe_erased.clear();
}

void cxx_compiler::optimize::symtab::literal::mark1(tac* tac)
{
  using namespace std;
  if (var* y = tac->y) {
    if (usr* u = y->usr_cast()) {
      if (u->isconstant() || is_string(u->m_name)) {
        canbe_erased.erase(u);
      }
    }
  }
  if (var* z = tac->z) {
    if (usr* u = z->usr_cast()) {
      if (u->isconstant() || is_string(u->m_name)) {
        canbe_erased.erase(u);
      }
    }
  }
}

void cxx_compiler::optimize::symtab::literal::mark2(scope* ptr)
{
  using namespace std;
  map<string, vector<usr*> >& usrs = ptr->m_usrs;
  for_each(usrs.begin(), usrs.end(), mark3);
  vector<scope*>& children = ptr->m_children;
  for_each(children.begin(), children.end(), mark2);
}

void cxx_compiler::optimize::symtab::literal::mark3(const std::pair<std::string, std::vector<usr*> >& x)
{
  using namespace std;
  const vector<usr*>& vec = x.second;
  for_each(vec.begin(), vec.end(), mark4);
}

void cxx_compiler::optimize::symtab::literal::mark4(usr* u)
{
  using namespace std;
  usr::flag_t flag = u->m_flag;
  if ( !(flag & usr::WITH_INI) )
    return;
  with_initial* wi = static_cast<with_initial*>(u);
  const map<int, var*>& value = wi->m_value;
  for_each(value.begin(), value.end(), mark5);
}

void cxx_compiler::optimize::symtab::literal::mark5(const std::pair<int, var*>& x)
{
  using namespace std;
  var* v = x.second;
  if (usr* u = v->usr_cast()) {
    assert(u->isconstant());
    canbe_erased.erase(u);
  }
  else {
    addrof* addr = v->addrof_cast();
    assert(addr);
    var* ref = addr->m_ref;
    if (usr* u = ref->usr_cast()) {
      if (is_string(u->m_name))
        canbe_erased.erase(u);
    }
  }
}

void cxx_compiler::optimize::symtab::literal::erase(usr* u)
{
  using namespace std;
  map<string, vector<usr*> >& usrs = scope::root.m_usrs;
  string name = u->m_name;
  map<string, vector<usr*> >::iterator it = usrs.find(name);
  assert(it != usrs.end());
  vector<usr*>& v = it->second;
  assert(v.size() == 1);
  assert(v[0] == u);
  usrs.erase(it);
  delete u;
}
