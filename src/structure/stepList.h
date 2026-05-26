#ifndef STEP_LIST_H
#define STEP_LIST_H

// ========== ESTRUTURA DE FASE ==========
typedef struct Phase {
    int phaseNumber;            // 1, 2, 3
    char phaseName[50];         // Nome da fase
    struct Phase *next;         // Próxima fase (circular)
    struct Phase *prev;         // Fase anterior (circular)
} Phase;

// ========== FUNÇÕES DE LISTA CIRCULAR ==========
Phase* createPhase(int number, const char *name);
void insertPhase(Phase **head, Phase *newPhase);
Phase* nextPhase(Phase *current);
Phase* prevPhase(Phase *current);
void freePhaseList(Phase *head);
int phaseCount(Phase *head);

#endif
