#ifndef _CXX_CORE_H_
#define _CXX_CORE_H_

namespace cxx_compiler {

struct usr;
struct tag;

struct scope {
  enum id_t { NONE, PARAM, BLOCK, TAG, NAMESPACE };
  id_t m_id;
  scope* m_parent;
  std::vector<scope*> m_children;
  std::map<std::string, std::vector<usr*> > m_usrs;
  std::map<std::string, tag*> m_tags;
  static scope* current;
  static scope root;
  std::vector<usr*> m_order;
  scope(id_t id = NONE) : m_id(id), m_parent(0) {}
  virtual ~scope();
};

struct var;

struct tac;

struct block : scope {
  std::vector<var*> m_vars;
  block() : scope(BLOCK) {}
  ~block();
};

struct file_t {
  std::string m_name;
  int m_lineno;
  file_t(std::string name = "", int lineno = 0) : m_name(name), m_lineno(lineno) {}
};

struct type;

struct base {
  int m_access;
  bool m_virtual;
  tag* m_tag;
  base(int a, bool v, tag* t) : m_access(a), m_virtual(v), m_tag(t) {}
};

struct tag : scope {
  enum kind_t { STRUCT, UNION, CLASS, ENUM };
  kind_t m_kind;
  std::string m_name;
  std::vector<file_t> m_file;
  std::pair<const type*, const type*> m_types;
  static std::string keyword(kind_t);
  std::vector<base*>* m_bases;
  tag(kind_t kind, std::string name, const file_t& file, std::vector<base*>* b)
    : scope(TAG), m_kind(kind), m_name(name), m_bases(b)
  {
    m_file.push_back(file);
  }
  ~tag();
};


template<class T> struct constant;
struct addrof;
struct generated;
struct genaddr;
class func_type;
struct to3ac;

namespace statements {
  struct base;
  namespace if_stmt { struct info_t; }
  namespace while_stmt { struct info_t; }
  namespace for_stmt { struct info_t; }
  namespace do_stmt { struct info_t; }
} // end of namespace statements

struct var {
  const type* m_type;
  scope* m_scope;
  var(const type* T)
    : m_type(T), m_scope(scope::current) {}
  virtual var* rvalue(){ return this; }
  virtual bool lvalue() const { return false; }
  virtual bool zero() const { return false; }
  virtual generated* generated_cast(){ return 0; }
  virtual usr* usr_cast(){ return 0; }
  virtual addrof* addrof_cast(){ return 0; }
  virtual genaddr* genaddr_cast(){ return 0; }
  var* promotion();

  virtual var* subscripting(var*);
  virtual var* call(std::vector<var*>*);
  var* member(var*, bool);
  virtual var* ppmm(bool, bool);
  virtual var* address();
  virtual var* indirection();
  virtual var* _not();
  virtual var* plus();
  virtual var* minus();
  virtual var* tilde();
  virtual var* size();

  virtual var* cast(const type*);

  virtual var* mul(var*);
  virtual var* mulr(constant<bool>*);
  virtual var* mulr(constant<char>*);
  virtual var* mulr(constant<signed char>*);
  virtual var* mulr(constant<unsigned char>*);
  virtual var* mulr(constant<wchar_t>*);
  virtual var* mulr(constant<short int>*);
  virtual var* mulr(constant<unsigned short int>*);
  virtual var* mulr(constant<int>*);
  virtual var* mulr(constant<unsigned int>*);
  virtual var* mulr(constant<long int>*);
  virtual var* mulr(constant<unsigned long int>*);
  virtual var* mulr(constant<__int64>*);
  virtual var* mulr(constant<unsigned __int64>*);
  virtual var* mulr(constant<float>*);
  virtual var* mulr(constant<double>*);
  virtual var* mulr(constant<long double>*);
  virtual var* div(var*);
  virtual var* divr(constant<bool>*);
  virtual var* divr(constant<char>*);
  virtual var* divr(constant<signed char>*);
  virtual var* divr(constant<unsigned char>*);
  virtual var* divr(constant<wchar_t>*);
  virtual var* divr(constant<short int>*);
  virtual var* divr(constant<unsigned short int>*);
  virtual var* divr(constant<int>*);
  virtual var* divr(constant<unsigned int>*);
  virtual var* divr(constant<long int>*);
  virtual var* divr(constant<unsigned long int>*);
  virtual var* divr(constant<__int64>*);
  virtual var* divr(constant<unsigned __int64>*);
  virtual var* divr(constant<float>*);
  virtual var* divr(constant<double>*);
  virtual var* divr(constant<long double>*);
  virtual var* mod(var*);
  virtual var* modr(constant<bool>*);
  virtual var* modr(constant<char>*);
  virtual var* modr(constant<signed char>*);
  virtual var* modr(constant<unsigned char>*);
  virtual var* modr(constant<wchar_t>*);
  virtual var* modr(constant<short int>*);
  virtual var* modr(constant<unsigned short int>*);
  virtual var* modr(constant<int>*);
  virtual var* modr(constant<unsigned int>*);
  virtual var* modr(constant<long int>*);
  virtual var* modr(constant<unsigned long int>*);
  virtual var* modr(constant<__int64>*);
  virtual var* modr(constant<unsigned __int64>*);
  virtual var* add(var*);
  virtual var* addr(constant<bool>*);
  virtual var* addr(constant<char>*);
  virtual var* addr(constant<signed char>*);
  virtual var* addr(constant<unsigned char>*);
  virtual var* addr(constant<wchar_t>*);
  virtual var* addr(constant<short int>*);
  virtual var* addr(constant<unsigned short int>*);
  virtual var* addr(constant<int>*);
  virtual var* addr(constant<unsigned int>*);
  virtual var* addr(constant<long int>*);
  virtual var* addr(constant<unsigned long int>*);
  virtual var* addr(constant<__int64>*);
  virtual var* addr(constant<unsigned __int64>*);
  virtual var* addr(constant<void*>*);
  virtual var* addr(constant<float>*);
  virtual var* addr(constant<double>*);
  virtual var* addr(constant<long double>*);
  virtual var* addr(addrof*);
  virtual var* sub(var*);
  virtual var* subr(constant<bool>*);
  virtual var* subr(constant<char>*);
  virtual var* subr(constant<signed char>*);
  virtual var* subr(constant<unsigned char>*);
  virtual var* subr(constant<wchar_t>*);
  virtual var* subr(constant<short int>*);
  virtual var* subr(constant<unsigned short int>*);
  virtual var* subr(constant<int>*);
  virtual var* subr(constant<unsigned int>*);
  virtual var* subr(constant<long int>*);
  virtual var* subr(constant<unsigned long int>*);
  virtual var* subr(constant<__int64>*);
  virtual var* subr(constant<unsigned __int64>*);
  virtual var* subr(constant<void*>*);
  virtual var* subr(constant<float>*);
  virtual var* subr(constant<double>*);
  virtual var* subr(constant<long double>*);
  virtual var* subr(addrof*);
  virtual var* lsh(var*);
  virtual var* lshr(constant<bool>*);
  virtual var* lshr(constant<char>*);
  virtual var* lshr(constant<signed char>*);
  virtual var* lshr(constant<unsigned char>*);
  virtual var* lshr(constant<wchar_t>*);
  virtual var* lshr(constant<short int>*);
  virtual var* lshr(constant<unsigned short int>*);
  virtual var* lshr(constant<int>*);
  virtual var* lshr(constant<unsigned int>*);
  virtual var* lshr(constant<long int>*);
  virtual var* lshr(constant<unsigned long int>*);
  virtual var* lshr(constant<__int64>*);
  virtual var* lshr(constant<unsigned __int64>*);
  virtual var* rsh(var*);
  virtual var* rshr(constant<bool>*);
  virtual var* rshr(constant<char>*);
  virtual var* rshr(constant<signed char>*);
  virtual var* rshr(constant<unsigned char>*);
  virtual var* rshr(constant<wchar_t>*);
  virtual var* rshr(constant<short int>*);
  virtual var* rshr(constant<unsigned short int>*);
  virtual var* rshr(constant<int>*);
  virtual var* rshr(constant<unsigned int>*);
  virtual var* rshr(constant<long int>*);
  virtual var* rshr(constant<unsigned long int>*);
  virtual var* rshr(constant<__int64>*);
  virtual var* rshr(constant<unsigned __int64>*);
  virtual var* lt(var*);
  virtual var* ltr(constant<bool>*);
  virtual var* ltr(constant<char>*);
  virtual var* ltr(constant<signed char>*);
  virtual var* ltr(constant<unsigned char>*);
  virtual var* ltr(constant<wchar_t>*);
  virtual var* ltr(constant<short int>*);
  virtual var* ltr(constant<unsigned short int>*);
  virtual var* ltr(constant<int>*);
  virtual var* ltr(constant<unsigned int>*);
  virtual var* ltr(constant<long int>*);
  virtual var* ltr(constant<unsigned long int>*);
  virtual var* ltr(constant<__int64>*);
  virtual var* ltr(constant<unsigned __int64>*);
  virtual var* ltr(constant<float>*);
  virtual var* ltr(constant<double>*);
  virtual var* ltr(constant<long double>*);
  virtual var* ltr(constant<void*>*);
  virtual var* ltr(addrof*);
  virtual var* gt(var*);
  virtual var* gtr(constant<bool>*);
  virtual var* gtr(constant<char>*);
  virtual var* gtr(constant<signed char>*);
  virtual var* gtr(constant<unsigned char>*);
  virtual var* gtr(constant<wchar_t>*);
  virtual var* gtr(constant<short int>*);
  virtual var* gtr(constant<unsigned short int>*);
  virtual var* gtr(constant<int>*);
  virtual var* gtr(constant<unsigned int>*);
  virtual var* gtr(constant<long int>*);
  virtual var* gtr(constant<unsigned long int>*);
  virtual var* gtr(constant<__int64>*);
  virtual var* gtr(constant<unsigned __int64>*);
  virtual var* gtr(constant<float>*);
  virtual var* gtr(constant<double>*);
  virtual var* gtr(constant<long double>*);
  virtual var* gtr(constant<void*>*);
  virtual var* gtr(addrof*);
  virtual var* le(var*);
  virtual var* ler(constant<bool>*);
  virtual var* ler(constant<char>*);
  virtual var* ler(constant<signed char>*);
  virtual var* ler(constant<unsigned char>*);
  virtual var* ler(constant<wchar_t>*);
  virtual var* ler(constant<short int>*);
  virtual var* ler(constant<unsigned short int>*);
  virtual var* ler(constant<int>*);
  virtual var* ler(constant<unsigned int>*);
  virtual var* ler(constant<long int>*);
  virtual var* ler(constant<unsigned long int>*);
  virtual var* ler(constant<__int64>*);
  virtual var* ler(constant<unsigned __int64>*);
  virtual var* ler(constant<float>*);
  virtual var* ler(constant<double>*);
  virtual var* ler(constant<long double>*);
  virtual var* ler(constant<void*>*);
  virtual var* ler(addrof*);
  virtual var* ge(var*);
  virtual var* ger(constant<bool>*);
  virtual var* ger(constant<char>*);
  virtual var* ger(constant<signed char>*);
  virtual var* ger(constant<unsigned char>*);
  virtual var* ger(constant<wchar_t>*);
  virtual var* ger(constant<short int>*);
  virtual var* ger(constant<unsigned short int>*);
  virtual var* ger(constant<int>*);
  virtual var* ger(constant<unsigned int>*);
  virtual var* ger(constant<long int>*);
  virtual var* ger(constant<unsigned long int>*);
  virtual var* ger(constant<__int64>*);
  virtual var* ger(constant<unsigned __int64>*);
  virtual var* ger(constant<float>*);
  virtual var* ger(constant<double>*);
  virtual var* ger(constant<long double>*);
  virtual var* ger(constant<void*>*);
  virtual var* ger(addrof*);
  virtual var* eq(var*);
  virtual var* eqr(constant<bool>*);
  virtual var* eqr(constant<char>*);
  virtual var* eqr(constant<signed char>*);
  virtual var* eqr(constant<unsigned char>*);
  virtual var* eqr(constant<wchar_t>*);
  virtual var* eqr(constant<short int>*);
  virtual var* eqr(constant<unsigned short int>*);
  virtual var* eqr(constant<int>*);
  virtual var* eqr(constant<unsigned int>*);
  virtual var* eqr(constant<long int>*);
  virtual var* eqr(constant<unsigned long int>*);
  virtual var* eqr(constant<__int64>*);
  virtual var* eqr(constant<unsigned __int64>*);
  virtual var* eqr(constant<float>*);
  virtual var* eqr(constant<double>*);
  virtual var* eqr(constant<long double>*);
  virtual var* eqr(constant<void*>*);
  virtual var* eqr(addrof*);
  virtual var* ne(var*);
  virtual var* ner(constant<bool>*);
  virtual var* ner(constant<char>*);
  virtual var* ner(constant<signed char>*);
  virtual var* ner(constant<unsigned char>*);
  virtual var* ner(constant<wchar_t>*);
  virtual var* ner(constant<short int>*);
  virtual var* ner(constant<unsigned short int>*);
  virtual var* ner(constant<int>*);
  virtual var* ner(constant<unsigned int>*);
  virtual var* ner(constant<long int>*);
  virtual var* ner(constant<unsigned long int>*);
  virtual var* ner(constant<__int64>*);
  virtual var* ner(constant<unsigned __int64>*);
  virtual var* ner(constant<float>*);
  virtual var* ner(constant<double>*);
  virtual var* ner(constant<long double>*);
  virtual var* ner(constant<void*>*);
  virtual var* ner(addrof*);
  virtual var* bit_and(var*);
  virtual var* bit_andr(constant<bool>*);
  virtual var* bit_andr(constant<char>*);
  virtual var* bit_andr(constant<signed char>*);
  virtual var* bit_andr(constant<unsigned char>*);
  virtual var* bit_andr(constant<wchar_t>*);
  virtual var* bit_andr(constant<short int>*);
  virtual var* bit_andr(constant<unsigned short int>*);
  virtual var* bit_andr(constant<int>*);
  virtual var* bit_andr(constant<unsigned int>*);
  virtual var* bit_andr(constant<long int>*);
  virtual var* bit_andr(constant<unsigned long int>*);
  virtual var* bit_andr(constant<__int64>*);
  virtual var* bit_andr(constant<unsigned __int64>*);
  virtual var* bit_xor(var*);
  virtual var* bit_xorr(constant<bool>*);
  virtual var* bit_xorr(constant<char>*);
  virtual var* bit_xorr(constant<signed char>*);
  virtual var* bit_xorr(constant<unsigned char>*);
  virtual var* bit_xorr(constant<wchar_t>*);
  virtual var* bit_xorr(constant<short int>*);
  virtual var* bit_xorr(constant<unsigned short int>*);
  virtual var* bit_xorr(constant<int>*);
  virtual var* bit_xorr(constant<unsigned int>*);
  virtual var* bit_xorr(constant<long int>*);
  virtual var* bit_xorr(constant<unsigned long int>*);
  virtual var* bit_xorr(constant<__int64>*);
  virtual var* bit_xorr(constant<unsigned __int64>*);
  virtual var* bit_or(var*);
  virtual var* bit_orr(constant<bool>*);
  virtual var* bit_orr(constant<char>*);
  virtual var* bit_orr(constant<signed char>*);
  virtual var* bit_orr(constant<unsigned char>*);
  virtual var* bit_orr(constant<wchar_t>*);
  virtual var* bit_orr(constant<short int>*);
  virtual var* bit_orr(constant<unsigned short int>*);
  virtual var* bit_orr(constant<int>*);
  virtual var* bit_orr(constant<unsigned int>*);
  virtual var* bit_orr(constant<long int>*);
  virtual var* bit_orr(constant<unsigned long int>*);
  virtual var* bit_orr(constant<__int64>*);
  virtual var* bit_orr(constant<unsigned __int64>*);
  virtual var* logic1(bool, int, var*);
  virtual var* logic2(bool, const type*);
  virtual var* cond(int, int, var*, var*);
  virtual var* assign(var*);
  
  var* comma(var*);

  virtual var* offref(const type*, var*);
  virtual const type* result_type() const { return m_type; }
  virtual bool isconstant(bool = false) const { return false; }
  virtual __int64 value() const { return 0; }
  virtual void if_code(statements::if_stmt::info_t*);
  virtual void while_code(statements::while_stmt::info_t*, to3ac*);
  virtual void for_code(statements::for_stmt::info_t*, to3ac*);
  virtual void do_code(statements::do_stmt::info_t*, to3ac*);
  virtual ~var(){}
};

struct with_initial;

struct usr : var {
  std::string m_name;
  enum flag_t {
    NONE        = 0,
    TYPEDEF     = 1 << 0,
    EXTERN      = 1 << 1,
    STATIC      = 1 << 2,
    AUTO        = 1 << 3,
    REGISTER    = 1 << 4,
    INLINE      = 1 << 5,
    FUNCTION    = 1 << 6,
    ENUM_MEMBER = 1 << 7,
    BIT_FIELD   = 1 << 8,
    MSB_FIELD   = 1 << 9,
    VL          = 1 << 10,
    CONST_PTR   = 1 << 11,
    WITH_INI    = 1 << 12,
    SUB_CONST_LONG = 1 << 13,
    
    VIRTUAL     = 1 << 15,
    EXPLICIT    = 1 << 16,
    FRIEND      = 1 << 17,
    OVERLOAD    = 1 << 18,
    C_SYMBOL    = 1 << 19,
    CTOR        = 1 << 20,
    DTOR        = 1 << 21,
    STATIC_DEF  = 1 << 22,
    NAMESPACE   = 1 << 23,
    INITIALIZE_FUNCTION = 1 << 24,
    NEW_SCALAR  = 1 << 25,
    NEW_ARRAY   = 1 << 26,
    DELETE_SCALAR = 1 << 27,
    DELETE_ARRAY  = 1 << 28,
    INLINE_REFED = 1 << 29,
  };
  flag_t m_flag;
  file_t m_file;
  virtual bool lvalue() const { return true; }
  var* address();
  var* assign(var*);
  static std::string keyword(flag_t);
  usr* usr_cast(){ return this; }
  virtual with_initial* with_initial_cast(){ return 0; }
  virtual int initialize();
  usr(std::string name, const type* T, flag_t flag, const file_t& file)
    : var(T), m_name(name), m_flag(flag), m_file(file) {}
};

template<class T>
struct constant : usr {
  T m_value;
  static std::map<T, constant<T>*> table;
  bool lvalue() const { return false; }
  var* offref(const type*, var*);
  var* indirection();
  var* mul(var* z){ return z->mulr(this); }
  var* mulr(constant<bool>*);
  var* mulr(constant<char>*);
  var* mulr(constant<signed char>*);
  var* mulr(constant<unsigned char>*);
  var* mulr(constant<wchar_t>*);
  var* mulr(constant<short int>*);
  var* mulr(constant<unsigned short int>*);
  var* mulr(constant<int>*);
  var* mulr(constant<unsigned int>*);
  var* mulr(constant<long int>*);
  var* mulr(constant<unsigned long int>*);
  var* mulr(constant<__int64>*);
  var* mulr(constant<unsigned __int64>*);
  var* mulr(constant<float>*);
  var* mulr(constant<double>*);
  var* mulr(constant<long double>*);
  var* div(var* z){ return z->divr(this); }
  var* divr(constant<bool>*);
  var* divr(constant<char>*);
  var* divr(constant<signed char>*);
  var* divr(constant<unsigned char>*);
  var* divr(constant<wchar_t>*);
  var* divr(constant<short int>*);
  var* divr(constant<unsigned short int>*);
  var* divr(constant<int>*);
  var* divr(constant<unsigned int>*);
  var* divr(constant<long int>*);
  var* divr(constant<unsigned long int>*);
  var* divr(constant<__int64>*);
  var* divr(constant<unsigned __int64>*);
  var* divr(constant<float>*);
  var* divr(constant<double>*);
  var* divr(constant<long double>*);
  var* mod(var* z){ return z->modr(this); }
  var* modr(constant<bool>*);
  var* modr(constant<char>*);
  var* modr(constant<signed char>*);
  var* modr(constant<unsigned char>*);
  var* modr(constant<wchar_t>*);
  var* modr(constant<short int>*);
  var* modr(constant<unsigned short int>*);
  var* modr(constant<int>*);
  var* modr(constant<unsigned int>*);
  var* modr(constant<long int>*);
  var* modr(constant<unsigned long int>*);
  var* modr(constant<__int64>*);
  var* modr(constant<unsigned __int64>*);
  var* add(var* z){ return z->addr(this); }
  var* addr(constant<bool>*);
  var* addr(constant<char>*);
  var* addr(constant<signed char>*);
  var* addr(constant<unsigned char>*);
  var* addr(constant<wchar_t>*);
  var* addr(constant<short int>*);
  var* addr(constant<unsigned short int>*);
  var* addr(constant<int>*);
  var* addr(constant<unsigned int>*);
  var* addr(constant<long int>*);
  var* addr(constant<unsigned long int>*);
  var* addr(constant<__int64>*);
  var* addr(constant<unsigned __int64>*);
  var* addr(constant<void*>*);
  var* addr(constant<float>*);
  var* addr(constant<double>*);
  var* addr(constant<long double>*);
  var* addr(addrof*);
  var* sub(var* z){ return z->subr(this); }
  var* subr(constant<bool>*);
  var* subr(constant<char>*);
  var* subr(constant<signed char>*);
  var* subr(constant<unsigned char>*);
  var* subr(constant<wchar_t>*);
  var* subr(constant<short int>*);
  var* subr(constant<unsigned short int>*);
  var* subr(constant<int>*);
  var* subr(constant<unsigned int>*);
  var* subr(constant<long int>*);
  var* subr(constant<unsigned long int>*);
  var* subr(constant<__int64>*);
  var* subr(constant<unsigned __int64>*);
  var* subr(constant<void*>*);
  var* subr(constant<float>*);
  var* subr(constant<double>*);
  var* subr(constant<long double>*);
  var* subr(addrof*);
  var* lsh(var* z){ return z->lshr(this); }
  var* lshr(constant<bool>*);
  var* lshr(constant<char>*);
  var* lshr(constant<signed char>*);
  var* lshr(constant<unsigned char>*);
  var* lshr(constant<wchar_t>*);
  var* lshr(constant<short int>*);
  var* lshr(constant<unsigned short int>*);
  var* lshr(constant<int>*);
  var* lshr(constant<unsigned int>*);
  var* lshr(constant<long int>*);
  var* lshr(constant<unsigned long int>*);
  var* lshr(constant<__int64>*);
  var* lshr(constant<unsigned __int64>*);
  var* rsh(var* z){ return z->rshr(this); }
  var* rshr(constant<bool>*);
  var* rshr(constant<char>*);
  var* rshr(constant<signed char>*);
  var* rshr(constant<unsigned char>*);
  var* rshr(constant<wchar_t>*);
  var* rshr(constant<short int>*);
  var* rshr(constant<unsigned short int>*);
  var* rshr(constant<int>*);
  var* rshr(constant<unsigned int>*);
  var* rshr(constant<long int>*);
  var* rshr(constant<unsigned long int>*);
  var* rshr(constant<__int64>*);
  var* rshr(constant<unsigned __int64>*);
  var* lt(var* z){ return z->ltr(this); }
  var* ltr(constant<bool>*);
  var* ltr(constant<char>*);
  var* ltr(constant<signed char>*);
  var* ltr(constant<unsigned char>*);
  var* ltr(constant<wchar_t>*);
  var* ltr(constant<short int>*);
  var* ltr(constant<unsigned short int>*);
  var* ltr(constant<int>*);
  var* ltr(constant<unsigned int>*);
  var* ltr(constant<long int>*);
  var* ltr(constant<unsigned long int>*);
  var* ltr(constant<__int64>*);
  var* ltr(constant<unsigned __int64>*);
  var* ltr(constant<float>*);
  var* ltr(constant<double>*);
  var* ltr(constant<long double>*);
  var* gt(var* z){ return z->gtr(this); }
  var* gtr(constant<bool>*);
  var* gtr(constant<char>*);
  var* gtr(constant<signed char>*);
  var* gtr(constant<unsigned char>*);
  var* gtr(constant<wchar_t>*);
  var* gtr(constant<short int>*);
  var* gtr(constant<unsigned short int>*);
  var* gtr(constant<int>*);
  var* gtr(constant<unsigned int>*);
  var* gtr(constant<long int>*);
  var* gtr(constant<unsigned long int>*);
  var* gtr(constant<__int64>*);
  var* gtr(constant<unsigned __int64>*);
  var* gtr(constant<float>*);
  var* gtr(constant<double>*);
  var* gtr(constant<long double>*);
  var* le(var* z){ return z->ler(this); }
  var* ler(constant<bool>*);
  var* ler(constant<char>*);
  var* ler(constant<signed char>*);
  var* ler(constant<unsigned char>*);
  var* ler(constant<wchar_t>*);
  var* ler(constant<short int>*);
  var* ler(constant<unsigned short int>*);
  var* ler(constant<int>*);
  var* ler(constant<unsigned int>*);
  var* ler(constant<long int>*);
  var* ler(constant<unsigned long int>*);
  var* ler(constant<__int64>*);
  var* ler(constant<unsigned __int64>*);
  var* ler(constant<float>*);
  var* ler(constant<double>*);
  var* ler(constant<long double>*);
  var* ge(var* z){ return z->ger(this); }
  var* ger(constant<bool>*);
  var* ger(constant<char>*);
  var* ger(constant<signed char>*);
  var* ger(constant<unsigned char>*);
  var* ger(constant<wchar_t>*);
  var* ger(constant<short int>*);
  var* ger(constant<unsigned short int>*);
  var* ger(constant<int>*);
  var* ger(constant<unsigned int>*);
  var* ger(constant<long int>*);
  var* ger(constant<unsigned long int>*);
  var* ger(constant<__int64>*);
  var* ger(constant<unsigned __int64>*);
  var* ger(constant<float>*);
  var* ger(constant<double>*);
  var* ger(constant<long double>*);
  var* eq(var* z){ return z->eqr(this); }
  var* eqr(constant<bool>*);
  var* eqr(constant<char>*);
  var* eqr(constant<signed char>*);
  var* eqr(constant<unsigned char>*);
  var* eqr(constant<wchar_t>*);
  var* eqr(constant<short int>*);
  var* eqr(constant<unsigned short int>*);
  var* eqr(constant<int>*);
  var* eqr(constant<unsigned int>*);
  var* eqr(constant<long int>*);
  var* eqr(constant<unsigned long int>*);
  var* eqr(constant<__int64>*);
  var* eqr(constant<unsigned __int64>*);
  var* eqr(constant<float>*);
  var* eqr(constant<double>*);
  var* eqr(constant<long double>*);
  var* eqr(constant<void*>*);
  var* ne(var* z){ return z->ner(this); }
  var* ner(constant<bool>*);
  var* ner(constant<char>*);
  var* ner(constant<signed char>*);
  var* ner(constant<unsigned char>*);
  var* ner(constant<wchar_t>*);
  var* ner(constant<short int>*);
  var* ner(constant<unsigned short int>*);
  var* ner(constant<int>*);
  var* ner(constant<unsigned int>*);
  var* ner(constant<long int>*);
  var* ner(constant<unsigned long int>*);
  var* ner(constant<__int64>*);
  var* ner(constant<unsigned __int64>*);
  var* ner(constant<float>*);
  var* ner(constant<double>*);
  var* ner(constant<long double>*);
  var* ner(constant<void*>*);
  var* bit_and(var* z){ return z->bit_andr(this); }
  var* bit_andr(constant<bool>*);
  var* bit_andr(constant<char>*);
  var* bit_andr(constant<signed char>*);
  var* bit_andr(constant<unsigned char>*);
  var* bit_andr(constant<wchar_t>*);
  var* bit_andr(constant<short int>*);
  var* bit_andr(constant<unsigned short int>*);
  var* bit_andr(constant<int>*);
  var* bit_andr(constant<unsigned int>*);
  var* bit_andr(constant<long int>*);
  var* bit_andr(constant<unsigned long int>*);
  var* bit_andr(constant<__int64>*);
  var* bit_andr(constant<unsigned __int64>*);
  var* bit_xor(var* z){ return z->bit_xorr(this); }
  var* bit_xorr(constant<bool>*);
  var* bit_xorr(constant<char>*);
  var* bit_xorr(constant<signed char>*);
  var* bit_xorr(constant<unsigned char>*);
  var* bit_xorr(constant<wchar_t>*);
  var* bit_xorr(constant<short int>*);
  var* bit_xorr(constant<unsigned short int>*);
  var* bit_xorr(constant<int>*);
  var* bit_xorr(constant<unsigned int>*);
  var* bit_xorr(constant<long int>*);
  var* bit_xorr(constant<unsigned long int>*);
  var* bit_xorr(constant<__int64>*);
  var* bit_xorr(constant<unsigned __int64>*);
  var* bit_or(var* z){ return z->bit_orr(this); }
  var* bit_orr(constant<bool>*);
  var* bit_orr(constant<char>*);
  var* bit_orr(constant<signed char>*);
  var* bit_orr(constant<unsigned char>*);
  var* bit_orr(constant<wchar_t>*);
  var* bit_orr(constant<short int>*);
  var* bit_orr(constant<unsigned short int>*);
  var* bit_orr(constant<int>*);
  var* bit_orr(constant<unsigned int>*);
  var* bit_orr(constant<long int>*);
  var* bit_orr(constant<unsigned long int>*);
  var* bit_orr(constant<__int64>*);
  var* bit_orr(constant<unsigned __int64>*);
  var* logic1(bool, int, var*);
  var* logic2(bool, const type*);
  var* cond(int, int, var*, var*);
  var* plus()
  {
    return (m_flag & CONST_PTR) ? var::plus() : promotion();
  }
  var* minus();
  var* tilde();
  var* cast(const type*);
  bool zero() const { return m_value == 0; }
  var* _not();
  bool isconstant(bool = false) const { return true; }
  __int64 value() const { return m_value; }
  void if_code(statements::if_stmt::info_t*);
  void while_code(statements::while_stmt::info_t*, to3ac*);
  void for_code(statements::for_stmt::info_t*, to3ac*);
  void do_code(statements::do_stmt::info_t*, to3ac*);
  constant(std::string name, const type* TT, flag_t flag, const file_t& file)
    : usr(name, TT, flag, file) {}
};

template<> struct constant<float> : usr {
  float m_value;
  static std::map<int, constant<float>*> table;
  bool lvalue() const { return false; }
  var* plus(){ return this; }
  var* minus();
  var* cast(const type*);

  var* mul(var*);
  var* mulr(constant<bool>*);
  var* mulr(constant<char>*);
  var* mulr(constant<signed char>*);
  var* mulr(constant<unsigned char>*);
  var* mulr(constant<wchar_t>*);
  var* mulr(constant<short int>*);
  var* mulr(constant<unsigned short int>*);
  var* mulr(constant<int>*);
  var* mulr(constant<unsigned int>*);
  var* mulr(constant<long int>*);
  var* mulr(constant<unsigned long int>*);
  var* mulr(constant<__int64>*);
  var* mulr(constant<unsigned __int64>*);
  var* mulr(constant<float>*);
  var* mulr(constant<double>*);
  var* mulr(constant<long double>*);
  var* div(var*);
  var* divr(constant<bool>*);
  var* divr(constant<char>*);
  var* divr(constant<signed char>*);
  var* divr(constant<unsigned char>*);
  var* divr(constant<wchar_t>*);
  var* divr(constant<short int>*);
  var* divr(constant<unsigned short int>*);
  var* divr(constant<int>*);
  var* divr(constant<unsigned int>*);
  var* divr(constant<long int>*);
  var* divr(constant<unsigned long int>*);
  var* divr(constant<__int64>*);
  var* divr(constant<unsigned __int64>*);
  var* divr(constant<float>*);
  var* divr(constant<double>*);
  var* divr(constant<long double>*);
  var* add(var*);
  var* addr(constant<bool>*);
  var* addr(constant<char>*);
  var* addr(constant<signed char>*);
  var* addr(constant<unsigned char>*);
  var* addr(constant<wchar_t>*);
  var* addr(constant<short int>*);
  var* addr(constant<unsigned short int>*);
  var* addr(constant<int>*);
  var* addr(constant<unsigned int>*);
  var* addr(constant<long int>*);
  var* addr(constant<unsigned long int>*);
  var* addr(constant<__int64>*);
  var* addr(constant<unsigned __int64>*);
  var* addr(constant<float>*);
  var* addr(constant<double>*);
  var* addr(constant<long double>*);
  var* sub(var*);
  var* subr(constant<bool>*);
  var* subr(constant<char>*);
  var* subr(constant<signed char>*);
  var* subr(constant<unsigned char>*);
  var* subr(constant<wchar_t>*);
  var* subr(constant<short int>*);
  var* subr(constant<unsigned short int>*);
  var* subr(constant<int>*);
  var* subr(constant<unsigned int>*);
  var* subr(constant<long int>*);
  var* subr(constant<unsigned long int>*);
  var* subr(constant<__int64>*);
  var* subr(constant<unsigned __int64>*);
  var* subr(constant<float>*);
  var* subr(constant<double>*);
  var* subr(constant<long double>*);
  var* lt(var*);
  var* ltr(constant<bool>*);
  var* ltr(constant<char>*);
  var* ltr(constant<signed char>*);
  var* ltr(constant<unsigned char>*);
  var* ltr(constant<wchar_t>*);
  var* ltr(constant<short int>*);
  var* ltr(constant<unsigned short int>*);
  var* ltr(constant<int>*);
  var* ltr(constant<unsigned int>*);
  var* ltr(constant<long int>*);
  var* ltr(constant<unsigned long int>*);
  var* ltr(constant<__int64>*);
  var* ltr(constant<unsigned __int64>*);
  var* ltr(constant<float>*);
  var* ltr(constant<double>*);
  var* ltr(constant<long double>*);
  var* gt(var*);
  var* gtr(constant<bool>*);
  var* gtr(constant<char>*);
  var* gtr(constant<signed char>*);
  var* gtr(constant<unsigned char>*);
  var* gtr(constant<wchar_t>*);
  var* gtr(constant<short int>*);
  var* gtr(constant<unsigned short int>*);
  var* gtr(constant<int>*);
  var* gtr(constant<unsigned int>*);
  var* gtr(constant<long int>*);
  var* gtr(constant<unsigned long int>*);
  var* gtr(constant<__int64>*);
  var* gtr(constant<unsigned __int64>*);
  var* gtr(constant<float>*);
  var* gtr(constant<double>*);
  var* gtr(constant<long double>*);
  var* le(var*);
  var* ler(constant<bool>*);
  var* ler(constant<char>*);
  var* ler(constant<signed char>*);
  var* ler(constant<unsigned char>*);
  var* ler(constant<wchar_t>*);
  var* ler(constant<short int>*);
  var* ler(constant<unsigned short int>*);
  var* ler(constant<int>*);
  var* ler(constant<unsigned int>*);
  var* ler(constant<long int>*);
  var* ler(constant<unsigned long int>*);
  var* ler(constant<__int64>*);
  var* ler(constant<unsigned __int64>*);
  var* ler(constant<float>*);
  var* ler(constant<double>*);
  var* ler(constant<long double>*);
  var* ge(var*);
  var* ger(constant<bool>*);
  var* ger(constant<char>*);
  var* ger(constant<signed char>*);
  var* ger(constant<unsigned char>*);
  var* ger(constant<wchar_t>*);
  var* ger(constant<short int>*);
  var* ger(constant<unsigned short int>*);
  var* ger(constant<int>*);
  var* ger(constant<unsigned int>*);
  var* ger(constant<long int>*);
  var* ger(constant<unsigned long int>*);
  var* ger(constant<__int64>*);
  var* ger(constant<unsigned __int64>*);
  var* ger(constant<float>*);
  var* ger(constant<double>*);
  var* ger(constant<long double>*);
  var* eq(var*);
  var* eqr(constant<bool>*);
  var* eqr(constant<char>*);
  var* eqr(constant<signed char>*);
  var* eqr(constant<unsigned char>*);
  var* eqr(constant<wchar_t>*);
  var* eqr(constant<short int>*);
  var* eqr(constant<unsigned short int>*);
  var* eqr(constant<int>*);
  var* eqr(constant<unsigned int>*);
  var* eqr(constant<long int>*);
  var* eqr(constant<unsigned long int>*);
  var* eqr(constant<__int64>*);
  var* eqr(constant<unsigned __int64>*);
  var* eqr(constant<float>*);
  var* eqr(constant<double>*);
  var* eqr(constant<long double>*);
  var* ne(var*);
  var* ner(constant<bool>*);
  var* ner(constant<char>*);
  var* ner(constant<signed char>*);
  var* ner(constant<unsigned char>*);
  var* ner(constant<wchar_t>*);
  var* ner(constant<short int>*);
  var* ner(constant<unsigned short int>*);
  var* ner(constant<int>*);
  var* ner(constant<unsigned int>*);
  var* ner(constant<long int>*);
  var* ner(constant<unsigned long int>*);
  var* ner(constant<__int64>*);
  var* ner(constant<unsigned __int64>*);
  var* ner(constant<float>*);
  var* ner(constant<double>*);
  var* ner(constant<long double>*);
  var* logic1(bool, int, var*);
  var* logic2(bool, const type*);
  var* cond(int, int, var*, var*);

  bool zero(){ return m_value == 0.0F; }
  var* _not();
  bool isconstant(bool = false) const { return true; }
  void if_code(statements::if_stmt::info_t*);
  void while_code(statements::while_stmt::info_t*, to3ac*);
  void for_code(statements::for_stmt::info_t*, to3ac*);
  void do_code(statements::do_stmt::info_t*, to3ac*);
  constant(std::string name, const type* T, flag_t flag, const file_t& file)
    : usr(name, T, flag, file) {}
};

template<> struct constant<double> : usr {
  double m_value;
  static std::map<__int64, constant<double>*> table;
  bool lvalue() const { return false; }
  var* plus(){ return this; }
  var* minus();
  var* cast(const type*);

  var* mul(var*);
  var* mulr(constant<bool>*);
  var* mulr(constant<char>*);
  var* mulr(constant<signed char>*);
  var* mulr(constant<unsigned char>*);
  var* mulr(constant<wchar_t>*);
  var* mulr(constant<short int>*);
  var* mulr(constant<unsigned short int>*);
  var* mulr(constant<int>*);
  var* mulr(constant<unsigned int>*);
  var* mulr(constant<long int>*);
  var* mulr(constant<unsigned long int>*);
  var* mulr(constant<__int64>*);
  var* mulr(constant<unsigned __int64>*);
  var* mulr(constant<float>*);
  var* mulr(constant<double>*);
  var* mulr(constant<long double>*);
  var* div(var*);
  var* divr(constant<bool>*);
  var* divr(constant<char>*);
  var* divr(constant<signed char>*);
  var* divr(constant<unsigned char>*);
  var* divr(constant<wchar_t>*);
  var* divr(constant<short int>*);
  var* divr(constant<unsigned short int>*);
  var* divr(constant<int>*);
  var* divr(constant<unsigned int>*);
  var* divr(constant<long int>*);
  var* divr(constant<unsigned long int>*);
  var* divr(constant<__int64>*);
  var* divr(constant<unsigned __int64>*);
  var* divr(constant<float>*);
  var* divr(constant<double>*);
  var* divr(constant<long double>*);
  var* add(var*);
  var* addr(constant<bool>*);
  var* addr(constant<char>*);
  var* addr(constant<signed char>*);
  var* addr(constant<unsigned char>*);
  var* addr(constant<wchar_t>*);
  var* addr(constant<short int>*);
  var* addr(constant<unsigned short int>*);
  var* addr(constant<int>*);
  var* addr(constant<unsigned int>*);
  var* addr(constant<long int>*);
  var* addr(constant<unsigned long int>*);
  var* addr(constant<__int64>*);
  var* addr(constant<unsigned __int64>*);
  var* addr(constant<float>*);
  var* addr(constant<double>*);
  var* addr(constant<long double>*);
  var* sub(var*);
  var* subr(constant<bool>*);
  var* subr(constant<char>*);
  var* subr(constant<signed char>*);
  var* subr(constant<unsigned char>*);
  var* subr(constant<wchar_t>*);
  var* subr(constant<short int>*);
  var* subr(constant<unsigned short int>*);
  var* subr(constant<int>*);
  var* subr(constant<unsigned int>*);
  var* subr(constant<long int>*);
  var* subr(constant<unsigned long int>*);
  var* subr(constant<__int64>*);
  var* subr(constant<unsigned __int64>*);
  var* subr(constant<float>*);
  var* subr(constant<double>*);
  var* subr(constant<long double>*);
  var* lt(var*);
  var* ltr(constant<bool>*);
  var* ltr(constant<char>*);
  var* ltr(constant<signed char>*);
  var* ltr(constant<unsigned char>*);
  var* ltr(constant<wchar_t>*);
  var* ltr(constant<short int>*);
  var* ltr(constant<unsigned short int>*);
  var* ltr(constant<int>*);
  var* ltr(constant<unsigned int>*);
  var* ltr(constant<long int>*);
  var* ltr(constant<unsigned long int>*);
  var* ltr(constant<__int64>*);
  var* ltr(constant<unsigned __int64>*);
  var* ltr(constant<float>*);
  var* ltr(constant<double>*);
  var* ltr(constant<long double>*);
  var* gt(var*);
  var* gtr(constant<bool>*);
  var* gtr(constant<char>*);
  var* gtr(constant<signed char>*);
  var* gtr(constant<unsigned char>*);
  var* gtr(constant<wchar_t>*);
  var* gtr(constant<short int>*);
  var* gtr(constant<unsigned short int>*);
  var* gtr(constant<int>*);
  var* gtr(constant<unsigned int>*);
  var* gtr(constant<long int>*);
  var* gtr(constant<unsigned long int>*);
  var* gtr(constant<__int64>*);
  var* gtr(constant<unsigned __int64>*);
  var* gtr(constant<float>*);
  var* gtr(constant<double>*);
  var* gtr(constant<long double>*);
  var* le(var*);
  var* ler(constant<bool>*);
  var* ler(constant<char>*);
  var* ler(constant<signed char>*);
  var* ler(constant<unsigned char>*);
  var* ler(constant<wchar_t>*);
  var* ler(constant<short int>*);
  var* ler(constant<unsigned short int>*);
  var* ler(constant<int>*);
  var* ler(constant<unsigned int>*);
  var* ler(constant<long int>*);
  var* ler(constant<unsigned long int>*);
  var* ler(constant<__int64>*);
  var* ler(constant<unsigned __int64>*);
  var* ler(constant<float>*);
  var* ler(constant<double>*);
  var* ler(constant<long double>*);
  var* ge(var*);
  var* ger(constant<bool>*);
  var* ger(constant<char>*);
  var* ger(constant<signed char>*);
  var* ger(constant<unsigned char>*);
  var* ger(constant<wchar_t>*);
  var* ger(constant<short int>*);
  var* ger(constant<unsigned short int>*);
  var* ger(constant<int>*);
  var* ger(constant<unsigned int>*);
  var* ger(constant<long int>*);
  var* ger(constant<unsigned long int>*);
  var* ger(constant<__int64>*);
  var* ger(constant<unsigned __int64>*);
  var* ger(constant<float>*);
  var* ger(constant<double>*);
  var* ger(constant<long double>*);
  var* eq(var*);
  var* eqr(constant<bool>*);
  var* eqr(constant<char>*);
  var* eqr(constant<signed char>*);
  var* eqr(constant<unsigned char>*);
  var* eqr(constant<wchar_t>*);
  var* eqr(constant<short int>*);
  var* eqr(constant<unsigned short int>*);
  var* eqr(constant<int>*);
  var* eqr(constant<unsigned int>*);
  var* eqr(constant<long int>*);
  var* eqr(constant<unsigned long int>*);
  var* eqr(constant<__int64>*);
  var* eqr(constant<unsigned __int64>*);
  var* eqr(constant<float>*);
  var* eqr(constant<double>*);
  var* eqr(constant<long double>*);
  var* ne(var*);
  var* ner(constant<bool>*);
  var* ner(constant<char>*);
  var* ner(constant<signed char>*);
  var* ner(constant<unsigned char>*);
  var* ner(constant<wchar_t>*);
  var* ner(constant<short int>*);
  var* ner(constant<unsigned short int>*);
  var* ner(constant<int>*);
  var* ner(constant<unsigned int>*);
  var* ner(constant<long int>*);
  var* ner(constant<unsigned long int>*);
  var* ner(constant<__int64>*);
  var* ner(constant<unsigned __int64>*);
  var* ner(constant<float>*);
  var* ner(constant<double>*);
  var* ner(constant<long double>*);
  var* logic1(bool, int, var*);
  var* logic2(bool, const type*);
  var* cond(int, int, var*, var*);

  bool zero(){ return m_value == 0.0; }
  var* _not();
  bool isconstant(bool = false) const { return true; }
  void if_code(statements::if_stmt::info_t*);
  void while_code(statements::while_stmt::info_t*, to3ac*);
  void for_code(statements::for_stmt::info_t*, to3ac*);
  void do_code(statements::do_stmt::info_t*, to3ac*);
  constant(std::string name, const type* T, flag_t flag, const file_t& file)
    : usr(name, T, flag, file) {}
};

template<> struct constant<long double> : usr {
  long double m_value;
  unsigned char* b;
  static std::map<std::pair<__int64,__int64>, constant<long double>*> table;
  bool lvalue() const { return false; }
  var* plus(){ return this; }
  var* minus();
  var* cast(const type*);

  var* mul(var*);
  var* mulr(constant<bool>*);
  var* mulr(constant<char>*);
  var* mulr(constant<signed char>*);
  var* mulr(constant<unsigned char>*);
  var* mulr(constant<wchar_t>*);
  var* mulr(constant<short int>*);
  var* mulr(constant<unsigned short int>*);
  var* mulr(constant<int>*);
  var* mulr(constant<unsigned int>*);
  var* mulr(constant<long int>*);
  var* mulr(constant<unsigned long int>*);
  var* mulr(constant<__int64>*);
  var* mulr(constant<unsigned __int64>*);
  var* mulr(constant<float>*);
  var* mulr(constant<double>*);
  var* mulr(constant<long double>*);
  var* div(var*);
  var* divr(constant<bool>*);
  var* divr(constant<char>*);
  var* divr(constant<signed char>*);
  var* divr(constant<unsigned char>*);
  var* divr(constant<wchar_t>*);
  var* divr(constant<short int>*);
  var* divr(constant<unsigned short int>*);
  var* divr(constant<int>*);
  var* divr(constant<unsigned int>*);
  var* divr(constant<long int>*);
  var* divr(constant<unsigned long int>*);
  var* divr(constant<__int64>*);
  var* divr(constant<unsigned __int64>*);
  var* divr(constant<float>*);
  var* divr(constant<double>*);
  var* divr(constant<long double>*);
  var* add(var*);
  var* addr(constant<bool>*);
  var* addr(constant<char>*);
  var* addr(constant<signed char>*);
  var* addr(constant<unsigned char>*);
  var* addr(constant<wchar_t>*);
  var* addr(constant<short int>*);
  var* addr(constant<unsigned short int>*);
  var* addr(constant<int>*);
  var* addr(constant<unsigned int>*);
  var* addr(constant<long int>*);
  var* addr(constant<unsigned long int>*);
  var* addr(constant<__int64>*);
  var* addr(constant<unsigned __int64>*);
  var* addr(constant<float>*);
  var* addr(constant<double>*);
  var* addr(constant<long double>*);
  var* sub(var*);
  var* subr(constant<bool>*);
  var* subr(constant<char>*);
  var* subr(constant<signed char>*);
  var* subr(constant<unsigned char>*);
  var* subr(constant<wchar_t>*);
  var* subr(constant<short int>*);
  var* subr(constant<unsigned short int>*);
  var* subr(constant<int>*);
  var* subr(constant<unsigned int>*);
  var* subr(constant<long int>*);
  var* subr(constant<unsigned long int>*);
  var* subr(constant<__int64>*);
  var* subr(constant<unsigned __int64>*);
  var* subr(constant<float>*);
  var* subr(constant<double>*);
  var* subr(constant<long double>*);
  var* lt(var*);
  var* ltr(constant<bool>*);
  var* ltr(constant<char>*);
  var* ltr(constant<signed char>*);
  var* ltr(constant<unsigned char>*);
  var* ltr(constant<wchar_t>*);
  var* ltr(constant<short int>*);
  var* ltr(constant<unsigned short int>*);
  var* ltr(constant<int>*);
  var* ltr(constant<unsigned int>*);
  var* ltr(constant<long int>*);
  var* ltr(constant<unsigned long int>*);
  var* ltr(constant<__int64>*);
  var* ltr(constant<unsigned __int64>*);
  var* ltr(constant<float>*);
  var* ltr(constant<double>*);
  var* ltr(constant<long double>*);
  var* gt(var*);
  var* gtr(constant<bool>*);
  var* gtr(constant<char>*);
  var* gtr(constant<signed char>*);
  var* gtr(constant<unsigned char>*);
  var* gtr(constant<wchar_t>*);
  var* gtr(constant<short int>*);
  var* gtr(constant<unsigned short int>*);
  var* gtr(constant<int>*);
  var* gtr(constant<unsigned int>*);
  var* gtr(constant<long int>*);
  var* gtr(constant<unsigned long int>*);
  var* gtr(constant<__int64>*);
  var* gtr(constant<unsigned __int64>*);
  var* gtr(constant<float>*);
  var* gtr(constant<double>*);
  var* gtr(constant<long double>*);
  var* le(var*);
  var* ler(constant<bool>*);
  var* ler(constant<char>*);
  var* ler(constant<signed char>*);
  var* ler(constant<unsigned char>*);
  var* ler(constant<wchar_t>*);
  var* ler(constant<short int>*);
  var* ler(constant<unsigned short int>*);
  var* ler(constant<int>*);
  var* ler(constant<unsigned int>*);
  var* ler(constant<long int>*);
  var* ler(constant<unsigned long int>*);
  var* ler(constant<__int64>*);
  var* ler(constant<unsigned __int64>*);
  var* ler(constant<float>*);
  var* ler(constant<double>*);
  var* ler(constant<long double>*);
  var* ge(var*);
  var* ger(constant<bool>*);
  var* ger(constant<char>*);
  var* ger(constant<signed char>*);
  var* ger(constant<unsigned char>*);
  var* ger(constant<wchar_t>*);
  var* ger(constant<short int>*);
  var* ger(constant<unsigned short int>*);
  var* ger(constant<int>*);
  var* ger(constant<unsigned int>*);
  var* ger(constant<long int>*);
  var* ger(constant<unsigned long int>*);
  var* ger(constant<__int64>*);
  var* ger(constant<unsigned __int64>*);
  var* ger(constant<float>*);
  var* ger(constant<double>*);
  var* ger(constant<long double>*);
  var* eq(var*);
  var* eqr(constant<bool>*);
  var* eqr(constant<char>*);
  var* eqr(constant<signed char>*);
  var* eqr(constant<unsigned char>*);
  var* eqr(constant<wchar_t>*);
  var* eqr(constant<short int>*);
  var* eqr(constant<unsigned short int>*);
  var* eqr(constant<int>*);
  var* eqr(constant<unsigned int>*);
  var* eqr(constant<long int>*);
  var* eqr(constant<unsigned long int>*);
  var* eqr(constant<__int64>*);
  var* eqr(constant<unsigned __int64>*);
  var* eqr(constant<float>*);
  var* eqr(constant<double>*);
  var* eqr(constant<long double>*);
  var* ne(var*);
  var* ner(constant<bool>*);
  var* ner(constant<char>*);
  var* ner(constant<signed char>*);
  var* ner(constant<unsigned char>*);
  var* ner(constant<wchar_t>*);
  var* ner(constant<short int>*);
  var* ner(constant<unsigned short int>*);
  var* ner(constant<int>*);
  var* ner(constant<unsigned int>*);
  var* ner(constant<long int>*);
  var* ner(constant<unsigned long int>*);
  var* ner(constant<__int64>*);
  var* ner(constant<unsigned __int64>*);
  var* ner(constant<float>*);
  var* ner(constant<double>*);
  var* ner(constant<long double>*);
  var* logic1(bool, int, var*);
  var* logic2(bool, const type*);
  var* cond(int, int, var*, var*);

  bool zero();
  var* _not();
  bool isconstant(bool = false) const { return true; }
  void if_code(statements::if_stmt::info_t*);
  void while_code(statements::while_stmt::info_t*, to3ac*);
  void for_code(statements::for_stmt::info_t*, to3ac*);
  void do_code(statements::do_stmt::info_t*, to3ac*);
  constant(std::string name, const type* T, flag_t flag, const file_t& file)
    : usr(name, T, flag, file) {}
};

template<> struct constant<void*> : usr {
  void* m_value;
  static std::pair<std::map<std::pair<const type*, void*>, constant<void*>* >,std::map<std::pair<const type*, void*>, constant<void*>* > > table;
  static void destroy_temporary(){ table.first.clear(); }
  bool lvalue() const { return false; }
  var* offref(const type*, var*);
  var* indirection();
  var* add(var* z){ return z->addr(this); }
  var* addr(constant<bool>*);
  var* addr(constant<char>*);
  var* addr(constant<signed char>*);
  var* addr(constant<unsigned char>*);
  var* addr(constant<wchar_t>*);
  var* addr(constant<short int>*);
  var* addr(constant<unsigned short int>*);
  var* addr(constant<int>*);
  var* addr(constant<unsigned int>*);
  var* addr(constant<long int>*);
  var* addr(constant<unsigned long int>*);
  var* addr(constant<__int64>*);
  var* addr(constant<unsigned __int64>*);
  var* sub(var* z){ return z->subr(this); }
  var* subr(constant<void*>*);
  var* lt(var*);
  var* ltr(constant<void*>*);
  var* gt(var*);
  var* gtr(constant<void*>*);
  var* le(var*);
  var* ler(constant<void*>*);
  var* ge(var*);
  var* ger(constant<void*>*);
  var* eq(var*);
  var* eqr(constant<bool>*);
  var* eqr(constant<char>*);
  var* eqr(constant<signed char>*);
  var* eqr(constant<unsigned char>*);
  var* eqr(constant<wchar_t>*);
  var* eqr(constant<short int>*);
  var* eqr(constant<unsigned short int>*);
  var* eqr(constant<int>*);
  var* eqr(constant<unsigned int>*);
  var* eqr(constant<long int>*);
  var* eqr(constant<unsigned long int>*);
  var* eqr(constant<__int64>*);
  var* eqr(constant<unsigned __int64>*);
  var* eqr(constant<void*>*);
  var* ne(var*);
  var* ner(constant<bool>*);
  var* ner(constant<char>*);
  var* ner(constant<signed char>*);
  var* ner(constant<unsigned char>*);
  var* ner(constant<wchar_t>*);
  var* ner(constant<short int>*);
  var* ner(constant<unsigned short int>*);
  var* ner(constant<int>*);
  var* ner(constant<unsigned int>*);
  var* ner(constant<long int>*);
  var* ner(constant<unsigned long int>*);
  var* ner(constant<__int64>*);
  var* ner(constant<unsigned __int64>*);
  var* ner(constant<void*>*);
  var* logic1(bool, int, var*);
  var* logic2(bool, const type*);
  var* cast(const type*);
  var* cond(int, int, var*, var*);

  bool zero() const { return m_value == 0; }
  var* _not();
  __int64 value() const { return (__int64)m_value; }
  bool isconstant(bool = false) const { return true; }
  void if_code(statements::if_stmt::info_t*);
  void while_code(statements::while_stmt::info_t*, to3ac*);
  void for_code(statements::for_stmt::info_t*, to3ac*);
  void do_code(statements::do_stmt::info_t*, to3ac*);
  constant(std::string name, const type* T, flag_t flag, const file_t& file)
    : usr(name, T, flag, file) {}
};

struct with_initial : usr {
  std::map<int,var*> m_value;

  with_initial(const usr& u) : usr(u) { m_flag = flag_t(m_flag|usr::WITH_INI); }

  with_initial(std::string name, const type* T, const file_t& file)
    : usr(name,T,flag_t(usr::STATIC|usr::WITH_INI),file) {}

  with_initial* with_initial_cast(){ return this; }
};

struct type_def : usr {
  std::vector<file_t> m_refed;
  type_def(const usr& u) : usr(u) {}
};
 
struct name_space : scope, usr {
  name_space(std::string name, const file_t& file)
    : scope(scope::NAMESPACE), usr(name,0,usr::NAMESPACE,file) {}
};

class pointer_type;

struct addrof : virtual var {
  var* m_ref;
  int m_offset;
  addrof(const type*, var*, int);
  var* rvalue();
  var* indirection();
  var* add(var*);
  var* addr(constant<bool>*);
  var* addr(constant<char>*);
  var* addr(constant<signed char>*);
  var* addr(constant<unsigned char>*);
  var* addr(constant<wchar_t>*);
  var* addr(constant<short int>*);
  var* addr(constant<unsigned short int>*);
  var* addr(constant<int>*);
  var* addr(constant<unsigned int>*);
  var* addr(constant<long int>*);
  var* addr(constant<unsigned long int>*);
  var* addr(constant<__int64>*);
  var* addr(constant<unsigned __int64>*);
  var* sub(var*);
  var* subr(addrof*);
  var* lt(var*);
  var* ltr(addrof*);
  var* gt(var*);
  var* gtr(addrof*);
  var* le(var*);
  var* ler(addrof*);
  var* ge(var*);
  var* ger(addrof*);
  var* eq(var*);
  var* eqr(addrof*);
  var* ne(var*);
  var* ner(addrof*);
  var* cast(const type*);
  var* offref(const type*, var*);
  bool isconstant(bool b) const { return b; }
  addrof* addrof_cast(){ return this; }
};

struct type {
  enum id_t {
    VOID, CHAR, SCHAR, UCHAR, WCHAR, BOOL, SHORT, USHORT, INT, UINT,
    LONG, ULONG, LONGLONG, ULONGLONG,
    FLOAT, DOUBLE, LONG_DOUBLE,
    BACKPATCH,
    CONST, VOLATILE, RESTRICT,
    POINTER, REFERENCE, ARRAY, FUNC, RECORD, ENUM, BIT_FIELD, ELLIPSIS,
    INCOMPLETE_TAGGED, VARRAY,
    POINTER_MEMBER,
  };
  id_t m_id;
  type(id_t id) : m_id(id) {}
  virtual void decl(std::ostream&, std::string) const = 0;
  virtual void encode(std::ostream&) const = 0;
  virtual const type* prev() const { return this; }
  virtual void post(std::ostream&) const {}
  virtual bool compatible(const type*) const;
  virtual const type* composite(const type*) const;
  virtual int size() const = 0;
  virtual int align() const;
  virtual bool scalar() const { return true; }
  virtual bool real() const { return false; }
  virtual bool integer() const { return scalar() && !real(); }
  bool arithmetic() const { return real() || integer(); }
  virtual bool modifiable() const { return true; }
  virtual const type* promotion() const { return this; }
  virtual const type* varg() const { return promotion(); }
  virtual bool _signed() const { return false; }
  virtual tag* get_tag() const { return 0; }
  virtual const type* complete_type() const { return this; }
  virtual const pointer_type* ptr_gen() const { return 0; }
  virtual const type* patch(const type* T, usr*) const { return T; }
  virtual bool backpatch() const { return false; }
  virtual const type* qualified(int) const;
  virtual std::pair<int, const type*> current(int) const;
  virtual const type* unqualified(int* cvr = 0) const { return this; }
  virtual bool aggregate() const { return false; }
  virtual bool tmp() const { return false; }
  virtual bool variably_modified() const { return false; }
  virtual const type* vla2a() const { return this; }
  virtual void decide_dim() const {}
  virtual var* vsize() const { return 0; }
  virtual ~type(){}
  static void destroy_tmp();
  static void collect_tmp(std::vector<const type*>&);
};

inline bool is_external_declaration(var* v)
{
  scope* s = v->m_scope;
  if ( !s->m_parent )
    return true;
  if ( s->m_id == scope::TAG ){
    const type* T = v->m_type;
    if ( T->m_id == type::FUNC )
      return true;
    if ( usr* u = v->usr_cast() ){
      usr::flag_t flag = u->m_flag;
      usr::flag_t mask = usr::flag_t(usr::STATIC | usr::STATIC_DEF);
      if ( flag & mask )
        return true;
    }
  }
  if ( s->m_id == scope::NAMESPACE )
    return true;
  return false;
}

class void_type : public type {
  static void_type obj;
  void_type() : type(VOID) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  int size() const { return 0; }
  bool scalar() const { return false; }
  static const void_type* create(){ return &obj; }
};

class char_type : public type {
  static char_type obj;
  char_type() : type(CHAR) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  int size() const { return 1; }
  const type* promotion() const;
  bool _signed() const { return true; }
  static const char_type* create(){ return &obj; }
};

class schar_type : public type {
  static schar_type obj;
  schar_type() : type(SCHAR) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  int size() const { return 1; }
  const type* promotion() const;
  bool _signed() const { return true; }
  static const schar_type* create(){ return &obj; }
};

class uchar_type : public type {
  static uchar_type obj;
  uchar_type() : type(UCHAR) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  int size() const { return 1; }
  const type* promotion() const;
  static const uchar_type* create(){ return &obj; }
};

class wchar_type : public type {
  static wchar_type obj;
  wchar_type() : type(WCHAR) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  int size() const;
  const type* promotion() const;
  bool _signed() const;
  static const wchar_type* create(){ return &obj; }
};

class bool_type : public type {
  static bool_type obj;
  bool_type() : type(BOOL) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  int size() const { return 1; }
  const type* promotion() const;
  static const bool_type* create(){ return &obj; }
};

class short_type : public type {
  static short_type obj;
  short_type() : type(SHORT) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  int size() const;
  const type* promotion() const;
  bool _signed() const { return true; }
  static const short_type* create(){ return &obj; }
};

class ushort_type : public type {
  static ushort_type obj;
  ushort_type() : type(USHORT) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  int size() const;
  const type* promotion() const;
  static const ushort_type* create(){ return &obj; }
};

class int_type : public type {
  static int_type obj;
  int_type() : type(INT) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  int size() const;
  bool _signed() const { return true; }
  static const int_type* create(){ return &obj; }
};

class uint_type : public type {
  static uint_type obj;
  uint_type() : type(UINT) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  int size() const;
  static const uint_type* create(){ return &obj; }
};

class long_type : public type {
  static long_type obj;
  long_type() : type(LONG) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  int size() const;
  bool _signed() const { return true; }
  static const long_type* create(){ return &obj; }
};

class ulong_type : public type {
  static ulong_type obj;
  ulong_type() : type(ULONG) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  int size() const;
  static const ulong_type* create(){ return &obj; }
};

class long_long_type : public type {
  static long_long_type obj;
  long_long_type() : type(LONGLONG) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  int size() const;
  bool _signed() const { return true; }
  static const long_long_type* create(){ return &obj; }
};

class ulong_long_type : public type {
  static ulong_long_type obj;
  ulong_long_type() : type(ULONGLONG) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  int size() const;
  static const ulong_long_type* create(){ return &obj; }
};

class float_type : public type {
  static float_type obj;
  float_type() : type(FLOAT) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  int size() const;
  bool real() const { return true; }
  const type* varg() const;
  static const float_type* create(){ return &obj; }
};

class double_type : public type {
  static double_type obj;
  double_type() : type(DOUBLE) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  int size() const;
  bool real() const { return true; }
  static const double_type* create(){ return &obj; }
};

class long_double_type : public type {
  static long_double_type obj;
  long_double_type() : type(LONG_DOUBLE) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  int size() const;
  bool real() const { return true; }
  static const long_double_type* create(){ return &obj; }
};

class backpatch_type : public type {
  static backpatch_type obj;
  backpatch_type() : type(BACKPATCH) {}
public:
  void decl(std::ostream&, std::string) const { assert(0); }
  void encode(std::ostream&) const { assert(0); }
  const type* patch(const type* T, usr*) const { return T; }
  int size() const { return 0; }
  bool scalar() const { return false; }
  bool backpatch() const { return true; }
  static const backpatch_type* create(){ return &obj; }
};

class volatile_type;
class restrict_type;

class const_type : public type {
  typedef std::map<const type*, const const_type*> table_t;
  static table_t tmp_tbl, pmt_tbl;
  const type* m_T;
  const_type(const type* T) : type(CONST), m_T(T) {}
  friend class volatile_type;
  friend class restrict_type;
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  bool compatible(const type* T) const;
  const type* composite(const type* T) const;
  int size() const { return m_T->size(); }
  bool scalar() const { return m_T->scalar(); }
  bool real() const { return m_T->real(); }
  bool integer() const { return m_T->integer(); }
  const type* complete_type() const { return create(m_T->complete_type()); }
  const type* unqualified(int* cvr) const { if ( cvr ) *cvr |= 1; return m_T->unqualified(cvr); }
  const type* patch(const type*, usr*) const;
  bool modifiable() const { return false; }
  const type* promotion() const { return create(m_T->promotion()); }
  const type* varg() const { return create(m_T->varg()); }
  bool _signed() const { return m_T->_signed(); }
  std::pair<int, const type*> current(int nth) const { return m_T->current(nth); }
  bool aggregate() const { return m_T->aggregate(); }
  bool tmp() const { return m_T->tmp(); }
  const type* qualified(int) const;
  bool variably_modified() const { return m_T->variably_modified(); }
  const type* vla2a() const { return create(m_T->vla2a()); }
  void decide_dim() const { m_T->decide_dim(); }
  tag* get_tag() const { return m_T->get_tag(); }
  static const type* create(const type*);
  static void destroy_tmp();
  static void collect_tmp(std::vector<const type*>&);
};

class volatile_type : public type {
  typedef std::map<const type*, const volatile_type*> table_t;
  static table_t tmp_tbl, pmt_tbl;
  const type* m_T;
  volatile_type(const type* T) : type(VOLATILE), m_T(T) {}
  friend class restrict_type;
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  bool compatible(const type* T) const;
  const type* composite(const type* T) const;
  const type* patch(const type*, usr*) const;
  int size() const { return m_T->size(); }
  bool scalar() const { return m_T->scalar(); }
  bool real() const { return m_T->real(); }
  bool integer() const { return m_T->integer(); }
  const type* complete_type() const { return create(m_T->complete_type()); }
  const type* unqualified(int* cvr) const { if ( cvr ) *cvr |= 2; return m_T->unqualified(cvr); }
  bool modifiable() const { return m_T->modifiable(); }
  const type* promotion() const { return create(m_T->promotion()); }
  const type* varg() const { return create(m_T->varg()); }
  bool _signed() const { return m_T->_signed(); }
  std::pair<int, const type*> current(int nth) const { return m_T->current(nth); }
  bool aggregate() const { return m_T->aggregate(); }
  bool tmp() const { return m_T->tmp(); }
  bool variably_modified() const { return m_T->variably_modified(); }
  const type* vla2a() const { return create(m_T->vla2a()); }
  void decide_dim() const { m_T->decide_dim(); }  
  const type* qualified(int) const;
  tag* get_tag() const { return m_T->get_tag(); }
  static const type* create(const type*);
  static void destroy_tmp();
  static void collect_tmp(std::vector<const type*>&);
};

class restrict_type : public type {
  typedef std::map<const type*, const restrict_type*> table_t;
  static table_t tmp_tbl, pmt_tbl;
  const type* m_T;
  restrict_type(const type* T) : type(RESTRICT), m_T(T) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  const type* patch(const type*, usr*) const;
  bool compatible(const type* T) const;
  const type* composite(const type* T) const;
  int size() const { return m_T->size(); }
  bool scalar() const { return m_T->scalar(); }
  bool integer() const { return m_T->integer(); }
  bool real() const { return m_T->real(); }
  const type* complete_type() const { return create(m_T->complete_type()); }
  const type* unqualified(int* cvr) const { if ( cvr ) *cvr |= 4; return m_T->unqualified(cvr); }
  bool modifiable() const { return m_T->modifiable(); }
  const type* promotion() const { return create(m_T->promotion()); }
  const type* varg() const { return create(m_T->varg()); }
  bool _signed() const { return m_T->_signed(); }
  std::pair<int, const type*> current(int nth) const { return m_T->current(nth); }
  bool aggregate() const { return m_T->aggregate(); }
  bool tmp() const { return m_T->tmp(); }
  bool variably_modified() const { return m_T->variably_modified(); }
  const type* vla2a() const { return create(m_T->vla2a()); }
  void decide_dim() const { m_T->decide_dim(); }  
  const type* qualified(int) const;
  tag* get_tag() const { return m_T->get_tag(); }
  static const type* create(const type*);
  static void destroy_tmp();
  static void collect_tmp(std::vector<const type*>&);
};

class func_type : public type {
  struct table_t;
  static table_t tmp_tbl, pmt_tbl;
  const type* m_T;
  std::vector<const type*> m_param;
  func_type(const type* T, const std::vector<const type*>& param) : type(FUNC), m_T(T), m_param(param) {}
public:
  void decl(std::ostream& os, std::string name) const;
  void encode(std::ostream&) const;
  bool compatible(const type*) const;
  const type* composite(const type*) const;
  int size() const { return 0; }
  bool scalar() const { return false; }
  const type* prev() const { return m_T->prev(); }
  void post(std::ostream&) const;
  const type* patch(const type*, usr*) const;
  const type* qualified(int) const;
  const type* complete_type() const;
  const pointer_type* ptr_gen() const;
  const type* return_type() const { return m_T; }
  const std::vector<const type*>& param() const { return m_param; }
  bool tmp() const;
  bool variably_modified() const;
  const type* vla2a() const;
  void decide_dim() const;
  bool overloadable(const func_type*) const;
  static const func_type* create(const type*, const std::vector<const type*>&);
  static void destroy_tmp();
  static void collect_tmp(std::vector<const type*>&);
};

class array_type : public type {
  struct table_t;
  static table_t tmp_tbl, pmt_tbl;
  const type* m_T;
  int m_dim;;
  array_type(const type* T, int dim) : type(ARRAY), m_T(T), m_dim(dim) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  bool compatible(const type*) const;
  const type* composite(const type*) const;
  int size() const { return m_T->size() * m_dim; }
  int align() const { return m_T->align(); }
  bool scalar() const { return false; }
  bool modifiable() const { return m_T->modifiable(); }
  const type* patch(const type*, usr*) const;
  const type* qualified(int) const;
  const type* complete_type() const;
  const type* prev() const { return m_T->prev(); }
  void post(std::ostream&) const;
  const pointer_type* ptr_gen() const;
  const type* element_type() const { return m_T; }
  int dim() const { return m_dim; }
  std::pair<int, const type*> current(int) const;
  bool aggregate() const { return true; }
  bool tmp() const { return m_T->tmp(); }
  bool variably_modified() const { return m_T->variably_modified(); }
  const type* vla2a() const { return create(m_T->vla2a(), m_dim); }
  void decide_dim() const { m_T->decide_dim(); }
  var* vsize() const;
  static const array_type* create(const type*, int);
  static void destroy_tmp();
  static void collect_tmp(std::vector<const type*>&);
};

class pointer_type : public type {
  typedef std::map<const type*, const pointer_type*> table_t;
  static table_t tmp_tbl, pmt_tbl;
  const type* m_T;
  pointer_type(const type* T) : type(POINTER), m_T(T) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  bool compatible(const type*) const;
  const type* composite(const type*) const;
  const type* patch(const type*, usr*) const;
  const type* referenced_type() const { return m_T; }
  int size() const;
  bool integer() const { return false; }
  const type* complete_type() const;
  bool tmp() const { return m_T->tmp(); }
  bool variably_modified() const { return m_T->variably_modified(); }
  const type* vla2a() const { return create(m_T->vla2a()); }
  void decide_dim() const { m_T->decide_dim(); }
  static const pointer_type* create(const type*);
  static void destroy_tmp();
  static void collect_tmp(std::vector<const type*>&);
};

class reference_type : public type {
  typedef std::map<const type*, const reference_type*> table_t;
  static table_t tmp_tbl, pmt_tbl;
  const type* m_T;
  reference_type(const type* T) : type(REFERENCE), m_T(T) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  bool compatible(const type*) const;
  const type* composite(const type*) const;
  const type* patch(const type*, usr*) const;
  const type* referenced_type() const { return m_T; }
  int size() const;
  bool integer() const { return false; }
  const type* complete_type() const;
  bool tmp() const { return m_T->tmp(); }
  static const reference_type* create(const type*);
  static void destroy_tmp();
  static void collect_tmp(std::vector<const type*>&);
};

class ellipsis_type : public type {
  static ellipsis_type m_obj;
  ellipsis_type() : type(ELLIPSIS) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  int size() const { return 0; }
  bool scalar() const { return false; }
  static const ellipsis_type* create();
};

class incomplete_tagged_type : public type {
  tag* m_tag;
  typedef std::set<const incomplete_tagged_type*> table_t;
  static table_t tmp_tbl;
  incomplete_tagged_type(tag* tag) : type(INCOMPLETE_TAGGED), m_tag(tag) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  bool compatible(const type*) const;
  const type* composite(const type*) const;
  int size() const { return 0; }
  bool scalar() const { return false; }
  tag* get_tag() const { return m_tag; }
  const type* complete_type() const;
  bool tmp() const;
  ~incomplete_tagged_type(){ delete m_tag; }  
  static const incomplete_tagged_type* create(tag*);
  static void destroy_tmp();
  static void collect_tmp(std::vector<const type*>&);
};

class record_type : public type {
  std::vector<usr*> m_member;
  std::map<std::string, std::pair<int, usr*> > m_layout;
  std::map<usr*, int> m_position;
  int m_size;
  bool m_modifiable;
  tag* m_tag;
  typedef std::set<const record_type*> table_t;
  static table_t tmp_tbl;
  
  with_initial* m_vbtbl;
  std::vector<base*> m_vbtbl_contents;
  with_initial* m_vftbl;
  std::vector<usr*> m_vftbl_contents;
  record_type(tag*);
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  bool compatible(const type*) const;
  const type* composite(const type*) const;
  int size() const { return m_size; }
  bool scalar() const { return false; }
  bool modifiable() const { return m_modifiable; }
  std::pair<int, usr*> offset(std::string) const;
  int position(usr*) const;
  std::pair<int, const type*> current(int) const;
  const std::vector<usr*> member() const { return m_member; }
  tag* get_tag() const { return m_tag; }
  bool aggregate() const { return true; }
  bool tmp() const;
  usr* vbtbl() const { return m_vbtbl; }
  const std::vector<base*>& vbtbl_contents() const { return m_vbtbl_contents; }
  usr* vftbl() const { return m_vftbl; }
  const std::vector<usr*>& vftbl_contents() const { return m_vftbl_contents; }
  static const record_type* create(tag*);
  static void destroy_tmp();
  static void collect_tmp(std::vector<const type*>&);
};

class enum_type : public type {
  tag* m_tag;
  const type* m_integer;
  typedef std::set<const enum_type*> table_t;
  static table_t tmp_tbl;
  enum_type(tag* tag, const type* integer) : type(ENUM), m_tag(tag), m_integer(integer) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  bool compatible(const type*) const;
  const type* composite(const type*) const;
  int size() const { return m_integer->size(); }
  bool _signed() const { return m_integer->_signed(); }
  tag* get_tag() const { return m_tag; }
  bool tmp() const;
  const type* get_integer() const { return m_integer; }  
  static const enum_type* create(tag*, const type*);
  static void destroy_tmp();
  static void collect_tmp(std::vector<const type*>&);
};

class bit_field_type : public type {
  struct table_t;
  static table_t table;
  int m_bit;
  const type* m_integer;
  bit_field_type(int bit, const type* integer) : type(BIT_FIELD), m_bit(bit), m_integer(integer) {}
public:
  const type* integer_type() const { return m_integer; }
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const { assert(0); }
  int size() const { return 0; }
  bool _signed() const { return m_integer->_signed(); }
  const type* patch(const type*, usr*) const;
  int bit() const { return m_bit; }
  static const bit_field_type* create(int, const type*);
};

class varray_type : public type {
  const type* m_T;
  var* m_dim;
  struct table_t;
  static table_t table;
  varray_type(const type* T, var* dim) : type(VARRAY), m_T(T), m_dim(dim) {}
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  bool compatible(const type*) const;
  const type* composite(const type*) const;
  int size() const { return 0; }
  int align() const { return m_T->align(); }
  bool scalar() const { return false; }
  bool modifiable() const { return m_T->modifiable(); }
  const type* patch(const type*, usr*) const;
  const type* qualified(int) const;
  const type* prev() const { return m_T->prev(); }
  void post(std::ostream&) const;
  const type* complete_type() const;
  const pointer_type* ptr_gen() const;
  const type* element_type() const { return m_T; }
  bool aggregate() const { return true; }
  bool tmp() const { return true; }
  bool variably_modified() const { return true; }
  const type* vla2a() const { return array_type::create(m_T->vla2a(), 0); }
  void decide_dim() const;
  var* vsize() const;
  static const varray_type* create(const type*, var*);
  static void destroy_tmp();
  static void collect_tmp(std::vector<const type*>&);
};

class pointer_member_type : public type {
  const tag* m_tag;
  const type* m_T;
  pointer_member_type(const tag* tag, const type* T)
    : type(POINTER_MEMBER), m_tag(tag), m_T(T) {}
  typedef std::map<std::pair<const tag*, const type*>, const pointer_member_type*> table_t;
  static table_t tmp_tbl, pmt_tbl;
public:
  void decl(std::ostream&, std::string) const;
  void encode(std::ostream&) const;
  bool integer() const { return false; }
  bool real() const { return false; }
  bool scalar() const
  {
    return m_T->m_id != type::FUNC;
  }
  int size() const
  {
    return m_T->m_id == type::FUNC ? 2 * sizeof(int) + sizeof(void*) : sizeof(int);
  }
  const type* backpatch(const type*) const;
  bool compatible(const type*) const;
  const type* composite(const type*) const;
  bool include_qualifier(const pointer_member_type*) const;
  static const pointer_member_type* create(const tag*, const type*);
  static void destroy_tmp();
  static void collect_tmp(std::vector<const type*>&);
};

struct fundef {
  usr* m_usr;
  scope* m_param;
  fundef(usr* u, scope* param)
    : m_usr(u), m_param(param) {}
  static fundef* current;
};

struct type;

struct tac {
  enum id_t {
    NONE,
    ASSIGN,
    ADD, SUB, MUL, DIV, MOD, LSH, RSH, AND, OR, XOR,
    UMINUS, TILDE, CAST,
    PARAM, CALL, RETURN,
    GOTO, TO,
    ADDR, INVLADDR, INVRADDR, LOFF, ROFF,
    ALLOCA,
    ASM,
    VASTART, VAARG, VAEND
  };
  id_t m_id;
  var* x;
  var* y;
  var* z;
  file_t m_file;
  tac(id_t, var*, var*, var*);
  virtual tac* new3ac() = 0;
  virtual ~tac(){}
};

struct assign3ac : tac {
  assign3ac(var* x, var* y) : tac(ASSIGN,x,y,0) {}
  tac* new3ac(){ return new assign3ac(*this); }
};

struct mul3ac : tac {
  mul3ac(var* x, var* y, var* z) : tac(MUL,x,y,z) {}
  tac* new3ac(){ return new mul3ac(*this); }
};

struct div3ac : tac {
  div3ac(var* x, var* y, var* z) : tac(DIV,x,y,z) {}
  tac* new3ac(){ return new div3ac(*this); }
};

struct mod3ac : tac {
  mod3ac(var* x, var* y, var* z) : tac(MOD,x,y,z) {}
  tac* new3ac(){ return new mod3ac(*this); }
};

struct add3ac : tac {
  add3ac(var* x, var* y, var* z) : tac(ADD,x,y,z) {}
  tac* new3ac(){ return new add3ac(*this); }
};

struct sub3ac : tac {
  sub3ac(var* x, var* y, var* z) : tac(SUB,x,y,z) {}
  tac* new3ac(){ return new sub3ac(*this); }
};

struct lsh3ac : tac {
  lsh3ac(var* x, var* y, var* z) : tac(LSH,x,y,z) {}
  tac* new3ac(){ return new lsh3ac(*this); }
};

struct rsh3ac : tac {
  rsh3ac(var* x, var* y, var* z) : tac(RSH,x,y,z) {}
  tac* new3ac(){ return new rsh3ac(*this); }
};

struct and3ac : tac {
  and3ac(var* x, var* y, var* z) : tac(AND,x,y,z) {}
  tac* new3ac(){ return new and3ac(*this); }
};

struct or3ac : tac {
  or3ac(var* x, var* y, var* z) : tac(OR,x,y,z) {}
  tac* new3ac(){ return new or3ac(*this); }
};

struct xor3ac : tac {
  xor3ac(var* x, var* y, var* z) : tac(XOR,x,y,z) {}
  tac* new3ac(){ return new xor3ac(*this); }
};

struct param3ac : tac {
  param3ac(var* y) : tac(PARAM,0,y,0) {}
  tac* new3ac(){ return new param3ac(*this); }
};

struct call3ac : tac {
  call3ac(var* x, var* y) : tac(CALL,x,y,0) {}
  tac* new3ac(){ return new call3ac(*this); }
};

struct return3ac : tac {
  return3ac(var* y) : tac(RETURN,0,y,0) {}
  tac* new3ac(){ return new return3ac(*this); }
};

struct addr3ac : tac {
  addr3ac(var* x, var* y) : tac(ADDR,x,y,0) {}
  tac* new3ac(){ return new addr3ac(*this); }
};

struct invraddr3ac : tac {
  invraddr3ac(var* x, var* y) : tac(INVRADDR,x,y,0) {}
  tac* new3ac(){ return new invraddr3ac(*this); }
};

struct invladdr3ac : tac {
  invladdr3ac(var* y, var* z) : tac(INVLADDR,0,y,z) {}
  tac* new3ac(){ return new invladdr3ac(*this); }
};

struct uminus3ac : tac {
  uminus3ac(var* x, var* y) : tac(UMINUS,x,y,0) {}
  tac* new3ac(){ return new uminus3ac(*this); }
};

struct tilde3ac : tac {
  tilde3ac(var* x, var* y) : tac(TILDE,x,y,0) {}
  tac* new3ac(){ return new tilde3ac(*this); }
};

struct cast3ac : tac {
  const type* m_type;
  cast3ac(var* x, var* y, const type* type) : tac(CAST,x,y,0), m_type(type) {}
  tac* new3ac(){ return new cast3ac(*this); }
};

struct goto3ac : tac {
  to3ac* m_to;
  enum op { NONE, EQ, NE, LE, GE, LT, GT };
  op m_op;
  goto3ac() : tac(GOTO,0,0,0), m_to(0), m_op(NONE) {}
  goto3ac(op op, var* y, var* z) : tac(GOTO,0,y,z), m_to(0), m_op(op) {}
  tac* new3ac(){ return new goto3ac(*this); }
};

struct to3ac : tac {
  std::vector<goto3ac*> m_goto;
  to3ac() : tac(TO,0,0,0) {}
  tac* new3ac(){ return new to3ac(*this); }
};

struct loff3ac : tac {
  loff3ac(var* x, var* y, var* z) : tac(LOFF,x,y,z) {}
  tac* new3ac(){ return new loff3ac(*this); }
};

struct roff3ac : tac {
  roff3ac(var* x, var* y, var* z) : tac(ROFF,x,y,z) {}
  tac* new3ac(){ return new roff3ac(*this); }
};

struct alloca3ac : tac {
  alloca3ac(var* x, var* y) : tac(ALLOCA,x,y,0) {}
  tac* new3ac(){ return new alloca3ac(*this); }
};

struct asm3ac : tac {
  std::string m_inst;
  asm3ac(std::string inst) : tac(ASM,0,0,0), m_inst(inst) {}
  tac* new3ac(){ return new asm3ac(*this); }
};

struct va_start3ac : tac {
  va_start3ac(var* x, var* y) : tac(VASTART,x,y,0) {}
  tac* new3ac(){ return new va_start3ac(*this); }
};

struct va_arg3ac : tac {
  const type* m_type;
  va_arg3ac(var* x, var* y, const type* type) : tac(VAARG,x,y,0), m_type(type) {}
  tac* new3ac(){ return new va_arg3ac(*this); }
};

struct va_end3ac : tac {
  va_end3ac(var* y) : tac(VAEND, 0, y, 0) {}
  tac* new3ac() { return new va_end3ac(*this); }
};

namespace generator {
  using namespace std;
  struct interface_t {
    const scope* m_root;
    const fundef* m_func;
    const vector<tac*>* m_code;
  };
  struct long_double_t {
    void (*bit)(unsigned char*, const char*);
    void (*add)(unsigned char*, const unsigned char*);
    void (*sub)(unsigned char*, const unsigned char*);
    void (*mul)(unsigned char*, const unsigned char*);
    void (*div)(unsigned char*, const unsigned char*);
    void (*minus)(unsigned char*, const unsigned char*);
    bool (*zero)(const unsigned char*);
    double (*to_double)(const unsigned char*);
    void (*from_double)(unsigned char*, double);
    bool (*cmp)(goto3ac::op, const unsigned char*, const unsigned char*);
  };
  struct last_interface_t {
    const scope* m_root;
    const vector<pair<const fundef*, vector<tac*> > >* m_funcs;
  };
} // end of namespace generator

inline bool is_string(std::string name)
{
  if ( name[0] == 'L' )
    return name.length() > 1 && name[1] == '"';
  else
    return name[0] == '"';
}

namespace optimize {
  namespace basic_block {
    using namespace std;
    struct info_t {
      tac** m_leader;
      int m_size;
      vector<info_t*> m_follow, m_preceed;
      info_t(tac** leader) : m_leader(leader), m_size(0) {}
    };
  } // end of namespace basic_block
} // end of namespace optimize
 
} // end of namespace cxx_compiler

#endif // _CXX_CORE_H_
