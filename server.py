import socket
import threading
import subprocess

BUFFER_SIZE = 1024

def call_openmp(valueArgs):
    print("Chamando código do OpenMp e MPI")
    comando = f"./openmp_mpi {valueArgs['powmin']} {valueArgs['powmax']}"
    subprocess.call(comando, shell=True)

def call_apache(valueArgs):
    print("Chamando código do Apache e Spark\n\n")
    comando = f"python3 ./jogo_vida.py {valueArgs['powmin']} {valueArgs['powmax']}"
    subprocess.call(comando, shell=True)

def handle_client(client_socket, address):
    print(f"Novo cliente conectado, IP: {address[0]}, Porta: {address[1]}")
    while True:
        data = client_socket.recv(BUFFER_SIZE)
        if not data:
            print(f"Host desconectado, IP: {address[0]}, Porta: {address[1]}")
            break

        nums = [int(num) for num in data.decode().split()]
        powmin, powmax = nums[0], nums[1]
        print(f"Cliente {address[0]}:{address[1]} enviou: powmin={powmin}, powmax={powmax}")

        valueArgs = {'powmin': powmin, 'powmax': powmax}

        openmp_thread = threading.Thread(target=call_openmp, args=(valueArgs,))
        apache_thread = threading.Thread(target=call_apache, args=(valueArgs,))

        openmp_thread.start()
        apache_thread.start()

        openmp_thread.join()
        apache_thread.join()

        print("\n\nEsperando cliente!!!!!!!!\n\n")
        sum_result = powmin + powmax

        response = f"Soma: {sum_result}"
        client_socket.send(response.encode())

    client_socket.close()

def main():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind(("0.0.0.0", 8080))
    server_socket.listen(5)

    print("Servidor escutando na porta 8080...")

    max_clients = 10
    client_sockets = [None] * max_clients

    while True:
        try:
            client_socket, address = server_socket.accept()

            for i in range(max_clients):
                if client_sockets[i] is None:
                    client_sockets[i] = client_socket
                    break

            client_thread = threading.Thread(target=handle_client, args=(client_socket, address))
            client_thread.start()
        except KeyboardInterrupt:
            print("Encerrando servidor...")
            server_socket.close()
            break

if __name__ == "__main__":
    main()
