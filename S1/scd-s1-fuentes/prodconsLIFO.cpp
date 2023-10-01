#include <iostream>
#include <thread>
#include <chrono>
#include "scd.h"

using namespace std;
using namespace scd;

int contador = 0;
int num_items = 75;
int primera_libre = 0;
mutex mtx;

#define tam_vec 15
int buffer[tam_vec] = {0};

Semaphore libres = tam_vec;
Semaphore ocupadas = 0;

void mostrar_buffer()
{
    cout << "   buffer: [";
    for (int i=0; i < tam_vec; i++)
    {
        cout << buffer[i];
        if (i < tam_vec -1)
            cout << ",";
    }
    cout << "]" << endl;
}

int producir_valor()
{
    contador++;
    chrono::milliseconds duracion_bloqueo_ms( aleatorio<20,100>() );
    this_thread::sleep_for(duracion_bloqueo_ms);
    cout << "producido: " << contador << endl;
    return contador;
}

void consumir_valor(int valor)
{
    chrono::milliseconds duracion_bloqueo_ms( aleatorio<20,100>() );
    this_thread::sleep_for(duracion_bloqueo_ms);
    cout << "       dato consumido: " << valor << endl;
}

void funcion_hebra_productora()
{
    for (int i=0; i < num_items; i++)
    {
        int valor_producido = producir_valor();

        mtx.lock();
        if (primera_libre == tam_vec)
            cout << "Esperando a que quede una libre..." << endl;
        mtx.unlock();

        // Espera a que quede al menos 1 elemento libre en el buffer
        // Al inicio todas estan libres (tam_vec)
        sem_wait(libres);

        buffer[primera_libre] = valor_producido;
        cout << "escrito: " << valor_producido << endl;
        primera_libre = (primera_libre+1) % tam_vec; 
            // Cuando esta completo o vacio vale 0

        mtx.lock();
        mostrar_buffer();
        if (primera_libre == tam_vec)
            cout << "El buffer se llenó: consumiendo datos" << endl;
        mtx.unlock();

        sem_signal(ocupadas); 
        // Indica que se ha ocupado un elemento extra en el buffer
    }
}

void funcion_hebra_consumidora()
{

    for (int i=0; i < num_items; i++)
    {
        // Espera hasta que se produzca al menos 1
        sem_wait(ocupadas);

        int valor_consumido = buffer[primera_libre-1];
        buffer[primera_libre-1] = 0; // Vaciar el elemento anterior
        primera_libre--;

        mtx.lock();
        cout << "       leído: " << valor_consumido << endl;
        mostrar_buffer();
        mtx.unlock();

        sem_signal(libres); // Libera 1 elementos en el buffer

        consumir_valor(valor_consumido);
    }
}

int main()
{
    thread p1(funcion_hebra_productora), p2(funcion_hebra_consumidora);

    p1.join();
    p2.join();
}