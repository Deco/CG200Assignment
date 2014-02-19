#include <cmath>
#include <cstdio>
#include <ctime>
#include <cassert>
#include <cstring>
#include <GL/freeglut.h>
#include "matrix.h"
#include "model.h"

class CEngine;
#include "CEngine.h"

#include "CWorld.h"
#include "CPlayer.h"
#include "CProp.h"
#include "CPropBuilding.h"
#include "CHeightField.h"

CEntity *meow;

CEngine::CEngine(
    TwBar *pTweakBar
) : m_iClock(0),
    m_fTime(0.0),
    m_fTimeAccumulation(0.0),
    m_eCurrentCursor(-9999),
    m_pTweakBar(pTweakBar),
    m_iNextEntityID(1),
    m_clEntities(),
    //m_clAddedEntities(),
    m_bRunning(true),
    m_iPrevFrameTime(),
    m_iSecondsPerFrameHistoryIndex(0),
    m_fSecondsPerFrameHistorySum(0),
    m_clOptions(),
    m_clWatchValues(),
    m_clButtons(),
    m_bAnimationStarted(false),
    m_bAnimationPaused(false)
{
    
    setWatchOutput("FPS", (double)0.0, "group='Main'");
    
    m_pRenderer = new CRenderer(this);
    m_pWorld = new CWorld(this);
    m_pPlayer = new CPlayer(this);
    
    m_pWorld->initWorld();
    
    m_fTimeStep= 1.0/60.0;
    m_fTimeScale = 1.0;
    
    srand(time(0));
    
    m_afSecondsPerFrameHistory = new double[m_iSecondsPerFrameHistoryCount];
    
    // ((*(new CHeightField(this, 10, 10)))
        // .setLabel("HeightField")
        // .setPosition(vec3(0, 0, 0))
        // .setRotation(quat())
        // .setExtents(vec3(-10, -10, -10), vec3(10, 10, 10))
    // );
    
    // ((*(meow = new CProp(this, CModel::makeCube())))
        // .setPosition(vec3(0, 0, 0))
        // .setRotation(quat())
    // );
    // ((*(meow = new CProp(this, CModel::loadModel("natla_car.obj", 1, false))))
        // .setLabel("meow")
        // .setPosition(vec3(0, 0, 0))
        // .setRotation(quat())
    // );
    ((*(meow = new CEntity(this)))
        .setPosition(vec3(0, 0, 0))
        .setRotation(quat())
    );
    // ((*(meow = new CPropBuilding(this, 5, 5, CWorld::EDirection::UP)))
        // .setPosition(vec3(0, 0, 0))
        // .setRotation(quat())
    // );
    
    for(int i = 0; i < 1; i++) {
        // ((*(new CProp(this, CModel::loadModel("statue.obj", 1, true))))
        ((*(new CProp(this, CModel::makeCube("img_test.dds", vec3(1, 2, 1)))))
            .setLabel("box")
            .setPosition(vec3(-10.0+number_rand()*20.0, 3.0, -10.0+number_rand()*20.0))
            .setRotation(quat(number_rand()*M_PI*2, vec3(0, 1, 0)))
        );
        // ((*(new CEntity(this)))
            // .setPosition(vec3(-10.0+number_rand()*20.0, 1.0+number_rand()*2.0, -10.0+number_rand()*20.0))
            // .setRotation(quat::random())
        // );
    }
    
    memset(m_afSecondsPerFrameHistory, 0, sizeof(double)*m_iSecondsPerFrameHistoryCount);
    
}
CEngine::~CEngine() {
    delete[] m_afSecondsPerFrameHistory;
}

bool CEngine::addEntity(CEntity *pEntity) {
    bool bSuccess = m_clEntities.insert(
        std::pair<EntityID, CEntity*>(
            m_iNextEntityID++,
            pEntity
        )
    ).second;
    //printf("Entity added: %s (%p)\n", pEntity->getLabel().c_str(), pEntity);
    return bSuccess;
}
bool CEngine::removeEntity(CEntity *pEntity) {
    pEntity->remove();
    return true;
}

bool CEngine::getShowUI() const {
    return 1;
}
bool CEngine::getLockMouse() const {
    return 0;
}

void CEngine::onThink(
    double fTime, double fDeltaTime
) {
    int starttime = getClock();
    
    //m_pRenderer->m_stFrustum.setCoordinates(vHitPos+vec3(0, 50, 0), vec3(0, -1, 0), vec3(1, 0, 0));
    
    //meow->setRotation(quat(fTime, (quat(fTime*3, vec3(0, 1, 0)).vecmul(vec3(1, 1, 0))).unit()));
    meow->setRotation(quat(fTime, vec3(0, 1, 0).unit()));
    
    for(auto itEntity = m_clEntities.begin(); itEntity != m_clEntities.end(); itEntity++) {
        CEntity *pEntity = itEntity->second;
        pEntity->m_vHighlightColor = vec3(0, 0, 0);
        
        bool bShowFrustum = false;
        getOption("Show Frustum", &bShowFrustum, bShowFrustum, "group='LOD'");
        if(bShowFrustum) {
            box_aabb stTestBox;
            pEntity->getWorldExtents(stTestBox.min, stTestBox.max);
            frustum::EResult result =  m_pRenderer->m_stFrustum.intersect(stTestBox);
            if(result == frustum::EResult::INSIDE) {
                pEntity->m_vHighlightColor = vec3(0, 1, 0);
            } else if(result == frustum::EResult::INTERSECT) {
                pEntity->m_vHighlightColor = vec3(1, 1, 0);
            }
        }
    }
    
    bool anyRemoved = false;
    size_t size = m_clEntities.size();
    for(auto itEntity = m_clEntities.begin(); itEntity != m_clEntities.end(); itEntity++) {
        CEntity *pEntity = itEntity->second;
        //printf("Entity: %s (%p)\n", pEntity->getLabel().c_str(), pEntity);
        if(!pEntity->isValid()) {
            //printf("Entity removed: %s (%p)\n", pEntity->getLabel().c_str(), pEntity);
            delete pEntity;
            anyRemoved = true;
            m_clEntities.erase(itEntity);
        } else {
            pEntity->onThink(fTime, fDeltaTime);
        }
    }
    if(anyRemoved) {
        //printf("%zu -> %zu\n", size, m_clEntities.size());
    }
    setWatchOutput("onThinkTime", getClock()-starttime, "group='Profile'");
}

void CEngine::onIdle() {
    
}

int CEngine::m_iSecondsPerFrameHistoryCount = 30;

bool CEngine::onWindowDisplay(
    int iWindowId
) {
    int eCursor = m_pPlayer->getInputCursor();
    if(eCursor != m_eCurrentCursor) {
        m_cbChangeCursor(eCursor);
        m_eCurrentCursor = eCursor;
    }
    
    // http://gafferongames.com/game-physics/fix-your-timestep/
    int iClock = m_cbGetClock();
    int iClockDelta = iClock-m_iClock;
    m_iClock = iClock;
    
    double fClockDeltaSeconds = (double)iClockDelta/1000.0;
    
    setWatchOutput("Animation Started", m_bAnimationStarted, "group='Main'");
    setWatchOutput("Animation Paused" , m_bAnimationPaused , "group='Main'");
    
    if(getButtonState("Animation - Start/Reset", "group='Main' key=a")) {
        m_bAnimationStarted = true;
        m_pWorld->initWorld();
    }
    if(getButtonState("Animation - Pause", "group='Main' key=t")) {
        m_bAnimationPaused = true;
    }
    if(getButtonState("Animation - Resume", "group='Main' key=c")) {
        m_bAnimationPaused = false;
    }
    
    getOption("Animation Rate", &m_fTimeScale, m_fTimeScale, "group='Main' min=0.25 max=10.0 step=0.25 keyincr=f keydecr=s");
    
    if(getButtonState("Animation - Reset Rate", "group='Main'")) {
        setOption("Animation Rate", 1.0);
        m_fTimeScale = 1.0;
    }
    if(!m_bAnimationStarted || m_bAnimationPaused) {
        m_fTimeScale = 0.0;
    }
    
    fClockDeltaSeconds *= m_fTimeScale;
    
    double fClockDeltaSecondsMax = 0.18;
    if(fClockDeltaSeconds > fClockDeltaSecondsMax) {
        fClockDeltaSeconds = fClockDeltaSecondsMax; // spiral of death!
    }
    
    m_fTimeAccumulation += fClockDeltaSeconds;
    
    while(m_fTimeAccumulation >= m_fTimeStep) {
        onThink(m_fTime, m_fTimeStep);
        m_fTimeAccumulation -= m_fTimeStep;
        m_fTime += m_fTimeStep;
    }
    
    int starttime = getClock();
    m_pRenderer->onDisplay(fClockDeltaSeconds, (double)iClockDelta/1000.0);
    setWatchOutput("onDisplay", getClock()-starttime, "group='Profile'");
    
    int iFrameTime = m_cbGetClock();
    int iFrameDelta = iFrameTime-m_iPrevFrameTime;
    m_iPrevFrameTime = iFrameTime;
    
    double fFrameDeltaSeconds = (double)iFrameDelta/1000.0;
    
    m_fSecondsPerFrameHistorySum -= m_afSecondsPerFrameHistory[m_iSecondsPerFrameHistoryIndex];
    m_fSecondsPerFrameHistorySum += fFrameDeltaSeconds;
    m_afSecondsPerFrameHistory[m_iSecondsPerFrameHistoryIndex] = fFrameDeltaSeconds;
    m_iSecondsPerFrameHistoryIndex += 1;
    if(m_iSecondsPerFrameHistoryIndex == m_iSecondsPerFrameHistoryCount) {
        m_iSecondsPerFrameHistoryIndex = 0;
    }
    
    double fAverageSecondsPerFrame = m_fSecondsPerFrameHistorySum/m_iSecondsPerFrameHistoryCount;
    
    setWatchOutput("FPS", 1/fAverageSecondsPerFrame, "group='Main'");
    
    vec3 vMousePos = getPlayer()->getPointerPosition();
    vec3 vWorldPos, vWorldDir;
    getRenderer()->screenToWorld(vMousePos, vWorldPos, vWorldDir);
    plane planeFloor(vec3(0, 1, 0), 0.0);
    vec3 vHitPos;
    if(planeFloor.intersect(line(vWorldPos, vWorldPos+vWorldDir*10000.0), vHitPos)) {
        meow->setPosition(vHitPos+vec3(0, 11, 0));
    }
    
    
    return m_bRunning;
}

void CEngine::onWindowDisplayOverlay(
    int iWindowId, int iLayerId
) {
    
}

void CEngine::onWindowResize(
    int iWindowId,
    int iSizeW, int iSizeH
) {
    m_pRenderer->onResize(iSizeW, iSizeH);
}

void CEngine::onWindowMouseButton(
    int iWindowId,
    int iButtonCode, int iButtonState,
    int iMousePosX, int iMousePosY
) {
    m_pPlayer->onInputMotionEvent(CPlayer::EInputKind::POINTER, (double)iMousePosX, (double)iMousePosY);
    
    int iEventButtonCode = (
            iButtonCode == GLUT_LEFT_BUTTON    ? CPlayer::EInputPointerButton::LEFTMOUSE
        :   iButtonCode == GLUT_RIGHT_BUTTON   ? CPlayer::EInputPointerButton::RIGHTMOUSE
        :   iButtonCode == GLUT_MIDDLE_BUTTON  ? CPlayer::EInputPointerButton::MIDDLEMOUSE
        :   iButtonCode == 3 /*             */ ? CPlayer::EInputPointerButton::WHEELUP
        :   iButtonCode == 4 /*     not     */ ? CPlayer::EInputPointerButton::WHEELDOWN
        :   iButtonCode == 8 /* documented! */ ? CPlayer::EInputPointerButton::FORWARD
        :   iButtonCode == 7 /*             */ ? CPlayer::EInputPointerButton::BACK
        :                                        CPlayer::EInputPointerButton::UNKNOWN
    );
    if(iEventButtonCode != CPlayer::EInputPointerButton::UNKNOWN) {
        int bEventButtonState = (iButtonState == GLUT_DOWN);
        m_pPlayer->onInputButtonEvent(CPlayer::EInputKind::POINTER, iEventButtonCode, bEventButtonState);
    }
}

void CEngine::onWindowMouseMotion(
    int iWindowId,
    int iMousePosX, int iMousePosY
) {
    m_pPlayer->onInputMotionEvent(CPlayer::EInputKind::POINTER, (double)iMousePosX, (double)iMousePosY);
}
/*
void CEngine::onWindowLook(
    int iWindowId,
    float fDeltaX, float fDeltaY
) {
    if(pEngine->bMouseLooking) {
        printf("dx = %f, dy = %f\n", fDeltaX, fDeltaY);
        pEngine->pRenderer->fRotation += fDeltaX;
    }
}*/

void CEngine::onWindowKey(
    int iWindowId,
    unsigned char chKeyCode, int iKeyState,
    int iMousePosX, int iMousePosY
) {
    m_pPlayer->onInputMotionEvent(CPlayer::EInputKind::POINTER, (double)iMousePosX, (double)iMousePosY);
    
    int iEventButtonCode = chKeyCode;
    int bEventButtonState = (iKeyState == GLUT_DOWN);
    m_pPlayer->onInputButtonEvent(CPlayer::EInputKind::KEY, iEventButtonCode, bEventButtonState);
}

void CEngine::onWindowKeySpecial(
    int iWindowId,
    int iKeyCode, int iKeyState,
    int iMousePosX, int iMousePosY
) {
    m_pPlayer->onInputMotionEvent(CPlayer::EInputKind::POINTER, (double)iMousePosX, (double)iMousePosY);
    
    int iEventButtonCode = iKeyCode;
    int bEventButtonState = (iKeyState == GLUT_DOWN);
    m_pPlayer->onInputButtonEvent(CPlayer::EInputKind::KEY, iEventButtonCode, bEventButtonState, true);
}

void CEngine::onWindowVisibility(
    int iWindowId, int iVisibilityState
) {
    
}
void CEngine::onWindowMouseEntry(
    int iWindowId, int iMousePresenceState
) {
    
}

void TW_CALL CEngine__buttonCallback(bool *state) {
    *state = true;
}

bool CEngine::getButtonState(
    const std::string sName,
    const std::string &sTwParams
) {
    auto it = m_clButtons.find(sName);
    if(it != m_clButtons.end()) {
        bool state = it->second;
        it->second = false;
        return state;
    } else {
        auto res = m_clButtons.insert(
            std::pair<std::string, bool>(
                sName,
                false
            )
        );
        
        TwAddButton(
            m_pTweakBar, sName.c_str(),
            (void (*)(void*))&CEngine__buttonCallback, &(res.first->second),
            sTwParams.c_str()
        );
    }
    return false;
}
