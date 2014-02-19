#pragma once
#ifndef idF6C14A32_669E_4430_8AAB87DAC4BC2E54
#define idF6C14A32_669E_4430_8AAB87DAC4BC2E54

#include <string>
#include <vector>
#include "matrix.h"
#include "model.h"

class CProp;
class CEntity;
#include "CEntity.h"

class CProp : public CEntity {
    
    public: // typedefs
        
    public: // constructors/destructors
        CProp(CEngine *pEngine, CModel *pModel);
        virtual ~CProp();
        
        virtual void renderInit();
        
    public: // public methods
        bool calculateExtents(vec3 &vExtentsMin, vec3 &vExtentsMax) const;
        
        virtual CProp &setModel(CModel *pModel);
        
    protected: // internal methods
        virtual void _think(double fTime, double fDeltaTime);
        virtual void _render(
            CRenderer *pRenderer, double fDeltaTime, double fDeltaClock,
            CEntity::ERenderMode eMode, int iDetailLevel
        );
        virtual void _onChange();
        
    protected: // private data
        CModel *m_pModel;
        
        std::vector<unsigned int> m_clShapeTextureIds;
        
};

#endif // header
