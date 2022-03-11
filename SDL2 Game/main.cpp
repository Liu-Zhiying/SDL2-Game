#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>


//颜色工具宏，在本游戏中颜色存储在Uint32中
//前四个是取颜色参数
//第五个宏是仅仅用于第六个宏
//第六个宏根据参数合成颜色变量
#define COLOR_R(color) (color & 0xff)
#define COLOR_G(color) ((color & 0xff00) >> 8)
#define COLOR_B(color) ((color & 0xff0000) >> 16)
#define COLOR_A(color) ((color & 0xff000000) >> 24)
#define BYTE_DATA(data) (data & 0xff)
#define COLOR(r,g,b,a)	( BYTE_DATA(r)         \
						| (BYTE_DATA(g) << 8)  \
						| (BYTE_DATA(b) << 16) \
						| (BYTE_DATA(a) << 24) )

//方块的的X/Y轴数目和像素大小
#define SQURE_X_NUM 14
#define SQURE_Y_NUM 22
#define SQURE_PIXEL_SIZE 20

//无效值和索引
#define INVALID_VALUE 0xffff
#define INVALID_INDEX -1

//一些基本颜色
#define BACKGROUND_COLOR COLOR(0,0,0,0)
#define OTHER_COLOR COLOR(100,100,100,100)
#define TEXT_COLOR COLOR(255,255,255,0)
#define BORDER_COLOR COLOR(25,100,25,25)

//形状的旋转状态
#define SHAPE_STATUS_NUM 4
#define SHAPE_NORMAL 0
#define SHAPE_LEFT_90 1
#define SHAPE_RIGHT_90 2
#define SHAPE_180 3

//指定对下落形状进行的操作
typedef enum Operate
{
	DOWN = 1,
	LEFT = 2,
	RIGHT = 3,
	TURN_LEFT = 4,
	TURN_RIGHT = 5
} Operate;

//方块对于原点的偏移量
typedef struct Offset
{
	int off_x;
	int off_y;
} Offset;

//形状信息
typedef struct Shape
{
	//初始化时的原点
	int init_x;
	int init_y;
	//各个方块对于原点的偏移
	std::vector<Offset> offsets[SHAPE_STATUS_NUM];
} Shape;

//表示坐标
typedef struct Point
{
	int x;
	int y;
} Point;

//存储全局游戏数据的二维数组（存储的是颜色）
Uint32 game_data[SQURE_X_NUM][SQURE_Y_NUM];
//形状目前的原点
int origin_x, origin_y;
//形状的旋转状态
int shape_status;
//形状信息的变长数组（使用随机数索引创建新形状）
std::vector<Shape> shapes;
//SDL窗口指针
SDL_Window* game_window;
//形状的种类（是一个可以在shapes中获取数据的索引）
int shape_index = -1;
//游戏是否结束
SDL_bool is_game_over = SDL_FALSE;
//得分
int score = 0;
//游戏方块的颜色（使用随机数索引为新形状指定颜色）
std::vector<Uint32> colors;

//移动形状
SDL_bool MoveShape(Operate o)
{
	//存储形状的绝对坐标和移动后的绝对坐标
	std::vector<Point> points;
	std::vector<Point> new_points;
	//获取形状的颜色
	int off_x = shapes.at(shape_index).offsets[shape_status].at(0).off_x;
	int off_y = shapes.at(shape_index).offsets[shape_status].at(0).off_y;
	Uint32 color = game_data[origin_x + off_x][origin_y + off_y];
	//是否还原
	SDL_bool restore = SDL_FALSE;
	int i;
	//计算移动前的绝对坐标
	for (i = 0; i < shapes.at(shape_index).offsets[shape_status].size(); i++)
	{
		Point point = {};
		int off_x = shapes.at(shape_index).offsets[shape_status].at(i).off_x;
		int off_y = shapes.at(shape_index).offsets[shape_status].at(i).off_y;
		point.x = origin_x + off_x;
		point.y = origin_y + off_y;
		points.push_back(point);
		new_points.push_back(point);
	}
	//分情况计算移动后的绝对坐标（在移动前的绝对坐标上加偏移）
	switch (o)
	{
	case DOWN:
		for (i = 0; i < new_points.size(); i++)
		{
			++new_points.at(i).y;
		}
		break;
	case LEFT:
		for (i = 0; i < new_points.size(); i++)
		{
			--new_points.at(i).x;
		}
		break;
	case RIGHT:
		for (i = 0; i < new_points.size(); i++)
		{
			++new_points.at(i).x;
		}
		break;
	}
	//抹除原来的形状
	for (i = 0; i < points.size(); i++)
	{
		game_data[points.at(i).x][points.at(i).y] = BACKGROUND_COLOR;
	}
	//检测是否可以移动：（不可移动restore = SDL_TRUE，反之为SDL_FALSE）
	for (i = 0; i < new_points.size(); i++)
	{
		if (new_points.at(i).x < 0 || new_points.at(i).x > SQURE_X_NUM - 1)
		{
			restore = SDL_TRUE;
			break;
		}
		if (new_points.at(i).y < 0 || new_points.at(i).y > SQURE_Y_NUM - 1)
		{
			restore = SDL_TRUE;
			break;
		}
		if (game_data[new_points.at(i).x][new_points.at(i).y] != BACKGROUND_COLOR)
		{
			restore = SDL_TRUE;
			break;
		}
	}
	//如果不能移动，还原形状
	if (restore == SDL_TRUE)
	{
		for (i = 0; i < points.size(); i++)
		{
			game_data[points.at(i).x][points.at(i).y] = color;
		}
		return SDL_FALSE;
	}
	//如果可以移动，为新形状加颜色
	else
	{
		for (i = 0; i < new_points.size(); i++)
		{
			game_data[new_points.at(i).x][new_points.at(i).y] = color;
		}
		switch (o)
		{
		case DOWN:
			++origin_y;
			break;
		case LEFT:
			--origin_x;
			break;
		case RIGHT:
			++origin_x;
			break;
		}
		return SDL_TRUE;
	}
}
//旋转形状
void SpinShape(Operate o)
{
	//初始化，获取当前的旋转状态
	int next_status = shape_status;
	//存储形状的绝对坐标和旋转后的绝对坐标
	std::vector<Point> points;
	std::vector<Point> new_points;
	//获取形状的颜色
	int off_x = shapes.at(shape_index).offsets[shape_status].at(0).off_x;
	int off_y = shapes.at(shape_index).offsets[shape_status].at(0).off_y;
	Uint32 color = game_data[origin_x + off_x][origin_y + off_y];
	//是否还原
	SDL_bool restore = SDL_FALSE;
	int i;
	//根据现在的旋转状态确定旋转后的状态
	switch (o)
	{
	case TURN_LEFT:
		switch (shape_status)
		{
		case SHAPE_NORMAL:
			next_status = SHAPE_LEFT_90;
			break;
		case SHAPE_LEFT_90:
			next_status = SHAPE_180;
			break;
		case SHAPE_180:
			next_status = SHAPE_RIGHT_90;
			break;
		case SHAPE_RIGHT_90:
			next_status = SHAPE_NORMAL;
			break;
		}
		break;
	case TURN_RIGHT:
		switch (shape_status)
		{
		case SHAPE_NORMAL:
			next_status = SHAPE_RIGHT_90;
			break;
		case SHAPE_RIGHT_90:
			next_status = SHAPE_180;
			break;
		case SHAPE_180:
			next_status = SHAPE_LEFT_90;
			break;
		case SHAPE_LEFT_90:
			next_status = SHAPE_NORMAL;
			break;
		}
	}
	//计算现在的绝对坐标
	for (i = 0; i < shapes.at(shape_index).offsets[shape_status].size(); i++)
	{
		Point point = {};
		int off_x = shapes.at(shape_index).offsets[shape_status].at(i).off_x;
		int off_y = shapes.at(shape_index).offsets[shape_status].at(i).off_y;
		point.x = origin_x + off_x;
		point.y = origin_y + off_y;
		points.push_back(point);
	}
	//读取形状数据，计算旋转后的绝对坐标
	for (i = 0; i < shapes.at(shape_index).offsets[next_status].size(); i++)
	{
		Point point = {};
		int off_x = shapes.at(shape_index).offsets[next_status].at(i).off_x;
		int off_y = shapes.at(shape_index).offsets[next_status].at(i).off_y;
		point.x = origin_x + off_x;
		point.y = origin_y + off_y;
		new_points.push_back(point);
	}
	//清空原有形状
	for (i = 0; i < points.size(); i++)
	{
		game_data[points.at(i).x][points.at(i).y] = BACKGROUND_COLOR;
	}
	//测试是否能旋转（不可旋转restore = SDL_TRUE，反之为SDL_FALSE）
	for (i = 0; i < new_points.size(); i++)
	{
		if (new_points.at(i).x < 0 || new_points.at(i).x > SQURE_X_NUM - 1)
		{
			restore = SDL_TRUE;
			break;
		}
		if (new_points.at(i).y < 0 || new_points.at(i).y > SQURE_Y_NUM - 1)
		{
			restore = SDL_TRUE;
			break;
		}
		if (game_data[new_points.at(i).x][new_points.at(i).y] != BACKGROUND_COLOR)
		{
			restore = SDL_TRUE;
			break;
		}
	}
	//如果不能旋转，还原形状
	if (restore == SDL_TRUE)
	{
		for (i = 0; i < points.size(); i++)
		{
			game_data[points.at(i).x][points.at(i).y] = color;
		}
	}
	//如果可以旋转，为形状加颜色
	else
	{
		for (i = 0; i < new_points.size(); i++)
		{
			game_data[new_points.at(i).x][new_points.at(i).y] = color;
		}
		shape_status = next_status;
	}
}
//操作形状（将任务分配给MoveShape，SpinShape）
void OperateShape(Operate o)
{
	//如果没有当前形状，初始化一个
	if (shape_index == INVALID_INDEX)
	{
		srand(time(NULL));
		//随机选设定形状之一
		shape_index = rand() % shapes.size();
		//随机选设定色彩之一
		Uint32 color = colors.at(rand() % colors.size());
		int i;
		for (i = 0; i < shapes.at(shape_index).offsets[SHAPE_NORMAL].size(); i++)
		{
			int off_x = shapes.at(shape_index).offsets[SHAPE_NORMAL].at(i).off_x;
			int off_y = shapes.at(shape_index).offsets[SHAPE_NORMAL].at(i).off_y;
			int x = shapes.at(shape_index).init_x + off_x;
			int y = shapes.at(shape_index).init_y + off_y;
			//如果初始化的位置已经有颜色，退出函数
			//并把is_game_over设置为true，代表游戏结束
			if (game_data[x][y] == BACKGROUND_COLOR)
			{
				game_data[x][y] = color;
			}
			else
			{
				is_game_over = SDL_TRUE;
				break;
			}
		}
		//如果成功设定形状，初始化全局变量
		if (is_game_over == SDL_FALSE)
		{
			origin_x = shapes[shape_index].init_x;
			origin_y = shapes[shape_index].init_y;
			shape_status = SHAPE_NORMAL;
		}
	}
	//对形状进行操作
	else
	{
		switch (o)
		{
		//左右移动
		case LEFT:
		case RIGHT:
			MoveShape(o);
			break;
		//下落（注意，到无法下落时，初始化一个新形状）
		case DOWN:
			if (MoveShape(o) == SDL_FALSE)
			{
				shape_index = INVALID_INDEX;
			}
			break;
		//旋转
		case TURN_LEFT:
		case TURN_RIGHT:
			SpinShape(o);
			break;
		}
	}
}
//初始化
void Init()
{
	int i1, i2;
	//初始化SDL2库
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
	Mix_Init(MIX_INIT_MP3);

	//创建窗口
	game_window = SDL_CreateWindow(
		u8"game",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		640,
		480,
		SDL_WINDOW_SHOWN
	);

	//对全局游戏数据进行初始化（设定为背景色）
	for (i1 = 0; i1 < SQURE_X_NUM; i1++)
	{
		for (i2 = 0; i2 < SQURE_Y_NUM; i2++)
		{
			game_data[i1][i2] = BACKGROUND_COLOR;
		}
	}
	
	Offset off_ = {};
	Shape s = {};
	s.init_x = 10;
	s.init_y = 0;

	//初始化形状数据（手动设定各种旋转状态的相对坐标和初始化的点）
	{
		off_.off_x = 0;
		off_.off_y = 0;
		s.offsets[SHAPE_NORMAL].push_back(off_);
		s.offsets[SHAPE_LEFT_90].push_back(off_);
		s.offsets[SHAPE_RIGHT_90].push_back(off_);
		s.offsets[SHAPE_180].push_back(off_);
		off_.off_x = -1;
		off_.off_y = 0;
		s.offsets[SHAPE_NORMAL].push_back(off_);
		s.offsets[SHAPE_LEFT_90].push_back(off_);
		s.offsets[SHAPE_RIGHT_90].push_back(off_);
		s.offsets[SHAPE_180].push_back(off_);
		off_.off_x = 0;
		off_.off_y = 1;
		s.offsets[SHAPE_NORMAL].push_back(off_);
		s.offsets[SHAPE_LEFT_90].push_back(off_);
		s.offsets[SHAPE_RIGHT_90].push_back(off_);
		s.offsets[SHAPE_180].push_back(off_);
		off_.off_x = -1;
		off_.off_y = 1;
		s.offsets[SHAPE_NORMAL].push_back(off_);
		s.offsets[SHAPE_LEFT_90].push_back(off_);
		s.offsets[SHAPE_RIGHT_90].push_back(off_);
		s.offsets[SHAPE_180].push_back(off_);

		shapes.push_back(s);

		s.offsets[SHAPE_NORMAL].clear();
		s.offsets[SHAPE_LEFT_90].clear();
		s.offsets[SHAPE_RIGHT_90].clear();
		s.offsets[SHAPE_180].clear();
	}

	{
		{
			off_.off_x = 0;
			off_.off_y = 0;
			s.offsets[SHAPE_NORMAL].push_back(off_);
			off_.off_x = -1;
			off_.off_y = 0;
			s.offsets[SHAPE_NORMAL].push_back(off_);
			off_.off_x = 1;
			off_.off_y = 0;
			s.offsets[SHAPE_NORMAL].push_back(off_);
			off_.off_x = 1;
			off_.off_y = 1;
			s.offsets[SHAPE_NORMAL].push_back(off_);
		}

		{
			off_.off_x = 0;
			off_.off_y = 0;
			s.offsets[SHAPE_LEFT_90].push_back(off_);
			off_.off_x = 0;
			off_.off_y = -1;
			s.offsets[SHAPE_LEFT_90].push_back(off_);
			off_.off_x = 0;
			off_.off_y = 1;
			s.offsets[SHAPE_LEFT_90].push_back(off_);
			off_.off_x = -1;
			off_.off_y = 1;
			s.offsets[SHAPE_LEFT_90].push_back(off_);
		}

		{
			off_.off_x = 0;
			off_.off_y = 0;
			s.offsets[SHAPE_RIGHT_90].push_back(off_);
			off_.off_x = 0;
			off_.off_y = -1;
			s.offsets[SHAPE_RIGHT_90].push_back(off_);
			off_.off_x = 0;
			off_.off_y = 1;
			s.offsets[SHAPE_RIGHT_90].push_back(off_);
			off_.off_x = 1;
			off_.off_y = -1;
			s.offsets[SHAPE_RIGHT_90].push_back(off_);
		}

		{
			off_.off_x = 0;
			off_.off_y = 0;
			s.offsets[SHAPE_180].push_back(off_);
			off_.off_x = -1;
			off_.off_y = 0;
			s.offsets[SHAPE_180].push_back(off_);
			off_.off_x = 1;
			off_.off_y = 0;
			s.offsets[SHAPE_180].push_back(off_);
			off_.off_x = -1;
			off_.off_y = -1;
			s.offsets[SHAPE_180].push_back(off_);
		}

		shapes.push_back(s);

		s.offsets[SHAPE_NORMAL].clear();
		s.offsets[SHAPE_LEFT_90].clear();
		s.offsets[SHAPE_RIGHT_90].clear();
		s.offsets[SHAPE_180].clear();
	}


	s.init_x = 10;
	s.init_y = 1;

	{
		{
			off_.off_x = 0;
			off_.off_y = -1;
			s.offsets[SHAPE_NORMAL].push_back(off_);
			off_.off_x = -1;
			off_.off_y = -1;
			s.offsets[SHAPE_NORMAL].push_back(off_);
			off_.off_x = -2;
			off_.off_y = -1;
			s.offsets[SHAPE_NORMAL].push_back(off_);
			off_.off_x = 1;
			off_.off_y = -1;
			s.offsets[SHAPE_NORMAL].push_back(off_);
		}

		{
			off_.off_x = 0;
			off_.off_y = -1;
			s.offsets[SHAPE_LEFT_90].push_back(off_);
			off_.off_x = 0;
			off_.off_y = 0;
			s.offsets[SHAPE_LEFT_90].push_back(off_);
			off_.off_x = 0;
			off_.off_y = 1;
			s.offsets[SHAPE_LEFT_90].push_back(off_);
			off_.off_x = 0;
			off_.off_y = 2;
			s.offsets[SHAPE_LEFT_90].push_back(off_);
		}

		{
			off_.off_x = 1;
			off_.off_y = 0;
			s.offsets[SHAPE_RIGHT_90].push_back(off_);
			off_.off_x = 1;
			off_.off_y = 1;
			s.offsets[SHAPE_RIGHT_90].push_back(off_);
			off_.off_x = 1;
			off_.off_y = -1;
			s.offsets[SHAPE_RIGHT_90].push_back(off_);
			off_.off_x = 1;
			off_.off_y = 2;
			s.offsets[SHAPE_RIGHT_90].push_back(off_);
		}

		{
			off_.off_x = 0;
			off_.off_y = 1;
			s.offsets[SHAPE_180].push_back(off_);
			off_.off_x = -1;
			off_.off_y = 1;
			s.offsets[SHAPE_180].push_back(off_);
			off_.off_x = 1;
			off_.off_y = 1;
			s.offsets[SHAPE_180].push_back(off_);
			off_.off_x = -2;
			off_.off_y = 1;
			s.offsets[SHAPE_180].push_back(off_);
		}

		shapes.push_back(s);

		s.offsets[SHAPE_NORMAL].clear();
		s.offsets[SHAPE_LEFT_90].clear();
		s.offsets[SHAPE_RIGHT_90].clear();
		s.offsets[SHAPE_180].clear();
	}
	
	//初始化颜色数据（手动设定形状的候选颜色）
	colors.push_back(COLOR(255, 0, 0, 0));
	colors.push_back(COLOR(0, 255, 0, 0));
	colors.push_back(COLOR(0, 0, 255, 0));
	colors.push_back(COLOR(255, 255, 0, 0));
	colors.push_back(COLOR(255, 0, 255, 0));
	colors.push_back(COLOR(0, 255, 255, 0));
}
//绘制图形
void Paint()
{
	//拼接得分字符串
	std::string s;
	std::stringstream ss;
	ss << score;
	ss >> s;
	s = "SCORE:" + s;

	int w, h;
	//方块的显示矩形
	SDL_Rect draw_rect = {};
	//字符在窗口的显示矩形
	SDL_Rect dest_rect = { 0,0,50,15 };
	int i1, i2;
	//创建渲染器
	SDL_Renderer* renderer = SDL_CreateRenderer(
		game_window, 
		-1,
		SDL_RENDERER_ACCELERATED
	);
	//创建失败，终止游戏
	if (!renderer)
	{
		is_game_over = SDL_TRUE;
		return;
	}

	SDL_Color color = { 255,255,255,0 };
	//打开字体
	TTF_Font* font = TTF_OpenFont("arial.ttf", 96);
	//打开失败，终止游戏
	if (!font)
	{
		std::cout << SDL_GetError() << std::endl;
		is_game_over = SDL_TRUE;
		return;
	}
	//创建文本surface
	SDL_Surface* text_surface = TTF_RenderText_Solid(
		font,
		s.c_str(),
		color
	);
	//创建文本surface失败，结束游戏
	if (!text_surface)
	{
		is_game_over = SDL_FALSE;
		return;
	}
	//从surface创建textture
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, text_surface);
	//如果失败，结束游戏
	if (!texture)
	{
		is_game_over = SDL_FALSE;
		return;
	}
	//获取窗口的可绘制大小
	SDL_GetRendererOutputSize(renderer, &w, &h);
	//绘制非游戏区域
	draw_rect.x = 0;
	draw_rect.y = 0;
	draw_rect.w = w;
	draw_rect.h = h;

	SDL_SetRenderDrawColor(
		renderer,
		COLOR_R(OTHER_COLOR),
		COLOR_G(OTHER_COLOR),
		COLOR_B(OTHER_COLOR),
		COLOR_A(OTHER_COLOR)
	);

	SDL_RenderClear(renderer);
	//绘制文本
	SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
	//计算游戏区域
	draw_rect.w = draw_rect.h = SQURE_PIXEL_SIZE;
	draw_rect.x = (w - SQURE_PIXEL_SIZE * SQURE_X_NUM) / 2;
	//绘制游戏区域
	for (i1 = 0; i1 < SQURE_X_NUM; i1++)
	{
		draw_rect.y = (h - SQURE_PIXEL_SIZE * SQURE_Y_NUM) / 2;
		for (i2 = 0; i2 < SQURE_Y_NUM; i2++)
		{
			SDL_SetRenderDrawColor(
				renderer,
				COLOR_R(game_data[i1][i2]),
				COLOR_G(game_data[i1][i2]),
				COLOR_B(game_data[i1][i2]),
				COLOR_A(game_data[i1][i2])
			);
			SDL_RenderFillRect(renderer, &draw_rect);
			SDL_SetRenderDrawColor(
				renderer,
				COLOR_R(BORDER_COLOR),
				COLOR_G(BORDER_COLOR),
				COLOR_B(BORDER_COLOR),
				COLOR_A(BORDER_COLOR)
			);
			SDL_RenderDrawRect(renderer, &draw_rect);
			draw_rect.y += SQURE_PIXEL_SIZE;
		}
		draw_rect.x += SQURE_PIXEL_SIZE;
	}
	//清理
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(text_surface);
	SDL_RenderPresent(renderer);
	TTF_CloseFont(font);
	SDL_DestroyRenderer(renderer);
}
//退出清理
void Quit()
{
	//清理资源，退出SDL2
	SDL_DestroyWindow(game_window);
	Mix_Quit();
	TTF_Quit();
	SDL_Quit();
}
//得分计算
int CountScore()
{
	int x, y;
	//是否删除整行（当整行都有颜色时）
	SDL_bool delete_line;
	//遍历游戏全集数据
	for (y = 0; y < SQURE_Y_NUM; y++)
	{
		delete_line = SDL_TRUE;
		//如果整行有颜色，提示删除该行
		for (x = 0; x < SQURE_X_NUM; x++)
		{
			if (game_data[x][y] == BACKGROUND_COLOR)
			{
				delete_line = SDL_FALSE;
				break;
			}
		}
		//删除该行，行数上面的颜色下移
		if (delete_line == SDL_TRUE)
		{
			int i = 0;
			for (i = 0; i < SQURE_X_NUM; i++)
			{
				game_data[i][y] = BACKGROUND_COLOR;
			}
			int i1, i2;
			for (i1 = y; i1 > 0; i1--)
			{
				for (i2 = 0; i2 < SQURE_X_NUM; i2++)
				{
					game_data[i2][i1] = game_data[i2][i1 - 1];
				}
			}
			//最顶上补背景色
			for (i2 = 0; i2 < SQURE_X_NUM; i2++)
			{
				game_data[i2][0] = BACKGROUND_COLOR;
			}
			++score;
		}
	}
	return 0;
}
//用于图形独立线程的函数
int GraphicThread(void* ptr)
{
	//无限循环实现一直绘制
	while (true)
	{
		//锁定资源（程序的全局变量）
		SDL_LockMutex((SDL_mutex*)ptr);
		OperateShape(DOWN);
		//如果形状下落完毕，进行一次得分计算
		if(shape_index == INVALID_INDEX)
			CountScore();
		//绘制图形
		Paint();
		//如果游戏结束，退出循环
		if (is_game_over == SDL_TRUE)
		{
			break;
		}
		//解锁
		SDL_UnlockMutex((SDL_mutex*)ptr);
		SDL_Delay(200);
	}
	//解锁
	SDL_UnlockMutex((SDL_mutex*)ptr);
	return 0;
}

int main(int argc, char** argv)
{
	//初始化
	Init();
	SDL_Event event = {};
	//创建锁
	SDL_mutex* mutex = SDL_CreateMutex();
	//创建图形线程
	SDL_CreateThread(GraphicThread, u8"t1", mutex);
	//事件循环
	while (event.type != SDL_QUIT)
	{
		//获取事件
		SDL_PollEvent(&event);
		//对键盘事件做处理
		if (event.type == SDL_KEYDOWN)
		{
			int i = 0;
			int	repeat = !event.key.repeat ? 1 : event.key.repeat;
			//锁定资源
			SDL_LockMutex(mutex);
			for (i = 0; i < repeat;i++)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_a:
					OperateShape(LEFT);
					break;
				case SDLK_d:
					OperateShape(RIGHT);
					break;
				case SDLK_s:
					OperateShape(DOWN);
					break;
				case SDLK_q:
					OperateShape(TURN_LEFT);
					break;
				case SDLK_e:
					OperateShape(TURN_RIGHT);
					break;
				default:
					break;
				}
			}
			//解锁
			SDL_UnlockMutex(mutex);
		}
		//游戏结束则退出事件循环
		if (is_game_over == SDL_TRUE)
		{
			break;
		}
	}
	//删除锁
	SDL_DestroyMutex(mutex);
	//退出
	Quit();
	return 0;
}