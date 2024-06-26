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
#include "main.h"
int main(void)
{
	srand(time(NULL));
	char fname[100];
	Matrixes M;
	M.count = 0;
	M.m = NULL;
	do
	{
		switch (CommandPicker(&M))
		{
		case LEAVE:
			//Пока есть матрицы, удалить крайнюю
			for (;M.count;M.count--)
				RM_Free(M.m[M.count - 1]);
			free(M.m);		//Освобождение из памяти вектора матриц
			return ERR_NO;	//Возврат кода отсутсвия ошибок

		case SWITCH:
			M.cur++;
			if (M.cur == M.count)
				M.cur = 0;
			break;

		case REMOVE:
			RemoveSingleMatrix(&M);
			break;

		case READ_BIN:
			printf("Название открываемого файла: ");
			scanf("%99s", fname);
			if (ReadBinary(&M, fname))
				puts("Не удалось создать матрицу!");
			break;

		case READ_TXT:
			printf("Название открываемого файла: ");
			scanf("%99s", fname);
			if (ReadTextFile(&M, fname))
				puts("Не удалось создать матрицу!");
			break;

		case MANUAL_CREATE:
			if (ManualMatrixCreation(&M))
				printf("Не удалось создать новую матрицу!\n");
			break;

		// Функции работы с открытой матрицей
		case RM_LINE:
		{
			uint16_t line;
			printf("Укажите удаляемую строку: ");
			scanf("%hu", &line);
			switch (RM_RemoveNthLine(M.m + M.cur, line))
			{
			case ERR_BADNUM:
				puts("Указана неправильный номер строки!");
				break;
			case ERR_MALLOC:
				puts("Не удалось удалить строку!");
				break;
			case ERR_GOTNULL:
				puts("Неправильная строка!");
				break;
			case ERR_NO:
				if (M.m[M.cur])	//Если строка не была удалена
					break;		//то ничего не делается
				RemoveSingleMatrix(&M); //Если была удалена, то значения счётчиков меняются
				break;
			}
		}
		break;

		case INSERT_LINE:
		{
			uint16_t line, lineLen;
			printf("Укажите вставляемую строку: ");
			scanf("%hu", &line); 
			printf("Укажите её длинну: ");
			scanf("%hu", &lineLen);
			switch (RM_InsertNthLine(M.m + M.cur, line, lineLen))
			{
			case ERR_BADNUM:
				puts("Указана неправильный номер строки!");
				break;
			case ERR_MALLOC:
				puts("Не удалось вставить строку!");
				break;
			case ERR_GOTNULL:
				puts("Неправильная строка!");
				break;
			case ERR_NO:
				break;
			}
		}
		break;

		case FILL_MANUAL:
			FillMatrixManualy(M.m[M.cur]);
			break;

		case PRINT:
			Print(M.m[M.cur]);
			break;

		case FILL_ZERO:;
			NullMatrix(M.m[M.cur]);
			break;

		case FILL_RANDOM: {
			double r[2];
			printf("Введите диапазон чисел: ");
			scanf("%lf %lf", r, r + 1);
			FillMatrixRandom(M.m[M.cur], *r, *(r + 1));
		}
		break;

		case COPY:
			if(CopyMatrix(&M))
				puts("Не удалось скопировать матрицу!");
			break;

		case WRITE_TXT:
			printf("Название записываемого файла: ");
			scanf("%99s", fname);
			if (WriteTextFile(fname, M.m[M.cur]))
				puts("Не удалось записать матрицу!");
			break;

		case WRITE_BIN:
			printf("Название записываемого файла: ");
			scanf("%99s", fname);
			if (WriteBinary(fname, M.m[M.cur]))
				puts("Не удалось записать матрицу!");
			break;
		}
	} while (1);
}

//Копирование текущей матрицы в дескрипторе M
uint8_t CopyMatrix(Matrixes *M)
{
	double ***tmp_vec =
		(double ***)realloc(M->m, sizeof(double **) * (1 + M->count));
	if (!tmp_vec)
		return ERR_MALLOC;

	double **tmp = RM_CopyMatrix(M->m[M->cur]);
	if(!tmp)
		return ERR_MALLOC;

	// Если удалось создать матрицу:
	M->m = tmp_vec; // новый указатель записывается
	// счётчик увеличивается
	(M->count) ? M->cur++ : (M->cur = 0);
	M->count++;
	M->m[M->cur] = tmp; // Запись матрицы в вектор
	return ERR_NO; // Возврат отсутствия кода ошибок
}

// Удаление одной матрицы из списка М
uint8_t RemoveSingleMatrix(Matrixes *M)
{
	// Выделение памяти под вектор матриц
	double ***tmp_vec;

	// Освобождение матрицы из памяти, уменьшение счётчика
	RM_Free(M->m[M->cur]);
	M->count--;
	// Если матрица была крайней, то её номер совпадает с новым значением
	// счётчика
	if (M->cur == M->count)
	{
		if (M->count) // Если таковая есть, то
			M->cur--; // Переключение на предыдущую матрицу
	}
	else // Если матрица не была крайней
		M->m[M->cur] =
			M->m[M->count]; // На место текущей подставится бывшая крайняя

	// Если после удаления ещё остались матрицы
	if (M->count)
	{ // осуществляется выделение памяти под новый вектор матриц
		tmp_vec = (double ***)realloc(M->m, sizeof(double **) * (M->count));
		if (!tmp_vec)
			return ERR_MALLOC;
		else // Он подставляется на место старого
			M->m = tmp_vec;
	}
	else //Если число матриц стало равным нулю
	{	//Освобождается память вектора, адрес зануляется
		free(M->m);
		M->m = NULL;
	}
	return ERR_NO;
}

/*Ручной ввод создание матрицы пользователем*/
uint8_t ManualMatrixCreation(Matrixes *M)
{
	uint16_t lines;
	// Выделение памяти под вектор матриц
	double ***tmp_vec =
		(double ***)realloc(M->m, sizeof(double **) * (1 + M->count));
	if (!tmp_vec)
		return ERR_MALLOC;

	printf("Введите число строк в матрице: ");
	scanf("%hu", &lines);
	// Создание массива размеров
	uint16_t *lineSizes = CreateSizeArray(lines);
	if (!lineSizes)
		return ERR_MALLOC;

	// Создание матрицы на основе массива размеров
	double **tmp = RM_CreateArray(lines, lineSizes);
	free(lineSizes); // Освобождение массива размеров
	if (!tmp)
		return ERR_MALLOC;

	// Если удалось создать матрицу:
	M->m = tmp_vec; // новый указатель записывается
	// счётчик увеличивается
	(M->count) ? M->cur++ : (M->cur = 0);
	M->count++;
	M->m[M->cur] = tmp; // Запись матрицы в вектор
	return ERR_NO; // Возврат отсутствия кода ошибок
}

// Создание массива размеров матрицы на rows строк. Возвращаемый адрес
// необходимо освободить!
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

// Печать матрицы на экран
void Print(double **arr)
{
	putchar('\n');
	RM_Print(arr, "%.3lf\t", stdout);
	putchar('\n');
}

/*Чтение матрицы из бинарного файла с названием fname
 * при неполадках возвращает NULL */
uint8_t ReadBinary(Matrixes *M, const char *fname)
{
	// Выделение памяти под вектор матриц
	double ***tmp_vec =
		(double ***)realloc(M->m, sizeof(double **) * (1 + M->count));
	if (!tmp_vec)
		return ERR_MALLOC;

	// Открытие нужного файла
	FILE *f = fopen(fname, "rb");

	// Попытка чтения этого файла, при неудаче возвращается NULL
	double **tmp = RM_ReadBinFile(f);
	if (f)		   // Если файл удалось открыть
		fclose(f); // то осуществляется его закрытие

	if (!tmp)
		return ERR_MALLOC;

	// Если удалось создать матрицу:
	M->m = tmp_vec; // новый указатель записывается
	// счётчик увеличивается
	(M->count) ? M->cur++ : (M->cur = 0);
	M->count++;
	M->m[M->cur] = tmp; // Запись матрицы в вектор
	return ERR_NO; // Возврат отсутствия кода ошибок
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
uint8_t ReadTextFile(Matrixes *M, const char *fname)
{
	// Выделение памяти под вектор матриц
	double ***tmp_vec =
		(double ***)realloc(M->m, sizeof(double **) * (1 + M->count));
	if (!tmp_vec)
		return ERR_MALLOC;

	// Открытие нужного файла
	FILE *f = fopen(fname, "rt");

	// Попытка чтения этого файла, при неудаче возвращается NULL
	double **tmp = RM_ReadTxtFile(f);

	if (f)		   // Если файл удалось открыть
		fclose(f); // то осуществляется его закрытие

	if (!tmp)
		return ERR_MALLOC;

	// Если удалось создать матрицу:
	M->m = tmp_vec; // новый указатель записывается
	// счётчик увеличивается
	(M->count) ? M->cur++ : (M->cur = 0);
	M->count++;
	M->m[M->cur] = tmp; // Запись матрицы в вектор
	return ERR_NO; // Возврат отсутствия кода ошибок
}

// Возврат случайного числа из диапазона [a, b]. Требует предварительной
// инициализации посредством srand()
double Random(double x, double y)
{
	return x +	//К полученному ниже прибавляется нижняя планка диапазона
		(double)(rand()) *	//rand() создаёт случайное число в диапазоне (0,RAND_MAX), 
			((y - x) / RAND_MAX); //которое умножается на отношение корридора чисел к RAND_MAX
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

// Выбор пользователем действия над матрицей
uint8_t CommandPicker(Matrixes *m)
{ // q b k m p w t 0 c d i l f
	printf("\n%s%s%s%s%s", "Выберите действие:\n",
		   "Для выхода из программы введите q,\n",
		   "Чтобы прочитать матрицу из бинарного файла, введите b,\n",
		   "Чтобы прочитать матрицу из текстового файла, введите k,\n",
		   "Чтобы в ручную ввести матрицу, введите m");
	if (m->count) // Если матрицы есть
	{
		printf(",\n%s%s%s%s%s%s%s%s%s%s\n",
			   "Чтобы вывести матрицу на экран, введите p\n",
			   "Чтобы записать матрицу в бинарный файл, введите w\n",
			  "Чтобы продублировать матрицу, введите c\n",
				"Чтобы записать матрицу в текстовый файл, введите t\n",
			"Чтобы удалить строку, введите l\n",
			"Чтобы вставить строку, введите i\n",
			   "0 - зануление матрицы\n", "f - ручное задание значений\n",
			   "r - Наполнение из некоторого диапазона\n",
			   "d - удаление матрицы из памяти\n");
		printf("Выбрана матрица %hu\n", m->cur);
	}
	else
		printf("\nМатриц не загружено!\n");

	do
	{
		char Choice;
		scanf("%c", &Choice);
		switch (Choice)
		{
		case 'q':
		case 'Q':
			return LEAVE;
		case 'b':
		case 'B':
			return READ_BIN;
		case 'k':
		case 'K':
			return READ_TXT;
		case 'm':
		case 'M':
			return MANUAL_CREATE;

			// Функции работы с открытой матрицей
		case 'd':
		case 'D':
			if (m->count)
				return REMOVE;
			break;
		case 'i':
		case 'I':
			if (m->count)
				return INSERT_LINE;
		case 'l':
		case 'L':
			if (m->count)
				return RM_LINE;
			break;
		case 's':
		case 'S':
			if (m->count)
				return SWITCH;
			break;
		case 'f':
		case 'F':
			if (m->count)
				return FILL_MANUAL;
			break;
		case 'p':
		case 'P':
			if (m->count)
				return PRINT;
			break;
		case '0':
			if (m->count)
				return FILL_ZERO;
			break;
		case 'c':
		case 'C':
			if (m->count)
				return COPY;
			break;
		case 'r':
		case 'R':
			if (m->count)
				return FILL_RANDOM;
			break;
		case 'w':
		case 'W':
			if (m->count)
				return WRITE_BIN;
			break;
		case 't':
		case 'T':
			if (m->count)
				return WRITE_TXT;
			break;
		default:
			continue;
		}
	} while (1);
}
