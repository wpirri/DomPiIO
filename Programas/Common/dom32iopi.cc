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
#include <cstdio>
#include <cstdarg>
using namespace std;

#include <unistd.h>
#include <sys/msg.h>
#include <time.h>
#include <string.h>

#include "strfunc.h"

#include <wiringPi.h> // Include WiringPi library!
#include <wiringSerial.h>

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
    memset(m_pi_config_data, 0, sizeof(pi_config_data));

    /* IO */
    m_pi_config_data.port[0].config = INPUT;
    m_pi_config_data.port[0].map = GPIO_IO1;
    m_pi_config_data.port[1].config = INPUT;
    m_pi_config_data.port[1].map = GPIO_IO2;
    m_pi_config_data.port[2].config = INPUT;
    m_pi_config_data.port[2].map = GPIO_IO3;
    m_pi_config_data.port[3].config = INPUT;
    m_pi_config_data.port[3].map = GPIO_IO4;
    m_pi_config_data.port[4].config = INPUT;
    m_pi_config_data.port[4].map = GPIO_IO5;
    m_pi_config_data.port[5].config = INPUT;
    m_pi_config_data.port[5].map = GPIO_IO6;
    m_pi_config_data.port[6].config = INPUT;
    m_pi_config_data.port[6].map = GPIO_IO7;
    m_pi_config_data.port[7].config = INPUT;
    m_pi_config_data.port[7].map = GPIO_IO8;
    /* OUT */
    m_pi_config_data.port[8].config = OUTPUT;
    m_pi_config_data.port[8].map = GPIO_OUT1;
    m_pi_config_data.port[9].config = OUTPUT;
    m_pi_config_data.port[9].map = GPIO_OUT2;
    /* EXP1 */
    m_pi_config_data.port[16].config = OUTPUT;
    m_pi_config_data.port[16].map = GPIO_EXP1_1;
    m_pi_config_data.port[17].config = OUTPUT;
    m_pi_config_data.port[17].map = GPIO_EXP1_2;
    m_pi_config_data.port[18].config = OUTPUT;
    m_pi_config_data.port[18].map = GPIO_EXP1_3;
    m_pi_config_data.port[19].config = OUTPUT;
    m_pi_config_data.port[19].map = GPIO_EXP1_4;
    m_pi_config_data.port[20].config = OUTPUT;
    m_pi_config_data.port[20].map = GPIO_EXP1_5;
    m_pi_config_data.port[21].config = OUTPUT;
    m_pi_config_data.port[21].map = GPIO_EXP1_6;
    m_pi_config_data.port[22].config = OUTPUT;
    m_pi_config_data.port[22].map = GPIO_EXP1_7;
    m_pi_config_data.port[23].config = OUTPUT;
    m_pi_config_data.port[23].map = GPIO_EXP1_8;
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
        if(fread(&m_pi_config_data, sizeof(pi_config_data), 1, fd))
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
        fwrite(&m_pi_config_data, sizeof(pi_config_data), 1, fd);
        fclose(fd);
    }
}

int Dom32IoPi::GetIOStatus(int port, int *iostatus)
{
    *iostatus = 0;

    switch (port)
    {
        case 1:
            if(digitalRead(gpio_pin[GPIO_A01])>0) (*iostatus) += 0x01;
            if(digitalRead(gpio_pin[GPIO_A02])>0) (*iostatus) += 0x02;
            if(digitalRead(gpio_pin[GPIO_A03])>0) (*iostatus) += 0x04;
            if(digitalRead(gpio_pin[GPIO_A04])>0) (*iostatus) += 0x08;
            if(digitalRead(gpio_pin[GPIO_A05])>0) (*iostatus) += 0x10;
            if(digitalRead(gpio_pin[GPIO_A06])>0) (*iostatus) += 0x20;
            if(digitalRead(gpio_pin[GPIO_A07])>0) (*iostatus) += 0x40;
            if(digitalRead(gpio_pin[GPIO_A08])>0) (*iostatus) += 0x80;
            break;
        case 2:
            if(digitalRead(gpio_pin[GPIO_B01])>0) (*iostatus) += 0x01;
            if(digitalRead(gpio_pin[GPIO_B02])>0) (*iostatus) += 0x02;
            if(digitalRead(gpio_pin[GPIO_B03])>0) (*iostatus) += 0x04;
            if(digitalRead(gpio_pin[GPIO_B04])>0) (*iostatus) += 0x08;
            if(digitalRead(gpio_pin[GPIO_B05])>0) (*iostatus) += 0x10;
            if(digitalRead(gpio_pin[GPIO_B06])>0) (*iostatus) += 0x20;
            if(digitalRead(gpio_pin[GPIO_B07])>0) (*iostatus) += 0x40;
            if(digitalRead(gpio_pin[GPIO_B08])>0) (*iostatus) += 0x80;
            break;
        case 3:
            if(digitalRead(gpio_pin[GPIO_C01])>0) (*iostatus) += 0x01;
            if(digitalRead(gpio_pin[GPIO_C02])>0) (*iostatus) += 0x02;
            if(digitalRead(gpio_pin[GPIO_C03])>0) (*iostatus) += 0x04;
            if(digitalRead(gpio_pin[GPIO_C04])>0) (*iostatus) += 0x08;
            if(digitalRead(gpio_pin[GPIO_C05])>0) (*iostatus) += 0x10;
            if(digitalRead(gpio_pin[GPIO_C06])>0) (*iostatus) += 0x20;
            if(digitalRead(gpio_pin[GPIO_C07])>0) (*iostatus) += 0x40;
            if(digitalRead(gpio_pin[GPIO_C08])>0) (*iostatus) += 0x80;
            break;
    }
    return (*iostatus);
}

int Dom32IoPi::GetConfig(int /*port*/, int */*ioconfig*/)
{
    return (-1);
}

int Dom32IoPi::ConfigIO(int port, int io, int config)
{


    if(port == 1)
        m_pi_config_data.port[io-1].config = (config)?INPUT:OUTPUT;
    else if(port == 2)
        m_pi_config_data.port[io+15].config = (config)?INPUT:OUTPUT;


            pinMode(gpio_pin[GPIO_A01], (ioconfig & 0x01)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_A02], (ioconfig & 0x02)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_A03], (ioconfig & 0x04)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_A04], (ioconfig & 0x08)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_A05], (ioconfig & 0x10)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_A06], (ioconfig & 0x20)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_A07], (ioconfig & 0x40)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_A08], (ioconfig & 0x80)?INPUT:OUTPUT);

            m_pi_config_io_file_data[GPIO_A01] = (ioconfig & 0x01)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_A02] = (ioconfig & 0x02)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_A03] = (ioconfig & 0x04)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_A04] = (ioconfig & 0x08)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_A05] = (ioconfig & 0x10)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_A06] = (ioconfig & 0x20)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_A07] = (ioconfig & 0x40)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_A08] = (ioconfig & 0x80)?INPUT:OUTPUT;
            break;
        case 2:
            pinMode(gpio_pin[GPIO_B01], (ioconfig & 0x01)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_B02], (ioconfig & 0x02)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_B03], (ioconfig & 0x04)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_B04], (ioconfig & 0x08)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_B05], (ioconfig & 0x10)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_B06], (ioconfig & 0x20)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_B07], (ioconfig & 0x40)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_B08], (ioconfig & 0x80)?INPUT:OUTPUT);

            m_pi_config_io_file_data[GPIO_B01] = (ioconfig & 0x01)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_B02] = (ioconfig & 0x02)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_B03] = (ioconfig & 0x04)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_B04] = (ioconfig & 0x08)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_B05] = (ioconfig & 0x10)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_B06] = (ioconfig & 0x20)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_B07] = (ioconfig & 0x40)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_B08] = (ioconfig & 0x80)?INPUT:OUTPUT;
            break;
        case 3:
            pinMode(gpio_pin[GPIO_C01], (ioconfig & 0x01)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_C02], (ioconfig & 0x02)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_C03], (ioconfig & 0x04)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_C04], (ioconfig & 0x08)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_C05], (ioconfig & 0x10)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_C06], (ioconfig & 0x20)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_C07], (ioconfig & 0x40)?INPUT:OUTPUT);
            pinMode(gpio_pin[GPIO_C08], (ioconfig & 0x80)?INPUT:OUTPUT);

            m_pi_config_io_file_data[GPIO_C01] = (ioconfig & 0x01)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_C02] = (ioconfig & 0x02)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_C03] = (ioconfig & 0x04)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_C04] = (ioconfig & 0x08)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_C05] = (ioconfig & 0x10)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_C06] = (ioconfig & 0x20)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_C07] = (ioconfig & 0x40)?INPUT:OUTPUT;
            m_pi_config_io_file_data[GPIO_C08] = (ioconfig & 0x80)?INPUT:OUTPUT;
            break;
    }

    SaveConfig();

    return (*config);
}

int Dom32IoPi::SetIO(int mask, int port, int *iostatus)
{
    SWITCH(port)
    {
        case 1:
            if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_A01], HIGH); }
            if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_A02], HIGH); }
            if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_A03], HIGH); }
            if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_A04], HIGH); }
            if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_A05], HIGH); }
            if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_A06], HIGH); }
            if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_A07], HIGH); }
            if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_A08], HIGH); }
            break;
        case 2:
            if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_B01], HIGH); }
            if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_B02], HIGH); }
            if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_B03], HIGH); }
            if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_B04], HIGH); }
            if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_B05], HIGH); }
            if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_B06], HIGH); }
            if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_B07], HIGH); }
            if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_B08], HIGH); }
            break;
        case 3:
            if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_C01], HIGH); }
            if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_C02], HIGH); }
            if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_C03], HIGH); }
            if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_C04], HIGH); }
            if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_C05], HIGH); }
            if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_C06], HIGH); }
            if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_C07], HIGH); }
            if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_C08], HIGH); }
            break;
    }

    return GetIOStatus(port, iostatus);
}

int Dom32IoPi::ResetIO(int mask, int port, int *iostatus)
{
    switch(port)
    {
        case 1:
            if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_A01], LOW); }
            if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_A02], LOW); }
            if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_A03], LOW); }
            if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_A04], LOW); }
            if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_A05], LOW); }
            if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_A06], LOW); }
            if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_A07], LOW); }
            if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_A08], LOW); }
            break;
        case 2:
            if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_B01], LOW); }
            if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_B02], LOW); }
            if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_B03], LOW); }
            if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_B04], LOW); }
            if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_B05], LOW); }
            if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_B06], LOW); }
            if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_B07], LOW); }
            if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_B08], LOW); }
            break;
        case 3:
            if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_C01], LOW); }
            if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_C02], LOW); }
            if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_C03], LOW); }
            if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_C04], LOW); }
            if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_C05], LOW); }
            if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_C06], LOW); }
            if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_C07], LOW); }
            if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_C08], LOW); }
            break;
    }

    return GetIOStatus(port, iostatus);
}

int Dom32IoPi::SwitchIO(int mask, int *iostatus)
{
    (*iostatus) = GetIOStatus(port, iostatus);

    switch(port)
    {
        case 1:
            if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_A01], ((*iostatus)&0x01)?LOW:HIGH); }
            if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_A02], ((*iostatus)&0x02)?LOW:HIGH); }
            if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_A03], ((*iostatus)&0x04)?LOW:HIGH); }
            if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_A04], ((*iostatus)&0x08)?LOW:HIGH); }
            if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_A05], ((*iostatus)&0x01)?LOW:HIGH); }
            if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_A06], ((*iostatus)&0x02)?LOW:HIGH); }
            if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_A07], ((*iostatus)&0x04)?LOW:HIGH); }
            if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_A08], ((*iostatus)&0x08)?LOW:HIGH); }
            break;
        case 2:
            if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_B01], ((*iostatus)&0x01)?LOW:HIGH); }
            if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_B02], ((*iostatus)&0x02)?LOW:HIGH); }
            if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_B03], ((*iostatus)&0x04)?LOW:HIGH); }
            if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_B04], ((*iostatus)&0x08)?LOW:HIGH); }
            if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_B05], ((*iostatus)&0x01)?LOW:HIGH); }
            if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_B06], ((*iostatus)&0x02)?LOW:HIGH); }
            if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_B07], ((*iostatus)&0x04)?LOW:HIGH); }
            if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_B08], ((*iostatus)&0x08)?LOW:HIGH); }
            break;
        case 3:
            if(mask & 0x01) { digitalWrite(gpio_pin[GPIO_C01], ((*iostatus)&0x01)?LOW:HIGH); }
            if(mask & 0x02) { digitalWrite(gpio_pin[GPIO_C02], ((*iostatus)&0x02)?LOW:HIGH); }
            if(mask & 0x04) { digitalWrite(gpio_pin[GPIO_C03], ((*iostatus)&0x04)?LOW:HIGH); }
            if(mask & 0x08) { digitalWrite(gpio_pin[GPIO_C04], ((*iostatus)&0x08)?LOW:HIGH); }
            if(mask & 0x10) { digitalWrite(gpio_pin[GPIO_C05], ((*iostatus)&0x01)?LOW:HIGH); }
            if(mask & 0x20) { digitalWrite(gpio_pin[GPIO_C06], ((*iostatus)&0x02)?LOW:HIGH); }
            if(mask & 0x40) { digitalWrite(gpio_pin[GPIO_C07], ((*iostatus)&0x04)?LOW:HIGH); }
            if(mask & 0x80) { digitalWrite(gpio_pin[GPIO_C08], ((*iostatus)&0x08)?LOW:HIGH); }
            break;
    }

    return GetIOStatus(iostatus);
}

int Dom32IoPi::PulseIO(int /*mask*/, int /*sec*/, int * /*iostatus*/)
{
    return (-1);
}

void Dom32IoPi::SetStatusLed(int status)
{
    digitalWrite(gpio_pin[GPIO_STATUS_LED], (status)?HIGH:LOW); // Turn LED ON/OFF
}

void Dom32IoPi::SetConfig( void )
{
    pinMode(gpio_pin[m_pi_config_data.port[0].map], m_pi_config_data.port[0].config);
    pinMode(gpio_pin[m_pi_config_data.port[1].map], m_pi_config_data.port[1].config);
    pinMode(gpio_pin[m_pi_config_data.port[2].map], m_pi_config_data.port[2].config);
    pinMode(gpio_pin[m_pi_config_data.port[3].map], m_pi_config_data.port[3].config);
    pinMode(gpio_pin[m_pi_config_data.port[4].map], m_pi_config_data.port[4].config);
    pinMode(gpio_pin[m_pi_config_data.port[5].map], m_pi_config_data.port[5].config);
    pinMode(gpio_pin[m_pi_config_data.port[6].map], m_pi_config_data.port[6].config);
    pinMode(gpio_pin[m_pi_config_data.port[7].map], m_pi_config_data.port[7].config);
    
    pinMode(gpio_pin[m_pi_config_data.port[8].map], m_pi_config_data.port[8].config);
    pinMode(gpio_pin[m_pi_config_data.port[9].map], m_pi_config_data.port[9].config);

    pinMode(gpio_pin[m_pi_config_data.port[16].map], m_pi_config_data.port[16].config);
    pinMode(gpio_pin[m_pi_config_data.port[17].map], m_pi_config_data.port[17].config);
    pinMode(gpio_pin[m_pi_config_data.port[18].map], m_pi_config_data.port[18].config);
    pinMode(gpio_pin[m_pi_config_data.port[19].map], m_pi_config_data.port[19].config);
    pinMode(gpio_pin[m_pi_config_data.port[20].map], m_pi_config_data.port[20].config);
    pinMode(gpio_pin[m_pi_config_data.port[21].map], m_pi_config_data.port[21].config);
    pinMode(gpio_pin[m_pi_config_data.port[22].map], m_pi_config_data.port[22].config);
    pinMode(gpio_pin[m_pi_config_data.port[23].map], m_pi_config_data.port[23].config);

    pinMode(gpio_pin[GPIO_MODE_LED], m_pi_config_io_file_data[GPIO_MODE_LED]);
    pinMode(gpio_pin[GPIO_STATUS_LED], m_pi_config_io_file_data[GPIO_STATUS_LED]);
    pinMode(gpio_pin[GPIO_TX], m_pi_config_io_file_data[GPIO_TX]);
    pinMode(gpio_pin[GPIO_RX], m_pi_config_io_file_data[GPIO_RX]);
}
