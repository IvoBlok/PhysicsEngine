#pragma once

#include <math.h>
#include <GLM/glm.hpp>
#include <vector>

//! Credit to the excellent wiki page https://en.wikipedia.org/wiki/Perlin_noise which this is strongely derived from (read copied)

float linearInterpolate(float firstValue, float secondValue, float weight) {
	return (secondValue - firstValue) * weight + firstValue;
}

glm::vec2 randomGradient(float x, float y) {
	// encode the given input coordinates into a single 'unique' random value
	const unsigned w = 8 * sizeof(unsigned);
	const unsigned s = w / 2; // rotation width
	unsigned a = x, b = y;
	a *= 3284157443; b ^= a << s | a >> w - s;
	b *= 1911520717; a ^= b << s | b >> w - s;
	a *= 2048419325;
	float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]
	glm::vec2 v;
	v.x = cos(random); v.y = sin(random);
	return v;
}

float dotGridGradient(int ix, int iy, float x, float y) {
	glm::vec2 gradient = randomGradient(ix, iy);

	float deltaX = x - (float)ix;
	float deltaY = y - (float)iy;

	return deltaX * gradient.x + deltaY * gradient.y;
}

float perlin(float x, float y) {
	// determine grid coordinates
	int xFloored = (int)floor(x);
	int xCeil = xFloored + 1;
	int yFloored = (int)floor(y);
	int yCeil = yFloored + 1;

	// determine interpolation weights
	float xDistanceToGrid = x - (float)xFloored;
	float yDistanceToGrid = y - (float)yFloored;

	float n0, n1, ix0, ix1;
	n0 = dotGridGradient(xFloored, yFloored, x, y);
	n1 = dotGridGradient(xCeil, yFloored, x, y);
	ix0 = linearInterpolate(n0, n1, xDistanceToGrid);
	
	n0 = dotGridGradient(xFloored, yCeil, x, y);
	n1 = dotGridGradient(xCeil, yCeil, x, y);
	ix1 = linearInterpolate(n0, n1, xDistanceToGrid);

	return linearInterpolate(ix0, ix1, yDistanceToGrid);
}

float layeredPerlin(float x, float y, std::vector<float> scale, std::vector<float> amplification, float xOffset = 0, float yOffset = 0) {
	if (scale.size() != amplification.size()) { return 0; }

	float value = 0;
	for (int i = 0; i < scale.size(); i++)
	{
		value += amplification[i] * perlin((x + xOffset) * (1 / scale[i]), (y + yOffset) * (1 / scale[i]));
	}
	return value;
}
