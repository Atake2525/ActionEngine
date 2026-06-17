#include "Sprite.h"
#include "Render2DBase.h"
#include "DirectXBase.h"
#include "TextureManager.h"
#include "SrvManager.h"
#include "AABB.h"

void Sprite::SetTransform(const Transform& transform){ 
	position.x = transform.translate.x;
	position.y = transform.translate.y;
	rotation = transform.rotate.z;
	scale.x = transform.scale.x;
	scale.y = transform.scale.y;
}

const Transform Sprite::GetTransform() const {
	Transform result;
	result.translate = { position.x, position.y, 0.0f };
	result.rotate = {0.0f, 0.0f, rotation };
	result.scale = { scale.x, scale.y, 1.0f };
	return result;
}

void Sprite::SetStatus(const Vector2& position, const float& rotation, const Vector2& scale, const Vector4& color){ 
	this->position = position; 
	this->rotation = rotation;
	this->scale = scale;
	materialData->color = color;
}

void Sprite::SetTransform(const Vector2& position, const float& rotation, const Vector2& scale) {
	this->position = position;
	this->rotation = rotation;
	this->scale = scale;
}

void Sprite::SetTexture(const std::string& textureFilePath) {
	texturefilePath = textureFilePath;
	textureIndex = TextureManager::GetInstance()->LoadTexture(textureFilePath);
	AdjustTextureSize();
}


void Sprite::Initialize(std::string textureFilePath) { 

	// MaterialResourceの作成
	CreateMaterialResource();
	// TransformationMatrixResourceの作成
	CreateTransformationMatrixResource();

	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));

	// MaterialBufferViewの作成
	SetMaterial();
	// TransformationMatrixBufferViewの作成
	SetTransformatinMatrix();

	texturefilePath = textureFilePath;
	textureIndex = TextureManager::GetInstance()->LoadTexture(textureFilePath);

	// テクスチャサイズの計算
	AdjustTextureSize();

}

void Sprite::Update() {
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(texturefilePath);

	Transform transform{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
    };

	Transform uvTransform{
	    {1.0f, 1.0f, 1.0f},
	    {0.0f, 0.0f, 0.0f},
	    {0.0f, 0.0f, 0.0f},
	};

	transform.translate = {position.x, position.y, 0.0f};
	transform.rotate = {0.0f, 0.0f, rotation};
	transform.scale = {scale.x, scale.y, 0.1f};

	uvTransform.scale = {
		textureSize.x / static_cast<float>(metadata.width),
		textureSize.y / static_cast<float>(metadata.height),
		1.0f
	};
	uvTransform.translate = {
		textureLeftTop.x / static_cast<float>(metadata.width),
		textureLeftTop.y / static_cast<float>(metadata.height),
		0.0f
	};

	Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransform.scale);
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransform.rotate.z));
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransform.translate));
	materialData->uvTransform = uvTransformMatrix;

	// ゲームの処理
	//  Sprite用のWorldViewProjectionMatrixを作る
	//  SpriteのTransform処理
	Matrix4x4 anchorMatrix = MakeTranslateMatrix({ -anchorPoint.x, -anchorPoint.y, 0.0f });
	Matrix4x4 worldMatrix = Multiply(anchorMatrix, MakeAffineMatrix(transform.scale, transform.rotate, transform.translate));
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::GetInstance()->GetkClientWidth()), float(WinApp::GetInstance()->GetkClientHeight()), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;
}

void Sprite::ChangeTexture(std::string textureFilePath) { 
	textureIndex = TextureManager::GetInstance()->LoadTexture(textureFilePath);
}

void Sprite::Draw() {
	// Spriteの描画。変更が必要なものだけ変更する
	const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView = Render2DBase::GetInstance()->GetSpriteVertexBufferView();
	const D3D12_INDEX_BUFFER_VIEW& indexBufferView = Render2DBase::GetInstance()->GetSpriteIndexBufferView();
	DirectXBase::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定

	DirectXBase::GetInstance()->GetCommandList()->IASetIndexBuffer(&indexBufferView); // IBVを設定

	// マテリアルCBufferの場所を設定
	DirectXBase::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// TransformationMatrixCBBufferの場所を設定
	DirectXBase::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	//SpriteBase::GetInstance()->GetDxBase()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex));
	SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(2, textureIndex);
	// 描画
	DirectXBase::GetInstance()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::CreateMaterialResource() { 
	materialResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(Material));
}

void Sprite::CreateTransformationMatrixResource() { 
	transformationMatrixResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
}

void Sprite::SetMaterial() {
	// マテリアルデータの初期値を書き込む
	materialData->color = Vector4{1.0f, 1.0f, 1.0f, 1.0f};
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();
}

void Sprite::SetTransformatinMatrix() {
	// 単位行列を書き込んでおく
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();
}

void Sprite::AdjustTextureSize() {
	// テクスチャメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(texturefilePath);

	textureSize.x = static_cast<float>(metadata.width);
	textureSize.y = static_cast<float>(metadata.height);
	// 画像サイズをテクスチャサイズに合わせる
	scale = textureSize;
}
