// Alumno: Ricardo Ruiz Fernández de Alba   GIM

// -----------------------------------------------------------------------------
// Archivo: sendrecv22.cpp
// Difusión de un mensaje en una cadena de procesos
// Modificaciones:

// No podemos pasar a la siguiente iteración hasta que el proceso 0
// reciba confirmación de que el último proceso ha recibido el valor difundido
// en la iteración anterior

// Para ello, enviamos un mensaje desde el ultimo proceso (num_procesos_actual-1)
// al proceso 0 con un valor booleano difusion=true
// El proceso 0 antes de pedir valor para la proxima iteracion (iteracion > 0)
// espera esta confirmacion 
// Se hace uso del envío síncrono con MPI_Ssend()


#include <iostream>
#include <mpi.h>

using namespace std;

const int num_min_procesos = 2 ;

int main( int argc, char *argv[] )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   if ( num_min_procesos <= num_procesos_actual  )
   {
      const int  id_anterior  = id_propio-1, // ident. proceso anterior
                 id_siguiente = id_propio+1 ; // ident. proceso siguiente
      int        valor;  // valor recibido o leído, y enviado
      MPI_Status estado; // estado de la recepción
      MPI_Status estado_difusion; // estado de la difusion
      int iteracion = 0;
      bool difusion;
      do
      {
         // recibir o leer valor
         if ( id_propio == 0 ) // si soy el primero (no hay anterior)
         {

            // Si no es la primera iteracion, esperar a recibir confirmación del ultimo proceso
            if (iteracion > 0)
            {
               MPI_Recv( &difusion, 1, MPI_CXX_BOOL, num_procesos_actual-1, 0, MPI_COMM_WORLD, &estado_difusion );
               if (difusion)
                  cout << "Recibida la confirmacion de la difusion anterior" << endl;
            }
               
            cout << "Introduce un número aquí debajo (-1 para acabar): " << endl ;
            cin >> valor ;        //     pedir valor por teclado (-1 para acabar)
         }
         else                     // si no soy el primer proceso: recibirlo
            MPI_Recv( &valor, 1, MPI_INT, id_anterior, 0, MPI_COMM_WORLD, &estado );
         
         // imprimir valor
         cout<< "Proc."<< id_propio<< ": recibido/leído: "<< valor<< endl ;
         difusion = false;

         // si no soy el último (si hay siguiente): enviar valor,
         if ( id_propio < num_procesos_actual - 1 )
            MPI_Ssend( &valor, 1, MPI_INT, id_siguiente, 0, MPI_COMM_WORLD );
         else if (id_propio == num_procesos_actual -1 && valor >= 0) 
         // Si soy el ultimo confirmo al proceso 0 que la difusión ha terminado
         {
            difusion = true;
            cout << "Enviando la confirmación de la difusión..." << endl;
            MPI_Ssend( &difusion, 1, MPI_CXX_BOOL, 0, 0, MPI_COMM_WORLD);
         }
         // Se ha completado una difusion
         iteracion++;
      }
      while( valor >= 0 ); // acaba cuando se teclea un valor negativo
   }
   else
      cerr << "Se esperaban 2 procesos como mínimo, pero hay 1." << endl ;

   MPI_Finalize();
   return 0;
}
