#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct{
    char *input;
    char *output;
    char tipo_sensore;
}params;

typedef struct {
    int id_sensore;
    long int timestamp;
    double valore;
    char tipo;
    int anomalia;
} Rilevazione;


typedef struct Node{
    Rilevazione scan;
    struct Node *nextPtr;
} Node;


Node* ReadDataAndCreateList (const char* filename){
    FILE *fPtr = fopen(filename, "r");
    if (fPtr == NULL){
        fprintf(stderr, "File %s could not be opened", filename);
        exit(EXIT_FAILURE);
    }

    char riga[256];
    Node *Head = NULL;
    while (fgets(riga, sizeof(riga), fPtr)){
        riga[strcspn(riga, "\n")];
        Node* newNode = (Node*) malloc(sizeof(Node));
        if (newNode == NULL) {
            fprintf(stderr,"Errore nella memoria!");
            exit(EXIT_FAILURE);
        }

        sscanf(riga,"%d\t%ld\t%lf\t%c", &newNode -> scan.id_sensore, &newNode -> scan.timestamp, &newNode -> scan.valore, &newNode -> scan.tipo);

        newNode -> scan.anomalia = 0;

        newNode -> nextPtr = Head;
        Head = newNode;
    }
    fclose(fPtr);
    return Head;

}

params decodeParameters(int argc, char *argv[]){
    if (argc != 4){ 
    fprintf(stderr,"You should insert four arguments"); // controlling if the arguments are 3 
    exit(1);
    }

    params parametri;
    parametri.input = argv[1];
    parametri.output = argv[2];
    parametri.tipo_sensore = *argv[3];
    char punto = '.';

    char *sPtr = strrchr(argv[1], punto);
    if (sPtr == NULL){
        fprintf(stderr, "File %s has an anomaly", argv[1]);
        exit(EXIT_FAILURE);
    }
    if (strcmp(sPtr, ".tsv") != 0){
        fprintf(stderr, "File %s needs to have extension a .tsv extension", argv[1]);
        exit(EXIT_FAILURE);
    }
    
    char *cPtr = strrchr(argv[2], punto);
    if (cPtr == NULL){
        fprintf(stderr, "File %s has an anomaly", argv[2]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(cPtr, ".txt")!= 0){
        fprintf(stderr, "File %s needs to have a .txt extension", argv[2]);
        exit(EXIT_FAILURE);
    }
    char sensore = *argv[3];

    if (sensore != 'U' && sensore != 'T' && sensore != 'P'){
        fprintf(stderr, "Il sensore deve essere in stato 'T', 'U', 'P'");
        exit(EXIT_FAILURE);
    }
    parametri.tipo_sensore = sensore;
   
    return parametri;
    

}

void PrintList(Node* Head, int show_anomaly){
    Node* current = Head;
    
    while (current != NULL ){
        printf("[ID: %d,\tTS: %ld,\tVALORE: %.1f,\tTIPO: %c]",
        current->scan.id_sensore, current->scan.timestamp, current->scan.valore, current->scan.tipo);
        if (show_anomaly == 1 && current->scan.anomalia == 1){
            printf("|**ANOMALIA**|");
        }
        printf("->\n");
        current=current->nextPtr;

    }
    printf("NULL");

    }

void FreeList(Node* Head) {
    Node* temp;
    while (Head != NULL) {
        temp = Head;
        Head = Head->nextPtr;
        free(temp);
    }
}


double CalculateAverage (Node* Head, char tipo_sensore){
    Node* current = Head;
    int count = 0;
    double sum = 0.0;
    while (current != NULL){
        if (current->scan.tipo == tipo_sensore){
            sum += current->scan.valore;
            count++;
        }
        current = current->nextPtr;    
    }
    return (count > 0) ? (sum / count) : (0.0);
}

void markAnomalies(Node* Head, double media, char tipo_sensore){
    Node* current = Head;
    double limite = media * 1.5;
    while (current != NULL){
        
        if (current->scan.tipo == tipo_sensore && current->scan.valore > limite){
            current->scan.anomalia = 1;
        }

        else{
        current->scan.anomalia = 0;
        }
        current = current -> nextPtr;
    }
}


void WriteAnomaliesToFile(Node* Head, char* filename){
    FILE* file = NULL;
    file = fopen(filename, "w");
    if (file == NULL){
        fprintf(stderr, "Errore nella memoria!");
        exit(EXIT_FAILURE);
    }
    Node* current = Head;
    while (current != NULL){
        if(current->scan.anomalia == 1){
            fprintf(file, "%d\t%ld\t%.1f\t%c\n", current->scan.id_sensore, current->scan.timestamp, current->scan.valore, current->scan.tipo);
        }
        current = current->nextPtr;
    }
    fclose(file);
}



int main(int argc, char *argv[]){

    // VERIFICA CHE I PARAMETRI INSERITI SIANO CORRETTI
    puts("----------1: PARAMETRI----------");
    params parametri = decodeParameters(argc, argv);
    printf("Input file: %s\n", parametri.input);
    printf("Output file: %s\n", parametri.output);
    printf("Tipo di sensore: %c\n", parametri.tipo_sensore);

    // 2. LEGGI I DATI E AGGIUNGILI NELLA LISTA
    Node* Head = ReadDataAndCreateList(parametri.input);
    puts("----------2: LISTA CREATA-------");
    PrintList(Head, 0);
    puts("");

    // 3. CALCOLA LA MEDIA DEI VALORI DELLO STESSO TIPO DI SENSORE
    puts("------------3: MEDIA------------");
    double media = CalculateAverage(Head, parametri.tipo_sensore);
    printf("LA MEDIA DEI VALORI DEL TIPO '%c' è: %.2f\n", parametri.tipo_sensore, media);

    // 4. RICERCA ANOMALIE
    puts("----------4: ANOMALIES----------");
    markAnomalies(Head, media, parametri.tipo_sensore);
    PrintList(Head, 1);
    puts("");
    // 5. SCRITTURA ANOMALIE SU FILE
    puts("------5: WRITING ANOMALIES------");
    WriteAnomaliesToFile(Head, parametri.output);
    printf("Scrittura su %s completata\n", parametri.output);
    FreeList(Head);
    puts("--------------------------------");
}

