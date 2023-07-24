# Trabalho de pspd

O objetivo deste projeto é construir uma aplicação que atenda aos requisitos de performance e elasticidade
para que ela possa se comportar como uma aplicação de larga escala (large-scale application).

## Grupo

Emily Dias Sousa - 18/0015966 <br>
Vitor Magalhães Lamego - 19/0020903 <br>
Klyssmann Henrique Ferreira De Oliveira - 20/2028202 <br>

## Configurar o kubernete e elastic

- O kubernete deve estar instalado local, alguns links para ajudar: [kubernetes](https://kubernetes.io/releases/download/), [kubeadm](https://kubernetes.io/pt-br/docs/setup/production-environment/tools/kubeadm/install-kubeadm/) e [elastic no kubernetes](https://www.elastic.co/guide/en/cloud-on-k8s/master/k8s-deploy-eck.html);

- Para rodar esse programa, é criado o index chamado 'resultado-jogo-vida';

- Caso queira rodar esse cluster, entre em contato com alguém do grupo. Pois ele está sendo usado local.

## Comandos para rodar

#### Abra 4 terminais

o primeiro rode:

```
kubectl port-forward service/quickstart-kb-http 5601
```

o segundo:

```
kubectl port-forward service/quickstart-es-http 9200
```

o terceiro, faça o clone do repositório e na pasta raiz e rode:

```
mpicc openmp-mpi.c -o openmp_mpi -lcurl -fopenmp
```

```
gcc client.c -o client
```

```
python3 server.py
```

no quarto (aqui pode ser aberto mais de um terminal para ter vários clientes):

```
./client
```

entre na porta https://localhost:5601/ , entre com o login criado por você e configure o dashboard para ver os dados. Os nossos ficaram assim: 

![Screenshot from 2023-07-24 00-46-44](https://github.com/coordene/site-coorlab/assets/52640974/93734194-9146-4187-8774-a7643687ad0c)
![Screenshot from 2023-07-24 00-46-52](https://github.com/coordene/site-coorlab/assets/52640974/fe97b554-084b-42ce-b79e-987ad0a8090b)
![Screenshot from 2023-07-24 00-47-04](https://github.com/coordene/site-coorlab/assets/52640974/bb1dc227-ca1b-4084-b7a0-fbd34d5320a8)
