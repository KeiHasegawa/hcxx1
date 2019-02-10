#---------------------------------------------
#State XXX
#
#  AAA declarator_id: id_expression .
#  BBB primary_expression: id_expression .
#
#---------------------------------------------
#state YYY
#
#  CCC primary_expression: id_expression .
#
#---------------------------------------------

while ( <> ){
    chop;
    if ( /^[Ss]tate (.*)/ ){
        $state = $1;
    }
    if ( $x_flag == 0 ){
        if ( /declarator_id: id_expression \./ ||
	     /declarator_id  \->  id_expression \./ ){
            $x_flag = 1;
        }
    }
    else {
        if ( $x_flag == 1 ){
            if ( /primary_expression: id_expression \./ ||
                 /primary_expression  \->  id_expression \./ ){
                $xxx = $state;
                $x_flag = 0;
            }
            else {
                $x_flag = 0;
            }
            next;
        }
    }
    if ( $y_flag == 0 ){
        if ( /^$/ ){
            $y_flag = 1;
        }
    }
    else {
        if ( $y_flag == 1 ){
            if ( /primary_expression: id_expression \./ || 
                 /primary_expression  \->  id_expression \./ ){
		 $y_flag = 2;
	     }
	     else {
		 $y_flag = 0;
	     }
	 }
	 else {
	     if ( $y_flag == 2 ){
		 if ( /^$/ ){
		     $yyy = $state;
		     $y_flag = 0;
                }
                else {
                    $y_flag = 0;
                }
                next;
            }
        }
    }
}

print <<EOF
  if ( yystate == $xxx ){
    if ( cxx_compiler::parse::identifier::flag != cxx_compiler::parse::identifier::new_obj ) {
      YYDPRINTF((stderr, "rule.00 is applied\\n"));
      yystate = $yyy;
    }
  }
EOF
