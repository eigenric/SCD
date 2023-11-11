#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "scd.h"

using namespace std ;
using namespace scd ;

//**********************************************************************
// Variables globales

const unsigned 
   num_items = 40 ,   // número de items
	tam_vec   = 10 ;   // tamaño del buffer
unsigned  
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   siguiente_dato       = 0 ;  // siguiente dato a producir en 'producir_dato' (solo se usa ahí)

Semaphore
   libres(tam_vec), // Semáforo que cuenta las posiciones libres del buffer
   ocupadas(0), //Semáforo que cuenta las posiciones ocupadas del buffer
   impresora_ocupada(0), // Semáforo que bloquea la hebra impresora hasta que se produzca un multiplo de 5
   productor_ocupado(0); // Semáforo que bloquea la hebra productora tras produce multiplo de 5 hasta que 
                         // la impresora imprime el número de celdas ocupadas en el momento actual;


unsigned int
   buffer[tam_vec] = {0}, // Buffer (Cola circular FIFO) de tam_vec elementos
   primera_libre = 0, // Índice en el buffer de la primera celda libre
   primera_ocupada = 0, // Índice en el buffer de la primera celda ocupada
   n = 0;

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato()
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   const unsigned dato_producido = siguiente_dato ;
   siguiente_dato++ ;
   cont_prod[dato_producido] ++ ;
   cout << "producido: " << dato_producido << endl << flush ;
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;

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

      // Inicio SC
      libres.sem_wait(); // Producir tantos datos como elementos libres

         // Inserción del dato
         buffer[primera_libre] = dato;
         cout << "inserción en buffer: " << buffer[primera_libre] << endl;
         primera_libre = (primera_libre +1) % tam_vec;
         n++;

         if (dato % 5 == 0)
         {
            impresora_ocupada.sem_signal();
            productor_ocupado.sem_wait();
         }

      ocupadas.sem_signal();
      // Fin SC
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato ;

      // Inicio SC
      ocupadas.sem_wait(); // Consumir cuando haya al menos un elemento en el buffer

         // Extracción del dato. 
         dato = buffer[primera_ocupada];
         buffer[primera_ocupada] = 0;
         primera_ocupada = (primera_ocupada +1) % tam_vec;
         cout << "extraido de buffer: " << dato << endl;
         n--;

      libres.sem_signal(); // Se ha liberado un elemento del buffer.
      // Fin SC

      consumir_dato(dato) ;
    }
}
//----------------------------------------------------------------------

void funcion_hebra_impresora()
{
   for (int i=0; i < num_items / 5; i++)
   {
      impresora_ocupada.sem_wait();

      cout << "El buffer tiene " << n << " celdas ocupadas." << endl;

      productor_ocupado.sem_signal();
   }
}

//----------------------------------------------------------------------

int main()
{
   cout << "-------------------------------------------------------------------" << endl
        << "Problema de 1 productor 1 consumidor FIFO con Impresora (Semáforos)" << endl 
        << "-------------------------------------------------------------------" << endl
        << flush ;

   thread hebra_productora ( funcion_hebra_productora ),
          hebra_consumidora( funcion_hebra_consumidora ),
          hebra_impresora( funcion_hebra_impresora );

   hebra_productora.join() ;
   hebra_consumidora.join() ;
   hebra_impresora.join();
   cout << "fin" << endl;

   test_contadores();
}
