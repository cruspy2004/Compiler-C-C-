================================================================================
CS-346 Compiler Construction  —  Mini-Compiler Project
NUST SEECS | BSCS-2023-CD
Author: Muhammad Haadhee Sheeraz Mian (Reg No: 478359)
================================================================================

PREREQUISITES
-------------
  Linux  : sudo apt-get install gcc flex bison clang
  Windows: Install MSYS2 (https://www.msys2.org/), then:
             pacman -S mingw-w64-x86_64-gcc flex bison clang

DIRECTORY STRUCTURE
-------------------
  project/
    Makefile             top-level build
    README.txt           this file
    main.c               integrated pipeline entry point
    common/              shared headers: ast.h  token.h  error.h/c
    lexer/               Module 1  — Flex lexer + postfix evaluator
    parser/              Modules 2 & 3 — Bison parsers (postfix/prefix/infix)
    first_follow/        Module 4  — FIRST / FOLLOW / LL(1) table
    semantic/            Module 5  — symbol table, type checker, scope checker
    ir/                  Module 6  — TAC IR generator
    optimizer/           Module 7  — optimisation passes + performance timing
    llvm/                Module 8  — LLVM test programs

BUILD INSTRUCTIONS
------------------
From inside the project/ directory:

  Build everything (non-parser targets):
    make all

  Build and run tests:
    make test

  Build individual modules:
    make postfix_eval       # standalone postfix evaluator
    make first              # FIRST sets
    make follow             # FOLLOW sets
    make ll1_table          # LL(1) parsing table
    make parsers            # all calculator parsers (needs flex + bison)
    make perf               # performance comparison
    make pipeline           # full integrated compiler (needs flex + bison)

  Clean generated files:
    make clean

RUNNING EACH MODULE
-------------------
Module 1 — Lexer:
  ./postfix_eval           (type: 3 4 + 2 * <Enter>)
  echo "3 4 + 2 *" | ./postfix_eval

Module 2/3 — Parsers (after `make parsers`):
  echo "4 + 8 * 2" | ./infix_calc
  echo "+ 4 8"     | ./prefix_calc
  echo "4 8 +"     | ./postfix_calc
  echo "2 ^ 3 ^ 2" | ./infix_calc       # right-assoc: 2^(3^2) = 512
  echo "log(2.718281828)" | ./infix_calc

Module 4 — First/Follow/LL(1):
  ./first_sets
  ./follow_sets
  ./ll1_table

Module 7 — Performance comparison:
  ./perf_compare

Module 8 — LLVM IR generation (requires clang):
  cd llvm
  clang test1.c -S -emit-llvm -o test1.ll
  clang test1.c -S -emit-llvm -O3 -o test1_O3.ll
  clang test2.c -S -emit-llvm -o test2.ll
  clang test2.c -S -emit-llvm -O3 -o test2_O3.ll

Full pipeline (after `make pipeline`):
  ./compiler <source-file.lang>

EXPECTED TEST OUTPUTS
---------------------
  echo "3 4 + 2 *" | ./postfix_eval
    -> Result: 14

  ./first_sets
    FIRST(E)  = { (, i, n }
    FIRST(E') = { +, -, epsilon }
    FIRST(T)  = { (, i, n }
    FIRST(T') = { *, /, epsilon }
    FIRST(F)  = { (, i, n }

  ./follow_sets
    FOLLOW(E)  = { ), $ }
    FOLLOW(E') = { ), $ }
    FOLLOW(T)  = { +, -, ), $ }
    FOLLOW(T') = { +, -, ), $ }
    FOLLOW(F)  = { *, /, +, -, ), $ }

NOTES
-----
- The lexer.l and parser .y files are designed for flex/bison.
  On Windows, substitute 'flex' -> 'win_flex' and 'bison' -> 'win_bison'
  in the Makefile (or set FLEX=win_flex BISON=win_bison on the command line).
- The integrated pipeline (main.c) calls clang as an external tool for
  the final native binary step (Module 8).
- All error messages go to stderr; TAC/result output goes to stdout.

================================================================================
END OF README
================================================================================
