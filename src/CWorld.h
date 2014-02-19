#pragma once
#ifndef id43107917_6B34_40FA_A4EEBE61AAA2BAB2
#define id43107917_6B34_40FA_A4EEBE61AAA2BAB2

#include <unordered_set>

class CPlayer;
class CEntity;
#include "CEntity.h"

class CWorld : public CEntity {
    
    public: // typedefs
        typedef enum {
            NONE,
            ROAD,
            LOT
        } ECellType;
        typedef enum {
            UP = 0,
            RIGHT,
            DOWN,
            LEFT
        } EDirection;
        typedef enum {
            BLANK,
            PARK,
            BUILDING,
            STATUE,
            STREET
        } ELotType;
        typedef enum {
            STRAIGHT,
            CORNER,
            TSECTION,
            CROSSROADS,
            DEADEND,
            TOTHEVOID
        } ERoadType;
        class CCell {
            public:
                vec3 fillcol;
                vec3 col;
                ECellType type;
                int roadid;
                int lotid;
                bool show;
                bool dbgused;
                vec3 dbg;
                vec3 dbgoverride;
                vec3 dbgfill;
                vec3 edgecols[4];
                
                ERoadType roadtype;
                bool straight_up;
                
                bool occupied_positive_first; // up/right
                bool occupied_positive_second;
                bool occupied_negative_first;
                bool occupied_negative_second;
                bool reserved_positive_first;
                bool reserved_positive_second;
                bool reserved_negative_first;
                bool reserved_negative_second;
                
                inline bool &occupied(bool positive, bool first) {
                    if(positive) {
                        if(first) return occupied_positive_first ;
                        else      return occupied_positive_second;
                    } else {
                        if(first) return occupied_negative_first ;
                        else      return occupied_negative_second;
                    }
                }
                inline bool &reserved(bool positive, bool first) {
                    if(positive) {
                        if(first) return reserved_positive_first ;
                        else      return reserved_positive_second;
                    } else {
                        if(first) return reserved_negative_first ;
                        else      return reserved_negative_second;
                    }
                }
                
                bool busy_ru;
                bool busy_rd;
                bool busy_lu;
                bool busy_ld;
                bool busy_mm;
        };
        class CRoadAnt {
            public:
                // CRoadAnt() { };
                // virtual ~CRoadAnt() { };
                
                bool done;
                vec3 pos;
                EDirection dir;
                int turncooldown;
                int justmade;
                int turnskip;
                vec3 dbg;
        };
        class CLot {
            public:
                bool done;
                bool built;
                vec3 tl;
                vec3 br;
                EDirection dir;
                vec3 dbg;
        };
        typedef enum {
            START,
            ROAD_GEN, ROAD_GEN_INIT = ROAD_GEN, ROAD_GEN_STEP, ROAD_GEN_BUILD,
            LOT_GEN, LOT_GEN_INIT = LOT_GEN, LOT_GEN_STEP, LOT_GEN_BUILD,
            DONE
        } EInitStage;
        
    public: // constructors/destructors
        CWorld(CEngine *pEngine, int iGridW = 40, int iGridH = 40);
        virtual ~CWorld();
        
    public: // public methods
        void initWorld();
        bool initWorldStep(double &rfSignificance);
        EInitStage roadGenInit(double &rfSignificance);
        EInitStage roadGenStep(double &rfSignificance);
        EInitStage roadGenBuild(double &rfSignificance);
        EInitStage lotGenInit(double &rfSignificance);
        EInitStage lotGenStep(double &rfSignificance);
        EInitStage lotGenBuild(double &rfSignificance);
        
        bool calculateExtents(vec3 &vExtentsMin, vec3 &vExtentsMax) const;
        
    public: // internal methods
        virtual void _think(double fTime, double fDeltaTime);
        virtual void _render(
            CRenderer *pRenderer, double fDeltaTime, double fDeltaClock,
            CEntity::ERenderMode eMode, int iDetailLevel
        );
        
        void highlightCells();
        void clearCellHighlight(bool complete = false);
        int checkDirection(std::vector<vec3> &outvec, vec3 pos, EDirection dir, int len, int start = 1, bool includeEmpty = false);
        int checkNeighbours(std::vector<vec3> &outvec, vec3 pos, bool includeEmpty = false);
        void checkArea(std::vector<vec3> &outvec, vec3 pos, int su, int sr, int sd, int sl, bool includeEmpty = false);
        CCell &cellAt(vec3 pos);
        vec3 cellPos(vec3 pos);
        
        void makeBuilding(CLot &stLot);
        void makeRoad(CCell &stCell, vec3 vCellPos);
        
    public: // private data
        static const double m_iCellW;
        static const double m_iCellH;
        
        int m_iGridW;
        int m_iGridH;
        CCell *m_aastGrid;
        
        EInitStage m_eInitStage;
        
        static double fRoadAntTurnChance;
        static double fRoadAntBranchChance;
        static int iRoadAntTurnCooldown;
        static int iRoadAntJoinDistance;
        std::vector<CRoadAnt> m_clstRoadAnts;
        int m_iRoadAntCount;
        
        static int iLotMaxSize;
        static double fMaxAspectRatio;
        //static double iLotSpawnLotChance;
        static double fLotEarlyFinishChance;
        static int iLotEarlyFinishMinSize;
        std::vector<CLot> m_clstLots;
        CLot *m_pActiveLot;
        std::vector<vec3> m_clFreeCells;
        
        static double fCarPlacementChance;
        
        std::vector<CEntity*> m_clGeneratedProps;
        
        bool m_bForceEnableLighting;
        double m_fForceEnableLightingTime;
};

inline vec3 dir2vec(CWorld::EDirection dir) {
    return (
            dir == CWorld::EDirection::UP    ? vec3( 0,  1, 0)
        :   dir == CWorld::EDirection::RIGHT ? vec3( 1,  0, 0)
        :   dir == CWorld::EDirection::DOWN  ? vec3( 0, -1, 0)
        :   dir == CWorld::EDirection::LEFT  ? vec3(-1,  0, 0)
        :   vec3(0, 0, 0) // wat
    );
}
inline CWorld::EDirection turndir(CWorld::EDirection dir, int cw90) {
    bool opp = (cw90 < 0);
    if(opp) cw90 = -cw90;
    while(cw90 > 0) {
        dir = (
                dir == CWorld::EDirection::UP    ? (!opp ? CWorld::EDirection::RIGHT : CWorld::EDirection::LEFT )
            :   dir == CWorld::EDirection::RIGHT ? (!opp ? CWorld::EDirection::DOWN  : CWorld::EDirection::UP   )
            :   dir == CWorld::EDirection::DOWN  ? (!opp ? CWorld::EDirection::LEFT  : CWorld::EDirection::RIGHT)
            :   dir == CWorld::EDirection::LEFT  ? (!opp ? CWorld::EDirection::UP    : CWorld::EDirection::DOWN )
            :   (CWorld::EDirection)-1 // ?!?
        );
        cw90--;
    }
    return dir;
}

#endif // header
