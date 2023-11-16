// Alumno: Ricardo Ruiz Fernandez de Alba - DNI: 77168601J

// Problema de los filosófos

#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

Semaphore msg(1);   // Exclusión mutua mensajes por pantalla;

void comer(unsigned int num_filosofo)
{
   msg.sem_wait();
   cout << "Filosofo " << num_filosofo << " empieza a comer ...." << endl;
   msg.sem_signal();

   chrono::milliseconds duracion_produ( aleatorio<10, 100>() );
   this_thread::sleep_for( duracion_produ );

   msg.sem_wait();
   cout << "Filosofo " << num_filosofo << " terminar comer ...." << endl;
   msg.sem_signal();
}

void pensar(unsigned int num_filosofo)
{
   msg.sem_wait();
   cout << "Filosofo " << num_filosofo << " empieza a pensar ...." << endl;
   msg.sem_signal();

   chrono::milliseconds duracion_produ( aleatorio<100, 200>() );
   this_thread::sleep_for( duracion_produ );

   msg.sem_wait();
   cout << "Filosofo " << num_filosofo << " terminar pensar ...." << endl;
   msg.sem_signal();
}
// clase para monitor para el problema de los lectores y escritores

class CenaFilo_NoSeguro : public HoareMonitor
{
 private:
   bool ten_ocup[5];     // Array de booleanos que india si el tenedor i está ocupado
   CondVar cola_ten[5];   // Array de variables condicion para esperar mientras el tenedor i está ocupado

 public:                    // constructor y métodos públicos
   CenaFilo_NoSeguro() ;             // constructor

   void coger_tenedor(unsigned int num_tenedor, unsigned int num_filosofo);
   void soltar_tenedor(unsigned int num_tenedor, unsigned int num_filosofo);
} ;
// -----------------------------------------------------------------------------

CenaFilo_NoSeguro::CenaFilo_NoSeguro()
{
   for (int i=0; i < 5; i++)
   {
      ten_ocup[i] = false;
      cola_ten[i] = newCondVar();
   }
}

void CenaFilo_NoSeguro::coger_tenedor(unsigned int num_tenedor, unsigned int num_filosofo)
{
   if (ten_ocup[num_tenedor])
      cola_ten[num_tenedor].wait();
   
   // Hay un tenedor libre. Lo cojo
   ten_ocup[num_tenedor] = true;
   if (num_tenedor == num_filosofo)
      cout << "El filosofo " << num_filosofo << " coge el tenedor de su derecha." << endl;
   else if (num_tenedor == (num_filosofo+1) % 5)
      cout << "El filosofo " << num_filosofo << " coge el tenedor de su izquierda." << endl;
}

void CenaFilo_NoSeguro::soltar_tenedor(unsigned int num_tenedor, unsigned int num_filosofo)
{
   ten_ocup[num_tenedor] = false;
   if (num_tenedor == num_filosofo)
      cout << "El filosofo " << num_filosofo << " suelta el tenedor de su derecha." << endl;
   else if (num_tenedor == (num_filosofo+1) % 5)
      cout << "El filosofo " << num_filosofo << " suelta el tenedor de su izquierda." << endl;
   cola_ten[num_tenedor].signal();
}

// -----------------------------------------------------------------------------

void funcion_hebra_filosofo(unsigned int num_filosofo, MRef<CenaFilo_NoSeguro> monitor)
{
   while (true)
   {  
      int tenedor_derecha = num_filosofo,
          tenedor_izquierda = (num_filosofo + 1) % 5;

      monitor->coger_tenedor(tenedor_derecha, num_filosofo);
      monitor->coger_tenedor(tenedor_izquierda, num_filosofo);
      comer(num_filosofo);
      monitor->soltar_tenedor(tenedor_derecha, num_filosofo);
      monitor->soltar_tenedor(tenedor_izquierda, num_filosofo);
      pensar(num_filosofo);
   }
}

int main()
{
   cout << "------------------------------------" << endl
        << "Problema de los Filósfos (Monitor SU)." << endl
        << "------------------------------------" << endl
        << flush ;

   // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
   MRef<CenaFilo_NoSeguro> monitor = Create<CenaFilo_NoSeguro>() ;
   
   thread hebra_filosofo[5];

   for (int i=0; i < 5; i++)
      hebra_filosofo[i] = thread(funcion_hebra_filosofo, i, monitor);

   for (int i=0; i < 5; i++)
      hebra_filosofo[i].join();

}
