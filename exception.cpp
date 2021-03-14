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
          if (rethrow::to) {
            goto3ac* go = new goto3ac;
            code.push_back(go);
            go->m_to = rethrow::to;
            rethrow::to->m_goto.push_back(go);
          }
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
        if (v) {
          assert(v->usr_cast());
          usr* u = static_cast<usr*>(v);
          u->m_flag2 = usr::flag2_t(u->m_flag2 | usr::DECLED_HANDLER);
          return declarations::action1(v, false);
        }
        v = new var(T);
        assert(scope::current->m_id == scope::BLOCK);
        block* b = static_cast<block*>(scope::current);
        b->m_vars.push_back(v);
        if (must_call_dtor(T))
          block_impl::dtor_tbl[b].push_back(v);
        return v;
      }
    } // end of namespace declaration
    namespace try_block {
      typedef statements::try_block::HANDLER HANDLER;
      struct gen_goto {
        var* m_reason;
        int m_cnt;
        gen_goto(var* v) : m_reason(v), m_cnt(0) {}
        to3ac* operator()(HANDLER* h)
        {
          ++m_cnt;
          if (!h->first)
            return 0;
          using namespace expressions::primary::literal;
          var* value = integer::create(m_cnt);
          goto3ac* go = new goto3ac(goto3ac::EQ, m_reason, value);
          to3ac* to = new to3ac;
          go->m_to = to;
          to->m_goto.push_back(go);
          code.push_back(go);
          return to;
        }
      };
      struct gen_catch {
        var* m_info;
        to3ac* m_finish;
        gen_catch(var* info, to3ac* finish)
          : m_info(info), m_finish(finish) {}
        void subr(HANDLER* h, to3ac* start)
        {
          if (start)
            code.push_back(start);
          var* x = h->first;
          code.push_back(new catch_begin3ac(x, m_info));
          statements::base* stmt = h->second;
          stmt->gen();
          if (x) {
            scope* ps = x->m_scope;
            assert(ps->m_id == scope::BLOCK);
            block* b = static_cast<block*>(ps);
            typedef map<block*, vector<var*> >::iterator IT;
            IT p = block_impl::dtor_tbl.find(b);
            if (p != block_impl::dtor_tbl.end()) {
              const vector<var*>& v = p->second;
              assert(v.size() == 1);
              assert(v[0] == x);
              call_dtor(x);
              block_impl::dtor_tbl.erase(p);
            }
          }
          code.push_back(new catch_end3ac);
          goto3ac* go = new goto3ac;
          go->m_to = m_finish;
          m_finish->m_goto.push_back(go);
          code.push_back(go);
        }
        bool operator()(HANDLER* p, to3ac* start)
        {
          if (start)
            subr(p, start);
          return true;
        }
      };
      typedef statements::try_block::HANDLERS HANDLERS;
      void action(statements::base* stmt, HANDLERS* handlers)
      {
        code.push_back(new try_begin3ac);
        stmt->gen();
        code.push_back(new try_end3ac);
        goto3ac* go = new goto3ac;
        to3ac* finish = new to3ac;
        go->m_to = finish;
        finish->m_goto.push_back(go);
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
        vector<to3ac*> tos;
        transform(begin(v), end(v), back_inserter(tos), gen_goto(t0));
        assert(!rethrow::to);
        rethrow::to = new to3ac;
        IT it2 = find_if(begin(v), end(v), [](HANDLER* h){ return !h->first;});
        if (it2 != end(v)) {
          gen_catch tmp(t1, finish);
          HANDLER* h = *it2;
          tmp.subr(h, 0);
        }
        else
          code.push_back(new unwind_resume3ac(t1));
        mismatch(begin(v), end(v), begin(tos), gen_catch(t1, finish));
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
        code.push_back(finish);
      }
    } // end of nmaepsace try_block
  } // end of namespace exception
} // end of namespace cxx_compiler
