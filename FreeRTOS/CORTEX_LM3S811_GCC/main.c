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
/*
 * Initialize the UART module.
 */
static void vUARTinit( void );

/*
 * The task that controls access to the LCD.
 */
static void vCounterTask( void *pvParameters );

/* String that is transmitted on the UART. */
static char *cMessage = "Task woken by button interrupt! --- ";
static volatile char *pcNextChar;


/* The queue used to send strings to the print task for display on the LCD. */
QueueHandle_t xPrintQueue;

/*-----------------------------------------------------------*/

int main( void )
{
	/* Configure the clocks, UART and GPIO. */
	vUARTinit();

	/* Create the queue used to pass message to vPrintTask. */
	//xPrintQueue = xQueueCreate( mainQUEUE_SIZE, sizeof( char * ) );

	/* Start the tasks defined within the file. */
	xTaskCreate( vCounterTask, "Counter", configMINIMAL_STACK_SIZE, NULL, 1, NULL );

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
    char cChar;

    // Obtener la causa de la interrupción UART
    ui32Status = UARTIntStatus(UART0_BASE, true);

    // Limpiar la bandera de interrupción
    UARTIntClear(UART0_BASE, ui32Status);

    // Procesar la interrupción de recepción UART
    if (ui32Status & (UART_INT_RX | UART_INT_RT)) {
        // Leer el carácter recibido
        cChar = UARTCharGet(UART0_BASE);

        // Imprimir el carácter en el display
        OSRAMClear();
		OSRAMStringDraw(&cChar,0,0);
	}
}
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

void vCounterTask(void *pvParameters) {
    TickType_t xLastWakeTime;
    const TickType_t xDelay = pdMS_TO_TICKS(10000);
    // Inicializar xLastWakeTime con el tiempo actual
    xLastWakeTime = xTaskGetTickCount();
	int counter = 0;
	char string[10];
    
    while (1) {
        // Realizar las acciones de la tarea
        counter++;
		enteroToString(counter,string);
		//OSRAMStringDraw(string,0,0);
        // Esperar 1 segundo desde la última ejecución
        vTaskDelayUntil(&xLastWakeTime, xDelay);
    }
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

