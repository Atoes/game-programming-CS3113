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

//TO WHOMEVER IS READING THIS, FOR SOME REASONS THERE MAY BE A NEED TO BUILD AND RUN MULTIPLE TIMES FOR THE GAME TO START OFF CORRECTLY. SORRY BUT I DON"T KNOW WHY
//BUT EVERYTIME I EXIT THE VISUAL STUDIO AND THEN TRY AGAIN, IT STARTS OFF WEIRD. TO REPEAT, BUILD AND RUN MULTIPLE TIMES IF THERE IS A PROBLEM.

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
	
		float lastFrameTicks = 0.0f;
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		//float elasped = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
	
		float x1 = 1.0f;
		float x2 = 2.0f;
		float count = 0.0f;
		//ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
		ShaderProgram program(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");

		GLuint p1Txt = LoadTexture("white.png");
		float p1Y = 0.0f;
		GLuint p2Txt = LoadTexture("white.png");
		float p2Y = 0.0f;
		GLuint ballTxt = LoadTexture("white.png");
		float bX = 0.0f;
		float bY = 0.0f;
		float directionX = 1.0f;
		float directionY = -1.0f;
		float angleMod = 4.0f;
		int tweak = 0;
		int win = 0;

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

			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);

			float ticks = (float)SDL_GetTicks() / 1000.0f;
			float elasped = ticks - lastFrameTicks;
			lastFrameTicks = ticks;

			//Update
			const Uint8 *keys = SDL_GetKeyboardState(NULL);
			//player one movement
			if (keys[SDL_SCANCODE_W]){
				p1Y += elasped * 2.0;
			}
			else if (keys[SDL_SCANCODE_S]){
				p1Y -= elasped * 2.0;
			}

			//player two movement
			if (keys[SDL_SCANCODE_UP]){
				p2Y += elasped * 2.0;
			}
			else if (keys[SDL_SCANCODE_DOWN]){
				p2Y -= elasped * 2.0;
			}

			//ball movement
			bX += cos(3.14 / angleMod)*elasped*1.75*directionX;
			bY += sin(3.14 / angleMod)*elasped*1.75*directionY;

			//ball collision with floor and ceiling
			if (0.07f + bY >= 2.0f || -0.07 + bY <= -2.0f){
				directionY *= -1.0f;
				//generates a random value which picks out a certain way of changing the angle so that there is variety of movement in game
				tweak = (rand() % 5);
				if (tweak == 0){
					angleMod = 4.0f;
				}
				else if (tweak == 1){
					angleMod = 3.8f;
				}
				else if (tweak == 2){
					angleMod = 3.6f;
				}
				else if (tweak == 3){
					angleMod = 4.2f;
				}
				else if (tweak == 4){
					angleMod = 4.4f;
				}
			}

			//ball collision with player 1
			if ((-0.07f + bX <= -3.30f) && (0.07 + bY <= 0.4 + p1Y) && (-0.07 + bY >= -0.4 + p1Y)){
				directionX *= -1.0f;
				tweak = (rand() % 5);
				if (tweak == 0){
					angleMod = 4.0f;
				}
				else if (tweak == 1){
					angleMod = 3.8f;
				}
				else if (tweak == 2){
					angleMod = 3.6f;
				}
				else if (tweak == 3){
					angleMod = 4.2f;
				}
				else if (tweak == 4){
					angleMod = 4.4f;
				}
			}

			//ball collision with player 2
			if ((0.07f + bX >= 3.30f) && (0.07 + bY <= 0.4 + p2Y) && (-0.07 + bY >= -0.4 + p2Y)){
				directionX *= -1.0f;
				tweak = (rand() % 5);
				if (tweak == 0){
					angleMod = 4.0f;
				}
				else if (tweak == 1){
					angleMod = 3.8f;
				}
				else if (tweak == 2){
					angleMod = 3.6f;
				}
				else if (tweak == 3){
					angleMod = 4.2f;
				}
				else if (tweak == 4){
					angleMod = 4.4f;
				}
			}

			//win conditions
			if (0.07 + bX >= 3.45){
				win = 1;
			}
			else if (-0.07 + bX < -3.45){
				win = 2;
			}

			//Player one
			//condition that holds as long as the other player didn't win, if there is a winner, the loser disappears
			if (win != 2){
				modelMatrix.identity();
				modelMatrix.Translate(0.0f, p1Y, 1.0f);

				program.setModelMatrix(modelMatrix);

				//glBindTexture(GL_TEXTURE_2D, p1Txt);

				float vertices[] = { -3.45, -0.4f, -3.30, -0.4f, -3.30, 0.4f, -3.45, -0.4f, -3.30, 0.4f, -3.45, 0.4f };
				glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
				glEnableVertexAttribArray(program.positionAttribute);
				float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
				//glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
				//glEnableVertexAttribArray(program.texCoordAttribute);

				glDrawArrays(GL_TRIANGLES, 0, 6);

				glDisableVertexAttribArray(program.positionAttribute);
				//glDisableVertexAttribArray(program.texCoordAttribute);
			}
			//Player two
			if (win != 1){
				modelMatrix.identity();
				modelMatrix.Translate(0.0f, p2Y, 1.0f);

				program.setModelMatrix(modelMatrix);

				//glBindTexture(GL_TEXTURE_2D, p2Txt);

				float vertices2[] = { 3.30f, -0.4f, 3.45f, -0.4f, 3.45f, 0.4f, 3.30f, 0.4f, 3.45f, 0.4f, 3.30f, 0.4f };
				glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
				glEnableVertexAttribArray(program.positionAttribute);
				float texCoords2[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
				//glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords2);
				//glEnableVertexAttribArray(program.texCoordAttribute);

				glDrawArrays(GL_TRIANGLES, 0, 6);

				glDisableVertexAttribArray(program.positionAttribute);
				//glDisableVertexAttribArray(program.texCoordAttribute);
			}
			//Ball

			modelMatrix.identity();
			modelMatrix.Translate(bX, bY, 1.0f);

			program.setModelMatrix(modelMatrix);

			//glBindTexture(GL_TEXTURE_2D, ballTxt);

			float vertices3[] = { -0.07f, -0.07f, 0.07f, -0.07f, 0.07f, 0.07f, -0.07f, -0.07f, 0.07, 0.07f, -0.07, 0.07f };
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices3);
			glEnableVertexAttribArray(program.positionAttribute);
			float texCoords3[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
			//glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords3);
			//glEnableVertexAttribArray(program.texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program.positionAttribute);
			//glDisableVertexAttribArray(program.texCoordAttribute);

			SDL_GL_SwapWindow(displayWindow);
		}

		SDL_Quit();
		return 0;
}
