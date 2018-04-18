/****************************************************************
*@Program: xrtdb -> Cambada soccer robots configuration files	*
*		automatic generator				*
*@Authors: Tiago Nunes Mec 37161				*
*	   Milton Gregório Mec 36275				*
*	   David Ferreira Mec 36129				*
*@Year: 2007							*
*@Univ: Universidade de Aveiro					*
*@File: rtdb_ini_creator.c 					*
*@Description: rtdb.ini DB initialization file generation	*
*		module						*
*****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "rtdb_ini_creator.h"

/* 
Function to write, compile and execute a rtdb_sizeof_tmp.c file
   that has included the headerfile where the datatype is defined
   and write a rtdb_size.tmp with the size of the datatype.
After that, the rtdb_size.tmp file is opened, the size of the
   datatype is read and returned 
*/
int getSizeof(char* headerfl, char* datatype)
{
	int sizeofdata;
	char line[50], *command;
	FILE *f, *ftmp /* , *hfl */;

	//If the headerfile to be included doesn't exist, abort
#if 0
	if ((hfl= fopen(headerfl, "r")) == NULL)
        {
                printf("\nO ficheiro de cabeçalho \e[33m%s\e[0m onde o tipo \e[33m%s\e[0m está definido não existe! A abortar!\n", headerfl, datatype);
		exit(1);
        }
	fclose(hfl);
#endif
	
	//Delete the rtdb_sizeof_tmp.c and the rtdb_size.tmp file just in case they exist
	command= malloc((1+strlen(RM_COMMAND)+strlen(" ./rtdb_sizeof_tmp.c ./rtdb_size.tmp"))*sizeof(char));
	sprintf(command, "%s ./rtdb_sizeof_tmp.c ./rtdb_size.tmp", RM_COMMAND);
	system(command);
	free(command);
	
	//Create the rtdb_sizeof_tmp.c file for writing
	f=fopen("rtdb_sizeof_tmp.c", "w");
	
	//Write the contents of the file
	fprintf(f, "/* AUTOGEN File: rtdb_sizeof_tmp.c */\n\n");
	fprintf(f, "#include <stdio.h>\n");
	fprintf(f, "#include <stdint.h>\n");
	//Include the necessary headerfile in rtdb_sizeof_tmp.c
	fprintf(f, "#include \"%s\"\n\n", headerfl);
	fprintf(f, "int main(void)\n{\n");
	fprintf(f, "\tFILE* f;\n");
	//Create the rtdb_size.tmp for writing
	fprintf(f, "\tf= fopen(\"rtdb_size.tmp\", \"w\");\n");
	//Write the size of the datatype in the rtdb_size.tmp file
#if defined( __GNUC__ ) && defined( __x86_64__ )
	fprintf(f, "\tfprintf(f, \"%%lu\\n\", sizeof(%s%s));\n", STRUCTPREFIX, datatype);
#else
	fprintf(f, "\tfprintf(f, \"%%u\\n\", sizeof(%s%s));\n", STRUCTPREFIX, datatype);
#endif
	fprintf(f, "\tfclose(f);\n");
	fprintf(f, "\n\treturn 0;\n}\n");
	fprintf(f, "\n/* EOF: rtdb_sizeof_tmp.c */\n");

	//Close the rtdb_sizeof_tmp.c file
	fclose(f);

	//If in Debug Sizeof mode, pause and ask the user what to do
	if (DEBUGSIZEOF)
	{
		char op = '\0';
		while(op != 'c')
		{
			printf("\n\e[33mFicheiro\e[0m \e[32mrtdb_sizeof_tmp.c\e[0m \e[33mescrito.\e[0m\nInsira\n\"\e[32mv\e[0m\" para ver o ficheiro gerado,\n\"\e[32mc\e[0m\" para continuar e compilar ou\n\"\e[32ma\e[0m\" para abortar: ");
			scanf("%c", &op);
			//Clean stdin
			purge();
			switch (op)
			{
				//If the user wants to see the rtdb_sizeof_tmp.c file, show it
				case 'v':	{
						printf("\n\e[32mA visualizar o ficheiro \e[33mrtdb_sizeof_tmp.c\e[0m\n");
						command= malloc((1+strlen(TXTVIEWER)+strlen(" rtdb_sizeof_tmp.c")) * sizeof(char));
						sprintf(command, "%s rtdb_sizeof_tmp.c", TXTVIEWER);
						system(command);
						free(command);
						printf("\e[32mFicheiro \e[33mrtdb_sizeof_tmp.c \e[32mvisto!\e[0m\n");
						break;
						}
				//If the user wants to continue, break
				case 'c':	break;
				//If the user wants to abort, exit the program
				case 'a':	{
						command= malloc((1+strlen(RM_COMMAND)+strlen(" ./rtdb_sizeof_tmp.c")) * sizeof(char));
						sprintf(command, "%s ./rtdb_sizeof_tmp.c", RM_COMMAND);
						system(command);
						free(command);
						printf("\n\e[33mAbortado pelo utilizador!\e[0m\n");
						exit(1);
						}
				//If not a correct option, notify the user
				default:	printf("\n\e[33mOpção inválida!\e[0m\n");
			}
		}
	}

	//Compile the generated rtdb_sizeof_tmp.c file
	command= malloc((strlen(CC)+1+strlen(CFLAGS)+strlen(" -o rtdb_sizeof_tmp rtdb_sizeof_tmp.c")+1)*sizeof(char));
	sprintf(command, "%s %s -o rtdb_sizeof_tmp rtdb_sizeof_tmp.c", CC, CFLAGS);
	//printf("CMD: %s\n", command); fflush(stdout);
	system(command);
	free(command);

	//Execute the rtdb_sizeof_tmp file just compiled
	system("./rtdb_sizeof_tmp");

	//Remove the rtdb_sizeof_tmp executable
	command= malloc((1+strlen(RM_COMMAND)+strlen(" ./rtdb_sizeof_tmp"))*sizeof(char));
	sprintf(command, "%s ./rtdb_sizeof_tmp", RM_COMMAND);
	system(command);
	free(command);

	//If in Debug Sizeof mode, pause and ask the user what to do
	if (DEBUGSIZEOF)
	{
		char op = '\0';
		while(op != 'c')
		{
			printf("\n\e[33mFicheiro\e[0m \e[32mrtdb_size.tmp\e[0m \e[33mescrito.\e[0m\nInsira\n\"\e[32mv\e[0m\" para ver o ficheiro gerado,\n\"\e[32mc\e[0m\" para continuar e compilar ou\n\"\e[32ma\e[0m\" para abortar: ");
			scanf("%c", &op);
			//Clean stdin
			purge();
			switch (op)
			{
				//If the user wants to see the rtdb_size.tmp file, show it
				case 'v':	{
						printf("\n\e[32mA visualizar o ficheiro \e[33mrtdb_size.tmp\e[0m\n");
						command= malloc((1+strlen(TXTVIEWER)+strlen(" rtdb_size.tmp")) * sizeof(char));
						sprintf(command, "%s rtdb_size.tmp", TXTVIEWER);
						system(command);
						free(command);	
						printf("\e[32mFicheiro \e[33mrtdb_size.tmp \e[32mvisto!\e[0m\n");		
						break;
						}
				//If the user wants to continue, break
				case 'c':	break;
				//If the user wants to abort, exit the program
				case 'a':	{
						command= malloc((1+strlen(RM_COMMAND)+strlen(" ./rtdb_size.tmp")) * sizeof(char));
						sprintf(command, "%s ./rtdb_size.tmp", RM_COMMAND);
						system(command);
						free(command);
						printf("\n\e[33mAbortado pelo utilizador!\e[0m\n");
						exit(1);
						}
				//If not a correct option, notify the user
				default:	printf("\n\e[33mOpção inválida!\e[0m\n");
			}
		}
	}
	
	//Open the rtdb_size.tmp file for reading
	ftmp= fopen("rtdb_size.tmp", "r");

	//Read the only line that should be in the rtdb_size.tmp file to the var line
	fgets(line, 50, ftmp);

	//Define sizeofdata with the integer just read to line
	sscanf(line, "%d", &sizeofdata);

	//Close the rtdb_size.tmp file
	fclose(ftmp);

	//Delete the rtdb_sizeof_tmp.c temporary file
	command= malloc((1+strlen(RM_COMMAND)+strlen(" ./rtdb_sizeof_tmp.c"))*sizeof(char));
	sprintf(command, "%s ./rtdb_sizeof_tmp.c", RM_COMMAND);
	system(command);
	free(command);
	
	//Delete the rtdb_size.tmp temporary file
	command= malloc((1+strlen(RM_COMMAND)+strlen(" ./rtdb_size.tmp"))*sizeof(char));
	sprintf(command, "%s ./rtdb_size.tmp", RM_COMMAND);
	system(command);
	free(command);

	//If in Debug Sizeof mode ask the user to continue or abort the program
	if (DEBUGSIZEOF)
	{
		char op = '\0';
		printf("\n\n\e[33mTodos os ficheiros temporários lidos e apagados.\nPrima \"\e[32ma\e[33m\" para abortar ou\nqualquer outra tecla para sair da função e prosseguir com o programa:\e[0m ");
		scanf("%c", &op);
		//Clean stdin
		purge();
		if (op == 'a')
		{
			printf("\n\e[33mAbortado pelo utilizador!\e[0m\n");
			exit(1);
		}
	}
	
	//Return the size of the datatype received
	return sizeofdata;
}


/*
Function to read the Global list of agents and the Global list
   of assignments and automatically generate the rtdb.ini file
*/
int printIniFile (rtdb_AgentList* agL, rtdb_AssignmentList* asL, rtdb_NetworkList* nwL)
{
	char* command;
        FILE *f;

#if 0
	//Open the file rtdb.ini for reading to check if it exists
        f= fopen(RTDB_INI, "r");

	if(f != NULL)
        {
		//If it exists close it and ask the user permission to overwrite it
                fclose(f);
		char op = '\0';
		while ((op != 'y') && (op != 'n'))
                {
                	printf("\nO ficheiro \e[33mrtdb.ini\e[0m ja existe!\nDeseja substitui-lo? (y/n): ");
                        scanf("%c", &op);
			//Clean stdin
			purge();
                }
                if (op == 'n')
		{
			return 1;
		}
        }
#endif

	//Remove the rtdb.ini file in case it exists, if we have permission from the user
	command= malloc((2+strlen(RM_COMMAND)+strlen(RTDB_INI)) * sizeof(char));
	sprintf(command, "%s %s", RM_COMMAND, RTDB_INI);
	system(command);
	free(command);

	//If for some reason we can't create the file, abort
	if ((f= fopen(RTDB_INI, "w")) == NULL)
	{
		return 2;
	}

	unsigned i, j, k, l;

	// start with agents
	fprintf(f, "<AGENTS>\n");
	//Run through all the agents
	for (i= 0; i < agL->numAg; i++)
	{
		//Print the line with #, the agent number and the agent name
 		fprintf(f, "# %-4u %s\n", agL->agents[i].num, agL->agents[i].id);
		//Run through all the assignments
		for (j= 0; j < asL->numAs; j++)
		{
			//Run through all the agents of the current assignment
			for (k= 0; k < asL->asList[j].agentList.numAg; k++)
			{
				//If the current agent is in the current assignment list
				if (strcmp(asL->asList[j].agentList.agents[k].id, agL->agents[i].id) == 0)
				{
					//Print all items from the local items list of the current assignment schema
					for (l= 0; l < asL->asList[j].schema->sharedItems.numIt; l++)
					{
						fprintf(f, "%-4u %-8d %-2u  s\n", asL->asList[j].schema->sharedItems.items[l].num, getSizeof(asL->asList[j].schema->sharedItems.items[l].headerfile, asL->asList[j].schema->sharedItems.items[l].datatype), asL->asList[j].schema->sharedItems.items[l].period);
					}
					//Print all items from the shared items list of the current assignment schema
					for (l= 0; l < asL->asList[j].schema->localItems.numIt; l++)
					{
						fprintf(f, "%-4u %-8d %-2u  l\n", asL->asList[j].schema->localItems.items[l].num, getSizeof(asL->asList[j].schema->localItems.items[l].headerfile, asL->asList[j].schema->localItems.items[l].datatype), asL->asList[j].schema->localItems.items[l].period);
					}	
				}
			} 
		}
		fprintf(f, "\n");
	}
	// Print all networks
	fprintf(f, "<NETWORKS>\n");
	//Run through all the networks
	for (i= 0; i < nwL->numNw; i++)
	{
		//Print the line with #, the network number and the network name
 		fprintf(f, "# %-4u %s\n", nwL->networks[i].num, nwL->networks[i].id);
 		// Run through all records for this network
 		for(j=0;j<nwL->networks[i].sharedItems.numIt; j++)
 		{
 			fprintf(f, "%-4u\n",nwL->networks[i].sharedItems.items[j].num);
 		}
	}
	//Close the rtdb.ini file
	fclose(f);

	//Return 0, meaning all went smoothly
	return 0;
}

/* EOF: rtdb_ini_creator.c */
