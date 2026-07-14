%option noyywrap yylineno

%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.tab.h"

#define YY_USER_ACTION \
    yylloc.first_line = yylineno; \
    yylloc.last_line = yylineno; \
    yylloc.first_column = column; \
    yylloc.last_column = column + yyleng - 1;

/* coluna atual do arquivo */
int column = 1;
int lexical_error = 0;

/* linha e coluna iniciais do comentário*/
int comment_start_line = 1;
int comment_start_column = 1;

void print_token(char* token, char* lexema);
char* process_char_literal(const char *raw, int len);
char* process_string_literal(const char *raw, int len);

%}

/* ------------------------ DEFINIÇÕES ------------------------ */

DIGIT      [0-9]
LETTER     [a-zA-Z]

ID         {LETTER}({LETTER}|{DIGIT})*

INTNUM     {DIGIT}+
FLOATNUM   {DIGIT}+"."{DIGIT}+

%x COMMENT

%%
%{
/* ----------------------- PALAVRAS-CHAVE ------------------- */
%}

"int" {
    print_token("KW_INT", yytext);
    column += yyleng;
    return KW_INT;
}

"float" {
    print_token("KW_FLOAT", yytext);
    column += yyleng;
    return KW_FLOAT;
}

"char" {
    print_token("KW_CHAR", yytext);
    column += yyleng;
    return KW_CHAR;
}

"string" {
    print_token("KW_STRING", yytext);
    column += yyleng;
    return KW_STRING;
}

"if" {
    print_token("KW_IF", yytext);
    column += yyleng;
    return KW_IF;
}

"else" {
    print_token("KW_ELSE", yytext);
    column += yyleng;
    return KW_ELSE;
}

"while" {
    print_token("KW_WHILE", yytext);
    column += yyleng;
    return KW_WHILE;
}

"print" {
    print_token("KW_PRINT", yytext);
    column += yyleng;
    return KW_PRINT;
}

"read" {
    print_token("KW_READ", yytext);
    column += yyleng;
    return KW_READ;
}

"return" {
    print_token("KW_RETURN", yytext);
    column += yyleng;
    return KW_RETURN;
}

"void" {
    print_token("KW_VOID", yytext);
    column += yyleng;
    return KW_VOID;
}

%{
/* ------------------------ OPERADORES ----------------------- */
%}

"==" { print_token("EQ", yytext); column += yyleng; return EQ; }
"!=" { print_token("NE", yytext); column += yyleng; return NE; }
"<=" { print_token("LE", yytext); column += yyleng; return LE; }
">=" { print_token("GE", yytext); column += yyleng; return GE; }
"<"  { print_token("LT", yytext); column += yyleng; return LT; }
">"  { print_token("GT", yytext); column += yyleng; return GT; }
"&&" { print_token("AND", yytext); column += yyleng; return AND; }
"||" { print_token("OR", yytext); column += yyleng; return OR; }
"!"  { print_token("NOT", yytext); column += yyleng; return NOT; }
"="  { print_token("ASSIG", yytext); column += yyleng; return ASSIG; }
"+"  { print_token("PLUS", yytext); column += yyleng; return PLUS; }
"-"  { print_token("MINUS", yytext); column += yyleng; return MINUS; }
"*"  { print_token("MULT", yytext); column += yyleng; return MULT; }
"/"  { print_token("DIV", yytext); column += yyleng; return DIV; }
"%"  { print_token("MOD", yytext); column += yyleng; return MOD; }

%{
/* ------------------------ DELIMITADORES ------------------------ */
%}

";" { print_token("SEMCOL", yytext); column += yyleng; return SEMCOL; }
"," { print_token("COMMA", yytext); column += yyleng; return COMMA; }
"(" { print_token("LPAREN", yytext); column += yyleng; return LPAREN; }
")" { print_token("RPAREN", yytext); column += yyleng; return RPAREN; }
"{" { print_token("LBRACE", yytext); column += yyleng; return LBRACE; }
"}" { print_token("RBRACE", yytext); column += yyleng; return RBRACE; }

%{
/* ------------------------ LITERAIS & IDENTIFICADORES ------------------------ */
%}

\'([^\'\\]|\\.)\' {
    char *val = process_char_literal(yytext + 1, yyleng - 2);
    if (!val) {
        fprintf(stderr,
            "[LEXICAL ERROR] Invalid character literal '%s' at Line %d, Column %d\n",
            yytext, yylloc.first_line, yylloc.first_column);
        lexical_error = 1;
        column += yyleng;
        return 0;
    }
    print_token("CHARLIT", val);
    yylval.ival = (unsigned char)val[0];
    free(val);
    column += yyleng;
    return CHARLIT;
}

\'\' {
    fprintf(stderr,
        "[LEXICAL ERROR] Empty character literal at Line %d, Column %d\n",
        yylloc.first_line, yylloc.first_column);
    lexical_error = 1;
    column += yyleng;
    return 0;
}

\'([^\'\n])* {
    fprintf(stderr,
        "[LEXICAL ERROR] Unterminated character literal at Line %d, Column %d\n",
        yylloc.first_line, yylloc.first_column);
    lexical_error = 1;
    column += yyleng;
    return 0;
}

\"([^\"\\]|\\.)*\" {
    // Processa a string ignorando as aspas de abertura e de fecho (+1 e -2)
    char *val = process_string_literal(yytext + 1, yyleng - 2);
    if (!val) {
        fprintf(stderr,
            "[LEXICAL ERROR] Invalid string literal at Line %d, Column %d\n",
            yylloc.first_line, yylloc.first_column);
        lexical_error = 1;
        column += yyleng;
        return 0;
    }
    print_token("STRINGLIT", val);
    yylval.str = val;
    column += yyleng;
    return STRINGLIT;
}

\"([^\"\\]|\\.)* {
    fprintf(stderr,
        "[LEXICAL ERROR] Unterminated string literal at Line %d, Column %d\n",
        yylloc.first_line, yylloc.first_column);
    lexical_error = 1;
    column += yyleng;
    return 0;
}

{FLOATNUM} {
    print_token("FLOATNUM", yytext);
    column += yyleng;
    yylval.fval = atof(yytext); /* PASSA O VALOR PARA O PARSER */
    return FLOATNUM;
}

{INTNUM} {
    print_token("INTNUM", yytext);
    column += yyleng;
    yylval.ival = atoi(yytext); /* PASSA O VALOR PARA O PARSER */
    return INTNUM;
}

{ID} {
    print_token("ID", yytext);
    column += yyleng;
    yylval.str = strdup(yytext); /* PASSA A STRING PARA O PARSER */
    return ID;
}

%{
/* ------------------------ ESPAÇOS E COMENTÁRIOS ------------------------ */
%}

[ \t\r]+ { column += yyleng; }

\n       { column = 1; }

"//".* { column += yyleng; }

"/*" { 
   comment_start_line = yylineno;
   comment_start_column = column;
   column += 2; 
   BEGIN(COMMENT);
}

"*/" {
    fprintf(stderr,
        "[LEXICAL ERROR] Unopened multiline comment '*/' at Line %d, Column %d\n",
        yylloc.first_line,
        yylloc.first_column
    );
    lexical_error = 1;
    return 0;
}

<COMMENT>{
    "*/" { column += 2; BEGIN(INITIAL); }
    \n   { column = 1; }
    .    { column += yyleng; }
    <<EOF>> {
        fprintf(stderr,
            "[LEXICAL ERROR] Unterminated comment started at Line %d, Column %d\n",
            comment_start_line,
            comment_start_column
        );
        lexical_error = 1;
        return 0;
    }
}

%{
/* ------------------------ ERROS LÉXICOS ------------------------ */
%}

{DIGIT}+{LETTER}({LETTER}|{DIGIT})* {
    fprintf(stderr,
        "[LEXICAL ERROR] Invalid identifier '%s' at Line %d, Column %d\n",
        yytext,
        yylloc.first_line,
        yylloc.first_column
    );
    lexical_error = 1;
    return 0;
}

. {
    fprintf(stderr,
        "[LEXICAL ERROR] Unexpected character '%s' at Line %d, Column %d\n",
        yytext,
        yylloc.first_line,
        yylloc.first_column
    );
    lexical_error = 1;
    return 0;
}

%%

/* ------------------------ IMPRESSÃO DOS TOKENS E PROCESSAMENTO ------------------------ */

void print_token(char* token, char* lexema) {
    printf("%-15s | %-15s | %-5d | %-5d\n",
           token,
           lexema,
           yylloc.first_line,
           yylloc.first_column);
}

static int decode_escape_char(const char *s, int len, char *out) {
    if (len <= 0) return 0;
    if (len == 1) {
        *out = s[0];
        return 1;
    }
    if (s[0] != '\\') return 0;
    switch (s[1]) {
        case 'n': *out = '\n'; return 1;
        case 't': *out = '\t'; return 1;
        case 'r': *out = '\r'; return 1;
        case '\\': *out = '\\'; return 1;
        case '"': *out = '"'; return 1;
        case '\'': *out = '\''; return 1;
        case '0': *out = '\0'; return 1;
        default: return 0;
    }
}

char* process_char_literal(const char *raw, int len) {
    char *result = (char*)malloc(2);
    if (!result) return NULL;
    if (!decode_escape_char(raw, len, result)) {
        free(result);
        return NULL;
    }
    result[1] = '\0';
    return result;
}

char* process_string_literal(const char *raw, int len) {
    char *result = (char*)malloc(len + 1);
    if (!result) return NULL;

    int j = 0;
    for (int i = 0; i < len; i++) {
        if (raw[i] == '\\') {
            char ch;
            if (i + 1 >= len || !decode_escape_char(raw + i, 2, &ch)) {
                free(result);
                return NULL;
            }
            result[j++] = ch;
            i++;
        } else {
            result[j++] = raw[i];
        }
    }
    result[j] = '\0';
    return result;
}