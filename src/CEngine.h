#pragma once
#ifndef idD988E080_CE99_4AE7_9CD95BAC0FA0545D
#define idD988E080_CE99_4AE7_9CD95BAC0FA0545D

#include <typeinfo>
#include <utility> // for pair
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <memory>
#include <GL/freeglut.h> // Including this here is horrible, but we need it for the cursors and other enums
#include <AntTweakBar.h> // same here

class CEngine;
class CRenderer;
#include "CRenderer.h"
class CEntity;
#include "CEntity.h"
class CWorld;
class CPlayer;

class CEngine {
    
    public: // typedefs
        typedef void (*FChangeCursor)(int eCursor);
        typedef int (*FGetClock)();
        
        typedef unsigned long EntityID;
        typedef std::map<EntityID, CEntity*> CEntityCollection;
        
    public: // constructors/destructors
        CEngine(TwBar *pTweakBar);
        virtual ~CEngine();
        
    public: // public methods
        void setChangeCursorCallback(FChangeCursor cbChangeCursor) { m_cbChangeCursor = cbChangeCursor; }
        void setGetClockCallback(FGetClock cbGetClock) { m_cbGetClock = cbGetClock; }
        
        template<typename TValueType>
        void getOption(
            const std::string sName, TValueType *p_OutValue, TValueType _Default,
            const std::string &sTwParams = ""
        );
        template<typename TValueType>
        void setOption(
            const std::string sName, TValueType _Value,
            const std::string &sTwParams = ""
        );
        template<typename TValueType>
        void setWatchOutput(
            const std::string sName, TValueType _Value,
            const std::string &sTwParams = ""
        );
        template<typename TValueType>
        void watchVariable(
            const std::string sName, TValueType *p_Variable,
            const std::string &sTwParams = "",
            bool bReadOnly = true
        );
        bool getButtonState(
            const std::string sName,
            const std::string &sTwParams = ""
        );
        
        CRenderer *getRenderer() const { return m_pRenderer; }
        
        const CEntityCollection& getEntities() const { return m_clEntities; }
        bool addEntity(CEntity *pEntity);
        bool removeEntity(CEntity *pEntity);
        
        CWorld *getWorld() const { return m_pWorld; }
        CPlayer *getPlayer() const { return m_pPlayer; }
        
        void quit() { m_bRunning = false; }
        bool getShowUI() const;
        bool getLockMouse() const;
        
        int getClock() const { return m_cbGetClock(); }
        double getTime() const { return m_fTime; }
        double getTimeStep() const { return m_fTime; }
        double getTimeScale() const { return m_fTimeScale; }
        void setTimeScale(double fTimeScale)  { m_fTimeScale = fTimeScale; }
        
        void onThink(double fTime, double fDeltaTime);
        
        // Events
        void onIdle();
        bool onWindowDisplay(int iWindowId);
        void onWindowDisplayOverlay(int iWindowId, int iLayerId);
        void onWindowResize(int iWindowId, int iSizeW, int iSizeH);
        void onWindowMouseButton(int iWindowId, int iButtonCode, int iButtonState, int iMousePosX, int iMousePosY);
        void onWindowMouseMotion(int iWindowId, int iMousePosX, int iMousePosY);
      //void onWindowLook(int iWindowId, float fDeltaX, float fDeltaY);
        void onWindowKey(int iWindowId, unsigned char chKeyCode, int iKeyState, int iMousePosX, int iMousePosY);
        void onWindowKeySpecial(int iWindowId, int iKeyCode, int iKeyState, int iMousePosX, int iMousePosY);
        void onWindowVisibility(int iWindowId, int iVisibilityState);
        void onWindowMouseEntry(int iWindowId, int iMousePresenceState);
      //void onMenuStatus(int iWindowId, int iMenuStatus, int iMousePosX, int iMousePosY);
        
    protected: // internal methods
        
    public: // private data
        FChangeCursor m_cbChangeCursor;
        FGetClock m_cbGetClock;
        
        std::unordered_map<std::string, std::pair<const std::type_info &, void*>> m_clOptions;
        std::unordered_map<std::string, std::pair<const std::type_info &, void*>> m_clWatchValues;
        std::unordered_map<std::string, bool> m_clButtons;
        
        TwBar *m_pTweakBar; // bad bad bad
        CRenderer *m_pRenderer;
        EntityID m_iNextEntityID;
        CEntityCollection m_clEntities;
        CWorld *m_pWorld;
        CPlayer *m_pPlayer;
        
        bool m_bRunning;
        int m_eCurrentCursor;
        
        long m_iClock; // milliseconds
        double m_fTimeAccumulation; // seconds
        double m_fTime; // seconds
        double m_fTimeStep; // seconds
        double m_fTimeScale;
        
        int m_iPrevFrameTime;
        static int m_iSecondsPerFrameHistoryCount;
        double *m_afSecondsPerFrameHistory;
        int m_iSecondsPerFrameHistoryIndex;
        double m_fSecondsPerFrameHistorySum;
        
        bool m_bAnimationStarted;
        bool m_bAnimationPaused;
        
};

// Didn't want to include AntTweakBar, but if I didn't this would be a mess

inline TwType getTwType(const std::type_info &stTypeInfo) {
    return ( // can't use switch, as typeid is not a const-expr
            stTypeInfo == typeid(int)          ? TW_TYPE_INT32
        :   stTypeInfo == typeid(unsigned int) ? TW_TYPE_UINT32
        :   stTypeInfo == typeid(double)       ? TW_TYPE_DOUBLE
        :   stTypeInfo == typeid(float)        ? TW_TYPE_FLOAT
        :   stTypeInfo == typeid(const char*)  ? TW_TYPE_CDSTRING
        :   stTypeInfo == typeid(char*      )  ? TW_TYPE_CDSTRING
        :   stTypeInfo == typeid(std::string)  ? TW_TYPE_STDSTRING
        :   stTypeInfo == typeid(bool)         ? TW_TYPE_BOOLCPP
        :   stTypeInfo == typeid(vec3)         ? TW_TYPE_DIR3D
        :   stTypeInfo == typeid(vec3*)        ? TW_TYPE_DIR3D
        :   stTypeInfo == typeid(col3)         ? TW_TYPE_COLOR3F
        :   stTypeInfo == typeid(col3*)        ? TW_TYPE_COLOR3F
        :   stTypeInfo == typeid(col4)         ? TW_TYPE_COLOR4F
        :   stTypeInfo == typeid(col4*)        ? TW_TYPE_COLOR4F
        :                                        TW_TYPE_UNDEF
    );
    //TW_TYPE_BOOLCPP
    //TW_TYPE_BOOL8
    //TW_TYPE_BOOL16
    //TW_TYPE_BOOL32
    //TW_TYPE_CHAR
    //TW_TYPE_INT8
    //TW_TYPE_UINT8
    //TW_TYPE_INT16
    //TW_TYPE_UINT16
    //TW_TYPE_INT32
    //TW_TYPE_UINT32
    //TW_TYPE_FLOAT
    //TW_TYPE_DOUBLE
    //TW_TYPE_COLOR32
    //TW_TYPE_COLOR3F
    //TW_TYPE_COLOR4F
    //TW_TYPE_CDSTRING
    //TW_TYPE_STDSTRING
    //TW_TYPE_QUAT4F
    //TW_TYPE_QUAT4D
    //TW_TYPE_DIR3F
    //TW_TYPE_DIR3D
}

template<typename TValueType>
void CEngine::getOption(
    const std::string sName,
    TValueType *p_OutValue,
    TValueType _Default,
    const std::string &sTwParams
) {
    auto it = m_clOptions.find(sName);
    if(it != m_clOptions.end()) {
        const std::type_info &stTypeInfo = it->second.first;
        TValueType *p_Value = (TValueType*)it->second.second;
        
        assert(stTypeInfo == typeid(TValueType));
        
        *p_OutValue = *p_Value;
        
    } else {
        TValueType *p_Value = new TValueType(_Default);
        m_clOptions.insert(
            std::pair<std::string, std::pair<const std::type_info &, void*>>(
                sName,
                std::pair<const std::type_info &, void*>(typeid(TValueType), (void*)p_Value)
            )
        );
        
        TwType iTwType = getTwType(typeid(TValueType));
        
        if(iTwType != TW_TYPE_UNDEF) {
            TwAddVarRW(m_pTweakBar, sName.c_str(), iTwType, p_Value, sTwParams.c_str());
        } else {
            std::cerr << "Unknown tweak type: " << typeid(TValueType).name() << std::endl;
        }
    }
}

template<typename TValueType>
void CEngine::setOption(
    const std::string sName,
    TValueType _InValue,
    const std::string &sTwParams
) {
    auto it = m_clOptions.find(sName);
    if(it != m_clOptions.end()) {
        const std::type_info &stTypeInfo = it->second.first;
        TValueType *p_Value = (TValueType*)it->second.second;
        
        assert(stTypeInfo == typeid(TValueType));
        
        *p_Value = _InValue;
        
    } else {
        TValueType *p_Value = new TValueType(_InValue);
        m_clOptions.insert(
            std::pair<std::string, std::pair<const std::type_info &, void*>>(
                sName,
                std::pair<const std::type_info &, void*>(typeid(TValueType), (void*)p_Value)
            )
        );
        
        TwType iTwType = getTwType(typeid(TValueType));
        
        if(iTwType != TW_TYPE_UNDEF) {
            TwAddVarRW(m_pTweakBar, sName.c_str(), iTwType, p_Value, sTwParams.c_str());
        } else {
            std::cerr << "Unknown tweak type: " << typeid(TValueType).name() << std::endl;
        }
    }
}

template<typename TValueType>
void CEngine::setWatchOutput(
    const std::string sName,
    TValueType _InValue,
    const std::string &sTwParams
) {
    auto it = m_clWatchValues.find(sName);
    if(it != m_clWatchValues.end()) {
        const std::type_info &stTypeInfo = it->second.first;
        TValueType *p_Value = (TValueType*)it->second.second;
        
        assert(stTypeInfo == typeid(TValueType));
        
        *p_Value = _InValue;
        
        TwRefreshBar(m_pTweakBar);
        
    } else {
        TValueType *p_Value = new TValueType(_InValue);
        m_clWatchValues.insert(
            std::pair<std::string, std::pair<const std::type_info &, void*>>(
                sName,
                std::pair<const std::type_info &, void*>(typeid(TValueType), (void*)p_Value)
            )
        );
        
        TwType iTwType = getTwType(typeid(TValueType));
        
        if(iTwType != TW_TYPE_UNDEF) {
            TwAddVarRO(m_pTweakBar, sName.c_str(), iTwType, p_Value, sTwParams.c_str());
        } else {
            std::cerr << "Unknown tweak type: " << typeid(TValueType).name() << std::endl;
        }
    }
}
template<typename TValueType>
void CEngine::watchVariable(
    const std::string sName,
    TValueType *p_Variable,
    const std::string &sTwParams,
    bool bReadOnly
) {
    auto it = m_clWatchValues.find(sName);
    if(it != m_clWatchValues.end()) {
        const std::type_info &stTypeInfo = it->second.first;
        
        assert(stTypeInfo == typeid(TValueType));
        
        it->second.second = p_Variable;
        
        TwRefreshBar(m_pTweakBar);
        
    } else {
        m_clWatchValues.insert(
            std::pair<std::string, std::pair<const std::type_info &, void*>>(
                sName,
                std::pair<const std::type_info &, void*>(typeid(TValueType), (void*)p_Variable)
            )
        );
        
        TwType iTwType = getTwType(typeid(TValueType));
        
        if(iTwType != TW_TYPE_UNDEF) {
            //printf("%p %s %d %p %s\n", m_pTweakBar, sName.c_str(), iTwType, p_Variable, "");
            if(bReadOnly) {
                TwAddVarRO(m_pTweakBar, sName.c_str(), iTwType, p_Variable, sTwParams.c_str());
            } else {
                TwAddVarRW(m_pTweakBar, sName.c_str(), iTwType, p_Variable, sTwParams.c_str());
            }
        } else {
            std::cerr << "Unknown tweak type: " << typeid(TValueType).name() << std::endl;
        }
    }
}

extern void TW_CALL CEngine__buttonCallback(bool *state);

#endif // header
