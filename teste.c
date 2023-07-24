#include <stdio.h>
#include <curl/curl.h>

void send_to_elasticsearch(int tam, double t0, double t1, double t2, double t3)
{
  CURL *curl;
  CURLcode res;

  char url[1000];
  snprintf(url, sizeof(url), "https://localhost:9200/resultado-jogo-vida/_doc/1");

  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");

  char json_data[1000];
  snprintf(json_data, sizeof(json_data), "{\"tam_mpi\": %d, \"init_mpi\": %f, \"comp_mpi\": %f, \"fim_mpi\": %f, \"tot_mpi\": %f}",
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
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

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

int main()
{
  int tam = 8; // Tamanho do tabuleiro (valor fictício para o exemplo)
  double t0 = 10.0; // Tempo de início (valor fictício para o exemplo)
  double t1 = 20.0; // Tempo após a inicialização do tabuleiro (valor fictício para o exemplo)
  double t2 = 30.0; // Tempo após as iterações (valor fictício para o exemplo)
  double t3 = 40.0; // Tempo após a verificação dos resultados (valor fictício para o exemplo)

  send_to_elasticsearch(tam, t0, t1, t2, t3);

  return 0;
}
