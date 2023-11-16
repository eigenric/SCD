
#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>

using namespace std;

int x = 0;
int c1 = 0, c2 = 0, turno = 1;

void funcion_hebra_p1()
{
    while (true)
    {
        cout << "P1 fuera de SC" << endl;
        c1 = 0;
        cout << "P1 con intención de entrar en SC" << endl;
        while (c2 == 0)
        {
            c1 = 1;
            if (turno == 2)
            {
                cout << "Turno 2 y P2 en SC. " << endl;
                while (turno == 2)
                    cout << ".......P1 espera ocupado......" << endl;

                c1 = 0;
                cout << "P1 con intención de entrar en SC." << endl;
            }
        }
        cout << "P1. Inicio Sección Crítica" << endl;
        x++;
        cout << "x= " << x << endl;
        turno = 2;
        cout << "Cambio a Turno 2" << endl;
        c1 = 1;
        cout << "P1. Fin Sección Crítica" << endl;
    }
}

void funcion_hebra_p2()
{
    while (true)
    {
        cout << "P2 fuera de SC" << endl;
        c2 = 0;
        cout << "P2 con intención de entrar en SC" << endl;
        while (c1 == 0)
        {
            c2 = 1;
            if (turno == 1) 
            {
                cout << "       Turno 1 y P1 en SC. " << endl;
                while (turno == 1)
                    cout << ".......P2 espera ocupado......" << endl;
                c2 = 0;
                cout << "       P2 con intención de entrar en SC. " << endl;
            }
        }
        cout << "       P2. Inicio Sección Crítica" << flush;
        x++;
        cout << "       x=" << x << endl;
        turno = 1;
        cout << "       Cambio a Turno 1" << endl;
        c2 = 1;
        cout << "       P2. Fin Sección Crítica" << flush;
    }
}

int main()
{

    thread p1(funcion_hebra_p1), p2(funcion_hebra_p2);

    p1.join();
    p2.join();

}