import  urllib.request,csv
url = 'https://data.ntpc.gov.tw/api/datasets/71cd1490-a2df-4198-bef1-318479775e8a/csv/file'
webpage = urllib.request.urlopen(url)  #開啟網頁
line = webpage.read().decode('utf-8').splitlines()#讀取資料到data陣列中
