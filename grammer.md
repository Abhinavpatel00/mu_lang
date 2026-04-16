

program         = { global_decl } function_def
global_decl     = identifier "::" expr ";"
function_def    = identifier "::" "(" ")" block
block           = "{" { stmt } "}"
stmt            = var_decl | assignment
var_decl        = identifier ":" type "=" expr ";"
                | identifier ":=" expr ";"
                | identifier "::" expr ";"   // local constant
type            = "i32" | "f32" | "string" | identifier
expr            = literal | identifier
literal         = integer_literal | float_literal | string_literal
