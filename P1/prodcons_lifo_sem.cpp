// Alumno: Ricardo Ruiz Fernández de Alba - DNI: 77168601J

// Enunciado en el simulacro:
//
//  1. 
//     Sube a la tarea un archivo llamado exactamente 'prodcons_lifo_sem.cpp' con tu solución 
//     LIFO al problema de un productor y un consumidor, con buffer intermedio, usando semáforos. 
//     Ten en cuenta que los semáforos usados sean estrictamente los necesarios.
//
// -----------------------------------------------------------------------------------
    

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
// variables compartidas, comunes a las dos soluciones (fifo y lifo)

const int num_items      = 40 ,        // número de items
	       tam_vector     = 10 ;        // tamaño del buffer
Semaphore puede_escribir = tam_vector, // semáforo compartido
	       puede_leer     = 0;          // semáforo compartido
mutex     mtx ;                 // para e.m. en escrituras en 'cout'
int       buffer[tam_vector] ;         // buffer intermedio (compartido)
unsigned  cont_prod[num_items] ={0},        // contadores de verificación: producidos
          cont_cons[num_items] ={0};       // contadores de verificación: consumidos


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

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato()
{
   static int contador = 0 ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   mtx.lock();
   cout << "producido: " << contador << endl << flush ;
   mtx.unlock();
   cont_prod[contador] ++ ;
   return contador++ ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   mtx.lock();
   cout << "                  consumido: " << dato << endl ;
   mtx.unlock();
}
//----------------------------------------------------------------------



//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}



//**********************************************************************
//**
//** solución LIFO
//**
//**********************************************************************

namespace solucion_lifo
{


unsigned primera_libre = 0 ;

//----------------------------------------------------------------------

void  funcion_hebra_productora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato = producir_dato() ;
      sem_wait( puede_escribir ) ;
      assert( primera_libre < tam_vector ) ; // debug
      mtx.lock();
         buffer[primera_libre] = dato ;
         primera_libre = primera_libre +1 ;
      mtx.unlock(); ;
      sem_signal( puede_leer ) ;
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato ;
      sem_wait( puede_leer );
      assert( primera_libre > 0 ) ;
      mtx.lock();
         primera_libre = primera_libre - 1 ;
         dato = buffer[primera_libre]  ;
      mtx.unlock();
      sem_signal( puede_escribir ) ;
      consumir_dato( dato ) ;
    }
}
//----------------------------------------------------------------------

void main()
{
   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO)." << endl
        << "--------------------------------------------------------" << endl
        << flush ;

   thread hebra_productora ( funcion_hebra_productora ),
          hebra_consumidora( funcion_hebra_consumidora );

   hebra_productora.join() ;
   hebra_consumidora.join() ;
}
//----------------------------------------------------------------------

} // fin namespace solución lifo


//**********************************************************************
//**
//** solución FIFO
//**
//**********************************************************************

namespace solucion_fifo
{

//----------------------------------------------------------------------

void  funcion_hebra_productora(  )
{
   static unsigned primera_libre = 0 ;

   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato = producir_dato() ;
      sem_wait( puede_escribir ) ;
         buffer[primera_libre] = dato ;
         primera_libre = (primera_libre + 1) % tam_vector ;
      sem_signal( puede_leer ) ;
    }
}

//----------------------------------------------------------------------

void  funcion_hebra_consumidora( )
{
   static unsigned primera_ocupada = 0 ;

   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato ;
      sem_wait( puede_leer );
         dato = buffer[primera_ocupada]  ;
         primera_ocupada = (primera_ocupada + 1) % tam_vector ;
      sem_signal( puede_escribir ) ;
      consumir_dato( dato ) ;
   }
}

//----------------------------------------------------------------------

void main()
{
   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución FIFO)." << endl
        << "--------------------------------------------------------" << endl
        << flush ;

	 thread hebra_productora ( funcion_hebra_productora ),
		 	 		hebra_consumidora( funcion_hebra_consumidora );

	 hebra_productora.join() ;
	 hebra_consumidora.join() ;
}

} // fin namespace solucion-fifo

// *****************************************************************************

int main()
{
   //solucion_lifo::main() ;
   solucion_fifo::main() ;
   test_contadores() ;
}
