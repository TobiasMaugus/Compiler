%{
/* Analisador Sintático, Semântico e Geração de Código Intermediário (TAC) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

extern int yylex();

extern int yylineno;
extern int column;
extern int lexical_error;
extern char *yytext;

extern FILE *yyin;

int syntax_error = 0;
int semantic_error = 0;

void yyerror(const char *s);
void syntax_error_msg_bef(const char *msg);
void print_symbol_table();

FILE *out = NULL;

/* ========================================================================= */
/* ESTRUTURAS DA ÁRVORE SINTÁTICA ABSTRATA (AST)                             */
/* ========================================================================= */

typedef enum {
    AST_VAR_DECL, AST_FUNC_DECL, AST_TYPE,
    AST_PARAM, AST_COMPOUND, AST_IF, AST_WHILE, AST_PRINT, AST_READ,
    AST_RETURN, AST_ASSIGN, AST_BINOP, AST_UNOP, AST_ID, AST_INT, AST_FLOAT,
    AST_FUNC_CALL
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char *lexema;
    int int_val;
    float float_val;
    
    int line;    
    int column;
    char exp_type[20]; 
    char temp[20];     
    
    struct ASTNode *child1;
    struct ASTNode *child2;
    struct ASTNode *child3;
    struct ASTNode *child4;
    struct ASTNode *next;
} ASTNode;

ASTNode *root = NULL;

ASTNode* create_node(ASTNodeType type, ASTNode *c1, ASTNode *c2, ASTNode *c3, ASTNode *c4) {
    ASTNode *n = (ASTNode*)malloc(sizeof(ASTNode));
    n->type = type;
    n->lexema = NULL;
    n->int_val = 0;
    n->float_val = 0.0;
    
    /* Se tiver um primeiro filho, herda a linha/coluna dele para máxima precisão */
    if (c1) {
        n->line = c1->line;
        n->column = c1->column;
    } else {
        n->line = yylineno; 
        n->column = column;
    }
    
    n->child1 = c1;
    n->child2 = c2;
    n->child3 = c3;
    n->child4 = c4;
    n->next = NULL;
    return n;
}

ASTNode* create_leaf_id(char *lex, int line, int col) {
    ASTNode *n = (ASTNode*)malloc(sizeof(ASTNode));
    n->type = AST_ID;
    /* REDE DE SEGURANÇA: Evita Segfault se o lexer.l não mandar a string */
    n->lexema = lex ? strdup(lex) : strdup("UNKNOWN_ID");
    n->line = line;
    n->column = col;
    n->child1 = n->child2 = n->child3 = n->child4 = n->next = NULL;
    return n;
}

ASTNode* create_leaf_int(int val, int line, int col) {
    ASTNode *n = (ASTNode*)malloc(sizeof(ASTNode));
    n->type = AST_INT;
    n->int_val = val;
    n->line = line;
    n->column = col;
    n->child1 = n->child2 = n->child3 = n->child4 = n->next = NULL;
    return n;
}

ASTNode* create_leaf_float(float val, int line, int col) {
    ASTNode *n = (ASTNode*)malloc(sizeof(ASTNode));
    n->type = AST_FLOAT;
    n->float_val = val;
    n->line = line;
    n->column = col;
    n->child1 = n->child2 = n->child3 = n->child4 = n->next = NULL;
    return n;
}

/* ========================================================================= */
/* SISTEMA DE INSTRUÇÕES TAC EM MEMÓRIA (substitui escrita direta em FILE)    */
/* ========================================================================= */

typedef struct TACInstr {
    char text[512];
    struct TACInstr *next;
} TACInstr;

TACInstr *tac_head = NULL;
TACInstr *tac_tail = NULL;

void emit_tac(const char *fmt, ...) {
    TACInstr *instr = (TACInstr*)malloc(sizeof(TACInstr));
    va_list args;
    va_start(args, fmt);
    vsnprintf(instr->text, sizeof(instr->text), fmt, args);
    va_end(args);
    instr->next = NULL;
    if (!tac_tail) {
        tac_head = tac_tail = instr;
    } else {
        tac_tail->next = instr;
        tac_tail = instr;
    }
}

%}

%define parse.error detailed
%locations

%union {
    int ival;
    float fval;
    char *str;
    struct ASTNode *node;
}

/* ------------------------ TOKENS -------------------------- */

%token KW_INT KW_FLOAT
%token KW_IF KW_ELSE KW_WHILE
%token KW_PRINT KW_READ KW_RETURN
%token <str> ID
%token <ival> INTNUM
%token <fval> FLOATNUM
%token LT GT LE GE EQ NE
%token AND OR NOT
%token PLUS MINUS MULT DIV MOD
%token ASSIG
%token SEMCOL COMMA
%token LPAREN RPAREN LBRACE RBRACE

%type <node> program declaration_list declaration variable_declaration type_specifier function_declaration function_body local_declarations statements statement expression_stmt expression selection_stmt iteration_stmt io_stmt return_stmt statement_list compound_stmt parameters parameter_list parameter variable_list variable function_call arguments argument_list

/* ---------------- PRECEDÊNCIA ---------------- */

%nonassoc LOWER_THAN_ELSE
%nonassoc KW_ELSE

%right ASSIG
%left OR
%left AND
%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left MULT DIV MOD
%right NOT
%right UMINUS

%%

/* ========================================================================= */
/* REGRAS GRAMATICAIS E CONSTRUÇÃO DA AST                                    */
/* ========================================================================= */

program
    : declaration_list { root = $1; $$ = $1; }
    ;

declaration_list
    : declaration_list declaration
      {
          if ($1 != NULL) {
              ASTNode *t = $1;
              while (t->next != NULL) t = t->next;
              t->next = $2;
              $$ = $1;
          } else { $$ = $2; }
      }
    | declaration_list error SEMCOL { yyerrok; $$ = $1; }
    | declaration_list error RBRACE { yyerrok; $$ = $1; }
    | declaration { $$ = $1; }
    ;

declaration
    : variable_declaration { $$ = $1; }
    | function_declaration { $$ = $1; }
    ;

variable_declaration
    : type_specifier variable_list SEMCOL { $$ = create_node(AST_VAR_DECL, $1, $2, NULL, NULL); }
    | type_specifier error SEMCOL { syntax_error_msg_bef("Invalid variable declaration"); yyerrok; $$ = NULL; }
    ;

variable_list
    : variable_list COMMA variable
      {
          ASTNode *t = $1;
          while (t->next != NULL) t = t->next;
          t->next = $3;
          $$ = $1;
      }
    | variable { $$ = $1; }
    ;

variable
    : ID { $$ = create_leaf_id($1, @1.first_line, @1.first_column); }
    | ID ASSIG expression
      {
          ASTNode *id_node = create_leaf_id($1, @1.first_line, @1.first_column);
          $$ = create_node(AST_ASSIGN, id_node, $3, NULL, NULL);
      }
    ;

type_specifier
    : KW_INT { $$ = create_node(AST_TYPE, NULL, NULL, NULL, NULL); $$->lexema = strdup("int"); }
    | KW_FLOAT { $$ = create_node(AST_TYPE, NULL, NULL, NULL, NULL); $$->lexema = strdup("float"); }
    ;

function_declaration
    : type_specifier ID LPAREN parameters RPAREN function_body
      {
          ASTNode *id_node = create_leaf_id($2, @2.first_line, @2.first_column);
          $$ = create_node(AST_FUNC_DECL, $1, id_node, $4, $6);
      }
    | type_specifier ID LPAREN parameters error function_body
      {
          syntax_error_msg_bef("Missing ')'");
          ASTNode *id_node = create_leaf_id($2, @2.first_line, @2.first_column);
          $$ = create_node(AST_FUNC_DECL, $1, id_node, $4, $6);
      }
    ;

parameters
    : parameter_list { $$ = $1; }
    | /* vazio */ { $$ = NULL; }
    ;

parameter_list
    : parameter_list COMMA parameter
      {
          ASTNode *t = $1;
          while (t->next != NULL) t = t->next;
          t->next = $3;
          $$ = $1;
      }
    | parameter { $$ = $1; }
    ;

parameter
    : type_specifier ID { ASTNode *id_node = create_leaf_id($2, @2.first_line, @2.first_column); $$ = create_node(AST_PARAM, $1, id_node, NULL, NULL); }
    ;

function_body
    : LBRACE local_declarations statements return_stmt RBRACE
      { $$ = create_node(AST_COMPOUND, $2, $3, $4, NULL); }
    | LBRACE local_declarations return_stmt RBRACE
      { $$ = create_node(AST_COMPOUND, $2, NULL, $3, NULL); }
    | LBRACE local_declarations statements RBRACE
      { syntax_error = 1; fprintf(stderr, "\n[SYNTAX ERROR] Missing 'return'\n\n"); $$ = create_node(AST_COMPOUND, $2, $3, NULL, NULL); }
    | LBRACE local_declarations RBRACE
      { syntax_error = 1; fprintf(stderr, "\n[SYNTAX ERROR] Missing 'return'\n\n"); $$ = create_node(AST_COMPOUND, $2, NULL, NULL, NULL); }
    ;

local_declarations
    : local_declarations variable_declaration
      {
          if ($1 != NULL) {
              ASTNode *t = $1;
              while (t->next != NULL) t = t->next;
              t->next = $2;
              $$ = $1;
          } else { $$ = $2; }
      }
    | /* vazio */ { $$ = NULL; }
    ;

statements
    : statements statement
      {
          if ($1 != NULL) {
              ASTNode *t = $1;
              while (t->next != NULL) t = t->next;
              t->next = $2;
              $$ = $1;
          } else { $$ = $2; }
      }
    | statement { $$ = $1; }
    ;

statement
    : expression_stmt { $$ = $1; }
    | compound_stmt { $$ = $1; }
    | selection_stmt { $$ = $1; }
    | iteration_stmt { $$ = $1; }
    | io_stmt { $$ = $1; }
    | error SEMCOL { syntax_error_msg_bef("Invalid command"); yyerrok; $$ = NULL; }
    ;

return_stmt
    : KW_RETURN expression SEMCOL { $$ = create_node(AST_RETURN, $2, NULL, NULL, NULL); }
    | KW_RETURN SEMCOL { $$ = create_node(AST_RETURN, NULL, NULL, NULL, NULL); }
    ;

compound_stmt
    : LBRACE local_declarations statement_list RBRACE { $$ = create_node(AST_COMPOUND, $2, $3, NULL, NULL); }
    ;

statement_list
    : statements { $$ = $1; }
    | statements return_stmt
      {
          if ($1 != NULL) {
              ASTNode *t = $1;
              while (t->next != NULL) t = t->next;
              t->next = $2;
              $$ = $1;
          } else { $$ = $2; }
      }
    | return_stmt { $$ = $1; }
    | /* vazio */ { $$ = NULL; }
    ;

expression_stmt
    : expression SEMCOL { $$ = $1; }
    | SEMCOL { $$ = NULL; }
    ;

selection_stmt
    : KW_IF LPAREN expression RPAREN statement %prec LOWER_THAN_ELSE { $$ = create_node(AST_IF, $3, $5, NULL, NULL); }
    | KW_IF LPAREN expression RPAREN statement KW_ELSE statement { $$ = create_node(AST_IF, $3, $5, $7, NULL); }
    ;

iteration_stmt
    : KW_WHILE LPAREN expression RPAREN statement { $$ = create_node(AST_WHILE, $3, $5, NULL, NULL); }
    ;

io_stmt
    : KW_PRINT LPAREN expression RPAREN SEMCOL { $$ = create_node(AST_PRINT, $3, NULL, NULL, NULL); }
    | KW_READ LPAREN ID RPAREN SEMCOL { ASTNode *id_node = create_leaf_id($3, @3.first_line, @3.first_column);
      $$ = create_node(AST_READ, id_node, NULL, NULL, NULL); }
    ;

expression
    : ID ASSIG expression { ASTNode *id_node = create_leaf_id($1, @1.first_line, @1.first_column); $$ = create_node(AST_ASSIGN, id_node, $3, NULL, NULL); }
    | expression PLUS expression { $$ = create_node(AST_BINOP, $1, $3, NULL, NULL); $$->lexema = strdup("+"); }
    | expression MINUS expression { $$ = create_node(AST_BINOP, $1, $3, NULL, NULL); $$->lexema = strdup("-"); }
    | expression MULT expression { $$ = create_node(AST_BINOP, $1, $3, NULL, NULL); $$->lexema = strdup("*"); }
    | expression DIV expression { $$ = create_node(AST_BINOP, $1, $3, NULL, NULL); $$->lexema = strdup("/"); }
    | expression MOD expression { $$ = create_node(AST_BINOP, $1, $3, NULL, NULL); $$->lexema = strdup("%"); }
    | expression LT expression { $$ = create_node(AST_BINOP, $1, $3, NULL, NULL); $$->lexema = strdup("<"); }
    | expression GT expression { $$ = create_node(AST_BINOP, $1, $3, NULL, NULL); $$->lexema = strdup(">"); }
    | expression LE expression { $$ = create_node(AST_BINOP, $1, $3, NULL, NULL); $$->lexema = strdup("<="); }
    | expression GE expression { $$ = create_node(AST_BINOP, $1, $3, NULL, NULL); $$->lexema = strdup(">="); }
    | expression EQ expression { $$ = create_node(AST_BINOP, $1, $3, NULL, NULL); $$->lexema = strdup("=="); }
    | expression NE expression { $$ = create_node(AST_BINOP, $1, $3, NULL, NULL); $$->lexema = strdup("!="); }
    | expression AND expression { $$ = create_node(AST_BINOP, $1, $3, NULL, NULL); $$->lexema = strdup("&&"); }
    | expression OR expression { $$ = create_node(AST_BINOP, $1, $3, NULL, NULL); $$->lexema = strdup("||"); }
    | NOT expression { $$ = create_node(AST_UNOP, $2, NULL, NULL, NULL); $$->lexema = strdup("!"); $$->line = @1.first_line; $$->column = @1.first_column;}
    | MINUS expression %prec UMINUS { $$ = create_node(AST_UNOP, $2, NULL, NULL, NULL); $$->lexema = strdup("minus"); $$->line = @1.first_line; $$->column = @1.first_column;}
    | LPAREN expression RPAREN { $$ = $2; }
    | ID { $$ = create_leaf_id($1, @1.first_line, @1.first_column); }
    | INTNUM { $$ = create_leaf_int($1, @1.first_line, @1.first_column); }
    | FLOATNUM { $$ = create_leaf_float($1, @1.first_line, @1.first_column); }
    | function_call { $$ = $1; }
    ;

function_call
    : ID LPAREN arguments RPAREN { ASTNode *id_node = create_leaf_id($1, @1.first_line, @1.first_column);
      $$ = create_node(AST_FUNC_CALL, id_node, $3, NULL, NULL); }
    ;

arguments
    : argument_list { $$ = $1; }
    | /* vazio */ { $$ = NULL; }
    ;

argument_list
    : argument_list COMMA expression
      {
          ASTNode *t = $1;
          while (t->next != NULL) t = t->next;
          t->next = $3;
          $$ = $1;
      }
    | expression { $$ = $1; }
    ;

%%

/* ========================================================================= */
/* MENSAGENS DE ERRO BISON                                                   */
/* ========================================================================= */

void yyerror(const char *s) {
    if (strstr(s, "expecting ';'") || strstr(s, "expecting SEMCOL")) { syntax_error_msg_bef("Expected ';'"); return; }
    if (strstr(s, "expecting ')'") || strstr(s, "expecting RPAREN")) { syntax_error_msg_bef("Expected ')'"); return; }
    if (strstr(s, "expecting '('") || strstr(s, "expecting LPAREN")) { syntax_error_msg_bef("Expected '('"); return; }
    if (strstr(s, "expecting '}'") || strstr(s, "expecting RBRACE")) { syntax_error_msg_bef("Expected '}'"); return; }
    if (strstr(s, "expecting '{'") || strstr(s, "expecting LBRACE")) { syntax_error_msg_bef("Expected '{'"); return; }
    if (strstr(s, "expecting ID")) { syntax_error_msg_bef("Expected identifier"); return; }
    syntax_error_msg_bef("Unexpected token");
}

void syntax_error_msg_bef(const char *msg) {
    syntax_error = 1;
    fprintf(stderr, "\n[SYNTAX ERROR] %s at Line %d, Column %d\n\n", msg, yylineno, column);
}

/* ========================================================================= */
/* FASE 1: ANÁLISE SEMÂNTICA (TABELA DE SÍMBOLOS E TYPE CHECKING)            */
/* ========================================================================= */

#define MAX_SCOPES 100

/* ========================================================================= */
/* FASE 1: ANÁLISE SEMÂNTICA (TABELA COM PONTEIRO PARA O PAI)                */
/* ========================================================================= */

typedef struct Symbol {
    char name[50];
    char type[20];
    char category[20];
    int num_params;
    char param_types[10][20];
    struct Symbol *next;
} Symbol;

typedef struct SymbolTable {
    int table_id;               /* NOVO: ID único da tabela (0, 1, 2...) */
    int scope_level;            /* Profundidade (0=Global, 1=Função, etc) */
    Symbol *symbols;
    struct SymbolTable *parent; 
} SymbolTable;

SymbolTable *current_table = NULL;
/* NOVO: Registro histórico de TODAS as tabelas criadas */
SymbolTable *all_tables[200]; 
int total_tables_created = 0;

char current_func_type[20] = "";
int is_function_body = 1;


void open_scope() {
    SymbolTable *new_table = (SymbolTable*)malloc(sizeof(SymbolTable));
    new_table->symbols = NULL;
    new_table->parent = current_table;
    /* Define o ID único desta tabela */
    new_table->table_id = total_tables_created;
    if (current_table) {
        new_table->scope_level = current_table->scope_level + 1;
    } else {
        new_table->scope_level = 0;
    }
    
    current_table = new_table;
    /* Guarda a tabela no registro histórico */
    all_tables[total_tables_created] = new_table;
    total_tables_created++;
}

void close_scope() {
    if (!current_table) return;
    /* Volta para o pai silenciosamente. A tabela local continua viva no all_tables */
    current_table = current_table->parent;
}

int add_symbol(const char *name, const char *type, const char *category) {
    if (!name || !current_table) return 0;
    /* Verifica se a variável JÁ EXISTE NO ESCOPO ATUAL */
    Symbol *s = current_table->symbols;
    while (s) {
        if (strcmp(s->name, name) == 0) return 0; // Erro de redeclaração
        s = s->next;
    }
    
    /* Cria e insere o símbolo no início da lista da tabela ATUAL */
    Symbol *ns = (Symbol*)malloc(sizeof(Symbol));
    strcpy(ns->name, name);
    strcpy(ns->type, type);
    strcpy(ns->category, category);
    ns->num_params = 0;
    
    ns->next = current_table->symbols;
    current_table->symbols = ns;
    
    return 1;
}

Symbol* lookup_symbol(const char *name) {
    if (!name) return NULL;
    /* A mágica do ponteiro parent: busca de dentro para fora! */
    SymbolTable *table = current_table;
    while (table != NULL) {
        Symbol *s = table->symbols;
        while (s) {
            if (strcmp(s->name, name) == 0) return s; // Achou!
            s = s->next;
        }
        /* Não achou neste escopo? Sobe para a Tabela Pai! */
        table = table->parent;
    }
    return NULL; // Se chegou à tabela Global e não achou, a variável não existe
}


void print_symbol_table() {
    printf("\n======================================================================================================\n");
    printf("                                      FINAL SYMBOL TABLES DUMP\n");
    printf("======================================================================================================\n");
    
    for (int i = 0; i < total_tables_created; i++) {
        SymbolTable *table = all_tables[i];
        printf("\n======================================================================================================\n");
        if (table->parent)
            printf("            TABLE ID: %d  |  SCOPE LEVEL: %d  |  PARENT ID: %d (Level %d)\n", 
                   table->table_id, table->scope_level, table->parent->table_id, table->parent->scope_level);
        else
            printf("            TABLE ID: %d  |  SCOPE LEVEL: %d  |  PARENT ID: NULL (GLOBAL)\n", 
                   table->table_id, table->scope_level);
        printf("======================================================================================================\n");
        
        printf("%-15s | %-10s | %-10s | %-10s | %-20s | %-15s\n", 
               "NAME", "TYPE", "CATEGORY", "NUM_PARAMS", "PARAM_TYPES", "NEXT SYMBOL");
        printf("------------------------------------------------------------------------------------------------------\n");
        
        Symbol *s = table->symbols;
        if (!s) {
            printf("  (Tabela Vazia)\n");
        } else {
            while (s) {
                char params_str[100] = "";
                if (strcmp(s->category, "func") == 0 && s->num_params > 0) {
                    for (int p = 0; p < s->num_params; p++) {
                        strcat(params_str, s->param_types[p]);
                        if (p < s->num_params - 1) strcat(params_str, ", ");
                    }
                } else {
                    strcpy(params_str, "-");
                }
                
                char next_name[50];
                if (s->next) strcpy(next_name, s->next->name);
                else strcpy(next_name, "NULL");
                
                int num_p = (strcmp(s->category, "func") == 0) ? s->num_params : 0;
                printf("%-15s | %-10s | %-10s | %-10d | %-20s | %-15s\n",
                       s->name, s->type, s->category, num_p, params_str, next_name);
                s = s->next;
            }
        }
    }
    printf("\n");
}

char* verificar_semantica(ASTNode *node) {
    if (!node) return "void";
    switch (node->type) {
        case AST_VAR_DECL: {
            char *decl_type = node->child1->lexema;
            ASTNode *var = node->child2; 
            while (var) {
                if (var->type == AST_ID) {
                    if (!add_symbol(var->lexema, decl_type, "var")) {
                        /* var é uma folha, a linha e coluna estão certas */
                        fprintf(stderr, "[SEMANTIC ERROR] Variable '%s' redeclared in same scope at Line %d, Column %d.\n", var->lexema, var->line, var->column);
                        semantic_error = 1;
                    }
                } else if (var->type == AST_ASSIGN) {
                    char *vname = var->child1->lexema;
                    if (!add_symbol(vname, decl_type, "var")) {
                        /* Aponta para a variável (child1) e não para a atribuição */
                        fprintf(stderr, "[SEMANTIC ERROR] Variable '%s' redeclared in same scope at Line %d, Column %d.\n", vname, var->child1->line, var->child1->column);
                        semantic_error = 1;
                    }
                    
                    /* VALIDAÇÃO DE TIPO NA INICIALIZAÇÃO */
                    char *rtype = verificar_semantica(var->child2);
                    if (strcmp(decl_type, "int") == 0 && strcmp(rtype, "float") == 0) {
                        /* Aponta para a variável que está a receber o valor errado (child2) */
                        fprintf(stderr, "[SEMANTIC ERROR] Type mismatch in initialization: cannot assign 'float' to 'int' variable '%s' at Line %d, Column %d.\n", vname, var->child2->line, var->child2->column);
                        semantic_error = 1;
                    }
                }
                var = var->next;
            }
            return "void";
        }
       case AST_FUNC_DECL: {
            char *ret_type = node->child1->lexema;
            char *fname = node->child2->lexema;
            strcpy(current_func_type, ret_type);

            if (strcmp(ret_type, "int") != 0 && strcmp(ret_type, "float") != 0) {
                /* Aponta para o TIPO de retorno que está errado (child1) */
                fprintf(stderr, "[SEMANTIC ERROR] Invalid return type '%s' for function '%s' at Line %d, Column %d. Only 'int' and 'float' are allowed.\n", 
                        ret_type, fname, node->child1->line, node->child1->column);
                semantic_error = 1;
            }

            if (!add_symbol(fname, ret_type, "func")) {
                /* Aponta para o NOME da função (child2) */
                fprintf(stderr, "[SEMANTIC ERROR] Function '%s' redeclared at Line %d, Column %d.\n", fname, node->child2->line, node->child2->column);
                semantic_error = 1;
            }

            /* --- SALVA A ASSINATURA DA FUNÇÃO (TIPOS DOS PARÂMETROS) --- */
            Symbol *func_sym = lookup_symbol(fname);
            if (func_sym) {
                func_sym->num_params = 0;
                ASTNode *p = node->child3;
                while (p && func_sym->num_params < 10) {
                    strcpy(func_sym->param_types[func_sym->num_params], p->child1->lexema);
                    func_sym->num_params++;
                    p = p->next;
                }
            }
            /* ----------------------------------------------------------- */

            open_scope();
            ASTNode *p2 = node->child3; /* Parametros */
            while (p2) {
                /* CORREÇÃO: Passar "param" em vez de "var" */
                add_symbol(p2->child2->lexema, p2->child1->lexema, "param");
                p2 = p2->next;
            }
            is_function_body = 1;
            verificar_semantica(node->child4);
            /* Body compound */
            close_scope();
            return "void";
        }
       case AST_COMPOUND: {
            int block_scope_created = 0;
            /* Se for corpo de função, consome a flag e NÃO ABRE escopo novo.
               Se for um bloco solto ou interno, abre escopo normal. */
            if (is_function_body) {
                is_function_body = 0;
            } else {
                open_scope();
                block_scope_created = 1;
            }
            
            ASTNode *ld = node->child1;
            while (ld) { verificar_semantica(ld); ld = ld->next; }
            ASTNode *st = node->child2;
            while (st) { verificar_semantica(st); st = st->next; }
            if (node->child3) verificar_semantica(node->child3);
            /* Return */
            
            if (block_scope_created) {
                close_scope();
            }
            return "void";
        }
        case AST_ID: {
            Symbol *s = lookup_symbol(node->lexema);
            if (!s) {
                fprintf(stderr, "[SEMANTIC ERROR] Variable '%s' undeclared at Line %d, Column %d.\n", node->lexema, node->line, node->column);
                semantic_error = 1;
                strcpy(node->exp_type, "int"); 
                return "int";
            }
            strcpy(node->exp_type, s->type);
            return s->type;
        }
        case AST_INT: strcpy(node->exp_type, "int"); return "int";
        case AST_FLOAT: strcpy(node->exp_type, "float");
            return "float";
        case AST_ASSIGN: {
            char *ltype = verificar_semantica(node->child1);
            char *rtype = verificar_semantica(node->child2);
            
            /* VALIDAÇÃO DE TIPO: int não pode receber float */
            if (strcmp(ltype, "int") == 0 && strcmp(rtype, "float") == 0) {
                /* Aponta para a variável (child1) e não para o sinal de igual */
                fprintf(stderr, "[SEMANTIC ERROR] Type mismatch: cannot assign 'float' to 'int' variable '%s' at Line %d, Column %d.\n", 
                        node->child1->lexema ? node->child1->lexema : "variable", node->child1->line, node->child1->column);
                semantic_error = 1;
            }
            
            strcpy(node->exp_type, ltype);
            return ltype;
        }
        case AST_BINOP: {
            char *t1 = verificar_semantica(node->child1);
            char *t2 = verificar_semantica(node->child2);
            char *op = node->lexema;
            
            if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 || strcmp(op, "*") == 0 || strcmp(op, "/") == 0) {
                if (strcmp(t1, "float") == 0 || strcmp(t2, "float") == 0) strcpy(node->exp_type, "float");
                else strcpy(node->exp_type, "int");
            } else if (strcmp(op, "%") == 0) {
                if (strcmp(t1, "float") == 0 || strcmp(t2, "float") == 0) {
                    /* Aponta para o primeiro operando da conta (child1) */
                    fprintf(stderr, "[SEMANTIC ERROR] Operator '%%' only accepts int at Line %d, Column %d.\n", node->child1->line, node->child1->column);
                    semantic_error = 1;
                }
                strcpy(node->exp_type, "int");
            } else {
                strcpy(node->exp_type, "int");
            }
            return node->exp_type;
        }
        case AST_UNOP: {
            char *t1 = verificar_semantica(node->child1);
            if (strcmp(node->lexema, "!") == 0) strcpy(node->exp_type, "int");
            else strcpy(node->exp_type, t1);
            return node->exp_type;
        }
        case AST_IF:
        case AST_WHILE: {
            verificar_semantica(node->child1);
            verificar_semantica(node->child2); 
            if (node->child3) verificar_semantica(node->child3); 
            return "void";
        }
        case AST_FUNC_CALL: {
            Symbol *s = lookup_symbol(node->child1->lexema);
            if (!s || strcmp(s->category, "func") != 0) {
                /* Aponta para o NOME da função (child1) */
                fprintf(stderr, "[SEMANTIC ERROR] Function '%s' undeclared or invalid at Line %d, Column %d.\n", node->child1->lexema, node->child1->line, node->child1->column);
                semantic_error = 1;
                strcpy(node->exp_type, "int");
                return "int";
            }
            
            ASTNode *arg = node->child2;
            int arg_count = 0;
            
            while (arg) { 
                char *arg_type = verificar_semantica(arg);
                /* VALIDAÇÃO DE TIPO DOS ARGUMENTOS */
                if (arg_count < s->num_params) {
                    char *expected_type = s->param_types[arg_count];
                    /* Bloqueia passar float para um parâmetro que espera int */
                    if (strcmp(expected_type, "int") == 0 && strcmp(arg_type, "float") == 0) {
                        /* Aponta diretamente para o ARGUMENTO que está com o tipo errado (arg) */
                        fprintf(stderr, "[SEMANTIC ERROR] Type mismatch in argument %d of function '%s': cannot pass 'float' to 'int' parameter at Line %d, Column %d.\n", 
                                arg_count + 1, s->name, arg->line, arg->column);
                        semantic_error = 1;
                    }
                }
                
                arg_count++;
                arg = arg->next; 
            }
            
            /* CHECA SE PASSOU A QUANTIDADE CERTA DE PARÂMETROS */
            if (arg_count != s->num_params) {
                /* Aponta para o NOME da função (child1) */
                fprintf(stderr, "[SEMANTIC ERROR] Function '%s' expects %d arguments, but got %d at Line %d, Column %d.\n", 
                        s->name, s->num_params, arg_count, node->child1->line, node->child1->column);
                semantic_error = 1;
            }
            
            strcpy(node->exp_type, s->type);
            return s->type;
        }
        case AST_PRINT:
        case AST_READ:
        {
            if (node->child1) verificar_semantica(node->child1);
            return "void";
        }
        case AST_RETURN: {
            if (node->child1) {
                char *rtype = verificar_semantica(node->child1);
                /* BLOQUEIA retornar float numa função declarada como int */
                if (strcmp(current_func_type, "int") == 0 && strcmp(rtype, "float") == 0) {
                    /* Aponta para o VALOR que está sendo retornado (child1) */
                    fprintf(stderr, "[SEMANTIC ERROR] Type mismatch: cannot return 'float' in a function returning 'int' at Line %d, Column %d.\n", node->child1->line, node->child1->column);
                    semantic_error = 1;
                }
            } else {
                /* NOVO: Se o return for vazio (return;) aponta para o próprio return */
                fprintf(stderr, "[SEMANTIC ERROR] Missing return value: function expects '%s' at Line %d, Column %d.\n", current_func_type, node->line, node->column);
                semantic_error = 1;
            }
            return "void";
            // O nó return em si não devolve tipo para a árvore
        }
        default: return "void";
    }
}


/* ========================================================================= */
/* FASE 2: GERAÇÃO DE CÓDIGO DE TRÊS ENDEREÇOS (TAC) — TIPADO               */
/* ========================================================================= */

int temp_counter = 1;
int label_counter = 1;

char* new_temp() {
    char *t = malloc(10);
    sprintf(t, "t%d", temp_counter++);
    return t;
}

char* new_label() {
    char *l = malloc(10);
    sprintf(l, "L%d", label_counter++);
    return l;
}

void gerar_tac(ASTNode *node);

void gerar_condicao(ASTNode *node, char *l_true, char *l_false) {
    if (!node) return;

    if (node->type == AST_BINOP && strcmp(node->lexema, "||") == 0) {
        /* OR: Se o da esquerda for verdadeiro, pula logo pro l_true (Curto-circuito). 
           Se for falso, avalia o da direita. */
        char *l_next = new_label();
        gerar_condicao(node->child1, l_true, l_next);
        emit_tac("%s:", l_next);
        gerar_condicao(node->child2, l_true, l_false);
    }
    else if (node->type == AST_BINOP && strcmp(node->lexema, "&&") == 0) {
        /* AND: Se o da esquerda for falso, pula logo pro l_false (Curto-circuito). 
           Se for verdadeiro, avalia o da direita. */
        char *l_next = new_label();
        gerar_condicao(node->child1, l_next, l_false);
        emit_tac("%s:", l_next);
        gerar_condicao(node->child2, l_true, l_false);
    }
    else if (node->type == AST_UNOP && strcmp(node->lexema, "!") == 0) {
        /* NOT: Apenas inverte os rótulos alvo */
        gerar_condicao(node->child1, l_false, l_true);
    }
    else if (node->type == AST_BINOP && (
                strcmp(node->lexema, "<") == 0 || strcmp(node->lexema, ">") == 0 ||
                strcmp(node->lexema, "<=") == 0 || strcmp(node->lexema, ">=") == 0 ||
                strcmp(node->lexema, "==") == 0 || strcmp(node->lexema, "!=") == 0)) {
        
        gerar_tac(node->child1); 
        gerar_tac(node->child2); 
        
        char op1_temp[20];
        char op2_temp[20];
        strcpy(op1_temp, node->child1->temp);
        strcpy(op2_temp, node->child2->temp);
        
        /* Verifica se algum operando é float */
        int operands_float = (strcmp(node->child1->exp_type, "float") == 0 ||
                              strcmp(node->child2->exp_type, "float") == 0);
        
        /* CAST NAS COMPARAÇÕES SE HOUVER MISTURA DE TIPOS */
        if (strcmp(node->child1->exp_type, "int") == 0 && strcmp(node->child2->exp_type, "float") == 0) {
            char *cast_temp = new_temp();
            emit_tac("%s = (float) %s", cast_temp, op1_temp);
            strcpy(op1_temp, cast_temp);
        } else if (strcmp(node->child1->exp_type, "float") == 0 && strcmp(node->child2->exp_type, "int") == 0) {
            char *cast_temp = new_temp();
            emit_tac("%s = (float) %s", cast_temp, op2_temp);
            strcpy(op2_temp, cast_temp);
        }
        
        if (operands_float) {
            emit_tac("if %s %sf %s goto %s", op1_temp, node->lexema, op2_temp, l_true);
        } else {
            emit_tac("if %s %s %s goto %s", op1_temp, node->lexema, op2_temp, l_true);
        }
        emit_tac("goto %s", l_false);
    }
    else {
        /* Fallback: caso seja uma variável solta como "if (x)" ou "while (1)" */
        gerar_tac(node);
        emit_tac("if %s != 0 goto %s", node->temp, l_true);
        emit_tac("goto %s", l_false);
    }
}

void gerar_tac(ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case AST_VAR_DECL: {
            char *decl_type = node->child1->lexema;
            ASTNode *var = node->child2;
            while (var) {
                if (var->type == AST_ASSIGN) {
                    gerar_tac(var->child2);
                    
                    char rhs_temp[20];
                    strcpy(rhs_temp, var->child2->temp);
                    
                    /* CAST NA INICIALIZAÇÃO SE VARIÁVEL É FLOAT E VALOR É INT */
                    if (strcmp(decl_type, "float") == 0 && strcmp(var->child2->exp_type, "int") == 0) {
                        char *cast_temp = new_temp();
                        emit_tac("%s = (float) %s", cast_temp, rhs_temp);
                        strcpy(rhs_temp, cast_temp);
                    }
                    
                    emit_tac("%s = %s", var->child1->lexema, rhs_temp);
                }
                var = var->next;
            }
            break;
        }
        
        case AST_FUNC_DECL: {
            char *ret_type = node->child1->lexema;
            char *fname = node->child2->lexema;
            
            /* Monta a string de parâmetros tipados: "int n, float x" */
            char params_str[256] = "";
            ASTNode *p = node->child3;
            int first = 1;
            while (p) {
                if (!first) strcat(params_str, ", ");
                strcat(params_str, p->child1->lexema); /* tipo */
                strcat(params_str, " ");
                strcat(params_str, p->child2->lexema); /* nome */
                first = 0;
                p = p->next;
            }
            
            emit_tac("func %s %s params: %s", fname, ret_type, params_str);
            
            gerar_tac(node->child4); /* Corpo da Função */
            
            emit_tac("endfunc");
            break;
        }
            
        case AST_COMPOUND: {
            ASTNode *ld = node->child1;
            while (ld) { gerar_tac(ld); ld = ld->next; }
            ASTNode *st = node->child2;
            while (st) { gerar_tac(st); st = st->next; }
            if (node->child3) gerar_tac(node->child3);
            /* Return */
            break;
        }
        
        case AST_INT:
            sprintf(node->temp, "%d", node->int_val);
            break;
            
        case AST_FLOAT:
            sprintf(node->temp, "%.2f", node->float_val);
            break;
        case AST_ID:
            strcpy(node->temp, node->lexema);
            break;
        case AST_ASSIGN: {
            gerar_tac(node->child2);
            
            char rhs_temp[20];
            strcpy(rhs_temp, node->child2->temp);
            
            /* CAST NA ATRIBUIÇÃO (ex: já declarado como float, mas recebendo int) */
            if (strcmp(node->child1->exp_type, "float") == 0 && strcmp(node->child2->exp_type, "int") == 0) {
                char *cast_temp = new_temp();
                emit_tac("%s = (float) %s", cast_temp, rhs_temp);
                strcpy(rhs_temp, cast_temp);
            }
            
            emit_tac("%s = %s", node->child1->lexema, rhs_temp);
            strcpy(node->temp, node->child1->lexema);
            break;
        }
            
        case AST_BINOP: {
            gerar_tac(node->child1);
            gerar_tac(node->child2);
            
            char op1_temp[20];
            char op2_temp[20];
            strcpy(op1_temp, node->child1->temp);
            strcpy(op2_temp, node->child2->temp);
            
            /* Verifica se algum operando é float (abrange aritméticos E relacionais) */
            int operands_float = (strcmp(node->child1->exp_type, "float") == 0 ||
                                  strcmp(node->child2->exp_type, "float") == 0);
            
            /* VERIFICA SE PRECISA DE CAST NO OPERANDO 1 */
            if (strcmp(node->child1->exp_type, "int") == 0 && strcmp(node->child2->exp_type, "float") == 0) {
                char *cast_temp = new_temp();
                emit_tac("%s = (float) %s", cast_temp, op1_temp);
                strcpy(op1_temp, cast_temp);
            }
            /* VERIFICA SE PRECISA DE CAST NO OPERANDO 2 */
            else if (strcmp(node->child1->exp_type, "float") == 0 && strcmp(node->child2->exp_type, "int") == 0) {
                char *cast_temp = new_temp();
                emit_tac("%s = (float) %s", cast_temp, op2_temp);
                strcpy(op2_temp, cast_temp);
            }
            
            strcpy(node->temp, new_temp());
            
            /* Usa sufixo 'f' nos operadores se os operandos são float.
               Exceção: %, && e || não usam sufixo float. */
            char *op = node->lexema;
            if (operands_float && strcmp(op, "%") != 0 && 
                strcmp(op, "&&") != 0 && strcmp(op, "||") != 0) {
                emit_tac("%s = %s %sf %s", node->temp, op1_temp, op, op2_temp);
            } else {
                emit_tac("%s = %s %s %s", node->temp, op1_temp, op, op2_temp);
            }
            break;
        }
        case AST_UNOP:
            gerar_tac(node->child1);
            strcpy(node->temp, new_temp());
            /* Usa sufixo 'f' para minus unário em floats */
            if (strcmp(node->lexema, "minus") == 0 && strcmp(node->exp_type, "float") == 0) {
                emit_tac("%s = minusf %s", node->temp, node->child1->temp);
            } else {
                emit_tac("%s = %s %s", node->temp, node->lexema, node->child1->temp);
            }
            break;
            
        case AST_IF: {
            char *l_true = new_label();
            char *l_false = new_label();
            
            /* Otimização: se não tiver ELSE, o l_end é o próprio l_false */
            char *l_end = node->child3 ? new_label() : l_false;
            
            /* 1. Gera o código da condição em formato de curto-circuito */
            gerar_condicao(node->child1, l_true, l_false);
            
            /* 2. Bloco TRUE */
            emit_tac("%s:", l_true);
            gerar_tac(node->child2); // Statement do if
            
            /* 3. Bloco FALSE / ELSE */
            if (node->child3) {
                emit_tac("goto %s", l_end);
                emit_tac("%s:", l_false);
                gerar_tac(node->child3); // Statement do else
            }
            
            /* 4. Fim */
            emit_tac("%s:", l_end);
            break;
        }
        
        case AST_WHILE: {
            char *l_start = new_label();
            char *l_true = new_label();
            char *l_false = new_label();
            
            /* Início do loop */
            emit_tac("%s:", l_start);
            
            /* Condição */
            gerar_condicao(node->child1, l_true, l_false);
            
            /* Bloco TRUE */
            emit_tac("%s:", l_true);
            gerar_tac(node->child2);
            emit_tac("goto %s", l_start);
            
            /* Saída do loop (FALSE) */
            emit_tac("%s:", l_false);
            break;
        }
        
        case AST_PRINT:
            gerar_tac(node->child1);
            /* Instrução tipada: print_int ou print_float */
            if (strcmp(node->child1->exp_type, "float") == 0)
                emit_tac("print_float %s", node->child1->temp);
            else
                emit_tac("print_int %s", node->child1->temp);
            break;
            
        case AST_READ:
            /* Instrução tipada: read_int ou read_float */
            if (strcmp(node->child1->exp_type, "float") == 0)
                emit_tac("read_float %s", node->child1->lexema);
            else
                emit_tac("read_int %s", node->child1->lexema);
            break;
            
        case AST_RETURN:
            if (node->child1) {
                gerar_tac(node->child1);
                emit_tac("return %s", node->child1->temp);
            } else {
                emit_tac("return");
            }
            break;
        case AST_FUNC_CALL: {
            ASTNode *arg = node->child2;
            int params = 0;
            while (arg) {
                gerar_tac(arg);
                emit_tac("param %s", arg->temp);
                params++;
                arg = arg->next;
            }
            strcpy(node->temp, new_temp());
            emit_tac("%s = call %s, %d", node->temp, node->child1->lexema, params);
            break;
        }
        default: break;
    }
}


/* ========================================================================= */
/* FASE 3: OTIMIZAÇÃO DO TAC (CONSTANT FOLDING + SIMPLIFICAÇÃO ALGÉBRICA)    */
/* ========================================================================= */

/* Helper: verifica se uma string é um literal inteiro (ex: "42", "-5") */
int is_int_literal(const char *s) {
    if (!s || !*s) return 0;
    int i = 0;
    if (s[0] == '-' || s[0] == '+') i = 1;
    if (!s[i]) return 0;
    for (; s[i]; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}

/* Helper: verifica se uma string é um literal float (ex: "3.14", "-1.50") */
int is_float_literal(const char *s) {
    if (!s || !*s) return 0;
    int i = 0, dot = 0;
    if (s[0] == '-' || s[0] == '+') i = 1;
    if (!s[i]) return 0;
    for (; s[i]; i++) {
        if (s[i] == '.') { dot++; if (dot > 1) return 0; }
        else if (!isdigit((unsigned char)s[i])) return 0;
    }
    return (dot == 1);
}

void optimize_tac() {
    TACInstr *instr = tac_head;
    while (instr) {
        char dest[64], op1[64], op[16], op2[64];
        
        /* Tenta casar o padrão: dest = op1 op op2 (operação binária) */
        if (sscanf(instr->text, "%63s = %63s %15s %63s", dest, op1, op, op2) == 4) {
            
            /* ============ CONSTANT FOLDING (int) ============ */
            if (is_int_literal(op1) && is_int_literal(op2)) {
                long a = atol(op1), b = atol(op2);
                long result = 0;
                int folded = 1;
                if (strcmp(op, "+") == 0)           result = a + b;
                else if (strcmp(op, "-") == 0)      result = a - b;
                else if (strcmp(op, "*") == 0)      result = a * b;
                else if (strcmp(op, "/") == 0 && b != 0)  result = a / b;
                else if (strcmp(op, "%") == 0 && b != 0)  result = a % b;
                else folded = 0;
                
                if (folded) {
                    sprintf(instr->text, "%s = %ld", dest, result);
                    instr = instr->next;
                    continue;
                }
            }
            
            /* ============ CONSTANT FOLDING (float) ============ */
            if ((is_float_literal(op1) || is_int_literal(op1)) &&
                (is_float_literal(op2) || is_int_literal(op2))) {
                /* Checa se o operador é float (sufixo 'f') */
                char clean_op[16];
                strcpy(clean_op, op);
                int oplen = strlen(clean_op);
                if (oplen > 1 && clean_op[oplen-1] == 'f') {
                    clean_op[oplen-1] = '\0';
                    double a = atof(op1), b = atof(op2), result = 0;
                    int folded = 1;
                    if (strcmp(clean_op, "+") == 0)      result = a + b;
                    else if (strcmp(clean_op, "-") == 0) result = a - b;
                    else if (strcmp(clean_op, "*") == 0) result = a * b;
                    else if (strcmp(clean_op, "/") == 0 && b != 0.0) result = a / b;
                    else folded = 0;
                    
                    if (folded) {
                        sprintf(instr->text, "%s = %.2f", dest, result);
                        instr = instr->next;
                        continue;
                    }
                }
            }
            
            /* ============ SIMPLIFICAÇÃO ALGÉBRICA ============ */
            
            /* x + 0 = x,  0 + x = x */
            if (strcmp(op, "+") == 0 || strcmp(op, "+f") == 0) {
                if ((is_int_literal(op2) && atol(op2) == 0) ||
                    (is_float_literal(op2) && atof(op2) == 0.0)) {
                    sprintf(instr->text, "%s = %s", dest, op1);
                } else if ((is_int_literal(op1) && atol(op1) == 0) ||
                           (is_float_literal(op1) && atof(op1) == 0.0)) {
                    sprintf(instr->text, "%s = %s", dest, op2);
                }
            }
            /* x - 0 = x */
            else if (strcmp(op, "-") == 0 || strcmp(op, "-f") == 0) {
                if ((is_int_literal(op2) && atol(op2) == 0) ||
                    (is_float_literal(op2) && atof(op2) == 0.0)) {
                    sprintf(instr->text, "%s = %s", dest, op1);
                }
            }
            /* x * 1 = x,  1 * x = x,  x * 0 = 0,  0 * x = 0 */
            else if (strcmp(op, "*") == 0 || strcmp(op, "*f") == 0) {
                int is_fop = (strcmp(op, "*f") == 0);
                if ((is_int_literal(op2) && atol(op2) == 1) ||
                    (is_float_literal(op2) && atof(op2) == 1.0)) {
                    sprintf(instr->text, "%s = %s", dest, op1);
                } else if ((is_int_literal(op1) && atol(op1) == 1) ||
                           (is_float_literal(op1) && atof(op1) == 1.0)) {
                    sprintf(instr->text, "%s = %s", dest, op2);
                } else if ((is_int_literal(op2) && atol(op2) == 0) ||
                           (is_float_literal(op2) && atof(op2) == 0.0)) {
                    sprintf(instr->text, "%s = %s", dest, is_fop ? "0.00" : "0");
                } else if ((is_int_literal(op1) && atol(op1) == 0) ||
                           (is_float_literal(op1) && atof(op1) == 0.0)) {
                    sprintf(instr->text, "%s = %s", dest, is_fop ? "0.00" : "0");
                }
            }
            /* x / 1 = x */
            else if (strcmp(op, "/") == 0 || strcmp(op, "/f") == 0) {
                if ((is_int_literal(op2) && atol(op2) == 1) ||
                    (is_float_literal(op2) && atof(op2) == 1.0)) {
                    sprintf(instr->text, "%s = %s", dest, op1);
                }
            }
        }
        
        instr = instr->next;
    }
}

void write_tac_to_file(FILE *f) {
    TACInstr *instr = tac_head;
    while (instr) {
        fprintf(f, "%s\n", instr->text);
        instr = instr->next;
    }
}


/* ========================================================================= */
/* MAIN - FLUXO INTEGRADO DE ANÁLISE E GERAÇÃO DE TAC OTIMIZADO             */
/* ========================================================================= */

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./compiler <source_file>\n");
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        printf("Error opening file.\n");
        return 1;
    }

    printf("\n========================================\n");
    printf("1. STARTING LEXICAL AND SYNTACTIC ANALYSIS\n");
    printf("========================================\n");

    yyparse();
    
    if (lexical_error || syntax_error) {
        printf("\nPROGRAM REJECTED DUE TO LEXICAL OR SYNTAX ERRORS\n");
        fclose(yyin);
        return 1;
    }

    printf("\n========================================\n");
    printf("2. STARTING SEMANTIC ANALYSIS\n");
    printf("========================================\n");
    
    open_scope();
    /* Dispara a varredura percorrendo a lista global */
    ASTNode *curr = root;
    while (curr) {
        verificar_semantica(curr);
        curr = curr->next;
    }

    if (semantic_error) {
        printf("\nPROGRAM REJECTED DUE TO SEMANTIC ERRORS\n");
        fclose(yyin);
        return 1;
    }

    printf("Semantic analysis passed successfully.\n");
    print_symbol_table();

    /* ---- FASE 2: Geração do TAC em memória ---- */
    printf("\n========================================\n");
    printf("3. GENERATING TYPED TAC\n");
    printf("========================================\n");

    curr = root;
    while (curr) {
        gerar_tac(curr);
        curr = curr->next;
    }

    /* ---- FASE 3: Otimização do TAC em memória ---- */
    printf("\n========================================\n");
    printf("4. OPTIMIZING TAC (Constant Folding + Algebraic Simplification)\n");
    printf("========================================\n");

    optimize_tac();

    /* ---- Escrita do TAC otimizado no arquivo ---- */
    out = fopen("output.tac", "w");
    if (!out) {
        printf("Error creating output.tac file.\n");
        fclose(yyin);
        return 1;
    }
    write_tac_to_file(out);
    fclose(out);

    printf("\n========================================\n");
    printf("TAC OTIMIZADO GERADO: 'output.tac'\n");
    printf("Use o backend para gerar Assembly: ./codegen output.tac\n");
    printf("========================================\n\n");

    fclose(yyin);
    return 0;
}