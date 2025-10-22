#pragma once

struct CullingTemplate {
	float drawHeight;
};

inline CullingTemplate operator+(const CullingTemplate& a, const CullingTemplate& b) {
	CullingTemplate result;
	result.drawHeight = a.drawHeight + b.drawHeight;
	return result;
}

inline CullingTemplate operator+=(CullingTemplate& a, const CullingTemplate& b) {
	a.drawHeight += b.drawHeight;
	return a;
}