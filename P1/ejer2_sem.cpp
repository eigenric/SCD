// -----------------------------------------------------------------------------------
//
// Prácticas SCD 20-21 GIM-GIADE (profesor: Carlos Ureña)
// Simulacro de examen (Práctica 1)
// Archivo: ejer2_sem.cpp 
// Extension para múltiples consumidores de prod-cons LIFO con 1 prod. y 1 cons. 
//
// Enunciado en el simulacro:
//
// 2.
//   A continuación copia ese archivo en otro llamado exactamente 'ejer2_sem.cpp', y extiende la 
//   solución para contemplar un número arbitrario de consumidores. Dicho número vendrá dado por 
//   una constante C, de forma que para cambiar el número de consumidores bastará única y exclusivamente con cambiar 
//   el valor de C. Asegurate que el número de total de items a producir (num_items)
//   es múltiplo de C, y que cada consumidor consume exactamente num_items/C valores en total.
//   Los consumidores, en el mensaje que imprimen al consumir, escriben además el número de consumidor (entre 0 y C-1)
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

constexpr unsigned
   num_items         = 400 ,    // número de items
	tam_vector        = 20 ,     // tamaño del buffer
   num_consumidores  = 10 ;     // número de consumidores

unsigned      
   buffer[tam_vector],          // buffer intermedio (compartido)
   contador        = 0 ,        // siguiente valor a producir por el productor
   primera_libre   = 0 ,    // índice de la primera entrada libre en el buffer
   primera_ocupada = 0 ;    // índice de la primera entrada ocupada en el buffer

Semaphore 
   puede_escribir = tam_vector, // semáforo compartido
	puede_leer     = 0,          // semáforo compartido
   mutex_leer     = 1 ;         // semáforo para EM entre consumidores en lecturas del buffer.

std::mutex 
   mtx ;

unsigned  
   cont_prod[num_items] = {0},  // contadores de verificación: producidos
   cont_cons[num_items] = {0};  // contadores de verificación: consumidos

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
   this_thread::sleep_for( chrono::milliseconds( aleatorio<10,20>() ));
   cont_prod[contador] ++ ;
   mtx.lock() ;
   cout << "Produzco: " << contador << endl ;
   mtx.unlock();
   return contador++ ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<10,20>() ));
   mtx.lock() ;
   cout << "                             Consumido: " << dato << endl ;
   mtx.unlock() ;
}
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
//----------------------------------------------------------------------

void  funcion_hebra_productora(  )
{
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
   for( unsigned i = 0 ; i < num_items/num_consumidores ; i++ )
   {
      int dato ;
      sem_wait( puede_leer );
      sem_wait( mutex_leer );
         dato = buffer[primera_ocupada]  ;
         primera_ocupada = (primera_ocupada + 1) % tam_vector ;
      sem_signal( mutex_leer );
      sem_signal( puede_escribir ) ;

      consumir_dato( dato ) ;
   }
}
//----------------------------------------------------------------------

int main()
{
   cout 
      << "----------------------------------------------------------------" << endl
      << "Problema de los 1 productor - n consumidores FIFO con semáforos." << endl
      << "----------------------------------------------------------------" << endl
      << flush ;

	thread 
      hebra_productora ( funcion_hebra_productora ),
		hebra_consumidora[num_consumidores] ;

   for( int i = 0 ; i < num_consumidores ; i++ )
      hebra_consumidora[i] = thread( funcion_hebra_consumidora );

	hebra_productora.join() ;
   for( int i = 0 ; i < num_consumidores ; i++ )
	   hebra_consumidora[i].join() ;

   test_contadores();
}




