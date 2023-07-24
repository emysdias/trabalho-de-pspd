from elasticsearch import Elasticsearch

# Dados de autenticação do Elasticsearch
usuario_elastic = "elastic"
senha_elastic = "Y6pK7W4r33F7R0rWd3B4ZZ9Z"

# Configuração do cliente Elasticsearch
es = Elasticsearch(
    ['https://localhost:9200'],
    http_auth=(usuario_elastic, senha_elastic),
    verify_certs=False
)


# Função para enviar os resultados para o Elasticsearch
def enviar_resultados_elasticsearch(tam, t1, t2, t3, t4):
    # Dados que você deseja enviar para o Elasticsearch
    dados = {
        "tam": tam,
        "t1": t1,
        "t2": t2,
        "t3": t3,
        "t4": t4
    }

    # Índice onde os dados serão armazenados
    indice = "resultado-jogo-vida"

    # Enviar os dados para o Elasticsearch
    res = es.index(index=indice, document=dados)

    # Verificar se o envio foi bem-sucedido
    if res["result"] == "created":
        print("Resultados enviados com sucesso para o Elasticsearch.")
    else:
        print("Erro ao enviar os resultados para o Elasticsearch.")

# Exemplo de uso da função
tam = 10
t1 = 1.234
t2 = 2.345
t3 = 3.456
t4 = 4.567
enviar_resultados_elasticsearch(tam, t1, t2, t3, t4)
