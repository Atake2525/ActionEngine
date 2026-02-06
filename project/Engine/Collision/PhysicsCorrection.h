#include "Object3d.h"
#include <vector>
#pragma once

namespace PhysicsEngine {

    enum class CollisionShapeType
    {
        AABB,
        Sphere,
        OBB,
        Capsule,
    };

    struct CorrectionObject
    {
        Object3d* object;             // 補正対象のオブジェクト
        CollisionShapeType shapeType; // 衝突形状のタイプ
        bool isDynamic;               // 衝突によって動くかどうか
        bool useCCD;                  // 連続的衝突検出を使用するかどうか
        float weight;                 // 重量
    };

    class PhysicsCorrection
    {
    public:
        /// <summary>
        /// 補正の計算と適用
        /// </summary>
        void Update();

        /// <summary>
        /// 適用した情報を使ってObjectを更新
        /// </summary>
        void UpdateObject();

        void SetCorretionObject(const CorrectionObject& obj) { m_correctionObjects.push_back(obj); }

    private:
        std::vector<CorrectionObject> m_correctionObjects;

        Vector3 m_correction = Vector3::Zero; // 補正量

    };
}