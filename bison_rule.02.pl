#---------------------------------------------
#State XXX
#
# AAA ptr_operator: '&' .
# BBB unary_operator: '&' .
#
#---------------------------------------------
#state YYY
#
#  CCC unary_operator: '&' .
#
#---------------------------------------------
while ( <> ){
    chop;
    if ( /^[Ss]tate (.*)/ ){
        $state = $1;
    }
    if ( $x_flag == 0 ){
        if ( /ptr_operator: '\&' \./ || /ptr_operator  \->  '\&' \./ ){
            $x_flag = 1;
        }
    }
    else {
        if ( $x_flag == 1 ){
            if ( /unary_operator: '\&' \./ || /unary_operator  \->  '\&' \./ ){
                $xxx = $state;
                $x_flag = 0;
            }
            else {
                $x_flag = 0;
            }
            next;
        }
        else {
            $x_flag = 0;
        }
        next;
    }
    if ( $y_flag == 0 ){
        if ( /^$/ ){
            $y_flag = 1;
        }
    }
    else {
        if ( $y_flag == 1 ){
            if ( /unary_operator: '\&' \./ || /unary_operator  \->  '\&' \./ ){
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
    if ( cxx_compiler::parse::identifier::flag != cxx_compiler::parse::identifier::new_obj )
      yystate = $yyy;
  }
EOF
