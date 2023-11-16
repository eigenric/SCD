// Alumno: Ricardo Ruiz Fernández de Alba - DNI: 77168601J

#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

constexpr int num_fumadores = 3;

Semaphore msg(1); // Semáforo para garantizar exclusión mutua en las salidas por pantalla;

//**********************************************************************
// funciones comunes 
//----------------------------------------------------------------------

int producir_ingrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10, 100>() );

   // informa de que comienza a producir
   msg.sem_wait();
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;
   msg.sem_signal();

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   int num_ingrediente = aleatorio<0, num_fumadores-1>();

   // informa de que ha terminado de producir
   msg.sem_wait();
   cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;
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
   int mostrador[2];                   // En cada celda, valor -1 si está vacío, 0,1,2 para los tres ingredientes.

   CondVar mostrador_vacio,             // Variable condición que bloquea el proceso cuando el mostrador está totalmente vacío
                                        // es decir, las dos celdas del mostrador tienen valor -1.
           esta_mi_ingred[num_fumadores]; // Array de variables condiciones. 
                                          // Bloquea al fumador hasta que su ingrediente esta disponible

 public:                    // constructor y métodos públicos
   Estanco() ;              // constructor
   void ponerIngredientes(int ingreX, int ingrY);  // Coloca los ingredientes X e Y en el mostrador.
   void esperarRecogidaIngredientes(); // Espera a que el ingrediente sea recogido (mostrador quede vacío)
   void obtenerIngrediente(int i);    // Retira el ingrediente i del mostrador
} ;
// -----------------------------------------------------------------------------

Estanco::Estanco()
{
   // El mostrador está inicialmente vacío.
   mostrador[0] = -1;
   mostrador[1] = -1;
    
   mostrador_vacio = newCondVar();

   for (int i=0; i < num_fumadores; i++)
      esta_mi_ingred[i] = newCondVar();
}
// -----------------------------------------------------------------------------

// función que coloca el ingrediente en el mostrador

void Estanco::ponerIngredientes(int ingreX, int ingreY)
{
   mostrador[0] = ingreX;
   mostrador[1] = ingreY;

   cout << "puesto ingr. X: " << ingreX << endl;
   cout << "puesto ingr. Y: " << ingreY << endl;

   esta_mi_ingred[ingreX].signal();
   esta_mi_ingred[ingreY].signal();
}

// -----------------------------------------------------------------------------

// funcion espera la recogida del ingrediente (mostrador vacío)

void Estanco::esperarRecogidaIngredientes()
{
   if (mostrador[0] != -1 || mostrador[1] != -1)
      mostrador_vacio.wait();
}

// -----------------------------------------------------------------------------

// funcion que retira el ingrediente i del mostrador.

void Estanco::obtenerIngrediente(int i)
{
   // Si el ingrediente no está en ninguna celda, espera.
   if (mostrador[0] != i && mostrador[1] != i)
      esta_mi_ingred[i].wait();
   
   // El ingrediente está en alguna celda. La vacío
   if (mostrador[0] == i)
      mostrador[0] = -1;
   else if (mostrador[1] == i) 
      mostrador[1] = -1;
   
   cout << "retirado ingr.: " << i << endl;

   // Si el mostrador ha quedado totalmente vacío, lo señalo
   if (mostrador[0] == -1 && mostrador[1] == -1)
      mostrador_vacio.signal();
}


// *****************************************************************************
// funciones de hebras

void estanquero( MRef<Estanco> monitor )
{
   while (true)
   {
      int ingreX = producir_ingrediente();

      int ingreY = producir_ingrediente();
      while (ingreY == ingreX)
         ingreY = producir_ingrediente();

      cout << "....... ESTANQUERO HA PRODUCIDO LOS INGREDIENTES " << ingreX << " e " << ingreY << endl;

      monitor->ponerIngredientes(ingreX, ingreY);
      monitor->esperarRecogidaIngredientes();
   }
}
// -----------------------------------------------------------------------------

void fumador(int i, MRef<Estanco>  monitor )
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

   // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
   MRef<Estanco> monitor = Create<Estanco>();

   thread hebra_estanquero(estanquero, monitor);

   thread hebra_fumador[num_fumadores];

   for (int i=0; i < num_fumadores; i++)
      hebra_fumador[i] = thread(fumador, i, monitor);

   // Bastaría esperar a alguna hebra para que no termine el programa.
   hebra_estanquero.join();

   for (int i=0; i < num_fumadores; i++)
      hebra_fumador[i].join();
}
