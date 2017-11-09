// hdr_bloom.h


#ifndef __INSTANCING__
#define __INSTANCING__

#include <gltools.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

class ConvolutionApp
    {
    public:
        ConvolutionApp();
        virtual ~ConvolutionApp() {};

        void Initialize(void);                            // Wywo�ywana po utworzeniu kontekstu
        void Shutdown(void);                            // Wywo�ywana przed usuni�ciem kontekstu
        void Resize(GLsizei nWidth, GLsizei nHeight);    // Wywo�ywana przy zmianie rozmiaru okna, przynajmniej raz, gdy okno jest tworzone 
        void Render(void);                                // Wywo�ywana w celu aktualizacji widoku OpenGL
        
        // Te metody s� u�ywane przez wywo�uj�cy szkielet. Ustaw odpowiednie wewn�trzne
        // zmienne chronione, aby poinformowa� szkielet nadrz�dny jaka jest ��dana konfiguracja
        inline GLuint GetWidth(void) { return screenWidth; }
        inline GLuint GetHeight(void) { return screenHeight; }
        inline GLboolean GetFullScreen(void) { return bFullScreen; }
        inline GLboolean GetAnimated(void) { return GL_FALSE; }

    protected:
        GLsizei     screenWidth;            // ��dana szeroko�� okna lub pulpitu
        GLsizei     screenHeight;            // ��dana wysoko�� okna lub pulpitu

        GLboolean bFullScreen;            // ��danie dzia�ania w trybie pe�noekranowym

        GLMatrixStack        modelViewMatrix;        // Macierz model-widok
        GLMatrixStack        projectionMatrix;        // Macierz rzutowania
        GLFrustum            viewFrustum;            // Frusta widoku
        GLGeometryTransform    transformPipeline;        // Potok przetwarzania geometrii
        GLFrame                cameraFrame;            // Uk�ad odniesienia kamery
        GLBatch             screenQuad;
        M3DMatrix44f        orthoMatrix;  

        GLuint                instancingProg;

        GLuint              absValueProg;

        GLuint              square_vao;
        GLuint              square_vbo;

        GLuint              sourceTexture;
        GLuint              gaussian_kernelTexture;
        GLuint              gaussian_kernelBuffer;

        GLuint              sobel_kernelTexture1;
        GLuint              sobel_kernelTexture2;
        GLuint              sobel_kernelBuffer1;
        GLuint              sobel_kernelBuffer2;

        GLuint              intermediateFBO;
        GLuint              intermediateTexture;

        GLuint              targetFBO;
        GLuint              targetTexture;

        GLfloat             kernelScale[2];

        void UpdateMode(void);
        void GenerateOrtho2DMat(GLuint imageWidth, GLuint imageHeight);
        bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);
        bool LoadOpenEXRImage(char *fileName, GLint textureName, GLuint &texWidth, GLuint &texHeight);
        void GenTexCoordOffsets(GLuint width, GLuint height);
        void SetupTexReplaceProg(GLfloat *vLightPos, GLfloat *vColor);
        void SetupFlatColorProg(GLfloat *vLightPos, GLfloat *vColor);
        void SetupHDRProg();
        void SetupBlurProg();
    };
        
#endif // __INSTANCING__
