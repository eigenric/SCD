---
title: "Sistemas Concurrentes y Distribuidos. Práctica 1"
author: ["Ricardo Ruiz Fernández de Alba"]
date: "01/10/2023"
subject: "Sistemas Concurrentes y Distribuidos"
keywords: ["Productor-Consumidor", "Semáforos"]
subtitle: "Sincronización de hebras con Semáforos. Problema Productor-Consumidor"
titlepage: true
titlepage-background: "P1/memoria/background1.pdf"
toc: true
toc-own-page: true
# titlepage-color: "3C9F53"
# titlepage-text-color: "FFFFFF"
# titlepage-rule-color: "FFFFFF"
# titlepage-rule-height: 2
output:
  pdf_document:
    pandoc_args: [
      "--listings"
    ]
    template: eisvogel
---

# Descripción del problema

El problema del productor consumidor surge cuando se quiere diseñar un programa en el cual una hebra produce items de datos en memoria mientras que otra hebra los consume.

En general, la productora calcula o produce una secuencia de items de datos (uno a uno), y la consumidora lee o consume dichos items (tambien uno a uno).

El tiempo que se tarda en producir un item de datos puede ser variable y en general distinto al que se tarda en consumirlo (también variable).

En esta práctica se pide implementar una solución al problema del productor consumidor utilizando semáforos.

# Esquema General de la Solución

```c++
{ variables compartidas y valores iniciales }
var tam_vec : integer := k ; { tamaño del vector }
num_items : integer := .... ; { número de items }
vec : array[0..tam_vec-1] of integer; { vector intermedio }

process HebraProductora ; var a : integer ;
  begin
    for i := 0 to num_items-1 do begin
      a := ProducirValor() ;
      { Sentencia E: } { (insertar valor 'a' en 'vec') }
    end
end

process HebraConsumidora var b : integer ;
begin
  for i := 0 to num_items-1 do begin
  { Sentencia L: } { (extraer valor 'b' de 'vec') } 
  ConsumirValor(b) ;
end
```

### Implementación función principal

```c++
void main() {
   [...]
   thread hebra_productora ( funcion_hebra_productora ),
          hebra_consumidora( funcion_hebra_consumidora );

   hebra_productora.join() ;
   hebra_consumidora.join() ;
   cout << "fin" << endl;
   test_contadores() ;
}
```

# Diseño general mediante semáforos

**Describe los semáforos necesarios, la utilidad de los mismos, el valor inicial y en qué puntos del programa se debe usar sem_wait y sem_signal sobre ellos.**


Utilizaremos tres semáforos para sincronizar las hebras: `libres`, `ocupadas` y `op_buffer`.

El semáforo `libres` nos indicará el número de posiciones libres del vector. Inicialmente, libres toma como valor el tamaño del vector (`libres = tam_vec`).

El semáforo `ocupadas` nos indicará el número de posiciones ocupadas del vector. Inicialmente, hay 0 celdas ocupadas en el vector (`ocupadas = 0`).

El semáforo `op_buffer` nos indicará si se está realizando una operación sobre el buffer. Inicialmente, puede realizarse una inserción (`op_buffer = 1`). Esto permite que la hebra productora y la hebra consumidora no puedan realizar operaciones sobre el buffer a la vez. (Exclusión mutua entre inserción y extracción).

## Sincronización entre los semáforos

La hebra productora deberá esperar a que haya al menos una posición libre en el vector para poder insertar un valor. Esto se consigue mediante la operación `libres.sem_wait()`.

La hebra consumidora deberá esperar a que haya al menos una posición ocupada en el vector para poder extraer un valor. Esto se consigue mediante la operación `ocupadas.sem_wait()`.

La hebra productora deberá esperar a que no se esté realizando ninguna operación sobre el buffer para poder insertar/extraer un valor. Esto se consigue mediante la operación `op_buffer.sem_wait()`.

Cuando una operación sobre el buffer ha finalizado, se debe liberar el semáforo `op_buffer` mediante la operación `op_buffer.sem_signal()`.

Cuando se ha insertado un valor en el vector, se debe incrementar el número de posiciones ocupadas mediante la operación `ocupadas.sem_signal()`.

Cuando se ha extraído un valor del vector, se debe incrementar el número de posiciones libres mediante la operación `libres.sem_signal()`.

## Diseño mediante pila acotada (LIFO)

La solución LIFO (Last In First Out) consiste en que la hebra productora inserta los valores en la última posición libre del vector, y la hebra consumidora extrae los valores de la última posición ocupada del vector. Esto es equivalente a considerar el vector como una pila acotada.

Haremos uso de una variable entera `primera_libre` que nos indicará la primera posición libre del vector. Inicialmente, `primera_libre = 0`. 

La hebra productora escribirá en la posición `vec[primera_libre]` y después incrementará el valor de `primera_libre`. 

La hebra consumidora leerá de la posición `vec[primera_libre - 1]` y después decrementará el valor de `primera_libre`.

## Implementación LIFO en C++11

### Función hebra productora


```c++
Semaphore 
  libres(tam_vec),
   ocupadas(0), 
   op_buffer(1); 

unsigned int buffer[tam_vec], /
primera_libre = 0; 
[...]
void  funcion_hebra_productora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato = producir_dato() ;

      // Inicio SC
      libres.sem_wait(); // Producir tantos datos como elementos libres haya en el buffer
      op_buffer.sem_wait(); // Inserción en exclusión mutua con la extracción

      // Inserción del dato
      assert(0 <= primera_libre && primera_libre < tam_vec);
      buffer[primera_libre++] = dato;
      cout << "inserción en buffer: " << buffer[primera_libre] << endl;
      mostrar_buffer();

      op_buffer.sem_signal();
      ocupadas.sem_signal(); // 
      // Fin SC
   }
}
```

### Función hebra consumidora

```c++
void funcion_hebra_consumidora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato ;

      // Inicio SC
      ocupadas.sem_wait(); // Espera hasta que haya al menos un elemento en el buffer
      op_buffer.sem_wait(); // La extraccion debe ocurrir en exclusion mutua con la inserción

      // Extracción del dato
      assert(0 <= primera_libre && primera_libre < tam_vec);
      dato = buffer[--primera_libre];
      buffer[primera_libre] = 0;
      cout << "extraído de buffer: " << dato << endl;
      mostrar_buffer();


      op_buffer.sem_signal();
      libres.sem_signal(); // Se ha extraido un elemento del buffer, queda uno mas libre
      // Fin SC

      consumir_dato( dato ) ;
    }
}
```



## Diseño mediante cola circular (FIFO)

La solución FIFO (First In First Out) consiste en que la hebra productora inserta los valores en la última posición libre del vector, y la hebra consumidora extrae los valores de la primera posición ocupada del vector. Esto es equivalente a considerar el vector como una cola circular.

En este caso, necesitaremos dos variables enteras `primera_libre` y `primera_ocupada` que nos indicarán la primera posición libre y la primera posición ocupada del vector, respectivamente. Inicialmente, `primera_libre = primera_ocupada = 0`.

La hebra productora escribirá en la posición `vec[primera_libre]` y después incrementará el valor de `primera_libre`.

La hebra consumidora leerá de la posición `vec[primera_ocupada]` y después incrementará el valor de `primera_ocupada`.

Cuando las variables `primera_libre` o `primera_ocupada` alcancen el valor `tam_vec`, se reiniciarán a 0. Esto se consigue mediante la operación módulo (`%`).

## Implementación FIFO en C++11


### Función hebra productora

```c++
void  funcion_hebra_productora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato = producir_dato() ;

      // Inicio SC
      libres.sem_wait(); // Producir tantos datos como elementos libres
      op_buffer.sem_wait(); // La inserción y extracción del buffer deben ocurrir
                            // en exclusión mutua.
      // Inserción del dato
      assert(0 <= primera_libre && primera_libre < tam_vec);
      buffer[primera_libre] = dato;
      cout << "inserción en buffer: " << buffer[primera_libre] << endl;
      primera_libre = (primera_libre +1) % tam_vec;
      mostrar_buffer();

      op_buffer.sem_signal();
      ocupadas.sem_signal();
      // Fin SC
   }
}

```

### Función hebra consumidora

```c++
void funcion_hebra_consumidora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato ;

      // Inicio SC
      ocupadas.sem_wait(); // Consumir cuando haya al menos un elemento en el buffer
      op_buffer.sem_wait(); // Extracción debe estar en exclusión mutua con la inserción.

      // Extracción del dato
      assert(0 <= primera_ocupada && primera_ocupada < tam_vec);
      dato = buffer[primera_ocupada];
      buffer[primera_ocupada] = 0;
      primera_ocupada = (primera_ocupada +1) % tam_vec;
      cout << "extraido de buffer: " << dato << endl;
      mostrar_buffer();

      op_buffer.sem_signal(); // Fin de la operacion
      libres.sem_signal(); // Se ha liberado un elemento del buffer.
      // Fin SC

      consumir_dato(dato) ;
    }
}
```

# Corrección del programa

Verificaremos mediante la función `test_contadores` que el número de veces que se produce un número natural es igual al número de veces que se consume dicho número.

```c++
void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}
```

## Resultado de ejecución

En ambos casos, el test de contadores se ha pasado correctamente.

### LIFO

```bash
[...]
fin
comprobando contadores ....
solución (aparentemente) correcta.
```

### FIFO

```bash
[...]
fin
comprobando contadores ....
solución (aparentemente) correcta.
```


# Código fuentes

Se adjuntan los códigos fuentes de los programas en C++11 en una carpeta `scd-p1-fuente``

## Compilación y ejecución

Para compilar los programas, se ha creado un `Makefile` que permite compilar los programas mediante el comando `make`.

```bash
$ make prodcons-lifo_exe
$ ./prodcons-fifo_exe

$ make prodcons-fifo_exe
$ ./prodcons-fifo_exe
```