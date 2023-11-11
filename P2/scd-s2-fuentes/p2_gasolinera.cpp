// Ricardo Ruiz Fernandez de Alba - DGIIM
// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// Archivo: p2_gasolinera
//
// Ejemplo de un monitor en C++11 con semántica SU, para el problema
// de los lectores / escritores
// -----------------------------------------------------------------------------------

#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

// clase para monitor para el problema de los lectores y escritores

class Gasolinera : public HoareMonitor
{
 private:
   int num_surtidores_ocupados,        // Nº total de surtidores ocupados en cada momento
//       num_surtidores_gasoil_ocupados, // Nº de surtidores de gasoil ocupados en cada momento
//       num_surtidores_gasolina_ocupados, // Nº de surtidores de gasolina ocupados en cada momento.
       surtidor_gasoil_libre,             // Índice del ultimo surtidor libre de gasoil
       surtidor_gasolina_libre;           // Índice del último surtidor libre de gasolina

   bool surtidor_gasolina[3],  // 3 Surtidores de gasolina. Inicialmente libres. (true)
        surtidor_gasoil[2];    // 2 Surtidores de gaosil. Inicialmente libres.

 CondVar cola_gasolina,       // Espera en la cola gasolina hasta que quede un surtidor gasolinalibre.
         cola_gasoil;         // Espera en la cola gasoil hasta que quede un surtidor gasoil libre

 public:                    // constructor y métodos públicos
   Gasolinera() ;             // constructor

   void entra_coche_gasoil(unsigned int n);
   void sale_coche_gasoil(unsigned int n);
   void entra_coche_gasolina(unsigned int m);
   void sale_coche_gasolina(unsigned int m);
//   void mostrar_surtidor_gasoil() const;
//   void mostrar_surtidor_gasolina() const;
   
} ;
// -----------------------------------------------------------------------------

Gasolinera::Gasolinera()
{
   num_surtidores_ocupados = 0;
//   num_surtidores_gasoil_ocupados = 0;
//   num_surtidores_gasolina_ocupados = 0;
   surtidor_gasoil_libre = 0;
   surtidor_gasolina_libre = 0;

   for (int i=0; i < 2; i++)
      surtidor_gasoil[i] = true;

   for (int j=0; j < 3; j++)
      surtidor_gasolina[j] = true;
   
   cola_gasoil = newCondVar();
   cola_gasolina = newCondVar();
}
// -----------------------------------------------------------------------------

// void Gasolinera::mostrar_surtidor_gasoil() const
// {
//    cout << "GASOIL: [";
//    for (int i=0; i < 2; i++)
//    {
//       if (surtidor_gasoil[i])
//          cout << "L";
//       else
//          cout << "O";
//       if (i < 1) cout << ",";
//    }
//    cout << "]" << endl;
// }

// // -----------------------------------------------------------------------------
// void Gasolinera::mostrar_surtidor_gasolina() const
// {
//    cout << "GASOLINA: [";
//    for (int i=0; i < 3; i++)
//    {
//       if (surtidor_gasolina[i])
//          cout << "L";
//       else
//          cout << "O";
//       if (i < 2) cout << ",";
//    }
//    cout << "]" << endl;
// }

// -----------------------------------------------------------------------------


void Gasolinera::entra_coche_gasoil(unsigned int n)
{
   // Si los surtidores de gasoil están ocupados, espero en la cola
   if (!surtidor_gasoil[0] && !surtidor_gasoil[1]) {
//      cout << "Coche gasoil " << n << ": 2 surtidores ocupados. Esperando....." << endl; 
      cola_gasoil.wait();
   }

   if (surtidor_gasoil[n % 2])
      surtidor_gasoil_libre= n % 2;
   else if (surtidor_gasoil[(n+1) % 2])
      surtidor_gasoil_libre = (n+1) % 2;
   
   surtidor_gasoil[surtidor_gasoil_libre] = false;
      
   num_surtidores_ocupados++;
//   num_surtidores_gasoil_ocupados++;

   cout << "Entra coche tipo gasoil nº " << n << endl;
//   cout << "Nº Surtidores gasoil ocupados: " << num_surtidores_gasoil_ocupados << endl;
//   mostrar_surtidor_gasoil();
   cout << "Nº Surtidores ocupados: " << num_surtidores_ocupados << endl;
}

void Gasolinera::sale_coche_gasoil(unsigned int n)
{
   // Libero al ultimo coche gasoil que entro
   surtidor_gasoil[surtidor_gasoil_libre] = true;

   num_surtidores_ocupados--;
//   num_surtidores_gasoil_ocupados--;

   cout << "Sale coche tipo gasoil nº " << n << endl;
//   cout << "Nº Surtidores gasoil ocupados: " << num_surtidores_gasoil_ocupados << endl;
//   mostrar_surtidor_gasoil();
   cout << "Nº Surtidores ocupados: " << num_surtidores_ocupados << endl;

   // Señalo de forma urgente al coche que esperaba algun surtidor de gasoil
   cola_gasoil.signal();
}

void Gasolinera::entra_coche_gasolina(unsigned int m)
{
   // Si los surtidores de gasolina están ocupados, espero en la cola
   if (!surtidor_gasolina[0] && !surtidor_gasolina[1] && !surtidor_gasolina[2])
   {
//      cout << "Coche gasolina " << m << ": 3 surtidores ocupados. Esperando....." << endl; 
      cola_gasolina.wait();
   }

   // Buscar el primer surtidor de gasolina libre
   surtidor_gasolina_libre = m % 3;
   while (surtidor_gasolina_libre < 3 && !surtidor_gasolina[surtidor_gasolina_libre])
      surtidor_gasolina_libre = (surtidor_gasolina_libre + 1) % 3;

   surtidor_gasolina[surtidor_gasolina_libre] = false;

   num_surtidores_ocupados++;
//   num_surtidores_gasolina_ocupados++;

   cout << "Entra coche tipo gasolina nº " << m << endl;
//   cout << "Nº Surtidores gasolina ocupados: " << num_surtidores_gasolina_ocupados << endl;
//   mostrar_surtidor_gasolina();
   cout << "Nº Surtidores ocupados: " << num_surtidores_ocupados << endl;
}

void Gasolinera::sale_coche_gasolina(unsigned int m)
{

   surtidor_gasolina_libre = m % 3;
   while (surtidor_gasolina_libre < 3 && surtidor_gasolina[surtidor_gasolina_libre])
      surtidor_gasolina_libre = (surtidor_gasolina_libre + 1) % 3;

   // Libero el primer surtidor ocupado empezando por el asociado a m
   surtidor_gasolina[surtidor_gasolina_libre] = true;
   
   num_surtidores_ocupados--;
//   num_surtidores_gasolina_ocupados--;

   cout << "Sale coche tipo gasolina nº " << m << endl;
//   cout << "Nº Surtidores gasolina ocupados: " << num_surtidores_gasolina_ocupados << endl;
//   mostrar_surtidor_gasolina();
   cout << "Nº Surtidores ocupados: " << num_surtidores_ocupados << endl;

   // Señalo de forma urgente al coche que esperaba algun surtidor de gasolina
   cola_gasolina.signal();
}


void funcion_hebra_gasoil( MRef<Gasolinera> monitor, unsigned int n)
{
   while (true)
   {
      monitor->entra_coche_gasoil(n);
      this_thread::sleep_for( chrono::milliseconds( aleatorio<5,30>() ));
      monitor->sale_coche_gasoil(n);
      this_thread::sleep_for( chrono::milliseconds( aleatorio<50,100>() ));
   }
}

void funcion_hebra_gasolina( MRef<Gasolinera> monitor, unsigned int m)
{
   while (true)
   {
      monitor->entra_coche_gasolina(m);
      this_thread::sleep_for( chrono::milliseconds( aleatorio<5,20>() ));
      monitor->sale_coche_gasolina(m);
      this_thread::sleep_for( chrono::milliseconds( aleatorio<50,100>() ));
   }
}


// -----------------------------------------------------------------------------

int main()
{
   cout << "------------------------------------" << endl
        << "Problema de Gasolinera (Monitor SU)." << endl
        << "------------------------------------" << endl
        << flush ;

   // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
   MRef<Gasolinera> monitor = Create<Gasolinera>() ;

   int N = 4, // Hebras gasoil
       M = 6; // Hebras gasolina

   thread hebra_gasoil[N], 
          hebra_gasolina[M];

   for (int n=0; n < N; n++)
      hebra_gasoil[n] = thread(funcion_hebra_gasoil, monitor, n);
   
   for (int m=0; m < M; m++)
      hebra_gasolina[m] = thread(funcion_hebra_gasolina, monitor, m);

   for (int n=0; n < N; n++)
      hebra_gasoil[n].join();

   for (int m=0; m < M; m++)
      hebra_gasolina[m].join();
}
