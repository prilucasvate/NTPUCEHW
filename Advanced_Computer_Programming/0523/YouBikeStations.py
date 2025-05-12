import  urllib.request,re
url = 'https://data.ntpc.gov.tw/api/datasets/71cd1490-a2df-4198-bef1-318479775e8a/csv/file'
webpage = urllib.request.urlopen(url)
line = webpage.read().decode('utf-8').splitlines()
dic={}
lstk=[]
lstv=[]
for j in range(1,len(line)):
    patn = re.compile(r'\"\d+')
    patc = re.compile(r'[^a-z|A-Z|\"|,]+')
    res = patn.findall(line[j])
    ress = patc.findall(line[j])
    if ress[4] in dic:
        dic[ress[4]] = (int(res[1][1:])+int(dic[ress[4]])) 
    else: 
        dic[ress[4]]=int(res[1][1:])
lstv=list(dic.values())
lstk=list(dic.keys())
lstv.sort()
for i in range(0,len(dic)):
    for j in range(0,len(dic)):
        if(lstv[i]==dic.get(lstk[j])):
            print(lstk[j],":",lstv[i])

#dic=sorted(dic.items(), key = lambda x:(x[1], x[0]))
#for i in dic:
#    print(i)