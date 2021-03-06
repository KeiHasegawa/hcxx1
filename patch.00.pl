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
#
#  void f()
#  {
#    int n;  // Declaration statement. Use `declarator_id -> id_expression'
#    n;  // Expression statement. Use `primary_expression -> id_expression'
#  }
#

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

if ($xxx == 0 || $yyy == 0) {
  print STDERR "Error detected at $0\n";
  exit 1;
}

print <<EOF
  if ( yystate == $xxx ){
    using namespace cxx_compiler::parse;
    if (identifier::mode != identifier::new_obj) {
      YYDPRINTF((stderr, "patch.00 is applied\\n"));
      yystate = $yyy;
    }
  }
EOF
