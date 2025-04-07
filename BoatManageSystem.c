#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*-------------------------------STRUCTS, ENUMS, UNIONS & DEFINES------------------------------*/
#define MAX_BOATS 120
#define MAX_NAME_LENGTH 128 // can hold 127 characters and a null terminator

// defines for all the place type rates
#define SLIP_RATE 12.50
#define LAND_RATE 14.00
#define TRAILOR_RATE 25.00
#define STORAGE_RATE 11.20


// enum to represent the place type
typedef enum {
    SLIP,
    LAND,
    TRAILOR,
    STORAGE
} PlaceType;

// union for info based on the place
typedef union {
    int slipNum; // for slip, number 1-85
    char bayLetter; // for land, A-Z
    char trailorLicense[10]; // for a trailor, license plate has up to 8 character so 10 just to make sure
    int storageNum; // for storage, number 1-50
} PlaceInfo;

// struct for the boats themselves
typedef struct {
    char name[MAX_NAME_LENGTH];
    int length; // apparently there are only whole-number length boats, so no need for float
    PlaceType placeType;
    PlaceInfo placeInfo;
    float amountOwed; 
} Boat;

/*-------------------------------GLOBAL VARIABLES------------------------------*/

Boat* boatsList[MAX_BOATS]; // array of pointers to the boats
int boatCount = 0; // global counter for the amount of boats

/*-------------------------------FUNCTIONS FOR OPENING, READING & WRITING TO THE FILE------------------------------*/

// helper function for qsort to compare boats by name
int compareBoats(const void *a, const void *b){
    const Boat *boatA = *(const Boat **)a;
    const Boat *boatB = *(const Boat **)b;
    return strcasecmp(boatA->name, boatB->name);
}


int loadBoatData(const char *fileName){
    FILE *file = fopen(fileName, "r"); // opening the file in read mode

    // if we can't open it, return 1 to indicate an error
    if (file == NULL){
        printf("Error: Could not open file %s. \n", fileName);
        return 1;
    }

    char line[256]; //buffer to hold all the characters in each line
    boatCount = 0; 

    // NOTE: info in the CSV file will be stored like <Boat Name>,<Boat Length>,<Place Type>,<Place Info>,<Amount Owed>

    while (boatCount < MAX_BOATS && fgets(line, sizeof(line), file) != NULL){
        Boat* boat = (Boat*)malloc(sizeof(Boat)); // allocating memory for each boat

        if (boat == NULL){
            printf("Error: Could not allocate memory for boat. \n");
            return 1;
        }

        // parsing line by line, using the "," as the delimiter separating the tokens
        char* token = strtok(line, ",");

        // first, pull boat name
        strcpy(boat->name, token);
        token = strtok(NULL, ",");

        // next, pull boat length
        boat->length = atoi(token); //converting the string to float for the length type

        token = strtok(NULL, ",");

        // next, pull place type (using our enum)
        if (strcmp(token, "slip") == 0) {
            boat->placeType = SLIP;
        } else if (strcmp(token, "land") == 0) {
            boat->placeType = LAND;
        } else if (strcmp(token, "trailor") == 0){
            boat->placeType = TRAILOR;
        } else if (strcmp(token, "storage") == 0){
            boat->placeType = STORAGE;
        }

        token = strtok(NULL, ",");

        // pulling additional info (based on the place type)
        switch(boat->placeType){
            case SLIP:
                boat->placeInfo.slipNum = atoi(token);
                break;
            case LAND:
                boat->placeInfo.bayLetter = token[0];
                break;
            case TRAILOR:
                strcpy(boat->placeInfo.trailorLicense, token);
                break;
            case STORAGE:
                boat->placeInfo.storageNum = atoi(token);
                break;
        }

        token = strtok(NULL, ",");

        // lastly, amount owed

        boat->amountOwed = atof(token);


        // add the boat and all of it's info to the list

        boatsList[boatCount] = boat;
        boatCount++;
    }

    // ensure that we start with an alphabetically sorted list
    qsort(boatsList, boatCount, sizeof(Boat*), compareBoats);

    fclose(file); 
    return 0; // for success
}

void saveBoatData(const char* fileName){
    FILE *file = fopen(fileName, "w"); // opening up the file in write mode

    //again, checking the file
    if (file == NULL){
        printf("Error: Could not open file %s to save. \n", fileName);
        return;
    }

    for (int i = 0; i < MAX_BOATS && boatsList[i] != NULL; i++){
        Boat *boat = boatsList[i];

        // printing boat name & length to file
        fprintf(file, "%s,%d,", boat->name, boat->length);

        //tricky part for boat placetype and placeinfo
        if(boat->placeType == SLIP){
            fprintf(file, "slip,%d,", boat->placeInfo.slipNum);
        } else if (boat->placeType == LAND){
            fprintf(file, "land,%c,", boat->placeInfo.bayLetter);
        } else if (boat->placeType == TRAILOR){
            fprintf(file, "trailor,%s,", boat->placeInfo.trailorLicense);
        } else if (boat->placeType == STORAGE){
            fprintf(file, "storage,%d,", boat->placeInfo.storageNum);
        }

        // lastly, amount owed
        fprintf(file, "%.2f\n", boat->amountOwed);
    }

    fclose(file);
}


/*-------------------------------FUNCTIONS FOR THE MENU------------------------------*/

void printInventory(){
    for (int i = 0; i < MAX_BOATS; i++){
        if (boatsList[i] != NULL){
            Boat *boat = boatsList[i];

            //convert the place information to a string so that it can be printed
            char placeInfoString[128];
            if(boatsList[i]->placeType == SLIP){
                sprintf(placeInfoString, "# %d", boat->placeInfo.slipNum);
            } else if (boatsList[i]->placeType == LAND){
                sprintf(placeInfoString, "%c", boat->placeInfo.bayLetter);
            } else if (boatsList[i]->placeType == TRAILOR){
                strcpy(placeInfoString, boat->placeInfo.trailorLicense);
            } else if (boatsList[i]->placeType == STORAGE){
                sprintf(placeInfoString, "# %d", boat->placeInfo.storageNum);
            }

            // long print statement to format and print all the info for each boat
            printf("%-20s %4d' %-8s %-7s Owes $%7.2f\n", 
                boat->name,
                boat->length,
                boatsList[i]->placeType == SLIP ? "slip" : 
                (boatsList[i]->placeType == LAND ?  "land" : 
                (boatsList[i]->placeType == TRAILOR ? "trailor" : "storage")),
                placeInfoString,
                boat->amountOwed
            );
        }
    }
}

void addBoat(const char* boatData){
    // make sure we don't have too many boats before we try to add one
    if (boatCount >= MAX_BOATS){
        printf("Error: At maximum capacity for boats. \n");
        return;
    }

    Boat *boat = (Boat*)malloc(sizeof(Boat)); 

    //make sure memory was allocated correctly
    if (boat == NULL){
        printf("Error: Could not allocate memory for boat. \n");
        return;
    }

    char dataCopy[256]; // creating a copy so we don't directly modify the data
    strncpy(dataCopy, boatData, sizeof(dataCopy));
    dataCopy[sizeof(dataCopy) - 1] = '\0'; // null-terminating the string

    // using the exact same logic as the loadBoatData function to parse the data
    char* token = strtok(dataCopy, ","); // comma is the delimiter

    // first, get the name
    strcpy(boat->name, token);
    token = strtok(NULL, ",");

    // next, the length
    boat->length = atoi(token);
    token = strtok(NULL, ",");

    // next, the place type
    if (strcmp(token, "slip") == 0) {
            boat->placeType = SLIP;
    } else if (strcmp(token, "land") == 0) {
            boat->placeType = LAND;
    } else if (strcmp(token, "trailor") == 0){
            boat->placeType = TRAILOR;
    } else if (strcmp(token, "storage") == 0){
            boat->placeType = STORAGE;
    }

    token = strtok(NULL, ",");

    // next, the place info
    switch(boat->placeType){
            case SLIP:
                boat->placeInfo.slipNum = atoi(token);
                break;
            case LAND:
                boat->placeInfo.bayLetter = token[0];
                break;
            case TRAILOR:
                strcpy(boat->placeInfo.trailorLicense, token);
                break;
            case STORAGE:
                boat->placeInfo.storageNum = atoi(token);
                break;
    }

    token = strtok(NULL, ",");

    // lastly, amount owed
    boat->amountOwed = atof(token);

    // adding at next open spot in the array
    boatsList[boatCount++] = boat;

    // sorting the array to be in alphabetical order
    qsort(boatsList, boatCount, sizeof(Boat*), compareBoats);

}

void removeBoat(const char* boatName){
    int found = -1;

    // find boat index using name (case-insensitive)
    for (int i = 0; i < boatCount; i++){
        if(strcasecmp(boatsList[i]->name, boatName) == 0){
            found = i;
            break;
        }
    }

    if (found == -1){
        printf("No boat with that name. \n");
        return;
    }

    // free the memory for the boat at index "found"
    free(boatsList[found]);

    // shift all remaining boats to keep the array "packed"

    for(int j = found; j < boatCount - 1; j++){
        boatsList[j] = boatsList[j + 1];
    }

    // decrement boat count since we removed
    boatCount--;

    // set the last element to NULL
    boatsList[boatCount] = NULL;
}

void pay(const char* boatName, float amount){
    for (int i = 0; i < boatCount; i++){
        // using strcasecmp to ignore case sensitivity
        if(boatsList[i] != NULL && strcasecmp(boatsList[i]->name, boatName) == 0){
            if(amount > boatsList[i]->amountOwed){
                printf("That is more than the amount owed, $%.2f \n", boatsList[i]->amountOwed);
                return;
            }

            boatsList[i]->amountOwed -= amount;
            return;
        }
        
    }
}

void updateMonthly(){
    for (int i = 0; i < boatCount; i++){
        if (boatsList[i] != NULL){
            Boat *boat = boatsList[i];
            switch (boat->placeType){
                case SLIP:
                    boat->amountOwed += boat->length * SLIP_RATE;
                    break;
                case LAND:
                    boat->amountOwed += boat->length * LAND_RATE;
                    break;
                case TRAILOR:
                    boat->amountOwed += boat->length * TRAILOR_RATE;
                    break;
                case STORAGE:
                    boat->amountOwed += boat->length * STORAGE_RATE;
                    break;
            }
        }
    }

}

int checkBoatExists(const char* boatName){
    for (int i = 0; i < boatCount; i++){
        if(boatsList[i] != NULL && strcasecmp(boatsList[i]->name, boatName) == 0){
            return 1;
        }
    }

    return 0;
}

/*-------------------------------MAIN FUNCTION------------------------------*/

int main(int argc, char* argv[]){

    // make sure the user provided a file to read from & write to
    if (argc != 2){
        printf("Error: Please provide a file to read from. \n");
        return 1;
    }

    const char *fileName = argv[1]; // ensures that the fileName is the second argument (first is the program name)

    // open and load the file from the user
    loadBoatData(fileName);

    printf("Welcome to the Boat Management System! \n");
    printf("------------------------------------- \n");

    char option;

    do {
        char boatName[MAX_NAME_LENGTH];
        char boatInput[256];
        float amount;

        printf("(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, E(x)it : ");
        scanf(" %c", &option);

        //using string function to put the input in lowercase
        option = tolower(option);
        

        switch(option) {
            case 'i':
                printInventory();
                break;
            case 'a':
                printf("Please enter the boat data in CSV format: ");
                getchar(); 
                fgets(boatInput, 256, stdin); //reads the entire line from standard input
                addBoat(boatInput); // giving the data we grabbed from the user to the addBoat() function
                break;
            case 'r':
                printf("Please enter the boat name: ");
                getchar();
                fgets(boatName, MAX_NAME_LENGTH, stdin);
                boatName[strcspn(boatName, "\n")] = 0; // removing the newline character so string comparisons work
                removeBoat(boatName);
                break;
            case 'p':
                printf("Please enter the boat name: ");
                getchar();
                fgets(boatName, MAX_NAME_LENGTH, stdin);
                boatName[strcspn(boatName, "\n")] = 0;
                if(checkBoatExists(boatName) == 1){
                    printf("Please enter the amount to be paid: ");
                    scanf("%f", &amount);
                    pay(boatName, amount);
                    break;
                } else {
                    printf("No boat with that name. \n");
                    break;
                }
            case 'm':
                updateMonthly();
                break;
            case 'x':
                printf("Exiting the Boat Management System. \n");
                saveBoatData(fileName);
                break;
            default:
                printf("Invalid option %c. \n", option);
                break;
        }
    } while(option != 'x');


    // free all allocated memory before exiting
    for (int i = 0; i < boatCount; i++){
        if(boatsList[i] != NULL){
            free(boatsList[i]);
            boatsList[i] = NULL;
        }
    }

    return 0;
}
