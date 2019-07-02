#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"
#include "cxx_y.h"

namespace cxx_compiler {
  using namespace std;
  struct pure_virt_value : constant<void*> {
    usr* m_usr;
    pure_virt_value(usr* u, constant<void*>* c)
      : m_usr(u), constant<void*>(*c)
    {
      m_flag2 = usr::PURE_VIRT_VALUE;
    }
  };
  struct ambiguous_override : usr {
    usr* m_org;
    ambiguous_override(usr* x, usr* y) : usr(*y), m_org(x)
    {
      m_flag2 = usr::AMBIGUOUS_OVERRIDE;
    }
  };
  inline usr* get_vf(var* v)
  {
    if (addrof* a = v->addrof_cast()) {
      v = a->m_ref;
      assert(v->usr_cast());
      return static_cast<usr*>(v);
    }
    assert(v->usr_cast());
    usr* u = static_cast<usr*>(v);
    usr::flag2_t flag2 = u->m_flag2;
    if (flag2 & usr::PURE_VIRT_VALUE) {
      pure_virt_value* pv = static_cast<pure_virt_value*>(u);
      return pv->m_usr;
    }
    assert(flag2 & usr::AMBIGUOUS_OVERRIDE);
    return u;
  }
  bool match_vf(pair<int, var*> p, usr* y)
  {
    var* v = p.second;
    usr* x = get_vf(v);
    string xn = x->m_name;
    string yn = y->m_name;
    if (xn != yn && xn[0] != '~' && yn[0] != '~')
      return false;
    const type* Tx = x->m_type;
    assert(Tx->m_id == type::FUNC);
    const type* Ty = y->m_type;
    assert(Ty->m_id == type::FUNC);
    typedef const func_type FT;
    FT* Fx = static_cast<FT*>(Tx);
    FT* Fy = static_cast<FT*>(Ty);
    const vector<const type*>& px = Fx->param();
    const vector<const type*>& py = Fy->param();
    if (px.size() != py.size())
      return false;
    if (mismatch(px.begin(),px.end(),py.begin(),cxx_compiler::compatible)
        != make_pair(px.end(),py.end())) {
      return false;
    }
    Tx = Fx->return_type();
    Ty = Fy->return_type();
    if (!Tx || !Ty) {
      assert(Tx || (x->m_flag & usr::DTOR));
      assert(Ty || (y->m_flag & usr::DTOR));
      return Tx == Ty;
    }
    if (compatible(Tx, Ty))
      return true;
    if (Tx->m_id != type::POINTER) {
      error::virtual_function::return_only(x, y);
      return false;
    }
    if (Ty->m_id != type::POINTER) {
      error::virtual_function::return_only(x, y);
      return false;
    }
    typedef const pointer_type PT;
    PT* Px = static_cast<PT*>(Tx);
    PT* Py = static_cast<PT*>(Ty);
    Tx = Px->referenced_type();
    Ty = Py->referenced_type();
    Tx = Tx->complete_type();
    Ty = Ty->complete_type();
    if (Tx->m_id != type::RECORD) {
      error::virtual_function::return_only(x, y);
      return false;
    }
    if (Ty->m_id == type::INCOMPLETE_TAGGED) {
      tag* xtag = Tx->get_tag();
      tag* ytag = Ty->get_tag();
      string name = xtag->m_name;
      int r = parse::identifier::base_lookup::action(name, ytag);
      if (r != CLASS_NAME_LEX) {
	error::virtual_function::return_only(x, y);
	return false;
      }
      return true;
    }
    if (Ty->m_id != type::RECORD) {
      error::virtual_function::return_only(x, y);
      return false;
    }
    typedef const record_type REC;
    REC* Rx = static_cast<REC*>(Tx);
    REC* Ry = static_cast<REC*>(Ty);
    vector<route_t> dummy;
    bool ambiguous = false;
    int offset = calc_offset(Ry, Rx, dummy, &ambiguous);
    if (ambiguous)
      error::not_implemented();
    return offset >= 0;
  }
  namespace record_impl {
    struct layouter {
      insert_iterator<map<string, pair<int, usr*> > > X;
      insert_iterator<map<usr*, int> > Y;
      usr* m_last;
      struct current {
        usr* m_member;
        const type* m_integer;
        int m_position;
        current(usr* member = 0, const type* T = 0)
          : m_member(member), m_integer(T), m_position(0) {}
      } m_current;
      int operator()(int, usr*);
      layouter(insert_iterator<map<string, pair<int, usr*> > > XX,
               insert_iterator<map<usr*, int> > YY,
               usr* last)
        : X(XX), Y(YY), m_last(last) {}
    };
    struct grounder {
      insert_iterator<map<usr*, int> > Y;
      pair<string, pair<int, usr*> > operator()(usr*);
      grounder(insert_iterator<map<usr*, int> > YY) : Y(YY) {}
    };
    inline int add_size(int n, const type* T)
    {
      int m = T->size();
      assert(m);
      return n + m;
    }
    void calc_common(const vector<const record_type*>& x,
                     const vector<const record_type*>& y,
                     vector<const record_type*>& common)
    {
      typedef const record_type REC;
      typedef vector<REC*>::const_iterator IT;
      vector<REC*> tmp;
      for (auto p : x) {
        for (auto q : y) {
          if (p == q)
            tmp.push_back(p);
        }
      }
      for (auto p : tmp) {
        IT q = find(begin(common), end(common), p);
        if (q == end(common))
          common.push_back(p);
      }
    }
    inline void insert(base* bp, set<const record_type*>& common)
    {
      assert(bp->m_flag & usr::VIRTUAL);
      tag* ptr = bp->m_tag;
      const type* T = ptr->m_types.second;
      assert(T);
      assert(T->m_id == type::RECORD);
      typedef const record_type REC;
      REC* rec = static_cast<REC*>(T);
      common.insert(rec);
    }
    inline void gather(base* xbp, base* ybp, vector<const record_type*>& tmp)
    {
      tag* xtag = xbp->m_tag;
      const type* Tx = xtag->m_types.second;
      assert(Tx);
      assert(Tx->m_id == type::RECORD);
      typedef const record_type REC;
      REC* xrec = static_cast<REC*>(Tx);
      tag* ytag = ybp->m_tag;
      const type* Ty = ytag->m_types.second;
      assert(Ty);
      assert(Ty->m_id == type::RECORD);
      typedef const record_type REC;
      REC* yrec = static_cast<REC*>(Ty);
      const vector<REC*>& x = xrec->virt_ancestor();
      const vector<REC*>& y = yrec->virt_ancestor();
      calc_common(x, y, tmp);
    }
    inline int base_layouter(int n, base* bp, map<base*, int>& base_offset,
			     const vector<const record_type*>& tmp)
    {
      assert(!(bp->m_flag & usr::VIRTUAL));
      tag* ptr = bp->m_tag;
      const type* T = ptr->m_types.second;
      assert(T);
      int m = T->size();
      assert(m);
      m -= accumulate(begin(tmp), end(tmp), 0, add_size);
      assert(m > 0);
      base_offset[bp] = n;
      return n + m;
    }
    struct vbase_layouter {
      map<base*, int>& m_base_offset;
      set<const record_type*>& m_common;
      map<const record_type*, int>& m_common_offset;
      vbase_layouter(map<base*, int>& bo, set<const record_type*>& c,
                     map<const record_type*, int>& co)
        : m_base_offset(bo), m_common(c), m_common_offset(co) {}
      int operator()(int n, base* bp)
      {
        if (!(bp->m_flag & usr::VIRTUAL)) {
	  assert(m_base_offset.find(bp) != m_base_offset.end());
          return n;
        }
        assert(m_base_offset.find(bp) == m_base_offset.end());
        m_base_offset[bp] = n;
        tag* ptr = bp->m_tag;
        const type* T = ptr->m_types.second;
        assert(T->m_id == type::RECORD);
        typedef const record_type REC;
        REC* rec = static_cast<REC*>(T);
        typedef set<const record_type*>::iterator IT;
        IT p = m_common.find(rec);
        if (p != end(m_common)) {
          m_common.erase(p);
          m_common_offset[rec] = n;
        }
        int m = T->size();
        assert(m > 0);
        return n + m;
      }
    };
    struct add_common {
      map<const record_type*, int>& m_common_offset;
      add_common(map<const record_type*, int>& common_offset)
        : m_common_offset(common_offset) {}
      int operator()(int n, const record_type* rec)
      {
        m_common_offset[rec] = n;
        return add_size(n, rec);
      }
    };
    inline int base_vcommon(tag* ptr, string vtbl_name)
    {
      const map<string, vector<usr*> >& usrs = ptr->m_usrs;
      typedef map<string, vector<usr*> >::const_iterator IT;
      IT p = usrs.find(vtbl_name);
      if (p == usrs.end())
        return 0;
      const vector<usr*>& v = p->second;
      assert(v.size() == 1);
      usr* u = v.back();
      assert(u->m_flag & usr::WITH_INI);
      with_initial* vtbl = static_cast<with_initial*>(u);
      return vtbl->m_value.size();
    }
    inline int vbase_vf(int n, const record_type* rec)
    {
      tag* ptr = rec->get_tag();
      return n + base_vcommon(ptr, vftbl_name);
    }
    inline int base_vf(int n, const base* b)
    {
      if (b->m_flag & usr::VIRTUAL)
        return n;
      tag* ptr = b->m_tag;
      const type* T = ptr->m_types.second;
      assert(T->m_id == type::RECORD);
      typedef const record_type REC;
      REC* rec = static_cast<REC*>(T);
      const vector<REC*>& va = rec->virt_ancestor();
      int x = base_vcommon(b->m_tag, vftbl_name);
      int y = accumulate(begin(va), end(va), 0, vbase_vf);
      assert(x - y >= 0);
      return n + x - y;
    }
    int base_vb(int n, const base* b)
    {
      return n + base_vcommon(b->m_tag, vbtbl_name);
    }
    struct base_vb_t {
      vector<const record_type*>& m_virt_ancestor;
      base_vb_t(vector<const record_type*>& v) : m_virt_ancestor(v) {}
      int operator()(int n, const base* bp)
      {
        tag* ptr = bp->m_tag;
        const type* T = ptr->m_types.second;
        assert(T);
        assert(T->m_id == type::RECORD);
        typedef const record_type REC;
        REC* rec = static_cast<REC*>(T);
        const vector<const record_type*>& va = rec->virt_ancestor();
        copy(begin(va), end(va), back_inserter(m_virt_ancestor));
        return base_vb(n, bp);
      }
    };
    struct direct_virt {
      vector<const record_type*>& m_virt_ancestor;
      direct_virt(vector<const record_type*>& v) : m_virt_ancestor(v) {}
      bool operator()(const base* bp)
      {
        if (!(bp->m_flag & usr::VIRTUAL))
          return false;
        tag* ptr = bp->m_tag;
        const type* T = ptr->m_types.second;
        assert(T);
        assert(T->m_id == type::RECORD);
        typedef const record_type REC;
        REC* rec = static_cast<REC*>(T);
        m_virt_ancestor.push_back(rec);
        return true;
      }
    };
    struct add_if {
      map<int, var*>& m_result;
      const vector<const record_type*>* m_va;
      add_if(map<int, var*>& result, const vector<const record_type*>* va)
        : m_result(result), m_va(va) {}
      static bool match(const record_type* rec, usr* vf)
      {
        tag* ptr = rec->get_tag();
	const map<string, vector<usr*> >& usrs = ptr->m_usrs;
	typedef map<string, vector<usr*> >::const_iterator ITx;
	ITx p = usrs.find(vftbl_name);
	if (p == usrs.end())
	  return false;
	const vector<usr*>& v = p->second;
	assert(v.size() == 1);
	usr* u = v.back();
	usr::flag_t flag = u->m_flag;
	assert(flag & usr::WITH_INI);
	with_initial* wi = static_cast<with_initial*>(u);
	const map<int, var*>& value = wi->m_value; 
	typedef map<int, var*>::const_iterator ITy;
	ITy q = find_if(begin(value), end(value),
			bind2nd(ptr_fun(match_vf), vf));
	return q != end(value);
      }
      static var* override(var* x, var* y)
      {
	usr* vfx = get_vf(x);
	usr* vfy = get_vf(y);
	scope* px = vfx->m_scope;
	scope* py = vfy->m_scope;
	assert(px->m_id == scope::TAG);
	assert(py->m_id == scope::TAG);
	tag* ptx = static_cast<tag*>(px);
	tag* pty = static_cast<tag*>(py);
	const type* Tx = ptx->m_types.second;
	const type* Ty = pty->m_types.second;
	assert(Tx->m_id == type::RECORD);
	assert(Ty->m_id == type::RECORD);
	typedef const record_type REC;
	REC* rx = static_cast<REC*>(Tx);
	REC* ry = static_cast<REC*>(Ty);
	vector<route_t> dummy;
	bool ambiguous = false;
	int offset = calc_offset(ry, rx, dummy, &ambiguous);
	if (ambiguous)
	  error::not_implemented();
	if (offset >= 0)
	  return y;
	return new ambiguous_override(vfx, vfy);
      }
      int operator()(int off, pair<int, var*> x)
      {
        var* v = x.second;
        if (m_va) {
	  usr* vf = get_vf(v);
	  typedef const record_type REC;
          typedef vector<REC*>::const_iterator IT;
	  IT p = find_if(begin(*m_va), end(*m_va),bind2nd(ptr_fun(match), vf));
	  if (p != end(*m_va)) {
	    typedef map<int, var*>::iterator IT;
	    IT r = find_if(begin(m_result), end(m_result),
			   bind2nd(ptr_fun(match_vf), vf));
	    assert(r != end(m_result));
	    r->second = override(r->second, v);
	    return off;
	  }
        }
        m_result[off] = v;
        const type* T = void_type::create();
        T = pointer_type::create(T);
        return off + T->size();
      }
    };
    inline int copy_base_vf_common(int offset, tag* ptr,
                                   map<int, var*>& result,
                                   const vector<const record_type*>* va)
    {
      const map<string, vector<usr*> >& usrs = ptr->m_usrs;
      typedef map<string, vector<usr*> >::const_iterator IT;
      IT p = usrs.find(vftbl_name);
      if (p == usrs.end())
        return offset;
      const vector<usr*>& v = p->second;
      assert(v.size() == 1);
      usr* u = v.back();
      assert(u->m_flag & usr::WITH_INI);
      with_initial* w = static_cast<with_initial*>(u);
      const map<int, var*>& src = w->m_value;
      return accumulate(begin(src), end(src), offset, add_if(result, va));
    } 
    struct copy_base_vf {
      map<int, var*>& m_value;
      map<base*, int>& m_vftbl_offset;
      copy_base_vf(map<int, var*>& value, map<base*, int>& vftbl_offset)
        : m_value(value), m_vftbl_offset(vftbl_offset) {}
      int operator()(int offset, base* b)
      {
        if (b->m_flag & usr::VIRTUAL)
          return offset;
        tag* ptr = b->m_tag;
        const type* T = ptr->m_types.second;
        assert(T->m_id == type::RECORD);
        typedef const record_type REC;
        REC* rec = static_cast<REC*>(T);
        const vector<REC*>& va = rec->virt_ancestor();
        int n = copy_base_vf_common(offset, ptr, m_value, &va);
        if (n != offset)
          m_vftbl_offset[b] = offset;
        return n;
      }
    };
    struct copy_vbase_vf {
      map<int, var*>& m_value;
      map<const record_type*, int>& m_common_vftbl_offset;
      copy_vbase_vf(map<int, var*>& value,
                    map<const record_type*, int>& common_vftbl_offset)
        : m_value(value), m_common_vftbl_offset(common_vftbl_offset) {}
      int operator()(int offset, const record_type* rec)
      {
        tag* ptr = rec->get_tag();
        int n = copy_base_vf_common(offset, ptr, m_value, 0);
        assert(n != offset);
        m_common_vftbl_offset[rec] = offset;
        return n;
      }
    };
    struct override_vf {
      map<int, var*>& m_value;
      override_vf(map<int, var*>& value) : m_value(value) {}
      void operator()(usr* u)
      {
        usr::flag_t flag = u->m_flag;
        if (!(flag & usr::FUNCTION))
          return;
        typedef map<int, var*>::iterator IT;
        IT p = find_if(begin(m_value), end(m_value),
                       bind2nd(ptr_fun(match_vf), u));
        if (p != end(m_value)) {
          u->m_flag = usr::flag_t(u->m_flag | usr::OVERRIDE);
          const type* T = u->m_type;
          T = pointer_type::create(T);
          p->second = new addrof(T, u, 0);
        }
      }
    };
    inline void check_override(pair<int, var*> x, tag* ptr)
    {
      var* v = x.second;
      ambiguous_override* p = dynamic_cast<ambiguous_override*>(v);
      if (!p)
	return;
      usr* vfx = p->m_org;
      usr* vfy = p;
      error::virtual_function::ambiguous_override(ptr, vfx, vfy);
    }
    struct own_vf {
      map<int, var*>& m_value;
      own_vf(map<int, var*>& value) : m_value(value) {}
      int operator()(int offset, usr* u)
      {
        using namespace expressions::primary::literal;
        usr::flag_t flag = u->m_flag;
        if (!(flag & usr::VIRTUAL))
          return offset;
        assert(flag & usr::FUNCTION);
        const type* T = u->m_type;
        T = pointer_type::create(T);
        if (flag & usr::PURE_VIRT) {
          var* tmp = integer::create(0)->cast(T);
          constant<void*>* c = static_cast<constant<void*>*>(tmp);
          m_value[offset] = new pure_virt_value(u, c);
        }
        else
          m_value[offset] = new addrof(T, u, 0);
        return offset + T->size();
      }
    };
    bool comp_size(usr*, usr*);
    bool comp_align(usr*, usr*);
    bool member_modifiable(usr*);
    bool base_modifiable(base*);
    struct set_org_vbtbl_subr {
      int* m_offset;
      int m_base_offset;
      map<const record_type*, int>& m_common_offset;
      set_org_vbtbl_subr(int* offset, int base_offset,
                         map<const record_type*, int>& common_offset)
        : m_offset(offset), m_base_offset(base_offset),
          m_common_offset(common_offset) {}
      pair<int, var*> operator()(const record_type* rec, pair<int, var*> org)
      {
        using namespace expressions::primary::literal;
        typedef map<const record_type*, int>::const_iterator IT;
        int delta = org.second->m_type->size();
        IT p = m_common_offset.find(rec);
        if (p != m_common_offset.end()) {
          int common_offset = p->second;
          int n = common_offset - m_base_offset;
          assert(n > 0);
          var* v = integer::create(n);
          pair<int, var*> ret(*m_offset, v);
          *m_offset += delta;
          return ret;
        }
        *m_offset += delta;
        return org;
      }
    };
    struct set_org_vbtbl {
      map<int, var*>& m_value;
      const map<base*, int>& m_base_offset;
      map<const record_type*, int>& m_common_offset;
      map<base*, int>& m_vbtbl_offset;
      set_org_vbtbl(map<int, var*>& value,
                    const map<base*, int>& base_offset,
                    map<const record_type*, int>& common_offset,
                    map<base*, int>& vbtbl_offset)
        : m_value(value), m_base_offset(base_offset),
          m_common_offset(common_offset), m_vbtbl_offset(vbtbl_offset) {}
      int operator()(int offset, base* bp)
      {
        typedef map<base*, int>::const_iterator IT;
        IT p = m_base_offset.find(bp);
        assert(p != m_base_offset.end());
        int base_offset = p->second;
        tag* ptr = bp->m_tag;
        const type* T = ptr->m_types.second;
        assert(T);
        assert(T->m_id == type::RECORD);
        typedef const record_type REC;
        REC* rec = static_cast<REC*>(T);
        const vector<REC*>& va = rec->virt_ancestor();
        if (va.empty())
          return offset;
        m_vbtbl_offset[bp] = offset;
        const map<string, vector<usr*> >& usrs = ptr->m_usrs;
        typedef map<string, vector<usr*> >::const_iterator IT2;
        IT2 q = usrs.find(vbtbl_name);
        assert(q != end(usrs));
        const vector<usr*>& v = q->second;
        usr* u = v.back();
        usr::flag_t flag = u->m_flag;
        assert(flag & usr::WITH_INI);
        with_initial* wi = static_cast<with_initial*>(u);
        const map<int, var*>& org = wi->m_value;
        assert(va.size() == org.size());
        transform(begin(va), end(va), begin(org),
                  inserter(m_value, m_value.begin()),
                  set_org_vbtbl_subr(&offset, base_offset, m_common_offset));
        return offset;
      }
    };
    struct set_own_vbtbl {
      int m_offset;
      map<int, var*>& m_value;
      const map<base*, int>& m_base_offset;
      set_own_vbtbl(int offset,
                    map<int, var*>& value,
                    const map<base*, int>& base_offset)
        : m_offset(offset), m_value(value), m_base_offset(base_offset) {}
      void operator()(base* b)
      {
        using namespace expressions::primary::literal;
        if (!(b->m_flag & usr::VIRTUAL))
          return;
        typedef map<base*, int>::const_iterator IT; 
        IT p = m_base_offset.find(b);
        assert(p != m_base_offset.end());
        int base_offset = p->second;
        var* v = integer::create(base_offset);
        assert(m_value.find(m_offset) == m_value.end());
        m_value[m_offset] = v;
        m_offset += v->m_type->size();
      }
    };
    inline usr* get_virt(tag* ptr, string virt_name)
    {
      const map<string, vector<usr*> >& usrs = ptr->m_usrs;
      typedef map<string, vector<usr*> >::const_iterator IT;
      IT p = usrs.find(virt_name);
      if (p == usrs.end())
        return 0;
      const vector<usr*>& v = p->second;
      assert(v.size() == 1);
      usr* u = v.back();
      return u;
    }
    inline usr* get_vfptr(tag* ptr)
    {
      return get_virt(ptr, vfptr_name);
    }
    inline usr* get_vbptr(tag* ptr)
    {
      return get_virt(ptr, vbptr_name);
    }
    inline usr* get_vftbl(tag* ptr)
    {
      return get_virt(ptr, vftbl_name);
    }
    inline usr* get_vbtbl(tag* ptr)
    {
      return get_virt(ptr, vbtbl_name);
    }
    inline const func_type* default_ctor_type()
    {
      vector<const type*> tmp;
      tmp.push_back(void_type::create());
      return func_type::create(0, tmp);
    }
    inline void common_ctor_dtor(tag* ptr, usr* this_ptr, block* pb,
				 bool is_dtor, int offset)
    {
      string tgn = ptr->m_name;
      if (is_dtor)
	tgn = '~' + tgn;
      map<string, vector<usr*> >& usrs = ptr->m_usrs;
      map<string, vector<usr*> >::const_iterator p = usrs.find(tgn);
      if (p == usrs.end())
	return;
      const vector<usr*>& v = p->second;
      const func_type* ft = default_ctor_type();
      vector<usr*>::const_iterator r =
	find_if(begin(v), end(v), [ft](usr* u){
	    const type* T = u->m_type;
	    return T ? compatible(T, ft) : false;
	  });
      if (r == end(v))
	return;
      usr* tor = *r;
      const type* T = ptr->m_types.second;
      assert(T->m_id == type::RECORD);
      T = pointer_type::create(T);
      var* tmp = new var(T);
      pb->m_vars.push_back(tmp);
      code.push_back(new cast3ac(tmp, this_ptr, T));
      if (offset) {
	using namespace expressions::primary::literal;
	var* off = integer::create(offset);
	code.push_back(new add3ac(tmp, tmp, off));
      }
      usr::flag_t org = tor->m_flag;
      tor->m_flag = usr::flag_t(tor->m_flag & ~usr::VIRTUAL);
      scope* org2 = scope::current;
      scope::current = pb;
      call_impl::wrapper(tor, 0, tmp);
      scope::current = org2;
      tor->m_flag = org;
    }
    struct base_ctor_dtor {
      const map<base*, int>& m_base_offset;
      usr* m_this;
      block* m_block;
      bool m_is_dtor;
      usr* m_ctor;
      base_ctor_dtor(const map<base*, int>& base_offset, usr* this_ptr,
		     block* b, bool is_dtor, usr* ctor)
        : m_base_offset(base_offset), m_this(this_ptr),
	  m_block(b), m_is_dtor(is_dtor), m_ctor(ctor)
      {
	is_dtor ? assert(!m_ctor) : assert(m_ctor);
      }
      void operator()(base* pb)
      {
	using namespace declarations::declarators::function::definition;
	using namespace mem_initializer;
        tag* ptr = pb->m_tag;
	if (!m_is_dtor) {
	  typedef map<usr*, BTBL_VALUE>::iterator ITx;
	  ITx p = btbl.find(m_ctor);
	  if (p != btbl.end()) {
	    BTBL_VALUE& tbl = p->second;
	    typedef BTBL_VALUE::iterator ITy;
	    ITy q = tbl.find(ptr);
	    if (q != tbl.end()) {
	      const vector<tac*>& v = q->second;
	      copy(begin(v), end(v), back_inserter(code));
	      tbl.erase(q);
	      return;
	    }
	  }
	}
	typedef map<base*, int>::const_iterator IT;
	IT q = m_base_offset.find(pb);
	assert(q != m_base_offset.end());
	int offset = q->second;
	common_ctor_dtor(ptr, m_this, m_block, m_is_dtor, offset);
      }
    };
    struct member_ctor_dtor {
      const map<string, pair<int, usr*> >& m_layout;
      usr* m_this;
      block* m_block;
      bool m_is_dtor;
      usr* m_ctor;
      member_ctor_dtor(const map<string, pair<int, usr*> >& layout,
		       usr* this_ptr, block* b, bool is_dtor, usr* ctor)
	: m_layout(layout), m_this(this_ptr), m_block(b), m_is_dtor(is_dtor),
	  m_ctor(ctor)
      {
	if (m_is_dtor)
	  assert(!m_ctor);
	else
	  assert(m_ctor);
      }
      void operator()(usr* u)
      {
	using namespace declarations::declarators::function::definition;
	using namespace mem_initializer;
	if (!m_is_dtor) {
	  typedef map<usr*, MTBL_VALUE>::iterator ITx;
	  ITx p = mtbl.find(m_ctor);
	  if (p != mtbl.end()) {
	    MTBL_VALUE& tbl = p->second;
	    typedef MTBL_VALUE::iterator ITy;
	    ITy q = tbl.find(u);
	    if (q != tbl.end()) {
	      const vector<tac*>& v = q->second;
	      copy(begin(v), end(v), back_inserter(code));
	      tbl.erase(q);
	      return;
	    }
	  }
	}
	const type* T = u->m_type;
	T = T->unqualified();
	if (T->m_id != type::RECORD)
	  return;
	typedef const record_type REC;
	REC* rec = static_cast<REC*>(T);
	tag* ptr = rec->get_tag();
	string name = u->m_name;
	typedef map<string, pair<int, usr*> >::const_iterator IT;
	IT p = m_layout.find(name);
	assert(p != m_layout.end());
	pair<int, usr*> off = p->second;
	int offset = off.first;
	common_ctor_dtor(ptr, m_this, m_block, m_is_dtor, offset);
      }
    };
    inline bool bases_have_ctor_dtor(tag* ptr, bool is_dtor)
    {
      if (!ptr->m_bases)
        return false;
      vector<base*>& bases = *ptr->m_bases;
      typedef vector<base*>::const_iterator IT;
      IT p = find_if(begin(bases), end(bases), [is_dtor](base* bp)
		     { return has_ctor_dtor(bp->m_tag, is_dtor); });
      return p != end(bases);
    }
    inline bool bases_have_ctor(tag* ptr)
    {
      return bases_have_ctor_dtor(ptr, false);
    }
    struct update_vptr {
      const map<base*, int>& m_base_offset;
      const map<base*, int>& m_vtbl_offset;
      var* m_this_ptr;
      block* m_block;
      var* m_vtbl_addr;
      string m_vptr_name;
      const map<const record_type*, int>* m_common_vftbl_offset;
      update_vptr(const map<base*, int>& base_offset,
                  const map<base*, int>& vtbl_offset,
                  var* this_ptr, block* bp, var* vtbl_addr,
                  string vptr_name,
                  const map<const record_type*, int>* common_vftbl_offset)
        : m_base_offset(base_offset), m_vtbl_offset(vtbl_offset),
          m_this_ptr(this_ptr), m_block(bp), m_vtbl_addr(vtbl_addr),
          m_vptr_name(vptr_name),
          m_common_vftbl_offset(common_vftbl_offset) {}
      int subr(base* bp, const record_type* rec)
      {
        map<base*, int>::const_iterator p = m_vtbl_offset.find(bp);
        if (p != m_vtbl_offset.end())
          return p->second;
        assert(m_common_vftbl_offset);
        typedef const record_type REC;
        map<REC*, int>::const_iterator q = m_common_vftbl_offset->find(rec);
        assert(q != m_common_vftbl_offset->end());
        return q->second;
      }
      void operator()(base* bp)
      {
        using namespace expressions::primary::literal;
        tag* ptr = bp->m_tag;
        if (ptr->m_bases) {
          vector<base*>& bases = *ptr->m_bases;
          const type* T = ptr->m_types.second;
          assert(T->m_id == type::RECORD);
          typedef const record_type REC;
          REC* rec = static_cast<REC*>(T);
          const map<base*, int>& base_offset = rec->base_offset();
          const map<base*, int>& vtbl_offset = m_vptr_name == vbptr_name ?
            rec->vbtbl_offset() : rec->vftbl_offset();
          T = pointer_type::create(rec);
          var* this_ptr = new var(T);
          m_block->m_vars.push_back(this_ptr);
          code.push_back(new cast3ac(this_ptr, m_this_ptr, T));
          map<base*, int>::const_iterator p = m_base_offset.find(bp);
          assert(p != m_base_offset.end());
          if (int offset = p->second) {
            var* off = integer::create(offset);
            var* tmp = new var(T);
            m_block->m_vars.push_back(tmp);
            code.push_back(new add3ac(tmp, this_ptr, off));
            this_ptr = tmp;
          }
          var* vtbl_addr = m_vtbl_addr;
          map<base*, int>::const_iterator q = m_vtbl_offset.find(bp);
          if (q != m_vtbl_offset.end()) {
            if (int offset = q->second) {
              var* off = integer::create(offset);
              var* tmp = new var(vtbl_addr->m_type);
              m_block->m_vars.push_back(tmp);
              code.push_back(new add3ac(tmp, vtbl_addr, off));
              vtbl_addr = tmp;
            }
            for_each(begin(bases), end(bases),
                     update_vptr(base_offset, vtbl_offset, this_ptr,
                                 m_block, vtbl_addr, m_vptr_name,
                                 m_common_vftbl_offset));
          }
        }
        usr* vptr = get_virt(ptr, m_vptr_name);
        if (!vptr)
          return;
        const type* T = ptr->m_types.second;
        assert(T->m_id == type::RECORD);
        typedef const record_type REC;
        REC* rec = static_cast<REC*>(T);
        pair<int, usr*> off = rec->offset(m_vptr_name);
        int vptr_off = off.first;
        assert(vptr_off >= 0);
        map<base*, int>::const_iterator p = m_base_offset.find(bp);
        assert(p != m_base_offset.end());
        vptr_off += p->second;
        T = vptr->m_type;
        var* t0 = new var(T);
        m_block->m_vars.push_back(t0);
        code.push_back(new cast3ac(t0, m_vtbl_addr, T));
        if (int vtbl_off = subr(bp, rec)) {
          var* vo = integer::create(vtbl_off);
          code.push_back(new add3ac(t0, t0, vo));
        }
        T = pointer_type::create(T);
        var* t1 = new var(T);
        m_block->m_vars.push_back(t1);
        code.push_back(new cast3ac(t1, m_this_ptr, T));
        if (vptr_off) {
          var* vo = integer::create(vptr_off);
          code.push_back(new add3ac(t1, t1, vo));
        }
        code.push_back(new invladdr3ac(t1, t0));
      }
    };
    struct update_common_vfptr {
      var* m_this_ptr;
      block* m_block;
      var* m_vftbl_addr;
      const map<const record_type*, int>& m_virt_common_offset;
      const map<const record_type*, int>& m_common_vftbl_offset;
      update_common_vfptr(var* this_ptr, block* pb,
                          var* vftbl_addr,
                          const map<const record_type*, int>& vco,
                          const map<const record_type*, int>& cvo)
        : m_this_ptr(this_ptr), m_block(pb), m_vftbl_addr(vftbl_addr),
          m_virt_common_offset(vco), m_common_vftbl_offset(cvo) {}
      void operator()(const record_type* rec)
      {
        using namespace expressions::primary::literal;
        typedef map<const record_type*, int>::const_iterator IT;

        pair<int, usr*> ret = rec->offset(vfptr_name);
        int vfptr_off = ret.first;
        if (vfptr_off < 0)
          return;
        usr* vfptr = ret.second;
        const type* T = vfptr->m_type;
        var* dst = new var(T);
        m_block->m_vars.push_back(dst);
        code.push_back(new cast3ac(dst, m_this_ptr, T));
        IT p = m_virt_common_offset.find(rec);
        assert(p != m_virt_common_offset.end());
        if (int vc_off = p->second) {
          var* tmp = new var(T);
          m_block->m_vars.push_back(tmp);
          var* off = integer::create(vc_off);
          code.push_back(new add3ac(tmp, dst, off));
          dst = tmp;
        }
        if (vfptr_off) {
          var* off = integer::create(vfptr_off);
          code.push_back(new add3ac(dst, dst, off));
        }

        var* src = m_vftbl_addr;
        IT q = m_common_vftbl_offset.find(rec);
        assert(q != m_common_vftbl_offset.end());
        if (int cv_off = q->second) {
          var* tmp = new var(src->m_type);
          m_block->m_vars.push_back(tmp);
          var* off = integer::create(cv_off);
          code.push_back(new add3ac(tmp, src, off));
          src = tmp;
        }

        code.push_back(new invladdr3ac(dst, src));
      }
    };
    void add_ctor_code(tag* ptr,
		       const map<string, pair<int, usr*> >& layout,
		       const map<base*, int>& base_offset,
		       const map<base*, int>& vbtbl_offset,
		       const map<base*, int>& vftbl_offset,
		       const set<const record_type*>& common,
		       const map<const record_type*, int>&
		       virt_common_offset,
		       const map<const record_type*, int>&
		       common_vftbl_offset,
		       const vector<usr*>& member,
		       block* pb,
		       usr* this_ptr,
		       usr* ctor,
		       scope* param)
    {
      if (ptr->m_bases) {
        vector<base*>& bases = *ptr->m_bases;
        scope* org = scope::current;
        scope::current = pb;
        for_each(begin(bases), end(bases),
                 base_ctor_dtor(base_offset, this_ptr, pb, false, ctor));
        scope::current = org;
        if (usr* vbtbl = get_vbtbl(ptr)) {
          const type* T = vbtbl->m_type;
          T = pointer_type::create(T);
          var* vbtbl_addr = new var(T);
          pb->m_vars.push_back(vbtbl_addr);
          code.push_back(new addr3ac(vbtbl_addr, vbtbl));
          for_each(begin(bases), end(bases),
                   update_vptr(base_offset, vbtbl_offset, this_ptr, pb,
                               vbtbl_addr, vbptr_name, 0));
        }
        if (usr* vftbl = get_vftbl(ptr)) {
          const type* T = vftbl->m_type;
          T = pointer_type::create(T);
          var* vftbl_addr = new var(T);
          pb->m_vars.push_back(vftbl_addr);
          code.push_back(new addr3ac(vftbl_addr, vftbl));
          for_each(begin(bases), end(bases),
                   update_vptr(base_offset, vftbl_offset, this_ptr, pb,
                               vftbl_addr, vfptr_name,
			       &common_vftbl_offset));
          for_each(begin(common), end(common),
                   update_common_vfptr(this_ptr, pb, vftbl_addr,
                                       virt_common_offset,
                                       common_vftbl_offset));
        }
      }
      if (usr* vbptr = get_vbptr(ptr)) {
        const type* T = vbptr->m_type;
        var* t0 = new var(T);
        pb->m_vars.push_back(t0);
        T = pointer_type::create(T);
        var* t1 = new var(T);
        pb->m_vars.push_back(t1);
        usr* vbtbl = get_vbtbl(ptr);
        code.push_back(new addr3ac(t0, vbtbl));
        code.push_back(new cast3ac(t1, this_ptr, T));
        typedef map<string, pair<int, usr*> >::const_iterator IT;
        IT p = layout.find(vbptr_name);
        assert(p != layout.end());
        pair<int, usr*> off = p->second;
        if (int offset = off.first) {
          using namespace expressions::primary::literal;
          var* off = integer::create(offset);
          code.push_back(new add3ac(t1,t1,off));
        }
        code.push_back(new invladdr3ac(t1, t0));
      }
      if (usr* vfptr = get_vfptr(ptr)) {
        const type* T = vfptr->m_type;
        var* t0 = new var(T);
        pb->m_vars.push_back(t0);
        T = pointer_type::create(T);
        var* t1 = new var(T);
        pb->m_vars.push_back(t1);
        usr* vftbl = get_vftbl(ptr);
        code.push_back(new addr3ac(t0, vftbl));
        code.push_back(new cast3ac(t1, this_ptr, T));
        typedef map<string, pair<int, usr*> >::const_iterator IT;
        IT p = layout.find(vfptr_name);
        assert(p != layout.end());
        pair<int, usr*> off = p->second;
        if (int offset = off.first) {
          using namespace expressions::primary::literal;
          var* off = integer::create(offset);
          code.push_back(new add3ac(t1,t1,off));
        }
        code.push_back(new invladdr3ac(t1, t0));
      }
      for_each(begin(member), end(member),
	       member_ctor_dtor(layout, this_ptr, pb, false, ctor));
    }
    
    inline bool member_have_ctor_dtor(const vector<usr*>& member,
				      bool is_dtor)
    {
      typedef vector<usr*>::const_iterator IT;
      IT p = find_if(begin(member), end(member), [is_dtor](usr* u)
		     {
		       const type* T = u->m_type;
		       T = T->unqualified();
		       if (T->m_id != type::RECORD)
			 return false;
		       typedef const record_type REC;
		       REC* rec = static_cast<REC*>(T);
		       tag* ptr = rec->get_tag();
		       return has_ctor_dtor(ptr, is_dtor) != 0;
		     });
      return p != end(member);
    }
    inline bool member_have_ctor(const vector<usr*>& member)
    {
      return member_have_ctor_dtor(member, false);
    }
    inline usr* add_ctor_dtor_common(tag* ptr, scope** param, usr** this_ptr,
				     block** pb, bool is_dtor)
    {
      if (has_ctor_dtor(ptr, is_dtor))
	return 0;
      string tgn = ptr->m_name;
      if (is_dtor)
	tgn = '~' + tgn;
      const func_type* ft = default_ctor_type();
      usr::flag_t flag = is_dtor ? usr::DTOR : usr::CTOR;
      flag = usr::flag_t(flag | usr::FUNCTION | usr::INLINE);
      usr* tor = new usr(tgn, ft, flag, file_t(),usr::NONE2);
      ptr->m_usrs[tgn].push_back(tor);
      using namespace declarations::declarators::function::definition;
      const vector<const type*>& parameter = ft->param();
      KEY key(make_pair(tgn, ptr), &parameter);
      dtbl[key] = tor;

      *param = new scope(scope::PARAM);
      using namespace class_or_namespace_name;
      assert(!before.empty());
      assert(before.back() == *param);
      before.pop_back();
      (*param)->m_parent = ptr;
      ptr->m_children.push_back(*param);
      string name = "this";
      const type* T = ptr->m_types.first;
      T = pointer_type::create(T);
      *this_ptr = new usr(name,T,usr::NONE,file_t(),usr::NONE2);
      (*this_ptr)->m_scope = *param;
      (*param)->m_order.push_back(*this_ptr);
      (*param)->m_usrs[name].push_back(*this_ptr);

      *pb = new block;
      assert(!before.empty());
      assert(before.back() == *pb);
      before.pop_back();
      (*pb)->m_parent = *param;
      (*param)->m_children.push_back(*pb);
      return tor;
    }
    void add_ctor(tag* ptr,
                  const map<string, pair<int, usr*> >& layout,
                  const map<base*, int>& base_offset,
                  const map<base*, int>& vbtbl_offset,
                  const map<base*, int>& vftbl_offset,
                  const set<const record_type*>& common,
                  const map<const record_type*, int>& virt_common_offset,
                  const map<const record_type*, int>& common_vftbl_offset,
		  const vector<usr*>& member)
    {
      if (!bases_have_ctor(ptr) && !get_vbptr(ptr) && !get_vfptr(ptr) &&
	  !member_have_ctor(member))
        return;

      block* pb; usr* this_ptr; scope* param;
      usr* ctor = add_ctor_dtor_common(ptr, &param, &this_ptr, &pb, false);
      if (!ctor)
	return;

      assert(code.empty());
      add_ctor_code(ptr, layout, base_offset, vbtbl_offset, vftbl_offset,
		    common, virt_common_offset, common_vftbl_offset,
		    member, pb, this_ptr, ctor, param);
      fundef* fdef = new fundef(ctor, param);
      declarations::declarators::function::definition::action(fdef, code);
    }
    inline bool member_have_dtor(const vector<usr*>& member)
    {
      return member_have_ctor_dtor(member, true);
    }
    inline bool bases_have_dtor(tag* ptr)
    {
      return bases_have_ctor_dtor(ptr, true);
    }
    void add_dtor_code(tag* ptr,
		       const map<string, pair<int, usr*> >& layout,
		       const vector<usr*>& member,
		       const map<base*, int>& base_offset,
		       block* pb,
		       usr* this_ptr)
    {
      for_each(rbegin(member), rend(member), 
	       member_ctor_dtor(layout, this_ptr, pb, true, 0));
      if (ptr->m_bases) {
	const vector<base*>& bases = *ptr->m_bases;
	for_each(rbegin(bases), rend(bases),
                 base_ctor_dtor(base_offset, this_ptr, pb, true, 0));
      }
    }
    void add_dtor(tag* ptr,
                  const map<string, pair<int, usr*> >& layout,
		  const vector<usr*>& member,
                  const map<base*, int>& base_offset,
		  with_initial* vftbl)
    {
      if (!member_have_dtor(member) && !bases_have_dtor(ptr))
        return;
      
      block* pb; usr* this_ptr; scope* param;
      usr* dtor = add_ctor_dtor_common(ptr, &param, &this_ptr, &pb, true);
      if (!dtor)
	return;

      if (vftbl) {
	map<int, var*>& value = vftbl->m_value;
	override_vf op(value);
	op(dtor);
      }

      assert(code.empty());
      scope* org = scope::current;
      scope::current = pb;
      add_dtor_code(ptr, layout, member, base_offset, pb, this_ptr);
      scope::current = org;
      fundef* fdef = new fundef(dtor, param);
      declarations::declarators::function::definition::action(fdef, code);
    }
    const usr::flag_t vtbl_flag
    = usr::flag_t(usr::WITH_INI| usr::STATIC | usr::STATIC_DEF);
    void dup_chk(const file_t& file, base* p, base* q)
    {
      tag* x = p->m_tag;
      tag* y = q->m_tag;
      if (x == y)
	error::classes::base::duplicate(file, x->m_name);
    }
    inline const reference_type* copy_ctor_arg_type(tag* ptr, bool const_f)
    {
      const type* T = ptr->m_types.first;
      if (const_f)
	T = const_type::create(T);
      return reference_type::create(T);
    }
    inline const func_type* copy_ctor_type(tag* ptr, bool const_f)
    {
      vector<const type*> tmp;
      const type* T = copy_ctor_arg_type(ptr, const_f);
      tmp.push_back(T);
      return func_type::create(0, tmp);
    }
    void add_copy_ctor(tag* ptr)
    {
      string tgn = ptr->m_name;
      const func_type* ft = copy_ctor_type(ptr, true);
      usr::flag_t flag =
	usr::flag_t(usr::CTOR | usr::FUNCTION | usr::INLINE);
      usr* ctor = new usr(tgn, ft, flag, file_t(),usr::NONE2);
      map<string, vector<usr*> >& usrs = ptr->m_usrs;
      map<string, vector<usr*> >::iterator p = usrs.find(tgn);
      assert(p != usrs.end());
      vector<usr*>& v = p->second;
      assert(!v.empty());
      usr* prev = v.back();
      usrs[tgn].push_back(ctor);
      usr* ovl = new overload(prev, ctor);
      usrs[tgn].push_back(ovl);

      using namespace declarations::declarators::function::definition;
      const vector<const type*>& parameter = ft->param();
      KEY key(make_pair(tgn, ptr), &parameter);
      dtbl[key] = ctor;

      using namespace class_or_namespace_name;
      scope* param = new scope(scope::PARAM);
      assert(before.back() == param);
      before.pop_back();
      scope::current->m_children.push_back(param);
      param->m_parent = scope::current;
      string name = "this";
      const type* T = ptr->m_types.second;
      assert(T);
      const type* pt = pointer_type::create(T);
      usr* this_ptr = new usr(name,pt,usr::NONE,file_t(),usr::NONE2);
      this_ptr->m_scope = param;
      param->m_order.push_back(this_ptr);
      param->m_usrs[name].push_back(this_ptr);

      string aname = new_name(".param");
      const reference_type* rt = copy_ctor_arg_type(ptr, true);
      usr* arg = new usr(aname, rt, usr::NONE, file_t(),usr::NONE2);
      arg->m_scope = param;
      param->m_order.push_back(arg);
      param->m_usrs[name].push_back(arg);

      block* bp = new block;
      assert(before.back() == bp);
      before.pop_back();
      param->m_children.push_back(bp);
      bp->m_parent = param;
      assert(code.empty());
      var* tmp = new var(T);
      bp->m_vars.push_back(tmp);
      code.push_back(new invraddr3ac(tmp, arg));
      code.push_back(new invladdr3ac(this_ptr, tmp));
      fundef* fdef = new fundef(ctor, param);
      declarations::declarators::function::definition::action(fdef, code);
    }
    inline usr* vdtor_entry(tag* ptr)
    {
      usr* dtor = has_ctor_dtor(ptr, true);
      if (!dtor)
	return 0;
      usr::flag_t flag = dtor->m_flag;
      return (flag & usr::VIRTUAL) ? dtor : 0;
    }
    inline usr* delete_entry(tag* ptr)
    {
      string name = operator_name(DELETE_KW);
      const map<string, vector<usr*> >& usrs = ptr->m_usrs;
      typedef map<string, vector<usr*> >::const_iterator IT;
      IT p = usrs.find(name);
      if (p == usrs.end())
	return 0;
      const vector<usr*>& v = p->second;
      return v.back();
    }
    struct virtual_delete : usr {
      usr* m_dtor;
      usr* m_del;
      virtual_delete(string name, const type* T, flag_t flag,
		     const file_t& file, flag2_t flag2, usr* dtor, usr* del)
	: usr(name, T, flag, file, flag2), m_dtor(dtor), m_del(del) {}
    };
    inline void add_vdel(tag* ptr, usr* dtor, usr* del)
    {
      string name = ptr->m_name + '.' + del->m_name;
      const type* T = void_type::create();
      vector<const type*> param;
      param.push_back(T);
      T = func_type::create(T, param);
      usr::flag_t flag = usr::flag_t(usr::FUNCTION | usr::VIRTUAL | usr::VDEL);
      virtual_delete* vdel =
	new virtual_delete(name, T, flag, file_t(), usr::NONE2, dtor, del);
      ptr->m_usrs[name].push_back(vdel);
      ptr->m_order.push_back(vdel);
    }
    struct add_override_vdel {
      usr* m_del;
      tag* m_tag;
      add_override_vdel(usr* del, tag* ptr) : m_del(del), m_tag(ptr) {}
      void operator()(const pair<int, var*>& x)
      {
	var* v = x.second;
	addrof* addr = v->addrof_cast();
	assert(addr);
	v = addr->m_ref;
	usr* u = v->usr_cast();
	if (!u)
	  return;
	if (!(u->m_flag & usr::VDEL))
	  return;
	virtual_delete* org = static_cast<virtual_delete*>(u);
	string name = u->m_name;
	const type* T = u->m_type;
	usr::flag_t flag = usr::flag_t(usr::FUNCTION | usr::VDEL);
	virtual_delete* vdel =
	  new virtual_delete(name, T, flag, file_t(), usr::NONE2,
			     org->m_dtor, m_del);
	m_tag->m_usrs[name].push_back(vdel);
	m_tag->m_order.push_back(vdel);
      }
    };
  } // end of namespace record_imp
} // end of namespace cxx_compiler

cxx_compiler::record_type::record_type(tag* ptr)
  : type(RECORD), m_size(0), m_modifiable(true), m_tag(ptr)
{
  using namespace std;
  using namespace record_impl;
  const vector<base*>* bases = m_tag->m_bases;
  with_initial* vbtbl = 0;
  if (bases) {
    const vector<file_t>& v = m_tag->m_file;
    assert(!v.empty());
    const file_t& file = v.back();
    for (auto p : *bases) {
      for (auto q : *bases)
	if (p < q)
	  dup_chk(file, p, q);
    }

    int nbvb = accumulate(begin(*bases), end(*bases), 0,
                          base_vb_t(m_virt_ancestor));
    int nvb = count_if(begin(*bases), end(*bases),
                       direct_virt(m_virt_ancestor));
    if (nbvb + nvb) {
      const type* T = int_type::create();
      T = const_type::create(T);
      T = array_type::create(T, nbvb + nvb);
      vbtbl = new with_initial(vbtbl_name,T,file_t());
      vbtbl->m_flag = vtbl_flag;
      map<string, vector<usr*> >& usrs = m_tag->m_usrs;
      usrs[vbtbl_name].push_back(vbtbl);
      if (nvb) {
        T = pointer_type::create(T);
        usr* vbptr = new usr(vbptr_name,T,usr::NONE,file_t(),usr::NONE2);
        usrs[vbptr_name].push_back(vbptr);
        m_member.push_back(vbptr);
        m_layout[vbptr_name] = make_pair(m_size, vbptr);
        m_position[vbptr] = 0;
        m_size += T->size();
      }
    }
  }
  const vector<usr*>& order = m_tag->m_order;
  int nvf = count_if(begin(order),end(order),
                     [](usr* u){ return u->m_flag & usr::VIRTUAL; });
  
  if (usr* dtor = has_ctor_dtor(m_tag, true)) {
    usr::flag_t flag = dtor->m_flag;
    if (flag & usr::VIRTUAL) {
      if (usr* del = delete_entry(m_tag)) {
	add_vdel(m_tag, dtor, del);
	++nvf;
      }
    }
  }
    
  int vfptr_offset = -1;
  if (nvf) {
    vfptr_offset = m_size;
    const type* T = void_type::create();
    T = pointer_type::create(T);
    m_size += T->size();
  }

  if (bases) {
    if (m_tag->m_kind != tag::UNION) {
      for (auto bp : *bases) {
	if (bp->m_flag & usr::VIRTUAL)
	  insert(bp, m_common);
	else {
	  vector<const record_type*> tmp;
	  for (auto bq : *bases) {
	    if (bp != bq)
	      gather(bp, bq, tmp);
	  }
	  copy(begin(tmp), end(tmp), inserter(m_common, m_common.begin()));
	  m_size = base_layouter(m_size, bp, m_base_offset, tmp);
	}
      }
    }
    else
      error::not_implemented();
  }

  int nbvf = 0;
  if (bases) {
    nbvf = accumulate(begin(*bases), end(*bases), 0, base_vf);
    nbvf = accumulate(begin(m_common), end(m_common), nbvf, vbase_vf);
  }

  with_initial* vftbl = 0;
  if (nbvf + nvf) {
    const type* T = void_type::create();
    T = pointer_type::create(T);
    T = const_type::create(T);
    T = array_type::create(T, nbvf + nvf);
    vftbl = new with_initial(vftbl_name, T, file_t());
    vftbl->m_flag = vtbl_flag;
    m_tag->m_usrs[vftbl_name].push_back(vftbl);
    if (nvf) {
      T = pointer_type::create(T);
      usr* vfptr = new usr(vfptr_name,T,usr::NONE,file_t(),usr::NONE2);
      m_tag->m_usrs[vfptr_name].push_back(vfptr);
      m_member.push_back(vfptr);
      m_layout[vfptr_name] = make_pair(vfptr_offset, vfptr);
      m_position[vfptr] = vfptr_offset ? 1 : 0;
    }
    int offset = 0;
    if (bases) {
      map<int, var*>& value = vftbl->m_value;
      offset = accumulate(begin(m_common), end(m_common), 0,
			  copy_vbase_vf(value, m_common_vftbl_offset));
      offset = accumulate(begin(*bases), end(*bases), offset,
                          copy_base_vf(value, m_vftbl_offset));
      if (usr* del = delete_entry(m_tag)) 
	for_each(begin(value), end(value), add_override_vdel(del, m_tag));
      for_each(begin(order), end(order), override_vf(value));
      for_each(begin(value), end(value),
	       bind2nd(ptr_fun(check_override), m_tag));
    }
    accumulate(begin(order), end(order), offset, own_vf(vftbl->m_value));
  }

  copy_if(begin(order),end(order),back_inserter(m_member),
          [](usr* u) {
            usr::flag_t flag = u->m_flag;
            usr::flag_t mask =
              usr::flag_t(usr::FUNCTION | usr::STATIC | usr::OVERLOAD);
            return !(flag & mask);
          });
  if (!bases && m_member.empty()) {
    string name = ".dummy";
    const type* T = char_type::create();
    usr* u = new usr(name, T, usr::NONE, file_t(), usr::NONE2);
    m_member.push_back(u);
    m_tag->m_usrs[name].push_back(u);
  }
  if (m_tag->m_kind != tag::UNION) {
    usr* last = m_member.empty() ? 0 : *m_member.rbegin();
    typedef vector<usr*>::iterator IT;
    IT beg = begin(m_member);
    if (beg != end(m_member) && (*beg)->m_name == vbptr_name)
      ++beg;
    if (beg != end(m_member) && (*beg)->m_name == vfptr_name)
      ++beg;
    m_size = accumulate(beg, end(m_member), m_size,
                        layouter(inserter(m_layout,m_layout.begin()),
                                 inserter(m_position,m_position.begin()),
                                 last));
    if (!m_member.empty()) {
      const type* T = m_member[0]->m_type;
      typedef const bit_field_type BF;
      if ( T->m_id == type::BIT_FIELD ){
        BF* bf = static_cast<BF*>(T);
        T = bf->integer_type();
      }
      T = m_member.back()->m_type;
      if ( T->m_id == type::BIT_FIELD ){
        BF* bf = static_cast<BF*>(T);
        T = bf->integer_type();
        m_size += T->size();
        usr::flag_t& flag = m_member.back()->m_flag;
        flag = usr::flag_t(flag | usr::MSB_FIELD);
      }
    }
  }
  else {
    if (!m_member.empty()) {
      transform(begin(m_member), end(m_member),
                inserter(m_layout,m_layout.begin()),
                grounder(inserter(m_position,m_position.begin())));
      {
        vector<usr*>::const_iterator p =
          max_element(begin(m_member),end(m_member),comp_size);
        assert(p != m_member.end());
        const type* T = (*p)->m_type;
        if ( T->m_id == type::BIT_FIELD ){
          typedef const bit_field_type BF;
          BF* bf = static_cast<BF*>(T);
          T = bf->integer_type();
        }
        m_size = T->size();
      }
      {
        vector<usr*>::const_iterator p =
          max_element(begin(m_member),end(m_member),comp_align);
        assert(p != m_member.end());
        const type* T = (*p)->m_type;
        if ( T->m_id == type::BIT_FIELD ){
          typedef const bit_field_type BF;
          BF* bf = static_cast<BF*>(T);
          T = bf->integer_type();
        }
      }
    }
  }
  if (bases) {
    if (m_tag->m_kind != tag::UNION) {
      for_each(begin(*bases), end(*bases), [](base* bp){});
      m_size = accumulate(begin(*bases), end(*bases), m_size,
                          vbase_layouter(m_base_offset, m_common,
                                         m_virt_common_offset));
      m_size = accumulate(begin(m_common), end(m_common), m_size,
                          add_common(m_virt_common_offset));
    }
    else
      error::not_implemented();
    if (vbtbl) {
      map<int, var*>& value = vbtbl->m_value;
      int offset = accumulate(begin(*bases), end(*bases), 0,
                              set_org_vbtbl(value, m_base_offset,
                                            m_virt_common_offset,
                                            m_vbtbl_offset));
      for_each(begin(*bases), end(*bases),
               set_own_vbtbl(offset, value, m_base_offset));
    }
  }
  if (bases) {
    typedef vector<base*>::const_iterator IT;
    IT p =
      find_if(begin(*bases), end(*bases), not1(ptr_fun(base_modifiable)));
    if (p != end(*bases))
      m_modifiable = false;
  }
  if (m_modifiable) {
    typedef vector<usr*>::const_iterator IT;
    IT p = find_if(begin(m_member), end(m_member),
                   not1(ptr_fun(member_modifiable)));
    if (p != end(m_member))
      m_modifiable = false;
  }
  int al = align();
  if ( int n = m_size % al ) {
    m_size += al - n;
  }
  add_ctor(m_tag, m_layout, m_base_offset, m_vbtbl_offset, m_vftbl_offset,
           m_common, m_virt_common_offset, m_common_vftbl_offset,
	   m_member);

  add_dtor(m_tag, m_layout, m_member, m_base_offset, vftbl);
}

int cxx_compiler::record_impl::layouter::operator()(int offset, usr* member)
{
  using namespace std;
  if ( member->m_flag & usr::BIT_FIELD ){
    const type* T = member->m_type;
    assert(T->m_id == type::BIT_FIELD);
    typedef const bit_field_type BF;
    BF* bf = static_cast<BF*>(T);
    T = bf->integer_type();
    bool update = false;
    if ( const type* C = m_current.m_integer ){
      if ( C != T ){
        usr::flag_t& flag = m_current.m_member->m_flag;
        flag = usr::flag_t(flag | usr::MSB_FIELD);
        offset += C->size();
        m_current = current(member,T);
      }
      else if ( m_current.m_position + bf->bit() > T->size() * 8 ){
        offset += C->size();
        usr::flag_t& flag = m_current.m_member->m_flag;
        flag = usr::flag_t(flag | usr::MSB_FIELD);
        m_current = current(member,T);
      }
      else {
        m_current.m_member = member;
        update = true;
      }
    }
    else {
      m_current = current(member,T);
      update = true;
    }
    int align = T->align();
    if ( int n = offset % align )
      offset += align - n;
    string name = member->m_name;
    *X++ = make_pair(name,make_pair(offset,member));
    *Y++ = make_pair(member,m_current.m_position);
    if ( update )
      m_current.m_position += bf->bit();
    return offset;
  }
  else {
    if ( const type* C = m_current.m_integer ){
      usr::flag_t& flag = m_current.m_member->m_flag;
      flag = usr::flag_t(flag | usr::MSB_FIELD);
      offset += C->size();
    }
    m_current = current();
    string name = member->m_name;
    const type* T = member->m_type;
    if (!T) {
      assert(member->m_flag & usr::OVERLOAD);
      return offset;
    }
    if (T->variably_modified()){
      using namespace error::classes;
      not_ordinary(member);
      T = member->m_type = int_type::create();
    }
    if ( !T->size() ){
      int n = code.size();
      typedef const array_type ARRAY;
      ARRAY* array = T->m_id == type::ARRAY ? static_cast<ARRAY*>(T) : 0;
      if ( member == m_last && array && !array->vsize() ){
        T = array->element_type();
        int align = T->align();
        assert(align);
        if ( int n = offset % align )
          offset += align - n;
        *X++ = make_pair(name,make_pair(offset,member));
        return offset;
      }
      for_each(code.begin()+n,code.end(),[](tac* p){ delete p; });
      code.resize(n);
      using namespace error::classes;
      incomplete_member(member);
      T = member->m_type = int_type::create();
    }
    int align = T->align();
    assert(align);
    if ( int n = offset % align )
      offset += align - n;
    *X++ = make_pair(name,make_pair(offset,member));
    return offset + T->size();
  }
}

std::pair<std::string, std::pair<int, cxx_compiler::usr*> >
cxx_compiler::record_impl::grounder::operator()(usr* member)
{
  using namespace std;
  const type* T = member->m_type;
  if ( !T->size() ){
    using namespace error::classes;
    incomplete_member(member);
    T = member->m_type = int_type::create();
  }
  if ( T->m_id == type::BIT_FIELD ){
    typedef const bit_field_type BF;
    BF* bf = static_cast<BF*>(T);
    *Y++ = make_pair(member,0);
  }
  string name = member->m_name;
  return make_pair(name,make_pair(0,member));
}

bool cxx_compiler::record_impl::comp_size(usr* x, usr* y)
{
  typedef const bit_field_type BF;
  const type* xx = x->m_type;
  if ( xx->m_id == type::BIT_FIELD ){
    BF* bf = static_cast<BF*>(xx);
    xx = bf->integer_type();
  }
  const type* yy = y->m_type;
  if ( yy->m_id == type::BIT_FIELD ){
    BF* bf = static_cast<BF*>(yy);
    yy = bf->integer_type();
  }
  return xx->size() < yy->size();
}

bool cxx_compiler::record_impl::comp_align(usr* x, usr* y)
{
  typedef const bit_field_type BF;
  typedef const bit_field_type BF;
  const type* xx = x->m_type;
  if ( xx->m_id == type::BIT_FIELD ){
    BF* bf = static_cast<BF*>(xx);
    xx = bf->integer_type();
  }
  const type* yy = y->m_type;
  if ( yy->m_id == type::BIT_FIELD ){
    BF* bf = static_cast<BF*>(yy);
    yy = bf->integer_type();
  }
  return xx->align() < yy->align();
}

bool cxx_compiler::record_impl::member_modifiable(usr* u)
{
  const type* T = u->m_type;
  return T->modifiable();
}

bool cxx_compiler::record_impl::base_modifiable(base* bp)
{
  tag* ptr = bp->m_tag;
  const type* T = ptr->m_types.second;
  assert(T);
  return T->modifiable();
}

namespace cxx_compiler {
  record_type::table_t record_type::tmp_tbl;
} // end of namespace cxx_compiler

void cxx_compiler::record_type::decl(std::ostream& os, std::string name) const
{
  os << tag::keyword(m_tag->m_kind) << ' ' << m_tag->m_name;
  if ( !name.empty() )
    os << ' ' << name;
}

void cxx_compiler::record_type::encode(std::ostream& os) const
{
  os << 1;
  os << m_tag->m_name;
}

bool cxx_compiler::record_type::compatible(const type* T) const
{
  if (this == T)
    return true;

  if (T->m_id != type::INCOMPLETE_TAGGED)
    return false;
  
  typedef const incomplete_tagged_type ITT;
  ITT* that = static_cast<ITT*>(T);
  return m_tag == that->get_tag();
}

const cxx_compiler::type* cxx_compiler::record_type::composite(const type* T) const
{
  if (this == T)
    return this;
  if (T->m_id != type::INCOMPLETE_TAGGED)
    return 0;
  typedef const incomplete_tagged_type ITT;
  ITT* that = static_cast<ITT*>(T);
  return m_tag == that->get_tag() ? this : 0;
}

std::pair<int, cxx_compiler::usr*> cxx_compiler::
record_type::offset(std::string name) const
{
  using namespace std;
  map<string, pair<int, usr*> >::const_iterator p = m_layout.find(name);
  if (p != m_layout.end())
    return p->second;
  return make_pair(-1, static_cast<usr*>(0));    
}

namespace cxx_compiler {
  namespace record_impl {
    using namespace std;
    inline bool virt(const route_t& r)
    {
      base* bp = r.first;
      if (!bp)
	return false;
      usr::flag_t flag = bp->m_flag;
      return flag & usr::VIRTUAL;
    }
    struct cmp_base {
      tag* m_tag;
      const vector<route_t>& m_route;
      cmp_base(tag* ptr, const vector<route_t>& route)
        : m_tag(ptr), m_route(route) {}
      bool operator()(base* pb)
      {
        if (!m_route.empty())
          return pb == m_route[0].first;
        tag* xtag = pb->m_tag;
        if (xtag == m_tag)
          return true;
        const type* Tx = xtag->m_types.second;
        assert(Tx->m_id == type::RECORD);
        typedef const record_type REC;
        REC* Rx = static_cast<REC*>(Tx);
        const type* Ty = m_tag->m_types.second;
        assert(Ty->m_id == type::RECORD);
        REC* Ry = static_cast<REC*>(Ty);
        vector<route_t> dummy;
	bool ambiguous = false;
        int offset = calc_offset(Rx, Ry, dummy, &ambiguous);
	if (ambiguous)
	  error::not_implemented();
        return offset >= 0;
      }
    };
  }  // end of namespace record_impl
  inline bool canbe_default_ctor(usr* u)
  {
    using namespace record_impl;
    using namespace declarations::declarators::function;
    const type* T = u->m_type;
    if (!T)
      return false;
    const type* dct = default_ctor_type();
    if (compatible(T, dct))
      return true;
    usr::flag_t flag = u->m_flag;
    if (flag & usr::HAS_DEFAULT_ARG) {
      typedef map<usr*, vector<var*> >::const_iterator ITx;
      ITx p = default_arg_table.find(u);
      assert(p != default_arg_table.end());
      const vector<var*>& v = p->second;
      assert(!v.empty());
      typedef vector<var*>::const_iterator ITy;
      ITy q = find(begin(v), end(v), (var*)0);
      return q == end(v);
    }
    return false;
  }
  bool canbe_copy_ctor(usr* u, tag* ptr)
  {
    using namespace record_impl;
    using namespace declarations::declarators::function;
    const type* T = u->m_type;
    if (!T)
      return false;
    const type* cct = copy_ctor_type(ptr, true);
    if (compatible(T, cct))
      return true;
    cct = copy_ctor_type(ptr, false);
    if (compatible(T, cct))
      return true;
    usr::flag_t flag = u->m_flag;
    if (flag & usr::HAS_DEFAULT_ARG) {
      typedef map<usr*, vector<var*> >::const_iterator ITx;
      ITx p = default_arg_table.find(u);
      assert(p != default_arg_table.end());
      const vector<var*>& v = p->second;
      assert(!v.empty());
      typedef vector<var*>::const_iterator ITy;
      ITy beg = begin(v) + 1;
      ITy q = find(beg, end(v), (var*)0);
      if (q != end(v))
	return false;
      assert(T->m_id == type::FUNC);
      typedef const func_type FT;
      FT* ft = static_cast<FT*>(T);
      const vector<const type*>& param = ft->param();
      assert(!param.empty());
      T = param[0];
      if (T->m_id != type::REFERENCE)
	return false;
      typedef const reference_type RT;
      RT* rt = static_cast<RT*>(T);
      T = rt->referenced_type();
      return T->get_tag() == ptr;
    }
    return false;
  }
  namespace record_impl {
    inline void add_vdel_code(usr* vdtor, usr* del, usr* this_ptr)
    {
      call_impl::wrapper(vdtor, 0, this_ptr);
      vector<var*> arg;
      arg.push_back(this_ptr);
      call_impl::wrapper(del, &arg, 0);
    }
    inline void handle_vdel1(usr* u, tag* ptr)
    {
      if (!(u->m_flag & usr::VDEL))
	return;
      virtual_delete* vdel = static_cast<virtual_delete*>(u);
      assert(code.empty());
      scope* param = new scope(scope::PARAM);
      using namespace class_or_namespace_name;
      assert(!before.empty());
      assert(before.back() == param);
      before.pop_back();
      param->m_parent = scope::current;
      scope::current->m_children.push_back(param);
      const type* T = ptr->m_types.second;
      T = pointer_type::create(T);
      string name = "this";
      usr* this_ptr = new usr(name, T, usr::NONE, file_t(), usr::NONE2);
      param->m_usrs[name].push_back(this_ptr);
      param->m_order.push_back(this_ptr);

      block* bp = new block;
      assert(!before.empty());
      assert(before.back() == bp);
      before.pop_back();
      bp->m_parent = param;
      param->m_children.push_back(bp);
      scope* org = scope::current;
      scope::current = bp;
      add_vdel_code(vdel->m_dtor, vdel->m_del, this_ptr);
      scope::current = org;

      using namespace declarations::declarators;
      fundef* fdef = new fundef(vdel, param);
      function::definition::action(fdef, code);
    }
  } // end of namespace record_impl
}  // end of namespace cxx_compiler

int
cxx_compiler::calc_offset(const record_type* xrec,
                          const record_type* yrec,
                          const std::vector<route_t>& route, bool* ambiguous)
{
  using namespace std;
  using namespace record_impl;
  typedef const record_type REC;

  if (route.empty()) {
    typedef map<const record_type*, int>::const_iterator ITx;
    const map<const record_type*, int>& vco = xrec->virt_common_offset();
    ITx px = vco.find(yrec);
    if (px != vco.end())
      return px->second;
  }
  else {
    if (REC* rec = route[0].second) {
      typedef map<const record_type*, int>::const_iterator ITx;
      const map<const record_type*, int>& vco = xrec->virt_common_offset();
      ITx px = vco.find(rec);
      assert(px != vco.end());
      int n = px->second;
      vector<route_t> route2;
      copy(begin(route)+1, end(route), back_inserter(route2));
      return n + calc_offset(rec, yrec, route2, ambiguous);
    }
  }

  typedef vector<route_t>::const_iterator IT;
  IT p = begin(route);
  while (p != end(route)) {
    p = find_if(p, end(route), virt);
    if (p != end(route)) {
      base* bp = p->first;
      tag* ptr = bp->m_tag;
      const type* T = ptr->m_types.second;
      assert(T->m_id == type::RECORD);
      REC* rec = static_cast<REC*>(T);
      vector<route_t> dummy;
      bool ambiguous = false;
      int n = calc_offset(xrec, rec, dummy, &ambiguous);
      if (ambiguous)
	error::not_implemented();
      if (n >= 0) {
	int m = calc_offset(rec, yrec, dummy, &ambiguous);
	if (ambiguous)
	  error::not_implemented();
	return n + m;
      }
      ++p;
    }
  }

  if (xrec == yrec)
    return 0;

  tag* xtag = xrec->get_tag();
  tag* ytag = yrec->get_tag();
  if (!xtag->m_bases)
    return -1;
  const vector<base*>& bases = *xtag->m_bases;
  typedef vector<base*>::const_iterator ITy;
  ITy py = find_if(begin(bases), end(bases), cmp_base(ytag, route));
  if (py == end(bases))
    return -1;
  ITy qy = find_if(py+1, end(bases), cmp_base(ytag, route));
  if (qy != end(bases) && ambiguous)
    *ambiguous = true;
  base* bp = *py;
  const map<base*, int>& base_offset = xrec->base_offset();
  map<base*, int>::const_iterator q = base_offset.find(bp);
  assert(q != base_offset.end());
  int n = q->second;
  assert(n >= 0);
  tag* btag = bp->m_tag;
  const type* Tb = btag->m_types.second;
  assert(Tb->m_id == type::RECORD);
  REC* Rb = static_cast<REC*>(Tb);
  vector<route_t> route2;
  if (!route.empty())
    copy(begin(route)+1, end(route), back_inserter(route2));
  int m = calc_offset(Rb, yrec, route2, ambiguous);
  assert(m >= 0);
  return n + m;
}

int cxx_compiler::record_type::position(usr* member) const
{
  using namespace std;
  map<usr*, int>::const_iterator p = m_position.find(member);
  assert(p != m_position.end());
  return p->second;
}

std::pair<int, const cxx_compiler::type*>
cxx_compiler::record_type::current(int nth) const
{
  using namespace std;
  if (m_tag->m_kind == tag::UNION && nth >= 1)
    return make_pair(-1,static_cast<const type*>(0));
  if (m_member.size() <= nth)
    return make_pair(-1,static_cast<const type*>(0));
  usr* u = m_member[nth];
  const type* T = u->m_type;
  string name = u->m_name;
  map<string, pair<int, usr*> >::const_iterator p = m_layout.find(name);
  assert(p != m_layout.end());
  int offset = p->second.first;
  return make_pair(offset,T);
}

bool cxx_compiler::record_type::tmp() const
{
  return tmp_tbl.find(this) != tmp_tbl.end();
}

const cxx_compiler::record_type*
cxx_compiler::record_type::create(tag* ptr)
{
  record_type* ret = new record_type(ptr);
  if (temporary(ptr))
    tmp_tbl.insert(ret);
  return ret;
}

void cxx_compiler::handle_copy_ctor(tag* ptr)
{
  using namespace std;
  using namespace record_impl;
  string name = ptr->m_name;
  const map<string, vector<usr*> >& usrs = ptr->m_usrs;
  map<string, vector<usr*> >::const_iterator p = usrs.find(name);
  if (p == usrs.end())
    return;
  const vector<usr*>& v = p->second;
  typedef vector<usr*>::const_iterator IT;
  IT q = find_if(begin(v), end(v), canbe_default_ctor);
  if (q == end(v))
    return;
  IT r = find_if(begin(v), end(v), bind2nd(ptr_fun(canbe_copy_ctor), ptr));
  if (r != end(v))
    return;
  add_copy_ctor(ptr);
}

void cxx_compiler::handle_vdel(tag* ptr)
{
  using namespace record_impl;
  const vector<usr*>& order = ptr->m_order;
  for_each(begin(order), end(order), bind2nd(ptr_fun(handle_vdel1), ptr));
}

cxx_compiler::usr* cxx_compiler::has_ctor_dtor(tag* ptr, bool is_dtor)
{
  string tgn = ptr->m_name;
  if (is_dtor)
    tgn = '~' + tgn;
  const map<string, vector<usr*> >& usrs = ptr->m_usrs;
  typedef map<string, vector<usr*> >::const_iterator IT;
  IT p = usrs.find(tgn);
  if (p == usrs.end())
    return 0;
  const vector<usr*>& v = p->second;
  return v.back();
}

void cxx_compiler::record_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p;
  tmp_tbl.clear();
}

void cxx_compiler::record_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : tmp_tbl)
    vt.push_back(p);
  tmp_tbl.clear();
}

void cxx_compiler::
record_type::tor_code(usr* tor, scope* param, usr* this_ptr, block* pb,
		      bool is_ctor) const
{
  using namespace record_impl;
  if (is_ctor)
    add_ctor_code(m_tag, m_layout, m_base_offset, m_vbtbl_offset,
		  m_vftbl_offset, m_common, m_virt_common_offset,
		  m_common_vftbl_offset, m_member, pb, this_ptr, tor, param);
  else
    add_dtor_code(m_tag, m_layout, m_member, m_base_offset, pb, this_ptr);
}

namespace cxx_compiler {
  namespace check_abstract_impl {
    string common(const type* T, vector<usr*>& vf)
    {
      if (!T)
	return "";
      T = T->complete_type();
      T = T->unqualified();
      if (T->m_id != type::RECORD)
	return "";
      typedef const record_type REC;
      REC* rec = static_cast<REC*>(T);
      tag* ptr = rec->get_tag();
      const map<string, vector<usr*> >& usrs = ptr->m_usrs;
      typedef map<string, vector<usr*> >::const_iterator IT;
      IT p = usrs.find(vftbl_name);
      if (p == usrs.end())
	return "";
      const vector<usr*>& v = p->second;
      assert(v.size() == 1);
      usr* u = v.back();
      usr::flag_t flag = u->m_flag;
      assert(flag & usr::WITH_INI);
      with_initial* wi = static_cast<with_initial*>(u);
      map<int, var*>& value = wi->m_value;
      for_each(begin(value), end(value), [&vf](const pair<int, var*>& p)
	       {
		 if (pure_virt_value* pv =
		     dynamic_cast<pure_virt_value*>(p.second))
		   vf.push_back(pv->m_usr);
	       });
      return ptr->m_name;
    }
    void param(int i, const type* T, usr* func)
    {
      vector<usr*> vf;
      string class_name = check_abstract_impl::common(T, vf);
      if (!vf.empty())
	error::classes::abstract_param(class_name, func, vf, i+1);
    }
  } // end of namespace check_abstract_impl
} // end of namespace cxx_compiler

void cxx_compiler::check_abstract_obj(usr* obj)
{
  const type* T = obj->m_type;
  vector<usr*> vf;
  string class_name = check_abstract_impl::common(T, vf);
  if (!vf.empty())
    error::classes::abstract_object(class_name, obj, vf);
}

void cxx_compiler::check_abstract_func(usr* func)
{
  const type* T = func->m_type;
  if (T->m_id != type::FUNC)
    return;
  typedef const func_type FT;
  FT* ft = static_cast<FT*>(T);
  T = ft->return_type();
  vector<usr*> vf;
  string class_name = check_abstract_impl::common(T, vf);
  if (!vf.empty())
    error::classes::abstract_return(class_name, func, vf);

  const vector<const type*>& param = ft->param();
  for (int i = 0 ; i != param.size() ; ++i)
    check_abstract_impl::param(i, param[i], func);
}

void cxx_compiler::call_default_ctor(var* v)
{
  const type* T = v->result_type();
  if (T->m_id == type::ARRAY) {
    typedef const array_type AT;
    AT* at = static_cast<AT*>(T);
    if (array_of_tor(at, true))
      ctor_dtor_common(v, at, call_default_ctor, true);
    return;
  }
  T = T->unqualified();
  if (T->m_id != type::RECORD )
    return;
  typedef const record_type REC;
  REC* rec = static_cast<REC*>(T);
  tag* ptr = rec->get_tag();
  string name = ptr->m_name;
  const map<string, vector<usr*> >& usrs = ptr->m_usrs;
  map<string, vector<usr*> >::const_iterator p = usrs.find(name);
  if (p == usrs.end())
    return;
  const vector<usr*>& ctors = p->second;
  typedef vector<usr*>::const_iterator IT;
  IT q = find_if(begin(ctors), end(ctors), canbe_default_ctor);
  if (q == end(ctors))
    return;
  usr* ctor = *q;
  call_impl::wrapper(ctor, 0, v);
}

namespace cxx_compiler {
  bool must_call_ctor_dtor_common(usr* u, bool ctor)
  {
    const type* T = u->m_type;
    if (T->m_id == type::ARRAY) {
      typedef const array_type AT;
      AT* at = static_cast<AT*>(T);
      return array_of_tor(at, ctor);
    }

    T = T->unqualified();
    if (T->m_id != type::RECORD )
      return false;
    typedef const record_type REC;
    REC* rec = static_cast<REC*>(T);
    tag* ptr = rec->get_tag();
    string name = ptr->m_name;
    if (!ctor)
      name = '~' + name;
    const map<string, vector<usr*> >& usrs = ptr->m_usrs;
    map<string, vector<usr*> >::const_iterator p = usrs.find(name);
    if (p == usrs.end())
      return false;
    const vector<usr*>& tors = p->second;
    if (ctor) {
      typedef vector<usr*>::const_iterator IT;
      IT q = find_if(begin(tors), end(tors), canbe_default_ctor);
      return q != end(tors);
    }
    assert(tors.size() == 1);
    return true;
  }
} // end of namesapce cxx_compiler

bool cxx_compiler::must_call_default_ctor(usr* u)
{
  return must_call_ctor_dtor_common(u, true);
}

bool cxx_compiler::must_call_dtor(usr* u)
{
  return must_call_ctor_dtor_common(u, false);
}
