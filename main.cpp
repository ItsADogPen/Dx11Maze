#define WIN32_LEAN_AND_MEAN
#include "main.hpp"
#include <iostream>
#include <stdlib.h>
#include <string>
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "canvas.hpp"

int scrn_w = 800;
int scrn_h = 600;
bool isRunning = false;

/*
鈴木先生、お疲れ様です。

提出が遅くってしまって申し訳ございません。

メイズを完成しました。ご覧ください。

ただ、ウィンドウのコンテキストがなんとなくの理由でフリーズしているようですが、空電のメイズ画像を１つだけを生成していますので、
メイズ自体は機能します。データはCMDにも出力されるので、機能するデータは確認できます。

よろしくお願いいたします。
*/


int main(int argc, char* args[])
{
	SDL_SetMainReady();
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3D");
	
	srand(time(NULL));

	std::string name = "Maze";
	Canvas app{name};

	isRunning = app.Init();

	while (isRunning) 
	{
		app.Loop();
	}

	return 0;
}