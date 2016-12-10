#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include <vector>
#include <algorithm>
#include <iostream>

#define FIXED_TIMESTEP 0.0000666f
#define MAX_TIMESTEPS 6

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

using namespace std;

SDL_Window* displayWindow;
ShaderProgram* program;

class Vector{
public:
	Vector(){}
	Vector(float x1, float y1, float z1){
		x = x1;
		y = y1;
		z = z1;
	}
	void normalize(){
		float length = sqrt(x*x + y*y);
		x /= length;
		y /= length;
	}
	float x;
	float y;
	float z;
};

bool testSATSeparationForEdge(float edgeX, float edgeY, const std::vector<Vector> &points1, const std::vector<Vector> &points2) {
	float normalX = -edgeY;
	float normalY = edgeX;
	float len = sqrtf(normalX*normalX + normalY*normalY);
	normalX /= len;
	normalY /= len;

	std::vector<float> e1Projected;
	std::vector<float> e2Projected;

	for (int i = 0; i < points1.size(); i++) {
		e1Projected.push_back(points1[i].x * normalX + points1[i].y * normalY);
	}
	for (int i = 0; i < points2.size(); i++) {
		e2Projected.push_back(points2[i].x * normalX + points2[i].y * normalY);
	}

	std::sort(e1Projected.begin(), e1Projected.end());
	std::sort(e2Projected.begin(), e2Projected.end());

	float e1Min = e1Projected[0];
	float e1Max = e1Projected[e1Projected.size() - 1];
	float e2Min = e2Projected[0];
	float e2Max = e2Projected[e2Projected.size() - 1];
	float e1Width = fabs(e1Max - e1Min);
	float e2Width = fabs(e2Max - e2Min);
	float e1Center = e1Min + (e1Width / 2.0);
	float e2Center = e2Min + (e2Width / 2.0);
	float dist = fabs(e1Center - e2Center);
	float p = dist - ((e1Width + e2Width) / 2.0);

	if (p < 0) {
		return true;
	}
	return false;
}

bool checkSATCollision(const std::vector<Vector> &e1Points, const std::vector<Vector> &e2Points) {
	for (int i = 0; i < e1Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e1Points.size() - 1) {
			edgeX = e1Points[0].x - e1Points[i].x;
			edgeY = e1Points[0].y - e1Points[i].y;
		}
		else {
			edgeX = e1Points[i + 1].x - e1Points[i].x;
			edgeY = e1Points[i + 1].y - e1Points[i].y;
		}

		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
		if (!result) {
			return false;
		}
	}
	for (int i = 0; i < e2Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e2Points.size() - 1) {
			edgeX = e2Points[0].x - e2Points[i].x;
			edgeY = e2Points[0].y - e2Points[i].y;
		}
		else {
			edgeX = e2Points[i + 1].x - e2Points[i].x;
			edgeY = e2Points[i + 1].y - e2Points[i].y;
		}
		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
		if (!result) {
			return false;
		}
	}
	return true;
}


class Rectan{
public:
	Rectan(){}
	Rectan(float r, float s1, float s2){
		rotation = r;
		scaleX = s1;
		scaleY = s2;
		acceleration = -1.0f;
	}
	void setVector(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float x5, float y5, float x6, float y6){
		points.push_back(Vector(x1, y1, 0));
		points.push_back(Vector(x2, y2, 0));
		points.push_back(Vector(x3, y3, 0));
		points.push_back(Vector(x4, y4, 0));
		points.push_back(Vector(x5, y5, 0));
		points.push_back(Vector(x6, y6, 0));
		modified = points;
	}
	void Update(float elapsed){
		rotation += elapsed * 2.0f;
		xMove += elapsed*acceleration;
	}
	void Draw(){
		program->setModelMatrix(this->modelMatrix);

		this->modelMatrix.identity();

		this->modelMatrix.Translate(xMove, yMove, 0.0f);
		this->modelMatrix.Rotate(rotation);
		this->modelMatrix.Scale(scaleX, scaleY, 1.0f);


		for (int i = 0; i < modified.size(); i++){
			float xPos = points.at(i).x*cos(rotation) - points.at(i).y*sin(rotation);
			float yPos = points.at(i).x*sin(rotation) + points.at(i).y*cos(rotation);
			xPos *= scaleX;
			yPos *= scaleY;
			xPos = xMove + xPos;
			yPos = yMove + yPos;
			modified.at(i).x = xPos;
			modified.at(i).y = yPos;
		}

		glUseProgram(program->programID);

		float vertices[] = { points.at(0).x, points.at(0).y, points.at(1).x, points.at(1).y, points.at(2).x, points.at(2).y, 
			points.at(3).x, points.at(3).y, points.at(4).x, points.at(4).y, points.at(5).x, points.at(5).y};
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program->positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program->positionAttribute);
	}
	Matrix modelMatrix;
	float xMove;
	float yMove;
	float rotation;
	float scaleX;
	float scaleY;
	float acceleration;
	std::vector<Vector> points;
	std::vector<Vector> modified;
};

void collision(Rectan& one, Rectan &two){
	int maxChecks = 10;
	while (checkSATCollision(one.modified, two.modified) && maxChecks > 0){
		Vector responseVector = Vector(one.xMove - two.xMove, one.yMove - two.yMove, 0.0f);
		responseVector.normalize();

		one.xMove -= responseVector.x * 0.002;
		one.yMove -= responseVector.y * 0.002;

		two.xMove -= responseVector.x * 0.002;
		two.yMove -= responseVector.y * 0.002;
		maxChecks -= 1;
	}
}

void Update(float elapsed, Rectan& one, Rectan& two, Rectan& three){
	one.Update(elapsed);
	two.Update(elapsed);
	three.Update(elapsed);
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
	program = new ShaderProgram(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	glUseProgram(program->programID);

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;

	Rectan one(3.14*45/180, 2.0f, 2.0f);
	one.setVector(-0.07f, -0.07f, 0.07f, -0.07f, 0.07f, 0.07f, -0.07f, -0.07f, 0.07, 0.07f, -0.07, 0.07f);
	one.xMove = 0.0f;
	one.yMove = 0.5f;
	one.acceleration = 1.0f;
	Rectan two(3.14 * 135 / 180, 3.0f, 3.0f);
	two.setVector(-0.07f, -0.07f, 0.07f, -0.07f, 0.07f, 0.07f, -0.07f, -0.07f, 0.07, 0.07f, -0.07, 0.07f);
	two.xMove = 1.0f;
	two.yMove = 0.5f;
	Rectan three(0.0f, 5.0f, 5.0f);
	three.setVector(-0.07f, -0.07f, 0.07f, -0.07f, 0.07f, 0.07f, -0.07f, -0.07f, 0.07, 0.07f, -0.07, 0.07f);
	three.xMove = 3.0f;
	three.yMove = 1.0f;

	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	float lastFrameTicks = 0.0f;
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	lastFrameTicks = ticks;

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

		glClear(GL_COLOR_BUFFER_BIT);

		program->setProjectionMatrix(projectionMatrix);
		program->setViewMatrix(viewMatrix);
		program->setModelMatrix(modelMatrix);

		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_W]){
			one.yMove += elapsed * 0.5;
		}
		else if (keys[SDL_SCANCODE_S]){
			one.yMove -= elapsed * 0.5;
		}
		if (keys[SDL_SCANCODE_D]){
			one.xMove += elapsed * 0.5;
		}
		else if (keys[SDL_SCANCODE_A]){
			one.xMove -= elapsed * 0.5;
		}

		float fixedElapsed = elapsed;
		if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS){
			fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
		}
		while (fixedElapsed >= FIXED_TIMESTEP){
			fixedElapsed -= FIXED_TIMESTEP;
			one.Update(FIXED_TIMESTEP);
			two.Update(FIXED_TIMESTEP);
			three.Update(FIXED_TIMESTEP);
		}
		one.Update(FIXED_TIMESTEP);
		two.Update(FIXED_TIMESTEP);
		three.Update(FIXED_TIMESTEP);

		one.Draw();
		two.Draw();
		three.Draw();

		if (checkSATCollision(one.modified, two.modified)){
			one.acceleration *= -1.0f;
			two.acceleration *= -1.0f;
		}
		if (checkSATCollision(one.modified, three.modified)){
			one.acceleration *= -1.0f;
			three.acceleration *= -1.0f;
		}
		if (checkSATCollision(two.modified, three.modified)){
			two.acceleration *= -1.0f;
			three.acceleration *= -1.0f;
		}

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
