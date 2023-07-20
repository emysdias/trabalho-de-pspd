#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 256

void process_client(int client_socket)
{
  int powmin, powmax;

  if (recv(client_socket, &powmin, sizeof(int), 0) < 0)
  {
    perror("Erro ao receber powmin do cliente");
    close(client_socket);
    return;
  }

  if (recv(client_socket, &powmax, sizeof(int), 0) < 0)
  {
    perror("Erro ao receber powmax do cliente");
    close(client_socket);
    return;
  }

  printf("Valores recebidos do cliente: powmin=%d, powmax=%d\n", powmin, powmax);

  if (send(client_socket, &powmin, sizeof(int), 0) < 0)
  {
    perror("Erro ao enviar powmin para o cliente");
    close(client_socket);
    return;
  }

  if (send(client_socket, &powmax, sizeof(int), 0) < 0)
  {
    perror("Erro ao enviar powmax para o cliente");
    close(client_socket);
    return;
  }
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    printf("Uso: %s <PORTA>\n", argv[0]);
    return 1;
  }

  int server_socket, client_socket;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  int port = atoi(argv[1]);

  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0)
  {
    perror("Erro ao criar o socket");
    return 1;
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    perror("Erro ao associar o socket à porta");
    close(server_socket);
    return 1;
  }

  if (listen(server_socket, MAX_CLIENTS) < 0)
  {
    perror("Erro ao iniciar o servidor");
    close(server_socket);
    return 1;
  }

  printf("Servidor iniciado. Aguardando conexões...\n");

  while (1)
  {
    client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_socket < 0)
    {
      perror("Erro ao aceitar a conexão");
      continue;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    printf("Cliente conectado: %s\n", client_ip);

    process_client(client_socket);
    close(client_socket);
  }

  close(server_socket);

  return 0;
}
