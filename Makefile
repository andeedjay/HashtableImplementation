build: hashtable.exe 
	
hashtable.exe: hashtable.obj hash.lib
	cl hash.lib hashtable.obj
	
hashtable.obj:
	cl /c hashtable.c
	
clean:	
	del hashtable.exe hashtable.obj