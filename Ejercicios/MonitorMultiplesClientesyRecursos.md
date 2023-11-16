---
title: "Monitor SU para múltiples clientes y recursos"
subtitle: "Sistemas Concurrentes y Distribuidos. DGIIM"
author: ["Ricardo Ruiz Fernández de Alba"]
date: "08/11/2023"
subject: "Sistemas Concurrentes y Distribuidos"
titlepage: true
titlepage-background: "background1.pdf"
# titlepage-color: "3C9F53"
# titlepage-text-color: "FFFFFF"
# titlepage-rule-color: "FFFFFF"
# titlepage-rule-height: 2
---

Proponemos el siguiente monitor para resolver el problema:

```
Monitor Recursos;
var
    libre: array[1,...,m] of boolean;
    peticion: array[1,...,n] of boolean;
    I: integer; // Índice del último recurso liberado
    no_peticion: condition;

procedure pedir(id_proceso: 1,...,n);
var k: integer; k := 1;
begin
    peticion[id_proceso] := true;
    while (not libre[k] and k < m ) do
        k := k + 1;
    end
    if (not libre[k]) then
        while(peticion[id_proceso]) do
            no_peticion.signal() // Desbloqueo en cadena

            {IM ^ L}
            no_peticion.wait();
            {C[id_proceso] ^ L}  // Se cumple la condición de sincronización. Se comprueba si hay otras peticiones prioritarias (menor índice de proceso)
        end
    else I := k;
    end
    libre[I] := false;
end

procedure devolver(id_recurso: 1,...,m);
var j: integer; j := 1;
begin
    while (not peticion[j] and j < n) do
        j := j + 1;
    end
    I := id_recurso;
    if (petición[j]) then
        {no_vacio(no_peticion) ^ L ^ C[j]}
        peticion[j] := false;
        no_peticion.signal();
        {IM ^ L}
    else libre[I] := true;
    end
end
```

\newpage

Código de inicialización
```
begin
{V}
    for i := 1 to m do
        libre[i] := true;
    end
    for i := 1 to n do
        peticion[i] := false;
    end
{IM ^ L}
end
```

Así pues, los procesos clientes $P_i$ llamarán al procedimiento pedir con parámetro el identificador $i$ del proceso.

Este procedimiento busca en el array de libres el primer recurso disponible. Si todos los recursos están ocupados, se termina con `k=m` y `libre[k]=false`.
En ese caso, se quqeda bloqueado en la cola de la variable condición no_peticion y con el valor `peticion[id_proceso] = true` hasta que haya un recurso disponible.

Cuando otro proceso llame al procedimiento devolver con parámetro el número de recurso, almacenará `I := id_recurso` como el último recurso liberado y se busca la primera petición pendiente, actualizando `peticion[j] := false` y señalando al proceso que tiene petición pendiente.

Como el monitor es de Señales Urgentes (SU), el proceso que espera en la cola se desbloquea, comprobando de nuevo si es el que cumple `C[id_proceso]` la condición lógica de sincronización. Así, desbloquea en cadena otros posibles procesos en la cola, y sale del while aquel con mayor prioridad (menor id de proceso). Finalmente, se escribiría `libre[I] := false`.

Si por el contrario hay procesos libres, `pedir(id_proceso)` actualiza `I := k` y \newline `libre[I] := false`.

Igualmente, si no hay peticiones pendientes, `devolver(id_recurso)` actualiza 
\newline
`I := id_recurso` y `libre[I] := true`, dejando `id_recurso` libre para futuras peticiones de procesos.

Finalmente, se ha verificado formalmente que el invariante del monitor se cumple tras la inicialización, antes y después de cada procedimiento, y para la operacion `signal`, se cumple 
\newline $\{no\_vacio(peticion) \land C[j] \land L \}$ como precondicion e $\{IM \land L \}$ como poscondición. 

Y $\{IM \land L \}$ como precondición para la operación  `wait` y $\{C[id\_proceso] \land L \}$ como poscondición.

\newpage

Una ejecución del monitor para 4 procesos y 3 recursos sería:


```
process P1
begin
Recursos.pedir(1);
end

process P2
begin
Recursos.pedir(2);
end

Process P3
begin
Recursos.pedir(3);
end

cobegin
Recursos.pedir(4) | Recursos.pedir(1) | Recursos.devolver(1) | Recursos.devolver(2)
coend
````

Una posible interfoliación sería, que tras pedir 3 recursos, el array de libres quedaría ocupados, haciendo que P4 tenga dos procesos en la cola `no_peticion` hasta que se libere un recurso. 

Una vez se ejecute el procedimiento devolver , el proceso 4 se desbloquearía, desbloqueando a su vez al proceso 1, que al tener menor índice de proceso, se ejecutaría primero. Por tanto, se devolvería el primer recurso que quedaría ocupado de nuevo. El proceso 4 se bloquearía de nuevo hasta que se libere el segundo recurso. Una vez este liberado, se ocuparía de nuevo terminando la ejecución del monitor.