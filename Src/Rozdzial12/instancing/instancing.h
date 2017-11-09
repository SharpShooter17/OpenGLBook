// hdr_bloom.h


#ifndef __INSTANCING__
#define __INSTANCING__

#include <gltools.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

class InstancingApp
    {
    public:
        InstancingApp();
        virtual ~InstancingApp() {};

        void Initialize(void);                            // Wywo³ywana po utworzeniu kontekstu
        void Shutdown(void);                            // Wywo³ywana przed usuniêciem kontekstu
        void Resize(GLsizei nWidth, GLsizei nHeight);    // Wywo³ywana przy zmianie rozmiaru okna, przynajmniej raz, gdy okno jest tworzone 
        void Render(void);                                // Wywo³ywana w celu aktualizacji widoku OpenGL
        
        // Te metody s¹ u¿ywane przez wywo³uj¹cy szkielet. Ustaw odpowiednie wewnêtrzne
        // zmienne chronione, aby poinformowaæ szkielet nadrzêdny jaka jest ¿¹dana konfiguracja
        inline GLuint GetWidth(void) { return screenWidth; }
        inline GLuint GetHeight(void) { return screenHeight; }
        inline GLboolean GetFullScreen(void) { return bFullScreen; }
        inline GLboolean GetAnimated(void) { return bAnimated; }

    protected:
        GLsizei     screenWidth;            // ¯¹dana szerokoœæ okna lub pulpitu
        GLsizei     screenHeight;            // ¯¹dana wysokoœæ okna lub pulpitu
        
        GLboolean bFullScreen;            // ¯¹danie dzia³ania w trybie pe³noekranowym
        GLboolean bAnimated;            // ¯¹danie ci¹g³ych aktualizacji

        GLMatrixStack        modelViewMatrix;        // Macierz model-widok
        GLMatrixStack        projectionMatrix;        // Macierz rzutowania
        GLFrustum            viewFrustum;            // Frusta widoku
        GLGeometryTransform    transformPipeline;        // Potok przetwarzania geometrii
        GLFrame                cameraFrame;            // Uk³ad odniesienia kamery
        GLBatch             screenQuad;
        M3DMatrix44f        orthoMatrix;  

        GLuint                instancingProg;

        GLuint              square_vao;
        GLuint              square_vbo;

        GLuint              hdrFBO[1];
        GLuint              brightPassFBO[4];
        GLuint                textures[1];
        GLuint                hdrTextures[1];
        GLuint                brightBlurTextures[5];
        GLuint                windowTexture;
        GLfloat                exposure;
        GLfloat                bloomLevel;
        GLfloat                texCoordOffsets[5*5*2];
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
