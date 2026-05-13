# =============================================================================
# Makefile  —  CS-346 Mini-Compiler
# =============================================================================
# Targets:
#   all          - build everything
#   lexer        - standalone lexer test binary
#   postfix_eval - standalone postfix evaluator
#   parsers      - postfix, prefix, infix parser binaries
#   first        - FIRST sets program
#   follow       - FOLLOW sets program
#   ll1_table    - LL(1) table program
#   semantic     - compile semantic object files
#   ir           - compile IR generator object files
#   optimizer    - compile optimiser object files
#   perf         - performance comparison binary
#   pipeline     - integrated compiler binary (requires flex/bison)
#   test         - run all module tests
#   clean        - remove all generated files
# =============================================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -g -I.
LDFLAGS = -lm

# Windows (WinFlexBison installed via winget) — adjust path if different
ifeq ($(OS),Windows_NT)
    WINFB   = $(LOCALAPPDATA)/Microsoft/WinGet/Packages/WinFlexBison.win_flex_bison_Microsoft.Winget.Source_8wekyb3d8bbwe
    FLEX    = "$(WINFB)/win_flex.exe"
    BISON   = "$(WINFB)/win_bison.exe"
else
    FLEX    = flex
    BISON   = bison
endif

# On Linux add -lfl; on Windows (no fl library) omit it.
ifeq ($(OS),Windows_NT)
    FLEX_LIB =
else
    FLEX_LIB = -lfl
endif

# =============================================================================
# Phony targets
# =============================================================================
.PHONY: all clean test lexer postfix_eval parsers first follow ll1_table \
        semantic ir optimizer perf pipeline

all: postfix_eval first follow ll1_table parsers perf

# =============================================================================
# Module 1 — Lexer
# =============================================================================

# Standalone lexer (no Bison, just prints tokens)
lexer/lex_standalone.yy.c: lexer/lexer.l
	$(FLEX) -o $@ lexer/lexer.l

lexer: lexer/lex_standalone.yy.c
	$(CC) $(CFLAGS) -o lexer_test lexer/lex_standalone.yy.c $(FLEX_LIB) $(LDFLAGS)

postfix_eval: lexer/postfix_eval.c
	$(CC) $(CFLAGS) -o postfix_eval lexer/postfix_eval.c

# =============================================================================
# Module 2 & 3 — Parsers (requires Flex + Bison)
# =============================================================================

# Generate parser tab files
parser/infix.tab.c parser/infix.tab.h: parser/infix.y
	$(BISON) -d -o parser/infix.tab.c parser/infix.y

parser/postfix.tab.c parser/postfix.tab.h: parser/postfix.y
	$(BISON) -d -o parser/postfix.tab.c parser/postfix.y

parser/prefix.tab.c parser/prefix.tab.h: parser/prefix.y
	$(BISON) -d -o parser/prefix.tab.c parser/prefix.y

# Lexer for calculators
parser/calc_lex.yy.c: parser/calc_lex.l parser/infix.tab.h
	$(FLEX) -o parser/calc_lex.yy.c parser/calc_lex.l

parsers: parser/infix.tab.c parser/calc_lex.yy.c \
         parser/postfix.tab.c parser/prefix.tab.c \
         common/ast.c
	$(CC) $(CFLAGS) -Iparser -o infix_calc \
	    parser/infix.tab.c parser/calc_lex.yy.c common/ast.c \
	    $(FLEX_LIB) $(LDFLAGS)
	$(CC) $(CFLAGS) -Iparser -o postfix_calc \
	    parser/postfix.tab.c parser/calc_lex.yy.c common/ast.c \
	    $(FLEX_LIB) $(LDFLAGS)
	$(CC) $(CFLAGS) -Iparser -o prefix_calc \
	    parser/prefix.tab.c parser/calc_lex.yy.c common/ast.c \
	    $(FLEX_LIB) $(LDFLAGS)

# =============================================================================
# Module 4 — First / Follow / LL(1)
# =============================================================================

first_follow/grammar.o: first_follow/grammar.c first_follow/grammar.h
	$(CC) $(CFLAGS) -c -o $@ $<

first: first_follow/first.c first_follow/grammar.o
	$(CC) $(CFLAGS) -o first_sets first_follow/first.c first_follow/grammar.o

follow: first_follow/follow.c first_follow/grammar.o
	$(CC) $(CFLAGS) -o follow_sets first_follow/follow.c first_follow/grammar.o

ll1_table: first_follow/ll1_table.c first_follow/grammar.o
	$(CC) $(CFLAGS) -o ll1_table first_follow/ll1_table.c first_follow/grammar.o

# =============================================================================
# Module 5 — Semantic analysis object files
# =============================================================================

semantic/symtab.o: semantic/symtab.c semantic/symtab.h common/ast.h
	$(CC) $(CFLAGS) -c -o $@ $<

semantic/typechecker.o: semantic/typechecker.c semantic/typechecker.h \
                        common/ast.h common/error.h semantic/symtab.h
	$(CC) $(CFLAGS) -c -o $@ $<

semantic/scopechecker.o: semantic/scopechecker.c semantic/scopechecker.h \
                         common/ast.h common/error.h semantic/symtab.h
	$(CC) $(CFLAGS) -c -o $@ $<

semantic: semantic/symtab.o semantic/typechecker.o semantic/scopechecker.o

# =============================================================================
# Module 6 — IR generator object files
# =============================================================================

ir/tac.o: ir/tac.c ir/tac.h
	$(CC) $(CFLAGS) -c -o $@ $<

ir/irgen.o: ir/irgen.c ir/irgen.h common/ast.h ir/tac.h
	$(CC) $(CFLAGS) -c -o $@ $<

ir: ir/tac.o ir/irgen.o

# =============================================================================
# Module 7 — Optimiser object files
# =============================================================================

optimizer/optimizer.o: optimizer/optimizer.c optimizer/optimizer.h ir/tac.h
	$(CC) $(CFLAGS) -c -o $@ $<

optimizer/const_fold.o: optimizer/const_fold.c optimizer/optimizer.h ir/tac.h
	$(CC) $(CFLAGS) -c -o $@ $<

optimizer/const_prop.o: optimizer/const_prop.c optimizer/optimizer.h ir/tac.h
	$(CC) $(CFLAGS) -c -o $@ $<

optimizer/cse.o: optimizer/cse.c optimizer/optimizer.h ir/tac.h
	$(CC) $(CFLAGS) -c -o $@ $<

optimizer/dead_code.o: optimizer/dead_code.c optimizer/optimizer.h ir/tac.h
	$(CC) $(CFLAGS) -c -o $@ $<

optimizer/loop_opt.o: optimizer/loop_opt.c optimizer/optimizer.h ir/tac.h
	$(CC) $(CFLAGS) -c -o $@ $<

optimizer/control_flow.o: optimizer/control_flow.c optimizer/optimizer.h ir/tac.h
	$(CC) $(CFLAGS) -c -o $@ $<

OPT_OBJS = optimizer/optimizer.o optimizer/const_fold.o optimizer/const_prop.o \
           optimizer/cse.o optimizer/dead_code.o optimizer/loop_opt.o \
           optimizer/control_flow.o

optimizer: $(OPT_OBJS)

# Performance comparison standalone binary
common/error.o: common/error.c common/error.h
	$(CC) $(CFLAGS) -c -o $@ $<

common/ast.o: common/ast.c common/ast.h
	$(CC) $(CFLAGS) -c -o $@ $<

perf: optimizer ir common/error.o
	$(CC) $(CFLAGS) -o perf_compare \
	    optimizer/perf.c \
	    $(OPT_OBJS) \
	    ir/tac.o \
	    common/error.o \
	    $(LDFLAGS)

# =============================================================================
# Integrated pipeline (requires flex + bison to generate parser)
# =============================================================================

pipeline: parser/infix.tab.c parser/calc_lex.yy.c \
          common/ast.o common/error.o \
          semantic ir optimizer
	$(CC) $(CFLAGS) -DBISON_MODE \
	    -o compiler \
	    main.c \
	    parser/infix.tab.c parser/calc_lex.yy.c \
	    common/ast.o common/error.o \
	    semantic/symtab.o semantic/typechecker.o semantic/scopechecker.o \
	    ir/tac.o ir/irgen.o \
	    $(OPT_OBJS) \
	    $(FLEX_LIB) $(LDFLAGS)

# =============================================================================
# Tests
# =============================================================================

test: postfix_eval first follow ll1_table
	@echo ""
	@echo "=== Test: postfix evaluator ==="
	echo "3 4 + 2 *" | ./postfix_eval
	@echo ""
	@echo "=== Test: FIRST sets ==="
	./first_sets
	@echo ""
	@echo "=== Test: FOLLOW sets ==="
	./follow_sets
	@echo ""
	@echo "=== Test: LL(1) table ==="
	./ll1_table

# =============================================================================
# Clean
# =============================================================================

clean:
	rm -f lexer_test postfix_eval infix_calc postfix_calc prefix_calc
	rm -f first_sets follow_sets ll1_table perf_compare compiler
	rm -f parser/infix.tab.c parser/infix.tab.h
	rm -f parser/postfix.tab.c parser/postfix.tab.h
	rm -f parser/prefix.tab.c parser/prefix.tab.h
	rm -f parser/calc_lex.yy.c lexer/lex_standalone.yy.c
	rm -f common/*.o semantic/*.o ir/*.o optimizer/*.o first_follow/*.o
	rm -f *.out *.out.c
