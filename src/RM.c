#include "RM.h"

// Получение адреса начала сегмента (строки или всего массива)
static void *int_GetMinusOne(void *p)
{
	return (void *)(((uint16_t *)p) - 1);
}

/*Функция считает сумму элементов данного массива arr из элементов Amount */
static uint16_t int_SumOfArray(uint16_t* arr, uint16_t Amount)
{
	/*Перебор элементов массива */
	uint16_t Sum = arr[0];
	for (uint16_t i = 1; i < Amount; ++i)
		Sum += arr[i];

	return Sum; // Возврат суммы
}

// Получение размера, который занимает матрица в памяти
static uint32_t int_GetMatrMem(double **M)
{
	uint16_t lines = RM_GetLineCount(M); // Число строк матрицы
	uint32_t mem = sizeof(uint16_t) +	 // Память под число строк
				   lines * (sizeof(uint16_t) + sizeof(double *));
	// Добавление памяти для вектора указателей и размеров каждой строки
	// Добавление памяти для самих элементов
	for (uint16_t i = lines; i; i--)
		mem += RM_GetElCount(M[i - 1]) * sizeof(double);

	return mem;
}

/*Расстановка указателей на строки матрицы p, если область значений уже существует*/
static void int_SetupPointers(double** p)
{
	// Установка указателя первой строки
	p[0] =
		(double*)((char*)p + // Начала вектора указателей +
			sizeof(uint16_t*) * RM_GetLineCount(p) + // Пространства под вектор указателей +
			sizeof(uint16_t)); // Пространства под размер строки

	// Установка остальных указателей относительно предыдущих
	for (uint16_t i = 1; i < RM_GetLineCount(p); ++i)
	{
		p[i] =
			(double*)((char*)(p[i - 1]) + // Начало предыдущей строки	+
				RM_GetElCount(p[i - 1]) * sizeof(double) + // Элементы предыдущей строки+
				sizeof(uint16_t));						  // Размеры строки
	}
}

/*Удаляет line строку из M. Возвращает код ошибки
ERR_GOTNULL, ERR_MALLOC, ERR_BADNUM*/
uint8_t RM_RemoveNthLine(double ***M, uint16_t line)
{
	if (!(*M))
		return ERR_GOTNULL;

	// Если номер удаляемой строки неверный, возврат соответствующего кода
	if (line >= RM_GetLineCount(*M))
		return ERR_BADNUM;

	// Если после удаления строки, матрицы не останется
	if (RM_GetLineCount(*M) == 1)
	{
		RM_Free(*M);
		*M = NULL;
		return ERR_NO;
	}

	/*Если в матрице несколько строк */

	/* Получение размеров матрицы в памяти, вычитание из него размера и
	указателя на вычитаемую строку */
	uint32_t mem_count = int_GetMatrMem(*M) - (sizeof(uint16_t) + sizeof(double *) +
											   sizeof(double) * RM_GetElCount((*M)[line]));

	// Выделение памяти под новую матрицу
	double **tmp = (double **)malloc(mem_count);
	if (!tmp)
		return ERR_MALLOC;

	// Запись числа элементов, сдвиг указателя
	uint16_t newLineCount = RM_GetLineCount(*M) - 1;
	*((uint16_t*)tmp) = newLineCount;
	tmp = (double **)((uint16_t*)tmp + 1);

	//Вспомогательные указатели. 
	//Указывают на область после указателей -- число элеметов нулевой строки
	double **pSrc = *M + RM_GetLineCount(*M);	//Старой матрицы
	double **pDest = tmp + newLineCount;		//Новой матрицы

	/* Подсчёт памяти, идущей перед удаляемой строкой, если строка нулевая,
	 * mem_count = 0 */
	mem_count = 0;
	for (uint16_t i = 0; i < line; ++i)
	{ // Добавление памяти размера строки и элементов этой строки
		mem_count += sizeof(uint16_t) + sizeof(double) * RM_GetElCount((*M)[i]);
	}

	// Если удаляется не первая линия
	if (mem_count) // то копируется область значений источника в новую матрицу,
	{	// в количестве mem_count
		memcpy(pDest, pSrc, mem_count); 
		// Сдвиг указателей на адреса после записанной области
		pDest = (double **)((char *)pDest + mem_count);
		pSrc = (double **)((char *)pSrc + mem_count);
	}

	// Сдвиг указателя на источник на адрес после пропускаемой строки
	pSrc = (double **)((uint16_t *)pSrc + 1);	//Сдвиг на размер строки
	pSrc = (double **)((double *)pSrc + RM_GetElCount((*M)[line]));//Сдвиг на число элементов
	
	/* Подсчёт памяти, идущей после удаляемой строки, если строка крайняя,
	 * mem_count = 0 */
	mem_count = 0;
	for (uint16_t i = line + 1; i < RM_GetLineCount(*M); ++i)
	{ // Добавление памяти размера строки и элементов этой строки
		mem_count += sizeof(uint16_t) + sizeof(double) * RM_GetElCount((*M)[i]);
	}
	// Если удаляется не крайняя линия
	if (mem_count) // то копируется область значений источника в новую матрицу,
		memcpy(pDest, pSrc, mem_count); // в количестве mem_count

	RM_Free(*M);
	*M = tmp;
	int_SetupPointers(*M);
	
	return ERR_NO;
}


/*Вставляет line строку в M. Возвращает код ошибки
ERR_GOTNULL, ERR_MALLOC, ERR_BADNUM*/
uint8_t RM_InsertNthLine(double*** M, uint16_t line, uint16_t lineLen)
{
	if (!(*M))
		return ERR_GOTNULL;
	// Если номер удаляемой строки неверный, возврат соответствующего кода
	if (line > RM_GetLineCount(*M))
		return ERR_BADNUM;

	/* Получение размеров матрицы в памяти, вычитание из него размера и
	указателя на вычитаемую строку */
	uint32_t mem_count = int_GetMatrMem(*M) + (sizeof(uint16_t) + sizeof(double*) +
		sizeof(double) * lineLen);

	// Выделение памяти под новую матрицу
	double** tmp = (double**)malloc(mem_count);
	if (!tmp)
		return ERR_MALLOC;

	// Запись числа элементов, сдвиг указателя
	uint16_t newLineCount = RM_GetLineCount(*M) + 1;
	*((uint16_t*)tmp) = newLineCount;
	tmp = (double**)((uint16_t*)tmp + 1);

	//Вспомогательные указатели. 
	//Указывают на область после указателей -- число элеметов нулевой строки
	double** pSrc = *M + RM_GetLineCount(*M);	//Старой матрицы
	double** pDest = tmp + newLineCount;		//Новой матрицы

	/* Подсчёт памяти, идущей перед вставляемой строкой, если строка нулевая,
	 * mem_count = 0 */
	mem_count = 0;
	for (uint16_t i = 0; i < line; ++i)
	{ // Добавление памяти размера строки и элементов этой строки
		mem_count += sizeof(uint16_t) + sizeof(double) * RM_GetElCount((*M)[i]);
	}
	// Если вставляется не первая линия
	if (mem_count) // то копируется область значений источника в новую матрицу,
	{	// в количестве mem_count
		memcpy(pDest, pSrc, mem_count);
		// Сдвиг указателей на адреса после записанной области
		pDest = (double**)((char*)pDest + mem_count);
		pSrc = (double**)((char*)pSrc + mem_count);
	}

	// Запись числа элементов, сдвиг указателя
	*((uint16_t*)pDest) = lineLen;
	pDest = (double**)((char *)pDest//Исходная позиция указателя
		+ sizeof(uint16_t)			//Сдвиг на размер
		+ sizeof(double) * lineLen);//Сдвиг на область элементов

	/* Подсчёт памяти, идущей после вставляемой строки, если строка крайняя,
	 * mem_count = 0 */
	mem_count = 0;
	for (uint16_t i = line; i < RM_GetLineCount(*M); ++i)
	{ // Добавление памяти размера строки и элементов этой строки
		mem_count += sizeof(uint16_t) + sizeof(double) * RM_GetElCount((*M)[i]);
	}
	// Если вставляется не крайняя линия
	if (mem_count) // то копируется область значений источника в новую матрицу,
		memcpy(pDest, pSrc, mem_count); // в количестве mem_count

	RM_Free(*M);
	*M = tmp;
	int_SetupPointers(*M);

	return ERR_NO;
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
	uint32_t mem =
		sizeof(double) * int_SumOfArray(cols, lines) + // Память под непосредственно элементы
		lines * (sizeof(uint16_t) +
				 sizeof(uint16_t *)) + // Память под размеры строк и вектор указателей
		sizeof(uint16_t); // Память под количество строк

	// Выделение памяти. Возврат NULL при ошибке
	double **p = (double **)malloc(mem);
	if (!p)
		return NULL;

	p = (double **)((uint16_t *)p + 1); // Сдвиг указателя
	((uint16_t *)p)[-1] = lines;		// Запись количества строк

	// Запись указателя на первую строку
	p[0] = (double *)((char *)p + // Начало вектора указателей +
					  sizeof(uint16_t *) * lines); // Пространство под вектор указателей

	((uint16_t *)p[0])[0] = cols[0]; // Запись числа элементов первой строки
	p[0] = (double *)((uint16_t *)(p[0]) + 1); // Сдвиг указателя

	// Расстановка указателей и запись размеров для всех остальных строк
	for (int i = 1; i < lines; ++i)
	{
		p[i] =
			(double *)((char *)p[i - 1] + // Адрес начала предыдущей строки +
					   (sizeof(double) * RM_GetElCount(p[i - 1]))); // элементы предыдущей строки

		((uint16_t *)p[i])[0] = cols[i]; // Запись числа элементов строки
		p[i] = (double *)((uint16_t *)(p[i]) + 1); // Сдвиг указателя
	}
	return p; // Возврат указателя
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
	if (!f)				 // Если файл некорректный
		return ERR_FILE; // возвращается код ошибки

	// Извлечение числа строк, запись
	uint16_t lines = RM_GetLineCount(arr);
	fwrite(&lines, sizeof(uint16_t), 1, f);

	// Вычисление числа записываемой памяти
	uint32_t mem = int_GetMatrMem(arr) - (sizeof(uint16_t) + lines * sizeof(double *));

	// Запись с начала области элементов
	fwrite(int_GetMinusOne(arr[0]), 1, mem, f);

	return ERR_NO; // Возврат кода отсутствия ошибок
}

// Функция копирования матрицы M
double **RM_CopyMatrix(double **M)
{
	if (!M)			 // Если источник равен NULL
		return NULL; // Завершение работы с возвратом NULL

	// Вычисление необходимого объёма матрицы
	uint32_t mem = int_GetMatrMem(M);
	uint16_t lines = RM_GetLineCount(M);

	// Выделение памяти для массива, проверка работы malloc
	double **p = (double **)malloc(mem);
	if (!p)
		return NULL;

	p = (double **)((uint16_t *)p + 1); // Сдвиг указателя
	((uint16_t *)p)[-1] = lines;		// Запись количества строк

	/* Копирование области значений из одной матрицы в другую,
	число байт равно числу выделяемой памяти минус вектор указателей минус число
	строк матрицы */
	memcpy(p + lines, M + lines, mem - (lines * sizeof(double) + sizeof(uint16_t)));

	int_SetupPointers(p);
	return p; // Возврат указателя на матрицу
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

	// Побайтное чтение файла в массив после оставления места под вектор
	// указателей
	fread(p + lines, 1, mem - sizeof(uint16_t), f);

	int_SetupPointers(p);
	return p; // Возврат указателя на матрицу
}

// Сохранение матрицы arr в текстовый сток f. Возвращает 0 при штатной работе,
// либо код ошибки
uint8_t RM_WriteTxtFile(double **arr, FILE *f)
{
	if (!f)				 // Если сток некорректный
		return ERR_FILE; // Функция прерывается с возвратом ошибки

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
	return ERR_NO;
}
