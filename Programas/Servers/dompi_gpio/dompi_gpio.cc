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
#include <gmonitor/gmerror.h>
#include <gmonitor/gmontdb.h>
/*#include <gmonitor/gmstring.h>*/
#include <gmonitor/gmswaited.h>

#include <string>
#include <iostream>
#include <csignal>
using namespace std;

#include <unistd.h>
#include <time.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>

#include <cjson/cJSON.h>

#include "dom32iopi.h"
#include "config.h"

#define MAX_BUFFER_LEN 32767

CGMServerWait *m_pServer;
DPConfig *pConfig;
void OnClose(int sig);

int internal_timeout;

/* POST
* 1.- %s: URI
* 2.- %s: Host
* 3.- %d: Content-Length
* 4.- %s: datos
*/
http_post =     "POST %s HTTP/1.1\r\n"
				"Host: %s\r\n"
				"Connection: keep-alive\r\n"
				"Content-Length: %d\r\n"
				"User-Agent: DomPiSrv/1.00 (RaspBerryPi;Dom32)\r\n"
				"Accept: text/html,text/xml\r\n"
				"Content-Type: application/x-www-form-urlencoded\r\n\r\n%s";

/*
* GET
* 1.- %s: URI
* 2.- %s: Host
*/
http_get =     "GET %s HTTP/1.1\r\n"
					"Host: %s\r\n"
					"Connection: close\r\n"
					"User-Agent: DomPiSrv/1.00 (RaspBerryPi;Dom32)\r\n"
					"Accept: text/html,text/xml\r\n\r\n";

void HTTPNotificarStatus( void )
{
	char http_rqst_data{1024};
	char http_rqst[4096];
	int first_change = 1;


    snprintf(http_rqst_data, 1024,
        "ID=%s&IO1=%i&IO2=%i&IO3=%i&IO4=%i&IO5=%i&IO6=%i&IO7=%i&IO8=%i&OUT1=%i&OUT2",
        hw_mac_address,
        io1, io2, io3, io4, io5, io6, io7, io8,
		out1, out2);
    
    if( change_io1  ||
        change_io2  ||
        change_io3  ||
        change_io4  ||
        change_io5  ||
        change_io6  ||
        change_io7  ||
        change_io8  ||
        change_out1 ||
        change_out2  )
    {
        strcat(http_rqst_data, "&CHG=");
        
        if(change_io1)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "IO1");
        }
        if(change_io2)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "IO2");
        }
        if(change_io3)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "IO3");
        }
        if(change_io4)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "IO4");
        }
        if(change_io5)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "IO5");
        }
        if(change_io6)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "IO6");
        }
        if(change_io7)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "IO7");
        }
        if(change_io8)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "IO8");
        }
        if(g_control_out.change1)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "OUT1");
        }
        if(g_control_out.change2)
        {
            if(!first_change) strcat(http_rqst_data, ",");
            first_change = 0;
            strcat(http_rqst_data, "OUT2");
        }
    }
    

    /* Agrego solicitud de configuración */
    if(g_wifi_info.f.config.io_config_default && (MAX_HTTP_DATA - strlen(http_rqst_data) > 10) )
    {
        strcat(http_rqst_data, "&GETCONF=1");
    }
    
    snprintf(http_rqst_uri, MAX_HTTP_URI, "%s/infoio.cgi", g_wifi_info.f.config.rqst_path);
/* POST
 * 1.- %s: URI
 * 2.- %s: Host
 * 3.- %d: Content-Length
 * 4.- %s: datos
 */
    snprintf(http_rqst, MAX_HTTP_DATA + MAX_HTTP_HEAD, http_post,
            http_rqst_uri,
            g_wifi_info.f.config.wifi_host1,
            strlen(http_rqst_data),
            http_rqst_data);

    LogMessage("HTTPNotificarStatus - data-len: %d", strlen(http_rqst));

    if(q.Query(raddr, 80, buffer, buffer, BUFFER_LEN, m_timeout) > 0)
    {
        if(m_pLog) m_pLog->Add(100, "[Dom32IoWifi] GetConfig Receive [%s]", buffer);
        rc = HttpRespCode(buffer);
        if(rc != 200) return rc;
        /* Me posiciono al final de la cabecera HTTP, al principio de los datos */
        p = strstr(buffer, "\r\n\r\n");
        if(p)
        {
            /* Salteo CR/LF CR/LF */
            p += 4;
            for(i = 0; Str.ParseDataIdx(p, label, value, i); i++)
            {


			}
		}
	}





    /* Reseteo los cambios */
    g_control_io.change1 = 0;
    g_control_io.change2 = 0;
    g_control_io.change3 = 0;
    g_control_io.change4 = 0;
    g_control_io.change5 = 0;
    g_control_io.change6 = 0;
    g_control_out.change1 = 0;
    g_control_out.change2 = 0;
    g_control_out.change3 = 0;
    g_control_out.change4 = 0;




}

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[MAX_BUFFER_LEN+1];
	unsigned long message_len;
	unsigned char blink;
	//unsigned long exclude_modem = 0;

	int current_io_status;
	int previus_io_status = 0;
	int current_ex_status;
	int previus_ex_status = 0;
	int delta_io_status;
	int delta_ex_status;
	int timer_count_keep_alive;
	char temp_s[64];
	char s[16];

	signal(SIGPIPE, SIG_IGN);
	signal(SIGKILL,         OnClose);
	signal(SIGTERM,         OnClose);
	signal(SIGSTOP,         OnClose);
	signal(SIGABRT,         OnClose);
	signal(SIGQUIT,         OnClose);
	signal(SIGINT,          OnClose);
	signal(SIGILL,          OnClose);
	signal(SIGFPE,          OnClose);
	signal(SIGSEGV,         OnClose);
	signal(SIGBUS,          OnClose);

	m_pServer = new CGMServerWait;
	m_pServer->Init("dompi_gpio");
	m_pServer->m_pLog->Add(1, "Iniciando interface GPIO");

	pConfig = new DPConfig("/etc/dompiweb.config");
	internal_timeout = 1000;
	if( pConfig->GetParam("INTERNAL-TIMEOUT", s))
	{
		internal_timeout = atoi(s) * 1000;
	}

	if( pConfig->GetParam("MAC-ADDRESS", hw_mac_address))
	{
		strcpy(hw_mac_address, "000000000000");
	}

    cJSON *json_Message;

	int return_int1;
	int return_int2;

    Dom32IoPi *pPI;
    pPI = new Dom32IoPi();
	pPI->LoadConfig();
	
	m_pServer->Suscribe("dompi_pi_ioconfig", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_pi_iostatus", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_pi_ioswitch", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_pi_wifi", GM_MSG_TYPE_CR);

	blink = 0;
	timer_count_keep_alive = 0;
	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, 10 )) >= 0)
	{
		json_req = NULL;

		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(50, "%s:(Q)[%s]", fn, message);

			json_Message = cJSON_Parse(message);
			message[0] = 0;
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			if( !strcmp(fn, "dompi_pi_ioconfig"))
			{





				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}

				cJSON_Delete(json_Message);
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_pi_iostatus"))
			{





				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}

				cJSON_Delete(json_Message);
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_pi_ioswitch"))
			{





				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}

				cJSON_Delete(json_Message);
			}
			/* ************************************************************* *
			 *
			 * ************************************************************* */
			else if( !strcmp(fn, "dompi_pi_wifi"))
			{





				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}

				cJSON_Delete(json_Message);
			}




			else
			{
				m_pServer->m_pLog->Add(50, "GME_SVC_NOTFOUND");
				m_pServer->Resp(NULL, 0, GME_SVC_NOTFOUND);
			}
		}
		else
		{
			/* expiracion del timer */

			timer_count_keep_alive++;
			if(timer_count_keep_alive >= 600)	/* 60 segundos */
			{
				timer_count_keep_alive = 0;
				send_keep_alive_report();
			}






		}
		
	}
	m_pServer->m_pLog->Add(50, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);
	m_pServer->UnSuscribe("dompi_pi_ioconfig", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_pi_iostatus", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_pi_ioswitch", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_pi_wifi", GM_MSG_TYPE_CR);

	delete m_pServer;
	exit(0);
}
