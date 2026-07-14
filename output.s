.data
.str_print_int: .string "%ld\n"
.str_print_float: .string "%f\n"
.str_print_char: .string "%c"
.str_print_string: .string "%s\n"
.str_read_int: .string "%d"
.str_read_float: .string "%f"
.str_read_char: .string " %c"
.str_read_string: .string "%255s"
.str_lit_0: .string "Is prime"
.str_lit_1: .string "Is not prime"
.text

.global printIsPrime
printIsPrime:
    pushq %rbp
    movq %rsp, %rbp
    subq $8192, %rsp
    movq %rdi, -8(%rbp)
    movq $0, %rax
    movq %rax, -16(%rbp)
    movq -8(%rbp), %rax
    cmpq -16(%rbp), %rax
    jne L1
    jmp L2
L1:
    leaq -272(%rbp), %rdi
    leaq .str_lit_0(%rip), %rsi
    call strcpy
    leaq -272(%rbp), %rsi
    leaq .str_print_string(%rip), %rdi
    movb $0, %al
    call printf
    jmp L3
L2:
    leaq -528(%rbp), %rdi
    leaq .str_lit_1(%rip), %rsi
    call strcpy
    leaq -528(%rbp), %rsi
    leaq .str_print_string(%rip), %rdi
    movb $0, %al
    call printf
L3:
    leave
    ret
.global isPrime
isPrime:
    pushq %rbp
    movq %rsp, %rbp
    subq $8192, %rsp
    movq %rdi, -8(%rbp)
    movq $2, %rax
    movq %rax, -16(%rbp)
    movq $2, %rax
    movq %rax, -24(%rbp)
    movq -8(%rbp), %rax
    cmpq -24(%rbp), %rax
    jl L4
    jmp L5
L4:
    movq $0, %rax
    leave
    ret
L5:
L6:
    movq -16(%rbp), %rax
    cmpq -8(%rbp), %rax
    jl L7
    jmp L8
L7:
    movq -8(%rbp), %rax
    cqto
    idivq -16(%rbp)
    movq %rdx, %rax
    movq %rax, -32(%rbp)
    movq $0, %rax
    movq %rax, -40(%rbp)
    movq -32(%rbp), %rax
    cmpq -40(%rbp), %rax
    je L9
    jmp L10
L9:
    movq $0, %rax
    leave
    ret
L10:
    movq $1, %rax
    movq %rax, -56(%rbp)
    movq -16(%rbp), %rax
    addq -56(%rbp), %rax
    movq %rax, -48(%rbp)
    movq -48(%rbp), %rax
    movq %rax, -16(%rbp)
    jmp L6
L8:
    movq $1, %rax
    leave
    ret
    leave
    ret
.global main
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $8192, %rsp
    movq $1, %rax
    movq %rax, -8(%rbp)
    movq $0, %rax
    movq %rax, -16(%rbp)
L11:
    movq $0, %rax
    movq %rax, -24(%rbp)
    movq -8(%rbp), %rax
    cmpq -24(%rbp), %rax
    jg L12
    jmp L13
L12:
    leaq .str_read_int(%rip), %rdi
    leaq -8(%rbp), %rsi
    movb $0, %al
    call scanf
    movq $0, %rax
    movq %rax, -24(%rbp)
    movq -8(%rbp), %rax
    cmpq -24(%rbp), %rax
    jg L14
    jmp L15
L14:
    movq -8(%rbp), %rdi
    movb $0, %al
    call isPrime
    movq %rax, -32(%rbp)
    movq -32(%rbp), %rax
    movq %rax, -16(%rbp)
    movq -16(%rbp), %rdi
    movb $0, %al
    call printIsPrime
    movq %rax, -40(%rbp)
L15:
    jmp L11
L13:
    movq $0, %rax
    leave
    ret
    leave
    ret
