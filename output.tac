func main int params: 
alloc_array matriz, 6, string
t1 = str "a"
matriz[0] = t1
t2 = str "b"
matriz[1] = t2
t3 = str "c"
matriz[2] = t3
t4 = str "d"
matriz[3] = t4
t5 = str "e"
matriz[4] = t5
t6 = str "f"
matriz[5] = t6
t7 = str "--- Teste Matriz 2D INT ---"
print_string t7
t8 = str "Elemento [1][2] (esperado 6):"
print_string t8
1_mul_0 = 3
acc_1 = 1_mul_0 + 2
t9 = matriz[acc_1]
print_string t9
t10 = str "ola"
0_mul_0 = 0
acc_w_1 = 0_mul_0 + 1
matriz[acc_w_1] = t10
t11 = str "Elemento [0][1] modificado (esperado 42):"
print_string t11
0_mul_0 = 0
acc_1 = 0_mul_0 + 1
t12 = matriz[acc_1]
0_mul_0 = 0
acc_1 = 0_mul_0 + 1
t13 = matriz[acc_1]
t14 = concat t12, t13
print_string t14
return 0
endfunc
