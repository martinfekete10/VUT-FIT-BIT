/*
*   IZP
*   Projekt 3
*   Martin Fekete
*   xfeket00
*   3.12.2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h> // sqrtf
#include <limits.h> // INT_MAX

/*****************************************************************
 * Ladici makra. Vypnout jejich efekt lze definici makra
 * NDEBUG, napr.:
 *   a) pri prekladu argumentem prekladaci -DNDEBUG
 *   b) v souboru (na radek pred #include <assert.h>
 *      #define NDEBUG
 */
#ifdef NDEBUG
#define debug(s)
#define dfmt(s, ...)
#define dint(i)
#define dfloat(f)
#else

// vypise ladici retezec
#define debug(s) printf("- %s\n", s)

// vypise formatovany ladici vystup - pouziti podobne jako printf
#define dfmt(s, ...) printf(" - "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

// vypise ladici informaci o promenne - pouziti dint(identifikator_promenne)
#define dint(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)

// vypise ladici informaci o promenne typu float - pouziti
// dfloat(identifikator_promenne)
#define dfloat(f) printf(" - " __FILE__ ":%u: " #f " = %g\n", __LINE__, f)

#endif

/*****************************************************************
 * Deklarace potrebnych datovych typu:
 *
 * TYTO DEKLARACE NEMENTE
 *
 *   struct obj_t - struktura objektu: identifikator a souradnice
 *   struct cluster_t - shluk objektu:
 *      pocet objektu ve shluku,
 *      kapacita shluku (pocet objektu, pro ktere je rezervovano
 *          misto v poli),
 *      ukazatel na pole shluku.
 */

struct obj_t {
    int id;
    float x;
    float y;
};

struct cluster_t {
    int size;
    int capacity;
    struct obj_t *obj;
};

/*****************************************************************
 * Deklarace potrebnych funkci.
 *
 * PROTOTYPY FUNKCI NEMENTE
 *
 * IMPLEMENTUJTE POUZE FUNKCE NA MISTECH OZNACENYCH 'TODO'
 *
 */

/*
 Inicializace shluku 'c'. Alokuje pamet pro cap objektu (kapacitu).
 Ukazatel NULL u pole objektu znamena kapacitu 0.
*/
void init_cluster(struct cluster_t *c, int cap)
{
    assert(c != NULL);
    assert(cap >= 0);

    // TODO
    // pocet objektov v poli je najprv 0
    c->size = 0;

    // ak je kapacita 0
    if (cap == 0)
    {
        c->obj = NULL; // ukazatel sa nastacy na NULL
        c->capacity = 0; // kapacita sa nastavy na hodnotu 0
        return; // funkcia sa ukonci
    }

    // ak je kapacita ina ako 0
    else
    {
        c->obj = malloc(cap * sizeof(struct obj_t)); // alokacia pamate pre kapacitu objektu
        // kontrola, ci sa alokacia podarila
        if (c->obj == NULL)
        {
            fprintf(stderr, "ERROR: memory alloc unsuccesfull.\n");
            return;
        }
        c->capacity = cap; // nastavenie kapacity clusteru na hodnotu cap
    }
}

/*
 Odstraneni vsech objektu shluku a inicializace na prazdny shluk.
 */
void clear_cluster(struct cluster_t *c)
{
    // TODO
    free(c->obj); // uvolnenie pamate pola objektov
    init_cluster(c, 0); // inicializacia clusteru s kapacitou 0
}

/// Chunk of cluster objects. Value recommended for reallocation.
const int CLUSTER_CHUNK = 10;

/*
 Zmena kapacity shluku 'c' na kapacitu 'new_cap'.
 */
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap)
{
    // TUTO FUNKCI NEMENTE
    assert(c);
    assert(c->capacity >= 0);
    assert(new_cap >= 0);

    if (c->capacity >= new_cap)
        return c;

    size_t size = sizeof(struct obj_t) * new_cap;

    void *arr = realloc(c->obj, size);
    if (arr == NULL)
        return NULL;

    c->obj = (struct obj_t*)arr;
    c->capacity = new_cap;
    return c;
}

/*
 Prida objekt 'obj' na konec shluku 'c'. Rozsiri shluk, pokud se do nej objekt
 nevejde.
 */
void append_cluster(struct cluster_t *c, struct obj_t obj)
{
    // TODO
    struct cluster_t *ptr = NULL;

    // ak je pocet objektov clusteri vacsi/rovny ako kapacie, kapacita sa navysi
    if (c->size >= c->capacity)
        ptr = resize_cluster(c, c->capacity + CLUSTER_CHUNK);

    // kontrola uspesnosti zmeny velkosti clusteru
    if (ptr == NULL && c->size >= c->capacity)
    {
        fprintf(stderr, "ERROR: append unsuccesfull\n");
        return;
    }

    // priadanie objektu do clusteru
    c->obj[c->size++] = obj;
}

/*
 Seradi objekty ve shluku 'c' vzestupne podle jejich identifikacniho cisla.
 */
void sort_cluster(struct cluster_t *c);

/*
 Do shluku 'c1' prida objekty 'c2'. Shluk 'c1' bude v pripade nutnosti rozsiren.
 Objekty ve shluku 'c1' budou serazeny vzestupne podle identifikacniho cisla.
 Shluk 'c2' bude nezmenen.
 */
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c2 != NULL);

    // TODO
    int c2_size = c2->size; // premenna j je potrebna preto, lebo bez nej sa stal cyklus nekonecnym
    // pridanie clusteru c2 na koniec clusteru c1
    for (int i = 0; i < c2_size; i++)
        append_cluster(c1, c2->obj[i]);

    sort_cluster(c1); // usporiadanie clusteru c1
}

/**********************************************************************/
/* Prace s polem shluku */

/*
 Odstrani shluk z pole shluku 'carr'. Pole shluku obsahuje 'narr' polozek
 (shluku). Shluk pro odstraneni se nachazi na indexu 'idx'. Funkce vraci novy
 pocet shluku v poli.
*/
int remove_cluster(struct cluster_t *carr, int narr, int idx)
{
    assert(idx < narr);
    assert(narr > 0);

    // TODO
    clear_cluster(&carr[idx]); // vymazanie zhluku na danom indexe

    // posunutie zvysku pola o 1 dolava
    for (int i = idx; i+1 < narr; i++)
        carr[i] = carr[i+1];

    int nnarr = narr - 1; // nova velkost pola zhlukov
    return nnarr;
}

/*
 Pocita Euklidovskou vzdalenost mezi dvema objekty.
 */
float obj_distance(struct obj_t *o1, struct obj_t *o2)
{
    assert(o1 != NULL);
    assert(o2 != NULL);

    // TODO
    // deklaracia pomocnych premennych - 'odvesny' v pravouhlom trojuholniku
    float x = 0.0;
    float y = 0.0;

    y = fabs(o1->y - o2->y); // vypocet vzdialenosti na y-osi
    x = fabs(o1->x - o2->x); // vypocet vzdialenosti na x-osi

    return sqrtf(x*x + y*y); // vratenie hodnoty pomocou pytag. vety
}

/*
 Pocita vzdalenost dvou shluku.
*/
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c1->size > 0);
    assert(c2 != NULL);
    assert(c2->size > 0);

    // TODO
    float distance = 0.0; // vzdialenost aktualnych clusterov
    float min_distance = INFINITY; // minimalna vzdialenost clusterov

    // zanoreny cyklus na najdenie najmensej vzdialenosti medzi 2 objektami
    for (int i = 0; i < c1->size; i++)
    {
        for (int j = 0; j < c2->size; j++)
        {
            distance = obj_distance(&c1->obj[i], &c2->obj[j]); // vypocet vzdialenosti medzi aktualnymi objektami
            // ak je aktualna vzdialenost medzi 2 clustermi mensia ako min. vzdialenost, min. vzdialenost sa zmeni
            if (distance < min_distance)
                min_distance = distance;
        }
    }
    return min_distance;
}

/*
 Funkce najde dva nejblizsi shluky. V poli shluku 'carr' o velikosti 'narr'
 hleda dva nejblizsi shluky. Nalezene shluky identifikuje jejich indexy v poli
 'carr'. Funkce nalezene shluky (indexy do pole 'carr') uklada do pameti na
 adresu 'c1' resp. 'c2'.
*/
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2)
{
    assert(narr > 0);

    // TODO
    // ak existuje iba jeden zhluk
    if (narr == 1)
    {
        *c1 = *c2;
        *c1 = 0;
        return;
    }

    float distance = 0.0; // povodna vzdialenost nastavena na 0
    float min_distance = INFINITY; // povodna minimalna vzdialenost nastavena na inf.
    // zanoreny cyklus na vypocet najblizsich clusterov
    for (int i = 0; i < narr; i++)
    {
        for (int j = i+1; j < narr; j++)
        {
            distance = cluster_distance(&carr[i], &carr[j]); // vypocet vzdialenosti medzi aktualnymi clustermi
            // ak je vzdialenost medzi 2 clustermi mensia ako minimalna, min. vzdialenost sa zmeni
            if (distance < min_distance)
            {
                min_distance = distance;
                *c1 = i; // ulozenie indexu clusteru 1 na adresu c1
                *c2 = j; // ulozenie indexu clusteru 2 na adresu c2
            }
        }
    }
}

// pomocna funkce pro razeni shluku
static int obj_sort_compar(const void *a, const void *b)
{
    // TUTO FUNKCI NEMENTE
    const struct obj_t *o1 = (const struct obj_t *)a;
    const struct obj_t *o2 = (const struct obj_t *)b;
    if (o1->id < o2->id) return -1;
    if (o1->id > o2->id) return 1;
    return 0;
}

/*
 Razeni objektu ve shluku vzestupne podle jejich identifikatoru.
*/
void sort_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    qsort(c->obj, c->size, sizeof(struct obj_t), &obj_sort_compar);
}

/*
 Tisk shluku 'c' na stdout.
*/
void print_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    for (int i = 0; i < c->size; i++)
    {
        if (i) putchar(' ');
        printf("%d[%g,%g]", c->obj[i].id, c->obj[i].x, c->obj[i].y);
    }
    putchar('\n');
}

/*
 Ze souboru 'filename' nacte objekty. Pro kazdy objekt vytvori shluk a ulozi
 jej do pole shluku. Alokuje prostor pro pole vsech shluku a ukazatel na prvni
 polozku pole (ukazatel na prvni shluk v alokovanem poli) ulozi do pameti,
 kam se odkazuje parametr 'arr'. Funkce vraci pocet nactenych objektu (shluku).
 V pripade nejake chyby uklada do pameti, kam se odkazuje 'arr', hodnotu NULL.
*/
int load_clusters(char *filename, struct cluster_t **arr)
{
    assert(arr != NULL);

    // TODO
    #define LINE_LEN 100

    *arr = NULL; // nastavenie ponteru na NULL
    FILE *obj_file = fopen(filename, "r"); // otvorenie suboru filename pre citanie

    // overenie otvorenia suboru.
    if (obj_file == NULL)
    {
        fprintf(stderr, "ERROR: file was not opened.\n");
        return 0;
    }

    int obj_count = 0; // object count - pocet objektov definovany v prvom riadku
    int line_no = 0; // pocitadlo riadkov v subore
    int line_count = 0;
    int id = 0, x = 0; // ID objektu a suradnica x
    float y = 0.0; // suradnica y je float kvoli kontrole spravneho datoveho typu
    char line[LINE_LEN]; // riadok suboru
    struct cluster_t *cluster = NULL; // cluster
    struct obj_t obj; // objekt

    fgets(line, LINE_LEN, obj_file); // nacitanie prveho riadku suboru
    // nacitanie poctu objektov v subore
    if (sscanf(line, "count=%d", &obj_count) != 1)
    {
        fprintf(stderr, "ERROR: first line of file should be in format 'count=N'.\n\n");
        fclose(obj_file);
        return 0;
    }

    // pocitadlo riadkov potrebne pre opatrenie nizsie
    while (fgets(line, LINE_LEN, obj_file))
        line_count++;

    // ak je v subore menej riadkov, ako je definovanych objektov v prvom riadku, program sa ukonci
    if (line_count < obj_count)
    {
        fprintf(stderr, "ERROR: number of objects should not be > than defined in the first line.\n\n");
        fclose(obj_file);
        return 0;
    }

    // znovunacitanie suboru
    if (fseek(obj_file, 0, SEEK_SET) != 0)
    {
        fprintf(stderr, "ERROR: file rewind not successfull.\n");
        fclose(obj_file);
        return 0;
    }
    fgets(line, LINE_LEN, obj_file); // precitanie prveho riadku naprazdno

    // ak je pocet objektov cislo mensie/rovne 0, program sa ukonci
    if (obj_count <= 0)
    {
        fprintf(stderr, "ERROR: number of objects declared in first line should be > 0.\n\n");
        fclose(obj_file);
        return 0;
    }

    *arr = malloc(obj_count * sizeof(struct cluster_t)); // alokacie pamate pre pole clusterov

    // kontrola, ci sa alokacia podarila
    if (*arr == NULL)
    {
        fprintf(stderr, "ERROR: memory alloc unsuccesfull.\n\n");
        fclose(obj_file);
        return 0;
    }

    // inicializacia objektov pola
    for (int i = 0; i < obj_count; i++)
        init_cluster(&(*arr)[i], 0);

    // cyklus na citanie kazdeho riadku v subore
    while (line_no < obj_count)
    {
        fgets(line, LINE_LEN, obj_file); // citanie kazdeho riadku v subore
        line_no++; // inkrementacia pocitadla riadkov

        // nacitanie koordinatov a id zo suboru a ulozenie do premennych, kontrola ich spravnosti
        if (sscanf(line, "%d %d %f", &id, &x, &y) != 3 || x < 0 || x > 1000 || y < 0 || y > 1000 || y != (int)y)
        {
            fprintf(stderr, "ERROR: each object should:\n"
                            "1. be written in format OBJID X Y\n"
                            "2. have natural coordinates >= 0 and <= 1000\n"
                            "3. have natural coordinates.\n\n");
            fclose(obj_file);
            return -1*line_no;
        }

        // zapisanie jednotlivych udajov do objektu
        obj.x = x;
        obj.y = y;
        obj.id = id;

        cluster = &(*arr)[line_no - 1]; // vytvorenie noveho zhluku v poli zhlukov (-1, lebo sa indexuje od 0)
        append_cluster(cluster, obj); // pridanie objektu do zhluku v poli zhlukov
    }
    // ak je v prvom riadku mensie cislo ako pocet objektov
    if (line_no != line_count)
    {
        fclose(obj_file);
        return line_no;
    }

    fclose(obj_file); // zatvorenie suboru
    return obj_count; // vrati pocet uspesne nacitanych objektov
}

/*
 Tisk pole shluku. Parametr 'carr' je ukazatel na prvni polozku (shluk).
 Tiskne se prvnich 'narr' shluku.
*/
void print_clusters(struct cluster_t *carr, int narr)
{
    printf("Clusters:\n");
    for (int i = 0; i < narr; i++)
    {
        printf("cluster %d: ", i);
        print_cluster(&carr[i]);
    }
}

int main(int argc, char *argv[])
{
    int n = 1; // ak nie je zadany druhy argument, n je 1
    char *endptr = NULL; // pomocny pointer

    // kontrola poctu argumentov
    if (argc < 2 || argc > 3)
    {
        fprintf(stderr, "ERROR: Wrong number of arguments.\n");
        return 1;
    }

    // ak je pocet argumentov 3, cislo n sa zmeni na cislo z argumentu
    if (argc == 3)
    {
        // konverzia argumentu na int
        n = strtod(argv[2], &endptr);
        // kontrola, ci je cislo v argumente zadane spravne
        if (*endptr != '\0' || n <= 0)
        {
            fprintf(stderr, "ERROR: Argument 2 should be natural number > 0.\n");
            return 1;
        }
    }

    // deklaracia clusteru clusters
    struct cluster_t *clusters;

    // nacitanie clusterov zo suboru
    int obj_count = load_clusters(argv[1], &clusters);

    // kontrola unikatnosti ID nacitanych objektov
    for (int i = 0; i < obj_count; i++)
    {
        for (int j = i+1; j < obj_count; j++)
        {
            if (clusters[i].obj->id == clusters[j].obj->id)
            {
                fprintf(stderr, "Each object should have unique ID.\n");
                for (int k = 0; k < obj_count; k++)
                    clear_cluster(&clusters[k]);
                free(clusters);
                return 1;
            }
        }
    }

    // kontrola spravnosti nacitania clusterov zo suboru
    if (obj_count < 0)
    {
        fprintf(stderr, "ERROR: Objects were not loaded from the file.\n");
        // odstranenie vsetkych objektov clustru
        for (int i = 0; i < -1*obj_count; i++)
            clear_cluster(&clusters[i]);
        //uvolnenie pamate zabranej clusterom clusters
        free(clusters);
        return 1;
    }

    // kontrola spravnosti nacitania clusterov zo suboru
    if (obj_count == 0)
    {
        fprintf(stderr, "ERROR: Objects were not loaded from the file.\n");
        return 1;
    }

    // ak je pozadovany pocet clusterov vacsi, ako pocet objektov v subore
    if (n > obj_count)
    {
        fprintf(stderr, "ERROR: number in argument should be <= number of objects loaded from the file.\n");
        // odstranenie vsetkych objektov clustru
        for (int i = 0; i < obj_count; i++)
            clear_cluster(&clusters[i]);
        //uvolnenie pamate zabranej clusterom clusters
        free(clusters);
        return 1;
    }

    // vytvaranie zhlukov kym pozadovany pocet clusterov nie je rovny poctu objektov
    int j;
    int k;
    while (n < obj_count)
    {
        find_neighbours(clusters, obj_count, &j, &k);
        merge_clusters(&clusters[j], &clusters[k]);
        obj_count = remove_cluster(clusters, obj_count, k);
    }

    // print clusterov na obrazovku
    print_clusters(clusters, n);

    // odstranenie vsetkych objektov clustru
    for (int i = 0; i < obj_count; i++)
        clear_cluster(&clusters[i]);

    //uvolnenie pamate zabranej clusterom clusters
    free(clusters);

   return 0;
}
