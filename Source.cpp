#include <Windows.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>

#pragma warning(disable:4996)

#define WIDTH 256
#define HEIGHT 256
#define CLRUSED 256
#define PELSMETER 3780

#define SIDE 64	// 分割ブロックサイズ
#define MBLX (WIDTH/SIDE)
#define MBLY (HEIGHT/SIDE)

typedef struct
{
	int x;
	int y;
	float z;
}VERTEX;

BITMAPFILEHEADER g_bf;
BITMAPINFOHEADER g_bi;
RGBQUAD g_pal[CLRUSED];
BYTE g_pix[WIDTH*HEIGHT];

RGBQUAD g_clrmin = { 0, 32, 64, 0 };
RGBQUAD g_clrmax = { 64, 128, 192, 0 };

float g_rough = 16.0f;	// でこぼこの度合い
VERTEX g_vertex[MBLY + 1][MBLX + 1];

// 指定位置に点を打つ
void PSet(int x, int y, BYTE color)
{
	if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
	{
		return;
	}
	g_pix[(HEIGHT - 1 - y)*WIDTH + x] = color;
}

// 中点の生成
void Middle(VERTEX & p1, VERTEX & p2, VERTEX & p3, int level)
{
	p3.x = (p1.x + p2.x) / 2;
	p3.y = (p1.y + p2.y) / 2;
	// todo
}

// 描画
void Plot(VERTEX & p1, VERTEX & p2, VERTEX & p3)
{
	if (p1.y < p2.y)
	{
		int c = static_cast<int>((p2.z - p1.z)*g_rough + CLRUSED / 2);
		if (c > 255)
		{
			c = 255;
		}
		else if(c < 0)
		{
			c = 0;
		}
		PSet(p1.x, p1.y, c);
	}
}

// 三角形の中点変異法による変形
void TriPoints(VERTEX a, VERTEX b, VERTEX c)
{
	int level = abs(a.y - b.y);
	if (level <= 1)
	{
		Plot(a, b, c);
	}
	else
	{
		VERTEX l, m, n;
		Middle(a, b, ab, level);

	}
	// todo
}

// ブロックの初期化
void InitBlock(void)
{
	srand(static_cast<unsigned>(time(NULL)));
	for (int y = 0; y <= MBLY; ++y)
	{
		for (int x = 0; x <= MBLX; ++x)
		{
			g_vertex[y][x].x = x*SIDE;
			g_vertex[y][x].y = y*SIDE;
			g_vertex[y][x].z = rand()*SIDE * 2 / static_cast<float>(RAND_MAX - SIDE);
		}
	}
}

// 山肌テクスチャの生成
void Crag(void)
{
	InitBlock();
	for (int y = 0; y < MBLY; ++y)
	{
		for (int x = 0; x < MBLX; ++x)
		{
			VERTEX & a = g_vertex[y][x];
			VERTEX & b = g_vertex[y + 1][x];
			VERTEX & c = g_vertex[y + 1][x + 1];
			VERTEX & d = g_vertex[y][x + 1];
			TriPoints(a, b, c);
			TriPoints(c, d, a);
		}
	}
}

int main(int argc, char ** argb)
{
	// BMPヘッダを埋める
	ZeroMemory(&g_bf, sizeof(g_bf));
	g_bf.bfType = 'B' | 'M' << 8;
	g_bf.bfSize = sizeof(g_bf) + sizeof(g_bi) + sizeof(g_pal) + sizeof(g_pix);
	g_bf.bfOffBits = sizeof(g_bf) + sizeof(g_bi) + sizeof(g_pal);

	ZeroMemory(&g_bi, sizeof(g_bi));
	g_bi.biSize = sizeof(g_bi);
	g_bi.biWidth = WIDTH;
	g_bi.biHeight = HEIGHT;
	g_bi.biPlanes = 1;
	g_bi.biBitCount = 8;
	g_bi.biSizeImage = sizeof(g_pix);
	g_bi.biXPelsPerMeter = PELSMETER;
	g_bi.biYPelsPerMeter = PELSMETER;
	g_bi.biClrUsed = CLRUSED;
	g_bi.biClrImportant = CLRUSED;

	// パレットを埋める(線形保管によるグラデーション)
	for (int i = 0; i < CLRUSED; ++i)
	{
		g_pal[i].rgbBlue = g_clrmin.rgbBlue + (g_clrmax.rgbBlue - g_clrmin.rgbBlue)*i / (CLRUSED - 1);
		g_pal[i].rgbGreen = g_clrmin.rgbGreen + (g_clrmax.rgbGreen - g_clrmin.rgbGreen)*i / (CLRUSED - 1);
		g_pal[i].rgbRed = g_clrmin.rgbRed + (g_clrmax.rgbRed - g_clrmin.rgbRed)*i / (CLRUSED - 1);
	}

	// フラクタルによる山肌テクスチャ生成
	Crag();

	FILE * fp = fopen("Texture.bmp", "wb");
	if (fp)
	{
		fwrite(&g_bf, sizeof(g_bf), 1, fp);
		fwrite(&g_bi, sizeof(g_bi), 1, fp);
		fwrite(g_pal, sizeof(g_pal), 1, fp);
		fwrite(g_pix, sizeof(g_pix), 1, fp);
		fclose(fp);
	}

	return EXIT_SUCCESS;
}