#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include "ShaderProgram.h"
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

using namespace std;

#define SPRITE_COUNT_X 24
#define SPRITE_COUNT_Y 16
#define TILE_SIZE 0.4f
#define LEVEL_HEIGHT 20
#define LEVEL_WIDTH 128
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

unsigned char **levelData;

ShaderProgram *program;
Matrix projectionMatrix;
Matrix modelMatrix;
Matrix viewMatrix;

Matrix playerMatrix;
Matrix enemyMatrix;
Matrix pointMatrix;

SDL_Window* displayWindow;

int mapHeight;
int mapWidth;

enum EntityType {Player, Enemy, Point};

class SheetSprite{
public:
	SheetSprite(){}
	void Draw(ShaderProgram *program){}
	float u;
	float v;
	float height;
	float width;
	unsigned int textureID;

};

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
GLuint sheet;

bool solidTile(int index){
	if (index == 74){
		return true;
	}
	else if (index == 99){
		return true;
	}
	else if (index == 1){
		return true;
	}
	else if (index == 2){
		return true;
	}
	return false;
}
void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY)
{
	*gridX = (int)(worldX / TILE_SIZE);
	*gridY = (int)(-worldY / TILE_SIZE);
}
float lerp(float v0, float v1, float t){
	return (1.0 - t)*v0 + t*v1;
}

class Entity{
public:
	Entity(){}
	Entity(float xPos, float yPos, int ind, string type){
		x = xPos;
		y = yPos;
		index = ind;
		if (type == "Player"){
			entityType = Player;
			friction_x = 0;
			acceleration_x = 0;
		}
		else if (type == "Enemy"){
			entityType = Enemy;
			friction_x = 0.5f;
			acceleration_x = -1.25f;
		}
		else if (type == "Point"){
			entityType = Point;
			friction_x = 0;
			acceleration_x = 0;
		}
	}
	void renderObject(ShaderProgram *program, Matrix& model){
		float u = (float)((index) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
		float v = (float)((index) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;

		float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
		float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;

		GLfloat texCoords[] = {
			u, v + spriteHeight,
			u + spriteWidth, v,
			u, v,
			u + spriteWidth, v,
			u, v + spriteHeight,
			u + spriteWidth, v + spriteHeight
		};

		float vertices[] = {
			-0.5f*TILE_SIZE, -0.5f*TILE_SIZE,
			0.5f*TILE_SIZE, 0.5f*TILE_SIZE,
			-0.5f*TILE_SIZE, 0.5f*TILE_SIZE,
			0.5f*TILE_SIZE, 0.5f*TILE_SIZE,
			-0.5f*TILE_SIZE, -0.5f*TILE_SIZE,
			0.5f*TILE_SIZE, -0.5f*TILE_SIZE
		};

		program->setModelMatrix(model);
		model.identity();
		model.Translate(x, y, 0.0f);

		glUseProgram(program->programID);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program->positionAttribute);
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program->texCoordAttribute);

		glBindTexture(GL_TEXTURE_2D, sheet);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);

	}
	void collided(Entity *entityEnemy, Entity *entityPoint){
		//between player and enemy
		if (entityType == Player){
			if (((x + width / 2 >= entityEnemy->x - width / 2) && (x + width/2 <= entityEnemy->x + width/2)) && 
				(y - height/2 <= entityEnemy->y + height/2)){
				hit = true;
			}
			else if (((x - width / 2 >= entityEnemy->x - width / 2) && (x - width / 2 <= entityEnemy->x + width / 2)) &&
				(y - height / 2 <= entityEnemy->y + height / 2)){
				hit = true;
			}
			if (((x + width / 2 >= entityPoint->x - width / 2) && (x + width / 2 <= entityPoint->x + width / 2)) &&
				(y - height / 2 <= entityPoint->y + height / 2)){
				win = true;
			}
			else if (((x - width / 2 >= entityPoint->x - width / 2) && (x - width / 2 <= entityPoint->x + width / 2)) &&
				(y - height / 2 <= entityPoint->y + height / 2)){
				win = true;
			}
		}

		//between player and plant
		

		int gridX = 0;
		int gridY = 0;

		//between entities and solid tiles

		//Bottom
		worldToTileCoordinates(x, y - height / 2, &gridX, &gridY);
		if (solidTile(levelData[gridY][gridX])){
			collidedBottom = true;
			velocity_y = 0;
			acceleration_y = 0;
			penetration_y = (-TILE_SIZE * gridY) - (y - height / 2);
			y += penetration_y;
		}
		else{
			collidedBottom = false;
			acceleration_y = -10.0f;
			penetration_y = 0;
		}
		//Right
		worldToTileCoordinates(x + width/2, y, &gridX, &gridY);
		if (solidTile(levelData[gridY][gridX])){
			collidedRight = true;
			velocity_x = 0;
			if (entityType == Enemy){
				acceleration_x *= -1.0f;
			}
			penetration_x = (x + width / 2) - (TILE_SIZE * gridX);
			x -= (penetration_x + 0.003);
		}
		else{
			collidedRight = false;
			penetration_x = 0;
		}
		//Left
		worldToTileCoordinates(x - width / 2, y, &gridX, &gridY);
		if (solidTile(levelData[gridY][gridX])){
			collidedLeft = true;
			velocity_x = 0;
			if (entityType == Enemy){
				acceleration_x *= -1.0f;
			}
			penetration_x = (TILE_SIZE * gridX) + TILE_SIZE - (x - width / 2);
			x += (penetration_x + 0.003);
		}
		else{
			collidedLeft = false;
			penetration_x = 0;
		}
	}

	void Update(float elapsed){
		velocity_x = lerp(velocity_x, 0.0f, elapsed * friction_x);
		velocity_y = lerp(velocity_y, 0.0f, elapsed * friction_y);
		if (entityType == Player){
			acceleration_x = 0;
			const Uint8 *keys = SDL_GetKeyboardState(NULL);
			if (keys[SDL_SCANCODE_RIGHT]){
				acceleration_x = 0.75f;
			}
			if (keys[SDL_SCANCODE_LEFT]){
				acceleration_x = -0.75f;
			}
			if (keys[SDL_SCANCODE_SPACE]){
				if (collidedBottom == true){
					velocity_y = 5.0f;
				}
			}
		}
		velocity_x += acceleration_x * elapsed;
		velocity_y += acceleration_y * elapsed;

		x += velocity_x * elapsed;
		y += velocity_y * elapsed;
	}

	float x;
	float y;
	float penetration_x = 0;
	float penetration_y = 0;
	float acceleration_x;
	float acceleration_y = -10.0;
	float velocity_x = 0;
	float velocity_y = 0;

	bool hit = false;
	bool win = false;

	float friction_x;
	float friction_y = 0.5;

	float height = TILE_SIZE;
	float width = TILE_SIZE;

	int index;

	bool collidedBottom = false;
	bool collidedTop = false;
	bool collidedLeft = false;
	bool collidedRight = false;
	
	EntityType entityType;

};

Entity player, enemy, point;

void centerPlayer(){
	viewMatrix.identity();	
	viewMatrix.Scale(2.0f, 2.0f, 1.0f);
	viewMatrix.Translate(-player.x, -player.y, 0.0f);
	program->setViewMatrix(viewMatrix);
}

void placeEntity(string type, float placeX, float placeY){
	if (type == "Player"){
		player = Entity(placeX, placeY, 90, type);
	}
	else if (type == "Enemy"){
		enemy = Entity(placeX, placeY, 136, type);
	}
	else if (type == "Point"){
		point = Entity(placeX, placeY, 184, type);
	}
}

bool readHeader(std::ifstream &stream){
	string line = "";
	mapWidth = -1;
	mapHeight = -1;
	while (getline(stream, line)){
		if (line == ""){ break; }

		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);

		if (key == "width"){
			mapWidth = atoi(value.c_str());
		}
		else if (key == "height"){
			mapHeight = atoi(value.c_str());
		}
	}

	if (mapWidth == -1 || mapHeight == -1){
		return false;
	}
	else{
		levelData = new unsigned char*[mapHeight];
		for (int i = 0; i < mapHeight; ++i){
			levelData[i] = new unsigned char[mapWidth];
		}
		return true;
	}
}
bool readLayerData(std::ifstream &stream){
	string line = "";
	while (getline(stream, line)){
		if (line == ""){ break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "data"){
			for (int y = 0; y < mapHeight; y++){
				getline(stream, line);
				istringstream lineStream(line);
				string tile;

				for (int x = 0; x < mapWidth; x++){
					getline(lineStream, tile, ',');
					unsigned char val = (unsigned char)atoi(tile.c_str());
					if (val > 0){
						levelData[y][x] = val - 1;
					}
					else{
						levelData[y][x] = 0;
					}
				}
			}
		}
	}
	return true;
}
bool readEntityData(std::ifstream &stream){
	string line = "";
	string type = "";

	while (getline(stream, line)){
		if (line == ""){ break; }

		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);

		if (key == "type"){
			type = value;
		}
		else if (key == "location"){
			istringstream lineStream(value);
			string xPosition, yPosition;
			getline(lineStream, xPosition, ',');
			getline(lineStream, yPosition, ',');

			float placeX = (float)atoi(xPosition.c_str()) * TILE_SIZE;
			float placeY = (float)atoi(yPosition.c_str()) * -TILE_SIZE + 0.5;

			placeEntity(type, placeX, placeY);
		}
	}
	return true;
}

std::vector<float> vertexData;
std::vector<float> texCoordData;
void makeMap(){
	for (int y = 0; y < LEVEL_HEIGHT; y++){
		for (int x = 0; x < LEVEL_WIDTH; x++){
			if (levelData[y][x] != 0){
				float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
				float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;

				float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
				float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;

				vertexData.insert(vertexData.end(), {
					TILE_SIZE * x, -TILE_SIZE * y,
					TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,

					TILE_SIZE * x, -TILE_SIZE * y,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
				});

				texCoordData.insert(texCoordData.end(), {
					u, v,
					u, v + (spriteHeight),
					u + spriteWidth, v + (spriteHeight),

					u, v,
					u + spriteWidth, v + (spriteHeight),
					u + spriteWidth, v
				});
			}
		}
	}
}

void drawMap(){
	program->setModelMatrix(modelMatrix);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);

	modelMatrix.identity();
	program->setModelMatrix(modelMatrix);

	glBindTexture(GL_TEXTURE_2D, sheet);
	glDrawArrays(GL_TRIANGLES, 0, vertexData.size() / 2);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void Update(float elapsed){
	player.Update(elapsed);
	player.collided(&enemy, &point);

	enemy.Update(elapsed);
	enemy.collided(&player, &point);

	point.Update(elapsed);
	point.collided(&player, &point);
}

void Render(){
	player.renderObject(program, playerMatrix);
	centerPlayer();

	enemy.renderObject(program, enemyMatrix);
	point.renderObject(program, pointMatrix);
}

void DrawText(ShaderProgram *program, GLuint fontTexture, std::string text, float size, float spacing){
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData1;
	std::vector<float> texCoordData1;

	for (int i = 0; i < text.size(); i++){
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		vertexData1.insert(vertexData1.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData1.insert(texCoordData1.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}
	glUseProgram(program->programID);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData1.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData1.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

		glViewport(0, 0, 640, 360);
		sheet = LoadTexture("dirt-tiles.png");
		GLuint font = LoadTexture("font1.png");

		//player = Entity(0.0f, -3.0f, 90);

		//File read
		ifstream infile("map.txt");
		string line;
		while (getline(infile, line)){
			if (line == "[header]"){
				if (!readHeader(infile)){
					break;
				}
			}
			else if (line == "[layer]"){
				readLayerData(infile);
			}
			else if (line == "[Object Layer 1]"){
				readEntityData(infile);
			}
		}

		float lastFrameTicks = 0.0f;
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		lastFrameTicks = ticks;

		makeMap();

		program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

		projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -4.0f, 4.0f, -1.0f, 1.0f);

		enum GameState { STATE_START, STATE_LOSE, STATE_WIN };

		int state = STATE_START;

		glUseProgram(program->programID);

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		program->setProjectionMatrix(projectionMatrix);
		program->setViewMatrix(viewMatrix);

		glEnable(GL_BLEND);

			drawMap();

			float fixedElapsed = elapsed;
			if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS){
				fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
			}
			while (fixedElapsed >= FIXED_TIMESTEP){
				fixedElapsed -= FIXED_TIMESTEP;
				Update(FIXED_TIMESTEP);
			}
			Update(fixedElapsed);

			Render();

			if (player.hit == true){
				state = STATE_LOSE;
			}
			else if (player.win == true){
				state = STATE_WIN;
			}

			if (state == STATE_LOSE){
				centerPlayer();
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				modelMatrix.identity();
				modelMatrix.Translate(player.x - 1.3f, player.y, 0.0f);
				program->setModelMatrix(modelMatrix);
				DrawText(program, font, "YOU LOSE", 0.3f, 0.08f);
			}
			else if (state == STATE_WIN){
				centerPlayer();
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				modelMatrix.identity();
				modelMatrix.Translate(player.x - 1.1f, player.y, 0.0f);
				program->setModelMatrix(modelMatrix);
				DrawText(program, font, "YOU WIN", 0.3f, 0.08f);
			}


		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
