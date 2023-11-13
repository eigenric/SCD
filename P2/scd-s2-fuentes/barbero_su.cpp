// Alumno: Ricardo Ruiz Fernandez de Alba - DNI: 77168601J

// -----------------------------------------------------------------------------
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// Archivo: barbero_su
//
// Ejemplo de un monitor en C++11 con semántica SU, para el problema
// del Barbero
// -----------------------------------------------------------------------------------

#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

/// Esperas aleatorias
void cortarPeloCliente()
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<50,100>() ));
   cout << ".........Cortando el pelo al cliente ........." << endl;
}

void esperarFueraBarberia(unsigned int i)
{
   cout << "Cliente " << i << " esperando fuera de la barberia " << i << " milisegundos" << endl;
   this_thread::sleep_for( chrono::milliseconds(i) );  
}

// Clase para monitor para el problema del Barbero Durmiente

class Barberia : public HoareMonitor
{
 private:
   bool silla_ocupada;      // true si el barbero está cortando el pelo false si esta durmiendo

   CondVar sala_espera,     // Los clientes esperan hasta que el barbero pueda atenderles
           pelando_espera,   // El cliente espera hasta que el barbero termine de pelarlo
           durmiendo;        // Si el barbero esta dormido, el cliente lo despierta.

 public:                    // constructor y métodos públicos
   Barberia() ;             // constructor

   void siguienteCliente();
   void cortarPelo(unsigned int i);
   void finCliente();
   
} ;

// -----------------------------------------------------------------------------

Barberia::Barberia()
{
   // No hay cliente en la silla al principio
   silla_ocupada = false;
   sala_espera = newCondVar();
   pelando_espera = newCondVar();
   durmiendo = newCondVar();
}

// -----------------------------------------------------------------------------
void Barberia::cortarPelo(unsigned int i)
{
   cout << "Cliente " << i << " entra en la barbería " << endl;

   if (silla_ocupada) {
      cout << "El barbero está ocupado. Cliente " << i << " espera en la sala de espera." << endl;
      sala_espera.wait();
   } else {
      cout << "El barbero estaba durmiendo. El Cliente " << i << " lo despierta" << endl;
      // Durmiendo: el cliente lo despierta
      durmiendo.signal();
   }

   cout << "Cliente " << i << " espera en la silla de corte" << endl;
   silla_ocupada = true;
   pelando_espera.wait();
}

// -----------------------------------------------------------------------------

void Barberia::siguienteCliente()
{
   if (sala_espera.empty()) {
      cout << "No hay clientes esperando. El barbero se echa a dormir: zzzZzZzz" << endl;
      durmiendo.wait();
   }

   cout << "Barbero llama al siguiente cliente de la sala de espera " << endl;
   sala_espera.signal();
}

// -----------------------------------------------------------------------------

void Barberia::finCliente()
{
   cout << "El barbero avisa al cliente: fin del corte de pelo" << endl;
   silla_ocupada = false;
   pelando_espera.signal();
}

// -----------------------------------------------------------------------------

void funcion_hebra_barbero( MRef<Barberia> monitor)
{
   while (true)
   {
      monitor->siguienteCliente();
      cortarPeloCliente();
      monitor->finCliente();
   }
}

// -----------------------------------------------------------------------------
void funcion_hebra_cliente( MRef<Barberia> monitor, unsigned int i)
{
   while (true)
   {
      monitor->cortarPelo(i);
      esperarFueraBarberia(i);
   }
}

// -----------------------------------------------------------------------------

int main()
{
   cout << "------------------------------------" << endl
        << "Problema del Barbero  (Monitor SU)." << endl
        << "------------------------------------" << endl
        << flush ;

   // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
   MRef<Barberia> monitor = Create<Barberia>() ;

   int n = 10;

   thread hebra_barbero(funcion_hebra_barbero, monitor),
          hebra_clientes[n];

   for (int i=0; i < n; i++)
      hebra_clientes[i] = thread(funcion_hebra_cliente, monitor, i);

   hebra_barbero.join();

   // for (int i=0; i < n; i++)
   //    hebra_clientes[i].join();
}
