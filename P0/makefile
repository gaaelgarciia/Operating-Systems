exec:
	echo "El primer descriptor será al usar open será el 4, ya que se crea el .txt de errores de valgrind. Si se ejecuta el ./p0 despues del make esto no pasará."
	gcc -Wall -g -o p0 p0.c list.c
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-report.txt ./p0
