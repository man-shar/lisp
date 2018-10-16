CC=cc

tut: tut_version.c mpc.c
	$(CC) -std=c99 -Wall tut_version.c mpc.c -ledit -lm -o tut

my: my_version.c mpc.c
	$(CC) -std=c99 -Wall my_version.c mpc.c -ledit -lm -o my