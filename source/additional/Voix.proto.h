
/* Voix.c */
int mOpenSound(int);
long ReadTable(int, int ***, long, int);
int Threshold(char[], int, int);
int DisplayTable(int, int **, int **, int, long, int, int, int, int);
long MakeEnvelope(int **, int **, int, long, int **, int **);
int MakeSonagram(int **, long, long ****, int **, int *, int *, int *);
int DrawSonagram(int, long ****, int, int, int);
int Rescale(float, float, float, int, int);
int DrawBlock(int, int, Rect);
int GetWalsh(int **, int, int, int, int, long ****, int, int **, int *);
int Walsh(int, int, int, int *);
int Wal01(int, int, int, int *);
int Rad01(int, int, int);
int Power2(int);
int Test(void);
int Test1(void);
int Mymod(int, int);
