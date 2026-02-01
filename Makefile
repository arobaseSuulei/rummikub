all:
	gcc main.c joueur.c Tuile.c manipulation.c menu.c -o main -lcjson
	./main