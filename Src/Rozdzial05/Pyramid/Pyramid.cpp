// Pyramid.cpp
// OpenGL. Ksi�ga eksperta, rozdzia� 5
// Demonstruje nak�adanie tekstury na ostros�up
// Autor programu: Richard S. Wright Jr.

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
// Zestaw potrzebnych klas
GLShaderManager		shaderManager;
GLMatrixStack		modelViewMatrix;
GLMatrixStack		projectionMatrix;
GLFrame				cameraFrame;
GLFrame             objectFrame;
GLFrustum			viewFrustum;

GLBatch             pyramidBatch;

GLuint              textureID;

GLGeometryTransform	transformPipeline;
M3DMatrix44f		shadowMatrix;


void MakePyramid(GLBatch& pyramidBatch)
    {
	pyramidBatch.Begin(GL_TRIANGLES, 18, 1);
    
	// Podstawa ostros�upa
	pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
	pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	pyramidBatch.Vertex3f(-1.0f, -1.0f, -1.0f);
    
	pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
	pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	pyramidBatch.Vertex3f(1.0f, -1.0f, -1.0f);
    
	pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
	pyramidBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
	pyramidBatch.Vertex3f(1.0f, -1.0f, 1.0f);
    
	pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
	pyramidBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
	pyramidBatch.Vertex3f(-1.0f, -1.0f, 1.0f);
    
	pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
	pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	pyramidBatch.Vertex3f(-1.0f, -1.0f, -1.0f);
    
	pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
	pyramidBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
	pyramidBatch.Vertex3f(1.0f, -1.0f, 1.0f);
    
	
	M3DVector3f vApex = { 0.0f, 1.0f, 0.0f };
	M3DVector3f vFrontLeft = { -1.0f, -1.0f, 1.0f };
	M3DVector3f vFrontRight = { 1.0f, -1.0f, 1.0f };
	M3DVector3f vBackLeft = { -1.0f, -1.0f, -1.0f };
	M3DVector3f vBackRight = { 1.0f, -1.0f, -1.0f };
	M3DVector3f n;
	
	// Przednia �ciana ostros�upa
	m3dFindNormal(n, vApex, vFrontLeft, vFrontRight);
	pyramidBatch.Normal3fv(n);
	pyramidBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
	pyramidBatch.Vertex3fv(vApex);		// Wierzcho�ek ostros�upa
    
	pyramidBatch.Normal3fv(n);
	pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	pyramidBatch.Vertex3fv(vFrontLeft);		// Lewy przedni r�g
    
	pyramidBatch.Normal3fv(n);
	pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	pyramidBatch.Vertex3fv(vFrontRight);		// Prawy przedni r�g
    
    
	m3dFindNormal(n, vApex, vBackLeft, vFrontLeft);
	pyramidBatch.Normal3fv(n);
	pyramidBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
	pyramidBatch.Vertex3fv(vApex);		// Wierzcho�ek ostros�upa
    
	pyramidBatch.Normal3fv(n);
	pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	pyramidBatch.Vertex3fv(vBackLeft);		// Lewy tylny r�g
	
	pyramidBatch.Normal3fv(n);
	pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	pyramidBatch.Vertex3fv(vFrontLeft);		// Lewy przedni r�g
    
	m3dFindNormal(n, vApex, vFrontRight, vBackRight);
	pyramidBatch.Normal3fv(n);
	pyramidBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
	pyramidBatch.Vertex3fv(vApex);				// Wierzcho�ek ostros�upa
    
	pyramidBatch.Normal3fv(n);
	pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	pyramidBatch.Vertex3fv(vFrontRight);		// Prawy przedni r�g
    
	pyramidBatch.Normal3fv(n);
	pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	pyramidBatch.Vertex3fv(vBackRight);			// Prawy tylny r�g
    
    
	m3dFindNormal(n, vApex, vBackRight, vBackLeft);
	pyramidBatch.Normal3fv(n);
	pyramidBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
	pyramidBatch.Vertex3fv(vApex);		// Wierzcho�ek ostros�upa
    
	pyramidBatch.Normal3fv(n);
	pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	pyramidBatch.Vertex3fv(vBackRight);		// Prawy tylny r�g

	pyramidBatch.Normal3fv(n);
	pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	pyramidBatch.Vertex3fv(vBackLeft);		// Lewy tylny r�g

	pyramidBatch.End();
	}

// �adowanie obrazu TGA jako tekstury dwuwymiarowej. Zainicjalizowanie stanu w ca�o�ci
bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
	GLbyte *pBits;
	int nWidth, nHeight, nComponents;
	GLenum eFormat;
	
	// Odczytywanie bit�w tekstury
	pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
	if(pBits == NULL) 
		return false;
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0,
				 eFormat, GL_UNSIGNED_BYTE, pBits);
	
    free(pBits);
    
    if(minFilter == GL_LINEAR_MIPMAP_LINEAR || 
       minFilter == GL_LINEAR_MIPMAP_NEAREST ||
       minFilter == GL_NEAREST_MIPMAP_LINEAR ||
       minFilter == GL_NEAREST_MIPMAP_NEAREST)
        glGenerateMipmap(GL_TEXTURE_2D);
    
	return true;
}


///////////////////////////////////////////////////////////////////////////////
// Funkcja inicjalizuj�ca w kontek�cie renderowania. 
// Pierwsza okazja do wykonania jakich� dzia�a� zwi�zanych z OpenGL.
void SetupRC()
	{
    // Czarne t�o
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f );

	shaderManager.InitializeStockShaders();

	glEnable(GL_DEPTH_TEST);
    
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    LoadTGATexture("stone.tga", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    
    MakePyramid(pyramidBatch);

	cameraFrame.MoveForward(-7.0f);
    }

///////////////////////////////////////////////////////////////////////////////
// Czyszczenie, np. usuwanie obiekt�w tekstur
void ShutdownRC(void)
    {
    glDeleteTextures(1, &textureID);
    }

///////////////////////////////////////////////////////////////////////////////
// Rysowanie sceny
void RenderScene(void)
	{    
    static GLfloat vLightPos [] = { 1.0f, 1.0f, 0.0f };
    static GLfloat vWhite [] = { 1.0f, 1.0f, 1.0f, 1.0f };
    
	// Czyszczenie okna bie��cym kolorem czyszczenia
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	modelViewMatrix.PushMatrix();
		M3DMatrix44f mCamera;
		cameraFrame.GetCameraMatrix(mCamera);
		modelViewMatrix.MultMatrix(mCamera);

        M3DMatrix44f mObjectFrame;
        objectFrame.GetMatrix(mObjectFrame);
        modelViewMatrix.MultMatrix(mObjectFrame);

        glBindTexture(GL_TEXTURE_2D, textureID);
        shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF, 
                                     transformPipeline.GetModelViewMatrix(),
                                     transformPipeline.GetProjectionMatrix(), 
                                     vLightPos, vWhite, 0);

        pyramidBatch.Draw();

		
	modelViewMatrix.PopMatrix();

	// Wykonanie instrukcji rysowania
	glutSwapBuffers();
    }


// Poruszanie uk�adem odniesienia kamery za pomoc� klawiszy strza�ek
void SpecialKeys(int key, int x, int y)
    {
	if(key == GLUT_KEY_UP)
		objectFrame.RotateWorld(m3dDegToRad(-5.0f), 1.0f, 0.0f, 0.0f);
    
	if(key == GLUT_KEY_DOWN)
		objectFrame.RotateWorld(m3dDegToRad(5.0f), 1.0f, 0.0f, 0.0f);
	
	if(key == GLUT_KEY_LEFT)
		objectFrame.RotateWorld(m3dDegToRad(-5.0f), 0.0f, 1.0f, 0.0f);
    
	if(key == GLUT_KEY_RIGHT)
		objectFrame.RotateWorld(m3dDegToRad(5.0f), 0.0f, 1.0f, 0.0f);
    
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
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
	}

///////////////////////////////////////////////////////////////////////////////
// Punkt rozpoczynania dzia�ania programu opartego na bibliotece GLUT
int main(int argc, char* argv[])
	{
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Pyramid");
    glutReshapeFunc(ChangeSize);
    glutSpecialFunc(SpecialKeys);
    glutDisplayFunc(RenderScene);
        
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "B��d GLEW: %s\n", glewGetErrorString(err));
		return 1;
		}
	

	SetupRC();

	glutMainLoop();
    
    ShutdownRC();
    
	return 0;
	}
