// sbmviewer.h


#ifndef __SBMVIEWER__
#define __SBMVIEWER__

#include <gltools.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

class SBMViewer
    {
    public:
        SBMViewer();
        virtual ~SBMViewer() {};

        void Initialize(void);                            // Wywo�ywana po utworzeniu kontekstu
        void Shutdown(void);                            // Wywo�ywana przed usuni�ciem kontekstu
        void Resize(GLsizei nWidth, GLsizei nHeight);    // Wywo�ywana przy zmianie rozmiaru okna, przynajmniej raz, gdy okno jest tworzone 
        void Render(void);                                // Wywo�ywana w celu aktualizacji widoku OpenGL
        
        // Te metody s� u�ywane przez wywo�uj�cy szkielet. Ustaw odpowiednie wewn�trzne
        // zmienne chronione, aby poinformowa� szkielet nadrz�dny jaka jest ��dana konfiguracja
        inline GLuint GetWidth(void) { return screenWidth; }
        inline GLuint GetHeight(void) { return screenHeight; }
        inline GLboolean GetFullScreen(void) { return bFullScreen; }
        inline GLboolean GetAnimated(void) { return bAnimated; }

    protected:
        GLsizei     screenWidth;            // ��dana szeroko�� okna lub pulpitu
        GLsizei     screenHeight;            // ��dana wysoko�� okna lub pulpitu
        
        GLboolean bFullScreen;            // ��danie dzia�ania w trybie pe�noekranowym
        GLboolean bAnimated;            // ��danie ci�g�ych aktualizacji

        GLMatrixStack        modelViewMatrix;        // Macierz model-widok
        GLMatrixStack        projectionMatrix;        // Macierz rzutowania
        GLFrustum            viewFrustum;            // Frusta widoku
        GLGeometryTransform    transformPipeline;        // Potok przetwarzania geometrii
        GLFrame                cameraFrame;            // Uk�ad odniesienia kamery
        GLBatch             screenQuad;
        M3DMatrix44f        orthoMatrix;  

        GLuint                instancingProg;

        GLuint              square_vao;
        GLuint              square_vbo;
        GLuint              element_buffer;

        GLuint              hdrFBO[1];
        GLuint              brightPassFBO[4];
        GLuint                textures[1];
        GLuint                hdrTextures[1];
        GLuint                brightBlurTextures[5];
        GLuint                windowTexture;
        GLfloat                exposure;
        GLfloat                bloomLevel;
        GLfloat                texCoordOffsets[5*5*2];
    };
        
#endif // __SBMVIEWER__
