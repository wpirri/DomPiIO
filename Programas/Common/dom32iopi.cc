/***************************************************************************
  Copyright (C) 2021   Walter Pirri
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ***************************************************************************/

/*
    XML: http://xmlparselib.sourceforge.net/
         http://xmlparselib.sourceforge.net/examp_xml_token_traverser.html
*/
#include "dom32iopi.h"

#include <string>
#include <iostream>
#include <cerrno>
#include <cstdlib>
#include <cstdarg>

using namespace std;

#include <unistd.h>
#include <sys/msg.h>
#include <time.h>
#include <string.h>

#include "strfunc.h"

#include <wiringPi.h> // Include WiringPi library!
#include <wiringSerial.h>

#include "config.h"

#include "gpiopin.h"

#define DOMPI_IO_DEFAULT_CONFIG "/var/gmonitor/dompi_io.config"

Dom32IoPi::Dom32IoPi()
{
    wiringPiSetupGpio(); // Initialize wiringPi -- using Broadcom pin numbers
    m_sfd = (-1);
    strcpy(m_config_file_name, DOMPI_IO_DEFAULT_CONFIG);
}

Dom32IoPi::Dom32IoPi(const char* filename)
{
    wiringPiSetupGpio(); // Initialize wiringPi -- using Broadcom pin numbers
    m_sfd = (-1);
    if(filename)
    {
        strncpy(m_config_file_name, filename, FILENAME_MAX);
    }
    else strcpy(m_config_file_name, DOMPI_IO_DEFAULT_CONFIG);
}

Dom32IoPi::~Dom32IoPi()
{

}

void Dom32IoPi::SetDefaultConfig()
{
    DPConfig cfg("/etc/dompiweb.config");
    char s[8];

    memset(&m_pi_data, 0, sizeof(m_pi_data.config));

	if( !cfg.GetParam("DOMPIWEB-DEFAULT-HOST", m_pi_data.config.comm.host1) )
	{
		strcpy(m_pi_data.config.comm.host1, "0.0.0.0");
	}
    m_pi_data.config.comm.host1_port = 80;
	if( cfg.GetParam("DOMPIWEB-DEFAULT-PORT", s) )
	{
		m_pi_data.config.comm.host1_port = atoi(s);
	}
	if( !cfg.GetParam("DOMPIWEB-DEFAULT-PROTO", m_pi_data.config.comm.host1_protocol) )
	{
		strcpy(m_pi_data.config.comm.host1_protocol, "http");
	}
	if( !cfg.GetParam("MAC-ADDRESS", m_pi_data.config.comm.hw_mac) )
	{
		strcpy(m_pi_data.config.comm.hw_mac, "000000000000");
	}

    /* IO */
    m_pi_data.config.port[0].config = INPUT;
    m_pi_data.config.port[0].map = GPIO_IO1;
    m_pi_data.config.port[1].config = INPUT;
    m_pi_data.config.port[1].map = GPIO_IO2;
    m_pi_data.config.port[2].config = INPUT;
    m_pi_data.config.port[2].map = GPIO_IO3;
    m_pi_data.config.port[3].config = INPUT;
    m_pi_data.config.port[3].map = GPIO_IO4;
    m_pi_data.config.port[4].config = INPUT;
    m_pi_data.config.port[4].map = GPIO_IO5;
    m_pi_data.config.port[5].config = INPUT;
    m_pi_data.config.port[5].map = GPIO_IO6;
    m_pi_data.config.port[6].config = INPUT;
    m_pi_data.config.port[6].map = GPIO_IO7;
    m_pi_data.config.port[7].config = INPUT;
    m_pi_data.config.port[7].map = GPIO_IO8;
    /* OUT */
    m_pi_data.config.port[8].config = OUTPUT;
    m_pi_data.config.port[8].map = GPIO_OUT1;
    m_pi_data.config.port[9].config = OUTPUT;
    m_pi_data.config.port[9].map = GPIO_OUT2;
    /* EXP1 */
    m_pi_data.config.port[16].config = OUTPUT;
    m_pi_data.config.port[16].map = GPIO_EXP1_1;
    m_pi_data.config.port[17].config = OUTPUT;
    m_pi_data.config.port[17].map = GPIO_EXP1_2;
    m_pi_data.config.port[18].config = OUTPUT;
    m_pi_data.config.port[18].map = GPIO_EXP1_3;
    m_pi_data.config.port[19].config = OUTPUT;
    m_pi_data.config.port[19].map = GPIO_EXP1_4;
    m_pi_data.config.port[20].config = OUTPUT;
    m_pi_data.config.port[20].map = GPIO_EXP1_5;
    m_pi_data.config.port[21].config = OUTPUT;
    m_pi_data.config.port[21].map = GPIO_EXP1_6;
    m_pi_data.config.port[22].config = OUTPUT;
    m_pi_data.config.port[22].map = GPIO_EXP1_7;
    m_pi_data.config.port[23].config = OUTPUT;
    m_pi_data.config.port[23].map = GPIO_EXP1_8;

    m_pi_data.config.default_config = 1;
}

void Dom32IoPi::LoadConfig( const char* filename )
{
    FILE *fd;

    if(filename)
    {
        strncpy(m_config_file_name, filename, FILENAME_MAX);
    }

    fd = fopen(m_config_file_name, "r");
    if(fd)
    {
        if(fread(&m_pi_data.config, sizeof(m_pi_data.config), 1, fd))
        {
            fclose(fd);
        }
        else
        {
            fclose(fd);
            SetDefaultConfig();
            SaveConfig();
        }
    }
    else
    {
        SetDefaultConfig();
        SaveConfig();
    }

    SetConfig();

}

void Dom32IoPi::SaveConfig( void )
{
    FILE *fd;

    fd = fopen(m_config_file_name, "w");
    if(fd)
    {
        fwrite(&m_pi_data.config, sizeof(m_pi_data.config), 1, fd);
        fclose(fd);
    }
}

int Dom32IoPi::GetIOStatus()
{
    int i;
    char status;
    int change = 0;

    for(i = 0; i < 24; i++)
    {
        if(m_pi_data.config.port[i].map >0)
        {
            status = digitalRead(gpio_pin[m_pi_data.config.port[i].map]);
            if(status != m_pi_data.status.port[i].status)
            {
                m_pi_data.status.port[i].status = status;
                m_pi_data.status.port[i].change = 1;
                change = 1;
            }
            
        }
    }
    return change;
}

void Dom32IoPi::SetIOStatus()
{
    int i;

    for(i = 0; i < 24; i++)
    {
        if(m_pi_data.config.port[i].map > 0 && m_pi_data.config.port[i].config == OUTPUT)
        {
            digitalWrite(gpio_pin[m_pi_data.config.port[i].map], (m_pi_data.status.port[i].status)?HIGH:LOW);
        }
    }
}

int Dom32IoPi::GetConfig(int /*port*/, int */*ioconfig*/)
{
    return (-1);
}

int Dom32IoPi::ConfigIO(const char* io, const char* mode)
{
    STRFunc str;
    char IO[9];
    char MODE[17];
    int i_mode;

    str.ToUpper(io, IO);
    str.ToUpper(mode, MODE);
    if( !strcmp(MODE, "IN"))
    {
        i_mode = INPUT;
    }
    else if( !strcmp(MODE, "OUT"))
    {
        i_mode = OUTPUT;
    }
    else
    {
        return (-1);
    }

    if( !strcmp(IO, "IO1")) m_pi_data.config.port[0].config = i_mode;
    if( !strcmp(IO, "IO2")) m_pi_data.config.port[1].config = i_mode;
    if( !strcmp(IO, "IO3")) m_pi_data.config.port[2].config = i_mode;
    if( !strcmp(IO, "IO4")) m_pi_data.config.port[3].config = i_mode;
    if( !strcmp(IO, "IO5")) m_pi_data.config.port[4].config = i_mode;
    if( !strcmp(IO, "IO6")) m_pi_data.config.port[5].config = i_mode;
    if( !strcmp(IO, "IO7")) m_pi_data.config.port[6].config = i_mode;
    if( !strcmp(IO, "IO8")) m_pi_data.config.port[7].config = i_mode;

    if( !strcmp(IO, "EXP1_1")) m_pi_data.config.port[16].config = i_mode;
    if( !strcmp(IO, "EXP1_2")) m_pi_data.config.port[17].config = i_mode;
    if( !strcmp(IO, "EXP1_3")) m_pi_data.config.port[18].config = i_mode;
    if( !strcmp(IO, "EXP1_4")) m_pi_data.config.port[19].config = i_mode;
    if( !strcmp(IO, "EXP1_5")) m_pi_data.config.port[20].config = i_mode;
    if( !strcmp(IO, "EXP1_6")) m_pi_data.config.port[21].config = i_mode;
    if( !strcmp(IO, "EXP1_7")) m_pi_data.config.port[22].config = i_mode;
    if( !strcmp(IO, "EXP1_8")) m_pi_data.config.port[23].config = i_mode;

    SetConfig();
    SaveConfig();

    return 0;
}

void Dom32IoPi::SetStatusLed(int st)
{
    digitalWrite(gpio_pin[GPIO_STATUS_LED], (st)?HIGH:LOW); // Turn LED ON/OFF
}

void Dom32IoPi::SetModeLed(int st)
{
    digitalWrite(gpio_pin[GPIO_MODE_LED], (st)?HIGH:LOW); // Turn LED ON/OFF
}

void Dom32IoPi::SetConfig( void )
{
    pinMode(gpio_pin[m_pi_data.config.port[0].map], m_pi_data.config.port[0].config);
    pinMode(gpio_pin[m_pi_data.config.port[1].map], m_pi_data.config.port[1].config);
    pinMode(gpio_pin[m_pi_data.config.port[2].map], m_pi_data.config.port[2].config);
    pinMode(gpio_pin[m_pi_data.config.port[3].map], m_pi_data.config.port[3].config);
    pinMode(gpio_pin[m_pi_data.config.port[4].map], m_pi_data.config.port[4].config);
    pinMode(gpio_pin[m_pi_data.config.port[5].map], m_pi_data.config.port[5].config);
    pinMode(gpio_pin[m_pi_data.config.port[6].map], m_pi_data.config.port[6].config);
    pinMode(gpio_pin[m_pi_data.config.port[7].map], m_pi_data.config.port[7].config);
    
    pinMode(gpio_pin[m_pi_data.config.port[8].map], m_pi_data.config.port[8].config);
    pinMode(gpio_pin[m_pi_data.config.port[9].map], m_pi_data.config.port[9].config);

    pinMode(gpio_pin[m_pi_data.config.port[16].map], m_pi_data.config.port[16].config);
    pinMode(gpio_pin[m_pi_data.config.port[17].map], m_pi_data.config.port[17].config);
    pinMode(gpio_pin[m_pi_data.config.port[18].map], m_pi_data.config.port[18].config);
    pinMode(gpio_pin[m_pi_data.config.port[19].map], m_pi_data.config.port[19].config);
    pinMode(gpio_pin[m_pi_data.config.port[20].map], m_pi_data.config.port[20].config);
    pinMode(gpio_pin[m_pi_data.config.port[21].map], m_pi_data.config.port[21].config);
    pinMode(gpio_pin[m_pi_data.config.port[22].map], m_pi_data.config.port[22].config);
    pinMode(gpio_pin[m_pi_data.config.port[23].map], m_pi_data.config.port[23].config);

    pinMode(gpio_pin[GPIO_MODE_LED], OUTPUT);
    pinMode(gpio_pin[GPIO_STATUS_LED], OUTPUT);
    pinMode(gpio_pin[GPIO_TX], OUTPUT);
    pinMode(gpio_pin[GPIO_RX], INPUT);
}

int Dom32IoPi::HttpRespCode(const char* http)
{
    char tmp[16];
    STRFunc Str;

    Str.Section(http, ' ', 1, tmp);

    return atoi(tmp);
}

int Dom32IoPi::HttpData(const char* http, char* data)
{
    char* p;

    *data = 0;

    p = strstr((char*)http, "\r\n\r\n");
    if(p)
    {
        strcpy(data, p+4);
    }

    return strlen(data);
}
