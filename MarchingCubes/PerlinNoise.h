#pragma once
#include "Common.h"

#include <vector>
#include <ctime>


/**
* A container to hold any perlin noise information
*/
class PerlinNoise
{
private:
	///
	/// Noise vars
	///
	const uint32 m_seed = 0;
	std::vector<uint8> m_permutationTable;

public:
	PerlinNoise(const uint32 seed = time(nullptr));

	/**
	* Calculate a 3D normalized perlin noise value for this coordinate
	* @param x,y,z				Coordinates to perform lookup for
	* @returns Value between 0-1
	*/
	float Get01(const float& x, const float& y, const float& z) const;
	/**
	* Calculate a 2D normalized perlin noise value for this coordinate
	* @param x,y				Coordinates to perform lookup for
	* @returns Value between 0-1
	*/
	float Get01(const float& x, const float& y) const;


	/**
	* Get noise by merging multiple 3D samples together to with given settings
	* @param x,y,z				Coordinates to perform lookup for
	* @param octaves			How many samples to blend between
	* @param persistence		How much amplitude decays by with each sample
	* @returns Value between 0-1
	*/
	float GetOctave(const float& x, const float& y, const float& z, const uint32& octaves, const float& persistence) const;
	/**
	* Get noise by merging multiple 2D samples together to with given settings
	* @param x,y				Coordinates to perform lookup for
	* @param octaves			How many samples to blend between
	* @param persistence		How much amplitude decays by with each sample
	* @returns Value between 0-1
	*/
	float GetOctave(const float& x, const float& y, const uint32& octaves, const float& persistence) const;

private:
	/** Get permutation at this index */
	inline uint8 GetPermutation(const uint32& i) const { return m_permutationTable[i % m_permutationTable.size()]; }

	///
	/// Getters & Setters
	///
public:
	inline uint32 GetSeed() const { return m_seed; }

	inline const uint8* GetTableData() const { return m_permutationTable.data(); }
	inline uint32 GetTableSize() const { return m_permutationTable.size(); }
};
