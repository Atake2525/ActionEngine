#include <Windows.h>

#pragma once


// 参考資料
// https://hakase0274.hatenablog.com/entry/2018/11/17/170918#%E3%83%95%E3%82%A9%E3%83%B3%E3%83%88%E8%AA%AD%E3%81%BF%E8%BE%BC%E3%81%BF

class Text
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	///  フォント読み込み
	/// </summary>
	void LoadFont();

	/// <summary>
	/// テクスチャ書き込み
	/// </summary>
	void EntryTexture();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

private:
	// フォントハンドルの生成
	int fontSize = 64;

	int fonrWeight = 1000;

	TEXTMETRIC tm;

	GLYPHMETRICS gm;

};

