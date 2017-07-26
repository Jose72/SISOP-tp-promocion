TP2: Multitarea con desalojo
========================

static_assert
-------------
¿cómo y por qué funciona la macro static_assert que define JOS?

La macro valida en tiempo de compilacion si el parametro es valido. Lo que nos ahorraria tener un crash (assert false) en tiempo de ejecucion. 
Para impementarlo usa un switch case en donde, si no entra al caso que se le pasa por parametro cae en un error (un case sin concluir):

```
#define static_assert(x) switch (x) case 0: case (x):
```

Si x es 0 (falso) se tiene un switch con case duplicado, lo que provoca un error en tiempo de compilacion.
Por el contrario, si x es cualquier otro valor (true), el switch es valido.


env_return
----------
- al terminar un proceso su función umain() ¿dónde retoma la ejecución el kernel? Describir la secuencia de llamadas desde que termina umain() hasta que el kernel dispone del proceso.

Cuando finaliza el umain(), se llama a exit(), el cual invoca a sys_env_destroy(0). Es decir que destruye el env actual. Al destruir un env, el kernel cambia el estado del mismo, y llama a sys_yield, por lo cual continua con el siguiente env disponible.

- ¿en qué cambia la función env_destroy() en este TP, respecto al TP anterior?

Maneja distintos estados y multiples environments.


sys_yield
---------
- explicar la salida de make qemu-nox
[00000000] new env 00001000
[00000000] new env 00001001
[00000000] new env 00001002
-> En este punto fueron creados los 3 environments

Hello, I am environment 00001000.
-> El primer environment llamo a umain, y al entrar al loop se llama a sys_yield, por lo que cambia de env.

Hello, I am environment 00001001.
-> Debido a que el primer environment llamo a sys_yield el segundo env fue lanzado, y, al igual que el primero, llama a sys_yield.

Hello, I am environment 00001002.
-> En este caso, cuando llame a sys_yield lo tomara el primer env que fue frenado dentro del loop

Back in environment 00001000, iteration 0.
-> Este print demuestra que el primer environment habia sido frenado dentro del loop, y que retoma su ejecucion desde alli. 
A continuacion los env seguiran llamando a sys_yield hasta que terminen de iterar 5 veces.

Back in environment 00001001, iteration 0.
Back in environment 00001002, iteration 0.
Back in environment 00001000, iteration 1.
Back in environment 00001001, iteration 1.
Back in environment 00001002, iteration 1.
Back in environment 00001000, iteration 2.
Back in environment 00001001, iteration 2.
Back in environment 00001002, iteration 2.
Back in environment 00001000, iteration 3.
Back in environment 00001001, iteration 3.
Back in environment 00001002, iteration 3.
Back in environment 00001000, iteration 4.

All done in environment 00001000.
-> Al terminar las iteraciones, el primer env finaliza su umain, iniciando el proceso de desalojo.

[00001000] exiting gracefully
[00001000] free env 00001000
-> El primer environment fue destruido y liberado.
Al destruirse el primer env, el kernel llama a sys_yield para corroborar que no haya otros procesos esperando a retomar su ejecucion (o comenzarla).

Back in environment 00001001, iteration 4.
-> El kernel encuentra que el segundo env estaba esperando y lo lanza nuevamente.

All done in environment 00001001.
[00001001] exiting gracefully
[00001001] free env 00001001
-> El segundo env termina su ejecucion al igual que el primero, y nuevamente el kernel llama, luego de liberar el env, a sys_yield.

Back in environment 00001002, iteration 4.
All done in environment 00001002.
[00001002] exiting gracefully
[00001002] free env 00001002
-> El kernel encuentra al tercer env y retoma su ejecucion.
El env termina y el kernel destruye y libera el ultimo env.
Al igual que antes llama a sys_yield para corroborar si hay nuevos env para ejecutar.

No runnable environments in the system!
-> Al no haberse lanzado nuevos environments, el scheduler llama a halt y finaliza.


contador_env
------------
- ¿qué ocurrirá con esa página en env_free() al destruir el proceso?

El pp->ref de la pag se incrementa en page_insert (en cada env_setup_vm), y se decrementa en page_decref (con cada env_free). Al final, pp->ref llegara a 0 y la pagina se añade a la lista de pags libres. 

- ¿qué código asegura que el buffer VGA físico no será nunca añadido a la lista de páginas libres?

Habria que inicializar el pp->ref de la pagina a 1 (u otro valor mayor) para que nunca llegue a 0.


envid2env
---------
- en JOS, si un proceso llama a sys_env_destroy(0)

Destruye el env actual

- en Linux, si un proceso llama a kill(0, 9)

Destruye a todos los procesos que pertenecen al mismo grupo del proceso llamador.

- JOS: sys_env_destroy(-1)

Busca el env con envid2env, en la pos ENVX(-1) = 1023 del array de envs, si el env no fue creado retorna error, sino se destruye.

- Linux: kill(-1, 9)

Destruye a todos los procesos que pueden escuchar interrupciones del proceso llamador (exeptuando al proceso init).


dumbfork
--------
- Si, antes de llamar a dumbfork(), el proceso se reserva a sí mismo una página con sys_page_alloc() ¿se propagará una copia al proceso hijo? ¿Por qué?

Depende, el dumbfork copia el espacio de direcciones por encima de UTEXT hasta end, y sys_page_alloc() permite alocar por debajo de UTOP, mientras este entre esos se propagara, si la pagina fue mapeada por debajo de UTEXT (por ej en UTEMP) no sera copiada.

- ¿Se preserva el estado de solo-lectura en las páginas copiadas? Mostrar, con código en espacio de usuario, cómo saber si una dirección de memoria es modificable por el proceso, o no.

No, en duppage cuando se aloca la pagina y se mapea en el dstenv, se hace siempre con permiso de escritura.

- Describir el funcionamiento de la función duppage().

Parametros: envid_t (dstenv) y una va (addr)
-reserva una pagina, y la mapea a la va addr (en el espacio de direcciones de dstenv)
-hace que la va UTEMP (en el espacio de direcciones del env actual) y mapee a la misma pagina fisica que la va addr (en el espacio de direcciones de dstenv)
-copia la pagina mapeada en la va addr (en el espacio de direcciones del env actual) a la va UTEMP (en el espacio de direcciones del env actual)
-desmapea la va UTEMP (en el espacio de direcciones del env actual)

De esta forma queda un pagina mapeada a la direccion addr en el espacio de direcciones de dstenv, con el mismo contenido que la pagina mapeada en la direccion addr en el espacio de direcciones del env actual


- Supongamos que se añade a duppage() un argumento booleano que indica si la página debe quedar como solo-lectura en el proceso hijo:
indicar qué llamada adicional se debería hacer si el booleano es true
describir un algoritmo alternativo que no aumente el número de llamadas al sistema, que debe quedar en 3 (1 × alloc, 1 × map, 1 × unmap).
   
Una llamada adicional a sys_page_map

sys_page_map(dstenv, addr, dstenv, addr, PTE_P|PTE_U) 

Alternativa para no hacer mas llamadas al sistema:
Se chequea el boleano, si es con escritura, realizamos el duppage que esta ahi, sino hacer sys_page_map(0, addr, dstenv, addr, PTE_P|PTE_U) 
Asi addr mapea a la misma pagina fisica desde los 2 espacios de direcciones, como es solo lectura la comparten.

- ¿Por qué se usa ROUNDDOWN(&addr) para copiar el stack? ¿Qué es addr y por qué, si el stack crece hacia abajo, se usa ROUNDDOWN y no ROUNDUP?

addr es una variable local de fork_v0, al hacer &addr tenemos un puntero al stack de fork_v0. Se debe copiar desde &addr hasta el inicio del stack (el stack crece hacia las direcciones bajas, pero lo copiamos desde las direcciones bajas hacia arriba), por eso se usa ROUNDOWN, si usaramos ROUNDUP habria parte de la memoria que no seria copiada


contador_fork
-------------
- ¿Funciona? ¿Qué está ocurriendo con el mapping de VGA_USER? ¿Dónde hay que arreglarlo?

No. Al crear un nuevo env, se aloca una nueva pagina, se copia el contenido de la pagina del buffer VGA y se mapea a la direccion virtual. Esto esta mal, el mapeo del buffer VGA ya se hizo en env_setup_vm, no es necesario hacer nada con la pagina.
En dup_or_share se puede solucionar, haciendo que esa pagina no sea mapeada de nuevo, simplemente hay que saltearla.

- ¿Podría fork() darse cuenta, en lugar de usando un flag propio, mirando los flags PTE_PWT y/o PTE_PCD? (Suponiendo que env_setup_vm() los añadiera para VGA_USER.)

Si, habria que agregar el chequeo en fork para las paginas que contengan esos flags sean salteadas cuando haga la copia.



Ejecución en paralelo (multi-core) 
----------------------------------
- It seems that using the big kernel lock guarantees that only one CPU can run the kernel code at a time. Why do we still need separate kernel stacks for each CPU? Describe a scenario in which using a shared kernel stack will go wrong, even with the protection of the big kernel lock.
En el caso que haya una interrupcion del hardware.

