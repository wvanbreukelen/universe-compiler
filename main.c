#include <stdio.h>
#include "lang.h"
#include "codegen/codegen.h"

int main(int argc, char* argv[]) {
  mpc_parser_t* lexer = init_lexer();

  mpc_result_t r;

  if (argc > 1) {
    if (mpc_parse_contents(argv[1], lexer, &r)) {
      mpc_ast_print(r.output);
      visitor_start(r.output);


      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
  }



  return 0;
}