// comma-expression
#include "stdafx.h"
#include "cxx_core.h"
#include "cxx_impl.h"

cxx_compiler::var* cxx_compiler::var::comma(var* right)
{
  rvalue();
  return right->rvalue();
}
