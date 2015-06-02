/*
 *  kernel/kernel.c
 *
 *  Minikernel. Versión 1.0
 *
 *  Fernando Pérez Costoya
 *
 */

/*
 *
 * Fichero que contiene la funcionalidad del sistema operativo
 *
 */

#include "kernel.h"	/* Contiene defs. usadas por este modulo */



/*
* Practica 1 - Mostrar la llista de processos
*/ 

static void muestra_lista(lista_BCPs* lista){
	BCP* head = lista->primero;
	if (head == NULL){
		printk("No hay procesos en la lista.\n");
	}
	while (head != NULL){
		printk("-> Proceso id: %d , ticks: %d, rodaja: %d , vueltas: %d, estado: %d \n", head->id, head->ticks, head->rodaja, head->vueltas, head->estado);
		head = head->siguiente;
	}
}

static void muestra_todas_listas(){
	printk("\n");
	printk("LISTOS:");
	muestra_lista(&lista_listos);
	printk("DORMIDOS:");
	muestra_lista(&lista_dormidos);
	printk("ESPERANDO:");
	muestra_lista(&lista_espera);
	printk("\n");
}

static void print_prueba(){
	printk("\n\n\nESTOY AQUI\n\n\n");
}


/*
 *
 * Funciones relacionadas con la tabla de procesos:
 *	iniciar_tabla_proc buscar_BCP_libre
 *
 */

/*
 * Función que inicia la tabla de procesos
 */
static void iniciar_tabla_proc(){
	int i;

	for (i=0; i<MAX_PROC; i++)
		tabla_procs[i].estado=NO_USADA;
}

/*
 * Función que busca una entrada libre en la tabla de procesos
 */
static int buscar_BCP_libre(){
	int i;

	for (i=0; i<MAX_PROC; i++)
		if (tabla_procs[i].estado==NO_USADA)
			return i;
	return -1;
}

/*
 *
 * Funciones que facilitan el manejo de las listas de BCPs
 *	insertar_ultimo eliminar_primero eliminar_elem
 *
 * NOTA: PRIMERO SE DEBE LLAMAR A eliminar Y LUEGO A insertar
 */

/*
 * Inserta un BCP al final de la lista.
 */
static void insertar_ultimo(lista_BCPs *lista, BCP * proc){
	if (lista->primero==NULL)
		lista->primero= proc;
	else
		lista->ultimo->siguiente=proc;
	lista->ultimo = proc;
	proc->siguiente=NULL;
}

/*
 * Inserta un BCP en la segundo posicion.
 */
static void insertar_segundo(lista_BCPs *lista, BCP * proc){
	if (lista->primero==NULL){
		lista->primero = proc;
		lista->ultimo = proc;
		proc->siguiente = NULL;
	} else if (lista->primero->siguiente == NULL){
		lista->primero->siguiente = proc;
		proc->siguiente = NULL;
		lista->ultimo = proc;
	} else {
		BCP* head_next = lista->primero->siguiente;
		lista->primero->siguiente = proc;
		proc->siguiente = head_next;
	}
}

/*
 * Elimina el primer BCP de la lista.
 */
static void eliminar_primero(lista_BCPs *lista){

	if (lista->ultimo==lista->primero)
		lista->ultimo=NULL;
	lista->primero=lista->primero->siguiente;
	
}

/*
 * Elimina un determinado BCP de la lista.
 */
static void eliminar_elem(lista_BCPs *lista, BCP * proc){
	BCP *paux=lista->primero;

	if (paux==proc)
		eliminar_primero(lista);
	else {
		for ( ; ((paux) && (paux->siguiente!=proc));
			paux=paux->siguiente);
		if (paux) {
			if (lista->ultimo==paux->siguiente)
				lista->ultimo=paux;
			paux->siguiente=paux->siguiente->siguiente;
		}
	}
}

/*
 *
 * Funciones relacionadas con la planificacion
 *	espera_int planificador
 */

/*
 * Espera a que se produzca una interrupcion
 */
static void espera_int(){
	int nivel;

	printk("-> NO HAY LISTOS. ESPERA INT\n");

	/* Baja al mínimo el nivel de interrupción mientras espera */
	nivel=fijar_nivel_int(NIVEL_1);
	halt();
	fijar_nivel_int(nivel);
}

/*
 * Función de planificacion que implementa un algoritmo FIFO.
 */
static BCP * planificador(){
	while (lista_listos.primero==NULL)
		espera_int();		/* No hay nada que hacer */
	return lista_listos.primero;
}

/*
* Practica 1 - Desbloquear procesos
*/
static void desbloquear (BCP* proc, lista_BCPs* lista){
	(proc->estado) = LISTO;
	eliminar_elem(lista, proc);
	if ((proc->rodaja) > 0){
		insertar_segundo(&lista_listos, proc);
	} else {
		(proc->rodaja) = TICKS_POR_RODAJA;
		insertar_ultimo(&lista_listos, proc);
	}
}

/**
 * Practica 3 - Tratar el padre
 */
static void tratar_padre(){
	tabla_procs[p_proc_actual->ppid].num_hijos--;
	
	if (p_proc_actual->id != 0 &&
	  (tabla_procs[p_proc_actual->ppid].num_hijos <= 0) && 
	  (tabla_procs[p_proc_actual->ppid].estado == ESPERANDO)){
		desbloquear(&tabla_procs[p_proc_actual->ppid], &lista_espera);
	}
}

/*
 * Practica 2 - Cambios de contexto voluntarios e involuntarios
 * Antes se llamaba bloquear -> para dormir procesos
 * Ahora debe hacer más cosas
 */
static void cambio_proceso (lista_BCPs* lista){
	int nivel = fijar_nivel_int(NIVEL_3);
	
	BCP* proc = p_proc_actual;
	
	eliminar_primero(&lista_listos);
		
	if(lista == &lista_dormidos){ // Dormir
		(proc->estado)=BLOQUEADO;
		insertar_ultimo(lista,proc);
	}
	else if(lista == &lista_espera){ // Espera
		(proc->estado)=ESPERANDO;
		insertar_ultimo(lista,proc);
	}
	else if(lista == &lista_listos){ // Cambio
		(proc->estado)=LISTO;
		insertar_ultimo(lista,proc);
	}
	else if(lista == NULL){ // Liberar
		liberar_imagen(proc->info_mem); 
		(proc->estado)=TERMINADO;
		tratar_padre();
		int i;
		for (i=0; i<MAX_PROC; i++){
			if((tabla_procs[i].ppid == proc->id) && (tabla_procs[i].estado != TERMINADO)){
				tabla_procs[i].ppid = 0;
			}
		}
		liberar_pila(proc->pila);
	}
	
	p_proc_actual = planificador();
	(p_proc_actual->estado) = EJECUCION;
	
	printk("-> C.CONTEXTO POR FIN: de %d a %d\n",
		proc->id, p_proc_actual->id);
	
	fijar_nivel_int(nivel);
	
	if (lista != NULL){
		cambio_contexto(&(proc->contexto_regs), &(p_proc_actual->contexto_regs));
	} else {
		cambio_contexto(NULL, &(p_proc_actual->contexto_regs));
	}
}

/*
 *
 * Funcion auxiliar que termina proceso actual liberando sus recursos.
 * Usada por llamada terminar_proceso y por rutinas que tratan excepciones
 *
 */
static void liberar_proceso(){
	cambio_proceso(NULL);
}

/*
* Practica 1 - Ajustar Dormidos
*/
static void ajustar_dormidos (){
	BCP* head = lista_dormidos.primero;
	while(head != NULL){
		(head->ticks)--;
		BCP* head2 = head->siguiente;
		if ((head->ticks) == 0){
			desbloquear(head, &lista_dormidos);
		}
		head = head2;
	}
}

/*
 * Practica 2 - Actualiza la rodaja de tiempo y al final de esta, ejecuta una interrupción de software
 */
static void actualizar_rodaja(){
	if (p_proc_actual != NULL && (p_proc_actual->estado) == EJECUCION){
		(p_proc_actual->rodaja)--;
		if ((p_proc_actual->rodaja)<=0){
			replanificacion_pendiente = 1;
			activar_int_SW();
		}
	}
}

/*
 *
 * Funciones relacionadas con el tratamiento de interrupciones
 *	excepciones: exc_arit exc_mem
 *	interrupciones de reloj: int_reloj
 *	interrupciones del terminal: int_terminal
 *	llamadas al sistemas: llam_sis
 *	interrupciones SW: int_sw
 *
 */

/*
 * Tratamiento de excepciones aritmeticas
 */
static void exc_arit(){

	if (!viene_de_modo_usuario())
		panico("excepcion aritmetica cuando estaba dentro del kernel");


	printk("-> EXCEPCION ARITMETICA EN PROC %d\n", p_proc_actual->id);
	liberar_proceso();

        return; /* no debería llegar aqui */
}

/*
 * Tratamiento de excepciones en el acceso a memoria
 */
static void exc_mem(){

	if (!viene_de_modo_usuario())
		panico("excepcion de memoria cuando estaba dentro del kernel");


	printk("-> EXCEPCION DE MEMORIA EN PROC %d\n", p_proc_actual->id);
	liberar_proceso();

        return; /* no debería llegar aqui */
}

/*
 * Tratamiento de interrupciones de terminal
 */
static void int_terminal(){

	printk("-> TRATANDO INT. DE TERMINAL %c\n", leer_puerto(DIR_TERMINAL));

        return;
}

/*
 * Tratamiento de interrupciones de reloj
 */
static void int_reloj(){  
	printk("-> TRATANDO INT. DE reloj \n");
	actualizar_rodaja();
	ajustar_dormidos();
}

/*
 * Tratamiento de llamadas al sistema
 */
static void tratar_llamsis(){
	int nserv, res;

	nserv=leer_registro(0);
	if (nserv<NSERVICIOS)
		res=(tabla_servicios[nserv].fservicio)();
	else
		res=-1;		/* servicio no existente */
	escribir_registro(0,res);
	return;
}

/*
 * Tratamiento de interrupciuones software
 */
static void int_sw(){
	printk("-> TRATANDO INT. SW\n");
	if (replanificacion_pendiente == 1){
		if((p_proc_actual->siguiente) == NULL){
			(p_proc_actual->rodaja) = TICKS_POR_RODAJA;
		} else {
			(p_proc_actual->vueltas)++;
			if ((p_proc_actual->vueltas) >= VUELTAS_MAX){
				(p_proc_actual->vueltas) = VUELTAS_INIT;
				(p_proc_actual->ticks) = (TICKS_POR_RODAJA * 3) / 4;
				cambio_proceso(&lista_dormidos);
			} else {
				(p_proc_actual->rodaja) = TICKS_POR_RODAJA / 2;
				cambio_proceso(&lista_listos);
			}
		}
	}
	return;
}

/*
 * Funcion auxiliar que crea un proceso reservando sus recursos.
 * Usada por llamada crear_proceso.
 *
 */
static int crear_tarea(char *prog){
	void * imagen, *pc_inicial;
	int error=0;
	int proc;
	BCP *p_proc;

	proc=buscar_BCP_libre();
	if (proc==-1)
		return -1;	/* no hay entrada libre */

	/* A rellenar el BCP ... */
	p_proc=&(tabla_procs[proc]);

	/* crea la imagen de memoria leyendo ejecutable */
	imagen=crear_imagen(prog, &pc_inicial);
	if (imagen)
	{
		p_proc->info_mem=imagen;
		p_proc->pila=crear_pila(TAM_PILA);
		fijar_contexto_ini(p_proc->info_mem, p_proc->pila, TAM_PILA,
			pc_inicial,
			&(p_proc->contexto_regs));
		p_proc->id=proc;
		p_proc->estado=LISTO;
		p_proc->rodaja=TICKS_POR_RODAJA;
		p_proc->vueltas=VUELTAS_INIT;
		p_proc->num_hijos=0;
		//NOTE Practica 3 -> asignando id del padre
		if(p_proc_actual){
			p_proc->ppid = p_proc_actual->id;
			p_proc_actual->num_hijos++;
		}		
		/* lo inserta al final de cola de listos */
		insertar_ultimo(&lista_listos, p_proc);
		error= 0;
	}
	else
		error= -1; /* fallo al crear imagen */

	return error;
}

/*
 *
 * Rutinas que llevan a cabo las llamadas al sistema
 *	sis_crear_proceso sis_escribir
 *
 */

/*
 * Tratamiento de llamada al sistema crear_proceso. Llama a la
 * funcion auxiliar crear_tarea sis_terminar_proceso
 */
int sis_crear_proceso(){
	char *prog;
	int res;

	printk("-> PROC %d: CREAR PROCESO\n", p_proc_actual->id);
	prog=(char *)leer_registro(1);
	res=crear_tarea(prog);
	return res;
}

/*
 * Tratamiento de llamada al sistema escribir. Llama simplemente a la
 * funcion de apoyo escribir_ker
 */
int sis_escribir(){
	char *texto;
	unsigned int longi;

	texto=(char *)leer_registro(1);
	longi=(unsigned int)leer_registro(2);

	escribir_ker(texto, longi);
	return 0;
}

/*
 * Tratamiento de llamada al sistema terminar_proceso. Llama a la
 * funcion auxiliar liberar_proceso
 */
int sis_terminar_proceso(){
	printk("-> FIN PROCESO %d\n", p_proc_actual->id);

	liberar_proceso();

        return 0; /* no debería llegar aqui */
}

/*
 * Practica 1 - sis_dormir
 */
int sis_dormir(){
	long int num_ticks = (long int)leer_registro(1);
 	if (p_proc_actual != NULL){
		num_ticks *= TICK;
		p_proc_actual->ticks = num_ticks;
		cambio_proceso(&lista_dormidos);
	}
	return 0;
}

/*
 * Practica 0 - retornar el identificador
 */
int get_pid() {
  return p_proc_actual->id;
}

/*
 * Practica 3 - Retornar identificador del procés pare
 */
int get_ppid(){
  return p_proc_actual->ppid;
}

/**
 * Practica 3 - Espera :
 *	return 0 -> han acabat d'executar tots els fills 
 *	return -1 -> encara els fills s'están executant
 */
int espera(){
	if (p_proc_actual->num_hijos > 0){
		cambio_proceso(&lista_espera);
		return 0;
	} else {
		return -1;
	}
}

/*
 * Rutina de inicialización invocada en arranque
 */
int main(){
	/* se llega con las interrupciones prohibidas */
	iniciar_tabla_proc();

	instal_man_int(EXC_ARITM, exc_arit); 
	instal_man_int(EXC_MEM, exc_mem); 
	instal_man_int(INT_RELOJ, int_reloj); 
	instal_man_int(INT_TERMINAL, int_terminal); 
	instal_man_int(LLAM_SIS, tratar_llamsis); 
	instal_man_int(INT_SW, int_sw); 

	iniciar_cont_int();		/* inicia cont. interr. */
	iniciar_cont_reloj(TICK);	/* fija frecuencia del reloj */
	iniciar_cont_teclado();		/* inici cont. teclado */
	
	/* crea proceso inicial */
	if (crear_tarea((void *)"init")<0)
		panico("no encontrado el proceso inicial");
	
	/* activa proceso inicial */
	p_proc_actual=planificador();
	cambio_contexto(NULL, &(p_proc_actual->contexto_regs));
	/* NOTE Práctica 3 : asignación inicial como proceso padre con 0 hijos */
	p_proc_actual->ppid = -1;
	p_proc_actual->num_hijos = 0;
	
	panico("S.O. reactivado inesperadamente");
	return 0;
}