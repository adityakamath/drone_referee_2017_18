#ifndef __RTDBDEFS_H
#define __RTDBDEFS_H

/* #define DEBUG */



#define CONFIG_FILE	"/usr/config/rtdb.ini"
#define NETWORK_CONFIG_FILE	"/usr/config/network.conf"

#define SHMEM_KEY 0x2000


#define MAX_AGENTS 35
#define MAX_RECS 100
#define MAX_NETWORKS 5

typedef struct
{
	int id;
	int size;
	int period;
} RTDBconf_var;

typedef struct
{
	int n_shared_recs;
	int shared_mem_size;
	RTDBconf_var shared[MAX_RECS];
	int n_local_recs;
	int local_mem_size;
	RTDBconf_var local[MAX_RECS];
    char version[16]; 					// md5sum of the .ini file.
} RTDBconf_agents;

typedef struct
{
	int n_shared_recs;
	int shared_mem_size;
	RTDBconf_var shared[MAX_RECS];
} RTDBconf_agents_comm;

typedef struct
{
	char* network_name;
	int network_id;
	int shared_record_present[MAX_RECS];
} RTDBconf_network;

typedef struct
{
	int n_networks;
	RTDBconf_network network[MAX_NETWORKS];
} RTDBconf_networks;

#endif
