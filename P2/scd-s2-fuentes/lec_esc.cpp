// Alumno: Ricardo Ruiz Fernandez de Alba - DNI: 77168601J

// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// Archivo: lec_escr.cpp
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

class LecEscSU : public HoareMonitor
{
 private:
   bool escrib;                       // Variable logica que vale true si un escritor está escribiendo
   unsigned int n_lec;               // Variable entera, número de electores que están leyendo en un momento dado


 CondVar                    // colas condicion:
   lectura,                 // Usada por los lectores para esperar cuando hay un escritor escribiendo
   escritura;               // Usada por los escritores para esperar cuando ya hay 
                            // otro escritor escribiendo o hay lectores leyendo

 public:                    // constructor y métodos públicos
   LecEscSU() ;             // constructor

   void ini_lectura(); 
   void fin_lectura();
   void ini_escritura();
   void fin_escritura(); 
   unsigned int get_nlec() const { return n_lec; }
   
} ;
// -----------------------------------------------------------------------------

LecEscSU::LecEscSU()
{
   escrib = false;
   n_lec = 0;
   lectura = newCondVar();
   escritura = newCondVar();
}
// -----------------------------------------------------------------------------

void LecEscSU::ini_lectura()
{
   if (escrib)
      lectura.wait();
   
   n_lec++;
   lectura.signal(); // Prioridad a los lectores
}

void LecEscSU::fin_lectura()
{
   n_lec--;

   if (n_lec == 0)
      escritura.signal();
}

void LecEscSU::ini_escritura()
{
   if (escrib || n_lec > 0)
      escritura.wait();
   
   escrib = true;
}

void LecEscSU::fin_escritura()
{
   escrib = false;
   // Sabemos n_lec == 0

   if (!lectura.empty())
      lectura.signal();
   else
      escritura.signal();

}

void lector(MRef<LecEscSU> Lec_Esc, unsigned int i)
{ 
   mutex mtx;
   while (true)
   {
      Lec_Esc->ini_lectura();
      cout << "Iniciando lectura (Lector " << i << ")" << endl;
      this_thread::sleep_for( chrono::milliseconds( aleatorio<10,100>() ));
      int n_lec = Lec_Esc->get_nlec();
      
      mtx.lock();
      cout << "n_lec = " << n_lec << endl;
      mtx.unlock();

      cout << "Finalizando lectura (Lector " << i << ")" << endl;
      Lec_Esc->fin_lectura();
      n_lec = Lec_Esc->get_nlec();
      
      mtx.lock();
      cout << "n_lec = " << n_lec << endl;
      mtx.unlock();
   }
}

void escritor(MRef<LecEscSU> Lec_Esc, unsigned int i)
{
   mutex mtx;
   while (true)
   {
      Lec_Esc->ini_escritura();
      cout << "Iniciando escritura (Escritor " << i << ")" << endl;
      this_thread::sleep_for( chrono::milliseconds( aleatorio<10,100>() ));
      cout << "Finalizando escritura (Escritor " << i << ")" << endl;
      Lec_Esc->fin_escritura();
   }
}

// -----------------------------------------------------------------------------

int main()
{
   cout << "--------------------------------------------------------------------" << endl
        << "Problema de los lectores-escritores (Monitor SU). " << endl
        << "--------------------------------------------------------------------" << endl
        << flush ;

   // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
   MRef<LecEscSU> monitor = Create<LecEscSU>() ;

   constexpr unsigned int n = 5, // Número de lectores
                          m = 3; // Número de escritores

   thread hebra_lectora[n], hebra_escritora[m];
   
   for (int i=0; i < n; i++)
      hebra_lectora[i] = thread(lector, monitor, i);
   
   for (int i=0; i < m; i++)
      hebra_escritora[i] = thread(escritor, monitor, i);

   for (int i=0; i < n; i++)
      hebra_lectora[i].join();

   for (int i=0; i < m; i++)
      hebra_escritora[i].join();
}
