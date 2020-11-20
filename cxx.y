%{
#include "cxx_core.h"
#include "cxx_impl.h"

namespace cxx_compiler {
  // work around. sytax error may causes invalid scope::current.
  inline void neaten()
  {
    if (error::counter) {
      while (scope::current->m_id == scope::BLOCK ||
             scope::current->m_id == scope::PARAM ||
             scope::current->m_id == scope::TAG) {
        scope::current = scope::current->m_parent;
      }
    }
    using namespace class_or_namespace_name;
    typedef vector<scope*>::iterator IT;
    IT p = find(begin(before), end(before), scope::current);
    if (p != end(before))
      before.erase(p+1, end(before));
  }
} // end of namespace cxx_compiler
%}

%token ORIGINAL_NAMESPACE_NAME_LEX NAMESPACE_ALIAS_LEX IDENTIFIER_LEX
%token PEEKED_NAME_LEX
%token INTEGER_LITERAL_LEX CHARACTER_LITERAL_LEX FLOATING_LITERAL_LEX
%token STRING_LITERAL_LEX
%token TYPEDEF_KW AUTO_KW REGISTER_KW STATIC_KW EXTERN_KW MUTABLE_KW
%token INLINE_KW VIRTUAL_KW EXPLICIT_KW FRIEND_KW

%token VOID_KW CHAR_KW WCHAR_T_KW BOOL_KW INT_KW FLOAT_KW DOUBLE_KW
%token SHORT_KW LONG_KW
%token SIGNED_KW UNSIGNED_KW
%token TYPEDEF_NAME_LEX CLASS_NAME_LEX TEMPLATE_NAME_LEX ENUM_NAME_LEX

%token CONST_KW VOLATILE_KW RESTRICT_KW
%token COLONCOLON_MK
%token TEMPLATE_KW
%token ENUM_KW
%token TYPENAME_KW
%token NAMESPACE_KW

%token USING_KW
%token ASM_KW
%token DOTS_MK

%token CLASS_KW STRUCT_KW UNION_KW
%token PRIVATE_KW PROTECTED_KW PUBLIC_KW
%token EXPORT_KW

%token MUL_ASSIGN_MK DIV_ASSIGN_MK MOD_ASSIGN_MK ADD_ASSIGN_MK SUB_ASSIGN_MK RSH_ASSIGN_MK
%token LSH_ASSIGN_MK AND_ASSIGN_MK XOR_ASSIGN_MK  OR_ASSIGN_MK OROR_MK ANDAND_MK
%token EQUAL_MK NOTEQ_MK LESSEQ_MK GREATEREQ_MK LSH_MK RSH_MK DOTASTER_MK ARROWASTER_MK
%token SIZEOF_KW PLUSPLUS_MK MINUSMINUS_MK

%token NEW_KW DELETE_KW

%token DYNAMIC_CAST_KW STATIC_CAST_KW REINTERPRET_CAST_KW CONST_CAST_KW
%token TYPEID_KW

%token STATIC_ASSERT_KW DECLTYPE_KW

%token ARROW_MK
%token THIS_KW
%token TRY_KW
%token BREAK_KW
%token FOR_KW
%token SWITCH_KW
%token DEFAULT_KW
%token WHILE_KW
%token CONTINUE_KW
%token GOTO_KW
%token CASE_KW
%token DO_KW
%token ELSE_KW
%token RETURN_KW
%token IF_KW
%token THROW_KW
%token CATCH_KW
%token OPERATOR_KW

%token FALSE_KW TRUE_KW
%token BUILTIN_VA_ARG BUILTIN_VA_START BUILTIN_VA_END BUILTIN_ADDRESSOF
%token BUILTIN_IS_BASE_OF BUILTIN_CONSTANT_P
%token NEW_ARRAY_LEX DELETE_ARRAY_LEX

%union {
  int m_ival;
  typedef cxx_compiler::usr usr;
  usr* m_usr;
  std::vector<usr*>* m_usrs;
  typedef cxx_compiler::var var;
  var* m_var;
  typedef const cxx_compiler::type type;
  type* m_type;
  typedef cxx_compiler::declarations::type_specifier type_specifier;
  type_specifier* m_type_specifier;
  cxx_compiler::declarations::type_specifier_seq::info_t* m_type_specifier_seq;
  cxx_compiler::declarations::specifier* m_specifier;
  cxx_compiler::declarations::specifier_seq::info_t* m_specifier_seq;
  std::vector<type*>* m_types;
  typedef cxx_compiler::expressions::base expr;
  expr* m_expression;
  typedef std::vector<expr*> exprs;
  exprs* m_expressions;
  std::vector<int>* m_vi;
  typedef cxx_compiler::statements::base stmt;
  stmt* m_statement;
  std::vector<stmt*>* m_statements;
  cxx_compiler::scope* m_scope;
  cxx_compiler::declarations::initializers::info_t* m_initializer;
  cxx_compiler::declarations::initializers::clause::info_t* m_clause;
  std::vector<cxx_compiler::declarations::initializers::element*>* m_list;
  std::vector<cxx_compiler::declarations::initializers::designator::info_t*>* m_designation;
  cxx_compiler::declarations::initializers::designator::info_t* m_designator;
  cxx_compiler::expressions::postfix::member::info_t* m_member;
  typedef cxx_compiler::tag tag;
  tag* m_tag;
  cxx_compiler::file_t* m_file;
  std::vector<cxx_compiler::base*>* m_base_clause;
  cxx_compiler::base* m_base_specifier;
  cxx_compiler::name_space* m_name_space;
  std::pair<usr*, tag*>* m_ut;
  typedef std::pair<type*, expr*> param;
  param* m_param;
  std::vector<param*>* m_params;
  std::pair<type_specifier*, bool>* m_pseudo_dest;
  std::list<std::pair<type*, exprs*> >* m_new_declarator;
  typedef cxx_compiler::scope::tps_t::val2_t val2_t;
  val2_t* m_templ_arg;
  std::vector<val2_t*>* m_templ_arg_list;
  cxx_compiler::statements::try_block::HANDLER* m_handler;
  cxx_compiler::statements::try_block::HANDLERS* m_handlers;
}

%type<m_var> IDENTIFIER_LEX unqualified_id id_expression declarator_id
%type<m_var> direct_declarator declarator enumerator qualified_id
%type<m_ut> mem_initializer_id TEMPLATE_NAME_LEX template_id
%type<m_usr> INTEGER_LITERAL_LEX CHARACTER_LITERAL_LEX FLOATING_LITERAL_LEX
%type<m_usr> TYPEDEF_NAME_LEX init_declarator boolean_literal
%type<m_usrs> block_declaration simple_declaration init_declarator_list
%type<m_usrs> asm_definition
%type<m_type> ptr_operator abstract_declarator direct_abstract_declarator
%type<m_type> class_specifier elaborated_type_specifier type_id enum_specifier
%type<m_type> conversion_type_id conversion_function_id conversion_declarator
%type<m_type_specifier> simple_type_specifier type_specifier type_name
%type<m_type_specifier_seq> type_specifier_seq
%type<m_specifier> decl_specifier
%type<m_specifier_seq> decl_specifier_seq
%type<m_expression> primary_expression postfix_expression unary_expression
%type<m_expression> cast_expression pm_expression multiplicative_expression
%type<m_expression> additive_expression shift_expression relational_expression
%type<m_expression> equality_expression and_expression exclusive_or_expression
%type<m_expression> inclusive_or_expression logical_and_expression
%type<m_expression> logical_or_expression conditional_expression
%type<m_expression> assignment_expression expression constant_expression
%type<m_expression> constant_initializer condition
%type<m_expression> new_expression delete_expression throw_expression
%type<m_member> member_access_begin
%type<m_statement> labeled_statement expression_statement compound_statement
%type<m_statement> selection_statement iteration_statement jump_statement
%type<m_statement> declaration_statement try_block function_body statement
%type<m_statement> for_init_statement
%type<m_ival> cvr_qualifier storage_class_specifier function_specifier
%type<m_ival> class_key enum_key unary_operator
%type<m_ival> assignment_operator access_specifier
%type<m_vi> cvr_qualifier_seq
%type<m_statements> statement_seq
%type<m_var> literal string_literal STRING_LITERAL_LEX
%type<m_expressions> expression_list new_initializer new_placement
%type<m_scope> enter_block
%type<m_clause> initializer_clause
%type<m_list> initializer_list
%type<m_designation> designation designator_list
%type<m_designator> designator
%type<m_tag> ENUM_NAME_LEX CLASS_NAME_LEX
%type<m_tag> enum_specifier_begin class_name
%type<m_file> DEFAULT_KW
%type<m_base_clause> base_clause base_specifier_list
%type<m_base_specifier> base_specifier
%type<m_name_space> ORIGINAL_NAMESPACE_NAME_LEX NAMESPACE_ALIAS_LEX namespace_name
%type<m_type> new_type_id
%type<m_initializer> initializer
%type<m_param> parameter_declaration
%type<m_params> parameter_declaration_clause parameter_declaration_list
%type<m_ival> operator_function_id operator
%type<m_pseudo_dest> pseudo_destructor_name
%type<m_expressions> direct_new_declarator
%type<m_new_declarator> new_declarator
%type<m_templ_arg> template_argument
%type<m_templ_arg_list> template_argument_list
%type<m_handler> handler
%type<m_handlers> handler_seq
%type<m_var> exception_declaration

%%

translation_unit
  : declaration_seq
  |
  ;

declaration_seq
  : declaration
    {
      using namespace cxx_compiler;
      neaten();
      declarations::destroy();
    }
  | declaration_seq declaration
    {
      using namespace cxx_compiler;
      neaten();
      declarations::destroy();
    }
  ;

declaration
  : block_declaration { delete $1; }
  | function_definition
  | template_declaration
  | explicit_instantiation
  | explicit_specialization
  | linkage_specification
  | namespace_definition
    {
      using namespace cxx_compiler::class_or_namespace_name;
      assert(!before.empty());
      before.pop_back();
    }
  | static_assert_declaration
  | error ';'
  ;

block_declaration
  : simple_declaration
  | asm_definition
  | namespace_alias_definition
    { cxx_compiler::error::not_implemented(); }
  | using_declaration
    { $$ = 0; }
  | using_directive
    { $$ = 0; }
  ;

simple_declaration
  : decl_specifier_seq init_declarator_list ';'
    {
      using namespace cxx_compiler;
      if (!declarations::specifier_seq::info_t::s_stack.empty())
        delete $1;
      parse::identifier::mode = parse::identifier::look;
      $$ = $2;
    }
  | init_declarator_list ';'
  | decl_specifier_seq ';'
    {
      using namespace cxx_compiler;
      if (!$1->m_tag)
        error::declarations::empty(parse::position);
      delete $1;
      parse::identifier::mode = parse::identifier::look;
      $$ = 0;
    }
  | ';' { $$ = 0; }
  ;

decl_specifier
  : storage_class_specifier
    { $$ = new cxx_compiler::declarations::specifier($1); }
  | type_specifier
    { $$ = new cxx_compiler::declarations::specifier($1); }
  | function_specifier
    { $$ = new cxx_compiler::declarations::specifier($1); }
  | FRIEND_KW
    { $$ = new cxx_compiler::declarations::specifier(FRIEND_KW); }
  | TYPEDEF_KW
    { $$ = new cxx_compiler::declarations::specifier(TYPEDEF_KW); }
  ;

decl_specifier_seq
  : decl_specifier_seq decl_specifier
    { $$ = new cxx_compiler::declarations::specifier_seq::info_t($1,$2); }
  | decl_specifier
    { $$ = new cxx_compiler::declarations::specifier_seq::info_t( 0,$1); }
  ;

storage_class_specifier
  : AUTO_KW      { $$ = AUTO_KW; }
  | REGISTER_KW  { $$ = REGISTER_KW; }
  | STATIC_KW    { $$ = STATIC_KW; }
  | EXTERN_KW    { $$ = EXTERN_KW; }
  | MUTABLE_KW   { $$ = MUTABLE_KW; }
  ;

function_specifier
  : INLINE_KW    { $$ = INLINE_KW; }
  | VIRTUAL_KW   { $$ = VIRTUAL_KW; }
  | EXPLICIT_KW  { $$ = EXPLICIT_KW; }
  ;

type_specifier
  : simple_type_specifier
  | class_specifier
    { $$ = new cxx_compiler::declarations::type_specifier($1); }
  | enum_specifier
    { $$ = new cxx_compiler::declarations::type_specifier($1); }
  | elaborated_type_specifier
    { $$ = new cxx_compiler::declarations::type_specifier($1); }
  | cvr_qualifier
    { $$ = new cxx_compiler::declarations::type_specifier($1); }
  ;

simple_type_specifier
  : COLONCOLON_MK move_to_root nested_name_specifier type_name
    {
      $$ = $4;
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | COLONCOLON_MK move_to_root type_name
    {
      $$ = $3;
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | nested_name_specifier type_name
    {
      $$ = $2;
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | type_name
  | COLONCOLON_MK move_to_root nested_name_specifier TEMPLATE_KW template_id
    { cxx_compiler::error::not_implemented(); }
  | nested_name_specifier TEMPLATE_KW template_id
    { cxx_compiler::error::not_implemented(); }
  | CHAR_KW
    { $$ = new cxx_compiler::declarations::type_specifier(CHAR_KW); }
  | WCHAR_T_KW
    { $$ = new cxx_compiler::declarations::type_specifier(WCHAR_T_KW); }
  | BOOL_KW
    { $$ = new cxx_compiler::declarations::type_specifier(BOOL_KW); }
  | SHORT_KW
    { $$ = new cxx_compiler::declarations::type_specifier(SHORT_KW); }
  | INT_KW
    { $$ = new cxx_compiler::declarations::type_specifier(INT_KW); }
  | LONG_KW
    { $$ = new cxx_compiler::declarations::type_specifier(LONG_KW); }
  | SIGNED_KW
    { $$ = new cxx_compiler::declarations::type_specifier(SIGNED_KW); }
  | UNSIGNED_KW
    { $$ = new cxx_compiler::declarations::type_specifier(UNSIGNED_KW); }
  | FLOAT_KW
    { $$ = new cxx_compiler::declarations::type_specifier(FLOAT_KW); }
  | DOUBLE_KW
    { $$ = new cxx_compiler::declarations::type_specifier(DOUBLE_KW); }
  | VOID_KW
    { $$ = new cxx_compiler::declarations::type_specifier(VOID_KW); }
  | DECLTYPE_KW '(' expression ')'
    { $$ = cxx_compiler::declarations::decl_type($3); }
  | DECLTYPE_KW '(' AUTO_KW ')'
    { cxx_compiler::error::not_implemented(); }
  ;

type_name
  : class_name
    { $$ = new cxx_compiler::declarations::type_specifier($1); }
  | ENUM_NAME_LEX
    { $$ = new cxx_compiler::declarations::type_specifier($1); }
  | TYPEDEF_NAME_LEX
    { $$ = new cxx_compiler::declarations::type_specifier($1); }
  ;

elaborated_type_specifier
  : class_key COLONCOLON_MK move_to_root nested_name_specifier IDENTIFIER_LEX
    { cxx_compiler::error::not_implemented(); }
  | class_key nested_name_specifier IDENTIFIER_LEX
    { cxx_compiler::error::not_implemented(); }
  | class_key COLONCOLON_MK move_to_root IDENTIFIER_LEX
    { cxx_compiler::error::not_implemented(); }
  | class_key IDENTIFIER_LEX
   { $$ = cxx_compiler::declarations::elaborated::action($1,$2); }
  | class_key COLONCOLON_MK move_to_root nested_name_specifier
    TEMPLATE_KW template_id
    {
      $$ = cxx_compiler::declarations::elaborated::action($1,$6);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | class_key nested_name_specifier TEMPLATE_KW template_id
    {
      $$ = cxx_compiler::declarations::elaborated::action($1,$4);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | class_key COLONCOLON_MK move_to_root TEMPLATE_KW template_id
    {
      $$ = cxx_compiler::declarations::elaborated::action($1,$5);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | class_key COLONCOLON_MK move_to_root nested_name_specifier template_id
    {
      $$ = cxx_compiler::declarations::elaborated::action($1,$5);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | class_key TEMPLATE_KW template_id
    {
      $$ = cxx_compiler::declarations::elaborated::action($1,$3);
    }
  | class_key COLONCOLON_MK move_to_root template_id
    {
      $$ = cxx_compiler::declarations::elaborated::action($1,$4);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | class_key nested_name_specifier template_id
    {
      $$ = cxx_compiler::declarations::elaborated::action($1,$3);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | class_key template_id
    { $$ = cxx_compiler::declarations::elaborated::action($1,$2); }
  | enum_key COLONCOLON_MK move_to_root nested_name_specifier IDENTIFIER_LEX
    {
      $$ = cxx_compiler::declarations::elaborated::action($1,$5);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | enum_key nested_name_specifier IDENTIFIER_LEX
    {
      $$ = cxx_compiler::declarations::elaborated::action($1,$3);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | enum_key COLONCOLON_MK move_to_root IDENTIFIER_LEX
    {
      $$ = cxx_compiler::declarations::elaborated::action($1,$4);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | enum_key IDENTIFIER_LEX
    { $$ = cxx_compiler::declarations::elaborated::action($1,$2); }
  | typenaming COLONCOLON_MK move_to_root nested_name_specifier IDENTIFIER_LEX
    {
      $$ = cxx_compiler::typenamed::action($5);
      cxx_compiler::class_or_namespace_name::after(false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming COLONCOLON_MK move_to_root nested_name_specifier CLASS_NAME_LEX
    {
      $$ = cxx_compiler::typenamed::action($5);
      cxx_compiler::class_or_namespace_name::after(false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming COLONCOLON_MK move_to_root nested_name_specifier
    TYPEDEF_NAME_LEX
    {
      $$ = cxx_compiler::typenamed::action($5);
      cxx_compiler::class_or_namespace_name::after(false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming COLONCOLON_MK move_to_root nested_name_specifier ENUM_NAME_LEX
    {
      $$ = cxx_compiler::typenamed::action($5);
      cxx_compiler::class_or_namespace_name::after(false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming nested_name_specifier IDENTIFIER_LEX
    {
      $$ = cxx_compiler::typenamed::action($3);
      cxx_compiler::class_or_namespace_name::after(false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming nested_name_specifier CLASS_NAME_LEX
    {
      $$ = cxx_compiler::typenamed::action($3);
      cxx_compiler::class_or_namespace_name::after(false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming nested_name_specifier TYPEDEF_NAME_LEX
    {
      $$ = cxx_compiler::typenamed::action($3);
      cxx_compiler::class_or_namespace_name::after(false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming nested_name_specifier ENUM_NAME_LEX
    {
      $$ = cxx_compiler::typenamed::action($3);
      cxx_compiler::class_or_namespace_name::after(false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming COLONCOLON_MK move_to_root nested_name_specifier
    TEMPLATE_KW template_id
    {
      $$ = cxx_compiler::typenamed::action($6);
      cxx_compiler::class_or_namespace_name::after(false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming nested_name_specifier TEMPLATE_KW template_id
    {
      $$ = cxx_compiler::typenamed::action($4);
      cxx_compiler::class_or_namespace_name::after(false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming COLONCOLON_MK move_to_root nested_name_specifier
    template_id
    {
      $$ = cxx_compiler::typenamed::action($5);
      cxx_compiler::class_or_namespace_name::after(false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming nested_name_specifier template_id
    {
      $$ = cxx_compiler::typenamed::action($3);
      cxx_compiler::class_or_namespace_name::after(false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  ;

typenaming
  : TYPENAME_KW
    { ++cxx_compiler::parse::identifier::typenaming; }
  ;

enum_specifier
  : enum_specifier_begin enumerator_list '}'
    { $$ = cxx_compiler::declarations::enumeration::end($1); }
  | enum_specifier_begin enumerator_list ',' '}'
   { $$ = cxx_compiler::declarations::enumeration::end($1); }
  | enum_specifier_begin '}'
    { $$ = cxx_compiler::declarations::enumeration::end($1); }
  ;

enum_specifier_begin
  : enum_key IDENTIFIER_LEX '{'
    { $$ = cxx_compiler::declarations::enumeration::begin($2); }
  | enum_key enum_base IDENTIFIER_LEX '{'
    { $$ = cxx_compiler::declarations::enumeration::begin($3); }
  | enum_key '{'
    { $$ = cxx_compiler::declarations::enumeration::begin(0); }
  | enum_key enum_base '{'
    { $$ = cxx_compiler::declarations::enumeration::begin(0); }
  ;

enum_key
  : ENUM_KW
    {
      using namespace cxx_compiler;
      $$ = ENUM_KW;
      parse::identifier::mode = parse::identifier::new_obj;
    }
  ;

enum_base
  : ':' type_specifier_seq
  ;

enumerator_list
  : enumerator_definition
  | enumerator_list ',' enumerator_definition
  ;

enumerator_definition
  : enumerator
    { cxx_compiler::declarations::enumeration::definition($1,0); }
  | enumerator '='
    {
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::look;
    }
    constant_expression
    {
      cxx_compiler::declarations::enumeration::definition($1,$4);
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::new_obj;
    }
  ;

enumerator
  : IDENTIFIER_LEX
  ;

namespace_definition
  : named_namespace_definition
  | unnamed_namespace_definition
  ;

named_namespace_definition
  : original_namespace_definition
  | extension_namespace_definition
  ;

unnamed_namespace_definition
  : NAMESPACE_KW '{' namespace_body '}'
  ;

namespace_body
  : declaration_seq
  |
  ;

extension_namespace_definition
  : NAMESPACE_KW ORIGINAL_NAMESPACE_NAME_LEX '{'
    { cxx_compiler::extension_namespace_definition($2); }
    namespace_body '}'
    {
      using namespace cxx_compiler;
      scope::current = scope::current->m_parent;
    }
  ;

original_namespace_definition
  : NAMESPACE_KW IDENTIFIER_LEX '{'
    { cxx_compiler::original_namespace_definition($2, false); }
    namespace_body '}'
    {
      using namespace cxx_compiler;
      scope::current = scope::current->m_parent;
    }
  | INLINE_KW NAMESPACE_KW IDENTIFIER_LEX '{'
    { cxx_compiler::original_namespace_definition($3, true); }
    namespace_body '}'
    {
      using namespace cxx_compiler;
      scope::current = scope::current->m_parent;
    }
  ;

namespace_name
  : ORIGINAL_NAMESPACE_NAME_LEX
  | NAMESPACE_ALIAS_LEX
  ;


namespace_alias_definition
  : NAMESPACE_KW IDENTIFIER_LEX '=' qualified_namespace_specifier ';'
  ;

qualified_namespace_specifier
  : COLONCOLON_MK move_to_root nested_name_specifier namespace_name
  |               nested_name_specifier namespace_name
  | COLONCOLON_MK move_to_root          namespace_name
  |                                     namespace_name
  ;

using_declaration
  : USING_KW typenaming COLONCOLON_MK move_to_root
    nested_name_specifier unqualified_id ';'
   {
     --cxx_compiler::parse::identifier::typenaming;
     assert(cxx_compiler::parse::identifier::typenaming >= 0);
     cxx_compiler::class_or_namespace_name::after(false);
     cxx_compiler::declarations::use::action($6);
   }
  | USING_KW typenaming COLONCOLON_MK move_to_root
    nested_name_specifier TYPEDEF_NAME_LEX ';'
   {
     --cxx_compiler::parse::identifier::typenaming;
     assert(cxx_compiler::parse::identifier::typenaming >= 0);
     cxx_compiler::class_or_namespace_name::after(false);
     cxx_compiler::declarations::use::action($6);
   }
  | USING_KW COLONCOLON_MK move_to_root nested_name_specifier
    unqualified_id ';'
   {
     cxx_compiler::class_or_namespace_name::after(false);
     cxx_compiler::declarations::use::action($5);
   }
  | USING_KW COLONCOLON_MK move_to_root nested_name_specifier
    TYPEDEF_NAME_LEX ';'
   {
      cxx_compiler::class_or_namespace_name::after(false);
      cxx_compiler::declarations::use::action($5);
   }
  | USING_KW typenaming nested_name_specifier unqualified_id ';'
   {
     --cxx_compiler::parse::identifier::typenaming;
     assert(cxx_compiler::parse::identifier::typenaming >= 0);
     cxx_compiler::class_or_namespace_name::after(false);
     cxx_compiler::declarations::use::action($4);
   }
  | USING_KW COLONCOLON_MK move_to_root unqualified_id ';'
   {
      cxx_compiler::class_or_namespace_name::after(false);
      cxx_compiler::declarations::use::action($4);
   }
  | USING_KW COLONCOLON_MK move_to_root TYPEDEF_NAME_LEX ';'
   {
      cxx_compiler::class_or_namespace_name::after(false);
      cxx_compiler::declarations::use::action($4);
   }
  | USING_KW nested_name_specifier unqualified_id ';'
    {
      cxx_compiler::class_or_namespace_name::after(false);
      cxx_compiler::declarations::use::action($3);
    }
  | USING_KW nested_name_specifier TYPEDEF_NAME_LEX ';'
    {
      cxx_compiler::class_or_namespace_name::after(false);
      cxx_compiler::declarations::use::action($3);
    }
  ;

using_directive
  : USING_KW NAMESPACE_KW COLONCOLON_MK move_to_root nested_name_specifier
    namespace_name ';'
    {
      cxx_compiler::using_directive::action($6);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | USING_KW NAMESPACE_KW nested_name_specifier namespace_name ';'
    {
      cxx_compiler::using_directive::action($4);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | USING_KW NAMESPACE_KW COLONCOLON_MK move_to_root namespace_name ';'
    {
      cxx_compiler::using_directive::action($5);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | USING_KW NAMESPACE_KW namespace_name ';'
    {
      cxx_compiler::using_directive::action($3);
    }
  ;

asm_definition
  : ASM_KW '(' string_literal ')' ';'
   {
     $$ = new std::vector<cxx_compiler::usr*>;
     $$->push_back(new cxx_compiler::declarations::asm_definition::info_t($3));
   }
  | ASM_KW '(' string_literal asm_operand_list ')' ';'
  {
     $$ = new std::vector<cxx_compiler::usr*>;
     $$->push_back(new cxx_compiler::declarations::asm_definition::info_t($3));
  }
  ;

asm_operand_list : ':' asm_operands ':' asm_operands ':' reg_list
                 | ':' asm_operands ':' asm_operands
                 | ':' asm_operands ':'              ':' reg_list
                 | ':' asm_operands
                 | ':'              ':' asm_operands ':' reg_list
                 | ':'              ':' asm_operands
                 | ':'              ':'              ':' reg_list
                 | ':'              ':'              ':'
                 ;

asm_operands : asm_operand
             | asm_operands ',' asm_operand
             ;

asm_operand : string_literal '(' expression ')'
            ;

reg_list : string_literal
         | reg_list ',' string_literal
         ;

linkage_specification
  : linkage_specification_begin declaration_seq '}'
    { cxx_compiler::declarations::linkage::infos.pop_back(); }
  | linkage_specification_begin '}'
    { cxx_compiler::declarations::linkage::infos.pop_back(); }
  | EXTERN_KW STRING_LITERAL_LEX
    { cxx_compiler::declarations::linkage::action($2, false); }
    declaration
    { cxx_compiler::declarations::linkage::infos.pop_back(); }
  ;

linkage_specification_begin
  : EXTERN_KW STRING_LITERAL_LEX '{'
    { cxx_compiler::declarations::linkage::action($2, true); }
  ;

init_declarator_list
  : init_declarator
    { $$ = new std::vector<cxx_compiler::usr*>; $$->push_back($1); }
  | init_declarator_list ','
    {
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::new_obj;
    }
    init_declarator
    { $$ = $1; $$->push_back($4); }
  ;

init_declarator
  : declarator
    {
      cxx_compiler::class_or_namespace_name::after(true);
      $1 = cxx_compiler::declarations::action1($1,true);
    }
    initializer
    { cxx_compiler::declarations::initializers::action($1,$3); }
  | declarator { $$ = cxx_compiler::declarations::action1($1,false); }
  ;

declarator
  : direct_declarator
  | ptr_operator declarator
    {
      $$ = $2;
      using namespace cxx_compiler::declarations::declarators;
      $$->m_type = pointer::action($1,$2->m_type);
    }
  ;

direct_declarator
  : declarator_id
  | direct_declarator '(' enter_parameter parameter_declaration_clause
    leave_parameter ')' cvr_qualifier_seq exception_specification
    {
      using namespace cxx_compiler::declarations::declarators;
      $$ = $1;
      $$->m_type = function::action($1->m_type,$4,$1,$7);
    }
  | direct_declarator '(' enter_parameter parameter_declaration_clause
    leave_parameter ')' exception_specification
    {
      using namespace cxx_compiler::declarations::declarators;
      $$ = $1;
      $$->m_type = function::action($1->m_type,$4,$1,0);
    }
  | direct_declarator '(' enter_parameter parameter_declaration_clause
    leave_parameter ')' cvr_qualifier_seq
    {
      using namespace cxx_compiler::declarations::declarators;
      $$ = $1;
      $$->m_type = function::action($1->m_type,$4,$1,$7);
    }
  | direct_declarator '(' enter_parameter parameter_declaration_clause
    leave_parameter ')'
    {
      using namespace cxx_compiler::declarations::declarators;
      $$ = $1;
      $$->m_type = function::action($1->m_type,$4,$1,0);
    }
  | direct_declarator begin_array assignment_expression end_array
    {
      using namespace cxx_compiler::declarations::declarators;
      $$ = $1;
      $$->m_type = array::action($1->m_type,$3,false,$1);
    }
  | direct_declarator begin_array end_array
    {
      using namespace cxx_compiler::declarations::declarators;
      $$ = $1;
      $$->m_type = array::action($1->m_type,0,false,$1);
    }
  | direct_declarator begin_array '*' end_array
    {
      using namespace cxx_compiler::declarations::declarators;
      $$ = $1;
      $$->m_type = array::action($1->m_type,0,true,$1);
    }
  | '(' declarator ')'
    {
      $$ = $2;
    }
  ;

ptr_operator
  : '*' cvr_qualifier_seq
    {
      using namespace cxx_compiler::declarations::declarators;
      $$ = pointer::action($2, false);
    }
  | '*'
    {
      using namespace cxx_compiler::declarations::declarators;
      $$ = pointer::action(0, false);
     }
  | '&'
    { $$ = cxx_compiler::declarations::declarators::reference::action(false); }
  | ANDAND_MK
    { $$ = cxx_compiler::declarations::declarators::reference::action(true); }
  | COLONCOLON_MK move_to_root nested_name_specifier '*' cvr_qualifier_seq
    {
      using namespace cxx_compiler;
      using namespace declarations::declarators;
      $$ = pointer::action($5, true);
      class_or_namespace_name::after(false);
    }
  | nested_name_specifier '*' cvr_qualifier_seq
    {
      using namespace cxx_compiler;
      using namespace declarations::declarators;
      $$ = pointer::action($3, true);
      class_or_namespace_name::after(false);
    }
  | COLONCOLON_MK move_to_root nested_name_specifier '*'
    {
      using namespace cxx_compiler;
      using namespace declarations::declarators;
      $$ = pointer::action(0, true);
      class_or_namespace_name::after(false);
    }
  | nested_name_specifier '*'
    {
      using namespace cxx_compiler;
      using namespace declarations::declarators;
      $$ = pointer::action(0, true);
      class_or_namespace_name::after(false);
    }
  ;

cvr_qualifier_seq
  : cvr_qualifier cvr_qualifier_seq
    { $$ = $2; $$->push_back($1); }
  | cvr_qualifier
    { $$ = new std::vector<int>; $$->push_back($1); }
  ;

cvr_qualifier
  : CONST_KW     { $$ = CONST_KW; }
  | VOLATILE_KW  { $$ = VOLATILE_KW; }
  | RESTRICT_KW  { $$ = RESTRICT_KW; }
  ;

declarator_id
  : id_expression
  | COLONCOLON_MK move_to_root nested_name_specifier type_name
    { $$ = cxx_compiler::declarations::declarators::ctor($4); }
  | nested_name_specifier type_name
    { $$ = cxx_compiler::declarations::declarators::ctor($2); }
  | COLONCOLON_MK move_to_root type_name
    { $$ = cxx_compiler::declarations::declarators::ctor($3); }
  | type_name
    {
      /* Note that $1 is already deleted */
      $$ = cxx_compiler::declarations::declarators::ctor();
    }
  ;

parameter_declaration_clause
  : parameter_declaration_list DOTS_MK
    {
      using namespace cxx_compiler;
      $$ = $1;
      typedef const type T;
      typedef expressions::base B;
      $$->push_back(new pair<T*, B*>(ellipsis_type::create(), (B*)0));
    }
  | DOTS_MK
    {
      using namespace std;
      using namespace cxx_compiler;
      typedef const type T;
      typedef expressions::base B;
      $$ = new vector<pair<T*, B*>*>;
      $$->push_back(new pair<T*, B*>(ellipsis_type::create(), (B*)0));
    }
  | parameter_declaration_list
  |
    {
      $$ = 0;
      cxx_compiler::declarations::specifier_seq::info_t::s_stack.pop();
    }
  | parameter_declaration_list ',' DOTS_MK
    {
      using namespace cxx_compiler;
      typedef const type T;
      typedef expressions::base B;
      $$ = $1;
      $$->push_back(new pair<T*, B*>(ellipsis_type::create(), (B*)0));
      cxx_compiler::declarations::specifier_seq::info_t::s_stack.pop();
    }
  ;

parameter_declaration_list
  : parameter_declaration
    {
      using namespace std;
      using namespace cxx_compiler;
      typedef const type T;
      typedef expressions::base B;
      $$ = new vector<pair<T*, B*>*>; $$->push_back($1);
    }
  | parameter_declaration_list ',' parameter_declaration
    { $$ = $1; $$->push_back($3); }
  ;

parameter_declaration
  : decl_specifier_seq declarator
    {
      using namespace std;
      using namespace cxx_compiler;
      typedef const type T;
      typedef expressions::base B;
      T* tmp = declarations::declarators::function::parameter($1, $2);
      $$ = new pair<T*, B*>(tmp, (B*)0);
    }
  | decl_specifier_seq declarator '=' 
    { 
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    }
    assignment_expression
    {
      using namespace std;
      using namespace cxx_compiler;
      typedef const type T;
      typedef expressions::base B;
      T* tmp = declarations::declarators::function::parameter($1, $2);
      $$ = new pair<T*, B*>(tmp, $5);
    }
  | decl_specifier_seq abstract_declarator
    {
      using namespace std;
      using namespace cxx_compiler;
      typedef const type T;
      typedef expressions::base B;
      T* tmp = declarations::declarators::function::parameter($1, $2);
      $$ = new pair<T*, B*>(tmp, (B*)0);
    }
  | decl_specifier_seq
    {
      using namespace std;
      using namespace cxx_compiler;
      typedef const type T;
      typedef expressions::base B;
      T* tmp = declarations::declarators::function::parameter($1, (usr*)0);
      $$ = new pair<T*, B*>(tmp, (B*)0);
    }
  | decl_specifier_seq abstract_declarator '='
    { 
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    }
    assignment_expression
    {
      using namespace std;
      using namespace cxx_compiler;
      typedef const type T;
      typedef expressions::base B;
      T* tmp = declarations::declarators::function::parameter($1, $2);
      $$ = new pair<T*, B*>(tmp, $5);
    }
  | decl_specifier_seq '='
    { 
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    }
    assignment_expression
    {
      using namespace std;
      using namespace cxx_compiler;
      typedef const type T;
      typedef expressions::base B;
      T* tmp = declarations::declarators::function::parameter($1, (usr*)0);
      $$ = new pair<T*, B*>(tmp, $4);
    }
  ;

abstract_declarator
  : ptr_operator abstract_declarator
    { $$ = $2->patch($1,0); }
  | ptr_operator
  | direct_abstract_declarator
  ;

direct_abstract_declarator
  : direct_abstract_declarator '(' enter_parameter parameter_declaration_clause leave_parameter ')' cvr_qualifier_seq exception_specification
   
  | direct_abstract_declarator '(' enter_parameter parameter_declaration_clause leave_parameter ')'                   exception_specification
   
  | direct_abstract_declarator '(' enter_parameter parameter_declaration_clause leave_parameter ')' cvr_qualifier_seq
   
  | direct_abstract_declarator '(' enter_parameter parameter_declaration_clause leave_parameter ')'
    {
      $$ = cxx_compiler::declarations::declarators::function::action($1,$4,0,0);
    }
  |                            '(' enter_parameter parameter_declaration_clause leave_parameter ')' cvr_qualifier_seq exception_specification
    { cxx_compiler::error::not_implemented(); }
  |                            '(' enter_parameter parameter_declaration_clause leave_parameter ')'                   exception_specification
    { cxx_compiler::error::not_implemented(); }
  |                            '(' enter_parameter parameter_declaration_clause leave_parameter ')' cvr_qualifier_seq
    { cxx_compiler::error::not_implemented(); }
  | '(' enter_parameter parameter_declaration_clause leave_parameter ')'
    {
      $$ = cxx_compiler::declarations::declarators::function::action(cxx_compiler::backpatch_type::create(),$3,0,0);
    }
  | direct_abstract_declarator begin_array assignment_expression end_array
    {
      $$ = cxx_compiler::declarations::declarators::array::action($1,$3,false,0);
    }
  |                            begin_array assignment_expression end_array
    {
      $$ = cxx_compiler::declarations::declarators::array::action(cxx_compiler::backpatch_type::create(),$2,false,0);
    }
  | direct_abstract_declarator begin_array end_array
    {
      $$ = cxx_compiler::declarations::declarators::array::action($1,0,false,0);
    }
  |                            begin_array end_array
    {
      $$ = cxx_compiler::declarations::declarators::array::action(cxx_compiler::backpatch_type::create(),0,false,0);
    }
  | direct_abstract_declarator begin_array '*' end_array
    {
      $$ = cxx_compiler::declarations::declarators::array::action($1,0,true,0);
    }
  |                            begin_array '*' end_array
    {
      $$ = cxx_compiler::declarations::declarators::array::action(cxx_compiler::backpatch_type::create(),0,true,0);
    }
  | '(' abstract_declarator ')' { $$ = $2; }
  ;

begin_array
  : '['
    {
      using namespace cxx_compiler::parse::identifier;
      mode = look;
      using namespace cxx_compiler::declarations::specifier_seq;
      info_t::s_stack.push(0);
      using namespace cxx_compiler;
      if (class_or_namespace_name::last) {
        using namespace class_or_namespace_name;
        before.push_back(scope::current);
        scope::current = last;
        last = 0;
        ++decl_array;
      }
    }
  ;

end_array
  : ']'
    {
      using namespace cxx_compiler::parse::identifier;
      mode = new_obj;
      using namespace cxx_compiler::declarations::specifier_seq;
      info_t::s_stack.pop();
      using namespace cxx_compiler;
      if (class_or_namespace_name::decl_array) {
        using namespace class_or_namespace_name;
        --decl_array;
        assert(decl_array >= 0);
        assert(!before.empty());
        scope::current = before.back();
        before.pop_back();
      }
    }
  ;

exception_specification
  : THROW_KW '(' enter_exception_specification type_id_list ')'
  | THROW_KW '(' enter_exception_specification              ')'
  ;

type_id_list
  : type_id
  | type_id_list ',' type_id
  ;

enter_exception_specification
  : {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    }
  ;

initializer
  : '=' initializer_clause
    { $$ = new cxx_compiler::declarations::initializers::info_t($2); }
  | '(' expression_list ')'
    { $$ = new cxx_compiler::declarations::initializers::info_t($2); }
  ;

initializer_clause
  : assignment_expression         { $$ = new cxx_compiler::declarations::initializers::clause::info_t($1); }
  | '{' initializer_list ',' '}'  { $$ = new cxx_compiler::declarations::initializers::clause::info_t($2); }
  | '{' initializer_list     '}'  { $$ = new cxx_compiler::declarations::initializers::clause::info_t($2); }
  | '{'                      '}'  { $$ = new cxx_compiler::declarations::initializers::clause::info_t((std::vector<cxx_compiler::declarations::initializers::element*>*)0); }
  ;

initializer_list
  :                                  initializer_clause
    {
      $$ = new std::vector<cxx_compiler::declarations::initializers::element*>;
      $$->push_back(new cxx_compiler::declarations::initializers::element(0,$1));
    }
  |                      designation initializer_clause
    {
      $$ = new std::vector<cxx_compiler::declarations::initializers::element*>;
      $$->push_back(new cxx_compiler::declarations::initializers::element($1,$2));
    }
  | initializer_list ','             initializer_clause
    {
      $$ = $1;
      $$->push_back(new cxx_compiler::declarations::initializers::element(0,$3));
    }
  | initializer_list ',' designation initializer_clause
    {
      $$ = $1;
      $$->push_back(new cxx_compiler::declarations::initializers::element($3,$4));
    }
  ;

designation
  : designator_list '='
  ;

designator_list
  : designator
    { $$ = new std::vector<cxx_compiler::declarations::initializers::designator::info_t*>; $$->push_back($1); }
  | designator_list designator
    { $$ = $1; $$->push_back($2); }
  ;

designator
  : '[' constant_expression ']'
    {
      using namespace cxx_compiler::declarations::initializers;
      $$ = new designator::info_t($2,0);
    }
  | '.'
    {
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::new_obj;
    }
    IDENTIFIER_LEX
    {
      using namespace cxx_compiler::declarations::initializers;
      $$ = new designator::info_t(0,$3);
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::look;
    }
  ;

type_id
  : type_specifier_seq abstract_declarator
    {
      using namespace cxx_compiler;
      $$ = declarations::declarators::type_id::action($2);
      parse::identifier::mode = parse::identifier::look;
      delete $1;
    }
  | type_specifier_seq
    {
      using namespace cxx_compiler;
      $$ = declarations::declarators::type_id::action(0);
      parse::identifier::mode = parse::identifier::look;
      delete $1;
    }
  ;

type_specifier_seq
  : type_specifier type_specifier_seq
    {
      $$ = new cxx_compiler::declarations::type_specifier_seq::info_t($1,$2);
    }
  | type_specifier
    {
      $$ = new cxx_compiler::declarations::type_specifier_seq::info_t($1,0);
    }
  ;

class_name
  : CLASS_NAME_LEX
  | template_id
    {
      assert(!$1->first);
      $$ = $1->second;
    }
  ;

class_key
  : CLASS_KW
    {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::new_obj; $$ = CLASS_KW;
    }
  | STRUCT_KW
    {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::new_obj; $$ = STRUCT_KW;
    }
  | UNION_KW
    {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::new_obj; $$ = UNION_KW;
    }
  ;

class_specifier
  : class_specifier_begin member_specification '}'
    { $$ = cxx_compiler::classes::specifier::action(); }
  | class_specifier_begin '}'
    {
      using namespace std;
      using namespace cxx_compiler::declarations::specifier_seq;
      stack<info_t*>& s = info_t::s_stack;
      assert(!s.empty());
      assert(!s.top());
      s.pop();
      $$ = cxx_compiler::classes::specifier::action();
    }
  ;

class_specifier_begin
  : class_key '{'
    { cxx_compiler::classes::specifier::begin($1,0,0); }
  | class_key base_clause '{'
    { cxx_compiler::classes::specifier::begin($1,0,$2); }
  | class_key IDENTIFIER_LEX '{'
    { cxx_compiler::classes::specifier::begin($1,$2,0); }
  | class_key IDENTIFIER_LEX base_clause '{'
    { cxx_compiler::classes::specifier::begin($1,$2,$3); }
  | class_key nested_name_specifier CLASS_NAME_LEX '{'
    { cxx_compiler::classes::specifier::begin2($1,$3,0); }
  | class_key nested_name_specifier CLASS_NAME_LEX base_clause '{'
    { cxx_compiler::classes::specifier::begin2($1,$3,$4); }
  | class_key template_id '{'
    { cxx_compiler::classes::specifier::begin3($1,$2,0); }
  | class_key template_id base_clause '{'
    { cxx_compiler::classes::specifier::begin3($1,$2,$3); }
  | class_key nested_name_specifier template_id '{'
    { cxx_compiler::error::not_implemented(); }
  | class_key nested_name_specifier template_id base_clause '{'
    { cxx_compiler::error::not_implemented(); }
  ;

member_specification
  : member_declaration member_specification
  | member_declaration
  | access_specifier ':' member_specification
  | access_specifier ':'
  ;

member_declaration
  : decl_specifier_seq member_declarator_list ';'
    {
      using namespace cxx_compiler::declarations;
      if (!specifier_seq::info_t::s_stack.empty())
        delete $1;
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::look;
      context_t::clear();
    }
  | decl_specifier_seq ';'
    {
      delete $1;
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::look;
      context_t::clear();
    }
  | member_declarator_list ';'
    {
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::look;
      context_t::clear();
    }
  | function_definition ';'
    {
      using namespace cxx_compiler::parse;
      context_t::clear();
    }
  | function_definition
    {
      using namespace cxx_compiler::parse;
      context_t::clear();
    }
  | COLONCOLON_MK move_to_root nested_name_specifier TEMPLATE_KW
    unqualified_id ';'
    {
      using namespace cxx_compiler::parse;
      context_t::clear();
    }
  | COLONCOLON_MK move_to_root nested_name_specifier unqualified_id ';'
    {
      using namespace cxx_compiler::parse;
      context_t::clear();
    }
  | nested_name_specifier TEMPLATE_KW unqualified_id ';'
    {
      using namespace cxx_compiler::parse;
      context_t::clear();
    }
  | nested_name_specifier unqualified_id ';'
    {
      using namespace cxx_compiler::parse;
      context_t::clear();
    }
  | using_declaration
    {
      using namespace cxx_compiler::parse;
      context_t::clear();
    }
  | template_declaration
    {
      using namespace cxx_compiler;
      vector<scope*>& children = scope::current->m_children;
      typedef vector<scope*>::iterator IT;
      for (IT p = begin(children) ; p != end(children); ) {
        scope* ps = *p;
        scope::id_t id = ps->m_id;
        if (id == scope::PARAM)
          p = children.erase(p);
        else
          ++p;
      }
      using namespace cxx_compiler::parse;
      context_t::clear();
    }
  | static_assert_declaration
  ;

member_declarator_list
  : member_declarator
  | member_declarator_list ',' member_declarator
  ;

member_declarator
  : declarator
    { cxx_compiler::classes::members::action($1); }
  | declarator
    {
      cxx_compiler::classes::members::action($1);
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::look;
    } constant_initializer
    {
      using namespace cxx_compiler;
      using namespace parse;
      identifier::mode = identifier::new_obj;
      cxx_compiler::classes::members::action2($1, $3);
    }
  | IDENTIFIER_LEX ':'
    {
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::look;
      using namespace cxx_compiler::declarations::specifier_seq;
      info_t::s_stack.push(0);
    }
    constant_expression
    {
      using namespace cxx_compiler;
      using namespace parse;
      identifier::mode = identifier::new_obj;
      using namespace cxx_compiler::declarations::specifier_seq;
      info_t::s_stack.pop();
      classes::members::bit_field($1,$4);
    }
  |                ':'
    {
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::look;
      using namespace cxx_compiler::declarations::specifier_seq;
      info_t::s_stack.push(0);
    }
    constant_expression
    {
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::new_obj;
      using namespace cxx_compiler::declarations::specifier_seq;
      info_t::s_stack.pop();
      cxx_compiler::classes::members::bit_field(0,$3);
    }
  ;

constant_initializer
  : '=' constant_expression { $$ = $2; }
  ;

base_clause
  : ':'
    {
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::look;
      ++cxx_compiler::parse::base_clause;
    }
    base_specifier_list
    {
      $$ = $3;
      --cxx_compiler::parse::base_clause;
    }
  ;

base_specifier_list
  : base_specifier
    { $$ = new std::vector<cxx_compiler::base*>; $$->push_back($1); }
  | base_specifier_list ',' base_specifier
    { $$ = $1; $$->push_back($3); }
  ;

base_specifier
  : COLONCOLON_MK move_to_root nested_name_specifier class_name
    {
      $$ = new cxx_compiler::base(0,false,$4);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | COLONCOLON_MK move_to_root nested_name_specifier TYPEDEF_NAME_LEX
    {
      $$ = cxx_compiler::create_base(0,false,$4);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | COLONCOLON_MK move_to_root class_name
    {
      $$ = new cxx_compiler::base(0,false,$3);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | COLONCOLON_MK move_to_root TYPEDEF_NAME_LEX
    {
      $$ = cxx_compiler::create_base(0,false,$3);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | nested_name_specifier class_name
    {
      $$ = new cxx_compiler::base(0,false,$2);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | nested_name_specifier TYPEDEF_NAME_LEX
    {
      $$ = cxx_compiler::create_base(0,false, $2);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | class_name
    { $$ = new cxx_compiler::base(0,false,$1); }
  | TYPEDEF_NAME_LEX
    { $$ = cxx_compiler::create_base(0,false,$1); }
  | VIRTUAL_KW access_specifier COLONCOLON_MK move_to_root
    nested_name_specifier class_name
    {
      $$ = new cxx_compiler::base($2,true,$6);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | VIRTUAL_KW access_specifier COLONCOLON_MK move_to_root
    nested_name_specifier TYPEDEF_NAME_LEX
    {
      $$ = cxx_compiler::create_base($2,true,$6);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | VIRTUAL_KW access_specifier COLONCOLON_MK move_to_root class_name
    {
      $$ = new cxx_compiler::base($2,true,$5);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | VIRTUAL_KW access_specifier COLONCOLON_MK move_to_root TYPEDEF_NAME_LEX
    {
      $$ = cxx_compiler::create_base($2,true,$5);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | VIRTUAL_KW access_specifier nested_name_specifier class_name
    {
      $$ = new cxx_compiler::base($2,true,$4);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | VIRTUAL_KW access_specifier nested_name_specifier TYPEDEF_NAME_LEX
    {
      $$ = cxx_compiler::create_base($2,true,$4);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | VIRTUAL_KW access_specifier class_name
    { $$ = new cxx_compiler::base(0,true,$3); }
  | VIRTUAL_KW access_specifier TYPEDEF_NAME_LEX
    { $$ = cxx_compiler::create_base(0,true,$3); }
  | VIRTUAL_KW COLONCOLON_MK move_to_root nested_name_specifier
    class_name
    {
      $$ = new cxx_compiler::base(0,true,$5);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | VIRTUAL_KW COLONCOLON_MK move_to_root nested_name_specifier
    TYPEDEF_NAME_LEX
    {
      $$ = cxx_compiler::create_base(0,true,$5);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | VIRTUAL_KW COLONCOLON_MK move_to_root class_name
    {
      $$ = new cxx_compiler::base(0,true,$4);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | VIRTUAL_KW COLONCOLON_MK move_to_root TYPEDEF_NAME_LEX
    {
      $$ = cxx_compiler::create_base(0,true,$4);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | VIRTUAL_KW nested_name_specifier class_name
    {
      $$ = new cxx_compiler::base(0,true,$3);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | VIRTUAL_KW nested_name_specifier TYPEDEF_NAME_LEX
    {
      $$ = cxx_compiler::create_base(0,true,$3);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | VIRTUAL_KW class_name
    { $$ = new cxx_compiler::base(0,true,$2); }
  | VIRTUAL_KW TYPEDEF_NAME_LEX
    { $$ = cxx_compiler::create_base(0,true,$2); }
  | access_specifier VIRTUAL_KW COLONCOLON_MK move_to_root
    nested_name_specifier class_name
    {
      $$ = new cxx_compiler::base($1,true,$6);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | access_specifier VIRTUAL_KW COLONCOLON_MK move_to_root
    nested_name_specifier TYPEDEF_NAME_LEX
    {
      $$ = cxx_compiler::create_base($1,true,$6);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | access_specifier VIRTUAL_KW COLONCOLON_MK move_to_root class_name
    {
      $$ = new cxx_compiler::base($1,true,$5);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | access_specifier VIRTUAL_KW COLONCOLON_MK move_to_root TYPEDEF_NAME_LEX
    {
      $$ = cxx_compiler::create_base($1,true,$5);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | access_specifier VIRTUAL_KW nested_name_specifier class_name
    {
      $$ = new cxx_compiler::base($1,true,$4);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | access_specifier VIRTUAL_KW nested_name_specifier TYPEDEF_NAME_LEX
    {
      $$ = cxx_compiler::create_base($1,true,$4);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | access_specifier VIRTUAL_KW class_name
    { $$ = new cxx_compiler::base($1,true,$3); }
  | access_specifier VIRTUAL_KW TYPEDEF_NAME_LEX
    { $$ = cxx_compiler::create_base($1,true,$3); }
  | access_specifier COLONCOLON_MK move_to_root nested_name_specifier
    class_name
    {
      $$ = new cxx_compiler::base($1,false,$5);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | access_specifier COLONCOLON_MK move_to_root nested_name_specifier
    TYPEDEF_NAME_LEX
    {
      $$ = cxx_compiler::create_base($1,false,$5);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | access_specifier COLONCOLON_MK move_to_root class_name
    {
      $$ = new cxx_compiler::base($1,false,$4);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | access_specifier COLONCOLON_MK move_to_root TYPEDEF_NAME_LEX
    {
      $$ = cxx_compiler::create_base($1,false,$4);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | access_specifier nested_name_specifier class_name
    {
      $$ = new cxx_compiler::base($1,false,$3);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | access_specifier nested_name_specifier TYPEDEF_NAME_LEX
    {
      $$ = cxx_compiler::create_base($1,false,$3);
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | access_specifier class_name
    { $$ = new cxx_compiler::base($1,false,$2); }
  | access_specifier TYPEDEF_NAME_LEX
    { $$ = cxx_compiler::create_base($1,false,$2); }
  ;

access_specifier
  : PRIVATE_KW  { $$ = PRIVATE_KW; }
  | PROTECTED_KW { $$ = PROTECTED_KW; }
  | PUBLIC_KW { $$ = PUBLIC_KW; }
  ;

ctor_initializer
  : ':' move_to_param mem_initializer_list move_from_param
  ;

move_to_param
  : {
      using namespace cxx_compiler;
      fundef * fdef = fundef::current;
      assert(fdef);
      scope::current = fdef->m_param;
      class_or_namespace_name::before.push_back(scope::current);
      assert(parse::identifier::mode == parse::identifier::look);
      parse::identifier::mode = parse::identifier::mem_ini;

      map<string, vector<usr*> >& usrs = scope::current->m_usrs;
      if (usrs.find(this_name) == usrs.end()) {
        scope* p = fdef->m_usr->m_scope;
        assert(p->m_id == scope::TAG);
        tag* ptr = static_cast<tag*>(p);
        const type* T = ptr->m_types.second;
        if (!T)
          T = ptr->m_types.first;
        T = pointer_type::create(T);
        usr* this_ptr = new usr(this_name, T , usr::NONE, parse::position,
        			usr::NONE2);
        usrs[this_name].push_back(this_ptr);
        vector<usr*>& order = scope::current->m_order;
        vector<usr*> tmp = order;
        order.clear();
        order.push_back(this_ptr);
        copy(begin(tmp), end(tmp), back_inserter(order));
      }
    }
  ;

move_from_param
  : {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
      scope::current = scope::current->m_parent;
      class_or_namespace_name::before.pop_back();
    }
  ;

mem_initializer_list
  : mem_initializer
  | mem_initializer ',' mem_initializer_list
  ;

mem_initializer
  : mem_initializer_id '(' expression_list')'
    {
      using namespace cxx_compiler::declarations::declarators;
      function::definition::mem_initializer::action($1, $3);
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::mem_ini;
    }
  | mem_initializer_id '(' ')'
    {
      using namespace cxx_compiler::declarations::declarators;
      function::definition::mem_initializer::action($1, 0);
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::mem_ini;
    }
  ;

mem_initializer_id
  : COLONCOLON_MK move_to_root nested_name_specifier class_name
    {
      using namespace std;
      using namespace cxx_compiler;
      $$ = new pair<usr*, tag*>(0, $4);
      class_or_namespace_name::after(false);
      parse::identifier::mode = parse::identifier::look;
    }
  | COLONCOLON_MK move_to_root class_name
    {
      using namespace std;
      using namespace cxx_compiler;
      $$ = new pair<usr*, tag*>(0, $3);
      class_or_namespace_name::after(false);
      parse::identifier::mode = parse::identifier::look;
    }
  | nested_name_specifier class_name
    {
      using namespace std;
      using namespace cxx_compiler;
      $$ = new pair<usr*, tag*>(0, $2);
      class_or_namespace_name::after(false);
      parse::identifier::mode = parse::identifier::look;
    }
  | class_name
    {
      using namespace std;
      using namespace cxx_compiler;
      $$ = new pair<usr*, tag*>(0, $1);
      parse::identifier::mode = parse::identifier::look;
    }
  | IDENTIFIER_LEX
    {
      using namespace std;
      using namespace cxx_compiler;
      assert($1->usr_cast());
      usr* u = static_cast<usr*>($1);
      $$ = new pair<usr*, tag*>(u, 0);
      parse::identifier::mode = parse::identifier::look;
    }
  ;

conversion_function_id
  : OPERATOR_KW conversion_type_id { $$ = $2; }
  ;

conversion_type_id
  : type_specifier_seq conversion_declarator
    {
      using namespace cxx_compiler;
      $$ = declarations::declarators::type_id::action($2);
      parse::identifier::mode = parse::identifier::look;
      delete $1;
    }
  | type_specifier_seq
    {
      using namespace cxx_compiler;
      $$ = declarations::declarators::type_id::action(0);
      parse::identifier::mode = parse::identifier::look;
      delete $1;
    }
  ;

conversion_declarator
  : ptr_operator conversion_declarator
    { cxx_compiler::error::not_implemented(); }
  | ptr_operator
  ;

operator_function_id
  : OPERATOR_KW operator
    { $$ = $2; }
  | OPERATOR_KW operator
    '<' enter_templ_arg template_argument_list leave_templ_arg '>'
    { cxx_compiler::error::not_implemented(); }
  | OPERATOR_KW operator '<' enter_templ_arg leave_templ_arg '>'
    { cxx_compiler::error::not_implemented(); }
  ;

operator
  : NEW_KW { $$ = NEW_KW; }
  | DELETE_KW { $$ = DELETE_KW; }
  | NEW_KW '[' ']' { $$ = NEW_ARRAY_LEX; }
  | DELETE_KW '[' ']' { $$ = DELETE_ARRAY_LEX; }
  | '+' { $$ = '+'; }
  | '-' { $$ = '-'; }
  | '*' { $$ = '*'; }
  | '/' { $$ = '/'; }
  | '%' { $$ = '%'; }
  | '^' { $$ = '^'; }
  | '&' { $$ = '&'; }
  | '|' { $$ = '|'; }
  | '~' { $$ = '~'; }
  | '!' { $$ = '!'; }
  | '=' { $$ = '='; }
  | '<' { $$ = '<'; }
  | '>' { $$ = '>'; }
  | ADD_ASSIGN_MK { $$ = ADD_ASSIGN_MK; }
  | SUB_ASSIGN_MK { $$ = SUB_ASSIGN_MK; }
  | MUL_ASSIGN_MK { $$ = MUL_ASSIGN_MK; }
  | DIV_ASSIGN_MK { $$ = DIV_ASSIGN_MK; }
  | MOD_ASSIGN_MK { $$ = MOD_ASSIGN_MK; }
  | XOR_ASSIGN_MK { $$ = XOR_ASSIGN_MK; }
  | AND_ASSIGN_MK { $$ = AND_ASSIGN_MK; }
  | OR_ASSIGN_MK { $$ = OR_ASSIGN_MK; }
  | LSH_MK { $$ = LSH_MK; }
  | RSH_MK { $$ = RSH_MK; }
  | LSH_ASSIGN_MK { $$ = LSH_ASSIGN_MK; }
  | RSH_ASSIGN_MK { $$ = RSH_ASSIGN_MK; }
  | EQUAL_MK { $$ = EQUAL_MK; }
  | NOTEQ_MK { $$ = NOTEQ_MK; }
  | LESSEQ_MK { $$ = LESSEQ_MK; }
  | GREATEREQ_MK { $$ = GREATEREQ_MK; }
  | ANDAND_MK { $$ = ANDAND_MK; }
  | OROR_MK { $$ = OROR_MK; }
  | PLUSPLUS_MK { $$ = PLUSPLUS_MK; }
  | MINUSMINUS_MK { $$ = MINUSMINUS_MK; }
  | ',' { $$ = ','; }
  | ARROWASTER_MK { $$ = ARROWASTER_MK; }
  | ARROW_MK { $$ = ARROW_MK; }
  | '(' ')' { $$ = '('; }
  | '[' ']' { $$ = '['; }
  ;

template_declaration
  : EXPORT_KW TEMPLATE_KW '<'
    enter_templ_param template_parameter_list leave_templ_param '>'
    templ_decl_begin declaration templ_decl_end
  | TEMPLATE_KW '<'
    enter_templ_param template_parameter_list leave_templ_param '>'
    templ_decl_begin declaration templ_decl_end
  ;

enter_templ_param
  : {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::new_obj;
      assert(!parse::templ::param);
      parse::templ::param = true;
      vector<scope::tps_t>& tps = scope::current->m_tps;
      tps.resize(tps.size()+1);
    }
  ;

leave_templ_param
  : {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
      assert(parse::templ::param);
      parse::templ::param = false;
    }
  ;

templ_decl_begin
  : {
      using namespace cxx_compiler;
      declarations::templ::decl_begin();
    }
  ;

templ_decl_end
  : {
      using namespace cxx_compiler;
      declarations::templ::decl_end();
    }
  ;

template_parameter_list
  : template_parameter
  | template_parameter_list ',' 
    {
      using namespace cxx_compiler;
      int c = parse::peek();
      if (c == CLASS_KW || c == TYPENAME_KW)
        parse::identifier::mode = parse::identifier::new_obj;
      else
        parse::identifier::mode = parse::identifier::look;
    }
    template_parameter
  ;

template_parameter
  : type_parameter
  | parameter_declaration
    { cxx_compiler::templ_parameter::action($1); }
  ;

type_parameter
  : CLASS_KW IDENTIFIER_LEX
    { cxx_compiler::type_parameter::action($2, 0); }
  | CLASS_KW
    { cxx_compiler::type_parameter::action(0, 0); }
  | CLASS_KW IDENTIFIER_LEX '='
    {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    } type_id
    { cxx_compiler::type_parameter::action($2, $5); }
  | CLASS_KW '='
    {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    } type_id
    { cxx_compiler::error::not_implemented(); }
  | typenaming IDENTIFIER_LEX
    {
      cxx_compiler::type_parameter::action($2, 0);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming
    {
      cxx_compiler::type_parameter::action(0, 0);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming IDENTIFIER_LEX '='
    {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    } type_id
    {
      cxx_compiler::type_parameter::action($2, $5);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming '='
    {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    } type_id
    {
      cxx_compiler::error::not_implemented();
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | TEMPLATE_KW '<' template_parameter_list '>' CLASS_KW IDENTIFIER_LEX
    { cxx_compiler::error::not_implemented(); }
  | TEMPLATE_KW '<' template_parameter_list '>' CLASS_KW
    { cxx_compiler::error::not_implemented(); }
  | TEMPLATE_KW '<' template_parameter_list '>' CLASS_KW IDENTIFIER_LEX
    '=' id_expression
    { cxx_compiler::error::not_implemented(); }
  | TEMPLATE_KW '<' template_parameter_list '>' CLASS_KW
    '=' id_expression
    { cxx_compiler::error::not_implemented(); }
  ;

template_id
  : TEMPLATE_NAME_LEX
    '<' enter_templ_arg template_argument_list leave_templ_arg '>'
    { $$ = cxx_compiler::declarations::templ::id::action($1, $4); }
  | TEMPLATE_NAME_LEX '<' enter_templ_arg leave_templ_arg '>'
    { $$ = cxx_compiler::declarations::templ::id::action($1, 0); }
  ;

enter_templ_arg
  : {
      using namespace cxx_compiler;
      if (!parse::templ::arg) {
        assert(!expressions::constant_flag);
        expressions::constant_flag = true;
      }
      ++parse::templ::arg;
      parse::identifier::mode = parse::identifier::look;
      if (!parse::base_clause || parse::templ::arg > 1) {
        if (!class_or_namespace_name::before.empty())
          class_or_namespace_name::after(false);
      }
    }
  ;

leave_templ_arg
  : {
      using namespace cxx_compiler;
      assert(parse::templ::arg > 0);
      --parse::templ::arg;
      if (!parse::templ::arg)
        expressions::constant_flag = false;
    }
  ;

template_argument_list
  : template_argument
    {
      using namespace std;
      using namespace cxx_compiler;
      $$ = new vector<scope::tps_t::val2_t*>;
      $$->push_back($1);
    }
  | template_argument_list ',' template_argument
    {
      $$->push_back($3);
    }
  ;

template_argument
  : assignment_expression
    {
      using namespace std;
      using namespace cxx_compiler;
      $$ = new scope::tps_t::val2_t(0, $1->gen());
      parse::identifier::mode = parse::identifier::look;
    }
  | type_id
    {
      using namespace std;
      using namespace cxx_compiler;
      $$ = new scope::tps_t::val2_t($1, 0);
      parse::identifier::mode = parse::identifier::look;
    }
  | id_expression
    {
      using namespace std;
      using namespace cxx_compiler;
      $$ = new scope::tps_t::val2_t(0, $1);
      parse::identifier::mode = parse::identifier::look;
    }
  ;

explicit_instantiation
  : TEMPLATE_KW declaration
  ;

explicit_specialization
  : TEMPLATE_KW '<' '>'
    {
      using namespace cxx_compiler;
      using namespace cxx_compiler::declarations::templ;
      specialization::nest.push(scope::current);;
    }
    declaration
    {
      using namespace cxx_compiler::declarations::templ;
      assert(!specialization::nest.empty());
      specialization::nest.pop();
    }
  ;

try_block
  : TRY_KW compound_statement handler_seq
  { $$ = new cxx_compiler::statements::try_block::info_t($2, $3); }
  ;

throw_expression
  : THROW_KW assignment_expression
    { $$ = new cxx_compiler::expressions::throw_impl::info_t($2); }
  | THROW_KW
    { $$ = new cxx_compiler::expressions::throw_impl::info_t(0); }
  ;

primary_expression
  : literal
    { $$ = new cxx_compiler::expressions::primary::info_t($1); }
  | THIS_KW
    { $$ = new cxx_compiler::expressions::primary::info_t(); }
  | '(' expression ')'
    { $$ = new cxx_compiler::expressions::primary::info_t($2); }
  | id_expression
    { $$ = new cxx_compiler::expressions::primary::info_t($1); }
  ;

literal
  : INTEGER_LITERAL_LEX { $$ = $1; }
  | CHARACTER_LITERAL_LEX { $$ = $1; }
  | FLOATING_LITERAL_LEX { $$ = $1; }
  | string_literal
  | boolean_literal { $$ = $1; }
  ;

string_literal
  : STRING_LITERAL_LEX
  | string_literal STRING_LITERAL_LEX
    { $$ = cxx_compiler::expressions::primary::literal::stringa::create($1,$2); }
  ;

boolean_literal
  : FALSE_KW
    {
      using namespace cxx_compiler::expressions::primary::literal;
      $$ = boolean::create(false);
    }
  | TRUE_KW 
    {
      using namespace cxx_compiler::expressions::primary::literal;
      $$ = boolean::create(true);
    }
  ;

id_expression
  : unqualified_id
  | qualified_id
    {
      $$ = cxx_compiler::qualified_id::action($1);
      cxx_compiler::class_or_namespace_name::after(true);
    }
  ;

unqualified_id
  : IDENTIFIER_LEX
    { $$ = cxx_compiler::unqualified_id::from_nonmember($1); }
  | operator_function_id
    { $$ = cxx_compiler::unqualified_id::operator_function_id($1); }
  | conversion_function_id
    { $$ = cxx_compiler::unqualified_id::conversion_function_id($1); }
  | '~' class_name
    { $$ = cxx_compiler::unqualified_id::dtor($2); }
  | template_id
    {
      assert(!$1->second);
      $$ = $1->first;
    }
  ;

qualified_id
  : COLONCOLON_MK move_to_root nested_name_specifier TEMPLATE_KW unqualified_id
    { $$ = $5; }
  | COLONCOLON_MK move_to_root nested_name_specifier unqualified_id
    { $$ = $4; }
  | nested_name_specifier TEMPLATE_KW unqualified_id
    { $$ = $3; }
  | nested_name_specifier unqualified_id
    { $$ = $2; }
  | COLONCOLON_MK move_to_root IDENTIFIER_LEX
    {
      $$ = $3;
      cxx_compiler::class_or_namespace_name::after(false);
    }
  | COLONCOLON_MK move_to_root operator_function_id
    { cxx_compiler::error::not_implemented(); }
  | COLONCOLON_MK move_to_root template_id
    {
      assert(!$3->second);
      $$ = $3->first;
    }
  ;

move_to_root
  : {
      using namespace cxx_compiler;
      scope::current = &cxx_compiler::scope::root;
    }
  ;

nested_name_specifier
  : class_or_namespace_name COLONCOLON_MK nested_name_specifier
  | class_or_namespace_name COLONCOLON_MK
  | class_or_namespace_name COLONCOLON_MK TEMPLATE_KW nested_name_specifier
    {
      using namespace cxx_compiler;
      assert(scope::current->m_id == scope::TAG);
      tag* ptr = static_cast<tag*>(scope::current);
      assert(ptr->m_flag & tag::INSTANTIATE);
      instantiated_tag* it = static_cast<instantiated_tag*>(ptr);
      template_tag* tt = it->m_src;
      if (tt->m_created) {
        assert(!class_or_namespace_name::before.empty());
        class_or_namespace_name::before.pop_back();
      }
    }
  ;

class_or_namespace_name
  : class_name
    {
      using namespace cxx_compiler;
      scope::current = class_or_namespace_name::conv($1);
    }
  | namespace_name
    { cxx_compiler::scope::current = $1; }
  ;

postfix_expression
  : primary_expression
  | postfix_expression '[' expression ']'
    { $$ = new cxx_compiler::expressions::binary::info_t($1,'[',$3); }
  | postfix_expression '(' expression_list ')'
    { $$ = new cxx_compiler::expressions::postfix::call($1,$3); }
  | postfix_expression '(' ')'
    { $$ = new cxx_compiler::expressions::postfix::call($1,0); }
  | simple_type_specifier '(' fcast_prev  expression_list ')'
    { $$ = new cxx_compiler::expressions::postfix::fcast($1, $4); }
  | simple_type_specifier '(' fcast_prev ')'
    { $$ = new cxx_compiler::expressions::postfix::fcast($1, 0); }
  | typenaming COLONCOLON_MK move_to_root nested_name_specifier
    IDENTIFIER_LEX '(' fcast_prev2 expression_list ')'
    {
      $$ = new cxx_compiler::expressions::postfix::fcast($5, $8);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming COLONCOLON_MK move_to_root nested_name_specifier
    CLASS_NAME_LEX '(' fcast_prev2 expression_list ')'
    {
      $$ = new cxx_compiler::expressions::postfix::fcast($5, $8, false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming COLONCOLON_MK move_to_root nested_name_specifier
    TYPEDEF_NAME_LEX '(' fcast_prev2 expression_list ')'
    {
      $$ = new cxx_compiler::expressions::postfix::fcast($5, $8, false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming COLONCOLON_MK move_to_root nested_name_specifier
    ENUM_NAME_LEX '(' fcast_prev2 expression_list ')'
    {
      $$ = new cxx_compiler::expressions::postfix::fcast($5, $8, false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming COLONCOLON_MK move_to_root nested_name_specifier
    IDENTIFIER_LEX '(' fcast_prev2 ')'
    {
      $$ = new cxx_compiler::expressions::postfix::fcast($5, 0);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming COLONCOLON_MK move_to_root nested_name_specifier
    CLASS_NAME_LEX '(' fcast_prev2 ')'
    {
      $$ = new cxx_compiler::expressions::postfix::fcast($5, 0, false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming COLONCOLON_MK move_to_root nested_name_specifier
    TYPEDEF_NAME_LEX '(' fcast_prev2 ')'
    {
      $$ = new cxx_compiler::expressions::postfix::fcast($5, 0, false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming COLONCOLON_MK move_to_root nested_name_specifier
    ENUM_NAME_LEX '(' fcast_prev2 ')'
    {
      $$ = new cxx_compiler::expressions::postfix::fcast($5, 0, false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming nested_name_specifier
    IDENTIFIER_LEX '(' fcast_prev2 expression_list ')'
    {
      $$ = new cxx_compiler::expressions::postfix::fcast($3, $6);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming nested_name_specifier
    CLASS_NAME_LEX '(' fcast_prev2 expression_list ')'
    {
      $$ = new cxx_compiler::expressions::postfix::fcast($3, $6, false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming nested_name_specifier
    TYPEDEF_NAME_LEX '(' fcast_prev2 expression_list ')'
    {
      $$ = new cxx_compiler::expressions::postfix::fcast($3, $6, false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming nested_name_specifier
    ENUM_NAME_LEX '(' fcast_prev2 expression_list ')'
    {
      $$ = new cxx_compiler::expressions::postfix::fcast($3, $6, false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming nested_name_specifier
    IDENTIFIER_LEX '(' fcast_prev2 ')'
    {
      $$ = new cxx_compiler::expressions::postfix::fcast($3, 0);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming nested_name_specifier
    CLASS_NAME_LEX '(' fcast_prev2 ')'
    {
      $$ = new cxx_compiler::expressions::postfix::fcast($3, 0, false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming nested_name_specifier
    TYPEDEF_NAME_LEX '(' fcast_prev2 ')'
    {
      $$ = new cxx_compiler::expressions::postfix::fcast($3, 0, false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming nested_name_specifier
    ENUM_NAME_LEX '(' fcast_prev2 ')'
    {
      $$ = new cxx_compiler::expressions::postfix::fcast($3, 0, false);
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming COLONCOLON_MK move_to_root nested_name_specifier
    TEMPLATE_KW template_id '(' fcast_prev2 expression_list ')'
    {
      cxx_compiler::error::not_implemented();
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming COLONCOLON_MK move_to_root nested_name_specifier
    TEMPLATE_KW template_id '(' fcast_prev2 ')'
    {
      cxx_compiler::error::not_implemented();
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming COLONCOLON_MK move_to_root nested_name_specifier
    template_id '(' fcast_prev2 expression_list ')'
    {
      cxx_compiler::error::not_implemented();
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming COLONCOLON_MK move_to_root nested_name_specifier 
    template_id '(' fcast_prev2 ')'
    {
      cxx_compiler::error::not_implemented();
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming nested_name_specifier TEMPLATE_KW template_id
    '(' fcast_prev2 expression_list ')'
    {
      cxx_compiler::error::not_implemented();
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming nested_name_specifier
    TEMPLATE_KW template_id '(' fcast_prev2 ')'
    {
      cxx_compiler::error::not_implemented();
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming nested_name_specifier
    template_id '(' fcast_prev2 expression_list ')'
    {
      cxx_compiler::error::not_implemented();
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | typenaming nested_name_specifier template_id '(' fcast_prev2 ')'
    {
      cxx_compiler::error::not_implemented();
      --cxx_compiler::parse::identifier::typenaming;
      assert(cxx_compiler::parse::identifier::typenaming >= 0);
    }
  | member_access_begin TEMPLATE_KW id_expression
    { $$ = cxx_compiler::expressions::postfix::member::end($1,$3); }
  | member_access_begin id_expression
    { $$ = cxx_compiler::expressions::postfix::member::end($1,$2); }
  | member_access_begin pseudo_destructor_name
    { $$ = cxx_compiler::expressions::postfix::member::end($1,$2); }
  | postfix_expression PLUSPLUS_MK
    { $$ = new cxx_compiler::expressions::postfix::ppmm($1,true); }
  | postfix_expression MINUSMINUS_MK
    { $$ = new cxx_compiler::expressions::postfix::ppmm($1,false); }
  | DYNAMIC_CAST_KW '<' type_id '>' '(' expression ')'
    { cxx_compiler::error::not_implemented(); }
  | STATIC_CAST_KW '<' type_id '>' '(' expression ')'
    { $$ = new cxx_compiler::expressions::cast::info_t($3,$6); }
  | REINTERPRET_CAST_KW '<' type_id '>' '(' expression ')'
    { $$ = new cxx_compiler::expressions::cast::info_t($3,$6); }
  | CONST_CAST_KW '<' type_id '>' '(' expression ')'
    { $$ = new cxx_compiler::expressions::cast::info_t($3,$6); }
  | TYPEID_KW '(' expression ')'
    { cxx_compiler::error::not_implemented(); }
  | TYPEID_KW '(' type_id ')'
    { cxx_compiler::error::not_implemented(); }
  | '(' type_id ')' '{' initializer_list '}'
    { $$ = new cxx_compiler::expressions::compound::info_t($2,$5); }
  | '(' type_id ')' '{' initializer_list ',' '}'
    { $$ = new cxx_compiler::expressions::compound::info_t($2,$5); }
  ;

fcast_prev
  :
    {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    }
  ;

fcast_prev2
  :
    {
      cxx_compiler::class_or_namespace_name::after(false);
    }
  ;

member_access_begin
  : postfix_expression '.'
    { $$ = cxx_compiler::expressions::postfix::member::begin($1,true); }
  | postfix_expression ARROW_MK
    { $$ = cxx_compiler::expressions::postfix::member::begin($1,false); }
  ;

expression_list
  : assignment_expression
    {
      $$ = new std::vector<cxx_compiler::expressions::base*>;
      $$->push_back($1);
    }
  | expression_list ',' assignment_expression
    { $$ = $1; $$->push_back($3); }
  ;

pseudo_destructor_name
  : COLONCOLON_MK move_to_root nested_name_specifier type_name
    COLONCOLON_MK '~' type_name
    {
      using namespace std;
      using namespace cxx_compiler::declarations;
      delete $4; $$ = new pair<type_specifier*, bool>($7, true);
    }
  | COLONCOLON_MK move_to_root type_name COLONCOLON_MK '~' type_name
    {
      using namespace std;
      using namespace cxx_compiler::declarations;
      delete $3; $$ = new pair<type_specifier*, bool>($6, true);
    }
  | nested_name_specifier type_name COLONCOLON_MK '~' type_name
    {
      using namespace std;
      using namespace cxx_compiler::declarations;
      delete $2; $$ = new pair<type_specifier*, bool>($5, true);
    }
  | type_name COLONCOLON_MK '~' type_name
    {
      using namespace std;
      using namespace cxx_compiler::declarations;
      delete $1; $$ = new pair<type_specifier*, bool>($4, true);
    }
  | COLONCOLON_MK move_to_root nested_name_specifier TEMPLATE_KW
    template_id COLONCOLON_MK '~' type_name
    {
      using namespace std;
      using namespace cxx_compiler::declarations;
      $$ = new pair<type_specifier*, bool>($8, true);
    }
  | nested_name_specifier TEMPLATE_KW template_id COLONCOLON_MK '~' type_name
    {
      using namespace std;
      using namespace cxx_compiler::declarations;
      $$ = new pair<type_specifier*, bool>($6, true);
    }
  | COLONCOLON_MK move_to_root nested_name_specifier '~' type_name
    {
      using namespace std;
      using namespace cxx_compiler::declarations;
      $$ = new pair<type_specifier*, bool>($5, true);
    }
  | COLONCOLON_MK '~' type_name
    {
      using namespace std;
      using namespace cxx_compiler::declarations;
      $$ = new pair<type_specifier*, bool>($3, true);
    }
  | nested_name_specifier '~' type_name
    {
      using namespace std;
      using namespace cxx_compiler::declarations;
      $$ = new pair<type_specifier*, bool>($3, true);
    }
  | '~' type_name
    {
      using namespace std;
      using namespace cxx_compiler::declarations;
      $$ = new pair<type_specifier*, bool>($2, false);
    }
  ;

unary_expression
  : postfix_expression
  | PLUSPLUS_MK cast_expression
    { $$ = new cxx_compiler::expressions::unary::ppmm(true,$2); }
  | MINUSMINUS_MK cast_expression
    { $$ = new cxx_compiler::expressions::unary::ppmm(false,$2); }
  | unary_operator cast_expression
    { $$ = new cxx_compiler::expressions::unary::ope($1,$2); }
  | SIZEOF_KW unary_expression
    { $$ = new cxx_compiler::expressions::unary::size_of($2); }
  | SIZEOF_KW '(' type_id ')'
    {
      using namespace cxx_compiler;
      $$ = new expressions::unary::size_of($3);
      parse::identifier::mode = parse::identifier::look;
    }
  | new_expression
  | delete_expression
  ;

unary_operator
  : '*'  { $$ = '*'; }
  | '&'  { $$ = '&'; }
  | '+'  { $$ = '+'; }
  | '-'  { $$ = '-'; }
  | '!'  { $$ = '!'; }
  | '~'  { $$ = '~'; }
  ;

new_expression
  : COLONCOLON_MK move_to_root NEW_KW new_placement new_type_id
    {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    }
    new_initializer
    { cxx_compiler::error::not_implemented(); }
  | COLONCOLON_MK move_to_root NEW_KW new_placement new_type_id
    { cxx_compiler::error::not_implemented(); }
  | COLONCOLON_MK move_to_root NEW_KW new_type_id
    {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    }
    new_initializer
    { cxx_compiler::error::not_implemented(); }
  | COLONCOLON_MK move_to_root NEW_KW new_type_id
    { cxx_compiler::error::not_implemented(); }
  | NEW_KW new_placement new_type_id
    {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    }
    new_initializer
    {
      using namespace cxx_compiler;
      using namespace cxx_compiler::expressions::unary;
      $$ = new new_expr($2, $3, $5, parse::position);
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::look;
    }
  | NEW_KW new_placement new_type_id
    {
      using namespace cxx_compiler;
      using namespace cxx_compiler::expressions::unary;
      $$ = new new_expr($2, $3, 0, parse::position);
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::look;
    }
  | NEW_KW new_type_id
    {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    }
    new_initializer
    {
      using namespace cxx_compiler;
      using namespace cxx_compiler::expressions::unary;
      $$ = new new_expr($2, $4, parse::position);
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::look;
    }
  | NEW_KW new_type_id
    {
      using namespace cxx_compiler;
      using namespace expressions::unary;
      $$ = new new_expr($2, parse::position);
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::look;
    }
  | COLONCOLON_MK move_to_root NEW_KW new_placement '(' type_id ')'
    {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    }
    new_initializer
    { cxx_compiler::error::not_implemented(); }
  | COLONCOLON_MK move_to_root NEW_KW new_placement '(' type_id ')'
    { cxx_compiler::error::not_implemented(); }
  | COLONCOLON_MK move_to_root NEW_KW  '(' type_id ')'
    {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    }
    new_initializer
    { cxx_compiler::error::not_implemented(); }
  | COLONCOLON_MK move_to_root NEW_KW '(' type_id ')'
    { cxx_compiler::error::not_implemented(); }
  | NEW_KW new_placement '(' type_id ')'
    {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    }
    new_initializer
    {
      cxx_compiler::error::not_implemented();
    }
  | NEW_KW new_placement '(' type_id ')'
    {
      cxx_compiler::error::not_implemented();
    }
  | NEW_KW '(' type_id ')'
    {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    }
    new_initializer
    { cxx_compiler::error::not_implemented(); }
  | NEW_KW '(' type_id ')'
    { cxx_compiler::error::not_implemented(); }
  ;

new_placement
  : '(' expression_list ')' { $$ = $2; }
  ;

new_initializer
  : '(' expression_list ')' { $$ = $2; }
  | '('                 ')' { $$ = 0; }
  ;

new_type_id
  : type_specifier_seq new_declarator
    { $$ = cxx_compiler::declarations::new_type_id::action($1, $2); }
  | type_specifier_seq
    { $$ = cxx_compiler::declarations::new_type_id::action($1, 0); }
  ;

new_declarator
  : ptr_operator new_declarator
    {
      using namespace std;
      typedef const cxx_compiler::type type;
      typedef cxx_compiler::expressions::base expr;
      typedef vector<expr*> exprs;
      $$ = $2;
      $$->push_front(pair<type*, exprs*>($1,0)); 
    }
  | ptr_operator
    {
      using namespace std;
      typedef const cxx_compiler::type type;
      typedef cxx_compiler::expressions::base expr;
      typedef vector<expr*> exprs;
      $$ = new list<pair<type*, exprs*> >;
      $$->push_back(pair<type*, exprs*>($1,0)); 
    }
  | direct_new_declarator
    {
      using namespace std;
      typedef const cxx_compiler::type type;
      typedef cxx_compiler::expressions::base expr;
      typedef vector<expr*> exprs;
      $$ = new list<pair<type*, exprs*> >;
      $$->push_back(pair<type*, exprs*>(0,$1)); 
    }
  ;

direct_new_declarator
  : '['
    {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    }
    expression ']'
    {
      using namespace std;
      using namespace cxx_compiler;
      $$ = new vector<expressions::base*>;
      $$->push_back($3); 
    }
  | direct_new_declarator '[' constant_expression ']'
    { $$ = $1; $$->push_back($3); }
  ;

delete_expression
  : COLONCOLON_MK move_to_root DELETE_KW cast_expression
    {
      $$ = new cxx_compiler::expressions::unary::delete_expr($4, false, true);
    }
  | DELETE_KW cast_expression
    {
      $$ = new cxx_compiler::expressions::unary::delete_expr($2, false, false);
    }
  | COLONCOLON_MK move_to_root DELETE_KW '[' ']' cast_expression
    { $$ = new cxx_compiler::expressions::unary::delete_expr($6, true, true); }
  | DELETE_KW '[' ']' cast_expression
    {
      $$ = new cxx_compiler::expressions::unary::delete_expr($4, true, false);
    }
  ;

cast_expression
  : unary_expression
  | '(' type_id ')'
    {
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::look;
    }
    cast_expression
    {
      $$ = new cxx_compiler::expressions::cast::info_t($2,$5);
    }
  | BUILTIN_VA_START '(' cast_expression ',' cast_expression ')'
    { $$ = new cxx_compiler::expressions::_va_start::info_t($3,$5); }
  | BUILTIN_VA_ARG '(' cast_expression ',' type_id ')'
    { $$ = new cxx_compiler::expressions::_va_arg::info_t($3,$5); }
  | BUILTIN_VA_END '(' cast_expression ')'
    { $$ = new cxx_compiler::expressions::_va_end::info_t($3); }
  | BUILTIN_ADDRESSOF '(' cast_expression ')'
    { $$ = new cxx_compiler::expressions::address_of::info_t($3); }
  | BUILTIN_IS_BASE_OF '(' type_id ',' type_id ')'
    { $$ = new cxx_compiler::expressions::is_base_of::info_t($3, $5); }
  | BUILTIN_CONSTANT_P '(' cast_expression ')'
    { $$ = new cxx_compiler::expressions::constant_p::info_t($3); }
  ;

pm_expression
  : cast_expression
  | pm_expression DOTASTER_MK cast_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,DOTASTER_MK,$3); }
  | pm_expression ARROWASTER_MK cast_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,ARROWASTER_MK,$3); }
  ;

multiplicative_expression
  : pm_expression
  | multiplicative_expression '*' pm_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,'*',$3); }
  | multiplicative_expression '/' pm_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,'/',$3); }
  | multiplicative_expression '%' pm_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,'%',$3); }
  ;

additive_expression
  : multiplicative_expression
  | additive_expression '+' multiplicative_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,'+',$3); }
  | additive_expression '-' multiplicative_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,'-',$3); }
  ;

shift_expression
  : additive_expression
  | shift_expression LSH_MK additive_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,LSH_MK,$3); }
  | shift_expression RSH_MK additive_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,RSH_MK,$3); }
  ;

relational_expression
  : shift_expression
  | relational_expression '<' shift_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,'<',$3); }
  | relational_expression '>' shift_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,'>',$3); }
  | relational_expression LESSEQ_MK shift_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,LESSEQ_MK,$3); }
  | relational_expression GREATEREQ_MK shift_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,GREATEREQ_MK,$3); }
  ;

equality_expression
  : relational_expression
  | equality_expression EQUAL_MK relational_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,EQUAL_MK,$3); }
  | equality_expression NOTEQ_MK relational_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,NOTEQ_MK,$3); }
  ;

and_expression
  : equality_expression
  | and_expression '&' equality_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,'&',$3); }
  ;

exclusive_or_expression
  : and_expression
  | exclusive_or_expression '^' and_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,'^',$3); }
  ;

inclusive_or_expression
  : exclusive_or_expression
  | inclusive_or_expression '|' exclusive_or_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,'|',$3); }
  ;

logical_and_expression
  : inclusive_or_expression
  | logical_and_expression ANDAND_MK inclusive_or_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,ANDAND_MK,$3); }
  ;

logical_or_expression
  : logical_and_expression
  | logical_or_expression OROR_MK logical_and_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,OROR_MK,$3); }
  ;

conditional_expression
  : logical_or_expression
  | logical_or_expression '?' expression ':' assignment_expression
    { $$ = new cxx_compiler::expressions::conditional::info_t($1,$3,$5); }
  ;

assignment_expression
  : conditional_expression
  | logical_or_expression assignment_operator assignment_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,$2,$3); }
  | throw_expression
  ;

assignment_operator
  : '='            { $$ = '='; }
  | MUL_ASSIGN_MK  { $$ = MUL_ASSIGN_MK; }
  | DIV_ASSIGN_MK  { $$ = DIV_ASSIGN_MK; }
  | MOD_ASSIGN_MK  { $$ = MOD_ASSIGN_MK; }
  | ADD_ASSIGN_MK  { $$ = ADD_ASSIGN_MK; }
  | SUB_ASSIGN_MK  { $$ = SUB_ASSIGN_MK; }
  | RSH_ASSIGN_MK  { $$ = RSH_ASSIGN_MK; }
  | LSH_ASSIGN_MK  { $$ = LSH_ASSIGN_MK; }
  | AND_ASSIGN_MK  { $$ = AND_ASSIGN_MK; }
  | XOR_ASSIGN_MK  { $$ = XOR_ASSIGN_MK; }
  |  OR_ASSIGN_MK  { $$ =  OR_ASSIGN_MK; }
  ;

expression
  : assignment_expression
  | expression ',' assignment_expression
    { $$ = new cxx_compiler::expressions::binary::info_t($1,',',$3); }
  ;

constant_expression
  : conditional_expression
  ;

function_definition_begin1
  : declarator
    {
      using namespace cxx_compiler::declarations::declarators;
      function::definition::begin(0,$1);
    }
  | decl_specifier_seq declarator
    {
      using namespace cxx_compiler::declarations::declarators;
      function::definition::begin($1,$2);
    }
  ;

function_definition_begin2
  : declarator
    { cxx_compiler::error::not_implemented(); }   
  | decl_specifier_seq declarator
    { cxx_compiler::error::not_implemented(); }   
  ;

function_definition
  : function_definition_begin1 function_body
    {
      cxx_compiler::declarations::declarators::
      function::definition::action($2);
    }
  | function_definition_begin2 function_try_block
  ;

function_body
  : compound_statement
  | ctor_initializer compound_statement { $$ = $2; }
  ;

function_try_block
  : TRY_KW ctor_initializer function_body handler_seq
  | TRY_KW                  function_body handler_seq
  ;

handler_seq
  : handler handler_seq
    {
      $$ = $2;
      $$->push_front($1);
    }
  | handler
    {
      using namespace cxx_compiler;
      $$ = new std::list<pair<var*, statements::base*>*>;
      $$->push_front($1);
    }
  ;

handler
  : CATCH_KW '(' enter_block exception_declaration ')'
    compound_statement leave_block
    {
      using namespace cxx_compiler;
      $$ = new std::pair<var*, statements::base*>($4, $6);
    }
  ;

exception_declaration
  : type_specifier_seq declarator
    {
      $$ = cxx_compiler::exception::declaration::action($1, $2);
    }
  | type_specifier_seq abstract_declarator
    { cxx_compiler::error::not_implemented(); }
  | type_specifier_seq
    {
      $$ = cxx_compiler::exception::declaration::action($1, 0);
    }
  | DOTS_MK
    { $$ = 0; }
  ;

statement
  : labeled_statement
  | expression_statement
    { $$ = $1; cxx_compiler::parse::context_t::clear(); }
  | compound_statement
  | selection_statement
  | iteration_statement
  | jump_statement
  | declaration_statement
    { $$ = $1; cxx_compiler::parse::context_t::clear(); }
  | try_block
  ;

labeled_statement
  : IDENTIFIER_LEX ':' statement
    { $$ = new cxx_compiler::statements::label::info_t($1,$3); }
  | CASE_KW constant_expression ':' statement
    { $$ = new cxx_compiler::statements::_case::info_t($2,$4); }
  | DEFAULT_KW ':' statement
    { $$ = new cxx_compiler::statements::_default::info_t($1,$3); }
  ;

expression_statement
  : expression ';' { $$ = new cxx_compiler::statements::expression::info_t($1); }
  ;

compound_statement
  : '{' enter_block statement_seq leave_block '}' { $$ = new cxx_compiler::statements::compound::info_t($3,$2); }
  | '{' enter_block               leave_block '}' { $$ = new cxx_compiler::statements::compound::info_t(0,$2); }
  ;

statement_seq
  : statement
    {
      $$ = new std::vector<cxx_compiler::statements::base*>;
      $$->push_back($1);
    }
  | statement_seq statement
    { $$ = $1; $$->push_back($2); }
  ;

selection_statement
  : IF_KW '(' condition ')' statement { $$ = new cxx_compiler::statements::if_stmt::info_t($3,$5,0); }
  | IF_KW '(' condition ')' statement ELSE_KW statement { $$ = new cxx_compiler::statements::if_stmt::info_t($3,$5,$7); }
  | SWITCH_KW '(' condition ')' statement { $$ = new cxx_compiler::statements::switch_stmt::info_t($3,$5); }
  ;

condition
  : expression
  | type_specifier_seq declarator '='
    {
      using namespace cxx_compiler;
      parse::identifier::mode = parse::identifier::look;
    }
    assignment_expression
    { $$ = cxx_compiler::statements::condition::action($1, $2, $5); }
  ;

iteration_statement
  : WHILE_KW '(' condition ')' statement
    { $$ = new cxx_compiler::statements::while_stmt::info_t($3,$5); }
  | DO_KW statement WHILE_KW '(' expression ')' ';'
    { $$ = new cxx_compiler::statements::do_stmt::info_t($2,$5); }
  | FOR_KW '(' for_init_statement condition ';' expression ')'
      statement leave_block
    { $$ = new cxx_compiler::statements::for_stmt::info_t($3,$4,$6,$8); }
  | FOR_KW '(' for_init_statement condition ';'            ')'
      statement leave_block
    { $$ = new cxx_compiler::statements::for_stmt::info_t($3,$4,0,$7); }
  | FOR_KW '(' for_init_statement           ';' expression ')'
      statement leave_block
    { $$ = new cxx_compiler::statements::for_stmt::info_t($3,0,$5,$7); }
  | FOR_KW '(' for_init_statement           ';'            ')'
      statement leave_block
    { $$ = new cxx_compiler::statements::for_stmt::info_t($3,0,0,$6); }
  ;

for_init_statement
  : enter_block expression_statement
    { $$ = $2; }
  | enter_block simple_declaration
    { $$ = new cxx_compiler::statements::declaration::info_t($2,true); }
  ;

jump_statement
  : BREAK_KW ';'
    { $$ = new cxx_compiler::statements::break_stmt::info_t; }
  | CONTINUE_KW ';'
    { $$ = new cxx_compiler::statements::continue_stmt::info_t; }
  | RETURN_KW expression ';'
    { $$ = new cxx_compiler::statements::return_stmt::info_t($2); }
  | RETURN_KW ';'
    { $$ = new cxx_compiler::statements::return_stmt::info_t(0); }
  | GOTO_KW
    {
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::new_obj;
    }
    IDENTIFIER_LEX ';'
    {
      using namespace cxx_compiler::parse;
      identifier::mode = identifier::look;
      $$ = new cxx_compiler::statements::goto_stmt::info_t($3);
    }
  ;

declaration_statement
  : block_declaration
    { $$ = new cxx_compiler::statements::declaration::info_t($1,false); }
  ;

enter_parameter
  : { cxx_compiler::parse::parameter::enter(); }
  ;

leave_parameter
  : { cxx_compiler::parse::parameter::leave(); }
  ;

enter_block
  : { cxx_compiler::parse::block::enter(); $$ = cxx_compiler::scope::current; }
  ;

leave_block
  : { cxx_compiler::parse::block::leave(); }
  ;

static_assert_declaration
  : STATIC_ASSERT_KW '(' constant_expression ',' STRING_LITERAL_LEX ')' ';'
  | STATIC_ASSERT_KW '(' constant_expression ')' ';'
  ;

%%
