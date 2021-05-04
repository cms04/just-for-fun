#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct blatt_s {
    uint8_t blatt_nr;
    float punkte_gesamt;
    float punkte_erreicht;
    struct blatt_s *next;
} blatt_t;

typedef struct vorlesung_s {
    char *name;
    uint8_t anzahl_blaetter;
    uint8_t bisherige_anzahl_blaetter;
    float anteil_fuer_zulassung;
    float gesamtpunktzahl;
    float gesamtpunktzahl_insgesamt;
    float erreichte_punkte;
    float punkte_pro_blatt;
    blatt_t *blaetter;
    struct vorlesung_s *next;
} vorlesung_t;

void print_file_format(void) {
    printf("\n"
           "Aufbau der CSV-Datei:\n"
           "Name_Vorlesung,Blaetter_gesamt,Anteil_fuer_Zulassung,Blatt1_erreicht,Blatt1_gesamt,Blatt2_erreicht,Blatt2_gesamt,...\n\n"
           "Diese Zeile in die erste Zeile der CSV-Datei einfügen und direkt darunter die\n"
           "richtigen Daten einfügen. Die Beschreibungszeile wird übersprungen.\n");
}

float berechne_punkte_pro_blatt(vorlesung_t *vorlesung) {
    if (vorlesung->anzahl_blaetter <= vorlesung->bisherige_anzahl_blaetter || vorlesung->gesamtpunktzahl >= vorlesung->gesamtpunktzahl_insgesamt) {
        return 0;
    }
    float diff = vorlesung->gesamtpunktzahl_insgesamt * vorlesung->anteil_fuer_zulassung - vorlesung->erreichte_punkte;
    float uebrige_blaetter = vorlesung->anzahl_blaetter - vorlesung->bisherige_anzahl_blaetter;
    return diff / uebrige_blaetter;
}

float berechne_durchschnittliche_punktzahl(vorlesung_t *vorlesung) {
    return vorlesung->gesamtpunktzahl / vorlesung->bisherige_anzahl_blaetter;
}

uint8_t get_blatt_anzahl(blatt_t *list) {
    if (list == NULL) {
        return 0;
    }
    return 1 + get_blatt_anzahl(list->next);
}

float berechne_erreichte_punktzahl(blatt_t *list) {
    if (list == NULL) {
        return 0.0;
    }
    return list->punkte_erreicht + berechne_erreichte_punktzahl(list->next);
}

float berechne_gesamtpunktzahl(blatt_t *list) {
    if (list == NULL) {
        return 0.0;
    }
    return list->punkte_gesamt + berechne_gesamtpunktzahl(list->next);
}

void free_blatt_list(blatt_t *list) {
    if (list != NULL) {
        free_blatt_list(list->next);
        free(list);
    }
}

void free_list(vorlesung_t *list) {
    if (list != NULL) {
        free_list(list->next);
        free_blatt_list(list->blaetter);
        free(list->name);
        free(list);
    }
}

blatt_t *add_blatt_to_list(blatt_t *list, blatt_t *new) {
    if (list == NULL) {
        return new;
    }
    list->next = add_blatt_to_list(list->next, new);
    return list;
}

vorlesung_t *add_vorlesung_to_list(vorlesung_t *list, vorlesung_t *new) {
    if (list == NULL) {
        return new;
    }
    list->next = add_vorlesung_to_list(list->next, new);
    return list;
}

vorlesung_t *read_vorlesung(char *line) {
    vorlesung_t *result = (vorlesung_t *) malloc(sizeof(vorlesung_t));
    result->next = NULL;
    char *token = NULL;
    token = strtok(line, ",");
    if (token == NULL) {
        printf("ERROR: Zeile '%s ...' fehlerhaft\n", line);
        print_file_format();
        free(result);
        return NULL;
    }
    result->name = (char *) malloc((strlen(token) + 1) * sizeof(char));
    bzero(result->name, strlen(token) + 1);
    strncpy(result->name, token, strlen(token));
    token = strtok(NULL, ",");
    if (token == NULL) {
        printf("ERROR: Zeile '%s ...' fehlerhaft\n", line);
        print_file_format();
        free(result->name);
        free(result);
        return NULL;
    }
    result->anzahl_blaetter = atoi(token);
    token = strtok(NULL, ",");
    if (token == NULL) {
        printf("ERROR: Zeile '%s ...' fehlerhaft\n", line);
        print_file_format();
        free(result->name);
        free(result);
        return NULL;
    }
    result->anteil_fuer_zulassung = strtof(token, NULL);
    result->blaetter = NULL;
    uint8_t blattnr = 1;
    while ((token = strtok(NULL, ",")) != NULL) {
        blatt_t *new = (blatt_t *) malloc(sizeof(blatt_t));
        new->blatt_nr = blattnr;
        blattnr++;
        new->next = NULL;
        new->punkte_erreicht = strtof(token, NULL);
        token = strtok(NULL, ",");
        if (token == NULL) {
            printf("ERROR: Zeile '%s ...' fehlerhaft\n", line);
            print_file_format();
            free(new);
            free_blatt_list(result->blaetter);
            free(result->name);
            free(result);
            return NULL;
        }
        new->punkte_gesamt = strtof(token, NULL);
        result->blaetter = add_blatt_to_list(result->blaetter, new);
    }
    result->gesamtpunktzahl = berechne_gesamtpunktzahl(result->blaetter);
    result->erreichte_punkte = berechne_erreichte_punktzahl(result->blaetter);
    result->bisherige_anzahl_blaetter = get_blatt_anzahl(result->blaetter);
    result->gesamtpunktzahl_insgesamt = result->anzahl_blaetter * berechne_durchschnittliche_punktzahl(result);
    result->punkte_pro_blatt = berechne_punkte_pro_blatt(result);
    return result;
}

void show_blaetter(blatt_t *list) {
    if (list == NULL) return;
    printf("    Blätter:\n");
    while (list != NULL) {
        printf("        Blatt %d: %.1f von %.1f erreicht ( %.1f Prozent )\n", list->blatt_nr, list->punkte_erreicht, list->punkte_gesamt, 100 * (list->punkte_erreicht / list->punkte_gesamt));
        list = list->next;
    }
    printf("\n");
}

void show_vorlesung(vorlesung_t *vorlesung) {
    if (vorlesung == NULL) return;
    printf("Vorlesung: %s\n", vorlesung->name);
    printf("    Anzahl Blätter gesamt:                    %d\n", vorlesung->anzahl_blaetter);
    if (vorlesung->gesamtpunktzahl > 0) {
        printf("    Insgesamt erreichbare Punkte:             %.1f \n", vorlesung->gesamtpunktzahl_insgesamt);
        printf("    Bisher erreichbare Punkte:                %.1f \n", vorlesung->gesamtpunktzahl);
        printf("    Bisher erreichte Punkte:                  %.1f \n\n", vorlesung->erreichte_punkte);
        printf("    Anteil an bisher erreichbaren Punkten:    %.1f Prozent \n", 100 * (vorlesung->erreichte_punkte / vorlesung->gesamtpunktzahl));
    }
    printf("    Anteil für Zulassung:                     %.1f Prozent \n", 100 * vorlesung->anteil_fuer_zulassung);
    if (vorlesung->punkte_pro_blatt > 0) {
        printf("\n    Fehlende Punkte:                          %.1f \n", (vorlesung->gesamtpunktzahl_insgesamt * vorlesung->anteil_fuer_zulassung) - vorlesung->erreichte_punkte);
        printf("    Punkte pro Blatt:                         %.1f \n", vorlesung->punkte_pro_blatt);
    }
    if (vorlesung->punkte_pro_blatt == 0 && vorlesung->gesamtpunktzahl > 0) {
        printf("\n    Die Zulassung ist durch die bisherigen Ergebnisse bereits erreicht!\n");
    }
    printf("\n");
    show_blaetter(vorlesung->blaetter);
    printf("\n\n");
}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("ERROR: Usage: ./zulassungsrechner daten.csv\n"
               "       mit einer daten.csv\n");
        print_file_format();
        return EXIT_FAILURE;
    }
    FILE *datei = fopen(argv[1], "r");
    if (datei == NULL) {
        printf("ERROR: Konnte Datei '%s' nicht öffnen\n", argv[1]);
        return EXIT_FAILURE;
    }
    char *line = NULL;
    size_t n = 0;
    vorlesung_t *vorlesungen = NULL;
    getline(&line, &n, datei);
    free(line);
    line = NULL;
    n = 0;
    while (getline(&line, &n, datei) > 0) {
        line[n - 1] = '\0';
        vorlesung_t *new = read_vorlesung(line);
        if (new == NULL) {
            fclose(datei);
            free(line);
            free_list(vorlesungen);
            return EXIT_FAILURE;
        }
        vorlesungen = add_vorlesung_to_list(vorlesungen, new);
        free(line);
        line = NULL;
        n = 0;
    }
    free(line);
    fclose(datei);
    vorlesung_t *toshow = vorlesungen;
    if (toshow != NULL) {
        while (toshow != NULL) {
            show_vorlesung(toshow);
            toshow = toshow->next;
        }
        printf("\033[A\033[A\033[A");
    }
    free_list(vorlesungen);
    return EXIT_SUCCESS;
}
