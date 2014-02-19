
class CCar;
#include "CCar.h"
class CRenderer;
#include "CRenderer.h"
class CEngine;
#include "CEngine.h"
class CPlayer;
#include "CPlayer.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <cstring>

struct carmodel {
    const char *mdl;
    double scale;
    double chance;
    double delay;
    quat rotation;
};

struct carmodel carmodels[] = {
    {"natla_car.obj"         , 1.00 , 1.0, 0.70, quat()},
    {"Jeep.obj"              , 0.75 , 0.6, 0.80, quat()},
  //{"Ford.obj"              , 0.75 , 1.0, 0.70, 180},
    {"Porsche_911_GT2.obj"   , 1.60 , 0.6, 0.55, quat(M_RAD(180), vec3(0, 1, 0))},
  //{"VH-BumbleBee.obj"      , 0.016, 0.6, 0.70, quat::fromEuler(90, 90, 90)},
    {"taxi_cab.obj"          , 0.032, 0.6, 0.65, quat(M_RAD(180), vec3(0, 1, 0))},
  //{"Citroen_C4.obj"        , 0.032, 9.6, 0.70, quat()},
    {"harry_car.obj"         , 1.1  , 0.6, 0.65, quat()},
    {"copcar.obj"            , 1.3  , 0.6, 0.60, quat(M_RAD(180), vec3(0, 1, 0))},
};

CCar::CCar(
    CEngine *pEngine, CWorld *pWorld, vec3 vCurrentCellPos
) : CProp(pEngine, NULL),
    m_vCurrentCellPos(vCurrentCellPos)
{
    m_pWorld = pWorld;
    
    const char *modelname = "cube.obj";
    double scale = 3;
    double delay = 3;
    quat rotation = quat();
    
    double totalchance = 0;
    for(int i = 0; i < sizeof(carmodels)/sizeof(*carmodels); i++) {
        totalchance += carmodels[i].chance;
    }
    
    double choice = number_rand()*totalchance;
    double v = 0;
    for(int i = 0; i < sizeof(carmodels)/sizeof(*carmodels); i++) {
        v += carmodels[i].chance;
        if(v >= choice) {
            modelname = carmodels[i].mdl;
            scale = carmodels[i].scale;
            delay = carmodels[i].delay;
            rotation = carmodels[i].rotation;
            break;
        }
    }
    m_bIsCopCar = (strcmp(modelname, "copcar.obj") == 0);
    if(m_bIsCopCar) {
        m_fCopLightsOffset = number_rand()*10;
        m_fCopLightsRate = 0.8+number_rand()*0.4;
    }
    
    setModel(CModel::loadModel(modelname, scale, false, 6, 9999));
    //setModel(CModel::makeCube("", vec3(1, 1, 1), vec3(1, 1, 1)));
    // setModel(CModel::loadModel("statue.obj", 1, false, 7, 9999));
    if(m_pModel != NULL) {
        number fMinX = std::numeric_limits<number>::infinity(), fMaxX = -std::numeric_limits<number>::infinity();
        number fMinY = std::numeric_limits<number>::infinity(), fMaxY = -std::numeric_limits<number>::infinity();
        number fMinZ = std::numeric_limits<number>::infinity(), fMaxZ = -std::numeric_limits<number>::infinity();
        
        auto &clShapes = m_pModel->m_clShapes;
        for(auto itShape = clShapes.begin(); itShape != clShapes.end(); itShape++) {
            CShape &stShape = *itShape;
            CMesh &stMesh = stShape.m_stMesh;
            for(auto itVtxPos = stMesh.m_clVertexPositions.begin(); itVtxPos != stMesh.m_clVertexPositions.end(); itVtxPos++) {
                vec3 &vVtxPos = *itVtxPos;
                fMinX = std::min(fMinX, vVtxPos.x); fMaxX = std::max(fMaxX, vVtxPos.x);
                fMinY = std::min(fMinY, vVtxPos.y); fMaxY = std::max(fMaxY, vVtxPos.y);
                fMinZ = std::min(fMinZ, vVtxPos.z); fMaxZ = std::max(fMaxZ, vVtxPos.z);
            }
        }
        for(auto itShape = clShapes.begin(); itShape != clShapes.end(); itShape++) {
            CShape &stShape = *itShape;
            stShape.m_vOffset = stShape.m_vOffset+vec3(-(fMinX+fMaxX)/2, -fMinY+0.05, -(fMinZ+fMaxZ)/2);
            stShape.m_vRotationOrigin = vec3((fMinX+fMaxX)/2, 0, (fMinZ+fMaxZ)/2);
            stShape.m_qRotation = rotation;
        }
        
        vec3 vExtentsMin, vExtentsMax;
        calculateExtents(vExtentsMin, vExtentsMax);
        
        double rgb[3];
        hsvToRgb(number_rand(), 0.7, 0.6, rgb);
        
        CModel *pCubeModel = CModel::makeCube("metal.jpg", vec3(vExtentsMax.x*2.1, vExtentsMax.y*2.1, vExtentsMax.z*2.1), vec3(rgb[0], rgb[1], rgb[2]));
        m_pModel->m_clShapes.push_back(pCubeModel->m_clShapes[0]);
        m_pModel->m_clShapes[m_pModel->m_clShapes.size()-1].m_iDetailLevelMin = 0;
        m_pModel->m_clShapes[m_pModel->m_clShapes.size()-1].m_iDetailLevelMax = 5;
        
        delete pCubeModel;
    }
    
    m_fDelay = delay;
    m_eState = INIT;
}
CCar::~CCar() {
}
void CCar::_onChange() {
    CEntity::_onChange();
}

void CCar::orientCar(
    bool moving, double fraction,
    vec3 curr, bool currfirst,
    vec3 dest, bool destfirst,
    bool fade
) {
    double w = m_pWorld->m_iCellW, l = m_pWorld->m_iCellH;
    double s = 5.5;
    
    CWorld::CCell &stCell = m_pWorld->cellAt(curr);
    
    double wp = w/s;
    double lp = l/s;
    
    double mp = (currfirst ? l/4 : -l/4);
    vec3 currpos = (
            m_pWorld->cellPos(curr)
        +   vec3(w/2, 0, l/2)
        +   (
                    m_eCurrentDir == CWorld::EDirection::UP    ? (vec3( wp, 0, 0 )+vec3( 0    , 0,-mp))
                :   m_eCurrentDir == CWorld::EDirection::RIGHT ? (vec3( 0 , 0,-lp)+vec3(-mp, 0, 0    ))
                :   m_eCurrentDir == CWorld::EDirection::DOWN  ? (vec3(-wp, 0, 0 )+vec3( 0    , 0, mp))
                :   m_eCurrentDir == CWorld::EDirection::LEFT  ? (vec3( 0 , 0, lp)+vec3( mp, 0, 0    ))
                :   vec3(0, 0, 0)
            )
    );
    quat currrot = quat(M_RAD(-90*m_eCurrentDir), vec3(0, 1, 0));
    
    mp = (destfirst ? l/4 : -l/4);
    vec3 destpos = (
            m_pWorld->cellPos(dest)
        +   vec3(w/2, 0, l/2)
        +   (
                    m_eDestinationDir == CWorld::EDirection::UP    ? (vec3( wp, 0, 0 )+vec3( 0    , 0,-mp))
                :   m_eDestinationDir == CWorld::EDirection::RIGHT ? (vec3( 0 , 0,-lp)+vec3(-mp, 0, 0    ))
                :   m_eDestinationDir == CWorld::EDirection::DOWN  ? (vec3(-wp, 0, 0 )+vec3( 0    , 0, mp))
                :   m_eDestinationDir == CWorld::EDirection::LEFT  ? (vec3( 0 , 0, lp)+vec3( mp, 0, 0    ))
                :   vec3(0, 0, 0)
            )
    );
    quat destrot = quat(M_RAD(-90*m_eDestinationDir), vec3(0, 1, 0));
    
    vec3 lerppos;
    if(m_eCurrentDir == m_eDestinationDir) {
        lerppos = currpos.lerp(destpos, fraction);
    } else {
        vec3 anchor = vec3(
            (       (m_eCurrentDir == CWorld::EDirection::UP   | m_eCurrentDir == CWorld::EDirection::DOWN ) ? currpos.x
                :   destpos.x
            ),
            0,
            (       (m_eCurrentDir == CWorld::EDirection::LEFT | m_eCurrentDir == CWorld::EDirection::RIGHT) ? currpos.z
                :   destpos.z
            )
        );
        lerppos = (pow((1.0-fraction), 2.0)*currpos) +(2.0*(1.0-fraction)*fraction*anchor) +(pow(fraction, 2.0)*destpos);
    }
    
    setPosition(lerppos);
    setRotation(currrot.slerp(destrot, fraction));
    if(fade) {
        m_fOpacity = 1-fraction;
    }
}

void CCar::_think(
    double fTime, double fDeltaTime
) {
    CWorld::CCell &stCurrentCell = m_pWorld->cellAt(m_vCurrentCellPos);
    if(stCurrentCell.type != CWorld::ECellType::ROAD) {
        dbgtext = "OH NO!";
        if(m_eState != DEAD) {
            printf("oh no!\n");
            m_eState = DEAD;
        }
    }
    // if(!m_pEngine->getButtonState("Step Cars", "group='Main' key=d")) {
        // return;
    // }
    bool currpositive = (m_eCurrentDir == CWorld::EDirection::UP || m_eCurrentDir == CWorld::EDirection::RIGHT);
    double movefraction = 0;
    std::stringstream dbgss;
    
    switch(m_eState) {
        case INIT: {
            dbgss << "Init";
            bool valid = false;
            CWorld::EDirection dirorig = turndir(CWorld::EDirection::UP, randrange(0, 3));
            CWorld::EDirection dir;
            for(int diroffset = 0; diroffset < 4; diroffset++) {
                dir = turndir(dirorig, diroffset);
                std::vector<vec3> neighbours;
                m_pWorld->checkDirection(neighbours, m_vCurrentCellPos, dir, 1);
                if(neighbours.size() > 0 && m_pWorld->cellAt(neighbours[0]).type == CWorld::ECellType::ROAD) {
                    valid = true;
                    break;
                }
            }
            if(valid) {
                m_eCurrentDir = dir;
                m_bCurrentFirst = true;
                m_bAtIntersection = false;
                m_bWasBlockingIntersection = false;
                m_fLastMoveTime = 0;
                
                currpositive = (m_eCurrentDir == CWorld::EDirection::UP || m_eCurrentDir == CWorld::EDirection::RIGHT);
                if(currpositive) {
                    stCurrentCell.occupied_positive_first = true;
                } else {
                    stCurrentCell.occupied_negative_first = true;
                }
                
                m_eState = DECIDING;
                
            } else {
                m_eState = DEAD;
                printf("Car in bad spot!\n");
                return;
            }
            //break;
        }
        case DECIDING: {
            dbgss << "Deciding";
            
            vec3 &pos = m_vCurrentCellPos;
            bool intersection = false;
            m_bAtIntersection = false;
            if(m_bCurrentFirst) {
                std::vector<vec3> ahead;  CWorld::EDirection aheaddir  = m_eCurrentDir;
                m_pWorld->checkDirection(ahead, pos, aheaddir, 2);
                
                if(ahead.size() == 0 || m_pWorld->cellAt(ahead[0]).type != CWorld::ECellType::ROAD) {
                    
                } else {
                    m_eDestinationDir = m_eCurrentDir;
                    m_vDestinationCellPos = pos;
                    m_bDestinationFirst = false;
                    m_bAtIntersection = false;
                    m_eState = WAITING;
                }
            }
            if(m_eState == DECIDING) {
                std::vector<std::pair<CWorld::EDirection, vec3>> destchoices;
                
                std::vector<vec3> ahead;  CWorld::EDirection aheaddir  = m_eCurrentDir;
                m_pWorld->checkDirection(ahead, pos, aheaddir, 2, 1, true);
                
                if(ahead.size() == 0 || m_pWorld->cellAt(ahead[0]).type != CWorld::ECellType::ROAD) {
                    // Somehow, we ended up in the middle of an intersection
                    std::vector<vec3> left ; CWorld::EDirection leftdir  = turndir(aheaddir, -1);
                    std::vector<vec3> right; CWorld::EDirection rightdir = turndir(aheaddir,  1);
                    
                    m_pWorld->checkDirection(left , pos, leftdir , 1);
                    m_pWorld->checkDirection(right, pos, rightdir, 1);
                    
                    if(left.size() > 0 && m_pWorld->cellAt(left[0]).type == CWorld::ECellType::ROAD) {
                        destchoices.push_back(std::make_pair(leftdir , pos+dir2vec(leftdir )));
                    }
                    if(right.size() > 0 && m_pWorld->cellAt(right[0]).type == CWorld::ECellType::ROAD) {
                        destchoices.push_back(std::make_pair(rightdir, pos+dir2vec(rightdir)));
                    }
                } else if(m_pWorld->cellAt(ahead[0]).type == CWorld::ECellType::ROAD) {
                    std::vector<vec3> aheadleft ; CWorld::EDirection leftdir  = turndir(aheaddir, -1);
                    std::vector<vec3> aheadright; CWorld::EDirection rightdir = turndir(aheaddir,  1);
                    
                    m_pWorld->checkDirection(aheadleft , pos+dir2vec(aheaddir), leftdir , 1);
                    m_pWorld->checkDirection(aheadright, pos+dir2vec(aheaddir), rightdir, 1);
                    
                    if(aheadleft.size() >= 1 && m_pWorld->cellAt(aheadleft[0]).type == CWorld::ECellType::ROAD) {
                        intersection = true;
                        destchoices.push_back(std::make_pair(leftdir , pos+dir2vec(aheaddir)+dir2vec(leftdir)));
                    }
                    if(aheadright.size() >= 1 && m_pWorld->cellAt(aheadright[0]).type == CWorld::ECellType::ROAD) {
                        intersection = true;
                        destchoices.push_back(std::make_pair(rightdir, pos+dir2vec(aheaddir)+dir2vec(rightdir)));
                    }
                    
                    if(intersection) {
                        if(
                                ahead.size() >= 2
                            &&  m_pWorld->cellAt(ahead[0]).type == CWorld::ECellType::ROAD
                            &&  m_pWorld->cellAt(ahead[1]).type == CWorld::ECellType::ROAD
                        ) {
                            destchoices.push_back(std::make_pair(aheaddir, pos+dir2vec(aheaddir)+dir2vec(aheaddir)));
                        }
                        m_bAtIntersection = true;
                    } else {
                        dbgss << " ahead";
                        destchoices.push_back(std::make_pair(aheaddir, pos+dir2vec(aheaddir)));
                        m_bAtIntersection = false;
                    }
                }
                
                dbgss << " " << destchoices.size();
                
                if(destchoices.size() > 0) {
                    std::random_shuffle(destchoices.begin(), destchoices.end());
                    
                    m_eDestinationDir     = destchoices[0].first;
                    m_vDestinationCellPos = destchoices[0].second;
                    m_bDestinationFirst   = true;
                    //m_vCurrentCellPos.print("m_vCurrentCellPos");
                    //m_vDestinationCellPos.print("m_eDestinationDir");
                    
                    m_eState = WAITING;
                    
                } else {
                    // Dead end!
                    m_eDestinationDir     = turndir(m_eCurrentDir, 2);
                    m_vDestinationCellPos = m_vCurrentCellPos;
                    m_bDestinationFirst   = true;
                    
                    m_eState = WAITING;
                    //m_eState = DEAD;
                }
            }
            if(m_eState != WAITING) { break; }
        }
        case WAITING: {
            dbgss << "Waiting";
            bool canmove = false;
            bool stopmove = false;
            
            CWorld::CCell &stDestinationCell = m_pWorld->cellAt(m_vDestinationCellPos);
            bool destpositive = (m_eDestinationDir == CWorld::EDirection::UP || m_eDestinationDir == CWorld::EDirection::RIGHT);
            
            if(m_bAtIntersection) {
                try {
                    CWorld::CCell &stIntersection = m_pWorld->cellAt(m_vCurrentCellPos+dir2vec(m_eCurrentDir));
                    // yay bruteforce
                    if(m_eCurrentDir == CWorld::EDirection::DOWN) {
                        if(m_eDestinationDir == CWorld::EDirection::DOWN) {
                            stopmove = stIntersection.busy_lu || stIntersection.busy_ld;
                        } else if(m_eDestinationDir == CWorld::EDirection::LEFT) {
                            stopmove = stIntersection.busy_ld;
                        } else if(m_eDestinationDir == CWorld::EDirection::RIGHT) {
                            stopmove = stIntersection.busy_ld || stIntersection.busy_ru || stIntersection.busy_mm;
                        }
                    } else if(m_eCurrentDir == CWorld::EDirection::LEFT) {
                        if(m_eDestinationDir == CWorld::EDirection::LEFT) {
                            stopmove = stIntersection.busy_rd || stIntersection.busy_ld;
                        } else if(m_eDestinationDir == CWorld::EDirection::UP) {
                            stopmove = stIntersection.busy_rd;
                        } else if(m_eDestinationDir == CWorld::EDirection::DOWN) {
                            stopmove = stIntersection.busy_rd || stIntersection.busy_lu || stIntersection.busy_mm;
                        }
                    } else if(m_eCurrentDir == CWorld::EDirection::RIGHT) {
                        if(m_eDestinationDir == CWorld::EDirection::RIGHT) {
                            stopmove = stIntersection.busy_ru || stIntersection.busy_lu;
                        } else if(m_eDestinationDir == CWorld::EDirection::DOWN) {
                            stopmove = stIntersection.busy_lu;
                        } else if(m_eDestinationDir == CWorld::EDirection::UP) {
                            stopmove = stIntersection.busy_lu || stIntersection.busy_rd || stIntersection.busy_mm;
                        }
                    } else if(m_eCurrentDir == CWorld::EDirection::UP) {
                        if(m_eDestinationDir == CWorld::EDirection::UP) {
                            stopmove = stIntersection.busy_ru || stIntersection.busy_rd;
                        } else if(m_eDestinationDir == CWorld::EDirection::RIGHT) {
                            stopmove = stIntersection.busy_ru;
                        } else if(m_eDestinationDir == CWorld::EDirection::LEFT) {
                            stopmove = stIntersection.busy_ru || stIntersection.busy_ld || stIntersection.busy_mm;
                        }
                    }
                    if(stDestinationCell.roadtype == CWorld::ERoadType::TSECTION || stDestinationCell.roadtype == CWorld::ERoadType::CROSSROADS) {
                        if(m_eDestinationDir == CWorld::EDirection::UP) {
                            stopmove = stDestinationCell.busy_ru || stDestinationCell.busy_rd;
                        } else if(m_eDestinationDir == CWorld::EDirection::RIGHT) {
                            stopmove = stDestinationCell.busy_lu || stDestinationCell.busy_ru;
                        } else if(m_eDestinationDir == CWorld::EDirection::DOWN) {
                            stopmove = stDestinationCell.busy_ld || stDestinationCell.busy_lu;
                        } else if(m_eDestinationDir == CWorld::EDirection::LEFT) {
                            stopmove = stDestinationCell.busy_rd || stDestinationCell.busy_ld;
                        }
                    }
                } catch(const char* c) {
                    
                }
            }
            if(!stopmove) {
                if(
                        !stDestinationCell.occupied(destpositive, m_bDestinationFirst)
                    &&  !stDestinationCell.reserved(destpositive, m_bDestinationFirst)
                ) {
                    stCurrentCell    .occupied(currpositive, m_bCurrentFirst    ) = false;
                    stDestinationCell.reserved(destpositive, m_bDestinationFirst) = true ;
                    canmove = true;
                }
            }
            if(canmove) {
                if(m_bAtIntersection) {
                    CWorld::CCell &stIntersection = m_pWorld->cellAt(m_vCurrentCellPos+dir2vec(m_eCurrentDir));
                    bool state = true;
                    bool shortturn = false;
                    if(m_eCurrentDir == CWorld::EDirection::DOWN) {
                        if(m_eDestinationDir == CWorld::EDirection::DOWN) {
                           stIntersection.busy_lu = stIntersection.busy_ld = state;
                        } else if(m_eDestinationDir == CWorld::EDirection::LEFT) {
                            stIntersection.busy_ld = state;
                            shortturn = true;
                        } else if(m_eDestinationDir == CWorld::EDirection::RIGHT) {
                            stIntersection.busy_ld = stIntersection.busy_ru = stIntersection.busy_mm = state;
                        }
                    } else if(m_eCurrentDir == CWorld::EDirection::LEFT) {
                        if(m_eDestinationDir == CWorld::EDirection::LEFT) {
                            stIntersection.busy_rd = stIntersection.busy_ld = state;
                        } else if(m_eDestinationDir == CWorld::EDirection::UP) {
                            stIntersection.busy_rd = state;
                            shortturn = true;
                        } else if(m_eDestinationDir == CWorld::EDirection::DOWN) {
                            stIntersection.busy_rd = stIntersection.busy_lu = stIntersection.busy_mm = state;
                        }
                    } else if(m_eCurrentDir == CWorld::EDirection::RIGHT) {
                        if(m_eDestinationDir == CWorld::EDirection::RIGHT) {
                            stIntersection.busy_ru = stIntersection.busy_lu = state;
                        } else if(m_eDestinationDir == CWorld::EDirection::DOWN) {
                            stIntersection.busy_lu = state;
                            shortturn = true;
                        } else if(m_eDestinationDir == CWorld::EDirection::UP) {
                            stIntersection.busy_lu = stIntersection.busy_rd = stIntersection.busy_mm = state;
                        }
                    } else if(m_eCurrentDir == CWorld::EDirection::UP) {
                        if(m_eDestinationDir == CWorld::EDirection::UP) {
                            stIntersection.busy_ru = stIntersection.busy_rd = state;
                        } else if(m_eDestinationDir == CWorld::EDirection::RIGHT) {
                            stIntersection.busy_ru = state;
                            shortturn = true;
                        } else if(m_eDestinationDir == CWorld::EDirection::LEFT) {
                            stIntersection.busy_ru = stIntersection.busy_ld = stIntersection.busy_mm = state;
                        }
                    }
                    if(stDestinationCell.roadtype == CWorld::ERoadType::TSECTION || stDestinationCell.roadtype == CWorld::ERoadType::CROSSROADS) {
                        if(m_eDestinationDir == CWorld::EDirection::UP) {
                            stDestinationCell.busy_ru = stDestinationCell.busy_rd = state;
                        } else if(m_eDestinationDir == CWorld::EDirection::RIGHT) {
                            stDestinationCell.busy_lu = stDestinationCell.busy_ru = state;
                        } else if(m_eDestinationDir == CWorld::EDirection::DOWN) {
                            stDestinationCell.busy_ld = stDestinationCell.busy_lu = state;
                        } else if(m_eDestinationDir == CWorld::EDirection::LEFT) {
                            stDestinationCell.busy_rd = stDestinationCell.busy_ld = state;
                        }
                    }
                    m_fMovingDuration = (shortturn ? 1.9 : 3)*m_fDelay;
                } else {
                    m_fMovingDuration = m_fDelay;
                }
                m_eState = MOVING;
                m_fMovingStartTime = fTime;
            } else {
                if(fTime > m_fLastMoveTime+5) {
                    // we're impatient!
                    m_eState = DECIDING;
                }
                break;
            }
        }
        case MOVING: {
            dbgss << "Moving";
            if(fTime > m_fMovingStartTime+m_fMovingDuration) {
                CWorld::CCell &stDestinationCell = m_pWorld->cellAt(m_vDestinationCellPos);
                bool destpositive = (m_eDestinationDir == CWorld::EDirection::UP || m_eDestinationDir == CWorld::EDirection::RIGHT);
                
                stDestinationCell.reserved(destpositive, m_bDestinationFirst) = false;
                stDestinationCell.occupied(destpositive, m_bDestinationFirst) = true ;
                
                if(m_bAtIntersection) {
                    CWorld::CCell &stIntersection = m_pWorld->cellAt(m_vCurrentCellPos+dir2vec(m_eCurrentDir));
                    bool state = false;
                    if(m_eCurrentDir == CWorld::EDirection::DOWN) {
                        if(m_eDestinationDir == CWorld::EDirection::DOWN) {
                           stIntersection.busy_lu = stIntersection.busy_ld = state;
                        } else if(m_eDestinationDir == CWorld::EDirection::LEFT) {
                            stIntersection.busy_ld = state;
                        } else if(m_eDestinationDir == CWorld::EDirection::RIGHT) {
                            stIntersection.busy_ld = stIntersection.busy_ru = stIntersection.busy_mm = state;
                        }
                    } else if(m_eCurrentDir == CWorld::EDirection::LEFT) {
                        if(m_eDestinationDir == CWorld::EDirection::LEFT) {
                            stIntersection.busy_rd = stIntersection.busy_ld = state;
                        } else if(m_eDestinationDir == CWorld::EDirection::UP) {
                            stIntersection.busy_rd = state;
                        } else if(m_eDestinationDir == CWorld::EDirection::DOWN) {
                            stIntersection.busy_rd = stIntersection.busy_lu = stIntersection.busy_mm = state;
                        }
                    } else if(m_eCurrentDir == CWorld::EDirection::RIGHT) {
                        if(m_eDestinationDir == CWorld::EDirection::RIGHT) {
                            stIntersection.busy_ru = stIntersection.busy_lu = state;
                        } else if(m_eDestinationDir == CWorld::EDirection::DOWN) {
                            stIntersection.busy_lu = state;
                        } else if(m_eDestinationDir == CWorld::EDirection::UP) {
                            stIntersection.busy_lu = stIntersection.busy_rd = stIntersection.busy_mm = state;
                        }
                    } else if(m_eCurrentDir == CWorld::EDirection::UP) {
                        if(m_eDestinationDir == CWorld::EDirection::UP) {
                            stIntersection.busy_ru = stIntersection.busy_rd = state;
                        } else if(m_eDestinationDir == CWorld::EDirection::RIGHT) {
                            stIntersection.busy_ru = state;
                        } else if(m_eDestinationDir == CWorld::EDirection::LEFT) {
                            stIntersection.busy_ru = stIntersection.busy_ld = stIntersection.busy_mm = state;
                        }
                    }
                }
                if(m_bWasBlockingIntersection) {
                    if(m_eCurrentDir == CWorld::EDirection::UP) {
                        stCurrentCell.busy_ru = stCurrentCell.busy_rd = false;
                    } else if(m_eCurrentDir == CWorld::EDirection::RIGHT) {
                        stCurrentCell.busy_lu = stCurrentCell.busy_ru = false;
                    } else if(m_eCurrentDir == CWorld::EDirection::DOWN) {
                        stCurrentCell.busy_ld = stCurrentCell.busy_lu = false;
                    } else if(m_eCurrentDir == CWorld::EDirection::LEFT) {
                        stCurrentCell.busy_rd = stCurrentCell.busy_ld = false;
                    }
                    m_bWasBlockingIntersection = false;
                }
                if(stDestinationCell.roadtype == CWorld::ERoadType::TSECTION || stDestinationCell.roadtype == CWorld::ERoadType::CROSSROADS) {
                    if(m_eDestinationDir == CWorld::EDirection::UP) {
                        stDestinationCell.busy_ru = stDestinationCell.busy_rd = true;
                    } else if(m_eDestinationDir == CWorld::EDirection::RIGHT) {
                        stDestinationCell.busy_lu = stDestinationCell.busy_ru = true;
                    } else if(m_eDestinationDir == CWorld::EDirection::DOWN) {
                        stDestinationCell.busy_ld = stDestinationCell.busy_lu = true;
                    } else if(m_eDestinationDir == CWorld::EDirection::LEFT) {
                        stDestinationCell.busy_rd = stDestinationCell.busy_ld = true;
                    }
                    m_bWasBlockingIntersection = true;
                } else {
                    m_bWasBlockingIntersection = false;
                }
                m_eCurrentDir = m_eDestinationDir;
                m_vCurrentCellPos = m_vDestinationCellPos;
                m_bCurrentFirst = m_bDestinationFirst;
                
                movefraction = 0;
                
                m_fLastMoveTime = fTime;
                
                m_eState = DECIDING;
            } else {
                movefraction = (fTime-m_fMovingStartTime)/m_fMovingDuration;
                if(m_bAtIntersection) {
                    CWorld::CCell &stIntersection = m_pWorld->cellAt(m_vCurrentCellPos+dir2vec(m_eCurrentDir));
                    if(m_eCurrentDir == CWorld::EDirection::DOWN) {
                        if(m_eDestinationDir == CWorld::EDirection::DOWN) {
                           stIntersection.busy_ld = (movefraction < 0.5);
                           stIntersection.busy_lu = (movefraction < 0.9);
                        } else if(m_eDestinationDir == CWorld::EDirection::LEFT) {
                            stIntersection.busy_ld = (movefraction < 0.8);
                        } else if(m_eDestinationDir == CWorld::EDirection::RIGHT) {
                            stIntersection.busy_ld = (movefraction < 0.5);
                            stIntersection.busy_mm = (movefraction < 0.7);
                            stIntersection.busy_ru = (movefraction < 0.9);
                        }
                    } else if(m_eCurrentDir == CWorld::EDirection::LEFT) {
                        if(m_eDestinationDir == CWorld::EDirection::LEFT) {
                            stIntersection.busy_rd = (movefraction < 0.5);
                            stIntersection.busy_ld = (movefraction < 0.9);
                        } else if(m_eDestinationDir == CWorld::EDirection::UP) {
                            stIntersection.busy_rd = (movefraction < 0.8);
                        } else if(m_eDestinationDir == CWorld::EDirection::DOWN) {
                            stIntersection.busy_rd = (movefraction < 0.5);
                            stIntersection.busy_mm = (movefraction < 0.7);
                            stIntersection.busy_lu = (movefraction < 0.9);
                        }
                    } else if(m_eCurrentDir == CWorld::EDirection::RIGHT) {
                        if(m_eDestinationDir == CWorld::EDirection::RIGHT) {
                            stIntersection.busy_ru = (movefraction < 0.5);
                            stIntersection.busy_lu = (movefraction < 0.9);
                        } else if(m_eDestinationDir == CWorld::EDirection::DOWN) {
                            stIntersection.busy_lu = (movefraction < 0.8);
                        } else if(m_eDestinationDir == CWorld::EDirection::UP) {
                            stIntersection.busy_lu = (movefraction < 0.5);
                            stIntersection.busy_mm = (movefraction < 0.7);
                            stIntersection.busy_rd = (movefraction < 0.9);
                        }
                    } else if(m_eCurrentDir == CWorld::EDirection::UP) {
                        if(m_eDestinationDir == CWorld::EDirection::UP) {
                            stIntersection.busy_ru = (movefraction < 0.5);
                            stIntersection.busy_rd = (movefraction < 0.9);
                        } else if(m_eDestinationDir == CWorld::EDirection::RIGHT) {
                            stIntersection.busy_ru = (movefraction < 0.8);
                        } else if(m_eDestinationDir == CWorld::EDirection::LEFT) {
                            stIntersection.busy_ru = (movefraction < 0.5);
                            stIntersection.busy_mm = (movefraction < 0.7);
                            stIntersection.busy_ld = (movefraction < 0.9);
                        }
                    }
                }
            }
            break;
        }
        case DEAD: {
            dbgss << "Dead";
            //remove();
            break;
        }
    }
    orientCar(
        (m_eState == MOVING), movefraction,
        m_vCurrentCellPos, m_bCurrentFirst,
        m_vDestinationCellPos, m_bDestinationFirst,
        false
    );
    dbgss << " " << (int)(10*movefraction);
    dbgss << " " << (m_bCurrentFirst ? "F" : "");
    dbgss << " " << (
            m_eCurrentDir == CWorld::EDirection::UP    ? "UP"
        :   m_eCurrentDir == CWorld::EDirection::RIGHT ? "RIGHT"
        :   m_eCurrentDir == CWorld::EDirection::DOWN  ? "DOWN"
        :   m_eCurrentDir == CWorld::EDirection::LEFT  ? "LEFT"
        :   "?!?"
    );
    dbgtext = std::string(dbgss.str());
}

void CCar::_render(
    CRenderer *pRenderer,
    double fDeltaTime, double fDeltaClock,
    CEntity::ERenderMode eMode,
    int iDetailLevel
) {
    if(eMode == CEntity::ERenderMode::LIGHTS) {
        if(!m_bIsCopCar) return;
        
        vec3 lightpos[2] = {vec3(-0.4, 2, 0), vec3(0.4, 2, 0)};
        vec4 lightcol[2] = {vec4(1, 0, 0, 1), vec4(0, 0, 1, 1)};
        
        int i = ((int)((m_pEngine->getTime()+m_fCopLightsOffset)*m_fCopLightsRate))%2;
        
        CRenderer::CLight light;
        light.directional = false;
        light.pos = lightpos[i];
        light.diffuse = lightcol[i]*5;
        light.ambient  = lightcol[i]*0.4;
        light.specular = lightcol[i];
        light.attConstant = 0.2;
        light.attLinear = 0;
        light.attQuadratic = 0.005;
        
        light.spotEnabled = false;
        // m_pEngine->getOption("Cars  - Lights Spot" , &light.spotEnabled, light.spotEnabled, "group='World'");
        // light.spotDirection = vec3(0, 0, 1);
        // light.spotExponent = 3.0;
        // light.spotCutoff = 45;
        
        pRenderer->light(light, this == m_pEngine->getPlayer()->m_pEntityToFollow);
        
        return;
    }
    
    if(eMode != CEntity::ERenderMode::NORMAL && eMode != CEntity::ERenderMode::TRANSPARENT) return;
    
    GLfloat aMatSpecular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat aMatShininess[] = { 50.0 };
    GL_CHECK( glMaterialfv(GL_FRONT, GL_SPECULAR, aMatSpecular) );
    GL_CHECK( glMaterialfv(GL_FRONT, GL_SHININESS, aMatShininess) );
    
    CProp::_render(pRenderer, fDeltaTime, fDeltaClock, eMode, iDetailLevel);
}

