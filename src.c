#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#define DIM 256                                                                 //dimensione massima nome_ingredienti e nome_ricette
#define ALF 27                                                                  //dimensioe hash-table preliminari

//abbreviazioni istruzioni
#define AGG "aggiungi_ricetta" 
#define RIM "rimuovi_ricetta"
#define RIF "rifornimento"
#define ORD "ordine"

#define BLACK 'n'
#define RED 'r'

#define TROVATO 0
#define BABBO_SX 1
#define BABBO_DX 2
#define VUOTO 3

int tempo = 0;
int aperto = 1;

//definizioni struct e puntatori a struct
typedef struct LOTTO {
    int qnt;
    int scadenza;
} lotto;
typedef lotto * lista_lotti;

typedef struct REPARTO {
    char * categoria;
    int qnt;
    lista_lotti * scaffale;
    struct REPARTO * sx;
    struct REPARTO * dx;
    struct REPARTO * babbo;
    char colore;
    int celle_lotti;
    int n_lotti;
} reparto;
typedef reparto * lista_reparti;

typedef struct INGREDIENTE {
    lista_reparti reparto;
    int qnt;
    struct INGREDIENTE * next;
} ingrediente;
typedef ingrediente * lista_ingredienti;

typedef struct RICETTA {
    char * nome;
    lista_ingredienti ingredienti;
    struct RICETTA * dx;
    struct RICETTA * sx;
    struct RICETTA * babbo;
    int ultimo_aggiormento;
    int qnt_limite;
    int qnt_ordini;
} ricetta;
typedef ricetta * lista_ricette;

typedef struct ORDINE {
    int peso;
    int data;
    int qnt;
    struct ORDINE * next;
    lista_ricette ricetta;
    struct ORDINE * dx;
    struct ORDINE * sx;
    struct ORDINE * babbo;
} ordine;
typedef ordine * lista_ordini;

//dichiarazioni strutture dati
lista_reparti hash_magazzino[ALF];    
lista_ricette hash_ricettario[ALF];
lista_ordini ordini_sospesi = NULL;
lista_ordini coda_sospesi = NULL;
lista_ordini * ordini_pronti = NULL;
lista_ordini camioncino = NULL;

//altro
int n_pronti = 0;
int celle_pronti = 0;
int flag_cerca_BST;

//definizioni hash-function
int hash_alfabeto(char * nome) {
    int i;
    for (i = 0; i < strlen(nome); i++) {
        if (isalpha(nome[i])) {
            break;
        }
    }
    
    return i < strlen(nome) ? (toascii(tolower(nome[i])) - 97) : (ALF - 1);
}

//definizioni funzioni RB-tree per reparti
void RIGHT_ROTATE(lista_reparti punt_reparto, int hash) {
    lista_reparti y = punt_reparto->sx;
    punt_reparto->sx = y->dx;

    if (y->dx != NULL) {
        y->dx->babbo = punt_reparto;
    }
    
    y->babbo = punt_reparto->babbo;
    if (punt_reparto->babbo == NULL) {
        hash_magazzino[hash] = y;
    } else if(punt_reparto == punt_reparto->babbo->dx) {
        punt_reparto->babbo->dx = y;
    } else {
        punt_reparto->babbo->sx = y;
    }
    
    y->dx = punt_reparto;
    punt_reparto->babbo = y;

    return;
}

void LEFT_ROTATE(lista_reparti punt_reparto, int hash) {
    lista_reparti y = punt_reparto->dx;
    punt_reparto->dx = y->sx;

    if (y->sx != NULL) {
        y->sx->babbo = punt_reparto;
    }
    
    y->babbo = punt_reparto->babbo;
    if (punt_reparto->babbo == NULL) {
        hash_magazzino[hash] = y;
    } else if(punt_reparto == punt_reparto->babbo->sx) {
        punt_reparto->babbo->sx = y;
    } else {
        punt_reparto->babbo->dx = y;
    }
    
    y->sx = punt_reparto;
    punt_reparto->babbo = y;

    return;
}

void RB_INSERT_FIXUP(lista_reparti punt_reparto, int hash) {
    lista_reparti y = NULL;

    while (punt_reparto->babbo != NULL && punt_reparto->babbo->colore == RED) {
        if (punt_reparto->babbo == punt_reparto->babbo->babbo->sx) {
            y = punt_reparto->babbo->babbo->dx;

            if (y == NULL || y->colore == RED) {
                punt_reparto->babbo->colore = BLACK;
                if(y != NULL) y->colore = BLACK;                                //caso in cui lo zio è null
                punt_reparto->babbo->babbo->colore = RED;
                punt_reparto = punt_reparto->babbo->babbo;

            } else {
                if (punt_reparto == punt_reparto->babbo->dx) {
                    punt_reparto = punt_reparto->babbo;
                    LEFT_ROTATE(punt_reparto, hash);

                }
                punt_reparto->babbo->colore = BLACK;
                punt_reparto->babbo->babbo->colore = RED;
                RIGHT_ROTATE(punt_reparto->babbo->babbo, hash); 

            }
            
        } else {
            y = punt_reparto->babbo->babbo->sx;            

            if (y == NULL || y->colore == RED) {
                punt_reparto->babbo->colore = BLACK;
                if(y != NULL) y->colore = BLACK;                                //caso in cui lo zio è null
                punt_reparto->babbo->babbo->colore = RED;
                punt_reparto = punt_reparto->babbo->babbo;

            } else {
                if (punt_reparto == punt_reparto->babbo->sx) {
                    punt_reparto = punt_reparto->babbo;
                    RIGHT_ROTATE(punt_reparto, hash);

                }
                punt_reparto->babbo->colore = BLACK;
                punt_reparto->babbo->babbo->colore = RED;
                LEFT_ROTATE(punt_reparto->babbo->babbo, hash); 

            }
        }
    }
    
    hash_magazzino[hash]->colore = BLACK;
    return;
} 

void RB_INSERT(lista_reparti punt_reparto, int hash) {
    lista_reparti x = hash_magazzino[hash];
    lista_reparti y = NULL;

    if (x == NULL) {
        punt_reparto->sx = NULL;
        punt_reparto->dx = NULL;
        punt_reparto->colore = BLACK;
        punt_reparto->babbo = NULL;
        hash_magazzino[hash] = punt_reparto;
        return;
    }
    
    while (x != NULL) {
        y = x;
        if (strcmp(punt_reparto->categoria, x->categoria) < 0) {
            x = x->sx;
        } else x = x->dx;
    }

    punt_reparto->babbo = y;

    if (y == NULL) {
        hash_magazzino[hash] = punt_reparto;

    } else if (strcmp(punt_reparto->categoria, y->categoria) < 0) {
        y->sx = punt_reparto;

    } else {
        y->dx = punt_reparto;

    }

    punt_reparto->sx = NULL;
    punt_reparto->dx = NULL;
    punt_reparto->colore = RED;

    RB_INSERT_FIXUP(punt_reparto, hash);
    
    return;
}   

lista_reparti cerca_RB(char * categoria, lista_reparti x) {

    int flag;

    if (x == NULL) {return NULL;}

    flag = strcmp(x->categoria, categoria);

    while (flag != 0) {

        if (flag > 0) {
            x = x->sx;
        } else {
            x = x->dx;
        }
        
        if (x == NULL) {return NULL;}
    
        flag = strcmp(x->categoria, categoria);
    }
    
    return x;

    /*              VERSIONE RICORSIVA                  */


    /* 
    if (x == NULL) return NULL;

    if (strcmp(x->categoria, categoria) == 0) return x;
    
    if (strcmp(x->categoria, categoria) > 0) return cerca_RB(categoria, x->sx);

    if (strcmp(x->categoria, categoria) < 0) return cerca_RB(categoria, x->dx);

    return NULL;
    */
}


//definizioni funzioni

void exctract_min_lotti(lista_lotti * punt_scaf, int num_celle, int num_lotti) {

    lista_lotti min = punt_scaf[0];
    int i = 0;

    if (num_lotti == 0) {return;}

    if (num_lotti == 1) {                                                       //caso con solo un lotto
        punt_scaf[0] = NULL;
        free(min);
        return;
    }
    
    if (num_lotti == 2) {                                                       //caso con solo due lotti
        punt_scaf[0] = punt_scaf[1];
        punt_scaf[1] = NULL;
        free(min);
        return;
    }

    punt_scaf[0] = punt_scaf[num_lotti - 1];
    punt_scaf[num_lotti - 1] = NULL;
    free(min);
    
    while (num_celle - 1 >= (2 * i) + 2) {

        if (punt_scaf[(2 * i) + 1] == NULL) {break;}

        if (punt_scaf[(2 * i) + 2] == NULL) {
            if (punt_scaf[i]->scadenza < punt_scaf[(2 * i) + 1]->scadenza) {break;}
            min = punt_scaf[i];
            punt_scaf[i] = punt_scaf[(2 * i) + 1];
            punt_scaf[(2 * i) + 1] = min;
            break;
        }
        
        if (punt_scaf[(2 * i) + 1]->scadenza > punt_scaf[(2 * i) + 2]->scadenza) {              //controllo quale dei due figli sia il più piccolo
            if (punt_scaf[i]->scadenza < punt_scaf[(2 * i) + 2]->scadenza) {break;}             //se padre minore dei figli esco
            min = punt_scaf[i];
            punt_scaf[i] = punt_scaf[(2 * i) + 2];
            punt_scaf[(2 * i) + 2] = min;
            i = (2 * i) + 2;
        } else {
            if (punt_scaf[i]->scadenza < punt_scaf[(2 * i) + 1]->scadenza) {break;}             //se padre minore dei figli esco
            min = punt_scaf[i];
            punt_scaf[i] = punt_scaf[(2 * i) + 1];
            punt_scaf[(2 * i) + 1] = min;
            i = (2 * i) + 1;
        }
        
    }
    
    return;
}

void elimina_scaduti_heap(lista_reparti punt_reparto) {                              //elimina gli elementi scaduti dallo scaffale
    lista_lotti * punt_scaf = punt_reparto->scaffale;

    while (punt_scaf[0] != NULL && punt_scaf[0]->scadenza <= tempo) {
        punt_reparto->qnt -= punt_scaf[0]->qnt;
        exctract_min_lotti(punt_scaf, punt_reparto->celle_lotti, punt_reparto->n_lotti);
        punt_reparto->n_lotti--;
    }

    if (punt_reparto->n_lotti <= (punt_reparto->celle_lotti - 1) / 2) {             //free della memoria eccessiva dello heap
        punt_reparto->scaffale = (lista_lotti *) realloc(punt_reparto->scaffale, ((punt_reparto->celle_lotti - 1) / 2) * sizeof(lista_lotti));
        punt_reparto->celle_lotti = (punt_reparto->celle_lotti - 1) / 2;
    }
    
    return;
}

lista_reparti cerca_reparto_per_ingredienti(char * nome_ingrediente) {          //ritorna il puntatore al reparto cercato o lo crea
    int hash = hash_alfabeto(nome_ingrediente);
    lista_reparti punt_reparto;

    punt_reparto = cerca_RB(nome_ingrediente, hash_magazzino[hash]);

    if (punt_reparto == NULL) {
        punt_reparto = (lista_reparti) malloc(sizeof(reparto));
        punt_reparto->categoria = (char *) malloc(sizeof(char) * strlen(nome_ingrediente) + 1);
        strcpy(punt_reparto->categoria, nome_ingrediente);
        punt_reparto->qnt = 0;
        RB_INSERT(punt_reparto, hash);
        punt_reparto->scaffale = NULL;
        punt_reparto->celle_lotti = 0;
        punt_reparto->n_lotti = 0;
        return punt_reparto;
    } else {
        return punt_reparto;
    }
    
}

lista_ricette cerca_ricetta_BST(char * nome_ricetta, int hash) {
    lista_ricette x = hash_ricettario[hash];
    lista_ricette punt_babbo = NULL;
    int flag;

    if (x == NULL) {
        flag_cerca_BST = VUOTO;
        return NULL;
    }
    

    while (x != NULL) {
        punt_babbo = x;
        flag = strcmp(nome_ricetta, x->nome);

        if (flag == 0) {
            flag_cerca_BST = TROVATO;
            return x;
        }

        if (flag < 0) {
            x = x->sx;
            flag_cerca_BST = BABBO_SX;
        } else {
            x = x->dx;
            flag_cerca_BST = BABBO_DX;
        }

    }   
    
    return punt_babbo;
}

void aggiungi_ingrediente(lista_ricette punt_ricetta, char * nome, int qnt) {   //aggiungo gli ingredienti alla ricetta

    lista_ingredienti punt_ingrediente = (lista_ingredienti) malloc(sizeof(ingrediente));
    punt_ingrediente->qnt = qnt;
    punt_ingrediente->reparto = cerca_reparto_per_ingredienti(nome);
    punt_ingrediente->next = NULL;

    lista_ingredienti x = punt_ricetta->ingredienti;
    lista_ingredienti y = NULL;

    if (punt_ricetta->ingredienti == NULL || punt_ingrediente->qnt >= punt_ricetta->ingredienti->qnt) {      //se è il primo ingrediente
        punt_ingrediente->next = punt_ricetta->ingredienti;
        punt_ricetta->ingredienti = punt_ingrediente;
        return;
    }

    while (x != NULL && punt_ingrediente->qnt < x->qnt) {
        y = x;
        x = x->next;
    }
    
    y->next = punt_ingrediente;
    punt_ingrediente->next = x;
    
    return;
}

void transplant_BST(lista_ricette punt_da_rimuovere, lista_ricette punt_sostituto, int hash) {

    if (punt_da_rimuovere->babbo == NULL) {
        hash_ricettario[hash] = punt_sostituto;
    } else if (punt_da_rimuovere == punt_da_rimuovere->babbo->sx) {
        punt_da_rimuovere->babbo->sx = punt_sostituto;
    } else {
        punt_da_rimuovere->babbo->dx = punt_sostituto;
    }
    
    if (punt_sostituto != NULL) {
        punt_sostituto->babbo = punt_da_rimuovere->babbo;
    }
    
    return;
}

lista_ricette min_BST(lista_ricette punt_ricetta) {
    lista_ricette x = punt_ricetta;

    while (x->sx != NULL) {
        x = x->sx;
    }
    
    return x;
}

void rimuovi_ricetta_BST(lista_ricette punt_ricetta, int hash) {

    lista_ricette y = NULL;
    lista_ingredienti punt_ingrediente;

    if (punt_ricetta->sx == NULL) {
        transplant_BST(punt_ricetta, punt_ricetta->dx, hash);
    } else if (punt_ricetta->dx == NULL) {
        transplant_BST(punt_ricetta, punt_ricetta->sx, hash);
    } else {
        y = min_BST(punt_ricetta->dx);
        if (y != punt_ricetta->dx) {
            transplant_BST(y, y->dx, hash);
            y->dx = punt_ricetta->dx;
            y->dx->babbo = y;
        }
        transplant_BST(punt_ricetta, y, hash);
        y->sx = punt_ricetta->sx;
        y->sx->babbo = y;
        
    }

    while (punt_ricetta->ingredienti != NULL) {                                 //free di tutti gli ingredienti
        punt_ingrediente = punt_ricetta->ingredienti->next;
        free(punt_ricetta->ingredienti);
        punt_ricetta->ingredienti = punt_ingrediente;
    }
    
    free(punt_ricetta->nome);                                                   //free nome della ricetta
    free(punt_ricetta);                                                         //free della ricetta

    return;
}

void fix_inserisci_lotto(lista_lotti * punt_scaf, int num_lotti) {

    int i = num_lotti;
    lista_lotti tmp;

    while (i != 0 && punt_scaf[i]->scadenza < punt_scaf[(i - 1) / 2]->scadenza) {
        tmp = punt_scaf[i];
        punt_scaf[i] = punt_scaf[(i - 1) / 2];
        punt_scaf[(i - 1) / 2] = tmp;
        i = (i - 1) / 2;
    }
    
    return;
}

void inserisci_lotto(lista_lotti punt_lotto, lista_reparti punt_reparto) {

    punt_reparto->qnt += punt_lotto->qnt;

    if (punt_reparto->scaffale != NULL &&
        punt_reparto->scaffale[0] != NULL &&
        punt_reparto->scaffale[0]->scadenza <= tempo) {

        elimina_scaduti_heap(punt_reparto);                                     //sfrutto l'occasione per rimuovere i lotti scaduti
    }

    int num_lotti = punt_reparto->n_lotti;
    int num_celle = punt_reparto->celle_lotti;
    
    if (punt_reparto->scaffale == NULL) {                                       //la prima volta alloco una cella per il lotto
        punt_reparto->scaffale = (lista_lotti *) malloc(sizeof(lotto));
        punt_reparto->scaffale[0] = punt_lotto;
        punt_reparto->celle_lotti++;
        return;
    }

    if (num_celle > num_lotti) {                                                //se ho spazio per il nuovo lotto
        punt_reparto->scaffale[num_lotti] = punt_lotto;
        fix_inserisci_lotto(punt_reparto->scaffale, num_lotti);
        return;
    }

                                                                                //se non ho abbastanza spazio
    punt_reparto->scaffale = (lista_lotti *) realloc(punt_reparto->scaffale, ((num_celle * 2) + 1) * sizeof(lista_lotti));
    for (int i = num_lotti; i < (num_celle * 2) + 1; i++) {
        punt_reparto->scaffale[i] = NULL;
    }
    
    punt_reparto->scaffale[num_lotti] = punt_lotto;
    punt_reparto->celle_lotti = (num_celle * 2) + 1;
    fix_inserisci_lotto(punt_reparto->scaffale, num_lotti);
    return;

}

void gestisci_nuovo_lotto(char * nuovo_lotto, int qnt, int scad) {
    
    int hash = hash_alfabeto(nuovo_lotto);
    lista_reparti punt_reparto;

    lista_lotti punt_lotto = (lista_lotti) malloc(sizeof(lotto));               //creo il nuovo lotto e lo inizializzo
    punt_lotto->scadenza = scad;                                                //
    punt_lotto->qnt = qnt;                                                      //

    punt_reparto = cerca_RB(nuovo_lotto, hash_magazzino[hash]);

    if (punt_reparto == NULL) {
        punt_reparto = (lista_reparti) malloc(sizeof(reparto));                 //non ho reparti, lo creo e inserisco
        punt_reparto->scaffale = NULL;                                          //
        punt_reparto->qnt = 0;                                                  //
        punt_reparto->categoria = (char *) malloc(sizeof(char) * strlen(nuovo_lotto) + 1);                                                 
        strcpy(punt_reparto->categoria, nuovo_lotto);                           //
        punt_reparto->n_lotti = 0;                                              //
        punt_reparto->celle_lotti = 0;                                          //

        RB_INSERT(punt_reparto, hash);

        inserisci_lotto(punt_lotto, punt_reparto);
    } else {                                                                    //se ho un reparto ci inserisco i lotti
        inserisci_lotto(punt_lotto, punt_reparto);                              
    }

    punt_reparto->n_lotti++;

    return;
}

int controlla_ingredienti(lista_ricette punt_ricetta, int qnt_ordine) {         //controlla che ci siano tutti gli ingredienti 
                                                                                //per l'ordine e resituisce il peso totale
    lista_ingredienti punt_ingredienti = punt_ricetta->ingredienti; 
    lista_reparti punt_reparto;
    int flag = 0;
    int qnt_necessaria;
    int peso = 0;

    while (punt_ingredienti != NULL && flag == 0) {                             //per ogni ingrediente della ricetta
        
        punt_reparto = punt_ingredienti->reparto;
        qnt_necessaria = qnt_ordine * punt_ingredienti->qnt;
        peso += qnt_necessaria;

        if (punt_reparto->scaffale != NULL &&
            punt_reparto->scaffale[0] != NULL &&
            punt_reparto->scaffale[0]->scadenza <= tempo) {

            elimina_scaduti_heap(punt_reparto);
        }

        if (punt_reparto->qnt < qnt_necessaria) {
            flag = 1;                                                           //non ho abbastanza ingredienti
        }
        
        punt_ingredienti = punt_ingredienti->next;
    }

    return flag == 0 ? peso : 0;
    
}

void elimina_ingredienti(lista_ricette punt_ricetta, int qnt_ordine) {

    lista_ingredienti punt_ingredienti = punt_ricetta->ingredienti; 
    lista_reparti punt_reparto;
    int qnt_disponibile, qnt_necessaria, diff;
    lista_lotti punt_lotti;

    while (punt_ingredienti != NULL) {                                          //per ogni ingrediente della ricetta
        
        punt_reparto = punt_ingredienti->reparto;
        qnt_disponibile = 0;
        qnt_necessaria = punt_ingredienti->qnt * qnt_ordine; 

        while (qnt_disponibile != qnt_necessaria) {                             //elimini lotti o qnt finche ne ho bisogno

            punt_lotti = punt_reparto->scaffale[0];

            diff = qnt_necessaria - qnt_disponibile;

            if (punt_lotti->qnt > diff) {
                qnt_disponibile = qnt_necessaria;
                punt_lotti->qnt -= diff;
            } else {
                qnt_disponibile += punt_lotti->qnt;
                exctract_min_lotti(punt_reparto->scaffale, punt_reparto->celle_lotti, punt_reparto->n_lotti);
                punt_reparto->n_lotti--;
            }
            
        }        

        if (punt_reparto->n_lotti <= (punt_reparto->celle_lotti - 1) / 2) {     //free della memoria eccessiva dello heap
            punt_reparto->scaffale = (lista_lotti *) realloc(punt_reparto->scaffale, ((punt_reparto->celle_lotti - 1) / 2) * sizeof(lista_lotti));
            punt_reparto->celle_lotti = (punt_reparto->celle_lotti - 1) / 2;
        }

        punt_reparto->qnt -= qnt_necessaria;

        punt_ingredienti = punt_ingredienti->next;                              //passo al prossimo ingrediente
    }

    return;
}

void aggiungi_ordine_sospeso(int peso, lista_ricette punt_ricetta, int qnt_ordine) {
    lista_ordini punt_ordine;                                                   //inizializza struct nuovo ordine
    punt_ordine = (lista_ordini) malloc(sizeof(ordine));
    punt_ordine->peso = peso;
    punt_ordine->data = tempo;
    punt_ordine->qnt = qnt_ordine;
    punt_ordine->ricetta = punt_ricetta;
    punt_ordine->next = NULL;
    punt_ordine->dx = NULL;
    punt_ordine->sx = NULL;

    if (ordini_sospesi == NULL) {                                               //se non ci sono altri ordini sospesi
        ordini_sospesi = punt_ordine;
        coda_sospesi = punt_ordine;
    } else {                                                                    //se ci sono altri ordini sospesi
        coda_sospesi->next = punt_ordine;
        coda_sospesi = punt_ordine;
    }
    
    return;
}

void aggiungi_ordine_pronto(int peso, lista_ricette punt_ricetta, int qnt_ordine) { 
    lista_ordini punt_ordine;

    punt_ordine = (lista_ordini) malloc(sizeof(ordine));                        //inizializza struct nuovo ordine
    punt_ordine->peso = peso;                                                   //
    punt_ordine->data = tempo;                                                  //
    punt_ordine->qnt = qnt_ordine;                                              //
    punt_ordine->ricetta = punt_ricetta;                                        //        
    punt_ordine->next = NULL;                                                   // 
    punt_ordine->dx = NULL;                                                     //
    punt_ordine->sx = NULL;                                                     //

    if (ordini_pronti == NULL) {                                                //se non ci sono altri ordini pronti
        ordini_pronti = (lista_ordini *) malloc(sizeof(lista_ordini));          //creo l'array per lo heap
        ordini_pronti[0] = punt_ordine;
        celle_pronti++;
        n_pronti++;
        return;
    }

    if (celle_pronti > n_pronti) {                                               //se ho spazio per inserire il nuovo ordine
        ordini_pronti[n_pronti] = punt_ordine;
        n_pronti++;
        return;
    } else {                                                                    //se devo allocare nuovo spazio
        ordini_pronti = (lista_ordini *) realloc(ordini_pronti, ((celle_pronti * 2) + 1) * sizeof(lista_ordini));
        for (int i = n_pronti + 1; i < (celle_pronti * 2) + 1; i++) {
            ordini_pronti[i] = NULL;
        }
        
        ordini_pronti[n_pronti] = punt_ordine;
        n_pronti++;
        celle_pronti = (celle_pronti * 2) + 1;
        return;
    }
    
}

void sposta_sospesi_pronti(lista_ordini punt_ordine) {

    punt_ordine->next = NULL;

    if (ordini_pronti == NULL) {                                                //se non ci sono altri ordini pronti
        ordini_pronti = (lista_ordini *) malloc(sizeof(lista_ordini));          //creo l'array per lo heap
        ordini_pronti[0] = punt_ordine;
        celle_pronti++;
        n_pronti++;
        return;
    }

    if (celle_pronti > n_pronti) {                                               //se ho spazio per inserire il nuovo ordine
        ordini_pronti[n_pronti] = punt_ordine;
        n_pronti++;
    } else {                                                                    //se non ho abbastanza spazio
        ordini_pronti = (lista_ordini *) realloc(ordini_pronti, ((celle_pronti * 2) + 1) * sizeof(lista_ordini));      
        for (int i = n_pronti + 1; i < (celle_pronti * 2) + 1; i++) {
            ordini_pronti[i] = NULL;
        }
        
        ordini_pronti[n_pronti] = punt_ordine;
        n_pronti++;
        celle_pronti = (celle_pronti * 2) + 1;
    }
    
    int i = n_pronti - 1;

    while (i > 0 && punt_ordine->data < ordini_pronti[(i - 1) / 2]->data) {     //sistemo il nuovo ordine pronto nel posto giusto
        ordini_pronti[i] = ordini_pronti[(i - 1) / 2];
        ordini_pronti[(i - 1) / 2] = punt_ordine;
        i = (i - 1) / 2;
    }
    
    return;
}

void controlla_ordini_sospesi() {
    lista_ordini x = ordini_sospesi;
    lista_ricette punt_ricetta;
    lista_ordini y = x;
    int peso_ordine = 0;

    while (x != NULL) {
        
        punt_ricetta = x->ricetta;                                              //mi salvo il puntatore alla ricetta dell'ordine da controllare

        if (punt_ricetta->ultimo_aggiormento == tempo && x->qnt >= punt_ricetta->qnt_limite)  {
            y = x;
            x = x->next;
        } else {
            peso_ordine = controlla_ingredienti(x->ricetta, x->qnt);            //controllo se posso fare l'ordine

            if (peso_ordine != 0) {
                x->peso = peso_ordine;
                elimina_ingredienti(x->ricetta, x->qnt);

                if (ordini_sospesi == x) {                                      //se l'ordine si trova in testa
                    ordini_sospesi = x->next;
                    sposta_sospesi_pronti(x);
                    x = ordini_sospesi;
                    y = x;

                } else {                                                        //se l'ordine si trova in mezzo
                    y->next = x->next;  
                    sposta_sospesi_pronti(x);
                    x = y->next;

                }
                
            } else {                                                            //l'ordine non è preparabile, passo al prossimo
                punt_ricetta->ultimo_aggiormento = tempo;
                punt_ricetta->qnt_limite = x->qnt;
                y = x;
                x = x->next;

            }

        }
        
    }

    coda_sospesi = y;                                                           //in tutti i casi y alla fine punterà all'ultimo elem rimasto
    
    return;
}

void inserisci_camioncino(lista_ordini punt_ordine) {

    lista_ordini x = camioncino;
    lista_ordini y = NULL;    
    
    while (x != NULL) {
        y = x;

        if (punt_ordine->peso == x->peso) {                                     //se hanno lo stesso peso

            if (punt_ordine->data < x->data) {
                x = x->sx;
            } else {
                x = x->dx;
            }
            
        } else if (punt_ordine->peso > x->peso) {                               //se il nuovo ha peso maggiore
            x = x->sx;
        } else {                                                                //altrimenti
            x = x->dx;
        }
    }
    
    punt_ordine->babbo = y;
    if (y == NULL) {
        camioncino = punt_ordine;
    } else if (punt_ordine->peso > y->peso) {
        y->sx = punt_ordine;
    } else {
        y->dx = punt_ordine;
    }
    
    return;
}

void exctract_min_pronti() {                                                     //rimuove il minimo dallo heap dei pronti
    lista_ordini tmp;

    ordini_pronti[0] = ordini_pronti[n_pronti - 1];
    ordini_pronti[n_pronti - 1] = NULL;

    int i = 0;

    if (n_pronti - 1 < 3) {                                                         //caso in cui ho solo 1 o 2 ordini rimasti
        if (ordini_pronti[1] != NULL && ordini_pronti[0]->data > ordini_pronti[1]->data) {
            tmp = ordini_pronti[0];
            ordini_pronti[0] = ordini_pronti[1];
            ordini_pronti[1] = tmp;
        }
        
        return;
    }
    

    if (ordini_pronti[0]->data > ordini_pronti[1]->data || ordini_pronti[0]->data > ordini_pronti[2]->data) {
        if (ordini_pronti[1]->data < ordini_pronti[2]->data) {
            tmp = ordini_pronti[0];
            ordini_pronti[0] = ordini_pronti[1];
            ordini_pronti[1] = tmp;
            i = 1;
        } else {
            tmp = ordini_pronti[0];
            ordini_pronti[0] = ordini_pronti[2];
            ordini_pronti[2] = tmp;
            i = 2;
        }
    }

    if (celle_pronti < (2 * i) + 2) {return;}

    while ((ordini_pronti[(2 * i) + 1] != NULL && ordini_pronti[i]->data > ordini_pronti[(2 * i) + 1]->data) ||
            (ordini_pronti[(2 * i) + 2] != NULL && ordini_pronti[i]->data > ordini_pronti[(2 * i) + 2]->data)) {    //sistemo l'heap

        if (ordini_pronti[(2 * i) + 2] == NULL) {
            tmp = ordini_pronti[i];
            ordini_pronti[i] = ordini_pronti[(2 * i) + 1];
            ordini_pronti[(2 * i) + 1] = tmp;
            break;
            
        } else if (ordini_pronti[(2 * i) + 1]->data < ordini_pronti[(2 * i) + 2]->data) {
            tmp = ordini_pronti[i];
            ordini_pronti[i] = ordini_pronti[(2 * i) + 1];
            ordini_pronti[(2 * i) + 1] = tmp;
            i = (2 * i) + 1;

        } else {
            tmp = ordini_pronti[i];
            ordini_pronti[i] = ordini_pronti[(2 * i) + 2];
            ordini_pronti[(2 * i) + 2] = tmp;
            i = (2 * i) + 2;

        }

        if (celle_pronti < (2 * i) + 2) {break;}
        
    }
    
    return;
}

void stampa_camioncino(lista_ordini punt_ordini) {

    lista_ordini punt_dx;

    if (punt_ordini == NULL) return;

    stampa_camioncino(punt_ordini->sx);
    printf("%d %s %d\n", punt_ordini->data, punt_ordini->ricetta->nome, punt_ordini->qnt);
    punt_dx = punt_ordini->dx;
    punt_ordini->ricetta->qnt_ordini--;                                         //decremento numero ordini per ricetta
    free(punt_ordini);                                                          //libero memoria
    stampa_camioncino(punt_dx);

    return;
}

void gestisci_corriere(int max_cap) {
    lista_ordini x;
    lista_ordini punt_ordine;
    int peso_attuale = 0;
    int flag = 1;

    if (ordini_pronti == NULL) {                                                //se non ho ordini pronti
        printf("camioncino vuoto\n");
        return;
    }
    
    while (flag) {                                                              //sposto gli ordini dai pronti al camioncino
        punt_ordine = ordini_pronti[0];

        if (punt_ordine != NULL && peso_attuale + punt_ordine->peso <= max_cap) {
            peso_attuale += punt_ordine->peso;
            exctract_min_pronti();
            n_pronti--;
            inserisci_camioncino(punt_ordine);
        } else {
            flag = 0;
        }
        
    }

    x = camioncino;                                                             
    if (x == NULL) {
        printf("camioncino vuoto\n");
        return;
    }
    
    stampa_camioncino(camioncino);
    camioncino = NULL;

    return;
    
}



int main() {

    //dichiarazione variabili
    char * curr_task;
    char * buffer;
    size_t bufsize = 0;
    int freq, max_cap;
    int qnt_ordine;
    int peso_ordine;
    char * nome_ricetta;
    char * nuovo_lotto;
    int qnt_lotto;
    int scad;
    char * nome_ingrediente;
    int qnt_ingrediente;
    lista_ricette punt_ricetta;
    ssize_t char_letti;
    int hash = 0;
    lista_ricette punt_babbo_ricetta;


    //inizio programma
    if(getline(&buffer, &bufsize, stdin) == -1) {return -1;}
    freq = atoi(strtok(buffer, " "));                                           //salvo le info del corriere
    max_cap = atoi(strtok(NULL, " "));                                          //
    free(buffer);
    buffer = NULL;

    if((char_letti = getline(&buffer, &bufsize, stdin)) == -1) {return -1;}

    if (buffer[char_letti - 1] == '\n') {
        buffer[char_letti - 1] = '\0';
    }
    
    curr_task = strtok(buffer, " ");

    while (aperto) {

/*              **********************************************               */
/*              ******* INIZIO A GESTIRE LE ISTRUZIONI *******               */
/*              **********************************************               */


        if (curr_task[0] == 'a') {                                      //AGGIUNGI RICETTA

            nome_ricetta = strtok(NULL, " ");
            hash = hash_alfabeto(nome_ricetta);

            punt_babbo_ricetta = cerca_ricetta_BST(nome_ricetta, hash);

            if (flag_cerca_BST == TROVATO) {                                    //la ricetta è già presente nel ricettario
                printf("ignorato\n");

            } else {                                                            //la ricetta deve essere aggiunta e inizializzata
                punt_ricetta = (lista_ricette) malloc(sizeof(ricetta));         //
                punt_ricetta->dx = NULL;                                        //
                punt_ricetta->sx = NULL;                                        //
                punt_ricetta->ingredienti = NULL;                               //
                punt_ricetta->nome = (char *) malloc(sizeof(char) * strlen(nome_ricetta) + 1);
                strcpy(punt_ricetta->nome, nome_ricetta);                       //
                punt_ricetta->ultimo_aggiormento = 0;                           //
                punt_ricetta->qnt_ordini = 0;                                   //
                                                                                //
                if (flag_cerca_BST == BABBO_DX) punt_babbo_ricetta->dx = punt_ricetta;
                else if (flag_cerca_BST == BABBO_SX) punt_babbo_ricetta->sx = punt_ricetta;                     
                else hash_ricettario[hash] = punt_ricetta;                      //
                                                                                //
                punt_ricetta->babbo = punt_babbo_ricetta;                       //
                

                nome_ingrediente = strtok(NULL, " ");

                while (nome_ingrediente != NULL) {                                      //aggiungo tutti gli ingredienti
                    qnt_ingrediente = atoi(strtok(NULL, " "));
                    aggiungi_ingrediente(punt_ricetta, nome_ingrediente, qnt_ingrediente);
                    nome_ingrediente = strtok(NULL, " ");
                }
                
                printf("aggiunta\n");                                           
            }

        } else if (curr_task[2] == 'm') {                               //RIMUOVI RICETTA

            nome_ricetta = strtok(NULL, " ");

            hash = hash_alfabeto(nome_ricetta);
            punt_ricetta = cerca_ricetta_BST(nome_ricetta, hash);

            if (flag_cerca_BST == TROVATO) {                            //se ho quella ricetta
                if (punt_ricetta->qnt_ordini > 0) {
                    printf("ordini in sospeso\n");
                } else {
                    rimuovi_ricetta_BST(punt_ricetta, hash);
                    printf("rimossa\n");
                }
            } else {
                printf("non presente\n");
            }

        } else if (curr_task[0] == 'o') {                               //ORDINE    

            nome_ricetta = strtok(NULL, " ");
            qnt_ordine = atoi(strtok(NULL, " "));

            hash = hash_alfabeto(nome_ricetta);
            punt_ricetta = cerca_ricetta_BST(nome_ricetta, hash);

            if (flag_cerca_BST == TROVATO) {                                     //se ho la ricetta
                punt_ricetta->qnt_ordini++;                                     //aumento il contatore del numero di ordini per ricetta
                peso_ordine = controlla_ingredienti(punt_ricetta, qnt_ordine);
                if (peso_ordine != 0) {
                    elimina_ingredienti(punt_ricetta, qnt_ordine);
                    aggiungi_ordine_pronto(peso_ordine, punt_ricetta, qnt_ordine);
                } else {
                    aggiungi_ordine_sospeso(peso_ordine, punt_ricetta, qnt_ordine);
                }

                printf("accettato\n");

            } else {
                printf("rifiutato\n");
            }

            
        } else if (curr_task[2] == 'f') {                               //RIFORNIMENTO

            nuovo_lotto = strtok(NULL, " ");

            while (nuovo_lotto != NULL) {
                qnt_lotto = atoi(strtok(NULL, " "));
                scad = atoi(strtok(NULL, " "));

                if (scad >= tempo) {
                    gestisci_nuovo_lotto(nuovo_lotto, qnt_lotto, scad);
                }
                
                nuovo_lotto = strtok(NULL, " ");
            }

            printf("rifornito\n");
            controlla_ordini_sospesi();

        } else {
            printf("*****\n");
        }

        free(buffer);
        buffer = NULL;

        if ((char_letti = getline(&buffer, &bufsize, stdin)) == -1) {free(buffer); aperto = 0;}                //se ho finito il file
        else {                                                                                  //altrimenti leggo la nuova linea
            if (buffer[char_letti - 1] == '\n') {
                buffer[char_letti - 1] = '\0';
            }
            curr_task = strtok(buffer, " ");
        }                                                                                       
        
        tempo++;

        if (tempo % freq == 0 && tempo != 0) {
            gestisci_corriere(max_cap);
        }

    }    

    return 0;

}