#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "creationFifos.c"
#include "eliminerFifos.c"
#include <sys/wait.h>
#include <sys/shm.h>

#define MAX 30

void print_string(char tab[]);
void print_tab_string(char tab[][MAX], int taille);
int enregistrer(char *nom);
int interp_commande(char *commande);

int enregistrement = 0;
char user1[MAX];
int j = 0;


/////////////////////////////////// Creation SHM ////////////////////////////////
key_t key;
int shmid;
char *str;
//////////////////////////////////////////////////////////////////////////////////


int main()
{
	// ftok to generate unique key
    key = ftok("shmfile",65);
  
    // shmget returns an identifier in shmid
    shmid = shmget(key,1024,0666|IPC_CREAT);
  
    // shmat to attach to shared memory
    str = (char*) shmat(shmid, NULL, 0);


	char commande[MAX];

	do
	{
		printf("\nPlease enter a command : \n\n");
		printf(" - 'e <username>' : register with username <username>, logs you in\n");
		printf(" - 'p <username>' : open a dialog with the user named <username>.\n");
		printf(" - 'q' : quit the instant messaging process\n");
		printf(" - 'l' : list all connected users\n");
		printf(" - 'd' : disconnect, impossible to talk to in this state\n\n");

		fgets(commande,MAX-1, stdin);

	} while(interp_commande(commande) != 0);

	return EXIT_SUCCESS;
}


char *decoupe_mots(char *commande)
{
	char c;
	c = commande[j+2];
	// on efface le début ('e ' ou 'd ') de la commande et on renvoie le <nom>
	while(c != '\0' && c != '\n')
	{	
		commande[j] = c;
		commande[j+2] = 0;
		j++;
		c = commande[j+2];
	}
	j = 0;
	return commande;
}

int est_connecte(char* nom)
{
	char d[] = " ";//Séparateur
	//// utiliser strtok sans modifier la vraie liste ///
	char *str2 = malloc( 360 * sizeof(char)); 
	strcpy(str2,str);
	char *p = strtok(str2, d); // p = premier utilisateur de la liste
	////

// On parcourt toute la listes des utilisateurs connectés pour savoir si nom y est.
	while(p !=NULL)
	{	
		if(strcmp(nom, p) == 0)
		{
			return 1;
		}
		p = strtok(NULL, d);// p = nom du prochain utilisateur connecté.
	}
	return 0;
}

int enregistrer(char *nom)
{
	if(enregistrement == 1)
	{
		printf("A user is already registered on this terminal!\n");
	}
	else if(est_connecte(nom))
	{
		printf("Username already used.\n");
	}
	else if(strlen(nom)>12)
	{
		printf("Maximum number of characters = 12 \n");
	}
	else
	{
		strcpy(user1, nom);
		strcat(str, user1);
		strcat(str, " ");

		enregistrement = 1;
		printf("\nRegistered user!\n");
		return 0;
	}
	return -1;
}

int parler(char *user2)
{
	if(enregistrement == 1)
	{
		if(est_connecte(user2) && strcmp(user1, user2) != 0)
		{
			//génération desdes tubes nommées (user1_user2 et user2_user1)
			char fifo1[MAX], fifo2[MAX];
			strcpy(fifo1, user1);
			strcat(fifo1, "_");
			strcat(fifo1, user2);

			strcpy(fifo2, user2);
			strcat(fifo2, "_");
			strcat(fifo2, user1);

			creationFifos(fifo1,fifo2);

			//ouverture de deux terminals xterm en parallèle pour dialoguer.
			if(fork() == 0)
			{
				execl("/usr/bin/xterm","xterm","-e","./communication",fifo1,fifo2,user1,user2, NULL);
				
			}
			if(fork() == 0)
			{
				execl("/usr/bin/xterm","xterm","-e","./communication",fifo2,fifo1,user2,user1, NULL);
			}
			else{
			// Le processus père attend que ces deux fils meurent (fin de dialogue)
				wait(NULL);
				wait(NULL);
			}

			printf("Conversation ended\n");
			return 0;
		}

		return -1; // Erreur car le destinataire n'était pas dans la liste des connectés.
	}
	else
	{
		return -1; // Erreur car expéditeur pas connecté.
	}	
}

int interp_commande(char *commande)
{

	char choix;
	choix = commande[0];	
	
	switch((int)choix)
	{
		case(101): //e
		{	
			if (enregistrer(decoupe_mots(commande)) == -1)
			{
				printf("Registration failed\n");
				return -1; // Problème d'enregistrement
			}
			break;
		}

		case(112): //p
		{
			if(parler(decoupe_mots(commande))== -1)
			{
				printf("Check that you and your contact are connected using the l command.\n");
				printf("You can't open a dialogue with yourself.\n");
				return -1;
			}
			break;
		}

		case(100): //d
		{
			if(enregistrement == 1)
			{
				char d[] = " ";//Séparateur

				char *str2 = malloc( 360 * sizeof(char)); //nouvelle liste des connectés.

				char *p = strtok(str, d); 
				while(p != NULL)
				{
					// On recopie tous les pseudos sauf user1 car il se déconnecte.
					if( strcmp(user1, p) != 0)
					{
				  		strcat(str2, p);
				  		strcat(str2, " ");
					}
				p = strtok(NULL, d);
				}
				strcpy(str, str2); // on met a jour la mémoire partagé avec la nouvelle liste.
				free(str2);
				enregistrement = 0; //déconnexion

				printf("Logout successful.");
			}
			else
			{
				printf("No user logged on to the terminal at the moment... \n");
				return -1;
			}
			break;
		}

		case(108): //l
		{
			printf("List of connected users :\n%s\n",str);
			break;
		}

		case(114)://r (commande pour effacer la mémoire partagé)( cas d'urgence)
		{
			if( strcmp("admin",decoupe_mots(commande)) == 0)
			{
				shmctl(shmid,IPC_RMID,NULL);
				printf("Shared memory erased.\n");
				interp_commande("q"); // quitter
				return 0;
			}
			else
				interp_commande("!"); // commande invalide

			break;
		}
		
		case(113): //q
		{
			// Si connecté, on déconnecte avant de quitter.
			if(enregistrement == 1)
				interp_commande("d");

			shmdt(str);
			printf("\nThe process has ended!\n");
			return 0;
			break;
		}

		default:
		{
			printf("\nPlease enter a valid command\n");
			break;
		}
	}	
	return 1;
}






