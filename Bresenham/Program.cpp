﻿#include <iostream>
#include <Windows.h>
#include <algorithm>
#include <chrono>

/**
 * \brief Оконная процедура (объявление)
 * \param hWnd Хендл окна
 * \param message Идентификатор сообщения
 * \param wParam Параметр сообщения
 * \param lParam Парамтер сообщения
 * \return Код выполнения
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/**
 * \brief Создание буфера кадра (двумерный массив структур RGBQUAD)
 * \param width Ширина буфера кадра
 * \param height Высота буфера кадра
 * \param clearColor Изначальный цвет
 * \return Указатель на массив
 */
RGBQUAD* CreateFrameBuffer(uint32_t width, uint32_t height, RGBQUAD clearColor = {0,0,0,0});

/**
 * \brief Заполнение буфера изображения каким-то конкретным цветом
 * \param buffer Буфер кадра (указатель на массив)
 * \param pixelCount Кол-во пикселей в буфере
 * \param clearColor Цвет
 */
void ClearFrame(RGBQUAD * buffer, uint32_t pixelCount, RGBQUAD clearColor = { 0,0,0,0 });

/**
 * \brief Установка пикселя
 * \param buffer Буфер кадра (указатель на массив)
 * \param x Положение по оси X
 * \param y Положение по ост Y
 * \param w Ширина фрейм-буфера
 * \param color Очистка цвета
 */
void SetPoint(RGBQUAD * buffer, int x, int y, uint32_t w, RGBQUAD color = { 0,0,0,0 });

/**
 * \brief Рисование линии (быстрый вариант, алгоритм Брэзенхема)
 * \param buffer Буфер кадра (указатель на массив)
 * \param x0 Начальная точка (компонента X)
 * \param y0 Начальная точка (компонента Y)
 * \param x1 Конечная точка (компонента X)
 * \param y1 Конечная точка (компонента Y)
 * \param w Ширина фрейм-буфера 
 * \param color Очистка цвета
 */
void SetLine(RGBQUAD * buffer, int x0, int y0, int x1, int y1, uint32_t w, RGBQUAD color = { 0,0,0,0 });

/**
 * \brief Рисование линии (медленный вариант, с использованием чисел с плавающей точкой)
 * \param buffer Буфер кадра (указатель на массив)
 * \param x0 Начальная точка (компонента X)
 * \param y0 Начальная точка (компонента Y)
 * \param x1 Конечная точка (компонента X)
 * \param y1 Конечная точка (компонента Y)
 * \param w Ширина фрейм-буфера
 * \param color Очистка цвета
 */
void SetLineSlow(RGBQUAD * buffer, int x0, int y0, int x1, int y1, uint32_t w, RGBQUAD color = { 0,0,0,0 });

/**
 * \brief Отрисовка кадра
 * \param width Ширина
 * \param height Высота
 * \param pixels Массив пикселов
 * \param hWnd Хендл окна, device context которого будет использован
 */
void PresentFrame(uint32_t width, uint32_t height, void* pixels, HWND hWnd);

// Размеры кадра и указатель на массив структур RGBQUAD (буфер кадра)
uint32_t frameWidth, frameHeight;
RGBQUAD* frameBuffer;

// Время последнего кадра
std::chrono::time_point<std::chrono::high_resolution_clock> lastFrameTime;

/**
 * \brief Точка входа
 * \param argc Кол-во аргументов на входе
 * \param argv Агрументы (массив C-строк)
 * \return Код выполнения
 */
int main(int argc, char* argv[])
{
	try
	{
		// Получение хендла исполняемого модуля
		HINSTANCE hInstance = GetModuleHandle(nullptr);

		// Регистрация класса окна
		WNDCLASSEX classInfo;
		classInfo.cbSize = sizeof(WNDCLASSEX);
		classInfo.style = CS_HREDRAW | CS_VREDRAW;
		classInfo.cbClsExtra = 0;
		classInfo.cbWndExtra = 0;
		classInfo.hInstance = hInstance;
		classInfo.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
		classInfo.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);
		classInfo.hCursor = LoadCursor(nullptr, IDC_ARROW);
		classInfo.hbrBackground = CreateSolidBrush(RGB(240, 240, 240));
		classInfo.lpszMenuName = nullptr;
		classInfo.lpszClassName = L"BrasenhamLinesWindow";
		classInfo.lpfnWndProc = WndProc;

		// Если не удалось зарегистрировать класс
		if (!RegisterClassEx(&classInfo)) {
			throw std::runtime_error("ERROR: Can't register window class.");
		}

		// Создание окна
		HWND mainWindow = CreateWindow(
			classInfo.lpszClassName,
			L"Brasenham Lines",
			WS_OVERLAPPEDWINDOW,
			0, 0,
			640, 480,
			NULL,
			NULL,
			hInstance,
			NULL);

		// Если не удалось создать окно
		if(!mainWindow){
			throw std::runtime_error("ERROR: Can't create main application window.");
		}

		// Показать окно
		ShowWindow(mainWindow, SW_SHOWNORMAL);

		// Получить размеры клиентской области окна
		RECT clientRect;
		GetClientRect(mainWindow, &clientRect);
		frameWidth = clientRect.right;
		frameHeight = clientRect.bottom;
		std::cout << "INFO: Client area sizes : " << frameWidth << "x" << frameHeight << std::endl;
		
		// Создать буфер кадра по размерам клиенсткой области
		frameBuffer = CreateFrameBuffer(frameWidth, frameHeight);
		std::cout << "INFO: Frame-buffer initialized  (size : " << (sizeof(RGBQUAD) * frameWidth * frameHeight) << " bytes)" << std::endl;

		// Оконное сообщение (пустая структура)
		MSG msg = {};

		// Линия
		struct {
			float speedX0 = 0.00000001f;
			float speedX1 = -0.00000001f;
			int x0 = 0, y0 = 0;
			int x1 = frameWidth, y1 = frameHeight;
		} line1;

		// Рисование линии
		SetLineSlow(frameBuffer, line1.x0, line1.y0, line1.x1, line1.y1, frameWidth, { 0,255,0,0 });

		// Вечный цикл (работает пока не пришло сообщение WM_QUIT)
		while (true)
		{
			// Если получено сообщение
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				DispatchMessage(&msg);

				if (msg.message == WM_QUIT) {
					break;
				}
			}

			// Если хендл окна не пуст
			if(mainWindow)
			{
				// Время текущего кадра (текущей итерации)
				const std::chrono::time_point<std::chrono::high_resolution_clock> currentFrameTime = std::chrono::high_resolution_clock::now();

				// Сколько микросекунд прошло с последней итерации
				// 1 миллисекунда = 1000 микросекунд = 1000000 наносекунд
				const int64_t delta = std::chrono::duration_cast<std::chrono::microseconds>(currentFrameTime - lastFrameTime).count();

				// Перевести в миллисекунды
				float deltaMs = static_cast<float>(delta) / 1000.0f;

				// Изменить положение начальной и конечной точки линии с учетом скорости
				line1.x0 += static_cast<int>(line1.speedX0 * deltaMs);
				line1.x1 += static_cast<int>(line1.speedX1 * deltaMs);

				// Если положение точек будет оказываться за пределами, установить положение
				// равное пределу и изменить направление движения (скорость)
				if(line1.x0 > static_cast<int>(frameWidth)){
					line1.x0 = frameWidth;
					line1.speedX0 *= -1;
				}

				if (line1.x0 < 0){
					line1.x0 = 0;
					line1.speedX0 *= -1;
				}

				if (line1.x1 > static_cast<int>(frameWidth)) {
					line1.x1 = frameWidth;
					line1.speedX1 *= -1;
				}

				if (line1.x1 < 0) {
					line1.x1 = 0;
					line1.speedX1 *= -1;
				}

				// Очистить кадр и нарисовать линию
				ClearFrame(frameBuffer, frameWidth*frameHeight, { 0,0,0,0 });
				SetLineSlow(frameBuffer, line1.x0, line1.y0, line1.x1, line1.y1, frameWidth, { 0,255,0,0 });

				// Сообщение "перерисовать"
				SendMessage(mainWindow, WM_PAINT, NULL, NULL);
			}
		}
	}
	catch(std::exception const &ex)
	{
		std::cout << ex.what() << std::endl;
	}

	return 0;
}

/**
 * \brief Оконная процедура (реализация)
 * \param hWnd Хендл окна
 * \param message Идентификатор сообщения
 * \param wParam Параметр сообщения
 * \param lParam Парамтер сообщения
 * \return Код выполнения
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		PresentFrame(frameWidth, frameHeight, frameBuffer, hWnd);
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

/**
* \brief Создание буфера кадра (двумерный массив структур RGBQUAD)
* \param width Ширина буфера кадра
* \param height Высота буфера кадра
* \param clearColor Изначальный цвет
* \return Указатель на массив
*/
RGBQUAD* CreateFrameBuffer(uint32_t width, uint32_t height, RGBQUAD clearColor)
{
	
	RGBQUAD * frame = new RGBQUAD[width * height];
	ClearFrame(frame, width * height, clearColor);
	return frame;
}

/**
* \brief Заполнение буфера изображения каким-то конкретным цветом
* \param buffer Буфер кадра (указатель на массив)
* \param pixelCount Кол-во пикселей в буфере
* \param clearColor Цвет
*/
void ClearFrame(RGBQUAD* buffer, uint32_t pixelCount, RGBQUAD clearColor)
{
	std::fill_n(buffer, pixelCount, clearColor);
}

/**
* \brief Установка пикселя
* \param buffer Буфер кадра (указатель на массив)
* \param x Положение по оси X
* \param y Положение по ост Y
* \param w Ширина фрейм-буфера
* \param color Очистка цвета
*/
void SetPoint(RGBQUAD* buffer, int x, int y, uint32_t w, RGBQUAD color)
{
	buffer[(y * w) + x] = color;
}

/**
* \brief Рисование линии (быстрый вариант, алгоритм Брэзенхема)
* \param buffer Буфер кадра (указатель на массив)
* \param x0 Начальная точка (компонента X)
* \param y0 Начальная точка (компонента Y)
* \param x1 Конечная точка (компонента X)
* \param y1 Конечная точка (компонента Y)
* \param w Ширина фрейм-буфера
* \param color Очистка цвета
*/
void SetLine(RGBQUAD* buffer, int x0, int y0, int x1, int y1, uint32_t w, RGBQUAD color)
{
	//TODO: имплементация алгоритма Брэзенхема
}

/**
* \brief Рисование линии (медленный вариант, с использованием чисел с плавающей точкой)
* \param buffer Буфер кадра (указатель на массив)
* \param x0 Начальная точка (компонента X)
* \param y0 Начальная точка (компонента Y)
* \param x1 Конечная точка (компонента X)
* \param y1 Конечная точка (компонента Y)
* \param w Ширина фрейм-буфера
* \param color Очистка цвета
*/
void SetLineSlow(RGBQUAD* buffer, int x0, int y0, int x1, int y1, uint32_t w, RGBQUAD color)
{
	int const deltaX = x1 - x0;
	int const deltaY = y1 - y0;

	if(std::abs(deltaX) >= std::abs(deltaY))
	{
		double const k = static_cast<double>(deltaY) / static_cast<double>(deltaX);
		for (int i = 0; deltaX > 0 ? i < deltaX : i > deltaX; deltaX > 0 ? i++ : i--)
		{
			int x = i + x0;
			int y = static_cast<int>(i * k) + y0;
			SetPoint(buffer, x, y, w, color);
		}
	}
	else
	{
		double const k = static_cast<double>(deltaX) / static_cast<double>(deltaY);
		for(int i = 0; deltaY > 0 ? i < deltaY : i > deltaY; deltaY > 0 ? i++ : i--)
		{
			int x = static_cast<int>(i * k) + x0;
			int y = i + y0;
			SetPoint(buffer, x, y, w, color);
		}
	}
}

/**
 * \brief Отрисовка кадра
 * \param width Ширина
 * \param height Высота
 * \param pixels Массив пикселов
 * \param hWnd Хендл окна, device context которого будет использован
 */
void PresentFrame(uint32_t width, uint32_t height, void* pixels, HWND hWnd)
{
	// Получить хендл на временный bit-map (4 байта на пиксель)
	HBITMAP hBitMap = CreateBitmap(width, height, 1, 8 * 4, pixels);

	// Получить device context окна
	HDC hdc = GetDC(hWnd);

	// Временный DC для переноса bit-map'а
	HDC srcHdc = CreateCompatibleDC(hdc);

	// Связать bit-map с временным DC
	SelectObject(srcHdc, hBitMap);

	// Копировать содержимое временного DC в DC окна
	BitBlt(
		hdc,    // HDC назначения
		0,      // Начало вставки по оси X
		0,      // Начало вставки по оси Y
		width,  // Ширина
		height, // Высота
		srcHdc, // Исходный HDC (из которого будут копироваться данные)
		0,      // Начало считывания по оси X
		0,      // Начало считывания по оси Y
		SRCCOPY // Копировать
	);

	// Уничтожить bit-map
	DeleteObject(hBitMap);
	// Уничтодить временный DC
	DeleteDC(srcHdc);
	// Уничтодить DC
	DeleteDC(hdc);
}
