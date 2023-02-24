
static:
	ar rcs lib/static/libstatic.a obj/static/hello.o obj/static/linux.o obj/static/sum.o 
	gcc obj/static/main.o -Llib/static -lstatic -o bin/statically-linked

shared:
	gcc -shared obj/shared/hello.o obj/shared/linux.o obj/shared/sum.o -o lib/shared/libshared.a
	gcc obj/shared/main.o -Llib/shared -lshared -o bin/use-shared-library

objectstatic:
	gcc -c main.c -o obj/static/main.o -I./inc
	gcc -c src/hello.c -o obj/static/hello.o -I./inc
	gcc -c src/linux.c -o obj/static/linux.o -I./inc
	gcc -c src/sum.c -o obj/static/sum.o -I.inc

objectshared:
	gcc -c -fPIC main.c -o obj/shared/main.o -I./inc
	gcc -c -fPIC src/hello.c -o obj/shared/hello.o -I./inc
	gcc -c -fPIC src/linux.c -o obj/shared/linux.o -I./inc
	gcc -c -fPIC src/sum.c -o obj/shared/sum.o -I.inc

clnstatic:
	rm -rf obj/static/* bin/statically-linked /lib/static/*

clnshared:
	rm -rf obj/shared/* bin/use-shared-library /lib/shared/*

