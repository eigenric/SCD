// Alumno: Ricardo Ruiz Fernandez de Alba - DNI: 77168601J

// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// Archivo: prodcons1_su.cpp
//
// Ejemplo de un monitor en C++11 con semántica SU, para el problema
// del productor/consumidor, con productor y consumidor únicos.
// Opcion LIFO
//
// Historial:
// Creado el 30 Sept de 2022. (adaptado de prodcons2_su.cpp)
// 20 oct 22 --> paso este archivo de FIFO a LIFO, para que se corresponda con lo que dicen las transparencias
// -----------------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

constexpr int
   num_items = 40,  // número de items a producir/consumir
   num_prod = 8,
   num_cons = 5,
   p = num_items / num_prod,
   c = num_items / num_cons;
int
   siguiente_dato = 0 ; // siguiente valor a devolver en 'producir_dato'
   
constexpr int               
   min_ms    = 5,     // tiempo minimo de espera en sleep_for
   max_ms    = 20 ;   // tiempo máximo de espera en sleep_for


mutex
   mtx ;                 // mutex de escritura en pantalla
unsigned
   cont_prod[num_items] = {0}, // contadores de verificación: producidos
   cont_cons[num_items] = {0}; // contadores de verificación: consumidos

unsigned int 
   producidos[num_prod] = {0}; // Array que almacena cuántos ha producido cada productor.
                               // Al final debe estar con todas entradas a p

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato(unsigned int i, unsigned int j)
{
   
   this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
   const int valor_producido = i*p + j;
   cont_prod[valor_producido]++ ;

   mtx.lock();
   cout << "hebra productora, produce " << valor_producido << " (hebra " << i << ")" << endl << flush ;
   mtx.unlock();

   // No es necesaria la exclusión mutua porque solo se accede por una hebra
   producidos[i]++;
   return valor_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned valor_consumir )
{
   if ( num_items <= valor_consumir )
   {
      cout << " valor a consumir === " << valor_consumir << ", num_items == " << num_items << endl ;
      assert( valor_consumir < num_items );
   }
   cont_cons[valor_consumir] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
   mtx.lock();
   cout << "                  hebra consumidora, consume: " << valor_consumir << endl ;
   mtx.unlock();
}
//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." << endl ;

   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      if ( cont_prod[i] != 1 )
      {
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
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

// *****************************************************************************
// clase para monitor buffer, version FIFO, semántica SC, multiples prod/cons

class Buffer : public HoareMonitor
{
 private:
 static const int            // constantes ('static' ya que no dependen de la instancia)
   num_celdas_total = 10;    //   núm. de entradas del buffer
 int                         // variables permanentes
   buffer[num_celdas_total], //   buffer de tamaño fijo, con los datos
   primera_libre,            //   indice de celda de la próxima inserción ( == número de celdas ocupadas)
   primera_ocupada,          //   índice de celda de la próxima extracción
   n;                        //   nº de elementos ne la cola

 CondVar                    // colas condicion:
   ocupadas,                //  cola donde espera el consumidor (n>0)
   libres ;                 //  cola donde espera el productor  (n<num_celdas_total)

 public:                    // constructor y métodos públicos
   Buffer() ;             // constructor
   int  leer();                // extraer un valor (sentencia L) (consumidor)
   void escribir( int valor ); // insertar un valor (sentencia E) (productor)
} ;
// -----------------------------------------------------------------------------

Buffer::Buffer(  )
{
   primera_libre = 0 ;
   primera_ocupada = 0;
   n = 0;
   ocupadas      = newCondVar();
   libres        = newCondVar();
}
// -----------------------------------------------------------------------------
// función llamada por el consumidor para extraer un dato

int Buffer::leer(  )
{
   // esperar bloqueado hasta que haya elementos
   if ( n == 0 )
      ocupadas.wait();

   //cout << "leer: ocup == " << primera_libre << ", total == " << num_celdas_total << endl ;
   assert( 0 < n  );

   // hacer la operación de lectura, actualizando estado del monitor
   const int valor = buffer[primera_ocupada];
   primera_ocupada = (primera_ocupada + 1) % num_celdas_total;
   n--;
   
   // señalar al productor que hay un hueco libre, por si está esperando
   libres.signal();

   // devolver valor
   return valor ;
}
// -----------------------------------------------------------------------------

void Buffer::escribir( int valor )
{
   // esperar bloqueado hasta que primera_libre < num_celdas_total
   if ( n == num_celdas_total )
      libres.wait();

   //cout << "escribir: ocup == " << primera_libre << ", total == " << num_celdas_total << endl ;
   assert( n < num_celdas_total );

   // hacer la operación de inserción, actualizando estado del monitor
   buffer[primera_libre] = valor ;
   primera_libre = (primera_libre + 1) % num_celdas_total;
   n++;

   // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
   ocupadas.signal();
}
// *****************************************************************************
// funciones de hebras

void funcion_hebra_productora( MRef<Buffer> monitor, unsigned int i)
{
   for( unsigned j = 0 ; j < p ; j++ )
   {
      int valor = producir_dato(i, j) ;
      monitor->escribir( valor );
   }
}
// -----------------------------------------------------------------------------

void funcion_hebra_consumidora( MRef<Buffer>  monitor, unsigned int i)
{
   for( unsigned j = 0 ; j < c ; j++ )
   {
      int valor = monitor->leer();
      consumir_dato( valor ) ;
   }
}
// -----------------------------------------------------------------------------

int main()
{
   cout << "----------------------------------------------------------------------------------------------" << endl
        << "Problema del productor-consumidor varios productores y consumidores (Monitor SU, buffer FIFO). " << endl
        << "-----------------------------------------------------------------------------------------------" << endl
        << flush ;

   thread hebra_productora[num_prod],
          hebra_consumidora[num_cons];

   // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
   MRef<Buffer> monitor = Create<Buffer>() ;

   for (unsigned int i = 0; i < num_prod; i++)
      hebra_productora[i] = thread(funcion_hebra_productora, monitor, i);
   
   for (unsigned int i=0; i < num_cons; i++)
      hebra_consumidora[i] = thread(funcion_hebra_consumidora, monitor, i);
   
   // esperar a que terminen las hebras
   for (int i=0; i < num_prod; i++)
      hebra_productora[i].join();

   for (int i=0; i < num_cons; i++)
      hebra_consumidora[i].join();   

   test_contadores() ;
}
