import numpy as np
from tabulate import tabulate as t
x = np.random.randint(0,high=10,size=(9,9))

xs = list()

for i in range(0,7,2):
    for j in range(0,7,2):
        xs.append(x[j:j+3,i:i+3])

o = t(x,tablefmt="grid")

for xi in xs:
    xi = t(xi,tablefmt="grid")
    print(xi)
    print()

print(o)

""" for i in range(0,7,3):
    print(xs[i],end='\t')
    print(xs[i+1],end='\t')
    print(xs[i+2],end='\t')
    print() """