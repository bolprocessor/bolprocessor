
/* Facteurs.c */
int AutomatonToGrammar(p_state **, int, int, p_state **, p_state **, p_state **);
int WriteTransitions(p_state **, int **);
int DrawRule(int, int, int, int, int);
int InitAutomaton(void);
int NewStateLabel(char[]);
int TransitionLabel(char[]);
int KillTransition(p_state **, int);
p_state **NewState(int);
p_transition **SetTransition(p_state **, int, p_state **, int);
p_state **NextState(p_state **, int);
int SelectionToString(int, char ***, int *);
int CopyState(p_state **, p_state ***, int);
int BuildAutomaton(void);
p_state **Suffix(p_state **, int);
