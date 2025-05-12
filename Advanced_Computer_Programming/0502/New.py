squares = [x**2 for x in range(10)]
print(squares)
[(x, y) for x in [1,2,3] for y in [3,1,4] if x != y]

sentence = "The fox jumps over the dog."
words = sentence.split()
wordsLen = [len(x) for x in words if x!="the"]
print(wordsLen)