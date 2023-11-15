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

Semaphore mostr_vacio(1),    // Semáforo que se bloquea cuando el mostrador está vacío
          ingr_disp[num_fumadores] = {0, 0, 0}; // Semáforo que indica si el ingrediente i está disponible

Semaphore msg(1);          // Semáforo para garantizar la exclusión mutua en la salida por pantalla

//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;

   // informa de que ha terminado de producir
   cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;

   return num_ingrediente ;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
   while (true)
   {
      int i = producir_ingrediente();
      mostr_vacio.sem_wait(); // Esperar a que el mostrador esté vacío
         cout << "puesto ingr.: " << i << endl;
      ingr_disp[i].sem_signal(); // El ingrediente i está sobre el mostrador.
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
      ingr_disp[num_fumador].sem_wait(); // Esperar a que el ingrediente i esté sobre el mostrador.
         cout << "retirado ingr.: " << num_fumador << endl;
      mostr_vacio.sem_signal(); // El mostrador ha quedado vacío tras retirar el fumador su ingrediente.
      fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
   // declarar hebras y ponerlas en marcha
   thread hebra_estanquero(funcion_hebra_estanquero),
          hebra_fumador[num_fumadores];
   
   for (int i=0; i < num_fumadores; i++)
      hebra_fumador[i] = thread(funcion_hebra_fumador, i);
   
   // Basta esperar a algun hebras para que no termine el programa.
   hebra_estanquero.join();

   for (int i=0; i < num_fumadores; i++)
      hebra_fumador[i].join();

}
