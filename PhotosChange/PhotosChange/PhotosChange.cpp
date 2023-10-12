#include <locale> 
#include<stdio.h> 
#include <iomanip> 
#include <iostream> 
using namespace std;
#pragma warning(disable: 4996)
#pragma pack(push,1) 
struct BITMAPFILEHEADER//заголовок файла 
{
	unsigned short Type; // формат ‘BM’ 0x4D42 
	unsigned long Size; // размер файла в байтах 
	unsigned short Reserved1; // всегда равен 0, зарезервирован 
	unsigned short Reserved2; // всегда равен 0, зарезервирован 
	unsigned long OffsetBits; // смещение данных от начала файла в байтах, по умолчанию равен 54 для BITMAPFILEGEADER(40 + 14)
		//где 40 - sizeof(BITMAPINFOHEADER), а 14 - sizeof(BITMAPFILEHEADER) 
};
#pragma pack(pop) 
#pragma pack(push,1) 
struct BITMAPINFOHEADER
{
	unsigned long Size; // число байтов, выделенное для структуры. Всегда равно 40 
	unsigned long Width; // ширина точечного рисунка в пикселях 
	unsigned long Height; // высота точечного рисунка в пикселях 
	unsigned short Planes; // число плоскостей целевого устройства. Всегда равно 1 
	unsigned short BitCount; // глубина цвета (число бит на точку). Может быть равно 0, 1, 4, 8, 16(палитровые), 24 или 32 бита(беспалитровые)
	unsigned long Compression; // тип сжатия. Равен 0 для несжатого изображения 
	unsigned long SizeImage; // размер изображения в байтах. Равен BitCount*Height*Width 
	unsigned long XPelsPerMeter; // разрешающая способность по горизонтали 
	unsigned long YPelsPerMeter; // разрешающая способность по вертикали 
	unsigned long ColorUsed; // число индексов используемых цветов.Если присутствует все цвета индекс равен 0
	unsigned long ColorImportant; // число необходимых цветов. Всегда равно 0 
};
#pragma pack(pop) 
#pragma pack(push,1) 
struct RGBTRIPLE//массив структур RGBTRIPLE(хранит 3 байта, в каждом из которых по 8 бит) 
{
	unsigned char Blue;//1 байт 
	unsigned char Green;//2 байт 
	unsigned char Red;//3 байт 
};
#pragma pack(pop) 
#pragma pack(push,1) 
struct RGBQUAD//массив структур RGBQUAD(хранит 4 байта, в каждом из которых по 8 бит) 
{
	unsigned char Blue;
	unsigned char Green;
	unsigned char Red;
	unsigned char Reserved;
};
#pragma pack(pop) 
//-------------------------------------------------------ЗАДАНИЕ 1.1------------------------------------------------------------ 
class Image
{
	BITMAPINFOHEADER bm; // заголовок изображения 
	RGBQUAD* Rgb; // одномерный массив с описанием пикселей типа структуры RGBQUAD 
	RGBQUAD* palette; // палитра изображения(если глубина цвета равна 1,4,8,16 бит) 
	void Paremeters(); // параметры пустого изображения 
	void BMParameters(int Width, int Height, int BitCount); //параметры bm + выделение памяти под изображение(одномерный массив) + создание и заполнение палитры(по формату)
	void CopyData(const Image& Inp); // если bm совпадают изображение копируется
	void ConvertData(const Image& Inp); // копирование и преобразование данных изображения в нужный формат
	int Remains(int Width, int Height, unsigned short BitCount) // кол-во байт для дозаписи в файл 
	{
		int remainder = (Width * BitCount / 8) % 4; //остаток от деления длины строки на 4 
		return (Width * BitCount / 8) + remainder ? 4 - remainder : 0 * Height;//кол-во байт для кратности 4 * кол - во строк
	}
	unsigned char get(RGBTRIPLE color) //градация серого для переданного цвета 
	{
		int result = color.Red * 0.299 + color.Green * 0.597 + color.Blue * 0.114; //формула яркости изображения
		if (result > 255) // если кол-во цветов больше, чем максимально возможное для байта 
		{
			result = 255;//присваивание байту максимально возможное кол-во цветов 
		}
		return (unsigned char)result;
	}
	unsigned char get(RGBQUAD color) //градация серого для переданного цвета 
	{
		int result = color.Red * 0.299 + color.Green * 0.597 + color.Blue * 0.114; //формула яркости изображения
		if (result > 255) // если кол-во цветов больше, чем максимально возможное для байта 
		{
			result = 255;//присваивание байту максимально возможное кол-во цветов 
		}
		return (unsigned char)result;
	}
	unsigned char Color(unsigned char color) // поиск ближайшего цвета в палитре 
	{
		int minIndex = 0; //минимальный индекс 
		int maxIndex = bm.ColorUsed - 1; //максимальный индекс 
		while (maxIndex >= minIndex) // поиск до тех пор пока minIndex не станет больше или равен maxIndex
			//предполагаем, что Palette[minIndex - 1]<color<Palette[minIndex] пока не найдется ближайший к color minindex
		{
			int middleIndex = (minIndex + maxIndex) / 2; // средний индекс 
			if (palette[middleIndex].Red == color) // равен ли индекс в палитре искомому значению 
			// Palette[middleIndex].Red == Palette[middleIndex].Green == Palette[middleIndex].Blue - т.к палитра состоит из оттенков серого
			{
				return middleIndex; // нужный индекс палитры 
			}
			else if (palette[middleIndex].Red < color) // если индекс палитры меньше color, сдвиг левого края поиска
				/*
				если ближайшего к color значения нет в палитре, то а предпоследней итерации
				minIndex == middleIndex == maxIndex - 1
				поэтому Palette[middleIndex].Red < color < Palette[middleIndex + 1].Red.
				На последнем цикле minindex=maxindex, потом maxIndex = minIndex - 1 и
				Palette[minIndex - 1]<color<Palette[minIndex]
				*/
			{
				minIndex = middleIndex + 1;//значение индекса палитры по сдвигу левого края поиска если color больше чем текущий индекс палитры
			}
			else
			{
				maxIndex = middleIndex - 1; //значение индекса палитры по сдвигу правого края поиска, если color меньше чем текущий индекс палитры
			}
		}
		if (minIndex == bm.ColorUsed) // переданного значение больше максимального в палитре 
		{
			return bm.ColorUsed - 1; //индекс наибольшего значения палитры 
		}
		if (minIndex == 0) // minindex не менялся => сдвига по левому краю не было, поэтому color меньше minindex
		{
			return 0; //возвращаем minindex палитры 
		}
		//если minindex<color<maxindex и сдвига по правому краю не было, то Palette[minIndex - 1] < color < Palette[minIndex]
		int rem = color - palette[minIndex - 1].Red; // ближайшее к color значение по левому краю 
		int next = palette[minIndex].Red - color;//ближайшее к color значение по правому краю 
		return rem < next ? minIndex - 1 : minIndex; //индекс ближайшего к color значения 
	}
public:
	Image(char Mode, unsigned short BCount, int Width, int Height); // конструктор создания изображения 
	Image(const char* fileName); // конструктор объекта изображения из файла 
	Image(); // конструктор без параметров-пустой контейнер под изображение 
	Image(const Image& i); // конструктор по создании копии изображения 
	~Image(); // деструктор 
	int loadimage(const char* fileName); // метод загрузки изображения 
	void writeimage(const char* fileName); // метод записи изображения в файл 
	Image& operator = (const Image& Inp); // перегрузка оператора = 
	Image operator / (short Depth); // перегрузка оператора / 
	Image& operator /= (const Image& Inp); // перегрузка оператора /= 
};
//ДОПОЛНИТЕЛЬНЫЕ МЕТОДЫ 
//Заполнение заголовка изображения + выделение памяти под одномерный массив+ создание и заполнение палитры
void Image::BMParameters(int Width, int Height, int BitCount)
{
	bm.Width = Width; // ширина точечного рисунка в пикселях 
	bm.Height = Height; // высота точечного рисунка в пикселях 
	bm.Planes = 1; // число плоскостей целевого устройства 
	bm.BitCount = BitCount; //глубина цвета, число бит на точку 
	bm.SizeImage = Remains(Width, Height, BitCount); //размер изображения в байтах 
	if (bm.BitCount <= 8)//если глубина цвета изображения меньше 8 
	{
		bm.ColorUsed = 1 << bm.BitCount; //кол-во возможных цветов палитры - 2 в степени bm.BitCount
		palette = new RGBQUAD[bm.ColorUsed]; // выделение памяти под палитру цветов 
		for (int i = 0; i < bm.ColorUsed; i++) // заполнение значений палитры градациями серого от 0 до 255
		{
			unsigned char color = (255 * i / (bm.ColorUsed - 1));
			palette[i].Red = color;
			palette[i].Green = color;
			palette[i].Blue = color;
			palette[i].Reserved = 0;
		}
	}
	if (bm.SizeImage > 0)//размер изображения больше 0 
	{
		//выделение памяти под одномерный массив с размерами bm.Height * bm.Width
		Rgb = new RGBQUAD[bm.Height * bm.Width];
	}
	else
	{
		//массив пуст 
		Rgb = NULL;
	}
}
// Параметры пустого изображения 
void Image::Paremeters()
{
	Rgb = NULL;//указатель на одномерный массив равен 0 
	palette = NULL;//указатель на палитру равен 0 
	bm.Size = 40;
	bm.Width = 0;
	bm.Height = 0;
	bm.Planes = 0;
	bm.BitCount = 0;
	bm.Compression = 0;
	bm.SizeImage = 0;
	bm.XPelsPerMeter = 0;
	bm.YPelsPerMeter = 0;
	bm.ColorUsed = 0;
	bm.ColorImportant = 0;
}
//Копирование изображений 
void Image::CopyData(const Image& Inp)
{
	if (bm.Width == Inp.bm.Width && bm.Height == Inp.bm.Height && bm.BitCount ==
		Inp.bm.BitCount)//если совпадают разрешение и битность изображений 
	{
		cout << "Разрешение и битность изображений совпадают. Можно копировать данные" << endl;
		for (int i = 0; i < bm.Height; i++)
		{
			for (int j = 0; j < bm.Width; j++)
			{
				memcpy(&Rgb[i * bm.Width + j], &Inp.Rgb[i * bm.Width + j],
					sizeof(RGBQUAD));//копирование пикселей изображения в ширину и высоту 
			}
		}
	}
	else//если разрешение и битность изображений не одинаковы 
	{
		cout << "Ошибка! Данные нельзя копировать, так как изображения имеют разное разрешение и битность" << endl;
	}
}
//Реализация метода преобразования данных переданного изображения в текущий формат 
void Image::ConvertData(const Image& Inp)
{
	if (bm.Width != Inp.bm.Width && bm.Height != Inp.bm.Height)//если разрешения и битность изображений совпадают, то данные копируются
	{
		cout << "Ошибка! При конвертировании битности изображений их разрешения должны быть равны" << endl;
		return;
	}
	else
		cout << "Битность и разрешения изображений одинаковы" << endl;
	if (Inp.bm.BitCount < bm.BitCount)//уменьшается ли битность не исходного изображения 
	{
		cout << "Ошибка! Битность второго изображения можно только уменьшить" << endl;
		return;
	}
	else
		cout << "Битность второго изображения уменьшилась" << endl;
	const bool s = Inp.palette != NULL; //имеется ли в исходном изображении палитра 
	for (int i = 0; i < bm.Height; i++)
	{
		for (int j = 0; j < bm.Width; j++)
		{
			// для исходного изображения берется оттенок серого с палитры. Если его нет, то он вычисляется
			unsigned char koj = s ? Inp.Rgb[i * bm.Width + j].Red : get(Inp.Rgb[i * bm.Width + j]);
			koj = palette[Color(koj)].Red; //значение серого в палитре второго изображения 
			// записывается оттенок серого в пиксель второго изображения 
			Rgb[i * bm.Width + j].Red = koj;
			Rgb[i * bm.Width + j].Green = koj;
			Rgb[i * bm.Width + j].Blue = koj;
			Rgb[i * bm.Width + j].Reserved = 0;
		}
	}
}
//ОСНОВНЫЕ МЕТОДЫ 
//-------------------------------------------------------ЗАДАНИЕ 1.3------------------------------------------------------------ 
//Реализация конструктора объекта изображения из файла 
Image::Image(const char* fileName)
{
	Paremeters(); //заполнение параметров пустого изображения 
	loadimage(fileName); // загрузка файла с именем в качестве входного параметра 
}
// Реализация метода загрузки изображения 
int Image::loadimage(const char* fileName)
{
	if (Rgb)//если одномерный массив заполнен данными изображения 
	{
		cout << "Ошибка! Невозможно загрузить данные в созданное изображение" << endl;
		return 0;
	}
	else
		cout << "Данные можно загрузить, так как изображение еще не создано" << endl;
	FILE* f;
	f = fopen(fileName, "rb"); // открытие бинарного файла 
	if (!f)//если бинарный файл не открыт 
	{
		cout << "Ошибка! Файл " << fileName << " не прочитан" << endl;
		return 0;
	}
	else
		cout << "Файл " << fileName << " прочитан" << endl;
	// ЗАГРУЗКА ИЗОБРАЖЕНИЯ ИЗ ПЕРЕДАННОГО ФАЙЛА 
	BITMAPFILEHEADER bf;//объявление заголовка файла 
	//СЧИТЫВАНИЕ ЗАГОЛОВКА ФАЙЛА ВНУТРИ ФУНКЦИИ(ПУНКТ 1) 
	if (fread(&bf, sizeof(BITMAPFILEHEADER), 1, f) != 1)//если заголовок файла не считан 
	{
		cout << "Ошибка! Заголовок файла изображения не считан" << endl;
		//ВОЗВРАЩЕНИЕ 0 В СЛУЧАЕ ОШИБКИ(ПУНКТ 5) 
		return 0;
	}
	else
		cout << "Заголовок файла изображения считан" << endl;
	if (bf.Type != 0x4D42)//если файл имеет формат BMP(проверка сигнатуры) 
	{
		cout << "Ошибка! Неправильный формат файла" << endl;
		//ВОЗВРАЩЕНИЕ 0 В СЛУЧАЕ ОШИБКИ(ПУНКТ 5) 
		return 0;
	}
	BITMAPINFOHEADER fileb;//объявление заголовка изображения 
	//СЧИТЫВАНИЕ ЗАГОЛОВКА ИЗОБРАЖЕНИЯ В ДАННЫХ ОБЪЕКТА(ПУНКТ 2) 
	if (fread(&fileb, sizeof(BITMAPINFOHEADER), 1, f) != 1)//если заголовок изображения не считан 
	{
		cout << "Ошибка! Информация об изображении не считана" << endl;
		//ВОЗВРАЩЕНИЕ 0 В СЛУЧАЕ ОШИБКИ(ПУНКТ 5) 
		return 0;
	}
	else
		cout << "Информация об изображении считана" << endl;
	if (!(fileb.BitCount == 24 || fileb.BitCount == 8 || fileb.BitCount == 4 || fileb.BitCount == 32))//если битность изображения в файле равна 24, 4, 8 или 32 битам
	{
		cout << "Ошибка! Битность изображения " << (int)fileb.BitCount << " недопустима" << endl;
		//ВОЗВРАЩЕНИЕ 0 В СЛУЧАЕ ОШИБКИ(ПУНКТ 5) 
		return 0;
	}
	else
		cout << "Битность изображения " << (int)fileb.BitCount << " допустима" << endl;
	//ВЫДЕЛЕНИЕ ПАМЯТИ ПОД МАССИВ ДАННЫХ ИЗОБРАЖЕНИЯ,НА КОТОРЫЙ УКАЗЫВАЕТ RGBQUAD
	BMParameters(fileb.Width, fileb.Height, fileb.BitCount);//заполняется заголовок + выделяется память под изображение в файле
	RGBQUAD* filePalette = NULL;//создание новой пустой палитры 
	if (bm.BitCount <= 8)//если изображение палитровое(1,4,8,16 бит) 
	{
		filePalette = new RGBQUAD[bm.ColorUsed];//выделение памяти под палитру 
		fread(filePalette, sizeof(RGBQUAD), bm.ColorUsed, f);//считывание палитры 
	}
	fseek(f, bf.OffsetBits, SEEK_SET);//смещение к началу данных изображения 
	int res = (fileb.Width * fileb.BitCount / 8) % 4;//остаток от деления длины строки на 4 
	int k = res ? 4 - res : 0 * fileb.Height;//смещение между пикселями 
	int cb = bm.BitCount * bm.Width;//кол-во битов в одной строке 
	int c4 = ((cb + 31) >> 5) << 2;//кол-во битов в строке чтобы было кратно 4 байтам 
	int add = ((c4 << 3) - cb) >> 3;//сколько добавить битов, чтобы было кратно 4 
	if (fileb.BitCount > 8 && fileb.BitCount != 16)//если битность изображения больше 8 и не равно 16 
	{
		cout << "Изображение в файле беспалитровое" << endl;
		char* ad = new char[add];//массив, содержащий кол-во бит,которые нужно добавить чтобы строка была кратна 4
		for (int i = bm.Height - 1; i >= 0; i--)//пиксели изображения считываются снизу вверх в BMP файле
		{
			for (int j = 0; j < bm.Width; j++)//строки считываютс слева направо в BMP файле 
			{
				//ПОСЛЕДОВАТЕЛЬНОЕ СЧИТЫВАНИЕ ДАННЫХ ИЗОБРАЖЕНИЯ RGBQUAD(ПУНКТ 4)
				if (fileb.BitCount == 24)//если битность изображения равна 24 
				{
					RGBTRIPLE triple;//объект типа массивы структур RGBTRIPLE
					fread(&triple, sizeof(RGBTRIPLE), 1, f);//считывание информации в объекте типа RGBTRIPLE
					Rgb[i * bm.Width + j].Red = triple.Red;
					Rgb[i * bm.Width + j].Green = triple.Green;
					Rgb[i * bm.Width + j].Blue = triple.Blue;
					Rgb[i * bm.Width + j].Reserved = 0;
				}
				else if (fileb.BitCount == 32)//если битность изображения равна 32 
				{
					fread(&Rgb[i * bm.Width + j], sizeof(RGBQUAD), 1, f);//считывание данных в массиве
					if (k)//если смещение между пикселями имеется 
					{
						fseek(f, k, SEEK_CUR);//считываем смещение 
					}
				}
			}
			if (add)//если имеются биты,которые не считаны 
			{
				fread(ad, add, 1, f);//считать оставшиеся биты 
			}
		}
	}
	else if (fileb.BitCount <= 8 || fileb.BitCount == 16) // если изображение палитровое 
	{
		cout << "Изображение в файле палитровое" << endl;
		const unsigned char q = 8 - bm.BitCount; // кол-во бит для достижения старших байтов 
		unsigned char m = (1 << bm.BitCount) - 1; // младшие биты выставляются в 1 - маска цвета 
		m <<= 8 - bm.BitCount; // единицы перемещаются в старшие биты 
		for (int i = bm.Height - 1; i >= 0; i--)//пиксели просматриваются снизу вверх 
		{
			int n = 0; // кол-во непрочитанных бит в байте 
			unsigned char t = 0; // текущий считанный байт в изображении 
			for (int j = 0; j < bm.Width; j++)//пиксели просматриваются слева направо по ширине 
			{
				if (!n) //если все биты в байте считаны 
				{
					fread(&t, 1, 1, f);//считывание битов в следующем байте 
					n = 8;
				}
				int in = (t & m) >> q; // текущий индекс в палитре, полученный через сдвиг и применение битовой маски
				n -= bm.BitCount; //обновление кол-ва необработанных бит 
				t <<= bm.BitCount; // перемещение кол-ва необработанных ит в старшую часть байта
				unsigned char s = get(filePalette[in]); //оттенок серого для цвета пикселя из файла 
				unsigned char p = palette[Color(s)].Red; // ближайший к оттенку серого цвет из палиты
				// заполняем данные изображения 
				Rgb[i * bm.Width + j].Red = p;
				Rgb[i * bm.Width + j].Green = p;
				Rgb[i * bm.Width + j].Blue = p;
				Rgb[i * bm.Width + j].Reserved = 0;
			}
			if (k)//если имеется смещение между пикселями изображения 
			{
				fseek(f, k, SEEK_CUR);//считать смещение 
			}
		}
	}
	if (filePalette)//если палитра файла была создана 
	{
		delete[] filePalette;//удалить файловую палитру 
	}
	return 1;
	fclose(f);//закрытие файла 
}
//-------------------------------------------------------ЗАДАНИЕ 1.4------------------------------------------------------------ 
// Реализация конструктора создания изображения 
Image::Image(char Mode, unsigned short BCount, int Width, int Height)
{
	//Заполнение структуры BITMAPINFOHEADER в зависимости от входных параметров Width, Height(ПУНКТ 1)
	Paremeters(); //заполнение параметров пустого изображения 
	if (BCount == 24 || BCount == 8 || BCount == 4 || BCount == 32)//если битность изображения равна 24,4,8 или 32 битам
	{
		cout << "Изображение задано с допустимой битностью,равной " << BCount << " бит" << endl;
		//ВЫДЕЛЕНИЕ ПАМЯТИ ПОД МАССИВ ДАННЫХ ИЗОБРАЖЕНИЯ НА КОТОРЫЙ УКАЗЫВАЕТ RGBQUAD(ПУНКТ 2)
		BMParameters(Width, Height, BCount); //заполнение заголовка изображения + выделение памяти под одномерный массив + создание и заполнение палитры
		cout << "Выделена память под одномерный массив и заполнен заголовок изображения" << endl;
		if (palette)//если палитра была создана 
		{
			cout << "Палитра была создана" << endl;
			Mode = palette[Color(Mode)].Red; //ближайший к переданному цвет в палитре 
		}
		else
			cout << "Палитра не создана" << endl;
		// заполнение данных изображения ближайшим цветом в палитре 
		//ЗАПИСЬ ДАННЫХ ИЗОБРАЖЕНИЯ В ЗАВИСИМОСТИ ОТ ПАРАМЕТРА MODE(ПУНКТ 3)
		for (int i = 0; i < bm.Height; i++)
		{
			for (int j = 0; j < bm.Width; j++)
			{
				Rgb[i * Width + j].Red = Mode;
				Rgb[i * Width + j].Green = Mode;
				Rgb[i * Width + j].Blue = Mode;
				Rgb[i * Width + j].Reserved = 0;
			}
		}
	}
	else//битность изображения не равна 24,4,8 или 32 битам 
		cout << "Ошибка! Можно создать изображение только на 4,8,24 или 32 бита" << endl;
}
//-------------------------------------------------------ЗАДАНИЕ 1.5------------------------------------------------------------ 
//Реализация метода записи данных изображения в файл 
void Image::writeimage(const char* fileName)
{
	if (!Rgb)//если данные изображения отсутствуют 
	{
		cout << "Ошибка! Изображение не имеет данных для сохранения" << endl;
		return;
	}
	else
		cout << "Изображение имеет данные для сохранения" << endl; FILE* f;
	//ОТКРЫТИЕ ФАЙЛА fileName ДЛЯ ЗАПИСИ(ПУНКТ 1) 
	f = fopen(fileName, "w+b"); // открытие бинарного файла 
	if (!f)//если файл не открыт 
	{
		cout << "Ошибка! Файл " << fileName << " не создан" << endl;
		return;
	}
	else
		cout << "Файл " << fileName << " создан" << endl;
	// СОХРАНЯЕТ ДАННЫЕ В ФАЙЛ 
	int res = (bm.Width * bm.BitCount / 8) % 4;//остаток от деления длины строки на 4 
	int k = res ? 4 - res : 0 * bm.Height;//смещение между пикселями 
	//ВЫЧИСЛЕНИЕ ПОЛЕЙ И ЗАПОЛНЕНИЕ СТРУКТУРЫ BITMAPFILEHEADER(ПУНКТ 2) 
	BITMAPFILEHEADER bk;//объявление заголовка файла 
	bk.Type = 0x4D42; // сигнатура 
	bk.OffsetBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);// смещение до данных изображения
	//полный размер файла = сумма размера всех заголовков, палитры и данных файла 
	bk.Size = bk.OffsetBits + Remains(bm.Width, bm.Height, bm.BitCount);
	bk.Reserved1 = 0; // всегда 0 
	bk.Reserved2 = 0; // всегда 0 
	if (palette) // если есть палитра 
	{
		const int paletteSize = sizeof(RGBQUAD) * bm.ColorUsed; // размер палитры 
		bk.Size += paletteSize; // размер палитры добавляется к размеру файла 
		bk.OffsetBits += paletteSize; //к смещению до данных изображения 
	}
	//ЗАПИСЬ СТРУКТУРЫ BITMAPFILEHEADER В ФАЙЛ(ПУНКТ 2) 
	if (fwrite(&bk, sizeof(BITMAPFILEHEADER), 1, f) != 1)//если заголовок файла не записан 
	{
		cout << "Ошибка! Информация в заголовке файла изображения не записана" << endl;
		return;
	}
	else
		cout << "Информация в заголовке файла изображения записана" << endl;
	//ЗАПИСЬ СТРУКТУРЫ BITMAPINFOHEADER В ФАЙЛ(ПУНКТ 3) 
	if (fwrite(&bm, sizeof(BITMAPINFOHEADER), 1, f) != 1)//если заголовок файла записан 
	{
		cout << "Ошибка! Информация в заголовке изображения не записана" << endl;
		return;
	}
	else
		cout << "Информация в заголовке изображения записана" << endl;
	int cb = bm.BitCount * bm.Width;//кол-во битов в одной строке 
	int c4 = ((cb + 31) >> 5) << 2;//кол-во битов в строке чтобы было кратно 4 байтам 
	int add = ((c4 << 3) - cb) >> 3;//сколько добавить битов, чтобы было кратно 4 
	//ПОСЛЕДОВАТЕЛЬНАЯ ЗАПИСЬ ДАННЫХ ИЗОБРАЖЕНИЯ В ФАЙЛ(ПУНКТ 4) 
	if (bm.BitCount > 8 && bm.BitCount != 16)//если битность изображения больше 8 и не равно 16 
	{
		cout << "Изображение беспалитровое" << endl;
		char* ad = new char[add];//массив, содержащий кол-во бит,которые нужно добавить чтобы строка была кратна 4
		for (int i = bm.Height - 1; i >= 0; i--)
		{
			for (int j = 0; j < bm.Width; j++)
			{
				if (bm.BitCount == 24)//если битность изображения равна 24 
				{
					fwrite(&(Rgb[i * bm.Width + j].Red), 1, 1, f);
					fwrite(&(Rgb[i * bm.Width + j].Green), 1, 1, f);
					fwrite(&(Rgb[i * bm.Width + j].Blue), 1, 1, f);
					fwrite(&(Rgb[i * bm.Width + j].Reserved), 0, 0, f);
				}
				else if (bm.BitCount == 32)//если битность изображения равна 32 
				{
					fwrite(&Rgb[i * bm.Width + j], sizeof(RGBQUAD), 1, f);//запись данных из массива
					if (k)//если смещение между пикселями имеется 
					{
						const int Zero = 0;
						fwrite(&Zero, 1, k, f);//запись смещений между пикселями 
					}
				}
			}
			if (ad)//если имеется кол-во оставшихся бит 
			{
				fwrite(&ad, add, 1, f);//запись оставшихся бит 
			}
		}
	}
	else if (bm.BitCount <= 8 || bm.BitCount == 16) // изображение с палитрой 
	{
		if (fwrite(palette, sizeof(RGBQUAD), bm.ColorUsed, f) != bm.ColorUsed)//если палитра не записана
		{
			cout << "Ошибка! Палитра изображения не записана" << endl;
			return;
		}
		else
			cout << "Палитра изображения записана" << endl;
		const int s = 8 - bm.BitCount;//смещение для перемещения индекса цвета 1 пикселя в старшие биты
		for (int i = bm.Height - 1; i >= 0; i--)//пиксели записываются снзу вверх 
		{
			int c = s; //на сколько сместить следующее значение при записи в байт 
			int n = 8; //количество возможных бит в байте 
			int p = 0; //текущее значение для записи в файл 
			for (int j = 0; j < bm.Width; j++)//пиксели записываются слева направо 
			{
				unsigned char h = Color(Rgb[i * bm.Width + j].Red); // индекс палитры текущего цвета
				p |= h << c; // смещение на кол-во бит и запись в результат 
				c -= bm.BitCount; // уменьшение требуемого сдвига для следующего индека 
				n -= bm.BitCount; // уменьшение кол-ва оставшихся бит 
				if (!n)//если нет свободных бит 
				{
					fwrite(&p, 1, 1, f);//собранный байт записывается в файл 
					//выставление начальных параметров 
					c = s;
					n = 8;
					p = 0;
				}
			}
			if (n != 8)//если остались незаписанные биты(байт не заполнился) 
			{
				fwrite(&p, 1, 1, f);//запись незаписанный бит в файл} 
				if (k)//если имеется кол-во бит, кратных 4 
				{
					const int Zero = 0;
					fwrite(&Zero, 1, k, f);//запись таких бит 
				}
			}
		}
		//ЗАКРЫТИЕ ФАЙЛА(ПУНКТ 5) 
		fclose(f);
	}
}
//-------------------------------------------------------ЗАДАНИЕ 1.6------------------------------------------------------------ 
// Реализация конструктора без параметров 
Image::Image()
{
	Paremeters(); //заполнение параметров пустого изображения 
}
//-------------------------------------------------------ЗАДАНИЕ 1.7------------------------------------------------------------ 
//Реализация конструктора копий изображения 
Image::Image(const Image& im)
{
	Paremeters(); // заполнение параметров пустого изображения 
	//ВЫДЕЛЕНИЕ ПАМЯТИ ПОД ДАННЫЕ КОПИИ ИЗОБРАЖЕНИЯ(ПУНКТ 1) 
	BMParameters(im.bm.Width, im.bm.Height, im.bm.BitCount); // заполнение заголовка изображения + выделение памяти
		//ПРОВЕРКА ФОРМАТА ИЗОБРАЖЕНИЯ И КОПИРОВАНИЕ ДАННЫХ ИЗОБРАЖЕНИЯ(ПУНКТ 2) 
	CopyData(im); // копирование данных изображения 
}
//Реализация прегрузки оператора = 
Image& Image::operator = (const Image& Inp)
{
	if (Rgb) // если изображение уже создано 
	{
		CopyData(Inp); // копируем данные при совпадении форматов изображения 
	}
	else//если изображение не создано 
	{
		Paremeters(); // заполнение параметров пустого изображения 
		//ВЫДЕЛЕНИЕ ПАМЯТИ ПОД ДАННЫЕ КОПИИ ИЗОБРАЖЕНИЯ(ПУНКТ 1) 
		BMParameters(Inp.bm.Width, Inp.bm.Height, Inp.bm.BitCount); // заполнение заголовка изображения + выделение памяти
			//ПРОВЕРКА ФОРМАТА ИЗОБРАЖЕНИЯ И КОПИРОВАНИЕ ДАННЫХ ИЗОБРАЖЕНИЯ(ПУНКТ 2)
		CopyData(Inp); // копирование данных изображения 
	}
	return *this;
}
//-------------------------------------------------------ЗАДАНИЕ 1.8------------------------------------------------------------ 
//Реализация деструктора 
Image::~Image()
{
	if (Rgb)//если массив соержит данные об изображении 
	{
		delete[] Rgb; // очищаем память, содержащую указатель на данные изображения 
		Rgb = NULL; // указатель на данные изображения равен 0 
		// (не обязательно, так как пороисходит в деструкторе) 
	}
	if (palette) // если палитра создана{ 
		delete[] palette;//удаляем палитру 
	palette = NULL;//указатель на палитру приравнивается к 0 
}

//-------------------------------------------------------ЗАДАНИЕ 2.1------------------------------------------------------------ 
//Реализация перегрузки оператора /=. Метод записывает переданное изображение в текущее с изменением размера
Image& Image::operator /= (const Image& Inp)
{
	// проверка совпадения форматов 
	if (bm.BitCount != Inp.bm.BitCount)//если битность переданного изображения не совпадает с битностью текущего
	{
		cout << "Ошибка! Изображения имеют разную битность, поэтому измененить размер второго изображения невозможно" << endl;
		return *this;
	}
	else
		cout << "Изображения имеют одинаковую битность, поэтому можно изменить размер второго изображения" << endl;
	float y = (float)Inp.bm.Width / bm.Width;//соотношение ширины переданного изображения по отношению к текущему
	float x = (float)Inp.bm.Height / bm.Height;//соотношение высоты переданного изображения по отношению к текущему
	for (int i = 0; i < bm.Height; i++)
	{
		for (int j = 0; j < bm.Width; j++)
		{
			int Y = (int)(j * y);//X координата в исходном изображении 
			int X = (int)(i * x);//Y координата в исходном изображении 
			Rgb[i * bm.Width + j] = Inp.Rgb[X * Inp.bm.Width + Y];//запись значения цвета из исходного изображения в текущее
		}
	}
	return *this;
}
//-------------------------------------------------------ЗАДАНИЕ 2.3------------------------------------------------------------ 
// Реализация перегрузки оператора /. Метод возвращает второе изображение-копию исходного с битностью Depth
Image Image::operator / (short Depth)
{
	if (!(Depth == 24 || Depth == 8 || Depth == 4 || Depth == 32))//равна ли битность изображения 24,4,8 или 32 бита
	{
		cout << "Ошибка! Битость не допустима" << endl;
		return Image(*this);
	}
	else
		cout << "Битность допустима" << endl;
	if (Depth > bm.BitCount)//если битность второго изображения больше исходного 
	{
		cout << "Ошибка! Битность второго изображения можно только уменьшить" << endl;
		return Image(*this);
	}
	else
		cout << "Битность второго изображения уменьшена" << endl;
	Image result(0, Depth, bm.Width, bm.Height); // создание пустого изображения 
	result.ConvertData(*this); // преобразование исходного изображения во второе69 
	return result;
}
int main()
{
	setlocale(LC_ALL, "ru");
	/*
	ПРИМЕР №1
	ИЗМЕНЕНИЕ РАЗМЕРА ИЗОБРАЖЕНИЯ И ГЛУБИНЫ ЦВЕТА НА 4 БИТА
	исходное изображение на 24 бита с размером 810*1013
	*/
	Image k("naruto_24.bmp"); //загрузка изображения в k 
	Image k1(0, 4, 400, 300); // создание пустого изображения k1 с параметрами: глубина цвета - 4 бит, размер - 400 * 300
	Image* k2 = new Image(k / 4); //создание копии исходного изображения с глубиной цвета - 4 бит 
	k1 /= *k2; //преобразование копии исходного изображения k2 к размерам пустого изображения k1 
	k1.writeimage("naruto_4.bmp");//запись в пустое изображение данных копии исходного изображения 
	k2->writeimage("naruto_min_scale.bmp");//запись копии исходного исходного изображения с меньшим размером
	delete k2;
	/*
	ПРИМЕР №2
	ИЗМЕНЕНИЕ РАЗМЕРА ИЗОБРАЖЕНИЯ
	исходное изображение на 32 бита с размером 900*680
	*/
	Image* q = new Image("moon_32.bmp");//загрузка изображения в q 
	Image* q1 = new Image(0, 32, 90, 65);//создание пустого изображения q1 с параметрами: глубина цвета - 32 бит, размер - 90 * 65
	(*q1) /= *q; // преобразование размеров исходного изображения в q к размерам пустого изображения q1 
	q1->writeimage("moon_min_scale.bmp");//запись в пустое изображение данных исходного изображения, хранящегося в q
		/*
		ПРИМЕР №3
		ИЗМЕНЕНИЕ РАЗМЕРА ИЗОБРАЖЕНИЯ И ГЛУБИНЫ ЦВЕТА НА 8 БИТ
		исходное изображение на 24 бита с размером 1440*1280
		*/
	Image* p = new Image("cat_24.bmp");//загрузка изображения в p 
	Image* p1 = new Image(0, 24, 900, 680);//создание пустого изображения p1 с параметрами: глубина цвета - 24 бит, размер - 900 * 680
	(*p1) /= *p;//преобразование размеров исходного изображения в p к размерам пустого изображения в p1
	Image p2(*p1 / 8); // создание копии пустого изображения в p1 - p2 с глубиной цвета, равной 8 бит 
	p2.writeimage("cat_8.bmp");//запись в пустое изображение с глубиной цвета - 8 бит данных исходного изображения с глубиной цвета в 24 бит
	p1->writeimage("cat_min_scale.bmp");//запись в пустое изображение p1 данных исходного изображения с размером 900 * 680
	system("pause");
	return 0;
}



