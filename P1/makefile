exec:
	echo "El primer descriptor al usar open será el 4, ya que se crea el .txt de errores de valgrind. Si se ejecuta el ./p1 despues del make esto no pasará."
	gcc -Wall -g -o p1 p1.c list.c
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-report.txt ./p1
