import os

time = 1200
for i in range(1,100,2):
	cmd =  "python script.py CLQBC "+ str(time) +" 1 "+ str(i)
	print cmd
	os.system(cmd)


#cmd =  "python script.py FN "+ str(time) +" 0 0"
#os.system(cmd)


#cmd =  "python script.py FN "+ str(time) +" 1 0"
#os.system(cmd)


#cmd =  "python script.py CLQBC "+ str(time) +" 0 0"
#os.system(cmd)

#cmd =  "python script.py CLQBC "+ str(time) +" 1 0"
#os.system(cmd)


#cmd =  "python script.py CLQBB "+ str(time) +" 0 0"
#os.system(cmd)


#cmd =  "python script.py CLQBB "+ str(time) +" 1 0"
#os.system(cmd)





