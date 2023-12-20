// Alumno: Ricardo Ruiz Fernandez de Alba - DNI: 77168601J

// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 4. Implementación de sistemas de tiempo real.
//
// Archivo: ejecutivo2.cpp
// Implementación del primer ejemplo de ejecutivo cíclico:
//
//   Datos de las tareas:
//   ------------
//   Ta.  T    C
//   ------------
//   A  500  100
//   B  500  150
//   C  1000 200
//   D  2000 240
//  -------------
//
//  Planificación (con Ts == 500 ms)
//  *---------*----------*---------*--------*
//  | A B C   | A B D    | A B C   | A B    |
//  *---------*----------*---------*--------*

//  Pregunta 1: ¿Cuál es el mínimo tiempo de espera que queda 
// al final de las iteraciones del ciclo secundario con tu solución ?

//  Los tiempos de espera son
//  C1 (Ciclo secundario 1): 500 - (100+150+200) = 50ms
//  C2: 500 - (100 + 150 + 240) = 10ms
//  C3 = C1 = 50ms
//  C4 = 500 - (100+150) = 250ms

// El mínimo tiempo de espera es de 10ms

// Pregunta 2: 
// ¿Sería planificable si la tarea D tuviese un tiempo cómputo de 250 ms ?

// A pesar de que se produce un retraso en el segundo ciclo secundario
// porque A B D sumarían (100+150+250) = 500 tiempos de ejecución 
// Como el tercer ciclo secundario tiene 50ms de espera y el cuarto ciclo secundario 250ms
// El retraso Delta t debería ser mayor a 300ms para no ser planificable lo cual no
// se produce comprobando la ejecución.

// Historial:
// Creado en Diciembre de 2017
// -----------------------------------------------------------------------------

#include <string>
#include <iostream> // cout, cerr
#include <thread>
#include <chrono>   // utilidades de tiempo
#include <ratio>    // std::ratio_divide

using namespace std ;
using namespace std::chrono ;
using namespace std::this_thread ;

// tipo para duraciones en segundos y milisegundos, en coma flotante:
//typedef duration<float,ratio<1,1>>    seconds_f ;
typedef duration<float,ratio<1,1000>> milliseconds_f ;

// -----------------------------------------------------------------------------
// tarea genérica: duerme durante un intervalo de tiempo (de determinada duración)

void Tarea( const std::string & nombre, milliseconds tcomputo )
{
   cout << "   Comienza tarea " << nombre << " (C == " << tcomputo.count() << " ms.) ... " ;
   sleep_for( tcomputo );
   cout << "fin." << endl ;
}

// -----------------------------------------------------------------------------
// tareas concretas del problema:

void TareaA() { Tarea( "A", milliseconds(100) );  }
void TareaB() { Tarea( "B", milliseconds( 150) );  }
void TareaC() { Tarea( "C", milliseconds( 200) );  }
void TareaD() { Tarea( "D", milliseconds( 240) );  }

// -----------------------------------------------------------------------------
// implementación del ejecutivo cíclico:

int main( int argc, char *argv[] )
{
   // Ts = duración del ciclo secundario (en unidades de milisegundos, enteros)
   const milliseconds Ts_ms( 500 );

   // ini_sec = instante de inicio de la iteración actual del ciclo secundario
   time_point<steady_clock> ini_sec = steady_clock::now();

   while( true ) // ciclo principal
   {
      cout << endl
           << "---------------------------------------" << endl
           << "Comienza iteración del ciclo principal." << endl ;

      for( int i = 1 ; i <= 4 ; i++ ) // ciclo secundario (4 iteraciones)
      {
         cout << endl << "Comienza iteración " << i << " del ciclo secundario." << endl ;

         switch( i )
         {
            case 1 : TareaA(); TareaB(); TareaC();           break ;
            case 2 : TareaA(); TareaB(); TareaD();           break ;
            case 3 : TareaA(); TareaB(); TareaC();           break ;
            case 4 : TareaA(); TareaB();                     break ;
         }

         // calcular el siguiente instante de inicio del ciclo secundario
         ini_sec += Ts_ms ;

         // esperar hasta el inicio de la siguiente iteración del ciclo secundario
         sleep_until( ini_sec );
         
         time_point<steady_clock> instante_fin       = steady_clock::now() ;
         steady_clock::duration retraso             = instante_fin - ini_sec ;

         cout << "Se ha producido un retraso de " << milliseconds_f(retraso).count() << " ms" << " respecto instante final esperado:" << endl;
      }
   }
}
