
#include "matrix.h"
#include "GL/freeglut.h"
#include <limits>
#include <algorithm>
#include <iostream>
#include <sstream>

class CProp;
#include "CProp.h"

#include "CEngine.h"
#include "CRenderer.h"

CProp::CProp(
    CEngine *pEngine,
    CModel *pModel
) : CEntity(pEngine),
    m_clShapeTextureIds()
{
    m_vExtentsMin = vec3(-0.5, -0.5, -0.5);
    m_vExtentsMax = vec3( 0.5,  0.5,  0.5);
    setModel(pModel);
}
CProp::~CProp() {
    if(m_pModel != NULL) {
        delete m_pModel;
        m_pModel = NULL;
    }
}

void CProp::_onChange() {
    CEntity::_onChange();
}
void CProp::_think(
    double fTime, double fDeltaTime
) {
    
}

void CProp::renderInit() {
    int textureCount = 0;
    if(m_pModel != NULL) {
        m_clShapeTextureIds.clear();
        
        std::map<std::string, unsigned int> clLoadedTextures;
        
        auto &clShapes = m_pModel->m_clShapes;
        for(auto itShape = clShapes.begin(); itShape != clShapes.end(); itShape++) {
            CShape &stShape = *itShape;
            for(auto itTexture = stShape.m_clMaterialNames.begin(); itTexture != stShape.m_clMaterialNames.end(); itTexture++) {
                unsigned int texid;
                auto itLoadedTexture = clLoadedTextures.find(*itTexture);
                if(itLoadedTexture != clLoadedTextures.end()) {
                    texid = itLoadedTexture->second;
                } else {
                    texid = m_pEngine->getRenderer()->loadTexture(*itTexture);
                    clLoadedTextures.insert(std::make_pair(*itTexture, texid));
                }
                m_clShapeTextureIds.push_back(texid);
                textureCount++;
            }
            // std::cout << "SHAPED " << (itShape-clShapes.begin())
                      // << " = " << stShape.m_sMaterialName << " -> "
                      // << m_pEngine->getRenderer()->loadTexture(stShape.m_sMaterialName)
                      // << std::endl
            // ;
        }
    }
}
void CProp::_render(
    CRenderer *pRenderer,
    double fDeltaTime, double fDeltaClock,
    CEntity::ERenderMode eMode,
    int iDetailLevel
) {
    if(eMode != CEntity::ERenderMode::TRANSPARENT && eMode != CEntity::ERenderMode::NORMAL) return;
    
    bool bEnableVertexColors = true;
    m_pEngine->getOption("Vertex Colors - Enable", &bEnableVertexColors, bEnableVertexColors, "group='Renderer'");
    bool bEnableMaterialAmbient = true;
    m_pEngine->getOption("Ambient - Enable", &bEnableMaterialAmbient, bEnableMaterialAmbient, "group='Prop Material'");
    bool bEnableMaterialDiffuse = true;
    m_pEngine->getOption("Diffuse - Enable", &bEnableMaterialDiffuse, bEnableMaterialDiffuse, "group='Prop Material'");
    bool bEnableMaterialSpecular = true;
    m_pEngine->getOption("Specular - Enable", &bEnableMaterialSpecular, bEnableMaterialSpecular, "group='Prop Material'");
    bool bEnableMaterialEmission = false;
    m_pEngine->getOption("Emission - Enable", &bEnableMaterialEmission, bEnableMaterialEmission, "group='Prop Material'");
    bool bEnableMaterialShininess = true;
    m_pEngine->getOption("Shininess - Enable", &bEnableMaterialShininess, bEnableMaterialShininess, "group='Prop Material'");
    
    int begintime = m_pEngine->getClock();
    if(m_pModel != NULL) {
        int textureCount = 0;
        
        auto &clShapes = m_pModel->m_clShapes;
        for(auto itShape = clShapes.begin(); itShape != clShapes.end(); itShape++) {
            //if(itShape-clShapes.begin() != clShapes.size()-1) continue;
            CShape &stShape = *itShape;
            CMesh &stMesh = stShape.m_stMesh;
            
            unsigned int iTextureId;
            for(int iTexNum = 0; iTexNum < stShape.m_clMaterialNames.size(); iTexNum++) {
                iTextureId = m_clShapeTextureIds[textureCount];
                textureCount++;
            }
            
            if(iDetailLevel < stShape.m_iDetailLevelMin) continue;
            if(iDetailLevel > stShape.m_iDetailLevelMax) continue;
            
            bool bIsTransparent = (stShape.m_iRenderPass == 2 || m_fOpacity <= 0.95);
            if(eMode == CEntity::ERenderMode::TRANSPARENT) {
                if(!bIsTransparent) continue;
            } else if(eMode == CEntity::ERenderMode::NORMAL) {
                if(bIsTransparent) continue;;
            }
            
            GL_CHECK( glPushMatrix() );
                GL_CHECK( glTranslated(stShape.m_vOffset.x, stShape.m_vOffset.y, stShape.m_vOffset.z) );
                
                GL_CHECK( glTranslated(stShape.m_vRotationOrigin.x, stShape.m_vRotationOrigin.y, stShape.m_vRotationOrigin.z) );
                quat ang = stShape.m_qRotation;
                mat4 m; ang.mat(m);
                GL_CHECK( glMultMatrixd(m) );
                GL_CHECK( glTranslated(-stShape.m_vRotationOrigin.x, -stShape.m_vRotationOrigin.y, -stShape.m_vRotationOrigin.z) );
                
                //GL_CHECK( glColor4f(1.0f, 1.0f, 1.0f, m_fOpacity) );
                GL_CHECK( glColor4f(1.0f, 1.0f, 1.0f, 1.0f) );
                GL_CHECK( glDisableClientState(GL_NORMAL_ARRAY) );
                GL_CHECK( glDisableClientState(GL_COLOR_ARRAY) );
                GL_CHECK( glDisableClientState(GL_TEXTURE_COORD_ARRAY) );
                GL_CHECK( glDisableClientState(GL_VERTEX_ARRAY) );
                
                GLfloat aMatAmbient  [] = {(float)stShape.m_vColAmbient .x, (float)stShape.m_vColAmbient .y, (float)stShape.m_vColAmbient .z, 1.0};
                GLfloat aMatDiffuse  [] = {(float)stShape.m_vColDiffuse .x, (float)stShape.m_vColDiffuse .y, (float)stShape.m_vColDiffuse .z, 1.0};
                GLfloat aMatSpecular [] = {(float)stShape.m_vColSpecular.x, (float)stShape.m_vColSpecular.y, (float)stShape.m_vColSpecular.z, 1.0};
                GLfloat aMatEmission [] = {(float)stShape.m_vColEmission.x, (float)stShape.m_vColEmission.y, (float)stShape.m_vColEmission.z, 1.0};
                // std::stringstream ss;
                // ss << "Emission - Color " << this;
                // std::string str = ss.str();
                // m_pEngine->setWatchOutput(str.c_str(), (col4*)aMatEmission, "group='Prop Material'");
                GLfloat aMatShininess[] = {(float)std::max(0.0, std::min(128.0, stShape.m_fShininess))};
                
                if(bEnableMaterialAmbient  ) { GL_CHECK( glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT  , aMatAmbient  ) ); }
                if(bEnableMaterialDiffuse  ) { GL_CHECK( glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE  , aMatDiffuse  ) ); }
                if(bEnableMaterialSpecular ) { GL_CHECK( glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR , aMatSpecular ) ); }
                if(bEnableMaterialEmission ) { GL_CHECK( glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION , aMatEmission ) ); }
                if(bEnableMaterialShininess) { GL_CHECK( glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, aMatShininess) ); }
                
                bool bEnableTexCoordPointer = (stMesh.m_clVertexTexCoords.size() > 0);
                bool bEnableNormalPointer = (stMesh.m_clVertexNormals.size() > 0);
                bool bEnableColorPointer = (stMesh.m_clVertexColors.size() > 0);
                
                GL_CHECK( glEnableClientState(GL_VERTEX_ARRAY) );
                GL_CHECK( glVertexPointer(
                    3,                               // size
                    GL_DOUBLE,                       // type
                    0,                               // stride
                    &(stMesh.m_clVertexPositions[0]) // pointer
                ) );
                
                if(bEnableNormalPointer) {
                    GL_CHECK( glEnableClientState(GL_NORMAL_ARRAY) );
                    GL_CHECK( glNormalPointer(
                        GL_DOUBLE,                       // type
                        0,                               // stride
                        &(stMesh.m_clVertexNormals[0]) // pointer
                    ) );
                }
                
                if(bEnableTexCoordPointer) {
                    GL_CHECK( glEnableClientState(GL_TEXTURE_COORD_ARRAY) );
                    GL_CHECK( glTexCoordPointer(
                        2,                               // size (x and y)
                        GL_DOUBLE,                       // type
                        sizeof(vec3),                    // stride (skip z)
                        &(stMesh.m_clVertexTexCoords[0]) // pointer
                    ) );
                }
                
                if(bEnableColorPointer && bEnableVertexColors) {
                    GL_CHECK( glEnableClientState(GL_COLOR_ARRAY) );
                    GL_CHECK( glColorPointer(
                        3,                            // size
                        GL_DOUBLE,                    // type
                        0,                            // stride
                        &(stMesh.m_clVertexColors[0]) // pointer
                    ) );
                }
                
                // printf("p:%d n:%d t:%d c:%d max:%d; tex:%d\n",
                    // stMesh.m_clVertexPositions.size(),
                    // stMesh.m_clVertexNormals.size(),
                    // stMesh.m_clVertexTexCoords.size(),
                    // stMesh.m_clVertexColors.size(),
                    // *std::max_element(stMesh.m_clTriangleVertexIndices.begin(), stMesh.m_clTriangleVertexIndices.end()),
                    // iTextureId
                // );
                
                if(iTextureId != 0) {
                    GL_CHECK( glEnable(GL_TEXTURE_2D) );
                    GL_CHECK( glBindTexture(GL_TEXTURE_2D, iTextureId) );
                }
                GL_CHECK( glDrawElements(
                    GL_TRIANGLES,                            // mode
                    stMesh.m_clTriangleVertexIndices.size(), // count
                    GL_UNSIGNED_INT,                         // type
                    &(stMesh.m_clTriangleVertexIndices[0])   // pointer
                ) );
                
                GL_CHECK( glBindTexture(GL_TEXTURE_2D, 0) );
                GL_CHECK( glDisable(GL_TEXTURE_2D) );
                
                GL_CHECK( glDisableClientState(GL_COLOR_ARRAY) );
                GL_CHECK( glDisableClientState(GL_TEXTURE_COORD_ARRAY) );
                GL_CHECK( glDisableClientState(GL_NORMAL_ARRAY) );
                GL_CHECK( glDisableClientState(GL_VERTEX_ARRAY) );
            GL_CHECK( glPopMatrix() );
        }
    } else {
        CEntity::_render(pRenderer, fDeltaTime, fDeltaClock, eMode, iDetailLevel);
    }
    //m_iDebugPassCount += m_pEngine->getClock()-begintime;//stMesh.m_clTriangleVertexIndices[0]/3;
    /*
    if(m_pMesh != NULL) {
        if(false) {
            glBegin(GL_LINES);
                std::vector<SVertex> &clVertices = m_pMesh->clVertices;
                for(SMeshShape &stShape : clShapes) {
                    std::vector<STriangle> &clTriangles = stShape->clTriangles;
                    for(STriangle &stTri : clTriangles) {
                        vec3 aTriangleVertices[3];
                        for(int iVertexNum = 0; iVertexNum < 3; iVertexNum++) {
                            int iVertexIndex = stTri.iVertexIndices[iVertexNum];
                            //printf("%d / %d\n", iVertexIndex, clVertices.size());
                            SVertex &stVertex = clVertices[iVertexIndex];
                            aTriangleVertices[iVertexNum] = stVertex.vPos;
                        }
                        glVertex3f(aTriangleVertices[0].x, aTriangleVertices[0].y, aTriangleVertices[0].z);
                        glVertex3f(aTriangleVertices[1].x, aTriangleVertices[1].y, aTriangleVertices[1].z);
                        
                        glVertex3f(aTriangleVertices[1].x, aTriangleVertices[1].y, aTriangleVertices[1].z);
                        glVertex3f(aTriangleVertices[2].x, aTriangleVertices[2].y, aTriangleVertices[2].z);
                        
                        glVertex3f(aTriangleVertices[2].x, aTriangleVertices[2].y, aTriangleVertices[2].z);
                        glVertex3f(aTriangleVertices[0].x, aTriangleVertices[0].y, aTriangleVertices[0].z);
                    }
                }
            glEnd();
        } else {
            std::vector<SVertex> &clVertices = m_pMesh->clVertices;
            for(SMeshShape &stShape : clShapes) {
                std::vector<STriangle> &clTriangles = stShape->clTriangles;
                glBegin(GL_TRIANGLES);
                    for(STriangle &stTri : clTriangles) {
                        for(int iVertexNum = 0; iVertexNum < 3; iVertexNum++) {
                            int iVertexIndex = stTri.iVertexIndices[iVertexNum];
                            //printf("%d / %d\n", iVertexIndex, clVertices.size());
                            SVertex &stVertex = clVertices[iVertexIndex];
                            glColor3f(stVertex.vCol.x, stVertex.vCol.y, stVertex.vCol.z);
                            glVertex3f(stVertex.vPos.x, stVertex.vPos.y, stVertex.vPos.z);
                        }
                    }
                glEnd();
            }
        }
    } else {
        glColor3f(1.0, 0.0, 0.0);
        glBegin(GL_LINE_STRIP);
            glVertex3f(m_vExtentsMin.x, m_vExtentsMin.y, m_vExtentsMin.z);
            glVertex3f(m_vExtentsMax.x, m_vExtentsMin.y, m_vExtentsMin.z);
            glVertex3f(m_vExtentsMax.x, m_vExtentsMin.y, m_vExtentsMax.z);
            glVertex3f(m_vExtentsMin.x, m_vExtentsMin.y, m_vExtentsMax.z);
            glVertex3f(m_vExtentsMin.x, m_vExtentsMin.y, m_vExtentsMin.z);
            glVertex3f(m_vExtentsMin.x, m_vExtentsMax.y, m_vExtentsMin.z);
            glVertex3f(m_vExtentsMax.x, m_vExtentsMax.y, m_vExtentsMin.z);
            glVertex3f(m_vExtentsMax.x, m_vExtentsMax.y, m_vExtentsMax.z);
            glVertex3f(m_vExtentsMin.x, m_vExtentsMax.y, m_vExtentsMax.z);
            glVertex3f(m_vExtentsMin.x, m_vExtentsMax.y, m_vExtentsMin.z);
        glEnd();
        
        glBegin(GL_LINES);
            glVertex3f(m_vExtentsMax.x, m_vExtentsMin.y, m_vExtentsMin.z);
            glVertex3f(m_vExtentsMax.x, m_vExtentsMax.y, m_vExtentsMin.z);
            glVertex3f(m_vExtentsMax.x, m_vExtentsMin.y, m_vExtentsMax.z);
            glVertex3f(m_vExtentsMax.x, m_vExtentsMax.y, m_vExtentsMax.z);
            glVertex3f(m_vExtentsMin.x, m_vExtentsMin.y, m_vExtentsMax.z);
            glVertex3f(m_vExtentsMin.x, m_vExtentsMax.y, m_vExtentsMax.z);
        glEnd();
    }
    */
}

bool CProp::calculateExtents(vec3 &vExtentsMin, vec3 &vExtentsMax) const {
    if(m_pModel != NULL) {
        number fMinX = std::numeric_limits<number>::infinity(), fMaxX = -std::numeric_limits<number>::infinity();
        number fMinY = std::numeric_limits<number>::infinity(), fMaxY = -std::numeric_limits<number>::infinity();
        number fMinZ = std::numeric_limits<number>::infinity(), fMaxZ = -std::numeric_limits<number>::infinity();
        
        auto &clShapes = m_pModel->m_clShapes;
        for(auto itShape = clShapes.begin(); itShape != clShapes.end(); itShape++) {
            CShape &stShape = *itShape;
            CMesh &stMesh = stShape.m_stMesh;
            
            for(auto itVtxPos = stMesh.m_clVertexPositions.begin(); itVtxPos != stMesh.m_clVertexPositions.end(); itVtxPos++) {
                vec3 vVtxPos = (*itVtxPos + stShape.m_vOffset);
                fMinX = std::min(fMinX, vVtxPos.x); fMaxX = std::max(fMaxX, vVtxPos.x);
                fMinY = std::min(fMinY, vVtxPos.y); fMaxY = std::max(fMaxY, vVtxPos.y);
                fMinZ = std::min(fMinZ, vVtxPos.z); fMaxZ = std::max(fMaxZ, vVtxPos.z);
            }
        }
        vExtentsMin = vec3(fMinX, fMinY, fMinZ);
        vExtentsMax = vec3(fMaxX, fMaxY, fMaxZ);
        return true;
    }
    return false;
}

CProp &CProp::setModel(CModel *pModel) {
    m_pModel = pModel;
    m_bRenderInitialised = false;
    return (CProp&)mark();
}
