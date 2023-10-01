#include <iostream>
#include <thread>
#include <chrono>
#include "scd.h"

using namespace std;
using namespace scd;

int num_iter = 10;
int valor_compartido = 0;
int contador = 0;

Semaphore puede_leer = 0;
Semaphore puede_escribir = 1;

int producir_valor()
{
    contador++;
    chrono::milliseconds duracion_bloqueo_ms( aleatorio<20,200>() );
    this_thread::sleep_for(duracion_bloqueo_ms);
    cout << "producido: " << contador << endl;
    return contador;
}

void consumir_valor(int valor)
{
    chrono::milliseconds duracion_bloqueo_ms( aleatorio<20,200>() );
    this_thread::sleep_for(duracion_bloqueo_ms);
    cout << "      consumido: " << valor << endl;
}

void funcion_hebra_productora()
{
    for (int i=0; i < num_iter; i++)
    {
        int valor_producido = producir_valor();
        sem_wait(puede_escribir);
        valor_compartido = valor_producido;
        cout << "escrito: " << valor_producido << endl;
        sem_signal(puede_leer);
    }
}

void funcion_hebra_consumidora()
{

    for (int i=0; i < num_iter; i++)
    {
        sem_wait(puede_leer);
        int valor_consumido = valor_compartido;
        cout << "      leído: " << valor_consumido << endl;
        sem_signal(puede_escribir);
        consumir_valor(valor_consumido);
    }
}

int main()
{
    thread p1(funcion_hebra_productora), p2(funcion_hebra_consumidora);

    p1.join();
    p2.join();
}