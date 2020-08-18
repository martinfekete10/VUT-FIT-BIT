# IPK projekt 1 : Implementácia HTTP resolveru doménových mien #
#### Martin Fekete | xfeket00@stud.fit.vutbr.cz ###


### **Programovací jazyk**
Projekt je implementovaný v jazyku Python 3 za použitia modulov `re, sys, socket, threading` a `signal`.

### **Spustenie serveru**
Server sa spúšťa pomocou príkazu `make run PORT=no`, kde `no` je číslo portu, na ktorom sa má server spustiť. V samotnom programe je potom kontrolované, či je zadané číslo portu v rozsahu od 0 do 65535.

### **Popis riešenia**
Nakoľko server musí podporovať spracovávanie požiadavkov od viacerých používateľov, základným kameňom programu je nekonečný cyklus, ktorého hlavnou úlohou je prijať nové spojenie a vytvoriť nové vlákno, v ktorom budú požiadavky prijaté týmto spojením spracovávané. Na tvorbu vlákien je využívaný Python modul `threading`.

Po prijatí nového spojenia je v novom vlákne zavolaná funkcia `new_client(clientsocket, addr)`, v ktorej je prijatý požiadavok rozparsovaný. Po rozparsovaní je zistený typ operácie - server podporuje iba operáciu **GET** a **POST**. V prípade inej operácie je odoslaná odpoveď *405 Method Not Allowed*.

### **Operácia GET**
Pri tejto operácii je najskôr skontrolovaná správnosť GET parametru pomocou regulárneho výrazu (pre potreby projektu počítam s tým, že riešenie GET nepodporuje viacnásobné parametre); v prípade nevalidného výrazu je vrátená odpoveď *400 Bad Request*. 

Ďalej je zistený typ odpovede, t.j. **PTR** alebo **A**, ktorý je takisto získavaný pomocou regulárneho výrazu. V prípade typu PTR je na získavanie doménového mena použtá funkcia `gethostbyaddr(ip)` z modulu `socket`. 

Obdobne pri type A je na získanie IP adresy použitá funkcia `gethostbyname(name)` z rovnakého modulu. V prípade, že daná funkcia neuspeje, je vyhodená výnimka a server vráti chybu *404 Not Found*. 

Ak náhodou používateľ požaduje odpoveď typu A a v name zadá IP adresu, je vrátená chyba *400 Bad Request*. Takisto v prípade, ak požaduje odpoveď typu PTR a zadá URL adresu.

### **Operácia POST**
Podobne ako pri operácii GET je naskôr skotrolovaná správnosť vstupnej URL, prípadné vrátenie odpovedajúcej chyby.

Následne je oddelená hlavička požiadavku od tela požiadavku, ktoré je iterované po riadkoch. Najprv je riadok zbavený akýchkoľvek bielych znakov a potom môžu nastať 2 prípady:

1. Ak je požiadavok na riadku napísaný v správnom formáte, je zistená odpoveď podľa požadovaného typu (A alebo PTR). Ak je na validne napísanom riadku odpoveď nájdená, pridá sa na koniec premennej `result`; v opačnom prípade sa riadok preskočí a iteruje sa ďalej.
2. Ak riadok nie je validne zapísaný, t.j. nie vo formáte *požiadavok:typ*, pričom typ je buď A alebo PTR, riadok je preskočený a je nastavená hodnota boolu `bad_request` na `True`

Po dokončení cyklu sa odosiela odpoveď podľa nasledujúcich pravidiel:

1. Ak je premenná `result` prázdna a bool `bad_request` je nastavený na `True`, vráti sa odpoveď s hlavičkou *400 Bad Request*
2. Ak nie je premenná `result` prázdna a bool `bad_request` je nastavený na `False`, vráti sa odpoveď s hlavičkou *404 Not Found*
3. Ak nie je premenná `result` prázdna a bool `bad_request` je nastavený na `False`, vráti sa odpoveď s hlavičkou *200 OK*.

V skratke je POST implemetovaný tak, že ak je nájdená odpoveď pre aspoň jeden požiadavok, odpoveď bude vždy *200 OK*. Ak nie a aspoň jeden riadok bol formálne zle zapísaný, je vrátená odpoveď *400 Bad Request* a inak *404 Not Found*.

### **Ukončovanie behu serveru**
Keďže server má bežať neustále, je nutné ho ukončovať napr. signálom *SIGINT*. Obsluha signálu je vyriešená pomocou signal handleru `handler(signal_received, frame)`, ktorý po prijatí signálu *SIGINT* uzavrie socket a ukončí beh serveru s návratovým kódom 0.