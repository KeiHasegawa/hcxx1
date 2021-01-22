#State XXX1
#
#  AAA1 template_id: TEMPLATE_NAME_LEX '<' enter_templ_arg leave_templ_arg . '>'
#
#State XXX2
#
#  AAA2 template_id: TEMPLATE_NAME_LEX '<' enter_templ_arg template_argument_list leave_templ_arg . '>'
#
#State XXX3
#
#  AAA3 operator_function_id: OPERATOR_KW operator '<' enter_templ_arg leave_templ_arg . '>'
#
#State XXX4
#
#  AAA4 template_id: TEMPLATE_NAME_LEX '<' enter_templ_arg template_argument_list DOTS_MK leave_templ_arg . '>'
#
#State XXX5
#
#  AAA5 operator_function_id: OPERATOR_KW operator '<' enter_templ_arg template_argument_list leave_templ_arg . '>'
# 
# Example:
#  template<class C1> struct S1 { /* ... */ };
#  template<class C2> struct S2 { /* ... */ };
#  ...
#  S1<S2<int>> x;
#

while (<>) {
    chop;
    next if ( !/^[Ss]tate (.*)/ );
    $xxx[$cnt] = $1;
    $_ = <>;
    $_ = <>; chop;
    next if (!/([0-9]+) .* leave_templ_arg \. '>'/);
    $aaa = $1;
    ++$cnt;
}

for ($i = 0; $i != $cnt; ++$i){
    if ($i == 0) {
      print "  if (";
    }
    else {
      print " || ";
    }
    print "yystate == ", $xxx[$i];
}

print<<EOF
) {
    if (yychar == RSH_MK) {
      using namespace cxx_compiler;
      YYDPRINTF((stderr, "patch.21 is applied\\n"));
      if (!parse::templ::save_t::nest.empty()) {
        using namespace parse::templ;
        save_t* p = save_t::nest.back();
        pair<int, file_t>& b = p->m_read.m_token.back();
        assert(b.first == RSH_MK);
        b.first = '>';
      }
      parse::g_read.m_token.push_front(make_pair('>', parse::position));
      yychar = '>';
    }
  }
EOF
