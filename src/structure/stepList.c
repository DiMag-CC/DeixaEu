#include "stepList.h"
#include <stdlib.h>
#include <string.h>

// ========== CRIAR FASE ==========
Phase* createPhase(int number, const char *name) {
    Phase *newPhase = (Phase*)malloc(sizeof(Phase));
    if (newPhase == NULL) return NULL;

    newPhase->phaseNumber = number;
    strncpy(newPhase->phaseName, name, 49);
    newPhase->phaseName[49] = '\0';
    newPhase->next = NULL;
    newPhase->prev = NULL;

    return newPhase;
}

// ========== INSERIR FASE NA LISTA CIRCULAR ==========
void insertPhase(Phase **head, Phase *newPhase) {
    if (newPhase == NULL) return;

    if (*head == NULL) {
        // Primeira fase
        *head = newPhase;
        newPhase->next = newPhase;
        newPhase->prev = newPhase;
    } else {
        // Inserir no final mantendo circularidade
        Phase *tail = (*head)->prev;
        tail->next = newPhase;
        newPhase->prev = tail;
        newPhase->next = *head;
        (*head)->prev = newPhase;
    }
}

// ========== PRÓXIMA FASE ==========
Phase* nextPhase(Phase *current) {
    if (current == NULL) return NULL;
    return current->next;
}

// ========== FASE ANTERIOR ==========
Phase* prevPhase(Phase *current) {
    if (current == NULL) return NULL;
    return current->prev;
}

// ========== CONTAR FASES ==========
int phaseCount(Phase *head) {
    if (head == NULL) return 0;

    int count = 0;
    Phase *cur = head;
    do {
        count++;
        cur = cur->next;
    } while (cur != head);

    return count;
}

// ========== LIBERAR LISTA DE FASES ==========
void freePhaseList(Phase *head) {
    if (head == NULL) return;

    Phase *cur = head;
    Phase *next;

    do {
        next = cur->next;
        free(cur);
        cur = next;
    } while (cur != head);
}
