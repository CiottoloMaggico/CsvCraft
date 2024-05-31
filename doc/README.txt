Documentazione CSVCraft

Struttura del progetto:
    bin/ => contiene l'eseguibile di csvcraft generato dopo la compilazione con il file make
    doc/ => contiene file README
    src/ => contiene i file sorgente .c/.h
    tests/ => contiene alcuni piccoli test fatti da me + uno script bash per eseguirli tutti in modo comodo e veloce,
              (lo script non fa nessun controllo sulla correttezza dell'output)
    makefile => file make da utilizzare per compilare il progetto

Utilizzo:
   Dopo aver compilato l'eseguibile di CSVCraft tramite il file make si può procedere con l'utilizzo del programma.
   Le funzioni da far eseguire al programma possono essere specificate tramite le seguenti flag in linea di comando:

   -m (obbligatorio) permette di specificare quale compito eseguire [1 : compito 1, 2 : compito 2]
   -i (obbligatorio) permette di specificare il nome del file in input
   -o (obbligatorio) permette di specificare il nome del file di output
   -p (obbligatorio) permette di specificare se svolgere il compito selezionato utilizzando
      la versione a singolo processo o a multi processo [1 : singolo processo, 2 : multi processo]

   Flag specifiche per il compito 2:
   --word-number (obbligatorio se -m 2) permette di specificare il numero di parole da generare
   --starting-word (opzionale) permette di specificare la parola con cui iniziare la generazione (se presente nel CSV in input)

Esempi:
    - Esecuzione compito 1 singolo processo: ./csvcraft -m 1 -p 1 -i "prova.txt" -o "prova_out.txt"
    - Esecuzione compito 2 singolo processo: ./csvcraft -m 2 -p 1 -i "prova.txt" -o "prova_out.txt" --word-number 10
    - per altri esempi si può consultare il codice dello script "tests.sh"