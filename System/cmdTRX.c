/*                                 SUCHAI
 *                      NANOSATELLITE FLIGHT SOFTWARE
 *
 *      Copyright 2013, Carlos Gonzalez Cortes, carlgonz@ug.uchile.cl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cmdTRX.h"
#include "csp.h"

/* Auxiliary variables */
int16_t TRX_REG_VAL = -1; /*Current value to write in trx_write_reg*/
nanocom_conf_t TRX_CONFIG; /*Stores TRX configuration*/
static uint16_t com_timeout = 2000;

cmdFunction trxFunction[TRX_NCMD];
int trx_sysReq[TRX_NCMD];

void trx_onResetCmdTRX(void){
    /*TRX*/
    trxFunction[(unsigned char)trx_id_send_beacon] = trx_send_beacon;
    trx_sysReq[(unsigned char)trx_id_send_beacon]  = CMD_SYSREQ_MIN;
    trxFunction[(unsigned char)trx_id_readreg] = trx_read_conf;
    trx_sysReq[(unsigned char)trx_id_readreg]  = CMD_SYSREQ_MIN;
    trxFunction[(unsigned char)trx_id_idleframe] = trx_test_frame;
    trx_sysReq[(unsigned char)trx_id_idleframe]  = CMD_SYSREQ_MIN;
    trxFunction[(unsigned char)trx_id_getstatus] = trx_getstatus;
    trx_sysReq[(unsigned char)trx_id_getstatus]  = CMD_SYSREQ_MIN;
    trxFunction[(unsigned char)trx_id_set_beacon] = trx_set_beacon;
    trx_sysReq[(unsigned char)trx_id_set_beacon]  = CMD_SYSREQ_MIN;
    trxFunction[(unsigned char)trx_id_initialize] = trx_initialize;
    trx_sysReq[(unsigned char)trx_id_initialize]  = CMD_SYSREQ_MIN;
    trxFunction[(unsigned char)trx_id_setmode] = trx_setmode;
    trx_sysReq[(unsigned char)trx_id_setmode]  = CMD_SYSREQ_MIN;
    trxFunction[(unsigned char)trx_id_asknewtc] = trx_asknewtc;
    trx_sysReq[(unsigned char)trx_id_asknewtc]  = CMD_SYSREQ_MIN;
    trxFunction[(unsigned char)trx_id_parsetcframe] = trx_parsetcframe;
    trx_sysReq[(unsigned char)trx_id_parsetcframe]  = CMD_SYSREQ_MIN;
    trxFunction[(unsigned char)trx_id_set_tm_pwr] = trx_set_tm_pwr;
    trx_sysReq[(unsigned char)trx_id_set_tm_pwr]  = CMD_SYSREQ_MIN;
    trxFunction[(unsigned char)trx_id_set_bc_pwr] = trx_set_bc_pwr;
    trx_sysReq[(unsigned char)trx_id_set_bc_pwr]  = CMD_SYSREQ_MIN;
    trxFunction[(unsigned char)trx_id_read_tcframe] = trx_read_tcframe;
    trx_sysReq[(unsigned char)trx_id_read_tcframe]  = CMD_SYSREQ_MIN;
    trxFunction[(unsigned char)trx_id_tm_trxstatus] = trx_tm_trxstatus;
    trx_sysReq[(unsigned char)trx_id_tm_trxstatus]  = CMD_SYSREQ_MIN+2;
    trxFunction[(unsigned char)trx_id_write_reg] = trx_write_reg;
    trx_sysReq[(unsigned char)trx_id_write_reg]  = CMD_SYSREQ_MIN;
    trxFunction[(unsigned char)trx_id_set_reg_val] = trx_set_reg_val;
    trx_sysReq[(unsigned char)trx_id_set_reg_val]  = CMD_SYSREQ_MIN;
    trxFunction[(unsigned char)trx_id_resend] = trx_resend;
    trx_sysReq[(unsigned char)trx_id_resend]  = CMD_SYSREQ_MIN+3; /* CMD_SYSREQ_MIN+3 */
    trxFunction[(unsigned char)trx_id_reset_tm_pointer] = trx_reset_tm_pointer;
    trx_sysReq[(unsigned char)trx_id_reset_tm_pointer]  = CMD_SYSREQ_MIN;
}

/**
 * Upload current configuration into TRX
 * @param param Not used
 * @return 0 Fail, 1 OK
 */
int trx_set_conf(void *param)
{
    #if SCH_CMDTRX_VERBOSE
        printf("Uploading TRX configuration\n");
        com_print_conf(&TRX_CONFIG);
    #endif
    return com_set_conf(&TRX_CONFIG, NODE_COM, com_timeout);
}

/**
 * Get TRX current configuration and save to local variable
 *
 * @param param Not used
 * @return 1 - OK, 0 - Fail
 */
int trx_read_conf(void *param)
{
    int result;
    result = com_get_conf(&TRX_CONFIG, NODE_COM, com_timeout);

    #if (SCH_CMDTRX_VERBOSE>=1)
        printf("Current TRX configuration\n");
        com_print_conf(&TRX_CONFIG);
    #endif

    return result;
}

/**
 * Set the standart beacon: 00SUCHAI00
 * @sa tcm_send_beacon()
 * 
 * @param param (0)SUCHAI beacon. (1)Test beacon 1. (2)Test beacon 2.
 * @return 1 - OK
 */
int trx_set_beacon(void *param)
{
    char stdbeacon[] = "00SUCHAI00";

    switch (*(int *)param)
    {
        case 1:
            strcpy(stdbeacon, "123456789\0");
            break;

        case 2:
            strcpy(stdbeacon, "000000000\0");
            break;

        default:
            strcpy(stdbeacon, "\0\0\0\0\0\0\0\0\0\0");
            break;
    }

    #if SCH_CMDTRX_VERBOSE
        printf("Setting beacon: %s\n", stdbeacon);
    #endif

    memcpy(TRX_CONFIG.morse_text, stdbeacon, COM_MORSE_LEN);
    int result = trx_set_conf(NULL);

    return result;
}

/**
 * Triggers the beacon
 *
 * @param param (1)Verbose. (0)No Verbose
 * @return 1 - OK
 * @deprecated
 */
int trx_send_beacon(void *param)
{
#if SCH_CMDTRX_VERBOSE
    printf("[DEPRECATED] (NOT) Sending beacon...\n");
#endif
    
    return 0;
}

/**
 * Send a frame for testing (ping) to desired node
 *
 * @param param int Node to transmit
 * @return 1 - OK
 */
int trx_test_frame(void *param)
{
    int result;
    int node = *((int *)param);

#if SCH_CMDTRX_VERBOSE
    printf("Sending test frame to node %d...\n", node);
#endif

    result = csp_ping(node, com_timeout, 10, CSP_O_NONE);

#if SCH_CMDTRX_VERBOSE
    printf("Ping to %d of size %d, took %d ms\n", node, 10, result);
#endif
    
    result = result > 0 ? 1:0;

    return result;
}

/**
 * Read and show TRX status. Debug only
 *
 * @param param Not used
 * @return 1 - OK
 */
int trx_getstatus(void *param)
{
    nanocom_data_t status;
    int result;

    result = com_get_status(&status, NODE_COM, com_timeout);

#if SCH_CMDTRX_VERBOSE
    //TODO: Print status
#endif

    return 1;
}


/**
 * Resend the telemetry buffer, from the base to the index of the last sended
 * TM Frame
 *
 * @param param (1)Verbose, (0)No Verbose
 * @return 1 - OK; 0 - Fail
 * @deprecated
 */
int trx_resend(void *param)
{
    int result = 0;
#if SCH_CMDTRX_VERBOSE
    printf("[DEPRECATED]\n");
#endif
    return result;
}

/**
 * Resets the tm buffer pointer to 0, both TMTF_OUT and TMTF_IN
 *
 * @param param 1-verboso, 0-no verbose
 * @return 1-success, 0-fail
 * @deprecated
 */
int trx_reset_tm_pointer(void *param)
{
    int result = 0;
#if SCH_CMDTRX_VERBOSE
    printf("[DEPRECATED]\n");
#endif
    return result;
}

/**
 * Initializes TRX main parameters
 * 
 * @param param (1)Verbose, (0)No Verbose
 * @return 1 - OK; 0 - Fail
 */
int trx_initialize(void *param)
{
    //TODO: implement
    return 1;
}

/**
 * Set TRX mode
 *
 * @param param (0)RESET, (1)SYSRESET, (2)SILENT, (3)ONLYBEACON, (4)NOBEACON, (5)NOMINAL
 * @return 1 - OK; 0 - Fail
 */
int trx_setmode(void *param)
{
    unsigned int value = 1;
    int mode = *(int *)param;

    //TODO: Implement

    return mode;
}

/**
 * Ask if new telecommand has not been read. Sets lascmd_day and new_tcframe
 * flags in data repository
 *
 * @param param (0) No verbose, (1) Verbose
 * @return 1 - OK; 0 - Fail
 */
int trx_asknewtc(void *param)
{
    // TODO: Implement
    return 1;
}

/**
 * Read one tc frame from trx and parse incoming tcs
 * TC Frame Format    :
 * @code
 *                      |  CMD1 |  ARG1 | ..... |  CMDN | ARGN  | STOP  |
 *                      |MSB|LSB|MSB|LSB| ..... |MSB|LSB|MSB|LSB|MSB|LSB|
 * @endcode
 * 
 * @param param (0) No verbose, (1) Verbose
 * @return Number of TC read.
 */
int trx_parsetcframe(void *param)
{
//    char tcframe[TRX_TCFRAMELEN];
//    int count = 0;
//    int step = 4;
//    int result = 0;
//    int mode = *(int *)param;
//
//    /* Read ONE TC Frame from TRX */
//    /* Return 0 if no TC was readed*/
//    result = TRX_ReadTelecomadFrame(tcframe);
//
//    if(result)
//    {
//        int parserindex = 0;
//        result = 0;
//        /* Parsing the TCTF in SUCHAI's commands */
//        for(count=0; count < TRX_TCFRAMELEN; count+=step)
//        {
//            /* BIG ENDIAN [MSB]<<8 | [LSB] */
//            int cmdid = tcframe[count]; cmdid = (cmdid <<8)|tcframe[count+1];
//            int cmdarg = tcframe[count+2]; cmdarg = (cmdarg<<8)|tcframe[count+3];
//
//            /* Check for stop bytes, then add new cmd */
//            if((cmdid != CMD_STOP) && (cmdarg != CMD_STOP))
//            {
//                /* Save TC and ARG into repo_telecmd */
//                dat_setTelecmdBuff(parserindex++,cmdid);
//                dat_setTelecmdBuff(parserindex++,cmdarg);
//                result++;
//            }
//            /* Stop bytes detected, end parsing */
//            else
//            {
//                break;
//            }
//        }
//
//        /* Fill remaining buffer space */
//        while(parserindex < SCH_DATAREPOSITORY_MAX_BUFF_TELECMD)
//        {
//            dat_setTelecmdBuff(parserindex++, CMD_CMDNULL);
//        }
//    }
//
//#if SCH_CMDTRX_VERBOSE
//    if(mode)
//    {
//        char ascii_val[10];
//        itoa(ascii_val, result, 10);
//        con_printf("Number of read TC: [");
//        con_printf(ascii_val); con_printf("]\n");
//    }
//#endif
//
//    if(result)
//    {
//        /* Aumentar el contador de TC recividos */
//        result += dat_getCubesatVar(dat_trx_count_tc);
//        dat_setCubesatVar(dat_trx_count_tc, result);
//
//        /* Indicar que hay comandos que procesar en el buffer de Cmd */
//        dat_setCubesatVar(dat_trx_newCmdBuff, 1);
//    }
//
//    return result;
}

/**
 * Read one tc frame from trx.
 * Debug only
 *
 * @param param (0) No verbose, (1) Verbose
 * @return 1 - OK; 0 - Fail
 * @deprecated
 */
int trx_read_tcframe(void *param)
{
//    /* Se lee un frame de telecomandos */
//    char tc_frame[TRX_TCFRAMELEN];
//    int result = TRX_ReadTelecomadFrame(tc_frame);
//
//    /* Se muestra en consola si corresponde */
//    if(*(int *)param)
//        SendRS232((unsigned char*)tc_frame, TRX_TCFRAMELEN, RS2_M_UART1);

    return 0;
}

/**
 * Reads and transmit telemetry ralated to trx's status
 * TMID: 0x000A
 *
 * @param param 0 - Only store, 1 - Only Send, 2 - Store and send @deprecated
 * @return 0 (Tx fail) - 1 (Tx OK)
 */
int trx_tm_trxstatus(void *param)
{
    int data_len = 0x36;
    int data = 0;
    unsigned char status[data_len];

    int mode = *((int *)param);

    if((mode == 0) || (mode == 2))
    {
        /* Start a new session (Single or normal) */
        data = 0x000A; /* TM ID */
        trx_tm_addtoframe(&data, 0, CMD_ADDFRAME_FIN);   /* Close previos sessions */
        trx_tm_addtoframe(&data, 1, CMD_ADDFRAME_START); /* New empty start frame */

        /* Read info and append to the frame */
//        TRX_GetStatus(status);
        /*To int*/
        int data_int[data_len]; int i;
        for(i=0; i<data_len;i++) {data_int[i] = (int)status[i];}
        /* Add data */
        trx_tm_addtoframe(data_int, data_len, CMD_ADDFRAME_ADD);

        // Close session
        // data = trx_tm_addtoframe(&data, 0, CMD_ADDFRAME_STOP);     /* Empty stop frame */
        data = trx_tm_addtoframe(&data, 0, CMD_ADDFRAME_FIN);      /* End session */
    }

    if((mode == 1) || (mode == 2))
    {
        /* Evitar enviar el comando transmitir si los flags nTX o nRX estan activos */
        while(!(PPC_nTX_FLAG && PPC_nRX_FLAG))
        {
            long i;
            for(i=0; i<0xFFFFFF; i++);
        }

        /* Transmmit info */
//        data = TRX_SendTelemetry();
    }

    return data;
}

/**
 * Sets power level for telemetry
 *
 * @param param (0-24) Disired power level
 * @return 1 - OK; 0 - Fail
 * @deprecated
 */
int trx_set_tm_pwr(void *param)
{
    return 0;
}

/**
 * Sets power level for beacon
 *
 * @param param (0-24) Disired power level
 * @return 1 - OK; 0 - Fail
 * @deprecated
 */
int trx_set_bc_pwr(void *param)
{
    return 0;
}

/**
 * Sets any TRX register with the value set in TRX_REG_VAL
 * @note Prior to use this cmd, user must set the value to write with the
 * trx_set_reg_val command.
 *
 * @param param register to write
 * @return 1 - OK; 0 - Fail
 */
int trx_write_reg(void *param)
{
    int reg = *((int *)param);
    int val = TRX_REG_VAL;

    if((reg > 0x00FF) || (val > 0x00FF))
        return 0;

//    TODO: TRX_WriteRegister((unsigned char)reg, (unsigned char)val);

    return 1;
}

/**
 * Sets the current value in TRX_REG_VAL. This value is saved in the reg, when
 * trx_write_reg is called.
 * @sa trx_write_reg()
 *
 * @param param value to store
 * @return 0 (fail) - 1 (OK)
 */
int trx_set_reg_val(void *param)
{
    TRX_REG_VAL = *((int *)param);
    return 1;
}

/**
 * Organize temletry data in frames and load this frames in TRX. Each telemetry
 * frame has the following format.
 * @code
 *          |--- Control fields ---||----        Data fields        ---|
 *           __________________________________________________________
 *          | Type (2) | Frame# (2)|| TM-ID(2) |       DATA(122)      ||
 *          |__________|___________||__________||DH|DL|___...___|DH|DL||
 * @endcode
 *
 * @warning Por problemas con el TNC cada vez que se crea un frame se transmite
 * inmediatamente. Esto evita el envio sucesivo de frames de telemetria, ya que
 * el TNC pierde datos ante una rafaga continua de frames de telemetria.
 *
 * @param data Pointer to data that will be append
 * @param len Lenght of the data array
 * @param mode Mode of append   CMD_ADDFRAME_START -> Start new frame
 *                              CMD_ADDFRAME_CONT  -> Inter frame
 *                              CMD_ADDFRAME_STOP  -> Stop frame
 *                              CMD_ADDFRAME_FIN   -> Finish current session
 *                              CMD_ADDFRAME_ADD   -> Add new data
 *                              CMD_ADDFRAME_SINGL -> Create a single frame
 *
 * @return 0, TX Fail. 1, TX Success. 2, No TX operation performe
 */
int trx_tm_addtoframe(int *data, int len, int mode)
{
    static unsigned char tmframe[TRX_TMFRAMELEN];
    static int byte_counter = 0;
    static int frame_counter = 0;
    static int tm_type = CMD_STOP;
    int send_stat = 2;

    while(mode != CMD_ADDFRAME_EXIT)
    {
        switch (mode)
        {
            /* A single frame that contains max TRX_TMFRAMELEN bytes of data */
            case CMD_ADDFRAME_SINGL:
                /* A new frame being configured */
                byte_counter = 0;
                /* Append control field  */
                tmframe[byte_counter++] = CMD_TMFRAME_TSINGLH;   /* Type (1) */
                tmframe[byte_counter++] = CMD_TMFRAME_TSINGLL;   /* Type (2) */
                tmframe[byte_counter++] = (char)(frame_counter >> 8);/* Frame# (1) */
                tmframe[byte_counter++] = (char)(frame_counter);     /* Frame# (2) */

                /* Adding data to the current frame. Max TRX_TMFRAMELEN bytes */
                while((len > 0) && (byte_counter < TRX_TMFRAMELEN))
                {
                    tmframe[byte_counter++] = (char)(*data >> 8);/* DataH */
                    tmframe[byte_counter++] = (char)(*data);     /* DataL */
                    data++;
                    len--;
                }

                /* Loading frame to TRX and exiting */
                for(;byte_counter<TRX_TMFRAMELEN;byte_counter++)
                {
                    tmframe[byte_counter++] = (char)((int)CMD_STOP>>8);
                    tmframe[byte_counter] = (char)CMD_STOP;
                }
                
//                TODO: TRX_LoadTelemetry(tmframe, TRX_TMFRAMELEN);
//                TODO: send_stat = TRX_SendTelemetry(); // toopazo - TNC falla al enviar varios frames seguidos

                #if SCH_CMDTRX_VERBOSE > 1
                    con_printf("Loading TM frame(Single Frame)\n");
                #endif
                frame_counter++;
                byte_counter = 0;
                        mode = CMD_ADDFRAME_EXIT;
                break;

            /* A new start frame for more than TRX_TMFRAMELEN bytes, so we need
             * to load several frames cotaining all the telemetry */
            case CMD_ADDFRAME_START:
                /* A new frame being configured */
                /* Load all remaining frames before */
                if(byte_counter > 0)
                {
                    for(;byte_counter<TRX_TMFRAMELEN;byte_counter++)
                    {
                        tmframe[byte_counter++] = (char)((int)CMD_STOP>>8);
                        tmframe[byte_counter] = (char)CMD_STOP;
                    }
//                    TODO: TRX_LoadTelemetry(tmframe, TRX_TMFRAMELEN);
//                    TODO: send_stat = TRX_SendTelemetry(); // toopazo - TNC falla al enviar varios frames seguidos

                    #if SCH_CMDTRX_VERBOSE > 1
                        con_printf("Loading TM frame(From START)\n");
                    #endif
                }

                byte_counter = 0;
                /* Append control field  */
                tmframe[byte_counter++] = CMD_TMFRAME_TSTARTH;   /* Type (1) */
                tmframe[byte_counter++] = CMD_TMFRAME_TSTARTL;   /* Type (2) */
                tmframe[byte_counter++] = (char)(frame_counter >> 8);/* Frame# (1) */
                tmframe[byte_counter++] = (char)(frame_counter);     /* Frame# (2) */

                /* Add data if needed */
                //for(i=0;i<byte_counter;i++) printf("%X|",tmframe[i]); printf("\n");
                frame_counter++;
                
                /* Adding current tm type */
                if(len > 0)
                {
                    tm_type = *data;
                    tmframe[byte_counter++] = (char)(tm_type >> 8);/* DataH */
                    tmframe[byte_counter++] = (char)(tm_type);     /* DataL */

                    /* Update the remaining data counter */
                    data++;
                    len--;
                }

                mode = CMD_ADDFRAME_ADD;
                break;

            case CMD_ADDFRAME_CONT:
                /* A new frame being configured */
                byte_counter = 0;
                /* Append control field  */
                tmframe[byte_counter++] = CMD_TMFRAME_TCONTH;   /* Type (1) */
                tmframe[byte_counter++] = CMD_TMFRAME_TCONTL;   /* Type (2) */
                tmframe[byte_counter++] = (char)(frame_counter >> 8);/* Frame# (1) */
                tmframe[byte_counter++] = (char)(frame_counter);     /* Frame# (2) */

                /* Add tm type */
                tmframe[byte_counter++] = (char)(tm_type >> 8);/* DataH */
                tmframe[byte_counter++] = (char)(tm_type);     /* DataL */

                /* Add data if needed */
                frame_counter++;
                mode = CMD_ADDFRAME_ADD;
                break;

            /* The last frame with the current telemetry */
            case CMD_ADDFRAME_STOP:
                /* Load all remaining frames before */
                if(byte_counter > 0)
                {
                    for(;byte_counter<TRX_TMFRAMELEN;byte_counter++)
                    {
                        tmframe[byte_counter++] = (char)((int)CMD_STOP>>8);
                        tmframe[byte_counter] = (char)CMD_STOP;
                    }
//                    TODO: TRX_LoadTelemetry(tmframe, TRX_TMFRAMELEN);
                    #if SCH_CMDTRX_VERBOSE > 1
                        con_printf("Loading TM frame (From STOP)\n");
                        //SendRS232((unsigned char*)tmframe,TRX_TMFRAMELEN,RS2_M_UART1);
                        //con_printf("\n\n");
                    #endif
                }
                /* A new frame stop being configured */
                byte_counter = 0;
                /* Append control field */
                tmframe[byte_counter++] = CMD_TMFRAME_TSTOPH;   /* Type (1) */
                tmframe[byte_counter++] = CMD_TMFRAME_TSTOPL;   /* Type (2) */
                tmframe[byte_counter++] = (char)(frame_counter >> 8);/* Frame# (1) */
                tmframe[byte_counter++] = (char)(frame_counter);     /* Frame# (2) */

                /* Add data to this frame */
                frame_counter++;
                mode = CMD_ADDFRAME_ADD;
                break;

            /* Load to TRX all reamaining frames and reset the control field
             * counters to start a new session. No new data will be append */
            case CMD_ADDFRAME_FIN:
                /* Load to trx the remaining frames */
                if(byte_counter > 0)
                {
                    for(;byte_counter<TRX_TMFRAMELEN;byte_counter++)
                    {
                        tmframe[byte_counter++] = (char)((int)CMD_STOP>>8);
                        tmframe[byte_counter] = (char)CMD_STOP;
                    }
//                    TODO: TRX_LoadTelemetry(tmframe, TRX_TMFRAMELEN);
//                    TODO: send_stat = TRX_SendTelemetry(); // toopazo - TNC falla al enviar varios frames seguidos
                    #if SCH_CMDTRX_VERBOSE  > 1
                        con_printf("Loading TM Frame (From FIN)");
                        //SendRS232((unsigned char*)tmframe,TRX_TMFRAMELEN,RS2_M_UART1);
                        //con_printf("\n\n");
                    #endif
                }
                byte_counter = 0;
                frame_counter = 0;
                tm_type = CMD_STOP;
                mode = CMD_ADDFRAME_EXIT;
                break;

            /* Append all the requiered data to the current frame or adds more
             * CONT frame if needed */
            case CMD_ADDFRAME_ADD:
                if(len > 0)
                {
                    /* Current frame is not initilized */
                    if(byte_counter == 0 )
                    {
                        mode = CMD_ADDFRAME_EXIT;
                    }
                    /* Current frame is not full */
                    else if(byte_counter < TRX_TMFRAMELEN)
                    {
                        /* Adding data to the current frame*/
                        tmframe[byte_counter++] = (char)(*data >> 8);/* DataH */
                        tmframe[byte_counter++] = (char)(*data);     /* DataL */
                        /* Update the remaining data counter */
                        data++;
                        len--;
                        mode = CMD_ADDFRAME_ADD;
                    }
                    else /* Current frame is full */
                    {
                        /* This frame is ready to be loaded into TRX buffer */
                        /* First fill tmframe's unused fields */
                        for(;byte_counter<TRX_TMFRAMELEN;byte_counter++)
                        {
                            tmframe[byte_counter++] = (char)((int)CMD_STOP>>8);
                            tmframe[byte_counter] = (char)CMD_STOP;
                        }

//TODO:                        TRX_LoadTelemetry(tmframe, TRX_TMFRAMELEN);
//       TODO:                 send_stat = TRX_SendTelemetry(); // toopazo - TNC falla al enviar varios frames seguidos
                        #if SCH_CMDTRX_VERBOSE > 1
                            con_printf("Loading  TMFrame (From ADD)\n");
                            //SendRS232((unsigned char*)tmframe,TRX_TMFRAMELEN,RS2_M_UART1);
                            //con_printf("\n\n");
                        #endif

                        /* Keep adding data in a new frame */
                        byte_counter = 0;
                        mode = CMD_ADDFRAME_CONT;
                    }
                }
                else /* No more data to load */
                {
                    /* Exit */
                    mode = CMD_ADDFRAME_EXIT;
                }
                break;

            /* Invalid state */
            default:
                mode = CMD_ADDFRAME_EXIT;
                break;
        }
    }
    //return frame_counter;

    return send_stat;
}
