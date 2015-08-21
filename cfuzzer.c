// Original code by Tek Huynh

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    char *letters = "abcdefghijklmnopqrstuvwxyz";
    int availableLetters[26];
    int letter;

    int i;
    for (i = 0; i < 26; i++) availableLetters[i] = 1;

    while (1) {
        letter = rand() % 26;
        if (availableLetters[letter]) {
            availableLetters[letter] = 0;
            printf("+ %c %d\n", letters[letter], rand() % 1000);
        } else {
            availableLetters[letter] = 1;
            printf("- %c\n", letters[letter]);
        }
        printf("!\n#\n");
    }
}
