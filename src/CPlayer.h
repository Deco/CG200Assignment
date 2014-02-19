#pragma once
#ifndef id28C6B871_28F0_44FD_8FF1DBA3E11F0D84
#define id28C6B871_28F0_44FD_8FF1DBA3E11F0D84

#include <string>
#include "matrix.h"

class CPlayer;
class CEntity;
#include "CEntity.h"

class CPlayer : public CEntity {
    
    public: // typedefs
        typedef enum {
            UNKNOWN      = 0,
            LEFTMOUSE    = 1,
            RIGHTMOUSE   = 2,
            MIDDLEMOUSE  = 3,
            WHEELUP      = 4,
            WHEELDOWN    = 5,
            FORWARD      = 6,
            BACK         = 7
        } EInputPointerButton;
        typedef enum {
            POINTER, KEY
        } EInputKind;
        typedef enum {
            TOPDOWN, FPS
        } EViewMode;
        typedef struct {
            vec3 vPosition;
            quat qRotation;
            number fFieldOfView;
        } SCameraData;
        
    public: // constructors/destructors
        CPlayer(CEngine *pEngine);
        virtual ~CPlayer();
        
    public: // public methods
        void getCameraData(SCameraData &pData);
        
        vec3 getPointerPosition() const { return vec3(m_fPointerX, m_fPointerY, 0); }
        
        void onInputButtonEvent(
            EInputKind eKind, int eButtonCode, bool bButtonState, bool bIsSpecial = false
        );
        void onInputMotionEvent(
            EInputKind eKind, double fPosX, double fPosY
        );
        int getInputCursor();
        
    protected: // internal methods
        virtual void _think(double fTime, double fDeltaTime);
        virtual void _render(
            CRenderer *pRenderer, double fDeltaTime, double fDeltaClock,
            CEntity::ERenderMode eMode, int iDetailLevel
        );
        
    public: // private data
        
        bool m_abPointerButtonStates[32];
        bool m_abButtonStates[256];
        bool m_abSpecialButtonStates[1024];
        double m_fPointerX, m_fPointerY;
        int m_iPointerUseCount;
        EViewMode m_eViewMode;
        vec3 m_vTopDownViewPosition;
        int m_iTopDownViewZoomLevel;
        bool m_bViewDragActive;
        vec3 m_vViewDrawClickPos;
        //vec3 m_vViewDragStartViewPos;
        //vec3 m_vViewDragStartClickPos;
        
        bool m_bAnimationState;
        
        CEntity *m_pEntityToFollow;
        
};

#endif // header
