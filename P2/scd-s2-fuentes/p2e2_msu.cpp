// Alumno: Ricardo Ruiz Fernandez de Alba - DNI: 77168601J

// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Practica 2. Problema de los fumadores con Monitores
//
// Archivo: fumadores_su.cpp
//
// Ejemplo de un monitor en C++11 con semántica SU, para el problema
// de los fumadores
//
// -----------------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

const int num_fumadores = 3,
          num_estanqueros = 2;

Semaphore msg(1);

//**********************************************************************
// funciones comunes 
//----------------------------------------------------------------------

int producir_ingrediente(int i)
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10, 100>() );

   msg.sem_wait();
   // informa de que comienza a producir
   cout << "Estanquero " << i << ": empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;
   msg.sem_signal();

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   int num_ingrediente = aleatorio<0, num_fumadores-1>();

   msg.sem_wait();
   // informa de que ha terminado de producir
   cout << "Estanquero " << i << ": termina de producir ingrediente " << num_ingrediente << endl;
   msg.sem_signal();

   return num_ingrediente ;
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
// *****************************************************************************
// clase para monitor buffer, version FIFO, semántica SC, multiples prod/cons

class Estanco : public HoareMonitor
{
 private:
   int mostrador[num_fumadores];      // Valor -1 si está vacío. Hasta 4 maxima

   CondVar mostrador_lleno[num_fumadores],    // Variable condición que bloquea el proceso cuando el mostrador está lleno
                                        // (mostrador == -1)
           esta_mi_ingred[num_fumadores]; // Array de variables condiciones. Hay una entrada para cada fumador

 public:                    // constructor y métodos públicos
   Estanco() ;              // constructor
   void ponerIngrediente(int ingre);  // Coloca el ingrediente ingre en el mostrador
   void esperarRecogidaIngrediente(); // Espera a que el ingrediente sea recogido (mostrador quede vacío)
   void obtenerIngrediente(int i);    // Retira el ingrediente i del mostrador
   void imprimirMostrador();
} ;
// -----------------------------------------------------------------------------

Estanco::Estanco()
{
   for (int i=0; i < num_fumadores; i++)
      mostrador[i] = 0; 

   for (int i=0; i < num_fumadores; i++)
      mostrador_lleno[i] = newCondVar();

   for (int i=0; i < num_fumadores; i++)
      esta_mi_ingred[i] = newCondVar();
}
// -----------------------------------------------------------------------------

// función que coloca el ingrediente en el mostrador

void Estanco::ponerIngrediente(int ingre)
{
   if (mostrador[ingre] == 4)
      mostrador_lleno[ingre].wait();
   
   mostrador[ingre]++;
   esta_mi_ingred[ingre].signal();
}

// -----------------------------------------------------------------------------


// funcion que retira el ingrediente i del mostrador.

void Estanco::obtenerIngrediente(int i)
{
   if (mostrador[i] == 0)
      esta_mi_ingred[i].wait();
   
   mostrador[i]--;
   mostrador_lleno[i].signal();
}

void Estanco::imprimirMostrador()
{
   cout << "Mostrador: [";
   for (int i=0; i < num_fumadores; i++)
   {
      cout << mostrador[i];
      if (i < num_fumadores -1)
         cout << ", ";
   }
   cout << "]" << endl;
}

// *****************************************************************************
// funciones de hebras

void estanquero(int i, MRef<Estanco> monitor )
{
   while (true)
   {
      int ingre = producir_ingrediente(i);
      monitor->ponerIngrediente(ingre);
//      monitor->imprimirMostrador();
   }
}
// -----------------------------------------------------------------------------

void fumador(int i, MRef<Estanco>  monitor )
{
   while (true)
   {
      monitor->obtenerIngrediente(i);
//      monitor->imprimirMostrador();
      fumar(i);
   }
}
// -----------------------------------------------------------------------------

int main()
{
   cout << "-------------------------------------------------" << endl
        << "Problema de los fumadores (Monitor SU). " << endl
        << "-------------------------------------------------" << endl
        << flush ;

   // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
   MRef<Estanco> monitor = Create<Estanco>();

   thread hebra_fumador[num_fumadores],
          hebra_estanquero[num_estanqueros];

   for (int i=0; i < num_fumadores; i++)
      hebra_fumador[i] = thread(fumador, i, monitor);

   for (int i=0; i < num_estanqueros; i++)
      hebra_estanquero[i] = thread(estanquero, i, monitor);

   for (int i=0; i < num_fumadores; i++)
      hebra_fumador[i].join();

   for (int i=0; i < num_estanqueros; i++)
      hebra_estanquero[i].join();

}
