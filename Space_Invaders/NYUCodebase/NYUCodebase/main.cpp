#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>
#include <algorithm>
#include "ShaderProgram.h"


#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

float twidth = 1.0 / 8;
float theight = 1.0 / 4;

GLuint sheet;

GLuint LoadTexture(const char *image_path){
	SDL_Surface *surface = IMG_Load(image_path);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SDL_FreeSurface(surface);

	return textureID;
}

void DrawText(ShaderProgram *program, GLuint fontTexture, std::string text, float size, float spacing){
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	for (int i = 0; i < text.size(); i++){
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}
	glUseProgram(program->programID);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
	
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

class SheetSprite {
public:
	SheetSprite(unsigned int xtextureID, float tu, float tv, float twidth, float theight, float tsize){
		u = tu;
		v = tv;
		width = twidth;
		height = theight;
		size = tsize;
		textureID = xtextureID;
	}
	SheetSprite(){}
	void Draw(ShaderProgram *program);

	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};

void SheetSprite::Draw(ShaderProgram *program){

	GLfloat texCoords[] = {
		u, v + height,
		u+width, v,
		u, v,
		u+width, v,
		u, v+height,
		u+width, v+height
	};

	float aspect = width / height;

	twidth = 0.5f * size + 0.5f * size;
	theight = 0.5f * size * aspect + 0.5f * size * aspect;

	float vertices[] = {
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, 0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, -0.5f * size
	};
	glUseProgram(program->programID);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);

}

class Vector3 {
public:
	Vector3(float x, float y, float z);

	float x;
	float y;
	float z;
};

class Entity{
public:
	Entity(){}
	Entity(float x, float y, float type){
		xPos = x;
		yPos = y;
	}
	void Draw();
	void Update(float elapsed){
		if (collide == false){
			yPos += directionY*elapsed;
			xPos += directionX*elapsed;
			if (yPos >= 4.0f){
				directionY = 0.0f;
				yPos = -5.0f;
			}
			if (xPos + twidth / 2 >= 5.0f){
				xPos -= 0.3f;
				directionX *= -1.0f;
				yPos -= (2.0f*theight);
			}
			else if (xPos <= -5.0f){
				xPos += 0.3f;
				directionX *= -1.0f;
				yPos -= (2.0f*theight);
			}
		}
		else if (collide == true){
			xPos = -7.0f;
			yPos = -6.0f;
		}
	}

	void ifCollide(Entity enemy){ //0.83      0.75
		if (enemy.yPos <= this->yPos){
			collide = true;
		}
		collide =  false;
	}


	bool collide = false;
	float type;
	float xPos;
	float yPos;
	float directionY = 0.0f;
	float directionX = 0.0f;

	SheetSprite sprite;
};

Entity player(0.0f, -3.3f, 0.0f);
Entity bullet(0, -10, 2);

void shootBullet(){
	bullet.collide = false;
	bullet.xPos = player.xPos;
	bullet.yPos = player.yPos + (2.0f*theight);// 31.0f / 1024.0f;
	bullet.directionY = 2.0f;
}
int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 700, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 700, 600);

	float lastFrameTicks = 0.0f;
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	lastFrameTicks = ticks;

	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	GLuint font = LoadTexture("font1.png");
	sheet = LoadTexture("sheet.png");

	float x = -4.1f;
	float y = 3.3f;
	float d = 1.0f;
	int count = 0;

	Entity enemies[10];
	for (int i = 0; i < 10; i++){
		enemies[i].sprite = SheetSprite(sheet, 425.0f / 1024.0f, 552.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 0.75);
		enemies[i].directionX = d * 2.0f;
		if (count == 5){
			y -= 2 * theight + 0.3;
			count = 0;
			x = -4.1f;
			enemies[i].xPos = x;
			enemies[i].yPos = y;
			d *= -1.0f;
			enemies[i].directionX *= -1.0f;
		}
		else{
			x += 0.5f * enemies[i].sprite.size + 0.5f * enemies[i].sprite.size + 0.5f;
			enemies[i].xPos = x;
			enemies[i].yPos = y;
			count++;
		}
	}


	player.sprite = SheetSprite(sheet, 535.0f / 1024.0f, 75.0f / 1024.0f, 51.0f / 1024.0f, 75.0f / 1024.0f, 0.75);

	bullet.sprite = SheetSprite(sheet, 834.0f / 1024.0f, 299.0f / 1024.0f, 14.0f / 1024.0f, 31.0f / 1024.0f, 0.75);



	//TEXT TO BE DISPLAYED
	std::string text = "SPACE INVADERS";
	std::string instruct = "PRESS SPACE TO SHOOT";
	std::string instruct2 = "ARROWS TO MOVE";
	std::string start = "E TO START";
	std::string lose = "YOU DIE";
	std::string win = "YOU ARE VICTORIOUS";

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-5.0f, 5.0f, -4.0f, 4.0f, -1.0f, 1.0f);

	enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL };

	int state = STATE_MAIN_MENU;

	glUseProgram(program.programID);


	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);

		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);
		program.setModelMatrix(modelMatrix);

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elasped = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		//RENDERING
		switch (state){
		case STATE_MAIN_MENU:
			modelMatrix.identity();
			modelMatrix.Translate(-4.0f, 2.0f, 0.0f);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, text, 0.5f, 0.15f);

			modelMatrix.identity();
			modelMatrix.Translate(-3.5f, 1.5f, 0.0f);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, instruct, 0.25f, 0.15f);

			modelMatrix.identity();
			modelMatrix.Translate(-2.5f, 1.0f, 0.0f);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, instruct2, 0.25f, 0.15f);

			modelMatrix.identity();
			modelMatrix.Translate(-2.5f, 0.0f, 0.0f);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, start, 0.5f, 0.15f);
			break;
		case STATE_GAME_LEVEL:
			int count = 0;
			for (int i = 0; i < 10; i++){
				if (enemies[i].collide == true){ count++; }
				else{
					modelMatrix.identity();
					modelMatrix.Translate(enemies[i].xPos, enemies[i].yPos, 0.0f);
					program.setModelMatrix(modelMatrix);
					enemies[i].sprite.Draw(&program);
					if (enemies[i].yPos - 0.75 < -4.0f){
						modelMatrix.identity();
						modelMatrix.Translate(-3.5f, 1.5f, 0.0f);
						program.setModelMatrix(modelMatrix);
						DrawText(&program, font, lose, 0.5f, 0.15f);
					}
				}
			}
			if (count == 10){
				modelMatrix.identity();
				modelMatrix.Translate(-3.5f, 1.5f, 0.0f);
				program.setModelMatrix(modelMatrix);
				DrawText(&program, font, win, 0.25f, 0.15f);
			}

			modelMatrix.identity();
			modelMatrix.Translate(player.xPos, -3.3f, 0.0f);
			program.setModelMatrix(modelMatrix);
			player.sprite.Draw(&program);

			modelMatrix.identity();
			modelMatrix.Translate(bullet.xPos, bullet.yPos, 0.0f);
			program.setModelMatrix(modelMatrix);
			bullet.sprite.Draw(&program);
			break;
		}

		//UPDATE
		switch (state){
		case STATE_MAIN_MENU:
			modelMatrix.identity();
			modelMatrix.Translate(-4.0f, 2.0f, 0.0f);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, text, 0.5f, 0.15f);

			modelMatrix.identity();
			modelMatrix.Translate(-3.5f, 1.5f, 0.0f);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, instruct, 0.25f, 0.15f);

			modelMatrix.identity();
			modelMatrix.Translate(-2.5f, 1.0f, 0.0f);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, instruct2, 0.25f, 0.15f);

			modelMatrix.identity();
			modelMatrix.Translate(-2.5f, 0.0f, 0.0f);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, start, 0.5f, 0.15f);
			break;
		case STATE_GAME_LEVEL:
			for (int i = 0; i < 10; i++){
				enemies[i].Update(elasped);
			}
			bullet.Update(elasped);
			for (int i = 0; i < 10; i++){
				if (enemies[i].collide != true){
					if (bullet.xPos >= enemies[i].xPos && bullet.xPos <= enemies[i].xPos + 0.83 && bullet.yPos >= enemies[i].yPos-0.75 &&
						bullet.yPos <= enemies[i].yPos){
						bullet.collide = true;
						enemies[i].collide = true;
						bullet.yPos -= 0.2;
						bullet.xPos = 0.0f;
						bullet.yPos = 10.0f;

						break;
					}
				}
			}
			bullet.collide = false;
			break;
		}


			//KEY INPUT
			if (keys[SDL_SCANCODE_E]){
				state = STATE_GAME_LEVEL;
			}
			if (keys[SDL_SCANCODE_RIGHT]){
				player.xPos += elasped * 2.0f;
				if (player.xPos + twidth/2 > 5.0f){
					player.xPos -= 0.4;
				}
			}
			else if (keys[SDL_SCANCODE_LEFT]){
				player.xPos -= elasped * 2.0f;
				if (player.xPos < -5.0f){
					player.xPos += 0.4;
				}
			}
			if (keys[SDL_SCANCODE_SPACE]){
				if (bullet.directionY != 0.0f){}
				else
					shootBullet();
			}

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			SDL_GL_SwapWindow(displayWindow);
		}

		SDL_Quit();
		return 0;
	}
