#pragma once

#include <cassert>
#include <memory>
#include <string>

#include "Object3d.h"
#include "Sprite.h"

class DirectXBase;
class GameTime;
class Object3dBase;
class Render2DBase;
class SrvManager;
class TextureManager;
class WinApp;

class Object3dFactory {
public:
    void SetContext(DirectXBase& directXBase, SrvManager& srvManager, Object3dBase& object3dBase, GameTime& gameTime) {
        m_pDirectXBase = &directXBase;
        m_pSrvManager = &srvManager;
        m_pObject3dBase = &object3dBase;
        m_pGameTime = &gameTime;
    }

    std::unique_ptr<Object3d> Create() {
        assert(m_pDirectXBase);
        assert(m_pSrvManager);
        assert(m_pObject3dBase);
        assert(m_pGameTime);

        Object3dContext context{
            *m_pDirectXBase,
            *m_pSrvManager,
            *m_pObject3dBase,
            *m_pGameTime,
        };

        auto object = std::make_unique<Object3d>();
        object->SetContext(context);
        object->Initialize();
        return object;
    }

private:
    DirectXBase* m_pDirectXBase = nullptr;
    SrvManager* m_pSrvManager = nullptr;
    Object3dBase* m_pObject3dBase = nullptr;
    GameTime* m_pGameTime = nullptr;
};

class SpriteFactory {
public:
    void SetContext(DirectXBase& directXBase, SrvManager& srvManager, TextureManager& textureManager, WinApp& winApp) {
        m_pDirectXBase = &directXBase;
        m_pSrvManager = &srvManager;
        m_pTextureManager = &textureManager;
        m_pWinApp = &winApp;
    }

    std::unique_ptr<Sprite> Create(const std::string& textureFilePath) {
        assert(m_pDirectXBase);
        assert(m_pSrvManager);
        assert(m_pTextureManager);
        assert(m_pWinApp);

        SpriteContext context{
            *m_pDirectXBase,
            *m_pSrvManager,
            *m_pTextureManager,
            *m_pWinApp,
        };

        auto sprite = std::make_unique<Sprite>();
        sprite->SetContext(context);
        sprite->Initialize(textureFilePath);
        return sprite;
    }

private:
    DirectXBase* m_pDirectXBase = nullptr;
    SrvManager* m_pSrvManager = nullptr;
    TextureManager* m_pTextureManager = nullptr;
    WinApp* m_pWinApp = nullptr;
};
