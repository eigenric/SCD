// Alumno: Ricardo Ruiz Fernández de Alba - DNI: 77168601J

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
	tam_vec   = 10 ,   // tamaño del buffer
   num_prod = 8 ,    // número de hebras productoras
   num_cons = 5,     // número de hebras consumidoras
   p = num_items / num_prod, // número de items producidos por cada productor
   c = num_items / num_cons;  // número de items consumidos por cada consumidor;
unsigned  
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   siguiente_dato       = 0 ;  // siguiente dato a producir en 'producir_dato' (solo se usa ahí)

Semaphore libres = tam_vec, // Array de semáforo que cuenta las posiciones libres del buffer para el productor i-ésimo
          ocupadas = 0; //Semáforo que cuenta las posiciones ocupadas del buffer

mutex mtx_lectura, mtx_escritura;

unsigned int buffer[tam_vec], // Buffer (Pila acotada LIFO) de tam_vec elementos
             producidos[num_prod] = {0}, // Array compartido que almacena el nº de elementos producidos por cada hebra productora
             primera_libre = 0,     // Posición de la primera celda libre en el buffer (FIFO)
             primera_ocupada = 0;   // Posición de la primera celda ocupada en el buffer (FIFO)
      ;

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato(unsigned int i, unsigned int j)
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   unsigned int dato_producido = i*p +j;
   cont_prod[dato_producido] ++ ;
   cout << "producido: " << dato_producido << " (hebra " << i << ")" << endl << flush ;
   producidos[i]++;
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato)
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
   cout << "comprobando contadores ...." << endl;
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
   cout << "comprobando nº producidos por hebra ...." << endl;
   for (unsigned i = 0; i < num_prod; i++)
   {
      if ( producidos[i] != p )
      {
         cout << "error: la hebra " << i << " no ha producido " << p << " veces" << endl;
         ok = false;
      }
   }

   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora( unsigned int i )
{
   assert(i < num_prod);
   for( unsigned j = 0; j < p; j++ )
   {
      int dato = producir_dato(i, j) ;
      
      libres.sem_wait();
         mtx_escritura.lock();
            buffer[primera_libre] = dato;
            primera_libre = (primera_libre + 1) % tam_vec;
            cout << "inserción en buffer: " << dato << " (hebra " << i << ")" << endl;
         mtx_escritura.unlock();
      ocupadas.sem_signal();
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora( unsigned int i )
{
   assert(i < num_cons);
   int dato;

   for( unsigned j = 0; j < c; j++ )
   {
      ocupadas.sem_wait();
         mtx_lectura.lock();
            dato = buffer[primera_ocupada];
            primera_ocupada = (primera_ocupada + 1) % tam_vec;
            cout << "extraído de buffer:  " << dato << " (hebra " << i << ")" << endl;
         mtx_lectura.unlock();
      libres.sem_signal();
      
      consumir_dato(dato);
   }
}
//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO)." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

   thread hebra_productora[num_prod],
          hebra_consumidora[num_cons];
   
   for (int i = 0; i < num_prod; i++)
      hebra_productora[i] = thread(funcion_hebra_productora, i);
   
   for (int i = 0; i < num_cons; i++)
      hebra_consumidora[i] = thread(funcion_hebra_consumidora, i);

   for (int i = 0; i < num_prod; i++)
      hebra_productora[i].join();
   
   for (int i = 0; i < num_cons; i++)
      hebra_consumidora[i].join();
   
   cout << "fin" << endl;

   test_contadores();
}
