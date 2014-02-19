#include <cmath>
#include <cstring>
#include <vector>
#include <queue>
#include <algorithm>
#include "matrix.h"
#include "GL/freeglut.h"

class CWorld;
#include "CWorld.h"
class CEngine;
#include "CEngine.h"
class CPropBuilding;
#include "CPropBuilding.h"
class CCar;
#include "CCar.h"

const double CWorld::m_iCellW = 16;
const double CWorld::m_iCellH = CWorld::m_iCellW;

CWorld::CWorld(
    CEngine *pEngine,
    int iGridW, int iGridH
) : CEntity(pEngine),
    m_iGridW(iGridW),
    m_iGridH(iGridH),
    m_clstRoadAnts(),
    m_clstLots(),
    m_clFreeCells(),
    m_clGeneratedProps(),
    m_bForceEnableLighting(false)
{
    m_sLabel = "World";
    
    m_aastGrid = new CCell[m_iGridW*m_iGridH];
    memset(m_aastGrid, 0, m_iGridW*m_iGridH*sizeof(*m_aastGrid));
    
    m_eInitStage = DONE;
}
CWorld::~CWorld() {
    
}

void CWorld::_think(
    double fTime, double fDeltaTime
) {
    if(m_bForceEnableLighting && fTime > m_fForceEnableLightingTime) {
        m_pEngine->setOption("Lighting - Enabled", true);
        m_bForceEnableLighting = false;
    }
}

void CWorld::_render(
    CRenderer *pRenderer,
    double fDeltaTime, double fDeltaClock,
    CEntity::ERenderMode eMode,
    int iDetailLevel
) {
    if(eMode != CEntity::ERenderMode::NORMAL) return;
    
    double cw = m_iCellW, ch = m_iCellH;
    double xmin = -cw*m_iGridW/2; double xmax = cw*m_iGridW/2;
    double ymin = -ch*m_iGridH/2; double ymax = ch*m_iGridH/2;
    //xmin += cw/2.0; ymin += ch/2.0;
    
    glColor3f(0, 0, 0);
    glBegin(GL_QUADS);
        glVertex3d(xmax, -0.45, ymin);
        glVertex3d(xmin, -0.45, ymin);
        glVertex3d(xmin, -0.45, ymax);
        glVertex3d(xmax, -0.45, ymax);
    glEnd();
    
    for(int y = 0; y < m_iGridH; y++) {
        for(int x = 0; x < m_iGridW; x++) {
            CCell &stCell = cellAt(vec3(x, y, 0));
            
            double lineWidth = 5;
            double gap = 0.08;
            vec3 a = vec3(xmin+cw*(x+0+gap), -0.22, ymin+ch*(y+0+gap));
            vec3 b = vec3(xmin+cw*(x+1-gap), -0.22, ymin+ch*(y+0+gap));
            vec3 c = vec3(xmin+cw*(x+1-gap), -0.22, ymin+ch*(y+1-gap));
            vec3 d = vec3(xmin+cw*(x+0+gap), -0.22, ymin+ch*(y+1-gap));
            /* if(stCell.busy_ru) {
                glColor3f(1, 0, 0);
                glBegin(GL_QUADS);
                    glVertex3d(xmin+cw*(x+1.0-gap), 1, ymin+ch*(y+0.0+gap));
                    glVertex3d(xmin+cw*(x+0.5+gap), 1, ymin+ch*(y+0.0+gap));
                    glVertex3d(xmin+cw*(x+0.5+gap), 1, ymin+ch*(y+0.5-gap));
                    glVertex3d(xmin+cw*(x+1.0-gap), 1, ymin+ch*(y+0.5-gap));
                glEnd();
            }
            if(stCell.busy_rd) {
                glColor3f(1, 0, 0);
                glBegin(GL_QUADS);
                    glVertex3d(xmin+cw*(x+1.0-gap), 0.5, ymin+ch*(y+0.5+gap));
                    glVertex3d(xmin+cw*(x+0.5+gap), 0.5, ymin+ch*(y+0.5+gap));
                    glVertex3d(xmin+cw*(x+0.5+gap), 0.5, ymin+ch*(y+1.0-gap));
                    glVertex3d(xmin+cw*(x+1.0-gap), 0.5, ymin+ch*(y+1.0-gap));
                glEnd();
            }
            if(stCell.busy_lu) {
                glColor3f(1, 0, 0);
                glBegin(GL_QUADS);
                    glVertex3d(xmin+cw*(x+0.5-gap), 0.5, ymin+ch*(y+0.0+gap));
                    glVertex3d(xmin+cw*(x+0.0+gap), 0.5, ymin+ch*(y+0.0+gap));
                    glVertex3d(xmin+cw*(x+0.0+gap), 0.5, ymin+ch*(y+0.5-gap));
                    glVertex3d(xmin+cw*(x+0.5-gap), 0.5, ymin+ch*(y+0.5-gap));
                glEnd();
            }
            if(stCell.busy_ld) {
                glColor3f(1, 0, 0);
                glBegin(GL_QUADS);
                    glVertex3d(xmin+cw*(x+0.5-gap), 0.5, ymin+ch*(y+0.5+gap));
                    glVertex3d(xmin+cw*(x+0.0+gap), 0.5, ymin+ch*(y+0.5+gap));
                    glVertex3d(xmin+cw*(x+0.0+gap), 0.5, ymin+ch*(y+1.0-gap));
                    glVertex3d(xmin+cw*(x+0.5-gap), 0.5, ymin+ch*(y+1.0-gap));
                glEnd();
            }
            if(stCell.busy_mm) {
                glColor3f(1, 0, 0);
                glBegin(GL_QUADS);
                    glVertex3d(xmin+cw*(x+0.7-gap), 0.5, ymin+ch*(y+0.3+gap));
                    glVertex3d(xmin+cw*(x+0.3+gap), 0.5, ymin+ch*(y+0.3+gap));
                    glVertex3d(xmin+cw*(x+0.3+gap), 0.5, ymin+ch*(y+0.7-gap));
                    glVertex3d(xmin+cw*(x+0.7-gap), 0.5, ymin+ch*(y+0.7-gap));
                glEnd();
            } */
            if(!stCell.show) continue;
            // glColor3f(0.0, 1.0, 0.0);
            //if(stCell.fillcol != vec3(0, 0, 0)) {
            if(true) {
                glColor3f(stCell.fillcol.x, stCell.fillcol.y, stCell.fillcol.z);
                glBegin(GL_QUADS);
                    glVertex3d(b.x, b.y, b.z);
                    glVertex3d(a.x, a.y, a.z);
                    glVertex3d(d.x, d.y, d.z);
                    glVertex3d(c.x, c.y, c.z);
                glEnd();
            }
            
            
            
            // glColor3f(0.0, 0.0, 0.0);
            glLineWidth(lineWidth);
            glBegin(GL_LINES);
                glColor3f(stCell.edgecols[0].x, stCell.edgecols[0].y, stCell.edgecols[0].z);
                glVertex3d(b.x, b.y+0.01, b.z);
                glVertex3d(a.x, a.y+0.01, a.z);
                glColor3f(stCell.edgecols[3].x, stCell.edgecols[3].y, stCell.edgecols[3].z);
                glVertex3d(a.x, a.y+0.01, a.z);
                glVertex3d(d.x, d.y+0.01, d.z);
                glColor3f(stCell.edgecols[2].x, stCell.edgecols[2].y, stCell.edgecols[2].z);
                glVertex3d(d.x, d.y+0.01, d.z);
                glVertex3d(c.x, c.y+0.01, c.z);
                glColor3f(stCell.edgecols[1].x, stCell.edgecols[1].y, stCell.edgecols[1].z);
                glVertex3d(c.x, c.y+0.01, c.z);
                glVertex3d(b.x, b.y+0.01, b.z);
            glEnd();
        }
    }
    
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_LINES);
        glVertex3f(-1.0, 0.0, 0.0);
        glVertex3f( 1.0, 0.0, 0.0);
        glVertex3f( 0.0,-1.0, 0.0);
        glVertex3f( 0.0, 1.0, 0.0);
        glVertex3f( 0.0, 0.0,-1.0);
        glVertex3f( 0.0, 0.0, 1.0);
    glEnd();
}

const vec3 vDefaultCol = vec3(0.1, 0.1, 0.1);
const vec3 vRoadCol = vec3(0.5, 0.1, 0.1);

double CWorld::fRoadAntTurnChance = 0.24;
double CWorld::fRoadAntBranchChance = 0.45;
int CWorld::iRoadAntTurnCooldown = 4;
int CWorld::iRoadAntJoinDistance = 2;

int CWorld::iLotMaxSize = 10;
double CWorld::fMaxAspectRatio = 1.6;
double CWorld::fLotEarlyFinishChance = 0.17;
int CWorld::iLotEarlyFinishMinSize = 2;

double CWorld::fCarPlacementChance = 0.65;

void CWorld::initWorld() {
    
    for(auto itProp = m_clGeneratedProps.begin(); itProp != m_clGeneratedProps.end(); itProp++) {
        m_pEngine->removeEntity(*itProp);
    }
    m_clGeneratedProps.clear();
    
    for(int y = 0; y < m_iGridH; y++) {
        for(int x = 0; x < m_iGridW; x++) {
            CCell &stCell = cellAt(vec3(x, y, 0));
            
            stCell.show = true;
            stCell.type = NONE;
            stCell.fillcol = vec3(0, 0, 0);
            stCell.col = vDefaultCol;
            stCell.dbgoverride = vec3(0, 0, 0);
            stCell.dbg = vec3(0, 0, 0);
            stCell.dbgfill = vec3(0, 0, 0);
            stCell.roadtype = STRAIGHT;
            stCell.straight_up = false;
            stCell.occupied_positive_first   = false;
            stCell.occupied_positive_second  = false;
            stCell.occupied_negative_first   = false;
            stCell.occupied_negative_second  = false;
            stCell.reserved_positive_first   = false;
            stCell.reserved_positive_second  = false;
            stCell.reserved_negative_first   = false;
            stCell.reserved_negative_second  = false;
            stCell.busy_ru = false;
            stCell.busy_rd = false;
            stCell.busy_lu = false;
            stCell.busy_ld = false;
            stCell.busy_mm = false;
            
            for(int edgei = 0; edgei < 4; edgei++) {
                stCell.edgecols[edgei] = vec3(0.05, 0.05, 0.05);
            }
        }
    }
    m_eInitStage = START;
    m_pEngine->watchVariable("Gen - Stage", (unsigned int *)&m_eInitStage, "group='World'");
    m_clstRoadAnts.clear();
    m_clstLots.clear();
    // m_clNavNodes.clear();
    m_iRoadAntCount = 0;
}

bool CWorld::initWorldStep(double &rfSignificance) {
    EInitStage startstage = m_eInitStage;
    switch(m_eInitStage) {
        case START: {
            m_eInitStage = ROAD_GEN;
            rfSignificance = 1;
            //break;
        }
        case ROAD_GEN_INIT: {
            m_eInitStage = roadGenInit(rfSignificance);
            break;
        }
        case ROAD_GEN_STEP: {
            m_eInitStage = roadGenStep(rfSignificance);
            break;
        }
        case ROAD_GEN_BUILD: {
            m_eInitStage = roadGenBuild(rfSignificance);
            break;
        }
        case LOT_GEN_INIT: {
            m_eInitStage = lotGenInit(rfSignificance);
            break;
        }
        case LOT_GEN_STEP: {
            m_eInitStage = lotGenStep(rfSignificance);
            break;
        }
        case LOT_GEN_BUILD: {
            m_eInitStage = lotGenBuild(rfSignificance);
            break;
        }
        case DONE: {
            rfSignificance = 1;
            return true;
            break;
        }
        default: {
            std::cerr << "Unknown CWorld.m_eInitStage value: " << m_eInitStage << std::endl;
        }
    }
    if(m_eInitStage != startstage) {
        clearCellHighlight(true);
        highlightCells();
    }
    return (m_eInitStage == DONE);
}

CWorld::EInitStage CWorld::lotGenInit(double &rfSignificance) {
    // init lot gen
    rfSignificance = 1;
    
    clearCellHighlight();
    
    m_pActiveLot = NULL;
    m_clFreeCells.clear();
    for(int y = 0; y < m_iGridH; y++) {
        for(int x = 0; x < m_iGridW; x++) {
            CCell &stCell = cellAt(vec3(x, y, 0));
            if(stCell.type == NONE) {
                m_clFreeCells.push_back(vec3(x, y, 0));
            }
        }
    }
    
    std::random_shuffle(m_clFreeCells.begin(), m_clFreeCells.end());
    
    highlightCells();
    
    return LOT_GEN_STEP;
}
CWorld::EInitStage CWorld::lotGenStep(double &rfSignificance) {
    // 
    rfSignificance = 0;
    if(m_pActiveLot == NULL) {
        rfSignificance = 1;
    }
    
    clearCellHighlight();
    bool done = false;
    if(m_pActiveLot == NULL) {
        bool found = false;
        vec3 foundPos;
        EDirection foundDir;
        while(m_clFreeCells.size() > 0 && !found) {
            foundPos = m_clFreeCells.back();
            CCell &stCell = cellAt(foundPos);
            //printf("%f, %f = %d\n", foundPos.x, foundPos.y, stCell.type);
            if(stCell.type == NONE) {
                std::vector<vec3> neighbours;
                checkNeighbours(neighbours, foundPos, false);
                for(auto itNeighbour = neighbours.begin(); itNeighbour != neighbours.end() && !found; itNeighbour++) {
                    CCell &stNeighbourCell = cellAt(*itNeighbour);
                    if(stNeighbourCell.type == ROAD) {
                        foundDir = (
                                ((*itNeighbour)+dir2vec(UP   ) == foundPos) ? UP
                            :   ((*itNeighbour)+dir2vec(RIGHT) == foundPos) ? RIGHT
                            :   ((*itNeighbour)+dir2vec(DOWN ) == foundPos) ? DOWN
                            :   ((*itNeighbour)+dir2vec(LEFT ) == foundPos) ? LEFT
                            :   (EDirection)-1
                        );
                        found = true;
                    }
                }
            }
            m_clFreeCells.pop_back();
            //printf("%d\n", m_clFreeCells.size());
        }
        if(found) {
            m_clstLots.push_back(CLot());
            CLot &stLot = m_clstLots[m_clstLots.size()-1];
            stLot.done = false;
            stLot.built = false;
            stLot.tl = foundPos;
            stLot.br = foundPos;
            stLot.dir = foundDir;
            double rgb[3];
            hsvToRgb(number_rand(), 0.7, 0.6, rgb);
            stLot.dbg = vec3(rgb[0], rgb[1], rgb[2]);
            
            CCell &stCell = cellAt(foundPos);
            stCell.type = LOT;
            
            m_pActiveLot = &stLot;
        } else {
            done = true;
        }
    } else {
        CLot &stLot = *m_pActiveLot;
        
        bool shouldStopIfBig = (number_rand() < fLotEarlyFinishChance);
        bool didGrow = false;
        EDirection growdirorig = turndir(UP, randrange(0, 3));
        for(int growdiroffset = 0; growdiroffset < 4; growdiroffset++) {
            EDirection growdir = turndir(growdirorig, growdiroffset);
            std::vector<vec3> neighbours;
            int w = stLot.br.x-stLot.tl.x+1, h = stLot.br.y-stLot.tl.y+1;
            bool canGrow = (
                    growdir == UP    ? (h < iLotMaxSize && (double)h/(double)w <= fMaxAspectRatio)
                :   growdir == RIGHT ? (w < iLotMaxSize && (double)w/(double)h <= fMaxAspectRatio)
                :   growdir == DOWN  ? (h < iLotMaxSize && (double)h/(double)w <= fMaxAspectRatio)
                :   growdir == LEFT  ? (w < iLotMaxSize && (double)w/(double)h <= fMaxAspectRatio)
                :   false
            );
            if(shouldStopIfBig && w >= iLotEarlyFinishMinSize && h >= iLotEarlyFinishMinSize) {
                canGrow = false;
            }
            if(!canGrow) {
                continue;
            }
            switch(growdir) {
                case UP   : { checkArea(neighbours, stLot.tl+vec3( 0,-1,0), 0  , w-1, 0  , 0  , true); break; }
                case RIGHT: { checkArea(neighbours, stLot.br+vec3( 1, 0,0), h-1, 0  , 0  , 0  , true); break; }
                case DOWN : { checkArea(neighbours, stLot.br+vec3( 0, 1,0), 0  , 0  , 0  , w-1, true); break; }
                case LEFT : { checkArea(neighbours, stLot.tl+vec3(-1, 0,0), 0  , 0  , h-1, 0  , true); break; }
            }
            //printf("l = %d\n", neighbours.size());
            if(neighbours.size() == 0) {
                canGrow = false; // at edge of grid
            }
            for(auto itNeighbour = neighbours.begin(); itNeighbour != neighbours.end() && canGrow; itNeighbour++) {
                CCell &stNeighbourCell = cellAt(*itNeighbour);
                //printf("%f, %f = %d\n", itNeighbour->x, itNeighbour->y, stNeighbourCell.type);
                if(stNeighbourCell.type != NONE) {
                    //stNeighbourCell.dbgoverride = vec3(0, 0, 1);
                    canGrow = false;
                }
            }
            if(canGrow) {
                //printf("%d\n", growdir);
                switch(growdir) {
                    case UP   : { stLot.tl = stLot.tl+vec3( 0,-1,0); break; }
                    case RIGHT: { stLot.br = stLot.br+vec3( 1, 0,0); break; }
                    case DOWN : { stLot.br = stLot.br+vec3( 0, 1,0); break; }
                    case LEFT : { stLot.tl = stLot.tl+vec3(-1, 0,0); break; }
                }
                for(auto itNeighbour = neighbours.begin(); itNeighbour != neighbours.end(); itNeighbour++) {
                    CCell &stNeighbourCell = cellAt(*itNeighbour);
                    stNeighbourCell.type = LOT;
                }
                didGrow = true;
                break;
            } else {
                continue;
            }
        }
        if(!didGrow) {
            stLot.done = true;
            
            m_pActiveLot = NULL;
        }
    }
    highlightCells();
    
    if(done) {
        return (EInitStage)(LOT_GEN_STEP+1);
    }
    return (EInitStage)(LOT_GEN_STEP);
}
CWorld::EInitStage CWorld::lotGenBuild(double &rfSignificance) {
    rfSignificance = 0.5;
    bool bComplete = true;
    for(int i = 0; i < 5; i++) {
        for(auto itLot = m_clstLots.begin(); itLot != m_clstLots.end(); itLot++) {
            CLot &stLot = *itLot;
            
            if(!stLot.built) {
                makeBuilding(stLot);
                stLot.built = true;
                bComplete = false;
                m_clstLots.erase(itLot);
                break;
            }
        }
    }
    if(bComplete) {
        m_bForceEnableLighting = true;
        m_fForceEnableLightingTime = m_pEngine->getTime()+9;
        return (EInitStage)(LOT_GEN_BUILD+1);
    }
    return LOT_GEN_BUILD;
}
void CWorld::makeBuilding(CLot &stLot) {
    vec3 pos = cellPos(stLot.tl);
    double cw = m_iCellW, ch = m_iCellH;
    
    int w = stLot.br.x-stLot.tl.x+1, h = stLot.br.y-stLot.tl.y+1;
    std::vector<EDirection> roaddirs;
    roaddirs.push_back(stLot.dir);
    
    m_clGeneratedProps.push_back(
        &((*(new CPropBuilding(m_pEngine, ELotType::BUILDING, w*cw, h*ch, roaddirs, stLot.dbg, cellAt(stLot.tl))))
            .setLabel("building")
            .setPosition(pos)
            .setRotation(quat())//quat::random())
            .tween(pos+vec3(0, -1000, 0), pos, 1.5+number_rand()*6, CEntity::ETweenType::LINEAR)
        )
    );
}


CWorld::EInitStage CWorld::roadGenInit(double &rfSignificance) {
    // create road ants
    rfSignificance = 1;
    
    clearCellHighlight();
    
    m_iRoadAntCount = randrange(10, 14);
    m_pEngine->getOption("Gen - Road Ant Count", &m_iRoadAntCount, m_iRoadAntCount, "group='World'");
    EDirection firstdir;
    
    for(int iRoadAntNum = 0; iRoadAntNum < m_iRoadAntCount; iRoadAntNum++) {
        m_clstRoadAnts.push_back(CRoadAnt());
        CRoadAnt &stRoadAnt = m_clstRoadAnts[iRoadAntNum];
        stRoadAnt.done = false;
        stRoadAnt.justmade = 1;
        stRoadAnt.turnskip = 0;
        // stRoadAnt.pos = vec3(
            // randrange(0, m_iGridW-1),
            // randrange(0, m_iGridH-1),
            // 0
        // );
        if(iRoadAntNum == 0) {
            stRoadAnt.pos = vec3(
                (int)(m_iGridW/2),
                (int)(m_iGridH/2),
                0
            );
            firstdir = turndir(UP, randrange(0, 3));
            stRoadAnt.dir = firstdir;
        } else if(iRoadAntNum == 1) {
            stRoadAnt.pos = vec3(
                (int)(m_iGridW/2),
                (int)(m_iGridH/2),
                0
            );
            stRoadAnt.dir = turndir(firstdir, 2);
        } else if(randrange(0,1) == 0) {
            bool right = (randrange(0,1) == 0);
            stRoadAnt.pos = vec3(
                (right ? m_iGridW-1 : 0),
                //(int)randrange(1, (int)(m_iGridH/4))*4-1,
               randrange(1, m_iGridH-2),
                0
            );
            stRoadAnt.dir = (right ? LEFT : RIGHT);
        } else {
            bool down = (randrange(0,1) == 0);
            stRoadAnt.pos = vec3(
                (int)randrange(1, (int)(m_iGridW/4))*4-1,
                (down ? m_iGridH-1 : 0),
                0
            );
            //stRoadAnt.dir = (down ? UP : DOWN);
            stRoadAnt.dir = (down ? DOWN : UP);
        }
        stRoadAnt.turncooldown = iRoadAntTurnCooldown+1;
        
        CCell &stCell = cellAt(stRoadAnt.pos);
        stCell.dbgoverride = vec3(0, 1, 0);
    }
    highlightCells();
    
    return ROAD_GEN_STEP;
}
CWorld::EInitStage CWorld::roadGenStep(double &rfSignificance) {
    // step a single road ant
    rfSignificance = 5.5;
    
    clearCellHighlight();
    
    int iActiveAntCount = 0;
    std::vector<CRoadAnt> clstNewRoadAnts;
    for(auto itRoadAnt = m_clstRoadAnts.begin(); itRoadAnt != m_clstRoadAnts.end(); itRoadAnt++) {
        int iRoadAntNum = (itRoadAnt- m_clstRoadAnts.begin());
        CRoadAnt &stRoadAnt = *itRoadAnt;
        stRoadAnt.dbg = vec3(0, 0, 0);
        
        if(stRoadAnt.done) continue;
        if(stRoadAnt.turnskip > 0) {
            stRoadAnt.dbg = vec3(1, 1, 0);
            continue;
        }
        if(stRoadAnt.justmade > 0) {
            //stRoadAnt.dbg = vec3(1, 1, 1);
            stRoadAnt.justmade--;
            //continue;
        }
        
        CCell &stCell = cellAt(stRoadAnt.pos);
        stCell.type = ROAD;
        stCell.roadid = iRoadAntNum;
        stCell.dbg = vec3(1, 0.4, 0.1);
        // for(int y = 0; y < m_iGridH; y++) {
            // for(int x = 0; x < m_iGridW; x++) {
                // CCell &stCell = m_aastGrid[y*m_iGridH+x];
                
                // printf("%d ", stCell.type);
            // }
            // printf("\n");
        // }
        
        if(stRoadAnt.justmade == 0) {
            std::vector<vec3> ahead, left, right;
            int distahead = checkDirection(ahead, stRoadAnt.pos, turndir(stRoadAnt.dir,  0), 1);
            int distleft  = checkDirection(left , stRoadAnt.pos, turndir(stRoadAnt.dir, -1), 1);
            int distright = checkDirection(right, stRoadAnt.pos, turndir(stRoadAnt.dir,  1), 1);
            if(
                    (distahead == 1) //&& ahead[0].roadid != iRoadAntNum)
                ||  (distleft  == 1) //&& left [0].roadid != iRoadAntNum)
                ||  (distright == 1) //&& right[0].roadid != iRoadAntNum)
            ) {
                stRoadAnt.done = true;
                //printf("Done at line %d (%d %d %d)\n", __LINE__, distleft, distahead, distright);
                continue;
                //stRoadAnt.dbg = vec3(1, 1, 1);
            }
        }
        
        stRoadAnt.pos = stRoadAnt.pos + dir2vec(stRoadAnt.dir);
        if(
                (int)stRoadAnt.pos.x < 0
            ||  (int)stRoadAnt.pos.x > m_iGridW-1
            ||  (int)stRoadAnt.pos.y < 0
            ||  (int)stRoadAnt.pos.y > m_iGridH-1
        ) {
            stRoadAnt.done = true;
            //printf("Done at line %d\n", __LINE__);
            continue;
        }
    }
    for(auto itRoadAnt = m_clstRoadAnts.begin(); itRoadAnt != m_clstRoadAnts.end(); itRoadAnt++) {
        int iRoadAntNum = (itRoadAnt- m_clstRoadAnts.begin());
        CRoadAnt &stRoadAnt = *itRoadAnt;
        
        if(stRoadAnt.done) continue;
        
        CCell &stNewCell = cellAt(stRoadAnt.pos);
        if(stNewCell.type == ROAD && stRoadAnt.justmade == 0) {// && stNewCell.roadid != iRoadAntNum) {
            stRoadAnt.done = true;
            //printf("Done at line %d\n", __LINE__);
            continue;
        }
        stNewCell.type = ROAD;
        stNewCell.roadid = iRoadAntNum;
    }
    for(auto itRoadAnt = m_clstRoadAnts.begin(); itRoadAnt != m_clstRoadAnts.end(); itRoadAnt++) {
        int iRoadAntNum = (itRoadAnt- m_clstRoadAnts.begin());
        CRoadAnt &stRoadAnt = *itRoadAnt;
        
        if(stRoadAnt.done) continue;
        if(stRoadAnt.turnskip > 0) {
            //printf("ts: %d\n", stRoadAnt.turnskip);
            stRoadAnt.dbg = vec3(1, 1, 0);
            stRoadAnt.turnskip--;
            continue;
        }
        iActiveAntCount++;
        
        CCell &stNewCell = cellAt(stRoadAnt.pos);
        
        //printf("%f < %f (%d)\n", number_rand(), fRoadAntTurnChance, stRoadAnt.turncooldown);
        std::vector<vec3> ahead, left, right;
        int distahead = checkDirection(ahead, stRoadAnt.pos, turndir(stRoadAnt.dir,  0), iRoadAntJoinDistance+1);
        int distleft  = checkDirection(left , stRoadAnt.pos, turndir(stRoadAnt.dir, -1), iRoadAntJoinDistance);
        int distright = checkDirection(right, stRoadAnt.pos, turndir(stRoadAnt.dir,  1), iRoadAntJoinDistance);
        //printf("[%d] %d %d %d\n", iRoadAntNum, distleft, distahead, distright);
        if(distahead == 1 || distleft == 1 || distright == 1) {
            CCell &stCell = cellAt(stRoadAnt.pos);
            stCell.type = ROAD;
            stCell.roadid = iRoadAntNum;
            
            stRoadAnt.done = true;
            //printf("Done at line %d\n", __LINE__);
            continue;
        }
        
        bool otherantahead = false;
        std::vector<vec3> neighbours;
        int aheadcount = checkNeighbours(neighbours, stRoadAnt.pos+dir2vec(turndir(stRoadAnt.dir,  0)))-1;
        if(stRoadAnt.justmade == 0) {
            for(auto itNeighbour = neighbours.begin(); itNeighbour != neighbours.end(); itNeighbour++) {
                vec3 &neighbourpos = *itNeighbour;
                CCell &stNeighbourCell = cellAt(neighbourpos);
                
                bool doturnskip = false;
                for(auto itOtherRoadAnt = m_clstRoadAnts.begin(); itOtherRoadAnt != m_clstRoadAnts.end(); itOtherRoadAnt++) {
                    CRoadAnt &stOtherRoadAnt = *itOtherRoadAnt;
                    if(
                            !stOtherRoadAnt.done
                        &&  (&stRoadAnt != &stOtherRoadAnt)
                        &&  number_eq(stOtherRoadAnt.pos.x, (double)neighbourpos.x)
                        &&  number_eq(stOtherRoadAnt.pos.y, (double)neighbourpos.y)
                    ) {
                        //stNeighbourCell.dbgoverride = vec3(1, 1, 1);
                        otherantahead = true;
                        if(
                                stOtherRoadAnt.turnskip == 0
                            &&  stOtherRoadAnt.justmade == 0
                        ) {
                            doturnskip = true;
                            break;
                        }
                    }
                }
                if(doturnskip) {
                    stRoadAnt.turnskip++;
                    continue;
                }
            }
        }
        int leftcount  = checkNeighbours(neighbours, stRoadAnt.pos+dir2vec(turndir(stRoadAnt.dir, -1)))-1;
        int rightcount = checkNeighbours(neighbours, stRoadAnt.pos+dir2vec(turndir(stRoadAnt.dir,  1)))-1;
        int bestturn = 0, bestcount = aheadcount;
        if(leftcount < bestcount || (leftcount == bestcount && randrange(0,1) == 0)) {
            bestturn = -1; bestcount = leftcount;
        }
        if(rightcount < bestcount || (rightcount == bestcount && randrange(0,1) == 0)) {
            bestturn = 1; bestcount = rightcount;
        }
        if(aheadcount >= 2 && bestcount < aheadcount) {
            // may cause strange roads ahead, turn if better
            if(bestcount < aheadcount) {
                stRoadAnt.dir = turndir(stRoadAnt.dir,  bestturn);
                stRoadAnt.turncooldown = iRoadAntTurnCooldown;
                stRoadAnt.justmade = 2;
                //stRoadAnt.dbg = vec3(1, 1, 0);
            }
            //stRoadAnt.dbg = vec3(0, 1, 0);
        /*} else if(aheadcount < 2 && ahead.size() > 0 && cellAt(ahead[0]).type == ROAD) {
            if(otherantahead) { printf("hmm.. %d\n", __LINE__); }
            // road ahead, don't change direction (so we can join it)
            //stRoadAnt.dbg = vec3(0, 1, 1);
        } else if(left.size() > 0 && cellAt(left[0]).type == ROAD) {
            // road left, turn
            stRoadAnt.dir = turndir(stRoadAnt.dir, -1);
            stRoadAnt.turncooldown = iRoadAntTurnCooldown;
            stRoadAnt.justmade = 2;
        } else if(right.size() > 0 && cellAt(right[0]).type == ROAD) {
            // road right, turn
            stRoadAnt.dir = turndir(stRoadAnt.dir,  1);
            stRoadAnt.turncooldown = iRoadAntTurnCooldown;
            stRoadAnt.justmade = 2;*/
        } else if(stRoadAnt.turncooldown > 0) {
            stRoadAnt.turncooldown--;
        } else if(otherantahead) {
            // other ant ahead, don't turn or branch
            stRoadAnt.dbg = vec3(0, 1, 0);
            //printf("Found me an ant!\n");
            
        } else if(number_rand() < fRoadAntTurnChance) {
            stRoadAnt.dir = turndir(stRoadAnt.dir, randrange(0,1) == 0 ? -1 : 1);
            stRoadAnt.turncooldown = iRoadAntTurnCooldown;
            stRoadAnt.justmade = 2;
        } else if(number_rand() < fRoadAntBranchChance) {
            int branches = randrange(1, 3);
            if((branches & 1) != 0) { // branch left
                CRoadAnt stNewRoadAnt;
                
                stNewRoadAnt.done = false;
                stNewRoadAnt.justmade = 2;
                stNewRoadAnt.pos = stRoadAnt.pos;
                stNewRoadAnt.dir = turndir(stRoadAnt.dir, -1);
                stNewRoadAnt.turncooldown = iRoadAntTurnCooldown+1;
                stNewRoadAnt.turnskip = 0;
                
                clstNewRoadAnts.push_back(stNewRoadAnt);
                
                stRoadAnt.justmade = 2;
                iActiveAntCount++;
            }
            if((branches & 2) != 0) { // branch right
                CRoadAnt stNewRoadAnt;
                
                stNewRoadAnt.done = false;
                stNewRoadAnt.justmade = 2;
                stNewRoadAnt.pos = stRoadAnt.pos;
                stNewRoadAnt.dir = turndir(stRoadAnt.dir, 1);
                stNewRoadAnt.turncooldown = iRoadAntTurnCooldown+1;
                stNewRoadAnt.turnskip = 0;
                
                clstNewRoadAnts.push_back(stNewRoadAnt);
                
                stRoadAnt.justmade = 2;
                iActiveAntCount++;
            }
            stRoadAnt.turncooldown = iRoadAntTurnCooldown;
        }
    }
    m_clstRoadAnts.insert(m_clstRoadAnts.end(), clstNewRoadAnts.begin(), clstNewRoadAnts.end());
    //m_clstRoadAnts.empty();
    highlightCells();
    
    if(iActiveAntCount == 0) {
        m_clstRoadAnts.clear();
        
        m_clFreeCells.clear();
        for(int y = 0; y < m_iGridH; y++) {
            for(int x = 0; x < m_iGridW; x++) {
                CCell &stCell = cellAt(vec3(x, y, 0));
                if(stCell.type == ROAD) {
                    m_clFreeCells.push_back(vec3(x, y, 0));
                }
            }
        }
        std::random_shuffle(m_clFreeCells.begin(), m_clFreeCells.end());
        
        return (EInitStage)(ROAD_GEN_STEP+1);
    }
    return ROAD_GEN_STEP;
}
CWorld::EInitStage CWorld::roadGenBuild(double &rfSignificance) {
    rfSignificance = 0.2;
    bool bComplete = true;
    
    for(int i = 0; i < 20 && m_clFreeCells.size() > 0; i++) {
        vec3 foundPos = m_clFreeCells.back();
        CCell &stCell = cellAt(foundPos);
        stCell.show = false;
        bComplete = false;
        
        makeRoad(stCell, foundPos);
        
        // if(stCell.roadtype == STRAIGHT) {
            // m_clNavNodes.resize(m_clNavNodes.size()+1);
            // CNavNode &stLeftNode = m_clNavNodes[m_clNavNodes.size()-1];
            // stLeftNode.pos = ;
        // }
        
        if(stCell.roadtype == STRAIGHT && number_rand() < fCarPlacementChance) {
            m_clGeneratedProps.push_back(
                &((*(new CCar(m_pEngine, this, foundPos)))
                    .setLabel("car")
                )
            );
        }
        
        m_clFreeCells.pop_back();
    }
    
    if(bComplete) {
        //return DONE;
        return (EInitStage)(ROAD_GEN_BUILD+1);
    }
    return ROAD_GEN_BUILD;
}
void CWorld::makeRoad(CCell &stCell, vec3 vCellPos) {
    vec3 pos = cellPos(vCellPos);
    double cw = m_iCellW, ch = m_iCellH;
    
    std::vector<EDirection> roaddirs;
    for(int diroffset = 0; diroffset < 4; diroffset++) {
        EDirection dir = turndir(UP, diroffset);
        std::vector<vec3> neighbours;
        checkDirection(neighbours, vCellPos, dir, 1);
        if(neighbours.size() >= 1 && cellAt(neighbours[0]).type == ROAD) {
            roaddirs.push_back(dir);
        }
    }
    
    m_clGeneratedProps.push_back(
        &((*(new CPropBuilding(m_pEngine, ELotType::STREET, cw, ch, roaddirs, vec3(1, 1, 1), cellAt(vCellPos))))
            .setLabel("building")
            .setPosition(pos)
            .setRotation(quat())//quat::random())
            .tween(pos+vec3(0, -20, 0), pos, 0.3+number_rand()*0.5, CEntity::ETweenType::LINEAR)
        )
    );
}

CWorld::CCell &CWorld::cellAt(vec3 pos) {
    if(
            (int)pos.x < 0
        ||  (int)pos.x > m_iGridW-1
        ||  (int)pos.y < 0
        ||  (int)pos.y > m_iGridH-1
    ) {
        throw "Bad cellAt args";
    }
    return m_aastGrid[(int)pos.y*m_iGridH+(int)pos.x];
}
vec3 CWorld::cellPos(vec3 pos) {
    double cw = m_iCellW, ch = m_iCellH;
    double xmin = -cw*m_iGridW/2; double xmax = cw*m_iGridW/2;
    double ymin = -ch*m_iGridH/2; double ymax = ch*m_iGridH/2;
    //xmin += cw/2.0; ymin += ch/2.0;
    return vec3(xmin+cw*pos.x, 0, ymin+ch*pos.y);
}
int CWorld::checkDirection(std::vector<vec3> &outvec, vec3 pos, EDirection dir, int len, int start, bool includeEmpty) {
    int closest = 9999;
    for(int i = start; i <= len; i++) {
        pos = pos+dir2vec(dir);
        
        if(
                (int)pos.x < 0
            ||  (int)pos.x > m_iGridW-1
            ||  (int)pos.y < 0
            ||  (int)pos.y > m_iGridH-1
        ) {
            return closest;
        }
        
        CCell &stCell = cellAt(pos);
        //stCell.dbgfill = vec3(0.4, 0.4, 0.4);
        
        if(stCell.type != NONE || includeEmpty) {
            //stCell.dbgfill = vec3(0.9, 0.9, 0.9);
            outvec.push_back(vec3(pos.x, pos.y, 0));
            if(closest == 9999) {
                closest = i;
            }
        }
    }
    return closest;
}
int CWorld::checkNeighbours(std::vector<vec3> &outvec, vec3 pos, bool includeEmpty) {
    return (0
        +   (checkDirection(outvec, pos, UP   , 1, 1, includeEmpty) == 9999 ? 0 : 1)
        +   (checkDirection(outvec, pos, RIGHT, 1, 1, includeEmpty) == 9999 ? 0 : 1)
        +   (checkDirection(outvec, pos, DOWN , 1, 1, includeEmpty) == 9999 ? 0 : 1)
        +   (checkDirection(outvec, pos, LEFT , 1, 1, includeEmpty) == 9999 ? 0 : 1)
    );
}
void CWorld::checkArea(std::vector<vec3> &outvec, vec3 pos, int su, int sr, int sd, int sl, bool includeEmpty) {
    for(int y = std::max(0, (int)pos.y-su); y <= std::min((int)pos.y+sd, m_iGridH-1); y++) {
        for(int x = std::max(0, (int)pos.x-sl); x <= std::min((int)pos.x+sr, m_iGridW-1); x++) {
            CCell &stCell = cellAt(vec3(x, y, 0));
            
            if(stCell.type != NONE || includeEmpty) {
                //stCell.dbgfill = vec3(0.9, 0.9, 0.9);
                outvec.push_back(vec3(x, y, 0));
            }
        }
    }
}

void CWorld::clearCellHighlight(bool complete) {
    for(int y = 0; y < m_iGridH; y++) {
        for(int x = 0; x < m_iGridW; x++) {
            CCell &stCell = cellAt(vec3(x, y, 0));
            
            for(int edgei = 0; edgei < 4; edgei++) {
                stCell.edgecols[edgei] = vec3(0, 0, 0);
            }
            if(complete) {
                stCell.dbgoverride = vec3(0, 0, 0);
                stCell.dbgfill = vec3(0, 0, 0);
            }
        }
    }
}
void CWorld::highlightCells() {
    for(int y = 0; y < m_iGridH; y++) {
        for(int x = 0; x < m_iGridW; x++) {
            CCell &stCell = cellAt(vec3(x, y, 0));
            
            stCell.col = (
                    stCell.dbg != vec3(0, 0, 0) ? stCell.dbg
                :   stCell.type == ROAD ? vRoadCol
                :   vDefaultCol
            );
            stCell.dbg = vec3(0, 0, 0);
            
            stCell.fillcol = (
                    stCell.dbgfill != vec3(0, 0, 0) ? stCell.dbgfill
                :   vec3(0, 0, 0)
            );
            stCell.dbgfill = vec3(0, 0, 0);
        }
    }
    for(int y = 0; y < m_iGridH; y++) {
        for(int x = 0; x < m_iGridW; x++) {
            CCell &stCell = cellAt(vec3(x, y, 0));
            
            stCell.dbgused = false;
            //printf("%d ", stCell.type);
            
            if(stCell.dbgoverride != vec3(0, 0, 0)) {
                stCell.col = stCell.dbgoverride;
            } else {
                for(auto itRoadAnt = m_clstRoadAnts.begin(); itRoadAnt != m_clstRoadAnts.end(); itRoadAnt++) {
                    CRoadAnt &stRoadAnt = *itRoadAnt;
                    // printf("%f, %f == %f, %f\n",
                        // m_astRoadAnts[iRoadAntNum].pos.x, (double)x,
                        // m_astRoadAnts[iRoadAntNum].pos.y, (double)y
                    // );
                    if(     number_eq(stRoadAnt.pos.x, (double)x)
                        &&  number_eq(stRoadAnt.pos.y, (double)y)
                    ) {
                        if(stRoadAnt.dbg != vec3(0, 0, 0)) {
                            stCell.col = stRoadAnt.dbg;
                            stCell.dbgused = true;
                        } else if(stCell.dbgused) {
                            // 
                        } else if(!stRoadAnt.done) {
                            stCell.col = vec3(1, 0, 0);
                            stCell.edgecols[stRoadAnt.dir] = vec3(0, 1, 1);
                        } else {
                            stCell.col = vec3(0.1, 0.1, 0.8);
                        }
                    }
                }
                for(auto itLot = m_clstLots.begin(); itLot != m_clstLots.end(); itLot++) {
                    CLot &stLot = *itLot;
                    if(     (stLot.tl.x < x || number_eq(stLot.tl.x, (double)x))
                        &&  (stLot.tl.y < y || number_eq(stLot.tl.y, (double)y))
                        &&  (stLot.br.x > x || number_eq(stLot.br.x, (double)x))
                        &&  (stLot.br.y > y || number_eq(stLot.br.y, (double)y))
                    ) {
                        if(stLot.built) {
                            stCell.show = false;
                        }
                        if(stCell.dbgused) {
                            // 
                        } else {
                            stCell.col = stLot.dbg;
                            stCell.edgecols[UP   ] = (number_eq(stLot.tl.y, (double)y) ? stLot.dbg : vec3(0.01, 0.01, 0.01));
                            stCell.edgecols[RIGHT] = (number_eq(stLot.br.x, (double)x) ? stLot.dbg : vec3(0.01, 0.01, 0.01));
                            stCell.edgecols[DOWN ] = (number_eq(stLot.br.y, (double)y) ? stLot.dbg : vec3(0.01, 0.01, 0.01));
                            stCell.edgecols[LEFT ] = (number_eq(stLot.tl.x, (double)x) ? stLot.dbg : vec3(0.01, 0.01, 0.01));
                        }
                    }
                }
            }
        }
        //printf("\n");
    }
    for(int y = 0; y < m_iGridH; y++) {
        for(int x = 0; x < m_iGridW; x++) {
            CCell &stCell = cellAt(vec3(x, y, 0));
            
            for(int edgei = 0; edgei < 4; edgei++) {
                if(stCell.edgecols[edgei] == vec3(0, 0, 0)) {
                    stCell.edgecols[edgei] = stCell.col;
                }
            }
        }
    }
}


bool CWorld::calculateExtents(vec3 &vExtentsMin, vec3 &vExtentsMax) const {
    double cw = m_iCellW, ch = m_iCellH;
    vExtentsMin = vec3(-cw*m_iGridW/2,  0.1, -cw*m_iGridH/2);
    vExtentsMax = vec3( cw*m_iGridW/2, -0.1,  cw*m_iGridH/2);
    return true;
}
