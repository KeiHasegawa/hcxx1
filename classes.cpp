#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "yy.h"
#include "cxx_y.h"

cxx_compiler::tag::kind_t cxx_compiler::classes::specifier::get(int keyword)
{
  switch (keyword) {
  case CLASS_KW:  return tag::CLASS;
  case STRUCT_KW: return tag::STRUCT;
  case UNION_KW:  return tag::UNION;
  default:        return tag::ENUM;
  }
}

namespace cxx_compiler {
  namespace classes_impl {
    inline string get_name(usr* u)
    {
      if (!template_tag::nest.empty()) {
        const template_tag::info_t& x = template_tag::nest.back();
        template_tag* tt = x.m_tt;
        instantiated_tag* it = x.m_it;
        if (!it)
          return tt->instantiated_name();
      }
      return u ? u->m_name : new_name(".tag");
    }
    namespace merge_impl {
      struct cmp {
        scope::tps_t& m_ptps;
        scope::tps_t& m_ctps;
        cmp(scope::tps_t& ptps, scope::tps_t& ctps)
          : m_ptps(ptps), m_ctps(ctps) {}
        const type* template_param_case(tag* ptr)
        {
          string pn = ptr->m_name;
          const vector<string>& po = m_ptps.m_order;
          typedef vector<string>::const_iterator ITx;
          ITx p = find(begin(po), end(po), pn);
          assert(p != end(po));
          int n = distance(begin(po), p);
          const vector<string>& co = m_ctps.m_order;
          string cn = co[n];
          const map<string, scope::tps_t::value_t>& table = m_ctps.m_table;
          typedef map<string, scope::tps_t::value_t>::const_iterator ITy;
          ITy q = table.find(cn);
          assert(q != table.end());
          const scope::tps_t::value_t& value = q->second;
          assert(!value.second);
          ptr = value.first;
          const type* T = ptr->m_types.second;
          assert(!T);
          T = ptr->m_types.first;
          assert(T);
          return T;
        }
        struct helper {
          scope::tps_t& m_ptps;
          scope::tps_t& m_ctps;
          helper(scope::tps_t& ptps, scope::tps_t& ctps)
            : m_ptps(ptps), m_ctps(ctps) {}
          scope::tps_t::val2_t* operator()(const scope::tps_t::val2_t& x)
          {
            if (var* v = x.second)
              return new scope::tps_t::val2_t(0, v);
            const type* T = x.first;
            cmp op(m_ptps, m_ctps);
            T = op.calc_type(T);
            return new scope::tps_t::val2_t(T, 0);
          }
        };
        const type* calc_type(const type* Tp)
        {
          tag* ptr = Tp->get_tag();
          if (!ptr)
            return Tp;
          if (Tp->m_id == type::TEMPLATE_PARAM)
            return template_param_case(ptr);
          if (!(ptr->m_flag & tag::INSTANTIATE))
            return Tp;
          typedef instantiated_tag IT;
          IT* it = static_cast<IT*>(ptr);
          const IT::SEED& seed = it->m_seed;
          vector<scope::tps_t::val2_t*>* pv =
            new vector<scope::tps_t::val2_t*>;
          transform(begin(seed), end(seed), back_inserter(*pv),
                    helper(m_ptps, m_ctps));
          template_tag* tt = it->m_src;
          ptr = tt->instantiate(pv);
          const type* Tc = ptr->m_types.second;
          if (Tc)
            return Tc;
          Tc = ptr->m_types.first;
          assert(Tc);
          return Tc;
        }
        pair<const type*, var*> calc(const pair<const type*, var*>& x)
        {
          if (const type* T = x.first)
            return make_pair(calc_type(T), (var*)0);
          var* v = x.second;
          return make_pair((const type*)0, v);
        }
        bool operator()(string p, string c)
        {
          map<string, pair<const type*, var*> >& pdef = m_ptps.m_default;
          map<string, pair<const type*, var*> >& cdef = m_ctps.m_default;
          typedef map<string, pair<const type*, var*> >::const_iterator IT;
          IT pit = pdef.find(p);
          if (pit == pdef.end())
            return true;
          pair<const type*, var*> x = pit->second;
          IT cit = cdef.find(c);
          if (cit == cdef.end()) {
            cdef[c] = calc(x);
            return true;
          }
          error::not_implemented();
          return false;
        }
      };
      inline void default_arg(template_tag* prev, template_tag* curr)
      {
        scope::tps_t& ptps = prev->templ_base::m_tps;
        scope::tps_t& ctps = curr->templ_base::m_tps;
        const vector<string>& porder = ptps.m_order;
        const vector<string>& corder = ctps.m_order;
        if (porder.size() != corder.size())
          error::not_implemented();
        typedef vector<string>::const_iterator IT;
        pair<IT, IT> ret =
          mismatch(begin(porder), end(porder), begin(corder), cmp(ptps, ctps));
        if (ret != make_pair(end(porder), end(corder)))
          error::not_implemented();
      }
    } // end of namespace merge_impl
    inline void merge(template_tag* prev, template_tag* curr)
    {
      merge_impl::default_arg(prev, curr);
      curr->m_prev = prev;
    }
    inline tag*
    get_tag(tag::kind_t kind, string name, const file_t& file,
            vector<base*>* bases)
    {
      if (!template_tag::nest.empty()) {
        template_tag::info_t& x = template_tag::nest.back();
        template_tag* tt = x.m_tt;
        if (!x.m_it) {
          x.m_it = new instantiated_tag(kind, name, file, bases, tt, x.m_key);
          return x.m_it;
        }
      }

      tag* ret = new tag(kind, name, file, bases);
      const vector<scope::tps_t>& tps = scope::current->m_tps;
      if (!tps.empty()) {
        const scope::tps_t& b = tps.back();
        if (!b.m_table.empty()) {
          using namespace parse::templ;
          assert(!save_t::nest.empty());
          save_t* p = save_t::nest.back();
          assert(!p->m_tag);
          assert(!class_or_namespace_name::before.empty());
          assert(class_or_namespace_name::before.back() == ret);
          template_tag* curr = new template_tag(*ret, b);
          p->m_tag = ret = curr;
          class_or_namespace_name::before.back() = ret;
          return ret;
        }
      }
      return ret;
    }
    inline void
    combine(tag* prev, tag::kind_t kind, const file_t& file,
            vector<base*>* bases)
    {
      if (prev->m_kind != kind) {
        using namespace error::classes;
        string name = prev->m_name;
        redeclaration(parse::position,prev->m_file.back(),name);
      }
      pair<const type*, const type*> types = prev->m_types;
      if (types.second) {
        using namespace error::classes;
        string name = prev->m_name;
        redeclaration(parse::position,prev->m_file.back(),name);
      }
      prev->m_file.push_back(file);
      tag::flag_t flag = prev->m_flag;
      if (!(flag & tag::TEMPLATE)) {
        if (prev->m_bases)
          error::not_implemented();
        prev->m_bases = bases;
        scope::current = prev;
        class_or_namespace_name::before.push_back(prev);
        declarations::specifier_seq::info_t::clear();
        if (flag & tag::INSTANTIATE) {
          if (!template_tag::nest.empty()) {
            template_tag* tt = template_tag::nest.back().m_tt;
            assert(!template_tag::nest.back().m_it);
            typedef instantiated_tag IT;
            IT* it = static_cast<IT*>(prev);
            it->m_src = tt;  // override
            template_tag::nest.back().m_it = it;
          }
        }
        return;
      }
      template_tag* tprev = static_cast<template_tag*>(prev);
      string name = tprev->m_name;
      tag* ptr = get_tag(kind, name, file, bases);
      if (!(ptr->m_flag & tag::TEMPLATE))
        error::not_implemented();

      template_tag* tcurr = static_cast<template_tag*>(ptr);
      merge(tprev, tcurr);

      tcurr->m_parent = scope::current;
      tcurr->m_parent->m_children.push_back(tcurr);
      tcurr->m_types.first = incomplete_tagged_type::create(tcurr);
      scope::current->m_tags[name] = tcurr;
      scope::current = tcurr;
      declarations::specifier_seq::info_t::clear();
    }
    void update(base* bp)
    {
      tag* ptr = bp->m_tag;
      const type* T = ptr->m_types.first;
      if (T->m_id != type::TEMPLATE_PARAM) {
        T = ptr->m_types.second;
        assert(T);
        return;
      }
      T = ptr->m_types.second;
      if (!T)
        return;
      tag* ptr2 = T->get_tag();
      bp->m_tag = ptr2;
    }
  } // end of namespace classes_impl
} // end of namespace cxx_compiler

void
cxx_compiler::classes::specifier::begin(int keyword, var* v,
                                        std::vector<base*>* bases)
{
  using namespace std;
  assert(!v || v->usr_cast());
  usr* u = static_cast<usr*>(v);
  auto_ptr<usr> sweeper(parse::templ::save_t::nest.empty() ? u : 0);
  tag::kind_t kind = get(keyword);
  string name = classes_impl::get_name(u);
  if (bases)
    for_each(begin(*bases), end(*bases), classes_impl::update);
  const file_t& file = u ? u->m_file : parse::position;
  map<string, tag*>& tags = scope::current->m_tags;
  map<string, tag*>::const_iterator p = tags.find(name);
  if (p != tags.end())
    return classes_impl::combine(p->second, kind, file, bases);

  tag* ptr = classes_impl::get_tag(kind, name, file, bases);
  ptr->m_parent = scope::current;
  ptr->m_parent->m_children.push_back(ptr);
  ptr->m_types.first = incomplete_tagged_type::create(ptr);
  tags[name] = ptr;
  scope::current = ptr;
  declarations::specifier_seq::info_t::clear();
}

void
cxx_compiler::classes::
specifier::begin2(int keyword, tag* ptr, std::vector<base*>* bases)
{
  using namespace std;
  string name = ptr->m_name;
  usr* tmp = new usr(name,0,usr::NONE,file_t(),usr::NONE2);
  begin(keyword,tmp,bases);
}

void
cxx_compiler::classes::
specifier::begin3(int keyword, pair<usr*, tag*>* x, std::vector<base*>* bases)
{
  using namespace std;
  auto_ptr<pair<usr*, tag*> > sweeper(x);
  assert(!x->first);
  tag* ptr = x->second;
  tag::kind_t kind = get(keyword);
  string name = ptr->m_name;
  map<string,tag*>& tags = scope::current->m_tags;
  map<string,tag*>::const_iterator p = tags.find(name);
  if (p != tags.end())
    return classes_impl::combine(p->second, kind, parse::position, bases);

  ptr->m_parent = scope::current;
  ptr->m_parent->m_children.push_back(ptr);
  ptr->m_types.first = incomplete_tagged_type::create(ptr);
  ptr->m_bases = bases;
  tags[name] = ptr;
  scope::current = ptr;
  declarations::specifier_seq::info_t::clear();
}

const cxx_compiler::type* cxx_compiler::classes::specifier::action()
{
  using namespace std;
  assert(scope::current->m_id == scope::TAG);
  tag* ptr = static_cast<tag*>(scope::current);
  scope* ps = ptr->m_parent;
  vector<scope::tps_t>& tps = ps->m_tps;
  if (!tps.empty()) {
    scope::tps_t& b = tps.back();
    const map<string, scope::tps_t::value_t>& table = b.m_table;
    if (!table.empty()) {
      if (ptr->m_flag & tag::TEMPLATE) {
        template_tag* tt = static_cast<template_tag*>(ptr);
        assert(!class_or_namespace_name::before.empty());
        assert(scope::current == class_or_namespace_name::before.back());
	class_or_namespace_name::before.pop_back();
        scope::current = ptr->m_parent;
        return ptr->m_types.first;
      }
    }
  }

  const type* ret = record_type::create(ptr);
  ptr->m_types.second = ret;
  handle_copy_ctor(ptr);

  map<usr*, parse::member_function_body::save_t>& tbl =
    parse::member_function_body::stbl[ptr];
  if (tbl.empty()) {
    handle_vdel(ptr);
    assert(!class_or_namespace_name::before.empty());
    assert(scope::current == class_or_namespace_name::before.back());
    class_or_namespace_name::before.pop_back();
    scope::current = ptr->m_parent;
    return ret;
  }
  scope* org = scope::current;
  for_each(tbl.begin(),tbl.end(),member_function_definition);
  tbl.clear();
  parse::member_function_body::stbl.erase(ptr);
  scope::current = org;
  handle_vdel(ptr);
  scope::current = org->m_parent;
  assert(!class_or_namespace_name::before.empty());
  assert(ptr == class_or_namespace_name::before.back());
  class_or_namespace_name::before.pop_back();
  return ret;
}

namespace cxx_compiler {
  namespace classes {
    namespace specifier {
      using namespace std;
      void
      member_function_definition(pair<usr* const,
                                      parse::member_function_body::save_t>& E)
      {
        using namespace declarations;
        usr* u = E.first;
	scope* p = u->m_scope;
	scope::id_t id = p->m_id;
	assert(id == scope::TAG);
	tag* ptr = static_cast<tag*>(p);
	tag::flag_t flag = ptr->m_flag;
	if (flag & tag::INSTANTIATE) {
	  typedef instantiated_tag IT;
	  IT* it = static_cast<IT*>(ptr);
	  IT::SEED& seed = it->m_seed;
	  typedef IT::SEED::const_iterator P;
	  P p = find_if(begin(seed), end(seed), template_param);
	  if (p != end(seed))
	    return;
	}
        u->m_type = u->m_type->complete_type();
        scope::current = u->m_scope;
        vector<scope*>& children = scope::current->m_children;
        scope* param = E.second.m_param;
        children.push_back(param);
        fundef::current = new fundef(u,param);
        const vector<usr*>& order = param->m_order;
        for_each(order.begin(),order.end(),check_object);
        using namespace parse;
        member_function_body::saved = &E.second;
        file_t org = position;
        identifier::mode_t org2 = identifier::mode;
        identifier::mode = identifier::look;
        bool org3 = expressions::constant_flag;
        expressions::constant_flag = false;
        cxx_compiler_parse();
        expressions::constant_flag = org3;
        identifier::mode = org2;
        position = org;
        member_function_body::saved = 0;
      }
    } // end of namespace specifier
  } // end of namespace classes
} // end of namespace cxx_compiler

void cxx_compiler::classes::members::action(var* v)
{
  using namespace std;
  assert(v->usr_cast());
  usr* u = static_cast<usr*>(v);
  u = declarations::action1(u, false);
  if (u->m_flag2 & usr::CONV_OPE) {
    using namespace declarations::specifier_seq;
    assert(!info_t::s_stack.empty());
    delete info_t::s_stack.top();
  }
  vector<scope*>& children = scope::current->m_children;
  typedef vector<scope*>::iterator IT;
  for ( IT p = begin(children) ; p != end(children) ; ) {
    scope* ptr = *p;
    scope::id_t id = ptr->m_id;
    if (id == scope::PARAM) {
      if (!parse::templ::save_t::nest.empty())
        ptr->m_usrs.clear();
      delete ptr;
      p = children.erase(p);
    }
    else
      ++p;
  }
}

void cxx_compiler::classes::members::action2(var* v, expressions::base* expr)
{
  using namespace std;
  var* cexpr = expr->gen();
  cexpr = cexpr->rvalue();
  const type* T = cexpr->m_type;
  if (tag* ptr = T->get_tag()) {
    tag::flag_t flag = ptr->m_flag;
    if (flag & tag::TYPENAMED)
      return;
  }
  if (!cexpr->isconstant()) {
    if (parse::templ::save_t::nest.empty() &&
        !instantiate_with_template_param<template_tag>())
      error::not_implemented();
  }
  usr* cons = cexpr->usr_cast();
  if (!cons)
    return;
  assert(v->usr_cast());
  usr* u = static_cast<usr*>(v);

  usr::flag_t flag = u->m_flag;
  usr::flag_t mask = usr::flag_t(usr::STATIC | usr::VIRTUAL);
  if (!(flag & mask))
    error::not_implemented();
  if ((flag & usr::STATIC) && (flag & usr::VIRTUAL))
    error::not_implemented();
  if (flag & usr::STATIC) {
    if (parse::templ::save_t::nest.empty()) {
      const type* T = u->m_type;
      T = T->unqualified();
      if (tag* ptr = T->get_tag()) {
	tag::flag_t flag = ptr->m_flag;
	if (flag & tag::TYPENAMED)
	  return;
      }
      if (T->m_id != type::TEMPLATE_PARAM) {
        var* tmp = cons->cast(T);
        assert(tmp->usr_cast());
        cons = static_cast<usr*>(tmp);
        with_initial* wi = new with_initial(*u);
        wi->m_value[0] = cons;
        scope* p = u->m_scope;
        string name = u->m_name;
        assert(p->m_order.back() == u);
        p->m_order.back() = wi;
        assert(p->m_usrs[name].back() == u);
        p->m_usrs[name].back() = wi;
        wi->m_flag = usr::flag_t(flag | usr::WITH_INI | usr::STATIC_DEF);
        delete u;
        u = wi;
      }
    }
  }
  if (flag & usr::VIRTUAL) {
    const type* T = cons->m_type;
    T = T->unqualified();
    if (T->m_id != type::INT)
      error::not_implemented();
    constant<int>* c = static_cast<constant<int>*>(cons);
    if (c->m_value != 0)
      error::not_implemented();
    u->m_flag = usr::flag_t(flag | usr::PURE_VIRT);
  }
}

void cxx_compiler::classes::members::bit_field(var* v, expressions::base* expr)
{
  using namespace std;
  using namespace error::classes::bit_field;
  assert(!v || v->usr_cast());
  usr* u = static_cast<usr*>(v);
  auto_ptr<expressions::base> sweeper(expr);
  expressions::constant_flag = true;
  v = expr->gen();
  v = v->rvalue();
  expressions::constant_flag = false;
  if ( !v->m_type->integer() ){
    not_integer_bit(u);
    v->m_type = int_type::create();
  }
  int bit = 1;
  if ( !v->isconstant() )
    not_constant(u);
  else
    bit = v->value();
  if ( bit < 0 ){
    negative(u);
    bit = 1;
  }
  if ( !u )
    u = new usr("",backpatch_type::create(),usr::NONE,parse::position,
                usr::NONE2); 
  const type* T = u->m_type;
  if ( !T->backpatch() ){
    T = backpatch_type::create();
    u = new usr(u->m_name,T,usr::NONE,parse::position,usr::NONE2); 
  }
  u->m_type = T->patch(bit_field_type::create(bit,backpatch_type::create()),0);
  declarations::action1(u,false);
}

namespace cxx_compiler {
  using namespace std;
  vector<scope*> class_or_namespace_name::before;
  scope* class_or_namespace_name::last;
  int class_or_namespace_name::decl_array;
} // end of namespace cxx_compiler

void cxx_compiler::class_or_namespace_name::after(bool set_last)
{
  assert(!before.empty());
  assert(before.back());
  if (set_last)
    class_or_namespace_name::last = scope::current;
  scope::current = before.back();
  if (scope::current->m_id == scope::TAG) {
    tag* ptr = static_cast<tag*>(scope::current);
    tag::kind_t kind = ptr->m_kind;
    if (kind == tag::ENUM)
      scope::current = scope::current->m_parent;
  }
}

cxx_compiler::tag* cxx_compiler::class_or_namespace_name::conv(tag* ptr)
{
  const type* T1 = ptr->m_types.first;
  if (T1->m_id != type::TEMPLATE_PARAM)
    return ptr;
  const type* T2 = ptr->m_types.second;
  if (!T2)
    return ptr;
  tag* ret = T2->get_tag();
  assert(ret);
  return ret;
}

namespace cxx_compiler {
  namespace declarations {
    namespace declarators {
      namespace function {
        namespace definition {
          namespace mem_initializer {
            using namespace std;
            void for_scalar(var* x, vector<expressions::base*>* p)
            {
              using namespace expressions;
              using namespace expressions::primary::literal;
	      if (parse::templ::func())
		return;
              const type* Tx = x->result_type();
              if (!p) {
                var* zero = integer::create(0);
                zero = zero->cast(Tx);
                code.push_back(new invladdr3ac(x, zero));
                return;
              }
              if (p->size() != 1)
                error::not_implemented();
              const type* Ux = Tx->unqualified();
              expressions::base* expr = (*p)[0];
              var* y = expr->gen();
              if (Ux->m_id != type::REFERENCE)
                y = y->rvalue();
              const type* Ty = y->result_type();
              var tmp(Ty);
              bool discard = false;
              bool ctor_conv = false;
              const type* T =
                assignment::valid(Tx, &tmp, &discard, &ctor_conv, 0);
              if (!T)
                error::not_implemented();
              y = Tx->aggregate() ? aggregate_conv(Tx, y, ctor_conv, 0) 
                : y->cast(Tx);
              code.push_back(new invladdr3ac(x,y));
            }
            template<class C> void zero(var* dst, int offset)
            {
              using namespace expressions::primary::literal;
              const type* T = C::create();
              const type* P = pointer_type::create(T);
              var* ptr = new var(P);
              assert(scope::current->m_id == scope::BLOCK);
              block* b = static_cast<block*>(scope::current);
              b->m_vars.push_back(ptr);
              if (offset) {
                var* off = integer::create(offset);
                code.push_back(new add3ac(ptr, dst, off));
              }
              else
                code.push_back(new assign3ac(ptr, dst));
              var* zero = integer::create(0);
              zero = zero->cast(T);
              code.push_back(new invladdr3ac(ptr, zero));
            }
            inline void zero_clear(var* dst)
            {
              const type* T = dst->result_type();
              int size = T->size();
              assert(long_long_type::create()->size() == 8);
              assert(size);
              for (int offset = 0 ; size ; offset += 8) {
                switch (size) {
                case 1: return zero<char_type>(dst, offset);
                case 2: return zero<short_type>(dst, offset);
                case 4: return zero<int_type>(dst, offset);
                default:
                  assert(size >= 8);
                  zero<long_long_type>(dst, offset);
                  size -= 8;
                  break;
                }
              }
            }
            void for_aggregate(var* dst, vector<expressions::base*>* p)
            {
              vector<var*> arg;
              if (p)
                transform(begin(*p), end(*p), back_inserter(arg),
                          mem_fun(&expressions::base::gen));
              const type* T = dst->result_type();
              tag* ptr = T->get_tag();
              if (ptr->m_flag & tag::TYPENAMED) {
                if (T->m_id == type::INCOMPLETE_TAGGED)
                  return;
              }
              usr* ctor = has_ctor_dtor(ptr, false);
              if (!ctor)
                return zero_clear(dst);
              usr::flag_t flag = ctor->m_flag;
              if (flag & usr::OVERLOAD) {
                overload* ovl = static_cast<overload*>(ctor);
                ovl->m_obj = dst;
                ovl->call(&arg);
                return;
              }
              call_impl::wrapper(ctor, &arg, dst);
            }
            struct sweeper {
              vector<expressions::base*>* m_expr;
              sweeper(vector<expressions::base*>* p) : m_expr(p) {}
              ~sweeper()
              {
                if (!m_expr) 
                  return;
                for (auto p : *m_expr)
                  delete p;
                delete m_expr;
              }
            };
            void gen(var* dst, vector<expressions::base*>* p)
            {
              sweeper sweeper(p);
              const type* T = dst->result_type();
              T->scalar() ? for_scalar(dst, p) : for_aggregate(dst, p);
            }
            map<usr*, VALUE> for_parse;
            inline block* get_block(tag* ptr)
            {
              scope* param = fundef::current->m_param;
              vector<scope*>& children = param->m_children;
              if (!children.empty()) {
                scope* ps = children.back();
                assert(ps->m_id == scope::BLOCK);
                return static_cast<block*>(ps);
              }
              block* b = new block;
              children.push_back(b);
              b->m_parent = param;
              map<string, vector<usr*> >& usrs = param->m_usrs;
              typedef map<string, vector<usr*> >::const_iterator IT;
              IT p = usrs.find(this_name);
              assert(p != usrs.end());
              return b;
            }
            map<usr*, map<usr*, pbc> > mtbl;
            void id_action(usr* u, EXPRS* exprs, usr* ctor)
            {
              using namespace expressions::primary;
              scope* ps = ctor->m_scope;
              assert(ps->m_id == scope::TAG);
              tag* ptr = static_cast<tag*>(ps);
              const type* T = ptr->m_types.second;
              if (!T) {
                assert(scope::current->m_id == scope::PARAM);
                for_parse[ctor].push_back(make_pair(new PAIR(u,0),exprs));
                return;
              }
              block* b = get_block(ptr);
              scope* org = scope::current;
              scope::current = b;
              int n = code.size();
              vector<route_t> dummy;
              var* dst = from_member(u, dummy);
              gen(dst, exprs);
              scope::current = org;
              vector<tac*> tmp;
              copy(begin(code) + n, end(code), back_inserter(tmp));
              mtbl[ctor][u] = pbc(b->m_parent, b, tmp);
              code.resize(n);
            }
            void gen(tag* ptr, var* this_ptr, int offset, EXPRS* exprs)
            {
              using namespace expressions::primary::literal;
              if (offset) {
                var* off = integer::create(offset);
                const type* T = this_ptr->m_type;
                var* tmp = new var(T);
                assert(scope::current->m_id == scope::BLOCK);
                block* b = static_cast<block*>(scope::current);
                b->m_vars.push_back(tmp);
                code.push_back(new add3ac(tmp, this_ptr, off));
                this_ptr = tmp;
              }
              vector<var*> arg;
              if (exprs)
                transform(begin(*exprs), end(*exprs), back_inserter(arg),
                          mem_fun(&expressions::base::gen));
              string name = tor_name(ptr);
              typedef map<string, vector<usr*> >::const_iterator IT;
              IT p = ptr->m_usrs.find(name);
              if (p == ptr->m_usrs.end()) {
                // Plain Old Type. No constructor is declared.
                if (arg.size() != 1)
                  error::not_implemented();
                var* z = arg[0];
                z = z->rvalue();
                const type* T = z->m_type;
                T = pointer_type::create(T);
                var* y = new var(T);
                assert(scope::current->m_id == scope::BLOCK);
                block* b = static_cast<block*>(scope::current);
                b->m_vars.push_back(y);
                code.push_back(new cast3ac(y, this_ptr, T));
                code.push_back(new invladdr3ac(y, z));
                return;
              }
              const vector<usr*>& v = p->second;
              usr* ctor = v.back();
              usr::flag_t flag = ctor->m_flag;
              if (flag & usr::OVERLOAD) {
                overload* ovl = static_cast<overload*>(ctor);
                ovl->m_obj = this_ptr;
                const vector<usr*>& cand = ovl->m_candidacy;
                vector<usr::flag_t> org;
                for_each(begin(cand), end(cand), [&org](usr* u) {
                    usr::flag_t flag = u->m_flag;
                    org.push_back(flag);
                    u->m_flag = usr::flag_t(flag & ~usr::INLINE);
                  });             // (*1)
                ovl->call(&arg);
                int i = 0;
                for (auto u : cand)
                  u->m_flag = org[i++];
                return;
              }

              // (*1) and bellow are coded for replacing special ctor version
              // See record_type.cpp base_ctor_dtor::operator()
              ctor->m_flag = usr::flag_t(ctor->m_flag & ~usr::INLINE);
              call_impl::wrapper(ctor, &arg, this_ptr);
              ctor->m_flag = flag;
            }
            inline usr* get_this(scope* param, const record_type* rec)
            {
              vector<scope*>& children = param->m_children;
              if (children.empty()) {
                block* bp = new block;
                assert(class_or_namespace_name::before.back() == bp);
		class_or_namespace_name::before.pop_back();
                param->m_children.push_back(bp);
                bp->m_parent = param;
              }
              map<string, vector<usr*> >& usrs = param->m_usrs;
              typedef map<string, vector<usr*> >::const_iterator IT;
              IT p = usrs.find(this_name);
              if (p != usrs.end()) {
                const vector<usr*>& v = p->second;
                assert(v.size() == 1);
                usr* this_ptr = v.back();
                return this_ptr;
              }
              const type* T = pointer_type::create(rec);
              usr* this_ptr = new usr(this_name, T, usr::NONE,
                                      parse::position, usr::NONE2);
              usrs[this_name].push_back(this_ptr);
              vector<usr*>& order = param->m_order;
              vector<usr*> tmp = order;
              order.clear();
              order.push_back(this_ptr);
              copy(begin(tmp), end(tmp), back_inserter(order));
              return this_ptr;
            }
            map<usr*, map<tag*, pbc> > btbl;
            inline void direct_base(base* bp, const record_type* rec,
                                    tag* btag, EXPRS* exprs, usr* ctor)
            {
              const map<base*, int>& bo = rec->base_offset();
              map<base*, int>::const_iterator q = bo.find(bp);
              assert(q != bo.end());
              int offset = q->second;
              scope* param = fundef::current->m_param;
              usr* this_ptr = get_this(param, rec);
              vector<scope*>& c = param->m_children;
              assert(!c.empty());
              scope* ps = c.back();
              assert(ps->m_id == scope::BLOCK);
              block* b = static_cast<block*>(ps);
              scope* org = scope::current;
              scope::current = b;
              assert(code.empty());
              gen(bp->m_tag, this_ptr, offset, exprs);
              scope::current = org;
              btbl[ctor][btag] = pbc(param, b, code);
              code.clear();
            }
            inline void common_case(const record_type* x,
                                    const record_type* rec,
                                    tag* btag, EXPRS* exprs, usr* ctor)
            {
              typedef const record_type REC;
              const map<REC*, int>& vco = rec->virt_common_offset();
              typedef map<REC*, int>::const_iterator IT;
              IT p = vco.find(x);
              assert(p != vco.end());
              int offset = p->second;
              scope* param = fundef::current->m_param;
              usr* this_ptr = get_this(param, rec);
              vector<scope*>& c = param->m_children;
              assert(!c.empty());
              scope* ps = c.back();
              assert(ps->m_id == scope::BLOCK);
              block* b = static_cast<block*>(ps);
              scope* org = scope::current;
              scope::current = b;
              assert(code.empty());
              gen(x->get_tag(), this_ptr, offset, exprs);
              scope::current = org;
              btbl[ctor][btag] = pbc(param, b, code);
              code.clear();
            }
            void tag_action(tag* btag, EXPRS* exprs, usr* ctor)
            {
              scope* tmp = ctor->m_scope;
              assert(tmp->m_id == scope::TAG);
              tag* ptr = static_cast<tag*>(tmp);
              if (!ptr->m_bases) {
                error::not_implemented();
                return;
              }
              const type* T = ptr->m_types.second;
              if (!T) {
                assert(scope::current->m_id == scope::PARAM);
                for_parse[ctor].push_back(make_pair(new PAIR(0, btag),exprs));
                return;
              }
              assert(T->m_id == type::RECORD);
              typedef const record_type REC;
              REC* rec = static_cast<REC*>(T);
              vector<base*>& bases = *ptr->m_bases;
              typedef vector<base*>::const_iterator ITx;
              ITx p = find_if(begin(bases), end(bases), 
                              [btag](base* bp){ return bp->m_tag == btag; });
              if (p != end(bases))
                return direct_base(*p, rec, btag, exprs, ctor);
              const vector<REC*>& common = rec->common();
              typedef vector<REC*>::const_iterator ITy;
              ITy q = find_if(begin(common), end(common),
                              [btag](REC* x){ return x->get_tag() == btag; });
              if (q != end(common))
                return common_case(*q, rec, btag, exprs, ctor);
              error::not_implemented();
            }
            void action(pair<usr*, tag*>* x, EXPRS* exprs)
            {
              auto_ptr<pair<usr*, tag*> > sweeper(x);
              assert(fundef::current);
              usr* ctor = fundef::current->m_usr;
              usr::flag_t flag = ctor->m_flag;
              if (!(flag & usr::CTOR)) {
                error::not_implemented();
                return;
              }
              usr* u = x->first;
              tag* ptr = x->second;
              if (ptr) {
                const type* T = ptr->m_types.first;
                if (T->m_id == type::TEMPLATE_PARAM) {
                  if (T = ptr->m_types.second)
                    ptr = T->get_tag();
                }
              }
              u ? id_action(u, exprs, ctor) : tag_action(ptr, exprs, ctor);
            }
          } // end of namespace mem_initializer
        } // end of namespace definition
      } // end of namespace function
    } // end of namespace declarators
  } // end of namespace declarations
}  // end of namespace cxx_compiler


cxx_compiler::base* cxx_compiler::create_base(int access, bool virt, usr* u)
{
  usr::flag_t flag = u->m_flag;
  assert(flag & usr::TYPEDEF);
  const type* T = u->m_type;
  assert(T);
  tag* ptr = T->get_tag();
  if (!ptr)
    error::not_implemented();
  return new base(access, virt, ptr);
}
