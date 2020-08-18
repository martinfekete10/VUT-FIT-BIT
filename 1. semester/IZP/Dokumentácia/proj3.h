/**
  * Dokumentacia k Projektu 3 - jednoducha zhlukova analyza.
  * @file proj3.h
  * @author Martin Fekete, xfeket00@stud.fit.vutbr.cz
  * @date 10.12.2018
  * @warning Program sa ukonci predcasne v pripade zleho pouzitia
  */

/**
  * Struktura reprezentujuca objekt s urcitym id a suradnicami x a y.
  */
struct obj_t {
    /** id objektu */
    int id;
    /** suradnica x */
    float x;
    /** suradnica y */
    float y;
};

/**
  * Struktura reprezentujuca zhluk objektov.
  */
struct cluster_t {
    /** pocet zhlukov v objekte */
    int size;
    /** kapacita zhluku */
    int capacity;
    /** pole objektov patriacich danemu zhluku */
    struct obj_t *obj;
};

/**
  * @defgroup clusters Funkcie pre pracu so zhlukmi
  * @{
  */

/**
  * Inicializacia zhluku c, alokuje sa pamat pre kapacitu objektov.
  * Ukazovatel NULL pri poli objektov znamena kapacitu 0.
  *
  * @pre
  * Kapacita je vacsia/rovna 0.
  *
  * @pre
  * Ukazovatel na zhluk nema hodnotu NULL.
  *
  * @post
  * Zhluk c bude mat alokovanu pamat pre kapacitu objektov,
  * ak nenastane chyba pri alokacii.
  *
  * @param c zhluk ktory inicializujeme
  * @param cap kapacita zhluku
  */
void init_cluster(struct cluster_t *c, int cap);

/**
  * Odstranenie vsetkych objektov zhluku a inicializacia na prazdny zluk.
  *
  * @post
  * Pamat alokovana pre objekty zhluku bude uvolnena.
  *
  * @param c zhluk ktoreho objekty budu uvolnovane
  */
void clear_cluster(struct cluster_t *c);

/// Konstanta o ktoru sa navysuje kapacita zhluku.
extern const int CLUSTER_CHUNK;

/**
  * Zmena kapacitu zhluku 'c' na novu kapacitu.
  *
  * @pre
  * Kapacita zhluku je vacsia/rovna 0.
  *
  * @pre
  * Kapacita new_cap je vacsia/rovna 0.
  *
  * @post
  * Alokuje sa pamat pre novu kapacitu zhluku.
  *
  * @param c zhluk, ktoreho kapacita sa meni
  * @param new_cap hodnota, na ktoru sa zmeni kapacita zhluku
  *
  * @return ukazovatel na zhluk so zmenenou kapacitou, NULL v pripade chyby.
  */
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap);

/**
  * Prida objekt 'obj' na koniec zhluku 'c'.
  * Rozsiri zhluk, ak sa donho objekt nevojde.
  *
  * @post
  * Prida objekt 'obj' na poslednu poziciu v zhluku 'c',
  * ak nenastane chyba pri alokacii pamate.
  *
  * @param c zhluk, do ktoreho pridavame objekt
  * @param obj objekt, ktory je pridavany do zhluku
  */
void append_cluster(struct cluster_t *c, struct obj_t obj);

/**
  * Do zhluku 'c1' prida objekty 'c2'. Zhluk 'c1' bude v pripade nutnosti
  * rozsireny. Objekty v zhluku 'c1' budu zoradene vzostupne podla
  * identifikacneho cisla. Zhluk 'c2' bude nezmeneny.
  *
  * @pre
  * Ukazovatel na zhluk 'c1' nema hodnotu NULL.
  *
  * @pre
  * Ukazovatel na zhluk 'c2' nema hodnotu NULL.
  *
  * @post
  * Zhluk 'c1' bude rozsireny o objekty zhluku 'c2'.
  *
  * @post
  * Objekty zhluku 'c1' budu zoradene vzostupne podla identifikacneho cisla.
  *
  * @param c1 zhluk 'c1', do ktoreho sa pridavaju objekty zhluku 'c2'
  * @param c2 zhluk 'c2', ktoreho objekty su pridavane do zhluku 'c1'
  */
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2);

/**
  * @}
  */

/**
  * @defgroup arrayOfClusters Funkcie pre pracu s polom zhlukov
  * @{
  */

/**
  * Odstrani zhluk z pola zhlukov 'carr'.
  * Pole zhlukov obsahuje 'narr' poloziek (zhlukov). Zhluk na odstranenie
  * sa nachadza na indexe 'idx'. Funkcia vracia novy pocet zhlukov v poli.
  *
  * @pre
  * Hodnota 'idx' bude mensia ako pocet prvkov v poli.
  *
  * @pre
  * Pocet prvkov v poli je vacsi ako 0.
  *
  * @post
  * Z pola zhlukov bude odstraneny zhluk na indexe 'idx', velkost
  * pola sa zmensi o 1.
  *
  * @param carr pole zhlukov
  * @param narr pocet zhlukov v poli
  * @param idx index zhluku, ktory je odstranovany
  *
  * @return novy pocet zhlukov v poli
  */
int remove_cluster(struct cluster_t *carr, int narr, int idx);

/**
  * Funkcia pocita Euklidovskou vzdialenost medzi 2 objektami.
  *
  * @pre
  * Ukazovatel na objekt 1 nie je NULL.
  *
  * @pre
  * Ukazovatel na objekt 2 nie je NULL.
  *
  * @param o1 objekt 1
  * @param o2 objekt 2
  *
  * @return vzdialenost medzi objektami 'o1' a 'o2'
  */
float obj_distance(struct obj_t *o1, struct obj_t *o2);

/**
  * Funkcia pocita vzdialenost medzi zhlukmi 'c1' a 'c2'.
  *
  * @pre
  * Ukazovatel na zhluk 1 nie je NULL a jeho velkost je vacsia ako 0.
  *
  * @pre
  * Ukazovatel na zhluk 2 nie je NULL a jeho velkost je vacsia ako 0.
  *
  * @param c1 zhluk 1
  * @param c2 zhluk 2
  *
  * @return najmensia vzdialenost medzi dvomi zhlukmi
  */
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2);

/**
  * Funkcia najde 2 najblizsie zhluky v poli zhlukov 'carr' velkosti 'narr'.
  * Funkcia najdene zhluky (resp. ich indexy v poli 'carr') uklada do pamate
  * na adresu 'c1' resp. 'c2'.
  *
  * @pre
  * Pole zhlukov 'carr' ma velkost vacsiu ako 0.
  *
  * @post
  * Indexy 2 najblizsich zhlukov budu ulozene na adresu 'c1' a 'c2'.
  *
  * @param carr pole zhlukov
  * @param narr pocet zhlukov v poli
  * @param c1 index jedenho z najblizsich zhlukov
  * @param c2 index druheho z najblizsich zhlukov
  */
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2);

/**
  * Funkcia zoradi objekty zhluku podla ich identifikacneho cisla.
  *
  * @post
  * Objekty zhluku budu zoradene vzostupne podla ich identifikacneho cisla.
  *
  * @param c zhluk, ktoreho objekty budu zoradovane
  */
void sort_cluster(struct cluster_t *c);

/**
  * Funkcia zobrazi zhluk 'c' na stdout.
  *
  * @post
  * Objekty zhluku 'c' budu zobrazene na stdout.
  *
  * @param c zhluk, ktoreho objekty budu zobrazovane.
  */
void print_cluster(struct cluster_t *c);

/**
  * Zo suboru 'filename' nacita objekty. Pre kazdy objekt vytvori zhluk
  * a ulozi ho do pola zhlukov. Alokuje priestor pre pole vsetkych zhlukov
  * a ukazovatel na prvu polozku pola (ukazovatel na prvy zhluk
  * v alokovanom poli) ulozi do pamate, kam sa odkazuje parameter 'arr'.
  * Funkcia vracia pocet nacitanych obkektov (zhlukov). V pripade nejakej
  * chyby uklada do pamate, kam sa odkazuje 'arr' hodnotu NULL.
  *
  * @pre
  * Existuje subor 'filname'.
  *
  * @pre
  * Data v subore su napisane spravym formatom, tj prvy riadok vo formate
  * 'count=N' a objekty pisane formatom'id x y'.
  *
  * @pre
  * Pocet objektov deklarovany v prvom riadku je mensi/rovny skutocnemu
  * poctu objektov zapisanych v subore.
  *
  * @post
  * Pre kazdy objekt v subore bude vytvoreny zhluk, ktory sa
  * ulozi do pola zhlukov 'arr', ktore bude mat alokovanu pamat
  * pre pocet zhlukov, resp. objektov uvedenych v prvom riadku suboru.
  *
  * @param filename nazov suboru s objektami
  * @param arr ukazovatel na pole zhlukov nacitanych zo suboru
  *
  * @return pocet uspesne nacitanych zhlukov (0 v pripade chyby,
  * zaporny pocet zhlukov v pripade zleho formatu objektu)
  */
int load_clusters(char *filename, struct cluster_t **arr);

/**
  * Funkcia zobrazi pole zhlukov na stdout.
  *
  * @post
  * Pole zhlukov bude vypisane na stdout.
  *
  * @param carr ukazovatel na prvu polozku (zhluk)
  * @param narr pocet zhlukov, ktory sa bude zobrazovat
  */
void print_clusters(struct cluster_t *carr, int narr);

/**
  * @}
  */
