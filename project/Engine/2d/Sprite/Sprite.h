#include <d3d12.h>
#include <wrl.h>
#include "WinApp.h"
#include "kMath.h"
#include "Transform.h"
#include <string>
#include "SpriteDefinitions.h"
#include "externals/DirectXTex/DirectXTex.h"


#pragma once

class Render2DBase;
class TextureManager;
class WinApp;
class SrvManager;
class DirectXBase;

// スプライト
class Sprite {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(std::string textureFilePath, DirectXBase& directXBase, TextureManager& textureManager);

	/// <summary>
	/// 更新
	/// </summary>
	void Update(WinApp& winApp);

	/// <summary>
	/// テクスチャ変更
	/// </summary>
	void ChangeTexture(std::string textureFilePath, TextureManager& textureManager);
	
	/// <summary>
	/// 描画
	/// </summary>
	void Draw(DirectXBase& directXBase, SrvManager& srvManager);

private:

	DirectX::TexMetadata m_metaData;

	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};

	struct Material {
		Vector4 color;
		int32_t enableLighting;
		float pad[3];
		Matrix4x4 uvTransform;
		float shininess;
		Vector3 specularColor;
	};

	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};

	// VertexResourceを作成する
	void CreateVertexResource(DirectXBase& directXBase);
	// IndexResourceを作成する
	void CreateIndexResource(DirectXBase& directXBase);
	// MaterialResouceを作成する
	void CreateMaterialResource(DirectXBase& directXBase);
	// TransformationMatrixを作成する
	void CreateTransformationMatrixResource(DirectXBase& directXBase);

	// VertexBufferViewを作成する (値を設定するだけ)
	void CreateVertexBufferView();
	// IndexBufferViewを作成する (値を設定するだけ)
	void CreateIndexBufferView();
	// Materialの値を設定
	void SetMaterial();
	// TransformationMatrixの値を設定
	void SetTransformatinMatrix();

	// テクスチャサイズを死めーじに合わせる
	void AdjustTextureSize();

private:

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	// バッファリソース内のデータを指すポインタ
	uint32_t* indexData = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	// バッファリソース内のデータを指すポインタ
	Material* materialData = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData = nullptr;

	
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	D3D12_INDEX_BUFFER_VIEW indexbufferView = {};

private:

	std::string texturefilePath;

	// 位置
	Vector2 position = {0.0f, 0.0f};
	// 回転
	float rotation = 0.0f;
	// 拡縮
	Vector2 scale = {0.0f, 0.0f};
	// 回転の原点
	Vector2 anchorPoint = ANCHORPOINT_LEFTTOP;
	// テクスチャ左上座標
	Vector2 textureLeftTop = {0.0f, 0.0f};
	// テクスチャ切り出しサイズ
	Vector2 textureSize = {100.0f, 100.0f};

	// テクスチャ番号
	uint32_t textureIndex = 0;

public:
	// Getter(Position)
	const Vector2& GetPosition() const { return position; }
	// Getter(Rotation)
	const float& GetRotation() const { return rotation; }
	// Getter(Scale)
	const Vector2& GetScale() const { return scale; }
	// Getter(Color)
	const Vector4& GetColor() const { return materialData->color; }
	// Getter(AnchorPoint)
	const Vector2& GetAnchorPoint() const { return anchorPoint; }
	// Getter(TextureLeftTop)
	const Vector2& GetTextureLeftTop() const { return textureLeftTop; }
	// Getter(TextureSize)
	const Vector2& GetTextureSize() const { return textureSize; }
	// Getter(Transform)
	const Transform GetTransform() const;

	// Setter(Position)
	void SetPosition(const Vector2& pos) { position = pos; }
	// Setter(Rotation)
	void SetRotatioin(const float& rotate) { rotation = rotate; }
	// Setter(Scale)
	void SetScale(const Vector2& size) { scale = size; }
	// Setter(Color)
	void SetColor(const Vector4& color) { materialData->color = color; }
	// Setter(AnchorPoint)
	void SetAnchorPoint(const Vector2& anchPoint) { anchorPoint = anchPoint; }
	// Setter(TextureLeftTop)
	void SetTextureLeftTop(const Vector2& LeftTop) { textureLeftTop = LeftTop; }
	// Setter(TextureSize)
	void SetTextureSize(const Vector2& size) { textureSize = size; }
	// Setter(Transform)
	void SetTransform(const Transform& transform);
	// Setter(Transform)
	void SetTransform(const Vector2& position, const float& rotation, const Vector2& scale);
	// Setter(Texture)
	void SetTexture(const std::string& textureFilePath, TextureManager& textureManager);
};