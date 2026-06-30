#include "WinApp.h"
#include "DirectXBase.h"
#include "Input.h"
#include "Render2DBase.h"
#include "Object3dBase.h"
#include "ModelBase.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Light.h"
#include "Audio.h"
#include "SrvManager.h"
#include "ImGuiManager.h"
#include "ParticleManager.h"
#include "SkyBox.h"
#include "JsonLoader.h"
#include "CollisionManager.h"
#include "FadeManager.h"
#include "GameTime.h"
#include "StageCount.h"
#include "DebugLineBase.h"
#include "SceneFactory.h"
#include "SceneManager.h"
#include "SettingManager.h"
#include "StageCount.h"
#include "OffScreenRendering.h"
#pragma once

struct PlatformContext {
    WinApp window;
    GameTime time;
    Input input;
};

struct GraphicsContext {
    DirectXBase dx;
    OffScreenRendering offScreen;
    SrvManager srv;
    ImGuiManager imgui;

    Render2DBase render2DBase;
    Object3dBase object3DBase;
    DebugLineBase debugLine;
};

struct AssetContext {
    TextureManager textures;
    ModelBase modelBase;
    ModelManager models;
    JsonLoader json;
    Audio audio;
};

struct WorldContext {
    Light light;
    CollisionManager collision;
    ParticleManager particles;
    SkyBox skyBox;
};

struct PresentationContext {
    FadeManager fade;
};

struct GameContext {
    SceneFactory sceneFactory;
    SceneManager sceneManager;
    SettingManager settings;
    ActionEngine::Stage::StageCount stageCount;
};

struct EngineContext {
    PlatformContext platform;
    GraphicsContext graphics;
    AssetContext assets;
    PresentationContext presentation;
};

struct AppContext {
    EngineContext engine;
    WorldContext world;
    GameContext game;
};