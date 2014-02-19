#include <cmath>
#include <cctype>

class CPlayer;
#include "CPlayer.h"

#include "CEngine.h"
#include "CWorld.h"


CPlayer::CPlayer(
    CEngine *pEngine
) : CEntity(pEngine),
    m_eViewMode(EViewMode::TOPDOWN),
    m_iPointerUseCount(0),
    m_abPointerButtonStates(),
    m_abButtonStates(),
    m_abSpecialButtonStates(),
    m_bViewDragActive(false),
    m_iTopDownViewZoomLevel(54),
    m_bAnimationState(false),
    m_pEntityToFollow(NULL)
{
    m_sLabel = "Player";
    m_vTopDownViewPosition = vec3(0.0, 0.0, -250.0);
    
    m_pEngine->watchVariable("Zoom", (int *)&m_iTopDownViewZoomLevel, "group='Camera' min=0 max=100 keyincr=shift+z keydecr=z", false);
}
CPlayer::~CPlayer() {
    
}

void CPlayer::getCameraData(
    SCameraData &pData
) {
    /*pData->vPosition = pos;
    pData->qRotation = quat();
    pData->fFieldOfView = 65;//30+20*sin(m_pEngine->getTime());
    return;*/
    if(m_pEngine->getButtonState("Top Down View", "group='Camera' key=2")) {
        m_eViewMode = EViewMode::TOPDOWN;
        m_pEntityToFollow = NULL;
    }
    if(m_pEngine->getButtonState("Car View", "group='Camera' key=1")) {
        m_eViewMode = EViewMode::FPS;
        
        CEntity *pClosestEntity = NULL;
        double fClosestDistance = 9999;
        auto clEntities = m_pEngine->getEntities();
        for(auto itEntity = clEntities.begin(); itEntity != clEntities.end(); itEntity++) {
            CEntity *pEntity = itEntity->second;
            if(!pEntity->isValid()) continue;
            
            if(pEntity == m_pEntityToFollow) {
                pClosestEntity = m_pEntityToFollow;
                break;
            }
            if(pEntity->getLabel() == "car") {
                if((pEntity->getPosition()-m_vTopDownViewPosition).length() < fClosestDistance) {
                    pClosestEntity = pEntity;
                    fClosestDistance = (pEntity->getPosition()-m_vTopDownViewPosition).length();
                }
            }
        }
        if(pClosestEntity != NULL) {
            m_pEntityToFollow = pClosestEntity;
        }
    }
    if(m_eViewMode == EViewMode::FPS) {
        CEntity *pClosestEntity = NULL;
        double fClosestDistance = 9999;
        auto clEntities = m_pEngine->getEntities();
        for(auto itEntity = clEntities.begin(); itEntity != clEntities.end(); itEntity++) {
            CEntity *pEntity = itEntity->second;
            if(!pEntity->isValid()) continue;
            
            if(pEntity == m_pEntityToFollow) {
                pClosestEntity = m_pEntityToFollow;
                break;
            }
            if(pEntity->getLabel() == "car") {
                if((pEntity->getPosition()-m_vTopDownViewPosition).length() < fClosestDistance) {
                    pClosestEntity = pEntity;
                    fClosestDistance = (pEntity->getPosition()-m_vTopDownViewPosition).length();
                }
            }
        }
        if(pClosestEntity != NULL) {
            m_pEntityToFollow = pClosestEntity;
            pData.vPosition = -pClosestEntity->getPosition()-vec3(0, 3, 0);
            vec3 diff = pClosestEntity->getRotation().forward();
            double ang = atan2(diff.x, diff.z);
            pData.qRotation = quat::fromEuler(0, M_DEG(-ang), 0);
            pData.fFieldOfView = 82;//30+20*sin(m_pEngine->getTime());
        } else {
            m_eViewMode = EViewMode::TOPDOWN;
        }
    }
    
    if(m_eViewMode == EViewMode::TOPDOWN) {
        m_vTopDownViewPosition.y = -(18.0+(m_iTopDownViewZoomLevel)*5);
        pData.vPosition = m_vTopDownViewPosition;
        double fTilt = 60;
        m_pEngine->getOption("Tilt", &fTilt, fTilt, "group='Camera'");
        pData.qRotation = quat(M_RAD(-fTilt), vec3(1, 0, 0));
        pData.fFieldOfView = 65;//30+20*sin(m_pEngine->getTime());
        m_pEngine->getOption("FOV", &pData.fFieldOfView, pData.fFieldOfView, "group='Camera'");
    }
}

double blah = 0.0;
double blahdelay = 0.02;

void CPlayer::_think(
    double fTime, double fDeltaTime
) {
    blah -= fDeltaTime;
    //m_pEngine->setWatchOutput("Gen - blah", blah, "group='World'");
    if(blah <= 0) {
        bool bStep = false;
        double fSpeed = 1.0;
        
        bool bQuickWorldGen = true;
        m_pEngine->getOption("Gen - Quick", &bQuickWorldGen, bQuickWorldGen, "group='World'");
        if(bQuickWorldGen) fSpeed = 1.0;
        
        bStep = (bQuickWorldGen || m_abButtonStates[' ']);
        if(bStep) {
            double fSignifigance = 1.0;
            int count = 0;
            do {
                if(m_pEngine->getWorld()->initWorldStep(fSignifigance)) {
                    //m_pEngine->getWorld()->initWorld();
                } else {
                    blah = blahdelay*fSignifigance/fSpeed;
                }
                count++;
            } while(fSignifigance < 0.08 && count < 1);
        }
    }
    
    bool bInstantWorldGen = false;
    m_pEngine->getOption("Gen - Instant", &bInstantWorldGen, bInstantWorldGen, "group='World' key=w");
    double fSignifigance = 1.0;
    int count = 0;
    while(bInstantWorldGen && count < 20 && !m_pEngine->getWorld()->initWorldStep(fSignifigance)) {
        // m_pEngine->getWorld()->initWorld();
        count++;
    }
    
    CPlayer::SCameraData stCameraData;
    getCameraData(stCameraData);
    vec3 vMousePos = getPointerPosition();
    vec3 vWorldPos, vWorldDir;
    m_pEngine->getRenderer()->screenToWorld(vMousePos, vWorldPos, vWorldDir);
    
    box_aabb stTestBox;
    CEntity *pClosestEntity = NULL;
    double fClosestDistance = 9999;
    
    auto clEntities = m_pEngine->getEntities();
    for(auto itEntity = clEntities.begin(); itEntity != clEntities.end(); itEntity++) {
        CEntity *pEntity = itEntity->second;
        if(!pEntity->isValid()) continue;
        
        pEntity->getWorldExtents(stTestBox.min, stTestBox.max);
        //stTestBox.min.print();
        //stTestBox.max.print();
        double t, tmin, tmax;
        if(stTestBox.intersectRay(vWorldPos, vWorldDir, t, tmin, tmax)) {
            if(!number_eq(tmax, t) && t < fClosestDistance && pEntity != m_pEntityToFollow) {
                fClosestDistance = t;
                pClosestEntity = pEntity;
            }
            //printf("%s: %d %f %f %f\n", pEntity->getLabel().c_str(), 1, t, tmin, tmax);
        } else {
            //printf("%s: %d %f %f %f\n", pEntity->getLabel().c_str(), 0, t, tmin, tmax);
        }
    }
    if(pClosestEntity != NULL) {
        m_pEngine->setWatchOutput("Mouse Entity", pClosestEntity->getLabel(), "group='Camera'");
        m_pEngine->setWatchOutput("Mouse Entity LOD", pClosestEntity->m_fLODDebug, "group='Camera'");
        m_pEngine->setWatchOutput("Mouse Entity Alpha", pClosestEntity->m_fOpacity, "group='Camera'");
        m_pEngine->setWatchOutput("Mouse Entity Debug", pClosestEntity->dbgtext, "group='Camera'");
        
        pClosestEntity->m_vHighlightColor = vec3(1, 0, 0);
    }
}
void CPlayer::_render(
    CRenderer *pRenderer,
    double fDeltaTime, double fDeltaClock,
    CEntity::ERenderMode eMode,
    int iDetailLevel
) {
    if(eMode == CEntity::ERenderMode::OVERLAY) {
        vec3 vViewKeyDragDirection;
        if(m_abSpecialButtonStates[100]) { vViewKeyDragDirection.x = vViewKeyDragDirection.x-1; }
        if(m_abSpecialButtonStates[102]) { vViewKeyDragDirection.x = vViewKeyDragDirection.x+1; }
        if(m_abSpecialButtonStates[101]) { vViewKeyDragDirection.z = vViewKeyDragDirection.z-1; }
        if(m_abSpecialButtonStates[103]) { vViewKeyDragDirection.z = vViewKeyDragDirection.z+1; }
        
        if(vViewKeyDragDirection != vec3::zero) {
            vViewKeyDragDirection = vViewKeyDragDirection.unit();
            
            double fKeySpeed = 55;
            m_pEngine->getOption("Key Move Speed", &fKeySpeed, fKeySpeed, "group='Camera'");
            fKeySpeed *= pow(1.03, m_iTopDownViewZoomLevel+8);
            m_vTopDownViewPosition.x -= vViewKeyDragDirection.x*fKeySpeed*fDeltaClock;
            m_vTopDownViewPosition.z -= vViewKeyDragDirection.z*fKeySpeed*fDeltaClock;
        }
        if(m_eViewMode == EViewMode::TOPDOWN) {
            if(m_bViewDragActive) {
                vec3 vCurrentPos = getPointerPosition();
                double fDragSpeed = 0.33;
                m_pEngine->getOption("Drag Move Speed", &fDragSpeed, fDragSpeed, "group='Camera'");
                fDragSpeed *= pow(1.03, m_iTopDownViewZoomLevel+8);
                m_vTopDownViewPosition.x -= (vCurrentPos.x-m_vViewDrawClickPos.x)*fDragSpeed*fDeltaClock;
                m_vTopDownViewPosition.z -= (vCurrentPos.y-m_vViewDrawClickPos.y)*fDragSpeed*fDeltaClock;
            }
        }
        
        if(m_abButtonStates['x']) { m_pEngine->getRenderer()->m_fRotationX += 10*fDeltaClock; }
        if(m_abButtonStates['X']) { m_pEngine->getRenderer()->m_fRotationX -= 10*fDeltaClock; }
        if(m_abButtonStates['y']) { m_pEngine->getRenderer()->m_fRotationY += 10*fDeltaClock; }
        if(m_abButtonStates['Y']) { m_pEngine->getRenderer()->m_fRotationY -= 10*fDeltaClock; }
        
        pRenderer->text(     "Move Camera: <Arrows Keys>");
        pRenderer->text(       "Start animation: <A>/<a>");
        pRenderer->text( "Animate faster/slower: <F>/<S>");
        pRenderer->text("Pause/resume animation: <T>/<C>");
        pRenderer->text(   "Smooth/Flat shading: <P>/<p>");
        pRenderer->text(           "Zoom in/out: <Z>/<z>");
        pRenderer->text(         "Rotate X axis: <X>/<x>");
        pRenderer->text(         "Rotate Y axis: <Y>/<y>");
        pRenderer->text(         "Rotate Y axis: <Y>/<y>");
        
        std::stringstream ss;
        int numkeys = 0;
        for(int iKeyCode = 0; iKeyCode < sizeof(m_abButtonStates)/sizeof(*m_abButtonStates); iKeyCode++) {
            if(m_abButtonStates[iKeyCode]) {
                if(numkeys > 0) {
                    ss << ", ";
                }
                if(isprint(iKeyCode) && (char)iKeyCode != ' ') {
                    ss << (char)iKeyCode;
                } else {
                    ss << "K" << iKeyCode;
                }
                numkeys++;
            }
        }
        for(int iKeyCode = 0; iKeyCode < sizeof(m_abSpecialButtonStates)/sizeof(*m_abButtonStates); iKeyCode++) {
            if(m_abSpecialButtonStates[iKeyCode]) {
                if(numkeys > 0) {
                    ss << ", ";
                }
                ss << "S" << iKeyCode;
                numkeys++;
            }
        }
        if(m_abPointerButtonStates[UNKNOWN    ]) { if(numkeys > 0) { ss << ", "; } ss << "Unknown"     ; numkeys++; }
        if(m_abPointerButtonStates[LEFTMOUSE  ]) { if(numkeys > 0) { ss << ", "; } ss << "LeftMouse"   ; numkeys++; }
        if(m_abPointerButtonStates[RIGHTMOUSE ]) { if(numkeys > 0) { ss << ", "; } ss << "RightMouse"  ; numkeys++; }
        if(m_abPointerButtonStates[MIDDLEMOUSE]) { if(numkeys > 0) { ss << ", "; } ss << "MiddleMouse" ; numkeys++; }
        if(m_abPointerButtonStates[WHEELUP    ]) { if(numkeys > 0) { ss << ", "; } ss << "WheelUp"     ; numkeys++; }
        if(m_abPointerButtonStates[WHEELDOWN  ]) { if(numkeys > 0) { ss << ", "; } ss << "Wheeldown"   ; numkeys++; }
        if(m_abPointerButtonStates[FORWARD    ]) { if(numkeys > 0) { ss << ", "; } ss << "MouseForward"; numkeys++; }
        if(m_abPointerButtonStates[BACK       ]) { if(numkeys > 0) { ss << ", "; } ss << "MouseBack"   ; numkeys++; }
        
        std::string keystr = ss.str();
        pRenderer->text(keystr.c_str());
        
        return;
    }
    
    if(eMode != CEntity::ERenderMode::NORMAL) return;
    
}

void CPlayer::onInputButtonEvent(
    EInputKind eKind,
    int eButtonCode, bool bButtonState,
    bool bIsSpecial
) {
    //printf("INPUT: %d %d %d %d\n", eKind, eButtonCode, bButtonState, bIsSpecial);
    if(eKind == EInputKind::POINTER) {
        m_abPointerButtonStates[eButtonCode] = bButtonState;
        m_iPointerUseCount += (bButtonState ? 1 : -1);
        if(m_iPointerUseCount > 1) return; // no multi-mouse button cases
    } else if(eKind == EInputKind::KEY) {
        if(!bIsSpecial) {
            m_abButtonStates[eButtonCode] = bButtonState;
        } else {
            m_abSpecialButtonStates[eButtonCode] = bButtonState;
        }
    }
    if(eKind == EInputKind::KEY) {
        if(eButtonCode == 'q') {
            m_pEngine->quit();
        }
    }
    if(eKind == EInputKind::POINTER) {
        if(eButtonCode == EInputPointerButton::LEFTMOUSE) {
            if(bButtonState) {
                if(true || m_eViewMode == EViewMode::TOPDOWN) {
                    CPlayer::SCameraData stCameraData;
                    getCameraData(stCameraData);
                    vec3 vMousePos = getPointerPosition();
                    vec3 vWorldPos, vWorldDir;
                    m_pEngine->getRenderer()->screenToWorld(vMousePos, vWorldPos, vWorldDir);
                    
                    box_aabb stTestBox;
                    CEntity *pClosestEntity = NULL;
                    double fClosestDistance = 9999;
                    
                    auto clEntities = m_pEngine->getEntities();
                    for(auto itEntity = clEntities.begin(); itEntity != clEntities.end(); itEntity++) {
                        CEntity *pEntity = itEntity->second;
                        if(!pEntity->isValid()) continue;
                        
                        pEntity->getWorldExtents(stTestBox.min, stTestBox.max);
                        double t, tmin, tmax;
                        if(stTestBox.intersectRay(vWorldPos, vWorldDir, t, tmin, tmax)) {
                            if(!number_eq(tmax, t) && t < fClosestDistance && pEntity != m_pEntityToFollow) {
                                fClosestDistance = t;
                                pClosestEntity = pEntity;
                            }
                        }
                    }
                    m_pEntityToFollow = pClosestEntity;
                    if(pClosestEntity != NULL) {
                        m_eViewMode = EViewMode::FPS;
                    } else {
                        m_eViewMode = EViewMode::TOPDOWN;
                    }
                }
            }
        }
    }
    if(m_eViewMode == EViewMode::TOPDOWN) {
        if(eKind == EInputKind::POINTER) {
            if(eButtonCode == EInputPointerButton::RIGHTMOUSE) {
                if(bButtonState && !m_bViewDragActive) {
                    m_bViewDragActive = true;
                    m_vViewDrawClickPos = getPointerPosition();
                    
                    /*SCameraData stCameraData;
                    getCameraData(stCameraData);
                    vec3 vMousePos = getPointerPosition();
                    vec3 vWorldPos, vWorldDir;
                    m_pEngine->getRenderer()->screenToWorld(vMousePos, vWorldPos, vWorldDir);
                    plane planeFloor(vec3(0, 1, 0), 0.0);
                    m_vViewDragStartClickPos = planeFloor.intersectLine(vWorldPos, vWorldPos+vWorldDir*10000.0);
                    m_vViewDragStartViewPos = m_vTopDownViewPosition;*/
                } else if(m_bViewDragActive) {
                    m_bViewDragActive = false;
                }
            } else if(eButtonCode == EInputPointerButton::WHEELUP) {
                m_iTopDownViewZoomLevel = std::max(0, m_iTopDownViewZoomLevel-1);
            } else if(eButtonCode == EInputPointerButton::WHEELDOWN) {
                m_iTopDownViewZoomLevel = m_iTopDownViewZoomLevel+1;
            }
        }
    }
    
    if(eKind == EInputKind::KEY) {
        if(bIsSpecial) {
            if(eButtonCode == ' ' && bButtonState) {
                double fSignifigance = 1.0;
                m_pEngine->getWorld()->initWorldStep(fSignifigance);
                blah = blahdelay*fSignifigance+0.5;
            } else if(eButtonCode == 'r' && bButtonState) {
                m_pEngine->getWorld()->initWorld();
            } /* else if(eButtonCode == 'c' && bButtonState) {
                m_vTopDownViewPosition.x = 0;
                m_vTopDownViewPosition.z = 0;
            } else if(eButtonCode == 'f' && bButtonState) {
                printf("Entities rendered\n");
                for(auto itEntity = m_pEngine->getEntities().begin(); itEntity != m_pEngine->getEntities().end(); itEntity++) {
                    CEntity *pEntity = itEntity->second;
                    
                    box_aabb stTestBox;
                    pEntity->getWorldExtents(stTestBox.min, stTestBox.max);
                    frustum::EResult result =  m_pEngine->getRenderer()->m_stFrustum.intersect(stTestBox);
                    //if(result != frustum::EResult::OUTSIDE) {
                    if(pEntity->m_bWasRendered) {
                        pEntity->m_vHighlightColor = vec3(0, 1, 0);
                        printf(" - %20s (%2d, %14lld)\n", pEntity->getLabel().c_str(), pEntity->m_iDebugPassCount, pEntity->m_iRenderTimeMs);
                        pEntity->m_iRenderTimeMs = 0;
                    }
                }
            } */
        } else {
        }
    }
}

int CPlayer::getInputCursor() {
    if(m_bViewDragActive) {
        return GLUT_CURSOR_CROSSHAIR;
    }
    return GLUT_CURSOR_INHERIT;
}

void CPlayer::onInputMotionEvent(
    EInputKind eKind,
    double fPosX, double fPosY
) {
    if (eKind != EInputKind::POINTER) return ;
    
    double fDeltaX = fPosX-m_fPointerX; m_fPointerX = fPosX;
    double fDeltaY = fPosY-m_fPointerY; m_fPointerY = fPosY;
    
    if(m_eViewMode == EViewMode::TOPDOWN) {
        
        // double t, tmin, tmax;
        // stTestBox.min = vec3(-1, -1, -1);
        // stTestBox.max = vec3( 1,  1,  1);
        // int res = stTestBox.intersectRay(vWorldPos, vWorldDir, t, tmin, tmax);
        // printf("%d: %f\n", res, t);
        
        if(m_bViewDragActive) {
            /*SCameraData stCameraData;
            getCameraData(stCameraData);
            vec3 vMousePos = getPointerPosition();
            vec3 vWorldPos, vWorldDir;
            m_pEngine->getRenderer()->screenToWorld(vMousePos, vWorldPos, vWorldDir);
            plane planeFloor(vec3(0, 1, 0), 0.0);
            vec3 vCurrentPos = planeFloor.intersectLine(vWorldPos, vWorldPos+vWorldDir*10000.0);
            
            m_vTopDownViewPosition.x = m_vViewDragStartViewPos.x-(vCurrentPos.x-m_vViewDragStartClickPos.x);
            m_vTopDownViewPosition.z = m_vViewDragStartViewPos.z-(vCurrentPos.z-m_vViewDragStartClickPos.z);
            */
            //double fSensitivity = 0.02;
            //m_pEngine->getOption("TopDownViewSensitivity", &fSensitivity, fSensitivity);
            //m_vTopDownViewPosition.x -= fDeltaX*fSensitivity;
            //m_vTopDownViewPosition.z -= fDeltaY*fSensitivity;
        }
    }
}
