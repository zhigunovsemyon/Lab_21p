#include "RM.h"

// Получение адреса начала сегмента (строки или всего массива)
static void *int_GetMinusOne(void *p)
{
	return (void *)(((uint16_t *)p) - 1);
}

/*Функция считает сумму элементов данного массива arr из элементов Amount */
static uint16_t int_SumOfArray(uint16_t *arr, uint16_t Amount)
{
	/*Перебор элементов массива */
	int16_t Sum = arr[0];
	for (int16_t i = 1; i < Amount; ++i)
		Sum += arr[i];

	return Sum; // Возврат суммы
}

// Освобождение памаяти массива
void RM_Free(double **arr)
{
	if (arr)
		free(int_GetMinusOne(arr));
}

// Извлечение числа элементов строки
uint16_t RM_GetElCount(double *line)
{
	return *((uint16_t *)int_GetMinusOne(line));
}

// Извлечение числа строк из массива
uint16_t RM_GetLineCount(double **arr)
{
	return *((uint16_t *)int_GetMinusOne(arr));
}

// Печать матрицы m формата spec в сток recepient
void RM_Print(double **m, const char *spec, FILE *recepient)
{
	for (uint16_t i = 0; i < RM_GetLineCount(m); i++)
	{
		for (uint16_t j = 0; j < RM_GetElCount(m[i]); j++)
			fprintf(recepient, spec, m[i][j]);
		fputc('\n', recepient);
	}
}

// Выделение памяти под массив. Возвращает NULL при неудаче
double **RM_CreateArray(uint16_t lines, uint16_t *cols)
{
	// Если были переданы неправильные размеры или адрес массива
	if ((!cols) || lines == 0)
		return NULL; // Возврат NULL

	// Вычисление выделяемой памяти под массив и размеры
	int mem = sizeof(double) * int_SumOfArray(cols, lines) + // Память под непосредственно элементы
			  lines * (sizeof(uint16_t) + sizeof(uint16_t *)) + // Память под размеры строк и вектор указателей
			  sizeof(uint16_t);									// Память под количество строк

	// Выделение памяти. Возврат NULL при ошибке
	double **p = (double **)malloc(mem);
	if (!p)
		return NULL;

	p = (double **)((uint16_t *)p + 1); // Сдвиг указателя
	((uint16_t *)p)[-1] = lines;		// Запись количества строк

	//Запись указателя на первую строку
	p[0] = (double *)((void *)p +	  // Начало вектора указателей +
		 sizeof(uint16_t *) * lines); // Пространство под вектор указателей

	((uint16_t *)p[0])[0] = cols[0];			//Запись числа элементов первой строки
	p[0] = (double *)((uint16_t *)(p[0]) + 1);	//Сдвиг указателя

	//Расстановка указателей и запись размеров для всех остальных строк
	for (int i = 1; i < lines; ++i)
	{
		p[i] = (double *)((void *)p[i - 1] +			//Адрес начала предыдущей строки +	
			(sizeof(double) * RM_GetElCount(p[i - 1])));//элементы предыдущей строки

		((uint16_t *)p[i])[0] = cols[i];				//Запись числа элементов строки
		p[i] = (double *)((uint16_t *)(p[i]) + 1);		//Сдвиг указателя
	}
	return p;	//Возврат указателя
}

/*Функция осуществляет чтение из текстового источника f
 * Возвращает NULL в случае ошибки, либо указатель на первый элемент массива*/
double **RM_ReadTxtFile(FILE *f)
{
	if (!f)			 // Если не удалось открыть файл,
		return NULL; // то возвращается NULL

	// Чтение числа строк, выход с возвратом NULL при неудаче
	uint16_t lines;
	if (fscanf(f, "%hu", &lines) <= 0)
		return NULL;

	// Выделение памяти под массив величин строк, возврат NULL при неудаче
	uint16_t *cols = (uint16_t *)malloc(sizeof(uint16_t) * lines);
	if (!cols)
		return NULL;

	/*Запись размеров каждой строки в массив размеров*/
	for (uint16_t i = 0; i < lines; ++i)
	{
		// Чтение числа строк, выход с возвратом NULL при неудаче
		if (fscanf(f, "%hu", cols + i) <= 0)
		{
			free(cols);
			return NULL;
		}
	}

	// Выделение памяти для матрицы, возврат NULL при неудаче
	double **p = RM_CreateArray(lines, cols);
	if (!p)
	{
		free(cols);
		return NULL;
	}

	// Чтение элементов из файла, возврат NULL при неудаче
	for (uint16_t i = 0; i < lines; ++i)
		for (uint16_t j = 0; j < cols[i]; j++)
			if ((fscanf(f, "%lf", p[i] + j)) <= 0)
			{
				free(cols);
				return NULL;
			}

	free(cols); // Освобождение памяти вектора размеров
	return p;	// Возврат матрицы
}

// Запись матрицы arr в файл f. Возвращает 0 при штатной работе, либо код ошибки
uint8_t RM_WriteBinary(double **arr, FILE *f)
{
	if (!f)				// Если файл некорректный
		return FILEERR; // возвращается код ошибки

	// Извлечение числа строк, запись
	uint16_t lines = RM_GetLineCount(arr);
	fwrite(&lines, sizeof(uint16_t), 1, f);

	// Вычисление числа записываемой памяти: размеры строк + сами элементы
	int mem = sizeof(uint16_t) * lines;
	for (uint16_t i = 0; i < lines; ++i) // Перебор размеров строк
		mem += RM_GetElCount(arr[i]) * sizeof(double); // Добавление размера каждой строки в счётчик

	// Запись с начала области элементов
	fwrite(int_GetMinusOne(arr[0]), 1, mem, f);

	return NOERR; // Возврат кода отсутствия ошибок
}

/*Чтение матрицы из бинарного источника f
 * возвращает NULL при ошибке*/
double **RM_ReadBinFile(FILE *f)
{
	if (!f)			 // Если источник равен NULL
		return NULL; // Завершение работы с возвратом NULL

	// Запись числа памяти для элементов матрицы и размеров
	fseek(f, 0, SEEK_END);
	int mem = ftell(f);
	rewind(f); // Перемотка файла в начало

	// Чтение числа элементов
	uint16_t lines;
	fread(&lines, sizeof(uint16_t), 1, f);
	mem += lines * sizeof(double *); // Добавление памяти для вектора указателей

	// Выделение памяти для массива, проверка работы malloc
	double **p = (double **)malloc(mem);
	if (!p)
		return NULL;

	p = (double **)((uint16_t *)p + 1); // Сдвиг указателя
	((uint16_t *)p)[-1] = lines;		// Запись количества строк

	// Побайтное чтение файла в массив после оставления места под вектор указателей
	fread(p + lines, 1, mem - sizeof(uint16_t), f);

	// Установка указателя первой строки
	p[0] = (double *)((void *)p +				   // Начала вектора указателей +
					  sizeof(uint16_t *) * lines + // Пространства под вектор указателей +
					  sizeof(uint16_t));		   // Пространства под размер строки

	// Установка остальных указателей относительно предыдущих
	for (uint16_t i = 1; i < lines; ++i)
	{
		p[i] = (double *)((void *)(p[i - 1]) +						 // Начало предыдущей строки	+
						  RM_GetElCount(p[i - 1]) * sizeof(double) + // Элементы предыдущей строки+
						  sizeof(uint16_t));						 // Размеры строки
	}
	return p; // Возврат указателя на матрицу
}

// Сохранение матрицы arr в текстовый сток f. Возвращает 0 при штатной работе, либо код ошибки
uint8_t RM_WriteTxtFile(double **arr, FILE *f)
{
	if (!f)				// Если сток некорректный
		return FILEERR; // Функция прерывается с возвратом ошибки

	// Извлечение числа строк, запись
	uint16_t lines = RM_GetLineCount(arr);
	fprintf(f, "%hu\n", lines);

	// Запись размеров каждой строки
	for (uint16_t i = 0; i < lines; ++i)
		fprintf(f, "%hu ", RM_GetElCount(arr[i]));
	fputc('\n', f);
	fputc('\n', f);

	// Сохранение непосредственно матрицы
	RM_Print(arr, "%lf ", f);

	// Возврат отсутсвия ошибки
	return NOERR;
}

