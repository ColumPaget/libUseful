CC = gcc
VERSION = 3.2
CFLAGS = -g -O2 
LIBS = -lcap -lc -lc -lc -lc -lc -lc 
FLAGS=$(LDFLAGS) $(CPPFLAGS) $(CFLAGS) -fPIC -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -DSTDC_HEADERS=1 -D_FILE_OFFSET_BITS=64 -DHAVE_LIBC=1 -DHAVE_PTSNAME_R=1 -DHAVE_LIBC=1 -DHAVE_CLEARENV=1 -DHAVE_LIBC=1 -DHAVE_SETRESUID=1 -DHAVE_LIBC=1 -DHAVE_UMOUNT2=1 -DHAVE_LIBC=1 -DHAVE_UMOUNT=1 -DHAVE_LIBC=1 -DHAVE_MKOSTEMP=1 -DHAVE_LIBCAP=1 -DUSE_CAPABILITIES=1 -DHAVE_MADVISE -DHAVE_MADVISE_NOFORK -DHAVE_MADVISE_DONTDUMP -DHAVE_MLOCK
prefix=/usr/local
OBJ=String.o List.o Socket.o UnixSocket.o Stream.o Errors.o Unicode.o Terminal.o FileSystem.o GeneralFunctions.o DataProcessing.o Pty.o Log.o Http.o Smtp.o inet.o Expect.o base64.o  crc32.o md5c.o sha1.o sha2.o whirlpool.o jh_ref.o Hash.o Ssh.o Compression.o OAuth.o LibSettings.o Vars.o Time.o Markup.o SpawnPrograms.o Tokenizer.o PatternMatch.o URL.o DataParser.o ConnectionChain.o OpenSSL.o Process.o Encodings.o RawData.o SecureMem.o CommandLineParser.o


all: $(OBJ)
	$(CC) $(FLAGS) -shared -o libUseful-$(VERSION).so $(OBJ) $(LIBS) 
	ar rcs libUseful-$(VERSION).a $(OBJ)
	-ln -s libUseful-$(VERSION).so libUseful-3.so &>/dev/null
	-ln -s libUseful-$(VERSION).a libUseful-3.a &>/dev/null
	-ln -s libUseful-$(VERSION).so libUseful.so &>/dev/null
	-ln -s libUseful-$(VERSION).a libUseful.a &>/dev/null


String.o: String.h String.c
	$(CC) $(FLAGS) -c String.c

List.o: List.h List.c
	$(CC) $(FLAGS) -c List.c

Socket.o: Socket.h Socket.c
	$(CC) $(FLAGS) -c Socket.c

Pty.o: Pty.h Pty.c
	$(CC) $(FLAGS) -c Pty.c

Http.o: Http.h Http.c
	$(CC) $(FLAGS) -c Http.c

Smtp.o: Smtp.h Smtp.c
	$(CC) $(FLAGS) -c Smtp.c

Stream.o: Stream.h Stream.c
	$(CC) $(FLAGS) -c Stream.c

Unicode.o: Unicode.c Unicode.h
	$(CC) $(FLAGS) -c Unicode.c

Terminal.o: Terminal.h Terminal.c
	$(CC) $(FLAGS) -c Terminal.c

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

Markup.o: Markup.h Markup.c
	$(CC) $(FLAGS) -c Markup.c

URL.o: URL.h URL.c
	$(CC) $(FLAGS) -c URL.c

DataParser.o: DataParser.h DataParser.c
	$(CC) $(FLAGS) -c DataParser.c

inet.o: inet.h inet.c
	$(CC) $(FLAGS) -c inet.c

Expect.o: Expect.h Expect.c
	$(CC) $(FLAGS) -c Expect.c

SecureMem.o: SecureMem.h SecureMem.c
	$(CC) $(FLAGS) -c SecureMem.c

GeneralFunctions.o: GeneralFunctions.h GeneralFunctions.c
	$(CC) $(FLAGS) -c GeneralFunctions.c

DataProcessing.o: DataProcessing.h DataProcessing.c
	$(CC) $(FLAGS) -c DataProcessing.c

Hash.o: Hash.h Hash.c
	$(CC) $(FLAGS) -c Hash.c

Ssh.o: Ssh.h Ssh.c
	$(CC) $(FLAGS) -c Ssh.c

Compression.o: Compression.h Compression.c
	$(CC) $(FLAGS) -c Compression.c

ConnectionChain.o: ConnectionChain.h ConnectionChain.c
	$(CC) $(FLAGS) -c ConnectionChain.c

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


#No dependancies, must always be compiled
LibSettings.o: LibSettings.h LibSettings.c
	$(CC) $(FLAGS) -c LibSettings.c

clean:
	-rm -f *.o *.so *.a
	-rm config.log config.status 
	-rm -r autom4te.cache config.cache

install:
	-mkdir -p $(prefix)/lib; cp *.so *.a $(prefix)/lib  
	-mkdir -p $(prefix)/include/libUseful-$(VERSION)
	cp *.h $(prefix)/include/libUseful-$(VERSION)
	-mkdir -p $(prefix)/include/libUseful-3
	cp *.h $(prefix)/include/libUseful-3

