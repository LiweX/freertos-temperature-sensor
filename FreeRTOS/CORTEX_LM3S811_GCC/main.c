/* Environment includes. */
#include "hw_include/DriverLib.h"
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


/* Delay between cycles of the 'check' task. */
#define mainCHECK_DELAY						( ( TickType_t ) 5000 / portTICK_PERIOD_MS )

/* UART configuration - note this does not use the FIFO so is not very
efficient. */
#define mainBAUD_RATE				( 19200 )
#define mainFIFO_SET				( 0x10 )

/* Misc. */
#define mainQUEUE_SIZE				( 3 )
#define SEED 420

int getRandomValue();
/*
 * Initialize the UART module.
 */
static void vUARTinit( void );

// typedef struct{
// 	int valuesArray[9];
// 	int index;
// 	float promedio;
// 	int N;
// }MyArray;

// void initArray(MyArray *array){
// 	array->index=0;
// 	array->promedio=0;
// 	array->N=9;
// }

// void addValue(MyArray *array,int value){
// 	array->valuesArray[array->index]=value;
// 	array->index++;
// 	if(array->index > (array->N - 1)) array->index=0;
// }

// void calcularPromedio(MyArray *array){
// 	int suma=0;
//     int i=0;
// 	do{
//         suma+=array->valuesArray[i];
//         i++;
//     }while(i<array->N-1);
// 	array->promedio=suma/array->N;
// }
// void setN(MyArray *array,int n){
// 	array->N=n;
// }


//TASKS
//static void vCounterTask( void *pvParameters );
static void vSensorTask(void *pvParameters);
static void vFilterTask(void *pvParameters);
static void vDisplayTask(void *pvParameters);


QueueHandle_t xSensorQueue;
QueueHandle_t xDisplayQueue;
//MyArray *array;
unsigned int rand_seed = SEED;
int N=1;
/*-----------------------------------------------------------*/

int main( void )
{
	vUARTinit();
	//initArray(array);
	xSensorQueue = xQueueCreate( mainQUEUE_SIZE, sizeof( int ) );
	xDisplayQueue = xQueueCreate( mainQUEUE_SIZE, sizeof( float ) );

	/* Start the tasks defined within the file. */
	//xTaskCreate( vCounterTask, "Counter", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	xTaskCreate( vSensorTask, "Sensor", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	xTaskCreate( vFilterTask, "Filter", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	//xTaskCreate( vDisplayTask, "Display", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	/* Start the scheduler. */
	vTaskStartScheduler();

	/* Will only get here if there was insufficient heap to start the
	scheduler. */

	return 0;
	
}
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

static void vUARTinit( void )
{
	SysCtlClockSet(0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	UARTConfigSet(UART0_BASE, mainBAUD_RATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
	UARTIntEnable(UART0_BASE, UART_INT_RX);
	IntPrioritySet(INT_UART0, configKERNEL_INTERRUPT_PRIORITY);
	IntEnable(INT_UART0);
	OSRAMInit(false);
}
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/
void vUART_ISR(void)
{
    uint32_t ui32Status;
    int n;
    char nchar;
    char string[4];
    string[0]='N';
    string[1]='=';
    string[3]='\0';
    // Obtener la causa de la interrupción UART
    ui32Status = UARTIntStatus(UART0_BASE, true);

    // Limpiar la bandera de interrupción
    UARTIntClear(UART0_BASE, ui32Status);

    // Procesar la interrupción de recepción UART
    if (ui32Status & (UART_INT_RX | UART_INT_RT)) {
        // Leer el carácter recibido
        nchar = UARTCharGet(UART0_BASE);
        n = nchar - 48;
        string[2]=nchar;
        if(n>0 && n<9){
            OSRAMStringDraw(string,78,0);
            N=n;
        }
	}
}
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/
void vSensorTask(void *pvParameters) {
	TickType_t xLastWakeTime;
    const TickType_t xDelay = pdMS_TO_TICKS(1000);
    // Inicializar xLastWakeTime con el tiempo actual
    xLastWakeTime = xTaskGetTickCount();
	int value = 30;
	char string[10];
    while (true) {
		if(getRandomValue()%2==0)value++;
		else value--;
		if(value<30)value=30;
		if(value>80)value=80;
        
        // Intentar enviar el mensaje a la cola
        if (xQueueSend(xSensorQueue, &value, portMAX_DELAY) != pdPASS) {
            // La cola está llena, realizar acciones de recuperación de errores si es necesario
        }
        // Retardo antes de enviar el siguiente mensaje para tener una frecuencia de 10Hz
        vTaskDelayUntil(&xLastWakeTime, xDelay);
    }
}

void vFilterTask(void *pvParameters){
	int value;
	int string[10];
    int valuesArray[9];
    int index=0;
    int promedio;
    //int N=1;
    int suma;
	while(true){
		if (xQueueReceive(xSensorQueue, &value, portMAX_DELAY) == pdPASS) {
            //enteroToString(value,string);
            //OSRAMClear();
            //OSRAMStringDraw(string,0,0);
			// addValue(array,value);
			// calcularPromedio(array);

            valuesArray[index]=value;
            index++;
            if(index>(N-1))index=0;
            suma=0;
            for(int i=0;i<N;i++)suma+=valuesArray[i];
            promedio=suma/N;
            enteroToString(promedio,string);
			//OSRAMClear();
			OSRAMStringDraw(string,0,0);			
			// xQueueSend(xDisplayQueue,&promedio,portMAX_DELAY);
        }
	}
}

// static void vDisplayTask(void *pvParameters){
// 	float temperature;
// 	char string[10];
// 	while(true){
// 		if (xQueueReceive(xDisplayQueue, &temperature, portMAX_DELAY) == pdPASS) {
// 			floatToString(temperature,string,2);
// 		}
// 		OSRAMClear();
// 		OSRAMStringDraw(string,0,0);
// 	}
// }

//algoritmo generador de números pseudoaleatorios conocido como "congruencia lineal"
int getRandomValue()
{
  rand_seed = rand_seed * 1103515245 + 12345;

  return (rand_seed / 131072) % 65536;
}



void enteroToString(int numero, char *cadena) {
    // Verificar si el número es negativo
    int esNegativo = 0;
    if (numero < 0) {
        esNegativo = 1;
        numero = -numero;
    }
    
    // Contador de dígitos
    int contador = 0;
    
    // Convertir dígitos a caracteres y almacenarlos en la cadena de caracteres
    do {
        cadena[contador++] = '0' + (numero % 10);
        numero /= 10;
    } while (numero > 0);
    
    // Agregar signo negativo si es necesario
    if (esNegativo) {
        cadena[contador++] = '-';
    }
    
    // Invertir la cadena de caracteres
    int i;
    for (i = 0; i < contador / 2; i++) {
        char temp = cadena[i];
        cadena[i] = cadena[contador - i - 1];
        cadena[contador - i - 1] = temp;
    }
    
    // Agregar el carácter nulo al final de la cadena
    cadena[contador] = '\0';
}

