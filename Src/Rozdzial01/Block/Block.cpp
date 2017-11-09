// Block.cpp
// OpenGL. Ksiêga eksperta, rozdzia³ 1
// Demonstruje kilka podstawowych pojêæ grafiki trójwymiarowej
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


// Kontrola kolejnych ekranów
int nStep = 0;

// Dane oœwietlenia
GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat lightDiffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
GLfloat lightSpecular[] = { 0.9f, 0.9f, 0.9f };
GLfloat vLightPos[] = { -8.0f, 20.0f, 100.0f, 1.0f };

GLuint textures[4];


///////////////////////////////////////////////////////////////////////////////
// Tworzenie szeœcianu z zestawu trójk¹tów. Podane s¹ tak¿e wspó³rzêdne tekstury
// i normalne.
void MakeCube(GLBatch& cubeBatch)
	{
	cubeBatch.Begin(GL_TRIANGLES, 36, 1);

	/////////////////////////////////////////////
	// Wierzch szeœcianu
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
	// Spód szeœcianu
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
	// Lewa strona szeœcianu
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

	// Prawa strona szeœcianu
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

	// Przód i ty³
	// Przód
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

	// Ty³
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
// Pod³oga — tylko wierzcho³ki i wspó³rzêdne tekstury
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
// Ta funkcja wykonuje wszystkie dzia³ania zwi¹zane z inicjalizowaniem w kontekœcie renderowania. 
// Jest to pierwsza okazja do wykonania jakichœ zadañ zwi¹zanych z OpenGL.
void SetupRC()
	{
    GLbyte *pBytes;
    GLint nWidth, nHeight, nComponents;
    GLenum format;

	shaderManager.InitializeStockShaders();

	// Czarne t³o
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

	// Góra
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

	// Przód
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

	// Za³adowanie czterech tekstur
	glGenTextures(4, textures);
        
	// Pod³oga z drewna
    pBytes = gltReadTGABits("floor.tga", &nWidth, &nHeight, &nComponents, &format);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D,0,nComponents,nWidth, nHeight, 0,
		format, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);

	// Jeden z boków bloku
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

			// Rysowanie szeœcianu
			cubeBatch.Draw();

			break;

		// Szkielet, ale nie boczna strona... chcemy, aby blok znajdowa³ siê tak¿e w buforze szablonowym
		case 1:
			shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);

			// Rysowanie bloku w buforze szablonowym
			// Dziêki usuniêciu p³aszczyzn tylnych tylne œciany nie bêd¹ przeœwiecaæ
			// Wzorzec szablonu jest u¿ywany jako maska, kiedy rysujemy pod³ogê,
			// aby zapobiec przeœwitywaniu.
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

			// Rysowanie szeœcianu na froncie
			cubeBatch.Draw();
			break;

		// Wype³nienie
		case 2:
			shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
			
			// Rysowanie szeœcianu
			cubeBatch.Draw();
			break;

		// Oœwietlenie
		case 3:
			shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, modelViewMatrix.GetMatrix(),
				projectionMatrix.GetMatrix(), vLightPos, vRed);

			// Rysowanie szeœcianu
			cubeBatch.Draw();
			break;

		// Tekstura i oœwietlenie
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
// Renderowanie pod³ogi
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

		// Wype³nienie
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
	
	// Rysowanie pod³ogi
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
	// Czyszczenie okna bie¿¹cym kolorem
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
		
		
	// Jeœli nie ma odbicia, pod³oga jest rysowana na koñcu
	if(nStep != 5)
		RenderFloor();

		
	modelViewMatrix.PopMatrix();


	// Wykonanie poleceñ rysowania
	glutSwapBuffers();
	}


///////////////////////////////////////////////////////////////////////////////
// Zosta³ naciœniêty normalny klawisz znaku ASCII.
// Przejœcie do nastêpnej sceny, gdy zostanie naciœniêta spacja.
void KeyPressFunc(unsigned char key, int x, int y)
	{
	if(key == 32)
		{
		nStep++;

		if(nStep > 5)
			nStep = 0;
		}

	// Odœwie¿enie okna
	glutPostRedisplay();
	}

///////////////////////////////////////////////////////////////////////////////
// Zmieni³ siê rozmiar okna lub okno zosta³o w³aœnie utworzone. W obu przypadkach musimy
// sprawdziæ wymiary tego okna, aby ustawiæ obszar widoku i utworzyæ macierz rzutowania.
void ChangeSize(int w, int h)
	{
	glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 500.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.LoadIdentity();
	}

///////////////////////////////////////////////////////////////////////////////
// G³ówny punkt pocz¹tkowy programu opartego na bibliotece GLUT
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
		// Problem: awaria glewInit, jest bardzo Ÿle.
		fprintf(stderr, "B³¹d: %s\n", glewGetErrorString(err));
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
