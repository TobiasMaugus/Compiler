.data
.str_print_int: .string "%ld\n"
.str_print_float: .string "%f\n"
.str_print_char: .string "%c"
.str_print_string: .string "%s\n"
.str_read_int: .string "%d"
.str_read_float: .string "%f"
.str_read_char: .string " %c"
.str_read_string: .string "%255s"
.str_lit_0: .string "a"
.str_lit_1: .string "b"
.str_lit_2: .string "c"
.str_lit_3: .string "d"
.str_lit_4: .string "e"
.str_lit_5: .string "f"
.str_lit_6: .string "--- Teste Matriz 2D INT ---"
.str_lit_7: .string "Elemento [1][2] (esperado 6):"
.str_lit_8: .string "ola"
.str_lit_9: .string "Elemento [0][1] modificado (esperado 42):"
.text

.global main
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $8192, %rsp
    leaq -1792(%rbp), %rdi
    leaq .str_lit_0(%rip), %rsi
    call strcpy
    leaq -1536(%rbp), %rdi
    leaq -1792(%rbp), %rsi
    call strcpy
    leaq -2048(%rbp), %rdi
    leaq .str_lit_1(%rip), %rsi
    call strcpy
    leaq -1280(%rbp), %rdi
    leaq -2048(%rbp), %rsi
    call strcpy
    leaq -2304(%rbp), %rdi
    leaq .str_lit_2(%rip), %rsi
    call strcpy
    leaq -1024(%rbp), %rdi
    leaq -2304(%rbp), %rsi
    call strcpy
    leaq -2560(%rbp), %rdi
    leaq .str_lit_3(%rip), %rsi
    call strcpy
    leaq -768(%rbp), %rdi
    leaq -2560(%rbp), %rsi
    call strcpy
    leaq -2816(%rbp), %rdi
    leaq .str_lit_4(%rip), %rsi
    call strcpy
    leaq -512(%rbp), %rdi
    leaq -2816(%rbp), %rsi
    call strcpy
    leaq -3072(%rbp), %rdi
    leaq .str_lit_5(%rip), %rsi
    call strcpy
    leaq -256(%rbp), %rdi
    leaq -3072(%rbp), %rsi
    call strcpy
    leaq -3328(%rbp), %rdi
    leaq .str_lit_6(%rip), %rsi
    call strcpy
    leaq -3328(%rbp), %rsi
    leaq .str_print_string(%rip), %rdi
    movb $0, %al
    call printf
    leaq -3584(%rbp), %rdi
    leaq .str_lit_7(%rip), %rsi
    call strcpy
    leaq -3584(%rbp), %rsi
    leaq .str_print_string(%rip), %rdi
    movb $0, %al
    call printf
    movq $3, %rax
    movq %rax, -3592(%rbp)
    movq $2, %rax
    movq %rax, -3608(%rbp)
    movq -3592(%rbp), %rax
    addq -3608(%rbp), %rax
    movq %rax, -3600(%rbp)
    movq -3600(%rbp), %rax
    imulq $256, %rax
    leaq -1536(%rbp), %rdx
    addq %rax, %rdx
    leaq -3864(%rbp), %rdi
    movq %rdx, %rsi
    call strcpy
    leaq -3864(%rbp), %rsi
    leaq .str_print_string(%rip), %rdi
    movb $0, %al
    call printf
    leaq -4120(%rbp), %rdi
    leaq .str_lit_8(%rip), %rsi
    call strcpy
    movq $0, %rax
    movq %rax, -4128(%rbp)
    movq $1, %rax
    movq %rax, -4144(%rbp)
    movq -4128(%rbp), %rax
    addq -4144(%rbp), %rax
    movq %rax, -4136(%rbp)
    movq -4136(%rbp), %rax
    imulq $256, %rax
    leaq -1536(%rbp), %rdx
    addq %rax, %rdx
    movq %rdx, %rdi
    leaq -4120(%rbp), %rsi
    call strcpy
    leaq -4400(%rbp), %rdi
    leaq .str_lit_9(%rip), %rsi
    call strcpy
    leaq -4400(%rbp), %rsi
    leaq .str_print_string(%rip), %rdi
    movb $0, %al
    call printf
    movq $0, %rax
    movq %rax, -4128(%rbp)
    movq $1, %rax
    movq %rax, -4144(%rbp)
    movq -4128(%rbp), %rax
    addq -4144(%rbp), %rax
    movq %rax, -3600(%rbp)
    movq -3600(%rbp), %rax
    imulq $256, %rax
    leaq -1536(%rbp), %rdx
    addq %rax, %rdx
    leaq -4656(%rbp), %rdi
    movq %rdx, %rsi
    call strcpy
    movq $0, %rax
    movq %rax, -4128(%rbp)
    movq $1, %rax
    movq %rax, -4144(%rbp)
    movq -4128(%rbp), %rax
    addq -4144(%rbp), %rax
    movq %rax, -3600(%rbp)
    movq -3600(%rbp), %rax
    imulq $256, %rax
    leaq -1536(%rbp), %rdx
    addq %rax, %rdx
    leaq -4912(%rbp), %rdi
    movq %rdx, %rsi
    call strcpy
    leaq -5168(%rbp), %rdi
    leaq -4656(%rbp), %rsi
    call strcpy
    leaq -5168(%rbp), %rdi
    leaq -4912(%rbp), %rsi
    call strcat
    leaq -5168(%rbp), %rsi
    leaq .str_print_string(%rip), %rdi
    movb $0, %al
    call printf
    movq $0, %rax
    leave
    ret
    leave
    ret
