print <<EOF
     switch (yytype) {
		case 174: /* declarator */
			using namespace std;
                        YYDPRINTF((stderr, "rule.11 is applied\\n"));
			cxx_compiler::parse::g_read.m_token.push_front(make_pair(')',cxx_compiler::file_t()));
			cxx_compiler::parse::g_read.m_token.push_front(make_pair(IDENTIFIER_LEX,cxx_compiler::file_t()));
			cxx_compiler::parse::g_read.m_lval.push_front((yyvaluep+2)->m_var);
			cxx_compiler::parse::g_read.m_token.push_front(make_pair('(',cxx_compiler::file_t()));
			break;
		case 136: /* decl_specifier_seq */
			using namespace std;
                        YYDPRINTF((stderr, "rule.11 is applied\\n"));
			cxx_compiler::parse::g_read.m_token.push_front(make_pair(INT_KW,cxx_compiler::file_t()));
			break;
     }
EOF
