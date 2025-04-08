#include <GL/glew.h>
#include <SOIL/SOIL.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <iostream>
#include <AL/alut.h>
#include <AL/alc.h>
#include <AL/al.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>



// Variaveis globais
GLuint textureID;
GLuint textureField;


// Configurações da janela e dimensões
const int windowWidth = 800;
const int windowHeight = 600;
const float paddleWidth = 1.0f;
const float paddleHeight = 1.0f;
const float paddleDepth = 4.0f;
const float wallThickness = 0.0f; // Espessura das paredes laterais

// Tamanho e posição da bola
const float ballSize = 0.8f;
float ballX = 0.0f;
float ballZ = 0.0f;
float ballVelocityX = 0.1f;
float ballVelocityZ = 0.1f;  // Bola se move horizontalmente no chão
float ballRotationAngle = 0.0f;
float ballRotationSpeed = 1.5f;

// Posições das raquetes
float leftPaddleZ = 0.0f;
float rightPaddleZ = 0.0f;

// Controle das teclas
bool upPressed = false;
bool downPressed = false;
bool wPressed = false;
bool sPressed = false;

// Controle do bot
bool isBotActive = false;

// Pontuação
int leftScore = 0;
int rightScore = 0;
void initSound(){
	alutInit(0,0);
}

// Carregar textura
GLuint loadTexture(const char* filename) {
    GLuint texture;
    int width, height;
    unsigned char* image = SOIL_load_image(filename, &width, &height, 0, SOIL_LOAD_RGBA);
    if (image == nullptr) {
        std::cerr << "Failed to load texture: " << SOIL_last_result() << std::endl;
        return 0;
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SOIL_free_image_data(image);
    return texture;
}

// inicializar texturas
void init() {
    textureID = loadTexture("bola3.png"); // Carrega a primeira textura
    textureField= loadTexture("campo.png"); // Carrega a segunda textura
}


// Função para configurar a iluminação
void setupLighting() {
    GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f }; // Posiciona a luz no centro da quadra
    GLfloat ambient_light[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat diffuse_light[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat specular_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular_light);

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}



// Funçao para iniciar o efeito sonoro
void playsound(const char* filename){
	ALuint buffer = alutCreateBufferFromFile(filename);
	ALuint source;
	alGenSources(1, &source);
	alSourcei(source, AL_BUFFER, buffer);
	alSourcePlay(source);
}

// Função para configurar as propriedades do material metálico
void setMetallicMaterialProperties() {
    GLfloat ambient_material[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat diffuse_material[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    GLfloat specular_material[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat shininess = 50.0f; // Brilho metálico

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_material);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_material);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular_material);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

void drawField() {
	glBindTexture(GL_TEXTURE_2D, textureField);
	glEnable(GL_TEXTURE_2D);
	
    // Desenha o chão
    glColor3f(0.0f, 1.0f, 0.0f); // Verde para o campo
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-15.0f, -1.0f, -10.0f);
	glTexCoord2f(1.0f, 0.0f);
    glVertex3f( 15.0f, -1.0f, -10.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f( 15.0f, -1.0f,  10.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-15.0f, -1.0f,  10.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

}
// Desenha a bola
void drawBall() {
	glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTranslatef(ballX, -0.2f, ballZ);
    glColor3f(1.0f, 1.0f, 1.0f);  // Posição da bola
    glRotatef(ballRotationAngle, ballVelocityZ, 0.0f, -ballVelocityX); // Rotação da esfera
    GLUquadric* quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE);
    gluSphere(quad, 0.8, 50, 50);     // Tamanho da bola
    gluDeleteQuadric(quad);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
	
}
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    if (isBotActive) {
        gluLookAt(-30.0f, 5.0f, 0.0f,    // Posição da câmera (mais alta e mais para trás)
                  0.0f, -0.5f, 0.0f,      // Ponto para onde a câmera está olhando
                  0.0f, 1.0f, 0.0f);      // Vetor "up" da câmera
    } else {
        gluLookAt(0.0f, 20.0f, 40.0f,    // Posição da câmera inicial
                  0.0f, 0.0f, 0.0f,      // Ponto para onde a câmera está olhando
                  0.0f, 1.0f, 0.0f);     // Vetor "up" da câmera
    }
    
    
    // Parede direita
    glPushMatrix();
    glColor3f(0.5f, 0.5f, 0.5f);
    glTranslatef(0.0f, 0.0f, 10.0f);
    glScalef(15.0f,0.8f,0.3f);
    glutSolidCube(2);
    glPopMatrix();
    
    
    // Parede esquerda
    glPushMatrix();
    glColor3f(0.5f, 0.5f, 0.5f);
    glTranslatef(0.0f, 0.0f, -10.0f);
    glScalef(15.0f,0.8f,0.3f);
    glutSolidCube(2);
    glPopMatrix();

    setupLighting();
    // Desenha o campo
    drawField();
    // Desenha a bola
    drawBall();
    
    
    // Desenha as raquetes
    glPushMatrix();
    glTranslatef(-14.0f, -0.5f, leftPaddleZ);
    glScalef(paddleWidth, paddleHeight, paddleDepth);
    glColor3f(0.5f, 0.5f, 0.5f); // Verde para a raquete esquerda
    setMetallicMaterialProperties(); // Aplica propriedades metálicas
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(14.0f, -0.5f, rightPaddleZ);
    glScalef(paddleWidth, paddleHeight, paddleDepth);
    glColor3f(0.5f, 0.5f, 0.5f); // Verde para a raquete direita
    setMetallicMaterialProperties(); // Aplica propriedades metálicas
    glutSolidCube(1.0);
    glPopMatrix();
    


    // Desenha a pontuação
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos3f(-10.0f, 9.0f, 0.0f);
    std::string leftScoreStr = "Player 1: " + std::to_string(leftScore);
    for (char c : leftScoreStr) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    glRasterPos3f(5.0f, 9.0f, 0.0f);
    if(isBotActive){
		std::string rightScoreStr = "Bot: " + std::to_string(rightScore);
    for (char c : rightScoreStr) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
	}else{
    std::string rightScoreStr = "Player 2: " + std::to_string(rightScore);
    for (char c : rightScoreStr) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
	}
    glutSwapBuffers();
}
void update(int value) {
    // Atualiza a posição da bola
    ballX += ballVelocityX;
    ballZ += ballVelocityZ;

    // Verifica colisões com as paredes laterais
    if (ballZ + ballSize > 10.0f || ballZ - ballSize < -10.0f) {
    	initSound();
    	playsound("pong.wav");
        ballVelocityZ = -ballVelocityZ;
    }
    //Calcula a rotaçao da bola
    ballRotationAngle += ballRotationSpeed;
    if (ballRotationAngle > 360.0f) {
        ballRotationAngle -= 360.0f;
    }

    // Verifica colisões com as raquetes
     if (ballX - ballSize < -14.0f && ballZ < leftPaddleZ + paddleDepth / 2 && ballZ > leftPaddleZ - paddleDepth / 2) {
        initSound();
    	playsound("pong.wav");
		ballVelocityX = -ballVelocityX;
		ballRotationSpeed *=1.1;
        ballVelocityX *= 1.03f; // Aumenta a velocidade da bola em 3%
        ballVelocityZ *= 1.03f; // Aumenta a velocidade da bola em 3%
    }
    if (ballX + ballSize > 14.0f && ballZ < rightPaddleZ + paddleDepth / 2 && ballZ > rightPaddleZ - paddleDepth / 2) {
        initSound();
    	playsound("pong.wav");
		ballVelocityX = -ballVelocityX;
		ballRotationSpeed *=1.1;
        ballVelocityX *= 1.03f; // Aumenta a velocidade da bola em 3%
        ballVelocityZ *= 1.03f; // Aumenta a velocidade da bola em 3%
    }

    // Verifica colisões com as paredes
    if (ballX - ballSize < -15.0f) {
    	initSound();
    	playsound("brasil.wav");
        rightScore++;
        ballX = 0.0f;
        ballZ = 0.0f;
        ballVelocityX = 0.1f; // Reseta a velocidade da bola
        ballRotationSpeed = 1.5f; // Reseta a velocidade de rotaçao da bola
        ballVelocityZ = 0.1f;
    }
    if (ballX + ballSize > 15.0f) {
    	initSound();
    	playsound("brasil.wav");
        leftScore++;
        ballX = 0.0f;
        ballZ = 0.0f;
        ballVelocityX = -0.1f; // Reseta a velocidade da bola
        ballRotationSpeed = 1.5f; // Reseta a velocidade de rotaçao da bola
        ballVelocityZ = -0.1f;
    }

    // Move as raquetes
    if (downPressed && rightPaddleZ + paddleDepth / 2 < 10) {
    	if(!isBotActive)
        rightPaddleZ += 0.5f;
    }
    if (upPressed && rightPaddleZ - paddleDepth / 2 > -10) {
    	if(!isBotActive)
        rightPaddleZ -= 0.5f;
    }
    if (sPressed && leftPaddleZ + paddleDepth / 2 < 10) {
        leftPaddleZ += 0.5f;
    }
    if (wPressed && leftPaddleZ - paddleDepth / 2 > -10) {
        leftPaddleZ -= 0.5f;
    }

    // Move o bot se estiver ativo
    if (isBotActive) {
        float botSpeed = 0.60f * sqrt(ballVelocityX * ballVelocityX + ballVelocityZ * ballVelocityZ);
        if (rightPaddleZ < ballZ && rightPaddleZ + paddleDepth / 2 < 10) {
            rightPaddleZ += botSpeed;
        }
        if (rightPaddleZ > ballZ && rightPaddleZ - paddleDepth / 2 > -10) {
            rightPaddleZ -= botSpeed;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}
// Controles
void handleKeys(unsigned char key, int x, int y) {
    if (key == 'w') {
        wPressed = true;
    }
    if (key == 's') {
        sPressed = true;
    }
    if (key == 'p') {
        isBotActive = !isBotActive; // Alterna entre bot e jogador
    }
    if (key == 27) { // ESC key
        exit(0);
    }
}

void handleKeysUp(unsigned char key, int x, int y) {
    if (key == 'w') {
        wPressed = false;
    }
    if (key == 's') {
        sPressed = false;
    }
}

void handleSpecialKeys(int key, int x, int y) {
    if (key == GLUT_KEY_UP) {
        upPressed = true;
    }
    if (key == GLUT_KEY_DOWN) {
        downPressed = true;
    }
}

void handleSpecialKeysUp(int key, int x, int y) {
    if (key == GLUT_KEY_UP) {
        upPressed = false;
    }
    if (key == GLUT_KEY_DOWN) {
        downPressed = false;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("3D Pong");
    init();
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)windowWidth / (double)windowHeight, 1.0, 200.0);

    glMatrixMode(GL_MODELVIEW);

    setupLighting(); // Configura a iluminação}
    glutDisplayFunc(display);
    glutTimerFunc(16, update, 0);
    glutKeyboardFunc(handleKeys);
    glutKeyboardUpFunc(handleKeysUp);
    glutSpecialFunc(handleSpecialKeys);
    glutSpecialUpFunc(handleSpecialKeysUp);

    glutMainLoop();
    return 0;
}
