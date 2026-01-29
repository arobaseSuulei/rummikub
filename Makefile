all:
	gcc main.c joueur.c Tuile.c table.c menu.c manipulation.c -o main -lcjson -std=c11 -Wall
	./main