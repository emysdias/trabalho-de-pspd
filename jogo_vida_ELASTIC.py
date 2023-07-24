#!/usr/bin/env python
import time
import sys
from pyspark import SparkContext
from elasticsearch import Elasticsearch

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

def send_to_elasticsearch(data):
    # Connect to Elasticsearch
    es = Elasticsearch(['http://elasticsearch:9200'], http_auth=('a202028202', 'klyssmannoliveira'))

    # Define the Elasticsearch index and document type
    index = "jogo_vida_results"
    doc_type = "_doc"

    # Insert data into Elasticsearch
    es.index(index=index, doc_type=doc_type, body=data)

if __name__ == "__main__":
    sc = SparkContext(appName="GameOfLife")
    if len(sys.argv) < 3:
      print("Falha nos argumentos")
    powmin = int(sys.argv[1])
    powmax = int(sys.argv[2])
    print(f"Inteiros recebidos no Apache: {powmin} e {powmax}")
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

        t3 = wall_time()
        print("----------------------RESULTADO---------------------------")
        print("tam=%d; tempos: init=%7.7f, comp=%7.7f, fim=%7.7f, tot=%7.7f" %
              (tam, t1 - t0, t2 - t1, t3 - t2, t3 - t0))
        print("----------------------RESULTADO---------------------------\n\n")

        # Send results to Elasticsearch
        data = {
            "tam": tam,
            "init_time": t1 - t0,
            "comp_time": t2 - t1,
            "end_time": t3 - t2,
            "total_time": t3 - t0
        }
        send_to_elasticsearch(data)

    sc.stop()