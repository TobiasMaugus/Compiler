/* ========================================================================= */
/* BACKEND DO COMPILADOR — Leitor de TAC → Gerador de Assembly x86_64        */
/* Programa independente: ./codegen output.tac → output.s                    */
/* ========================================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ========================================================================= */
/* MAPA DE VARIÁVEIS DA FUNÇÃO ATUAL (PILHA x86_64)                          */
/* ========================================================================= */

typedef struct
{
    char name[64];
    int offset;
    char type[12]; /* int, float, char, string */
    int size;
} VarSlot;

#define MAX_VARS 500
#define STRING_BUF_SIZE 256
VarSlot var_map[MAX_VARS];
int var_count = 0;
int frame_size = 0;
int string_lit_counter = 0;

typedef struct
{
    char label[32];
    char content[512];
} StringLiteral;

#define MAX_STRING_LITS 256
StringLiteral string_literals[MAX_STRING_LITS];
int string_lit_count = 0;

/* Tabela de tipos de retorno das funções já declaradas */
typedef struct
{
    char name[64];
    char ret_type[12];
} FuncInfo;

#define MAX_FUNCS 100
FuncInfo func_table[MAX_FUNCS];
int func_count = 0;

/* Buffer de parâmetros para chamadas de função */
#define MAX_PARAMS 10
char param_buffer[MAX_PARAMS][64];
int param_count = 0;

FILE *asm_out = NULL;

/* ========================================================================= */
/* HELPERS DE CONTEXTO                                                        */
/* ========================================================================= */

void reset_function_context()
{
    var_count = 0;
    frame_size = 0;
    param_count = 0;
}

int slot_size_for_type(const char *type)
{
    if (type && strcmp(type, "string") == 0)
        return STRING_BUF_SIZE;
    return 8;
}

int get_offset_typed(const char *name, const char *type)
{
    for (int i = 0; i < var_count; i++)
    {
        if (strcmp(var_map[i].name, name) == 0)
            return var_map[i].offset;
    }
    int size = slot_size_for_type(type);
    frame_size += size;
    strcpy(var_map[var_count].name, name);
    var_map[var_count].offset = -frame_size;
    var_map[var_count].size = size;
    strcpy(var_map[var_count].type, type ? type : "int");
    var_count++;
    return -frame_size;
}

int array_slot_size_for_type(const char *type)
{
    if (type && strcmp(type, "char") == 0)
        return 1;
    if (type && strcmp(type, "string") == 0)
        return STRING_BUF_SIZE;
    return 8;
}

int get_offset_typed_array(const char *name, const char *type, int total_bytes)
{
    for (int i = 0; i < var_count; i++)
    {
        if (strcmp(var_map[i].name, name) == 0)
            return var_map[i].offset;
    }
    int size = total_bytes;
    frame_size += size;
    strcpy(var_map[var_count].name, name);
    var_map[var_count].offset = -frame_size;
    var_map[var_count].size = size;
    strcpy(var_map[var_count].type, type ? type : "int");
    var_count++;
    return -frame_size;
}

int get_offset(const char *name)
{
    return get_offset_typed(name, "int");
}

void set_var_type(const char *name, const char *type)
{
    get_offset_typed(name, type);
    for (int i = 0; i < var_count; i++)
    {
        if (strcmp(var_map[i].name, name) == 0)
        {
            strcpy(var_map[i].type, type);
            return;
        }
    }
}

const char *get_var_type(const char *name)
{
    for (int i = 0; i < var_count; i++)
    {
        if (strcmp(var_map[i].name, name) == 0)
            return var_map[i].type;
    }
    return "int"; /* fallback */
}

void register_func(const char *name, const char *ret_type)
{
    for (int i = 0; i < func_count; i++)
    {
        if (strcmp(func_table[i].name, name) == 0)
            return; /* já registrada */
    }
    strcpy(func_table[func_count].name, name);
    strcpy(func_table[func_count].ret_type, ret_type);
    func_count++;
}

const char *get_func_ret_type(const char *name)
{
    for (int i = 0; i < func_count; i++)
    {
        if (strcmp(func_table[i].name, name) == 0)
            return func_table[i].ret_type;
    }
    return "int"; /* fallback */
}

/* ========================================================================= */
/* HELPERS DE PARSING                                                         */
/* ========================================================================= */

int is_int_literal(const char *s)
{
    if (!s || !*s)
        return 0;
    int i = 0;
    if (s[0] == '-' || s[0] == '+')
        i = 1;
    if (!s[i])
        return 0;
    for (; s[i]; i++)
    {
        if (!isdigit((unsigned char)s[i]))
            return 0;
    }
    return 1;
}

int is_float_literal(const char *s)
{
    if (!s || !*s)
        return 0;
    int i = 0, dot = 0;
    if (s[0] == '-' || s[0] == '+')
        i = 1;
    if (!s[i])
        return 0;
    for (; s[i]; i++)
    {
        if (s[i] == '.')
        {
            dot++;
            if (dot > 1)
                return 0;
        }
        else if (!isdigit((unsigned char)s[i]))
            return 0;
    }
    return (dot >= 1);
}

char *trim(char *s);

int parse_array_access(const char *text, char *array_name, char *index_expr)
{
    if (!text || !array_name || !index_expr)
        return 0;

    char buffer[256];
    strncpy(buffer, text, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char *open = strchr(buffer, '[');
    char *close = strrchr(buffer, ']');
    if (!open || !close || close <= open)
        return 0;

    *open = '\0';
    *close = '\0';

    char *name = trim(buffer);
    char *index = trim(open + 1);
    if (!name[0] || !index[0])
        return 0;

    strncpy(array_name, name, 63);
    array_name[63] = '\0';
    strncpy(index_expr, index, 63);
    index_expr[63] = '\0';
    return 1;
}

char *trim(char *s)
{
    while (*s && isspace((unsigned char)*s))
        s++;
    char *end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end))
    {
        *end = '\0';
        end--;
    }
    return s;
}

void emit_frame_alloc()
{
    fprintf(asm_out, "    subq $8192, %%rsp\n");
}

void emit_copy_string_buffer(int dest_off, int src_off)
{
    fprintf(asm_out, "    leaq %d(%%rbp), %%rdi\n", dest_off);
    fprintf(asm_out, "    leaq %d(%%rbp), %%rsi\n", src_off);
    fprintf(asm_out, "    call strcpy\n");
}

void emit_store_string_literal(int dest_off, const char *label)
{
    fprintf(asm_out, "    leaq %d(%%rbp), %%rdi\n", dest_off);
    fprintf(asm_out, "    leaq %s(%%rip), %%rsi\n", label);
    fprintf(asm_out, "    call strcpy\n");
}

void emit_chartostr(int dest_off, int src_off)
{
    fprintf(asm_out, "    movq %d(%%rbp), %%rax\n", src_off);
    fprintf(asm_out, "    leaq %d(%%rbp), %%rdi\n", dest_off);
    fprintf(asm_out, "    movb %%al, 0(%%rdi)\n");
    fprintf(asm_out, "    movb $0, 1(%%rdi)\n");
}

const char *register_string_literal(const char *content)
{
    for (int i = 0; i < string_lit_count; i++)
    {
        if (strcmp(string_literals[i].content, content) == 0)
            return string_literals[i].label;
    }

    if (string_lit_count >= MAX_STRING_LITS)
        return ".str_lit_0";

    sprintf(string_literals[string_lit_count].label, ".str_lit_%d", string_lit_counter++);
    strncpy(string_literals[string_lit_count].content, content,
            sizeof(string_literals[string_lit_count].content) - 1);
    string_literals[string_lit_count].content[sizeof(string_literals[string_lit_count].content) - 1] = '\0';
    return string_literals[string_lit_count++].label;
}

void emit_string_data_section()
{
    for (int i = 0; i < string_lit_count; i++)
    {
        fprintf(asm_out, "%s: .string \"", string_literals[i].label);
        for (const char *p = string_literals[i].content; *p; p++)
        {
            if (*p == '\\' || *p == '"')
                fputc('\\', asm_out);
            fputc(*p, asm_out);
        }
        fprintf(asm_out, "\"\n");
    }
}

void escape_asm_string(const char *in, char *out, size_t out_size)
{
    size_t j = 0;
    for (size_t i = 0; in[i] && j + 2 < out_size; i++)
    {
        if (in[i] == '\\' || in[i] == '"')
            out[j++] = '\\';
        out[j++] = in[i];
    }
    out[j] = '\0';
}

/* ========================================================================= */
/* HELPERS DE EMISSÃO DE ASSEMBLY                                             */
/* ========================================================================= */

/* Carrega um operando (literal ou variável) para %rax (int) */
void load_int_operand(const char *operand)
{
    if (is_int_literal(operand))
    {
        fprintf(asm_out, "    movq $%s, %%rax\n", operand);
    }
    else
    {
        int off = get_offset(operand);
        fprintf(asm_out, "    movq %d(%%rbp), %%rax\n", off);
    }
}

/* Materializa um operando int na pilha e retorna o offset */
int materialize_int(const char *operand)
{
    if (is_int_literal(operand))
    {
        /* Cria um temporário interno para o literal */
        char tmp_name[80];
        sprintf(tmp_name, "__lit_%s", operand);
        /* Substitui sinal negativo por 'n' para nome válido */
        for (int i = 0; tmp_name[i]; i++)
        {
            if (tmp_name[i] == '-')
                tmp_name[i] = 'n';
        }
        int off = get_offset(tmp_name);
        set_var_type(tmp_name, "int");
        fprintf(asm_out, "    movq $%s, %%rax\n", operand);
        fprintf(asm_out, "    movq %%rax, %d(%%rbp)\n", off);
        return off;
    }
    return get_offset(operand);
}

/* Materializa um operando float na pilha e retorna o offset */
int materialize_float(const char *operand)
{
    if (is_float_literal(operand))
    {
        char tmp_name[80];
        sprintf(tmp_name, "__flit_%.0f", atof(operand));
        int off = get_offset(tmp_name);
        set_var_type(tmp_name, "float");
        float val = (float)atof(operand);
        unsigned int bits;
        memcpy(&bits, &val, sizeof(float));
        fprintf(asm_out, "    movl $%u, %%eax\n", bits);
        fprintf(asm_out, "    movl %%eax, %d(%%rbp)\n", off);
        return off;
    }
    return get_offset(operand);
}

/* ========================================================================= */
/* PROCESSAMENTO DE CADA LINHA DO TAC                                         */
/* ========================================================================= */

void process_line(char *raw_line)
{
    char line[512];
    strncpy(line, raw_line, sizeof(line) - 1);
    line[sizeof(line) - 1] = '\0';

    char *trimmed = trim(line);
    if (!*trimmed)
        return; /* linha vazia */

    /* ================================================================== */
    /* func <nome> <tipo> params: [<tipo> <param>, ...]                   */
    /* ================================================================== */
    if (strncmp(trimmed, "func ", 5) == 0)
    {
        reset_function_context();

        char name[64], ret_type[10];
        char *p = trimmed + 5;
        sscanf(p, "%63s %9s", name, ret_type);
        register_func(name, ret_type);

        /* Prólogo da função */
        fprintf(asm_out, ".global %s\n", name);
        fprintf(asm_out, "%s:\n", name);
        fprintf(asm_out, "    pushq %%rbp\n");
        fprintf(asm_out, "    movq %%rsp, %%rbp\n");
        emit_frame_alloc();

        /* Parse dos parâmetros */
        char *params_start = strstr(trimmed, "params:");
        if (params_start)
        {
            params_start += 7; /* pula "params:" */
            while (isspace((unsigned char)*params_start))
                params_start++;

            if (*params_start)
            {
                /* Copia para buffer local (strtok modifica) */
                char params_buf[256];
                strncpy(params_buf, params_start, sizeof(params_buf) - 1);
                params_buf[sizeof(params_buf) - 1] = '\0';

                int int_reg_idx = 0;
                int float_reg_idx = 0;
                const char *int_regs[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

                char *token = strtok(params_buf, ",");
                while (token)
                {
                    while (isspace((unsigned char)*token))
                        token++;
                    char ptype[10], pname[64];
                    if (sscanf(token, "%9s %63s", ptype, pname) == 2)
                    {
                        int off = get_offset(pname);
                        set_var_type(pname, ptype);

                        if (strcmp(ptype, "float") == 0)
                        {
                            fprintf(asm_out, "    movss %%xmm%d, %d(%%rbp)\n", float_reg_idx++, off);
                        }
                        else if (strcmp(ptype, "string") == 0)
                        {
                            fprintf(asm_out, "    movq %s, %%rax\n", int_regs[int_reg_idx++]);
                            fprintf(asm_out, "    leaq %d(%%rbp), %%rdi\n", off);
                            fprintf(asm_out, "    movq %%rax, %%rsi\n");
                            fprintf(asm_out, "    call strcpy\n");
                        }
                        else if (strcmp(ptype, "char") == 0)
                        {
                            fprintf(asm_out, "    movq %s, %%rax\n", int_regs[int_reg_idx++]);
                            fprintf(asm_out, "    movq %%rax, %d(%%rbp)\n", off);
                        }
                        else
                        {
                            fprintf(asm_out, "    movq %s, %d(%%rbp)\n", int_regs[int_reg_idx++], off);
                        }
                    }
                    token = strtok(NULL, ",");
                }
            }
        }
        return;
    }

    /* ================================================================== */
    /* endfunc                                                             */
    /* ================================================================== */
    if (strcmp(trimmed, "endfunc") == 0)
    {
        fprintf(asm_out, "    leave\n");
        fprintf(asm_out, "    ret\n");
        return;
    }

    /* ================================================================== */
    /* Labels  (ex: L1:)                                                   */
    /* ================================================================== */
    {
        int len = strlen(trimmed);
        if (len > 1 && trimmed[len - 1] == ':')
        {
            /* Verifica que não contém espaço (senão não é label) */
            int has_space = 0;
            for (int i = 0; i < len - 1; i++)
            {
                if (isspace((unsigned char)trimmed[i]))
                {
                    has_space = 1;
                    break;
                }
            }
            if (!has_space)
            {
                fprintf(asm_out, "%s\n", trimmed);
                return;
            }
        }
    }

    /* ================================================================== */
    /* goto <label>                                                        */
    /* ================================================================== */
    if (strncmp(trimmed, "goto ", 5) == 0)
    {
        char label[64];
        sscanf(trimmed + 5, "%63s", label);
        fprintf(asm_out, "    jmp %s\n", label);
        return;
    }

    /* ================================================================== */
    /* if <a> <op> <b> goto <label>   (salto condicional)                  */
    /* ================================================================== */
    if (strncmp(trimmed, "if ", 3) == 0)
    {
        char a[64], op[16], b[64], label[64];

        if (sscanf(trimmed, "if %63s %15s %63s goto %63s", a, op, b, label) == 4)
        {
            /* Verifica se é comparação float (operador termina com 'f') */
            int is_float_cmp = 0;
            char clean_op[16];
            strcpy(clean_op, op);
            int oplen = strlen(clean_op);
            if (oplen > 1 && clean_op[oplen - 1] == 'f')
            {
                clean_op[oplen - 1] = '\0';
                is_float_cmp = 1;
            }

            if (is_float_cmp)
            {
                /* Comparação float: ucomiss */
                int off_a = materialize_float(a);
                int off_b = materialize_float(b);

                fprintf(asm_out, "    movss %d(%%rbp), %%xmm0\n", off_a);
                fprintf(asm_out, "    ucomiss %d(%%rbp), %%xmm0\n", off_b);

                if (strcmp(clean_op, "<") == 0)
                    fprintf(asm_out, "    jb %s\n", label);
                else if (strcmp(clean_op, ">") == 0)
                    fprintf(asm_out, "    ja %s\n", label);
                else if (strcmp(clean_op, "<=") == 0)
                    fprintf(asm_out, "    jbe %s\n", label);
                else if (strcmp(clean_op, ">=") == 0)
                    fprintf(asm_out, "    jae %s\n", label);
                else if (strcmp(clean_op, "==") == 0)
                    fprintf(asm_out, "    je %s\n", label);
                else if (strcmp(clean_op, "!=") == 0)
                    fprintf(asm_out, "    jne %s\n", label);
            }
            else
            {
                /* Comparação int: cmpq */
                int off_a, off_b;

                /* Carrega operando A em %rax */
                if (is_int_literal(a))
                {
                    off_a = materialize_int(a);
                }
                else
                {
                    off_a = get_offset(a);
                }

                /* Carrega operando B na pilha (para usar com cmpq) */
                if (is_int_literal(b))
                {
                    off_b = materialize_int(b);
                }
                else
                {
                    off_b = get_offset(b);
                }

                fprintf(asm_out, "    movq %d(%%rbp), %%rax\n", off_a);
                fprintf(asm_out, "    cmpq %d(%%rbp), %%rax\n", off_b);

                if (strcmp(clean_op, "<") == 0)
                    fprintf(asm_out, "    jl %s\n", label);
                else if (strcmp(clean_op, ">") == 0)
                    fprintf(asm_out, "    jg %s\n", label);
                else if (strcmp(clean_op, "<=") == 0)
                    fprintf(asm_out, "    jle %s\n", label);
                else if (strcmp(clean_op, ">=") == 0)
                    fprintf(asm_out, "    jge %s\n", label);
                else if (strcmp(clean_op, "==") == 0)
                    fprintf(asm_out, "    je %s\n", label);
                else if (strcmp(clean_op, "!=") == 0)
                    fprintf(asm_out, "    jne %s\n", label);
            }
            return;
        }
    }

    /* ================================================================== */
    /* return [<valor>]                                                     */
    /* ================================================================== */
    if (strncmp(trimmed, "return", 6) == 0 &&
        (trimmed[6] == '\0' || trimmed[6] == ' '))
    {
        char val[64];
        if (sscanf(trimmed, "return %63s", val) == 1)
        {
            if (is_int_literal(val))
            {
                fprintf(asm_out, "    movq $%s, %%rax\n", val);
            }
            else if (is_float_literal(val))
            {
                int off = materialize_float(val);
                fprintf(asm_out, "    movss %d(%%rbp), %%xmm0\n", off);
            }
            else
            {
                int off = get_offset(val);
                const char *vtype = get_var_type(val);
                if (strcmp(vtype, "float") == 0)
                {
                    fprintf(asm_out, "    movss %d(%%rbp), %%xmm0\n", off);
                }
                else if (strcmp(vtype, "string") == 0)
                {
                    fprintf(asm_out, "    leaq %d(%%rbp), %%rax\n", off);
                }
                else if (strcmp(vtype, "char") == 0)
                {
                    fprintf(asm_out, "    movq %d(%%rbp), %%rax\n", off);
                }
                else
                {
                    fprintf(asm_out, "    movq %d(%%rbp), %%rax\n", off);
                }
            }
        }
        fprintf(asm_out, "    leave\n");
        fprintf(asm_out, "    ret\n");
        return;
    }

    /* ================================================================== */
    /* print_int <x>                                                       */
    /* ================================================================== */
    if (strncmp(trimmed, "print_int ", 10) == 0)
    {
        char var[64];
        sscanf(trimmed + 10, "%63s", var);
        int off;
        if (is_int_literal(var))
        {
            off = materialize_int(var);
        }
        else
        {
            off = get_offset(var);
        }
        fprintf(asm_out, "    movq %d(%%rbp), %%rsi\n", off);
        fprintf(asm_out, "    leaq .str_print_int(%%rip), %%rdi\n");
        fprintf(asm_out, "    movb $0, %%al\n");
        fprintf(asm_out, "    call printf\n");
        return;
    }

    /* ================================================================== */
    /* print_float <x>                                                     */
    /* ================================================================== */
    if (strncmp(trimmed, "print_float ", 12) == 0)
    {
        char var[64];
        sscanf(trimmed + 12, "%63s", var);
        int off;
        if (is_float_literal(var))
        {
            off = materialize_float(var);
        }
        else
        {
            off = get_offset(var);
        }
        fprintf(asm_out, "    cvtss2sd %d(%%rbp), %%xmm0\n", off);
        fprintf(asm_out, "    leaq .str_print_float(%%rip), %%rdi\n");
        fprintf(asm_out, "    movb $1, %%al\n");
        fprintf(asm_out, "    call printf\n");
        return;
    }

    /* ================================================================== */
    /* print_string <x>                                                    */
    /* ================================================================== */
    if (strncmp(trimmed, "print_string ", 13) == 0)
    {
        char var[64];
        sscanf(trimmed + 13, "%63s", var);
        int off = get_offset_typed(var, "string");
        set_var_type(var, "string");
        fprintf(asm_out, "    leaq %d(%%rbp), %%rsi\n", off);
        fprintf(asm_out, "    leaq .str_print_string(%%rip), %%rdi\n");
        fprintf(asm_out, "    movb $0, %%al\n");
        fprintf(asm_out, "    call printf\n");
        return;
    }

    /* ================================================================== */
    /* print_char <x>                                                      */
    /* ================================================================== */
    if (strncmp(trimmed, "print_char ", 11) == 0)
    {
        char var[64];
        sscanf(trimmed + 11, "%63s", var);
        int off;
        if (is_int_literal(var))
        {
            off = materialize_int(var);
        }
        else
        {
            off = get_offset_typed(var, "char");
            set_var_type(var, "char");
        }
        fprintf(asm_out, "    movq %d(%%rbp), %%rsi\n", off);
        fprintf(asm_out, "    leaq .str_print_char(%%rip), %%rdi\n");
        fprintf(asm_out, "    movb $0, %%al\n");
        fprintf(asm_out, "    call printf\n");
        return;
    }

    /* ================================================================== */
    /* read_int <x>                                                        */
    /* ================================================================== */
    if (strncmp(trimmed, "read_int ", 9) == 0)
    {
        char var[64];
        sscanf(trimmed + 9, "%63s", var);
        int off = get_offset(var);
        set_var_type(var, "int");
        fprintf(asm_out, "    leaq .str_read_int(%%rip), %%rdi\n");
        fprintf(asm_out, "    leaq %d(%%rbp), %%rsi\n", off);
        fprintf(asm_out, "    movb $0, %%al\n");
        fprintf(asm_out, "    call scanf\n");
        return;
    }

    /* ================================================================== */
    /* read_float <x>                                                      */
    /* ================================================================== */
    if (strncmp(trimmed, "read_float ", 11) == 0)
    {
        char var[64];
        sscanf(trimmed + 11, "%63s", var);
        int off = get_offset_typed(var, "float");
        set_var_type(var, "float");
        fprintf(asm_out, "    leaq .str_read_float(%%rip), %%rdi\n");
        fprintf(asm_out, "    leaq %d(%%rbp), %%rsi\n", off);
        fprintf(asm_out, "    movb $0, %%al\n");
        fprintf(asm_out, "    call scanf\n");
        return;
    }

    /* ================================================================== */
    /* read_string <x>                                                     */
    /* ================================================================== */
    if (strncmp(trimmed, "read_string ", 12) == 0)
    {
        char var[64];
        sscanf(trimmed + 12, "%63s", var);
        int off = get_offset_typed(var, "string");
        set_var_type(var, "string");
        fprintf(asm_out, "    leaq .str_read_string(%%rip), %%rdi\n");
        fprintf(asm_out, "    leaq %d(%%rbp), %%rsi\n", off);
        fprintf(asm_out, "    movb $0, %%al\n");
        fprintf(asm_out, "    call scanf\n");
        return;
    }

    /* ================================================================== */
    /* read_char <x>                                                       */
    /* ================================================================== */
    if (strncmp(trimmed, "read_char ", 10) == 0)
    {
        char var[64];
        sscanf(trimmed + 10, "%63s", var);
        int off = get_offset_typed(var, "char");
        set_var_type(var, "char");
        fprintf(asm_out, "    leaq .str_read_char(%%rip), %%rdi\n");
        fprintf(asm_out, "    leaq %d(%%rbp), %%rsi\n", off);
        fprintf(asm_out, "    movb $0, %%al\n");
        fprintf(asm_out, "    call scanf\n");
        return;
    }

    /* ================================================================== */
    /* param <x>   (armazena no buffer para a próxima instrução call)      */
    /* ================================================================== */
    if (strncmp(trimmed, "param ", 6) == 0)
    {
        char var[64];
        sscanf(trimmed + 6, "%63s", var);
        if (param_count < MAX_PARAMS)
        {
            strcpy(param_buffer[param_count++], var);
        }
        return;
    }

    /* ================================================================== */
    /* <dest> = call <func>, <nargs>                                       */
    /* ================================================================== */
    {
        char dest[64], func_name[64];
        int nargs;
        if (sscanf(trimmed, "%63s = call %63[^,], %d", dest, func_name, &nargs) == 3)
        {
            /* Carrega os parâmetros do buffer nos registradores da ABI */
            int int_reg_idx = 0;
            int float_reg_idx = 0;
            const char *int_regs[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

            for (int i = 0; i < param_count; i++)
            {
                const char *pval = param_buffer[i];

                if (is_int_literal(pval))
                {
                    fprintf(asm_out, "    movq $%s, %s\n", pval, int_regs[int_reg_idx++]);
                }
                else if (is_float_literal(pval))
                {
                    int off = materialize_float(pval);
                    fprintf(asm_out, "    movss %d(%%rbp), %%xmm%d\n", off, float_reg_idx++);
                }
                else
                {
                    int off = get_offset(pval);
                    const char *ptype = get_var_type(pval);
                    if (strcmp(ptype, "float") == 0)
                    {
                        fprintf(asm_out, "    movss %d(%%rbp), %%xmm%d\n", off, float_reg_idx++);
                    }
                    else if (strcmp(ptype, "string") == 0)
                    {
                        fprintf(asm_out, "    leaq %d(%%rbp), %%rax\n", off);
                        fprintf(asm_out, "    movq %%rax, %s\n", int_regs[int_reg_idx++]);
                    }
                    else
                    {
                        fprintf(asm_out, "    movq %d(%%rbp), %s\n", off, int_regs[int_reg_idx++]);
                    }
                }
            }

            fprintf(asm_out, "    movb $%d, %%al\n", float_reg_idx);
            fprintf(asm_out, "    call %s\n", func_name);

            /* Guarda o resultado */
            const char *ret_type = get_func_ret_type(func_name);
            int off_dest = get_offset(dest);
            set_var_type(dest, ret_type);

            if (strcmp(ret_type, "float") == 0)
            {
                fprintf(asm_out, "    movss %%xmm0, %d(%%rbp)\n", off_dest);
            }
            else if (strcmp(ret_type, "string") == 0)
            {
                fprintf(asm_out, "    leaq %d(%%rbp), %%rdi\n", off_dest);
                fprintf(asm_out, "    movq %%rax, %%rsi\n");
                fprintf(asm_out, "    call strcpy\n");
            }
            else
            {
                fprintf(asm_out, "    movq %%rax, %d(%%rbp)\n", off_dest);
            }

            param_count = 0; /* limpa o buffer */
            return;
        }
    }

    /* ================================================================== */
    /* alloc_array <name>, <size>, <type>                                  */
    /* ================================================================== */
    if (strncmp(trimmed, "alloc_array ", 12) == 0)
    {
        char name[64];
        int size;
        char type[16];
        if (sscanf(trimmed + 12, "%63[^,], %d, %15s", name, &size, type) == 3)
        {
            int slot_size = 8;
            if (strcmp(type, "char") == 0)
                slot_size = 1;
            else if (strcmp(type, "string") == 0)
                slot_size = STRING_BUF_SIZE;
            else if (strcmp(type, "float") == 0)
                slot_size = 8;
            else if (strcmp(type, "int") == 0)
                slot_size = 8;

            int total_bytes = size * slot_size;
            get_offset_typed(name, type);
            var_map[var_count - 1].size = total_bytes;
            frame_size += total_bytes - slot_size;
            var_map[var_count - 1].offset = -frame_size;
            return;
        }
    }

    /* ================================================================== */
    /* <dest> = str "..."                                                  */
    /* ================================================================== */
    {
        char dest[64];
        char content[256];
        if (sscanf(trimmed, "%63s = str \"%255[^\"]\"", dest, content) == 2)
        {
            const char *label = register_string_literal(content);
            int off_dest = get_offset_typed(dest, "string");
            set_var_type(dest, "string");
            emit_store_string_literal(off_dest, label);
            return;
        }
    }

    /* ================================================================== */
    /* <dest> = chartostr <src>                                            */
    /* ================================================================== */
    {
        char dest[64], src[64];
        if (sscanf(trimmed, "%63s = chartostr %63s", dest, src) == 2)
        {
            int off_dest = get_offset_typed(dest, "string");
            set_var_type(dest, "string");
            int off_src = is_int_literal(src) ? materialize_int(src) : get_offset_typed(src, "char");
            if (!is_int_literal(src))
                set_var_type(src, "char");
            emit_chartostr(off_dest, off_src);
            return;
        }
    }

    /* ================================================================== */
    /* <dest> = concat <left>, <right>                                     */
    /* ================================================================== */
    {
        char dest[64], left[64], right[64];
        if (sscanf(trimmed, "%63s = concat %63[^,], %63s", dest, left, right) == 3)
        {
            char *l = trim(left);  // NOVO: Remove os espaços acidentais
            char *r = trim(right); // NOVO: Remove os espaços acidentais

            int off_dest = get_offset_typed(dest, "string");
            set_var_type(dest, "string");
            int off_left = get_offset_typed(l, "string");
            set_var_type(l, "string");
            int off_right = get_offset_typed(r, "string");
            set_var_type(r, "string");

            emit_copy_string_buffer(off_dest, off_left);
            fprintf(asm_out, "    leaq %d(%%rbp), %%rdi\n", off_dest);
            fprintf(asm_out, "    leaq %d(%%rbp), %%rsi\n", off_right);
            fprintf(asm_out, "    call strcat\n");
            return;
        }
    }

    /* ================================================================== */
    /* <dest> = (float) <src>   (cast int → float)                         */
    /* ================================================================== */
    {
        char dest[64], src[64];
        if (sscanf(trimmed, "%63s = (float) %63s", dest, src) == 2)
        {
            int off_src = get_offset(src);
            int off_dest = get_offset(dest);
            set_var_type(dest, "float");
            /* cvtsi2ss espera um inteiro de 64 bits */
            fprintf(asm_out, "    movq %d(%%rbp), %%rax\n", off_src);
            fprintf(asm_out, "    cvtsi2ss %%eax, %%xmm0\n");
            fprintf(asm_out, "    movss %%xmm0, %d(%%rbp)\n", off_dest);
            return;
        }
    }

    /* ================================================================== */
    /* <dest> = <op_unario> <src>  (minus, minusf, !)                      */
    /* ================================================================== */
    {
        char dest[64], unop[16], src[64];
        if (sscanf(trimmed, "%63s = %15s %63s", dest, unop, src) == 3)
        {
            /* Verifica se é realmente um operador unário (e não parte de uma binop) */
            if (strcmp(unop, "minus") == 0)
            {
                int off_src = get_offset(src);
                int off_dest = get_offset(dest);
                set_var_type(dest, "int");
                fprintf(asm_out, "    movq %d(%%rbp), %%rax\n", off_src);
                fprintf(asm_out, "    negq %%rax\n");
                fprintf(asm_out, "    movq %%rax, %d(%%rbp)\n", off_dest);
                return;
            }
            if (strcmp(unop, "minusf") == 0)
            {
                int off_src = get_offset(src);
                int off_dest = get_offset(dest);
                set_var_type(dest, "float");
                fprintf(asm_out, "    xorps %%xmm0, %%xmm0\n");
                fprintf(asm_out, "    subss %d(%%rbp), %%xmm0\n", off_src);
                fprintf(asm_out, "    movss %%xmm0, %d(%%rbp)\n", off_dest);
                return;
            }
            if (strcmp(unop, "!") == 0)
            {
                int off_src = get_offset(src);
                int off_dest = get_offset(dest);
                set_var_type(dest, "int");
                fprintf(asm_out, "    cmpq $0, %d(%%rbp)\n", off_src);
                fprintf(asm_out, "    sete %%al\n");
                fprintf(asm_out, "    movzbq %%al, %%rax\n");
                fprintf(asm_out, "    movq %%rax, %d(%%rbp)\n", off_dest);
                return;
            }
        }
    }

    /* ================================================================== */
    /* <dest> = <a> <op> <b>  (operação binária int ou float)              */
    /* ================================================================== */
    {
        char dest[64], a[64], op[16], b[64];
        if (sscanf(trimmed, "%63s = %63s %15s %63s", dest, a, op, b) == 4)
        {
            /* Verifica se é operador float (sufixo 'f') */
            int is_float_op = 0;
            char clean_op[16];
            strcpy(clean_op, op);
            int oplen = strlen(clean_op);
            if (oplen > 1 && clean_op[oplen - 1] == 'f')
            {
                clean_op[oplen - 1] = '\0';
                is_float_op = 1;
            }

            int off_dest = get_offset(dest);

            if (is_float_op)
            {
                /* ---- OPERAÇÃO FLOAT ---- */
                set_var_type(dest, "float");

                int off_a = materialize_float(a);
                int off_b = materialize_float(b);

                fprintf(asm_out, "    movss %d(%%rbp), %%xmm0\n", off_a);

                /* Aritméticos */
                if (strcmp(clean_op, "+") == 0)
                    fprintf(asm_out, "    addss %d(%%rbp), %%xmm0\n", off_b);
                else if (strcmp(clean_op, "-") == 0)
                    fprintf(asm_out, "    subss %d(%%rbp), %%xmm0\n", off_b);
                else if (strcmp(clean_op, "*") == 0)
                    fprintf(asm_out, "    mulss %d(%%rbp), %%xmm0\n", off_b);
                else if (strcmp(clean_op, "/") == 0)
                    fprintf(asm_out, "    divss %d(%%rbp), %%xmm0\n", off_b);
                /* Relacionais float (resultado é int: 0 ou 1) */
                else if (strcmp(clean_op, "<") == 0 || strcmp(clean_op, ">") == 0 ||
                         strcmp(clean_op, "<=") == 0 || strcmp(clean_op, ">=") == 0 ||
                         strcmp(clean_op, "==") == 0 || strcmp(clean_op, "!=") == 0)
                {

                    fprintf(asm_out, "    ucomiss %d(%%rbp), %%xmm0\n", off_b);

                    if (strcmp(clean_op, "<") == 0)
                        fprintf(asm_out, "    setb %%al\n");
                    else if (strcmp(clean_op, ">") == 0)
                        fprintf(asm_out, "    seta %%al\n");
                    else if (strcmp(clean_op, "<=") == 0)
                        fprintf(asm_out, "    setbe %%al\n");
                    else if (strcmp(clean_op, ">=") == 0)
                        fprintf(asm_out, "    setae %%al\n");
                    else if (strcmp(clean_op, "==") == 0)
                        fprintf(asm_out, "    sete %%al\n");
                    else if (strcmp(clean_op, "!=") == 0)
                        fprintf(asm_out, "    setne %%al\n");

                    fprintf(asm_out, "    movzbq %%al, %%rax\n");
                    fprintf(asm_out, "    movq %%rax, %d(%%rbp)\n", off_dest);
                    set_var_type(dest, "int"); /* resultado de comparação é int */
                    return;
                }

                fprintf(asm_out, "    movss %%xmm0, %d(%%rbp)\n", off_dest);
            }
            else
            {
                /* ---- OPERAÇÃO INT ---- */
                set_var_type(dest, "int");

                int off_a = materialize_int(a);
                int off_b = materialize_int(b);

                fprintf(asm_out, "    movq %d(%%rbp), %%rax\n", off_a);

                /* Aritméticos */
                if (strcmp(clean_op, "+") == 0)
                    fprintf(asm_out, "    addq %d(%%rbp), %%rax\n", off_b);
                else if (strcmp(clean_op, "-") == 0)
                    fprintf(asm_out, "    subq %d(%%rbp), %%rax\n", off_b);
                else if (strcmp(clean_op, "*") == 0)
                    fprintf(asm_out, "    imulq %d(%%rbp), %%rax\n", off_b);
                else if (strcmp(clean_op, "/") == 0 || strcmp(clean_op, "%") == 0)
                {
                    fprintf(asm_out, "    cqto\n");
                    fprintf(asm_out, "    idivq %d(%%rbp)\n", off_b);
                    if (strcmp(clean_op, "%") == 0)
                        fprintf(asm_out, "    movq %%rdx, %%rax\n");
                }
                /* Relacionais int */
                else if (strcmp(clean_op, "<") == 0 || strcmp(clean_op, ">") == 0 ||
                         strcmp(clean_op, "<=") == 0 || strcmp(clean_op, ">=") == 0 ||
                         strcmp(clean_op, "==") == 0 || strcmp(clean_op, "!=") == 0)
                {

                    fprintf(asm_out, "    cmpq %d(%%rbp), %%rax\n", off_b);

                    if (strcmp(clean_op, "<") == 0)
                        fprintf(asm_out, "    setl %%al\n");
                    else if (strcmp(clean_op, ">") == 0)
                        fprintf(asm_out, "    setg %%al\n");
                    else if (strcmp(clean_op, "<=") == 0)
                        fprintf(asm_out, "    setle %%al\n");
                    else if (strcmp(clean_op, ">=") == 0)
                        fprintf(asm_out, "    setge %%al\n");
                    else if (strcmp(clean_op, "==") == 0)
                        fprintf(asm_out, "    sete %%al\n");
                    else if (strcmp(clean_op, "!=") == 0)
                        fprintf(asm_out, "    setne %%al\n");

                    fprintf(asm_out, "    movzbq %%al, %%rax\n");
                }
                /* Lógicos (&&, ||) — avaliação simples (sem curto-circuito em expr) */
                else if (strcmp(clean_op, "&&") == 0)
                {
                    fprintf(asm_out, "    cmpq $0, %%rax\n");
                    fprintf(asm_out, "    setne %%al\n");
                    fprintf(asm_out, "    movzbq %%al, %%rax\n");
                    fprintf(asm_out, "    cmpq $0, %d(%%rbp)\n", off_b);
                    fprintf(asm_out, "    setne %%cl\n");
                    fprintf(asm_out, "    andb %%cl, %%al\n");
                    fprintf(asm_out, "    movzbq %%al, %%rax\n");
                }
                else if (strcmp(clean_op, "||") == 0)
                {
                    fprintf(asm_out, "    orq %d(%%rbp), %%rax\n", off_b);
                    fprintf(asm_out, "    cmpq $0, %%rax\n");
                    fprintf(asm_out, "    setne %%al\n");
                    fprintf(asm_out, "    movzbq %%al, %%rax\n");
                }

                fprintf(asm_out, "    movq %%rax, %d(%%rbp)\n", off_dest);
            }
            return;
        }
    }

    /* ================================================================== */
    /* <dest> = <array>[<index>]                                          */
    /* ================================================================== */
    {
        char dest[64];
        char rhs[128];
        if (sscanf(trimmed, "%63[^=]=%127[^\n]", dest, rhs) == 2 || sscanf(trimmed, "%63[^=] = %127[^\n]", dest, rhs) == 2)
        {
            char *dtrim = trim(dest);
            char *rtrim = trim(rhs);
            char array_name[64];
            char index_expr[64];
            if (parse_array_access(rtrim, array_name, index_expr))
            {
                const char *arr_type = get_var_type(array_name);
                int off_arr = get_offset(array_name);
                int elem_size = array_slot_size_for_type(arr_type);
                int off_dest = get_offset_typed(dtrim, arr_type);
                set_var_type(dtrim, arr_type);

                if (is_int_literal(index_expr))
                {
                    int idx_val = atoi(index_expr);
                    int elem_off = off_arr + idx_val * elem_size;
                    if (strcmp(arr_type, "string") == 0)
                    {
                        fprintf(asm_out, "    leaq %d(%%rbp), %%rsi\n", elem_off);
                        fprintf(asm_out, "    leaq %d(%%rbp), %%rdi\n", off_dest);
                        fprintf(asm_out, "    call strcpy\n");
                    }
                    else if (strcmp(arr_type, "float") == 0)
                    {
                        fprintf(asm_out, "    movss %d(%%rbp), %%xmm0\n", elem_off);
                        fprintf(asm_out, "    movss %%xmm0, %d(%%rbp)\n", off_dest);
                    }
                    else if (strcmp(arr_type, "char") == 0)
                    {
                        fprintf(asm_out, "    movzbq %d(%%rbp), %%rax\n", elem_off);
                        fprintf(asm_out, "    movq %%rax, %d(%%rbp)\n", off_dest);
                    }
                    else
                    {
                        fprintf(asm_out, "    movq %d(%%rbp), %%rax\n", elem_off);
                        fprintf(asm_out, "    movq %%rax, %d(%%rbp)\n", off_dest);
                    }
                    return;
                }

                int off_index = materialize_int(index_expr);
                fprintf(asm_out, "    movq %d(%%rbp), %%rax\n", off_index);
                if (elem_size != 1)
                    fprintf(asm_out, "    imulq $%d, %%rax\n", elem_size);
                fprintf(asm_out, "    leaq %d(%%rbp), %%rdx\n", off_arr);
                fprintf(asm_out, "    addq %%rax, %%rdx\n");

                if (strcmp(arr_type, "string") == 0)
                {
                    fprintf(asm_out, "    leaq %d(%%rbp), %%rdi\n", off_dest);
                    fprintf(asm_out, "    movq %%rdx, %%rsi\n");
                    fprintf(asm_out, "    call strcpy\n");
                }
                else if (strcmp(arr_type, "float") == 0)
                {
                    fprintf(asm_out, "    movss (%%rdx), %%xmm0\n");
                    fprintf(asm_out, "    movss %%xmm0, %d(%%rbp)\n", off_dest);
                }
                else if (strcmp(arr_type, "char") == 0)
                {
                    fprintf(asm_out, "    movzbq (%%rdx), %%rax\n");
                    fprintf(asm_out, "    movq %%rax, %d(%%rbp)\n", off_dest);
                }
                else
                {
                    fprintf(asm_out, "    movq (%%rdx), %%rax\n");
                    fprintf(asm_out, "    movq %%rax, %d(%%rbp)\n", off_dest);
                }
                return;
            }
        }
    }

    /* ================================================================== */
    /* <array>[<index>] = <src>                                           */
    /* ================================================================== */
    {
        char lhs[128];
        char rhs[64];
        if (sscanf(trimmed, "%127[^=] = %63s", lhs, rhs) == 2)
        {
            char *ltrim = trim(lhs);
            char *rtrim = trim(rhs);
            char array_name[64];
            char index_expr[64];
            if (parse_array_access(ltrim, array_name, index_expr))
            {
                const char *arr_type = get_var_type(array_name);
                int off_arr = get_offset(array_name);
                int elem_size = array_slot_size_for_type(arr_type);

                if (is_int_literal(index_expr))
                {
                    int idx_val = atoi(index_expr);
                    int elem_off = off_arr + idx_val * elem_size;
                    if (strcmp(arr_type, "string") == 0)
                    {
                        int off_src = get_offset_typed(rtrim, "string");
                        set_var_type(rtrim, "string");
                        fprintf(asm_out, "    leaq %d(%%rbp), %%rdi\n", elem_off);
                        fprintf(asm_out, "    leaq %d(%%rbp), %%rsi\n", off_src);
                        fprintf(asm_out, "    call strcpy\n");
                    }
                    else if (strcmp(arr_type, "float") == 0)
                    {
                        int off_src = materialize_float(rtrim);
                        fprintf(asm_out, "    movss %d(%%rbp), %%xmm0\n", off_src);
                        fprintf(asm_out, "    movss %%xmm0, %d(%%rbp)\n", elem_off);
                    }
                    else if (strcmp(arr_type, "char") == 0)
                    {
                        int off_src = materialize_int(rtrim);
                        fprintf(asm_out, "    movq %d(%%rbp), %%rax\n", off_src);
                        fprintf(asm_out, "    movb %%al, %d(%%rbp)\n", elem_off);
                    }
                    else
                    {
                        int off_src = materialize_int(rtrim);
                        fprintf(asm_out, "    movq %d(%%rbp), %%rax\n", off_src);
                        fprintf(asm_out, "    movq %%rax, %d(%%rbp)\n", elem_off);
                    }
                    return;
                }

                int off_index = materialize_int(index_expr);
                fprintf(asm_out, "    movq %d(%%rbp), %%rax\n", off_index);
                if (elem_size != 1)
                    fprintf(asm_out, "    imulq $%d, %%rax\n", elem_size);
                fprintf(asm_out, "    leaq %d(%%rbp), %%rdx\n", off_arr);
                fprintf(asm_out, "    addq %%rax, %%rdx\n");

                if (strcmp(arr_type, "string") == 0)
                {
                    int off_src = get_offset_typed(rtrim, "string");
                    set_var_type(rtrim, "string");
                    fprintf(asm_out, "    movq %%rdx, %%rdi\n");
                    fprintf(asm_out, "    leaq %d(%%rbp), %%rsi\n", off_src);
                    fprintf(asm_out, "    call strcpy\n");
                }
                else if (strcmp(arr_type, "float") == 0)
                {
                    int off_src = materialize_float(rtrim);
                    fprintf(asm_out, "    movss %d(%%rbp), %%xmm0\n", off_src);
                    fprintf(asm_out, "    movss %%xmm0, (%%rdx)\n");
                }
                else if (strcmp(arr_type, "char") == 0)
                {
                    int off_src = materialize_int(rtrim);
                    fprintf(asm_out, "    movq %d(%%rbp), %%rax\n", off_src);
                    fprintf(asm_out, "    movb %%al, (%%rdx)\n");
                }
                else
                {
                    int off_src = materialize_int(rtrim);
                    fprintf(asm_out, "    movq %d(%%rbp), %%rax\n", off_src);
                    fprintf(asm_out, "    movq %%rax, (%%rdx)\n");
                }
                return;
            }
        }
    }

    /* ================================================================== */
    /* <dest> = <valor>  (atribuição simples: cópia ou literal)            */
    /* ================================================================== */
    {
        char dest[64], val[64], extra[128];
        /* Use a stricter sscanf pattern to ensure we only match simple "A = B"
           and not array writes, function calls or concat/ops. The format with
           a third capture allows us to detect extra tokens: result==2 means
           exactly two tokens were found. */
        int sc = sscanf(trimmed, "%63s = %63s %127s", dest, val, extra);
        if (sc == 2 && !strchr(trimmed, '[') && !strstr(trimmed, "call") && !strstr(trimmed, "str "))
        {
            int off_dest = get_offset(dest);

            if (is_int_literal(val))
            {
                set_var_type(dest, "int");
                fprintf(asm_out, "    movq $%s, %%rax\n", val);
                fprintf(asm_out, "    movq %%rax, %d(%%rbp)\n", off_dest);
            }
            else if (is_float_literal(val))
            {
                set_var_type(dest, "float");
                float fval = (float)atof(val);
                unsigned int bits;
                memcpy(&bits, &fval, sizeof(float));
                fprintf(asm_out, "    movl $%u, %%eax\n", bits);
                fprintf(asm_out, "    movl %%eax, %d(%%rbp)\n", off_dest);
            }
            else
            {
                /* Cópia de variável/temporário */
                int off_src = get_offset(val);
                const char *src_type = get_var_type(val);
                off_dest = get_offset_typed(dest, src_type);
                set_var_type(dest, src_type);

                if (strcmp(src_type, "float") == 0)
                {
                    fprintf(asm_out, "    movss %d(%%rbp), %%xmm0\n", off_src);
                    fprintf(asm_out, "    movss %%xmm0, %d(%%rbp)\n", off_dest);
                }
                else if (strcmp(src_type, "string") == 0)
                {
                    emit_copy_string_buffer(off_dest, off_src);
                }
                else
                {
                    fprintf(asm_out, "    movq %d(%%rbp), %%rax\n", off_src);
                    fprintf(asm_out, "    movq %%rax, %d(%%rbp)\n", off_dest);
                }
            }
            return;
        }
    }
}

/* ========================================================================= */
/* MAIN — LEITURA DO TAC E GERAÇÃO DO ASSEMBLY                               */
/* ========================================================================= */

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: ./codegen <file.tac>\n");
        return 1;
    }

    FILE *tac = fopen(argv[1], "r");
    if (!tac)
    {
        fprintf(stderr, "Error: cannot open '%s'\n", argv[1]);
        return 1;
    }

    char lines[4096][512];
    int line_count = 0;
    while (line_count < 4096 && fgets(lines[line_count], sizeof(lines[line_count]), tac))
    {
        line_count++;
    }
    fclose(tac);

    /* Pré-registra literais de string para a seção .data */
    for (int i = 0; i < line_count; i++)
    {
        char local[512];
        strncpy(local, lines[i], sizeof(local) - 1);
        local[sizeof(local) - 1] = '\0';
        char *trimmed = trim(local);
        char dest[64], content[512];
        const char *quote1 = strstr(trimmed, "str \"");
        if (quote1 && sscanf(trimmed, "%63s = str", dest) == 1)
        {
            quote1 += 5;
            const char *quote2 = strrchr(quote1, '"');
            if (quote2 && quote2 > quote1)
            {
                size_t len = (size_t)(quote2 - quote1);
                if (len >= sizeof(content))
                    len = sizeof(content) - 1;
                strncpy(content, quote1, len);
                content[len] = '\0';
                register_string_literal(content);
            }
        }
    }

    asm_out = fopen("output.s", "w");
    if (!asm_out)
    {
        fprintf(stderr, "Error: cannot create output.s\n");
        return 1;
    }

    /* Seção de dados: strings de formatação para print/read */
    fprintf(asm_out, ".data\n");
    fprintf(asm_out, ".str_print_int: .string \"%%ld\\n\"\n");
    fprintf(asm_out, ".str_print_float: .string \"%%f\\n\"\n");
    fprintf(asm_out, ".str_print_char: .string \"%%c\"\n");
    fprintf(asm_out, ".str_print_string: .string \"%%s\\n\"\n");
    fprintf(asm_out, ".str_read_int: .string \"%%d\"\n");
    fprintf(asm_out, ".str_read_float: .string \"%%f\"\n");
    fprintf(asm_out, ".str_read_char: .string \" %%c\"\n");
    fprintf(asm_out, ".str_read_string: .string \"%%255s\"\n");
    emit_string_data_section();
    fprintf(asm_out, ".text\n\n");

    /* Processa cada linha do TAC */
    for (int i = 0; i < line_count; i++)
    {
        process_line(lines[i]);
    }

    fclose(asm_out);

    printf("Assembly gerado com sucesso: 'output.s'\n");
    return 0;
}
