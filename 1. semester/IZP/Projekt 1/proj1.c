/*
* 	IZP 					
*	Projekt 1		    		
* 	Martin Fekete				
*	xfeket00			 		
*											
*	Nepovinne prikazy:			
*	1. detekcia zacyklenia			
* 	2. prikaz 'fPATTERN'			
*	3. prikaz 'e'					
*/

#include <stdio.h>
#include <string.h>

#define LINE_LEN 10000 // MAX dlzka jedneho riadku v stdin
#define COMM_LEN 1000  // MAX dlzka prikazu vo vstupnom subore
#define PARE_LEN 1000  // MAX dlzka patternu a replacementu v prikazoch 's' a 'S'
#define ASCII_0 48	   // ASCII hodnota nuly
#define ASCII_9 57	   // ASCII hodnota deviatky

// Skonvertuje cislo za prikazom do int a vrati ho.
int commandNumber(char command[COMM_LEN])
{
	int no = 0;

	if (strlen(command) > 2)
	{
		// Cislo za akymkolvek prikazom prekonvertuje na integer no.
		for (unsigned long int i = 1; i < strlen(command)-1; i++)
		{													
			// V pripade, ze je za prikazom, ktory ma byt nasledovany cislom iny znak, funkcia vrati -1
			if ((command[i] < ASCII_0) || (command[i] > ASCII_9))
				return -1;
			no = no * 10 + ( command[i] - '0' ); // konverzia cisla za prikazom na int
		}
		return no;
	}
	else // ak za prikazom nie je nijake cislo, vrati jednotku (napr 'n' == 'n1')
		return 1;
}

int n_command(char command[], char line[])
{
	// Ak funckia vrati commandNumber vrati -1, pouzivatelovi bude vypisane, ako ma zadavat prikazy
	if (commandNumber(command) == -1)
	{
		fprintf(stderr, "Command 'n' should be followed by numbers (n3) or by nothing (n).\n");
		return 0;
	}

	for (int i = 0; i < commandNumber(command); i++)
	{
		printf("%s", line);
		memset(line, 0, LINE_LEN);
		if (fgets(line, LINE_LEN, stdin) == NULL)
			return 0;
	}
	return 1;
}

int g_command(FILE* commandFile, char command[], int nReturn, int count)
{
	
	// Ak funckia vrati commandNumber vrati -1, pouzivatelovi bude vypisane, ako ma zadavat prikazy
	if (commandNumber(command) == -1)
	{
		fprintf(stderr, "Command 'g' should be followed by numbers (g3) or by nothing (g).\n");
		return -1;
	}

	// Ak pred prikazom 'g' nie je prikaz 'n', alebo ak prikaz 'g' ukazuje sam na seba, program sa ukonci.
	else if (nReturn != 1 || count == commandNumber(command))
	{
		fprintf(stderr, "ERROR: infinte loop.\n");
		return -1;
	}
	
	else if (commandNumber(command) == 1)
		rewind(commandFile); // nacitanie suboru s prikazmi od zaciatku

	else
	{
		int number = commandNumber(command);
		rewind(commandFile); // nacitanie suboru s prikazmi od zaciatku
		for (int i = 0; i < number-1; i++) // -1 preto, lebo prikazy su indexovane od 1, nie od nuly
		{	
			memset(command, 0, COMM_LEN);
			fgets(command, COMM_LEN, commandFile); // citanie prikazov, kym sa cislo za 'g' nerovna cislu commandu
		}
	}
	return 0;
}

int d_command(char command[], char line[])
{
	// Ak funckia vrati commandNumber vrati -1, pouzivatelovi bude vypisane, ako ma zadavat prikazy
	if (commandNumber(command) == -1)
	{
		fprintf(stderr, "Command 'd' should be followed by numbers (d3) or by nothing (d).\n");
		return 0;
	}

	for (int i = 0; i < commandNumber(command); i++)
	{
		if (fgets(line, LINE_LEN, stdin) == NULL) // citanie prikazov naprazdno == "vymazavanie"
			return 0;
	}
	return 1;
}

void r_command(char line[])
{
	if (line[strlen(line)-1] == '\n') // ak je predposledny znak '\n', nahradi ho znakom '\0'
		line[strlen(line)-1] = '\0';
}


void s1(char command[], char pattern[], char replacement[])
{
	char divider = command[1]; // zistenie oddelovaca patternu a replacementu
				
	int divPos = 0; // pozicia oddelovaca patternu a replacementu
	for (int i = 2; command[i] != divider; i++)
	{
		pattern[i-2] = command[i];	// ulozenie patternu do premennej pattern
		divPos = i + 2;	// ulozenie pozicie oddelovaca patternu a replacementu
	}

	// Ulozenie replacementu do premennej replacement.
	for (int i = divPos; command[i] != '\n'; i++)
		replacement[i-divPos] = command[i];	
}

void s2(char line[], char pattern[], char replacement[])
{
	char* pos = (strstr(line,pattern)); // zistenie polohy patternu
	char line1[LINE_LEN]; // premenna na ulozenie riadku pred patternom
	char line2[LINE_LEN]; // premenna na ulozenie riadku za patternom
					
	// Ulozenie riadku pred patternom do premennej line1
	for (int i = 0; i != (pos - line); i++)
		line1[i] = line[i];
					
	// Ulozenie riadku za patternom do premennej line2
	int j = 0;
	for (int i = strlen(pattern) + strlen(line1); line[i] != '\0'; i++)
	{
		line2[j] = line[i];
		j++;
	}

	memset(line, 0, LINE_LEN);	// nahradenia obsahu riadku nulami
	strcat(line, line1);		// spojenie stringov line1 a line		
	strcat(line, replacement);	// spojenie stringov replacement a line
	strcat(line, line2);		// spojenie stringov line2 a line
	memset(line1, 0, LINE_LEN); // nahradenia obsahu premennej line1 nulami
	memset(line2, 0, LINE_LEN); // nahradenia obsahu premennej line2 nulami
}

int main(int argc, char *argv[])
{	
	// Napoveda pre pouzivatela, ze jediny argument ma byt subor s prikazmi.
	if (argc != 2)
	{
		fprintf(stderr, "First and only argument should be FILE_WITH_COMMANDS.\n");
		return -1;
	}

	FILE* commandF = fopen(argv[1], "r"); // otvorenie suboru s prikazmi pre citanie

	char line[LINE_LEN];	// pomocna premenna
	char command[COMM_LEN];	// premenna s prikazmi
	int nReturn = 0;		// navratova hodnota prikazu n - potrebne vo funkcii gX proti zacykleniu
	int count = 0;			// pocitanie poctu iteracii pocas nacitavania prikazov - pouzivam v prikaze g proti zacykleniu

	
	if (fgets(line, LINE_LEN, stdin) == NULL)
	{
		fprintf(stderr, "ERROR: stdin is empty.\n");
		return -1;
	}

	// Overenie spravnosti otvorenia suboru s prikazmi.
	if (commandF)
	{
		// Citanie prikazov od zaciatku po koniec suboru.
		while (fgets(command, COMM_LEN, commandF))
		{
			count++;
			
			// 'q'
			if (command[0] == 'q')
			{
				fclose(commandF);
				return 0;
			}
	
			// 'n' a 'nN'
			else if (command[0] == 'n')
			{
				if (n_command(command, line) == 0) // ked funkcia n_command vrati 0, program sa ukonci
					return 0;
				else
					nReturn = 1;
			}

			// 'd' a 'dN'
			else if (command[0] == 'd')
			{
				nReturn = 0;
				if (d_command(command, line) == 0) // ked funkcia d_command vrati 0, program sa ukonci
					return 0;
			}		

			// 'gX'
			else if (command[0] == 'g')
			{	
				if (g_command(commandF, command, nReturn, count) == -1)
					return 0;	// ak funkcia vrati -1, program sa ukonci s chybovym hlasenim
			}

			// 'e'
			else if (command[0] == 'e')
			{
				nReturn = 0;
				sprintf(line, "%s\n", line);
			}

			// 'r'
			else if (command[0] == 'r')
			{
				nReturn = 0;
				r_command(line);
			}

			// 'aCONTENT'
			else if (command[0] == 'a')
			{
				nReturn = 0;
				r_command(line); // vymazanie znaku konca riadku

				// Za aktualny riadok pridava char po chare vsetko co je v prikaze za indexom 1.
				for (long unsigned int i = 1; i < strlen(command); i++)
					strcat(&line[strlen(line)+i-1],&command[i]);
			}

			// 'bCONTENT'
			else if (command[0] == 'b')
			{
				nReturn = 0;
				// skopirovanie riadku
				char lineCopy[LINE_LEN];
				for (long unsigned int i = 0; i < strlen(line)-1; i++)
					lineCopy[i] = line[i];

				memset(line, 0, LINE_LEN); // nahradenie obsahu riadku nulou

				/* Zapisanie obsahu prikazu na zaciatok riadku
				** od 1 sa ide preto, lebo nechcem prvy znak prikazu a takisto nechcem \n na konci. */
				for (long unsigned int i = 1; i < (strlen(command)-1); i++)
					line[i-1] = command[i];
				
				sprintf(line, "%s%s\n", line, lineCopy); // spojenie riadku so zapisanym obsahom prikazu s povodnym riadkom
				memset(lineCopy, 0, LINE_LEN); // vynulovanie obsahu skopirovaneho riadku
			}

			// 'iCONTENT'
			else if (command[0] == 'i')
			{
				nReturn = 0;
				char commContent[COMM_LEN];
				// Skopiruje obsah prikazu aj so znakom konca riadku.
				for (long unsigned int i = 1; i < strlen(command); i++)
					commContent[i-1] = command[i];
				
				printf("%s", commContent);
				memset(commContent, 0, COMM_LEN);
			}

			// 's PATTERN REPLACEMENT' a 's:PATTERN:REPLACEMENT'
			else if (command[0] == 's')
			{
				nReturn = 0;
				nReturn = 0;
				char pattern[LINE_LEN];
				char replacement[LINE_LEN];

				s1(command, pattern, replacement);

				if (strstr(line,pattern))
					s2(line, pattern, replacement);
			}

			// 'S:PATTERN:REPLACEMENT'
			else if (command[0] == 'S')
			{
				nReturn = 0;
				char pattern[LINE_LEN];
				char replacement[LINE_LEN];

				s1(command, pattern, replacement);				

				while (strstr(line,pattern))
					s2(line, pattern, replacement);			
			}

			// 'fPATTERN'
			else if (command[0] == 'f')
			{
				nReturn = 0;
				char pattern[PARE_LEN];
				
				// Ulozenie patternu do premennej pattern.
				for (long unsigned int i = 1; i < strlen(command)-1; i++)
					pattern[i-1] = command[i];
	
				// Kym funkcia strstr vracia NULL, nacitava stdin.
				while (strstr(line,pattern) == NULL)
				{
					if (fgets(line, LINE_LEN, stdin) == NULL)
					{
						fprintf(stderr, "ERROR: pattern not found in stdin.");
						return -1;
					}
				}

			}

			else
				fprintf(stderr, "ERROR: unknown command.\n");
			
		}
		// Zatvorenie suboru s prikazmi.
		fclose(commandF);
		
		// Vypisanie zvysku suboru v pripade, ze boli spracovane vsetky prikazy.
		printf("%s", line);
		while (fgets(line, LINE_LEN, stdin) != NULL)
			printf("%s", line);
	}

	// Vypisanie error hlasenia v pripade neuspesneho otvorenia suboru.
	else
	{
		fprintf(stderr, "ERROR: file with commands was not opened.\n");
		return -1;
	}
	return 0;
}
