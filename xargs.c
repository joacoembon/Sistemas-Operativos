#include <stdio.h>     // Para printf, getline
#include <stdlib.h>    // Para exit, malloc, free
#include <string.h>    // Para strlen, strcpy
#include <unistd.h>    // Para fork, execvp
#include <sys/wait.h>  // Para wait

#ifndef NARGS
#define NARGS 4
#endif

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <command>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	char *command = argv[1];
	char *args[NARGS + 2];  // +2: espacio para el comando y NULL final
	args[0] = command;
	args[NARGS + 1] = NULL;

	char *line = NULL;
	size_t len = 0;
	ssize_t nread;
	int arg_count = 0;

	while ((nread = getline(&line, &len, stdin)) != -1) {
		// Eliminar el carácter de nueva línea al final de cada línea leída
		if (line[nread - 1] == '\n') {
			line[nread - 1] = '\0';
		}

		// Copiar la línea al arreglo de argumentos
		args[arg_count + 1] =
		        strdup(line);  // +1 porque args[0] es el comando
		arg_count++;

		if (arg_count == NARGS) {
			// Límite de argumentos alcanzado, ejecutar el comando
			pid_t pid = fork();
			if (pid == 0) {
				// Proceso hijo
				execvp(command, args);
				perror("execvp failed");
				exit(EXIT_FAILURE);
			} else if (pid > 0) {
				// Proceso padre
				wait(NULL);  // Esperar al proceso hijo
				arg_count =
				        0;  // Reiniciar el contador de argumentos
				for (int i = 1; i <= NARGS; i++) {
					free(args[i]);  // Liberar memoria para cada argumento
				}
			} else {
				perror("fork failed");
				exit(EXIT_FAILURE);
			}
		}
	}

	// Ejecutar el comando con los argumentos restantes, si hay
	if (arg_count > 0) {
		pid_t pid = fork();
		if (pid == 0) {
			execvp(command, args);
			perror("execvp failed");
			exit(EXIT_FAILURE);
		} else if (pid > 0) {
			wait(NULL);
			for (int i = 1; i <= arg_count; i++) {
				free(args[i]);
			}
		} else {
			perror("fork failed");
			exit(EXIT_FAILURE);
		}
	}

	free(line);
	return 0;
}
