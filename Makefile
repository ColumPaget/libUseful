CC = gcc
VERSION = 5.26
MAJOR=5
LIBFILE=libUseful.so.$(VERSION)
SONAME=libUseful.so.$(MAJOR)
CFLAGS = -g -O2  -Wl,-soname,${SONAME}
LDFLAGS=
LIBS = -lz -lssl -lcrypto -lc -lc -lc -lc 
prefix=/usr/local
sysconfdir=${prefix}/etc
FLAGS=$(LDFLAGS) $(CPPFLAGS) $(CFLAGS) -fPIC -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -D_FILE_OFFSET_BITS=64 -DHAVE_LIBC=1 -DHAVE_GET_CURR_DIR=1 -DHAVE_PTSNAME_R=1 -DHAVE_CLEARENV=1 -DHAVE_SETRESUID=1 -DHAVE_INITGROUPS=1 -DHAVE_POLL=1 -DHAVE_MLOCK=1 -DHAVE_MLOCKALL=1 -DHAVE_MUNLOCKALL=1 -DHAVE_MADVISE=1 -DHAVE_MKOSTEMP=1 -DHAVE_MOUNT=1 -DHAVE_UMOUNT=1 -DHAVE_UMOUNT2=1 -DHAVE_GETENTROPY=1 -DHAVE_PRCTL=1 -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_SENDFILE=1 -DUSE_INET6=1 -DHAVE_LIBC=1 -DHAVE_XATTR=1 -DHAVE_LIBC=1 -DHAVE_UNSHARE=1 -DHAVE_LIBC=1 -DHAVE_SETNS=1 -DHAVE_LIBCRYPTO=1 -DHAVE_LIBSSL=1 -DHAVE_EVP_MD_CTX_CREATE=1 -DHAVE_EVP_MD_CTX_NEW=1 -DHAVE_EVP_MD_CTX_DESTROY=1 -DHAVE_EVP_MD_CTX_FREE=1 -DHAVE_X509_CHECK_HOST=1 -DHAVE_SSL_SET_MIN_PROTO_VERSION=1 -DHAVE_DECL_OPENSSL_ADD_ALL_ALGORITHMS=1 -DHAVE_OPENSSL_ADD_ALL_ALGORITHMS=1 -DHAVE_DECL_SSL_SET_TLSEXT_HOST_NAME=1 -DHAVE_SSL_SET_TLSEXT_HOST_NAME=1 -DHAVE_LIBZ=1 -DVERSION=\"$(VERSION)\" -DSYSCONFDIR=\"$(sysconfdir)\"
OBJ=StrLenCache.o String.o Array.o List.o IPAddress.o Socket.o Server.o UnixSocket.o Stream.o StreamAuth.o Errors.o Unicode.o TerminalKeys.o Terminal.o TerminalWidget.o TerminalMenu.o TerminalChoice.o TerminalBar.o TerminalProgress.o TerminalTheme.o FileSystem.o GeneralFunctions.o DataProcessing.o Pty.o Log.o HttpUtil.o HttpChunkedTransfer.o Http.o Gemini.o Smtp.o Inet.o Expect.o base32.o base64.o  crc32.o md5c.o sha1.o sha2.o whirlpool.o jh_ref.o HashCRC32.o HashMD5.o HashSHA.o HashJH.o HashWhirlpool.o HashOpenSSL.o Hash.o HMAC.o Ssh.o Compression.o Encryption.o OAuth.o LibSettings.o Vars.o Time.o Markup.o SpawnPrograms.o Tokenizer.o StringList.o PatternMatch.o URL.o DataParser.o ConnectionChain.o OpenSSL.o Process.o Container.o Encodings.o RawData.o SecureMem.o CommandLineParser.o SysInfo.o Entropy.o Users.o UnitsOfMeasure.o HttpServer.o WebSocket.o ContentType.o PasswordFile.o OTP.o CGI.o


all: $(OBJ)
	$(CC) $(FLAGS) -shared -o $(LIBFILE) $(OBJ) $(LIBS) $(LDFLAGS)
	-ln -s -r -f $(LIBFILE) libUseful-$(VERSION).so
	-ln -s -r -f $(LIBFILE) libUseful-$(MAJOR).so
	-ln -s -r -f $(LIBFILE) $(SONAME)
	-ln -s -r -f $(LIBFILE) libUseful.so
	ar rcs libUseful-$(VERSION).a $(OBJ)
	-ln -s -r -f libUseful-$(VERSION).a libUseful-$(MAJOR).a
	-ln -s -r -f libUseful-$(VERSION).a libUseful.a


StrLenCache.o: StrLenCache.h StrLenCache.c
	$(CC) $(FLAGS) -c StrLenCache.c

String.o: String.h String.c
	$(CC) $(FLAGS) -c String.c

Array.o: Array.h Array.c
	$(CC) $(FLAGS) -c Array.c

List.o: List.h List.c
	$(CC) $(FLAGS) -c List.c

IPAddress.o: IPAddress.h IPAddress.c
	$(CC) $(FLAGS) -c IPAddress.c

Socket.o: Socket.h Socket.c
	$(CC) $(FLAGS) -c Socket.c

Server.o: Server.h Server.c
	$(CC) $(FLAGS) -c Server.c

Pty.o: Pty.h Pty.c
	$(CC) $(FLAGS) -c Pty.c

HttpChunkedTransfer.o: HttpChunkedTransfer.h HttpChunkedTransfer.c
	$(CC) $(FLAGS) -c HttpChunkedTransfer.c

Http.o: Http.h Http.c
	$(CC) $(FLAGS) -c Http.c

HttpUtil.o: HttpUtil.h HttpUtil.c
	$(CC) $(FLAGS) -c HttpUtil.c

HttpServer.o: HttpServer.h HttpServer.c
	$(CC) $(FLAGS) -c HttpServer.c

Gemini.o: Gemini.h Gemini.c
	$(CC) $(FLAGS) -c Gemini.c

Smtp.o: Smtp.h Smtp.c
	$(CC) $(FLAGS) -c Smtp.c

Stream.o: Stream.h Stream.c
	$(CC) $(FLAGS) -c Stream.c

StreamAuth.o: StreamAuth.h StreamAuth.c
	$(CC) $(FLAGS) -c StreamAuth.c

Unicode.o: Unicode.c Unicode.h
	$(CC) $(FLAGS) -c Unicode.c

TerminalKeys.o: TerminalKeys.h TerminalKeys.c
	$(CC) $(FLAGS) -c TerminalKeys.c

Terminal.o: Terminal.h Terminal.c
	$(CC) $(FLAGS) -c Terminal.c

TerminalWidget.o: TerminalWidget.h TerminalWidget.c
	$(CC) $(FLAGS) -c TerminalWidget.c

TerminalMenu.o: TerminalMenu.h TerminalMenu.c
	$(CC) $(FLAGS) -c TerminalMenu.c

TerminalChoice.o: TerminalChoice.h TerminalChoice.c
	$(CC) $(FLAGS) -c TerminalChoice.c

TerminalBar.o: TerminalBar.h TerminalBar.c
	$(CC) $(FLAGS) -c TerminalBar.c

TerminalProgress.o: TerminalProgress.h TerminalProgress.c
	$(CC) $(FLAGS) -c TerminalProgress.c

TerminalTheme.o: TerminalTheme.h TerminalTheme.c
	$(CC) $(FLAGS) -c TerminalTheme.c

Errors.o: Errors.h Errors.c
	$(CC) $(FLAGS) -c Errors.c

Log.o: Log.h Log.c
	$(CC) $(FLAGS) -c Log.c

UnixSocket.o: UnixSocket.h UnixSocket.c
	$(CC) $(FLAGS) -c UnixSocket.c

PatternMatch.o: PatternMatch.h PatternMatch.c
	$(CC) $(FLAGS) -c PatternMatch.c

FileSystem.o: FileSystem.h FileSystem.c
	$(CC) $(FLAGS) -c FileSystem.c

Time.o: Time.h Time.c
	$(CC) $(FLAGS) -c Time.c

Tokenizer.o: Tokenizer.h Tokenizer.c
	$(CC) $(FLAGS) -c Tokenizer.c

StringList.o: StringList.h StringList.c
	$(CC) $(FLAGS) -c StringList.c

Markup.o: Markup.h Markup.c
	$(CC) $(FLAGS) -c Markup.c

URL.o: URL.h URL.c
	$(CC) $(FLAGS) -c URL.c

DataParser.o: DataParser.h DataParser.c
	$(CC) $(FLAGS) -c DataParser.c

Inet.o: Inet.h Inet.c
	$(CC) $(FLAGS) -c Inet.c

Expect.o: Expect.h Expect.c
	$(CC) $(FLAGS) -c Expect.c

SecureMem.o: SecureMem.h SecureMem.c
	$(CC) $(FLAGS) -c SecureMem.c

GeneralFunctions.o: GeneralFunctions.h GeneralFunctions.c
	$(CC) $(FLAGS) -c GeneralFunctions.c

DataProcessing.o: DataProcessing.h DataProcessing.c
	$(CC) $(FLAGS) -c DataProcessing.c

HashCRC32.o: HashCRC32.h HashCRC32.c
	$(CC) $(FLAGS) -c HashCRC32.c

HashMD5.o: HashMD5.h HashMD5.c
	$(CC) $(FLAGS) -c HashMD5.c

HashSHA.o: HashSHA.h HashSHA.c
	$(CC) $(FLAGS) -c HashSHA.c

HashJH.o: HashJH.h HashJH.c
	$(CC) $(FLAGS) -c HashJH.c

HashWhirlpool.o: HashWhirlpool.h HashWhirlpool.c
	$(CC) $(FLAGS) -c HashWhirlpool.c

HashOpenSSL.o: HashOpenSSL.h HashOpenSSL.c
	$(CC) $(FLAGS) -c HashOpenSSL.c

Hash.o: Hash.h Hash.c
	$(CC) $(FLAGS) -c Hash.c

HMAC.o: HMAC.h HMAC.c
	$(CC) $(FLAGS) -c HMAC.c

Ssh.o: Ssh.h Ssh.c
	$(CC) $(FLAGS) -c Ssh.c

WebSocket.o: WebSocket.h WebSocket.c
	$(CC) $(FLAGS) -c WebSocket.c

Compression.o: Compression.h Compression.c
	$(CC) $(FLAGS) -c Compression.c

Encryption.o: Encryption.h Encryption.c
	$(CC) $(FLAGS) -c Encryption.c

ConnectionChain.o: ConnectionChain.h ConnectionChain.c
	$(CC) $(FLAGS) -c ConnectionChain.c

base32.o: base32.c base32.h
	$(CC) $(FLAGS) -c base32.c

base64.o: base64.c base64.h
	$(CC) $(FLAGS) -c base64.c

crc32.o: crc32.c crc32.h
	$(CC) $(FLAGS) -c crc32.c

md5c.o: md5c.c md5-global.h md5.h
	$(CC) $(FLAGS) -c md5c.c

sha1.o: sha1.c sha1.h
	$(CC) $(FLAGS) -c sha1.c

sha2.o: sha2.c sha2.h
	$(CC) $(FLAGS) -DSHA2_UNROLL_TRANSFORM -c sha2.c

whirlpool.o: whirlpool.c whirlpool.h
	$(CC) $(FLAGS) -c whirlpool.c

jh_ref.o: jh_ref.c jh_ref.h
	$(CC) $(FLAGS) -c jh_ref.c

OAuth.o: OAuth.c OAuth.h
	$(CC) $(FLAGS) -c OAuth.c

OpenSSL.o: OpenSSL.c OpenSSL.h
	$(CC) $(FLAGS) -c OpenSSL.c

Process.o: Process.c Process.h
	$(CC) $(FLAGS) -c Process.c

Container.o: Container.c Container.h
	$(CC) $(FLAGS) -c Container.c

Vars.o: Vars.c Vars.h
	$(CC) $(FLAGS) -c Vars.c

SpawnPrograms.o: SpawnPrograms.c SpawnPrograms.h
	$(CC) $(FLAGS) -c SpawnPrograms.c

Encodings.o: Encodings.c Encodings.h
	$(CC) $(FLAGS) -c Encodings.c

RawData.o: RawData.c RawData.h
	$(CC) $(FLAGS) -c RawData.c

CommandLineParser.o: CommandLineParser.c CommandLineParser.h
	$(CC) $(FLAGS) -c CommandLineParser.c

SysInfo.o: SysInfo.c SysInfo.h
	$(CC) $(FLAGS) -c SysInfo.c

Entropy.o: Entropy.c Entropy.h
	$(CC) $(FLAGS) -c Entropy.c

Users.o: Users.c Users.h
	$(CC) $(FLAGS) -c Users.c

UnitsOfMeasure.o: UnitsOfMeasure.c UnitsOfMeasure.h
	$(CC) $(FLAGS) -c UnitsOfMeasure.c

ContentType.o: ContentType.c ContentType.h
	$(CC) $(FLAGS) -c ContentType.c

PasswordFile.o: PasswordFile.c PasswordFile.h
	$(CC) $(FLAGS) -c PasswordFile.c

OTP.o: OTP.c OTP.h
	$(CC) $(FLAGS) -c OTP.c

CGI.o: CGI.c CGI.h
	$(CC) $(FLAGS) -c CGI.c



#No dependancies, must always be compiled
LibSettings.o: LibSettings.h LibSettings.c
	$(CC) $(FLAGS) -c LibSettings.c

clean:
	-rm -f *.o *.so *.so.* *.a *.orig .*.swp
	-rm config.log config.status 
	-rm -r autom4te.cache config.cache
	-$(MAKE) clean -C examples

install: libUseful.so
	-mkdir -p $(DESTDIR)$(prefix)/lib 
	cp -P *.so *.so.* *.a $(DESTDIR)$(prefix)/lib  
	-mkdir -p $(DESTDIR)$(prefix)/include/libUseful-$(VERSION)
	cp *.h $(DESTDIR)$(prefix)/include/libUseful-$(VERSION)
	-ln -s -r -f $(DESTDIR)$(prefix)/include/libUseful-$(VERSION) $(DESTDIR)$(prefix)/include/libUseful-5
	-mkdir -p $(DESTDIR)$(prefix)/etc
	cp *.conf $(DESTDIR)$(prefix)/etc


test: libUseful.so
	-echo "No tests written yet"
