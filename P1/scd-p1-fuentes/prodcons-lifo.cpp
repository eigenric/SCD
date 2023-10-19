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
   puede_acceder(1); // Semáforo para gestionar la exclusión mutua en la inserción y extracción

unsigned int buffer[tam_vec], // Buffer (Pila acotada LIFO) de tam_vec elementos
            primera_libre = 0; // Índice del buffer de la primera celda libre.

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

void mostrar_buffer()
{
    cout << "   buffer: [";
    for (int i=0; i < primera_libre; i++)
    {
      cout << buffer[i];
      if (i < tam_vec -1)
         cout << ","; 
    }
    cout << "]" << endl;
}

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
      libres.sem_wait(); // Producir tantos datos como elementos libres haya en el buffer

         puede_acceder.sem_wait(); // Inserción en exclusión mutua con la extracción

         // Inserción del dato
         buffer[primera_libre++] = dato;
         cout << "inserción en buffer: " << dato << endl;
         mostrar_buffer();

         puede_acceder.sem_signal();

      ocupadas.sem_signal(); // 
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
      ocupadas.sem_wait(); // Espera hasta que haya al menos un elemento en el buffer

         puede_acceder.sem_wait(); // La extraccion debe ocurrir en exclusion mutua con la inserción

         // Extracción del dato
         int indice = primera_libre;
         dato = buffer[--primera_libre];
         buffer[indice] = 0;
         cout << "extraído de buffer: " << dato << endl;
         mostrar_buffer();

         puede_acceder.sem_signal();
         
      libres.sem_signal(); // Se ha extraido un elemento del buffer, queda uno mas libre
      // Fin SC

      consumir_dato( dato ) ;
    }
}
//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO)." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

   thread hebra_productora ( funcion_hebra_productora ),
          hebra_consumidora( funcion_hebra_consumidora );

   hebra_productora.join() ;
   hebra_consumidora.join() ;
   cout << "fin" << endl;

   test_contadores();
}
