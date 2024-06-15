// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-plantilla.cpp
// Implementación del problema de los filósofos (sin camarero).
// Plantilla para completar.
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos = 5 ,              // número de filósofos 
   num_filo_ten  = 2*num_filosofos, // número de filósofos y tenedores 
   id_camarero  = 10 ,              // número de camarero
   num_procesos  = num_filo_ten+1 , // número de procesos total (por ahora solo hay filo y ten)
   etiq_sentarse  = 1,
   etiq_levantarse = 2;


//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// ---------------------------------------------------------------------

void funcion_filosofos( int id )
{
   int id_ten_izq = (id+1)              % num_filo_ten, //id. tenedor izq.
      id_ten_der = (id+num_filo_ten-1) % num_filo_ten; //id. tenedor der.

   int peticion, tamanio;

   int vector[5] = {0, 0, 0, 0, 0};

  while ( true )
  {
    int valor = 1; // Simboliza coger el tenedor

    tamanio = aleatorio<1,num_filosofos>();
    std::cout << "Filósofo " << id << " solicita sentarse en mesa" << std::endl;

    MPI_Ssend(&vector, tamanio, MPI_INT, id_camarero, etiq_sentarse, MPI_COMM_WORLD);

    // Para eliminar el interbloqueo hay que romper la simetría
    // Si soy el proceso 0 coger primero el de la derecha y luego el de la izquierda
    
    if (id == 0)
    {
      // ... solicitar tenedor derecho (completar)
      cout <<"Filósofo " <<id <<" solicita ten. der." <<id_ten_der <<endl;
      MPI_Ssend(&valor, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

      // ... solicitar tenedor izquierdo (completar)
      cout <<"Filósofo " <<id << " solicita ten. izq." <<id_ten_izq <<endl;
      MPI_Ssend(&valor, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);

    } else {
      cout <<"Filósofo " <<id << " solicita ten. izq." <<id_ten_izq <<endl;
      // ... solicitar tenedor izquierdo (completar)
      MPI_Ssend(&valor, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);

      cout <<"Filósofo " <<id <<" solicita ten. der." <<id_ten_der <<endl;
      // ... solicitar tenedor derecho (completar)
      MPI_Ssend(&valor, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);
    }

    cout <<"Filósofo " <<id <<" comienza a comer" <<endl ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    valor = 0; // Simboliza soltar el tenedor
    cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;
    // ... soltar el tenedor izquierdo (completar)
    MPI_Ssend(&valor, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);

    cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
    // ... soltar el tenedor derecho (completar)
    MPI_Ssend(&valor, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

    cout << "Filosofo " << id << " comienza a pensar" << endl;
    sleep_for( milliseconds( aleatorio<10,100>() ) );
 }
}
// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  while ( true )
  {
     // ...... recibir petición de cualquier filósofo (completar)
     MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &estado);    

     id_filosofo = estado.MPI_SOURCE;
     // ...... guardar en 'id_filosofo' el id. del emisor (completar)
     cout <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;

     MPI_Recv(&valor, 1, MPI_INT, id_filosofo, 0, MPI_COMM_WORLD, &estado);
     // ...... recibir liberación de filósofo 'id_filosofo' (completar)
     cout <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
  }
}
// ---------------------------------------------------------------------

void funcion_camarero( int id )
{
   int s = 0; // Número de filósofos sentados
   int valor, id_filosofo, etiq_aceptada;
   MPI_Status estado;

   
   while (true)
   {
      if (s < num_filosofos - 1)
         etiq_aceptada = MPI_ANY_TAG;
      else
         etiq_aceptada = etiq_levantarse;

      MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_aceptada, MPI_COMM_WORLD, &estado);
      id_filosofo = estado.MPI_SOURCE;


      switch (estado.MPI_TAG)
      {
         case etiq_sentarse:
            cout << "Filósofo " << id_filosofo << " se sienta en la mesa" << endl;
            s++;
            break;
         case etiq_levantarse:
            cout << "Filósofo " << id_filosofo << " se levanta de la mesa" << endl;
            s--;
            break;
      }

      cout << "Tenemos sentados " << s << " filósofos" << endl;
   }

}

// ---------------------------------------------------------------------
int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if  (id_propio == id_camarero)
         funcion_camarero( id_camarero );
      else {
         if ( id_propio % 2 == 0 )          // si es par
            funcion_filosofos( id_propio ); //   es un filósofo
         else                               // si es impar
            funcion_tenedores( id_propio ); //   es un tenedor
      }
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------
