
def dec(fn):
    def newFn(n):
        if type(n)!=int or n < 0:
            print("The argument must be an integer and greater than 0.")
            return ''
        return fn(n)
    return newFn
@dec
def sum2N(n):
    ttl=0
    for i in range(1,n+1,1):
        ttl=ttl+i
    return ttl

