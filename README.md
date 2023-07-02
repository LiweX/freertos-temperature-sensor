# Laboratorio N°7 - FreeRTOS
## Introduction
El presente es un trabajo práctico de laboratorio cuyo objetivo es el de desarrollar un programa para un sistema operativo de tiempo real ([FreeRTOS](https://www.freertos.org/)) que implemente distintas tareas involucradas en la simulación de un sensor de temperatura. Todo esto simulando una placa de desarrollo Stellaris LM3S811 por medio de QEMU.
## Desarrollo
Se desarrollaron las siguientes tareas:
- Simulación de un sensor de temperatura
- Cálculo del promedio de la temperatura mediante un filtro de ventana
- Interrupcion por UART para cambiar el tamaño de la ventana
- Representacion numerica de los valores promediados en el display
- Recopilación de estadísticas de ejecución de las tareas mostrándolas por puerto serie

### 🟢 Simulador de sensor de temperatura
Una de las tareas simula el sensor de temperatura haciendo uso de un algoritmo conocido como "congruencia lineal" donde se generan numeros aleatoreos a partir de una seed. Si el valor es par, la temperatura aumenta un grado, caso contrario disminuye un grado. Usando delays se configuro una frecuencia 10Hz para generar estos valores. Se trabajó con un rango limitado de 0°C a 96°C. Los valores se envian por medio de una queue hacia la tarea del filtro.
### 🟢 Calculador de promedio
Los valores que llegan del sensor se van guardando en un arreglo de tamaño fijo de manera circular, es decir que cuando se ocupa el ultimo valor del arreglo, el proximo valor reemplazara al primer valor del arreglo.
De esta manera se toman los ultimos N valores de ese arreglo para calcular su promedio. N solo toma valores entre 1 y 9, valor que puede ser seteado por UART.
El resultado del promedio calculado se envia por otra queue hacia la tarea del Display que se encargara de representar el valor en pantalla.
### 🟢 Utilización de UART
La recepción de datos por `UART0` se hizo mediante interrupciones. Dicha interrupcion toma el valor que el usuario ingreso por teclado y si este valor se encuentra dentro de los limites establecidos (de 1 a 9) se cambia el valor de N y se envia por una queue a la tarea del display.
### 🟢 Display
Para la tarea del display se utiliza el primer renglon para indicar el valor de la temperatura obtenido del filtro y el valor de N que se esta utilizando para el calculo de promedio. El segundo renglon representa el valor de la tempreatura en forma de una barra que se alarga o se acorta segun dicho valor.
### 🟢 Recopilación de estadísticas
Para esta tarea se hizo uso de [vTaskGetRunTimeStats](https://www.freertos.org/a00021.html#vTaskGetRunTimeStats) que brinda las estadisticas solicitadas en una estructura que almacena datos útiles sobre las tareas, como el porcentaje de CPU utilizado y la cantidad de stack libre que le queda. Estas estadísticas son enviadas por `serial0`.