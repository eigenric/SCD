#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

// numero de fumadores 

const int num_fumadores = 3 ;
Semaphore mostr_vacio(1), // Semáforo que toma 1 cuando el mostrador está vacío y 0 en caso contrario.
          ingr_disp[3] = {0, 0, 0}; // Semáforo que indica si el ingrediente i está disponible


Semaphore enviar_sobre(3),  // Semáforo para enviar sobres del buzón. Puede meter como mucho 3 sobres. 1 cigarro por cada fumador
          retirar_sobre(0);  // Semáforo para retirar sobres del buzón. Esperar a que haya al menos un sobre para retirar.

int buzon[num_fumadores] = {-1, -1, -1}; // Buzón que utilizan los fumadores para enviar sobres de
                                      // cigarrillos liados al contrabandista

int primera_ocupada = 0,
    primera_libre = 0;

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

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

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

//      fumar(num_fumador);
      enviar_sobre.sem_wait();
         buzon[primera_libre] = num_fumador;
         primera_libre = (primera_libre +1) % num_fumadores;
         cout << "El fumador " << num_fumador << " envia sobre al contrabandista " << endl;
      retirar_sobre.sem_signal();
   }
}

void imprimir_sobres_fumador(int);

//----------------------------------------------------------------------
// funcion que ejecuta la hebra contrabandista
void funcion_hebra_contrabandista()
{
   int num_sobres_retirados = 0;
   int num_fumador;
   while (true)
   {
      this_thread::sleep_for(chrono::milliseconds(aleatorio<20,150>() ));

      // Espera a que haya sobres en el buzón
      retirar_sobre.sem_wait();

      num_fumador = buzon[primera_ocupada];
      primera_ocupada = (primera_ocupada + 1) % num_fumadores;
      cout << "El contrabandista recibe el sobre del fumador " << num_fumador << endl;
      num_sobres_retirados++;

      if (num_sobres_retirados % 4 == 0)
         imprimir_sobres_fumador(num_fumador);
      
      // Puede enviarse un sobre más en el buzón
      enviar_sobre.sem_signal();
   }
}

//----------------------------------------------------------------------

void imprimir_sobres_fumador(int num_ultimo_sobre)
{
   map<int,int> num_cigarrillos_por_fumador = {{0,0},{1,0},{2,0}};

   // Contamos cuantos cigarrillos aporta cada fumador
   for (int i=0; i < num_fumadores; i++) 
      num_cigarrillos_por_fumador[buzon[i]]++;
   
   // Añadimos la 4rta extraccion
   num_cigarrillos_por_fumador[num_ultimo_sobre]++;

   cout << "************ 4 nuevas extracciones del buzón: **********" << endl;
   for (int i=0; i < num_fumadores; i++)
      cout << "El fumador " << i << " aporta  " << num_cigarrillos_por_fumador[i] << " cigarro(s) al contrabandista" << endl;
}

//----------------------------------------------------------------------

int main()
{
   // declarar hebras y ponerlas en marcha
   thread hebra_estanquero(funcion_hebra_estanquero),
          hebra_contrabandista(funcion_hebra_contrabandista),
          hebra_fumador[num_fumadores];
   
   for (int i=0; i < num_fumadores; i++)
      hebra_fumador[i] = thread(funcion_hebra_fumador, i);
   
   // Basta esperar a algun hebras para que no termine el programa.
   hebra_estanquero.join();

// No es necesario esperar a los fumadores pues no terminan.s
//    for (int i=0; i < num_fumadores; i++)
//       hebra_fumador[i].join();

}
