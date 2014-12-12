#include <stdio.h>
#include <stdlib.h>

#include <semaphore.h>
#include <pthread.h>

//Semaforo mutex para exclusion mutua al acceder al buffer.
pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t datosLeidos_lock = PTHREAD_MUTEX_INITIALIZER;


//Semaforos que indican si el buffer tiene datos y sitio disponible.
sem_t hay_datos;
sem_t hay_sitio;


int Nhilos, Nnumeros, Tambuffer;

int datosLeidos = 0;

int *buffer;
int in = 0;
int out = 0;

// Funcion para introducir desde teclado.
int addInt(char message[]){
	printf("%s\n", message);
	int value;
	scanf("%d", &value);
	return value;
}

int esPrimo(int n){
	int a =0, i;
	for(i=1;i<(n+1);i++){  

		if(n%i==0){  
		a++;  
		}  
	}  
	if(a!=2){  
		return 0;  
	}else{  
		return 1;
	}  
}

void meteNumero(int value){
	pthread_mutex_lock(&buffer_lock);
	buffer[in] = value;
	in = (in + 1) % Tambuffer;
	pthread_mutex_unlock(&buffer_lock);
}

void sacaNumero(int *value, int *numD){
	pthread_mutex_lock(&buffer_lock);
	*value = buffer[out];
	out = (out + 1) % Tambuffer;
	datosLeidos++;
	*numD = datosLeidos;
	pthread_mutex_unlock(&buffer_lock);
}

void *productor(void *arg1){

	// Inicializamos el random
	srand ( time(NULL) );

	int i;
	for (i = 0; i < Nnumeros; i++) {

		//Esperamos en el caso de que el buffer este lleno
		sem_wait(&hay_sitio);

		meteNumero(rand() % 9999);

		//Avisamos de que hay un nuevo dato
		sem_post(&hay_datos);
	}

	pthread_exit(NULL);
}


void *consumidor(void* arg2){

	int idHilo = (int *) arg2;
	int num;
	int dL;

	while (datosLeidos != Nnumeros){

		//Esperamos en el caso de que no haya datos en el buffer.
		sem_wait(&hay_datos);

		sacaNumero(&num, &dL);

		//Avisamos de que hay un nuevo huevo en el buffer.
		sem_post(&hay_sitio);

		printf("%d    %d    %d    %d \n",num, idHilo, esPrimo(num), dL);

	}

	pthread_exit(NULL);



}

//
// Funcion principal
int main(){

	char introduceNhilos[] = "Introduzca la cantidad de hilos: ";
	char introduceNnumeros[] = "Introduzca la cantidad de numeros: ";
	char introduceTamBuffer[] = "Introduzca el tamano del buffer: ";

	if ( (Nhilos = addInt(introduceNhilos) ) <= 0) {
		printf("Error en el numero de hilos.\n");
		return 1;
	}

	if ( (Nnumeros = addInt(introduceNnumeros) ) <= 0) {
		printf("Error en la cantidad de numeros.\n");
		return 1;
	}

	if ( (Tambuffer = addInt(introduceTamBuffer) ) <= 0) {
		printf("Error el tamano del buffer.\n");
		return 1;
	}

	if ( (buffer=(int*)malloc(Tambuffer * sizeof(int))) == NULL) {
		printf("ERROR al reservar memoria\n");
		return 1;
	}


	//Declaracion de los procesos, tanto productor, como consumidores.
	pthread_t producer, consumer[Nhilos];


	//Inicializamos los semaforos
	//El semaforo hay_datos a 0 para indicar que el buffer esta vacio al principio.
	//El semaforo hay_sitio con el tamano del buffer, ya que estos son los huecos disponibles.
	sem_init(&hay_datos, 0, 0);
	sem_init(&hay_sitio, 0, Tambuffer);


	//Creacion del proceso productor.
	pthread_create(&producer, NULL, productor,(void *) NULL);

	//Creacion de los procesos consumidores.
	int i;
	for (i = 0 ; i< Nhilos; i++){
		pthread_create(&consumer[i], NULL, consumidor,(void*) i);
	}


	// Se espera a que los hilos terminen
	pthread_join(producer, NULL);

	for (i = 0 ; i< Nhilos; i++){
		pthread_join(consumer[i], NULL);
	}

}