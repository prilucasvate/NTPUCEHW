import Sum
from Sum import dec

@dec
def test(n):
	return n

print(test(5))
print(Sum.sum2N(5))
test(-1)
test(1.1)
Sum.sum2N(-1)
Sum.sum2N(1.1)