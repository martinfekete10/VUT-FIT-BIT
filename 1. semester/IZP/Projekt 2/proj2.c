/*
* 	IZP
*	Projekt 2
*	Martin Fekete
*	xfeket00
*	25.11.2018
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define DOUBLE_UPRANGE 1.7e+308   // maximalna hodnota dat. typu double
#define DOUBLE_DOWNRANGE 2.3e-308 // minimalna hodnota dat. typu double
#define POS_INF (1.0/0.0) // konstanta pre nekonecno
#define NEG_INF (-1.0/0.0) // konstanta pre -nekonecno
#define EPS 0.000000001 // epsilon pre premiove ulohy


double taylor_log(double x, unsigned int n)
{
	if (x == 0)
		return -1.0/0.0;
	else if (x == POS_INF)
		return 1.0/0.0;
	else if (isnan(x) || x < 0)
		return NAN;

	double term = x;     // aktualne pocitany clen polynomu
	double term_p = 1.0; // citatel predchazajuceho clenu polynomu (s prvotnou hodnotou 1.0)
	double sum = 0.0;    // sucet clenov polynomu
	unsigned int i = 1;  // counter

	// Ak x >= 1, logaritmus sa bude pocitat pomocou vzorca pre x > 1/2 zo zadania.
	if (x >= 1)
	{
		// Kym i-1 je mensie ako pocet iteracii zadanych pouzivatelom, robi sa nasledujuci loop.
		while (i-1 < n)
		{
			term = (((x-1)/(x))*term_p)/i; // vypocet aktualneho clena polynomu
			term_p = ((x-1)/(x))*term_p; // zmena predosleho citatela polynomu na aktualny
			sum = sum + term; // sucet predoslych clenov polynomu s aktualne vypocitanym
			i++; // inkrementacia counteru i
		}
	}

	// Ak x < 1, logaritmus sa bude pocitat pomocou vzorca pre 0 < x < 2 zo zadania.
	else
	{
		x = 1 - x; // vypocet cisla x, ktore dosadzame do polynomu
		// Kym i-1 je mensie ako pocet iteracii zadanych pouzivatelom, robi sa nasledujuci loop.
		while (i-1 < n)
		{
			term = (x*term_p)/i; // vypocet aktualneho clena polynomu
			term_p = x*term_p; // zmena predosleho citatela polynomu na aktualny 
			sum = sum + term; // sucet predoslych clenov polynomu s aktualne vypocitanym
			i++; // inkrementacia counteru i
		}
		sum *= -1; // zmena celeho suctu na opacnu hodnotu (podla vzorca)
	}

	return sum; // funkcia vracia sucet polynomov
}

double cfrac_log(double x, unsigned int n)
{
	if (x == 0)
		return -1.0/0.0;
	else if (x == POS_INF)
		return 1.0/0.0;
	else if (isnan(x) || x < 0)
		return NAN;

	double cf = 0.0; // prvotny zlomok s hodnotou 1.0
	int k = 2*n-1;   // vypocet posledneho cisla, od ktoreho sa odcitava posledny zlomok (podla vzorcu v zadani)
	x = (x - 1) / (x + 1); // vypocet x pre algoritmus zretazenych zlomkov
	double xx = x*x; // x*x vypocitane pred cyklom, aby sa tento vypocet nemusel opakovat n-krat v cykle

	// Kym sa loop nezopakuje (n-1)-krat, bude sa pocitat zretazeny zlomok.
	for (int i = n-1; i >= 0; i--)
	{
		if (i == 0)
		{
			cf = (2*x)/(1-cf);
			break;
		}

		/* Citatel - akutalne "zanorenie" v zlomku ^2 vynasobene x^2
		 * Menovatel - predosly zretazeny zlomok odpocitany od konstanty k */
		cf = (i*i*xx)/(k-cf);
		k -= 2; // dekrementacia konstanty k o 2
	}
	return cf;
}

double taylor_pow(double x, double y, unsigned int n)
{
	if (isnan(x) || isnan(y))
		return NAN;

	double term = x;    // prvnotna hodnota polynomu nastavena na hodnotu x
	double sum = 0.0;   // prvnotna hodnota suctu nastavena na hodnotu 0.0
	double dp = 1.0;    // prvotny citatel nastaveny na hodnotu 1.0
	unsigned int i = 1; // counter
	double taylor = taylor_log(x,n); // premenna taylor eliminuje nutnost volat funkciu cfrac_log() v cykle
	double fact = 1.0;  // faktorial

	// Cokolvek umocnene na nultu je vzdy jeden.
	if (y == 0)
		return 1;
	// Ak je zaklad nula a exponent vaccsi ako 0, vysledok je 0.
	else if (x == 0 && y > 0)
		return 0;
	// Ak je zaklad nula a exponent mensi ako 0, vysledok je nekonecno.
	else if (x == 0 && y < 0)
		return 1.0/0.0;
	// Ak je zaklad nekonecno a exponent vacsi ako 0, vysledok je nekonecno.
	else if (x == POS_INF && y > 0)
		return 1.0/0.0;	
	// Ak je zaklad nekonecno a exponent mensi ako 0, vysledok je nekonecno.
	else if (x == POS_INF && y < 0)
		return 0;
	// Ak je zaklad nekonecno, vysledok je nekonecno (okrem opatreni vyssie).
	else if (y == POS_INF)
		return 1.0/0.0;
	// Ak je zaklad -nekonecno, vysledok je nekonecno (okrem opatreni vyssie).
	else if (y == NEG_INF)
		return 0;

	// Kym i je mensie ako (pocet iteracii - 1)
	while ((i - 1 < n - 1)) 
	{
		fact = fact * i; // vypocet aktualneho faktorialu
		term = (y*taylor*dp)/fact; // vypocet aktualneho clena polynomu
		dp = (y*taylor*dp); // zmena predosleho citatela polynomu na aktualny
		
		// Ak predosly citatel alebo faktorial presahuju maximalnu hodnotu double, cyklus sa ukonci
		if (dp >= DOUBLE_UPRANGE || fact >= DOUBLE_UPRANGE)
			break;
	
		sum = sum + term; // sucet predoslych clenov polynomu s aktualne vypocitanym
		i++; // inkrementacia counteru i
	}
	return (sum+1); // funkcia vrati sucet polynomov + 1 (podla vzorca)
}

double taylorcf_pow(double x, double y, unsigned int n)
{
	if (isnan(x) || isnan(y))
		return NAN;

	double term = y;  // prvotny clen polynomu rovny y
	double sum = 0.0; // prvnotna hodnota suctu nastavena na hodnotu 0.0
	double dp = 1.0;  //  prvotny citatel nastaveny na hodnotu 1.0
	unsigned int i = 1; // counter
	double taylorcf = cfrac_log(x,n); // premenna taylorcf eliminuje nutnost volat funkciu cfrac_log() v cykle
	double fact = 1.0; // faktorial zo zlomku

	// Cokolvek umocnene na nultu je vzdy jeden.
	if (y == 0)
		return 1;
	// Ak je zaklad nula a exponent vaccsi ako 0, vysledok je 0.
	else if (x == 0 && y > 0)
		return 0;
	// Ak je zaklad nula a exponent mensi ako 0, vysledok je nekonecno.
	else if (x == 0 && y < 0)
		return 1.0/0.0;
	// Ak je zaklad nekonecno a exponent vacsi ako 0, vysledok je nekonecno.
	else if (x == POS_INF && y > 0)
		return 1.0/0.0;	
	// Ak je zaklad nekonecno a exponent mensi ako 0, vysledok je nekonecno.
	else if (x == POS_INF && y < 0)
		return 0;
	// Ak je zaklad nekonecno, vysledok je nekonecno (okrem opatreni vyssie).
	else if (y == POS_INF)
		return 1.0/0.0;
	// Ak je zaklad -nekonecno, vysledok je nekonecno (okrem opatreni vyssie).
	else if (y == NEG_INF)
		return 0;

	// Kym i je mensie ako (pocet iteracii - 1)
	while ((i - 1 < n - 1)) 
	{
		fact = fact * i; // vypocet aktualneho faktorialu
		term = (y*taylorcf*dp)/(fact); // vypocet aktualneho clena polynomu
		dp = (y*taylorcf*dp); // zmena predosleho citatela polynomu na aktualny

		// Ak predosly citatel alebo faktorial presahuju maximalnu hodnotu double, cyklus sa ukonci
		if (dp >= DOUBLE_UPRANGE || fact >= DOUBLE_UPRANGE)
			break;

		sum = sum + term; // sucet predoslych clenov polynomu s aktualne vypocitanym
		i++; // inkrementacia counteru i
	}
	return (sum+1); // funkcia vrati sucet polynomov + 1 (podla vzorca)
}

double mylog(double x)
{
	if (x == 0) // ak je zaklad 0, funkcia vrati zaporne nekonecno
		return -1.0/0.0;
	else if (isnan(x) || x < 0) // ak zaklad nie je cislo alebo je mensi ako 0, funkcia vrati nan
		return NAN;

	int n = 1; // counter
	double cf = cfrac_log(x,n); // povodna hodnota log vypocitaneho zretazenym zlomkom
	double cfp = 0.0; // premenna na ukladanie predoslej hodnoty zretazeneho logu()

	while(1) // nekonecny cyklus, ktory bude ukonceny returnom
	{
		while (fabs(cfp - cf) > EPS) // kym je rozdiel 2 vysledkov vacsi ako eps, cyklus bude pokracovat
		{
			n++; // inkrementacia counteru
			cfp = cf; // priradenie predoslej hodnoty do premennej cfp pre porovnanie
			cf = cfrac_log(x,n); // vypocitanie novej hodnoty
		}
		printf("No of iterations: %d.\n", n); // print poctu iteracii potrebnych na vypocet funkcie s danou presnostou
		return cf; // vratenie vypocitanej hodnoty
	}
	return cf;
}

double mypow(double x, double y)
{
	if (isnan(x) || isnan(y)) // ak exponent alebo zaklad nie je cislo, funkcia vrati nan
		return NAN;
	else if (x == NEG_INF && y == NEG_INF) // ak su exponent aj zaklad rovny zapornemu nekonecnu, vysledok je nekonecno
		return 0;
	else if (x == NEG_INF && y == POS_INF) // ak je exponent alebo zaklad nekonecno, vysledok je tiez nekonecno
		return 1.0/0.0;
	else if (x < 0 && (abs((int)y) < fabs(y))) // ak je exponent desatinne cislo a zaklad je mensi ako 0, vysledok je nan (resp. komplexne cislo)
		return NAN;
	else if (x == 0 && y < 0 && y != NEG_INF) // ak je zaklad nula a exponent je zaporny okrem zaporneho nekonecna, vysledok je nekonecno
		return 1.0/0.0;
	else if (x == POS_INF && y == NEG_INF) // ak je zaklad nekonecno a exponent negativne nekonecno, vysledok je 0
		return 0;
	
	int n = 1; // pocet iteracii
	double cf = taylorcf_pow(x,y,n); // vypocet povodnej exp. funkcie s poctom iteracii 1
	double cfp = 0.0; // premenna na ukladanie predoslej hodnoty zretazeneho logu()
	double xc = x; // copy of x

	if (x < 0) // ak je x zaporne, zmeni sa na kladne
		x = fabs(x);
	
	while(1) // nekonecny cyklus, ktory bude ukonceny returnom
	{
		while (fabs(cfp - cf) > EPS) // kym je rozdiel 2 vysledkov vacsi ako eps, cyklus bude pokracovat
		{
			n++; // inkrementacia poctu iteracii potrebnych na vypocet exp. funkcie
			cfp = cf; // ulozenie predchadzajuceho vysledku
			cf = taylorcf_pow(x,y,n); // vypocet sucasnej exp funckie s novym poctom iteracii
		}
		printf("No of iterations: %d.\n", n); // print poctu iteracii potrebnych na vypocet funkcie s danou presnostou
		
		if (xc < 0 && abs((int)y) % 2 == 0) // ak je zaklad zaporny a exponent parny, vrati sa povodny vysledok z cyklu vyssie
			return cf;
		else if (xc < 0 && y > 0) // ak je zaklad zaporny a exponent kladny, vrati sa zaporna hodnota vysledku
			return -1*cf;
		else if (xc < 0 && abs((int)y) % 2 != 0) // ak je zaklad zaporny a exponent neparny, vrati sa zaporna hodnota vysledku
			return -1*cf;
		else // pre vsetky ostatne pripady sa vrati povodna hodnota vysledku
			return cf;
	}
	return cf;
}

int main(int argc, char *argv[])
{
	// ERROR vypisy v pripade zadania zleho poctu argumentov.
	if (argc < 2 || argc > 5)
	{
		fprintf(stderr, "ERROR: invalid number of arguments\n");
		return -1;
	}
	else if (strcmp(argv[1], "--log") == 0 && argc != 4)
	{
		fprintf(stderr, "ERROR: invalid number of arguments\n");
		return -1;
	}
	else if (strcmp(argv[1], "--pow") == 0 && argc != 5)
	{
		fprintf(stderr, "ERROR: invalid number of arguments\n");
		return -1;
	}
	else if (strcmp(argv[1], "--premie") == 0 && argc != 4)
	{
		fprintf(stderr, "ERROR: invalid number of arguments\n");
		return -1;
	}

	// Funkcia log.
	if (strcmp(argv[1], "--log") == 0)
	{			
		char* ptr2 = NULL;
		char* ptr3 = NULL;
		double x = strtod(argv[2], &ptr2); // konverzia argumentu 2 na dat. typ double
		int n_s = strtol(argv[3], &ptr3, 10); // konverzia argumentu 3 na dat. typ integer
		
		// Kontrola, ci su v argumentoch iba cisla.
		if ((*ptr2 != '\0') || (*ptr3 != '\0'))
		{
			fprintf(stderr, "ERROR: wrong argument.\n");
			return -1;
		}

		// Ak je pocet iteracii cislo mensie alebo rovne 0, program skonci s danou napovedou.
		if (n_s <= 0)
		{
			fprintf(stderr, "ERROR: number of iterations needs to be > 0.\n");
			return -1;
		}
		unsigned int n = n_s; // ak je pocet iteracii zadany spravne, skopiruje sa hodnota n_s do unsigned int n
		
		// Vyprintovanie vysledku.
		printf("       log(%g) = %.12g\n", x, log(x));
		printf(" cfrac_log(%g) = %.12g\n", x, cfrac_log(x, n));
		printf("taylor_log(%g) = %.12g\n", x, taylor_log(x, n));
	}

	// Funkcia pow.
	else if (strcmp(argv[1], "--pow") == 0)
	{
		char* ptr2 = NULL;
		char* ptr3 = NULL;
		char* ptr4 = NULL;
		double x = strtod(argv[2], &ptr2); // konverzia argumentu 2 na dat. typ double
		double y = strtod(argv[3], &ptr3); // konverzia tretieho argumentu na double
		int n_s = strtol(argv[4], &ptr4, 10); // konverzia argumentu 3 na dat. typ integer
		
		// Kontrola, ci su v argumentoch iba cisla.
		if ((*ptr2 != '\0') || (*ptr3 != '\0') || (*ptr4 != '\0'))
		{
			fprintf(stderr, "ERROR: wrong argument.\n");
			return -1;
		}

		// Ak je pocet iteracii cislo mensie alebo rovne 0, program skonci s danou napovedou.
		if (n_s <= 0)
		{
			fprintf(stderr, "ERROR: number of iterations needs to be > 0.\n");
			return -1;
		}
		unsigned int n = n_s; // ak je pocet iteracii zadany spravne, skopiruje sa hodnota n_s do unsigned int n

		// Vyprintovanie vysledku.
		printf("         pow(%g,%g) = %.12g\n", x, y, pow(x,y));
		printf("  taylor_pow(%g,%g) = %.12g\n", x, y, taylor_pow(x, y, n));
		printf("taylorcf_pow(%g,%g) = %.12g\n", x, y, taylorcf_pow(x, y, n));
	}

	// Premiove ulohy.
	else if (strcmp(argv[1], "--premie") == 0)
	{
		char* ptr2 = NULL;
		char* ptr3 = NULL;
		double x = strtod(argv[2], &ptr2); // konverzia argumentu 2 na double
		double y = strtod(argv[3], &ptr3); // konverzia argumentu 3 na double
		
		// Kontrola, ci su v argumentoch iba cisla.
		if ((*ptr2 != '\0') || (*ptr3 != '\0'))
		{
			fprintf(stderr, "ERROR: wrong argument.\n");
			return -1;
		}

		// POW
		printf("pow(%g,%g) = %.7e\n", x, y, pow(x,y));
		printf("mypow(%g,%g) = %.7e\n", x, y, mypow(x,y));

		printf("\n");

		// LOG
		printf("log(%g) = %.7e\n", x, log(x));
		printf("mylog(%g) = %.7e\n", x, mylog(x));
	}

	// Ak pouzivatel zadal do prveho argumentu cokolvek ine ako --log alebo --pow, program sa ukonci s danym hlasenim.
	else
	{
		fprintf(stderr, "ERROR: Unknown command.\n");
		return -1;
	}

	return 0;
}