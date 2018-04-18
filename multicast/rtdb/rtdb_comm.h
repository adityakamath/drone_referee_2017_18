#ifndef __RTDB_COMM_H
#define __RTDB_COMM_H


#include "rtdb_api.h"


//	*************************
//	DB_comm_put: Escreve na base de dados - apenas para outros agentes!
//
//	Entrada:
//		int _agent = numero do agente
//		int _id = identificador da 'variavel'
//		void *_value = ponteiro com os dados
//		int life = tempo de vida da 'variavel' em ms
//	Saida:
//		0 = OK
//		-1 = erro
//
int DB_comm_put (int _agent, int _id, int _size, void *_value, int life);


//	*************************
//	DB_comm_ini:
//
//	Entrada:
//		RTDBconf_agents *rtdb_conf_for_comm = pointer to configuration struct
// 		char* network_name = name of the network, use to obtain the network settings
//								from network.conf, and to obtain the shared records
//								to be shared in this network.
//
//	Saida:
//		int n = numero the configured agents in rtdb.conf
//
//
int DB_comm_ini(RTDBconf_agents_comm *rtdb_conf_for_comm, char* network_name);

/**
 * Copy shared data of given agent into local repesentation
 * of shared data. Requires DB_comm_init_peer(agent_id)
 * By Peter Soetens.
 */
int DB_comm_copy_agent(int agent_id);


#endif
