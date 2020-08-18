
/* c206.c **********************************************************}
{* Téma: Dvousměrně vázaný lineární seznam
**
**                   Návrh a referenční implementace: Bohuslav Křena, říjen 2001
**                            Přepracované do jazyka C: Martin Tuček, říjen 2004
**                                            Úpravy: Kamil Jeřábek, září 2019
**
** Implementujte abstraktní datový typ dvousměrně vázaný lineární seznam.
** Užitečným obsahem prvku seznamu je hodnota typu int.
** Seznam bude jako datová abstrakce reprezentován proměnnou
** typu tDLList (DL znamená Double-Linked a slouží pro odlišení
** jmen konstant, typů a funkcí od jmen u jednosměrně vázaného lineárního
** seznamu). Definici konstant a typů naleznete v hlavičkovém souboru c206.h.
**
** Vaším úkolem je implementovat následující operace, které spolu
** s výše uvedenou datovou částí abstrakce tvoří abstraktní datový typ
** obousměrně vázaný lineární seznam:
**
**      DLInitList ...... inicializace seznamu před prvním použitím,
**      DLDisposeList ... zrušení všech prvků seznamu,
**      DLInsertFirst ... vložení prvku na začátek seznamu,
**      DLInsertLast .... vložení prvku na konec seznamu,
**      DLFirst ......... nastavení aktivity na první prvek,
**      DLLast .......... nastavení aktivity na poslední prvek,
**      DLCopyFirst ..... vrací hodnotu prvního prvku,
**      DLCopyLast ...... vrací hodnotu posledního prvku,
**      DLDeleteFirst ... zruší první prvek seznamu,
**      DLDeleteLast .... zruší poslední prvek seznamu,
**      DLPostDelete .... ruší prvek za aktivním prvkem,
**      DLPreDelete ..... ruší prvek před aktivním prvkem,
**      DLPostInsert .... vloží nový prvek za aktivní prvek seznamu,
**      DLPreInsert ..... vloží nový prvek před aktivní prvek seznamu,
**      DLCopy .......... vrací hodnotu aktivního prvku,
**      DLActualize ..... přepíše obsah aktivního prvku novou hodnotou,
**      DLSucc .......... posune aktivitu na další prvek seznamu,
**      DLPred .......... posune aktivitu na předchozí prvek seznamu,
**      DLActive ........ zjišťuje aktivitu seznamu.
**
** Při implementaci jednotlivých funkcí nevolejte žádnou z funkcí
** implementovaných v rámci tohoto příkladu, není-li u funkce
** explicitně uvedeno něco jiného.
**
** Nemusíte ošetřovat situaci, kdy místo legálního ukazatele na seznam 
** předá někdo jako parametr hodnotu NULL.
**
** Svou implementaci vhodně komentujte!
**
** Terminologická poznámka: Jazyk C nepoužívá pojem procedura.
** Proto zde používáme pojem funkce i pro operace, které by byly
** v algoritmickém jazyce Pascalovského typu implemenovány jako
** procedury (v jazyce C procedurám odpovídají funkce vracející typ void).
**/

#include "c206.h"

int solved;
int errflg;

/*
** Vytiskne upozornění na to, že došlo k chybě.
** Tato funkce bude volána z některých dále implementovaných operací.
**/	
void DLError()
{
    printf ("*ERROR* The program has performed an illegal operation.\n");
    errflg = TRUE;             /* globální proměnná -- příznak ošetření chyby */
    return;
}

/*
** Provede inicializaci seznamu L před jeho prvním použitím (tzn. žádná
** z následujících funkcí nebude volána nad neinicializovaným seznamem).
** Tato inicializace se nikdy nebude provádět nad již inicializovaným
** seznamem, a proto tuto možnost neošetřujte. Vždy předpokládejte,
** že neinicializované proměnné mají nedefinovanou hodnotu.
**/
void DLInitList (tDLList *L)
{
    L->Act = NULL;
    L->First = NULL;
    L->Last = NULL;
}

/*
** Zruší všechny prvky seznamu L a uvede seznam do stavu, v jakém
** se nacházel po inicializaci. Rušené prvky seznamu budou korektně
** uvolněny voláním operace free. 
**/
void DLDisposeList (tDLList *L)
{
    tDLElemPtr element = L->First;
	
    while (L->First != NULL) // priechod celym zoznamom
	{
		L->First = L->First->rptr; // posunutie ukazovatela na dalsi prvok
		free(element); // uvolnenie prvku
		element = L->First; // zmenenie ukazovatela
	}

	// vsetky ukazovatele zoznamu sa nastavia na NULL
    L->First = NULL;
    L->Last = NULL;
    L->Act = NULL;
}

/*
** Vloží nový prvek na začátek seznamu L.
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/
void DLInsertFirst (tDLList *L, int val)
{
    // alokacia priestoru pre novy prvok a kontrola uspesnosti alokacie
	tDLElemPtr newElement = (tDLElemPtr) malloc(sizeof(struct tDLElem));
    if (newElement == NULL)
    {
        DLError();
        return;
    }

    newElement->data = val; // vlozenie dat do prvku
    newElement->rptr = L->First; // ukazovatel na nasledujuci prvok vkladaneho ukazuje na aktualny prvy
    newElement->lptr = NULL; // ukazovatel na lavy prvok vkladaneho je NULL

    // zoznam uz obsahoval nejaky prvok
    if (L->First != NULL)
    {
        L->First->lptr = newElement; // lavy ukazovatel aktualneho prveho prvku ukazuje na vkladany
    }
    // zoznam je prazdny
    else
    {
        L->Last = newElement; // posledny prvok zoznamu je aktualny vkladany
    }

    L->First = newElement; // korekcia ukazovatela prveho prvku zoznamu
}

/*
** Vloží nový prvek na konec seznamu L (symetrická operace k DLInsertFirst).
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/ 
void DLInsertLast(tDLList *L, int val)
{
    // alokacia pamate pre novy prvok a kontrola uspesnosti alokacie
	tDLElemPtr newElement = (tDLElemPtr) malloc(sizeof(struct tDLElem));
    if (newElement == NULL)
    {
        DLError();
        return;
    }

    newElement->data = val; // vlozenie dat do prvku
    newElement->lptr = L->Last; // lavy ukazovatel ukazuje na aktualne posledny prvok
    newElement->rptr = NULL; // pravy ukazovatel je NULL

    // zoznam uz obsahoval nejaky prvok
    if (L->Last != NULL)
    {
        L->Last->rptr = newElement; // pravy ukazovatel posledneho prvku ukazuje na novy prvok
    }
    // zoznam je prazdny
    else
    {
        L->First = newElement; // prvy prvok je aktualne vkladany prvok
    }

    L->Last = newElement; // korekcia ukazovatela posledneho prvku zoznamu
}

/*
** Nastaví aktivitu na první prvek seznamu L.
** Funkci implementujte jako jediný příkaz (nepočítáme-li return),
** aniž byste testovali, zda je seznam L prázdný.
**/
void DLFirst (tDLList *L)
{
	L->Act = L->First;
}

/*
** Nastaví aktivitu na poslední prvek seznamu L.
** Funkci implementujte jako jediný příkaz (nepočítáme-li return),
** aniž byste testovali, zda je seznam L prázdný.
**/
void DLLast (tDLList *L)
{
	L->Act = L->Last;
}

/*
** Prostřednictvím parametru val vrátí hodnotu prvního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/
void DLCopyFirst (tDLList *L, int *val)
{
    // kontrola, ci je zoznam naplneny
    if (L->First == NULL)
    {
        DLError();
        return;
    }

    *val = L->First->data; // vratenie hodnoty prveho prvku do premennej val
}

/*
** Prostřednictvím parametru val vrátí hodnotu posledního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/
void DLCopyLast (tDLList *L, int *val)
{
	// kontrola, ci je zoznam naplneny
    if (L->First == NULL)
    {
        DLError();
        return;
    }

    *val = L->Last->data; // vratenie hodnoty posledneho prvku do premennej val
}

/*
** Zruší první prvek seznamu L. Pokud byl první prvek aktivní, aktivita 
** se ztrácí. Pokud byl seznam L prázdný, nic se neděje.
**/
void DLDeleteFirst (tDLList *L) 
{
	// kontrola, ci je zoznam naplneny
    if (L->First == NULL)
    {
        return;
    }

    tDLElemPtr newElement = L->First;
    
    // kontrola, resp. zrusenie aktivity prveho prvku
    if (L->First == L->Act)
    {
        L->Act = NULL;
    }
    // kontrola, ci je v zozname jeden prvok, ak ano, zrusi sa
    if (L->First == L->Last)
    {
        L->First = NULL;
        L->Last = NULL;
    }
    // zoznam ma viacej prvkov
    else
    {
        L->First = L->First->rptr; // na prvy prvok sa nastavi druhy prvok zoznamu
        L->First->lptr = NULL; // povodny prvy prvok sa vymaze
    }

    free(newElement);
}	

/*
** Zruší poslední prvek seznamu L. Pokud byl poslední prvek aktivní,
** aktivita seznamu se ztrácí. Pokud byl seznam L prázdný, nic se neděje.
**/ 
void DLDeleteLast (tDLList *L)
{
    // kontrola, ci je zoznam naplneny
    if (L->First == NULL)
    {
        return;
    }
    
    tDLElemPtr newElement = L->Last;

    // kontrola, resp. zrusenie aktivity posledneho prvku
    if (L->Last == L->Act)
    {
        L->Act = NULL;
    }
    // kontrola, ci je v zozname jeden prvok, ak ano, zrusi sa
    if (L->First == L->Last)
    {
        L->First = NULL;
        L->Last = NULL;
    }
    // zoznam ma viacej prvkov
    else
    {
        L->Last = L->Last->lptr; // na prvy prvok sa nastavi druhy prvok zoznamu
        L->Last->rptr = NULL; // povodny prvy prvok sa vymaze
    }

    free(newElement); // uvolnenie newElement
}

/*
** Zruší prvek seznamu L za aktivním prvkem.
** Pokud je seznam L neaktivní nebo pokud je aktivní prvek
** posledním prvkem seznamu, nic se neděje.
**/
void DLPostDelete (tDLList *L)
{
    // ak je zoznam neaktivny, alebo aktivny prvok je posledny, nic sa nedeje
	if (L->Act == NULL || L->Act == L->Last)
    {
        return;
    }

    tDLElemPtr newElement = L->Act->rptr; // ukazovatel na prvok za rusenym prvkom
    L->Act->rptr = newElement->rptr; // ukazovatel dalsieho prvku aktivneho prvku ukazuje na ob-prvok
    
    if (newElement == L->Last) // newElement prvok je posledny
    {
        L->Last = L->Act; // posledny prvok bude aktivny
    }
    else // newElement  nie je posledny
    {
        newElement->rptr->lptr = L->Act; // prvok nalavo od ruseneho ukazuje na active
    }

    free(newElement); // uvolnenie newElement
}

/*
** Zruší prvek před aktivním prvkem seznamu L .
** Pokud je seznam L neaktivní nebo pokud je aktivní prvek
** prvním prvkem seznamu, nic se neděje.
**/
void DLPreDelete (tDLList *L)
{
	// ak je zoznam neaktivny, alebo aktivny prvok je posledny, nic sa nedeje
	if (L->Act == NULL || L->Act == L->First)
    {
        return;
    }

    tDLElemPtr newElement = L->Act->lptr; // ukazovatel na prvok pred rusenym prvkom
    L->Act->lptr = newElement->lptr; // ukazovatel predosleho prvku aktivneho prvku ukazuje na ob-prvok
    if (newElement == L->First) // newElement je prvy prvok zoznamu
    {
        L->First = L->Act; // prvy prvok bude aktivny
    }
    else // newElement nie je prvu prvok zoznamu
    {
        newElement->lptr->rptr = L->Act; // pravy ukazovatel prvku pred neElement je aktivny
    }

    free(newElement); // uvolnenie newElement
}

/*
** Vloží prvek za aktivní prvek seznamu L.
** Pokud nebyl seznam L aktivní, nic se neděje.
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/
void DLPostInsert (tDLList *L, int val)
{
	// kontrola aktivity zoznamu
    if (L->Act == NULL)
    {
        return;
    }

    // alokacia priesotoru pre novy prvok a kontrola uspesnosti alokacie
    tDLElemPtr newElement = (tDLElemPtr) malloc(sizeof(struct tDLElem));
    if (newElement == NULL)
    {
        DLError();
        return;
    }

    newElement->data = val; // vlozenie hodnoty do noveho prvku
    newElement->lptr = L->Act; // nastavenie predchadzajuceho prvku novemu
    newElement->rptr = L->Act->rptr; // nastavenie nasledujuceho prvku novemu

    if (L->Act == L->Last) // aktivny prvok je zaroven posledny
    {
        L->Last = newElement; // vkladany prvok sa stava poslednym
    }
    else // aktivny prvok nie je posledny
    {
        L->Act->rptr->lptr = newElement; // lavy ukazovatel prvku napravo od aktivneho prvku bude vkladany prvok
    }

    L->Act->rptr = newElement; // pravy ukazovatel aktivneho prvku bude novy prvok
}

/*
** Vloží prvek před aktivní prvek seznamu L.
** Pokud nebyl seznam L aktivní, nic se neděje.
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/
void DLPreInsert (tDLList *L, int val)
{
	// kontrola aktivity zoznamu
    if (L->Act == NULL)
    {
        return;
    }

    // alokacia priesotoru pre novy prvok a kontrola uspesnosti alokacie
    tDLElemPtr newElement = (tDLElemPtr) malloc(sizeof(struct tDLElem));
    if (newElement == NULL)
    {
        DLError();
        return;
    }

    newElement->data = val; // vlozenie hodnoty do noveho prvku
    newElement->lptr = L->Act->rptr; // nastavenie predchadzajuceho prvku novemu
    newElement->rptr = L->Act; // nastavenie nasledujuceho prvku novemu

    if (L->Act == L->First) // aktivny prvok je zaroven prvy
    {
        L->First = newElement; // vkladany prvok sa stava prvym
    }
    else // aktivny prvok nie je prvy
    {
        L->Act->lptr->rptr = newElement; // pravy ukazovatel prvku nalavo od aktivneho prvku bude vkladany prvok
    }

    L->Act->lptr = newElement; // lavy ukazovatel od aktivneho prvku bude novy prvok
}

/*
** Prostřednictvím parametru val vrátí hodnotu aktivního prvku seznamu L.
** Pokud seznam L není aktivní, volá funkci DLError ().
**/
void DLCopy (tDLList *L, int *val)
{
	// kontrola aktivity zoznamu
    if (L->Act == NULL)
    {
        DLError();
        return;
    }

    *val = L->Act->data; // vratenie hodnoty aktivneho prvku do premennej val
}

/*
** Přepíše obsah aktivního prvku seznamu L.
** Pokud seznam L není aktivní, nedělá nic.
**/
void DLActualize (tDLList *L, int val)
{
	// kontrola aktivity zoznamu
    if (L->Act == NULL)
    {
        return;
    }

    L->Act->data = val; // prepisanie dat aktivneho prvku
}

/*
** Posune aktivitu na následující prvek seznamu L.
** Není-li seznam aktivní, nedělá nic.
** Všimněte si, že při aktivitě na posledním prvku se seznam stane neaktivním.
**/
void DLSucc (tDLList *L)
{
	// kontrola aktivity zoznamu
    if (L->Act == NULL)
    {
        return;
    }

    L->Act = L->Act->rptr; // posunutie aktivity na dalsi prvok
}

/*
** Posune aktivitu na předchozí prvek seznamu L.
** Není-li seznam aktivní, nedělá nic.
** Všimněte si, že při aktivitě na prvním prvku se seznam stane neaktivním.
**/
void DLPred (tDLList *L)
{
	// kontrola aktivity zoznamu
    if (L->Act == NULL)
    {
        return;
    }

    L->Act = L->Act->lptr; // posunutie aktivity na dalsi prvok
}

/*
** Je-li seznam L aktivní, vrací nenulovou hodnotu, jinak vrací 0.
** Funkci je vhodné implementovat jedním příkazem return.
**/
int DLActive (tDLList *L)
{
	return (L->Act != NULL);
}

/* Konec c206.c*/
