cmake_minimum_required(VERSION 3.0)
project(compiler)

add_executable(
  compiler
  token.c
  lexer.c
  parser.c
  ast.c
  main.c
  defs.h
  sema.c
)
