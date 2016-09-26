#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

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
		glClearColor(0.4f, 0.2f, 0.4f, 1.0f);
		float lastFrameTicks = 0.0f;
		float x1 = 1.0f;
		float x2 = 2.0f;
		float count = 0.0f;
		ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

		GLuint harambeTxt = LoadTexture("harambe.png");
		GLuint kidTxt = LoadTexture("kid.png");
		GLuint gunTxt = LoadTexture("gunman.png");

		Matrix projectionMatrix;
		Matrix modelMatrix;
		Matrix viewMatrix;

		projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

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

		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elasped = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		//Harambe
		glBindTexture(GL_TEXTURE_2D, harambeTxt);
		
		float vertices[] = { x1, -0.5f, x2, -0.5f, x2, 0.5f, x1, -0.5f, x2, 0.5f, x1, 0.5f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
		if (count == 0.0f){
			x1 += elasped * 0.5;
			x2 += elasped *0.5;
			if (x1 >= 1.5f)
				count = 1.0f;
		}
		else if (count == 1.0f){
			x1 -= elasped * 0.5;
			x2 -= elasped *0.5;
			if (x1 <= 1.0f)
				count = 0.0f;
		}
		

		//Kid
		glBindTexture(GL_TEXTURE_2D, kidTxt);

		float vertices2[] = { 2.55f, -0.5f, 3.55f, -0.5f, 3.55f, 0.5f, 2.55f, -0.5f, 3.55f, 0.5f, 2.55f, 0.5f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
		glEnableVertexAttribArray(program.positionAttribute);
		float texCoords2[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords2);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//Sniper
		glBindTexture(GL_TEXTURE_2D, gunTxt);

		float vertices3[] = { -2.55f, -0.5f, -1.55f, -0.5f, -1.55f, 0.5f, -2.55f, -0.5f, -1.55f, 0.5f, -2.55f, 0.5f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices3);
		glEnableVertexAttribArray(program.positionAttribute);
		float texCoords3[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords3);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
