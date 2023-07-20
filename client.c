#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 256

int main(int argc, char **argv)
{
  if (argc != 5)
  {
    printf("Uso: %s <IP_SERVIDOR> <PORTA_SERVIDOR> <POWMIN> <POWMAX>\n", argv[0]);
    return 1;
  }

  char *server_ip = argv[1];
  int server_port = atoi(argv[2]);
  int powmin = atoi(argv[3]);
  int powmax = atoi(argv[4]);

  int client_socket;
  struct sockaddr_in server_addr;

  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket < 0)
  {
    perror("Erro ao criar o socket do cliente");
    return 1;
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(server_ip);
  server_addr.sin_port = htons(server_port);

  if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    perror("Erro ao conectar ao servidor");
    close(client_socket);
    return 1;
  }

  printf("Conexão estabelecida com o servidor %s:%d\n", server_ip, server_port);

  if (send(client_socket, &powmin, sizeof(int), 0) < 0)
  {
    perror("Erro ao enviar powmin para o servidor");
    close(client_socket);
    return 1;
  }

  if (send(client_socket, &powmax, sizeof(int), 0) < 0)
  {
    perror("Erro ao enviar powmax para o servidor");
    close(client_socket);
    return 1;
  }

  if (recv(client_socket, &powmin, sizeof(int), 0) < 0)
  {
    perror("Erro ao receber powmin do servidor");
    close(client_socket);
    return 1;
  }

  if (recv(client_socket, &powmax, sizeof(int), 0) < 0)
  {
    perror("Erro ao receber powmax do servidor");
    close(client_socket);
    return 1;
  }

  printf("Valores recebidos do servidor: powmin=%d, powmax=%d\n", powmin, powmax);
  close(client_socket);

  return 0;
}
