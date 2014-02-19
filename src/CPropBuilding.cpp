
class CPropBuilding;
#include "CPropBuilding.h"
class CEngine;
#include "CEngine.h"

CPropBuilding::CPropBuilding(
    CEngine *pEngine,
    CWorld::ELotType eBuildingType,
    double fWidth, double fLength,
    std::vector<CWorld::EDirection> clRoadDirs,
    vec3 vCol,
    CWorld::CCell &stCell
) : CProp(pEngine, NULL)
{
    setModel(generateModel(eBuildingType, fWidth, fLength, clRoadDirs, vCol, stCell));
    
    //m_fOpacity = number_rand();
    m_pCell = &stCell;
}
CPropBuilding::~CPropBuilding() {
    
}

std::pair<CWorld::EDirection, quat> frontdir(std::vector<CWorld::EDirection> clRoadDirs) {
    CWorld::EDirection iFrontDir = turndir(CWorld::EDirection::UP, randrange(0, 3));
    if(!clRoadDirs.empty()) {
        iFrontDir = clRoadDirs[0];
    }
    quat dir = (
            iFrontDir == CWorld::EDirection::UP    ? (quat(M_RAD(-90*0), vec3(0, 1, 0)))
        :   iFrontDir == CWorld::EDirection::RIGHT ? (quat(M_RAD(-90*1), vec3(0, 1, 0)))
        :   iFrontDir == CWorld::EDirection::DOWN  ? (quat(M_RAD(-90*2), vec3(0, 1, 0)))
        :   iFrontDir == CWorld::EDirection::LEFT  ? (quat(M_RAD(-90*3), vec3(0, 1, 0)))
        :   quat()
    );
    return std::make_pair(iFrontDir, dir);
}

void CPropBuilding::_render(
    CRenderer *pRenderer,
    double fDeltaTime, double fDeltaClock,
    CEntity::ERenderMode eMode,
    int iDetailLevel
) {
    std::stringstream dbgss;
    dbgss <<  "PF=" << (m_pCell->occupied_positive_first  ? "O" : "") << (m_pCell->reserved_positive_first  ? "R" : "");
    dbgss << " PS=" << (m_pCell->occupied_positive_second ? "O" : "") << (m_pCell->reserved_positive_second ? "R" : "");
    dbgss << " NF=" << (m_pCell->occupied_negative_first  ? "O" : "") << (m_pCell->reserved_negative_first  ? "R" : "");
    dbgss << " NS=" << (m_pCell->occupied_negative_second ? "O" : "") << (m_pCell->reserved_negative_second ? "R" : "");
    dbgtext = std::string(dbgss.str());
    
    if(eMode != CEntity::ERenderMode::NORMAL && eMode != CEntity::ERenderMode::TRANSPARENT) return;
    
    GLfloat aMatAmbient [] = {
        (float)number_rand(), (float)number_rand(), (float)number_rand(), (float)number_rand()
    };
    GLfloat aMatDiffuse [] = /*{
        (float)number_rand(), (float)number_rand(), (float)number_rand(), (float)number_rand()
    };*/{ 0.0, 0.0, 0.0, 1.0 };
    GLfloat aMatSpecular[] = /*{
        (float)number_rand(), (float)number_rand(), (float)number_rand(), (float)number_rand()
    };*/{ 0.0, 0.0, 0.0, 1.0 };
    GLfloat aMatEmission[] = /*{
        (float)number_rand(), (float)number_rand(), (float)number_rand(), (float)number_rand()
    };*/{ 0.0, 0.0, 0.0, 1.0 };
    GLfloat aMatShininess[] = { (float)number_rand()*128 };
    
    GL_CHECK( glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT , aMatAmbient ) );
    GL_CHECK( glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE , aMatDiffuse ) );
    GL_CHECK( glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, aMatSpecular) );
    GL_CHECK( glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, aMatEmission) );
    GL_CHECK( glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, aMatShininess) );
    
    CProp::_render(pRenderer, fDeltaTime, fDeltaClock, eMode, iDetailLevel);
}

double swheight = 0.38;

void CPropBuilding::generateSimpleBuilding(
    CModel &stModel,
    CWorld::ELotType eBuildingType,
    double fWidth, double fLength,
    std::vector<CWorld::EDirection> clRoadDirs,
    vec3 vCol,
    CWorld::CCell &stCell,
    bool bJustPavement
) {
    
    std::vector<std::string> buildingtextures = m_pEngine->getRenderer()->findTexture("building*.jpg");
    std::string &buildingtexture = buildingtextures[randrange(0, buildingtextures.size()-1)];
    std::vector<std::string> rooftextures = m_pEngine->getRenderer()->findTexture("buildingroof*.jpg");
    std::string &rooftexture = rooftextures[randrange(0, rooftextures.size()-1)];
    std::vector<std::string> pavementtextures = m_pEngine->getRenderer()->findTexture("floor*.jpg");
    std::string &pavementtexture = pavementtextures[randrange(0, pavementtextures.size()-1)];
    
    double walltexscale = 0.1/4;
    double rooftexscale = 0.10/4;
    double pvtexscale = 0.4/4;
    
    auto front = frontdir(clRoadDirs);
    CWorld::EDirection iFrontDir = front.first;
    quat dir = front.second;
    
    vec3 o_r = dir.right(), o_u = dir.up(), o_b = dir.back();
    vec3 o_l = -o_r       , o_d = -o_u    , o_f = -o_b      ;
    
    double border = 0+number_rand()*std::min(fWidth, fLength)/2.8;
    double height = 4+number_rand()*80;
    
    double wallvalue = 0.7+number_rand()*0.3;
    vec3 wallcol = vec3(wallvalue*0.9+number_rand()*0.1, wallvalue*0.9+number_rand()*0.1, wallvalue*0.9+number_rand()*0.1);
    double roofvalue = 0.3+number_rand()*0.7;
    vec3 roofcol = vec3(roofvalue*0.9+number_rand()*0.1, roofvalue*0.9+number_rand()*0.1, roofvalue*0.9+number_rand()*0.1);
    double pvvalue = 0.3+number_rand()*0.7;
    vec3 pvcol = vec3(pvvalue*0.9+number_rand()*0.1, pvvalue*0.9+number_rand()*0.1, pvvalue*0.9+number_rand()*0.1);
    
    //printf("--\n");
    vec3 s = vec3(fWidth-border*2, height, fLength-border*2);
    //s.print();
    s = dir.vecmul(s).abs();
    // s.print();
    
    // double rgb[3];
    // hsvToRgb(number_rand(), 0.7, 0.6, rgb);
    vec3 col = vec3(1, 1, 1);//vCol;//vec3(rgb[0], rgb[1], rgb[2]);
    
    vec3 o = vec3(border, swheight, border)+(
            iFrontDir == CWorld::EDirection::UP    ? vec3(0, 0, 0)
        :   iFrontDir == CWorld::EDirection::RIGHT ? vec3(s.z, 0, 0)
        :   iFrontDir == CWorld::EDirection::DOWN  ? vec3(s.x, 0, s.z)
        :   iFrontDir == CWorld::EDirection::LEFT  ? vec3(0, 0, s.x)
        :   vec3(0, -20, 0)
    );
    
    vec3 v_rub = o+(s.x*o_r + s.y*o_u + s.z*o_b), c_rub = col;//vec3(1, 1, 1);
    vec3 v_ruf = o+(s.x*o_r + s.y*o_u +   0*o_f), c_ruf = col;//vec3(1, 1, 0);
    vec3 v_rdb = o+(s.x*o_r +   0*o_d + s.z*o_b), c_rdb = col;//vec3(1, 0, 1);
    vec3 v_rdf = o+(s.x*o_r +   0*o_d +   0*o_f), c_rdf = col;//vec3(1, 0, 0);
    vec3 v_lub = o+(  0*o_l + s.y*o_u + s.z*o_b), c_lub = col;//vec3(0, 1, 1);
    vec3 v_luf = o+(  0*o_l + s.y*o_u +   0*o_f), c_luf = col;//vec3(0, 1, 0);
    vec3 v_ldb = o+(  0*o_l +   0*o_d + s.z*o_b), c_ldb = col;//vec3(0, 0, 1);
    vec3 v_ldf = o+(  0*o_l +   0*o_d +   0*o_f), c_ldf = col;//vec3(0, 0, 0);
    
    int detailmax = 10;
    int detailstep = 2;
    for(int detail = 0; detail <= detailmax; detail += detailstep) {
        
        int subdivide = detail/2;
        int levelmin = detail;
        int levelmax = (detail == detailmax ? 9999 : detail+detailstep-1);
        
        stModel.m_clShapes.resize(stModel.m_clShapes.size()+(bJustPavement ? 1 : 3));
        
        CShape &stPavementShape = stModel.m_clShapes[stModel.m_clShapes.size()-1];
        CShape &stWallShape     = stModel.m_clShapes[stModel.m_clShapes.size()-2]; // !!
        CShape &stRoofShape     = stModel.m_clShapes[stModel.m_clShapes.size()-3]; // !!
        
        stPavementShape.m_clMaterialNames.push_back(pavementtexture);
        stPavementShape.m_iDetailLevelMin = levelmin;
        stPavementShape.m_iDetailLevelMax = levelmax;
        CMesh &stPvMesh = stPavementShape.m_stMesh;
        
        if(!bJustPavement) {
            stWallShape.m_clMaterialNames.push_back(buildingtexture);
            stWallShape.m_iDetailLevelMin = levelmin;
            stWallShape.m_iDetailLevelMax = levelmax;
            CMesh &stWallMesh = stWallShape.m_stMesh;
            
            stRoofShape.m_clMaterialNames.push_back(rooftexture);
            stRoofShape.m_iDetailLevelMin = levelmin;
            stRoofShape.m_iDetailLevelMax = levelmax;
            CMesh &stRoofMesh = stRoofShape.m_stMesh;
            
            // right face
            size_t fi_r_tl = stWallMesh.pushVertex(v_rub, o_r, vec3(0*s.z*walltexscale, 0*height*walltexscale, 0), wallcol);//c_rub);
            size_t fi_r_tr = stWallMesh.pushVertex(v_ruf, o_r, vec3(1*s.z*walltexscale, 0*height*walltexscale, 0), wallcol);//c_ruf);
            size_t fi_r_br = stWallMesh.pushVertex(v_rdf, o_r, vec3(1*s.z*walltexscale, 1*height*walltexscale, 0), wallcol);//c_rdf);
            size_t fi_r_bl = stWallMesh.pushVertex(v_rdb, o_r, vec3(0*s.z*walltexscale, 1*height*walltexscale, 0), wallcol);//c_rdb);
            stWallMesh.pushQuad(fi_r_tl, fi_r_tr, fi_r_br, fi_r_bl, true, subdivide);
            // left face
            size_t fi_l_tl = stWallMesh.pushVertex(v_luf, o_l, vec3(0*s.z*walltexscale, 0*height*walltexscale, 0), wallcol);//, c_luf);
            size_t fi_l_tr = stWallMesh.pushVertex(v_lub, o_l, vec3(1*s.z*walltexscale, 0*height*walltexscale, 0), wallcol);//, c_lub);
            size_t fi_l_br = stWallMesh.pushVertex(v_ldb, o_l, vec3(1*s.z*walltexscale, 1*height*walltexscale, 0), wallcol);//, c_ldb);
            size_t fi_l_bl = stWallMesh.pushVertex(v_ldf, o_l, vec3(0*s.z*walltexscale, 1*height*walltexscale, 0), wallcol);//, c_ldf);
            stWallMesh.pushQuad(fi_l_tl, fi_l_tr, fi_l_br, fi_l_bl, true, subdivide);
            // up face
            size_t fi_u_tl = stRoofMesh.pushVertex(v_luf, o_u, vec3(0*s.x*rooftexscale, 0*s.z*rooftexscale, 0), roofcol);//, c_luf);
            size_t fi_u_tr = stRoofMesh.pushVertex(v_ruf, o_u, vec3(1*s.x*rooftexscale, 0*s.z*rooftexscale, 0), roofcol);//, c_ruf);
            size_t fi_u_br = stRoofMesh.pushVertex(v_rub, o_u, vec3(1*s.x*rooftexscale, 1*s.z*rooftexscale, 0), roofcol);//, c_rub);
            size_t fi_u_bl = stRoofMesh.pushVertex(v_lub, o_u, vec3(0*s.x*rooftexscale, 1*s.z*rooftexscale, 0), roofcol);//, c_lub);
            stRoofMesh.pushQuad(fi_u_tl, fi_u_tr, fi_u_br, fi_u_bl, true, subdivide);
            // down face
            // size_t fi_d_tl = stWallMesh.pushVertex(v_rdf, o_d, vec3(0, 0, 0), c_rdf);
            // size_t fi_d_tr = stWallMesh.pushVertex(v_ldf, o_d, vec3(1, 0, 0), c_ldf);
            // size_t fi_d_br = stWallMesh.pushVertex(v_ldb, o_d, vec3(1, 1, 0), c_ldb);
            // size_t fi_d_bl = stWallMesh.pushVertex(v_rdb, o_d, vec3(0, 1, 0), c_rdb);
            // stWallMesh.pushQuad(fi_d_tl, fi_d_tr, fi_d_br, fi_d_bl);
            // back face
            size_t fi_b_tl = stWallMesh.pushVertex(v_lub, o_b, vec3(0*s.x*walltexscale, 0*height*walltexscale, 0), wallcol);//, c_lub);
            size_t fi_b_tr = stWallMesh.pushVertex(v_rub, o_b, vec3(1*s.x*walltexscale, 0*height*walltexscale, 0), wallcol);//, c_rub);
            size_t fi_b_br = stWallMesh.pushVertex(v_rdb, o_b, vec3(1*s.x*walltexscale, 1*height*walltexscale, 0), wallcol);//, c_rdb);
            size_t fi_b_bl = stWallMesh.pushVertex(v_ldb, o_b, vec3(0*s.x*walltexscale, 1*height*walltexscale, 0), wallcol);//, c_ldb);
            stWallMesh.pushQuad(fi_b_tl, fi_b_tr, fi_b_br, fi_b_bl, true, subdivide);
            // front face
            size_t fi_f_tl = stWallMesh.pushVertex(v_ruf, o_f, vec3(0*s.x*walltexscale, 0*height*walltexscale, 0), wallcol);//, c_ruf);
            size_t fi_f_tr = stWallMesh.pushVertex(v_luf, o_f, vec3(1*s.x*walltexscale, 0*height*walltexscale, 0), wallcol);//, c_luf);
            size_t fi_f_br = stWallMesh.pushVertex(v_ldf, o_f, vec3(1*s.x*walltexscale, 1*height*walltexscale, 0), wallcol);//, c_ldf);
            size_t fi_f_bl = stWallMesh.pushVertex(v_rdf, o_f, vec3(0*s.x*walltexscale, 1*height*walltexscale, 0), wallcol);//, c_rdf);
            stWallMesh.pushQuad(fi_f_tl, fi_f_tr, fi_f_br, fi_f_bl, true, subdivide);
        }
        // pavement face
        size_t fi_pv_tl = stPvMesh.pushVertex(vec3(0     , swheight, 0      ), o_u, vec3(0*fWidth*pvtexscale, 0*fLength*pvtexscale, 0), pvcol);
        size_t fi_pv_tr = stPvMesh.pushVertex(vec3(fWidth, swheight, 0      ), o_u, vec3(1*fWidth*pvtexscale, 0*fLength*pvtexscale, 0), pvcol);
        size_t fi_pv_br = stPvMesh.pushVertex(vec3(fWidth, swheight, fLength), o_u, vec3(1*fWidth*pvtexscale, 1*fLength*pvtexscale, 0), pvcol);
        size_t fi_pv_bl = stPvMesh.pushVertex(vec3(0     , swheight, fLength), o_u, vec3(0*fWidth*pvtexscale, 1*fLength*pvtexscale, 0), pvcol);
        stPvMesh.pushQuad(fi_pv_tl, fi_pv_tr, fi_pv_br, fi_pv_bl, true, subdivide);
    }
}

void CPropBuilding::generateRoad(
    CModel &stModel,
    CWorld::ELotType eBuildingType,
    double fWidth, double fLength,
    std::vector<CWorld::EDirection> clRoadDirs,
    vec3 vCol,
    CWorld::CCell &stCell
) {
    std::swap(fWidth, fLength); // *sigh*
    //vec3 col = vCol;
    vec3 col = vec3(0.5, 0.5, 0.5);
    vec3 pvcol = vec3(0.85, 0.85, 0.85);
    
    double swsize = 2.5;
    double swtexscale = 0.25;
    
    vec3 road_tl = vec3(0      , 0, swsize);
    vec3 road_tr = vec3(fLength, 0, swsize);
    vec3 road_br = vec3(fLength, 0, fWidth-swsize);
    vec3 road_bl = vec3(0      , 0, fWidth-swsize);
    
    vec3 sw_dbl = road_tl;
    vec3 sw_ubl = road_tl+vec3(0, swheight, 0);
    vec3 sw_utl = sw_ubl+vec3(0, 0, -swsize);
    vec3 sw_dbr = road_tr;
    vec3 sw_ubr = road_tr+vec3(0, swheight, 0);
    vec3 sw_utr = sw_ubr+vec3(0, 0, -swsize);
    
    int detailmax = 10;
    int detailstep = 2;
    for(int detail = 0; detail <= detailmax; detail += detailstep) {
        
        int subdivide = detail/4;
        int levelmin =  detail;
        int levelmax = (detail == detailmax ? 9999 : detail+detailstep-1);
        
        stModel.m_clShapes.resize(stModel.m_clShapes.size()+2);
        CShape &stRoadShape     = stModel.m_clShapes[stModel.m_clShapes.size()-1];
        CShape &stSidewalkShape = stModel.m_clShapes[stModel.m_clShapes.size()-2];
        
        stRoadShape.m_clMaterialNames.push_back("road.jpg");
        stRoadShape.m_iDetailLevelMin = levelmin;
        stRoadShape.m_iDetailLevelMax = levelmax;
        CMesh &stRoadMesh = stRoadShape.m_stMesh;
        
        stSidewalkShape.m_clMaterialNames.push_back("sidewalk.jpg");
        stSidewalkShape.m_iDetailLevelMin = levelmin;
        stSidewalkShape.m_iDetailLevelMax = levelmax;
        CMesh &stSidewalkMesh = stSidewalkShape.m_stMesh;
        
        stRoadShape.m_vRotationOrigin = vec3(fLength/2, 0, fWidth/2);
        stSidewalkShape.m_vRotationOrigin = vec3(fLength/2, 0, fWidth/2);
        
        stRoadShape.m_qRotation = quat(0, vec3(0, 1, 0));
        stSidewalkShape.m_qRotation = quat(0, vec3(0, 1, 0));
        
        if(
               (clRoadDirs.size() == 1) // on edge of grid
            || (clRoadDirs.size() == 2 && turndir(clRoadDirs[0], 2) == clRoadDirs[1])
        ) {
            // straight road
            std::sort(clRoadDirs.begin(), clRoadDirs.end());
            
            stCell.roadtype = (clRoadDirs.size() == 1) ? CWorld::ERoadType::TOTHEVOID : CWorld::ERoadType::STRAIGHT;
            stCell.straight_up = (clRoadDirs[0] == CWorld::EDirection::UP);
            
            if(clRoadDirs[0] == CWorld::EDirection::UP || clRoadDirs[0] == CWorld::EDirection::DOWN) {
                stRoadShape.m_qRotation = stSidewalkShape.m_qRotation = quat(M_RAD(90), vec3(0, 1, 0));
            }
            
            size_t fi_road_tl = stRoadMesh.pushVertex(road_tl, vec3(0, 1, 0), vec3(0, 0, 0), col);
            size_t fi_road_tr = stRoadMesh.pushVertex(road_tr, vec3(0, 1, 0), vec3(1, 0, 0), col);
            size_t fi_road_br = stRoadMesh.pushVertex(road_br, vec3(0, 1, 0), vec3(1, 1, 0), col);
            size_t fi_road_bl = stRoadMesh.pushVertex(road_bl, vec3(0, 1, 0), vec3(0, 1, 0), col);
            // road surface
            stRoadMesh.pushQuad(fi_road_tl, fi_road_tr, fi_road_br, fi_road_bl, true, subdivide);
            
            for(int i = 0; i < 2; i++) {
                vec3 o = (i == 0 ? vec3(0, 0, 0) : vec3(0, 0, fWidth));
                vec3 s = (i == 0 ? vec3(1, 1, 1) : vec3(1, 1, -1));
                size_t fi_sw_dbl = stSidewalkMesh.pushVertex(o+s*sw_dbl,s*vec3(0,0, 1),vec3(0,-swheight*swtexscale,0),pvcol);
                size_t fi_sw_ubl = stSidewalkMesh.pushVertex(o+s*sw_ubl,s*vec3(0,1, 0),vec3(0,0,0),pvcol);
                size_t fi_sw_utl = stSidewalkMesh.pushVertex(o+s*sw_utl,s*vec3(0,1, 0),vec3(0,-swsize*swtexscale,0),pvcol);
                size_t fi_sw_dbr = stSidewalkMesh.pushVertex(o+s*sw_dbr,s*vec3(0,0, 1),vec3(fWidth*swtexscale,-swheight*swtexscale,0),pvcol);
                size_t fi_sw_ubr = stSidewalkMesh.pushVertex(o+s*sw_ubr,s*vec3(0,1, 0),vec3(fWidth*swtexscale,0,0),pvcol);
                size_t fi_sw_utr = stSidewalkMesh.pushVertex(o+s*sw_utr,s*vec3(0,1, 0),vec3(fWidth*swtexscale,-swsize*swtexscale,0),pvcol);
                // top sidewalk face
                stSidewalkMesh.pushQuad(fi_sw_utl, fi_sw_utr, fi_sw_ubr, fi_sw_ubl, i == 0, subdivide);
                // vertical sidewalk face against road
                stSidewalkMesh.pushQuad(fi_sw_ubl, fi_sw_ubr, fi_sw_dbr, fi_sw_dbl, i == 0, subdivide);
            }
        } else if(clRoadDirs.size() == 2) {
            // corner road
            stCell.roadtype = CWorld::ERoadType::CORNER;
            std::sort(clRoadDirs.begin(), clRoadDirs.end());
            //col = vec3(1, 1, 1);
            
            quat dir = (
                    (clRoadDirs[0] == CWorld::EDirection::UP    && clRoadDirs[1] == CWorld::EDirection::RIGHT) ? (quat(M_RAD(90*2), vec3(0, 1, 0)))
                :   (clRoadDirs[0] == CWorld::EDirection::UP    && clRoadDirs[1] == CWorld::EDirection::LEFT ) ? (quat(M_RAD(90*3), vec3(0, 1, 0)))
                :   (clRoadDirs[0] == CWorld::EDirection::RIGHT && clRoadDirs[1] == CWorld::EDirection::DOWN ) ? (quat(M_RAD(90*1), vec3(0, 1, 0)))
                :   (clRoadDirs[0] == CWorld::EDirection::DOWN  && clRoadDirs[1] == CWorld::EDirection::LEFT ) ? (quat(M_RAD(90*0), vec3(0, 1, 0)))
                :   quat()
            );
            stRoadShape.m_qRotation = stSidewalkShape.m_qRotation = dir;
            
            int segmentcount = 2+((double)(detail/detailmax))*30.0;
            double swradius = swsize;
            double roadradius = fWidth-swsize;
            vec3 offset = vec3(0, 0, swsize);
            size_t fi_road_inner_prev, fi_road_outer_prev;
            size_t fi_swinner_innerup_prev, fi_swinner_outerup_prev, fi_swinner_outerdown_prev;
            size_t fi_swouter_innerup_prev, fi_swouter_innerdown_prev, fi_swouter_outerup_prev;
            for(int segment = 0; segment <= segmentcount; segment++) {
                double ang = M_RAD(90*segment/segmentcount);
                vec3 road_inner = vec3(sin(ang)*swradius, 0, cos(ang)*swradius);
                vec3 road_outer = vec3(sin(ang)*roadradius, 0, cos(ang)*roadradius);
                size_t fi_road_inner = stRoadMesh.pushVertex(road_inner, vec3(0, 1, 0), vec3((double)segment/(double)segmentcount, 0, 0), col);
                size_t fi_road_outer = stRoadMesh.pushVertex(road_outer, vec3(0, 1, 0), vec3((double)segment/(double)segmentcount, 1, 0), col);
                if(segment != 0) {
                    stRoadMesh.pushQuad(fi_road_inner, fi_road_outer, fi_road_outer_prev, fi_road_inner_prev, true, subdivide/2);
                }
                fi_road_inner_prev = fi_road_inner;
                fi_road_outer_prev = fi_road_outer;
                
                vec3 swinner_innerup   = vec3(0, swheight, 0);
                vec3 swinner_outerup   = vec3(sin(ang)*swradius, swheight, cos(ang)*swradius);
                vec3 swinner_outerdown = vec3(sin(ang)*swradius, 0       , cos(ang)*swradius);
                double segtex = (double)segment/(double)segmentcount;
                size_t fi_swinner_innerup   = stSidewalkMesh.pushVertex(swinner_innerup  , vec3(0, 1, 0), vec3(ang*swradius*swtexscale, 0, 0), pvcol);
                size_t fi_swinner_outerup   = stSidewalkMesh.pushVertex(swinner_outerup  , vec3(0, 1, 0), vec3(ang*swradius*swtexscale, swsize*swtexscale, 0), pvcol);
                size_t fi_swinner_outerdown = stSidewalkMesh.pushVertex(swinner_outerdown, vec3(0, 1, 0), vec3(swheight*swtexscale, 0, 0), pvcol);
                if(segment != 0) {
                    stSidewalkMesh.pushQuad(fi_swinner_innerup, fi_swinner_outerup, fi_swinner_outerup_prev, fi_swinner_innerup_prev, true, subdivide/2);
                    stSidewalkMesh.pushQuad(fi_swinner_outerdown, fi_swinner_outerup, fi_swinner_outerup_prev, fi_swinner_outerdown_prev, false, subdivide/2);
                }
                fi_swinner_innerup_prev   = fi_swinner_innerup;
                fi_swinner_outerup_prev   = fi_swinner_outerup;
                fi_swinner_outerdown_prev = fi_swinner_outerdown;
                
                vec3 swouter_innerup   = vec3(sin(ang)*roadradius, swheight, cos(ang)*roadradius);
                vec3 swouter_innerdown = vec3(sin(ang)*roadradius, 0       , cos(ang)*roadradius);
                vec3 swouter_outerup   = (
                        segment == 0            ? vec3(0      , swheight, fWidth)
                    :   segment == segmentcount ? vec3(fLength, swheight, 0     )
                    :                             vec3(fLength, swheight, fWidth)
                );
                segtex = (double)segment/(double)segmentcount;
                size_t fi_swouter_innerup   = stSidewalkMesh.pushVertex(swouter_innerup  , vec3(0, 1, 0), vec3(ang*roadradius*swtexscale, 0, 0), pvcol);
                size_t fi_swouter_innerdown = stSidewalkMesh.pushVertex(swouter_innerdown, vec3(0, 1, 0), vec3(swheight*swtexscale, 0, 0), pvcol);
                size_t fi_swouter_outerup   = stSidewalkMesh.pushVertex(swouter_outerup  , vec3(0, 1, 0), vec3(ang*roadradius*swtexscale, roadradius*swtexscale, 0), pvcol);
                if(segment != 0) {
                    stSidewalkMesh.pushQuad(fi_swouter_innerup, fi_swouter_outerup, fi_swouter_outerup_prev, fi_swouter_innerup_prev, true, subdivide/2);
                    stSidewalkMesh.pushQuad(fi_swouter_innerdown, fi_swouter_innerup, fi_swouter_innerup_prev, fi_swouter_innerdown_prev, true, subdivide/2);
                }
                fi_swouter_innerup_prev   = fi_swouter_innerup;
                fi_swouter_innerdown_prev = fi_swouter_innerdown;
                fi_swouter_outerup_prev   = fi_swouter_outerup;
            }
        } else if(clRoadDirs.size() == 3) {
            // T-section
            stCell.roadtype = CWorld::ERoadType::TSECTION;
            //col = vec3(1, 1, 1);
            stRoadShape.m_clMaterialNames[0] = "roadintersection.jpg";
            
            std::sort(clRoadDirs.begin(), clRoadDirs.end());
            CWorld::EDirection a = clRoadDirs[0], b = clRoadDirs[1], c = clRoadDirs[2];
            quat dir = (
                    (a == CWorld::EDirection::UP    && b == CWorld::EDirection::RIGHT && c == CWorld::EDirection::LEFT) ? (quat(M_RAD(90*3), vec3(0, 1, 0)))
                :   (a == CWorld::EDirection::UP    && b == CWorld::EDirection::RIGHT && c == CWorld::EDirection::DOWN) ? (quat(M_RAD(90*2), vec3(0, 1, 0)))
                :   (a == CWorld::EDirection::RIGHT && b == CWorld::EDirection::DOWN  && c == CWorld::EDirection::LEFT) ? (quat(M_RAD(90*1), vec3(0, 1, 0)))
                :   (a == CWorld::EDirection::UP    && b == CWorld::EDirection::DOWN  && c == CWorld::EDirection::LEFT) ? (quat(M_RAD(90*0), vec3(0, 1, 0)))
                :   quat()
            );
            stRoadShape.m_qRotation = stSidewalkShape.m_qRotation = dir;
            
            size_t fi_road_tl = stRoadMesh.pushVertex(vec3(0      , 0, 0     ), vec3(0, 1, 0), vec3(0, 0, 0), col);
            size_t fi_road_tr = stRoadMesh.pushVertex(vec3(fLength, 0, 0     ), vec3(0, 1, 0), vec3(1, 0, 0), col);
            size_t fi_road_br = stRoadMesh.pushVertex(vec3(fLength, 0, fWidth), vec3(0, 1, 0), vec3(1, 1, 0), col);
            size_t fi_road_bl = stRoadMesh.pushVertex(vec3(0      , 0, fWidth), vec3(0, 1, 0), vec3(0, 1, 0), col);
            // road surface
            stRoadMesh.pushQuad(fi_road_tl, fi_road_tr, fi_road_br, fi_road_bl, true, subdivide);
            
            int segmentcount = 2+((double)(detail/detailmax))*30.0;
            double swradius = swsize;
            double roadradius = fWidth-swsize;
            vec3 offset = vec3(0, 0, swsize);
            for(int corner = 0; corner < 2; corner++) {
                quat rot = quat(M_RAD(90*corner), vec3(0, 1, 0));
                vec3 o = (
                        corner == 0 ? vec3(0      , 0, 0     )
                    :   corner == 1 ? vec3(0      , 0, fWidth)
                    :   vec3(0, 0, 0)
                );
                size_t fi_road_inner_prev, fi_road_outer_prev;
                size_t fi_swinner_innerup_prev, fi_swinner_outerup_prev, fi_swinner_outerdown_prev;
                size_t fi_swouter_innerup_prev, fi_swouter_innerdown_prev, fi_swouter_outerup_prev;
                for(int segment = 0; segment <= segmentcount; segment++) {
                    double ang = M_RAD(90*segment/segmentcount);
                    
                    vec3 swinner_innerup   = o+rot.vecmul(vec3(0, swheight, 0));
                    vec3 swinner_outerup   = o+rot.vecmul(vec3(sin(ang)*swradius, swheight, cos(ang)*swradius));
                    vec3 swinner_outerdown = o+rot.vecmul(vec3(sin(ang)*swradius, 0       , cos(ang)*swradius));
                    double segtex = (double)segment/(double)segmentcount;
                    size_t fi_swinner_innerup   = stSidewalkMesh.pushVertex(swinner_innerup  , vec3(0, 1, 0), vec3(ang*swradius*swtexscale, 0, 0), pvcol);
                    size_t fi_swinner_outerup   = stSidewalkMesh.pushVertex(swinner_outerup  , vec3(0, 1, 0), vec3(ang*swradius*swtexscale, swsize*swtexscale, 0), pvcol);
                    size_t fi_swinner_outerdown = stSidewalkMesh.pushVertex(swinner_outerdown, vec3(0, 1, 0), vec3(swheight*swtexscale, 0, 0), pvcol);
                    if(segment != 0) {
                        stSidewalkMesh.pushQuad(fi_swinner_innerup, fi_swinner_outerup, fi_swinner_outerup_prev, fi_swinner_innerup_prev, true, subdivide/2);
                        stSidewalkMesh.pushQuad(fi_swinner_outerdown, fi_swinner_outerup, fi_swinner_outerup_prev, fi_swinner_outerdown_prev, false, subdivide/2);
                    }
                    fi_swinner_innerup_prev   = fi_swinner_innerup;
                    fi_swinner_outerup_prev   = fi_swinner_outerup;
                    fi_swinner_outerdown_prev = fi_swinner_outerdown;
                }
            }
            
            vec3 o = vec3(fLength, 0, 0);
            quat rot = quat(M_RAD(90*-1), vec3(0, 1, 0));
            size_t fi_sw_dbl = stSidewalkMesh.pushVertex(o+rot.vecmul(sw_dbl),vec3(0,0, 1),vec3(0,-swheight*swtexscale,0),pvcol);
            size_t fi_sw_ubl = stSidewalkMesh.pushVertex(o+rot.vecmul(sw_ubl),vec3(0,1, 0),vec3(0,0,0),pvcol);
            size_t fi_sw_utl = stSidewalkMesh.pushVertex(o+rot.vecmul(sw_utl),vec3(0,1, 0),vec3(0,-swsize*swtexscale,0),pvcol);
            size_t fi_sw_dbr = stSidewalkMesh.pushVertex(o+rot.vecmul(sw_dbr),vec3(0,0, 1),vec3(fWidth*swtexscale,-swheight*swtexscale,0),pvcol);
            size_t fi_sw_ubr = stSidewalkMesh.pushVertex(o+rot.vecmul(sw_ubr),vec3(0,1, 0),vec3(fWidth*swtexscale,0,0),pvcol);
            size_t fi_sw_utr = stSidewalkMesh.pushVertex(o+rot.vecmul(sw_utr),vec3(0,1, 0),vec3(fWidth*swtexscale,-swsize*swtexscale,0),pvcol);
            // top sidewalk face
            stSidewalkMesh.pushQuad(fi_sw_utl, fi_sw_utr, fi_sw_ubr, fi_sw_ubl, true, subdivide);
            // vertical sidewalk face against road
            stSidewalkMesh.pushQuad(fi_sw_ubl, fi_sw_ubr, fi_sw_dbr, fi_sw_dbl, true, subdivide);
        } else if(clRoadDirs.size() == 4) {
            // four-way intersection
            stCell.roadtype = CWorld::ERoadType::CROSSROADS;
            //col = vec3(1, 1, 1);
            stRoadShape.m_clMaterialNames[0] = "roadintersection.jpg";
            
            std::sort(clRoadDirs.begin(), clRoadDirs.end());
            CWorld::EDirection a = clRoadDirs[0], b = clRoadDirs[1], c = clRoadDirs[2];
            quat dir = (
                    (a == CWorld::EDirection::UP    && b == CWorld::EDirection::RIGHT && c == CWorld::EDirection::LEFT) ? (quat(M_RAD(90*2), vec3(0, 1, 0)))
                :   (a == CWorld::EDirection::UP    && b == CWorld::EDirection::RIGHT && c == CWorld::EDirection::DOWN) ? (quat(M_RAD(90*3), vec3(0, 1, 0)))
                :   (a == CWorld::EDirection::RIGHT && b == CWorld::EDirection::DOWN  && c == CWorld::EDirection::LEFT) ? (quat(M_RAD(90*1), vec3(0, 1, 0)))
                :   (a == CWorld::EDirection::UP    && b == CWorld::EDirection::DOWN  && c == CWorld::EDirection::LEFT) ? (quat(M_RAD(90*0), vec3(0, 1, 0)))
                :   quat()
            );
            stRoadShape.m_qRotation = stSidewalkShape.m_qRotation = dir;
            
            size_t fi_road_tl = stRoadMesh.pushVertex(vec3(0      , 0, 0     ), vec3(0, 1, 0), vec3(0, 0, 0), col);
            size_t fi_road_tr = stRoadMesh.pushVertex(vec3(fLength, 0, 0     ), vec3(0, 1, 0), vec3(1, 0, 0), col);
            size_t fi_road_br = stRoadMesh.pushVertex(vec3(fLength, 0, fWidth), vec3(0, 1, 0), vec3(1, 1, 0), col);
            size_t fi_road_bl = stRoadMesh.pushVertex(vec3(0      , 0, fWidth), vec3(0, 1, 0), vec3(0, 1, 0), col);
            // road surface
            stRoadMesh.pushQuad(fi_road_tl, fi_road_tr, fi_road_br, fi_road_bl, true, subdivide);
            
            int segmentcount = 2+((double)(detail/detailmax))*30.0;
            double swradius = swsize;
            double roadradius = fWidth-swsize;
            vec3 offset = vec3(0, 0, swsize);
            for(int corner = 0; corner < 4; corner++) {
                quat rot = quat(M_RAD(90*corner), vec3(0, 1, 0));
                vec3 o = (
                        corner == 0 ? vec3(0      , 0, 0     )
                    :   corner == 1 ? vec3(0      , 0, fWidth)
                    :   corner == 2 ? vec3(fLength, 0, fWidth)
                    :   corner == 3 ? vec3(fLength, 0, 0     )
                    :   vec3(0, 0, 0)
                );
                size_t fi_road_inner_prev, fi_road_outer_prev;
                size_t fi_swinner_innerup_prev, fi_swinner_outerup_prev, fi_swinner_outerdown_prev;
                size_t fi_swouter_innerup_prev, fi_swouter_innerdown_prev, fi_swouter_outerup_prev;
                for(int segment = 0; segment <= segmentcount; segment++) {
                    double ang = M_RAD(90*segment/segmentcount);
                    
                    vec3 swinner_innerup   = o+rot.vecmul(vec3(0, swheight, 0));
                    vec3 swinner_outerup   = o+rot.vecmul(vec3(sin(ang)*swradius, swheight, cos(ang)*swradius));
                    vec3 swinner_outerdown = o+rot.vecmul(vec3(sin(ang)*swradius, 0       , cos(ang)*swradius));
                    double segtex = (double)segment/(double)segmentcount;
                    size_t fi_swinner_innerup   = stSidewalkMesh.pushVertex(swinner_innerup  , vec3(0, 1, 0), vec3(ang*swradius*swtexscale, 0, 0), pvcol);
                    size_t fi_swinner_outerup   = stSidewalkMesh.pushVertex(swinner_outerup  , vec3(0, 1, 0), vec3(ang*swradius*swtexscale, swsize*swtexscale, 0), pvcol);
                    size_t fi_swinner_outerdown = stSidewalkMesh.pushVertex(swinner_outerdown, vec3(0, 1, 0), vec3(swheight*swtexscale, 0, 0), pvcol);
                    if(segment != 0) {
                        stSidewalkMesh.pushQuad(fi_swinner_innerup, fi_swinner_outerup, fi_swinner_outerup_prev, fi_swinner_innerup_prev, true, subdivide/2);
                        stSidewalkMesh.pushQuad(fi_swinner_outerdown, fi_swinner_outerup, fi_swinner_outerup_prev, fi_swinner_outerdown_prev, false, subdivide/2);
                    }
                    fi_swinner_innerup_prev   = fi_swinner_innerup;
                    fi_swinner_outerup_prev   = fi_swinner_outerup;
                    fi_swinner_outerdown_prev = fi_swinner_outerdown;
                }
            }
        } else {
            // ?!?
        }
    }
}

struct modelpair {
    const char *mdl;
    double scale;
    double chance;
    bool shouldrotate;
};

struct modelpair modelpairs[] = {
    {"statue.obj"           ,  5.0   , 0.30, true },
    {"teapot2.obj"          ,  3.0   , 0.05, true },
    {"windmill02.obj"       ,  0.008 , 0.30, false},
  //{"Home.obj"             ,  0.1   , 0.20, true },
    {"Observatory.obj"      ,  0.017 , 0.20, true },
    {"Rotunda.obj"          ,  0.002 , 0.12, true },
    {"Rabbit.obj"           , 10.5   , 0.20, true },
  //{"eifel.obj"            ,  0.10  , 0.10, true },
    {"court.obj"            ,  0.011 , 0.06, false},
    {"MONUMENTO_CRISTO.obj" ,  0.3   , 0.01, false}
};

void CPropBuilding::generateStatue(
    CModel &stModel, CWorld::ELotType eBuildingType,
    double fWidth, double fLength, std::vector<CWorld::EDirection> clRoadDirs,
    vec3 vCol, CWorld::CCell &stCell
) {
    auto front = frontdir(clRoadDirs);
    CWorld::EDirection iFrontDir = front.first;
    quat dir = front.second;
    
    const char *modelname = "cube.obj";
    double scale = 3;
    bool shouldrotate = false;
    
    double totalchance = 0;
    for(int i = 0; i < sizeof(modelpairs)/sizeof(*modelpairs); i++) {
        totalchance += modelpairs[i].chance;
    }
    
    double choice = number_rand()*totalchance;
    double v = 0;
    for(int i = 0; i < sizeof(modelpairs)/sizeof(*modelpairs); i++) {
        v += modelpairs[i].chance;
        if(v >= choice) {
            modelname = modelpairs[i].mdl;
            scale = modelpairs[i].scale;
            shouldrotate = modelpairs[i].shouldrotate;
            break;
        }
    }
    
    dbgtext = modelname;
    
    CModel *pLoadedModel = CModel::loadModel(modelname, scale, false, 7, 9999);
    
    quat rot = shouldrotate ? quat(M_RAD(number_rand()*360), vec3(0, 1, 0)) : quat();
    
    auto &clShapes = pLoadedModel->m_clShapes;
    for(auto itShape = clShapes.begin(); itShape != clShapes.end(); itShape++) {
        CShape &stShape = *itShape;
        
        stModel.m_clShapes.push_back(stShape);
        CShape &stLoadedShape = stModel.m_clShapes[stModel.m_clShapes.size()-1];
        
        stLoadedShape.m_vOffset = vec3(0, 0, 0);
        stLoadedShape.m_qRotation = rot;
    }
    
    delete pLoadedModel;
}

CModel *CPropBuilding::generateModel(
    CWorld::ELotType eBuildingType,
    double fWidth, double fLength,
    std::vector<CWorld::EDirection> clRoadDirs,
    vec3 vCol,
    CWorld::CCell &stCell
) {
    CModel *pModel = new CModel();
    
    if(eBuildingType == CWorld::ELotType::BUILDING) {
        bool filled = false;
        if(fWidth < 40 || fLength < 40) {
            if(number_rand() < 0.5) {
                
                generateStatue(*pModel, eBuildingType, fWidth, fLength, clRoadDirs, vCol, stCell);
                
                filled = true;
            }
        }
        if(filled) {
            number fMinX = std::numeric_limits<number>::infinity(), fMaxX = -std::numeric_limits<number>::infinity();
            number fMinY = std::numeric_limits<number>::infinity(), fMaxY = -std::numeric_limits<number>::infinity();
            number fMinZ = std::numeric_limits<number>::infinity(), fMaxZ = -std::numeric_limits<number>::infinity();
            
            auto &clShapes = pModel->m_clShapes;
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
                stShape.m_vOffset = vec3(fWidth/2, 0, fLength/2)+stShape.m_vOffset+vec3(-(fMinX+fMaxX)/2, -fMinY+swheight, -(fMinZ+fMaxZ)/2);
                stShape.m_vRotationOrigin = vec3((fMinX+fMaxX)/2, 0, (fMinZ+fMaxZ)/2);
            }
            
            CModel *pCubeModel = CModel::makeCube("metal.jpg", vec3(14, 16, 14), vec3(0.7, 0.7, 0.7));
            pModel->m_clShapes.push_back(pCubeModel->m_clShapes[0]);
            pModel->m_clShapes[pModel->m_clShapes.size()-1].m_iDetailLevelMin = 0;
            pModel->m_clShapes[pModel->m_clShapes.size()-1].m_iDetailLevelMax = 6;
            pModel->m_clShapes[pModel->m_clShapes.size()-1].m_vOffset = vec3(fWidth/2, 0, fLength/2);
            delete pCubeModel;
            //pModel = pCubeModel;
            
            // pavement
            generateSimpleBuilding(*pModel, eBuildingType, fWidth, fLength, clRoadDirs, vCol, stCell, true);
            
        } else {
            generateSimpleBuilding(*pModel, eBuildingType, fWidth, fLength, clRoadDirs, vCol, stCell);
            m_bEnableTransparency = true;
        }
    } else if(eBuildingType == CWorld::ELotType::STREET) {
        generateRoad(*pModel, eBuildingType, fWidth, fLength, clRoadDirs, vCol, stCell);
    }
    
    return pModel;
    
}
