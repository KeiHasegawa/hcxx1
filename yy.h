#ifndef _YY_H_
#define _YY_H_

extern "C" int cxx_compiler_wrap();
extern int cxx_compiler_lex();
extern void cxx_compiler_error(const char*);
extern char* cxx_compiler_text;
extern FILE* cxx_compiler_in;
extern int cxx_compiler_parse();
extern int cxx_compiler_char;
#ifdef YYDEBUG
extern int cxx_compiler_debug;
#endif // YYDEBUG

#endif // _YY_H_
