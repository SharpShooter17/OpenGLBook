// SphereWorld_redux.cpp
// OpenGL. Ksiêga eksperta, Rozdzia³ 13
// Ulepszona wersja
// Autor programu: Richard S. Wright Jr. and Nicholas Haemel

#include <gltools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <Stopwatch.h>
#include <gl\wglew.h>
#include <math.h>
#include <stdio.h>
#include <iostream>


GLShaderManager		shaderManager;			// Manager wierzcho³ków
GLMatrixStack		modelViewMatrix;		// Macierz model-widok
GLMatrixStack		projectionMatrix;		// Macierz rzutowania
GLFrustum			viewFrustum;			// Frusta widoku
GLGeometryTransform	transformPipeline;		// Potok przetwarzania geometrii
GLFrame				cameraFrame;			// Uk³ad odniesienia kamery

GLTriangleBatch		torusBatch;
GLTriangleBatch		sphereBatch;
GLBatch				groundBatch;
GLBatch				floorBatch;

GLuint				uiTextures[3];

static const TCHAR g_szAppName[] = TEXT("Sphere World Redux");
static const TCHAR g_szClassName[50]  =  TEXT("OGL_CLASS");

static const int g_nWinWidth  = 800;
static const int g_nWinHeight = 600;

    HWND         g_hWnd;
    HGLRC        g_hRC;
    HDC          g_hDC;
HINSTANCE    g_hInstance;
WNDCLASS     g_windClass; 
RECT         g_windowRect;
bool         g_ContinueRendering;
bool         g_InFullScreen;

void DrawSongAndDance(GLfloat yRot)		// Rysuje tañcz¹ce obiekty
{
	static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static GLfloat vLightPos[] = { 0.0f, 3.0f, 0.0f, 1.0f };
	
	// Pozycja Ÿród³a œwiat³a w przestrzeni oka
	M3DVector4f	vLightTransformed;
	M3DMatrix44f mCamera;
	modelViewMatrix.GetMatrix(mCamera);
	m3dTransformVector4(vLightTransformed, vLightPos, mCamera);
	
	// Rysowanie Ÿród³a œwiat³a
	modelViewMatrix.PushMatrix();
	modelViewMatrix.Translatev(vLightPos);
	shaderManager.UseStockShader(GLT_SHADER_FLAT, 
								 transformPipeline.GetModelViewProjectionMatrix(),
								 vWhite);
	sphereBatch.Draw();
	modelViewMatrix.PopMatrix();
	
	// Œpiew i taniec
	modelViewMatrix.Translate(0.0f, 0.2f, -2.5f);
	modelViewMatrix.PushMatrix();	// Przechowuje przesuniêty pocz¹tek
	modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
	
	// Rysowanie obiektów wzglêdem kamery
	glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
	shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
								 modelViewMatrix.GetMatrix(),
								 transformPipeline.GetProjectionMatrix(),
								 vLightTransformed, 
								 vWhite,
								 0);
	torusBatch.Draw();
	modelViewMatrix.PopMatrix(); // Usuniêcie obrotu
	
	modelViewMatrix.Rotate(yRot * -2.0f, 0.0f, 1.0f, 0.0f);
	modelViewMatrix.Translate(0.8f, 0.0f, 0.0f);
	
	glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
	shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
								 modelViewMatrix.GetMatrix(),
								 transformPipeline.GetProjectionMatrix(),
								 vLightTransformed, 
								 vWhite,
								 0);
	sphereBatch.Draw();
}
	
	
bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
	GLbyte *pBits;
	int nWidth, nHeight, nComponents;
	GLenum eFormat;
	
	// Wczytanie bitów tekstury
	pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
	if(pBits == NULL) 
		return false;
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	
	if(minFilter == GL_LINEAR_MIPMAP_LINEAR || 
	   minFilter == GL_LINEAR_MIPMAP_NEAREST ||
	   minFilter == GL_NEAREST_MIPMAP_LINEAR ||
	   minFilter == GL_NEAREST_MIPMAP_NEAREST)
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0,
				 eFormat, GL_UNSIGNED_BYTE, pBits);
	
	free(pBits);
	return true;
}

        
//////////////////////////////////////////////////////////////////
// Funkcja inicjalizuj¹ca kontekst renderowania

void SetupRC()
{
	// Inicjalizacja managera shaderów
	shaderManager.InitializeStockShaders();
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	// Utworzenie torusa
	gltMakeTorus(torusBatch, 0.4f, 0.15f, 20, 20);
	
	// Utworzenie kuli
	gltMakeSphere(sphereBatch, 0.1f, 26, 13);
	
	// Utworzenie linii na pod³o¿u
	groundBatch.Begin(GL_LINES, 328);
	for(GLfloat x = -20.0f; x <= 20.0f; x+= 0.5f)
	{
		groundBatch.Vertex3f(x, -0.4f, 20.0f);	// Równoleg³e do osi x, prostopad³e do osi z
		groundBatch.Vertex3f(x, -0.4f, -20.0f);
		
		groundBatch.Vertex3f(20.0f, -0.4f, x); // Równoleg³e do osi z, prostopad³e do osi x
		groundBatch.Vertex3f(-20.0f, -0.4f, x);
	}
	groundBatch.End();
	
	
	// Utworzenie jednolitego pod³o¿a
	GLfloat alpha = 0.25f;
	GLfloat texSize = 10.0f;
	floorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
	floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	floorBatch.Color4f(1.0f, 1.0f, 1.0f, alpha);
	floorBatch.Vertex3f(-20.0f, -0.41f, 20.0f);
	
	floorBatch.MultiTexCoord2f(0, texSize, 0.0f);
	floorBatch.Color4f(1.0f, 1.0f, 1.0f, alpha);
	floorBatch.Vertex3f(20.0f, -0.41f, 20.0f);
	
	floorBatch.MultiTexCoord2f(0, texSize, texSize);
	floorBatch.Color4f(1.0f, 1.0f, 1.0f, alpha);
	floorBatch.Vertex3f(20.0f, -0.41f, -20.0f);
	
	floorBatch.MultiTexCoord2f(0, 0.0f, texSize);
	floorBatch.Color4f(1.0f, 1.0f, 1.0f, alpha);
	floorBatch.Vertex3f(-20.0f, -0.41f, -20.0f);
	floorBatch.End();
	
	// Utworzenie trzech obiektów teksturowych
	glGenTextures(3, uiTextures);
	
	// Wczytanie marmumru
	glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
	LoadTGATexture("marble.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
	
	// Wczytanie Marsa
	glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
	LoadTGATexture("marslike.tga", GL_LINEAR_MIPMAP_LINEAR, 
				   GL_LINEAR, GL_CLAMP_TO_EDGE);
	
	// Wczytanie ksiê¿yca
	glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
	LoadTGATexture("moonlike.tga", GL_LINEAR_MIPMAP_LINEAR,
				   GL_LINEAR, GL_CLAMP_TO_EDGE);
}

void CheckErrors(GLuint progName = 0)
{
	GLenum error = glGetError();
		
	if (error != GL_NO_ERROR)
	{
		fprintf(stderr, "B³¹d GL\n");
	}
	
	GLenum fboStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

	if(fboStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		switch (fboStatus)
		{
		case GL_FRAMEBUFFER_UNDEFINED:
			// Nie ma ¿adnego okna?
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			// Sprawdzenie stanu wszystkich wi¹zañ
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			// Przywi¹zanie przynajmniej jednego bufora do FBO
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			// Sprawdzenie czy wszystkie wi¹zania w³¹czone za pomoc¹ funkcji
			// glDrawBuffers s¹ w FBO
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			/// Sprawdzenie czy bufor okreœlony za pomoc¹ funkcji
			// glReadBuffer jest w FBO
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			// Zrewiduj formaty buforów
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			// Upewnienie siê, czy liczba próbek dla ka¿dego
			// wi¹zania jest taka sama
			break;
		//case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			// Upewnienie siê, czy liczba warstw dla ka¿dego
			// wi¹zania jest taka sama
			//break;
		}
		fprintf(stderr,"Bufora obrazu jest niekompletny\n");
	}

	if (progName != 0)
	{
		glValidateProgram(progName);
		int iIsProgValid = 0;
		glGetProgramiv(progName, GL_VALIDATE_STATUS, &iIsProgValid);
		if(iIsProgValid == 0)
		{
			fprintf(stderr,"Aktualny program jest nieprawid³owy\n");
		}
	}
}

        
// Rysuje scenê
void RenderScene(void)
{
    if(!g_ContinueRendering)
        return;

	static CStopWatch	rotTimer;
	float yRot = rotTimer.GetElapsedSeconds() * 60.0f;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	modelViewMatrix.PushMatrix();	
	M3DMatrix44f mCamera;
	cameraFrame.GetCameraMatrix(mCamera);
	modelViewMatrix.MultMatrix(mCamera);
	
	// Rysuje œwiat do góry nogami
	modelViewMatrix.PushMatrix();
	modelViewMatrix.Scale(1.0f, -1.0f, 1.0f); // Przekrêca oœ y
	modelViewMatrix.Translate(0.0f, 0.8f, 0.0f);
	glFrontFace(GL_CW);
	DrawSongAndDance(yRot);
	glFrontFace(GL_CCW);
	modelViewMatrix.PopMatrix();
	
	// Rysowanie jednolitego pod³o¿a
	glEnable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	static GLfloat vFloorColor[] = { 1.0f, 1.0f, 1.0f, 0.75f};
	shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE,
								 transformPipeline.GetModelViewProjectionMatrix(),
								 vFloorColor,
								 0);
	
	floorBatch.Draw();
	glDisable(GL_BLEND);
	
	DrawSongAndDance(yRot);
	
	modelViewMatrix.PopMatrix();
       
    // Zamiana buforów
    SwapBuffers(g_hDC);

    CheckErrors();
}



// Poruszanie kamer¹ w odpowiedzi na naciskanie klawiszy strza³ek
void SpecialKeys(int key)
    {
	float linear = 0.1f;
	float angular = float(m3dDegToRad(5.0f));
	
	if(key == VK_UP)
		cameraFrame.MoveForward(linear);
	
	if(key == VK_DOWN)
		cameraFrame.MoveForward(-linear);
	
	if(key == VK_LEFT)
		cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);
	
	if(key == VK_RIGHT)
		cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);		
    
	if(key == VK_ESCAPE ||key == 'q' || key == 'Q')
    {
        g_ContinueRendering = false;
        PostQuitMessage(0);
    }
}

void ChangeSize(int nWidth, int nHeight)
{
	glViewport(0, 0, nWidth, nHeight);
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
	
	viewFrustum.SetPerspective(35.0f, float(nWidth)/float(nHeight), 1.0f, 100.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.LoadIdentity();
}

///////////////////////////////////////////////////////////////////////////////
// Funkcje zwrotne do obs³ugi wszystkich funkcji okna, którymi zajmuje siê ta aplikacja
// Po zakoñczeniu, przekazujemy komunikat do nastêpnej aplikacji
LRESULT CALLBACK WndProc(	HWND	hWnd,		// Uchwyt do tego okna
                            UINT	uMsg,		// Komunikat dla tego okna
                            WPARAM	wParam,		// Dodatkowe informacje komunikatu
                            LPARAM	lParam)		// Dodatkowe informacje komunikatu
{
    // Indywidualna obs³uga poszczególnych komunikatów
    switch(uMsg)
    {
    case WM_ACTIVATE:
    case WM_SETFOCUS:
        RenderScene();
        return 0;
    case WM_SIZE:
        ChangeSize(LOWORD(lParam),HIWORD(lParam));
        RenderScene();
        break;
    case WM_CLOSE:
        g_ContinueRendering = false;
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        SpecialKeys((unsigned int)wParam);
        return 0;
    default:
        // Nie ma nic do roboty
        break;
    }

    // Przekazanie wszystkich nieobs³u¿onych komunikatów do DefWindowProc
    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}



///////////////////////////////////////////////////////////////////////////////
// Konfiguracja okna i jego stanu.
// Utworzenie okna, znalezienie formatu piksela i utworzenie kontekstu OpenGL
bool SetupWindow(int nWidth, int nHeight, bool bUseFS)
{
    bool bRetVal = true;

    int nWindowX = 0;
    int nWindowY = 0;
    int nPixelFormat  = -1;
    PIXELFORMATDESCRIPTOR pfd;

    DWORD dwExtStyle;
    DWORD dwWindStyle;

    HINSTANCE g_hInstance = GetModuleHandle(NULL);

    // Konfiguracja klasy okna
    g_windClass.lpszClassName = g_szClassName;     // Nazwa klasy
    g_windClass.lpfnWndProc   = (WNDPROC)WndProc;
    g_windClass.hInstance     = g_hInstance; // Uchwyt modu³u

    // Wybór domyœlnego kursora myszy
    g_windClass.hCursor       = LoadCursor(NULL, IDC_ARROW);

    // Wybór domyœlnych ikon okien
    g_windClass.hIcon         = LoadIcon(NULL, IDI_WINLOGO); 
    g_windClass.hbrBackground = NULL; // Brak t³a
    g_windClass.lpszMenuName  = NULL; // Brak menu w tym oknie

    // Ustawienie stylów tej klasy, aby przechwyciæ
    // operacje ponownego rysowania okna, unikatowe konteksty urz¹dzenia i zmiany rozmiaru
    g_windClass.style         = CS_HREDRAW | CS_OWNDC | CS_VREDRAW; 
    g_windClass.cbClsExtra    = 0; // Dodatkowa pamiêæ klasy
    g_windClass.cbWndExtra    = 0; // Dodatkowa pamiêæ okna


    // Rejestracja nowej klasy
    if(!RegisterClass( &g_windClass ))
        bRetVal = false;

    dwExtStyle  = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    dwWindStyle = WS_OVERLAPPEDWINDOW;
    ShowCursor(TRUE);

    g_windowRect.left   = nWindowX;
    g_windowRect.right  = nWindowX + nWidth;
    g_windowRect.top    = nWindowY;
    g_windowRect.bottom = nWindowY + nHeight;

    // Ustawienie szerokoœci i wysokoœci okna
    AdjustWindowRectEx(&g_windowRect, dwWindStyle, FALSE, dwExtStyle);

    // Dostosowanie do ozdób
    int nWindowWidth = g_windowRect.right   - g_windowRect.left;
    int nWindowHeight = g_windowRect.bottom - g_windowRect.top;

    // Utworzenie okna
    g_hWnd = CreateWindowEx(dwExtStyle,     // Rozszerzony styl
                            g_szClassName,    // Nazwa klasy
                            g_szAppName,   // Nazwa okna
                            dwWindStyle |        
                            WS_CLIPSIBLINGS | 
                            WS_CLIPCHILDREN,// Styl okna
        nWindowX,       // Po³o¿enie okna — x
        nWindowY,       // Po³o¿enie okna — y
        nWindowWidth,   // Wysokoœæ
        nWindowHeight,  // Szerokoœæ
        NULL,           // Okno nadrzêdne
        NULL,           // Menu
        g_hInstance,    // Egzemplarz
        NULL);          // Przeka¿ to do WM_CREATE


    // Mamy ju¿ okno, wiêc przechodzimy do deskryptora formatu pikseli
    g_hDC = GetDC(g_hWnd);

    // Ustawienie zastêpczego formatu pikseli, aby uzyskaæ dostêp do funkcji WGL
    SetPixelFormat( g_hDC, 1,&pfd);
    // Utworzenie kontekstu OGL i ustawienie go jako bie¿¹cy
    g_hRC = wglCreateContext( g_hDC );
    wglMakeCurrent( g_hDC, g_hRC );

    if (g_hDC == 0 ||
        g_hDC == 0)
    {
        bRetVal = false;
        printf("!!! Podczas tworzenia okna OpenGL wyst¹pi³ b³¹d.\n");
    }

    // Ustawienie biblioteki GLEW, która ³aduje wskaŸniki funkcji OGL
    GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: wywo³anie funkcji glewInit nie powiod³o siê */
        bRetVal = false;
		printf("B³¹d: %s\n", glewGetErrorString(err));
	}
    const GLubyte *oglVersion = glGetString(GL_VERSION);
    printf("Ten system obs³uguje OpenGL w wersji %s.\n", oglVersion);

    // Rozszerzenia s¹ skonfigurowane. Usuwamy okno i zaczynamy od nowa wybór odpowiedniego formatu
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_hRC);
    ReleaseDC(g_hWnd, g_hDC);
    DestroyWindow(g_hWnd);

    if(bUseFS)
    {
        // Przygotowanie na tryb ustawiony na ¿¹dan¹ rozdzielczoœæ
        DEVMODE dm;					
		memset(&dm,0,sizeof(dm));	
		dm.dmSize=sizeof(dm);		
		dm.dmPelsWidth	= nWidth;	
		dm.dmPelsHeight	= nHeight;	
		dm.dmBitsPerPel	= 32;		
		dm.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

        long error = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
        
        if (error != DISP_CHANGE_SUCCESSFUL)
		{
            // Coœ posz³o nie tak
            if (MessageBox(NULL, "Ustawienie trybu pe³noekranowego nie powiod³o siê.\n"
                "Twoja karta graficzna prawdopodobnie nie obs³uguje ¿¹danego trybu..\n"
                "Czy chcesz u¿yæ w zamian trybu okiennego?", 
                g_szAppName, MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
            {
                g_InFullScreen = false;
                dwExtStyle  = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
                dwWindStyle = WS_OVERLAPPEDWINDOW;
			}
			else
			{
				MessageBox(NULL, "Program zostanie zamkniêty.", "ERROR", MB_OK|MB_ICONSTOP);
				return false;
			}
        }
        else
        {
            // Tryb ustawiony, konfiguracja stylów dla trybu pe³noekranowego
            g_InFullScreen = true;
            dwExtStyle  = WS_EX_APPWINDOW;
            dwWindStyle = WS_POPUP;
            ShowCursor(FALSE);
        }
    }

    AdjustWindowRectEx(&g_windowRect, dwWindStyle, FALSE, dwExtStyle);

    // Ponowne utworzenie okna
    g_hWnd = CreateWindowEx(dwExtStyle,     // Styl rozszerzony
                            g_szClassName,    // Nazwa klasy
                            g_szAppName,   // Nazwa okna
                            dwWindStyle |        
                            WS_CLIPSIBLINGS | 
                            WS_CLIPCHILDREN,// Styl okna
                            nWindowX,       // Po³o¿enie okna — x
        nWindowY,       // Po³o¿enie okna — y
        nWindowWidth,   // Wysokoœæ
        nWindowHeight,  // Szerokoœæ
        NULL,           // Okno nadrzêdne
        NULL,           // Menu
        g_hInstance,    // Egzemplarz
        NULL);          // Przeka¿ to do WM_CREATE

    g_hDC = GetDC(g_hWnd);

    int nPixCount = 0;

    // Okreœlenie interesuj¹cych nas atrybutów
    int pixAttribs[] = { 
        WGL_SUPPORT_OPENGL_ARB,  1, // Musi obs³ugiwaæ rendering OGL
        WGL_DRAW_TO_WINDOW_ARB,  1, // Format pikseli, którego mo¿na u¿ywaæ w oknie
        WGL_ACCELERATION_ARB,    1, // Musi mieæ wsparcie sprzêtowe
        WGL_COLOR_BITS_ARB,     24, // 8 bitów dla wszystkich trzech kana³ów koloru
        WGL_DEPTH_BITS_ARB,     16, // 16 bitów g³êbi
        WGL_DOUBLE_BUFFER_ARB,   GL_TRUE, // Kontekst podwójnego buforowania
        WGL_SAMPLE_BUFFERS_ARB, GL_TRUE, // W³¹czony antyaliasing wielopróbkowy (MSAA)
        WGL_SAMPLES_ARB,         8, // 8x MSAA 
        WGL_PIXEL_TYPE_ARB,      WGL_TYPE_RGBA_ARB, // Format pikseli powinien byæ typu RGBA
        0 }; // Zerowe zakoñczenie


    // ¯¹damy, aby OpenGL znalaz³a najlepszy format odpowiadaj¹cy naszym wymaganiom
        // W odpowiedzi otrzymujemy tylko jeden format
    wglChoosePixelFormatARB(g_hDC, &pixAttribs[0], NULL, 1, &nPixelFormat, (UINT*)&nPixCount);

    // Usuñ ten komentarz, aby otrzymaæ listê wszystkich obs³ugiwanych formatów pikseli
    /*    
    // Obs³ugiwane formaty pikseli
    GLint pfAttribCount[] = {WGL_NUMBER_PIXEL_FORMATS_ARB};
    GLint pfAttribList[] = { WGL_DRAW_TO_WINDOW_ARB,
                             WGL_ACCELERATION_ARB,
                             WGL_SUPPORT_OPENGL_ARB,
                             WGL_DOUBLE_BUFFER_ARB,
                             WGL_DEPTH_BITS_ARB,
                             WGL_STENCIL_BITS_ARB,
                             WGL_RED_BITS_ARB,
                             WGL_GREEN_BITS_ARB,
                             WGL_BLUE_BITS_ARB,
                             WGL_ALPHA_BITS_ARB
                             };
    int nPixelFormatCount = 0;
    wglGetPixelFormatAttribivARB(g_hDC, 1, 0, 1, pfAttribCount, &nPixelFormatCount);
    for (int i=0; i<nPixelFormatCount; i++)
    {
        GLint results[10];
        printf("Pixel format %d details:\n", nPixelFormatCount);
        wglGetPixelFormatAttribivARB(g_hDC, i, 0, 10, pfAttribList, results);
        printf("    Draw to Window  = %d:\n", results[0]);
        printf("    HW Accelerated  = %d:\n", results[1]);
        printf("    Supports OpenGL = %d:\n", results[2]);
        printf("    Double Buffered = %d:\n", results[3]);
        printf("    Depth Bits   = %d:\n", results[4]);
        printf("    Stencil Bits = %d:\n", results[5]);
        printf("    Red Bits     = %d:\n", results[6]);
        printf("    Green Bits   = %d:\n", results[7]);
        printf("    Blue Bits    = %d:\n", results[8]);
        printf("    Alpha Bits   = %d:\n", results[9]);
    }
    */

    if(nPixelFormat == -1) 
    {
        printf("!!! Wyst¹pi³ b³¹d podczas szukania formatu pikseli MSAA o podanych atrybutach.\n");

        // Ponowienie próby bez MSAA
        pixAttribs[15] = 1;
        wglChoosePixelFormatARB(g_hDC, &pixAttribs[0], NULL, 1, &nPixelFormat, (UINT*)&nPixCount);
            
        if(nPixelFormat == -1) 
        {
            // Nie mo¿na znaleŸæ formatu
            g_hDC = 0;
            g_hDC = 0;
            bRetVal = false;
            printf("!!! Wyst¹pi³ b³¹d podczas szukania formatu pikseli o podanych atrybutach.\n");
        }
    }
    
    if(nPixelFormat != -1) 
    {
        // Sprawdzenie obs³ugi MSAA
        int attrib[] = { WGL_SAMPLES_ARB };
        int nResults = 0;
        wglGetPixelFormatAttribivARB(g_hDC, nPixelFormat, 0, 1, attrib, &nResults);
        printf("Wybrany format pikseli to MSAA z %d próbek.\n", nResults);

        // Mamy format, wiêc ustawiamy go jako bie¿¹cy
        SetPixelFormat( g_hDC, nPixelFormat, &pfd );

        GLint attribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB,  3,
	                       WGL_CONTEXT_MINOR_VERSION_ARB,  1,
                           0 };
        g_hRC = wglCreateContextAttribsARB(g_hDC, 0, attribs);
		if (g_hRC == NULL)
        {
            printf("!!! Nie mo¿na utworzyæ kontekstu OpenGL 3.1.\n");
            attribs[3] = 0;
            g_hRC = wglCreateContextAttribsARB(g_hDC, 0, attribs);
            if (g_hRC == NULL)
            {
                printf("!!! Nie mo¿na utworzyæ kontekstu OpenGL 3.0.\n");
                printf("!!! Biblioteka OpenGL 3.0 i jej nowsze wersje nie s¹ obs³ugiwane.\n");
            }
        }                   

        wglMakeCurrent( g_hDC, g_hRC );
    }

    if (g_hDC == 0 ||
        g_hDC == 0)
    {
        bRetVal = false;
        printf("!!! Wyst¹pi³ b³¹d podczas tworzenia okna OpenGL.\n");
    }

    // Jeœli wszystko posz³o zgodnie z planem, wyœwietl okno
    if( bRetVal )
    {
        ShowWindow( g_hWnd, SW_SHOW );
        SetForegroundWindow( g_hWnd );
        SetFocus( g_hWnd );
        g_ContinueRendering = true;
    }
    
    return bRetVal;
}

///////////////////////////////////////////////////////////////////////////////
// Porz¹dkowanie okna, kontekstu OGL i zwi¹zanego z nim stanu
// Wywo³ywana przy zamykaniu programu i po wyst¹pieniu b³êdu
bool KillWindow( )
{ 
    bool bRetVal = true;

    glDeleteTextures(3,uiTextures);
    
    if (g_InFullScreen)
    {
        ChangeDisplaySettings(NULL,0);	
		ShowCursor(TRUE);
    }
    // Czyszczenie OGL RC
    if(g_hRC) 
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(g_hRC);
        g_hRC = NULL;
    }

    // Zwolnienie DC
    if(g_hDC)
    {
        ReleaseDC(g_hWnd, g_hDC);
        g_hDC = NULL;
    }

    // Usuniêcie okna
    if(g_hWnd)
    {
        DestroyWindow(g_hWnd);
        g_hWnd = NULL;;
    }

    // Usuniêcie klasy okna
    UnregisterClass(g_szClassName, g_hInstance);
    g_hInstance = NULL;
    ShowCursor(TRUE);
    return bRetVal;
}

///////////////////////////////////////////////////////////////////////////////
// G³ówna pêtla renderuj¹ca
// Sprawdza komunikaty okna i obs³uguje zdarzenia, a tak¿e rysuje scenê
void mainLoop()
{
    MSG		msg;

    // Sprawdzenie komunikatów
    if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
    {
	    if (msg.message==WM_QUIT)
	    {
            g_ContinueRendering = false;
	    }
	    else
	    {
            // Obs³uga komunikatów
		    TranslateMessage(&msg);
		    DispatchMessage(&msg);
	    }
    }
    else
    {
        RenderScene();       
    }
}

///////////////////////////////////////////////////////////////////////////////
// Funkcja startowa programu
// Najpierw konfiguruje okno i stan OGL, a potem wykonuje pêtlê renderuj¹c¹
int main(int argc, char* argv[])
{
    gltSetWorkingDirectory(argv[0]);
        
    bool bUseFS = false;
    if (MessageBox(NULL, "Czy chcesz uruchomiæ program w trybie pe³noekranowym?", g_szAppName, MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
    {
        bUseFS = true;
    }

    if(SetupWindow(g_nWinWidth, g_nWinHeight, bUseFS))
    {
        SetupRC();
        ChangeSize(g_nWinWidth, g_nWinHeight);
        while (g_ContinueRendering)
        {   
            mainLoop();
            Sleep(0);
        }
    }
    KillWindow();
	return 0;
}

