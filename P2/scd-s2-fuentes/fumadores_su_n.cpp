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

//**********************************************************************
// funciones comunes 
//----------------------------------------------------------------------

template <int num_fumadores>
int producir_ingrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10, 100>() );

   // informa de que comienza a producir
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   int num_ingrediente = aleatorio<0, num_fumadores-1>();

   // informa de que ha terminado de producir
   cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;


   return num_ingrediente ;
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}
// *****************************************************************************
// clase para monitor buffer, version FIFO, semántica SC, multiples prod/cons

template<int num_fumadores>
class Estanco : public HoareMonitor
{
 private:
   int mostrador;                   // Valor -1 si está vacío, 0,1,2 para los tres ingredientes.

   CondVar mostrador_vacio;       // Variable condición que bloquea el proceso cuando el mostrador está vacío
                                  // (mostrador == -1)

   vector<CondVar> esta_mi_ingred; // Vector de variable condiciones. Hay tantos como num_fumadores
                                  // Se bloquea la variable i cuando en el mostrador tiene el ingrediente i



 public:                    // constructor y métodos públicos
   Estanco() ;              // constructor
   void ponerIngrediente(int ingre);  // Coloca el ingrediente ingre en el mostrador
   void esperarRecogidaIngrediente(); // Espera a que el ingrediente sea recogido (mostrador quede vacío)
   void obtenerIngrediente(int i);    // Retira el ingrediente i del mostrador
   int getNumFumadores() const { return esta_mi_ingred.size(); };
} ;
// -----------------------------------------------------------------------------

template<int num_fumadores>
Estanco<num_fumadores>::Estanco()
{
   mostrador = -1; // Mostrador vacio
   mostrador_vacio = newCondVar();

   esta_mi_ingred.resize(num_fumadores);
   
   for (int i=0; i < esta_mi_ingred.size(); i++)
      esta_mi_ingred[i] = newCondVar();
}
// -----------------------------------------------------------------------------

// función que coloca el ingrediente en el mostrador

template<int num_fumadores>
void Estanco<num_fumadores>::ponerIngrediente(int ingre)
{
   mostrador = ingre;

   esta_mi_ingred[ingre].signal();
}

// -----------------------------------------------------------------------------

// funcion espera la recogida del ingrediente (mostrador vacío)

template<int num_fumadores>
void Estanco<num_fumadores>::esperarRecogidaIngrediente()
{
   if (mostrador != -1)
      mostrador_vacio.wait();
}

// -----------------------------------------------------------------------------

// funcion que retira el ingrediente i del mostrador.

template<int num_fumadores>
void Estanco<num_fumadores>::obtenerIngrediente(int i)
{
   if (mostrador != i)
      esta_mi_ingred[i].wait();
   
   mostrador = -1;

   mostrador_vacio.signal();
}


// *****************************************************************************
// funciones de hebras

template <int num_fumadores>
void estanquero( MRef<Estanco<num_fumadores>> monitor )
{
   while (true)
   {
      int ingre = producir_ingrediente<num_fumadores>();
      monitor->ponerIngrediente(ingre);
      monitor->esperarRecogidaIngrediente();
   }
}
// -----------------------------------------------------------------------------

template <int num_fumadores>
void fumador(int i, MRef<Estanco<num_fumadores>>  monitor )
{
   while (true)
   {
      monitor->obtenerIngrediente(i);
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

   constexpr int num_fumadores = 5;
   // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
   MRef<Estanco<num_fumadores>> monitor = Create<Estanco<num_fumadores>>();

   thread hebra_estanquero(estanquero<num_fumadores>, monitor);

   thread hebra_fumador[num_fumadores];

   for (int i=0; i < num_fumadores; i++)
      hebra_fumador[i] = thread(fumador<num_fumadores>, i, monitor);

   // esperar a que terminen las hebras
   hebra_estanquero.join();

   for (int i=0; i < num_fumadores; i++)
      hebra_fumador[i].join();
}
