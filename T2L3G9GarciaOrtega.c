#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
//Semáforo mutex para exclusión mutua al acceder al buffer.
pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t datosLeidos_lock = PTHREAD_MUTEX_INITIALIZER;


//Semáforos que indican si el buffer tiene datos y sitio disponible.
sem_t hay_datos;
sem_t hay_sitio;

//Declaraciones de variables
int Nhilos, Nnumeros, Tambuffer;

int datosLeidos = 0;

int *buffer;
int in = 0;
int out = 0;

//Cálculo de la primalidad del número generado.
int esPrimo(int num){
	int i ;
	int sq = ( int ) sqrt ( num );
	
	for (i = 2 ; i <= sq ; i++ ) {
		if ( num % i == 0 ) { 
    		return 0;
    	}
	}
    return  1;
}
//Función que añade un número al buffer, asegurándose previamente la exclusión mutua.
void meteNumero(int value){
	pthread_mutex_lock(&buffer_lock);
	buffer[in] = value;
	in = (in + 1) % Tambuffer;
	pthread_mutex_unlock(&buffer_lock);
}
//Función que extrae un número del buffer, asegurándose previamente la exclusión mutua.
void sacaNumero(int *value){
	pthread_mutex_lock(&buffer_lock);
	*value = buffer[out];
	out = (out + 1) % Tambuffer;
	pthread_mutex_unlock(&buffer_lock);
}
//Función que comprueba si quedan o no datos que leer en el buffer.
int quedanDatos(int *dato){
	
	int a = 0;

	if (datosLeidos < Nnumeros){
		a = 1;
		pthread_mutex_lock(&datosLeidos_lock);
		*dato = datosLeidos;
		datosLeidos++;
		pthread_mutex_unlock(&datosLeidos_lock);
	}

	
	
	return a;
}
//Inicialización del hilo productor.
void *productor(void *arg1){

	//Inicializamos el random.
	srand ( time(NULL) );

	int i;
	for (i = 0; i < Nnumeros; i++) {

		//Esperamos en el caso de que el buffer este lleno.
		sem_wait(&hay_sitio);
		//Al llegar a este punto, hemos asegurado que hay al menos un hueco libre en el buffer.
		
		//Llamamos a la función meteNumero para que cree un dato y lo introduzca en el buffer.
		meteNumero(rand() % 9999);

		//Avisamos de que hay un nuevo dato.
		sem_post(&hay_datos);
	}

	pthread_exit(NULL);
}

//Inicialización de los hilos consumidores.
void *consumidor(void* arg2){

	int *idHilo = (int *) arg2;
	int num, idDato;
	char primo[3];
	while (quedanDatos(&idDato)){

		//Esperamos en el caso de que no haya datos en el buffer.
		sem_wait(&hay_datos);
		//Al llegar a este punto, estamos seguros de que en el buffer hay al menos un dato que leer.
		
		//Llamamos a la función sacaNumero.
		sacaNumero(&num);

		//Avisamos de que hay un nuevo hueco en el buffer.
		sem_post(&hay_sitio);
		if (esPrimo(num)){
			strcpy(primo, "si");
		} else{
			strcpy(primo, "no");
		}

		printf("Hilo numero %d : Ha sido consumido el %d valor generado. El numero  %d %s es primo. \n", *idHilo+1, idDato+1,num, primo);
	}

	pthread_exit(NULL);
}


//Programa principal.
int main(int argc, char *argv[]){
	
	//Validación del número de argumentos pasados al programa.
	if(argc != 4){
		printf("Error: numero de argumentos invalido.\n");
		return 1;
	}
	
	//Validación del valor del primer argumento introducido. Corresponde al número de hilos que se desean crear.		
	if ( (Nhilos = atoi(argv[1]) ) <= 0) {
		printf("Error: el numero de hilos debe ser mayor que cero.\n");
		return 1;
	}

	//Validación del valor del segundo argumento introducido. Corresponde a la cantidad de números que se desean crear.
	if ( (Nnumeros = atoi(argv[2]) ) <= 0) {
		printf("Error: la cantidad de numeros para analizar debe ser mayor que cero.\n");
		return 1;
	}

	//Las siguientes dos sentencias if realizar la validación del valor del tercer y último argumento introducido. Corresponde al tamaño del buffer.
	if ( (Tambuffer = atoi(argv[3]) ) <= 0) {
		printf("Error el tamano del buffer debe ser mayor que cero.\n");
		return 1;
	}
	
	
	if (Tambuffer > (Nnumeros / 2)){
		printf("Error: el tamano del buffer es demasiado grande.\n");
		return 1;

	}

	//Comprobación de la correcta reserva de memoria del buffer que servirá para almacenar los datos que se generen posteriormente.
	if ( (buffer=(int*)malloc(Tambuffer * sizeof(int))) == NULL) {
		printf("ERROR al reservar memoria\n");
		return 1;
	}


	//Declaracion de los procesos, tanto productor, como consumidores.
	pthread_t producer, consumer[Nhilos];


	//Inicializamos los semáforos.
	//El semáforo hay_datos a 0 para indicar que el buffer esta vacío al principio.
	//El semáforo hay_sitio con el tamaño del buffer, ya que éstos son los huecos disponibles.
	sem_init(&hay_datos, 0, 0);
	sem_init(&hay_sitio, 0, Tambuffer);


	//Creación del proceso productor.
	pthread_create(&producer, NULL, productor,(void *) NULL);

	//Creación de los procesos consumidores.
	int i;
	int id[Nhilos];
	for (i = 0; i < Nhilos; i++){
		id[i] = i;
		pthread_create(&consumer[i], NULL, consumidor,(void*) &id[i]);
	}

	//Se espera a que los hilos terminen.
	pthread_join(producer, NULL);

	for (i = 0 ; i< Nhilos; i++){
		pthread_join(consumer[i], NULL);
	}

}
