/* c402.c: ********************************************************************}
{* Téma: Nerekurzivní implementace operací nad BVS
**                                     Implementace: Petr Přikryl, prosinec 1994
**                                           Úpravy: Petr Přikryl, listopad 1997
**                                                     Petr Přikryl, květen 1998
**			  	                        Převod do jazyka C: Martin Tuček, srpen 2005
**                                         Úpravy: Bohuslav Křena, listopad 2009
**                                                 Karel Masařík, říjen 2013
**                                                 Radek Hranický 2014-2018
**
** S využitím dynamického přidělování paměti, implementujte NEREKURZIVNĚ
** následující operace nad binárním vyhledávacím stromem (předpona BT znamená
** Binary Tree a je u identifikátorů uvedena kvůli možné kolizi s ostatními
** příklady):
**
**     BTInit .......... inicializace stromu
**     BTInsert ........ nerekurzivní vložení nového uzlu do stromu
**     BTPreorder ...... nerekurzivní průchod typu pre-order
**     BTInorder ....... nerekurzivní průchod typu in-order
**     BTPostorder ..... nerekurzivní průchod typu post-order
**     BTDisposeTree ... zruš všechny uzly stromu
**
** U všech funkcí, které využívají některý z průchodů stromem, implementujte
** pomocnou funkci pro nalezení nejlevějšího uzlu v podstromu.
**
** Přesné definice typů naleznete v souboru c402.h. Uzel stromu je typu tBTNode,
** ukazatel na něj je typu tBTNodePtr. Jeden uzel obsahuje položku int Cont,
** která současně slouží jako užitečný obsah i jako vyhledávací klíč
** a ukazatele na levý a pravý podstrom (LPtr a RPtr).
**
** Příklad slouží zejména k procvičení nerekurzivních zápisů algoritmů
** nad stromy. Než začnete tento příklad řešit, prostudujte si důkladně
** principy převodu rekurzivních algoritmů na nerekurzivní. Programování
** je především inženýrská disciplína, kde opětné objevování Ameriky nemá
** místo. Pokud se Vám zdá, že by něco šlo zapsat optimálněji, promyslete
** si všechny detaily Vašeho řešení. Povšimněte si typického umístění akcí
** pro různé typy průchodů. Zamyslete se nad modifikací řešených algoritmů
** například pro výpočet počtu uzlů stromu, počtu listů stromu, výšky stromu
** nebo pro vytvoření zrcadlového obrazu stromu (pouze popřehazování ukazatelů
** bez vytváření nových uzlů a rušení starých).
**
** Při průchodech stromem použijte ke zpracování uzlu funkci BTWorkOut().
** Pro zjednodušení práce máte předem připraveny zásobníky pro hodnoty typu
** bool a tBTNodePtr. Pomocnou funkci BTWorkOut ani funkce pro práci
** s pomocnými zásobníky neupravujte
** Pozor! Je třeba správně rozlišovat, kdy použít dereferenční operátor *
** (typicky při modifikaci) a kdy budeme pracovat pouze se samotným ukazatelem
** (např. při vyhledávání). V tomto příkladu vám napoví prototypy funkcí.
** Pokud pracujeme s ukazatelem na ukazatel, použijeme dereferenci.
**/

#include "c402.h"
int solved;

/*   ---------
** Pomocná funkce, kterou budete volat při průchodech stromem pro zpracování
** uzlu určeného ukazatelem Ptr. Tuto funkci neupravujte.
**/
void BTWorkOut (tBTNodePtr Ptr) {
	if (Ptr==NULL)
    	printf("Chyba: Funkce BTWorkOut byla volána s NULL argumentem!\n");
  	else
    	printf("Výpis hodnoty daného uzlu> %d\n",Ptr->Cont);
}

/* -------------------------------------------------------------------------- */
/*
** Funkce pro zásobník hotnot typu tBTNodePtr. Tyto funkce neupravujte.
**/

/*   ------
** Inicializace zásobníku.
**/
void SInitP (tStackP *S) {
	S->top = 0;
}

/*   ------
** Vloží hodnotu na vrchol zásobníku.
**/
void SPushP (tStackP *S, tBTNodePtr ptr) {
	/* Při implementaci v poli může dojít k přetečení zásobníku. */
	if (S->top==MAXSTACK)
    	printf("Chyba: Došlo k přetečení zásobníku s ukazateli!\n");
  	else {
		S->top++;
		S->a[S->top]=ptr;
	}
}

/*         --------
** Odstraní prvek z vrcholu zásobníku a současně vrátí jeho hodnotu.
**/
tBTNodePtr STopPopP (tStackP *S) {
    /* Operace nad prázdným zásobníkem způsobí chybu. */
	if (S->top==0) {
		printf("Chyba: Došlo k podtečení zásobníku s ukazateli!\n");
		return(NULL);
	}
	else {
		return (S->a[S->top--]);
	}
}

/*   -------
** Je-li zásobník prázdný, vrátí hodnotu true.
**/
bool SEmptyP (tStackP *S) {
  return(S->top==0);
}

/* -------------------------------------------------------------------------- */
/*
** Funkce pro zásobník hotnot typu bool. Tyto funkce neupravujte.
*/

/*   ------
** Inicializace zásobníku.
**/
void SInitB (tStackB *S) {
	S->top = 0;
}

/*   ------
** Vloží hodnotu na vrchol zásobníku.
**/
void SPushB (tStackB *S,bool val) {
     /* Při implementaci v poli může dojít k přetečení zásobníku. */
	if (S->top==MAXSTACK)
		printf("Chyba: Došlo k přetečení zásobníku pro boolean!\n");
	else {
		S->top++;
		S->a[S->top]=val;
	}
}

/*   --------
** Odstraní prvek z vrcholu zásobníku a současně vrátí jeho hodnotu.
**/
bool STopPopB (tStackB *S) {
    /* Operace nad prázdným zásobníkem způsobí chybu. */
	if (S->top==0) {
		printf("Chyba: Došlo k podtečení zásobníku pro boolean!\n");
		return(NULL);
	}
	else {
		return(S->a[S->top--]);
	}
}

/*   -------
** Je-li zásobník prázdný, vrátí hodnotu true.
**/
bool SEmptyB (tStackB *S) {
	return(S->top==0);
}

/* -------------------------------------------------------------------------- */
/*
** Následuje jádro domácí úlohy - funkce, které máte implementovat.
*/

/*   ------
** Provede inicializaci binárního vyhledávacího stromu.
**
** Inicializaci smí programátor volat pouze před prvním použitím binárního
** stromu, protože neuvolňuje uzly neprázdného stromu (a ani to dělat nemůže,
** protože před inicializací jsou hodnoty nedefinované, tedy libovolné).
** Ke zrušení binárního stromu slouží procedura BTDisposeTree.
**
** Všimněte si, že zde se poprvé v hlavičce objevuje typ ukazatel na ukazatel,
** proto je třeba při práci s RootPtr použít dereferenční operátor *.
**/
void BTInit (tBTNodePtr *RootPtr) {
	*RootPtr = NULL;
}

/*   --------
** Vloží do stromu nový uzel s hodnotou Content.
**
** Z pohledu vkládání chápejte vytvářený strom jako binární vyhledávací strom,
** kde uzly s hodnotou menší než má otec leží v levém podstromu a uzly větší
** leží vpravo. Pokud vkládaný uzel již existuje, neprovádí se nic (daná hodnota
** se ve stromu může vyskytnout nejvýše jednou). Pokud se vytváří nový uzel,
** vzniká vždy jako list stromu. Funkci implementujte nerekurzivně.
**/
void BTInsert (tBTNodePtr *RootPtr, int Content) {
	tBTNodePtr tmp = NULL;
	tBTNodePtr tmp2 = NULL;
	
	if (*RootPtr == NULL) {
		if ((tmp = malloc(sizeof(struct tBTNode))) == NULL) {
			return;
		}
		tmp->Cont = Content;
		tmp->RPtr = NULL;
		tmp->LPtr = NULL;
		*RootPtr = tmp;
	} else {
		tmp = *RootPtr;
		while(1) {
			if (tmp->Cont < Content && tmp->RPtr == NULL) {
				if ((tmp2 = malloc(sizeof(struct tBTNode))) == NULL) {
					return;
				}
				tmp->RPtr = tmp2;
				tmp2->Cont = Content;
				tmp2->LPtr = NULL;
				tmp2->RPtr = NULL;
				return;
			} else if (tmp->Cont > Content && tmp->LPtr == NULL) {
				if ((tmp2 = malloc(sizeof(struct tBTNode))) == NULL) {
					return;
				}
				tmp->LPtr = tmp2;
				tmp2->Cont = Content;
				tmp2->LPtr = NULL;
				tmp2->RPtr = NULL;
				return;
			} else if (tmp->Cont == Content) {
				return;
			} else if (tmp->Cont < Content) {
				tmp = tmp->RPtr;
				continue;
			} else if (tmp->Cont > Content) {
				tmp = tmp->LPtr;
				continue;
			}
		}
	}
}

/*                                  PREORDER                                  */

/*   -----------------
** Jde po levě větvi podstromu, dokud nenarazí na jeho nejlevější uzel.
**
** Při průchodu Preorder navštívené uzly zpracujeme voláním funkce BTWorkOut()
** a ukazatele na ně is uložíme do zásobníku.
**/
void Leftmost_Preorder (tBTNodePtr ptr, tStackP *Stack)	{
	while (ptr != NULL) {
		BTWorkOut(ptr);
		SPushP(Stack, ptr);
		ptr = ptr->LPtr;
	}
}

/*   ----------
** Průchod stromem typu preorder implementovaný nerekurzivně s využitím funkce
** Leftmost_Preorder a zásobníku ukazatelů. Zpracování jednoho uzlu stromu
** realizujte jako volání funkce BTWorkOut().
**/
void BTPreorder (tBTNodePtr RootPtr) {
	tStackP s;
	SInitP(&s);
	Leftmost_Preorder(RootPtr, &s);
	while (!SEmptyP(&s)) {
		RootPtr = STopPopP(&s);
		Leftmost_Preorder(RootPtr->RPtr, &s);
	}
}


/*                                  INORDER                                   */

/*   ----------------
** Jde po levě větvi podstromu, dokud nenarazí na jeho nejlevější uzel.
**
** Při průchodu Inorder ukládáme ukazatele na všechny navštívené uzly do
** zásobníku.
**/
void Leftmost_Inorder(tBTNodePtr ptr, tStackP *Stack) {
	while (ptr != NULL) {
		SPushP(Stack, ptr);
		ptr = ptr->LPtr;
	}
}

/*   ---------
** Průchod stromem typu inorder implementovaný nerekurzivně s využitím funkce
** Leftmost_Inorder a zásobníku ukazatelů. Zpracování jednoho uzlu stromu
** realizujte jako volání funkce BTWorkOut().
**/
void BTInorder (tBTNodePtr RootPtr)	{
	tStackP s;
	SInitP(&s);
	Leftmost_Inorder(RootPtr, &s);
	while (!SEmptyP(&s)) {
		RootPtr = STopPopP(&s);
		BTWorkOut(RootPtr);
		Leftmost_Inorder(RootPtr->RPtr, &s);
	}
}

/*                                 POSTORDER                                  */

/*           --------
** Jde po levě větvi podstromu, dokud nenarazí na jeho nejlevější uzel.
**
** Při průchodu Postorder ukládáme ukazatele na navštívené uzly do zásobníku
** a současně do zásobníku bool hodnot ukládáme informaci, zda byl uzel
** navštíven poprvé a že se tedy ještě nemá zpracovávat.
**/
void Leftmost_Postorder (tBTNodePtr ptr, tStackP *StackP, tStackB *StackB) {
	while (ptr != NULL) {
		SPushP(StackP, ptr);
		SPushB(StackB, TRUE);
		ptr = ptr->LPtr;
	}
}

/*           -----------
** Průchod stromem typu postorder implementovaný nerekurzivně s využitím funkce
** Leftmost_Postorder, zásobníku ukazatelů a zásobníku hotdnot typu bool.
** Zpracování jednoho uzlu stromu realizujte jako volání funkce BTWorkOut().
**/
void BTPostorder (tBTNodePtr RootPtr) {
	bool left;
	tStackP pSTack;
	tStackB bStack;
	SInitP(&pSTack);
	SInitB(&bStack);
	Leftmost_Postorder(RootPtr, &pSTack, &bStack);
	
	while (!SEmptyP(&pSTack)) {
		RootPtr = pSTack.a[pSTack.top];
		left = STopPopB(&bStack);
		if (left) {
			SPushB(&bStack, FALSE);
			Leftmost_Postorder(RootPtr->RPtr, &pSTack, &bStack);
		} else {
			STopPopP(&pSTack);
			BTWorkOut(RootPtr);
		}
	}
}


/*   -------------
** Zruší všechny uzly stromu a korektně uvolní jimi zabranou paměť.
**
** Funkci implementujte nerekurzivně s využitím zásobníku ukazatelů.
**/
void BTDisposeTree (tBTNodePtr *RootPtr) {
	tStackP s;
	SInitP(&s);

	while (((*RootPtr) != NULL) || (!SEmptyP(&s))) {
		if ((*RootPtr == NULL) && (!SEmptyP(&s))) {
			(*RootPtr) = STopPopP(&s);
		} else {
			if ((*RootPtr)->RPtr != NULL) {
				SPushP(&s, (*RootPtr)->RPtr);
			}
			tBTNodePtr tmp2 = (*RootPtr)->LPtr;
            tBTNodePtr tmp = (*RootPtr);
            free(tmp);
            (*RootPtr) = tmp2;
		}
	}
}

/* konec c402.c */

