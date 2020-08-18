
/* ******************************* c204.c *********************************** */
/*  Předmět: Algoritmy (IAL) - FIT VUT v Brně                                 */
/*  Úkol: c204 - Převod infixového výrazu na postfixový (s využitím c202)     */
/*  Referenční implementace: Petr Přikryl, listopad 1994                      */
/*  Přepis do jazyka C: Lukáš Maršík, prosinec 2012                           */
/*  Upravil: Kamil Jeřábek, září 2019                                         */
/* ************************************************************************** */
/*
** Implementujte proceduru pro převod infixového zápisu matematického
** výrazu do postfixového tvaru. Pro převod využijte zásobník (tStack),
** který byl implementován v rámci příkladu c202. Bez správného vyřešení
** příkladu c202 se o řešení tohoto příkladu nepokoušejte.
**
** Implementujte následující funkci:
**
**    infix2postfix .... konverzní funkce pro převod infixového výrazu 
**                       na postfixový
**
** Pro lepší přehlednost kódu implementujte následující pomocné funkce:
**    
**    untilLeftPar .... vyprázdnění zásobníku až po levou závorku
**    doOperation .... zpracování operátoru konvertovaného výrazu
**
** Své řešení účelně komentujte.
**
** Terminologická poznámka: Jazyk C nepoužívá pojem procedura.
** Proto zde používáme pojem funkce i pro operace, které by byly
** v algoritmickém jazyce Pascalovského typu implemenovány jako
** procedury (v jazyce C procedurám odpovídají funkce vracející typ void).
**
**/

#include "c204.h"
int solved;

/*
** Pomocná funkce untilLeftPar.
** Slouží k vyprázdnění zásobníku až po levou závorku, přičemž levá závorka
** bude také odstraněna. Pokud je zásobník prázdný, provádění funkce se ukončí.
**
** Operátory odstraňované ze zásobníku postupně vkládejte do výstupního pole
** znaků postExpr. Délka převedeného výrazu a též ukazatel na první volné
** místo, na které se má zapisovat, představuje parametr postLen.
**
** Aby se minimalizoval počet přístupů ke struktuře zásobníku, můžete zde
** nadeklarovat a používat pomocnou proměnnou typu char.
*/
void untilLeftPar ( tStack* s, char* postExpr, unsigned* postLen )
{
    char topItem; // pomocna premenna
    stackTop(s, &topItem); // ulozenie hodnoty vrchneho prvku

    // cyklus na prechadzanie zasobnikom
    while(1)
    {
        // ak je zasobnik prazdny, cyklus sa ukonci
        if (stackEmpty(s))
        {
            return;
        }
        // ak je na vrchu zasobiku '(', cyklus sa ukonci
        if (topItem == '(')
        {
            stackPop(s); // odstranenie vrchneho prvku '(' zo zasobniku
            return; // ukoncenie cyklu
        }

        postExpr[(*postLen)++] = topItem;
		stackPop(s);
		stackTop(s, &topItem);
    }
}

/*
** Pomocná funkce doOperation.
** Zpracuje operátor, který je předán parametrem c po načtení znaku ze
** vstupního pole znaků.
**
** Dle priority předaného operátoru a případně priority operátoru na
** vrcholu zásobníku rozhodneme o dalším postupu. Délka převedeného 
** výrazu a taktéž ukazatel na první volné místo, do kterého se má zapisovat, 
** představuje parametr postLen, výstupním polem znaků je opět postExpr.
*/
void doOperation ( tStack* s, char c, char* postExpr, unsigned* postLen )
{
    // operator je vkladany na vrchol zasobniku ak je zasobnik prazdny
    if (stackEmpty(s))
    {
        stackPush(s, c); // pridanie operatoru do zasobniku
        return;
    }

    char topItem; // premenna reprezentujuca vrchol zasobniku
    stackTop(s, &topItem); // ulozenie hodnoty vrchneho prvku zasobniku

    // na vrchole zasobniku je operator s vyssou prioritou ako aktualny operator
    // operator sa teda odstrani, vlozi do vystupneho retazca a funkcia sa opakuje znovu
    if ((topItem == '*' || topItem == '/') && (c == '*' || c == '/' || c == '+' || c == '-'))
    {
        postExpr[(*postLen)++] = topItem; // umiestnenie vrchola zasobniku do vystupneho pola znakov
        stackPop(s); // odstranenie vrchola zasobniku
        doOperation(s, c, postExpr, postLen); // operacia sa opakuje
        return;
    }
    // na vrchole zasobniku je operator s rovnakou prioritou ako aktualny operator
    // operator sa teda odstrani, vlozi do vystupneho retazca a funkcia sa opakuje znovu
    else if ((topItem == '+' || topItem == '-') && (c == '+' || c == '-'))
    {
        postExpr[(*postLen)++] = topItem; // umiestnenie vrchola zasobniku do vystupneho pola znakov
        stackPop(s); // odstranenie vrchola zasobniku
        doOperation(s, c, postExpr, postLen); // operacia sa opakuje
        return;
    }
    // operator je vkladany na vrchol zasobniku ak:
    // - na vrchole zasobiku je lava zatvorka
    // - na vrchole zasobniku je operator s nizsou prioritou
    else
    {
        stackPush(s, c); // pridanie operatoru do zasobniku
        return;
    }
}

/* 
** Konverzní funkce infix2postfix.
** Čte infixový výraz ze vstupního řetězce infExpr a generuje
** odpovídající postfixový výraz do výstupního řetězce (postup převodu
** hledejte v přednáškách nebo ve studijní opoře). Paměť pro výstupní řetězec
** (o velikosti MAX_LEN) je třeba alokovat. Volající funkce, tedy
** příjemce konvertovaného řetězce, zajistí korektní uvolnění zde alokované
** paměti.
**
** Tvar výrazu:
** 1. Výraz obsahuje operátory + - * / ve významu sčítání, odčítání,
**    násobení a dělení. Sčítání má stejnou prioritu jako odčítání,
**    násobení má stejnou prioritu jako dělení. Priorita násobení je
**    větší než priorita sčítání. Všechny operátory jsou binární
**    (neuvažujte unární mínus).
**
** 2. Hodnoty ve výrazu jsou reprezentovány jednoznakovými identifikátory
**    a číslicemi - 0..9, a..z, A..Z (velikost písmen se rozlišuje).
**
** 3. Ve výrazu může být použit předem neurčený počet dvojic kulatých
**    závorek. Uvažujte, že vstupní výraz je zapsán správně (neošetřujte
**    chybné zadání výrazu).
**
** 4. Každý korektně zapsaný výraz (infixový i postfixový) musí být uzavřen 
**    ukončovacím znakem '='.
**
** 5. Při stejné prioritě operátorů se výraz vyhodnocuje zleva doprava.
**
** Poznámky k implementaci
** -----------------------
** Jako zásobník použijte zásobník znaků tStack implementovaný v příkladu c202. 
** Pro práci se zásobníkem pak používejte výhradně operace z jeho rozhraní.
**
** Při implementaci využijte pomocné funkce untilLeftPar a doOperation.
**
** Řetězcem (infixového a postfixového výrazu) je zde myšleno pole znaků typu
** char, jenž je korektně ukončeno nulovým znakem dle zvyklostí jazyka C.
**
** Na vstupu očekávejte pouze korektně zapsané a ukončené výrazy. Jejich délka
** nepřesáhne MAX_LEN-1 (MAX_LEN i s nulovým znakem) a tedy i výsledný výraz
** by se měl vejít do alokovaného pole. Po alokaci dynamické paměti si vždycky
** ověřte, že se alokace skutečně zdrařila. V případě chyby alokace vraťte namísto
** řetězce konstantu NULL.
*/
char* infix2postfix (const char* infExpr)
{
	// alokacia pamate pre vystupny vyraz a kontrola uspesnosti alokacie
    char *postExpr = (char *) malloc(MAX_LEN);
    if (postExpr == NULL)
    {
        return NULL;
    }
	
    // alokacia pamate pre zasobnik a kontrola uspesnosti alokacie
    tStack *stack = malloc(sizeof(char *) * STACK_SIZE);
    if (stack == NULL)
    {
        free(postExpr);
        return NULL;
    }

    stackInit(stack); // inicializacia zasobniku
    unsigned int postLen = 0; // zaciatocna dlzka postfixoveho vyrazu

    for(int i = 0; i <= snprintf(NULL, 0, "%s", infExpr); i++)
    {
        // spracovavana polozka je operand
        if ((infExpr[i] >= '0' && infExpr[i] <= '9') || (infExpr[i] >= 'a' && infExpr[i] <= 'z') || (infExpr[i] >= 'A' && infExpr[i] <= 'Z'))
        {
            postExpr[postLen++] = infExpr[i]; // prida sa do vystupneho pola znakov
        }
        // spracovavana polozka je operator
        else if (infExpr[i] == '+' || infExpr[i] == '-' || infExpr[i] == '*' || infExpr[i] =='/')
        {
            doOperation(stack, infExpr[i], postExpr, &postLen);
        }
        // spracovavana polozka je lava zatvorka
        else if(infExpr[i] == '(')
        {
            stackPush(stack, infExpr[i]); // prida sa na zasobnik
        }
        // spracovavana polozka je prava zatvorka
        else if(infExpr[i] == ')')
        {
            untilLeftPar(stack, postExpr, &postLen);
        }
        // sme na konci vyrazu
        else if (infExpr[i] == '=')
        {
            // prvky zo zasobniku su postupne pridavaneho do vystupneho pola znakov 
            while(1)
            {
                if (stackEmpty(stack))
                {
                    break;
                }

                stackTop(stack, &postExpr[postLen++]);
                stackPop(stack);
            }
        }
    }

    // do vystupneho pola znakov sa prida znak = a ukoncovaci znak
    postExpr[postLen++] = '=';
    postExpr[postLen] = '\0';

    free(stack);
    return(postExpr);
}

/* Konec c204.c */
