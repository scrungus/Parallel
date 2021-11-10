from ctypes import *

libCompute = CDLL("./main.so")

readfromfile = libCompute.readfromfile
readfromfile.restype = POINTER(POINTER(c_double))
readfromfile.argtypes = c_char_p,c_int

p = 3
P = 0.1
n = 10

compute = libCompute.compute
compute.restype = POINTER(POINTER(c_double))
compute.argtypes = c_int,c_double,c_int,POINTER(POINTER(c_double))

file = "e".encode('utf-8')

a = readfromfile(file,n)
ar = compute(p,P,n,a);

for i in range(n):
    for j in range(n):
        print(a[i][j],end=' ')
    print()

print("COMPUTED : ")
for i in range(n):
    for j in range(n):
        print(ar[i][j],end=' ')
    print()
