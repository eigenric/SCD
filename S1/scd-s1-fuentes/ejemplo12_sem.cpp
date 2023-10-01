#include <iostream>
#include <thread>
#include "scd.h"

// declaración de la función {\bf factorial} (parámetro {\bf int}, resultado {\bf long})
long factorial( int n ) { return n > 0 ? n*factorial(n-1) : 1 ; }

int compartida = 0;
scd::Semaphore puede_leer = 0;

int producir_valor(){
    return factorial(5);
}

void usar_valor(int local) {
    std::cout << "Usado valor: " << local << std::endl;
}

void funcion_hebra_productora() {
    int local;
    local = producir_valor();
    std::cout << "Producido valor: " << local << std::endl;
    compartida = local;
    puede_leer.sem_signal();
}

void funcion_hebra_consumidora() {
    int local;

    puede_leer.sem_wait();
    local = compartida;
    std::cout << "Leido valor: " << local << std::endl;
    usar_valor(local);
}

int main()
{
    std::thread p1(funcion_hebra_productora), p2(funcion_hebra_consumidora);

    p1.join();
    p2.join();
}