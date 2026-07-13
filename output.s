.data
.str_print_int: .string "%ld\n"
.str_print_float: .string "%f\n"
.str_read_int: .string "%d"
.str_read_float: .string "%f"
.text

.global isPrime
isPrime:
    pushq %rbp
    movq %rsp, %rbp
    subq $400, %rsp
    movq %rdi, -8(%rbp)
    movq $2, %rax
    movq %rax, -16(%rbp)
    movq $2, %rax
    movq %rax, -24(%rbp)
    movq -8(%rbp), %rax
    cmpq -24(%rbp), %rax
    jl L1
    jmp L2
L1:
    movq $0, %rax
    leave
    ret
L2:
L3:
    movq -16(%rbp), %rax
    cmpq -8(%rbp), %rax
    jl L4
    jmp L5
L4:
    movq -8(%rbp), %rax
    cqto
    idivq -16(%rbp)
    movq %rdx, %rax
    movq %rax, -32(%rbp)
    movq $0, %rax
    movq %rax, -40(%rbp)
    movq -32(%rbp), %rax
    cmpq -40(%rbp), %rax
    je L6
    jmp L7
L6:
    movq $0, %rax
    leave
    ret
L7:
    movq $1, %rax
    movq %rax, -56(%rbp)
    movq -16(%rbp), %rax
    addq -56(%rbp), %rax
    movq %rax, -48(%rbp)
    movq -48(%rbp), %rax
    movq %rax, -16(%rbp)
    jmp L3
L5:
    movq $1, %rax
    leave
    ret
    leave
    ret
.global main
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $400, %rsp
    movq $1, %rax
    movq %rax, -8(%rbp)
    movq $0, %rax
    movq %rax, -16(%rbp)
L8:
    movq $0, %rax
    movq %rax, -24(%rbp)
    movq -8(%rbp), %rax
    cmpq -24(%rbp), %rax
    jg L9
    jmp L10
L9:
    leaq .str_read_int(%rip), %rdi
    leaq -8(%rbp), %rsi
    movb $0, %al
    call scanf
    movq $0, %rax
    movq %rax, -24(%rbp)
    movq -8(%rbp), %rax
    cmpq -24(%rbp), %rax
    jg L11
    jmp L12
L11:
    movq -8(%rbp), %rdi
    movb $0, %al
    call isPrime
    movq %rax, -32(%rbp)
    movq -32(%rbp), %rax
    movq %rax, -16(%rbp)
    movq -16(%rbp), %rsi
    leaq .str_print_int(%rip), %rdi
    movb $0, %al
    call printf
L12:
    jmp L8
L10:
    movq $0, %rax
    leave
    ret
    leave
    ret
