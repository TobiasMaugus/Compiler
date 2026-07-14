func printIsPrime void params: int x
if x != 0 goto L1
goto L2
L1:
t1 = str "Is prime"
print_string t1
goto L3
L2:
t2 = str "Is not prime"
print_string t2
L3:
endfunc
func isPrime int params: int n
i = 2
if n < 2 goto L4
goto L5
L4:
return 0
L5:
L6:
if i < n goto L7
goto L8
L7:
t3 = n % i
if t3 == 0 goto L9
goto L10
L9:
return 0
L10:
t4 = i + 1
i = t4
goto L6
L8:
return 1
endfunc
func main int params: 
num = 1
resultado = 0
L11:
if num > 0 goto L12
goto L13
L12:
read_int num
if num > 0 goto L14
goto L15
L14:
param num
t5 = call isPrime, 1
resultado = t5
param resultado
t6 = call printIsPrime, 1
L15:
goto L11
L13:
return 0
endfunc
