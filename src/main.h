#include "RM.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum Commands
{
	INVALID,
	LEAVE,
	MANUAL_CREATE,
	READ_TXT,
	READ_BIN,
	FILL_ZERO,
	FILL_RANDOM,
	FILL_MANUAL,
	PRINT,
	WRITE_TXT,
	WRITE_BIN,
	REMOVE,
	SWITCH
};

typedef struct
{
	double ***m;
	uint8_t cur;
	uint8_t count;
} Matrixes;

uint8_t ManualMatrixCreation(Matrixes *M);
void NullMatrix(double **arr);
void FillMatrixRandom(double **arr, double a, double b);
void FillMatrixManualy(double **arr);
double Random(double x, double y);
double **ReadTextFile(const char *fname);
uint8_t WriteTextFile(const char *fname, double **arr);
uint8_t WriteBinary(const char *fname, double **arr);
double **ReadBinary(const char *fname);
void Print(double **arr);
uint8_t CommandPicker(Matrixes *m);
uint16_t *CreateSizeArray(uint16_t rows);

