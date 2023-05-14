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
		printf("\nVeuillez entrer une commande : \n\n");
		printf(" - 'e <nom>' : s'enregister avec le nom <nom>, permet de se connecter\n");
		printf(" - 'p <nom>' : ouvrir un dialogue avec l'utilisateur nommé <nom>\n");
		printf(" - 'q' : quitter le processus de messagerie instantanée\n");
		printf(" - 'l' : lister tous les utilisateurs connectés\n");
		printf(" - 'd' : se déconnecter, impossible de dialoguer dans cet état\n\n");

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
		printf("Un utilisateur est déja enregistré sur ce terminal!\n");
	}
	else if(est_connecte(nom))
	{
		printf("Pseudo déjà utilisé.\n");
	}
	else if(strlen(nom)>12)
	{
		printf("Nombre de caractères max  = 12 \n");
	}
	else
	{
		strcpy(user1, nom);
		strcat(str, user1);
		strcat(str, " ");

		enregistrement = 1;
		printf("\nUtilisateur enregistré !\n");
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

			printf("Conversation terminée\n");
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
				printf("Echec de l'enregistrement\n");
				return -1; // Problème d'enregistrement
			}
			break;
		}

		case(112): //p
		{
			if(parler(decoupe_mots(commande))== -1)
			{
				printf("Vérifiez que vous et votre contact êtes bien connectés avec la commande l.\n");
				printf("NB: vous ne pouvez pas ouvrir un dialogue avec vous-mêmes.\n");
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

				printf("Déconnexion réussi.");
			}
			else
			{
				printf("Aucun utilisateur connecté au terminal en ce moment... \n");
				return -1;
			}
			break;
		}

		case(108): //l
		{
			printf("Liste des utilisateurs connectés :\n%s\n",str);
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
			printf("\nProcessus quitté !\n");
			return 0;
			break;
		}

		default:
		{
			printf("\nVeuillez entrer une commande valide\n");
			break;
		}
	}	
	return 1;
}






