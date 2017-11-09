// CubeMapped.cpp
// OpenGL. Ksiêga eksperta
// Demonstruje nak³adanie tekstury szeœciennej na obiekt (kulê)
// i wykorzystanie tej tekstury do utworzenia pud³a nieba
// Autor programu: Richard S. Wright Jr.

#include <GLTools.h>	// Biblioteka OpenGL
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

#include <math.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif


GLFrame             viewFrame;
GLFrustum           viewFrustum;
GLTriangleBatch     sphereBatch;
GLBatch             cubeBatch;
GLMatrixStack       modelViewMatrix;
GLMatrixStack       projectionMatrix;
GLGeometryTransform transformPipeline;
GLuint              cubeTexture;
GLint               reflectionShader;
GLint               skyBoxShader;

GLint               locMVPReflect, locMVReflect, locNormalReflect, locInvertedCamera;
GLint				locMVPSkyBox;


// Szeœæ boków tekstury szeœciennej
const char *szCubeFaces[6] = { "pos_x.tga", "neg_x.tga", "pos_y.tga", "neg_y.tga", "pos_z.tga", "neg_z.tga" };

GLenum  cube[6] = {  GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                     GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                     GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                     GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                     GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                     GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };

        
//////////////////////////////////////////////////////////////////
// Ta funkcja wykonuje wszystkie dzia³ania zwi¹zane z inicjalizowaniem w kontekœcie renderowania
 
void SetupRC()
    {
    GLbyte *pBytes;
    GLint iWidth, iHeight, iComponents;
    GLenum eFormat;
    int i;
       
    // Usuwanie tylnych œcian wielok¹tów
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
        
    glGenTextures(1, &cubeTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);
        
    // Tworzenie map tekstur        
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);       
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        
  
    // £adowanie obrazów tekstury szeœciennej
    for(i = 0; i < 6; i++)
        {        
        // £adowanie tej mapy tekstury
        pBytes = gltReadTGABits(szCubeFaces[i], &iWidth, &iHeight, &iComponents, &eFormat);
        glTexImage2D(cube[i], 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
        free(pBytes);
        }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    
    viewFrame.MoveForward(-4.0f);
    gltMakeSphere(sphereBatch, 1.0f, 52, 26);
    gltMakeCube(cubeBatch, 20.0f);
    
    reflectionShader = gltLoadShaderPairWithAttributes("Reflection.vp", "Reflection.fp", 2, 
                                                GLT_ATTRIBUTE_VERTEX, "vVertex",
                                                GLT_ATTRIBUTE_NORMAL, "vNormal");
                                                
    locMVPReflect = glGetUniformLocation(reflectionShader, "mvpMatrix");
    locMVReflect = glGetUniformLocation(reflectionShader, "mvMatrix");
    locNormalReflect = glGetUniformLocation(reflectionShader, "normalMatrix");
	locInvertedCamera = glGetUniformLocation(reflectionShader, "mInverseCamera");
                                                
                                                
    skyBoxShader = gltLoadShaderPairWithAttributes("SkyBox.vp", "SkyBox.fp", 2, 
                                                GLT_ATTRIBUTE_VERTEX, "vVertex",
                                                GLT_ATTRIBUTE_NORMAL, "vNormal");

	locMVPSkyBox = glGetUniformLocation(skyBoxShader, "mvpMatrix");

    
    }

void ShutdownRC(void)
    {
    glDeleteTextures(1, &cubeTexture);
    }

        
// Rysowanie sceny
void RenderScene(void)
    {
    // Wyczyszczenie okna
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
    M3DMatrix44f mCamera;
    M3DMatrix44f mCameraRotOnly;
	M3DMatrix44f mInverseCamera;
    
    viewFrame.GetCameraMatrix(mCamera, false);
    viewFrame.GetCameraMatrix(mCameraRotOnly, true);
	m3dInvertMatrix44(mInverseCamera, mCameraRotOnly);

    modelViewMatrix.PushMatrix();    
        // Rysowanie kuli
        modelViewMatrix.MultMatrix(mCamera);
        glUseProgram(reflectionShader);
        glUniformMatrix4fv(locMVPReflect, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
        glUniformMatrix4fv(locMVReflect, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
        glUniformMatrix3fv(locNormalReflect, 1, GL_FALSE, transformPipeline.GetNormalMatrix());
		glUniformMatrix4fv(locInvertedCamera, 1, GL_FALSE, mInverseCamera);

		glEnable(GL_CULL_FACE);
        sphereBatch.Draw();
		glDisable(GL_CULL_FACE);
	modelViewMatrix.PopMatrix();

	modelViewMatrix.PushMatrix();
	    modelViewMatrix.MultMatrix(mCameraRotOnly);
		glUseProgram(skyBoxShader);
		glUniformMatrix4fv(locMVPSkyBox, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
		cubeBatch.Draw();       
    modelViewMatrix.PopMatrix();
        
    // Zamiana buforów
    glutSwapBuffers();
    }



// Do sterowania kamer¹ s³u¿¹ klawisze strza³ek
void SpecialKeys(int key, int x, int y)
    {
    if(key == GLUT_KEY_UP)
        viewFrame.MoveForward(0.1f);

    if(key == GLUT_KEY_DOWN)
        viewFrame.MoveForward(-0.1f);

    if(key == GLUT_KEY_LEFT)
        viewFrame.RotateLocalY(0.1);
      
    if(key == GLUT_KEY_RIGHT)
        viewFrame.RotateLocalY(-0.1);
                        
    // Odœwie¿enie okna
    glutPostRedisplay();
    }


void ChangeSize(int w, int h)
    {
    // Ochrona przed dzieleniem przez zero
    if(h == 0)
        h = 1;
    
    // Ustawienie widoku na rozmiar okna
    glViewport(0, 0, w, h);
    
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 1000.0f);
    
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

int main(int argc, char* argv[])
    {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800,600);
    glutCreateWindow("Tekstury szeœcienne w OpenGL");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "B³¹d GLEW: %s\n", glewGetErrorString(err));
        return 1;
    }
    
    
    SetupRC();

    glutMainLoop();
    
    ShutdownRC();
        
    return 0;
    }
