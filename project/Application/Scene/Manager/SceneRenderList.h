#pragma once

#include <memory>
#include <vector>

#include "Object3d.h"
#include "Sprite.h"

class SceneRenderList {
public:
    Object3d* AddObject3d(std::unique_ptr<Object3d> object) {
        Object3d* ptr = object.get();
        m_objects.push_back(std::move(object));
        return ptr;
    }

    Sprite* AddSprite(std::unique_ptr<Sprite> sprite) {
        Sprite* ptr = sprite.get();
        m_sprites.push_back(std::move(sprite));
        return ptr;
    }

    void UpdateObject3d() {
        for (auto& object : m_objects) {
            object->Update();
        }
    }

    void UpdateSprites() {
        for (auto& sprite : m_sprites) {
            sprite->Update();
        }
    }

    void Update() {
        UpdateObject3d();
        UpdateSprites();
    }

    void DrawObject3d() {
        for (auto& object : m_objects) {
            object->Draw();
        }
    }

    void DrawSprites() {
        for (auto& sprite : m_sprites) {
            sprite->Draw();
        }
    }

    void Clear() {
        m_sprites.clear();
        m_objects.clear();
    }

private:
    std::vector<std::unique_ptr<Object3d>> m_objects;
    std::vector<std::unique_ptr<Sprite>> m_sprites;
};
