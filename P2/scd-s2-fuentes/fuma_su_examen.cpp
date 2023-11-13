// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Practica 2.
//
// Archivo: fumadores_su_examen.cpp
//
// Ejemplo de un monitor en C++11 con semántica SU, para el problema
// fuma_examen
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

constexpr int num_fumadores = 3;
int num_fumados[num_fumadores] = {0};

//**********************************************************************
// funciones comunes 
//----------------------------------------------------------------------

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

   num_fumados[num_fumador]++;

}
// *****************************************************************************
// clase para monitor buffer, version FIFO, semántica SC, multiples prod/cons

class Estanco : public HoareMonitor
{
 private:
   int mostrador;                       // Valor -1 si está vacío, 0,1,2 para los tres ingredientes.

   CondVar mostrador_vacio,             // Variable condición que bloquea el proceso cuando el mostrador está vacío
                                        // (mostrador == -1)
           esta_mi_ingred[num_fumadores]; // Array de variables condiciones. Hay una entrada para cada fumador

 public:                    // constructor y métodos públicos
   Estanco() ;              // constructor
   void ponerIngrediente(int ingre);  // Coloca el ingrediente ingre en el mostrador
   void esperarRecogidaIngrediente(); // Espera a que el ingrediente sea recogido (mostrador quede vacío)
   void obtenerIngrediente(int i);    // Retira el ingrediente i del mostrador
} ;
// -----------------------------------------------------------------------------

Estanco::Estanco()
{
   mostrador = -1; // Mostrador vacio
   mostrador_vacio = newCondVar();

   for (int i=0; i < num_fumadores; i++)
      esta_mi_ingred[i] = newCondVar();
}
// -----------------------------------------------------------------------------

// función que coloca el ingrediente en el mostrador

void Estanco::ponerIngrediente(int ingre)
{
   mostrador = ingre;

   esta_mi_ingred[ingre].signal();
}

// -----------------------------------------------------------------------------

// funcion espera la recogida del ingrediente (mostrador vacío)

void Estanco::esperarRecogidaIngrediente()
{
   if (mostrador != -1)
      mostrador_vacio.wait();
}

// -----------------------------------------------------------------------------

// funcion que retira el ingrediente i del mostrador.

void Estanco::obtenerIngrediente(int i)
{
   if (mostrador != i)
      esta_mi_ingred[i].wait();

   mostrador = -1;

   mostrador_vacio.signal();
}

class Hospital : public HoareMonitor
{
 private:
   int num_fumador_sanitaria; // Índice del fumador que ha fumado por 5ta vez

   CondVar sanitaria,    // Variable condición que bloquea a la hebra sanitaria hasta que hayan fumado 5 fumadores
           vicioso[num_fumadores];  // Variables condicion que bloquea al fumador i tras el aviso

 public:                    // constructor y métodos públicos
   Hospital() ;              // constructor
   void avisoFumar();  // Espera hasta que haya fumado 5 fumadores.s
   void intentaDespertarSanitaria(unsigned int i);  // Espera hasta que haya fumado 5 fumadores.s
   void reciboAvisoVicioso();  // El fumador recibe el aviso de la hebra sanitaria
} ;
// -----------------------------------------------------------------------------

Hospital::Hospital()
{
   sanitaria = newCondVar();
   
   for (int i=0; i < num_fumadores; i++)
      vicioso[i] = newCondVar();
}

// -----------------------------------------------------------------------------

void Hospital::avisoFumar()
{
   sanitaria.wait();
   cout << "FUMAR MATA: ya lo sabes, fumador " << num_fumador_sanitaria << endl;
   vicioso[num_fumador_sanitaria].wait();
}

void Hospital::intentaDespertarSanitaria(unsigned int i)
{
   if (num_fumados[i] == 5)
   {
      num_fumador_sanitaria = i;
      sanitaria.signal();
      reciboAvisoVicioso();
   }
}

void Hospital::reciboAvisoVicioso()
{
   vicioso[num_fumador_sanitaria].signal();
   cout << "Soy el fumador " << num_fumador_sanitaria << " y me han llamado vicioso" << endl;
}

// *****************************************************************************
// funciones de hebras

void estanquero( MRef<Estanco> monitor )
{
   while (true)
   {
      int ingre = producir_ingrediente();
      monitor->ponerIngrediente(ingre);
      monitor->esperarRecogidaIngrediente();
   }
}
// -----------------------------------------------------------------------------

void fumador(int i, MRef<Estanco>  estanco, MRef<Hospital> hospital)
{
   while (true)
   {
      estanco->obtenerIngrediente(i);
      fumar(i);
      hospital->intentaDespertarSanitaria(i);
   }
}
// -----------------------------------------------------------------------------

void funcion_hebra_sanitaria(MRef<Hospital> hospital)
{
   while (true)
   {
      hospital->avisoFumar();
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
   MRef<Estanco> estanco = Create<Estanco>();
   MRef<Hospital> hospital = Create<Hospital>();

   thread hebra_estanquero(estanquero, estanco),
          hebra_sanitaria(funcion_hebra_sanitaria, hospital);

   thread hebra_fumador[num_fumadores];

   for (int i=0; i < num_fumadores; i++)
      hebra_fumador[i] = thread(fumador, i, estanco, hospital);

   // Basta esperar a una hebra pues se ejecuta continuamente 
   hebra_estanquero.join();

   // for (int i=0; i < num_fumadores; i++)
   //    hebra_fumador[i].join();
}
