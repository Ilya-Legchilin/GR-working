#include "parameters.h"
#include "hal/internal_flash.h"
#include "utilities/crc32.h"
#include "defines.h"
#include "utilities/misc.h"
#include "hal/rtc.h"
#include "protocol.h"

parameters_t parameters;

const uint8_t   parameters_mf = METER_TYPE;
const uint8_t   parameters_hw = HW_VERSION;
const char      parameters_vr[] = SW_VERSION;
const uint16_t  parameters_tm = 200; //dedfault temperature
const uint8_t   parameters_op = METER_TYPE;

uint32_t parameters_work_time = 0;
uint32_t parameters_sabotage_time = 0;


static void parameters_clear()
{
    uint16_t i;
    for(i = 0; i < sizeof(parameters_t); ++i)
        ((uint8_t*)&parameters)[i] = 0;
}

static uint32_t parameters_calc_crc()
{
    return crc32(((uint8_t*)&parameters), sizeof(parameters_t) - 4, 0xFFFF);
}

void parameters_read()
{
    internal_flash_read_sector(USERD, ((uint8_t*)&parameters), sizeof(parameters_t));

    if ( parameters.crc != parameters_calc_crc() )
        parameters_clear();
}

void parameters_save()
{
    parameters.time = rtc_get_utc();
    parameters.crc = parameters_calc_crc();
    internal_flash_write_sector(USERD, ((uint8_t*)&parameters), sizeof(parameters_t));
}

#define ONE_MONTH (32 * 24 * 3600)

void parameters_update(parameters_t* new_parameters)
{
    if (new_parameters->period && new_parameters->period < ONE_MONTH) {
        //
        if (new_parameters->period < 100)
            new_parameters->period = 100;

        parameters.period = new_parameters->period;
    }

    if (new_parameters->tarif && new_parameters->tarif  < 0xFFFF) {
        //
        parameters.tarif = new_parameters->tarif;
    }

    if (new_parameters->balance < 0xFFFF) {
        //
        parameters.balance = new_parameters->balance;
    }

    if (new_parameters->server.ip[0] && new_parameters->server.port[0]) {
        if (str_is_ip(new_parameters->server.ip)) {
            //
            strcp(new_parameters->server.ip,    parameters.server.ip);
            strcp(new_parameters->server.port,  parameters.server.port);
            rtc_add_alarm(20, proto_init);
        }
    }

    if (strlen(new_parameters->password) == 8) {
        //
        strcp(new_parameters->password, parameters.password);
    }

    if (strlen(new_parameters->spassword) == 32) {
        strcp(new_parameters->spassword, parameters.spassword);
    }

    if (new_parameters->time) {
        //
        parameters.time = new_parameters->time;
        rtc_init(new_parameters->time);
    }


    parameters_save();
}
