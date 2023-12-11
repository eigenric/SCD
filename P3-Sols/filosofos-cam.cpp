// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-cam.cpp
// Implementación del problema de los filósofos, usando un proceso camarero.
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
   num_filosofos         = 5 ,
   id_camarero           = 2*num_filosofos ;
const int
   num_filo_ten          = 2*num_filosofos,
   num_procesos_esperado = 2*num_filosofos+1 ;
const int
   etiq_sentarse         = 0,
   etiq_levantarse       = 1;


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
      id_tenedor_izq = (id_propio+1             ) % num_filo_ten,
      id_tenedor_der = (id_propio+num_filo_ten-1) % num_filo_ten;

   int valor  ;

   while( true )
   {
      // Solicita sentarse al camarero
      cout << "Filósofo " << id_propio << " solicita sentarse." << endl << flush;
      MPI_Ssend( &valor, 1,MPI_INT, id_camarero, etiq_sentarse, MPI_COMM_WORLD );

      // Solicita tenedor primero
      cout << "Filósofo "<<id_propio<<" solicita 1er tenedor "<< id_tenedor_izq << endl << flush;
      MPI_Ssend( &valor, 1,MPI_INT, id_tenedor_izq, 0, MPI_COMM_WORLD );

      // Solicita tenedor segundo
      cout << "Filósofo "<<id_propio<<" solicita 2o  tenedor "<< id_tenedor_der << endl << flush;
      MPI_Ssend( &valor, 1,MPI_INT, id_tenedor_der, 0, MPI_COMM_WORLD );

      // comiendo (espera bloqueada)
      cout << "Filósofo " << id_propio << " está comiendo."<<endl<<flush;
      sleep_for( milliseconds( aleatorio<10,200>()) );

      // Suelta el tenedor primero
      MPI_Ssend( &valor, 1,MPI_INT, id_tenedor_izq, 0, MPI_COMM_WORLD );
      cout <<"Filósofo "<<id_propio<< " ha soltado 1er tenedor " << id_tenedor_izq << endl << flush;

      // Suelta el tenedor segundo
      MPI_Ssend( &valor, 1,MPI_INT, id_tenedor_der, 0, MPI_COMM_WORLD );
      cout <<"Filósofo "<<id_propio<< " ha soltado 2o tenedor: " << id_tenedor_der << endl << flush;

      // Solicita levantarse al camarero
      cout << "Filósofo " << id_propio << " solicita levantarse." << endl << flush;
      MPI_Ssend( &valor, 1,MPI_INT, id_camarero, etiq_levantarse, MPI_COMM_WORLD );

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

void funcion_camarero( int id_propio )
{

   MPI_Status estado ;
   int        valor, num_sentados = 0 ;

   while( true )
   {
      // Recibe un petición de cualquier filósofo
      const int etiq_aceptada = (num_sentados < 4) ? MPI_ANY_TAG : etiq_levantarse;
      MPI_Recv( &valor, 1,MPI_INT, MPI_ANY_SOURCE, etiq_aceptada, MPI_COMM_WORLD, &estado );

      // procesa la petición
      if ( estado.MPI_TAG == etiq_sentarse )
      {
         num_sentados ++ ;
         cout << "Camarero ha autorizado a sentarse a filósofo " << estado.MPI_SOURCE << " (ahora hay " << num_sentados << " filo. sentados)" << endl << flush ;
      }
      else
      {
         num_sentados -- ;
         cout << "Camarero ha autorizado a levantarse a filósofo " << estado.MPI_SOURCE << " (ahora hay " << num_sentados << " filo. sentados)" << endl << flush ;
      }
   }
}


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
      if ( id_propio == id_camarero )
         funcion_camarero( id_propio );
      else if ( id_propio % 2 == 0 )
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
