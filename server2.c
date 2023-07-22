#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

#define BUFFER_SIZE 1024


struct ValuesArgs {
  int powmin;
  int powmax;
};

void *callOpenMp(void *arg) {
  struct ValuesArgs *valuesArgs = (struct ValuesArgs *)arg;
  printf("Chamando código do OpenMp e MPI\n");
  char comando[100];
  printf("%d %d\n", valuesArgs->powmin, valuesArgs->powmax);
  snprintf(comando, sizeof(comando), "./openmp_mpi %d %d", valuesArgs->powmin, valuesArgs->powmax);
  system(comando);
}

void *callApache(void *arg) {
  struct ValuesArgs *valuesArgs = (struct ValuesArgs *)arg;
  printf("Chamando codigo do Apache e Spark\n");
  char comando[100];
  snprintf(comando, sizeof(comando), "python ./jogo_vida.py %d %d", valuesArgs->powmin, valuesArgs->powmax);
  system(comando);
}

int main()
{
  int server_fd, new_socket;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  char buffer[BUFFER_SIZE] = {0};
  int powmin, powmax;
  pthread_t openmp_thread;
  pthread_t apache_thread;


  // Criando o socket servidor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
    perror("Falha ao criar o socket servidor");
    exit(EXIT_FAILURE);
  }

  // Definindo opções do socket
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
  {
    perror("Erro na configuração do socket");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(8080);

  // Vinculando o socket ao endereço e porta
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    perror("Falha no binding");
    exit(EXIT_FAILURE);
  }

  // Definindo o socket como passivo, pronto para aceitar conexões
  if (listen(server_fd, 5) < 0)
  {
    perror("Erro ao escutar");
    exit(EXIT_FAILURE);
  }

  printf("Servidor escutando na porta 8080...\n");

  int max_clients = 10;
  int client_sockets[max_clients];
  int i, activity, valread, sd;
  int nums[2] = {0};

  // Inicializando os descritores de clientes
  for (i = 0; i < max_clients; i++)
  {
    client_sockets[i] = 0;
  }

  while (1)
  {
    fd_set readfds;

    // Limpar o conjunto de descritores de arquivo
    FD_ZERO(&readfds);

    // Adicionar o socket do servidor ao conjunto
    FD_SET(server_fd, &readfds);
    int max_sd = server_fd;

    // Adicionar os sockets dos clientes ao conjunto
    for (i = 0; i < max_clients; i++)
    {
      sd = client_sockets[i];

      // Se o socket estiver válido, adicionar ao conjunto
      if (sd > 0)
      {
        FD_SET(sd, &readfds);
      }

      // Atualizar o descritor de arquivo máximo
      if (sd > max_sd)
      {
        max_sd = sd;
      }
    }

    // Esperar por atividade em algum socket
    activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

    if ((activity < 0) && (errno != EINTR))
    {
      printf("Erro na seleção de socket\n");
    }

    // Novo cliente se conectou
    if (FD_ISSET(server_fd, &readfds))
    {
      if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
      {
        perror("Erro na aceitação da conexão");
        exit(EXIT_FAILURE);
      }

      // Adicionando novo socket à lista de descritores de clientes
      for (i = 0; i < max_clients; i++)
      {
        if (client_sockets[i] == 0)
        {
          client_sockets[i] = new_socket;
          break;
        }
      }

      printf("Novo cliente conectado, socket fd: %d, IP: %s, Porta: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
    }

    // Tratando dados recebidos de clientes
    for (i = 0; i < max_clients; i++)
    {
      sd = client_sockets[i];

      if (FD_ISSET(sd, &readfds))
      {
        valread = recv(sd, buffer, BUFFER_SIZE, 0);

        if (valread <= 0)
        {
          // Cliente desconectado
          getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
          printf("Host desconectado, IP: %s, Porta: %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

          close(sd);
          client_sockets[i] = 0;
        }
        else
        {
          // Processar os números recebidos
          sscanf(buffer, "%d %d", &nums[0], &nums[1]);
          struct ValuesArgs valueArgs;
          valueArgs.powmin = nums[0];
          valueArgs.powmax = nums[1];

          printf("Cliente %d enviou: powmin=%d, powmax=%d\n", sd, valueArgs.powmin, valueArgs.powmax);
          int thread_1 = pthread_create(&openmp_thread, NULL, callOpenMp, (void *)&valueArgs);
          int thread_2 = pthread_create(&apache_thread, NULL, callApache, (void *)&valueArgs);

	  if (thread_1 != 0) {
            printf("Erro ao criar thread do OpenMp\n");
 	  }

  	  if (thread_2 != 0) {
            printf("Erro ao criar thread do Apache\n");
          }

// 	  pthread_join(openmp_thread, NULL);
  //	  pthread_join(apache_thread, NULL);

 	   // Calcular a soma
          int sum = nums[0] + nums[1];

          // Enviar a soma de volta para o cliente
          char response[BUFFER_SIZE] = {0};
          snprintf(response, sizeof(response), "Soma: %d", sum);
          send(sd, response, strlen(response), 0);

          memset(buffer, 0, sizeof(buffer));
        }
      }
    }
  }

  return 0;
}
