#include <iostream>
#include <sstream>
#include <limits>
#include "tiny_obj_loader.h"

#include "model.h"

std::map<std::string, CModel*> g_clModelCache;

CModel::CModel(
    
) : //m_sName("?"),
    m_clShapes()
{
    std::stringstream ss;
    for(int i = 0; i < 20; i++) {
        ss << (char)randrange('a', 'z');
    }
    m_sName = ss.str();
    
    //printf("Model make: %s.\n", m_sName.c_str());
    deletedebug = false;
}
CModel::~CModel() {
    //printf("Model free: %s.\n", m_sName.c_str());
}

CModel *CModel::loadModel(
    const std::string &sModelName,
    double fScale, bool bWithColor,
    int iDetailLevelMin, int iDetailLevelMax
) {
    auto it = g_clModelCache.find(sModelName);
    if(it != g_clModelCache.end()) {
        CModel *pModel = new CModel(*it->second);
        return pModel;
    }
    std::vector<tinyobj::shape_t> clObjShapes;
    std::string sObjLoaderErr = tinyobj::LoadObj(clObjShapes, sModelName.c_str(), "");
    if(!sObjLoaderErr.empty()) {
        std::cerr << "Failed to load model " << sModelName << ": " << sObjLoaderErr << std::endl;
        return NULL;
    }
    
    CModel *pModel = new CModel();
    
    for (size_t iShapeIndex = 0; iShapeIndex < clObjShapes.size(); iShapeIndex++) {
        pModel->m_clShapes.resize(pModel->m_clShapes.size()+1);
        CShape &stShape = pModel->m_clShapes[iShapeIndex];
        
        stShape.m_sName = clObjShapes[iShapeIndex].name;
        stShape.m_clMaterialNames.push_back(clObjShapes[iShapeIndex].material.diffuse_texname);
        //printf("%s\n", stShape.m_clMaterialNames[0].c_str());
        stShape.m_vColAmbient  = *((vec3*)clObjShapes[iShapeIndex].material.ambient  );
        stShape.m_vColDiffuse  = *((vec3*)clObjShapes[iShapeIndex].material.diffuse  );
        stShape.m_vColSpecular = *((vec3*)clObjShapes[iShapeIndex].material.specular );
        stShape.m_vColEmission = *((vec3*)clObjShapes[iShapeIndex].material.emission );
        stShape.m_fShininess   = clObjShapes[iShapeIndex].material.shininess;
        stShape.m_iDetailLevelMin = iDetailLevelMin;
        stShape.m_iDetailLevelMax = iDetailLevelMax;
        stShape.m_iRenderPass = 1;
        auto itParams = clObjShapes[iShapeIndex].material.unknown_parameter.find("illum");
        if(itParams != clObjShapes[iShapeIndex].material.unknown_parameter.end()) {
            stShape.m_iRenderPass = (int)strtol(itParams->second.c_str(), (char**)NULL, 0);
        }
        
        CMesh &stMesh = stShape.m_stMesh;
        
        for(size_t iIndexNum = 0; iIndexNum < clObjShapes[iShapeIndex].mesh.indices.size(); iIndexNum++) {
            stMesh.m_clTriangleVertexIndices.push_back(clObjShapes[iShapeIndex].mesh.indices[iIndexNum]);
        }
        
        if((clObjShapes[iShapeIndex].mesh.positions.size() % 3) != 0) {
            std::cerr << "Failed to load model " << sModelName << ": " << "Mesh not in triangle format." << std::endl;
            delete pModel;
            return NULL;
        }
        for (size_t iVertexNum = 0; iVertexNum < clObjShapes[iShapeIndex].mesh.positions.size() / 3; iVertexNum++) {
            stMesh.m_clVertexPositions.push_back(
                vec3(
                    clObjShapes[iShapeIndex].mesh.positions[3*iVertexNum+0]*fScale,
                    clObjShapes[iShapeIndex].mesh.positions[3*iVertexNum+1]*fScale,
                    clObjShapes[iShapeIndex].mesh.positions[3*iVertexNum+2]*fScale
                )//+vec3(number_rand(), number_rand(), number_rand())
            );
            //m_clVertexColors.push_back(vec3(number_rand(), number_rand(), number_rand()));
        }
        
        if(clObjShapes[iShapeIndex].mesh.normals.size() > 0) {
            for (size_t iVertexNum = 0; iVertexNum < clObjShapes[iShapeIndex].mesh.normals.size() / 3; iVertexNum++) {
                stMesh.m_clVertexNormals.push_back(
                    vec3(
                        clObjShapes[iShapeIndex].mesh.normals[3*iVertexNum+0],
                        clObjShapes[iShapeIndex].mesh.normals[3*iVertexNum+1],
                        clObjShapes[iShapeIndex].mesh.normals[3*iVertexNum+2]
                    )
                );
            }
        } else {
            pModel->estimateNormals();
        }
        if(bWithColor && stMesh.m_clVertexColors.size() < stMesh.m_clVertexNormals.size()) {
            for(auto itVtxNrm = stMesh.m_clVertexNormals.begin(); itVtxNrm != stMesh.m_clVertexNormals.end(); itVtxNrm++) {
                vec3 &vVertexNormal = *itVtxNrm;
                stMesh.m_clVertexColors.push_back(vVertexNormal);
            }
        }
        
        assert((clObjShapes[iShapeIndex].mesh.texcoords.size() % 2) == 0);
        for (size_t iVertexNum = 0; iVertexNum < clObjShapes[iShapeIndex].mesh.texcoords.size() / 2; iVertexNum++) {
            stMesh.m_clVertexTexCoords.push_back(
                vec3(
                    clObjShapes[iShapeIndex].mesh.texcoords[2*iVertexNum+0],
                    clObjShapes[iShapeIndex].mesh.texcoords[2*iVertexNum+1],
                    0
                )
            );
            //m_clVertexColors.push_back(vec3(number_rand(), number_rand(), number_rand()));
        }
    }
    
    // TODO: Cache model
    //m_clModelCache
    
    std::cerr << "Model loaded: " << sModelName << std::endl;
    
    CModel *pCachedModel = new CModel(*pModel);
    g_clModelCache.insert(std::pair<std::string, CModel*>(sModelName, pCachedModel));
    
    return pModel;
}

void CModel::estimateNormals() {
    // https://github.com/zeyangl/MassLight/blob/master/src/Mesh.cpp
    
    for(auto itShape = m_clShapes.begin(); itShape != m_clShapes.end(); itShape++) {
        CShape &stShape = *itShape;
        CMesh &stMesh = stShape.m_stMesh;
        for(int iIndexTripletNum = 0; iIndexTripletNum < stMesh.m_clTriangleVertexIndices.size(); iIndexTripletNum += 3) {
            const vec3 &v0 = stMesh.m_clVertexPositions[stMesh.m_clTriangleVertexIndices[iIndexTripletNum+0]];
            const vec3 &v1 = stMesh.m_clVertexPositions[stMesh.m_clTriangleVertexIndices[iIndexTripletNum+1]];
            const vec3 &v2 = stMesh.m_clVertexPositions[stMesh.m_clTriangleVertexIndices[iIndexTripletNum+2]];
            
            vec3 normal = ((v1-v0).cross(v2-v0)).unit();
            stMesh.m_clVertexNormals.push_back(normal);
        }
    }
}

CMesh::CMesh(
    
) : m_clVertexPositions(),
    m_clVertexNormals(),
    m_clVertexTexCoords(),
    m_clVertexColors(),
    m_clTriangleVertexIndices()
{
    
}
CMesh::~CMesh() {
    
}

size_t CMesh::pushVertex(vec3 p, vec3 n, vec3 t) {
    size_t index = m_clVertexPositions.size();
    m_clVertexPositions.push_back(p);
    m_clVertexNormals.push_back(n);
    assert(m_clVertexPositions.size() == m_clVertexNormals.size());
    m_clVertexTexCoords.push_back(t);
    assert(m_clVertexPositions.size() == m_clVertexTexCoords.size());
    return index;
}
size_t CMesh::pushVertex(vec3 p, vec3 n, vec3 t, vec3 c) {
    size_t index = pushVertex(p, n, t);
    m_clVertexColors.push_back(c);
    assert(m_clVertexColors.size() == index+1);
    return index;
}
void CMesh::pushTriangle(size_t va, size_t vb, size_t vc, bool flip) {//, int subdivide) {
    if(flip) {
        std::swap(va, vc);
    }
    //if(subdivide == 0) {
        m_clTriangleVertexIndices.push_back(va);
        m_clTriangleVertexIndices.push_back(vb);
        m_clTriangleVertexIndices.push_back(vc);
    /* } else {
        vec3 p[3] = {m_clVertexPositions[va], m_clVertexPositions[vb], m_clVertexPositions[vc]};
        vec3 n[3] = {m_clVertexNormals  [va], m_clVertexNormals  [vb], m_clVertexNormals  [vc]};
        vec3 t[3] = {m_clVertexTexCoords[va], m_clVertexTexCoords[vb], m_clVertexTexCoords[vc]};
        vec3 c[3] = {m_clVertexColors   [va], m_clVertexColors   [vb], m_clVertexColors   [vc]};
        
        // find the longest side
        int best = 0;
        double dist = 0, bestDist = 0;
        
        dist = (p[2]-p[1]).length(); if(dist > bestDist) { best = 0; bestDist = dist; }
        dist = (p[2]-p[0]).length(); if(dist > bestDist) { best = 1; bestDist = dist; }
        dist = (p[1]-p[0]).length(); if(dist > bestDist) { best = 2; bestDist = dist; }
        
        
        
        vec3 n_a = m_clVertexNormals[va]; vec3 t_a = m_clVertexTexCoords[va]; vec3 c_a = m_clVertexColors[va];
        vec3 p_b = m_clVertexPositions[vb]; vec3 n_b = m_clVertexNormals[vb]; vec3 t_b = m_clVertexTexCoords[vb]; vec3 c_b = m_clVertexColors[vb];
        vec3 p_c = m_clVertexPositions[vc]; vec3 n_c = m_clVertexNormals[vc]; vec3 t_c = m_clVertexTexCoords[vc]; vec3 c_c = m_clVertexColors[vc];
        
        
    } */
}
void CMesh::pushQuad(size_t va, size_t vb, size_t vc, size_t vd, bool flip, int subdivide) {
    //m_clVertexPositions[va] = m_clVertexPositions[va]+vec3(number_rand(), number_rand(), number_rand());
    if(subdivide == 0) {
        pushTriangle(va, vb, vd, flip);// , subdivide);
        pushTriangle(vb, vc, vd, flip);// , subdivide);
    } else {
        vec3 p_tl = m_clVertexPositions[va], n_tl = m_clVertexNormals[va], t_tl = m_clVertexTexCoords[va], c_tl = m_clVertexColors[va];
        vec3 p_tr = m_clVertexPositions[vb], n_tr = m_clVertexNormals[vb], t_tr = m_clVertexTexCoords[vb], c_tr = m_clVertexColors[vb];
        vec3 p_br = m_clVertexPositions[vc], n_br = m_clVertexNormals[vc], t_br = m_clVertexTexCoords[vc], c_br = m_clVertexColors[vc];
        vec3 p_bl = m_clVertexPositions[vd], n_bl = m_clVertexNormals[vd], t_bl = m_clVertexTexCoords[vd], c_bl = m_clVertexColors[vd];
        
        vec3 p_tm = (p_tl+p_tr)/2, n_tm = (n_tl+n_tr)/2, t_tm = (t_tl+t_tr)/2, c_tm = (c_tl+c_tr)/2;
        vec3 p_rm = (p_tr+p_br)/2, n_rm = (n_tr+n_br)/2, t_rm = (t_tr+t_br)/2, c_rm = (c_tr+c_br)/2;
        vec3 p_bm = (p_br+p_bl)/2, n_bm = (n_br+n_bl)/2, t_bm = (t_br+t_bl)/2, c_bm = (c_br+c_bl)/2;
        vec3 p_lm = (p_tl+p_bl)/2, n_lm = (n_tl+n_bl)/2, t_lm = (t_tl+t_bl)/2, c_lm = (c_tl+c_bl)/2;
        vec3 p_mm = (p_tl+p_tr+p_br+p_bl)/4, n_mm = (n_tl+n_tr+n_br+n_bl)/4, t_mm = (t_tl+t_tr+t_br+t_bl)/4, c_mm = (c_tl+c_tr+c_br+c_bl)/4;
        
        // c_tm = vec3(number_rand(), number_rand(), number_rand());
        // c_rm = vec3(number_rand(), number_rand(), number_rand());
        // c_bm = vec3(number_rand(), number_rand(), number_rand());
        // c_lm = vec3(number_rand(), number_rand(), number_rand());
        // c_mm = vec3(number_rand(), number_rand(), number_rand());
        
        size_t vtm = pushVertex(p_tm, n_tm, t_tm, c_tm);
        size_t vrm = pushVertex(p_rm, n_rm, t_rm, c_rm);
        size_t vbm = pushVertex(p_bm, n_bm, t_bm, c_bm);
        size_t vlm = pushVertex(p_lm, n_lm, t_lm, c_lm);
        size_t vmm = pushVertex(p_mm, n_mm, t_mm, c_mm);
        
        pushQuad(va , vtm, vmm, vlm, flip, subdivide-1); // tl quadrant
        pushQuad(vtm, vb , vrm, vmm, flip, subdivide-1); // tr quadrant
        pushQuad(vmm, vrm, vc , vbm, flip, subdivide-1); // br quadrant
        pushQuad(vlm, vmm, vbm, vd , flip, subdivide-1); // bl quadrant
    }
}

CShape::CShape(
    
) : m_sName(""),
    m_stMesh(),
    m_clMaterialNames(),
    m_iDetailLevelMin(std::numeric_limits<int>::min()),
    m_iDetailLevelMax(std::numeric_limits<int>::max())
{
    /*std::stringstream ss;
    for(int i = 0; i < 10; i++) {
        ss << (char)randrange('a', 'z');
    }
    m_sName = ss.str();
    
    printf("Shape make: %s.\n", m_sName.c_str());*/
}
CShape::~CShape() {
    //printf("Shape free: %s (%d, %d).\n", m_sName.c_str(), m_stMesh.m_clVertexPositions.size(), m_stMesh.m_clTriangleVertexIndices.size());
}

// old versions of above methods, used to make a cube
int pushVtx(CMesh &stMesh, vec3 p, vec3 n, bool withcol, vec3 c) {
    stMesh.m_clVertexPositions.push_back(p);
    stMesh.m_clVertexNormals.push_back(n);
    if(withcol) {
        stMesh.m_clVertexColors.push_back(c);
    }
    
    return stMesh.m_clVertexPositions.size()-1;
}
void pushQuad(CMesh &stMesh, bool withcol, vec3 n, vec3 ap, vec3 bp, vec3 cp, vec3 dp, vec3 ac, vec3 bc, vec3 cc, vec3 dc) {
    //printf("%d %d %d %d\n", a, b, c, d);
    n = n.unit();
    
    int ai = pushVtx(stMesh, ap, n, withcol, ac);
    stMesh.m_clVertexTexCoords.push_back(vec3(0, 0, 0));
    int bi = pushVtx(stMesh, bp, n, withcol, bc);
    stMesh.m_clVertexTexCoords.push_back(vec3(0, 1, 0));
    int ci = pushVtx(stMesh, cp, n, withcol, cc);
    stMesh.m_clVertexTexCoords.push_back(vec3(1, 1, 0));
    int di = pushVtx(stMesh, dp, n, withcol, dc);
    stMesh.m_clVertexTexCoords.push_back(vec3(1, 0, 0));
    
    stMesh.m_clTriangleVertexIndices.push_back(ai);
    stMesh.m_clTriangleVertexIndices.push_back(bi);
    stMesh.m_clTriangleVertexIndices.push_back(di);
    
    stMesh.m_clTriangleVertexIndices.push_back(bi);
    stMesh.m_clTriangleVertexIndices.push_back(ci);
    stMesh.m_clTriangleVertexIndices.push_back(di);
}

CModel *CModel::makeCube(const std::string &sMaterialName, vec3 size, vec3 col, bool withcol) {
    CModel *pModel = new CModel();
    pModel->m_clShapes.push_back(CShape());
    CShape &stShape = pModel->m_clShapes[0];
    stShape.m_clMaterialNames.push_back(sMaterialName);
    CMesh &stMesh = stShape.m_stMesh;
    
    vec3 s = size/2;
    
    vec3 o_r = quat().right(), o_u = quat().up(), o_b = quat().back();
    vec3 o_l = -o_r          , o_d = -o_u       , o_f = -o_b         ;
    
    vec3 v_rub = (s.x*o_r + s.y*o_u + s.z*o_b), c_rub = col;//vec3(1, 1, 1);
    vec3 v_ruf = (s.x*o_r + s.y*o_u + s.z*o_f), c_ruf = col;//vec3(1, 1, 0);
    vec3 v_rdb = (s.x*o_r + s.y*o_d + s.z*o_b), c_rdb = col;//vec3(1, 0, 1);
    vec3 v_rdf = (s.x*o_r + s.y*o_d + s.z*o_f), c_rdf = col;//vec3(1, 0, 0);
    vec3 v_lub = (s.x*o_l + s.y*o_u + s.z*o_b), c_lub = col;//vec3(0, 1, 1);
    vec3 v_luf = (s.x*o_l + s.y*o_u + s.z*o_f), c_luf = col;//vec3(0, 1, 0);
    vec3 v_ldb = (s.x*o_l + s.y*o_d + s.z*o_b), c_ldb = col;//vec3(0, 0, 1);
    vec3 v_ldf = (s.x*o_l + s.y*o_d + s.z*o_f), c_ldf = col;//vec3(0, 0, 0);
    
    //bool withcol = (sMaterialName == "");
    pushQuad(stMesh, withcol, o_r, v_rdb, v_rdf, v_ruf, v_rub, c_rdb, c_rdf, c_ruf, c_rub);
    pushQuad(stMesh, withcol, o_l, v_ldb, v_lub, v_luf, v_ldf, c_ldb, c_lub, c_luf, c_ldf);
    pushQuad(stMesh, withcol, o_u, v_ruf, v_luf, v_lub, v_rub, c_ruf, c_luf, c_lub, c_rub);
    pushQuad(stMesh, withcol, o_d, v_rdf, v_rdb, v_ldb, v_ldf, c_rdf, c_rdb, c_ldb, c_ldf);
    pushQuad(stMesh, withcol, o_b, v_lub, v_ldb, v_rdb, v_rub, c_lub, c_ldb, c_rdb, c_rub);
    pushQuad(stMesh, withcol, o_f, v_luf, v_ruf, v_rdf, v_ldf, c_luf, c_ruf, c_rdf, c_ldf);
    
    return pModel;
}
