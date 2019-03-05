#State XXX
#
#  AAA direct_declarator: '(' . declarator ')'
#  BBB direct_abstract_declarator: '(' . enter_parameter parameter_declaration_clause leave_parameter ')' cvr_qualifier_seq exception_specification
#  CCC                           | '(' . enter_parameter parameter_declaration_clause leave_parameter ')' exception_specification
#  DDD                           | '(' . enter_parameter parameter_declaration_clause leave_parameter ')' cvr_qualifier_seq
#  EEE                           | '(' . enter_parameter parameter_declaration_clause leave_parameter ')'
#  FFF                           | '(' . abstract_declarator ')'
#
#    ORIGINAL_NAMESPACE_NAME_LEX  shift, and go to state SSS0
#    NAMESPACE_ALIAS_LEX          shift, and go to state SSS1
#    IDENTIFIER_LEX               shift, and go to state SSS2
#    TYPEDEF_NAME_LEX             shift, and go to state SSS3
#    CLASS_NAME_LEX               shift, and go to state SSS4
#    TEMPLATE_NAME_LEX            shift, and go to state SSS5
#    ENUM_NAME_LEX                shift, and go to state SSS6
#    COLONCOLON_MK                shift, and go to state SSS7
#    OPERATOR_KW                  shift, and go to state SSS8
#    '('                          shift, and go to state SSS9
#    '*'                          shift, and go to state SSS10
#    '&'                          shift, and go to state SSS11
#    '['                          shift, and go to state SSS12
#    '~'                          shift, and go to state SSS13
#
#    ORIGINAL_NAMESPACE_NAME_LEX  [reduce using rule YYY (enter_parameter)]
#    NAMESPACE_ALIAS_LEX          [reduce using rule YYY (enter_parameter)]
#    TYPEDEF_NAME_LEX             [reduce using rule YYY (enter_parameter)]
#    CLASS_NAME_LEX               [reduce using rule YYY (enter_parameter)]
#    TEMPLATE_NAME_LEX            [reduce using rule YYY (enter_parameter)]
#    ENUM_NAME_LEX                [reduce using rule YYY (enter_parameter)]
#    COLONCOLON_MK                [reduce using rule YYY (enter_parameter)]
#    $default                     reduce using rule YYY (enter_parameter)
#
#
#  Example:
#
#  typdef int T;
#  void f(double (T));  // reduce using rule YYY at ( . T)

while ( <> ){
    chop;
    next if ( !/^[Ss]tate ([0-9]+)/ );
    $xxx = $1;
    $_ = <>;
    $_ = <>; chop;
    next if ( !/[0-9]+ direct_declarator: '\(' \. declarator '\)'/ );
    $_ = <>; chop;
    next if ( !/[0-9]+ direct_abstract_declarator: '\(' \. enter_parameter parameter_declaration_clause leave_parameter '\)' cvr_qualifier_seq exception_specification/ );
    $_ = <>; chop;
    next if ( !/[0-9]+ +| '\(' \. enter_parameter parameter_declaration_clause leave_parameter '\)' exception_specification/);
    $_ = <>; chop;
    next if ( !/[0-9]+ +| '\(' \. enter_parameter parameter_declaration_clause leave_parameter '\)' cvr_qualifier_seq/);
    $_ = <>; chop;
    next if ( !/[0-9]+ +| '\(' \. enter_parameter parameter_declaration_clause leave_parameter '\)'/);
    $_ = <>; chop;
    next if ( !/[0-9]+ +| '\(' \. abstract_declarator '\)'/);
    $_ = <>;
    $_ = <>; chop;
    next if ( !/ORIGINAL_NAMESPACE_NAME_LEX +shift, and go to state [0-9]+/);
    $_ = <>; chop;
    next if ( !/NAMESPACE_ALIAS_LEX +shift, and go to state [0-9]+/);
    $_ = <>; chop;
    next if ( !/IDENTIFIER_LEX  +shift, and go to state [0-9]+/);
    $_ = <>; chop;
    next if ( !/TYPEDEF_NAME_LEX +shift, and go to state [0-9]+/);
    $_ = <>; chop;
    next if ( !/CLASS_NAME_LEX  +shift, and go to state [0-9]+/);
    $_ = <>; chop;
    next if ( !/TEMPLATE_NAME_LEX +shift, and go to state [0-9]+/);
    $_ = <>; chop;
    next if ( !/ENUM_NAME_LEX +shift, and go to state [0-9]+/);
    $_ = <>; chop;
    next if ( !/COLONCOLON_MK  +shift, and go to state [0-9]+/);
    $_ = <>; chop;
    next if ( !/OPERATOR_KW +shift, and go to state [0-9]+/);
    $_ = <>; chop;
    next if ( !/'\(' +shift, and go to state [0-9]+/);
    $_ = <>; chop;
    next if ( !/'\*' +shift, and go to state [0-9]+/);
    $_ = <>; chop;
    next if ( !/'&' +shift, and go to state [0-9]+/);
    $_ = <>; chop;
    next if ( !/'\[' +shift, and go to state [0-9]+/);
    $_ = <>; chop;
    next if ( !/'~'  +shift, and go to state [0-9]+/);
    $_ = <>;
    $_ = <>; chop;
    next if ( !/ORIGINAL_NAMESPACE_NAME_LEX  \[reduce using rule ([0-9]+) \(enter_parameter\)\]/);
    $yyy = $1;
    goto label;
}

print STDERR "Error detected at $0\n";
exit 1;

label:
print <<EOF
  if (yystate == $xxx) {
    using namespace cxx_compiler;
    if (scope::current->m_id == scope::PARAM) {
      switch (yychar) {
      case ORIGINAL_NAMESPACE_NAME_LEX:
      case NAMESPACE_ALIAS_LEX:
      case TYPEDEF_NAME_LEX:
      case CLASS_NAME_LEX:
      case TEMPLATE_NAME_LEX:
      case ENUM_NAME_LEX:
      case COLONCOLON_MK:
        YYDPRINTF((stderr, "rule.05 is applied\\n"));
        yyn = $yyy + 1;
        goto yyreduce;
      }
    }
  }
EOF
    ;
exit;
