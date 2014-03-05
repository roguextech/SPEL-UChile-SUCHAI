/**
 * @file  cmdPayload.h
 * @author Tomas Opazo T
 * @date 2013
 * @copyright GNU Public License.
 *
 * Este header contiene las definiciones de comandos asociados a los diferentes
 * payloads. La logica de trabajo de los payloads es: tomar datos y guardarlos
 * en memoria externa de manera periodica hasta que la memoria se llena. Una vez
 * que su buffer se vacia, producto de enviar la informacion como telemtria,
 * retoma su funcionamiento.
 * Los comandos permiten controlar la logica de la maquina de estados de cada
 * payload, funciones de debugm asi como ejecutar de manera independiente cada uno.
 */

#ifndef CMDPAYLOAD_H
#define	CMDPAYLOAD_H

#include "FreeRTOS.h"
#include "queue.h"

#include "langmuir.h"
#include "fis_payload.h"
#include "camera.h"
#include "dig_gyro.h"
#include "sensTemp.h"

#include "console.h"

#include "cmdIncludes.h"
#include "dataRepository.h"

/**
 * Lista de comandos disponibles.
 */
typedef enum{
    pay_id_debug=((unsigned int)(CMD_PAY)*0x100), ///< @cmd_first

    pay_id_FSM_default, ///< @cmd

    pay_id_init_sensTemp, ///< @cmd
    pay_id_take_sensTemp, ///< @cmd
    pay_id_stop_sensTemp, ///< @cmd

    pay_id_init_gyro, ///< @cmd
    pay_id_take_gyro, ///< @cmd
    pay_id_stop_gyro, ///< @cmd

    pay_id_init_camera, ///< @cmd
    pay_id_take_camera, ///< @cmd
    pay_id_stop_camera, ///< @cmd

    pay_id_init_gps, ///< @cmd
    pay_id_take_gps, ///< @cmd
    pay_id_stop_gps, ///< @cmd

    pay_id_init_expFis, ///< @cmd
    pay_id_take_expFis, ///< @cmd
    pay_id_stop_expFis, ///< @cmd

    pay_id_init_test1, ///< @cmd
    pay_id_take_test1, ///< @cmd
    pay_id_stop_test1, ///< @cmd

    pay_id_init_test2, ///< @cmd
    pay_id_take_test2, ///< @cmd
    pay_id_stop_test2, ///< @cmd

    pay_id_init_lagmuirProbe, ///< @cmd
    pay_id_take_lagmuirProbe, ///< @cmd
    pay_id_stop_lagmuirProbe, ///< @cmd
    pay_id_debug_langmuirProbe, ///< @cmd
    pay_id_send_to_lagimur, ///< @cmd
            
    //*********************
    pay_id_last_one    //Elemento sin sentido, solo se utiliza para marcar el largo del arreglo
}PAY_CmdIndx;

#define PAY_NCMD ((unsigned char)pay_id_last_one)
void pay_onResetCmdPAY(void);


typedef enum{
    pay_state_pre_init=0,   ///< estado de partida (condicion inicial), no hace cosas
    pay_state_init,         ///< estado de config del Payload, lo inicializa (init)
    pay_state_take,         ///< estado de operacion nominal del Payload, toma datos
    pay_state_stop,         ///< estado de termino del ciclo del Payload,
                            ///< configuraciones de termino y post procesamiento
}PAY_State;

PAY_State pay_nextStateLogic(PAY_State pay_sem, DAT_Payload pay_i);
void pay_currentStateLogic(PAY_State pay_sem_state, DAT_Payload pay_i);

//Comandos
//Debug
int pay_debug(void *param);
//comandos para inicializar/realizar los experimientos
int pay_FSM_default(void *param);
//test1
int pay_init_test1(void *param);
int pay_take_test1(void *param);
int pay_stop_test1(void *param);
//test2
int pay_init_test2(void *param);
int pay_take_test2(void *param);
int pay_stop_test2(void *param);
//expFis
int pay_init_expFis(void *param);
int pay_take_expFis(void *param);
int pay_stop_expFis(void *param);
//sensTemp
int pay_take_sensTemp(void *param);
int pay_init_sensTemp(void *param);
int pay_stop_sensTemp(void *param);
//LagmiurProbe
int pay_take_lagmuirProbe(void *param);
int pay_init_lagmuirProbe(void *param);
int pay_stop_lagmuirProbe(void *param);
int pay_send_to_lagimur(void *param);
int pay_debug_langmuir(void *param);
//Gyro
int pay_take_gyro(void *param);
int pay_init_gyro(void *param);
int pay_stop_gyro(void *param);
//tmEstado
int pay_init_tmEstado(void *param);
int pay_take_tmEstado(void *param);
int pay_stop_tmEstado(void *param);
//Camera
int pay_take_camera(void *param);
int pay_init_camera(void *param);
int pay_stop_camera(void *param);
//GPS
int pay_take_gps(void *param);
int pay_init_gps(void *param);
int pay_stop_gps(void *param);


#endif	/* CMDPAYLOAD_H */
