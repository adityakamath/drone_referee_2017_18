/*
 * network_config.c
 *
 *  Created on: Dec 23, 2015
 *      Author: Koen Meessen
 */

#include "rtdbdefs.h"
#include "network_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PERR(txt, par...) \
	printf("ERROR: (%s / %s): " txt "\n", __FILE__, __FUNCTION__, ## par)

int set_network_iface(network_config_t* config, char * name)
{
	config->iface = name;

	return 0;
}

static int read_configuration(char **network_name, network_config_t* config)
{
	/* if network_name == NULL, the default network will be returned */
	FILE * fp;
	void* rc;
	char tline[256];
	char str[256];
	char internal_network_name[256];
	char *ptr;
	int found = 0;
	int use_default = 0;
	fp = fopen(NETWORK_CONFIG_FILE,"r");

	if(fp == NULL)
	{
		PERR("Could not open %s",NETWORK_CONFIG_FILE);
		return -1;
	}

	/* reset network_config */
	memset(config, 0, sizeof(network_config_t));

	/* if no network name is provided, use the default network */
	if(*network_name == NULL)
	{
		use_default = 1;
	}

	do{
		rc = fgets(tline, 200, fp);

		if(rc == 0)
			break;

		if(tline[0] == '#')
			continue;

		if(strstr(tline,"NETWORK") == tline)
		{
			/* check if the network was already found, then we can directly stop */
			if(found){
				break;
			}
			/* obtain name of the network */
			sscanf(tline,"%*s %s",internal_network_name);

			if(!use_default)
			{
				if(strcmp(*network_name,internal_network_name) == 0){
					/* use this network if the network_name is correct */
					found = 1;
				}
			}
		}

		if(found || use_default)
		{
			if((ptr = strstr(tline,"multicast_ip")) != NULL)
			{
				if( sscanf(ptr, "%*[^=]=%*[ ]%127[^;]", str) > 0){
					memcpy(config->multicast_ip,str,16*sizeof(char));
				}
			}
			if((ptr = strstr(tline,"multicast_port")) != NULL)
			{
				if( sscanf(ptr, "%*[^=]=%*[ ]%127[^;]", str) > 0){
					config->port = atoi(str);
				}
			}
			if((ptr = strstr(tline,"frequency")) != NULL)
			{
				if( sscanf(ptr, "%*[^=]=%*[ ]%127[^;]", str) > 0){
					config->frequency = atoi(str);
				}
			}
			if((ptr = strstr(tline,"default")) != NULL)
			{
				if( sscanf(ptr, "%*[^=]=%*[ ]%127[^;]", str) > 0){
					if(strncasecmp(str,"true", 4) == 0)
					{
						found = 1;
						*network_name = strdup(internal_network_name);
					}
				}
			}
		}
	}while(rc);

	fclose(fp);
	if(!found && use_default){
		PERR("Could not find 'default' network");
		return -1;
	}
	if(!found){
		PERR("Could not find correct network");
		return -1;
	}
	if(config->multicast_ip[0] == 0){
		PERR("No multicast_ip specified in network.conf");
		return -1;
	}
	if(config->port == 0){
		PERR("No port specified in network.conf");
		return -1;
	}
	if(config->frequency == 0){
		PERR("No frequency specified in network.conf");
		return -1;
	}
	return 0;
}
// Retrieve network configuration, read from network.conf.
//	INOUT:
//		char **network_name 	 : network name, if NULL, network_name will be set to the correct name
//		network_config_t* config : network configuration struct to be filled by this function
//
int get_network_config(char **network_name, network_config_t* config)
{
	return read_configuration(network_name, config);
}
