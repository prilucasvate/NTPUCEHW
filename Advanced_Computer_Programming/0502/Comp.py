import sys

newFile = open(sys.argv[1] , 'r+',encoding='utf-8')
oldFile = open(sys.argv[2], 'r+',encoding='utf-8')
countLine = len(open(sys.argv[2],'r').readlines())
oldLine=[]
newLine=[]
for i in range(0,countLine):
    newLine.append(newFile.readline())
    oldLine.append(oldFile.readline())
if oldLine==newLine:
    print("Identical files!")
else:
    diffFile = open('Diff', 'w+', encoding='utf-8')
    for j in range(0,countLine):
        if oldLine[j]==newLine[j]:
            diffFile.write(newLine[j])
        else:
            diffFile.write(newLine[j].upper())
