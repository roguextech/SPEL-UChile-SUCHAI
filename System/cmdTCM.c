#include "cmdTCM.h"

cmdFunction tcmFunction[TCM_NCMD];
int tcm_sysReq[TCM_NCMD];

char beacon_buff[50] = "SUCHAIATINGDOTUCHILEDOTCL-";
char *p_beacon_buff = beacon_buff+26;//+strlen(beacon_buff);

void tcm_onResetCmdTCM(void){
    int i;
    for(i=0; i<TCM_NCMD; i++) tcm_sysReq[i] = CMD_SYSREQ_MIN+SCH_TCM_SYS_REQ;

    tcmFunction[(unsigned char)tcm_id_testframe] = tcm_testframe;
    tcmFunction[(unsigned char)tcm_id_resend] = tcm_resend;

    //sendTM Payload
    tcmFunction[(unsigned char)tcm_id_sendTM_all_pay_i] = tcm_sendTM_all_pay_i;
    tcmFunction[(unsigned char)tcm_id_sendTM_pay_i] = tcm_sendTM_pay_i;

    //dat_CubesatVar
    tcmFunction[(unsigned char)tcm_id_sendTM_cubesatVar] = tcm_sendTM_cubesatVar;

    //Beacon
    tcmFunction[(unsigned char)tcm_id_send_beacon] = tcm_send_beacon;
    tcm_sysReq[(unsigned char)tcm_id_send_beacon]  = CMD_SYSREQ_MIN+SCH_BCN_SYS_REQ;

    tcmFunction[(unsigned char)tcm_id_set_sysreq] = tcm_set_sysreq;
    tcm_sysReq[(unsigned char)tcm_id_set_sysreq] = CMD_SYSREQ_MIN;
}

/**
 * Send an message for testing
 *
 * @param param (1)Verbose, (0)No Verbose
 * @return 1 - OK
 */
int tcm_testframe(void *param)
{
    int result;
    unsigned char testframe[40] = "Proyecto SUCHAI - suchai@ing.uchile.cl\n\0";
    //unsigned char testframe[80] = "Proyecto SUCHAI - suchai@ing.uchile.cl\n\0Proyecto SUCHAI - suchai@ing.uchile.cl\n\0";

    if(*(int *)param)
    {
        con_printf("Sending test frame: ");
        con_printf((char *)testframe);
    }

    result = TRX_LoadTelemetry(testframe, 40) ? TRX_SendTelemetry() : 0;
    //result = trx_LoadTelemetry(testframe, 80) ? trx_SendTelemetry() : 0;
    return result;
}

/**
 * Resend the telemetry buffer
 *
 * @param param (1)Verbose, (0)No Verbose
 * @return 1 - OK; 0 - Fail
 */
int tcm_resend(void *param)
{
    int result = 0;

    /* Get position of last sent data */
    unsigned char outl = TRX_ReadRegister(TRX_TMTF_OUT_L);
    unsigned char outh = TRX_ReadRegister(TRX_TMTF_OUT_H);

    /* Get position of last buffered data*/
    unsigned char inl = TRX_ReadRegister(TRX_TMTF_IN_L);
    unsigned char inh = TRX_ReadRegister(TRX_TMTF_IN_H);

    /* Resend only if all buffered data was sent */
    if((outl == inl) && (outh == inh))
    {
        /* Reset position of last telemtry */
        if(*(int *)param) con_printf("Reseting buffer pointer...");
        TRX_WriteRegister(TRX_TMTF_OUT_H, 0);
        TRX_WriteRegister(TRX_TMTF_OUT_L, 0);

        /* Now resend the telemetry buffer */
        if(*(int *)param) con_printf("Resending telemetry buffer...");
        result = TRX_SendTelemetry();
    }

    return result;
}
/**
 * Envia la TM de TODOS los DAT_Payload pay_i usado como arumento (param)
 * Toma el buffer de dat_get_PayloadVar(pay_i, ..) y la transmite por el TRX
 *
 * @param param no usado
 * @return 1 succes, 0 failure
 */
int tcm_sendTM_all_pay_i(void *param){

    //envio TM de payload
    DAT_Payload pay_i;
    for(pay_i=0; pay_i<dat_pay_last_one; pay_i++)
    {
        tcm_sendTM_pay_i( (void *)(&pay_i) );
        ClrWdt();
    }

    return 1;
}
/**
 * Envia la TM de un DAT_Payload pay_i usado como arumento (param)
 * Toma el buffer de dat_get_PayloadVar(pay_i, ..) y la transmite por el TRX
 *
 * @param param puntero a DAT_Payload con el pay_i del cual enviar su buffer
 * @return 1 succes, 0 failure
 */
int tcm_sendTM_pay_i(void *param){
    con_printf("tcm_sendTM_pay_i\r\n");
    
    DAT_Payload pay_i = *((DAT_Payload*)param);
    int mode=2;
    DAT_CubesatVar dat_pay_xxx_perform=dat_pay_i_to_performVar(pay_i);

    int res = tcm_sendTM_PayloadVar(mode, pay_i);
    if(res!=0x0000){
        //inicia nuevamente el ciclo del Payload
        dat_setCubesatVar(dat_pay_xxx_perform, 0x0001 );
    }
    return res;
}

/**
 * Reads and transmit telemetry ralated to satellite's status
 *
 * Envia telemetria instantanea (en ese momento) del estado del SUCHAI.
 * Envia por TM todas las variables de estado de cubesatVar
 *
 * TMID : 0x0000
 *
 * @param param 0 - Only store, 1 - Only Send, 2 - Store and send @deprecated @sa trx_tm_addtoframe()
 * @return 0 (Tx fail) - 1 (Tx OK)
 */
int tcm_sendTM_cubesatVar(void *param)
{
    int tm_id = 0x0000;
//    int mode = *((int *)param);

    /* Start a new session (Single or normal) */
    tm_id = dat_pay_last_one; /* TM ID */ /*Para distingur cubesatVar de los Paylaods*/
    trx_tm_addtoframe(&tm_id, 0, CMD_ADDFRAME_FIN);   /* Close previos sessions */
    trx_tm_addtoframe(&tm_id, 1, CMD_ADDFRAME_START); /* New empty start frame */

    /* Read info and append to the frame */
    DAT_CubesatVar indxVar;
    for(indxVar=0; indxVar<dat_cubesatVar_last_one; indxVar++){
        tm_id = dat_getCubesatVar(indxVar);
        trx_tm_addtoframe(&tm_id, 1, CMD_ADDFRAME_ADD);

        #if (SCH_CMDTCM_VERBOSE>=2)
            char buffer[10];
            con_printf("dat_CubesatVar[");
            //itoa(buffer, (unsigned int)indxVar, 10);
            sprintf( buffer, "%d", (unsigned int)indxVar );
            con_printf(buffer); con_printf("]=");
            //itoa(buffer,(unsigned int)dat_getCubesatVar(indxVar), 10);
            sprintf( buffer, "0x%X", (unsigned int)dat_getCubesatVar(indxVar) );
            con_printf(buffer); con_printf("\r\n");
        #endif
        }


    // Close session
    // data = trx_tm_addtoframe(&data, 0, CMD_ADDFRAME_STOP);     /* Empty stop frame */
    tm_id = trx_tm_addtoframe(&tm_id, 0, CMD_ADDFRAME_FIN);      /* End session */

    return tm_id;
}

/**
 * Este comando construye el beacon del satelite y lo envia. Dependiendo del
 * parametro, construye diferentes beacons que contienen el identificador del
 * proyecto e informacion base del sistema.
 *
 * @note Para una descripcion detallada de cada beacon, revisar la planilla en
 * linea.
 * https://docs.google.com/spreadsheet/ccc?key=0AlJNKX_r8AXcdFZwNURWdmZUVjNtM1RpdTlTRG9LV0E#gid=6
 *
 * @param param Tipo de beacon a construir y enviar.
 *          0 - Identificador
 *          1 - Identificador + Variables de estado
 *
 * @return 0-Fallo, 1-Exito
 */
int tcm_send_beacon(void *param)
{
    int mode = *((int *)param);
    int ok = 0;
    int val = -1;
    char buff[10];
    char *p_buff = p_beacon_buff;
    double d_val = 0;

    if(mode==0)
    {
        /* Tipo */
        itoa(buff, mode, 10);
//        strcpy(p_buff++, buff);
        strcpy(p_buff++, buff);
        ok = 1;
    }
    else if(mode == 1)
    {
        /* Tipo */
        itoa(buff, mode, 10);
        strcpy(p_buff++, buff);

        /* opMode */
        val = dat_getCubesatVar(dat_ppc_opMode);
        itoa(buff,val,10);
        strcpy(p_buff++, buff);

        /* hoursWithoutReset */
        val = dat_getCubesatVar(dat_ppc_hoursWithoutReset);
        itoa(buff,val/10,10);
        strcpy(p_buff++, buff);
        itoa(buff,val%10,10);
        strcpy(p_buff++, buff);
        
        /* resetCounter */
        val = dat_getCubesatVar(dat_ppc_resetCounter);
        itoa(buff,val/10,10);
        strcpy(p_buff++, buff);
        itoa(buff,val%10,10);
        strcpy(p_buff++, buff);

        /* ant_deployed */
        val = dat_getCubesatVar(dat_dep_ant_deployed);
        itoa(buff,val,10);
        strcpy(p_buff++, buff);

        /* ant_tries */
        val = dat_getCubesatVar(dat_dep_ant_tries);
        itoa(buff,val,10);
        strcpy(p_buff++, buff);

        /* hours */
        val = dat_getCubesatVar(dat_dep_hours);
        itoa(buff,val,36); //Map 0-36 to 0-Z
        strcpy(p_buff++, buff);

        /* minutes */
        val = dat_getCubesatVar(dat_dep_minutes);
        itoa(buff,val/10,10);
        strcpy(p_buff++, buff);
        itoa(buff,val%10,10);
        strcpy(p_buff++, buff);

        /* bat0_voltage */
        val = dat_getCubesatVar(dat_eps_bat0_voltage);
        d_val = -0.00939*val + 9.791;
        val = (int)(d_val*10.0); /* 7.4V -> 74 */
        itoa(buff,val/10,10);
        strcpy(p_buff++, buff);
        itoa(buff,val%10,10);
        strcpy(p_buff++, buff);

        /* bat0_tmp */
        val = dat_getCubesatVar(dat_eps_bat0_temp);
        d_val =  -0.163*val+110.338;
        val = (int)(d_val); /* 18.3C -> 18 */
        itoa(buff,val/10,10);
        strcpy(p_buff++, buff);
        itoa(buff,val%10,10);
        strcpy(p_buff++, buff);

        /* eps_soc */
        val = dat_getCubesatVar(dat_eps_soc);
        itoa(buff,val,10);
        strcpy(p_buff++, buff);

        /* eps_charging*/
        val = dat_getCubesatVar(dat_eps_charging);
        itoa(buff,val,10);
        strcpy(p_buff++, buff);

        /* rssi_mean */
        val = dat_getCubesatVar(dat_trx_rssi_mean);
        val = val < 0 ? -1*val:val;
        itoa(buff,val/10,10);
        strcpy(p_buff++, buff);
        itoa(buff,val%10,10);
        strcpy(p_buff++, buff);
        
        /* count_tm */
        val = dat_getCubesatVar(dat_trx_count_tm);
        itoa(buff,val/10,10);
        strcpy(p_buff++, buff);
        itoa(buff,val%10,10);
        strcpy(p_buff++, buff);
        
        /* count_tc */
        val = dat_getCubesatVar(dat_trx_count_tc);
        itoa(buff,val,36); //Map 0-36 to 0-Z
        strcpy(p_buff++, buff);
        
        /* msd_status */
        val = dat_getCubesatVar(dat_msd_status);
        itoa(buff,val,10);
        strcpy(p_buff++, buff);

        ok = 1;
    }
    else
    {
        ok = 0;
    }

#if SCH_CMDTCM_VERBOSE
    con_printf(">>Beacon:");
    con_printf(beacon_buff);
    con_printf("\n\r");
#endif

    unsigned int len = strlen(beacon_buff);
    TRX_SetBeaconContent((unsigned char*)beacon_buff, len);
//    TRX_BeaconAction(1);

    TRX_SetMode(TRX_MODE_ONLYBCN); //Triggers beacon tx

    return ok;
}

/**
 * Debug command that set the SysReq field to all TCM commands. If -1 is used
 * as argument then default values are restored.
 *
 * @param param new SysReq value. Use -1 to restore default values
 * @return 1 success.
 */
int tcm_set_sysreq(void *param)
{
    int new_sysreq = *((int *)param);
    int i = 0;

    // Resto to default values
    if(new_sysreq < 0)
    {
        //Default
        for(i=0; i<TCM_NCMD; i++) tcm_sysReq[i] = CMD_SYSREQ_MIN+SCH_TCM_SYS_REQ;

        //Special cases
        tcm_sysReq[(unsigned char)tcm_id_send_beacon]  = CMD_SYSREQ_MIN+SCH_BCN_SYS_REQ;
        tcm_sysReq[(unsigned char)tcm_id_set_sysreq] = CMD_SYSREQ_MIN;
    }
    //Set new value
    else
    {
        for(i=0; i<TCM_NCMD; i++)
        {
            tcm_sysReq[i] = new_sysreq;
        }

        //Do not change the sysReq of this command!
        tcm_sysReq[(unsigned char)tcm_id_set_sysreq] = CMD_SYSREQ_MIN;
    }

    return 1;
}

/* Funciones auxiliares */

/**
 * Reads and transmit telemetry ralated to payloads
 * @note param mode is deprecated
 * @param mode 0 - Only store, 1 - Only Send, 2 - Store and send @deprecated @sa trx_tm_addtoframe()
 * @param pay_i Payload id
 * @return 0 (Tx fail) - 1 (Tx OK)
 */
int tcm_sendTM_PayloadVar(int mode, DAT_Payload pay_i){
    con_printf("tcm_sendTM_PayloadVar...\r\n");

    int tm_id, nfrm;

    /* Start a new session (Single or normal) */
    tm_id = (int)pay_i; /* TM ID */
    nfrm = trx_tm_addtoframe(&tm_id, 0, CMD_ADDFRAME_FIN);   /* Close previos sessions */
    nfrm = trx_tm_addtoframe(&tm_id, 1, CMD_ADDFRAME_START); /* New empty start frame */

    /* Read info and append to the frame */
    //Add Payload Indxs info
    unsigned int maxIndx = dat_getMaxPayIndx( pay_i);
    unsigned int nextIndx = dat_getNextPayIndx( pay_i);

    nfrm = trx_tm_addtoframe( (int *)&maxIndx, 1, CMD_ADDFRAME_ADD);
    nfrm = trx_tm_addtoframe( (int *)&nextIndx, 1, CMD_ADDFRAME_ADD);

    #if (SCH_CMDTCM_VERBOSE>=1)
        char buffer[50];
        sprintf( buffer, "pay_i=%d\r\n", (unsigned int)pay_i );
        con_printf(buffer); 
        sprintf( buffer, "maxIndx=%d, nextIndx=%d\r\n", maxIndx, nextIndx );
        con_printf(buffer);
    #endif

    //Add Payload Data
    unsigned int indx; int val;
    for(indx=0; indx<=maxIndx; indx++)
    {
        dat_getPayloadVar(pay_i, indx, &val);
        nfrm = trx_tm_addtoframe(&val, 1, CMD_ADDFRAME_ADD);

        #if (SCH_CMDTCM_VERBOSE>=2)
            char buffer[10];
            con_printf("dat_getPayloadVar[");
            //itoa(buffer, (unsigned int)indxVar, 10);
            sprintf( buffer, "%d", (unsigned int)indx );
            con_printf(buffer); con_printf("]=");
            //itoa(buffer,(unsigned int)dat_getCubesatVar(indxVar), 10);
            sprintf( buffer, "0x%X", (unsigned int)val );
            con_printf(buffer); con_printf("\r\n");
        #endif


        ClrWdt();
    }

    // Close session
    // data = trx_tm_addtoframe(&data, 0, CMD_ADDFRAME_STOP);     /* Empty stop frame */
    //nfrm = trx_tm_addtoframe(&tm_id, 0, CMD_ADDFRAME_FIN);      /* End session */
    trx_tm_addtoframe(&tm_id, 0, CMD_ADDFRAME_FIN);      /* End session */

    return nfrm;
}