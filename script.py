import os
import sys
#os.system("make");
#nao esqueca de compilar antes de executar
dirList = os.listdir("in")
dirList.sort()

if len(sys.argv)!=5:
	print "*******************************************************"
	print "Entrada: algoritmo tempo cut maxcust"
	print "Ex:{./bnc CLQBC 120 1 5} CLQBC, 120 segundos, c/ cortes"
	print "*******************************************************"
	sys.exit();

output = "out/"+sys.argv[1] + "_"+ sys.argv[2] + "_" + sys.argv[3]+"_"+sys.argv[4]

if not os.path.exists(output):
    os.makedirs(output)

cont=1
for p in dirList:
	#print str(p);
		cmd =  "./bnc "+ sys.argv[1] + " "+ sys.argv[2] + " " +sys.argv[3] + " " +  " in/" + str(p) + " "+ sys.argv[4]+ " > "+output+"/" +str(p) ;
		print str(cont) + " " +cmd
		cont=cont+1
	#	print str(i) + " " +str(p);
		os.system(cmd);
		

