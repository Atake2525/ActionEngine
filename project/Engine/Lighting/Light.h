#include <wrl.h>
#include <d3d12.h>
#include "Vector3.h"
#include "Vector4.h"

#pragma once

struct DirectionalLight {
	Vector4 color;     //!< ライトの色
	Vector3 direction; //!< ライトの向き
	float intensity;   //!< 輝度
	Vector3 specularColor;
	float padding[2];
};

struct PointLight {
	Vector4 color;    //!< ライトの色
	Vector3 position; //!< ライトの位置
	float intensity;  //!< 輝度
	float radius;     //!< ライトの届く最大距離
	float dacay;      //!< 減衰率
	float padding[2];
	Vector3 specularColor;
};

struct SpotLight {
	Vector4 color;         //!< ライトの色
	Vector3 position;      //!< ライトの位置
	float intensity;       //!< 輝度
	Vector3 direction;     //!< ライトの向き
	float distance;        //!< ライトの届く最大距離
	float dacay;           //!< 減衰率
	float cosAngle;        //!< スポットライトの余弦
	float cosFalloffStart; // falloffが開始される角度
	float padding1;
	Vector3 specularColor;
	float padding2;
};

class DirectXBase;

class Light {
	// シングルトンパターンを適用
	static Light* instance;

	// コンストラクタ、デストラクタの隠蔽
	Light() = default;
	~Light() = default;
	// コピーコンストラクタ、コピー代入演算子の封印
	Light(Light&) = delete;
	Light& operator=(Light&) = delete;
public:

	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns>Light* instance</returns>
	static Light* GetInstance();

	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();

	void Initialize(DirectXBase* directxBase);

	void Update();

	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetDirectionalLightResource() const { return directionalLightResource; }

	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetPointlLightResource() const { return pointLightResource; }

	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetSpotLightResource() const { return spotLightResource; }

private:
	// ライトリソース宣言
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;

	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource;

	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource;

	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;

	DirectionalLight* directionalLightData = nullptr;

	PointLight* pointLightData = nullptr;

	SpotLight* spotLightData = nullptr;

	DirectXBase* directxBase_ = nullptr;

public:
	/// ↓----- Getter -----↓ ///

	// ↓-- DirectionalLight --↓

	// ライトの色
	const Vector4& GetColorDirectionalLight() const { return directionalLightData->color; }
	// ライトの向き
	const Vector3& GetDirectionDirectionalLight() const { return directionalLightData->direction; }
	// 輝度
	const float& GetIntensityDirectionalLight() const { return directionalLightData->intensity; }
	// 鏡面反射色
	const Vector3& GetSpecularColorDirectionalLight() const { return directionalLightData->specularColor; }

	// ↑-- DirectionalLight --↑


	// ↓-- PointLight --↓

	// ライトの色
	const Vector4& GetColorPointLight() const { return pointLightData->color; }
	// ライトの位置
	const Vector3& GetPositionPointLight() const { return pointLightData->position; }
	// 輝度
	const float& GetIntensityPointLight() const { return pointLightData->intensity; }
	// ライトの届く最大距離
	const float& GetRadiusPointLight() const { return pointLightData->radius; }
	// 減衰率
	const float& GetdacayPointLight() const { return pointLightData->dacay; }
	// 鏡面反射色
	const Vector3& GetSpecularColorPointLight() const { return pointLightData->specularColor; }

	// ↑-- PointLight --↑


	// ↓-- SpotLight --↓

	// ライトの色
	void GetColorSpotLight() const { spotLightData->color; }
	// ライトの位置
	void GetPositionSpotLight() const { spotLightData->position; }
	// 輝度
	void GetIntensitySpotLight() const { spotLightData->intensity; }
	// ライトの向き
	void GetDirectionSpotLight() const { spotLightData->direction; }
	// ライトの届く最大距離
	void GetDistanceSpotLight() const { spotLightData->distance; }
	// 減衰率
	void GetdacaySpotLight() const { spotLightData->dacay; }
	// スポットライトの余弦
	void GetcosAngleSpotLight() const { spotLightData->cosAngle; }
	// falloffが開始される角度
	void GetcosFalloffStartSpotLight() const { spotLightData->cosFalloffStart; }
	// 鏡面反射色
	void GetSpecularColorSpotLight() const { spotLightData->specularColor; }

	// ↑--SpotLight --↑

	/// ↑----- Getter -----↑ ///

	/// ↓----- Setter -----↓ ///

	// ↓-- DirectionalLight --↓

	// ライトの色
	void SetColorDirectionalLight(const Vector4& color) const { directionalLightData->color = color; }
	// ライトの向き
	void SetDirectionDirectionalLight(const Vector3& direction) const { directionalLightData->direction = direction; }
	// 輝度
	void SetIntensityDirectionalLight(const float& intensity) const { directionalLightData->intensity = intensity; }
	// 鏡面反射色
	void SetSpecularColorDirectionalLight(const Vector3& specularColor) const { directionalLightData->specularColor = specularColor; }

	// ↑-- DirectionalLight --↑


	// ↓-- PointLight --↓

	// ライトの色
	void SetColorPointLight(const Vector4& color) const { pointLightData->color = color; }
	// ライトの位置
	void SetPositionPointLight(const Vector3& position) const { pointLightData->position = position; }
	// 輝度
	void SetIntensityPointLight(const float& intensity) const { pointLightData->intensity = intensity; }
	// ライトの届く最大距離
	void SetRadiusPointLight(const float& radius) const { pointLightData->radius = radius; }
	// 減衰率
	void SetdacayPointLight(const float& dacay) const { pointLightData->dacay = dacay; }
	// 鏡面反射色
	void SetSpecularColorPointLight(const Vector3& specularColor) const { pointLightData->specularColor = specularColor; }

	// ↑-- PointLight --↑


	// ↓-- SpotLight --↓

	// ライトの色
	void SetColorSpotLight(const Vector4& color) const { spotLightData->color = color; }
	// ライトの位置
	void SetPositionSpotLight(const Vector3& position) const { spotLightData->position = position; }
	// 輝度
	void SetIntensitySpotLight(const float& intensity) const { spotLightData->intensity = intensity; }
	// ライトの向き
	void SetDirectionSpotLight(const Vector3& direction) const { spotLightData->direction = direction; }
	// ライトの届く最大距離
	void SetDistanceSpotLight(const float& distance) const { spotLightData->distance = distance; }
	// 減衰率
	void SetdacaySpotLight(const float& dacay) const { spotLightData->dacay = dacay; }
	// スポットライトの余弦
	void SetcosAngleSpotLight(const float& cosAngle) const { spotLightData->cosAngle = cosAngle; }
	// falloffが開始される角度
	void SetcosFalloffStartSpotLight(const float& cosFalloffStart) const { spotLightData->cosFalloffStart = cosFalloffStart; }
	// 鏡面反射色
	void SetSpecularColorSpotLight(const Vector3& specularColor) const { spotLightData->specularColor = specularColor; }

	// ↑--SpotLight --↑

	/// ↑----- Setter -----↑ ///
};

