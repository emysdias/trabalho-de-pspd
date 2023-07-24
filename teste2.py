import sys
import time
import numpy as np
from multiprocessing import Pool, cpu_count

def ind2d(i, j, tam):
    return i * (tam + 2) + j

def wall_time():
    return time.time()

def uma_vida(args):
    tabulIn, tabulOut, tam, i = args
    for j in range(1, tam + 1):
        vizviv = (
            tabulIn[ind2d(i - 1, j - 1, tam)]
            + tabulIn[ind2d(i - 1, j, tam)]
            + tabulIn[ind2d(i - 1, j + 1, tam)]
            + tabulIn[ind2d(i, j - 1, tam)]
            + tabulIn[ind2d(i, j + 1, tam)]
            + tabulIn[ind2d(i + 1, j - 1, tam)]
            + tabulIn[ind2d(i + 1, j, tam)]
            + tabulIn[ind2d(i + 1, j + 1, tam)]
        )

        if tabulIn[ind2d(i, j, tam)] and vizviv < 2:
            tabulOut[ind2d(i, j, tam)] = 0
        elif tabulIn[ind2d(i, j, tam)] and vizviv > 3:
            tabulOut[ind2d(i, j, tam)] = 0
        elif not tabulIn[ind2d(i, j, tam)] and vizviv == 3:
            tabulOut[ind2d(i, j, tam)] = 1
        else:
            tabulOut[ind2d(i, j, tam)] = tabulIn[ind2d(i, j, tam)]

def init_tabul(tam):
    tabulIn = np.zeros((tam + 2) * (tam + 2), dtype=int)
    tabulOut = np.zeros((tam + 2) * (tam + 2), dtype=int)

    tabulIn[ind2d(1, 2, tam)] = 1
    tabulIn[ind2d(2, 3, tam)] = 1
    tabulIn[ind2d(3, 1, tam)] = 1
    tabulIn[ind2d(3, 2, tam)] = 1
    tabulIn[ind2d(3, 3, tam)] = 1

    return tabulIn, tabulOut

def correto(tabul, tam):
    cnt = np.sum(tabul)
    return (
        cnt == 5
        and tabul[ind2d(tam - 2, tam - 1, tam)]
        and tabul[ind2d(tam - 1, tam, tam)]
        and tabul[ind2d(tam, tam - 2, tam)]
        and tabul[ind2d(tam, tam - 1, tam)]
        and tabul[ind2d(tam, tam, tam)]
    )

def main():
    if len(sys.argv) < 3:
      print("Falha nos argumentos")
    powmin = int(sys.argv[1])
    powmax = int(sys.argv[2])
    print(f"Inteiros recebidos no Apache: {powmin} e {powmax}")

    if powmin < 0 or powmax < 0 or powmin > powmax:
        print("Invalid values for POWMIN and POWMAX.")
        return

    num_cpus = cpu_count()

    for pow in range(powmin, powmax + 1):
        tam = 1 << pow
        tabulIn, tabulOut = init_tabul(tam)

        t0 = wall_time()
        for i in range(0, 2 * (tam - 3)):
            args = [(tabulIn, tabulOut, tam, i) for i in range(1, tam + 1)]
            with Pool(num_cpus) as p:
                p.map(uma_vida, args)
            args = [(tabulOut, tabulIn, tam, i) for i in range(1, tam + 1)]
            with Pool(num_cpus) as p:
                p.map(uma_vida, args)
        t1 = wall_time()

        is_correct = correto(tabulIn, tam)

        if is_correct:
            print("**Ok, RESULTADO CORRETO**")
        else:
            print("**Nok, RESULTADO ERRADO**")

        t2 = wall_time()

        print(
            f"tam={tam}; tempos: init={t1 - t0:.7f}, comp={t2 - t1:.7f}, fim={t2 - t0:.7f}"
        )

if __name__ == "__main__":
    main()
