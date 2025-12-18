import os
print("\033[40;37m\ngive me a file to edit create\n")
i=input().strip()
r=""
if os.path.exists(i):
    f1=open(i,"r")
    r=f1.read()
    f1.close()
    print(r)
while(True):
    ii=input()
    iii=ii.strip()
    if iii=="":
        break
    f1=open(i,"a")
    f1.write(ii+"\r\n")
    f1.close()