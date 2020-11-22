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
      int typenaming;
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
      int create_templ(std::string);
    } // end of namespace identifier
  } // end of namespace parse and
} // end of namespace cxx_compiler

int cxx_compiler::parse::identifier::judge(std::string name)
{
  if (mode == peeking)
    return create(name), PEEKED_NAME_LEX;

  if (last_token == COLONCOLON_MK || peek() == COLONCOLON_MK) {
    if (int r = lookup(name, scope::current)) {
      if (r == IDENTIFIER_LEX) {
        var* v = cxx_compiler_lval.m_var;
        if (genaddr* ga = v->genaddr_cast()) {
          assert(!class_or_namespace_name::before.empty());
          ga->m_scope = class_or_namespace_name::before.back();
        }
      }
      return r;
    }
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

  if (mode == new_obj && peek() == '<') {
    int r = lookup(name, scope::current);
    if (r == TEMPLATE_NAME_LEX) {
      mode = look;
      return r;
    }
  }

  if (mode == new_obj || mode == canbe_ctor)
    return create(name);

  if (mode == templ_name && peek() == '<')
    return create_templ(name);

  if (int r = lookup(name, search_scope())) {
    if (context_t::retry[DECL_FCAST_CONFLICT_STATE])
      return r;
    if (templ::arg)
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

int cxx_compiler::parse::identifier::create_templ(std::string name)
{
  tag* ptr = new tag(tag::TEMPL, name, parse::position, 0);
  ptr->m_parent = scope::current;
  ptr->m_types.first = incomplete_tagged_type::create(ptr);
  scope::tps_t dummy;
  template_tag* tt = new template_tag(*ptr, dummy);
  tt->m_created = true;
  cxx_compiler_lval.m_ut = new pair<usr*, tag*>(0, tt);
  map<string, tag*>& tags = scope::current->m_tags;
  assert(tags.find(name) == tags.end());
  tags[name] = tt;
  scope::current->m_children.push_back(tt);
  return TEMPLATE_NAME_LEX;
}

namespace cxx_compiler {
  namespace parse {
    namespace identifier {
      struct templ_param : usr {
        templ_param(string name, const type* T, flag_t flag,
                    const file_t& file, flag2_t flag2)
          : usr(name, T, flag, file, flag2) {}
        bool isconstant(bool) const { return true; }
        __int64 value() const { return 1; }
      };
      inline int templ_param_lex(string name,
                		 const scope::tps_t::value_t& x,
                		 bool instantiate)
      {
        if (tag* ptr = x.first) {
          cxx_compiler_lval.m_tag = ptr;
          return CLASS_NAME_LEX;
        }
        scope::tps_t::val2_t* y = x.second;
        assert(y);
        const type* T = y->first;
        assert(T);
        var* v = y->second;
        if (!v) {
          assert(!instantiate);
          usr::flag2_t flag2 = usr::TEMPL_PARAM;
          usr* u = new templ_param(name, T, usr::NONE, parse::position, flag2);
          cxx_compiler_lval.m_usr = u;
          return IDENTIFIER_LEX;
        }
        assert(instantiate);
        if (!v->isconstant(true))
          error::not_implemented();
        if (T->integer()) {
          assert(v->usr_cast());
          usr* u = static_cast<usr*>(v);
          T = T->unqualified();
          type::id_t id = T->m_id;
          if (id == type::BOOL) {
            string name = u->m_name;
            if (name == "false")
              return FALSE_KW;
            assert(name == "true");
            return TRUE_KW;
          }
          cxx_compiler_lval.m_usr = u;
          if (id == type::CHAR || id == type::WCHAR)
            return CHARACTER_LITERAL_LEX;
          return INTEGER_LITERAL_LEX;
        }
        if (T->arithmetic()) {
          assert(v->usr_cast());
          usr* u = static_cast<usr*>(v);
          cxx_compiler_lval.m_usr = u;
          return FLOATING_LITERAL_LEX;
        }
        cxx_compiler_lval.m_var = v;
        return IDENTIFIER_LEX;
      }
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

            if (parse::identifier::mode != mem_ini) {
              if (lookup(ptr->m_usrs, bp))
                return;
            }
            
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
      int templ_usr_param(string name, scope* ptr)
      {
        if (template_usr::nest.empty())
          return 0;
        template_usr::info_t& info = template_usr::nest.back();
        const template_usr* tu = info.m_tu;
        if (tu->m_scope != ptr)
          return 0;
        const scope::tps_t& tps = tu->m_tps;
        const map<string, scope::tps_t::value_t>& table = tps.m_table;
        typedef map<string, scope::tps_t::value_t>::const_iterator IT;
        IT p = table.find(name);
        if (p == table.end())
          return 0;
        return templ_param_lex(name, p->second, true);
      }
    }  // end of namespace identifier
  }  // end of namespace parse
}  // end of namespace cxx_compiler

namespace cxx_compiler {
  namespace parse {
    namespace identifier {
      using namespace std;
      int get_here(string name, scope* ptr)
      {
        const map<string, vector<usr*> >& usrs = ptr->m_usrs;
        map<string, vector<usr*> >::const_iterator p = usrs.find(name);
        if (p != usrs.end()) {
          const vector<usr*>& v = p->second;
          usr* u = v.back();
          cxx_compiler_lval.m_usr = u;
          if (u->m_flag2 & usr::ALIAS) {
            alias* al = static_cast<alias*>(u);
            u = al->m_org;
          }
          usr::flag_t flag = u->m_flag;
          if (flag & usr::TYPEDEF) {
            type_def* tdef = static_cast<type_def*>(u);
            tdef->m_refed.push_back(parse::position);
            if (mode == mem_ini || peek() == COLONCOLON_MK) {
              const type* T = u->m_type;
              if (tag* ptr = T->get_tag()) {
                cxx_compiler_lval.m_tag = ptr;
                return CLASS_NAME_LEX;
              }
            }
            return TYPEDEF_NAME_LEX;
          }
          if (flag & usr::CTOR) {
            assert(ptr->m_id == scope::TAG);
            tag* ptag = static_cast<tag*>(ptr);
            if ((ptag->m_flag & tag::INSTANTIATE) && peek() == '<')
              return lookup(name, ptr->m_parent);
            return lookup(ptag->m_name, ptr->m_parent);
          }
          if (flag & usr::OVERLOAD) {
            overload* ovl = static_cast<overload*>(u);
            const vector<usr*>& v = ovl->m_candidacy;
            assert(!v.empty());
            usr* uu = v.back();
            if (uu->m_flag & usr::CTOR) {
              assert(ptr->m_id == scope::TAG);
              tag* ptag = static_cast<tag*>(ptr);
              if ((ptag->m_flag & tag::INSTANTIATE) && peek() == '<')
                return lookup(name, ptr->m_parent);
              return lookup(ptag->m_name, ptr->m_parent);
            }
            if (peek() == '<') {
              typedef vector<usr*>::const_iterator IT;
              IT p = find_if(begin(v), end(v),
                	     [](usr* u){ return u->m_flag2 & usr::TEMPLATE; });
              if (p != end(v)) {
                cxx_compiler_lval.m_ut = new pair<usr*, tag*>(*p, 0);
                return TEMPLATE_NAME_LEX;
              }
            }
            if (mode == new_obj)
              return create(name);
            if (!declarations::specifier_seq::info_t::s_stack.empty()) {
              if (declarations::specifier_seq::info_t::s_stack.top())
                return create(name);
            }
            return IDENTIFIER_LEX;
          }
          if (flag & usr::NAMESPACE) {
            cxx_compiler_lval.m_name_space = static_cast<name_space*>(u);
            return ORIGINAL_NAMESPACE_NAME_LEX;
          }
          usr::flag2_t flag2 = u->m_flag2;
          if (flag2 & usr::TEMPLATE) {
            if (peek() == '<') {
              cxx_compiler_lval.m_ut = new pair<usr*, tag*>(u, 0);
              return TEMPLATE_NAME_LEX;
            }
            if (mode == new_obj)
              return create(name);

	    // If function definition like
	    // template<class C> void f(C x) { ... }
	    // create(`f')
	    if (!tinfos.empty()) {
	      typedef template_usr::info_t X;
	      typedef template_tag::info_t Y;
	      pair<X*, Y*> p = tinfos.back();
	      if (X* x = p.first) {
		template_usr* tu = x->m_tu;
		if (tu->m_name == name)
		  return create(name);
	      }
	    }
          }
          if (flag2 & usr::PARTIAL_ORDERING) {
            if (mode == new_obj)
              return create(name);
            if (!declarations::specifier_seq::info_t::s_stack.empty()) {
              if (declarations::specifier_seq::info_t::s_stack.top())
                return create(name);
            }
            return IDENTIFIER_LEX;
          }
          const type* T = u->m_type;
          if (const pointer_type* G = T->ptr_gen()) {
            cxx_compiler_lval.m_var = new genaddr(G,T,u,0);
            if (template_usr::nest.empty())
              garbage.push_back(cxx_compiler_lval.m_var);
          }
          return IDENTIFIER_LEX;
        }
        const map<string, tag*>& tags = ptr->m_tags;
        map<string, tag*>::const_iterator q = tags.find(name);
        if (q != tags.end()) {
          tag* ptag = q->second;
          cxx_compiler_lval.m_tag = ptag;
          if (ptag->m_kind == tag::ENUM)
            return ENUM_NAME_LEX;
          if (!(ptag->m_flag & tag::TEMPLATE))
            return CLASS_NAME_LEX;
          if (peek() != '<')
            return CLASS_NAME_LEX;
          cxx_compiler_lval.m_ut = new pair<usr*, tag*>(0, ptag);
          return TEMPLATE_NAME_LEX;
        }
        return 0;
      }
    } // end of namespace identifier
  } // end of namespace parse
  namespace inline_namespace {
    using namespace std;
    bool helper(string name, scope* ptr, int *res)
    {
      if (ptr->m_id != scope::NAMESPACE)
        return false;
      name_space* ns = static_cast<name_space*>(ptr);
      usr::flag_t flag = ns->m_flag;
      if (!(flag & usr::INLINE))
        return false;
      return *res = parse::identifier::get_here(name, ns);
    }
    int lookup(string name, scope* ptr)
    {
      const vector<scope*>& c = ptr->m_children;
      typedef vector<scope*>::const_iterator IT;
      int n = 0;
      IT p = find_if(begin(c), end(c),
                     [name, &n](scope* ps){ return helper(name, ps, &n); });
      if (p != end(c))
        return n;
      if (scope* parent = ptr->m_parent)
        return lookup(name, parent);
      return 0;
    }
  } // end of namespace inline_namespace
  int builtin_memcmp_entry()
  {
    map<string, vector<usr*> >& usrs = scope::root.m_usrs;
    string name = "memcmp";
    typedef map<string, vector<usr*> >::const_iterator IT;
    IT p = usrs.find(name);
    if (p != usrs.end()) {
      const vector<usr*>& vec = p->second;
      usr* u = vec.back();
      const type* T = u->m_type;
      const pointer_type* pt = T->ptr_gen();
      assert(pt);
      var* v = new genaddr(pt, T, u, 0);
      cxx_compiler_lval.m_var = v;
      garbage.push_back(v);
      return IDENTIFIER_LEX;
    }
    const type* T = void_type::create();
    T = const_type::create(T);
    const pointer_type* pcv = pointer_type::create(T);
    vector<const type*> param;
    param.push_back(pcv);
    param.push_back(pcv);
    param.push_back(sizeof_type());
    const func_type* ft = func_type::create(int_type::create(), param);
    usr::flag_t flag = usr::flag_t(usr::FUNCTION | usr::C_SYMBOL);
    usr* u = new usr(name, ft, flag, parse::position, usr::GENED_BY_COMP);
    usrs[name].push_back(u);
    const pointer_type* pt = pointer_type::create(ft);
    var* v = new genaddr(pt, ft, u, 0);
    cxx_compiler_lval.m_var = v;
    garbage.push_back(v);
    return IDENTIFIER_LEX;
  }
} // end of namespace cxx_compiler

int
cxx_compiler::parse::identifier::lookup(std::string name, scope* ptr)
{
  using namespace std;
  const vector<scope::tps_t>& tps = ptr->m_tps;
  typedef vector<scope::tps_t>::const_reverse_iterator IT;
  int n = 0;
  IT r = find_if(rbegin(tps), rend(tps),
                 [&n, name](const scope::tps_t& x)
                 {
                   const map<string, scope::tps_t::value_t>& table = x.m_table;
                   map<string, scope::tps_t::value_t>::const_iterator p =
                   table.find(name);
                   if (p == table.end())
                     return false;
                   n = templ_param_lex(name, p->second, false);
                   return n != 0;
                 });
  if (r != rend(tps))
    return n;

  if (int r = templ_usr_param(name, ptr))
    return r;

  if (int r = get_here(name, ptr))
    return r;

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
    using namespace expressions::postfix;
    assert(!member::handling.empty());
    member::info_t* info = member::handling.top();
    mode = no_err;
    int r = lookup(name, info->m_scope);
    if (r == TYPEDEF_NAME_LEX) {
      usr* u = cxx_compiler_lval.m_usr;
      const type* T = u->m_type;
      if (tag* ptr = T->get_tag()) {
        cxx_compiler_lval.m_tag = ptr;
        return CLASS_NAME_LEX;
      }
    }
    if (r == CLASS_NAME_LEX)
      return r;
    return 0;
  }

  if (ptr->m_id == scope::TAG) {
    tag* ptag = static_cast<tag*>(ptr);
    if (int n = base_lookup::action(name, ptag))
      return n;
    if (parse::templ::ptr) {
      if (ptag->m_flag & tag::INSTANTIATE) {
        instantiated_tag* it = static_cast<instantiated_tag*>(ptag);
        template_tag* src = it->m_src;
        const map<string, scope::tps_t::value_t>& table =
          src->templ_base::m_tps.m_table;
        map<string, scope::tps_t::value_t>::const_iterator p =
          table.find(name);
        if (p != table.end())
          return templ_param_lex(name, p->second, true);
      }
    }
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
  if (mode == new_obj)
    return create(name);
  if (last_token == COLONCOLON_MK) {
    if (typenaming)
      return create(name);
    if (!parse::templ::save_t::nest.empty()) {
      if (scope::current->m_id == scope::TAG) {
        tag* ptr = static_cast<tag*>(scope::current);
        const type* T = ptr->m_types.first;
        if (T->m_id == type::TEMPLATE_PARAM) {
          int r = create(name);
          usr* u = cxx_compiler_lval.m_usr;
          u->m_type = int_type::create();
          return r;
        }
      }
    }
  }
  if (last_token != COLONCOLON_MK) {
    if (int r = inline_namespace::lookup(name, scope::current))
      return r;
  }
  const vector<name_space*>& us = scope::current->m_using;
  typedef vector<name_space*>::const_iterator ITy;
  int x = 0;
  ITy p = find_if(begin(us), end(us),
		  [&x, name](name_space* ns){ return x = lookup(name, ns); });
  if (p != end(us))
    return x;
  if (name == "__builtin_memcmp")
    return builtin_memcmp_entry();
  error::undeclared(parse::position, name);
  int ret = create(name, int_type::create());
  usr* u = cxx_compiler_lval.m_usr;
  scope::current->m_usrs[name].push_back(u);
  return ret;
}

cxx_compiler::parse::read_t cxx_compiler::parse::g_read;

int cxx_compiler::parse::peek()
{
  using namespace std;
  if (!g_read.m_token.empty()) {
    parse::position = g_read.m_token.front().second;
    return g_read.m_token.front().first;
  }

  if (member_function_body::saved) {
    const list<pair<int, file_t> >& token =
      member_function_body::saved->m_read.m_token;
    if (token.empty())
      return 0;  // YYEOF
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
    namespace templ_colon_impl {
      typedef list<pair<int, file_t> >::const_iterator IT;
      int get(IT& p, IT finish)
      {
        if (p != finish) {
          int c = p->first;
          ++p;
          return c;
        }
        return lex_and_save();
      }
      int skip(IT& p, IT finish)
      {
        int c;
        while ((c = get(p, finish)) != EOF) {
          if (c == '>')
            break;
          if (c == '<') {
            c = skip(p, finish);
            if (c != '>')
              error::not_implemented();
          }
        }
        return c;
      }
      bool common_case()
      {
        const list<pair<int, file_t> >& token = g_read.m_token;
        IT p = token.begin();
        IT finish = token.end();

        assert(p != finish);
        if (p->first != '<')
          return false;
        ++p;

        identifier::mode_t org = identifier::mode;
        var* org2 = cxx_compiler_lval.m_var;
        identifier::mode = identifier::peeking;

        int c = skip(p, finish);
        if (c != '>')
          error::not_implemented();

        c = get(p, finish);

        identifier::mode = org;
        cxx_compiler_lval.m_var = org2;

        return c == COLONCOLON_MK;
      }
      typedef list<pair<int, file_t> >::const_iterator IT;
      IT match(IT p, IT q)
      {
	assert(p != q);
	if (p->first != '<')
	  return q;
        ++p;
	assert(p != q);
        for ( ; p->first != EOF ; ++p, assert(p != q)) {
          if (p->first == '>')
            break;
          if (p->first == '<')
	    p = match(p, q);
        }
        if (p->first != '>')
          error::not_implemented();
        ++p;
	return p;
      }
      bool templ_ptr_case()
      {
        const list<pair<int, file_t> >& token = templ::ptr->m_read.m_token;
	IT p = match(begin(token), end(token));
	if (p == end(token))
	  return false;
	return p->first == COLONCOLON_MK;
      }
    } // end of namespace templ_colon_impl
  } // end of namespace parse
} // end of namespace cxx_compiler

bool cxx_compiler::parse::templ_arg_and_coloncolon()
{
  using namespace templ_colon_impl;
  return templ::ptr ? templ_ptr_case() : common_case();
}

namespace cxx_compiler {
  namespace parse {
    using namespace std;
    inline void save(genaddr* ga)
    {
      typedef vector<var*>::reverse_iterator IT;
      IT p = find(rbegin(garbage), rend(garbage), ga);
      if (p != rend(garbage)) {
        vector<var*>::iterator q = p.base() - 1;
        garbage.erase(q);
        assert(!ga->m_code_copied);
        ga->m_code_copied = true;
        assert(!ga->m_appear_templ);
        ga->m_appear_templ = true;
      }
      else {
        if (!parse::templ::ptr) {
          assert(ga->m_code_copied);
          assert(ga->m_appear_templ);
        }
      }
    }
    inline bool save_cond(int n)
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
        return true;
      }
      return false;
    }
    void save_common(int n, list<void*>& lval, list<void*>* src = 0,
                     bool save_genaddr = false)
    {
      if (save_cond(n)) {
        if (src) {
          assert(!src->empty());
          lval.push_back(src->front());
          src->pop_front();
        }
        else {
          var* v = cxx_compiler_lval.m_var;
          if (save_genaddr) {
            if (n == IDENTIFIER_LEX || n == STRING_LITERAL_LEX) {
              if (genaddr* ga = v->genaddr_cast())
                save(ga);
            }
          }
          lval.push_back(v);
        }
      }
    }
    inline instantiated_tag* get_itag(scope* p)
    {
      assert(p);
      if (p->m_id != scope::TAG)
        return get_itag(p->m_parent);
      tag* ptr = static_cast<tag*>(p);
      if (ptr->m_flag & tag::INSTANTIATE)
        return static_cast<instantiated_tag*>(ptr);
      return get_itag(p->m_parent);
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
      int r = identifier::judge(u->m_name);
      if (r != CLASS_NAME_LEX)
        return r;
      tag* ptr = cxx_compiler_lval.m_tag;
      tag::flag_t flag = ptr->m_flag;
      if (flag & tag::TEMPLATE) {
        instantiated_tag* it = get_itag(scope::current);
        cxx_compiler_lval.m_tag = it;
      }
      return r;
    }
    inline bool inside_templ(scope* p)
    {
      if (!p)
        return false;
      if (p->m_id != scope::TAG)
        return false;
      tag* ptr = static_cast<tag*>(p);
      tag::flag_t flag = ptr->m_flag;
      tag::flag_t mask = tag::flag_t(tag::TEMPLATE | tag::INSTANTIATE);
      if (ptr->m_flag & mask)
        return true;
      return inside_templ(ptr->m_parent);
    }
    inline bool should_lookup_templ()
    {
      scope::id_t id = scope::current->m_id;
      if (id != scope::TAG)
        return true;
      tag* ptr = static_cast<tag*>(scope::current);
      const type* T = ptr->m_types.second;
      return T;
    }
    inline bool should_be_new_obj(scope* p)
    {
      if (identifier::mode == identifier::canbe_ctor)
	return false;
      if (peek() != IDENTIFIER_LEX)
	return false;
      if (p->m_id != scope::TAG)
	return false;
      tag* ptr = static_cast<tag*>(p);
      tag::flag_t flag = ptr->m_flag;
      return flag & tag::INSTANTIATE;
    }
    int get_common(int n, list<void*>& lval, bool from_mem_fun_body,
                   bool templ)
    {
      switch (n) {
      case PEEKED_NAME_LEX:
        assert(!lval.empty());
        cxx_compiler_lval.m_var = static_cast<var*>(lval.front());
        lval.pop_front();
        if (from_mem_fun_body)
          return get_id_from_mem_fun_body(n);
        {
          var* v = cxx_compiler_lval.m_var;
          assert(v->usr_cast());
          usr* u = static_cast<usr*>(v);
          assert(u->m_type->m_id == type::BACKPATCH);
          string name = u->m_name;
          last_token = identifier::judge(name);
          if (!templ)
            delete u;
        }
        return last_token;
      case IDENTIFIER_LEX:
        assert(!lval.empty());
        cxx_compiler_lval.m_var = static_cast<var*>(lval.front());
        lval.pop_front();
        if (from_mem_fun_body)
          return get_id_from_mem_fun_body(n);
        if (templ) {
          var* v = cxx_compiler_lval.m_var;
          if (genaddr* ga = v->genaddr_cast())
            v = ga->m_ref;
          assert(v->usr_cast());
          usr* u = static_cast<usr*>(v);
          string name = u->m_name;
          return last_token = identifier::judge(name);
        }
        if (context_t::retry[DECL_FCAST_CONFLICT_STATE] ||
            context_t::retry[TYPE_NAME_CONFLICT_STATE] ) {
          var* v = cxx_compiler_lval.m_var;
          assert(v->usr_cast());
          usr* u = static_cast<usr*>(v);
          assert(u->m_type->backpatch());
          string name = u->m_name;
          last_token = identifier::lookup(name, scope::current);
          if (!templ)
            delete u;
        }
        return n;
      case INTEGER_LITERAL_LEX:
      case CHARACTER_LITERAL_LEX:
      case FLOATING_LITERAL_LEX:
        assert(!lval.empty());
        cxx_compiler_lval.m_usr = static_cast<usr*>(lval.front());
        lval.pop_front();
        return n;
      case TYPEDEF_NAME_LEX:
        assert(!lval.empty());
        cxx_compiler_lval.m_usr = static_cast<usr*>(lval.front());
        lval.pop_front();
        if (templ) {
          usr* u = cxx_compiler_lval.m_usr;
          string name = u->m_name;
          last_token = identifier::lookup(name, scope::current);
	  if (should_be_new_obj(cxx_compiler_lval.m_usr->m_scope))
	    identifier::mode = identifier::new_obj;
        }
        return n;
      case STRING_LITERAL_LEX:
        assert(!lval.empty());
        cxx_compiler_lval.m_var = static_cast<var*>(lval.front());
        lval.pop_front();
        if (context_t::retry[DECLARATOR_ID_CONFLICT_STATE])
          cxx_compiler_lval.m_var->m_scope = scope::current;
        return n;
      case CLASS_NAME_LEX:
        assert(!lval.empty());
        cxx_compiler_lval.m_tag = static_cast<tag*>(lval.front());
        lval.pop_front();
        if (templ) {
          tag* ptr = cxx_compiler_lval.m_tag;
          tag::flag_t flag = ptr->m_flag;
          if (flag & tag::INSTANTIATE) {
            assert(!tinfos.empty());
            int n = tinfos.size();
            if (n >= 2) {
              if (tinfos[n-1].first) {
                if (tinfos[n-2].second)
                  return CLASS_NAME_LEX;
              }
            }
            instantiated_tag* it = static_cast<instantiated_tag*>(ptr);
            const instantiated_tag::SEED& seed = it->m_seed;
            typedef instantiated_tag::SEED::const_iterator IT;
            IT p = find_if(begin(seed), end(seed),
                	   not1(ptr_fun(template_param)));
            if (p == end(seed)) {
              template_tag* tt = it->m_src;
              string name = tt->m_name;
	      int c = peek();
	      scope* p = c == COLONCOLON_MK ? tt->m_parent : scope::current;
              int r = identifier::lookup(name, p);
              assert(r == CLASS_NAME_LEX);
              return r;
            }
          }
          if ((flag & tag::TYPENAMED) || inside_templ(ptr->m_parent)) {
            string name = ptr->m_name;
            int r = identifier::lookup(name, scope::current);
            assert(r == CLASS_NAME_LEX || r == TYPEDEF_NAME_LEX ||
		   identifier::typenaming && r == IDENTIFIER_LEX);
            if ((flag & tag::TYPENAMED) && templ) {
	      scope* p = (r == CLASS_NAME_LEX) ?
		cxx_compiler_lval.m_tag->m_parent :
		cxx_compiler_lval.m_usr->m_scope;
	      if (should_be_new_obj(p))
        	identifier::mode = identifier::new_obj;
            }
            return r;
          }
          if (flag & tag::TEMPLATE) {
            instantiated_tag* it = get_itag(scope::current);
            cxx_compiler_lval.m_tag = it;
            return CLASS_NAME_LEX;
          }
          if (!templ::save_t::nest.empty()) {
            string name = ptr->m_name;
            identifier::mode = identifier::no_err;
            int r = identifier::lookup(name, scope::current);
            if (r) {
              assert(r == CLASS_NAME_LEX);
              return r;
            }
          }
        }
        return n;
      case ENUM_NAME_LEX:
        assert(!lval.empty());
        cxx_compiler_lval.m_tag = static_cast<tag*>(lval.front());
        lval.pop_front();
        return n;
      case TEMPLATE_NAME_LEX:
        {
          assert(!lval.empty());
          typedef pair<usr*, tag*> T;
          T* tmp = static_cast<T*>(lval.front());
          cxx_compiler_lval.m_ut = new T(*tmp);
          lval.pop_front();
          if (tag* ptr = cxx_compiler_lval.m_ut->second) {
            assert(ptr->m_flag & tag::TEMPLATE);
            template_tag* tt = static_cast<template_tag*>(ptr);
            string name = tt->m_name;
            using namespace identifier;
            if (tt->m_created) {
              last_token = should_lookup_templ() ?
                lookup(name, scope::current) : create_templ(name);
            }
            else
              last_token = lookup(name, scope::current);
	    n = last_token;
          }
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
    inline void save_c(context_t& x)
    {
      x.m_read.m_token.push_back(make_pair(last_token,position));
      save_common(last_token, x.m_read.m_lval);
    }
    inline void save_s(templ::save_t* p)
    {
      read_t& r = p->m_read;
      r.m_token.push_back(make_pair(last_token, parse::position));
      if (last_token == CLASS_NAME_LEX) {
        tag* ptr = cxx_compiler_lval.m_tag;
        const type* T = ptr->m_types.first;
        if (T->m_id == type::TEMPLATE_PARAM) {
          if (const type* T2 = ptr->m_types.second) {
            tag* ptr2 = new tag(*ptr);
            cxx_compiler_lval.m_tag = ptr2;
          }
        }
      }
      save_common(last_token, r.m_lval, 0, true);
    }
    inline int save_for_retry()
    {
      for_each(begin(context_t::all), end(context_t::all), save_c);
      for_each(begin(templ::save_t::nest), end(templ::save_t::nest), save_s);
      return last_token;
    }
    namespace templ {
      templ_base* ptr;
      bool param;
      int arg;
      bool func()
      {
        if (!save_t::nest.empty()) {
	  save_t* p = save_t::nest.back();
	  return p->m_usr;
	}
	return instantiate_with_template_param<template_usr>();
      }
      void patch_13_2(save_t* p, const read_t& rd, pair<int, int> x)
      {
        tag* ptr = p->m_tag;
        assert(ptr);
        pair<int, file_t>& b = p->m_read.m_token.back();
        char c = b.first;
        assert(c == '{' || c == ':' || c == TRY_KW);
        tag::flag_t flag = ptr->m_flag;
        if (!(flag & tag::TEMPLATE)) {
          b.first = ';';
          return;
        }
        typedef list<pair<int, file_t> >::const_iterator ITx;
        ITx ex = rd.m_token.end();
        ITx bx = ex;
        int n = x.first;
        assert(n);
        while (n--)
          --bx;
        copy(bx, ex, back_inserter(p->m_read.m_token));
        typedef list<void*>::const_iterator ITy;
        ITy ey = rd.m_lval.end();
        ITy by = ey;
        int m = x.second;
        while (m--)
          --by;
        copy(by, ey, back_inserter(p->m_read.m_lval));
      }
    } // end of namespac templ
  } // end of namesapce parse
} // end of namesapce cxx_compiler

int cxx_compiler::parse::get_token()
{
  if (!g_read.m_token.empty()) {
    position = g_read.m_token.front().second;
    int prev = last_token;
    last_token = g_read.m_token.front().first;
    g_read.m_token.pop_front();
    if (prev == COLONCOLON_MK && last_token == PEEKED_NAME_LEX) {
      int n = last_token;
      last_token = prev;
      get_common(n, g_read.m_lval, false, false);
    }
    else
      get_common(last_token, g_read.m_lval, false, false);
    if (last_token == COLONCOLON_MK) {
      using namespace declarations::specifier_seq;
      if (info_t::s_stack.empty()) {
	if (!identifier::typenaming)
	  identifier::mode = identifier::look;
      }
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
  identifier::mode = identifier::look;
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
      if (!(func->m_flag & usr::STATIC)) {
        map<string, vector<usr*> >& usrs = scope::current->m_usrs;
        typedef map<string, vector<usr*> >::const_iterator IT;
        IT p = usrs.find(this_name);
        if (p == usrs.end()) {
	  assert(T);
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

namespace cxx_compiler {
  namespace parse {
    namespace member_function_body {
      map<tag*, map<usr*, save_t> > stbl;
      save_t* saved;
      inline read_t* get(usr* key, scope* param)
      {
        if (!templ::save_t::nest.empty()) {
          templ::save_t* p = templ::save_t::nest.back();
          if (p->m_tag)
            return &p->m_read;
        }
	scope* p = key->m_scope;
	assert(p->m_id == scope::TAG);
	tag* ptr = static_cast<tag*>(p);
	stbl[ptr][key].m_param = param;
        read_t* pr = &stbl[ptr][key].m_read;
        pr->m_token.push_back(make_pair(cxx_compiler_char,position));
        return pr;
      }
    } // end of namespace member_function_body
  } // end of namespace parse
} // end of namespace cxx_compiler

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

  read_t* pr = get(key, param);

  identifier::mode = identifier::peeking;
  save_brace(pr, cxx_compiler_char == '{');
  identifier::mode = identifier::look;
  scope::current = ptr;
}

std::pair<int, int>
cxx_compiler::parse::member_function_body::save_brace(read_t* ptr, bool b)
{
  using namespace std;
  list<pair<int, file_t> >& token = ptr->m_token;
  int tsz = token.size();
  list<void*>& lval = ptr->m_lval;
  int lsz = lval.size();
  while (1) {
    int n;
    if (!g_read.m_token.empty()) {
      position = g_read.m_token.front().second;
      n = g_read.m_token.front().first;
      g_read.m_token.pop_front();
      token.push_back(make_pair(n,position));
      save_common(n, lval, &g_read.m_lval);
    }
    else if (templ::ptr) {
      position = templ::ptr->m_read.m_token.front().second;
      n = templ::ptr->m_read.m_token.front().first;
      templ::ptr->m_read.m_token.pop_front();
      token.push_back(make_pair(n,position));
      save_common(n, lval, &templ::ptr->m_read.m_lval);
    }
    else {
      n = cxx_compiler_lex();
      if (n == 0) // YYEOF
        break;
      token.push_back(make_pair(n,position));
      save_common(n, lval, 0, !templ::save_t::nest.empty());
    }

    if (n == '{') {
      if (b)
        save_brace(ptr, false);
      else
        b = true;
    }
    if (n == '}')
      break;
  }

  return make_pair(token.size() - tsz, lval.size() - lsz);
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
    context_t::context_t(int state, const vector<short>& vs,
                	 const vector<void*>& vv, int c)
    : m_state(state), m_stack0(vs), m_stack1(vv), m_char(c),
      m_scope(scope::current), m_before(class_or_namespace_name::before),
      m_last(class_or_namespace_name::last) {}
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
    inline void helper(templ::save_t* p, bool b)
    {
      read_t& r = p->m_read;
      int n = g_read.m_token.size();
      if (b)
        --n;
      assert(n >= 0);
      list<pair<int, file_t> >& token = r.m_token;
      list<void*>& lval = r.m_lval;
      for (int i = 0 ; i != n ; ++i) {
        pair<int, file_t> x = token.back();
        token.pop_back();
        if (save_cond(x.first))
          lval.pop_back();
      }
    }
    void restore(int* state, short** b0, short** t0, short* a0,
                 YYSTYPE** b1, YYSTYPE** t1, YYSTYPE* a1, bool b)
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
      last_token = cxx_compiler_char = x.m_char;
      read_t tmp = g_read;
      g_read = x.m_read;
      scope::current = x.m_scope;
      class_or_namespace_name::before = x.m_before;
      class_or_namespace_name::last = x.m_last;
      copy(begin(tmp.m_token), end(tmp.m_token),
           back_inserter(g_read.m_token));
      copy(begin(tmp.m_lval), end(tmp.m_lval), back_inserter(g_read.m_lval));
      context_t::all.pop_back();
      using namespace templ;
      for_each(begin(save_t::nest), end(save_t::nest),
               bind2nd(ptr_fun(helper), b));
    }
  } // end of namesapce parse
} // end of namesapce cxx_compiler


namespace cxx_compiler {
  namespace parse {
    namespace templ {
      vector<save_t*> save_t::nest;
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
      cout << '\n';
      const list<void*>& ls2 = r.m_lval;
      for (auto p : ls2)
        cout << p << ' ';
      cout << endl;
    }
    void debug_stbl()
    {
      using namespace member_function_body;
      for (auto p : stbl) {
	const map<usr*, save_t>& m = p.second;
	for (auto q : m) {
	  usr* u = q.first;
	  cout << u << '(' << u->m_name << ')' << endl;
	  const read_t& r = q.second.m_read;
	  debug_read(r);
	}
      }
    }
  } // end of namespace parse
} // end of namespace cxx_compiler

int cxx_compiler::parse::base_clause;
