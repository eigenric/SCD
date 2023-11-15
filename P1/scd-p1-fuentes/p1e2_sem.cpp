// Alumno: Ricardo Ruiz Fernández de Alba - DNI: 77168601J

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

// Constante para el número de fumadores
const int num_fumadores = 3 ;
const int num_estanqueros = 2;
   
         // Semáforo que se bloquea cuando no hay elementos del ingrediente 
Semaphore mostr_vacio[num_fumadores] = {0, 0, 0},
         // Semáforo que indica que hay 4 huecos disponibles para el ingrediente i
          ingr_huecos[num_fumadores] = {4, 4, 4};

Semaphore msg(1);          // Semáforo para garantizar la exclusión mutua en la salida por pantalla

// int raciones[num_fumadores] = {0, 0, 0};

// void imprimir_raciones()
// {
//    cout << "Raciones: [";
//    for (int i=0; i < num_fumadores; i++)
//    {
//       cout << raciones[i];
//       if (i < num_fumadores -1)
//          cout << ",";
//    }
//    cout << "]" << endl;
// }

//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente(int num_estanquero)
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   msg.sem_wait();
   cout << "Estanquero " << num_estanquero  << ": empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;
   msg.sem_signal();

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;

   // informa de que ha terminado de producir
   msg.sem_wait();
   cout << "Estanquero " << num_estanquero << ": termina de producir ingrediente " << num_ingrediente << endl;
   msg.sem_signal();

   return num_ingrediente ;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(int num_estanquero )
{
   while (true)
   {
      int i = producir_ingrediente(num_estanquero);
      // Ocupo un elemento más. El número de huecos disminuye en 1.
      // Si no hay huecos (4 ocupados) espera
      ingr_huecos[i].sem_wait(); 
         // msg.sem_wait();
         // raciones[i]++;
         // msg.sem_signal();
         // imprimir_raciones();
      cout << "puesto ingr.: " << i << endl;
      mostr_vacio[i].sem_signal(); // Nuevo elemento en el mostrador
   }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

   msg.sem_wait();
   cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
   msg.sem_signal();

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar
   msg.sem_wait();
    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
   msg.sem_signal();
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while (true)
   {
      // Espera a que el mostrador tenga elementos del fumador
      mostr_vacio[num_fumador].sem_wait();
         // msg.sem_wait();
         // raciones[num_fumador]--;
         // msg.sem_signal();       
         // imprimir_raciones();
         
      cout << "retirado ingr.: " << num_fumador << endl;
      // Retira un ingrediente disponible del mostrador. El número de huecos aumenta en 1 (max 4)
      ingr_huecos[num_fumador].sem_signal();
      fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
   // declarar hebras y ponerlas en marcha
   thread hebra_estanquero[num_estanqueros],
          hebra_fumador[num_fumadores];
   
   for (int i=0; i < num_fumadores; i++)
      hebra_fumador[i] = thread(funcion_hebra_fumador, i);

   for (int i=0; i < num_estanqueros; i++)
      hebra_estanquero[i] = thread(funcion_hebra_estanquero, i);

   for (int i=0; i < num_fumadores; i++)
      hebra_fumador[i].join();

   for (int i=0; i < num_estanqueros; i++)
      hebra_estanquero[i].join();
}
