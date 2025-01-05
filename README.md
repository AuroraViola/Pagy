# Pagy

Pagy è un programma che ti permette di formattare un file di testo in un file in stile "pagina di giornale".

## Funzionamento e struttura del codice

### Funzionamento

Il programma prende in input un testo codificato in UTF-8 e produrrà un testo formattato in stile "pagina di giornale" codificato sempre in UTF-8.

Formato dell'output:
- Il testo sarà diviso in colonne su ogni pagina
- Ogni riga è giustificata (tranne quelle di fine paragrafo)
- Ogni parola su una riga ha almeno uno spazio tra le altre parole
- Ogni riga di fine paragrafo è allineata a sinistra
- Ogni paragrafo è separato da una linea vuota
- Ogni pagina è separata da "\n %%%\n"
- Le parole troppo lunghe per entrare nella riga vengono divise su una o più righe
- Non viene gestita la sillabazione

Il programma restituirà:
- -2 in caso di fallimento
- 0 in caso di successo

### Struttura

I file del codice sono all'interno della directory `src` e sono strutturati nel seguente modo:
```
src
├── includes
│   ├── file_reader.h
│   ├── formatting.h
│   └── output.h
├── modules
│   ├── file_reader.c
│   ├── formatting.c
│   └── output.c
└── main.c
```

Nella directory `includes` sono presenti i rispettivi header dei file .c nella directory `modules`.

È stata scelta questa struttura per rendere semplice l'organizzazione dei file nel progetto.

`file_reader.c` e `file_reader.h` contiene la struttura e le funzioni per leggere l'input da file o `stdin`.

`formatting.c` e `formatting.h` contiene le strutture che definiscono il come è fatta la pagina e le funzioni per formattare la pagina.

`output.c` e `output.h` contengono le funzioni per scrivere le pagine su `stdout` o su file.

Nel file `main.c` invece vengono gestiti gli argomenti e le opzioni e infine eseguita la parte importante del programma.

#### Versione mono-processo

La versione mono-processo del programma è strutturata nel seguente modo:
1. Viene salvato l'input in una struttura apposita
2. Vengono formattate le pagine riga per riga e parola per parola in base al loro formato
3. Vengono stampate le pagine in base al loro formato
4. Viene terminata l'esecuzione

#### Versione multi-processo

La versione multi-processo del programma è strutturata nel seguente modo:
1. Il processo padre genera il processo `file_parser`
2. Il padre comunica con il processo `file_parser` per ottenere l'input
3. il processo `file_parser` termina
4. Vengono creati due processi "produttore" e "consumatore"
5. Il padre attende che i due processi terminino
6. Viene terminata l'esecuzione

Il processo "produttore" si occupa di formattare la pagina e mandarla al consumatore

Il processo "consumatore" si occupa di ricevere la pagina prodotta e stamparla su file o `stdout`

## Compilazione

Per compilare il progetto basta un semplice make.

```shell
make
```

Una volta compilato verranno generati gli eseguibili `pagy-mono` e `pagy-multi` nella cartella `build`.

`pagy-mono` è la versione mono-processo del programma e `pagy-multi` è la versione multi-processo.

## Uso

Il programma, se usato senza nessuna opzione o argomento, prenderà l'input da `stdin` e scriverà l'output su `stdout`. Eventuali errori saranno stampati su `stderr`.

Si può specificare il file di input specificando la path negli argomenti

Se non specificato nelle opzioni, verranno usati i seguenti parametri per la formattazione della pagina:
- 3 Colonne per pagina
- 32 Righe per colonna
- 25 Caratteri per riga
- 8 Spazi tra una colonna e l'altra

Possono essere utilizzate le seguenti opzioni:

| Opzione | Opzione lunga | Argomenti richiesti | Descrizione                                          |
|---------|---------------|---------------------|------------------------------------------------------|
| `-c`    | `--columns`   | intero > 0          | Indica quante colonne stampare per pagina            |
| `-h`    | `--help`      | -                   | Stampa una schermata di aiuto ed esce                |
| `-l`    | `--length`    | intero > 0          | Indica il numero di caratteri per riga               |
| `-o`    | `--output`    | stringa             | Indica la path dove salvare l'output del programma   |
| `-r`    | `--rows`      | intero > 0          | Indica il numero di righe che deve avere una colonna |
| `-s`    | `--spaces`    | intero >= 0         | Indica il numero di spazi tra una colonna e l'altra  |
| `-v`    | `--version`   | -                   | Stampa informazioni sulla versione ed esce           |

## Esempi

**Nota:** Negli esempi verrà usato `pagy`, che può essere inteso come un alias per `pagy-mono` o `pagy-multi`

### Input

Il programma accetta input da stdin in caso non venga specificata la path file di input.

```shell
cat "example.txt" | pagy
```
```shell
pagy "example.txt"
```
Per tanto i seguenti due comandi daranno il seguente output:
```
Lorem    Ipsum è un testo        recentemente  da software
segnaposto     utilizzato        di   impaginazione   come
nel     settore     della        Aldus    PageMaker,   che
tipografia     e    della        includeva   versioni  del
stampa.                          Lorem Ipsum.             
                                                          
Lorem       Ipsum       è
considerato    il   testo
segnaposto  standard  sin
dal   sedicesimo  secolo,
quando     un     anonimo
tipografo    prese    una
cassetta   di caratteri e
li      assemblò      per
preparare     un    testo
campione.                
                         
È  sopravvissuto non solo
a   più di cinque secoli,
ma   anche  al  passaggio
alla  videoimpaginazione,
pervenendoci             
sostanzialmente          
inalterato.              
                         
Fu   reso popolare, negli
anni    ’60,    con    la
diffusione   dei fogli di
caratteri    trasferibili
“Letraset”,           che
contenevano  passaggi del
Lorem    Ipsum,   e   più
```

### Output

può essere usata l'opzione `-o` per specificare dove scrivere l'output:

```shell
pagy example.txt -o example-o.txt
```
Il seguente comando scriverà l'output sul file "example-o.txt" anziché su `stdout`

### Opzioni varie

Possono essere usate diverse opzioni per scegliere come stampare la pagina.

Per formattare le pagine nel seguente modo:
- 4 Colonne per pagina
- 10 Righe per colonna
- 15 Caratteri per riga
- 6 Spazi tra una colonna e l'altra

Si può usare il seguente comando:
```shell
pagy example.txt -c 4 -r 10 -l 15 -s 6
```
Verrà prodotto il seguente output:
```
Lorem  Ipsum  è      testo                assemblò    per      anche        al
un        testo      segnaposto           preparare    un      passaggio  alla
segnaposto           standard    sin      testo                videoimpaginazi
utilizzato  nel      dal  sedicesimo      campione.            one,           
settore   della      secolo,  quando                           pervenendoci   
tipografia    e      un      anonimo      È                    sostanzialmente
della stampa.        tipografo            sopravvissuto        inalterato.    
                     prese       una      non  solo a più                     
Lorem  Ipsum  è      cassetta     di      di       cinque      Fu         reso
considerato  il      caratteri  e li      secoli,      ma      popolare,      

 %%%
negli      anni      Lorem  Ipsum, e
’60,   con   la      più            
diffusione  dei      recentemente   
fogli        di      da  software di
caratteri            impaginazione  
trasferibili         come      Aldus
“Letraset”,          PageMaker,  che
che                  includeva      
contenevano          versioni    del
passaggi    del      Lorem Ipsum.
```

# Licenza

Questo software è sotto la licenza [GNU General Public License v3.0](https://github.com/AuroraViola/Pagy/blob/main/LICENSE.md)