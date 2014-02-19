#pragma once
#ifndef id0A62A018_33EA_4A44_81B59E8F324CD94D
#define id0A62A018_33EA_4A44_81B59E8F324CD94D

class CCar;
class CProp;
#include "CProp.h"
class CWorld;
#include "CWorld.h"

class CCar : public CProp {
    
    public: // typedefs
        typedef enum {
            INIT,
            DECIDING,
            WAITING,
            MOVING,
            DEAD
        } EState;
        
    public: // constructors/destructors
        CCar(CEngine *pEngine, CWorld *pWorld, vec3 vCurrentCellPos);
        virtual ~CCar();
        
    public: // public methods
        
    protected: // internal methods
        virtual void _think(double fTime, double fDeltaTime);
        virtual void _render(
            CRenderer *pRenderer, double fDeltaTime, double fDeltaClock,
            CEntity::ERenderMode eMode, int iDetailLevel
        );
        virtual void _onChange();
        
        void orientCar(
            bool moving, double fraction,
            vec3 curr, bool currfirst,
            vec3 dest, bool destfirst,
            bool fade
        );
        
    protected: // private data
        CWorld *m_pWorld;
        
        double m_fDelay;
        
        EState m_eState;
        CWorld::EDirection m_eCurrentDir;
        vec3 m_vCurrentCellPos;
        bool m_bCurrentFirst;
        CWorld::EDirection m_eDestinationDir;
        vec3 m_vDestinationCellPos;
        bool m_bDestinationFirst;
        bool m_bAtIntersection;
        bool m_bWasBlockingIntersection;
        double m_fMovingStartTime;
        double m_fMovingDuration;
        double m_fLastMoveTime;
        
        bool m_bIsCopCar;
        double m_fCopLightsOffset;
        double m_fCopLightsRate;
};


#endif // header
