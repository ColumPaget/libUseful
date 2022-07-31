LIBS=../libUseful-4.a -lssl -lcrypto -lz

all:
	gcc -oJSON.exe JSON.c $(LIBS)
	gcc -oYAML.exe YAML.c $(LIBS)
	gcc -oCMON.exe CMON.c $(LIBS)
	gcc -oTerminal.exe Terminal.c $(LIBS)
	gcc -oTermBar.exe TermBar.c $(LIBS)
	gcc -oVars.exe Vars.c $(LIBS)
	gcc -oParseRSS.exe ParseRSS.c $(LIBS)
	gcc -oFileCopy.exe FileCopy.c $(LIBS)
	gcc -oTokenize.exe Tokenize.c $(LIBS)
	gcc -oSpawnContainer.exe SpawnContainer.c $(LIBS)
	gcc -oMaps.exe Maps.c $(LIBS)
	gcc -oTTYTest.exe TTYTest.c $(LIBS)
	gcc -oSSLClient.exe SSLClient.c $(LIBS)
	gcc -oErrors.exe Errors.c $(LIBS)
