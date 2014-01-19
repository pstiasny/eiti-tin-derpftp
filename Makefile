
default: libs server tests


server: bin/derpftpd

bin/derpftpd: src/server.o src/types.h
	gcc -o bin/derpftpd src/server.o

src/server.o: src/server.c
	gcc -g -c -o src/server.o src/server.c


tests: bin/testclient

bin/testclient: src/testclient.o lib/libderpftp.a
	gcc -o bin/testclient -L./lib src/testclient.o -lderpftp

src/testclient.o: src/testclient.c include/fs_server.h
	gcc -g -c -o src/testclient.o src/testclient.c


libs: lib/libderpftp.a

lib/libderpftp.a: src/client.o
	ar ruv lib/libderpftp.a src/client.o
	ranlib lib/libderpftp.a

src/client.o: src/client.c src/types.h include/fs_server.h
	gcc -fPIC -g -c -Wall -o src/client.o src/client.c


clean:
	rm -f lib/* bin/* src/*.o


test:
	bash tests/test.sh

