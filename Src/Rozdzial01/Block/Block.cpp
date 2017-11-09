// Block.cpp
// OpenGL. Ksi�ga eksperta, rozdzia� 1
// Demonstruje kilka podstawowych poj�� grafiki tr�jwymiarowej
// Autor: Richard S. Wright Jr.

#include <GLTools.h>	// Biblioteka OpenGL
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLGeometryTransform.h>

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

/////////////////////////////////////////////////////////////////////////////////
// Potrzebne klasy
GLShaderManager		shaderManager;
GLMatrixStack		modelViewMatrix;
GLMatrixStack		projectionMatrix;
GLFrame				cameraFrame;
GLFrustum			viewFrustum;
GLBatch				cubeBatch;
GLBatch				floorBatch;
GLBatch				topBlock;
GLBatch				frontBlock;
GLBatch				leftBlock;

GLGeometryTransform	transformPipeline;
M3DMatrix44f		shadowMatrix;


// Kontrola kolejnych ekran�w
int nStep = 0;

// Dane o�wietlenia
GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat lightDiffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
GLfloat lightSpecular[] = { 0.9f, 0.9f, 0.9f };
GLfloat vLightPos[] = { -8.0f, 20.0f, 100.0f, 1.0f };

GLuint textures[4];


///////////////////////////////////////////////////////////////////////////////
// Tworzenie sze�cianu z zestawu tr�jk�t�w. Podane s� tak�e wsp�rz�dne tekstury
// i normalne.
void MakeCube(GLBatch& cubeBatch)
	{
	cubeBatch.Begin(GL_TRIANGLES, 36, 1);

	/////////////////////////////////////////////
	// Wierzch sze�cianu
	cubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
	cubeBatch.Vertex3f(1.0f, 1.0f, 1.0f);

	cubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	cubeBatch.Vertex3f(1.0f, 1.0f, -1.0f);

	cubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	cubeBatch.Vertex3f(-1.0f, 1.0f, -1.0f);

	cubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
	cubeBatch.Vertex3f(1.0f, 1.0f, 1.0f);

	cubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	cubeBatch.Vertex3f(-1.0f, 1.0f, -1.0f);

	cubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
	cubeBatch.Vertex3f(-1.0f, 1.0f, 1.0f);


	////////////////////////////////////////////
	// Sp�d sze�cianu
	cubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	cubeBatch.Vertex3f(-1.0f, -1.0f, -1.0f);

	cubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	cubeBatch.Vertex3f(1.0f, -1.0f, -1.0f);

	cubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
	cubeBatch.Vertex3f(1.0f, -1.0f, 1.0f);

	cubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
	cubeBatch.Vertex3f(-1.0f, -1.0f, 1.0f);

	cubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	cubeBatch.Vertex3f(-1.0f, -1.0f, -1.0f);

	cubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
	cubeBatch.Vertex3f(1.0f, -1.0f, 1.0f);

	///////////////////////////////////////////
	// Lewa strona sze�cianu
	cubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
	cubeBatch.Vertex3f(-1.0f, 1.0f, 1.0f);

	cubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	cubeBatch.Vertex3f(-1.0f, 1.0f, -1.0f);

	cubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	cubeBatch.Vertex3f(-1.0f, -1.0f, -1.0f);

	cubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
	cubeBatch.Vertex3f(-1.0f, 1.0f, 1.0f);

	cubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	cubeBatch.Vertex3f(-1.0f, -1.0f, -1.0f);

	cubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
	cubeBatch.Vertex3f(-1.0f, -1.0f, 1.0f);

	// Prawa strona sze�cianu
	cubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	cubeBatch.Vertex3f(1.0f, -1.0f, -1.0f);

	cubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	cubeBatch.Vertex3f(1.0f, 1.0f, -1.0f);

	cubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
	cubeBatch.Vertex3f(1.0f, 1.0f, 1.0f);

	cubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
	cubeBatch.Vertex3f(1.0f, 1.0f, 1.0f);

	cubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
	cubeBatch.Vertex3f(1.0f, -1.0f, 1.0f);

	cubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
	cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	cubeBatch.Vertex3f(1.0f, -1.0f, -1.0f);

	// Prz�d i ty�
	// Prz�d
	cubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
	cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	cubeBatch.Vertex3f(1.0f, -1.0f, 1.0f);

	cubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
	cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
	cubeBatch.Vertex3f(1.0f, 1.0f, 1.0f);

	cubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
	cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
	cubeBatch.Vertex3f(-1.0f, 1.0f, 1.0f);

	cubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
	cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
	cubeBatch.Vertex3f(-1.0f, 1.0f, 1.0f);

	cubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
	cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	cubeBatch.Vertex3f(-1.0f, -1.0f, 1.0f);

	cubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
	cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	cubeBatch.Vertex3f(1.0f, -1.0f, 1.0f);

	// Ty�
	cubeBatch.Normal3f(0.0f, 0.0f, -1.0f);
	cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	cubeBatch.Vertex3f(1.0f, -1.0f, -1.0f);

	cubeBatch.Normal3f(0.0f, 0.0f, -1.0f);
	cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	cubeBatch.Vertex3f(-1.0f, -1.0f, -1.0f);

	cubeBatch.Normal3f(0.0f, 0.0f, -1.0f);
	cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
	cubeBatch.Vertex3f(-1.0f, 1.0f, -1.0f);

	cubeBatch.Normal3f(0.0f, 0.0f, -1.0f);
	cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
	cubeBatch.Vertex3f(-1.0f, 1.0f, -1.0f);

	cubeBatch.Normal3f(0.0f, 0.0f, -1.0f);
	cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
	cubeBatch.Vertex3f(1.0f, 1.0f, -1.0f);

	cubeBatch.Normal3f(0.0f, 0.0f, -1.0f);
	cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	cubeBatch.Vertex3f(1.0f, -1.0f, -1.0f);

	cubeBatch.End();
	}

/////////////////////////////////////////////////////////////////////////////
// Pod�oga � tylko wierzcho�ki i wsp�rz�dne tekstury
void MakeFloor(GLBatch& floorBatch)
	{
	GLfloat x = 5.0f;
    GLfloat y = -1.0f;

	floorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
		floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
		floorBatch.Vertex3f(-x, y, x);

		floorBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
		floorBatch.Vertex3f(x, y, x);

		floorBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
		floorBatch.Vertex3f(x, y, -x);

		floorBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
		floorBatch.Vertex3f(-x, y, -x);
	floorBatch.End();
	}


///////////////////////////////////////////////////////////////////////////////
// Ta funkcja wykonuje wszystkie dzia�ania zwi�zane z inicjalizowaniem w kontek�cie renderowania. 
// Jest to pierwsza okazja do wykonania jakich� zada� zwi�zanych z OpenGL.
void SetupRC()
	{
    GLbyte *pBytes;
    GLint nWidth, nHeight, nComponents;
    GLenum format;

	shaderManager.InitializeStockShaders();

	// Czarne t�o
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
	glEnable(GL_DEPTH_TEST);
	glLineWidth(2.5f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);

	cameraFrame.MoveForward(-15.0f);
	cameraFrame.MoveUp(6.0f);
	cameraFrame.RotateLocalX(float(m3dDegToRad(20.0f)));
	
	MakeCube(cubeBatch);
	MakeFloor(floorBatch);

	// G�ra
	topBlock.Begin(GL_TRIANGLE_FAN, 4, 1);
		topBlock.Normal3f(0.0f, 1.0f, 0.0f);
		topBlock.MultiTexCoord2f(0, 0.0f, 0.0f);
		topBlock.Vertex3f(-1.0f, 1.0f, 1.0f);

		topBlock.Normal3f(0.0f, 1.0f, 0.0f);
		topBlock.MultiTexCoord2f(0, 1.0f, 0.0f);
		topBlock.Vertex3f(1.0f, 1.0f, 1.0f);

		topBlock.Normal3f(0.0f, 1.0f, 0.0f);
		topBlock.MultiTexCoord2f(0, 1.0f, 1.0f);
		topBlock.Vertex3f(1.0f, 1.0f, -1.0f);

		topBlock.Normal3f(0.0f, 1.0f, 0.0f);
		topBlock.MultiTexCoord2f(0, 0.0f, 1.0f);
		topBlock.Vertex3f(-1.0f, 1.0f, -1.0f);
	topBlock.End();

	// Prz�d
	frontBlock.Begin(GL_TRIANGLE_FAN, 4, 1);
		frontBlock.Normal3f(0.0f, 0.0f, 1.0f);
		frontBlock.MultiTexCoord2f(0, 0.0f, 0.0f);
		frontBlock.Vertex3f(-1.0f, -1.0f, 1.0f);

		frontBlock.Normal3f(0.0f, 0.0f, 1.0f);
		frontBlock.MultiTexCoord2f(0, 1.0f, 0.0f);
		frontBlock.Vertex3f(1.0f, -1.0f, 1.0f);

		frontBlock.Normal3f(0.0f, 0.0f, 1.0f);
		frontBlock.MultiTexCoord2f(0, 1.0f, 1.0f);
		frontBlock.Vertex3f(1.0f, 1.0f, 1.0f);

		frontBlock.Normal3f(0.0f, 0.0f, 1.0f);
		frontBlock.MultiTexCoord2f(0, 0.0f, 1.0f);
		frontBlock.Vertex3f(-1.0f, 1.0f, 1.0f);
	frontBlock.End();

	// Lewa
	leftBlock.Begin(GL_TRIANGLE_FAN, 4, 1);
		leftBlock.Normal3f(-1.0f, 0.0f, 0.0f);
		leftBlock.MultiTexCoord2f(0, 0.0f, 0.0f);
		leftBlock.Vertex3f(-1.0f, -1.0f, -1.0f);

		leftBlock.Normal3f(-1.0f, 0.0f, 0.0f);
		leftBlock.MultiTexCoord2f(0, 1.0f, 0.0f);
		leftBlock.Vertex3f(-1.0f, -1.0f, 1.0f);

		leftBlock.Normal3f(-1.0f, 0.0f, 0.0f);
		leftBlock.MultiTexCoord2f(0, 1.0f, 1.0f);
		leftBlock.Vertex3f(-1.0f, 1.0f, 1.0f);

		leftBlock.Normal3f(-1.0f, 0.0f, 0.0f);
		leftBlock.MultiTexCoord2f(0, 0.0f, 1.0f);
		leftBlock.Vertex3f(-1.0f, 1.0f, -1.0f);
	leftBlock.End();

	// Tworzenie macierzy rzutowania cienia
	GLfloat floorPlane[] = { 0.0f, 1.0f, 0.0f, 1.0f};
	m3dMakePlanarShadowMatrix(shadowMatrix, floorPlane, vLightPos);

	// Za�adowanie czterech tekstur
	glGenTextures(4, textures);
        
	// Pod�oga z drewna
    pBytes = gltReadTGABits("floor.tga", &nWidth, &nHeight, &nComponents, &format);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D,0,nComponents,nWidth, nHeight, 0,
		format, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	// Jeden z bok�w bloku
	pBytes = gltReadTGABits("Block4.tga", &nWidth, &nHeight, &nComponents, &format);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D,0,nComponents,nWidth, nHeight, 0,
		format, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	// Drugi bok bloku
	pBytes = gltReadTGABits("block5.tga", &nWidth, &nHeight, &nComponents, &format);
        glBindTexture(GL_TEXTURE_2D, textures[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D,0,nComponents,nWidth, nHeight, 0,
		format, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	// Kolejny bok bloku
	pBytes = gltReadTGABits("block6.tga", &nWidth, &nHeight, &nComponents, &format);
        glBindTexture(GL_TEXTURE_2D, textures[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D,0,nComponents,nWidth, nHeight, 0,
		format, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);
    }




///////////////////////////////////////////////////////////////////////////////
// Renderowanie bloku
void RenderBlock(void)
	{
	GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f};
	GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	switch(nStep)
		{
		// Szkielet
		case 0:
			glEnable(GL_BLEND);
			glEnable(GL_LINE_SMOOTH);
			shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable(GL_CULL_FACE);

			// Rysowanie sze�cianu
			cubeBatch.Draw();

			break;

		// Szkielet, ale nie boczna strona... chcemy, aby blok znajdowa� si� tak�e w buforze szablonowym
		case 1:
			shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);

			// Rysowanie bloku w buforze szablonowym
			// Dzi�ki usuni�ciu p�aszczyzn tylnych tylne �ciany nie b�d� prze�wieca�
			// Wzorzec szablonu jest u�ywany jako maska, kiedy rysujemy pod�og�,
			// aby zapobiec prze�witywaniu.
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_STENCIL_TEST);
			glStencilFunc(GL_NEVER, 0, 0);
			glStencilOp(GL_INCR, GL_INCR, GL_INCR);
			cubeBatch.Draw();
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			glDisable(GL_STENCIL_TEST);

			glEnable(GL_BLEND);
			glEnable(GL_LINE_SMOOTH);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			// Rysowanie sze�cianu na froncie
			cubeBatch.Draw();
			break;

		// Wype�nienie
		case 2:
			shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
			
			// Rysowanie sze�cianu
			cubeBatch.Draw();
			break;

		// O�wietlenie
		case 3:
			shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, modelViewMatrix.GetMatrix(),
				projectionMatrix.GetMatrix(), vLightPos, vRed);

			// Rysowanie sze�cianu
			cubeBatch.Draw();
			break;

		// Tekstura i o�wietlenie
		case 4:
		case 5:
		default:
			glBindTexture(GL_TEXTURE_2D, textures[2]);
			shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF, modelViewMatrix.GetMatrix(),
				projectionMatrix.GetMatrix(), vLightPos, vWhite, 0);

			glBindTexture(GL_TEXTURE_2D, textures[1]);
			topBlock.Draw();
			glBindTexture(GL_TEXTURE_2D, textures[2]);
			frontBlock.Draw();
			glBindTexture(GL_TEXTURE_2D, textures[3]);
			leftBlock.Draw();

			break;
		}
	
	// Wycofanie wszystkiego
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_STENCIL_TEST);
	}

///////////////////////////////////////////////////////////////////////////////
// Renderowanie pod�ogi
void RenderFloor(void)
	{
	GLfloat vBrown [] = { 0.55f, 0.292f, 0.09f, 1.0f};
	GLfloat vFloor[] = { 1.0f, 1.0f, 1.0f, 0.6f };

	switch(nStep)
		{
		// Szkielet
		case 0:
			glEnable(GL_BLEND);
			glEnable(GL_LINE_SMOOTH);
			shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBrown);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable(GL_CULL_FACE);
			break;

		// Szkielet, aled bez bocznej strony... i tylko, gdy stencil == 0
		case 1:
			glEnable(GL_BLEND);
			glEnable(GL_LINE_SMOOTH);

			glEnable(GL_STENCIL_TEST);
			glStencilFunc(GL_EQUAL, 0, 0xff);

			shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBrown);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			break;

		// Wype�nienie
		case 2:
		case 3:	
			shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBrown);
			break;

		// Tekstura
		case 4:
		case 5:
		default:
			glBindTexture(GL_TEXTURE_2D, textures[0]);
			shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetModelViewProjectionMatrix(), vFloor, 0);
			break;
		}
	
	// Rysowanie pod�ogi
	floorBatch.Draw();

	// Wycofanie wszystkiego
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_STENCIL_TEST);
	}



///////////////////////////////////////////////////////////////////////////////
// Rysowanie sceny
void RenderScene(void)
	{
	// Czyszczenie okna bie��cym kolorem
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	modelViewMatrix.PushMatrix();
		M3DMatrix44f mCamera;
		cameraFrame.GetCameraMatrix(mCamera);
		modelViewMatrix.MultMatrix(mCamera);

		if(nStep == 5) {
			glDisable(GL_CULL_FACE);
			modelViewMatrix.PushMatrix();
			modelViewMatrix.Scale(1.0f, -1.0f, 1.0f);
			modelViewMatrix.Translate(0.0f, 2.0f, 0.0f);
			modelViewMatrix.Rotate(35.0f, 0.0f, 1.0f, 0.0f);
			RenderBlock();
			modelViewMatrix.PopMatrix();
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			RenderFloor();
			glDisable(GL_BLEND);			
			}


		modelViewMatrix.PushMatrix();

			// Rysowanie normalne
			modelViewMatrix.Rotate(35.0f, 0.0f, 1.0f, 0.0f);
			RenderBlock();
		modelViewMatrix.PopMatrix();
		
		
	// Je�li nie ma odbicia, pod�oga jest rysowana na ko�cu
	if(nStep != 5)
		RenderFloor();

		
	modelViewMatrix.PopMatrix();


	// Wykonanie polece� rysowania
	glutSwapBuffers();
	}


///////////////////////////////////////////////////////////////////////////////
// Zosta� naci�ni�ty normalny klawisz znaku ASCII.
// Przej�cie do nast�pnej sceny, gdy zostanie naci�ni�ta spacja.
void KeyPressFunc(unsigned char key, int x, int y)
	{
	if(key == 32)
		{
		nStep++;

		if(nStep > 5)
			nStep = 0;
		}

	// Od�wie�enie okna
	glutPostRedisplay();
	}

///////////////////////////////////////////////////////////////////////////////
// Zmieni� si� rozmiar okna lub okno zosta�o w�a�nie utworzone. W obu przypadkach musimy
// sprawdzi� wymiary tego okna, aby ustawi� obszar widoku i utworzy� macierz rzutowania.
void ChangeSize(int w, int h)
	{
	glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 500.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.LoadIdentity();
	}

///////////////////////////////////////////////////////////////////////////////
// G��wny punkt pocz�tkowy programu opartego na bibliotece GLUT
int main(int argc, char* argv[])
	{
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("3D Effects Demo");
	
	GLenum err = glewInit();
	if (GLEW_OK != err)
		{
		// Problem: awaria glewInit, jest bardzo �le.
		fprintf(stderr, "B��d: %s\n", glewGetErrorString(err));
		return 1;
		}
	
	glutReshapeFunc(ChangeSize);
	glutKeyboardFunc(KeyPressFunc);
	glutDisplayFunc(RenderScene);

	SetupRC();

	glutMainLoop();
	glDeleteTextures(4,textures);
	return 0;
	}
