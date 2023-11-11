#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;


constexpr int               
   min_ms    = 5,     // tiempo minimo de espera en sleep_for
   max_ms    = 20 ;   // tiempo máximo de espera en sleep_for

mutex    mtx ;                 // mutex de escritura en pantalla

const int num_fum=3; // Número de hebras fumadoras


//-------------------------------------------------------------------------
int Producir_ingrediente()
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
   
   mtx.lock();
   cout << "Estanquero : EMPIEZA a producir ingrediente." << endl;
   mtx.unlock();

   const int ingrediente = aleatorio<0,num_fum-1>() ;

   mtx.lock();
   cout << "Estanquero : TERMINA de producir ingrediente " << ingrediente << endl;
   mtx.unlock();

   return ingrediente ;
}


//-------------------------------------------------------------------------
void Fumar( int num_fumador )
{
   mtx.lock();
   cout << "                                                Fumador " << num_fumador << ":"<< " EMPIEZA a fumar" << endl;
   mtx.unlock();
  
   this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
   
   mtx.lock();
   cout << "                                                Fumador " << num_fumador << " TERMINA DE FUMAR!!" << endl;
   mtx.unlock();
}

//----------------------------------------------------------------------


// *****************************************************************************
// clase para monitor Estanco para el problema de los fumadores
class Estanco : public HoareMonitor
{
 private:

 int  mostrador; 	     //   Estado del mostrador


 CondVar                    // colas condicion:
   mostrador_vacio,         //  cola donde espera el estanquero (mostrador==-1)
   esta_mi_ingred[num_fum];       //  cola donde espera cada fumador  (mostrador==i)


 public:                    // constructor y métodos públicos
   Estanco() ;             // constructor
   void ponerIngrediente( const int ingre );    
   void esperarRecogidaIngrediente(); 
   void obtenerIngrediente( const int  i );   

   
} ;
// -----------------------------------------------------------------------------

Estanco::Estanco( )
{
   mostrador=-1; // Mostrador inicialmente vacio
   mostrador_vacio   = newCondVar();
   for( unsigned i = 0 ; i < num_fum ; i++ )
        esta_mi_ingred[i] = newCondVar();



}



// -----------------------------------------------------------------------------
// función llamada por el consumidor para extraer un dato
void Estanco::ponerIngrediente( const int ingre )
   {
      mostrador=ingre;
      //(*  mostrador==ingre *)
      esta_mi_ingred[ingre].signal();

   }
   
 void Estanco::esperarRecogidaIngrediente()
  {    
      if (mostrador!=-1) mostrador_vacio.wait();
      //(*  mostrador==-1 *)

   }


void Estanco::obtenerIngrediente( const int  i )
 {
    if (mostrador!=i) 
        esta_mi_ingred[i].wait();   
      //(*  mostrador==i  *)
    mostrador=-1;
     //  (*  mostrador==-1 *)
    mostrador_vacio.signal();
}
   
   // -----------------------------------------------------------------------------





// *****************************************************************************
// clase para monitor Buffer para el problema del productor-Consumidor FIFO
class Buffer : public HoareMonitor
{
 private:

 int buzon[3];  
 int primera_libre, primera_ocupada, n;
 CondVar buzon_no_lleno, buzon_no_vacio;  

 public:                    // constructor y métodos públicos
   Buffer() ;             // constructor
   void Insertar_sobre( const int fumador );
   int Sacar_sobre( );
   
} ;
// -----------------------------------------------------------------------------

Buffer::Buffer( )
{
   primera_libre=0;
   primera_ocupada=0;
   n=0;
   buzon_no_lleno   = newCondVar();
   buzon_no_vacio   = newCondVar();
}

// -----------------------------------------------------------------------------
void Buffer::Insertar_sobre( const int fumador )
   {
      if (n==3) buzon_no_lleno.wait();
     
      buzon[primera_libre]=fumador;
      primera_libre=(primera_libre+1)%3;
      n++;

      buzon_no_vacio.signal();
   }

// -----------------------------------------------------------------------------
int Buffer::Sacar_sobre( )
   {
      if (n==0) buzon_no_vacio.wait();
     
      int dato=buzon[primera_ocupada];
      primera_ocupada=(primera_ocupada+1)%3;
      n--;
      buzon_no_lleno.signal();
      return dato;
   }

   
   // -----------------------------------------------------------------------------







// *****************************************************************************
// funciones de hebras
// *****************************************************************************

// -----------------------------------------------------------------------------
void funcion_estanquero( MRef<Estanco> monitor)
{
   while(true)
   {
      int ingre = Producir_ingrediente();
      monitor->ponerIngrediente( ingre );
      monitor->esperarRecogidaIngrediente();
   }
}


// -----------------------------------------------------------------------------
void funcion_fumadora( MRef<Estanco>  monitor_estanco,
                       MRef<Buffer>  monitor_buffer, 
                       const int i )
{
   while(true)
   {
     monitor_estanco->obtenerIngrediente( i );
     this_thread::sleep_for( chrono::milliseconds( aleatorio<20,150>() ));

     //Fumar( i ) ;
     monitor_buffer->Insertar_sobre(i);
     mtx.lock();
       cout << "Fumador  "<<i<<" ha insertado sobre en el buzon." << endl<<flush;
     mtx.unlock();

   }
}
 
// -----------------------------------------------------------------------------
void funcion_contrabandista( MRef<Buffer>  monitor)
{  int cigarros[3]={0,0,0};
   int contador=0;
   while(true)
   {
     this_thread::sleep_for( chrono::milliseconds( aleatorio<100,150>() ));
   
     mtx.lock();
     cout << "                  Contrabandista Intenta sacar sobre del buzon." << endl<<flush;
     mtx.unlock();

     int fumador=monitor->Sacar_sobre();

     mtx.lock();
     cout << "                  Contrabandista saca sobre del fumador "
           <<fumador<<" del buzon." << endl<<flush;
     mtx.unlock();

     cigarros[fumador]++;
     contador++;
     if (contador%4==0)
      cout<<"                   Cigarros fumados="<<cigarros[0]<<","<<cigarros[1]<<","<<cigarros[2]<<endl<<flush;
   }
}
 


// -----------------------------------------------------------------------------

int main()
{
  
   cout << "--------------------------------------------------------------------" << endl
        << "   Problema de los fumadores  (Monitor SU).                         " << endl
        << "--------------------------------------------------------------------" << endl
        << flush ;



   MRef<Estanco> monitor_estanco = Create<Estanco>() ;
   MRef<Buffer> monitor_buffer = Create<Buffer>() ;

   thread hebra_estanquero(funcion_estanquero, monitor_estanco),
          hebra_contrabandista(funcion_contrabandista, monitor_buffer) ,
          hebra_fum[num_fum]; 

   for( unsigned i = 0 ; i < num_fum ; i++ )
      hebra_fum[i]=thread ( funcion_fumadora, 
                            monitor_estanco,monitor_buffer, i );

   // esperar a que terminen las hebras
   for( unsigned i = 0 ; i < num_fum ; i++ )
      hebra_fum[i].join();

   hebra_estanquero.join();
   hebra_contrabandista.join();

}
