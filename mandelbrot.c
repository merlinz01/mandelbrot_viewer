
#define UNICODE
#include <windows.h>
#include <stdio.h>
#include <math.h>

#define T(s) L ## s

typedef unsigned char uint8_t;

typedef struct Pixel {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t reserved;
} Pixel;

void set_pixel(Pixel *px, int i);
int mandelbrot_iterations(double x, double y);
int test_gradient(int px, int pwidth);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define MANDELBROT_MINX -2.0f
#define MANDELBROT_MAXX 0.47f
#define MANDELBROT_MINY -1.12f
#define MANDELBROT_MAXY 1.12f

#define MAX_ITERATION 1000

Pixel gradient[MAX_ITERATION];

extern void mandelbrot(Pixel *data, double scale, double startx, double starty, unsigned int pwidth, unsigned int pheight) {
	double x = startx;
	for (unsigned int px = 0; px < pwidth; px++) {
		int py = 0;
		double y = starty;
		for (unsigned int py = 0; py < pheight; py++) {
			data[py*pwidth+px] = gradient[mandelbrot_iterations(x, y)];
			//data[py*pwidth+px] = gradient[test_gradient(px, pwidth)];
			y += scale;
		}
		x += scale;
	}
}

int mandelbrot_iterations(double x, double y) {
	//if (x < MANDELBROT_MINX || x > MANDELBROT_MAXX || y < MANDELBROT_MINY || y > MANDELBROT_MAXY) return 0;
	int iteration = 0;
	double cx = 0.0f;
	double cy = 0.0f;
	while (cx*cx + cy+cy < 4.0f && iteration < MAX_ITERATION) {
		double xtemp = cx*cx - cy*cy + x;
		cy = 2.0f * cx * cy + y;
		cx = xtemp;
		iteration++;
	}
	return iteration;
}

int test_gradient(int px, int pwidth) {
	return px * MAX_ITERATION / pwidth;
}

#define PI 3.141592653589793f
#define PI2 (PI*2.0f) 

void set_pixel(Pixel *px, int i) {
	double j = (double)i / MAX_ITERATION;
	double k = (j+1.5) * (1-j) * 127;
	j = (1-j)*PI2;
	px->reserved = 0;
	#define clamp(x) (uint8_t) (min(max((x), 0), 255))
	px->r = clamp((1 + sin(j     )) * k);
	px->g = clamp((1 + cos(j     )) * k);
	px->b = clamp((1 + sin(j + PI)) * k);
}

int main() {
	// generate gradient
	for (int i = 0; i < MAX_ITERATION; i++) {
		set_pixel(&gradient[i], i);
	}
	WNDCLASS wc;
	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC) WindowProc;
	wc.cbWndExtra = 0;
	wc.cbClsExtra = 0;
	wc.hInstance = NULL;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = T("MANDELBROTWIN");
	if (!RegisterClass(&wc)) {
		printf("failed to register window class");
		return 1;
	}
	HWND hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		T("MANDELBROTWIN"),
		T("The Mandelbrot Set - move with arrow keys, zoom with +/- keys"),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
		10, 10, 
		800, 600, 
		NULL, NULL, NULL, NULL);
	if (!hwnd) {
		printf("failed to create window");
		return 2;
	}
	
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

Pixel MB_DATA[2500*2500];
double SCALE = 0.005;
double STARTX = -0.5;
double STARTY = 0.0;

void repaint(HWND hwnd) {
	RECT rc;
	GetClientRect(hwnd, &rc);
	UINT w = rc.right;
	UINT h = rc.bottom;
	mandelbrot(MB_DATA, SCALE, STARTX-(w*SCALE/2), STARTY-(h*SCALE/2), w, h);
	InvalidateRect(hwnd, NULL, 0);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	RECT rc;
	switch (msg) {
		case WM_PAINT: 
			{
				PAINTSTRUCT ps;
				HDC hdc;
				hdc = BeginPaint(hwnd, &ps);
				GetClientRect(hwnd, &rc);
				UINT w = rc.right;
				UINT h = rc.bottom;
				BITMAPINFO bmi;
				bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
				bmi.bmiHeader.biWidth = w;
				bmi.bmiHeader.biHeight = -h;
				bmi.bmiHeader.biPlanes = 1;
				bmi.bmiHeader.biBitCount = 32;
				bmi.bmiHeader.biCompression = BI_RGB;
				bmi.bmiHeader.biSizeImage = (w * h * sizeof(Pixel));
				bmi.bmiHeader.biXPelsPerMeter = 3780;
				bmi.bmiHeader.biYPelsPerMeter = 3780;
				bmi.bmiHeader.biClrUsed = 0;
				bmi.bmiHeader.biClrImportant = 0;
				if (!SetDIBitsToDevice(hdc, 0, 0, w, h, 0, 0, 0, h, (LPVOID)&MB_DATA, &bmi, DIB_RGB_COLORS)) {
					TextOut(hdc, 0, 0, T("Failed"), 6);
				}
				EndPaint(hwnd, &ps);
			} 
			break;
		case WM_KEYDOWN:
			switch (wParam) {
				case VK_UP:
					STARTY += SCALE*-100;
					break;
				case VK_DOWN:
					STARTY += SCALE*100;
					break;
				case VK_LEFT:
					STARTX += SCALE*-100;
					break;
				case VK_RIGHT:
					STARTX += SCALE*100;
					break;
				case VK_ADD:
					SCALE *= 0.5;
					break;
				case VK_SUBTRACT:
					SCALE *= 2;
					break;
			}
			printf("Scale: %G, X: %G, Y: %G\n", SCALE, STARTX, STARTY);
			repaint(hwnd);
			UpdateWindow(hwnd);
			break;
		case WM_SIZE:
			repaint(hwnd);
			break;
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}