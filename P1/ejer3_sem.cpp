// Alumno: Ricardo Ruiz Fernández de Alba - DNI: 77168601J

// -----------------------------------------------------------------------------------
//
// Prácticas SCD 20-21 GIM-GIADE (profesor: Carlos Ureña)
// Simulacro de examen (Práctica 1)
// Archivo: ejer3_sem.cpp 
// Extension de prod-cons con una hebra 'contadora'
//
// Enunciado en el SIMULACRO:
//
//  3.
//
//   A continuación, copia el archivo 'ejer2_sem.cpp' en otro llamado exactamente 'ejer2_sem.cpp' , y extiende 
//   la solución según se indica a continuación. En primer lugar, debes de asegurarte que el número total 
//   de items  (num_items) es múltiplo de 5 (además del número de consumidores). Además, habrá 
//   una hebra nueva llamada 'contadora', esta hebra ejecuta un bucle de num_items/5 interaciones, en cada iteración 
//   da estos dos pasos: (a) espera bloqueada hasta que los consumidores hayan consumido (entre todos) exactamente 5 
//   nuevos valores y (b) imprime un  mensaje indicando que ya se han consumido 5 nuevos valores. En el mensaje 
//   debe de imprimir la suma de dichos valores.
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
   num_items         = 100 ,    // número de items
	tam_vector        = 20 ,     // tamaño del buffer
   num_consumidores  = 10 ;     // número de consumidores

unsigned      
   buffer[tam_vector],      // buffer intermedio (compartido)
   suma_par_c      = 0,     // suma parcial de valores consumidos
   cont_par_c      = 0,     // cuenta parcial de valores consumidos
   contador        = 0 ,    // siguiente valor a producir por el productor
   primera_libre   = 0 ,    // índice de la primera entrada libre en el buffer
   primera_ocupada = 0 ;    // índice de la primera entrada ocupada en el buffer

Semaphore 
   puede_escribir = tam_vector, // semáforo compartido
	puede_leer     = 0,          // semáforo compartido
   mutex_leer     = 1 ,         // semáforo para EM entre consumidores en lecturas (extracciones) del buffer.
   mutex_cont     = 1 ,         // semáforo para EM entre consumidores en accesos a contador y suma ('suma_par_c' y 'cont_par_c')
   consumidos_5_valores  = 0 ,  // semáforo donde la contadora espera que se consuman 5 valores
   contadora_ha_impreso  = 0 ;  // semáforo donde un consumidor espera a que la contadora imprima

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

      // esperar a que ningún consumidor esté accediendo a 'suma_par_c' y 'con_par_c'
      sem_wait( mutex_cont );  

         // contabiliar nuevo dato consumido
         suma_par_c += dato ;
         cont_par_c ++ ;
         
         // si es el quinto, 
         if ( cont_par_c == 5 ) 
         {  
            sem_signal( consumidos_5_valores ); // despertar a contadora
            sem_wait( contadora_ha_impreso );  // esperar a que haya impreso
            suma_par_c = 0 ;             // resetear suma parcial
            cont_par_c = 0 ;             // resetear contador parcial
         }
      sem_signal( mutex_cont ); // liberar EM
   }
}

//----------------------------------------------------------------------


void funcion_hebra_contadora()
{
   for( unsigned i = 0 ; i < num_items/5 ; i++ )
   {
      sem_wait( consumidos_5_valores );
      
         mtx.lock();
         cout << " hebra contadora, cuenta parcial es: " << cont_par_c << ", la suma parcial es: " << suma_par_c << endl ;
         mtx.unlock();

      sem_signal( contadora_ha_impreso );
   }
}

//----------------------------------------------------------------------

int main()
{
   cout 
      << "--------------------------------------------------------------------" << endl
      << "Problema de 1 prod - n cons. FIFO y hebra 'contadora', con semáforos" << endl
      << "--------------------------------------------------------------------" << endl
      << flush ;

	thread 
      hebra_productora ( funcion_hebra_productora ),
      hebra_contadora ( funcion_hebra_contadora ),
		hebra_consumidora[num_consumidores] ;

   for( int i = 0 ; i < num_consumidores ; i++ )
      hebra_consumidora[i] = thread( funcion_hebra_consumidora );

	hebra_productora.join() ;
   hebra_contadora.join() ;
   for( int i = 0 ; i < num_consumidores ; i++ )
	   hebra_consumidora[i].join() ;

   test_contadores();
}




