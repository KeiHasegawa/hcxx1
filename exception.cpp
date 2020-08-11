#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

namespace cxx_compiler {
  namespace rethrow {
    to3ac* to;
  } // end of namespace rethrow
  namespace expressions {
    namespace throw_impl {
      info_t::info_t(base* expr) : m_expr(expr), m_file(parse::position) {}
      const file_t& info_t::file() const
      {
	return m_expr ? m_expr->file() : m_file;
      }
      var* info_t::gen()
      {
	if (!m_expr) {
	  code.push_back(new rethrow3ac);
	  goto3ac* go = new goto3ac;
	  code.push_back(go);
	  assert(rethrow::to);
	  go->m_to = rethrow::to;
	  rethrow::to->m_goto.push_back(go);
	  var* ret = new var(void_type::create());
	  garbage.push_back(ret);
	  return ret;
	}
	var* expr = m_expr->gen();
	expr = expr->rvalue();
	const type* T = expr->m_type;
	T = T->complete_type();
	int size = T->size();
	if (!size)
	  error::not_implemented();
	using namespace primary::literal;
	var* sz = integer::create(size);
	const pointer_type* pt = pointer_type::create(T);
	var* ptr = new var(pt);
	if (scope::current->m_id == scope::BLOCK) {
	  block* b = static_cast<block*>(scope::current);
	  b->m_vars.push_back(ptr);
	}
	else
	  garbage.push_back(ptr);
	code.push_back(new alloce3ac(ptr, sz));
	code.push_back(new invladdr3ac(ptr, expr));
	if (T->m_id == type::POINTER) {
	  typedef const pointer_type PT;
	  PT* pt = static_cast<PT*>(T);
	  const type* RT = pt->referenced_type();
	  T = const_type::create(RT);
	  T = pointer_type::create(T);
	}
	code.push_back(new throw3ac(ptr, T));
	var* ret = new var(void_type::create());
	garbage.push_back(ret);
	return ret;
      }
    }  // end of namespace thorw_impl
  }  // end of namespace expressions
} // end of namespace cxx_compiler

namespace cxx_compiler {
  namespace exception {
    namespace declaration {
      var* action(declarations::type_specifier_seq::info_t* p, var* v)
      {
	typedef declarations::type_specifier_seq::info_t X;
	auto_ptr<X> sweeper(p);
	p->update();
	const type* T = p->m_type;
	using namespace declarations;
	type_specifier* ts = new type_specifier(T);
	specifier* sp = new specifier(ts);
	typedef specifier_seq::info_t Y;
	Y* q = new specifier_seq::info_t(0, sp);
	auto_ptr<Y> sweeper2(q);
	assert(v->usr_cast());
	usr* u = static_cast<usr*>(v);
	u->m_flag2 = usr::flag2_t(u->m_flag2 | usr::DECLED_HANDLER);
	return declarations::action1(v, false);
      }
    } // end of namespace declaration
    namespace try_block {
      typedef pair<var*, statements::base*> HANDLER;
      typedef list<HANDLER*> HANDLERS;
      struct sweeper {
	HANDLERS* m_handlers;
	sweeper(HANDLERS* handlers) : m_handlers(handlers) {}
	~sweeper()
	{
	  for (auto p : *m_handlers) {
	    delete p->second;
	    delete p;
	  }
	  delete m_handlers;
	}
      };
      inline void gen_catch(HANDLER* p, var* info, to3ac* to)
      {
	var* v = p->first;
	code.push_back(new catch_begin3ac(v, info));
	statements::base* stmt = p->second;
	stmt->gen();
	code.push_back(new catch_end3ac);
	goto3ac* go = new goto3ac;
	go->m_to = to;
	to->m_goto.push_back(go);
	code.push_back(go);
      }
      void action(statements::base* stmt, HANDLERS* handlers)
      {
	sweeper sweeper(handlers);
	code.push_back(new try_begin3ac);
	stmt->gen();
	delete stmt;
	code.push_back(new try_end3ac);
	to3ac* to = new to3ac;
	goto3ac* go = new goto3ac;
	go->m_to = to;
	to->m_goto.push_back(go);
	code.push_back(go);
	here3ac* here = new here3ac;
	code.push_back(here);

	var* t0 = 0;
	typedef HANDLERS::const_iterator IT;
	HANDLERS& v = *handlers;
	IT it = find_if(begin(v), end(v), [](HANDLER* h){ return h->first;});
	if (it != end(v)) {
	  t0 = new var(int_type::create());
	  if (scope::current->m_id == scope::BLOCK) {
	    block* b = static_cast<block*>(scope::current);
	    b->m_vars.push_back(t0);
	  }
	  else
	    garbage.push_back(t0);
	  here_reason3ac* here_reason = new here_reason3ac(t0);
	  code.push_back(here_reason);
	}
	const type* vp = pointer_type::create(void_type::create());
	var* t1 = new var(vp);
	if (scope::current->m_id == scope::BLOCK) {
	  block* b = static_cast<block*>(scope::current);
	  b->m_vars.push_back(t1);
	}
	else
	  garbage.push_back(t1);
	here_info3ac* here_info = new here_info3ac(t1);
	code.push_back(here_info);
	if (it != end(v)) {
	  using namespace expressions::primary::literal;
	  var* one = integer::create(1);
	  goto3ac* go2 = new goto3ac(goto3ac::EQ, t0, one);
	  to3ac* to2 = new to3ac;
	  go2->m_to = to2;
	  to2->m_goto.push_back(go2);
	  code.push_back(go2);
	  code.push_back(new unwind_resume3ac(t1));
	  code.push_back(to2);
	}
	assert(!rethrow::to);
	rethrow::to = new to3ac;
	for (auto p : v)
	  gen_catch(p, t1, to);
	if (!rethrow::to->m_goto.empty()) {
	  code.push_back(rethrow::to);
	  there3ac* there = new there3ac;
	  code.push_back(there);
	  var* t2 = new var(vp);
	  if (scope::current->m_id == scope::BLOCK) {
	    block* b = static_cast<block*>(scope::current);
	    b->m_vars.push_back(t2);
	  }
	  else
	    garbage.push_back(t2);
	  here_info3ac* here_info2 = new here_info3ac(t2);
	  code.push_back(here_info2);
	  code.push_back(new catch_end3ac);
	  code.push_back(new unwind_resume3ac(t2));
	}
	else
	  delete rethrow::to;
	rethrow::to = 0;
	code.push_back(to);
      }
    } // end of nmaepsace try_block
  } // end of namespace exception
} // end of namespace cxx_compiler

