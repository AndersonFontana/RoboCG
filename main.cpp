//*****************************************************
//
//		Construção de um robô para disciplina de
//				   Computação Gráfica
//
//		Prof:      Evandro L. Viapiana
//
//		Aluno:	   Anderson A. Fontana
//
//
//*****************************************************

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <math.h>

#if __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <GLUT/glut.h>
	#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif linux
	#include <GL/glut.h>
#else
	#include <gl/glut.h>
#endif

using namespace std;

#ifndef M_PI
#define M_PI 3.1415926
#endif

#define LARGURA		800
#define ALTURA		600

#define NUM_OBJETOS   9
#define CENA          0
#define ROBO          1
#define TRONCO        2
#define CABECA        3
#define BRACO_D       4
#define BRACO_E       5
#define QUADRIL       6
#define PERNA_D       7
#define PERNA_E       8

#define NUM_TEX       5
#define TEXTURA1   1000
#define TEXTURA2   1001
#define TEXTURA3   1002
#define TEXTURA4   1003
#define TEXTURA5   1004
#define TEXTURA6   1005

GLuint  texture_id[ NUM_TEX ];

struct tipo_transformacao{
	GLfloat dx, dy, dz;         // paramatros de translacao
	GLfloat sx, sy, sz;         // parametros de escala
	GLfloat angx , angy , angz; // parametros de rotacao
};

// definicao dos valores de transformacao
struct tipo_transformacao transf[ NUM_OBJETOS ];

// Variáveis para controles de navegação
GLfloat angle, fAspect;
GLfloat rotX, rotY, rotX_ini, rotY_ini;
GLfloat obsX, obsY, obsZ, obsX_ini, obsY_ini, obsZ_ini;
GLint x_ini, y_ini, bot, objeto, passo;
char transformacao, eixo;

string objetos[] = {"cena", "robô", "tronco", "cabeça", "braço direito", "braço esquerdo", "quadril", "perna direita", "perna esquerda"};
/*
  Função responsável pela carga de
  um arquivo BMP

  Esta função utiliza leitura direta do BMP sem
  a necessidade de outras bibliotecas assim
  segue abaixo a descrição de cada deslocamento
  do Header.
  Referencia :http://www.fastgraph.com/help/bmp_header_format.html

 Formato do header de arquivos BMP (Windows)
 Windows BMP files begin with a 54-byte header:
 offset  size   description
   0       2     signature, must be 4D42 hex
   2       4     size of BMP file in bytes (unreliable)
   6       2     reserved, must be zero
   8       2     reserved, must be zero
  10       4     offset to start of image data in bytes
  14       4     size of BITMAPINFOHEADER structure, must be 40
  18       4     image width in pixels
  22       4     image height in pixels
  26       2     number of planes in the image, must be 1
  28       2     number of bits per pixel (1, 4, 8, or 24)
  30       4     compression type (0=none, 1=RLE-8, 2=RLE-4)
  34       4     size of image data in bytes (including padding)
  38       4     horizontal resolution in pixels per meter (unreliable)
  42       4     vertical resolution in pixels per meter (unreliable)
  46       4     number of colors in image, or zero
  50       4     number of important colors, or zero
*/
int LoadBMP(char* filename)
{
	#define SAIR        {fclose(fp_arquivo); return -1;}
	#define CTOI(C)     (*(int*)&C)

	GLubyte     *image;
	GLubyte     Header[0x54];
	GLuint      DataPos, imageSize;
	GLsizei     Width,Height;

	// Abre o arquivo e efetua a leitura do Header do arquivo BMP
	FILE * fp_arquivo = fopen(filename,"rb");
	if (!fp_arquivo)
		return -1;
	if (fread(Header,1,0x36,fp_arquivo)!=0x36)
		SAIR;
	if (Header[0]!='B' || Header[1]!='M')
		SAIR;
	if (CTOI(Header[0x1E])!=0)
		SAIR;
	if (CTOI(Header[0x1C])!=24)
		SAIR;

	// cout << "ASFASISSF!" << endl;
	// Recupera a informação dos atributos de
	// altura e largura da imagem
	Width   = CTOI(Header[0x12]);
	Height  = CTOI(Header[0x16]);
	( CTOI(Header[0x0A]) == 0 ) ? ( DataPos=0x36 ) : ( DataPos = CTOI(Header[0x0A]) );

	imageSize=Width*Height*3;

	// Efetura a Carga da Imagem
	// image = (GLubyte *) malloc ( imageSize );
	image = new GLubyte[imageSize];
	int retorno;
	retorno = fread(image,1,imageSize,fp_arquivo);

	if (retorno != imageSize) {
		delete (image);
		SAIR;
	}

	// Inverte os valores de R e B
	int t, i;

	for ( i = 0; i < imageSize; i += 3 )
	{
		t = image[i];
		image[i] = image[i+2];
		image[i+2] = t;
	}

	// Tratamento da textura para o OpenGL

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S    ,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T    ,GL_REPEAT);

	glTexEnvf ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	// Faz a geraçao da textura na memória
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	fclose (fp_arquivo);
	delete (image);
	return 1;
}

void Texturizacao() //faz o carregamento
{
	glEnable(GL_TEXTURE_2D);
	glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );//Como armazena o pixel
	glGenTextures ( NUM_TEX , texture_id );//armazena q qtidade de textura

	texture_id[ 0 ] = TEXTURA1; // define um numero (identificacao) para a textura
	glBindTexture ( GL_TEXTURE_2D, texture_id[0] );//armazena na posição 0 do vetor
	LoadBMP ( "Texturas/body.bmp" ); // lê a textura

	texture_id[ 1 ] = TEXTURA2;
	glBindTexture ( GL_TEXTURE_2D, texture_id[1] );
	LoadBMP ( "Texturas/head.bmp" );

	texture_id[ 2 ] = TEXTURA3;
	glBindTexture ( GL_TEXTURE_2D, texture_id[2] );
	LoadBMP ( "Texturas/hip.bmp" );

	texture_id[ 3 ] = TEXTURA4;
	glBindTexture ( GL_TEXTURE_2D, texture_id[3] );
	LoadBMP ( "Texturas/arm.bmp" );

	texture_id[ 4 ] = TEXTURA5;
	glBindTexture ( GL_TEXTURE_2D, texture_id[4] );
	LoadBMP ( "Texturas/leg.bmp" );

	texture_id[ 5 ] = TEXTURA6;
	glBindTexture ( GL_TEXTURE_2D, texture_id[5] );
	LoadBMP ( "Texturas/ground.bmp" );

	glTexGeni( GL_S , GL_TEXTURE_GEN_MODE , GL_SPHERE_MAP );
	glTexGeni( GL_T , GL_TEXTURE_GEN_MODE , GL_SPHERE_MAP );
}

// Função responsável pela especificação dos parâmetros de iluminação
void DefineIluminacao (void)
{
	GLfloat luzAmbiente[4]={0.2,0.2,0.2,1.0};
	GLfloat luzDifusa[4]={0.7,0.7,0.7,1.0};	   // "cor"
	GLfloat luzEspecular[4]={1.0, 1.0, 1.0, 1.0};// "brilho"
	GLfloat posicaoLuz[4]={0.0, 50.0, 200.0, 1.0};

	// Capacidade de brilho do material
	GLfloat especularidade[4]={1.0,1.0,1.0,1.0};
	// GLint especMaterial = 60;
	GLint especMaterial = 20;

	// Define a refletância do material
	glMaterialfv(GL_FRONT,GL_SPECULAR, especularidade);
	// Define a concentração do brilho
	glMateriali(GL_FRONT,GL_SHININESS,especMaterial);

	// Ativa o uso da luz ambiente
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luzAmbiente);

	// Define os parâmetros da luz de número 0
	glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbiente);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa );
	glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular );
	glLightfv(GL_LIGHT0, GL_POSITION, posicaoLuz );
}


void SRT (int i, float dx=0, float dy=0, float dz=0, float angx=0, float angy=0, float angz=0, float sx=0, float sy=0, float sz=0)
{
	if (i == -1)
	{
		glTranslatef( dx, dy, dz);
		glRotatef( angx, 1 , 0 , 0 );
		glRotatef( angy, 0 , 1 , 0 );
		glRotatef( angz, 0 , 0 , 1 );
		glScalef( sx, sy, sz);
	}
	else
	{
		glTranslatef( dx + transf[ i ].dx , dy + transf[ i ].dy , dz + transf[ i ].dz );
		glRotatef( angx + transf[ i ].angx , 1 , 0 , 0 );
		glRotatef( angy + transf[ i ].angy , 0 , 1 , 0 );
		glRotatef( angz + transf[ i ].angz , 0 , 0 , 1 );
		glScalef( sx + transf[ i ].sx , sy + transf[ i ].sy , sz + transf[ i ].sz );
	}
}

// desenha cubo passando {textura, height, width e depth}
void DrawCube(int texture, int w, int h, int d)
{
	float TXTx[5] = {0.0, 1/4.0, 2/4.0, 3/4.0, 4/4.0};
	float TXTy[4] = {0.0, 1/3.0, 2/3.0, 3/3.0};

	w/=2; h/=2; d/=2;
	glBindTexture ( GL_TEXTURE_2D, texture );
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin( GL_QUADS );

		// Face frontal
		glNormal3f(   0.0 ,   0.0 ,  1.0 );	// Normal da face
		glTexCoord2f( TXTx[1] , TXTy[1] ); glVertex3f( -w , -h , d );
		glTexCoord2f( TXTx[2] , TXTy[1] ); glVertex3f(  w , -h , d );
		glTexCoord2f( TXTx[2] , TXTy[2] ); glVertex3f(  w ,  h , d );
		glTexCoord2f( TXTx[1] , TXTy[2] ); glVertex3f( -w ,  h , d );

		// Face traseira
		glNormal3f(   0.0 ,   0.0 ,  -1.0 );	// Normal da face
		glTexCoord2f( TXTx[4] , TXTy[2] ); glVertex3f( -w ,  h , -d );
		glTexCoord2f( TXTx[3] , TXTy[2] ); glVertex3f(  w ,  h , -d );
		glTexCoord2f( TXTx[3] , TXTy[1] ); glVertex3f(  w , -h , -d );
		glTexCoord2f( TXTx[4] , TXTy[1] ); glVertex3f( -w , -h , -d );

		// Face superior
		glNormal3f(   0.0 , 1.0 ,  0.0 );	// Normal da face
		glTexCoord2f( TXTx[1] , TXTy[2] ); glVertex3f( -w ,  h ,  d );
		glTexCoord2f( TXTx[2] , TXTy[2] ); glVertex3f(  w ,  h ,  d );
		glTexCoord2f( TXTx[2] , TXTy[3] ); glVertex3f(  w ,  h , -d );
		glTexCoord2f( TXTx[1] , TXTy[3] ); glVertex3f( -w ,  h , -d );

		// Face inferior
		glNormal3f(   0.0 ,-1.0 ,  0.0 );	// Normal da face
		glTexCoord2f( TXTx[1] , TXTy[0] ); glVertex3f( -w , -h , -d );
		glTexCoord2f( TXTx[2] , TXTy[0] ); glVertex3f(  w , -h , -d );
		glTexCoord2f( TXTx[2] , TXTy[1] ); glVertex3f(  w , -h ,  d );
		glTexCoord2f( TXTx[1] , TXTy[1] ); glVertex3f( -w , -h ,  d );

		// Face esquerda
		glNormal3f(   -1.0 , 0.0 ,  0.0 );	// Normal da face
		glTexCoord2f( TXTx[0] , TXTy[1] ); glVertex3f( -w , -h , -d );
		glTexCoord2f( TXTx[1] , TXTy[1] ); glVertex3f( -w , -h ,  d );
		glTexCoord2f( TXTx[1] , TXTy[2] ); glVertex3f( -w ,  h ,  d );
		glTexCoord2f( TXTx[0] , TXTy[2] ); glVertex3f( -w ,  h , -d );

		// Face direita
		glNormal3f(    1.0 , 0.0 ,  0.0 );	// Normal da face
		glTexCoord2f( TXTx[2] , TXTy[1] ); glVertex3f(  w , -h ,  d );
		glTexCoord2f( TXTx[3] , TXTy[1] ); glVertex3f(  w , -h , -d );
		glTexCoord2f( TXTx[3] , TXTy[2] ); glVertex3f(  w ,  h , -d );
		glTexCoord2f( TXTx[2] , TXTy[2] ); glVertex3f(  w ,  h ,  d );

	glEnd();

}

void desenha_chao(int texture, int sz) {
	// sz/=2;
	glBindTexture ( GL_TEXTURE_2D, texture );
	glBegin( GL_QUADS );
		glNormal3f(   0.0 , 1.0 ,  0.0 );	// Normal da face
		glTexCoord2f( 0, 0 ); glVertex3f( -sz ,  0 ,  sz );
		glTexCoord2f( 1, 0 ); glVertex3f(  sz ,  0 ,  sz );
		glTexCoord2f( 1, 1 ); glVertex3f(  sz ,  0 , -sz );
		glTexCoord2f( 0, 1 ); glVertex3f( -sz ,  0 , -sz );
	glEnd();
}

void desenha_robo_3D(void)
{
	glPushMatrix();	// ROBO
	SRT(ROBO, 0, 0, 0,  0, 0, 0,  1, 1, 1);

	glPushMatrix();	// TRONCO
	SRT(TRONCO, 0, -20, 0,  0, 0, 0,  0, 0, 0);
	glPushMatrix();
	SRT(-1, 0, 20, 0,  0, 0, 0,  2.5, 2, 1.5);
	DrawCube(TEXTURA1, 20, 20, 20);
	glPopMatrix();

	glPushMatrix();	// CABECA
	SRT(CABECA, 0, 40, 0,  0, 0, 0,  0, 0, 0);
	glPushMatrix();
	SRT(-1, 0, 15, 0,  0, 0, 0,  3, 1.5, 1);
	DrawCube(TEXTURA2, 20, 20, 20);
	glPopMatrix();

	glPopMatrix();	// FECHA CABECA

	glPushMatrix();	// BRACO_D
	SRT(BRACO_D, -24.8, 25, 0,  0, 0, 0,  0, 0, 0);
	glPushMatrix();
	SRT(-1, -22.8, 0, 0,  0, 0, 0,  4.5, 1, 1);
	DrawCube(TEXTURA4, 10, 10, 10);
	glPopMatrix();

	glPushMatrix();	// MAO_D
	SRT(-1, -55, 0, 0,  0, 90, 0,  1, 1, 1);
	glutSolidCone(5, 10, 8, 3);
	glPopMatrix();	// FECHA MAO_D

	glPopMatrix();	// FECHA BRACO_D

	glPushMatrix();	// BRACO_E
	SRT(BRACO_E, 24.8, 25, 0,  0, 0, 0,  0, 0, 0);
	glPushMatrix();
	SRT(-1, 22.8, 0, 0,  0, 0, 0,  4.5, 1, 1);
	DrawCube(TEXTURA4, 10, 10, 10);
	glPopMatrix();

	glPushMatrix();	// MAO_E
	SRT(-1, 55, 0, 0,  0, -90, 0,  1, 1, 1);
	glutSolidCone(5, 10, 8, 3);
	glPopMatrix();	// FECHA MAO_E

	glPopMatrix();	// FECHA BRACO_E

	glPopMatrix();	// FECHA TRONCO

	glPushMatrix();	// QUADRIL
	SRT(QUADRIL, 0, -30, 0,  0, 0, 0,  0, 0, 0);
	glPushMatrix();
	SRT(-1, 0, 0, 0,  0, 0, 0,  3, 1, 1);
	DrawCube(TEXTURA3, 20, 20, 20);
	glPopMatrix();


	glPushMatrix();	// PERNA_D
	SRT(PERNA_D, -20, -10, 0,  0, 0, 0,  0, 0, 0);
	glPushMatrix();
	SRT(-1, 0, -15, 0,  0, 0, 0,  1, 3, 1);
	DrawCube(TEXTURA5, 10, 10, 10);
	glPopMatrix();

	glPushMatrix();	// PE_D
	SRT(-1, 0, -30, 0,  90, 0, 0,  1, 1, 0.5);
	glutSolidTorus(4, 6, 5, 15);
	glPopMatrix();	// FECHA PE_D

	glPopMatrix();	// FECHA PERNA_D

	glPushMatrix();	// PERNA_E
	SRT(PERNA_E, 20, -10, 0,  0, 0, 0,  0, 0, 0);
	glPushMatrix();
	SRT(-1, 0, -15, 0,  0, 0, 0,  1, 3, 1);
	DrawCube(TEXTURA5, 10, 10, 10);
	glPopMatrix();

	glPushMatrix();	// PE_D
	SRT(-1, 0, -30, 0,  90, 0, 0,  1, 1, 0.5);
	glutSolidTorus(4, 6, 5, 15);
	glPopMatrix();	// PE_E

	glPopMatrix();	// PERNA_E

	glPopMatrix();	// FECHA QUADRIL
	glPopMatrix();	// FECHA ROBO

	glPushMatrix();	// Chão
		SRT(-1, 0, -144, 0,  0, 0, 0,  1, 1, 1);
		desenha_chao(TEXTURA6, 150);
	glPopMatrix();	// Chão
}


// Função callback de redesenho da janela de visualização
void Desenha(void)
{
	// Limpa a janela de visualização com a cor
	// de fundo definida previamente
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Chama a função que especifica os parâmetros de iluminação
	DefineIluminacao();


	glColor3f(1.0f, 1.0f, 1.0f);
	// Função da GLUT para fazer o desenho de um "torus"
	// com a cor corrente
	glPushMatrix();
	SRT(CENA);

	desenha_robo_3D();

	glPopMatrix();

	// Executa os comandos OpenGL
	glutSwapBuffers();
}

// Função usada para especificar a posição do observador virtual
void PosicionaObservador(void)
{
	// Especifica sistema de coordenadas do modelo
	glMatrixMode(GL_MODELVIEW);
	// Inicializa sistema de coordenadas do modelo
	glLoadIdentity();
	DefineIluminacao();
	// Posiciona e orienta o observador
	glTranslatef(-obsX,-obsY,-obsZ);
	glRotatef(rotX,1,0,0);
	glRotatef(rotY,0,1,0);
}

// Função usada para especificar o volume de visualização
void EspecificaParametrosVisualizacao(void)
{
	// Especifica sistema de coordenadas de projeção
	glMatrixMode(GL_PROJECTION);
	// Inicializa sistema de coordenadas de projeção
	glLoadIdentity();

	// Especifica a projeção perspectiva(angulo,aspecto,zMin,zMax)
	gluPerspective(angle,fAspect,0.5,5000);

	PosicionaObservador();
}


// Função responsável por inicializar parâmetros e variáveis
void Inicializa (void)
{
	// Define a cor de fundo da janela de visualização como branca
	glClearColor(0.8, 0.8, 0.8, 1.0);

	// Habilita a definição da cor do material a partir da cor corrente
	glEnable(GL_COLOR_MATERIAL);
	//Habilita o uso de iluminação
	glEnable(GL_LIGHTING);
	// Habilita a luz de número 0
	glEnable(GL_LIGHT0);
	// Habilita o depth-buffering
	glEnable(GL_DEPTH_TEST);

	// ativa a possibilidade de transparência dos objetos - canal alfa
	glEnable( GL_BLEND );

	// define a forma de cálculo da transparência
	glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );

	// Habilita antialiasing
	glEnable( GL_LINE_SMOOTH );

	// Solicita melhor qualidade
	glHint( GL_LINE_SMOOTH_HINT , GL_NICEST );

	// ativa a remoçào das faces ocultas
	glEnable( GL_CULL_FACE );

	// Habilita o modelo de colorização de Gouraud
	glShadeModel(GL_SMOOTH);

	for(int i = 0 ; i < NUM_OBJETOS ; i++)
	{
		transf[ i ].dx   = 0.0;
		transf[ i ].dy   = 0.0;
		transf[ i ].dz   = 0.0;
		transf[ i ].sx   = 1.0;
		transf[ i ].sy   = 1.0;
		transf[ i ].sz   = 1.0;
		transf[ i ].angx = 0.0;
		transf[ i ].angy = 0.0;
		transf[ i ].angz = 0.0;
	}

	objeto = CENA;
	transformacao = 'T';
	eixo = 'X';
	passo = 5;

	// Inicializa a variável que especifica o ângulo da projeção
	// perspectiva
	angle=45;

	// Inicializa as variáveis usadas para alterar a posição do
	// observador virtual
	rotX = 0;
	rotY = 0;
	obsX = obsY = 0;
	obsZ = 400;

	Texturizacao();

	cout << endl;
	cout  << "Mouse      Movimenta observador" << endl;
	cout  << "ESC        Sair" << endl;
	cout  << "ALT  +     Aumenta passo" << endl;
	cout  << "ALT  -     Diminui passo" << endl;
	cout  << "HOME       Diminui angulo de abertura da camera" << endl;
	cout  << "END        Aumenta angulo de abertura da camera" << endl;
	cout  << "I          Inicializa" << endl;
	cout  << "X Y Z      Escolhe eixo" << endl;
	cout  << "S R T      Escolhe transformação" << endl;
	cout  << "+ -        Aplica transformação" << endl;
	cout  << "W A D      Movimenta robô" << endl;
	cout  << "0:8        Partes da cena/robo" << endl;
	cout  << "  0        Seleciona cena" << endl;
	cout  << "  1        Seleciona robô" << endl;
	cout  << "  2        Seleciona tronco" << endl;
	cout  << "  3        Seleciona cabeça" << endl;
	cout  << "  4        Seleciona braço direito" << endl;
	cout  << "  5        Seleciona braço esquerdo" << endl;
	cout  << "  6        Seleciona quadril" << endl;
	cout  << "  7        Seleciona perna direita" << endl;
	cout  << "  8        Seleciona perna esquerda" << endl;
	cout << endl;
}

bool invWalk = false;
char sentido;

void walk()
{
	transf[ PERNA_D ].angx += (invWalk ? -5 : 5);
	transf[ PERNA_E ].angx += (invWalk ? 5 : -5);

	if (transf[ PERNA_D ].angx > 20 || transf[ PERNA_E ].angx > 20)
		invWalk = !invWalk;

	int ang = transf[ ROBO ].angy;
	if (ang < 0)
		ang = 360 + (ang % 360);
	else
		ang = ang % 360;
	float res = 2 * M_PI * ang / 360;
	// cout << res << endl;
	float x = passo * sin( res );
	float z = passo * cos( res );
	transf[ ROBO ].dx += x;
	transf[ ROBO ].dz += z;
	// cout << "X:" << x << endl;
	// cout << "Z:" << z << endl;
}

// Função callback chamada para gerenciar eventos de teclas normais (ESC)
void Teclado ( GLubyte key , GLint x , GLint y )
{
	GLint modificador = glutGetModifiers();

	if ( modificador & GLUT_ACTIVE_ALT)
	{
		// ALT pressionado
		if( key == '-' || key == '_' )
			if( passo - 1 > 0 )
				passo--;

		if( key == '+' || key == '=' )
			passo++;
	}
	else
	{
		if ( key == 27 )
			exit( 0 );

		if ( toupper( key ) == 'I' )
			Inicializa();

		if ( toupper( key ) == 'X' || toupper( key ) == 'Y' || toupper( key ) == 'Z')
			eixo = toupper( key );

		if ( toupper( key ) == 'S' || toupper( key ) == 'R' || toupper( key ) == 'T' )
			transformacao = toupper( key );

		if (key-'0' >= 0 && key-'0' <= NUM_OBJETOS-1)
			objeto = key - '0';

		if ( key == 'l' || key == 'L' )
		{
			if( glIsEnabled( GL_LINE_SMOOTH ) )
				glDisable( GL_LINE_SMOOTH );
			else
				glEnable( GL_LINE_SMOOTH );
		}

		if ( toupper( key ) == 'W' )
			walk();
		if ( toupper( key ) == 'A' )
			transf[ ROBO ].angy += passo;
		if ( toupper( key ) == 'D' )
			transf[ ROBO ].angy -= passo;

		if ( key == '+' || key == '=')
			switch( transformacao ){
				case 'S':
					if ( eixo == 'X')
						transf[ objeto ].sx += 0.1;
					if ( eixo == 'Y')
						transf[ objeto ].sy += 0.1;
					if ( eixo == 'Z')
						transf[ objeto ].sz += 0.1;
					break;

				case 'R':
					if ( eixo == 'X')
						transf[ objeto ].angx += passo;
					if ( eixo == 'Y')
						transf[ objeto ].angy += passo;
					if ( eixo == 'Z')
						transf[ objeto ].angz += passo;
					break;

				case 'T':
					if ( eixo == 'X')
						transf[ objeto ].dx += passo;
					if ( eixo == 'Y')
						transf[ objeto ].dy += passo;
					if ( eixo == 'Z')
						transf[ objeto ].dz += passo;
					break;
			}

		if ( key == '-' || key == '_' )
			switch( transformacao ){
				case 'S':
					if ( eixo == 'X')
						transf[ objeto ].sx -= 0.1;
					if ( eixo == 'Y')
						transf[ objeto ].sy -= 0.1;
					if ( eixo == 'Z')
						transf[ objeto ].sz -= 0.1;
					break;

				case 'R':
					if ( eixo == 'X')
						transf[ objeto ].angx -= passo;
					if ( eixo == 'Y')
						transf[ objeto ].angy -= passo;
					if ( eixo == 'Z')
						transf[ objeto ].angz -= passo;
					break;

				case 'T':
					if ( eixo == 'X')
						transf[ objeto ].dx -= passo;
					if ( eixo == 'Y')
						transf[ objeto ].dy -= passo;
					if ( eixo == 'Z')
						transf[ objeto ].dz -= passo;
					break;
			}

	}

	if ( modificador || (key != '+' && key != '=' && key != '-' && key != '_') )
		cout << endl
			 << "    Objeto selecionado: " << objetos[objeto] << endl
			 << "    Transformação: " << transformacao
			 << "    Eixo: " << eixo
			 << "    Passo: " << passo << endl;

	EspecificaParametrosVisualizacao();

	// obriga redesenhar
	glutPostRedisplay();

}

// Função callback para tratar eventos de teclas especiais
void TeclasEspeciais (int tecla, int x, int y)
{
	switch (tecla)
	{
		case GLUT_KEY_HOME:	if(angle>=10)  angle -=5;
							break;
		case GLUT_KEY_END:	if(angle<=150) angle +=5;
							break;
	}
	EspecificaParametrosVisualizacao();
	glutPostRedisplay();
}

// Função callback para eventos de botões do mouse
void GerenciaMouse(int button, int state, int x, int y)
{
	if(state==GLUT_DOWN)
	{
		// Salva os parâmetros atuais
		x_ini = x;
		y_ini = y;
		obsX_ini = obsX;
		obsY_ini = obsY;
		obsZ_ini = obsZ;
		rotX_ini = rotX;
		rotY_ini = rotY;
		bot = button;
	}
	else bot = -1;
}

// Função callback para eventos de movimento do mouse
#define SENS_ROT	5.0
#define SENS_OBS	10.0
#define SENS_TRANSL	10.0
void GerenciaMovim(int x, int y)
{
	// Botão esquerdo ?
	if(bot==GLUT_LEFT_BUTTON)
	{
		// Calcula diferenças
		int deltax = x_ini - x;
		int deltay = y_ini - y;
		// E modifica ângulos
		rotY = rotY_ini - deltax/SENS_ROT;
		rotX = rotX_ini - deltay/SENS_ROT;
	}
	// Botão direito ?
	else if(bot==GLUT_RIGHT_BUTTON)
	{
		// Calcula diferença
		int deltaz = y_ini - y;
		// E modifica distância do observador
		obsZ = obsZ_ini + deltaz/SENS_OBS;
	}
	// Botão do meio ?
	else if(bot==GLUT_MIDDLE_BUTTON)
	{
		// Calcula diferenças
		int deltax = x_ini - x;
		int deltay = y_ini - y;
		// E modifica posições
		obsX = obsX_ini + deltax/SENS_TRANSL;
		obsY = obsY_ini - deltay/SENS_TRANSL;
	}
	PosicionaObservador();
	glutPostRedisplay();
}

// Função callback chamada quando o tamanho da janela é alterado
void AlteraTamanhoJanela(GLsizei w, GLsizei h)
{
	// Para previnir uma divisão por zero
	if ( h == 0 ) h = 1;

	// Especifica as dimensões da viewport
	glViewport(0, 0, w, h);

	// Calcula a correção de aspecto
	fAspect = (GLfloat)w/(GLfloat)h;

	EspecificaParametrosVisualizacao();
}

// Programa Principal
int main(int argc , char *argv[])
{
	glutInit( &argc , argv );

	// Define o modo de operação da GLUT
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// Especifica o tamanho inicial em pixels da janela GLUT
	glutInitWindowSize(LARGURA, ALTURA);

	// Especifica a posição inicial da janela GLUT
	glutInitWindowPosition( ( glutGet( GLUT_SCREEN_WIDTH  ) - LARGURA ) / 2 ,
							( glutGet( GLUT_SCREEN_HEIGHT ) - ALTURA  ) / 2 );

	// Cria a janela passando como argumento o título da mesma
	glutCreateWindow("Robo - Anderson A. Fontana");

	// Registra a função callback de redesenho da janela de visualização
	glutDisplayFunc(Desenha);

	// Registra a função callback de redimensionamento da janela de visualização
	glutReshapeFunc(AlteraTamanhoJanela);

	// Registra a função callback para tratamento das teclas normais
	glutKeyboardFunc (Teclado);

	// Registra a função callback para tratamento das teclas especiais
	glutSpecialFunc (TeclasEspeciais);

	// Registra a função callback para eventos de botões do mouse
	glutMouseFunc(GerenciaMouse);

	// Registra a função callback para eventos de movimento do mouse
	glutMotionFunc(GerenciaMovim);

	// Chama a função responsável por fazer as inicializações
	Inicializa();

	// Inicia o processamento e aguarda interações do usuário
	glutMainLoop();

	return 0;
}
