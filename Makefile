all:
	gcc main.c joueur.c Tuile.c manipulation.c menu.c table.c partie.c -o main -lcjson
	./main