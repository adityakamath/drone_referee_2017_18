/*
 * network_config.h
 *
 *  Created on: Dec 23, 2015
 *      Author: Koen Meessen
 */

#ifndef NETWORK_CONFIG_H_
#define NETWORK_CONFIG_H_

typedef struct tag_network_config
{
	char 			multicast_ip[16];
	char*			iface;
	unsigned int	port;
	unsigned int	frequency;
}network_config_t;

int get_network_config(char **network_name, network_config_t* config);
int set_network_iface(network_config_t* config, char * name);

#endif /* NETWORK_CONFIG_H_ */
