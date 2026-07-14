/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "parser.y"

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
    AST_VAR_DECL, AST_ARRAY_DECL, AST_ARRAY_ACCESS, AST_ARRAY_INIT, AST_FUNC_DECL, AST_TYPE,
    AST_PARAM, AST_COMPOUND, AST_IF, AST_WHILE, AST_PRINT, AST_READ,
    AST_RETURN, AST_ASSIGN, AST_BINOP, AST_UNOP, AST_ID, AST_INT, AST_FLOAT,
    AST_CHAR, AST_STRING,
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
    int num_array_dims;
    int array_dims[10];
    
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
    n->num_array_dims = 0;
    for (int i = 0; i < 10; i++) n->array_dims[i] = 0;
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

ASTNode* create_leaf_char(int val, int line, int col) {
    ASTNode *n = (ASTNode*)malloc(sizeof(ASTNode));
    n->type = AST_CHAR;
    n->int_val = val;
    n->line = line;
    n->column = col;
    n->child1 = n->child2 = n->child3 = n->child4 = n->next = NULL;
    return n;
}

ASTNode* create_leaf_string(char *val, int line, int col) {
    ASTNode *n = (ASTNode*)malloc(sizeof(ASTNode));
    n->type = AST_STRING;
    n->lexema = val ? strdup(val) : strdup("");
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


#line 238 "parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_KW_INT = 3,                     /* KW_INT  */
  YYSYMBOL_KW_FLOAT = 4,                   /* KW_FLOAT  */
  YYSYMBOL_KW_CHAR = 5,                    /* KW_CHAR  */
  YYSYMBOL_KW_STRING = 6,                  /* KW_STRING  */
  YYSYMBOL_KW_VOID = 7,                    /* KW_VOID  */
  YYSYMBOL_KW_IF = 8,                      /* KW_IF  */
  YYSYMBOL_KW_ELSE = 9,                    /* KW_ELSE  */
  YYSYMBOL_KW_WHILE = 10,                  /* KW_WHILE  */
  YYSYMBOL_KW_PRINT = 11,                  /* KW_PRINT  */
  YYSYMBOL_KW_READ = 12,                   /* KW_READ  */
  YYSYMBOL_KW_RETURN = 13,                 /* KW_RETURN  */
  YYSYMBOL_ID = 14,                        /* ID  */
  YYSYMBOL_INTNUM = 15,                    /* INTNUM  */
  YYSYMBOL_CHARLIT = 16,                   /* CHARLIT  */
  YYSYMBOL_FLOATNUM = 17,                  /* FLOATNUM  */
  YYSYMBOL_STRINGLIT = 18,                 /* STRINGLIT  */
  YYSYMBOL_LT = 19,                        /* LT  */
  YYSYMBOL_GT = 20,                        /* GT  */
  YYSYMBOL_LE = 21,                        /* LE  */
  YYSYMBOL_GE = 22,                        /* GE  */
  YYSYMBOL_EQ = 23,                        /* EQ  */
  YYSYMBOL_NE = 24,                        /* NE  */
  YYSYMBOL_AND = 25,                       /* AND  */
  YYSYMBOL_OR = 26,                        /* OR  */
  YYSYMBOL_NOT = 27,                       /* NOT  */
  YYSYMBOL_PLUS = 28,                      /* PLUS  */
  YYSYMBOL_MINUS = 29,                     /* MINUS  */
  YYSYMBOL_MULT = 30,                      /* MULT  */
  YYSYMBOL_DIV = 31,                       /* DIV  */
  YYSYMBOL_MOD = 32,                       /* MOD  */
  YYSYMBOL_ASSIG = 33,                     /* ASSIG  */
  YYSYMBOL_SEMCOL = 34,                    /* SEMCOL  */
  YYSYMBOL_COMMA = 35,                     /* COMMA  */
  YYSYMBOL_LPAREN = 36,                    /* LPAREN  */
  YYSYMBOL_RPAREN = 37,                    /* RPAREN  */
  YYSYMBOL_LBRACE = 38,                    /* LBRACE  */
  YYSYMBOL_RBRACE = 39,                    /* RBRACE  */
  YYSYMBOL_LBRACKET = 40,                  /* LBRACKET  */
  YYSYMBOL_RBRACKET = 41,                  /* RBRACKET  */
  YYSYMBOL_LOWER_THAN_ELSE = 42,           /* LOWER_THAN_ELSE  */
  YYSYMBOL_UMINUS = 43,                    /* UMINUS  */
  YYSYMBOL_YYACCEPT = 44,                  /* $accept  */
  YYSYMBOL_program = 45,                   /* program  */
  YYSYMBOL_declaration_list = 46,          /* declaration_list  */
  YYSYMBOL_declaration = 47,               /* declaration  */
  YYSYMBOL_variable_declaration = 48,      /* variable_declaration  */
  YYSYMBOL_variable_list = 49,             /* variable_list  */
  YYSYMBOL_variable = 50,                  /* variable  */
  YYSYMBOL_decl_dimensions = 51,           /* decl_dimensions  */
  YYSYMBOL_access_dimensions = 52,         /* access_dimensions  */
  YYSYMBOL_expression_list = 53,           /* expression_list  */
  YYSYMBOL_initializer_list = 54,          /* initializer_list  */
  YYSYMBOL_initializer_items = 55,         /* initializer_items  */
  YYSYMBOL_array_access = 56,              /* array_access  */
  YYSYMBOL_type_specifier = 57,            /* type_specifier  */
  YYSYMBOL_function_declaration = 58,      /* function_declaration  */
  YYSYMBOL_parameters = 59,                /* parameters  */
  YYSYMBOL_parameter_list = 60,            /* parameter_list  */
  YYSYMBOL_parameter = 61,                 /* parameter  */
  YYSYMBOL_function_body = 62,             /* function_body  */
  YYSYMBOL_statements = 63,                /* statements  */
  YYSYMBOL_statement = 64,                 /* statement  */
  YYSYMBOL_return_stmt = 65,               /* return_stmt  */
  YYSYMBOL_compound_stmt = 66,             /* compound_stmt  */
  YYSYMBOL_statement_list = 67,            /* statement_list  */
  YYSYMBOL_expression_stmt = 68,           /* expression_stmt  */
  YYSYMBOL_selection_stmt = 69,            /* selection_stmt  */
  YYSYMBOL_iteration_stmt = 70,            /* iteration_stmt  */
  YYSYMBOL_io_stmt = 71,                   /* io_stmt  */
  YYSYMBOL_expression = 72,                /* expression  */
  YYSYMBOL_function_call = 73,             /* function_call  */
  YYSYMBOL_arguments = 74,                 /* arguments  */
  YYSYMBOL_argument_list = 75              /* argument_list  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  12
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   405

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  44
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  32
/* YYNRULES -- Number of rules.  */
#define YYNRULES  94
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  170

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   298


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   218,   218,   222,   231,   232,   233,   237,   238,   242,
     243,   247,   254,   258,   259,   264,   269,   274,   282,   289,
     294,   301,   306,   313,   317,   318,   319,   323,   330,   338,
     346,   347,   348,   349,   350,   354,   359,   368,   369,   373,
     380,   384,   388,   393,   402,   406,   407,   408,   409,   410,
     411,   412,   413,   417,   418,   422,   426,   427,   431,   432,
     436,   437,   441,   445,   446,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     479,   484,   485,   489,   496
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  static const char *const yy_sname[] =
  {
  "end of file", "error", "invalid token", "KW_INT", "KW_FLOAT",
  "KW_CHAR", "KW_STRING", "KW_VOID", "KW_IF", "KW_ELSE", "KW_WHILE",
  "KW_PRINT", "KW_READ", "KW_RETURN", "ID", "INTNUM", "CHARLIT",
  "FLOATNUM", "STRINGLIT", "LT", "GT", "LE", "GE", "EQ", "NE", "AND", "OR",
  "NOT", "PLUS", "MINUS", "MULT", "DIV", "MOD", "ASSIG", "SEMCOL", "COMMA",
  "LPAREN", "RPAREN", "LBRACE", "RBRACE", "LBRACKET", "RBRACKET",
  "LOWER_THAN_ELSE", "UMINUS", "$accept", "program", "declaration_list",
  "declaration", "variable_declaration", "variable_list", "variable",
  "decl_dimensions", "access_dimensions", "expression_list",
  "initializer_list", "initializer_items", "array_access",
  "type_specifier", "function_declaration", "parameters", "parameter_list",
  "parameter", "function_body", "statements", "statement", "return_stmt",
  "compound_stmt", "statement_list", "expression_stmt", "selection_stmt",
  "iteration_stmt", "io_stmt", "expression", "function_call", "arguments",
  "argument_list", YY_NULLPTR
  };
  return yy_sname[yysymbol];
}
#endif

#define YYPACT_NINF (-114)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-58)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     137,  -114,  -114,  -114,  -114,  -114,    29,    53,  -114,  -114,
      16,  -114,  -114,   -24,  -114,    -3,    50,   -14,  -114,  -114,
    -114,  -114,   219,   137,    34,    56,  -114,    37,    52,  -114,
    -114,  -114,  -114,   219,   219,   219,    30,   345,  -114,    70,
      13,    31,  -114,    58,   166,    82,    61,  -114,   219,   219,
     219,    72,  -114,  -114,   237,   219,   219,   219,   219,   219,
     219,   219,   219,   219,   219,   219,   219,   219,   219,  -114,
      92,    92,   137,  -114,   166,  -114,   345,   105,   345,   345,
      86,   118,   186,   219,  -114,   345,   221,   221,   221,   221,
     373,   373,    85,   359,   169,   169,  -114,  -114,  -114,    64,
    -114,  -114,  -114,   110,   112,   117,   345,  -114,  -114,   219,
    -114,   200,   120,   122,   126,   133,   143,   -11,  -114,    64,
    -114,    18,   121,  -114,  -114,  -114,   146,  -114,  -114,  -114,
    -114,   313,   219,  -114,   148,  -114,   148,  -114,   345,  -114,
    -114,   219,   219,   219,   174,  -114,   329,   151,  -114,  -114,
    -114,   345,  -114,  -114,   256,   275,   294,   154,  -114,  -114,
     160,   160,   158,   163,   194,  -114,  -114,  -114,   160,  -114
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,    30,    31,    32,    33,    34,     0,     0,     6,     7,
       0,     8,     1,     0,     3,     0,    13,     0,    12,     4,
       5,    10,     0,    38,     0,    16,     9,     0,    84,    85,
      87,    86,    88,     0,     0,     0,    83,    17,    89,     0,
       0,    37,    40,     0,     0,     0,    13,    11,     0,    92,
       0,    29,    80,    81,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,    19,     0,    15,    14,     0,    65,    94,
       0,    91,     0,     0,    82,    66,    72,    73,    74,    75,
      76,    77,    78,    79,    67,    68,    69,    70,    71,     0,
      36,    35,    39,     0,     0,     0,    23,    18,    90,     0,
      21,     0,     0,     0,     0,     0,     0,     0,    59,     0,
      50,     0,     0,    44,    51,    46,     0,    45,    47,    48,
      49,     0,     0,    24,     0,    26,     0,    25,    93,    20,
      52,     0,     0,     0,     0,    54,     0,     0,    43,    42,
      58,    22,    28,    27,     0,     0,     0,     0,    53,    55,
       0,     0,     0,     0,    60,    62,    63,    64,     0,    61
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -114,  -114,  -114,   206,   111,  -114,   211,  -114,  -114,  -114,
     -72,  -114,  -114,     1,  -114,  -114,  -114,   167,   171,  -114,
    -113,  -114,  -114,   124,  -114,  -114,  -114,  -114,   -22,  -114,
    -114,  -114
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     6,     7,     8,   120,    17,    18,    25,    51,   103,
      75,   105,    36,   121,    11,    40,    41,    42,   100,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,    38,
      80,    81
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      37,    10,   104,    28,    29,    30,    31,    32,    10,   148,
      19,    52,    53,    54,    70,    20,    33,    15,    34,    15,
      26,    27,    76,   145,    39,    35,    78,    79,    82,    12,
      16,    21,    46,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,   164,   165,    43,
      71,    46,   106,    -2,    13,   169,     1,     2,     3,     4,
       5,   111,   152,    55,   153,   112,    72,     1,     2,     3,
       4,     5,   113,    39,   114,   115,   116,   117,    28,    29,
      30,    31,    32,    22,    69,    48,    23,   138,    49,    44,
      24,    33,    50,    34,    22,   146,    45,    77,   118,    73,
      35,    24,   119,   -57,    56,    57,    58,    59,    60,    61,
     151,     9,    83,    64,    65,    66,    67,    68,     9,   154,
     155,   156,   112,   108,     1,     2,     3,     4,     5,   113,
      99,   114,   115,   116,   117,    28,    29,    30,    31,    32,
       1,     2,     3,     4,     5,   132,   107,   134,    33,   133,
      34,   135,   136,   109,   140,   118,   137,    35,   141,   119,
     -56,   112,   142,     1,     2,     3,     4,     5,   113,   143,
     114,   115,   116,   117,    28,    29,    30,    31,    32,   144,
      28,    29,    30,    31,    32,   149,    74,    33,   157,    34,
     159,   163,   166,    33,   118,    34,    35,   167,   119,    66,
      67,    68,    35,   168,    74,    56,    57,    58,    59,    60,
      61,    62,    63,    14,    64,    65,    66,    67,    68,    56,
      57,    58,    59,    60,    61,    62,    63,   110,    64,    65,
      66,    67,    68,    28,    29,    30,    31,    32,    47,   102,
       0,   139,   101,   147,     0,     0,    33,     0,    34,    64,
      65,    66,    67,    68,     0,    35,    56,    57,    58,    59,
      60,    61,    62,    63,     0,    64,    65,    66,    67,    68,
       0,     0,     0,     0,    84,    56,    57,    58,    59,    60,
      61,    62,    63,     0,    64,    65,    66,    67,    68,     0,
       0,     0,     0,   160,    56,    57,    58,    59,    60,    61,
      62,    63,     0,    64,    65,    66,    67,    68,     0,     0,
       0,     0,   161,    56,    57,    58,    59,    60,    61,    62,
      63,     0,    64,    65,    66,    67,    68,     0,     0,     0,
       0,   162,    56,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,    67,    68,     0,   150,    56,    57,
      58,    59,    60,    61,    62,    63,     0,    64,    65,    66,
      67,    68,     0,   158,    56,    57,    58,    59,    60,    61,
      62,    63,     0,    64,    65,    66,    67,    68,    56,    57,
      58,    59,    60,    61,    62,     0,     0,    64,    65,    66,
      67,    68,    56,    57,    58,    59,     0,     0,     0,     0,
       0,    64,    65,    66,    67,    68
};

static const yytype_int16 yycheck[] =
{
      22,     0,    74,    14,    15,    16,    17,    18,     7,   122,
      34,    33,    34,    35,     1,    39,    27,     1,    29,     1,
      34,    35,    44,    34,    23,    36,    48,    49,    50,     0,
      14,    34,    14,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,   160,   161,    15,
      37,    14,    74,     0,     1,   168,     3,     4,     5,     6,
       7,    83,   134,    33,   136,     1,    35,     3,     4,     5,
       6,     7,     8,    72,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    33,    14,    33,    36,   109,    36,    33,
      40,    27,    40,    29,    33,   117,    40,    15,    34,    41,
      36,    40,    38,    39,    19,    20,    21,    22,    23,    24,
     132,     0,    40,    28,    29,    30,    31,    32,     7,   141,
     142,   143,     1,    37,     3,     4,     5,     6,     7,     8,
      38,    10,    11,    12,    13,    14,    15,    16,    17,    18,
       3,     4,     5,     6,     7,    35,    41,    35,    27,    39,
      29,    39,    35,    35,    34,    34,    39,    36,    36,    38,
      39,     1,    36,     3,     4,     5,     6,     7,     8,    36,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    36,
      14,    15,    16,    17,    18,    39,    38,    27,    14,    29,
      39,    37,    34,    27,    34,    29,    36,    34,    38,    30,
      31,    32,    36,     9,    38,    19,    20,    21,    22,    23,
      24,    25,    26,     7,    28,    29,    30,    31,    32,    19,
      20,    21,    22,    23,    24,    25,    26,    41,    28,    29,
      30,    31,    32,    14,    15,    16,    17,    18,    27,    72,
      -1,    41,    71,   119,    -1,    -1,    27,    -1,    29,    28,
      29,    30,    31,    32,    -1,    36,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    28,    29,    30,    31,    32,
      -1,    -1,    -1,    -1,    37,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    28,    29,    30,    31,    32,    -1,
      -1,    -1,    -1,    37,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    28,    29,    30,    31,    32,    -1,    -1,
      -1,    -1,    37,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    28,    29,    30,    31,    32,    -1,    -1,    -1,
      -1,    37,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    28,    29,    30,    31,    32,    -1,    34,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    28,    29,    30,
      31,    32,    -1,    34,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    28,    29,    30,    31,    32,    19,    20,
      21,    22,    23,    24,    25,    -1,    -1,    28,    29,    30,
      31,    32,    19,    20,    21,    22,    -1,    -1,    -1,    -1,
      -1,    28,    29,    30,    31,    32
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,     6,     7,    45,    46,    47,    48,
      57,    58,     0,     1,    47,     1,    14,    49,    50,    34,
      39,    34,    33,    36,    40,    51,    34,    35,    14,    15,
      16,    17,    18,    27,    29,    36,    56,    72,    73,    57,
      59,    60,    61,    15,    33,    40,    14,    50,    33,    36,
      40,    52,    72,    72,    72,    33,    19,    20,    21,    22,
      23,    24,    25,    26,    28,    29,    30,    31,    32,    14,
       1,    37,    35,    41,    38,    54,    72,    15,    72,    72,
      74,    75,    72,    40,    37,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    38,
      62,    62,    61,    53,    54,    55,    72,    41,    37,    35,
      41,    72,     1,     8,    10,    11,    12,    13,    34,    38,
      48,    57,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    35,    39,    35,    39,    35,    39,    72,    41,
      34,    36,    36,    36,    36,    34,    72,    67,    64,    39,
      34,    72,    54,    54,    72,    72,    72,    14,    34,    39,
      37,    37,    37,    37,    64,    64,    34,    34,     9,    64
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    44,    45,    46,    46,    46,    46,    47,    47,    48,
      48,    49,    49,    50,    50,    50,    50,    50,    51,    51,
      52,    52,    53,    53,    54,    54,    54,    55,    55,    56,
      57,    57,    57,    57,    57,    58,    58,    59,    59,    60,
      60,    61,    62,    63,    63,    64,    64,    64,    64,    64,
      64,    64,    64,    65,    65,    66,    67,    67,    68,    68,
      69,    69,    70,    71,    71,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      73,    74,    74,    75,    75
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     3,     3,     1,     1,     1,     3,
       3,     3,     1,     1,     4,     4,     2,     3,     4,     3,
       4,     3,     3,     1,     3,     3,     3,     3,     3,     2,
       1,     1,     1,     1,     1,     6,     6,     1,     0,     3,
       1,     2,     3,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     3,     2,     3,     1,     0,     2,     1,
       5,     7,     5,     5,     5,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     3,     1,     1,     1,     1,     1,     1,     1,
       4,     1,     0,     3,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]));
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif



static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yystrlen (yysymbol_name (yyarg[yyi]));
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp = yystpcpy (yyp, yysymbol_name (yyarg[yyi++]));
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* program: declaration_list  */
#line 218 "parser.y"
                       { root = (yyvsp[0].node); (yyval.node) = (yyvsp[0].node); }
#line 1799 "parser.tab.c"
    break;

  case 3: /* declaration_list: declaration_list declaration  */
#line 223 "parser.y"
      {
          if ((yyvsp[-1].node) != NULL) {
              ASTNode *t = (yyvsp[-1].node);
              while (t->next != NULL) t = t->next;
              t->next = (yyvsp[0].node);
              (yyval.node) = (yyvsp[-1].node);
          } else { (yyval.node) = (yyvsp[0].node); }
      }
#line 1812 "parser.tab.c"
    break;

  case 4: /* declaration_list: declaration_list error SEMCOL  */
#line 231 "parser.y"
                                    { yyerrok; (yyval.node) = (yyvsp[-2].node); }
#line 1818 "parser.tab.c"
    break;

  case 5: /* declaration_list: declaration_list error RBRACE  */
#line 232 "parser.y"
                                    { yyerrok; (yyval.node) = (yyvsp[-2].node); }
#line 1824 "parser.tab.c"
    break;

  case 6: /* declaration_list: declaration  */
#line 233 "parser.y"
                  { (yyval.node) = (yyvsp[0].node); }
#line 1830 "parser.tab.c"
    break;

  case 7: /* declaration: variable_declaration  */
#line 237 "parser.y"
                           { (yyval.node) = (yyvsp[0].node); }
#line 1836 "parser.tab.c"
    break;

  case 8: /* declaration: function_declaration  */
#line 238 "parser.y"
                           { (yyval.node) = (yyvsp[0].node); }
#line 1842 "parser.tab.c"
    break;

  case 9: /* variable_declaration: type_specifier variable_list SEMCOL  */
#line 242 "parser.y"
                                          { (yyval.node) = create_node(AST_VAR_DECL, (yyvsp[-2].node), (yyvsp[-1].node), NULL, NULL); }
#line 1848 "parser.tab.c"
    break;

  case 10: /* variable_declaration: type_specifier error SEMCOL  */
#line 243 "parser.y"
                                  { syntax_error_msg_bef("Invalid variable declaration"); yyerrok; (yyval.node) = NULL; }
#line 1854 "parser.tab.c"
    break;

  case 11: /* variable_list: variable_list COMMA variable  */
#line 248 "parser.y"
      {
          ASTNode *t = (yyvsp[-2].node);
          while (t->next != NULL) t = t->next;
          t->next = (yyvsp[0].node);
          (yyval.node) = (yyvsp[-2].node);
      }
#line 1865 "parser.tab.c"
    break;

  case 12: /* variable_list: variable  */
#line 254 "parser.y"
               { (yyval.node) = (yyvsp[0].node); }
#line 1871 "parser.tab.c"
    break;

  case 13: /* variable: ID  */
#line 258 "parser.y"
         { (yyval.node) = create_leaf_id((yyvsp[0].str), (yylsp[0]).first_line, (yylsp[0]).first_column); }
#line 1877 "parser.tab.c"
    break;

  case 14: /* variable: ID decl_dimensions ASSIG expression  */
#line 260 "parser.y"
      {
          ASTNode *id_node = create_leaf_id((yyvsp[-3].str), (yylsp[-3]).first_line, (yylsp[-3]).first_column);
          (yyval.node) = create_node(AST_ARRAY_DECL, id_node, (yyvsp[-2].node), (yyvsp[0].node), NULL);
      }
#line 1886 "parser.tab.c"
    break;

  case 15: /* variable: ID decl_dimensions ASSIG initializer_list  */
#line 265 "parser.y"
      {
          ASTNode *id_node = create_leaf_id((yyvsp[-3].str), (yylsp[-3]).first_line, (yylsp[-3]).first_column);
          (yyval.node) = create_node(AST_ARRAY_DECL, id_node, (yyvsp[-2].node), (yyvsp[0].node), NULL);
      }
#line 1895 "parser.tab.c"
    break;

  case 16: /* variable: ID decl_dimensions  */
#line 270 "parser.y"
      {
          ASTNode *id_node = create_leaf_id((yyvsp[-1].str), (yylsp[-1]).first_line, (yylsp[-1]).first_column);
          (yyval.node) = create_node(AST_ARRAY_DECL, id_node, (yyvsp[0].node), NULL, NULL);
      }
#line 1904 "parser.tab.c"
    break;

  case 17: /* variable: ID ASSIG expression  */
#line 275 "parser.y"
      {
          ASTNode *id_node = create_leaf_id((yyvsp[-2].str), (yylsp[-2]).first_line, (yylsp[-2]).first_column);
          (yyval.node) = create_node(AST_ASSIGN, id_node, (yyvsp[0].node), NULL, NULL);
      }
#line 1913 "parser.tab.c"
    break;

  case 18: /* decl_dimensions: decl_dimensions LBRACKET INTNUM RBRACKET  */
#line 283 "parser.y"
      {
          ASTNode *t = (yyvsp[-3].node);
          while (t->next != NULL) t = t->next;
          t->next = create_leaf_int((yyvsp[-1].ival), (yylsp[-1]).first_line, (yylsp[-1]).first_column);
          (yyval.node) = (yyvsp[-3].node);
      }
#line 1924 "parser.tab.c"
    break;

  case 19: /* decl_dimensions: LBRACKET INTNUM RBRACKET  */
#line 290 "parser.y"
      { (yyval.node) = create_leaf_int((yyvsp[-1].ival), (yylsp[-1]).first_line, (yylsp[-1]).first_column); }
#line 1930 "parser.tab.c"
    break;

  case 20: /* access_dimensions: access_dimensions LBRACKET expression RBRACKET  */
#line 295 "parser.y"
      {
          ASTNode *t = (yyvsp[-3].node);
          while (t->next != NULL) t = t->next;
          t->next = (yyvsp[-1].node);
          (yyval.node) = (yyvsp[-3].node);
      }
#line 1941 "parser.tab.c"
    break;

  case 21: /* access_dimensions: LBRACKET expression RBRACKET  */
#line 302 "parser.y"
      { (yyval.node) = (yyvsp[-1].node); }
#line 1947 "parser.tab.c"
    break;

  case 22: /* expression_list: expression_list COMMA expression  */
#line 307 "parser.y"
      {
          ASTNode *t = (yyvsp[-2].node);
          while (t->next != NULL) t = t->next;
          t->next = (yyvsp[0].node);
          (yyval.node) = (yyvsp[-2].node);
      }
#line 1958 "parser.tab.c"
    break;

  case 23: /* expression_list: expression  */
#line 313 "parser.y"
                 { (yyval.node) = (yyvsp[0].node); }
#line 1964 "parser.tab.c"
    break;

  case 24: /* initializer_list: LBRACE expression_list RBRACE  */
#line 317 "parser.y"
                                    { (yyval.node) = create_node(AST_ARRAY_INIT, (yyvsp[-1].node), NULL, NULL, NULL); }
#line 1970 "parser.tab.c"
    break;

  case 25: /* initializer_list: LBRACE initializer_items RBRACE  */
#line 318 "parser.y"
                                      { (yyval.node) = create_node(AST_ARRAY_INIT, (yyvsp[-1].node), NULL, NULL, NULL); }
#line 1976 "parser.tab.c"
    break;

  case 26: /* initializer_list: LBRACE initializer_list RBRACE  */
#line 319 "parser.y"
                                     { (yyval.node) = create_node(AST_ARRAY_INIT, (yyvsp[-1].node), NULL, NULL, NULL); }
#line 1982 "parser.tab.c"
    break;

  case 27: /* initializer_items: initializer_items COMMA initializer_list  */
#line 324 "parser.y"
      {
          ASTNode *t = (yyvsp[-2].node);
          while (t->next != NULL) t = t->next;
          t->next = (yyvsp[0].node);
          (yyval.node) = (yyvsp[-2].node);
      }
#line 1993 "parser.tab.c"
    break;

  case 28: /* initializer_items: initializer_list COMMA initializer_list  */
#line 331 "parser.y"
      {
          (yyvsp[-2].node)->next = (yyvsp[0].node);
          (yyval.node) = (yyvsp[-2].node);
      }
#line 2002 "parser.tab.c"
    break;

  case 29: /* array_access: ID access_dimensions  */
#line 339 "parser.y"
      {
          ASTNode *id_node = create_leaf_id((yyvsp[-1].str), (yylsp[-1]).first_line, (yylsp[-1]).first_column);
          (yyval.node) = create_node(AST_ARRAY_ACCESS, id_node, (yyvsp[0].node), NULL, NULL);
      }
#line 2011 "parser.tab.c"
    break;

  case 30: /* type_specifier: KW_INT  */
#line 346 "parser.y"
             { (yyval.node) = create_node(AST_TYPE, NULL, NULL, NULL, NULL); (yyval.node)->lexema = strdup("int"); }
#line 2017 "parser.tab.c"
    break;

  case 31: /* type_specifier: KW_FLOAT  */
#line 347 "parser.y"
               { (yyval.node) = create_node(AST_TYPE, NULL, NULL, NULL, NULL); (yyval.node)->lexema = strdup("float"); }
#line 2023 "parser.tab.c"
    break;

  case 32: /* type_specifier: KW_CHAR  */
#line 348 "parser.y"
              { (yyval.node) = create_node(AST_TYPE, NULL, NULL, NULL, NULL); (yyval.node)->lexema = strdup("char"); }
#line 2029 "parser.tab.c"
    break;

  case 33: /* type_specifier: KW_STRING  */
#line 349 "parser.y"
                { (yyval.node) = create_node(AST_TYPE, NULL, NULL, NULL, NULL); (yyval.node)->lexema = strdup("string"); }
#line 2035 "parser.tab.c"
    break;

  case 34: /* type_specifier: KW_VOID  */
#line 350 "parser.y"
              { (yyval.node) = create_node(AST_TYPE, NULL, NULL, NULL, NULL); (yyval.node)->lexema = strdup("void"); }
#line 2041 "parser.tab.c"
    break;

  case 35: /* function_declaration: type_specifier ID LPAREN parameters RPAREN function_body  */
#line 355 "parser.y"
      {
          ASTNode *id_node = create_leaf_id((yyvsp[-4].str), (yylsp[-4]).first_line, (yylsp[-4]).first_column);
          (yyval.node) = create_node(AST_FUNC_DECL, (yyvsp[-5].node), id_node, (yyvsp[-2].node), (yyvsp[0].node));
      }
#line 2050 "parser.tab.c"
    break;

  case 36: /* function_declaration: type_specifier ID LPAREN parameters error function_body  */
#line 360 "parser.y"
      {
          syntax_error_msg_bef("Missing ')'");
          ASTNode *id_node = create_leaf_id((yyvsp[-4].str), (yylsp[-4]).first_line, (yylsp[-4]).first_column);
          (yyval.node) = create_node(AST_FUNC_DECL, (yyvsp[-5].node), id_node, (yyvsp[-2].node), (yyvsp[0].node));
      }
#line 2060 "parser.tab.c"
    break;

  case 37: /* parameters: parameter_list  */
#line 368 "parser.y"
                     { (yyval.node) = (yyvsp[0].node); }
#line 2066 "parser.tab.c"
    break;

  case 38: /* parameters: %empty  */
#line 369 "parser.y"
                  { (yyval.node) = NULL; }
#line 2072 "parser.tab.c"
    break;

  case 39: /* parameter_list: parameter_list COMMA parameter  */
#line 374 "parser.y"
      {
          ASTNode *t = (yyvsp[-2].node);
          while (t->next != NULL) t = t->next;
          t->next = (yyvsp[0].node);
          (yyval.node) = (yyvsp[-2].node);
      }
#line 2083 "parser.tab.c"
    break;

  case 40: /* parameter_list: parameter  */
#line 380 "parser.y"
                { (yyval.node) = (yyvsp[0].node); }
#line 2089 "parser.tab.c"
    break;

  case 41: /* parameter: type_specifier ID  */
#line 384 "parser.y"
                        { ASTNode *id_node = create_leaf_id((yyvsp[0].str), (yylsp[0]).first_line, (yylsp[0]).first_column); (yyval.node) = create_node(AST_PARAM, (yyvsp[-1].node), id_node, NULL, NULL); }
#line 2095 "parser.tab.c"
    break;

  case 42: /* function_body: LBRACE statement_list RBRACE  */
#line 389 "parser.y"
      { (yyval.node) = create_node(AST_COMPOUND, (yyvsp[-1].node), NULL, NULL, NULL); }
#line 2101 "parser.tab.c"
    break;

  case 43: /* statements: statements statement  */
#line 394 "parser.y"
      {
          if ((yyvsp[-1].node) != NULL) {
              ASTNode *t = (yyvsp[-1].node);
              while (t->next != NULL) t = t->next;
              t->next = (yyvsp[0].node);
              (yyval.node) = (yyvsp[-1].node);
          } else { (yyval.node) = (yyvsp[0].node); }
      }
#line 2114 "parser.tab.c"
    break;

  case 44: /* statements: statement  */
#line 402 "parser.y"
                { (yyval.node) = (yyvsp[0].node); }
#line 2120 "parser.tab.c"
    break;

  case 45: /* statement: expression_stmt  */
#line 406 "parser.y"
                      { (yyval.node) = (yyvsp[0].node); }
#line 2126 "parser.tab.c"
    break;

  case 46: /* statement: compound_stmt  */
#line 407 "parser.y"
                    { (yyval.node) = (yyvsp[0].node); }
#line 2132 "parser.tab.c"
    break;

  case 47: /* statement: selection_stmt  */
#line 408 "parser.y"
                     { (yyval.node) = (yyvsp[0].node); }
#line 2138 "parser.tab.c"
    break;

  case 48: /* statement: iteration_stmt  */
#line 409 "parser.y"
                     { (yyval.node) = (yyvsp[0].node); }
#line 2144 "parser.tab.c"
    break;

  case 49: /* statement: io_stmt  */
#line 410 "parser.y"
              { (yyval.node) = (yyvsp[0].node); }
#line 2150 "parser.tab.c"
    break;

  case 50: /* statement: variable_declaration  */
#line 411 "parser.y"
                           { (yyval.node) = (yyvsp[0].node); }
#line 2156 "parser.tab.c"
    break;

  case 51: /* statement: return_stmt  */
#line 412 "parser.y"
                  { (yyval.node) = (yyvsp[0].node); }
#line 2162 "parser.tab.c"
    break;

  case 52: /* statement: error SEMCOL  */
#line 413 "parser.y"
                   { syntax_error_msg_bef("Invalid command"); yyerrok; (yyval.node) = NULL; }
#line 2168 "parser.tab.c"
    break;

  case 53: /* return_stmt: KW_RETURN expression SEMCOL  */
#line 417 "parser.y"
                                  { (yyval.node) = create_node(AST_RETURN, (yyvsp[-1].node), NULL, NULL, NULL); }
#line 2174 "parser.tab.c"
    break;

  case 54: /* return_stmt: KW_RETURN SEMCOL  */
#line 418 "parser.y"
                       { (yyval.node) = create_node(AST_RETURN, NULL, NULL, NULL, NULL); }
#line 2180 "parser.tab.c"
    break;

  case 55: /* compound_stmt: LBRACE statement_list RBRACE  */
#line 422 "parser.y"
                                   { (yyval.node) = create_node(AST_COMPOUND, (yyvsp[-1].node), NULL, NULL, NULL); }
#line 2186 "parser.tab.c"
    break;

  case 56: /* statement_list: statements  */
#line 426 "parser.y"
                 { (yyval.node) = (yyvsp[0].node); }
#line 2192 "parser.tab.c"
    break;

  case 57: /* statement_list: %empty  */
#line 427 "parser.y"
                  { (yyval.node) = NULL; }
#line 2198 "parser.tab.c"
    break;

  case 58: /* expression_stmt: expression SEMCOL  */
#line 431 "parser.y"
                        { (yyval.node) = (yyvsp[-1].node); }
#line 2204 "parser.tab.c"
    break;

  case 59: /* expression_stmt: SEMCOL  */
#line 432 "parser.y"
             { (yyval.node) = NULL; }
#line 2210 "parser.tab.c"
    break;

  case 60: /* selection_stmt: KW_IF LPAREN expression RPAREN statement  */
#line 436 "parser.y"
                                                                     { (yyval.node) = create_node(AST_IF, (yyvsp[-2].node), (yyvsp[0].node), NULL, NULL); }
#line 2216 "parser.tab.c"
    break;

  case 61: /* selection_stmt: KW_IF LPAREN expression RPAREN statement KW_ELSE statement  */
#line 437 "parser.y"
                                                                 { (yyval.node) = create_node(AST_IF, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node), NULL); }
#line 2222 "parser.tab.c"
    break;

  case 62: /* iteration_stmt: KW_WHILE LPAREN expression RPAREN statement  */
#line 441 "parser.y"
                                                  { (yyval.node) = create_node(AST_WHILE, (yyvsp[-2].node), (yyvsp[0].node), NULL, NULL); }
#line 2228 "parser.tab.c"
    break;

  case 63: /* io_stmt: KW_PRINT LPAREN expression RPAREN SEMCOL  */
#line 445 "parser.y"
                                               { (yyval.node) = create_node(AST_PRINT, (yyvsp[-2].node), NULL, NULL, NULL); }
#line 2234 "parser.tab.c"
    break;

  case 64: /* io_stmt: KW_READ LPAREN ID RPAREN SEMCOL  */
#line 446 "parser.y"
                                      { ASTNode *id_node = create_leaf_id((yyvsp[-2].str), (yylsp[-2]).first_line, (yylsp[-2]).first_column);
      (yyval.node) = create_node(AST_READ, id_node, NULL, NULL, NULL); }
#line 2241 "parser.tab.c"
    break;

  case 65: /* expression: ID ASSIG expression  */
#line 451 "parser.y"
                          { ASTNode *id_node = create_leaf_id((yyvsp[-2].str), (yylsp[-2]).first_line, (yylsp[-2]).first_column); (yyval.node) = create_node(AST_ASSIGN, id_node, (yyvsp[0].node), NULL, NULL); }
#line 2247 "parser.tab.c"
    break;

  case 66: /* expression: array_access ASSIG expression  */
#line 452 "parser.y"
                                    { (yyval.node) = create_node(AST_ASSIGN, (yyvsp[-2].node), (yyvsp[0].node), NULL, NULL); }
#line 2253 "parser.tab.c"
    break;

  case 67: /* expression: expression PLUS expression  */
#line 453 "parser.y"
                                 { (yyval.node) = create_node(AST_BINOP, (yyvsp[-2].node), (yyvsp[0].node), NULL, NULL); (yyval.node)->lexema = strdup("+"); }
#line 2259 "parser.tab.c"
    break;

  case 68: /* expression: expression MINUS expression  */
#line 454 "parser.y"
                                  { (yyval.node) = create_node(AST_BINOP, (yyvsp[-2].node), (yyvsp[0].node), NULL, NULL); (yyval.node)->lexema = strdup("-"); }
#line 2265 "parser.tab.c"
    break;

  case 69: /* expression: expression MULT expression  */
#line 455 "parser.y"
                                 { (yyval.node) = create_node(AST_BINOP, (yyvsp[-2].node), (yyvsp[0].node), NULL, NULL); (yyval.node)->lexema = strdup("*"); }
#line 2271 "parser.tab.c"
    break;

  case 70: /* expression: expression DIV expression  */
#line 456 "parser.y"
                                { (yyval.node) = create_node(AST_BINOP, (yyvsp[-2].node), (yyvsp[0].node), NULL, NULL); (yyval.node)->lexema = strdup("/"); }
#line 2277 "parser.tab.c"
    break;

  case 71: /* expression: expression MOD expression  */
#line 457 "parser.y"
                                { (yyval.node) = create_node(AST_BINOP, (yyvsp[-2].node), (yyvsp[0].node), NULL, NULL); (yyval.node)->lexema = strdup("%"); }
#line 2283 "parser.tab.c"
    break;

  case 72: /* expression: expression LT expression  */
#line 458 "parser.y"
                               { (yyval.node) = create_node(AST_BINOP, (yyvsp[-2].node), (yyvsp[0].node), NULL, NULL); (yyval.node)->lexema = strdup("<"); }
#line 2289 "parser.tab.c"
    break;

  case 73: /* expression: expression GT expression  */
#line 459 "parser.y"
                               { (yyval.node) = create_node(AST_BINOP, (yyvsp[-2].node), (yyvsp[0].node), NULL, NULL); (yyval.node)->lexema = strdup(">"); }
#line 2295 "parser.tab.c"
    break;

  case 74: /* expression: expression LE expression  */
#line 460 "parser.y"
                               { (yyval.node) = create_node(AST_BINOP, (yyvsp[-2].node), (yyvsp[0].node), NULL, NULL); (yyval.node)->lexema = strdup("<="); }
#line 2301 "parser.tab.c"
    break;

  case 75: /* expression: expression GE expression  */
#line 461 "parser.y"
                               { (yyval.node) = create_node(AST_BINOP, (yyvsp[-2].node), (yyvsp[0].node), NULL, NULL); (yyval.node)->lexema = strdup(">="); }
#line 2307 "parser.tab.c"
    break;

  case 76: /* expression: expression EQ expression  */
#line 462 "parser.y"
                               { (yyval.node) = create_node(AST_BINOP, (yyvsp[-2].node), (yyvsp[0].node), NULL, NULL); (yyval.node)->lexema = strdup("=="); }
#line 2313 "parser.tab.c"
    break;

  case 77: /* expression: expression NE expression  */
#line 463 "parser.y"
                               { (yyval.node) = create_node(AST_BINOP, (yyvsp[-2].node), (yyvsp[0].node), NULL, NULL); (yyval.node)->lexema = strdup("!="); }
#line 2319 "parser.tab.c"
    break;

  case 78: /* expression: expression AND expression  */
#line 464 "parser.y"
                                { (yyval.node) = create_node(AST_BINOP, (yyvsp[-2].node), (yyvsp[0].node), NULL, NULL); (yyval.node)->lexema = strdup("&&"); }
#line 2325 "parser.tab.c"
    break;

  case 79: /* expression: expression OR expression  */
#line 465 "parser.y"
                               { (yyval.node) = create_node(AST_BINOP, (yyvsp[-2].node), (yyvsp[0].node), NULL, NULL); (yyval.node)->lexema = strdup("||"); }
#line 2331 "parser.tab.c"
    break;

  case 80: /* expression: NOT expression  */
#line 466 "parser.y"
                     { (yyval.node) = create_node(AST_UNOP, (yyvsp[0].node), NULL, NULL, NULL); (yyval.node)->lexema = strdup("!"); (yyval.node)->line = (yylsp[-1]).first_line; (yyval.node)->column = (yylsp[-1]).first_column;}
#line 2337 "parser.tab.c"
    break;

  case 81: /* expression: MINUS expression  */
#line 467 "parser.y"
                                    { (yyval.node) = create_node(AST_UNOP, (yyvsp[0].node), NULL, NULL, NULL); (yyval.node)->lexema = strdup("minus"); (yyval.node)->line = (yylsp[-1]).first_line; (yyval.node)->column = (yylsp[-1]).first_column;}
#line 2343 "parser.tab.c"
    break;

  case 82: /* expression: LPAREN expression RPAREN  */
#line 468 "parser.y"
                               { (yyval.node) = (yyvsp[-1].node); }
#line 2349 "parser.tab.c"
    break;

  case 83: /* expression: array_access  */
#line 469 "parser.y"
                   { (yyval.node) = (yyvsp[0].node); }
#line 2355 "parser.tab.c"
    break;

  case 84: /* expression: ID  */
#line 470 "parser.y"
         { (yyval.node) = create_leaf_id((yyvsp[0].str), (yylsp[0]).first_line, (yylsp[0]).first_column); }
#line 2361 "parser.tab.c"
    break;

  case 85: /* expression: INTNUM  */
#line 471 "parser.y"
             { (yyval.node) = create_leaf_int((yyvsp[0].ival), (yylsp[0]).first_line, (yylsp[0]).first_column); }
#line 2367 "parser.tab.c"
    break;

  case 86: /* expression: FLOATNUM  */
#line 472 "parser.y"
               { (yyval.node) = create_leaf_float((yyvsp[0].fval), (yylsp[0]).first_line, (yylsp[0]).first_column); }
#line 2373 "parser.tab.c"
    break;

  case 87: /* expression: CHARLIT  */
#line 473 "parser.y"
              { (yyval.node) = create_leaf_char((yyvsp[0].ival), (yylsp[0]).first_line, (yylsp[0]).first_column); }
#line 2379 "parser.tab.c"
    break;

  case 88: /* expression: STRINGLIT  */
#line 474 "parser.y"
                { (yyval.node) = create_leaf_string((yyvsp[0].str), (yylsp[0]).first_line, (yylsp[0]).first_column); free((yyvsp[0].str)); }
#line 2385 "parser.tab.c"
    break;

  case 89: /* expression: function_call  */
#line 475 "parser.y"
                    { (yyval.node) = (yyvsp[0].node); }
#line 2391 "parser.tab.c"
    break;

  case 90: /* function_call: ID LPAREN arguments RPAREN  */
#line 479 "parser.y"
                                 { ASTNode *id_node = create_leaf_id((yyvsp[-3].str), (yylsp[-3]).first_line, (yylsp[-3]).first_column);
      (yyval.node) = create_node(AST_FUNC_CALL, id_node, (yyvsp[-1].node), NULL, NULL); }
#line 2398 "parser.tab.c"
    break;

  case 91: /* arguments: argument_list  */
#line 484 "parser.y"
                    { (yyval.node) = (yyvsp[0].node); }
#line 2404 "parser.tab.c"
    break;

  case 92: /* arguments: %empty  */
#line 485 "parser.y"
                  { (yyval.node) = NULL; }
#line 2410 "parser.tab.c"
    break;

  case 93: /* argument_list: argument_list COMMA expression  */
#line 490 "parser.y"
      {
          ASTNode *t = (yyvsp[-2].node);
          while (t->next != NULL) t = t->next;
          t->next = (yyvsp[0].node);
          (yyval.node) = (yyvsp[-2].node);
      }
#line 2421 "parser.tab.c"
    break;

  case 94: /* argument_list: expression  */
#line 496 "parser.y"
                 { (yyval.node) = (yyvsp[0].node); }
#line 2427 "parser.tab.c"
    break;


#line 2431 "parser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 499 "parser.y"


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
    int declaration_line;
    int declaration_column;
    int num_params;
    char param_types[10][20];
    int num_array_dims;
    int array_dims[10];
    int array_total_size;
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

int is_numeric_type(const char *t) {
    return t && (strcmp(t, "int") == 0 || strcmp(t, "float") == 0);
}

int is_char_type(const char *t) {
    return t && strcmp(t, "char") == 0;
}

int is_string_type(const char *t) {
    return t && strcmp(t, "string") == 0;
}

int is_string_like_type(const char *t) {
    return is_char_type(t) || is_string_type(t);
}

void report_type_mismatch(const char *context, const char *expected, const char *got,
                          int line, int col) {
    fprintf(stderr,
            "[SEMANTIC ERROR] Type mismatch in %s: expected '%s', got '%s' at Line %d, Column %d.\n",
            context, expected, got, line, col);
    semantic_error = 1;
}

int check_assignment_types(const char *ltype, const char *rtype, const char *varname,
                           int line, int col) {
    if (!ltype || !rtype) return 0;

    if (strcmp(ltype, rtype) == 0) return 1;

    if (strcmp(ltype, "float") == 0 && strcmp(rtype, "int") == 0) return 1;

    if (strcmp(ltype, "string") == 0 && is_char_type(rtype)) return 1;

    if (strcmp(ltype, "char") == 0 && is_char_type(rtype)) return 1;

    report_type_mismatch(
        varname ? "assignment" : "initialization",
        ltype,
        rtype,
        line,
        col
    );
    if (varname) {
        fprintf(stderr,
                "               Cannot assign '%s' to '%s' variable '%s'.\n",
                rtype, ltype, varname);
    }
    return 0;
}

int is_concat_expression(const char *t1, const char *t2) {
    return is_string_like_type(t1) || is_string_like_type(t2);
}


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

int add_symbol(const char *name, const char *type, const char *category, int line, int column) {
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
    ns->declaration_line = line;
    ns->declaration_column = column;
    ns->num_params = 0;
    
    ns->num_params = 0;
    ns->num_array_dims = 0;
    ns->array_total_size = 0;
    for (int i = 0; i < 10; i++) ns->array_dims[i] = 0;
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

int statement_definitely_returns(ASTNode *node) {
    if (!node) return 0;
    switch (node->type) {
        case AST_RETURN:
            return 1;
        case AST_COMPOUND: {
            ASTNode *stmt = node->child1;
            while (stmt) {
                if (statement_definitely_returns(stmt)) return 1;
                if (stmt->type == AST_RETURN) return 1;
                stmt = stmt->next;
            }
            return 0;
        }
        case AST_IF:
            if (!node->child3) return 0;
            return statement_definitely_returns(node->child2) && statement_definitely_returns(node->child3);
        default:
            return 0;
    }
}

int contains_return(ASTNode *node) {
    if (!node) return 0;
    if (node->type == AST_RETURN) return 1;
    if (node->type == AST_COMPOUND) {
        ASTNode *stmt = node->child1;
        while (stmt) {
            if (contains_return(stmt)) return 1;
            stmt = stmt->next;
        }
        if (node->child2 && contains_return(node->child2)) return 1;
        if (node->child3 && contains_return(node->child3)) return 1;
        return 0;
    }
    if (node->type == AST_IF) {
        int has_true = contains_return(node->child2);
        int has_false = node->child3 ? contains_return(node->child3) : 0;
        return has_true && has_false;
    }
    if (node->type == AST_WHILE) {
        return contains_return(node->child2);
    }
    return 0;
}

/* Coleta recursiva de folhas (sem modificar next) */
#define MAX_INIT_ITEMS 1024
static ASTNode *init_items_buf[MAX_INIT_ITEMS];
static int init_items_count;

void collect_init_leaves(ASTNode *node) {
    if (!node) return;
    if (node->type == AST_ARRAY_INIT) {
        /* Descer recursivamente nos filhos do ARRAY_INIT */
        collect_init_leaves(node->child1);
        /* Processar irmãos (next) do nó ARRAY_INIT */
        collect_init_leaves(node->next);
    } else {
        /* É uma folha (expressão): adicionar ao buffer */
        if (init_items_count < MAX_INIT_ITEMS) {
            init_items_buf[init_items_count++] = node;
        }
        /* Processar irmãos da folha */
        collect_init_leaves(node->next);
    }
}

void collect_array_initializers(ASTNode *node, ASTNode **first, ASTNode **last) {
    init_items_count = 0;
    collect_init_leaves(node);
    *first = NULL;
    *last = NULL;
    for (int i = 0; i < init_items_count; i++) {
        init_items_buf[i]->next = NULL; /* limpa para segurança */
        if (*last) {
            (*last)->next = init_items_buf[i];
            *last = init_items_buf[i];
        } else {
            *first = *last = init_items_buf[i];
        }
    }
}

/* Similar to contains_return but requires returns to have a value (child1 != NULL).
   Used to validate non-void functions have return with expressions on all paths. */
/* Returns 1 if all control-flow paths through 'node' lead to a RETURN with a value. */
int returns_on_all_paths(ASTNode *node) {
    if (!node) return 0;
    switch (node->type) {
        case AST_RETURN:
            return node->child1 != NULL;
        case AST_COMPOUND: {
            ASTNode *stmt = node->child1;
            while (stmt) {
                if (returns_on_all_paths(stmt)) return 1;
                /* if this statement doesn't return on all paths, execution may continue */
                stmt = stmt->next;
            }
            return 0;
        }
        case AST_IF:
            if (!node->child3) return 0;
            return returns_on_all_paths(node->child2) && returns_on_all_paths(node->child3);
        default:
            return 0;
    }
}

char* verificar_semantica(ASTNode *node) {
    if (!node) return "void";
    switch (node->type) {
        case AST_VAR_DECL: {
            char *decl_type = node->child1->lexema;
            ASTNode *var = node->child2; 
            while (var) {
                if (var->type == AST_ID) {
                    if (!add_symbol(var->lexema, decl_type, "var", var->line, var->column)) {
                        fprintf(stderr, "[SEMANTIC ERROR] Variable '%s' redeclared in same scope at Line %d, Column %d.\n", var->lexema, var->line, var->column);
                        semantic_error = 1;
                    }
                } else if (var->type == AST_ASSIGN) {
                    char *vname = var->child1->lexema;
                    if (!add_symbol(vname, decl_type, "var", var->child1->line, var->child1->column)) {
                        fprintf(stderr, "[SEMANTIC ERROR] Variable '%s' redeclared in same scope at Line %d, Column %d.\n", vname, var->child1->line, var->child1->column);
                        semantic_error = 1;
                    }
                    strcpy(var->child1->exp_type, decl_type);
                    
                    /* VALIDAÇÃO DE TIPO NA INICIALIZAÇÃO */
                    char *rtype = verificar_semantica(var->child2);
                    check_assignment_types(decl_type, rtype, vname,
                                           var->child1->line, var->child1->column);
                } else if (var->type == AST_ARRAY_DECL) {
                    char *vname = var->child1->lexema;
                    if (!add_symbol(vname, decl_type, "array", var->child1->line, var->child1->column)) {
                        fprintf(stderr, "[SEMANTIC ERROR] Array '%s' redeclared in same scope at Line %d, Column %d.\n", vname, var->child1->line, var->child1->column);
                        semantic_error = 1;
                    }
                    Symbol *s = lookup_symbol(vname);
                    if (s) {
                        ASTNode *dims = var->child2;
                        int total_size = 1;
                        int idx = 0;
                        while (dims && idx < 10) {
                            if (dims->type == AST_INT) {
                                s->array_dims[idx++] = dims->int_val;
                                total_size *= dims->int_val;
                            }
                            dims = dims->next;
                        }
                        s->num_array_dims = idx;
                        s->array_total_size = total_size;
                    }
                    if (var->child3) {
                        ASTNode *init = var->child3;
                        ASTNode *flat_init = NULL;
                        ASTNode *last_item = NULL;
                        collect_array_initializers(init, &flat_init, &last_item);
                        ASTNode *item = flat_init;
                        int index = 0;
                        int total_size = lookup_symbol(vname)->array_total_size;
                        while (item) {
                            if (index >= total_size) {
                                fprintf(stderr, "[SEMANTIC ERROR] Too many initializers for array '%s' at Line %d, Column %d.\n", vname, var->child1->line, var->child1->column);
                                semantic_error = 1;
                                break;
                            }
                            char *rtype = verificar_semantica(item);
                            if (!check_assignment_types(decl_type, rtype, vname, item->line, item->column)) {
                                fprintf(stderr, "               In initializer of array '%s'.\n", vname);
                            }
                            index++;
                            item = item->next;
                        }
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

            if (strcmp(ret_type, "int") != 0 && strcmp(ret_type, "float") != 0 &&
                strcmp(ret_type, "char") != 0 && strcmp(ret_type, "string") != 0 && strcmp(ret_type, "void") != 0) {
                /* Aponta para o TIPO de retorno que está errado (child1) */
                fprintf(stderr, "[SEMANTIC ERROR] Invalid return type '%s' for function '%s' at Line %d, Column %d. Allowed: int, float, char, string, void.\n", 
                        ret_type, fname, node->child1->line, node->child1->column);
                semantic_error = 1;
            }

            if (!add_symbol(fname, ret_type, "func", node->child2->line, node->child2->column)) {
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
                add_symbol(p2->child2->lexema, p2->child1->lexema, "param", p2->child2->line, p2->child2->column);
                p2 = p2->next;
            }
            is_function_body = 1;
            verificar_semantica(node->child4);

            if (strcmp(ret_type, "void") != 0 && !returns_on_all_paths(node->child4)) {
                fprintf(stderr, "[SEMANTIC ERROR] Missing return statement in function '%s' returning '%s' at Line %d, Column %d.\n",
                        fname, ret_type, node->child2->line, node->child2->column);
                semantic_error = 1;
            }

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
            
            ASTNode *stmt = node->child1;
            int returned = 0;
            while (stmt) {
                if (returned) {
                    fprintf(stderr, "[SEMANTIC ERROR] Unreachable code at Line %d, Column %d.\n", stmt->line, stmt->column);
                    semantic_error = 1;
                }
                verificar_semantica(stmt);
                if (!returned && statement_definitely_returns(stmt)) {
                    returned = 1;
                }
                stmt = stmt->next;
            }
            if (node->child2) verificar_semantica(node->child2);
            if (node->child3) verificar_semantica(node->child3);
            
            if (block_scope_created) {
                close_scope();
            }
            return "void";
        }
        case AST_ID: {
            Symbol *s = lookup_symbol(node->lexema);
            if (!s) {
                fprintf(stderr, "[SEMANTIC ERROR] Variable '%s' used before declaration at Line %d, Column %d.\n",
                        node->lexema, node->line, node->column);
                semantic_error = 1;
                strcpy(node->exp_type, "int");
                return "int";
            }
            strcpy(node->exp_type, s->type);
            return s->type;
        }
        case AST_INT: strcpy(node->exp_type, "int"); return "int";
        case AST_FLOAT: strcpy(node->exp_type, "float"); return "float";
        case AST_CHAR: strcpy(node->exp_type, "char"); return "char";
        case AST_STRING: strcpy(node->exp_type, "string"); return "string";
        case AST_ASSIGN: {
            char *ltype = verificar_semantica(node->child1);
            char *rtype = verificar_semantica(node->child2);
            
            check_assignment_types(ltype, rtype,
                                   node->child1->lexema ? node->child1->lexema : "variable",
                                   node->child1->line, node->child1->column);
            
            strcpy(node->exp_type, ltype);
            return ltype;
        }
        case AST_ARRAY_DECL: {
            char *decl_type = node->child1->lexema;
            ASTNode *dims = node->child2;
            int total_size = 1;
            ASTNode *d = dims;

            while (d) {
                if (d->type != AST_INT) {
                    fprintf(stderr, "[SEMANTIC ERROR] Array dimensions must be integer constants at Line %d, Column %d.\n", d->line, d->column);
                    semantic_error = 1;
                    break;
                }
                if (d->int_val <= 0) {
                    fprintf(stderr, "[SEMANTIC ERROR] Array dimensions must be positive constants at Line %d, Column %d.\n", d->line, d->column);
                    semantic_error = 1;
                }
                total_size *= d->int_val;
                d = d->next;
            }

            if (!add_symbol(node->child1->lexema, decl_type, "array", node->child1->line, node->child1->column)) {
                fprintf(stderr, "[SEMANTIC ERROR] Array '%s' redeclared in same scope at Line %d, Column %d.\n", node->child1->lexema, node->child1->line, node->child1->column);
                semantic_error = 1;
            }

            Symbol *s = lookup_symbol(node->child1->lexema);
            if (s) {
                s->num_array_dims = 0;
                s->array_total_size = total_size;
                ASTNode *dim = dims;
                int idx = 0;
                while (dim && idx < 10) {
                    s->array_dims[idx++] = dim->int_val;
                    dim = dim->next;
                }
                s->num_array_dims = idx;
            }

            if (node->child3) {
                ASTNode *init = node->child3;
                ASTNode *flat_init = NULL;
                ASTNode *last_item = NULL;
                collect_array_initializers(init, &flat_init, &last_item);
                ASTNode *item = flat_init;
                int index = 0;
                while (item) {
                    if (index >= total_size) {
                        fprintf(stderr, "[SEMANTIC ERROR] Too many initializers for array '%s' at Line %d, Column %d.\n", node->child1->lexema, node->child1->line, node->child1->column);
                        semantic_error = 1;
                        break;
                    }
                    char *rtype = verificar_semantica(item);
                    if (!check_assignment_types(decl_type, rtype, node->child1->lexema, item->line, item->column)) {
                        fprintf(stderr, "               In initializer of array '%s'.\n", node->child1->lexema);
                    }
                    index++;
                    item = item->next;
                }
                if (index < total_size) {
                    /* sem problema, faltam inicializadores; inicialização parcial é permitida */
                }
            }
            return "void";
        }
        case AST_ARRAY_ACCESS: {
            Symbol *s = lookup_symbol(node->child1->lexema);
            if (!s) {
                fprintf(stderr, "[SEMANTIC ERROR] Array '%s' used before declaration at Line %d, Column %d.\n", node->child1->lexema, node->child1->line, node->child1->column);
                semantic_error = 1;
                strcpy(node->exp_type, "int");
                return "int";
            }
            if (strcmp(s->category, "array") != 0) {
                fprintf(stderr, "[SEMANTIC ERROR] '%s' is not an array at Line %d, Column %d.\n", node->child1->lexema, node->child1->line, node->child1->column);
                semantic_error = 1;
            }

            int dims = 0;
            ASTNode *idx = node->child2;
            while (idx) {
                char *t = verificar_semantica(idx);
                if (strcmp(t, "int") != 0) {
                    fprintf(stderr, "[SEMANTIC ERROR] Array index must be int at Line %d, Column %d.\n", idx->line, idx->column);
                    semantic_error = 1;
                }
                /* Bounds check: se o índice é constante inteira, verifica limites */
                if (s && dims < s->num_array_dims && idx->type == AST_INT) {
                    int idx_val = idx->int_val;
                    int dim_size = s->array_dims[dims];
                    if (idx_val < 0 || idx_val >= dim_size) {
                        fprintf(stderr, "[SEMANTIC ERROR] Array index out of range: '%s' dimension %d has size %d, but index is %d at Line %d, Column %d.\n",
                                node->child1->lexema, dims + 1, dim_size, idx_val, idx->line, idx->column);
                        semantic_error = 1;
                    }
                }
                dims++;
                idx = idx->next;
            }
            if (s && s->num_array_dims != dims) {
                fprintf(stderr, "[SEMANTIC ERROR] Array '%s' expects %d indices, got %d at Line %d, Column %d.\n",
                        node->child1->lexema, s->num_array_dims, dims, node->child1->line, node->child1->column);
                semantic_error = 1;
            }
            /* Propagar dimensões do símbolo para o nó da AST, pois a tabela de
               símbolos não estará acessível durante a geração de TAC (escopo fechado). */
            if (s) {
                node->num_array_dims = s->num_array_dims;
                for (int i = 0; i < s->num_array_dims; i++)
                    node->array_dims[i] = s->array_dims[i];
            }
            strcpy(node->exp_type, s->type);
            return node->exp_type;
        }
        case AST_BINOP: {
            char *t1 = verificar_semantica(node->child1);
            char *t2 = verificar_semantica(node->child2);
            char *op = node->lexema;

            /* REGRA 1: Concatenação de Strings
               Se a operação é '+' e pelo menos um dos lados é string/char, ambos
               os operandos devem ser string ou char — caso contrário é erro semântico.
               O resultado é sempre string. */
            if (strcmp(op, "+") == 0) {
                int left_is_str = is_string_like_type(t1);
                int right_is_str = is_string_like_type(t2);
                if (left_is_str || right_is_str) {
                    if (!left_is_str || !right_is_str) {
                        fprintf(stderr, "[SEMANTIC ERROR] Both operands of '+' must be string or char for concatenation at Line %d, Column %d.\n", node->line, node->column);
                        semantic_error = 1;
                    }
                    strcpy(node->exp_type, "string");
                    return "string";
                }
            }

            /* REGRA 2: Bloquear outros operadores matemáticos em Strings/Chars
               Se não for um '+' (já capturado acima), e envolver string/char, é erro! */
            if (strcmp(t1, "string") == 0 || strcmp(t2, "string") == 0 || 
                strcmp(t1, "char") == 0 || strcmp(t2, "char") == 0) {
                
                // Exceção: Permitir == e != para comparar chars/strings (se sua linguagem suportar)
                if (strcmp(op, "==") != 0 && strcmp(op, "!=") != 0) {
                    fprintf(stderr,
                            "[SEMANTIC ERROR] Operator '%s' cannot be applied to char/string operands at Line %d, Column %d.\n",
                            op, node->line, node->column);
                    semantic_error = 1;
                }
                strcpy(node->exp_type, "int"); // Tipagem de fallback para erros
                return "int";
            }
            
            /* REGRA 3: Lógica normal para Ints e Floats */
            if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 || strcmp(op, "*") == 0 || strcmp(op, "/") == 0) {
                if (strcmp(t1, "float") == 0 || strcmp(t2, "float") == 0) strcpy(node->exp_type, "float");
                else strcpy(node->exp_type, "int");
            } else if (strcmp(op, "%") == 0) {
                if (strcmp(t1, "float") == 0 || strcmp(t2, "float") == 0) {
                    fprintf(stderr, "[SEMANTIC ERROR] Operator '%%' only accepts int at Line %d, Column %d.\n", node->line, node->column);
                    semantic_error = 1;
                }
                strcpy(node->exp_type, "int");
            } else {
                /* Operadores relacionais (<, >, ==, etc) sempre retornam int (0 ou 1) */
                strcpy(node->exp_type, "int");
            }
            
            return node->exp_type;
        }
        case AST_UNOP: {
            char *t1 = verificar_semantica(node->child1);
            if (is_string_like_type(t1)) {
                fprintf(stderr,
                        "[SEMANTIC ERROR] Unary operator '%s' cannot be applied to '%s' at Line %d, Column %d.\n",
                        node->lexema, t1, node->line, node->column);
                semantic_error = 1;
                strcpy(node->exp_type, "int");
                return "int";
            }
            if (strcmp(node->lexema, "!") == 0) strcpy(node->exp_type, "int");
            else strcpy(node->exp_type, t1);
            return node->exp_type;
        }
        case AST_IF:
        case AST_WHILE: {
            char *cond_type = verificar_semantica(node->child1);
            if (is_string_like_type(cond_type) || is_string_type(cond_type)) {
                fprintf(stderr,
                        "[SEMANTIC ERROR] Condition must be numeric, not '%s' at Line %d, Column %d.\n",
                        cond_type, node->child1->line, node->child1->column);
                semantic_error = 1;
            }
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
                    if (!check_assignment_types(expected_type, arg_type, NULL, arg->line, arg->column)) {
                        fprintf(stderr,
                                "               In argument %d of function '%s'.\n",
                                arg_count + 1, s->name);
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
        {
            if (node->child1) {
                char *ptype = verificar_semantica(node->child1);
                if (strcmp(ptype, "string") != 0 && !is_numeric_type(ptype) && !is_char_type(ptype)) {
                    fprintf(stderr,
                            "[SEMANTIC ERROR] print expects int, float, char or string expression at Line %d, Column %d.\n",
                            node->child1->line, node->child1->column);
                    semantic_error = 1;
                }
            }
            return "void";
        }
        case AST_READ:
        {
            if (node->child1) {
                char *vtype = verificar_semantica(node->child1);
                if (!is_numeric_type(vtype) && !is_char_type(vtype) && !is_string_type(vtype)) {
                    fprintf(stderr,
                            "[SEMANTIC ERROR] read expects int, float, char or string variable at Line %d, Column %d.\n",
                            node->child1->line, node->child1->column);
                    semantic_error = 1;
                }
            }
            return "void";
        }
        case AST_RETURN: {
            if (node->child1) {
                char *rtype = verificar_semantica(node->child1);
                if (strcmp(current_func_type, "void") == 0) {
                    fprintf(stderr, "[SEMANTIC ERROR] Cannot return a value from void function at Line %d, Column %d.\n", node->child1->line, node->child1->column);
                    semantic_error = 1;
                } else {
                    if (!check_assignment_types(current_func_type, rtype, NULL,
                                                node->child1->line, node->child1->column)) {
                        fprintf(stderr,
                                "               Cannot return '%s' from function returning '%s'.\n",
                                rtype, current_func_type);
                    }
                }
            } else {
                /* bare return */
                if (strcmp(current_func_type, "void") != 0) {
                    fprintf(stderr, "[SEMANTIC ERROR] Missing return value: function expects '%s' at Line %d, Column %d.\n", current_func_type, node->line, node->column);
                    semantic_error = 1;
                }
                /* if void, bare return is allowed */
            }
            return "void";
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

void materialize_string_operand(ASTNode *node, char *out_temp) {
    gerar_tac(node);
    if (strcmp(node->exp_type, "string") == 0) {
        strcpy(out_temp, node->temp);
    } else if (is_char_type(node->exp_type)) {
        char *cast_temp = new_temp();
        emit_tac("%s = chartostr %s", cast_temp, node->temp);
        strcpy(out_temp, cast_temp);
    } else {
        strcpy(out_temp, node->temp);
    }
}

void emit_assignment_rhs(ASTNode *lhs, ASTNode *rhs, char *rhs_temp) {
    gerar_tac(rhs);
    strcpy(rhs_temp, rhs->temp);

    if (strcmp(lhs->exp_type, "float") == 0 && strcmp(rhs->exp_type, "int") == 0) {
        char *cast_temp = new_temp();
        emit_tac("%s = (float) %s", cast_temp, rhs_temp);
        strcpy(rhs_temp, cast_temp);
    } else if (is_string_type(lhs->exp_type) && is_char_type(rhs->exp_type)) {
        char *cast_temp = new_temp();
        emit_tac("%s = chartostr %s", cast_temp, rhs_temp);
        strcpy(rhs_temp, cast_temp);
    }
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
                if (var->type == AST_ARRAY_DECL) {
                    ASTNode *dims = var->child2;
                    int total_size = 1;
                    ASTNode *d = dims;
                    while (d) {
                        total_size *= d->int_val;
                        d = d->next;
                    }
                    emit_tac("alloc_array %s, %d, %s", var->child1->lexema, total_size, decl_type);

                    if (var->child3) {
                        ASTNode *init = var->child3;
                        ASTNode *flat_init = NULL;
                        ASTNode *last_item = NULL;
                        collect_array_initializers(init, &flat_init, &last_item);
                        ASTNode *item = flat_init;
                        int index = 0;
                        while (item) {
                            if (index >= total_size) break;
                            gerar_tac(item);
                            emit_tac("%s[%d] = %s", var->child1->lexema, index, item->temp);
                            index++;
                            item = item->next;
                        }
                    } else {
                        for (int i = 0; i < total_size; i++) {
                            if (strcmp(decl_type, "float") == 0) {
                                emit_tac("%s[%d] = 0.0", var->child1->lexema, i);
                            } else if (strcmp(decl_type, "char") == 0) {
                                emit_tac("%s[%d] = 0", var->child1->lexema, i);
                            } else if (strcmp(decl_type, "string") == 0) {
                                emit_tac("%s[%d] = str \"\"", var->child1->lexema, i);
                            } else {
                                emit_tac("%s[%d] = 0", var->child1->lexema, i);
                            }
                        }
                    }
                }
                else if (var->type == AST_ASSIGN) {
                    gerar_tac(var->child2);
                    char rhs_temp[64];
                    strcpy(rhs_temp, var->child2->temp);

                    if (strcmp(decl_type, "float") == 0 && strcmp(var->child2->exp_type, "int") == 0) {
                        char *cast_temp = new_temp();
                        emit_tac("%s = (float) %s", cast_temp, rhs_temp);
                        strcpy(rhs_temp, cast_temp);
                    } else if (strcmp(decl_type, "string") == 0 && is_char_type(var->child2->exp_type)) {
                        char *cast_temp = new_temp();
                        emit_tac("%s = chartostr %s", cast_temp, rhs_temp);
                        strcpy(rhs_temp, cast_temp);
                    }

                    emit_tac("%s = %s", var->child1->lexema, rhs_temp);
                } else {
                    const char *name = var->lexema;
                    if (strcmp(decl_type, "float") == 0) {
                        emit_tac("%s = 0.0", name);
                    } else if (strcmp(decl_type, "char") == 0) {
                        emit_tac("%s = 0", name);
                    } else if (strcmp(decl_type, "string") == 0) {
                        emit_tac("%s = str \"\"", name);
                    } else {
                        emit_tac("%s = 0", name);
                    }
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
            ASTNode *stmt = node->child1;
            while (stmt) { gerar_tac(stmt); stmt = stmt->next; }
            if (node->child2) gerar_tac(node->child2);
            break;
        }
        
        case AST_INT:
            sprintf(node->temp, "%d", node->int_val);
            break;

        case AST_CHAR:
            sprintf(node->temp, "%d", node->int_val);
            break;

        case AST_STRING: {
            strcpy(node->temp, new_temp());
            emit_tac("%s = str \"%s\"", node->temp, node->lexema ? node->lexema : "");
            break;
        }
            
        case AST_FLOAT:
            sprintf(node->temp, "%.2f", node->float_val);
            break;
        case AST_ID:
            strcpy(node->temp, node->lexema);
            break;
        case AST_ARRAY_ACCESS: {
            Symbol *s = lookup_symbol(node->child1->lexema);
            const char *arr_type = node->exp_type;
            int multipliers[10] = {0};
            int dim_count = node->num_array_dims;

            if (s) {
                arr_type = s->type;
                if (dim_count == 0) {
                    dim_count = s->num_array_dims;
                    node->num_array_dims = dim_count;
                    for (int i = 0; i < dim_count; i++) {
                        node->array_dims[i] = s->array_dims[i];
                    }
                }
            }

            if (dim_count == 0) {
                ASTNode *tmp = node->child2;
                while (tmp) {
                    dim_count++;
                    tmp = tmp->next;
                }
                node->num_array_dims = dim_count;
            }

            for (int i = 0; i < dim_count; i++) {
                multipliers[i] = 1;
                for (int j = i + 1; j < dim_count; j++) {
                    multipliers[i] *= node->array_dims[j];
                }
            }



            ASTNode *index_expr = node->child2;
            char accumulated[64] = "";
            int idx = 0;

            while (index_expr && idx < dim_count) {
                gerar_tac(index_expr);
                char temp[64];
                if (multipliers[idx] != 1) {
                    sprintf(temp, "%s_mul_%d", index_expr->temp, idx);
                    emit_tac("%s = %s * %d", temp, index_expr->temp, multipliers[idx]);
                } else {
                    strcpy(temp, index_expr->temp);
                }
                if (idx == 0) {
                    strcpy(accumulated, temp);
                } else {
                    char new_acc[64];
                    sprintf(new_acc, "acc_%d", idx);
                    emit_tac("%s = %s + %s", new_acc, accumulated, temp);
                    strcpy(accumulated, new_acc);
                }
                index_expr = index_expr->next;
                idx++;
            }

            char *result = new_temp();
            emit_tac("%s = %s[%s]", result, node->child1->lexema, accumulated);
            strcpy(node->temp, result);
            break;
        }
       case AST_ASSIGN: {
            char rhs_temp[64];
            emit_assignment_rhs(node->child1, node->child2, rhs_temp);
            
            if (node->child1->type == AST_ARRAY_ACCESS) {
                ASTNode *arr = node->child1;
                int dim_count = arr->num_array_dims;

                /* Fallback: contar dimensões pelos nós de índice */
                if (dim_count == 0) {
                    ASTNode *tmp = arr->child2;
                    while (tmp) { dim_count++; tmp = tmp->next; }
                }

                /* Calcular multiplicadores para flattening */
                int multipliers[10] = {0};
                for (int i = 0; i < dim_count; i++) {
                    multipliers[i] = 1;
                    for (int j = i + 1; j < dim_count; j++) {
                        multipliers[i] *= arr->array_dims[j];
                    }
                }

                /* Gerar TAC com flattening completo */
                ASTNode *index_expr = arr->child2;
                char accumulated[64] = "";
                int idx = 0;

                while (index_expr && idx < dim_count) {
                    gerar_tac(index_expr);
                    char temp[64];
                    if (multipliers[idx] != 1) {
                        sprintf(temp, "%s_mul_%d", index_expr->temp, idx);
                        emit_tac("%s = %s * %d", temp, index_expr->temp, multipliers[idx]);
                    } else {
                        strcpy(temp, index_expr->temp);
                    }
                    if (idx == 0) {
                        strcpy(accumulated, temp);
                    } else {
                        char new_acc[64];
                        sprintf(new_acc, "acc_w_%d", idx);
                        emit_tac("%s = %s + %s", new_acc, accumulated, temp);
                        strcpy(accumulated, new_acc);
                    }
                    index_expr = index_expr->next;
                    idx++;
                }

                emit_tac("%s[%s] = %s", arr->child1->lexema, accumulated, rhs_temp);
                strcpy(node->temp, rhs_temp);
            } else {
                emit_tac("%s = %s", node->child1->lexema, rhs_temp);
                strcpy(node->temp, node->child1->lexema);
            }
            break;
        }
            
       case AST_BINOP: {
            // 1. PRIMEIRO: Sempre gera o TAC dos filhos para que as variáveis temporárias sejam criadas!
            gerar_tac(node->child1);
            gerar_tac(node->child2);

            // 2. GERAÇÃO ESPECIAL: Concatenação de Strings
            // Como a verificação semântica já cravou o node->exp_type como "string", usamos isso a nosso favor.
            if (strcmp(node->exp_type, "string") == 0 && strcmp(node->lexema, "+") == 0) {
                strcpy(node->temp, new_temp());
                char t1_temp[32], t2_temp[32];

                /* Só converte `char` para string; strings já estão prontos.
                   Tipos inválidos para concatenação devem ter sido rejeitados na
                   checagem semântica, então aqui fazemos apenas um fallback
                   conservador caso algo passe adiante. */
                if (strcmp(node->child1->exp_type, "string") == 0) {
                    strcpy(t1_temp, node->child1->temp);
                } else if (strcmp(node->child1->exp_type, "char") == 0) {
                    strcpy(t1_temp, new_temp());
                    emit_tac("%s = chartostr %s", t1_temp, node->child1->temp);
                } else {
                    /* fallback: use as-is (semantic should have rejected other types) */
                    strcpy(t1_temp, node->child1->temp);
                }

                if (strcmp(node->child2->exp_type, "string") == 0) {
                    strcpy(t2_temp, node->child2->temp);
                } else if (strcmp(node->child2->exp_type, "char") == 0) {
                    strcpy(t2_temp, new_temp());
                    emit_tac("%s = chartostr %s", t2_temp, node->child2->temp);
                } else {
                    strcpy(t2_temp, node->child2->temp);
                }

                emit_tac("%s = concat %s, %s", node->temp, t1_temp, t2_temp);
                break; // Sai do case para não gerar código matemático!
            }

            // ====================================================================
            // 3. GERAÇÃO NORMAL: Matemática e Lógica (Ints e Floats)
            // ====================================================================
            char op1_temp[32];
            char op2_temp[32];
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
            if (strcmp(node->child1->exp_type, "float") == 0)
                emit_tac("print_float %s", node->child1->temp);
            else if (strcmp(node->child1->exp_type, "string") == 0)
                emit_tac("print_string %s", node->child1->temp);
            else if (is_char_type(node->child1->exp_type))
                emit_tac("print_char %s", node->child1->temp);
            else
                emit_tac("print_int %s", node->child1->temp);
            break;
            
        case AST_READ:
            if (strcmp(node->child1->exp_type, "float") == 0)
                emit_tac("read_float %s", node->child1->lexema);
            else if (is_string_type(node->child1->exp_type))
                emit_tac("read_string %s", node->child1->lexema);
            else if (is_char_type(node->child1->exp_type))
                emit_tac("read_char %s", node->child1->lexema);
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
