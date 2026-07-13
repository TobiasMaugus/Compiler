all:
	bison -d parser.y
	flex lexer.l
	gcc parser.tab.c lex.yy.c -o compiler
	gcc backend.c -o codegen

run:
	./compiler test.txt
	./codegen output.tac
	gcc output.s -o executable -lc -no-pie

exec:
	@echo "\n--- Executing the final program ---"
	./executable

clean:
	rm -f compiler codegen parser.tab.c parser.tab.h lex.yy.c executable