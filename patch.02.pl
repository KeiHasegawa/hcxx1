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
#
#  Example:
#
#  void f(int n)
#  {
#    int& r = n;  // Declaration statement. Use ptr_operator -> '&'
#    &r; // Expression statement. Use unary_operator -> '&'
#  }

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

if ($xxx == 0 || $yyy == 0) {
  print STDERR "Error detected at $0\n";
  exit 1;
}

print <<EOF
  if ( yystate == $xxx ){
    using namespace cxx_compiler::parse;
    if (identifier::mode != identifier::new_obj) {
      YYDPRINTF((stderr, "patch.02 is applied\\n"));
      yystate = $yyy;
    }
  }
EOF
