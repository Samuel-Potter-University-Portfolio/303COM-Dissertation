#include "PerlinNoise.h"

#include <list>


/**
* Fade the value so that it will ease in towards intergral values
*/
inline static float FadeValue(const float& v)
{
	return v*v*v * (v * (v * 6.0f - 15.0f) + 10.0f);
}

/**
* Calculate the gradient by taking the dot product of the values at each coordinate
* Where the normals are assigned as follows:
* (1,1,0),(-1,1,0),(1,-1,0),(-1,-1,0),
* (1,0,1),(-1,0,1),(1,0,-1),(-1,0,-1),
* (0,1,1),(0,-1,1),(0,1,-1),(0,-1,-1)
* http://riven8192.blogspot.com/2010/08/calculate-perlinnoise-twice-as-fast.html
*/
inline static float CalculateGradient(const uint8& hash, const float& x, const float& y, const float& z)
{
	switch (hash & 0xF)
	{
	case 0x0: return  x + y;
	case 0x1: return -x + y;
	case 0x2: return  x - y;
	case 0x3: return -x - y;
	case 0x4: return  x + z;
	case 0x5: return -x + z;
	case 0x6: return  x - z;
	case 0x7: return -x - z;
	case 0x8: return  y + z;
	case 0x9: return -y + z;
	case 0xA: return  y - z;
	case 0xB: return -y - z;
	case 0xC: return  y + x;
	case 0xD: return -y + z;
	case 0xE: return  y - x;
	case 0xF: return -y - z;
	default: return 0;
	}
}

/**
* Perform cosine interpolation between A and B
* @param t			From 0-1 (0 = a, 1 = b)
*/
inline float CosLerp(const float& a, const float& b, const float& t)
{
	const float mu = (1.0f - std::cosf(t * 3.14152f)) * 0.5f;
	return a * (1.0f - mu) + b * mu;
}


PerlinNoise::PerlinNoise(const uint32 seed) : m_seed(seed)
{
	// Hash lookup table (By Ken Perlin) for all numbers from 0-255 randomly arranged
	std::list<uint8> basePermutations
	{
		151,160,137,91,90,15, 131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
		190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33, 88,237,149,56,87,174,
		20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166, 77,146,158,231,83,111,229,122,60,211,133,
		230,220,105,92,41,55,46,245,40,244, 102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,
		18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,5,202,
		38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,
		2,44,154,163,70,221,153,101,155,167, 43,172,9,129,22,39,253, 19,98,108,110,79,113,224,232,178,185,
		112,104,218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,138,236,205,93,222,114,
		67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};

	// Shuffle permutation table based on noise
	m_permutationTable.reserve(512 * 2);
	srand(seed);

	while (basePermutations.size() != 0)
	{
		const uint32 index = rand() % basePermutations.size();
		auto it = basePermutations.begin();
		if (index != 0)
			std::advance(it, index);

		m_permutationTable.push_back(*it);
		basePermutations.erase(it);
	}
}

float PerlinNoise::Get01(const float& x, const float& y, const float& z) const
{
	// Transfrom input based on seed
	float tX = std::abs(x + 91.25f);
	float tY = std::abs(y - 71.33f);
	float tZ = std::abs(z + 110.73f);

	// Get whole and fraction for xyz
	int32 xW = (int32)tX & 255;
	int32 yW = (int32)tY & 255;
	int32 zW = (int32)tZ & 255;
	float xF = tX - (int32)tX;
	float yF = tY - (int32)tY;
	float zF = tZ - (int32)tZ;


	// Get values to use for interpolation
	float u = FadeValue(xF);
	float v = FadeValue(yF);
	float w = FadeValue(zF);


	/// Get values at each cell
	uint8 v000 = GetPermutation(GetPermutation(GetPermutation(xW + 0) + yW + 0) + zW + 0);
	uint8 v001 = GetPermutation(GetPermutation(GetPermutation(xW + 0) + yW + 0) + zW + 1);
	uint8 v010 = GetPermutation(GetPermutation(GetPermutation(xW + 0) + yW + 1) + zW + 0);
	uint8 v011 = GetPermutation(GetPermutation(GetPermutation(xW + 0) + yW + 1) + zW + 1);
	uint8 v100 = GetPermutation(GetPermutation(GetPermutation(xW + 1) + yW + 0) + zW + 0);
	uint8 v101 = GetPermutation(GetPermutation(GetPermutation(xW + 1) + yW + 0) + zW + 1);
	uint8 v110 = GetPermutation(GetPermutation(GetPermutation(xW + 1) + yW + 1) + zW + 0);
	uint8 v111 = GetPermutation(GetPermutation(GetPermutation(xW + 1) + yW + 1) + zW + 1);


	// Get interpolated values along each axiss
	const float vX00 = CosLerp(CalculateGradient(v000, xF - 0, yF - 0, zF - 0), CalculateGradient(v100, xF - 1, yF - 0, zF - 0), u);
	const float vX10 = CosLerp(CalculateGradient(v010, xF - 0, yF - 1, zF - 0), CalculateGradient(v110, xF - 1, yF - 1, zF - 0), u);
	const float vX01 = CosLerp(CalculateGradient(v001, xF - 0, yF - 0, zF - 1), CalculateGradient(v101, xF - 1, yF - 0, zF - 1), u);
	const float vX11 = CosLerp(CalculateGradient(v011, xF - 0, yF - 1, zF - 1), CalculateGradient(v111, xF - 1, yF - 1, zF - 1), u);

	const float vY0 = CosLerp(vX00, vX10, v);
	const float vY1 = CosLerp(vX01, vX11, v);

	// Clamp final value between 0-1
	return (CosLerp(vY0, vY1, w) + 1.0f) * 0.5f;
}

float PerlinNoise::Get01(const float& x, const float& y) const
{
	// Transfrom input based on seed
	float tX = std::abs(x + 91.25f);
	float tY = std::abs(y - 71.33f);

	// Get whole and fraction for xyz
	int32 xW = (int32)tX & 255;
	int32 yW = (int32)tY & 255;
	float xF = tX - (int32)tX;
	float yF = tY - (int32)tY;


	// Get values to use for interpolation
	float u = FadeValue(xF);
	float v = FadeValue(yF);


	/// Get values at each cell
	uint8 v00 = GetPermutation(GetPermutation(xW + 0) + yW + 0);
	uint8 v01 = GetPermutation(GetPermutation(xW + 0) + yW + 1);
	uint8 v10 = GetPermutation(GetPermutation(xW + 1) + yW + 0);
	uint8 v11 = GetPermutation(GetPermutation(xW + 1) + yW + 1);

	// Get interpolated values along each axiss
	const float vX0 = CosLerp(CalculateGradient(v00, xF - 0, yF - 0, 0), CalculateGradient(v10, xF - 1, yF - 0, 0), u);
	const float vX1 = CosLerp(CalculateGradient(v01, xF - 0, yF - 1, 0), CalculateGradient(v11, xF - 1, yF - 1, 0), u);

	// Clamp final value between 0-1
	return (CosLerp(vX0, vX1, v) + 1.0f) * 0.5f;
}

float PerlinNoise::GetOctave(const float& x, const float& y, const float& z, const uint32& octaves, const float& persistence) const
{
	float frequency = 1.0f;
	float amplitude = 1.0f;

	float value = 0.0f;
	float maximum = 0.0f;

	for (uint32 i = 0; i < octaves; ++i)
	{
		value += Get01(x * frequency, y * frequency, z* frequency) * amplitude;

		// Decay values with each iteration
		maximum += amplitude;
		amplitude *= persistence;
		frequency *= 2.0f;
	}

	return value / maximum;
}

float PerlinNoise::GetOctave(const float& x, const float& y, const uint32& octaves, const float& persistence) const
{
	float frequency = 1.0f;
	float amplitude = 1.0f;

	float value = 0.0f;
	float maximum = 0.0f;

	for (uint32 i = 0; i < octaves; ++i)
	{
		value += Get01(x * frequency, y * frequency) * amplitude;

		// Decay values with each iteration
		maximum += amplitude;
		amplitude *= persistence;
		frequency *= 2.0f;
	}

	return value / maximum;
}