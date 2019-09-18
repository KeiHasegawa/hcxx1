#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "cxx_y.h"
#include "yy.h"
#include "patch.03.q"
#include "patch.04.q"
#include "patch.10.q"

namespace cxx_compiler {
  namespace parse {
    file_t position;
    namespace identifier {
      mode_t mode;
      int create(std::string, const type* = backpatch_type::create());
      inline scope* search_scope()
      {
	if (mode != mem_ini)
	  return scope::current;
	assert(scope::current->m_id == scope::PARAM);
	scope* parent = scope::current->m_parent;
	assert(parent->m_id == scope::TAG);
	return parent;
      }
    } // end of namespace identifier
  } // end of namespace parse and
} // end of namespace cxx_compiler

int cxx_compiler::parse::identifier::judge(std::string name)
{
  if (mode == peeking)
    return create(name), PEEKED_NAME_LEX;

  if (last_token == COLONCOLON_MK || peek() == COLONCOLON_MK) {
    if (int r = lookup(name, scope::current))
      return r;
    error::undeclared(parse::position, name);
    if (last_token == COLONCOLON_MK) {
      int r = create(name, int_type::create());
      usr* u = cxx_compiler_lval.m_usr;
      scope::current->m_usrs[name].push_back(u);
      return r;
    }
    cxx_compiler_lval.m_name_space = new name_space(name, parse::position);
    scope::current->m_usrs[name].push_back(cxx_compiler_lval.m_name_space);
    return ORIGINAL_NAMESPACE_NAME_LEX;
  }
  
  if (last_token == '(' && scope::current->m_id == scope::PARAM ||
      mode == canbe_ctor) {
    // guess abstract-declarator or declaration of constructor
    if (int r = lookup(name, scope::current)) {
      switch (r) {
      case ORIGINAL_NAMESPACE_NAME_LEX:
      case NAMESPACE_ALIAS_LEX:
      case TYPEDEF_NAME_LEX:
      case CLASS_NAME_LEX:
      case TEMPLATE_NAME_LEX:
      case ENUM_NAME_LEX:
        return r;
      }
    }
  }

  if (mode == new_obj || mode == canbe_ctor)
    return create(name);

  if (int r = lookup(name, search_scope())) {
    if (context_t::retry[DECL_FCAST_CONFLICT_STATE])
      return r;
    using namespace declarations::specifier_seq;
    const stack<info_t*>& s = info_t::s_stack;
    if (s.empty()) {
      if (last_token == EXTERN_KW && r == IDENTIFIER_LEX) {
        // Rare case like:
        // int n; extern n;
        // For 2nd `n', return create(name) not return entry of symbol table.
        return create(name);
      }
      return r;
    }
    info_t* p = s.top();
    if (!p)
      return r;
    if (p->m_type)
      return r;
    switch (r) {
    case TYPEDEF_NAME_LEX:
    case CLASS_NAME_LEX:
    case ENUM_NAME_LEX:
    case TEMPLATE_NAME_LEX:
      return r;
    }
    // For static x; static x;
    // For 2nd `x' return create(name) not return exsisting entry.
    return create(name);
  }
  error::undeclared(parse::position, name);
  int r = create(name, int_type::create());
  usr* u = cxx_compiler_lval.m_usr;
  scope::current->m_usrs[name].push_back(u);
  return r;
}

int cxx_compiler::parse::identifier::create(std::string name, const type* T)
{
  cxx_compiler_lval.m_usr =
    new usr(name,T,usr::NONE,parse::position,usr::NONE2);
  return IDENTIFIER_LEX;
}

namespace cxx_compiler {
  namespace parse {
    namespace identifier {
      namespace underscore_func {
        int action();
      } // end of namespace underscore_func
      using namespace std;
      namespace base_lookup {
	struct info_t {
	  int m_kind;
	  var* m_lval;
	  vector<base*> m_base;
	  info_t(int kind, var* lval, base* bp)
	    : m_kind(kind), m_lval(lval) { m_base.push_back(bp); }
	};
        struct gather {
          string m_name;
	  vector<info_t>& m_choice; 
          bool lookup(const map<string, vector<usr*> >& usrs, base* bp)
          {
            map<string, vector<usr*> >::const_iterator p = usrs.find(m_name);
            if (p == end(usrs))
              return false;
            const vector<usr*>& v = p->second;
            usr* u = v.back();
            usr::flag_t flag = u->m_flag;
            if (flag & usr::CTOR)
              return false;
	    if (flag & usr::OVERLOAD) {
	      overload* ovl = static_cast<overload*>(u);
	      vector<usr*>& v = ovl->m_candidacy;
	      assert(!v.empty());
	      usr* uu = v.back();
	      if (uu->m_flag & usr::CTOR)
		return false;
	    }
	    int kind = (flag & usr::TYPEDEF) ? TYPEDEF_NAME_LEX
	      : IDENTIFIER_LEX;
	    info_t tmp(kind, u, bp);
	    if (flag & usr::OVERLOAD) {
	      m_choice.push_back(tmp);
              return true;
	    }
            const type* T = u->m_type;
            if (const pointer_type* G = T->ptr_gen())
              garbage.push_back(tmp.m_lval = new genaddr(G,T,u,0));
	    m_choice.push_back(tmp);
            return true;
          }
          bool lookup(const map<string, tag*>& tags, base* bp)
          {
            map<string, tag*>::const_iterator p = tags.find(m_name);
            if (p == end(tags))
              return false;
	    tag* ptr = p->second;
	    info_t tmp(CLASS_NAME_LEX, (var*)ptr, bp);
	    m_choice.push_back(tmp);
            return true;
          }
	  static inline void insert(info_t& info, base* bp)
	  {
	    vector<base*>& v = info.m_base;
	    v.insert(v.begin(), bp);
	  }
          gather(string name, vector<info_t>& choice)
            : m_name(name), m_choice(choice) {}
          void operator()(base* bp)
          {
            tag* ptr = bp->m_tag;
            if (lookup(ptr->m_usrs, bp))
              return;
            
            if (lookup(ptr->m_tags, bp))
              return;

            if (ptr->m_name == m_name) {
	      m_choice.push_back(info_t(CLASS_NAME_LEX, (var*)ptr, bp));
              return;
            }

            if (vector<base*>* bases = ptr->m_bases) {
	      vector<info_t> tmp;
              for_each(begin(*bases), end(*bases), gather(m_name, tmp));
	      for (auto& info : tmp)
		insert(info, bp);
	      copy(begin(tmp), end(tmp), back_inserter(m_choice));
	    }
          }
        };
	inline bool operator<(const info_t& x, const info_t& y)
	{
	  const vector<base*>& xb = x.m_base;
	  const vector<base*>& yb = y.m_base;
	  typedef vector<base*>::const_iterator IT;
	  auto virt_base = [](base* bp){ return bp->m_flag & usr::VIRTUAL; };
	  IT xp = find_if(begin(xb), end(xb), virt_base);
	  IT yp = find_if(begin(yb), end(yb), virt_base);
	  if (xp == end(xb) && yp == end(yb))
	    return xb.size() < yb.size();
	  if (xp != end(xb) && yp == end(yb))
	    return false;
	  if (xp == end(xb) && yp != end(yb))
	    return true;
	  assert(xp != end(xb) && yp != end(yb));
	  return distance(begin(xb), xp) < distance(begin(yb), yp);
	}
	bool conflict(base* bx, base* by, vector<route_t>& result)
	{
	  if (bx->m_tag != by->m_tag)
	    return true;
	  if (!(bx->m_flag & usr::VIRTUAL))
	    return true;
	  if (!(by->m_flag & usr::VIRTUAL))
	    return true;
	  tag* ptr = bx->m_tag;
	  const type* T = ptr->m_types.second;
	  assert(T->m_id == type::RECORD);
	  typedef const record_type REC;
	  REC* rec = static_cast<REC*>(T);
	  typedef vector<route_t>::iterator IT;
	  IT p = find(begin(result)+1, end(result), route_t(bx,0));
	  if (p == end(result)) {
	    p = find(begin(result)+1, end(result), route_t(0,rec));
	    assert(p != end(result));
	    return false;
	  }
	  *p = route_t(0, rec);
	  result.erase(--p);
	  return false;
	}
	bool conflict(const info_t& x, const info_t& y, vector<route_t>& xres)
	{
	  const vector<base*>& xb = x.m_base;
	  const vector<base*>& yb = y.m_base;
	  if (xb.size() != yb.size()) {
	    auto virt = [](const base* bp)
	      { return bp->m_flag & usr::VIRTUAL; };
	    typedef vector<base*>::const_iterator IT;
	    IT p = find_if(begin(xb), end(xb), virt);
	    if (p != end(xb))
	      return false;
	    IT q = find_if(begin(yb), end(yb), virt);
	    if (q != end(yb)) {
	       // If `x' is direct base, `x' is conflict with `y'.
	      if (xb.size() != 1)
		return false;
	      if (x.m_kind != y.m_kind)
		error::not_implemented();
	      return x.m_kind == CLASS_NAME_LEX;
	    }
	    return true;
	  }
	  if (x.m_kind != y.m_kind)
	    return true;
	  if (x.m_lval != y.m_lval) {
	    genaddr* xg = x.m_lval->genaddr_cast();
	    genaddr* yg = y.m_lval->genaddr_cast();
	    if (!xg || !yg)
	      return true;
	    var* xr = xg->m_ref;
	    var* yr = yg->m_ref;
	    if (xr != yr)
	      return true;
	    int xo = xg->m_offset;
	    int yo = yg->m_offset;
	    assert(xo == yo);
	  }
	  if (x.m_kind == IDENTIFIER_LEX) {
	    var* v = x.m_lval;
	    if (genaddr* ga = v->genaddr_cast())
	      v = ga->m_ref;
	    usr* u = v->usr_cast();
	    usr::flag_t flag = u->m_flag;
	    usr::flag_t mask = usr::flag_t(usr::ENUM_MEMBER | usr::STATIC);
	    if (flag & mask)
	      return false;
	  }
	  assert(xb.size() == yb.size());
	  typedef vector<base*>::const_reverse_iterator IT;
	  pair<IT, IT> pit(rbegin(xb), rbegin(yb));
	  pair<IT, IT> last(rend(xb), rend(yb));
	  while (pit != last) {
	    pit = mismatch(pit.first, rend(xb), pit.second);
	    if (pit != last) {
	      if (conflict(*pit.first, *pit.second, xres))
		return true;
	      ++pit.first, ++pit.second;
	      assert(pit != last);
	      ++pit.first, ++pit.second;
	      for ( ; pit != last ; ++pit.first, ++pit.second ) {
		if (*pit.first != *pit.second) {
		  base* bx = *pit.first;
		  typedef vector<route_t>::iterator IT;
		  IT p = find(begin(xres), end(xres), route_t(bx,0));
		  if (p != end(xres))
		    xres.erase(p);
		}
	      }
	    }
	  }
	  return false;
	}
	int action(string name, tag* ptr)
	{
	  vector<info_t> choice;
          if (const vector<base*>* bases = ptr->m_bases)
	    for_each(begin(*bases), end(*bases), gather(name, choice));

	  if (choice.empty())
	    return 0;
	  if (choice.size() == 1) {
	    const info_t& x = choice.back();
	    cxx_compiler_lval.m_var = x.m_lval;
	    const vector<base*>& v = x.m_base;
	    transform(begin(v), end(v), back_inserter(route),
		      [](base* bp){
			const record_type* zero = 0;
			return make_pair(bp, zero);
		      });
	    return x.m_kind;
	  }

	  typedef vector<info_t>::const_iterator IT;
	  IT p = min_element(begin(choice), end(choice));
	  assert(p != end(choice));
	  const info_t& x = *p;
	  const vector<base*>& v = x.m_base;
	  vector<route_t> xres;
	  transform(begin(v), end(v), back_inserter(xres), [](base* bp) {
		      const record_type* zero = 0;
		      return make_pair(bp, zero);
		    });
	  IT q = find_if(begin(choice), end(choice),
			 [&x, &xres](const info_t& y)
			 { return &x == &y ? false : conflict(x,y,xres); } );
	  if (q != end(choice)) {
	    const info_t&  y = *q;
	    using namespace error::base_lookup;
	    ambiguous(parse::position, name, x.m_base, y.m_base);
	  }
	  copy(begin(xres), end(xres), back_inserter(route));
	  cxx_compiler_lval.m_var = x.m_lval;
	  return x.m_kind;
	}
        vector<route_t> route;
      }  // end of namespace base_lookup
    }  // end of namespace identifier
  }  // end of namespace parse
}  // end of namespace cxx_compiler

int
cxx_compiler::parse::identifier::lookup(std::string name, scope* ptr)
{
  using namespace std;
  const map<string, tag*>& tpsf = ptr->m_tps.first;
  map<string, tag*>::const_iterator r = tpsf.find(name);
  if (r != tpsf.end()) {
    cxx_compiler_lval.m_tag = r->second;
    return CLASS_NAME_LEX;
  }
  const map<string, vector<usr*> >& usrs = ptr->m_usrs;
  map<string, vector<usr*> >::const_iterator p = usrs.find(name);
  if (p != usrs.end()) {
    const vector<usr*>& v = p->second;
    usr* u = v.back();
    cxx_compiler_lval.m_usr = u;
    usr::flag_t flag = u->m_flag;
    if (flag & usr::TYPEDEF) {
      if (mode == member || mode == mem_ini) {
	const type* T = u->m_type;
	if (tag* ptr = T->get_tag()) {
	  cxx_compiler_lval.m_tag = ptr;
	  return CLASS_NAME_LEX;
	}
      }
      type_def* tdef = static_cast<type_def*>(u);
      tdef->m_refed.push_back(parse::position);
      return TYPEDEF_NAME_LEX;
    }
    if (flag & usr::CTOR)
      return lookup(name, ptr->m_parent);
    if (flag & usr::OVERLOAD) {
      overload* ovl = static_cast<overload*>(u);
      const vector<usr*>& v = ovl->m_candidacy;
      assert(!v.empty());
      usr* uu = v.back();
      if (uu->m_flag & usr::CTOR)
	return lookup(name, ptr->m_parent);
      return IDENTIFIER_LEX;
    }
    if (flag & usr::NAMESPACE) {
      cxx_compiler_lval.m_name_space = static_cast<name_space*>(u);
      return ORIGINAL_NAMESPACE_NAME_LEX;
    }
    const type* T = u->m_type;
    if (const pointer_type* G = T->ptr_gen())
      garbage.push_back(cxx_compiler_lval.m_var = new genaddr(G,T,u,0));
    return IDENTIFIER_LEX;
  }
  const map<string, tag*>& tags = ptr->m_tags;
  map<string, tag*>::const_iterator q = tags.find(name);
  if (q != tags.end()) {
    tag* ptag = q->second;
    cxx_compiler_lval.m_tag = q->second;
    if (ptag->m_kind == tag::ENUM)
      return ENUM_NAME_LEX;
    if (!ptag->m_template)
      return CLASS_NAME_LEX;
    template_tag* tt = static_cast<template_tag*>(ptag);
    return tt->m_specified ? TEMPLATE_NAME_LEX : CLASS_NAME_LEX;
  }
  if (mode == member) {
    if (scope::current->m_id == scope::TAG) {
      tag* ptag = static_cast<tag*>(scope::current);
      if (ptag->m_name == name) {
	cxx_compiler_lval.m_tag = ptag;
	return CLASS_NAME_LEX;
      }
      if (int r = base_lookup::action(name, ptag))
	return r;
    }
    if (ptr->m_parent) {
      if (int r = lookup(name, ptr->m_parent)) {
	return r;
      }
    }
    error::undeclared(parse::position,name);
    int r = create(name,int_type::create());
    usr* u = cxx_compiler_lval.m_usr;
    scope::current->m_usrs[name].push_back(u);
    return r;
  }

  if (ptr->m_id == scope::TAG) {
    tag* ptag = static_cast<tag*>(ptr);
    if (int n = base_lookup::action(name, ptag))
      return n;
  }
  if (ptr->m_parent)
    return lookup(name,ptr->m_parent);

  if (mode == no_err || mode == canbe_ctor)
    return 0;

  if ( name == "__func__" )
    return underscore_func::action();
  if (last_token == '(' && scope::current->m_id == scope::PARAM)
    return 0;
  if ( peek() == ':' ) // for labeled-statement
    return create(name);
  if (last_token == NAMESPACE_KW)
    return create(name);
  error::undeclared(parse::position,name);
  int ret = create(name,int_type::create());
  usr* u = cxx_compiler_lval.m_usr;
  scope::current->m_usrs[name].push_back(u);
  return ret;
}

cxx_compiler::parse::read_t cxx_compiler::parse::g_read;

int cxx_compiler::parse::peek()
{
  using namespace std;
  if ( !g_read.m_token.empty() ){
    parse::position = g_read.m_token.front().second;
    return g_read.m_token.front().first;
  }

  if (member_function_body::saved) {
    const list<pair<int, file_t> >& token =
      member_function_body::saved->m_read.m_token;
    assert(!token.empty());
    parse::position = token.front().second;
    return token.front().first;
  }

  if (templ::ptr) {
    const list<pair<int, file_t> >& token = templ::ptr->m_read.m_token;
    assert(!token.empty());
    parse::position = token.front().second;
    return token.front().first;
  }

  identifier::mode_t org = identifier::mode;
  var* org2 = cxx_compiler_lval.m_var;

  identifier::mode = identifier::peeking;
  int r = lex_and_save();
  identifier::mode = org;
  cxx_compiler_lval.m_var = org2;
  return r;
}

namespace cxx_compiler {
  namespace parse {
    using namespace std;
    void save_common(int n, list<void*>& lval, list<void*>* src = 0)
    {
      switch (n) {
      case IDENTIFIER_LEX:
      case PEEKED_NAME_LEX:
      case INTEGER_LITERAL_LEX:
      case CHARACTER_LITERAL_LEX:
      case FLOATING_LITERAL_LEX:
      case TYPEDEF_NAME_LEX:
      case STRING_LITERAL_LEX:
      case CLASS_NAME_LEX:
      case ENUM_NAME_LEX:
      case DEFAULT_KW:
      case ORIGINAL_NAMESPACE_NAME_LEX:
      case NAMESPACE_ALIAS_LEX:
      case TEMPLATE_NAME_LEX:
        if (src) {
          assert(!src->empty());
          lval.push_back(src->front());
          src->pop_front();
        }
        else
          lval.push_back(cxx_compiler_lval.m_var);
        break;
      }
    }
    inline int get_id_from_mem_fun_body(int n)
    {
      assert(n == PEEKED_NAME_LEX || n == IDENTIFIER_LEX);
      var* v = cxx_compiler_lval.m_var;
      usr* u = v->usr_cast();
      if (!u) {
        assert(v->genaddr_cast());
        return IDENTIFIER_LEX;
      }
      return identifier::judge(u->m_name);
    }
    int get_common(int n, list<void*>& lval, bool from_mem_fun_body,
		   bool templ)
    {
      switch (n) {
      case PEEKED_NAME_LEX:
	assert(!templ);
        assert(!lval.empty());
        cxx_compiler_lval.m_var = static_cast<var*>(lval.front());
        lval.pop_front();
        if (from_mem_fun_body)
          return get_id_from_mem_fun_body(n);
        {
          usr* u = static_cast<usr*>(cxx_compiler_lval.m_var);
          assert(u->m_type->m_id == type::BACKPATCH);
          string name = u->m_name;
          last_token = identifier::judge(name);
          delete u;
        }
        return n;
      case IDENTIFIER_LEX:
        assert(!lval.empty());
        cxx_compiler_lval.m_var = static_cast<var*>(lval.front());
        lval.pop_front();
        if (from_mem_fun_body)
          return get_id_from_mem_fun_body(n);
	if (templ) {
	  var* v = cxx_compiler_lval.m_var;
	  assert(v->usr_cast());
          usr* u = static_cast<usr*>(v);
          string name = u->m_name;
	  last_token = identifier::judge(name);
	}
        else if (context_t::retry[DECL_FCAST_CONFLICT_STATE] ||
		 context_t::retry[TYPE_NAME_CONFLICT_STATE]) {
          usr* u = static_cast<usr*>(cxx_compiler_lval.m_var);
          assert(u->m_type->backpatch());
          string name = u->m_name;
          last_token = identifier::lookup(name, scope::current);
          delete u;
        }
        return n;
      case INTEGER_LITERAL_LEX:
      case CHARACTER_LITERAL_LEX:
      case FLOATING_LITERAL_LEX:
      case TYPEDEF_NAME_LEX:
        assert(!lval.empty());
        cxx_compiler_lval.m_usr = static_cast<usr*>(lval.front());
        lval.pop_front();
        return n;
      case STRING_LITERAL_LEX:
        assert(!lval.empty());
        cxx_compiler_lval.m_var = static_cast<var*>(lval.front());
        lval.pop_front();
	if (context_t::retry[DECLARATOR_ID_CONFLICT_STATE])
	  cxx_compiler_lval.m_var->m_scope = scope::current;
        return n;
      case CLASS_NAME_LEX:
      case ENUM_NAME_LEX:
      case TEMPLATE_NAME_LEX:
        assert(!lval.empty());
        cxx_compiler_lval.m_tag = static_cast<tag*>(lval.front());
        lval.pop_front();
	if (templ) {
	  if (cxx_compiler_lval.m_tag == template_tag::current)
	    cxx_compiler_lval.m_tag = template_tag::result;
	}
        return n;
      case DEFAULT_KW:
        assert(!lval.empty());
        cxx_compiler_lval.m_file = static_cast<file_t*>(lval.front());
        lval.pop_front();
        return n;
      case ORIGINAL_NAMESPACE_NAME_LEX:
      case NAMESPACE_ALIAS_LEX:
        assert(!lval.empty());
        cxx_compiler_lval.m_name_space =
          static_cast<name_space*>(lval.front());
        lval.pop_front();
        return n;
      default:
        return n;
      }
    }
  } // end of namespace parse
} // end of namespace cxx_compiler

int cxx_compiler::parse::lex_and_save()
{
  using namespace std;
  int n = cxx_compiler_lex();
  g_read.m_token.push_back(make_pair(n,position));
  save_common(n , g_read.m_lval);
  return n;
}

namespace cxx_compiler {
  namespace parse {
    inline void save_each(context_t& x)
    {
      x.m_read.m_token.push_back(make_pair(last_token,position));
      save_common(last_token, x.m_read.m_lval);
    }
    inline int save_for_retry()
    {
      for_each(begin(context_t::all), end(context_t::all), save_each);
      if (!templ::save_t::s_stack.empty()) {
	templ::save_t* p = templ::save_t::s_stack.top();
	read_t& r = p->m_read;
	r.m_token.push_back(make_pair(last_token, parse::position));
	save_common(last_token, r.m_lval);
      }
      return last_token;
    }
    namespace templ {
      templ_base* ptr;
    } // end of namespac templ
  } // end of namesapce parse
} // end of namesapce cxx_compiler

int cxx_compiler::parse::get_token()
{
  if (!g_read.m_token.empty()) {
    position = g_read.m_token.front().second;
    last_token = g_read.m_token.front().first;
    g_read.m_token.pop_front();
    get_common(last_token, g_read.m_lval, false, false);
    if (last_token == COLONCOLON_MK) {
      if (scope::current->m_id != scope::TAG && peek() != '*')
        identifier::mode = identifier::look;
    }
    return save_for_retry();
  }

  if (member_function_body::saved) {
    last_token = member_function_body::get_token();
    if (last_token == COLONCOLON_MK)
      identifier::mode = identifier::look;
    return save_for_retry();
  }

  if (templ::ptr) {
    const read_t& r = templ::ptr->m_read;
    last_token = templ::get_token();
    if (last_token == COLONCOLON_MK)
      identifier::mode = identifier::look;
    return save_for_retry();
  }

  last_token = cxx_compiler_lex();
  if (last_token == COLONCOLON_MK)
    identifier::mode = identifier::look;

  return save_for_retry();
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

bool cxx_compiler::parse::is_last_decl = true;

void cxx_compiler::parse::parameter::enter()
{
  using namespace std;
  if (class_or_namespace_name::last) {
    using namespace class_or_namespace_name;
    assert(!before.empty());
    before.back() = scope::current;
    scope::current = last;
    last = 0;
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
  switch (scope::current->m_id) {
  case scope::NONE:
  case scope::TAG:
  case scope::NAMESPACE:
    return;
  }
  using namespace class_or_namespace_name;
  assert(!before.empty());
  assert(before.back() == scope::current);
  before.pop_back();
  scope* org = scope::current;
  scope::current = scope::current->m_parent;
  vector<scope*>& children = scope::current->m_children;
  scope::id_t id = scope::current->m_id;
  switch (id) {
  case scope::NONE:
  case scope::TAG:
  case scope::NAMESPACE:
    if (children.size() > 1) {
      /*
       * void (*f(int a))(float a)
       * {
       *   ...
       * } 
       */
      assert(children.back() == org);
      scope* ptr = children[children.size()-2];
      if (ptr->m_id == scope::PARAM) {
        children.pop_back();
        delete org;
      }
    }
    break;
  default:
    assert(!children.empty());
    assert(children.back() == org);
    children.pop_back();
    delete org;
    break;
  }
}

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
  inline void decide_dim()
  {
    using namespace std;
    if (scope::current->m_id != scope::PARAM)
      return;
    const vector<usr*>& o = scope::current->m_order;
    for (auto u : o)
      u->m_type->decide_dim();
  }
} } } // end of namespace parameter, parse and cxx_compiler

namespace cxx_compiler { namespace parse { namespace block {
  void new_block();
} } } // end of namespace block, parse and cxx_compiler

void cxx_compiler::parse::block::enter()
{
  using namespace std;

  scope::id_t id = scope::current->m_id;
  switch (id) {
  case scope::NONE: case scope::NAMESPACE:
    {
      vector<scope*>& children = scope::current->m_children;
      typedef vector<scope*>::const_iterator IT;
      IT p = find_if(begin(children), end(children),
                     bind2nd(ptr_fun(cmp), scope::PARAM));
      assert(p != end(children));
      scope::current = *p;
      IT end = children.end();
      assert(find_if(++p, end, bind2nd(ptr_fun(cmp), scope::PARAM)) == end);
      return parameter::decide_dim(), new_block(), parameter::move();
    }
  case scope::TAG:
    {
      tag* ptr = static_cast<tag*>(scope::current);
      const type* T = ptr->m_types.second;
      vector<scope*>& children = scope::current->m_children;
      assert(!children.empty());
      scope::current = children.back();
      usr* func = fundef::current->m_usr;
      assert(T);
      if (!(func->m_flag & usr::STATIC)) {
	map<string, vector<usr*> >& usrs = scope::current->m_usrs;
	typedef map<string, vector<usr*> >::const_iterator IT;
	IT p = usrs.find(this_name);
	if (p == usrs.end()) {
	  const type* pt = pointer_type::create(T);
	  usr* this_ptr = new usr(this_name, pt, usr::NONE, parse::position,
				  usr::NONE2);
	  usrs[this_name].push_back(this_ptr);
	  vector<usr*>& order = scope::current->m_order;
	  vector<usr*> tmp = order;
	  order.clear();
	  order.push_back(this_ptr);
	  copy(begin(tmp), end(tmp), back_inserter(order));
	}
      }
      vector<scope*>& c = scope::current->m_children;
      if (!c.empty()) {
        assert(c.size() == 1);
        scope::current = c.back();
        using namespace class_or_namespace_name;
        before.push_back(scope::current);
        return;
      }
      return parameter::decide_dim(), new_block(), parameter::move();
    }
  default:
    return new_block();
  }
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
  using namespace class_or_namespace_name;
  assert(!before.empty());
  assert(scope::current == before.back());
  before.pop_back();
  scope::current = scope::current->m_parent;
  if (scope::current->m_parent == &scope::root)
    scope::current = &scope::root;
  else {
    scope::id_t id = scope::current->m_parent->m_id;
    if (id == scope::TAG || id == scope::NAMESPACE)
      scope::current = scope::current->m_parent;
  }
}

namespace cxx_compiler { namespace parse { namespace member_function_body {
  std::map<usr*, save_t> stbl;
  save_t* saved;
  void save_brace(usr*);
} } } // end of namespace parse, member_function_body and cxx_compiler

void cxx_compiler::parse::member_function_body::save(usr* key)
{
  using namespace std;
  if (key->m_type->m_id != type::FUNC)
    return;
  assert(scope::current->m_id == scope::TAG);
  tag* ptr = static_cast<tag*>(scope::current);
  const type* T = ptr->m_types.second;
  assert(!T);
  vector<scope*>& children = scope::current->m_children;
  assert(!children.empty());
  scope::current = children.back();
  children.pop_back();
  scope* param = scope::current;
  assert(param->m_id == scope::PARAM);

  parameter::decide_dim(), block::new_block(), parameter::move();
  class_or_namespace_name::before.pop_back();

  assert(!(key->m_flag & usr::OVERLOAD));
  key->m_flag = usr::flag_t(key->m_flag | usr::INLINE);

  stbl[key].m_param = param;
  stbl[key].m_read.m_token.push_back(make_pair(cxx_compiler_char,position));
  identifier::mode = identifier::new_obj;
  save_brace(key);
  identifier::mode = identifier::look;
  scope::current = ptr;
}

void cxx_compiler::parse::member_function_body::save_brace(usr* key)
{
  using namespace std;
  save_t& tmp = stbl[key];
  list<pair<int, file_t> >& token = tmp.m_read.m_token;
  list<void*>& lval = tmp.m_read.m_lval;
  while (1) {
    int n;
    if (!g_read.m_token.empty()) {
      position = g_read.m_token.front().second;
      n = g_read.m_token.front().first;
      g_read.m_token.pop_front();
      token.push_back(make_pair(n,position));
      save_common(n, lval, &g_read.m_lval);
    }
    else {
      n = cxx_compiler_lex();
      if (n == 0) // YYEOF
	break;
      token.push_back(make_pair(n,position));
      save_common(n, lval);
    }

    if (n == '{') {
      save_brace(key);
      break;
    }
    if (n == '}')
      break;
  }
}

int cxx_compiler::parse::member_function_body::get_token()
{
  using namespace std;
  list<pair<int, file_t> >& token = saved->m_read.m_token;
  if (token.empty())
    return 0; // YYEOF
  position = token.front().second;
  int n = token.front().first;
  token.pop_front();
  list<void*>& lval = saved->m_read.m_lval;
  return get_common(n, lval, true, false);
}

int cxx_compiler::parse::templ::get_token()
{
  using namespace std;
  list<pair<int, file_t> >& token = ptr->m_read.m_token;
  if (token.empty())
    return 0; // YYEOF
  position = token.front().second;
  int n = token.front().first;
  token.pop_front();
  list<void*>& lval = ptr->m_read.m_lval;
  return get_common(n, lval, false, true);
}

int cxx_compiler::parse::last_token;

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

namespace cxx_compiler {
  namespace parse {
    vector<context_t> context_t::all;
    map<int, bool> context_t::retry;
    void save(int state, short* b0, short* t0, YYSTYPE* b1, YYSTYPE* t1)
    {
      using namespace std;
      vector<short> vs;
      copy(b0, t0-1, back_inserter(vs));
      vector<void*> vv;
      for (; b1 != t1; ++b1) {
        YYSTYPE tmp = *b1;
        vv.push_back(tmp.m_base_clause);
      }
      context_t::all.push_back(context_t(state, vs, vv, cxx_compiler_char));
    }
    void restore(int* state, short** b0, short** t0, short* a0,
                 YYSTYPE** b1, YYSTYPE** t1, YYSTYPE* a1)
    {
      assert(!context_t::all.empty());
      const context_t& x = context_t::all.back();
      *state = x.m_state;
      copy(begin(x.m_stack0), end(x.m_stack0), a0);
      *b0 = a0;
      *t0 = a0 + x.m_stack0.size();
      void** dst = reinterpret_cast<void**>(a1);
      for (auto vp : x.m_stack1)
        *dst++ = vp;
      *b1 = a1;
      *t1 = a1 + x.m_stack1.size();
      cxx_compiler_char = x.m_char;
      read_t tmp = g_read;
      g_read = x.m_read;
      copy(begin(tmp.m_token), end(tmp.m_token),
	   back_inserter(g_read.m_token));
      copy(begin(tmp.m_lval), end(tmp.m_lval), back_inserter(g_read.m_lval));
      context_t::all.pop_back();
      if (!templ::save_t::s_stack.empty()) {
	templ::save_t* p = templ::save_t::s_stack.top();
	read_t& r = p->m_read;
	for (int i = 0 ; i != g_read.m_token.size() ; ++i)
	  r.m_token.pop_back();
      }
    }
  } // end of namesapce parse
} // end of namesapce cxx_compiler


namespace cxx_compiler {
  namespace parse {
    namespace templ {
      stack<save_t*> save_t::s_stack;
    } // end of namespace templ
  } // end of namespace parse
} // end of namespace cxx_compiler

namespace cxx_compiler {
  namespace parse {
    // called from debugger command line
    void debug_read(const read_t& r)
    {
      const list<pair<int, file_t> >& ls = r.m_token;
      for (auto p : ls)
	cout << p.first << ' ';
      cout << endl;
    }
  } // end of namespace parse
} // end of namespace cxx_compiler

