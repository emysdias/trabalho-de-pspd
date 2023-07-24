#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

// #define SERVER_IP "127.0.0.1"
#define SERVER_IP "server-service"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int main()
{
  int client_socket;
  struct sockaddr_in server_address;
  char buffer[BUFFER_SIZE] = {0};

  // Criando o socket cliente
  if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Falha ao criar o socket cliente");
    exit(EXIT_FAILURE);
  }

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(SERVER_PORT);

  // Convertendo o endereço IP para o formato da rede
  if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0)
  {
    perror("Endereço inválido ou não suportado");
    exit(EXIT_FAILURE);
  }

  // Conectando ao servidor
  if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
  {
    perror("Falha na conexão");
    exit(EXIT_FAILURE);
  }

  printf("Conectado ao servidor em %s:%d\n", SERVER_IP, SERVER_PORT);

  while (1)
  {
    int num1, num2;

    printf("Digite dois números inteiros separados por espaço: ");
    scanf("%d %d", &num1, &num2);

    // Enviar os números para o servidor
    snprintf(buffer, sizeof(buffer), "%d %d", num1, num2);
    send(client_socket, buffer, strlen(buffer), 0);

    // Limpar o buffer para a resposta
    memset(buffer, 0, sizeof(buffer));
  }

  close(client_socket);

  return 0;
}
