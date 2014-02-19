#pragma once
#ifndef idCD77871E_97C0_4D15_844BDD094C20F162
#define idCD77871E_97C0_4D15_844BDD094C20F162

#include <vector>
#include <map>
#include <string>
#include "matrix.h"

class CModel;

extern std::map<std::string, CModel*> g_clModelCache;

class CMesh {
    public:
        CMesh();
        virtual ~CMesh();
        
    public:
        size_t pushVertex(vec3 p, vec3 n, vec3 t = vec3(0, 0, 0));
        size_t pushVertex(vec3 p, vec3 n, vec3 t, vec3 c);
        void pushTriangle(size_t va, size_t vb, size_t vc, bool flip = true);
        void pushQuad(size_t va, size_t vb, size_t vc, size_t vd, bool flip = true, int subdivide = 0);
        
    public:
        std::vector<vec3> m_clVertexPositions;
        std::vector<vec3> m_clVertexNormals;
        std::vector<vec3> m_clVertexTexCoords;
        std::vector<vec3> m_clVertexColors;
        //std::vector<CTriangle> m_clTriangles;
        std::vector<unsigned int> m_clTriangleVertexIndices;
};
class CShape {
    public:
        CShape();
        virtual ~CShape();
    public:
        
        
    public:
        std::string m_sName;
        CMesh m_stMesh;
        std::vector<std::string> m_clMaterialNames;
        vec3 m_vColAmbient;
        vec3 m_vColDiffuse;
        vec3 m_vColSpecular;
        vec3 m_vColEmission;
        double m_fShininess;
        int m_iRenderPass;
        
        vec3 m_vOffset;
        quat m_qRotation;
        vec3 m_vRotationOrigin; // messed up early, easier to do this :)
        int m_iDetailLevelMin;
        int m_iDetailLevelMax;
};
typedef std::vector<CShape> CShapeCollection;

class CModel {
    public:
        /*class CTriangle {
            public:
                unsigned int m_uiVertexIndices[3];
        };*/
    public:
        CModel();
        virtual ~CModel();
        
    public:
        static CModel *loadModel(
            const std::string &sModelName,
            double fScale, bool bWithColor = false,
            int iDetailLevelMin = 9999, int iDetailLevelMax = 0
        );
        
        void estimateNormals();
        void estimateColors();
        
        static CModel *makeCube(
            const std::string &sMaterialName = "",
            vec3 size = vec3(1, 1, 1),
            vec3 col = vec3(1, 1, 1),
            bool withcol = false
        );
        
    public:
        std::string m_sName;
        CShapeCollection m_clShapes;
        
        bool deletedebug;
};

#endif // header
