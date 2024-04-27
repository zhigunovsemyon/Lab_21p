/*Массив представляется указателем на вектор указателей на
строки. Количество строк хранится в -1 элементе вектора указателей.
Количество элементов строки хранится в -1 элементе строки. Память
выделяется одним блоком.

Описание последовательности шагов для следующих
действий:
*	создание массива;
*	доступ к элементам;
*	доступ к размерам массива (количество строк и длина произвольной строки);
*	сохранение в текстовый файл;
*	сохранение в бинарный файл;
*	создание из данных в текстовом файле;
*	создание из данных в бинарном файле;
*	уничтожение массива.
*/

/*Текстовый файл содержит в первой строке первым
 * числом количество строк, в следующих числах этой строки -- размеры строк,
 * дальнейшие строки содержат непосредственно числа*/
#include "RM.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void NullMatrix(double **arr);
void FillMatrixRandom(double **arr, double a, double b);
void FillMatrixManualy(double **arr);
double Random(double x, double y);
double **ManualCreate(void);
double **ReadTextFile(const char *fname);
uint8_t WriteTextFile(const char *fname, double **arr);
uint8_t WriteBinary(const char *fname, double **arr);
double **ReadBinary(const char *fname);
void Print(double **arr);
uint8_t CommandPicker(double **flag);
uint16_t *CreateSizeArray(uint16_t rows);

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
	REMOVE
};

int main(void)
{
	srand(time(NULL));
	double **matrix = NULL;
	char fname[256];
	uint8_t runs = 1;

	do
	{
		switch (CommandPicker(matrix))
		{
		default:
			break;

		case MANUAL_CREATE: {
			printf("Число строк: ");
			uint16_t rows;
			scanf("%hu", &rows);
			uint16_t *cols = CreateSizeArray(rows);
			matrix = RM_CreateArray(rows, cols);
			free(cols);
		}
		break;

		case FILL_RANDOM:{
			double r[2];
			printf("Введите диапазон чисел: ");
			scanf("%lf %lf", r, r + 1);
			FillMatrixRandom(matrix, *r, *(r + 1));
		}
			break;

		case FILL_MANUAL:
			FillMatrixManualy(matrix);
			break;

		case FILL_ZERO:
			NullMatrix(matrix);
			break;

		case LEAVE:
			runs = 0;
			RM_Free(matrix);
			return NOERR;

		case REMOVE:
			RM_Free(matrix);
			matrix = NULL;
			break;

		case WRITE_TXT:
			printf("Название открываемого файла: ");
			scanf("%255s", fname);
			if (WriteTextFile(fname, matrix))
				puts("Не удалось записать матрицу!");
			break;

		case WRITE_BIN:
			printf("Название открываемого файла: ");
			scanf("%255s", fname);
			if (WriteBinary(fname, matrix))
				puts("Не удалось записать матрицу!");
			break;

		case READ_BIN:
			printf("Название открываемого файла: ");
			scanf("%255s", fname);
			matrix = ReadBinary(fname);
			break;

		case READ_TXT:
			printf("Название открываемого файла: ");
			scanf("%255s", fname);
			matrix = ReadTextFile(fname);
			break;

		case PRINT:
			Print(matrix);
			break;
		}
	} while (runs);
}

// Создание массива размеров матрицы на rows строк. Возвращаемый адрес необходимо освободить!
uint16_t *CreateSizeArray(uint16_t rows)
{
	// Выделение памяти под массив
	uint16_t *cols = (uint16_t *)malloc(rows * sizeof(uint16_t));

	// Перебор и наполнение матрицы
	for (uint16_t i = 0; i < rows; ++i)
	{
		printf("Размер %hu строки: ", i);
		scanf("%hu", cols + i);
	}

	return cols;
}
// Выбор пользователем действия над матрицей
uint8_t CommandPicker(double **flag)
{
	puts("Для выхода из программы введите q");
	if (flag) // Если матрица есть
	{
		printf("Выберите действие над матрицей: %s%s%s%s%s%s%s\n", "p - вывод на экран\n",
			   "b - запись в бинарный файл\n", "t - запись в текстовый файл\n", "0 - зануление\n",
			   "m - ручное задание значений\n", "r - Наполнение из некоторого диапазона\n",
			   "d - удаление матрицы из памяти");
	}
	else // Если матрицы нет
	{
		puts("Матрица не была загружена!");
		printf("Укажите способ создания матрицы: %s%s%s\n", "m - ручное задание размеров\n",
			   "b - прочтение из бинарного файла\n", "t - прочтение из текстового файла\n");
	}

	do
	{
		char Choice;
		scanf("%c", &Choice);
		switch (Choice)
		{
		case 'q':
		case 'Q':
			return LEAVE;
		case 'd':
		case 'D':
			return REMOVE;
			// return (flag) ? REMOVE : INVALID;
		case 'p':
		case 'P':
			return (flag) ? PRINT : INVALID;
		case '0':
			return (flag) ? FILL_ZERO : INVALID;
		case 'r':
		case 'R':
			return (flag) ? FILL_RANDOM : INVALID;
		case 'b':
		case 'B':
			return (flag) ? WRITE_BIN : READ_BIN;
		case 't':
		case 'T':
			return (flag) ? WRITE_TXT : READ_TXT;
		case 'm':
		case 'M':
			return (flag) ? FILL_MANUAL : MANUAL_CREATE;
		default:
			continue;
		}
	} while (1);
}

// Печать матрицы на экран
void Print(double **arr)
{
	putchar('\n');
	RM_Print(arr, "%.3lf\t", stdout);
	putchar('\n');
}

/*Чтение матрицы из бинарного файла с названием fname
 * при неполадках возвращает NULL */
double **ReadBinary(const char *fname)
{
	// Открытие нужного файла
	FILE *f = fopen(fname, "rb");

	// Попытка чтения этого файла, при неудаче возвращается NULL
	double **ret = RM_ReadBinFile(f);

	if (f)		   // Если файл удалось открыть
		fclose(f); // то осуществляется его закрытие
	return ret;	   // Возврат матрицы, или NULL
}

/*Запись матрицы в бинарный файл с названием fname
 * при неполадках возвращает код ошибки*/
uint8_t WriteBinary(const char *fname, double **arr)
{
	// Открытие нужного файла
	FILE *f = fopen(fname, "wb");

	// Запись в открытый файл, сохранение возвращённого кода ошибки
	uint8_t ret = RM_WriteBinary(arr, f);

	if (!ret)	   // Если файл удалось открыть
		fclose(f); // то осуществляется его закрытие
	return ret;	   // Возврат кода ошибки
}

/*Запись матрицы в текстовый файл с названием fname
 * при неполадках возвращает код ошибки */
uint8_t WriteTextFile(const char *fname, double **arr)
{
	// Открытие нужного файла
	FILE *f = fopen(fname, "wt");

	// Запись в открытый файл, сохранение возвращённого кода ошибки
	uint8_t ret = RM_WriteTxtFile(arr, f);

	if (!ret)	   // Если файл удалось открыть
		fclose(f); // то осуществляется его закрытие
	return ret;	   // Возврат кода ошибки
}

/*Чтение матрицы из текстового файла с названием fname
 * при неполадках возвращает NULL */
double **ReadTextFile(const char *fname)
{
	// Открытие нужного файла
	FILE *f = fopen(fname, "rt");

	// Попытка чтения этого файла, при неудаче возвращается NULL
	double **ret = RM_ReadTxtFile(f);

	if (f)		   // Если файл удалось открыть
		fclose(f); // то осуществляется его закрытие
	return ret;	   // Возврат матрицы, или NULL
}

/*Ручной ввод создание матрицы пользователем*/
double **ManualCreate(void)
{
	uint16_t rows, *cols;
	printf("Укажите число столбцов в матрице: ");
	scanf("%hu", &rows);

	// Создание массива размеров
	cols = (uint16_t *)malloc(sizeof(uint16_t) * rows);
	if (!cols)
		return NULL;

	// Наполнение массива размеров
	for (uint16_t i = 0; i < rows; ++i)
	{
		printf("Введите размер %hu-ой строки", i);
		scanf("%hu", cols + i);
	}

	// Создание матрицы из данных размеров, будет возвращён NULL при неудаче
	double **m = RM_CreateArray(rows, cols);
	free(cols);
	return m;
}

// Возврат случайного числа из диапазона [a, b]. Требует предварительной инициализации посредством srand()
double Random(double x, double y)
{
	// Требует доработки для генерации нецелых чисел
	int a = (int)x;
	int b = (int)y;
	return (double)(rand() % (b - a + 1) + a);
}

// Заполнение матрицы arr случайными числами
void FillMatrixRandom(double **arr, double a, double b)
{
	if (a > b) //"Переворот" чисел
	{
		double tmp = b;
		b = a;
		a = tmp;
	}

	for (uint16_t j, n, i = 0, m = RM_GetLineCount(arr); i < m; ++i)
		for (j = 0, n = RM_GetElCount(arr[i]); j < n; ++j)
			arr[i][j] = Random(a, b);
}

// Зануление матрицы arr
void NullMatrix(double **arr)
{
	for (uint16_t j, n, i = 0, m = RM_GetLineCount(arr); i < m; ++i)
		for (j = 0, n = RM_GetElCount(arr[i]); j < n; ++j)
			arr[i][j] = 0;
}

// Ручное наполнение матрицы arr
void FillMatrixManualy(double **arr)
{
	for (uint16_t j, n, i = 0, m = RM_GetLineCount(arr); i < m; ++i)
		for (j = 0, n = RM_GetElCount(arr[i]); j < n; ++j)
		{
			printf("Элемент [%hu][%hu] = ", i, j);
			scanf("%lf", arr[i] + j);
		}
}
