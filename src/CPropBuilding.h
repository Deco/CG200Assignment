#pragma once
#ifndef idE9C30BBE_0940_4F49_A1A620E320ACD98C
#define idE9C30BBE_0940_4F49_A1A620E320ACD98C

// put stuff here
#include <string>
#include <vector>
#include "matrix.h"
#include "model.h"

class CPropBuilding;
class CProp;
#include "CProp.h"
class CWorld;
#include "CWorld.h"

class CPropBuilding : public CProp {
    
    public: // typedefs
        
    public: // constructors/destructors
        CPropBuilding(
            CEngine *pEngine,
            CWorld::ELotType eBuildingType,
            double fWidth, double fLength,
            std::vector<CWorld::EDirection> clRoadDirs,
            vec3 vCol,
            CWorld::CCell &stCell
        );
        virtual ~CPropBuilding();
        
    public: // public methods;
        virtual void _render(
            CRenderer *pRenderer, double fDeltaTime, double fDeltaClock,
            CEntity::ERenderMode eMode, int iDetailLevel
        );
        
        CModel *generateModel(
            CWorld::ELotType eBuildingType,
            double fWidth, double fLength,
            std::vector<CWorld::EDirection> clRoadDirs,
            vec3 vCol,
            CWorld::CCell &stCell
        );
        void generateSimpleBuilding(
            CModel &stModel,
            CWorld::ELotType eBuildingType,
            double fWidth, double fLength,
            std::vector<CWorld::EDirection> clRoadDirs,
            vec3 vCol,
            CWorld::CCell &stCell,
            bool bJustPavement = false
        );
        void generateRoad(
            CModel &stModel,
            CWorld::ELotType eBuildingType,
            double fWidth, double fLength,
            std::vector<CWorld::EDirection> clRoadDirs,
            vec3 vCol,
            CWorld::CCell &stCell
        );
        void generateTeapot(
            CModel &stModel,
            CWorld::ELotType eBuildingType,
            double fWidth, double fLength,
            std::vector<CWorld::EDirection> clRoadDirs,
            vec3 vCol,
            CWorld::CCell &stCell
        );
        void generateStatue(
            CModel &stModel,
            CWorld::ELotType eBuildingType,
            double fWidth, double fLength,
            std::vector<CWorld::EDirection> clRoadDirs,
            vec3 vCol,
            CWorld::CCell &stCell
        );
        
    protected: // internal methods
        
        
    protected: // private data
        // bool m_bImATeapot;
        // double m_fTeapotShortAndStoutness;
        
        CWorld::CCell *m_pCell;
        
};

#endif // header
