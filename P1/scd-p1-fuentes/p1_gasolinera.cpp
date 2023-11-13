// Alumno: Ricardo Ruiz Fernández de Alba - DNI: 77168601J

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "scd.h"

using namespace std ;
using namespace scd ;

// Variables globales

const unsigned 
       A = 4, // Hebras tipo Diesel
       B = 6, // Hebras tipo Gasolina
       C = 2, // Nº de surtidores diesel
       D = 3; // Nº de surtidores gasolina

int surtidores_ocupados = 0;  // Nº de surtidores en uso <= C+D
//    surtidores_diesel_ocupados = 0,
//    surtidores_gasolina_ocupados = 0;

Semaphore
   surtidor_diesel_libre(C), // Semáforo que cuenta el número de surtidores de diesel libres.
   surtidor_gasolina_libre(D); // Semáforo que cuenta el número de surtidorws de gasolina libres.

Semaphore
   mtx_cout(1);

void repostar(unsigned int n, string combustible)
{
   cout << "Coche número " << n << " de " << combustible << " comienza a repostar." << endl;

   this_thread::sleep_for( chrono::milliseconds( aleatorio<5,50>() ));

   cout << "Coche número " << n << " de " << combustible << " termina de repostar." << endl;
}

void funcion_hebra_gasoil(unsigned int n)
{
   while (true)
   {
      // Se espera hasta que quede surtidor diesel libre. Si hay ocupa y disminuye
      surtidor_diesel_libre.sem_wait();

      mtx_cout.sem_wait();
      surtidores_ocupados++;
      cout << "El número de surtidores en uso ahora es " << surtidores_ocupados << endl;
      mtx_cout.sem_signal();

      repostar(n, "diesel");

      mtx_cout.sem_wait();
      surtidores_ocupados--;
      cout << "El número de surtidores en uso ahora es " << surtidores_ocupados << endl;
      mtx_cout.sem_signal();

      // Se termina de respotar y queda surtior diesel libre
      surtidor_diesel_libre.sem_signal();

      // Fuera de gasolinera
      this_thread::sleep_for( chrono::milliseconds( aleatorio<50,100>() ));
   }
}

//----------------------------------------------------------------------

void funcion_hebra_gasolina(unsigned int m)
{
   while (true)
   {
      // Se espera hasta que quede surtidor gasolina libre. Si hay ocupa y disminuye
      surtidor_gasolina_libre.sem_wait();

      mtx_cout.sem_wait();
      surtidores_ocupados++;
      cout << "El número de surtidores en uso ahora es " << surtidores_ocupados << endl;
      repostar(m, "gasolina");
      mtx_cout.sem_signal();

      mtx_cout.sem_wait();
      surtidores_ocupados--;
//      surtidores_gasolina_ocupados--;
//      cout << "El número de surtidores gasolina en uso ahora es " << surtidores_gasolina_ocupados << endl;
      cout << "El número de surtidores en uso ahora es " << surtidores_ocupados << endl;
      mtx_cout.sem_signal();

      // Se termina de respotar y queda surtior gasolina libre
      surtidor_gasolina_libre.sem_signal();

      // Fuera de gasolinera
      this_thread::sleep_for( chrono::milliseconds( aleatorio<50,100>() ));
   }
}

//----------------------------------------------------------------------


int main()
{
   cout << "-------------------------------"<< endl
        << "Problema Gasolinera (Semáforos)" << endl 
        << "-------------------------------" << endl
        << flush ;


   thread hebra_tipo_diesel[A],
          hebra_tipo_gasolina[B];
         
   for (int n=0; n < A; n++)
      hebra_tipo_diesel[n] = thread(funcion_hebra_gasoil, n);
   
   for (int m=0; m < B; m++)
      hebra_tipo_gasolina[m] = thread(funcion_hebra_gasolina, m);

   // Basta esperarar a cualquier hebra 
   hebra_tipo_diesel[0].join();
   
   // for (int n=0; n < A; n++)
   //    hebra_tipo_diesel[n].join();
   
   // for (int m=0; m < B; m++)
   //    hebra_tipo_gasolina[m].join();
}
