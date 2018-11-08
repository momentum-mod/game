from os import listdir
from os.path import isfile, join
mypath = "C:\\Users\\Nick\\Documents\\GitHub\\game\\mp\\src\\public\\cryptopp"

onlyfiles = [f for f in listdir(mypath) if isfile(join(mypath, f))]
for file in onlyfiles:
    print("$File \"$SRCDIR\\public\\cryptopp\\" + file + "\"")
