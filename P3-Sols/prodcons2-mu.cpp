// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: prodcons2-mu.cpp
// Implementación del problema del productor-consumidor con
// un proceso intermedio que gestiona un buffer finito y recibe peticiones
// en orden arbitrario
// (versión con múltiples productores y consumidores)

//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------

#include <iostream>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <mpi.h>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int // constantes configurables
   tam_vector        = 10,
   num_items         = 20,
   num_productores   = 4,
   num_consumidores  = 5 ;

const int // etiquetas
   etiq_productores  = 0 ,
   etiq_consumidores = 1 ;

const int // constantes calculadas a partir de las anteriores:
   id_productor_min      = 0 ,
   id_productor_max      = id_productor_min + num_productores - 1,
   id_buffer             = id_productor_max + 1 ,
   id_consumidor_min     = id_buffer + 1 ,
   id_consumidor_max     = id_consumidor_min + num_consumidores - 1 ,
   num_procesos_esperado = id_consumidor_max + 1 ;



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
// ptoducir produce los numeros en secuencia (1,2,3,....)
// y lleva espera aleatorio
int producir( int num_productor ) // necesita número de productor, comenzando en 0
{
   static int contador = 0 ;
   sleep_for( milliseconds( aleatorio<10,100>()) );
   contador++ ;
   // devolver el siguiente número en el rango adecuado
   const int valor_producido = num_productor*(num_items/num_productores) + contador ;
   cout << "Productor " << num_productor << " ha producido valor " << valor_producido << endl << flush;
   return  valor_producido;
}
// ---------------------------------------------------------------------

void funcion_productor( int num_productor )
{
   for ( unsigned int i= 0 ; i < num_items/num_productores ; i++ )
   {
      // producir valor
      int valor_prod = producir( num_productor );
      // enviar valor
      cout << "Productor " << num_productor << " va a enviar valor " << valor_prod << endl << flush;
      MPI_Ssend( &valor_prod, 1, MPI_INT, id_buffer, etiq_productores, MPI_COMM_WORLD );

   }
}
// ---------------------------------------------------------------------

void consumir( int valor_cons, int num_consumidor )
{
   // espera bloqueada
   sleep_for( milliseconds( aleatorio<110,200>()) );
   cout << "Consumidor "<< num_consumidor << " ha consumido valor " << valor_cons << endl << flush ;
}
// ---------------------------------------------------------------------

void funcion_consumidor( int num_consumidor )
{
   int         peticion,
               valor_rec = 1 ;
   MPI_Status  estado ;

   for( unsigned int i=0 ; i < num_items/num_consumidores ; i++ )
   {
      MPI_Ssend( &peticion,  1, MPI_INT, id_buffer, etiq_consumidores, MPI_COMM_WORLD);
      MPI_Recv ( &valor_rec, 1, MPI_INT, id_buffer, etiq_consumidores, MPI_COMM_WORLD,&estado );
      cout << "Consumidor "<< num_consumidor << " ha recibido valor " << valor_rec << endl << flush ;
      consumir( valor_rec, num_consumidor );
   }
}
// ---------------------------------------------------------------------

void funcion_buffer()
{
   int        buffer[tam_vector],      // buffer con celdas ocupadas y vacías
              valor,                   // valor recibido o enviado
              primera_libre       = 0, // índice de primera celda libre
              primera_ocupada     = 0, // índice de primera celda ocupada
              num_celdas_ocupadas = 0, // número de celdas ocupadas
              etiq_aceptable ;         // etiqueta aceptable
   MPI_Status estado ;                 // metadatos del mensaje recibido

   for( unsigned int i=0 ; i < num_items*2 ; i++ )
   {
      // 1. determinar si puede enviar solo prod., solo cons, o todos

      if ( num_celdas_ocupadas == 0 )              // si buffer vacío
         etiq_aceptable = etiq_productores ;       //   solo prod.
      else if ( num_celdas_ocupadas == tam_vector )// si buffer lleno
         etiq_aceptable = etiq_consumidores ;      //   solo cons.
      else                                         // si no vacío ni lleno
         etiq_aceptable = MPI_ANY_TAG ;            //   cualquiera

      // 2. recibir un mensaje del emisor o emisores aceptables

      MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_aceptable, MPI_COMM_WORLD, &estado );

      // 3. procesar el mensaje recibido

      switch( estado.MPI_TAG ) // leer etiqueta del mensaje en metadatos
      {
         case etiq_productores: // si ha sido un productor: insertar en buffer
            buffer[primera_libre] = valor ;
            primera_libre = (primera_libre+1) % tam_vector ;
            num_celdas_ocupadas++ ;
            cout << "Buffer ha recibido valor " << valor << endl;
            break;

         case etiq_consumidores: // si ha sido un consumidor: extraer y enviarle
            valor = buffer[primera_ocupada] ;
            primera_ocupada = (primera_ocupada+1) % tam_vector ;
            num_celdas_ocupadas-- ;
            cout << "Buffer va a enviar valor " << valor << endl ;
            MPI_Ssend( &valor, 1, MPI_INT, estado.MPI_SOURCE, etiq_consumidores, MPI_COMM_WORLD);

            break;
      }
   }
}

// ---------------------------------------------------------------------

int main( int argc, char *argv[] )
{
   int id_propio, num_procesos_actual;

   // inicializar MPI, leer identif. de proceso y número de procesos
   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   if ( num_procesos_esperado == num_procesos_actual )
   {
      // ejecutar la operación apropiada a 'id_propio'
      if ( id_productor_min <= id_propio && id_propio <= id_productor_max )
         funcion_productor( id_propio - id_productor_min );
      else if ( id_consumidor_min <= id_propio && id_propio <= id_consumidor_max )
         funcion_consumidor( id_propio - id_consumidor_min );
      else // id_propio == id_buffer
         funcion_buffer();
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos_esperado << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   // al terminar el proceso, finalizar MPI
   MPI_Finalize( );
   return 0;
}
