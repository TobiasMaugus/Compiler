func isPrime int params: int n
i = 2
if n < 2 goto L1
goto L2
L1:
return 0
L2:
L3:
if i < n goto L4
goto L5
L4:
t1 = n % i
if t1 == 0 goto L6
goto L7
L6:
return 0
L7:
t2 = i + 1
i = t2
goto L3
L5:
return 1
endfunc
func main int params: 
num = 1
resultado = 0
L8:
if num > 0 goto L9
goto L10
L9:
read_int num
if num > 0 goto L11
goto L12
L11:
param num
t3 = call isPrime, 1
resultado = t3
print_int resultado
L12:
goto L8
L10:
return 0
endfunc
