%{
#ifdef _MSC_VER
#define YY_NO_UNISTD_H
#define isatty _isatty
#define fileno _fileno
extern "C" int isatty(int);
#endif // _MSC_VER

#include "cxx_core.h"
#include "cxx_impl.h"
#include "cxx_y.h"

namespace cxx_compiler { namespace parse {

void line_control(bool ms)
{
  using namespace std;
  istringstream is(yytext);
  char sharpe;
  is >> sharpe;
  if (ms) {
    string line;
    is >> line;
    assert(line == "line");
  }
  is >> position.m_lineno;
  --position.m_lineno;
  if (ms) {
    char dq = '\0';
    is >> dq;
    if (dq == '"') {
      string fn;
      char c;
      while (is.get(c)) {
        if (c == dq)
          break;
        if (c == '\\') {
          is.get(c);
          assert(c == '\\');
        }
        fn += c;
      }
      position.m_name = fn;
    }
  }
  else {
    std::string fn;
    is >> fn;
    if ( !fn.empty() )
      position.m_name = fn.substr(1,fn.size()-2);
  }
}

} } // end of namespace parse and cxx_compiler

%}

hex-quad               {hexadecimal-digit}{hexadecimal-digit}{hexadecimal-digit}{hexadecimal-digit}

universal-character-name \\u{hex-quad}|\\U{hex-quad}{hex-quad}

identifier             {nondigit}({nondigit}|{digit})*
nondigit               {universal-character-name}|[_a-zA-Z]
digit                  [0-9]

integer-literal        {decimal-literal}{integer-suffix}?|{octal-literal}{integer-suffix}?|{hexadecimal-literal}{integer-suffix}?
decimal-literal        {nonzero-digit}{digit}*
octal-literal          0{octal-digit}*
hexadecimal-literal    0[xX]{hexadecimal-digit}+
nonzero-digit          [1-9]
octal-digit            [0-7]
hexadecimal-digit      [0-9a-fA-F]
integer-suffix         {unsigned-suffix}{long-suffix}?|{long-suffix}{unsigned-suffix}?|{unsigned-suffix}{long-long-suffix}?|{long-long-suffix}{unsigned-suffix}?
unsigned-suffix        [uU]
long-suffix            [lL]
long-long-suffix       ll|LL

character-literal              L?'{c-char-sequence}'
c-char-sequence                {c-char}+|{shift-jis-char}|{jis-char}
c-char                         [^'\\\n\r]|{escape-sequence}|{universal-character-name}
escape-sequence                {simple-escape-sequence}|{octal-escape-sequnce}|{hexadecimal-escape-sequence}
simple-escape-sequence         \\'|\\\"|\\\?|\\\\|\\a|\\b|\\f|\\n|\\r|\\t|\\v
octal-escape-sequnce           \\{octal-digit}{1,3}
hexadecimal-escape-sequence    \\x{hexadecimal-digit}+

floating-literal  {decimal-floating-literal}|{hexadecimal-floating-literal}
decimal-floating-literal  {fractional-constant}{exponent-part}?{floating-suffix}?|{digit-sequence}{exponent-part}{floating-suffix}?
hexadecimal-floating-literal  0[xX]({hexadecimal-fractional-constant}|{hexadecimal-digit-sequence}){binary-exponent-part}{floating-suffix}?
fractional-constant  {digit-sequence}?\.{digit-sequence}|{digit-sequence}\.
exponent-part  [eE]{sign}?{digit-sequence}
sign  [+-]
digit-sequence  {digit}+
hexadecimal-fractional-constant  ({hexadecimal-digit-sequence}?\.{hexadecimal-digit-sequence})|({hexadecimal-digit-sequence}\.)
binary-exponent-part  [pP]{sign}?{digit-sequence}
hexadecimal-digit-sequence  {hexadecimal-digit}+
floating-suffix  [flFL]

string-literal  L?\"{s-char-sequence}?\"
s-char-sequence  {s-char}+|{shift-jis-char}|{jis-char}
shift-jis-char   [\x81-\x9F\xE0-\xFC][\x40-\x7E\x80-\xFC]
jis-kanji-start-old     \x1b\x24\x40
jis-kanji-start         \x1b\x24\x42
jis-kanji-start2        \x1b\x26\x40\x1b\x24\x42
jis-ascii-start         \x1b\x28\x42
jis-roman-start         \x1b\x28\x4a
jis-hankaku-kana-start  \x1b\x28\x49
jis-2byte               [\x21-\x7e][\x21-\x7e]
jis-p1                  {jis-kanji-start-old}{jis-2byte}+{jis-ascii-start}
jis-p2                  {jis-kanji-start}{jis-2byte}+{jis-ascii-start}
jis-p3                  {jis-kanji-start2}{jis-2byte}+{jis-ascii-start}
jis-p4                  {jis-roman-start}{jis-2byte}+{jis-ascii-start}
jis-hankaku-kana        [\x21-\x5f\xa1-\xdf]
jis-p5           {jis-hankaku-kana-start}{jis-hankaku-kana}+{jis-ascii-start}
jis-char                {jis-p1}|{jis-p2}|{jis-p3}|{jis-p4}|{jis-p5}
s-char  [^\"\\\n\r]|{escape-sequence}|{universal-character-name}|{shift-jis-char}|{jis-char}

%%

__alignof__         { return ALIGNOF_KW; }
__builtin_va_arg    { return BUILTIN_VA_ARG; }
__builtin_va_end    { return BUILTIN_VA_END; }
__builtin_va_start  { return BUILTIN_VA_START; }
__builtin_addressof { return BUILTIN_ADDRESSOF; }
__builtin_constant_p  { return BUILTIN_CONSTANT_P; }
__builtin_clz       { return BUILTIN_CLZ; }
__builtin_clzl      { return BUILTIN_CLZL; }
__builtin_clzll     { return BUILTIN_CLZLL; }
__is_base_of        { return BUILTIN_IS_BASE_OF; }
__is_class          { return BUILTIN_IS_CLASS; }
__is_enum           { return BUILTIN_IS_ENUM; }
__is_union          { return BUILTIN_IS_UNION; }
__is_same_as        { return BUILTIN_IS_SAME_AS; }
__is_trivial        { return BUILTIN_IS_TRIVIAL; }
__is_trivially_copyable     { return BUILTIN_IS_TRIVIALLY_COPYABLE; }
__is_standard_layout        { return BUILTIN_IS_STD_LAYOUT; }
__is_pod            { return BUILTIN_IS_POD; }
__is_literal_type   { return BUILTIN_IS_LITERAL_TYPE; }
__is_empty          { return BUILTIN_IS_EMPTY; }
__is_polymorphic    { return BUILTIN_IS_POLYMORPHIC; }
__is_final          { return BUILTIN_IS_FINAL; }
__is_abstract       { return BUILTIN_IS_ABSTRACT; }
__is_trivially_assignable { return BUILTIN_IS_TRIVIALLY_ASSIGNABLE; }
__has_trivial_destructor { return BUILTIN_HAS_TRIVIAL_DESTRUCTOR; }
__is_constructible  { return BUILTIN_IS_CONSTRUCTIBLE; }
__is_assignable     { return BUILTIN_IS_ASSIGNABLE; }
__is_trivially_constructible { return BUILTIN_IS_TRIVIALLY_CONSTRUCTIBLE; }
__has_virtual_destructor { return BUILTIN_HAS_VIRTUAL_DESTRUCTOR; }
__underlying_type   { return BUILTIN_UNDERLYING_TYPE; }
alignof             { return ALIGNOF_KW; }
asm                 { return ASM_KW; }
auto                { return AUTO_KW; }
bool                { return BOOL_KW; }
break               { return BREAK_KW; }
case                { return CASE_KW; }
catch               { return CATCH_KW; }
char                { return CHAR_KW; }
char16_t            { return CHAR16_T_KW; }
char32_t            { return CHAR32_T_KW; }
class               { return CLASS_KW; }
const_cast          { return CONST_CAST_KW; }
const               { return CONST_KW; }
constexpr           { return CONSTEXPR_KW; }
continue            { return CONTINUE_KW; }
decltype            { return DECLTYPE_KW; }
default             { cxx_compiler_lval.m_file = new cxx_compiler::file_t(cxx_compiler::parse::position); return DEFAULT_KW; }
delete              { return DELETE_KW; }
double              { return DOUBLE_KW; }
do                  { return DO_KW; }
dynamic_cast        { return DYNAMIC_CAST_KW; }
else                { return ELSE_KW; }
enum                { return ENUM_KW; }
explicit            { return EXPLICIT_KW; }
export              { return EXPORT_KW; }
extern              { return EXTERN_KW; }
false               { return FALSE_KW; }
float               { return FLOAT_KW; }
__float128          { return FLOAT128_KW; }
for                 { return FOR_KW; }
friend              { return FRIEND_KW; }
goto                { return GOTO_KW; }
if                  { return IF_KW; }
inline              { return INLINE_KW; }
int                 { return INT_KW; }
long                { return LONG_KW; }
mutable             { return MUTABLE_KW; }
namespace           { return NAMESPACE_KW; }
new                 { return NEW_KW; }
noexcept            { return NOEXCEPT_KW; }
nullptr             { return NULLPTR_KW; }
operator            { return OPERATOR_KW; }
private             { return PRIVATE_KW; }
protected           { return PROTECTED_KW; }
public              { return PUBLIC_KW; }
register            { return REGISTER_KW; }
reinterpret_cast    { return REINTERPRET_CAST_KW; }
restrict            { return RESTRICT_KW; }
return              { return RETURN_KW; }
short               { return SHORT_KW; }
signed              { return SIGNED_KW; }
sizeof              { return SIZEOF_KW; }
static_assert       { return STATIC_ASSERT_KW; }
static_cast         { return STATIC_CAST_KW; }
static              { return STATIC_KW; }
struct              { return STRUCT_KW; }
switch              { return SWITCH_KW; }
template            { return TEMPLATE_KW; }
this                { return THIS_KW; }
throw               { return THROW_KW; }
true                { return TRUE_KW; }
try                 { return TRY_KW; }
typedef             { return TYPEDEF_KW; }
typeid              { return TYPEID_KW; }
typename            { return TYPENAME_KW; }
union               { return UNION_KW; }
unsigned            { return UNSIGNED_KW; }
using               { return USING_KW; }
virtual             { return VIRTUAL_KW; }
void                { return VOID_KW; }
volatile            { return VOLATILE_KW; }
wchar_t             { return WCHAR_T_KW; }
while               { return WHILE_KW; }

"!="      { return NOTEQ_MK; }
"%="      { return MOD_ASSIGN_MK; }
"&&"      { return ANDAND_MK; }
"&="      { return AND_ASSIGN_MK; }
"*="      { return MUL_ASSIGN_MK; }
"++"      { return PLUSPLUS_MK; }
"+="      { return ADD_ASSIGN_MK; }
"--"      { return MINUSMINUS_MK; }
"-="      { return SUB_ASSIGN_MK; }
"->"      { return ARROW_MK; }
"->*"     { return ARROWASTER_MK; }
".*"      { return DOTASTER_MK; }
"..."     { return DOTS_MK; } 
"/="      { return DIV_ASSIGN_MK; }
"::"      { return COLONCOLON_MK; }
"<<"      { return LSH_MK; }
"<<="     { return LSH_ASSIGN_MK; }
"<="      { return LESSEQ_MK; }
"=="      { return EQUAL_MK; }
">="      { return GREATEREQ_MK; }
">>"      { return RSH_MK; }
">>="     { return RSH_ASSIGN_MK; }
"^="      { return XOR_ASSIGN_MK; }
"|="      { return OR_ASSIGN_MK; }
"||"      { return OROR_MK; }
"<:"      { return '['; }
":>"      { return ']'; }
"<%"      { return '{'; }
"%>"      { return '}'; }

#                                  {}
#[ \t]*pragma[ \t]*.*              {}
#[ \t]*{digit}+([ \t]+\"[^\"]+\")?([ \t]*{digit})* { cxx_compiler::parse::line_control(false); }
#[ \t]*line[ \t]*{digit}+([ \t]+\"[^\"]+\")?       { cxx_compiler::parse::line_control(true); }

{identifier}               { return cxx_compiler::parse::identifier::judge(cxx_compiler::ucn::conv(yytext)); }
{integer-literal}          { cxx_compiler_lval.m_usr = cxx_compiler::expressions::primary::literal::integer::create(yytext); return INTEGER_LITERAL_LEX; }
{character-literal}        { cxx_compiler_lval.m_usr = cxx_compiler::expressions::primary::literal::character::create(yytext); return CHARACTER_LITERAL_LEX; }
{floating-literal}         { cxx_compiler_lval.m_usr = cxx_compiler::expressions::primary::literal::floating::create(yytext); return FLOATING_LITERAL_LEX; }
{string-literal}           { cxx_compiler_lval.m_var = cxx_compiler::expressions::primary::literal::stringa::create(cxx_compiler::ucn::conv(yytext)); return STRING_LITERAL_LEX; }
[ \t]    {}
\n|\r\n  { ++cxx_compiler::parse::position.m_lineno; }
.        { return yytext[0]; }

%%

#ifdef _DEBUG
void cxx_compiler::parse::delete_buffer()
{
  yy_delete_buffer(YY_CURRENT_BUFFER);
}
#endif // _DEBUG

int yywrap(){ return 1; }
