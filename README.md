# Simulazione di Gestione Ordini per Pasticceria Industriale

## Italiano [ENGLISH follows]

### Overview
Questo progetto implementa, in linguaggio C, una simulazione a tempo discreto del sistema di gestione degli ordini di una pasticceria industriale. La simulazione supporta:
- L’aggiunta e la rimozione dinamica di ricette.
- Il rifornimento periodico di lotti di ingredienti, ciascuno con quantità e data di scadenza.
- L’accettazione e la gestione di ordini (preparazione immediata o messa in attesa, se gli ingredienti non sono sufficienti).
- Il ritiro periodico degli ordini pronti da parte di un corriere, rispettando capacità di carico e algoritmo di selezione/ordinamento.
- **Questo progetto è stato realizzato come parte del corso di Algoritmi e Principi dell’Informatica, con valutazione finale 30/30 con lode.**

### Specifica
1. **Tempo di simulazione**  
   - Discreto: ogni comando corrisponde a un istante di tempo incrementale (si parte da t=0).

2. **Ricette**  
   - Identificate da un nome (alfabeto `[a–zA–Z_]`), ciascuna con quantità intere di ingredienti in grammi.  
   - Comandi:
     - `aggiungi_ricetta <nome> <ingrediente> <qnt> …` → `aggiunta` o `ignorato`
     - `rimuovi_ricetta <nome>` → `rimossa`, `ordini in sospeso` o `non presente`

3. **Magazzino Ingredienti**  
   - Ogni ingrediente ha un insieme di lotti, caratterizzati da quantità e tempo di scadenza.  
   - Comando:
     - `rifornimento <ingrediente> <qnt> <scadenza> …` → `rifornito`  
   - Prelievo sempre dal lotto con scadenza più prossima.

4. **Ordini Clienti**  
   - Comando: `ordine <nome_ricetta> <n_elementi>` → `accettato` o `rifiutato`  
   - Preparazione immediata se ingredienti disponibili, altrimenti messa in attesa (FIFO).

5. **Ritiro Corriere**  
   - Frequenza e capacità (in grammi) definite all’avvio.  
   - Ad ogni arrivo del corriere (al tempo `k⋅periodicità`):
     1. Seleziona tutti gli ordini pronti in arrivo cronologico finché il peso non supera la capacità residua.  
     2. Ordina quelli scelti per peso decrescente (tie-breaker: ordine di arrivo) e li stampa.  
     3. Se il camion è vuoto, stampa `camioncino vuoto`.

### Strumenti Utilizzati
- **Linguaggio**: C  
- **Compilatore**: GCC (C11)  
- **Debugging**: GDB  
- **Analisi Memoria**: Valgrind (per individuare memory leak)  
- **Librerie Standard**: `<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<ctype.h>`, `<assert.h>`  
- **Parsing Input**: `getline()`, `strtok()`  
- **Version Control**: Git / GitHub

### Implementazione
- **Tabelle Hash + RB-Tree**  
  - Uso di un array di 27 puntatori (una per ogni lettera + “altro”) come tavola hash preliminare.  
  - Ogni bucket contiene la radice di un Red-Black Tree per:
    - Il **ricettario** (ricette ordinate per nome).  
    - Il **magazzino** (reparti di ingredienti, ciascuno con un array di lotti).

- **Strutture per i lotti**  
  - Ogni ingrediente mantiene un vettore di strutture `lotto` (quantità e scadenza).  
  - Lotti prelevati sempre in ordine di scadenza crescente.

- **Gestione Ordini**  
  - Liste collegate (`lista_ordini`) per:
    - Ordini sospesi (FIFO).  
    - Ordini pronti in attesa di caricamento.  
  - Dopo ogni rifornimento, si tenta di sbloccare gli ordini sospesi nell’ordine di arrivo.

- **Ordinamento per il Caricamento**  
  - Dopo la selezione in base alla capacità, gli ordini vengono ordinati con `qsort()`:
    1. Peso decrescente (somma dei grammi di ingredienti).  
    2. Tempo di arrivo crescente in caso di pareggio di peso.

- **Controlli di Coerenza**  
  - Uso di `assert()` per verifiche interne.  
  - Gestione accurata di `malloc()`/`free()` per evitare perdite di memoria.

---

## English

### Overview
This project implements, in C, a discrete-time simulation of an industrial pastry shop’s order management system. It supports:
- Dynamic addition and removal of recipes.
- Periodic restocking of ingredient lots, each with quantity and expiration time.
- Order acceptance and handling (immediate preparation or queuing if ingredients are insufficient).
- Periodic pickup of ready orders by a courier, respecting load capacity and selection/sorting algorithm.
- **This project was completed as part of the Algorithms and Principles of Computer Science course, with a final grade of 30/30 with honors.**

### Specification
1. **Simulation Time**  
   - Discrete: each command corresponds to an incremental time step (starting at t=0).

2. **Recipes**  
   - Identified by a name (`[a–zA–Z_]`), with integer ingredient quantities (grams).  
   - Commands:
     - `aggiungi_ricetta <name> <ingredient> <qty> …` → `aggiunta` or `ignorato`
     - `rimuovi_ricetta <name>` → `rimossa`, `ordini in sospeso` or `non presente`

3. **Ingredient Warehouse**  
   - Each ingredient has multiple lots (quantity + expiration time).  
   - Command:
     - `rifornimento <ingredient> <qty> <expiry> …` → `rifornito`  
   - Always withdraw from the soonest-to-expire lot.

4. **Customer Orders**  
   - Command: `ordine <recipe_name> <n_items>` → `accettato` or `rifiutato`  
   - Prepare immediately if possible; otherwise queue (FIFO).

5. **Courier Pickup**  
   - Frequency and capacity (grams) set at startup.  
   - At each courier arrival (`k × periodicity`):
     1. Select ready orders in arrival order until capacity is reached.  
     2. Sort selected orders by descending weight (tie-breaker: arrival time) and print them.  
     3. If none, print `camioncino vuoto`.

### Tools Used
- **Language**: C  
- **Compiler**: GCC (C11)  
- **Debugger**: GDB  
- **Memory Analysis**: Valgrind (memory-leak detection)  
- **Standard Libraries**: `<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<ctype.h>`, `<assert.h>`  
- **Input Parsing**: `getline()`, `strtok()`  
- **Version Control**: Git / GitHub

### Implementation
- **Hash Table + RB-Tree**  
  - An array of 27 pointers (one per letter + “other”) serves as a preliminary hash table.  
  - Each bucket points to a Red-Black Tree for:
    - The **recipe catalog** (recipes sorted by name).  
    - The **warehouse** (ingredient sections, each with its lots array).

- **Lots Management**  
  - Ingredients store an array of `lotto` structs (quantity & expiry).  
  - Withdrawal always from the lot with the earliest expiry.

- **Order Handling**  
  - Linked lists (`lista_ordini`) for:
    - Suspended orders (FIFO).  
    - Ready orders awaiting shipment.  
  - After each restocking, attempts to unlock suspended orders in arrival order.

- **Loading Sort**  
  - After capacity-based selection, orders are sorted with `qsort()`:
    1. By descending weight (sum of ingredient grams).  
    2. By ascending arrival time on ties.

- **Consistency Checks**  
  - `assert()` calls for internal invariants.  
  - Careful `malloc()`/`free()` management to avoid leaks.