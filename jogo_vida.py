#!/usr/bin/env python
import time
import sys
import requests
from pyspark import SparkContext
from elasticsearch import Elasticsearch
import urllib3

def ind2d(i, j, tam):
    return i * (tam + 2) + j

def wall_time():
    return time.time()

def UmaVida(args):
    (tabulIn, tabulOut, tam, i) = args
    for j in range(1, tam + 1):
        vizviv = (tabulIn[ind2d(i - 1, j - 1, tam)] + tabulIn[ind2d(i - 1, j, tam)] +
                  tabulIn[ind2d(i - 1, j + 1, tam)] + tabulIn[ind2d(i, j - 1, tam)] +
                  tabulIn[ind2d(i, j + 1, tam)] + tabulIn[ind2d(i + 1, j - 1, tam)] +
                  tabulIn[ind2d(i + 1, j, tam)] + tabulIn[ind2d(i + 1, j + 1, tam)])

        if tabulIn[ind2d(i, j, tam)] and vizviv < 2:
            tabulOut[ind2d(i, j, tam)] = 0
        elif tabulIn[ind2d(i, j, tam)] and vizviv > 3:
            tabulOut[ind2d(i, j, tam)] = 0
        elif not tabulIn[ind2d(i, j, tam)] and vizviv == 3:
            tabulOut[ind2d(i, j, tam)] = 1
        else:
            tabulOut[ind2d(i, j, tam)] = tabulIn[ind2d(i, j, tam)]

def InitTabul(tam):
    tabulIn = [0] * (tam + 2) * (tam + 2)
    tabulOut = [0] * (tam + 2) * (tam + 2)
    tabulIn[ind2d(1, 2, tam)] = 1
    tabulIn[ind2d(2, 3, tam)] = 1
    tabulIn[ind2d(3, 1, tam)] = 1
    tabulIn[ind2d(3, 2, tam)] = 1
    tabulIn[ind2d(3, 3, tam)] = 1
    return tabulIn, tabulOut

def Correto(tabul, tam):
    cnt = sum(tabul)
    return cnt == 5 and tabul[ind2d(tam - 2, tam - 1, tam)] and \
           tabul[ind2d(tam - 1, tam, tam)] and \
           tabul[ind2d(tam, tam - 2, tam)] and \
           tabul[ind2d(tam, tam - 1, tam)] and \
           tabul[ind2d(tam, tam, tam)]

def index_to_elasticsearch(es, index_name, tam, t1, t2, t3, t0):
    doc = {
      'tam': tam,
      'init': t1 - t0,
      'comp': t2 - t1,
      'fim': t3 - t2,
      'tot': t3 - t0
    }
    
    res = es.index(index=index_name, document=doc)

    if res["result"] == "created":
        print("Resultados enviados com sucesso para o Elasticsearch.")
    else:
        print("Erro ao enviar os resultados para o Elasticsearch.")

if __name__ == "__main__":
    sc = SparkContext(appName="GameOfLife")
    if len(sys.argv) < 3:
        print("Falha nos argumentos")
    powmin = int(sys.argv[1])
    powmax = int(sys.argv[2])
    print(f"Inteiros recebidos no Apache: {powmin} e {powmax}")

    urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)
    es = Elasticsearch(["https://localhost:9200"], basic_auth=('elastic', 'Y6pK7W4r33F7R0rWd3B4ZZ9Z'), verify_certs=False)

    for pow in range(powmin, powmax + 1):
        tam = 1 << pow

        t0 = wall_time()
        tabulIn, tabulOut = InitTabul(tam)
        t1 = wall_time()

        iterations = 2 * (tam - 3)
        for _ in range(iterations):
            # Broadcast the current tabulIn to all workers
            broadcasted_tabulIn = sc.broadcast(tabulIn)

            # Create an RDD for parallel computation
            rdd = sc.parallelize(range(1, tam + 1))

            # Perform the computation in parallel using the UmaVida function
            rdd.foreach(lambda i: UmaVida((broadcasted_tabulIn.value, tabulOut, tam, i)))

            # Swap tabulIn and tabulOut for the next iteration
            tabulIn, tabulOut = tabulOut, tabulIn

        t2 = wall_time()

        is_correct = Correto(tabulIn, tam)
        global_is_correct = sc.parallelize([is_correct]).reduce(lambda x, y: x and y)

        if sc.getConf().get('spark.driver.host') == 'localhost':
            if global_is_correct:
                print("**Ok, RESULTADO CORRETO**")
            else:
                print("**Nok, RESULTADO ERRADO**")

        # t3 calculation moved here
        t3 = wall_time()

        # Index data to Elasticsearch
        index_name = "resultado-jogo-vida"  # Change this to a suitable index name
        index_to_elasticsearch(es, index_name, tam, t1, t2, t3, t0)

        print("----------------------RESULTADO---------------------------")
        print("tam=%d; tempos: init=%7.7f, comp=%7.7f, fim=%7.7f, tot=%7.7f" %
              (tam, t1 - t0, t2 - t1, t3 - t2, t3 - t0))
        print("----------------------RESULTADO---------------------------\n\n")

    sc.stop()
