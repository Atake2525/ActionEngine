#include <vector>
#include <wrl.h>
#include <d3d12.h>
#include <string>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "AABB.h"
#include "kMath.h"
#include "Quaternion.h"
#include "Animator.h"
#include "Model.h"
//#include "DebugAnimation.h"

#pragma once

//class Model;
class Camera;

class Object3d {
public: // メンバ関数
	// 初期化
	void Initialize();

	// 更新
	void Update();

	// SkinCulsterの更新
	void UpdateSkinCluster(std::vector<SkinCluster>& skinCluster, const Skeleton& skeleton);

	// デストラクタ
	~Object3d();

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="worldPos">CameraPosition</param>
	void Draw();

	void SetModel(const std::string& filePath);

	void SetModel(const std::string& directoryPath, const std::string& filePath, const bool& enableLighting = false, const bool isAnimation = false);

	void SetCamera(Camera* camera) { this->camera = camera; }

	void ToggleStartAnimation() { startAnimation = !startAnimation; }

	void SetStartAnimation(bool start) { startAnimation = start; }

	void ResetAnimationTime();

	void AddAnimation(std::string directoryPath, std::string filename, std::string animationName);

	void PlayDefaultAnimation();

	void ChangePlayAnimation(const std::string key = "DefaultAnimation");

	void ResetAnimationSpeed() { animationSpeed = 1.0f; }

	const float& GetChangeAnimationSpeed() const { return changeAnimationSpeed; }
	// アニメーションの切り替え時間の設定 Default : 0.4f
	void SetChangeAnimationSpeed(const float speed = 0.4f) { changeAnimationSpeed = speed; }

	// Parentを登録(子)
	// Setter(Parent)
	void SetParent(const Matrix4x4& worldMatrix) {
		parent = worldMatrix;
		isParent = true;
		isTranslateParent = false;
		isRotateParent = false;
	}
	// Delete Parent(Parent 解除)
	void DeleteParent() {
		isParent = false;
	}
	// Setter(Parent)
	void SetTranslateParent(const Matrix4x4& worldMatrix) {
		translateParent = worldMatrix;
		isTranslateParent = true;
		isParent = false;
		isRotateParent = false;
	}
	// Delete Parent(Parent 解除)
	void DeleteTranslateParent() {
		isTranslateParent = false;
	}
	// Setter(Parent)
	void SetRotateParent(const Matrix4x4& worldMatrix) {
		rotateParent = worldMatrix;
		isRotateParent = true;
		isTranslateParent = false;
		isParent = false;
	}
	// Delete Parent(Parent 解除)
	void DeleteRotateParent() {
		isRotateParent = false;
	}

private:

	Transform transform;

	Vector3 axisAngle;


	Matrix4x4 rotateQuaternionMatrix;

	Matrix4x4 parent;
	bool isParent = false;
	Matrix4x4 translateParent;
	bool isTranslateParent = false;
	Matrix4x4 rotateParent;
	bool isRotateParent = false;

	Camera* camera = nullptr;

private:

	struct CameraForGPU {
		Vector3 worldPosition;
	};

	// 座標変換リソースのバッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	// 座標変換行列リソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrix = nullptr;

	// PhongShading用カメラ
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;
	CameraForGPU* cameraData = nullptr;

	Model* model_ = nullptr;

	std::map<std::string, Animation> animation;
	std::string beforAnimationKey = "DefaultAnimation";
	std::string animationKey = "DefaultAnimation";

	bool changingAnimation = false;

	float animationTime = 0.0f;
	float changeAnimationTime = 0.0f;
	float changeAnimationSpeed = 0.4f;

	bool startAnimation = false;
	float animationSpeed = 1.0f;

	Skeleton skeleton;

	std::vector<SkinCluster> skinCluster;


	// 衝突判定に必要

	// Getterに返すようのAABB(座標を更新する)
	AABB aabb;

	std::vector<AABB> multiMeshAABB;

	// 初期位置のAABB
	AABB first;

	std::vector<AABB> firstMultiMeshAABB;

	Matrix4x4 worldMatrix;

public:

	// Getter(Transform)
	const Transform& GetTransform() const { return transform; }
	// Getter(Translate)
	const Vector3& GetTranslate() const { return transform.translate; }
	// Getter(Scale)
	const Vector3& GetScale() const { return transform.scale; }
	// Getter(Rotate)
	const Vector3& GetRotate() const { return transform.rotate; }
	// Getter(Rotate Degree)
	const Vector3 GetRotateInDegree() const;
	// Gettre(Color)
	const Vector4& GetColor() const;
	// Getter(EnableLighting)
	const bool GetEnableLighting() const;
	// 環境マップの反射数値
	const float GetEnvironmentCoefficient() const;
	// Getter(shininess)
	const float& GetShininess() const;
	// Getter(AABB)
	const AABB& GetAABB() const { return aabb; }
	// Getter(worldMatrix)
	const Matrix4x4& GetWorldMatrix() const { return worldMatrix; }
	// アニメーションの再生速度を取得
	const float& GetAnimationSpeed() const { return animationSpeed; }

	// Setter(Transform)
	void SetTransform(const Transform& transform) { this->transform = transform; }
	// Setter(Transform, pos,scale,rotate)
	void SetTransform(const Vector3& translate, const Vector3& scale, const Vector3& rotate);
	// Setter(Translate)
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }
	// Setter(Scale)
	void SetScale(const Vector3& scale) { transform.scale = scale; }
	// Setter(Rotate)
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	// Setter(Rotate Degree)
	void SetRotateInDegree(const Vector3& rotate);
	// Setter(Color)
	void SetColor(const Vector4 color);
	// Setter(EnableLighting)
	void SetEnableLighting(const bool enableLighting);
	// Setter(shininess)
	void SetShininess(const float& shininess);
	// 任意軸回転の軸を指定の回転角に変更
	void SetAxisAngle(const Vector3& rotate) { axisAngle = Normalize(rotate); }
	// 任意軸回転の回転量を設定
	void SetQuaternionAngle(const float& angle) { rotateQuaternionMatrix = MakeRotateAxisAngle(axisAngle, angle); }
	// デバッグ(ワイヤーフレーム)用にテクスチャをwhite1x1に変更
	void DebugMode(const bool debugMode);
	// アニメーションの再生速度の設定(初期値 : 1.0f)
	void SetAnimationSpeed(const float speed) { animationSpeed = speed; }

	const Vector3 GetJointPosition(const std::string jointName);

	const Vector3 GetJointNormal(const std::string jointName);


	void SetEnvironmentCoefficient(const float amount);

	void SetEnableMetallic(const bool flag) { model_->SetEnableMetallic(flag); }

	const bool GetEnableMetallic() { return model_->GetEnableMetallic(); }

public:
	// 衝突チェック(AABBとAABB)
	const bool CheckCollision(Object3d* object) const;

	const std::vector<AABB> GetAABBMultiMeshed();

	//const bool& CheckCollisionSphere(const Sphere& sphere) const;

private:

	// TransformationMatrixResourceを作る
	void CreateTransformationMatrixResource();
	// CameraResourceを作る
	void CreateCameraResource();

	// AABBをモデルを参照して自動的に作成
	void CreateAABB();

	// アニメーションの適用(Skeltonに対してAnimationの適用を行う)
	void ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime);

	const bool ChangeAnimation(Animation& beforAnimation, Animation& afterAnimation, float animationTime, float changeTime);

	// skeltonの作成
	const Skeleton CreateSkelton(const Node& rootNode);
	// Jointの作成
	const int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints);

	void UpdateSkelton(Skeleton& skelton);

	// SkinClusterの作成
	std::vector<SkinCluster> CreateSkinCluster(const Skeleton& skeleton, const ModelData& modelData);

};
