LIBS=../libUseful.a -lssl -lcrypto -lz #-lcap

all:
	gcc -oJSON.exe JSON.c $(LIBS)
	gcc -oCMON.exe CMON.c $(LIBS)
	gcc -oYAML.exe YAML.c $(LIBS)
	gcc -oHash.exe Hash.c $(LIBS)
	gcc -oHMAC.exe HMAC.c $(LIBS)
	gcc -oTerminal.exe Terminal.c $(LIBS)
	gcc -oTermBar.exe TermBar.c $(LIBS)
	gcc -oTermMenu.exe TermMenu.c $(LIBS)
	gcc -oTermChoice.exe TermChoice.c $(LIBS)
	gcc -oTermProgress.exe TermProgress.c $(LIBS)
	gcc -oTermCalendar.exe TermCalendar.c $(LIBS)
	gcc -oVars.exe Vars.c $(LIBS)
	gcc -oParseRSS.exe ParseRSS.c $(LIBS)
	gcc -oFileCopy.exe FileCopy.c $(LIBS)
	gcc -oTokenize.exe Tokenize.c $(LIBS)
	gcc -oSpawnContainer.exe SpawnContainer.c $(LIBS)
	gcc -oMaps.exe Maps.c $(LIBS)
	gcc -oTTYTest.exe TTYTest.c $(LIBS)
	gcc -oErrors.exe Errors.c $(LIBS)
	gcc -ossh-time.exe ssh-time.c $(LIBS)
	gcc -ossh-tunnel.exe ssh-tunnel.c $(LIBS)
	gcc -ossh-listfiles.exe ssh-listfiles.c $(LIBS)
	gcc -oConfig.exe Config.c $(LIBS)
	gcc -oStrLen.exe StrLen.c $(LIBS)
	gcc -oSSLClient.exe SSLClient.c $(LIBS)
	gcc -oSysInfo.exe SysInfo.c $(LIBS)
	gcc -oWebsocket.exe Websocket.c $(LIBS)
	gcc -oHttpServer.exe HttpServer.c $(LIBS)
	gcc -oPasswordFile.exe PasswordFile.c $(LIBS)
	gcc -obase32.exe base32.c $(LIBS)
	gcc -obase64.exe base64.c $(LIBS)
	gcc -ouudecode.exe uudecode.c $(LIBS)
	gcc -ononewprivs.exe nonewprivs.c $(LIBS)
	gcc -ochroot.exe chroot.c $(LIBS)
	gcc -omovetest.exe movetest.c $(LIBS)
	gcc -oencrypt.exe encrypt.c $(LIBS)
	gcc -odecrypt.exe decrypt.c $(LIBS)
	gcc -opbkdf2.exe pbkdf2.c $(LIBS)
	gcc -ofmtduration.exe fmtduration.c $(LIBS)
	gcc -ocalendar.exe calendar.c $(LIBS)
	gcc -oLineEdit.exe LineEdit.c $(LIBS)
	gcc -ocontainer.exe container.c $(LIBS)
	gcc -oListLimits.exe ListLimits.c $(LIBS)

seccomp.exe: seccomp.c
	gcc -oseccomp.exe seccomp.c $(LIBS)

clean:
	rm -f *.exe
