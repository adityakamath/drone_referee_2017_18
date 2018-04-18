#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>

#include "rtdb_sem.h"
#include "rtdb_api.h"
#include "rtdbdefs.h"

/* for semid */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define SELF 3333
/*#define DEBUG */

#define PERRNO(txt) \
	printf("ERROR: (%s / %s): " txt ": %s\n", __FILE__, __FUNCTION__, strerror(errno))

#define PERR(txt, par...) \
	printf("ERROR: (%s / %s): " txt "\n", __FILE__, __FUNCTION__, ## par)

#ifdef DEBUG
#define PDEBUG(txt, par...) \
	printf("DEBUG: (%s / %s): " txt "\n", __FILE__, __FUNCTION__, ## par)
#else
#define PDEBUG(txt, par...)
#endif


typedef struct
{
	int id;							// id da 'variavel'
	int size;						// sizeof da 'variavel'
	int period;						// refresh period for broadcast
	int offset;						// offset para o campo de dados da 'variavel'
	int read_bank;					// variavel mais actual
	struct timeval timestamp[2];	// relogio da maquina local
} TRec;


typedef struct
{
	int self_agent;						// numero do agente onde esta a correr
	int n_agents;						// numero total de agentes registados
	int n_shared_recs[MAX_AGENTS];		// numero de 'variaveis' shared
	int n_local_recs;					// numero de 'variaveis' local
	int shared_mem_size[MAX_AGENTS];	// tamanho das areas shared
	int local_mem_size;					// tamanho da area local
	int rec_lut[MAX_AGENTS][MAX_RECS];	// lookuptable
    char version[16]; 					// md5sum of the .ini file.
} RTDBdef;

/**
 * This struct is created for each agent.
 */
typedef struct
{
        int def_shmid;                          // identificador da area de definicoes
        RTDBdef *p_def;                         // ponteiro para a area de definicoes

        int shared_shmid[MAX_AGENTS];   // array de identificadores das areas shared
        void *p_shared_mem[MAX_AGENTS]; // array de ponteiros para as areas shared

        int local_shmid;                                // identificador da area local
        void *p_local_mem;                              // ponteiro para a area local
        int sem_id;                     // semaphore to protect shared memory.
} RTDB_peer;

typedef enum {
	agent_config = 0,
	network_config
} config_item_t;

/**
 * Local copy of our agent id.
 */
int my_id = 0;

/**
 * Local buffer to store the results of copying agent data
 * Initialize the pointer to NULL.
 */
void* comm_copy_buf = NULL;

RTDB_peer peers[MAX_AGENTS];

/**
 * Represents our global config file.
 */
RTDBconf_agents rtdb_conf[MAX_AGENTS];

// Static declarations
static TRec* get_record_in_lut(int _agent, int _id);
static int read_configuration(RTDBconf_agents *conf, RTDBconf_networks *network_conf);

//	*************************
//	read_configuration: Le as configuracoes da RTDB a partir do ficheiro
//		de configuracao CONFIG_FILE
//
//	Saida:
//		numero total de agentes registados
//		-1 se erro
//
static int read_configuration(RTDBconf_agents *conf, RTDBconf_networks *network_conf)
{
	FILE *f_def;
	int rc;
	char s[100];
	int agent;
	int network_id = -1;
	int id, size, period;
	char type;
	char str[100];
	int n_agents = -1;

	config_item_t config_item = 0;

	if ((f_def = fopen(CONFIG_FILE, "r")) == NULL)
	{
		PERRNO("fopen");
		return -1;
	}

	/* reset network configuration */
	memset(network_conf, 0, sizeof(RTDBconf_networks));

	/* reset agent configuration */
	for (agent=0;agent<MAX_AGENTS;agent++){
		conf[agent].n_shared_recs = 0;
		conf[agent].n_local_recs = 0;
	}

	do
	{
		rc = fscanf(f_def, "%100[^\n]", s);
		fgetc(f_def);

		if (rc == -1)
			break;

		if (rc != 0)
		{
			if(s[0] == '<')
			{
				sscanf(s,"<%127[^>]", str);
				if(strncmp(str,"AGENTS",100) == 0){
					config_item = agent_config;
				}else if(strncmp(str,"NETWORKS",100) == 0){
					config_item = network_config;
				}else{
					printf("ERROR: unknown config type : %s\n",str);
					return -1;
				}
			}
			else
			{
				switch (config_item){
					case agent_config:
						if (s[0] != '#')
						{
							sscanf(s, "%d\t%d\t%d\t%c\n", &id, &size, &period, &type);
							if (type == 's')
							{
								conf[agent].shared[conf[agent].n_shared_recs].id = id;
								conf[agent].shared[conf[agent].n_shared_recs].size = size;
								conf[agent].shared[conf[agent].n_shared_recs].period = period;
								conf[agent].n_shared_recs ++;
								conf[agent].shared_mem_size += size;
							}
							else
							{
								conf[agent].local[conf[agent].n_local_recs].id = id;
								conf[agent].local[conf[agent].n_local_recs].size = size;
								conf[agent].local[conf[agent].n_local_recs].period = period;
								conf[agent].n_local_recs ++;
								conf[agent].local_mem_size += size;
							}
							if ((conf[agent].n_shared_recs + conf[agent].n_local_recs) > MAX_RECS)
							{
								PERR("Increase MAX_RECS");
								return -1;
							}
						}
						else
						{
							sscanf(s+1, "%d\n", &agent);
							if (agent >= MAX_AGENTS)
							{
								PERR("Increase MAX_AGENTS");
								return -1;
							}
							n_agents ++;
							conf[n_agents].n_shared_recs = 0;
							conf[n_agents].n_local_recs = 0;
							conf[n_agents].shared_mem_size = 0;
							conf[n_agents].local_mem_size = 0;
						}
						break;
					case network_config:
						if (s[0] != '#')
						{
							sscanf(s, "%d\n", &id);

							if (id >= MAX_RECS)
							{
								PERR("Increase MAX_RECS");
								return -1;
							}
							/* mark shared record present in the current network */
							network_conf->network[network_id].shared_record_present[id] = 1;
						}
						else
						{
							sscanf(s+1, "%d\t%s\n", &network_id, str);

							if (network_id >= MAX_NETWORKS)
							{
								PERR("Increase MAX_NETWORKS");
								return -1;
							}

							network_conf->network[network_id].network_id = network_id;
							network_conf->network[network_id].network_name = strdup(str);
							network_conf->n_networks++;

						}
						break;
					default :
						break;
				}
			}

		}

	} while (rc != -1);

	fclose(f_def);
	n_agents ++;

#ifdef DEBUG
	int j,i;

	for (i = 0; i < n_agents; i++)
	{
		printf("Agent : %d\n", i);
		for (j = 0; j < conf[i].n_shared_recs; j++)
			printf("  Shared: id: %d, size: %d, period: %d\n", conf[i].shared[j].id, conf[i].shared[j].size, conf[i].shared[j].period);
	}
	for (i = 0; i < network_conf->n_networks; i++)
	{
		printf("Network : %s\n", network_conf->network[i].network_name);

		for (j = 0; j < MAX_RECS; j++){
			if(network_conf->network[i].shared_record_present[j]){
				printf("  Shared record id: %d\n", j);
			}
		}

	}
#endif

	return (n_agents);
}


int DB_use_count()
{
  struct shmid_ds shmem_status;

  int key,mdef_shmid;
  char* environment;

  if((environment = getenv("AGENT")) == NULL)
    {
      PERR("getenv: AGENT variable not found.");
      return -1;
    }

  key = SHMEM_KEY + (atoi(environment) * MAX_AGENTS * 2);
  mdef_shmid = shmget(key, sizeof(RTDBdef), 0644 );
  if (mdef_shmid == -1)
    {
      return 0;
    }

  if (shmctl(mdef_shmid, IPC_STAT, &shmem_status) == -1) {
    PERRNO("shmctl");
    return -1;
  }
  // success:
  return shmem_status.shm_nattch;
}


//	*************************
//	DB_free: Liberta acesso a base de dados
//
void DB_free (void)
{
	int i;
	struct shmid_ds shmem_status;

	if (peers[my_id].p_def == 0)
		return;

    // zona shared
	for (i = 0; i < peers[my_id].p_def->n_agents; i++)
	{
		shmdt(peers[my_id].p_shared_mem[i]);

		// se e o ultimo
		if (shmctl(peers[my_id].shared_shmid[i], IPC_STAT, &shmem_status) == -1)
			PERRNO("shmctl");
		printf ("-- shared %d --left attached: %d ---\n", i, (int)shmem_status.shm_nattch);
		if (shmem_status.shm_nattch == 0)
			shmctl(peers[my_id].shared_shmid[i], IPC_RMID, NULL);
	}

	// zona local
	shmdt(peers[my_id].p_local_mem);
	// se e o ultimo
	if (shmctl(peers[my_id].local_shmid, IPC_STAT, &shmem_status) == -1)
		PERRNO("shmctl");
	printf ("-- local --left attached: %d ---\n", (int)shmem_status.shm_nattch);
	if (shmem_status.shm_nattch == 0)
		shmctl(peers[my_id].local_shmid, IPC_RMID, NULL);

	// zona def
	shmdt(peers[my_id].p_def);
	// se e o ultimo
	if (shmctl(peers[my_id].def_shmid, IPC_STAT, &shmem_status) == -1)
		PERRNO("shmctl");
	printf ("-- def --left attached: %d ---\n", (int)shmem_status.shm_nattch);
	if (shmem_status.shm_nattch == 0)
		shmctl(peers[my_id].def_shmid, IPC_RMID, NULL);
    
    peers[my_id].p_def = 0;

    if (comm_copy_buf != NULL) {
    	free(comm_copy_buf);
    	comm_copy_buf = NULL;
    }
}


    
//	*************************
//	DB_init: Aloca acesso a base de dados
//
//	Saida:
//		0 = OK
//		-1 = erro
//
int DB_init (void)
{
	char *environment;
   	// resolucao do numero do agent
	if((environment = getenv("AGENT")) == NULL)
	{
        PERR("getenv: AGENT variable not found.");
		return -1;
	}

	my_id = atoi(environment);
	PDEBUG("agent = %d", my_id);
    return DB_init_agent( my_id );
}

int DB_init_agent(int agent_id )
{	int i, j;
	int key;
	int offset;
	int max_shared_mem_size = 0;
	TRec *p_rec;
	struct shmid_ds shmem_status;
	RTDBconf_networks network_conf; /* unused here */

    if (peers[agent_id].p_def) {
        printf("Skipping init of agent %d\n",agent_id);
        return 0; // DB_init already done.
	}

    printf("Doing init of agent %d\n",agent_id);

	// aloca memoria para a zona de definicoes
	key = SHMEM_KEY + (agent_id * MAX_AGENTS * 2);
	peers[agent_id].def_shmid = shmget(key, sizeof(RTDBdef), 0644 | IPC_CREAT);
	if (peers[agent_id].def_shmid == -1)
	{
		PERRNO("shmget");
		return -1;
	}

    // the key number of the semaphore is the same as the main shm key.
    peers[agent_id].sem_id = rtdb_sem_open(key);
    if (peers[agent_id].sem_id == -1) {
        PERRNO("DB_init::rtdb_sem_open");
        DB_free();
        return -1;
    }

	key++;
	peers[agent_id].p_def = (RTDBdef*)shmat(peers[agent_id].def_shmid, (void *)0, 0);
	if ((char *)peers[agent_id].p_def == (char *)(-1))
	{
        PERRNO("shmat");
		DB_free();
		return -1;
	}

	// se e o primeiro, e necessario carregar as definicoes
	if (shmctl(peers[agent_id].def_shmid, IPC_STAT, &shmem_status) == -1)
	{
		PERRNO("shmctl");
		DB_free();
		return -1;
	}

    // is this ever != 1 ???
	if (shmem_status.shm_nattch == 1)
	{
		// carregar configuracoes
		if ((peers[agent_id].p_def->n_agents = read_configuration(rtdb_conf, &network_conf)) < 1)
		{
			PERR("read_configuration");
			DB_free();
			return -1;
		}

		printf("Setting up memory for receiving from %d agents.\n",peers[agent_id].p_def->n_agents);
		peers[agent_id].p_def->self_agent = agent_id;

        int tmp_local_mem_size = 0;
		for (i = 0; i < peers[agent_id].p_def->n_agents; i++)
		{
            int tmp_shared_mem_size = 0;

			if (i == peers[agent_id].p_def->self_agent)
			{
				peers[agent_id].p_def->n_local_recs = rtdb_conf[i].n_local_recs;
				for (j = 0; j < peers[agent_id].p_def->n_local_recs; j++)
					tmp_local_mem_size += rtdb_conf[i].local[j].size;
				tmp_local_mem_size = tmp_local_mem_size * 2 + sizeof (TRec) * peers[agent_id].p_def->n_local_recs;
			}
			peers[agent_id].p_def->n_shared_recs[i] = rtdb_conf[i].n_shared_recs;
			for (j = 0; j < peers[agent_id].p_def->n_shared_recs[i]; j++)
				tmp_shared_mem_size += rtdb_conf[i].shared[j].size;

			// calcula tamanho de dados a alocar
			tmp_shared_mem_size = tmp_shared_mem_size * 2 + sizeof (TRec) * peers[agent_id].p_def->n_shared_recs[i];

            peers[agent_id].p_def->shared_mem_size[i] = tmp_shared_mem_size;
            // keep track of largest segment
            if ( tmp_shared_mem_size > max_shared_mem_size)
            	max_shared_mem_size = tmp_shared_mem_size;
		}

        peers[agent_id].p_def->local_mem_size = tmp_local_mem_size;
	} else {
		assert( peers[agent_id].p_def );
		// get size of shared memory segment for each agent.
		for (i = 0; i < peers[agent_id].p_def->n_agents; i++) {
            int tmp_shared_mem_size = peers[agent_id].p_def->shared_mem_size[i];
            if ( tmp_shared_mem_size > max_shared_mem_size)
            	max_shared_mem_size = tmp_shared_mem_size;
		}
	}
	// the scomm copy buffer is equal to the largest shared memory segment.
    assert(max_shared_mem_size);
    if ( comm_copy_buf != NULL ) {
    	free(comm_copy_buf);
    	comm_copy_buf = NULL;
    }
    comm_copy_buf = malloc(max_shared_mem_size);
	assert(comm_copy_buf);

	// aloca memoria para a zona shared
	for (i = 0; i < peers[agent_id].p_def->n_agents; i++)
	{
		peers[agent_id].shared_shmid[i] = shmget(key, peers[agent_id].p_def->shared_mem_size[i], 0644 | IPC_CREAT);
		if (peers[agent_id].shared_shmid[i] == -1)
		{
			PERRNO("shmget2");
            PDEBUG("peers[agent_id].p_def->shared_mem_size[i]=%d, key=%d, shmem_status.shm_nattch=%d\n",peers[agent_id].p_def->shared_mem_size[i], key,shmem_status.shm_nattch);
			DB_free();
			return -1;
		} else {
            PDEBUG("SHARED: peers[agent_id].p_def->shared_mem_size[i]=%d, key=%d,shmem_status.shm_nattch=%d\n",peers[agent_id].p_def->shared_mem_size[i], key,shmem_status.shm_nattch);
        }
		key ++;
		peers[agent_id].p_shared_mem[i] = shmat(peers[agent_id].shared_shmid[i], (void *)0, 0);
		if (peers[agent_id].p_shared_mem[i] == (char *)(-1))
		{
			PERRNO("shmat2");
			DB_free();
			return -1;
		}
	}

    PDEBUG("LOCAL: peers[agent_id].p_def->local_mem_size=%d, key=%d,shmem_status.shm_nattch=%d\n",peers[agent_id].p_def->local_mem_size, key,shmem_status.shm_nattch);

	// aloca memoria para a zona local
	if (peers[agent_id].p_def->local_mem_size == 0)
		peers[agent_id].local_shmid = shmget(key, 1, 0644 | IPC_CREAT);
	else
		peers[agent_id].local_shmid = shmget(key, peers[agent_id].p_def->local_mem_size, 0644 | IPC_CREAT);
	if (peers[agent_id].local_shmid == -1)
	{
		PERRNO("shmget3");
		DB_free();
		return -1;
	}
	peers[agent_id].p_local_mem = shmat(peers[agent_id].local_shmid, (void *)0, 0);
	if (peers[agent_id].p_local_mem == (char *)(-1))
	{
		PERRNO("shmat3");
		DB_free();
		return -1;
	}

	// se e o primeiro, e necessario inicializar a memoria
	if (shmctl(peers[agent_id].local_shmid, IPC_STAT, &shmem_status) == -1)
	{
		PERRNO("shmctl");
		DB_free();
		return -1;
	}
	if (shmem_status.shm_nattch > 1)
	{
		PDEBUG("Memory already configurated");
		return 0;
	}

	// inicializacao das areas de memoria
	for (i = 0; i < MAX_AGENTS; i++)
		for (j = 0; j < MAX_RECS; j++)
			peers[agent_id].p_def->rec_lut[i][j] = -1;

	for (i = 0; i < peers[agent_id].p_def->n_agents; i++)
	{
		offset = peers[agent_id].p_def->n_shared_recs[i] * sizeof(TRec);
		for (j = 0; j < peers[agent_id].p_def->n_shared_recs[i]; j++)
		{
			p_rec = (TRec*)((char*)(peers[agent_id].p_shared_mem[i]) + j * sizeof(TRec));
			p_rec->id = rtdb_conf[i].shared[j].id;
			p_rec->size = rtdb_conf[i].shared[j].size;
			p_rec->offset = offset;
			p_rec->period = rtdb_conf[i].shared[j].period;
			p_rec->read_bank = 0;
			peers[agent_id].p_def->rec_lut[i][p_rec->id] = j;
			offset = offset + (p_rec->size * 2) - sizeof(TRec);
			PDEBUG("agent: %d, shared: %d, size: %d, offset:%d, period: %d, lut: %d", i, p_rec->id, p_rec->size, p_rec->offset, p_rec->period, j);
		}
	}

	offset = peers[agent_id].p_def->n_local_recs * sizeof(TRec);
	for (j = 0; j < peers[agent_id].p_def->n_local_recs; j++)
	{
		p_rec = (TRec*)((char*)(peers[agent_id].p_local_mem) + j * sizeof(TRec));
		p_rec->id = rtdb_conf[peers[agent_id].p_def->self_agent].local[j].id;
		p_rec->size = rtdb_conf[peers[agent_id].p_def->self_agent].local[j].size;
		p_rec->offset = offset;
		p_rec->period = rtdb_conf[peers[agent_id].p_def->self_agent].local[j].period;
		p_rec->read_bank = 0;
		peers[agent_id].p_def->rec_lut[peers[agent_id].p_def->self_agent][p_rec->id] = MAX_RECS + j;
		offset = offset + (p_rec->size * 2) - sizeof(TRec);
		PDEBUG("local: %d, size: %d, offset:%d, period: %d, lut: %d", p_rec->id, p_rec->size, p_rec->offset, p_rec->period, MAX_RECS + j);
	}

	return 0;
}

/** Free shared memory allocated to one specific agent.
 *  This is the corresponding function to DB_init_agent(agent_id).
 */
void DB_free_agent(int agent_id) {
	int i;
	struct shmid_ds shmem_status;

	if (peers[agent_id].p_def == 0)
		return;

	// zona shared
	for (i = 0; i < peers[agent_id].p_def->n_agents; i++) {
		shmdt(peers[agent_id].p_shared_mem[i]);

		// se e o ultimo
		if (shmctl(peers[agent_id].shared_shmid[i], IPC_STAT, &shmem_status)
				== -1)
			PERRNO("shmctl");

		printf("-- shared %d --left attached: %d ---\n", i,
				(int) shmem_status.shm_nattch);

		if (shmem_status.shm_nattch == 0)
			shmctl(peers[agent_id].shared_shmid[i], IPC_RMID, NULL);
	}

	// zona local
	shmdt(peers[agent_id].p_local_mem);
	// se e o ultimo
	if (shmctl(peers[agent_id].local_shmid, IPC_STAT, &shmem_status) == -1)
		PERRNO("shmctl");

	printf("-- local --left attached: %d ---\n", (int) shmem_status.shm_nattch);

	if (shmem_status.shm_nattch == 0)
		shmctl(peers[agent_id].local_shmid, IPC_RMID, NULL);

	// zona def
	shmdt(peers[agent_id].p_def);

	// se e o ultimo
	if (shmctl(peers[agent_id].def_shmid, IPC_STAT, &shmem_status) == -1)
		PERRNO("shmctl");

	printf("-- def --left attached: %d ---\n", (int) shmem_status.shm_nattch);

	if (shmem_status.shm_nattch == 0)
		shmctl(peers[agent_id].def_shmid, IPC_RMID, NULL);

	peers[agent_id].p_def = 0;
}

int DB_comm_copy_agent(int agent_id)
{
        void* srcmem, *dstmem;
        int size, sret;
        if ( agent_id < 0 || agent_id >= MAX_AGENTS)
                return -1;

        srcmem = peers[agent_id].p_shared_mem[agent_id];
        size = peers[agent_id].p_def->shared_mem_size[agent_id];
        assert ( peers[agent_id].p_def->shared_mem_size[agent_id] == peers[my_id].p_def->shared_mem_size[agent_id] );
        dstmem = peers[my_id].p_shared_mem[agent_id];

        assert(comm_copy_buf);
        assert(srcmem);
        assert(size);
        assert(dstmem);

        // Source
        // do not mix these locks, since it would certainly deadlock.
        sret = rtdb_sem_lock( peers[agent_id].sem_id );

        memcpy(comm_copy_buf,srcmem,size);

    	if ( sret == 0 )
    		rtdb_sem_unlock( peers[agent_id].sem_id );

        // Destination
        sret = rtdb_sem_lock( peers[my_id].sem_id );

        memcpy(dstmem,comm_copy_buf,size);

    	if ( sret == 0 )
    		rtdb_sem_unlock( peers[my_id].sem_id );

        return 0;
}

//	*************************
//	DB_put_all: Escreve na base de dados
//		atencao: permite escrever em qualquer area!
//
//	Entrada:
//		int _agent = numero do agente
//		int _id = identificador da 'variavel'
//		void *_value = ponteiro com os dados
//		int life = tempo de vida da 'variavel' em ms
//	Saida:
//		int size = size of record data
//		-1 = erro
//
static int DB_put_all (int _agent, int _id, void *_value, int life)
{
	int sret;
	TRec *p_rec;
	void *p_data;
	int write_bank;
	struct timeval time;

    if ( _id < 0 || _id > MAX_RECS) {
        PERR("Invalid item id %d (for agent %d)", _id, _agent);
        return -1;
    }
    if ( _agent < 0 || _agent > MAX_AGENTS) {
        PERR("Invalid agent (id %d) for agent %d", _id, _agent);
        return -1;
    }

	if ((p_rec = get_record_in_lut(_agent, _id)) == NULL)
	{
		PERR("Unknown record %d for agent %d", _id, _agent);
		return -1;
	}

    /** XXX psoetens: See the race condition note below */
	sret = rtdb_sem_lock( peers[my_id].sem_id );
	write_bank = (p_rec->read_bank + 1) % 2;

	p_data = (void*)((char*)(p_rec) + p_rec->offset + write_bank * p_rec->size);
	memcpy(p_data, _value, p_rec->size);

	gettimeofday(&time, NULL);
	p_rec->timestamp[write_bank].tv_sec = time.tv_sec - life / 1000;
	p_rec->timestamp[write_bank].tv_usec = time.tv_usec - (life % 1000) * 1000;

	p_rec->read_bank = write_bank;
	if ( sret == 0 )
		rtdb_sem_unlock( peers[my_id].sem_id );

	PDEBUG("agent: %d, id: %d, lut: %d, size: %d, write_bank: %d, previous life: %umsec", _agent, p_rec->id, lut, p_rec->size, p_rec->read_bank, life);

	return p_rec->size;
}

static TRec* get_record_in_lut(int _agent, int _id)
{
	int lut;
	TRec* p_rec;

	if ((lut = peers[my_id].p_def->rec_lut[_agent][_id]) == -1)
	{
		PERR("Unknown record %d for agent %d", _id, _agent);
		return NULL;
	}

	if (lut < MAX_RECS)
		p_rec = (TRec*)((char*)(peers[my_id].p_shared_mem[_agent]) + lut * sizeof(TRec));
	else
		p_rec = (TRec*)((char*)(peers[my_id].p_local_mem) + (lut - MAX_RECS) * sizeof(TRec));

	return p_rec;
}


//	*************************
//	DB_comm_put: Escreve na base de dados
//		atencao: permite escrever em qualquer area!
//
//	Entrada:
//		int _agent = numero do agente
//		int _id = identificador da 'variavel'
//		void *_value = ponteiro com os dados
//		int life = tempo de vida da 'variavel' em ms
//	Saida:
//		int size = size of record data
//		-1 = erro
//
int DB_comm_put (int _agent, int _id, int _size, void *_value, int life)
{
	TRec *p_rec;

	if ((_agent == SELF) || (_agent == peers[my_id].p_def->self_agent))
	{
		PERR("Impossible to write in the running agent!");
		return -1;
	}
    
    if ( _id < 0 || _id > MAX_RECS) {
        PERR("Invalid item id %d (for agent %d)", _id, _agent);
        return -1;
    }
    if ( _agent < 0 || _agent > MAX_AGENTS) {
        PERR("Invalid agent (id %d) for agent %d", _id, _agent);
        return -1;
    }

	if (peers[my_id].p_def->rec_lut[_agent][_id] >= MAX_RECS)
	{
		PERR("Impossible to write local records!");
		return -1;
	}

	if ((p_rec = get_record_in_lut(_agent, _id)) == NULL)
	{
		PERR("Unknown record %d for agent %d", _id, _agent);
		return -1;
	}

	if(p_rec->size != _size)
	{
		PERR("Invalid size of record %d for agent %d (%d instead of %d)", _id, _agent, p_rec->size, _size);
		return -1;
	}

	return DB_put_all(_agent, _id, _value, life);
}



//	*************************
//	DB_put: Escreve na base de dados do proprio agente
//
//	Entrada:
//		int _id = identificador da 'variavel'
//		void *_value = ponteiro com os dados
//	Saida:
//		int size = size of record data
//		-1 = error
//
int DB_put (int _id, void *_value)
{
	return DB_put_all(peers[my_id].p_def->self_agent, _id, _value, 0);
}



//	*************************
//	DB_get: Le da base de dados
//
//	Entrada:
//		int _agent = numero do agente
//		int _id = identificador da 'variavel'
//		void *_value = ponteiro para onde sao copiados os dados
//	Saida:
//		int life = tempo de vida da 'variavel' em ms
//			-1 se erro
//
int DB_get (int _agent, int _id, void *_value)
{
	TRec *p_rec;
	void *p_data;
	struct timeval time;
	int life, sret;

    if ( _id < 0 || _id > MAX_RECS) {
        PERR("Invalid item id %d for agent %d", _id, _agent);
        return -1;
    }
    if ( _agent < 0 || _agent > MAX_AGENTS) {
        PERR("Invalid item id %d for agent %d", _id, _agent);
        return -1;
    }
    
	if (_agent == SELF)
		_agent = peers[my_id].p_def->self_agent;
    
	if ((p_rec = get_record_in_lut(_agent, _id)) == NULL)
	{
		PERR("Unknown record %d for agent %d", _id, _agent);
		return -1;
	}

	p_data = (void *)((char *)(p_rec) + p_rec->offset);

    /** XXX psoetens: This is clearly a race condition. The read_bank pointer may be
        changed in another thread during the next 4 lines. 
        Even worse, the memory pointed to may then change as well (memcpy is not atomic)
        and lead to corrupt values in _value. WE WILL DEFINITELY HIT THIS.
        Solution: Use a mutex.
    **/
    sret = rtdb_sem_lock( peers[my_id].sem_id );
	memcpy(_value, (char *)p_data + (p_rec->read_bank * p_rec->size), p_rec->size);

	gettimeofday(&time, NULL);
	life = (int)(((time.tv_sec - (p_rec->timestamp[p_rec->read_bank]).tv_sec) * 1E3) + ((time.tv_usec - (p_rec->timestamp[p_rec->read_bank]).tv_usec) / 1E3));

	if(life < 0){
		life = INT_MAX;
	}

	if ( sret == 0 )
		rtdb_sem_unlock( peers[my_id].sem_id );
	PDEBUG("agent: %d, id: %d, read_bank: %d, life: %umsec", _agent, p_rec->id, p_rec->read_bank, life);

	return (life);
}



//	*************************
//	Whoami: identifica o agente onde esta a correr
//
//	Saida:
//		int agent_number = numero do agente
//
int Whoami(void)
{
  if (!peers[my_id].p_def)
    return -1;
  return (peers[my_id].p_def->self_agent);
}

/**
 * Checks if a certain agent is alive.
 * @_agent The agent ID to check. If _agent is equal to Whoami(),
 * this function returns always 1.
 * @time_out The maximum age time in milliseconds (ms) of the last message of the agent.
 * If the last communication is older than this age, the agent
 * is considered no longer alive.
 * @retval 1 The agent is alive.
 * @retval 0 The agent is not alive or unknown.
 */
int IsAlive(int _agent, int time_out)
{
    int lut = 0, _id = 0;
    TRec *p_rec;
    struct timeval time;
    int life, alive,sret;

    if (_agent == SELF)
        return 1;

    // use loop to find a shared id:
    while( _id < MAX_RECS && (lut = peers[my_id].p_def->rec_lut[_agent][_id]) == -1 )
        {
            ++_id;
        }

    if (lut == -1)
        return 0; // no shared records found !

    // lookup first record found in shared memory segment. 
    p_rec = (TRec*)((char*)(peers[my_id].p_shared_mem[_agent]) + lut * sizeof(TRec));

    gettimeofday(&time, NULL);
    sret = rtdb_sem_lock( peers[my_id].sem_id );
    life = (int)(((time.tv_sec - (p_rec->timestamp[p_rec->read_bank]).tv_sec) * 1E3) + ((time.tv_usec - (p_rec->timestamp[p_rec->read_bank]).tv_usec) / 1E3));
	if ( sret == 0 )
		rtdb_sem_unlock( peers[my_id].sem_id );

    alive = ((0 < life) && (life < time_out)) ? 1 : 0;

    PDEBUG("IsAlive agent %d ? %d (timeout: %d, lifetime: %dms, id:%d ).", _agent, alive, time_out, life, _id);

    return  alive;
}


//	*************************
//	DB_comm_ini:
//
//	Entrada:
//		RTDBconf_agents *rtdb_conf_for_comm = pointer to configuration struct
//	Saida:
//		int n = numero the configured agents in rtdb.conf
//

int DB_comm_ini(RTDBconf_agents_comm *rtdb_conf_for_comm, char* network_name)
{
	int rec,j;
	int network_id = -1;
	RTDBconf_networks network_conf;
	RTDBconf_agents _rtdb_conf[MAX_AGENTS];

	/* read rtdb configuration */
	read_configuration(_rtdb_conf, &network_conf);

	/* loop over networks and lookup the network name */
	for(j=0;j<network_conf.n_networks;j++)
	{
		if(strcmp(network_conf.network[j].network_name, network_name) == 0)
		{
			network_id = j;
			break;
		}
	}

	/* if network is not found, share all records on this network */
	if(network_id < 0)
	{
		printf("Initializing comm for all shared records\n");
		/* no network specified, copy all shared records in the RTDB to the rtdb_conf_for_comm */
		memcpy(rtdb_conf_for_comm->shared,_rtdb_conf[my_id].shared,_rtdb_conf[my_id].n_shared_recs*sizeof(RTDBconf_var));
		rtdb_conf_for_comm->shared_mem_size = _rtdb_conf[my_id].shared_mem_size;
		rtdb_conf_for_comm->n_shared_recs = _rtdb_conf[my_id].n_shared_recs;

		return 0;
	}

	printf("Initializing comm for network : %s\n",network_conf.network[network_id].network_name);

	/* copy the shared records to the comm configuration of agent=my_id */
	rtdb_conf_for_comm->n_shared_recs = 0;
	rtdb_conf_for_comm->shared_mem_size = 0;

	for(rec=0;rec<_rtdb_conf[my_id].n_shared_recs;rec++)
	{
		/* check if the record is present in this network configuration */
		if( network_conf.network[network_id].shared_record_present[_rtdb_conf[my_id].shared[rec].id] )
		{
			/* copy shared record */
			rtdb_conf_for_comm->shared[rtdb_conf_for_comm->n_shared_recs] = _rtdb_conf[my_id].shared[rec];
			rtdb_conf_for_comm->shared_mem_size += _rtdb_conf[my_id].shared[rec].size;
			rtdb_conf_for_comm->n_shared_recs++;
		}
	}

	/* free network names */
	for(j=0;j<MAX_NETWORKS;j++)
	{
		if(network_conf.network[j].network_name != NULL)
		{
			free(network_conf.network[j].network_name);
		}
	}

	return 0;
}

int IsProcessRunning(const char* name)
{
  char call[60];
  snprintf(call,60,"pidof %s >/dev/null",name);
  return system( call ) == 0;
}

int RequireComm(const char* iface)
{
  struct sembuf sops[2];
  int ret;
  char call[80];
  int semid = rtdb_sem_open( 0x53888 ); // arbitrary number that should not class with 3rd party software.

  if (semid <0)
      return -1;

  // sem starts at zero ! We raise it.
  sops[0].sem_num = 0;        /* Operate on semaphore 0 */
  sops[0].sem_op = 1;         /* Raise by 1 */
  sops[0].sem_flg = SEM_UNDO;
  
  if (semop(semid, sops, 1) == -1) {
      perror("Error RequireComm::semop");
      return -1;
  }
  
  if (IsProcessRunning("comm")) {
      printf("comm was already started by another process...\n");
      return 0;
  }

  snprintf(call,80,"/usr/bin/comm %s -fork > /tmp/comm.log", iface);
  printf("Starting comm with '%s'\n",call);
  ret = system(call);

  return ret;
}

int CheckComm(const char* iface)
{
  char call[80];
  int ret;
  if (iface == 0)
	return -1;

  if (IsProcessRunning("comm")) {
      return 0;
  }

  snprintf(call,80,"/usr/bin/comm %s -fork > /tmp/comm.log &", iface);
  printf("Re-starting comm with '%s'\n",call);
  ret = system(call);

  return ret;
}

int RestartComm(const char* iface)
{
  int ret = system("killall -s 2 comm");

  if(ret == -1){
	  printf("system(killall -s 2 comm) failed\n");
  }

  return CheckComm(iface);
}
    

int ReleaseComm()
{
	struct sembuf sops[2];
	int ret;
	int semval;

	int semid = rtdb_sem_open( 0x53888 ); // arbitrary number that should not class with 3rd party software.

	if (semid <0)
		return -1;

	// sem starts at zero !
	sops[0].sem_num = 0;        /* Operate on semaphore 0 */
	sops[0].sem_op = -1;         /* Lower by 1 */
	sops[0].sem_flg = SEM_UNDO | IPC_NOWAIT;

	if (semop(semid, sops, 1) == -1) {
		perror("Error ReleaseComm::semop");
		return -1;
	}

	// read the value back
	if( (semval = semctl(semid,0,GETVAL)) == -1) {
		perror("Error ReleseComm::semctl GETVAL");
		return -1;
	}


	// if it's again null, kill comm.
	if (semval == 0)
	{
		ret = 0;
		ret = system("killall -s 2 comm");

		if(ret == -1){
		  printf("system(killall -s 2 comm) failed\n");
		}

		return 1;
	}
	return 0;
}
