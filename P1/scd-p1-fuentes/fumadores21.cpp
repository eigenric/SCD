// Alumnno: Ricardo Ruiz Fernández de Alba - DNI: 77168601J

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

// Número de fumadores 
const int num_fumadores = 3 ;

Semaphore ingr_disp[num_fumadores] = {0, 0, 0}, // Semáforos que se bloquean hasta que el ingrediente i está disponible
          ingr_ret[num_fumadores] = {0, 0, 0}; // Semáforos que se bloquean hasta que el fumador i ha retirado su ingrediente. 

Semaphore msg(1); // Semáforo para garantizar la exclusión mutua entre mensajes por pantalla

//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   msg.sem_wait();
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;
   msg.sem_signal();

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;

   msg.sem_wait();
   // informa de que ha terminado de producir
   cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;
   msg.sem_signal();

   return num_ingrediente ;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
   while (true)
   {
      // Ingrediente X
      int ingreX = producir_ingrediente();

      // Ingrediente Y. Debe ser diferente a X.
      int ingreY = producir_ingrediente();
      while (ingreX == ingreY)
         ingreY = producir_ingrediente();

      cout << "....... ESTANQUERO HA PRODUCIDO LOS INGREDIENTES " << ingreX << " e " << ingreY << endl;
      cout << "puesto ingr. X: " << ingreX << endl;
      cout << "puesto ingr. Y: " << ingreY << endl;

      ingr_disp[ingreX].sem_signal(); // El ingrediente X está sobre el mostrador.
      ingr_disp[ingreY].sem_signal(); // El ingrediente Y está sobre el mostrador.

      ingr_ret[ingreX].sem_wait(); // Espera a que el primer fumador seleccionado recoja el ingrediente X
      ingr_ret[ingreY].sem_wait(); // Espera a que el segunda fumador seleccionado recoja el ingrediente Y
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
      ingr_disp[num_fumador].sem_wait(); // Esperar a que el ingrediente del fumador esté sobre el mostrador.
         msg.sem_wait();
         cout << "retirado ingr.: " << num_fumador << endl;
         msg.sem_signal();
      ingr_ret[num_fumador].sem_signal(); // Se señala que se ha retirado el ingrediente del fumador
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
   
   // Bastaría esperar a alguna hebra para que no termine el programa.
   hebra_estanquero.join();

   for (int i=0; i < num_fumadores; i++)
      hebra_fumador[i].join();

}
