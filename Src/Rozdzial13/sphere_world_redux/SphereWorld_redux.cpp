// SphereWorld_redux.cpp
// OpenGL. Ksi�ga eksperta, Rozdzia� 13
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


GLShaderManager		shaderManager;			// Manager wierzcho�k�w
GLMatrixStack		modelViewMatrix;		// Macierz model-widok
GLMatrixStack		projectionMatrix;		// Macierz rzutowania
GLFrustum			viewFrustum;			// Frusta widoku
GLGeometryTransform	transformPipeline;		// Potok przetwarzania geometrii
GLFrame				cameraFrame;			// Uk�ad odniesienia kamery

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

void DrawSongAndDance(GLfloat yRot)		// Rysuje ta�cz�ce obiekty
{
	static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static GLfloat vLightPos[] = { 0.0f, 3.0f, 0.0f, 1.0f };
	
	// Pozycja �r�d�a �wiat�a w przestrzeni oka
	M3DVector4f	vLightTransformed;
	M3DMatrix44f mCamera;
	modelViewMatrix.GetMatrix(mCamera);
	m3dTransformVector4(vLightTransformed, vLightPos, mCamera);
	
	// Rysowanie �r�d�a �wiat�a
	modelViewMatrix.PushMatrix();
	modelViewMatrix.Translatev(vLightPos);
	shaderManager.UseStockShader(GLT_SHADER_FLAT, 
								 transformPipeline.GetModelViewProjectionMatrix(),
								 vWhite);
	sphereBatch.Draw();
	modelViewMatrix.PopMatrix();
	
	// �piew i taniec
	modelViewMatrix.Translate(0.0f, 0.2f, -2.5f);
	modelViewMatrix.PushMatrix();	// Przechowuje przesuni�ty pocz�tek
	modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
	
	// Rysowanie obiekt�w wzgl�dem kamery
	glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
	shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
								 modelViewMatrix.GetMatrix(),
								 transformPipeline.GetProjectionMatrix(),
								 vLightTransformed, 
								 vWhite,
								 0);
	torusBatch.Draw();
	modelViewMatrix.PopMatrix(); // Usuni�cie obrotu
	
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
	
	// Wczytanie bit�w tekstury
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
// Funkcja inicjalizuj�ca kontekst renderowania

void SetupRC()
{
	// Inicjalizacja managera shader�w
	shaderManager.InitializeStockShaders();
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	// Utworzenie torusa
	gltMakeTorus(torusBatch, 0.4f, 0.15f, 20, 20);
	
	// Utworzenie kuli
	gltMakeSphere(sphereBatch, 0.1f, 26, 13);
	
	// Utworzenie linii na pod�o�u
	groundBatch.Begin(GL_LINES, 328);
	for(GLfloat x = -20.0f; x <= 20.0f; x+= 0.5f)
	{
		groundBatch.Vertex3f(x, -0.4f, 20.0f);	// R�wnoleg�e do osi x, prostopad�e do osi z
		groundBatch.Vertex3f(x, -0.4f, -20.0f);
		
		groundBatch.Vertex3f(20.0f, -0.4f, x); // R�wnoleg�e do osi z, prostopad�e do osi x
		groundBatch.Vertex3f(-20.0f, -0.4f, x);
	}
	groundBatch.End();
	
	
	// Utworzenie jednolitego pod�o�a
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
	
	// Utworzenie trzech obiekt�w teksturowych
	glGenTextures(3, uiTextures);
	
	// Wczytanie marmumru
	glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
	LoadTGATexture("marble.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
	
	// Wczytanie Marsa
	glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
	LoadTGATexture("marslike.tga", GL_LINEAR_MIPMAP_LINEAR, 
				   GL_LINEAR, GL_CLAMP_TO_EDGE);
	
	// Wczytanie ksi�yca
	glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
	LoadTGATexture("moonlike.tga", GL_LINEAR_MIPMAP_LINEAR,
				   GL_LINEAR, GL_CLAMP_TO_EDGE);
}

void CheckErrors(GLuint progName = 0)
{
	GLenum error = glGetError();
		
	if (error != GL_NO_ERROR)
	{
		fprintf(stderr, "B��d GL\n");
	}
	
	GLenum fboStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

	if(fboStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		switch (fboStatus)
		{
		case GL_FRAMEBUFFER_UNDEFINED:
			// Nie ma �adnego okna?
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			// Sprawdzenie stanu wszystkich wi�za�
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			// Przywi�zanie przynajmniej jednego bufora do FBO
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			// Sprawdzenie czy wszystkie wi�zania w��czone za pomoc� funkcji
			// glDrawBuffers s� w FBO
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			/// Sprawdzenie czy bufor okre�lony za pomoc� funkcji
			// glReadBuffer jest w FBO
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			// Zrewiduj formaty bufor�w
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			// Upewnienie si�, czy liczba pr�bek dla ka�dego
			// wi�zania jest taka sama
			break;
		//case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			// Upewnienie si�, czy liczba warstw dla ka�dego
			// wi�zania jest taka sama
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
			fprintf(stderr,"Aktualny program jest nieprawid�owy\n");
		}
	}
}

        
// Rysuje scen�
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
	
	// Rysuje �wiat do g�ry nogami
	modelViewMatrix.PushMatrix();
	modelViewMatrix.Scale(1.0f, -1.0f, 1.0f); // Przekr�ca o� y
	modelViewMatrix.Translate(0.0f, 0.8f, 0.0f);
	glFrontFace(GL_CW);
	DrawSongAndDance(yRot);
	glFrontFace(GL_CCW);
	modelViewMatrix.PopMatrix();
	
	// Rysowanie jednolitego pod�o�a
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
       
    // Zamiana bufor�w
    SwapBuffers(g_hDC);

    CheckErrors();
}



// Poruszanie kamer� w odpowiedzi na naciskanie klawiszy strza�ek
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
// Funkcje zwrotne do obs�ugi wszystkich funkcji okna, kt�rymi zajmuje si� ta aplikacja
// Po zako�czeniu, przekazujemy komunikat do nast�pnej aplikacji
LRESULT CALLBACK WndProc(	HWND	hWnd,		// Uchwyt do tego okna
                            UINT	uMsg,		// Komunikat dla tego okna
                            WPARAM	wParam,		// Dodatkowe informacje komunikatu
                            LPARAM	lParam)		// Dodatkowe informacje komunikatu
{
    // Indywidualna obs�uga poszczeg�lnych komunikat�w
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

    // Przekazanie wszystkich nieobs�u�onych komunikat�w do DefWindowProc
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
    g_windClass.hInstance     = g_hInstance; // Uchwyt modu�u

    // Wyb�r domy�lnego kursora myszy
    g_windClass.hCursor       = LoadCursor(NULL, IDC_ARROW);

    // Wyb�r domy�lnych ikon okien
    g_windClass.hIcon         = LoadIcon(NULL, IDI_WINLOGO); 
    g_windClass.hbrBackground = NULL; // Brak t�a
    g_windClass.lpszMenuName  = NULL; // Brak menu w tym oknie

    // Ustawienie styl�w tej klasy, aby przechwyci�
    // operacje ponownego rysowania okna, unikatowe konteksty urz�dzenia i zmiany rozmiaru
    g_windClass.style         = CS_HREDRAW | CS_OWNDC | CS_VREDRAW; 
    g_windClass.cbClsExtra    = 0; // Dodatkowa pami�� klasy
    g_windClass.cbWndExtra    = 0; // Dodatkowa pami�� okna


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

    // Ustawienie szeroko�ci i wysoko�ci okna
    AdjustWindowRectEx(&g_windowRect, dwWindStyle, FALSE, dwExtStyle);

    // Dostosowanie do ozd�b
    int nWindowWidth = g_windowRect.right   - g_windowRect.left;
    int nWindowHeight = g_windowRect.bottom - g_windowRect.top;

    // Utworzenie okna
    g_hWnd = CreateWindowEx(dwExtStyle,     // Rozszerzony styl
                            g_szClassName,    // Nazwa klasy
                            g_szAppName,   // Nazwa okna
                            dwWindStyle |        
                            WS_CLIPSIBLINGS | 
                            WS_CLIPCHILDREN,// Styl okna
        nWindowX,       // Po�o�enie okna � x
        nWindowY,       // Po�o�enie okna � y
        nWindowWidth,   // Wysoko��
        nWindowHeight,  // Szeroko��
        NULL,           // Okno nadrz�dne
        NULL,           // Menu
        g_hInstance,    // Egzemplarz
        NULL);          // Przeka� to do WM_CREATE


    // Mamy ju� okno, wi�c przechodzimy do deskryptora formatu pikseli
    g_hDC = GetDC(g_hWnd);

    // Ustawienie zast�pczego formatu pikseli, aby uzyska� dost�p do funkcji WGL
    SetPixelFormat( g_hDC, 1,&pfd);
    // Utworzenie kontekstu OGL i ustawienie go jako bie��cy
    g_hRC = wglCreateContext( g_hDC );
    wglMakeCurrent( g_hDC, g_hRC );

    if (g_hDC == 0 ||
        g_hDC == 0)
    {
        bRetVal = false;
        printf("!!! Podczas tworzenia okna OpenGL wyst�pi� b��d.\n");
    }

    // Ustawienie biblioteki GLEW, kt�ra �aduje wska�niki funkcji OGL
    GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: wywo�anie funkcji glewInit nie powiod�o si� */
        bRetVal = false;
		printf("B��d: %s\n", glewGetErrorString(err));
	}
    const GLubyte *oglVersion = glGetString(GL_VERSION);
    printf("Ten system obs�uguje OpenGL w wersji %s.\n", oglVersion);

    // Rozszerzenia s� skonfigurowane. Usuwamy okno i zaczynamy od nowa wyb�r odpowiedniego formatu
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_hRC);
    ReleaseDC(g_hWnd, g_hDC);
    DestroyWindow(g_hWnd);

    if(bUseFS)
    {
        // Przygotowanie na tryb ustawiony na ��dan� rozdzielczo��
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
            // Co� posz�o nie tak
            if (MessageBox(NULL, "Ustawienie trybu pe�noekranowego nie powiod�o si�.\n"
                "Twoja karta graficzna prawdopodobnie nie obs�uguje ��danego trybu..\n"
                "Czy chcesz u�y� w zamian trybu okiennego?", 
                g_szAppName, MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
            {
                g_InFullScreen = false;
                dwExtStyle  = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
                dwWindStyle = WS_OVERLAPPEDWINDOW;
			}
			else
			{
				MessageBox(NULL, "Program zostanie zamkni�ty.", "ERROR", MB_OK|MB_ICONSTOP);
				return false;
			}
        }
        else
        {
            // Tryb ustawiony, konfiguracja styl�w dla trybu pe�noekranowego
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
                            nWindowX,       // Po�o�enie okna � x
        nWindowY,       // Po�o�enie okna � y
        nWindowWidth,   // Wysoko��
        nWindowHeight,  // Szeroko��
        NULL,           // Okno nadrz�dne
        NULL,           // Menu
        g_hInstance,    // Egzemplarz
        NULL);          // Przeka� to do WM_CREATE

    g_hDC = GetDC(g_hWnd);

    int nPixCount = 0;

    // Okre�lenie interesuj�cych nas atrybut�w
    int pixAttribs[] = { 
        WGL_SUPPORT_OPENGL_ARB,  1, // Musi obs�ugiwa� rendering OGL
        WGL_DRAW_TO_WINDOW_ARB,  1, // Format pikseli, kt�rego mo�na u�ywa� w oknie
        WGL_ACCELERATION_ARB,    1, // Musi mie� wsparcie sprz�towe
        WGL_COLOR_BITS_ARB,     24, // 8 bit�w dla wszystkich trzech kana��w koloru
        WGL_DEPTH_BITS_ARB,     16, // 16 bit�w g��bi
        WGL_DOUBLE_BUFFER_ARB,   GL_TRUE, // Kontekst podw�jnego buforowania
        WGL_SAMPLE_BUFFERS_ARB, GL_TRUE, // W��czony antyaliasing wielopr�bkowy (MSAA)
        WGL_SAMPLES_ARB,         8, // 8x MSAA 
        WGL_PIXEL_TYPE_ARB,      WGL_TYPE_RGBA_ARB, // Format pikseli powinien by� typu RGBA
        0 }; // Zerowe zako�czenie


    // ��damy, aby OpenGL znalaz�a najlepszy format odpowiadaj�cy naszym wymaganiom
        // W odpowiedzi otrzymujemy tylko jeden format
    wglChoosePixelFormatARB(g_hDC, &pixAttribs[0], NULL, 1, &nPixelFormat, (UINT*)&nPixCount);

    // Usu� ten komentarz, aby otrzyma� list� wszystkich obs�ugiwanych format�w pikseli
    /*    
    // Obs�ugiwane formaty pikseli
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
        printf("!!! Wyst�pi� b��d podczas szukania formatu pikseli MSAA o podanych atrybutach.\n");

        // Ponowienie pr�by bez MSAA
        pixAttribs[15] = 1;
        wglChoosePixelFormatARB(g_hDC, &pixAttribs[0], NULL, 1, &nPixelFormat, (UINT*)&nPixCount);
            
        if(nPixelFormat == -1) 
        {
            // Nie mo�na znale�� formatu
            g_hDC = 0;
            g_hDC = 0;
            bRetVal = false;
            printf("!!! Wyst�pi� b��d podczas szukania formatu pikseli o podanych atrybutach.\n");
        }
    }
    
    if(nPixelFormat != -1) 
    {
        // Sprawdzenie obs�ugi MSAA
        int attrib[] = { WGL_SAMPLES_ARB };
        int nResults = 0;
        wglGetPixelFormatAttribivARB(g_hDC, nPixelFormat, 0, 1, attrib, &nResults);
        printf("Wybrany format pikseli to MSAA z %d pr�bek.\n", nResults);

        // Mamy format, wi�c ustawiamy go jako bie��cy
        SetPixelFormat( g_hDC, nPixelFormat, &pfd );

        GLint attribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB,  3,
	                       WGL_CONTEXT_MINOR_VERSION_ARB,  1,
                           0 };
        g_hRC = wglCreateContextAttribsARB(g_hDC, 0, attribs);
		if (g_hRC == NULL)
        {
            printf("!!! Nie mo�na utworzy� kontekstu OpenGL 3.1.\n");
            attribs[3] = 0;
            g_hRC = wglCreateContextAttribsARB(g_hDC, 0, attribs);
            if (g_hRC == NULL)
            {
                printf("!!! Nie mo�na utworzy� kontekstu OpenGL 3.0.\n");
                printf("!!! Biblioteka OpenGL 3.0 i jej nowsze wersje nie s� obs�ugiwane.\n");
            }
        }                   

        wglMakeCurrent( g_hDC, g_hRC );
    }

    if (g_hDC == 0 ||
        g_hDC == 0)
    {
        bRetVal = false;
        printf("!!! Wyst�pi� b��d podczas tworzenia okna OpenGL.\n");
    }

    // Je�li wszystko posz�o zgodnie z planem, wy�wietl okno
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
// Porz�dkowanie okna, kontekstu OGL i zwi�zanego z nim stanu
// Wywo�ywana przy zamykaniu programu i po wyst�pieniu b��du
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

    // Usuni�cie okna
    if(g_hWnd)
    {
        DestroyWindow(g_hWnd);
        g_hWnd = NULL;;
    }

    // Usuni�cie klasy okna
    UnregisterClass(g_szClassName, g_hInstance);
    g_hInstance = NULL;
    ShowCursor(TRUE);
    return bRetVal;
}

///////////////////////////////////////////////////////////////////////////////
// G��wna p�tla renderuj�ca
// Sprawdza komunikaty okna i obs�uguje zdarzenia, a tak�e rysuje scen�
void mainLoop()
{
    MSG		msg;

    // Sprawdzenie komunikat�w
    if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
    {
	    if (msg.message==WM_QUIT)
	    {
            g_ContinueRendering = false;
	    }
	    else
	    {
            // Obs�uga komunikat�w
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
// Najpierw konfiguruje okno i stan OGL, a potem wykonuje p�tl� renderuj�c�
int main(int argc, char* argv[])
{
    gltSetWorkingDirectory(argv[0]);
        
    bool bUseFS = false;
    if (MessageBox(NULL, "Czy chcesz uruchomi� program w trybie pe�noekranowym?", g_szAppName, MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
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

