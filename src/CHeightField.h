#pragma once
#ifndef id05562A51_F804_44AC_A298C5601668EFA0
#define id05562A51_F804_44AC_A298C5601668EFA0

#include "matrix.h"
#include <cmath>
#include <cstdio>
#include <ctime>

class CHeightField;
class CEntity;
#include "CEntity.h"

class CHeightField : public CEntity {
    
    public: // typedefs
        
    public: // constructors/destructors
        CHeightField(CEngine *pEngine, int iGridW, int iGridH);
        virtual ~CHeightField();
        
    public: // public methods
        bool calculateExtents(vec3 &vExtentsMin, vec3 &vExtentsMax) const;
        
    protected: // internal methods
        virtual void _think(double fTime, double fDeltaTime);
        virtual void _render(
            CRenderer *pRenderer, double fDeltaTime, double fDeltaClock,
            CEntity::ERenderMode eMode, int iDetailLevel
        );
        
    protected: // private data
        int m_iGridW;
        int m_iGridH;
        double *m_aafGridValues;
        
};

#endif // header
