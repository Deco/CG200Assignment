#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <map>

#include <GL/freeglut.h>
#include <AntTweakBar.h>

#include "matrix.h"
#include "CEngine.h"

// Settings
int iWindowSizeW = 1440;
int iWindowSizeH =  900;
// int iWindowSizeW = 1280;
// int iWindowSizeH =  720;
float fMouseSensitivity = 1.0;

// CG = Cactus Game!
CEngine *g_pEngine;
TwBar *g_pTweakBar;
int bLockMouse = 0;
int eCurrentCursor = GLUT_CURSOR_INHERIT;
int bShowCursor = 1;
int iCurrMousePosX = 0, iCurrMousePosY = 0;
bool abKeyButtonStates[256];  int iKeyButtonDownCount   = 0;
bool abMouseButtonStates[16]; int iMouseButtonDownCount = 0;

void onDisplay() {
    int iWindowId = glutGetWindow();
    
    ClearOpenGLError();
    
    bool running;
    GL_CHECK( running = g_pEngine->onWindowDisplay(iWindowId) );
    if(!running) {
        glutLeaveMainLoop();
    }
    
    if(g_pEngine->getShowUI()) {
        //GL_CHECK( TwDraw() );
        TwDraw();
        // AntTweakBar does some.. weird things!
        ClearOpenGLError();
        GL_CHECK();
    }
    
    GL_CHECK( glutSwapBuffers() );
    
    GL_CHECK( glutPostRedisplay() );
    
    GL_CHECK();
}
void onOverlayDisplay() {
    int iWindowId = glutGetWindow();
    int iLayerId = glutLayerGet(GLUT_LAYER_IN_USE);
    
    g_pEngine->onWindowDisplayOverlay(iWindowId, iLayerId);
    
    //glutSwapBuffers();
}

void onReshape(int iSizeW, int iSizeH) {
    int iWindowId = glutGetWindow();
    
    iWindowSizeW = iSizeW;
    iWindowSizeH = iSizeH;
    
    g_pEngine->onWindowResize(iWindowId, iSizeW, iSizeH);
    
    TwWindowSize(iSizeW, iSizeH);
}
void onVisibility(int iVisibilityState) {
    int iWindowId = glutGetWindow();
    
    g_pEngine->onWindowVisibility(iWindowId, iVisibilityState);
}

void onKeyboardDown(unsigned char chKeyCode, int iMousePosX, int iMousePosY) {
    int iWindowId = glutGetWindow();
    
    if(!g_pEngine->getShowUI() || !TwEventKeyboardGLUT(chKeyCode, iMousePosX, iMousePosY)) {
        
        //printf("onKeyboardDown: %d\n", chKeyCode);
        if(!abKeyButtonStates[chKeyCode]) {
            iKeyButtonDownCount++;
            abKeyButtonStates[chKeyCode] = true;
        }
        
        g_pEngine->onWindowKey(iWindowId, chKeyCode, GLUT_DOWN, iMousePosX, iMousePosY);
    }
}
void onKeyboardUp(unsigned char chKeyCode, int iMousePosX, int iMousePosY) {
    int iWindowId = glutGetWindow();
    
    /*int eModifiers = glutGetModifiers();
    int eTwModifiers = (
            (eModifiers & GLUT_ACTIVE_SHIFT ? TW_KMOD_SHIFT : 0)
        |   (eModifiers & GLUT_ACTIVE_CTRL  ? TW_KMOD_CTRL  : 0)
        |   (eModifiers & GLUT_ACTIVE_ALT   ? TW_KMOD_ALT   : 0)
    );
    
    if(!g_pEngine->getShowUI() || !TwKeyTest(chKeyCode, eTwModifiers)) {*/
    
    if(!g_pEngine->getShowUI() || true) {//abKeyButtonStates[chKeyCode]) {
        
        //printf("onKeyboardUp: %d\n", chKeyCode);
        if(abKeyButtonStates[chKeyCode]) {
            iKeyButtonDownCount--;
            abKeyButtonStates[chKeyCode] = false;
        }
        
        g_pEngine->onWindowKey(iWindowId, chKeyCode, GLUT_UP, iMousePosX, iMousePosY);
    }
}

std::map<int, int> g_mSpecialKeyTranslation {
    {GLUT_KEY_F1       , TW_KEY_F1       },
    {GLUT_KEY_F2       , TW_KEY_F2       },
    {GLUT_KEY_F3       , TW_KEY_F3       },
    {GLUT_KEY_F4       , TW_KEY_F4       },
    {GLUT_KEY_F5       , TW_KEY_F5       },
    {GLUT_KEY_F6       , TW_KEY_F6       },
    {GLUT_KEY_F7       , TW_KEY_F7       },
    {GLUT_KEY_F8       , TW_KEY_F8       },
    {GLUT_KEY_F9       , TW_KEY_F9       },
    {GLUT_KEY_F10      , TW_KEY_F10      },
    {GLUT_KEY_F11      , TW_KEY_F11      },
    {GLUT_KEY_F12      , TW_KEY_F12      },
    {GLUT_KEY_LEFT     , TW_KEY_LEFT     },
    {GLUT_KEY_UP       , TW_KEY_UP       },
    {GLUT_KEY_RIGHT    , TW_KEY_RIGHT    },
    {GLUT_KEY_DOWN     , TW_KEY_DOWN     },
    {GLUT_KEY_PAGE_UP  , TW_KEY_PAGE_UP  },
    {GLUT_KEY_PAGE_DOWN, TW_KEY_PAGE_DOWN},
    {GLUT_KEY_HOME     , TW_KEY_HOME     },
    {GLUT_KEY_END      , TW_KEY_END      },
    {GLUT_KEY_INSERT   , TW_KEY_INSERT   }
};
void onSpecialDown(int iKeyCode, int iMousePosX, int iMousePosY) {
    int iWindowId = glutGetWindow();
    
    auto it = g_mSpecialKeyTranslation.find(iKeyCode);
    if(it != g_mSpecialKeyTranslation.end()) {
        if(!g_pEngine->getShowUI() || !TwEventSpecialGLUT(iKeyCode, iMousePosX, iMousePosY)) {
            //printf("onSpecialDown: %d\n", iKeyCode);
            g_pEngine->onWindowKeySpecial(iWindowId, iKeyCode, GLUT_DOWN, iMousePosX, iMousePosY);
        }
    }
}
void onSpecialUp(int iKeyCode, int iMousePosX, int iMousePosY) {
    int iWindowId = glutGetWindow();
    auto it = g_mSpecialKeyTranslation.find(iKeyCode);
    if(it != g_mSpecialKeyTranslation.end()) {
        int iTwKeyCode = (*it).second;
        int eModifiers = glutGetModifiers();
        int eTwModifiers = (
                (eModifiers & GLUT_ACTIVE_SHIFT ? TW_KMOD_SHIFT : 0)
            |   (eModifiers & GLUT_ACTIVE_CTRL  ? TW_KMOD_CTRL  : 0)
            |   (eModifiers & GLUT_ACTIVE_ALT   ? TW_KMOD_ALT   : 0)
        );
        if(!g_pEngine->getShowUI() || true) {// || !TwKeyTest(iKeyCode, eTwModifiers)) {
            g_pEngine->onWindowKeySpecial(iWindowId, iKeyCode, GLUT_UP, iMousePosX, iMousePosY);
        }
    }
}

void onMouse(int iButtonCode, int iButtonState, int iMousePosX, int iMousePosY) {
    int iWindowId = glutGetWindow();
    //printf("%d\n", iButtonCode);
    
    if(
            !g_pEngine->getShowUI()
        ||  (iButtonState == GLUT_UP && abMouseButtonStates[iButtonCode])
        ||  !TwEventMouseButtonGLUT(iButtonCode, iButtonState, iMousePosX, iMousePosY)
    ) {
        if(!abMouseButtonStates[iButtonCode]) {
            if(iButtonState == GLUT_DOWN) {
                iMouseButtonDownCount++;
            }
        } else if(iButtonState == GLUT_UP) {
            iMouseButtonDownCount--;
        }
        abMouseButtonStates[iButtonCode] = (iButtonState == GLUT_DOWN);
        g_pEngine->onWindowMouseButton(iWindowId, iButtonCode, iButtonState, iMousePosX, iMousePosY);
    }
    iCurrMousePosX = iMousePosX;
    iCurrMousePosY = iMousePosY;
}
void onMotion(int iMousePosX, int iMousePosY) {
    int iWindowId = glutGetWindow();
    
    //printf("onMotion: %d, %d\n", iMousePosX, iMousePosY);
    
    int bWillLockMouse = g_pEngine->getLockMouse();
    
    if(bWillLockMouse) {
        if(!bLockMouse) {
            bLockMouse = 1;
            bShowCursor = 0;
            //glutSetCursor(GLUT_CURSOR_NONE);
            
            //glutWarpPointer(iWindowSizeW / 2, iWindowSizeH / 2);
        }
    } else {
        if(bLockMouse) {
            bLockMouse = 0;
            bShowCursor = 1;
            glutSetCursor(eCurrentCursor);
        }
    }
    int iDeltaX, iDeltaY;
    if(bLockMouse) {
        iDeltaX = iMousePosX-iCurrMousePosX;
        iDeltaY = iMousePosY-iCurrMousePosY;
        /*g_pEngine->onWindowLook(
            iWindowId,
            ((float)iDeltaX)*fMouseSensitivity,
            ((float)iDeltaY)*fMouseSensitivity
        );*/
    } else if(
            !g_pEngine->getShowUI()
        ||  (iMouseButtonDownCount > 0)
        ||  !TwEventMouseMotionGLUT(iMousePosX, iMousePosY)
    ) {
        g_pEngine->onWindowMouseMotion(iWindowId, iMousePosX, iMousePosY);
    }
    if(bLockMouse && (iDeltaX != 0 || iDeltaY != 0)) {
        //glutWarpPointer(iWindowSizeW / 2, iWindowSizeH / 2);
    }
    iCurrMousePosX = iMousePosX;
    iCurrMousePosY = iMousePosY;
}
void onPassiveMotion(int iMousePosX, int iMousePosY) {
    onMotion(iMousePosX, iMousePosY);
}
void onEntry(int iMousePresenceState) {
    int iWindowId = glutGetWindow();
    
    g_pEngine->onWindowMouseEntry(iWindowId, iMousePresenceState);
}

/*void onMenuStatus(int iMenuStatus, int iMousePosX, int iMousePosY) {
    int iWindowId = glutGetWindow();
    
    g_pEngine->onMenuStatus(iWindowId, iMenuStatus, iMousePosX, iMousePosY);
}*/

void onIdle() {
    g_pEngine->onIdle();
}

void initTweakBar() {
    g_pTweakBar = TwNewBar("CG");
    TwDefine(
        " CG label='CG' refresh=0.5 position='16 16' size='250 868' valueswidth=75"
    );
    
    /*g_pEngine->getRenderer()->m_fRotation = 0;
    TwAddVarRW(g_pTweakBar,
        "Rotation", TW_TYPE_FLOAT, &g_pEngine->getRenderer()->m_fRotation,
        "min=0 max=360 step=1 keyIncr=d keyDecr=a help='Rotate the scene.'"
    );*/
}

void ChangeCursor(int eCursor) {
    eCurrentCursor = eCursor;
    if(bShowCursor) {
        glutSetCursor(eCursor);
    }
}

int GetClock() {
    return glutGet(GLUT_ELAPSED_TIME);
}

int main(int iArgCount, char **aszArgList) {
    
    glutInit(&iArgCount, aszArgList);
    
    glutInitDisplayMode(
            0
        |   GLUT_RGB
        |   GLUT_DOUBLE
        |   GLUT_DEPTH
        |   GLUT_MULTISAMPLE
      //|   GLUT_ALPHA
        |   GLUT_STENCIL
    );
    glutInitWindowSize(iWindowSizeW, iWindowSizeH);
    //glutInitWindowPosition(100, 100);
    glutCreateWindow("Cactus Game");
    
    glutDisplayFunc        (onDisplay        );
    glutOverlayDisplayFunc (onOverlayDisplay );
    glutReshapeFunc        (onReshape        );
    glutKeyboardFunc       (onKeyboardDown   );
    glutKeyboardUpFunc     (onKeyboardUp     );
    glutSpecialFunc        (onSpecialDown    );
    glutSpecialUpFunc      (onSpecialUp      );
    glutMouseFunc          (onMouse          );
    glutMotionFunc         (onMotion         );
    glutPassiveMotionFunc  (onPassiveMotion  );
    glutVisibilityFunc     (onVisibility     );
    glutEntryFunc          (onEntry          );
    //glutSpaceballMotionFunc(onSpaceballMotion);
    //glutSpaceballRotateFunc(onSpaceballRotate);
    //glutSpaceballButtonFunc(onSpaceballButton);
    //glutButtonBoxFunc      (onButtonBox      );
    //glutDialsFunc          (onDials          );
    //glutTabletMotionFunc   (onTabletMotion   );
    //glutTabletButtonFunc   (onTabletButton   );
    //glutMenuStatusFunc     (onMenuStatus     );
    glutIdleFunc           (onIdle           );
    
    glutIgnoreKeyRepeat(1);
    
    TwInit(TW_OPENGL, NULL);
    TwWindowSize(iWindowSizeW, iWindowSizeH);
    //TwDefine("Blah position='200 40'");
    initTweakBar();
    
    g_pEngine = new CEngine(g_pTweakBar);
    g_pEngine->setChangeCursorCallback(&ChangeCursor);
    g_pEngine->setGetClockCallback(&GetClock);
    #ifdef DEBUG
        glutMainLoop();
    #else
        try {
            glutMainLoop();
        } 
        catch(const char* e) {
            std::cerr << e << std::endl;
        }
    #endif
    
    printf("Goodbye!\n");
    
    return 0;
}
