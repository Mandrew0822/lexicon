#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_WORDS 200
#define MAX_LENGTH 50
#define GREEN "\033[32m"
#define RED "\033[31m"
#define RESET "\033[0m"

typedef struct {
    char word[MAX_LENGTH];
    char translations[10][MAX_LENGTH];
    int num_translations;
} WordEntry;

typedef struct {
    int correct;
    int total;
} Stats;

void toLower(char *str) {
    for(int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

void stripNewline(char *str) {
    str[strcspn(str, "\n")] = 0;
}

Stats loadStats() {
    Stats stats = {0, 0};
    FILE *statsFile = fopen("stats.txt", "r");
    if (statsFile != NULL) {
        fscanf(statsFile, "Correct: %d\nTotal: %d", &stats.correct, &stats.total);
        fclose(statsFile);
    }
    return stats;
}

void saveStats(Stats stats) {
    FILE *statsFile = fopen("~/.lexicon/stats.txt", "w");
    if (statsFile != NULL) {
        fprintf(statsFile, "Correct: %d\nTotal: %d", stats.correct, stats.total);
        fclose(statsFile);
    }
}

void printUsage() {
    printf("Usage: ./program [--german | --latin]\n");
    printf("  --german    Practice German words\n");
    printf("  --latin     Practice Latin words\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc != 2 || (strcmp(argv[1], "--german") != 0 && strcmp(argv[1], "--latin") != 0)) {
        printUsage();
    }

    WordEntry words[MAX_WORDS];
    int word_count = 0;
    char input[MAX_LENGTH];
    Stats stats = loadStats();
    const char *language = strcmp(argv[1], "--german") == 0 ? "German" : "Latin";
    const char *filename = strcmp(argv[1], "--german") == 0 ? "~/.lexicon/german.lang" : "~/.lexicon/latin.lang";
    
    FILE *file = fopen(filename, "r");
    char line[100];
    
    if (file == NULL) {
        printf("Error opening %s\n", filename);
        return 1;
    }

    // Load words from file
    while (fgets(line, sizeof(line), file) && word_count < MAX_WORDS) {
        char *foreign = strtok(line, ">");
        char *english = strtok(NULL, ">");
        
        if (foreign && english) {
            while(*foreign == ' ') foreign++;
            while(*english == ' ') english++;
            
            stripNewline(foreign);
            stripNewline(english);
            
            strcpy(words[word_count].word, foreign);
            
            words[word_count].num_translations = 0;
            char *trans = strtok(english, ",");
            while (trans && words[word_count].num_translations < 10) {
                while(*trans == ' ') trans++;
                stripNewline(trans);
                strcpy(words[word_count].translations[words[word_count].num_translations], trans);
                words[word_count].num_translations++;
                trans = strtok(NULL, ",");
            }
            
            word_count++;
        }
    }
    fclose(file);

    if (word_count == 0) {
        printf("No words found in %s\n", filename);
        return 1;
    }

    srand(time(NULL));
    printf("Welcome to %s Practice!\n", language);
    printf("Loaded %d words\n", word_count);
    printf("Type your answer and press Enter\n");
    printf("Press Enter with no input to continue to next word\n");
    printf("Type 'q' or 'exit' to quit\n");
    printf("Current stats - Correct: %d, Total: %d\n\n", stats.correct, stats.total);

    int index = rand() % word_count;
    printf("\nTranslate: %s\n", words[index].word);
    printf("Your answer: ");

    while (1) {
        fgets(input, MAX_LENGTH, stdin);
        stripNewline(input);
        
        if (strlen(input) == 0) {
            index = rand() % word_count;
            printf("\nTranslate: %s\n", words[index].word);
            printf("Your answer: ");
            continue;
        }

        toLower(input);

        if (strcmp(input, "q") == 0 || strcmp(input, "exit") == 0) {
            printf("\nFinal Stats - Correct: %d, Total: %d\n", stats.correct, stats.total);
            saveStats(stats);
            printf(strcmp(language, "German") == 0 ? "Auf Wiedersehen!\n" : "Vale!\n");
            break;
        }

        int correct = 0;
        for (int i = 0; i < words[index].num_translations; i++) {
            char temp[MAX_LENGTH];
            strcpy(temp, words[index].translations[i]);
            toLower(temp);
            
            if (strcmp(input, temp) == 0) {
                correct = 1;
                break;
            }
        }

        stats.total++;
        if (correct) {
            stats.correct++;
            printf("%sCorrect!%s\n", GREEN, RESET);
        } else {
            printf("%sWrong. The correct translation(s): ", RED);
            for (int i = 0; i < words[index].num_translations; i++) {
                printf("%s", words[index].translations[i]);
                if (i < words[index].num_translations - 1) {
                    printf(", ");
                }
            }
            printf("%s\n", RESET);
        }

        index = rand() % word_count;
        printf("\nTranslate: %s\n", words[index].word);
        printf("Your answer: ");
    }

    return 0;
}