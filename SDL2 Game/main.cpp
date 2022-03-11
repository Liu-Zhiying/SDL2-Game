#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>


//��ɫ���ߺ꣬�ڱ���Ϸ����ɫ�洢��Uint32��
//ǰ�ĸ���ȡ��ɫ����
//��������ǽ������ڵ�������
//����������ݲ����ϳ���ɫ����
#define COLOR_R(color) (color & 0xff)
#define COLOR_G(color) ((color & 0xff00) >> 8)
#define COLOR_B(color) ((color & 0xff0000) >> 16)
#define COLOR_A(color) ((color & 0xff000000) >> 24)
#define BYTE_DATA(data) (data & 0xff)
#define COLOR(r,g,b,a)	( BYTE_DATA(r)         \
						| (BYTE_DATA(g) << 8)  \
						| (BYTE_DATA(b) << 16) \
						| (BYTE_DATA(a) << 24) )

//����ĵ�X/Y����Ŀ�����ش�С
#define SQURE_X_NUM 14
#define SQURE_Y_NUM 22
#define SQURE_PIXEL_SIZE 20

//��Чֵ������
#define INVALID_VALUE 0xffff
#define INVALID_INDEX -1

//һЩ������ɫ
#define BACKGROUND_COLOR COLOR(0,0,0,0)
#define OTHER_COLOR COLOR(100,100,100,100)
#define TEXT_COLOR COLOR(255,255,255,0)
#define BORDER_COLOR COLOR(25,100,25,25)

//��״����ת״̬
#define SHAPE_STATUS_NUM 4
#define SHAPE_NORMAL 0
#define SHAPE_LEFT_90 1
#define SHAPE_RIGHT_90 2
#define SHAPE_180 3

//ָ����������״���еĲ���
typedef enum Operate
{
	DOWN = 1,
	LEFT = 2,
	RIGHT = 3,
	TURN_LEFT = 4,
	TURN_RIGHT = 5
} Operate;

//�������ԭ���ƫ����
typedef struct Offset
{
	int off_x;
	int off_y;
} Offset;

//��״��Ϣ
typedef struct Shape
{
	//��ʼ��ʱ��ԭ��
	int init_x;
	int init_y;
	//�����������ԭ���ƫ��
	std::vector<Offset> offsets[SHAPE_STATUS_NUM];
} Shape;

//��ʾ����
typedef struct Point
{
	int x;
	int y;
} Point;

//�洢ȫ����Ϸ���ݵĶ�ά���飨�洢������ɫ��
Uint32 game_data[SQURE_X_NUM][SQURE_Y_NUM];
//��״Ŀǰ��ԭ��
int origin_x, origin_y;
//��״����ת״̬
int shape_status;
//��״��Ϣ�ı䳤���飨ʹ�������������������״��
std::vector<Shape> shapes;
//SDL����ָ��
SDL_Window* game_window;
//��״�����ࣨ��һ��������shapes�л�ȡ���ݵ�������
int shape_index = -1;
//��Ϸ�Ƿ����
SDL_bool is_game_over = SDL_FALSE;
//�÷�
int score = 0;
//��Ϸ�������ɫ��ʹ�����������Ϊ����״ָ����ɫ��
std::vector<Uint32> colors;

//�ƶ���״
SDL_bool MoveShape(Operate o)
{
	//�洢��״�ľ���������ƶ���ľ�������
	std::vector<Point> points;
	std::vector<Point> new_points;
	//��ȡ��״����ɫ
	int off_x = shapes.at(shape_index).offsets[shape_status].at(0).off_x;
	int off_y = shapes.at(shape_index).offsets[shape_status].at(0).off_y;
	Uint32 color = game_data[origin_x + off_x][origin_y + off_y];
	//�Ƿ�ԭ
	SDL_bool restore = SDL_FALSE;
	int i;
	//�����ƶ�ǰ�ľ�������
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
	//����������ƶ���ľ������꣨���ƶ�ǰ�ľ��������ϼ�ƫ�ƣ�
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
	//Ĩ��ԭ������״
	for (i = 0; i < points.size(); i++)
	{
		game_data[points.at(i).x][points.at(i).y] = BACKGROUND_COLOR;
	}
	//����Ƿ�����ƶ����������ƶ�restore = SDL_TRUE����֮ΪSDL_FALSE��
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
	//��������ƶ�����ԭ��״
	if (restore == SDL_TRUE)
	{
		for (i = 0; i < points.size(); i++)
		{
			game_data[points.at(i).x][points.at(i).y] = color;
		}
		return SDL_FALSE;
	}
	//��������ƶ���Ϊ����״����ɫ
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
//��ת��״
void SpinShape(Operate o)
{
	//��ʼ������ȡ��ǰ����ת״̬
	int next_status = shape_status;
	//�洢��״�ľ����������ת��ľ�������
	std::vector<Point> points;
	std::vector<Point> new_points;
	//��ȡ��״����ɫ
	int off_x = shapes.at(shape_index).offsets[shape_status].at(0).off_x;
	int off_y = shapes.at(shape_index).offsets[shape_status].at(0).off_y;
	Uint32 color = game_data[origin_x + off_x][origin_y + off_y];
	//�Ƿ�ԭ
	SDL_bool restore = SDL_FALSE;
	int i;
	//�������ڵ���ת״̬ȷ����ת���״̬
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
	//�������ڵľ�������
	for (i = 0; i < shapes.at(shape_index).offsets[shape_status].size(); i++)
	{
		Point point = {};
		int off_x = shapes.at(shape_index).offsets[shape_status].at(i).off_x;
		int off_y = shapes.at(shape_index).offsets[shape_status].at(i).off_y;
		point.x = origin_x + off_x;
		point.y = origin_y + off_y;
		points.push_back(point);
	}
	//��ȡ��״���ݣ�������ת��ľ�������
	for (i = 0; i < shapes.at(shape_index).offsets[next_status].size(); i++)
	{
		Point point = {};
		int off_x = shapes.at(shape_index).offsets[next_status].at(i).off_x;
		int off_y = shapes.at(shape_index).offsets[next_status].at(i).off_y;
		point.x = origin_x + off_x;
		point.y = origin_y + off_y;
		new_points.push_back(point);
	}
	//���ԭ����״
	for (i = 0; i < points.size(); i++)
	{
		game_data[points.at(i).x][points.at(i).y] = BACKGROUND_COLOR;
	}
	//�����Ƿ�����ת��������תrestore = SDL_TRUE����֮ΪSDL_FALSE��
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
	//���������ת����ԭ��״
	if (restore == SDL_TRUE)
	{
		for (i = 0; i < points.size(); i++)
		{
			game_data[points.at(i).x][points.at(i).y] = color;
		}
	}
	//���������ת��Ϊ��״����ɫ
	else
	{
		for (i = 0; i < new_points.size(); i++)
		{
			game_data[new_points.at(i).x][new_points.at(i).y] = color;
		}
		shape_status = next_status;
	}
}
//������״������������MoveShape��SpinShape��
void OperateShape(Operate o)
{
	//���û�е�ǰ��״����ʼ��һ��
	if (shape_index == INVALID_INDEX)
	{
		srand(time(NULL));
		//���ѡ�趨��״֮һ
		shape_index = rand() % shapes.size();
		//���ѡ�趨ɫ��֮һ
		Uint32 color = colors.at(rand() % colors.size());
		int i;
		for (i = 0; i < shapes.at(shape_index).offsets[SHAPE_NORMAL].size(); i++)
		{
			int off_x = shapes.at(shape_index).offsets[SHAPE_NORMAL].at(i).off_x;
			int off_y = shapes.at(shape_index).offsets[SHAPE_NORMAL].at(i).off_y;
			int x = shapes.at(shape_index).init_x + off_x;
			int y = shapes.at(shape_index).init_y + off_y;
			//�����ʼ����λ���Ѿ�����ɫ���˳�����
			//����is_game_over����Ϊtrue��������Ϸ����
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
		//����ɹ��趨��״����ʼ��ȫ�ֱ���
		if (is_game_over == SDL_FALSE)
		{
			origin_x = shapes[shape_index].init_x;
			origin_y = shapes[shape_index].init_y;
			shape_status = SHAPE_NORMAL;
		}
	}
	//����״���в���
	else
	{
		switch (o)
		{
		//�����ƶ�
		case LEFT:
		case RIGHT:
			MoveShape(o);
			break;
		//���䣨ע�⣬���޷�����ʱ����ʼ��һ������״��
		case DOWN:
			if (MoveShape(o) == SDL_FALSE)
			{
				shape_index = INVALID_INDEX;
			}
			break;
		//��ת
		case TURN_LEFT:
		case TURN_RIGHT:
			SpinShape(o);
			break;
		}
	}
}
//��ʼ��
void Init()
{
	int i1, i2;
	//��ʼ��SDL2��
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
	Mix_Init(MIX_INIT_MP3);

	//��������
	game_window = SDL_CreateWindow(
		u8"game",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		640,
		480,
		SDL_WINDOW_SHOWN
	);

	//��ȫ����Ϸ���ݽ��г�ʼ�����趨Ϊ����ɫ��
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

	//��ʼ����״���ݣ��ֶ��趨������ת״̬���������ͳ�ʼ���ĵ㣩
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
	
	//��ʼ����ɫ���ݣ��ֶ��趨��״�ĺ�ѡ��ɫ��
	colors.push_back(COLOR(255, 0, 0, 0));
	colors.push_back(COLOR(0, 255, 0, 0));
	colors.push_back(COLOR(0, 0, 255, 0));
	colors.push_back(COLOR(255, 255, 0, 0));
	colors.push_back(COLOR(255, 0, 255, 0));
	colors.push_back(COLOR(0, 255, 255, 0));
}
//����ͼ��
void Paint()
{
	//ƴ�ӵ÷��ַ���
	std::string s;
	std::stringstream ss;
	ss << score;
	ss >> s;
	s = "SCORE:" + s;

	int w, h;
	//�������ʾ����
	SDL_Rect draw_rect = {};
	//�ַ��ڴ��ڵ���ʾ����
	SDL_Rect dest_rect = { 0,0,50,15 };
	int i1, i2;
	//������Ⱦ��
	SDL_Renderer* renderer = SDL_CreateRenderer(
		game_window, 
		-1,
		SDL_RENDERER_ACCELERATED
	);
	//����ʧ�ܣ���ֹ��Ϸ
	if (!renderer)
	{
		is_game_over = SDL_TRUE;
		return;
	}

	SDL_Color color = { 255,255,255,0 };
	//������
	TTF_Font* font = TTF_OpenFont("arial.ttf", 96);
	//��ʧ�ܣ���ֹ��Ϸ
	if (!font)
	{
		std::cout << SDL_GetError() << std::endl;
		is_game_over = SDL_TRUE;
		return;
	}
	//�����ı�surface
	SDL_Surface* text_surface = TTF_RenderText_Solid(
		font,
		s.c_str(),
		color
	);
	//�����ı�surfaceʧ�ܣ�������Ϸ
	if (!text_surface)
	{
		is_game_over = SDL_FALSE;
		return;
	}
	//��surface����textture
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, text_surface);
	//���ʧ�ܣ�������Ϸ
	if (!texture)
	{
		is_game_over = SDL_FALSE;
		return;
	}
	//��ȡ���ڵĿɻ��ƴ�С
	SDL_GetRendererOutputSize(renderer, &w, &h);
	//���Ʒ���Ϸ����
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
	//�����ı�
	SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
	//������Ϸ����
	draw_rect.w = draw_rect.h = SQURE_PIXEL_SIZE;
	draw_rect.x = (w - SQURE_PIXEL_SIZE * SQURE_X_NUM) / 2;
	//������Ϸ����
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
	//����
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(text_surface);
	SDL_RenderPresent(renderer);
	TTF_CloseFont(font);
	SDL_DestroyRenderer(renderer);
}
//�˳�����
void Quit()
{
	//������Դ���˳�SDL2
	SDL_DestroyWindow(game_window);
	Mix_Quit();
	TTF_Quit();
	SDL_Quit();
}
//�÷ּ���
int CountScore()
{
	int x, y;
	//�Ƿ�ɾ�����У������ж�����ɫʱ��
	SDL_bool delete_line;
	//������Ϸȫ������
	for (y = 0; y < SQURE_Y_NUM; y++)
	{
		delete_line = SDL_TRUE;
		//�����������ɫ����ʾɾ������
		for (x = 0; x < SQURE_X_NUM; x++)
		{
			if (game_data[x][y] == BACKGROUND_COLOR)
			{
				delete_line = SDL_FALSE;
				break;
			}
		}
		//ɾ�����У������������ɫ����
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
			//��ϲ�����ɫ
			for (i2 = 0; i2 < SQURE_X_NUM; i2++)
			{
				game_data[i2][0] = BACKGROUND_COLOR;
			}
			++score;
		}
	}
	return 0;
}
//����ͼ�ζ����̵߳ĺ���
int GraphicThread(void* ptr)
{
	//����ѭ��ʵ��һֱ����
	while (true)
	{
		//������Դ�������ȫ�ֱ�����
		SDL_LockMutex((SDL_mutex*)ptr);
		OperateShape(DOWN);
		//�����״������ϣ�����һ�ε÷ּ���
		if(shape_index == INVALID_INDEX)
			CountScore();
		//����ͼ��
		Paint();
		//�����Ϸ�������˳�ѭ��
		if (is_game_over == SDL_TRUE)
		{
			break;
		}
		//����
		SDL_UnlockMutex((SDL_mutex*)ptr);
		SDL_Delay(200);
	}
	//����
	SDL_UnlockMutex((SDL_mutex*)ptr);
	return 0;
}

int main(int argc, char** argv)
{
	//��ʼ��
	Init();
	SDL_Event event = {};
	//������
	SDL_mutex* mutex = SDL_CreateMutex();
	//����ͼ���߳�
	SDL_CreateThread(GraphicThread, u8"t1", mutex);
	//�¼�ѭ��
	while (event.type != SDL_QUIT)
	{
		//��ȡ�¼�
		SDL_PollEvent(&event);
		//�Լ����¼�������
		if (event.type == SDL_KEYDOWN)
		{
			int i = 0;
			int	repeat = !event.key.repeat ? 1 : event.key.repeat;
			//������Դ
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
			//����
			SDL_UnlockMutex(mutex);
		}
		//��Ϸ�������˳��¼�ѭ��
		if (is_game_over == SDL_TRUE)
		{
			break;
		}
	}
	//ɾ����
	SDL_DestroyMutex(mutex);
	//�˳�
	Quit();
	return 0;
}