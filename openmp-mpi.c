#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include <mpi.h>
#include <curl/curl.h> // Inclua a biblioteca libcurl

#define ind2d(i, j) (i) * (tam + 2) + j

double wall_time(void)
{
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  return (tv.tv_sec + tv.tv_usec / 1000000.0);
}

void UmaVida(int *tabulIn, int *tabulOut, int tam)
{
  int i, j, vizviv;
#pragma omp parallel for private(i, j, vizviv) shared(tabulIn, tabulOut) collapse(2)
  for (i = 1; i <= tam; i++)
  {
    for (j = 1; j <= tam; j++)
    {
      vizviv = tabulIn[ind2d(i - 1, j - 1)] + tabulIn[ind2d(i - 1, j)] +
               tabulIn[ind2d(i - 1, j + 1)] + tabulIn[ind2d(i, j - 1)] +
               tabulIn[ind2d(i, j + 1)] + tabulIn[ind2d(i + 1, j - 1)] +
               tabulIn[ind2d(i + 1, j)] + tabulIn[ind2d(i + 1, j + 1)];

      if (tabulIn[ind2d(i, j)] && vizviv < 2)
        tabulOut[ind2d(i, j)] = 0;
      else if (tabulIn[ind2d(i, j)] && vizviv > 3)
        tabulOut[ind2d(i, j)] = 0;
      else if (!tabulIn[ind2d(i, j)] && vizviv == 3)
        tabulOut[ind2d(i, j)] = 1;
      else
        tabulOut[ind2d(i, j)] = tabulIn[ind2d(i, j)];
    }
  }
}

void InitTabul(int *tabulIn, int *tabulOut, int tam)
{
  int ij;
  for (ij = 0; ij < (tam + 2) * (tam + 2); ij++)
  {
    tabulIn[ij] = 0;
    tabulOut[ij] = 0;
  }
  tabulIn[ind2d(1, 2)] = 1;
  tabulIn[ind2d(2, 3)] = 1;
  tabulIn[ind2d(3, 1)] = 1;
  tabulIn[ind2d(3, 2)] = 1;
  tabulIn[ind2d(3, 3)] = 1;
}

int Correto(int *tabul, int tam)
{
  int ij, cnt;
  cnt = 0;
  for (ij = 0; ij < (tam + 2) * (tam + 2); ij++)
    cnt = cnt + tabul[ij];
  return (cnt == 5 && tabul[ind2d(tam - 2, tam - 1)] &&
          tabul[ind2d(tam - 1, tam)] && tabul[ind2d(tam, tam - 2)] &&
          tabul[ind2d(tam, tam - 1)] && tabul[ind2d(tam, tam)]);
}

// Função para enviar os dados para o Elasticsearch usando libcurl
void send_to_elasticsearch(int tam, double t0, double t1, double t2, double t3)
{
  CURL *curl;
  CURLcode res;

  char url[1000];
  snprintf(url, sizeof(url), "https://localhost:9200/resultado-jogo-vida/_doc/1");

  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");

  char json_data[1000];
  snprintf(json_data, sizeof(json_data), "{\"arroz\": %d, \"init_mpi\": %f, \"comp_mpi\": %f, \"fim_mpi\": %f, \"tot_mpi\": %f}",
           tam, t1 - t0, t2 - t1, t3 - t2, t3 - t0);

  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();

  if (curl)
  {
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);

    // Desabilita a verificação do certificado SSL e verificação do nome do host (NÃO RECOMENDADO para produção)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    // Define o usuário e a senha para autenticação
    curl_easy_setopt(curl, CURLOPT_USERPWD, "elastic:Y6pK7W4r33F7R0rWd3B4ZZ9Z");

    // Define o método HTTP como PUT
    // curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
      fprintf(stderr, "Erro ao enviar os dados para o Elasticsearch: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
  }

  curl_slist_free_all(headers);
  curl_global_cleanup();
}

int main(int argc, char **argv)
{
  int pow, i, tam, *tabulIn, *tabulOut;
  double t0, t1, t2, t3;

  if (argc != 3)
  {
    printf("Uso: %s <POWMIN> <POWMAX>\n", argv[0]);
    return 1;
  }

  int powmin = atoi(argv[1]);
  int powmax = atoi(argv[2]);

  if (powmin < 0 || powmax < 0 || powmin > powmax)
  {
    printf("Valores inválidos para POWMIN e POWMAX.\n");
    return 1;
  }

  MPI_Init(&argc, &argv);
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // para todos os tamanhos do tabuleiro
  for (pow = powmin; pow <= powmax; pow++)
  {
    tam = 1 << pow;

    // aloca e inicializa tabuleiros
    t0 = wall_time();
    tabulIn = (int *)malloc((tam + 2) * (tam + 2) * sizeof(int));
    tabulOut = (int *)malloc((tam + 2) * (tam + 2) * sizeof(int));
    InitTabul(tabulIn, tabulOut, tam);
    t1 = wall_time();

    for (i = 0; i < 2 * (tam - 3); i++)
    {
      UmaVida(tabulIn, tabulOut, tam);
      UmaVida(tabulOut, tabulIn, tam);
    }

    t2 = wall_time();

    int is_correct = Correto(tabulIn, tam);
    int global_is_correct;
    MPI_Allreduce(&is_correct, &global_is_correct, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);

    if (rank == 0)
    {
      if (global_is_correct)
        printf("**Ok, RESULTADO CORRETO**\n");
      else
        printf("**Nok, RESULTADO ERRADO**\n");
    }

    t3 = wall_time();

    printf("tam=%d; tempos: init=%7.7f, comp=%7.7f, fim=%7.7f, tot=%7.7f \n",
           tam, t1 - t0, t2 - t1, t3 - t2, t3 - t0);

    // Envio de dados para o Elasticsearch
    send_to_elasticsearch(tam, t0, t1, t2, t3);

    free(tabulIn);
    free(tabulOut);
  }

  MPI_Finalize();
  return 0;
}
