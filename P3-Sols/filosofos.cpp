// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos.cpp
// Implementación del problema de los filósofos (sin camarero).
// Solución sin posibilidad de interbloqueo (los procesos cogen
// los tenedores en dos órdenes distintos)
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------

#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos         = 5 ,               // número de filósfos
   num_filo_ten          = 2*num_filosofos,  // número de filosofos y tenedores
   num_procesos_esperado = num_filo_ten;     // número de procesos total esperado


//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// ---------------------------------------------------------------------

void funcion_filosofos( int id_propio )
{
   const int
      id_tenedor_izq = (id_propio+1)              % num_filo_ten,
      id_tenedor_der = (id_propio+num_filo_ten-1) % num_filo_ten;

   int valor  ;

   const int
      id_tenedor_1 = (id_propio/2) & 1 ? id_tenedor_izq : id_tenedor_der , // primer tenedor
      id_tenedor_2 = (id_propio/2) & 1 ? id_tenedor_der : id_tenedor_izq ; // segundo tenedor

   while( true )
   {
      // Solicita tenedor primero
      cout << "Filósofo "<<id_propio<<" solicita 1er tenedor "<< id_tenedor_1 << endl << flush;
      MPI_Ssend( &valor, 1,MPI_INT, id_tenedor_1, 0, MPI_COMM_WORLD );

      // Solicita tenedor segundo
      cout << "Filósofo "<<id_propio<<" solicita 2o  tenedor "<< id_tenedor_2 << endl << flush;
      MPI_Ssend( &valor, 1,MPI_INT, id_tenedor_2, 0, MPI_COMM_WORLD );

      // commiendo (espera bloqueada)
      cout << "Filósofo " << id_propio << " está comiendo."<<endl<<flush;
      sleep_for( milliseconds( aleatorio<10,200>()) );

      // Suelta el tenedor primero
      MPI_Ssend( &valor, 1,MPI_INT, id_tenedor_1, 0, MPI_COMM_WORLD );
      cout <<"Filósofo "<<id_propio<< " ha soltado 1er tenedor " << id_tenedor_1 << endl << flush;

      // Suelta el tenedor segundo
      MPI_Ssend( &valor, 1,MPI_INT, id_tenedor_2, 0, MPI_COMM_WORLD );
      cout <<"Filósofo "<<id_propio<< " ha soltado 2o tenedor: " << id_tenedor_2 << endl << flush;

      // Piensa (espera bloqueada aleatorio del proceso)
      cout << "Filósofo " << id_propio << " está pensando." << endl << flush;
      sleep_for( milliseconds( aleatorio<10,200>()) );

   }
}
// ---------------------------------------------------------------------

void funcion_tenedores( int id_propio )
{

  MPI_Status estado ;
  int        id_filosofo, valor ;

  while( true )
  {
   // Recibe la peticion del filosofo ...
   MPI_Recv( &valor, 1,MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &estado );
   id_filosofo = estado.MPI_SOURCE ;
   cout << "Tenedor " << id_propio << " ha sido obtenido por filósofo " << id_filosofo << endl << flush;

    // Espera a que el filosofo suelte el tenedor...
   MPI_Recv( &valor, 1,MPI_INT, id_filosofo, 0, MPI_COMM_WORLD, &estado );
   cout << "Tenedor " << id_propio << " ha sido liberado por filósofo " << id_filosofo << endl << flush;
  }
}
// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   srand(time(0));
   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos_esperado == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if ( id_propio % 2 == 0 )
         funcion_filosofos( id_propio );
      else
         funcion_tenedores( id_propio );
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos_esperado << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------
