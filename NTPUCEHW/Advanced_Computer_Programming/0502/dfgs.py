import re
a="I have four emails: htpai@mail.ntpu.edu.tw, htpai99@gmail.com, htp@yahoo.com, and a1234@mail.ntpc.gov.tw."
e= re.findall(r'\w+@(mail\.ntpu\.edu\.tw|gmail\.com|mail\.ntpc\.gov\.tw)', a)
f = [match.group(0) for match in re.finditer(r'\w+@(mail\.ntpu\.edu\.tw|gmail\.com|mail\.ntpc\.gov\.tw)', a)]
for e in f:
    print(e)
