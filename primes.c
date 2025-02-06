#include <stdio.h>     // Para printf
#include <stdlib.h>    // Para exit, atoi
#include <unistd.h>    // Para fork, pipe, read, write, close
#include <sys/wait.h>  // Para wait


int
main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr,
		        "Uso: %s <número natural mayor o igual a 2>\n",
		        argv[0]);
		exit(1);
	}

	int n = atoi(argv[1]);  // Convertir el argumento a un número entero
	if (n < 2) {
		fprintf(stderr, "El número debe ser mayor o igual a 2.\n");
		exit(1);
	}

	// Crear el pipe inicial
	int left_pipe[2];
	if (pipe(left_pipe) == -1) {
		perror("pipe");
		exit(1);
	}

	// Crear el primer proceso hijo
	if (fork() == 0) {
		// Este es el proceso hijo
		close(left_pipe[1]);  // Cerrar el extremo de escritura del pipe
		create_filter(left_pipe);
		exit(0);  // Fin del proceso hijo
	}

	// Este es el proceso padre
	close(left_pipe[0]);  // Cerrar el extremo de lectura del pipe

	// Generar números del 2 al n y enviarlos al pipe
	for (int i = 2; i <= n; i++) {
		write(left_pipe[1], &i, sizeof(int));
	}

	close(left_pipe[1]);  // Cerrar el extremo de escritura del pipe

	// Esperar a que el proceso hijo termine
	wait(NULL);

	return 0;
}

void
create_filter(int left_pipe[2])
{
	int p;

	// Leer el primer número primo del pipe
	if (read(left_pipe[0], &p, sizeof(int)) <= 0) {
		close(left_pipe[0]);
		return;
	}

	printf("primo %d\n", p);

	// Crear un nuevo pipe para el siguiente proceso
	int right_pipe[2];
	if (pipe(right_pipe) == -1) {
		perror("pipe");
		exit(1);
	}

	if (fork() == 0) {
		// Proceso hijo: se convierte en el siguiente filtro
		close(left_pipe[0]);  // Cerrar el pipe del lado izquierdo ya que no se usará más
		close(right_pipe[1]);  // Cerrar el extremo de escritura del pipe derecho
		create_filter(right_pipe);
		exit(0);
	}

	close(right_pipe[0]);  // El proceso actual no necesita leer del pipe derecho

	// Leer del pipe izquierdo, filtrar y enviar al pipe derecho
	int num;
	while (read(left_pipe[0], &num, sizeof(int)) > 0) {
		if (num % p != 0) {
			write(right_pipe[1], &num, sizeof(int));
		}
	}

	close(left_pipe[0]);  // Cerrar el extremo de lectura del pipe izquierdo
	close(right_pipe[1]);  // Cerrar el extremo de escritura del pipe derecho

	// Esperar a que el proceso hijo termine
	wait(NULL);
}