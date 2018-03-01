#pragma once
#include "Common.h"

#include <vector>


/**
* Lets user load and access texture data
*/
class Texture
{
private:
	///
	/// IDs
	///
	uint32 m_id = 0;

	///
	/// Texture Settings
	///
	uint32 m_width;
	uint32 m_height;
	bool bIsLoaded = false;

	bool bUsesMipmaps = true;
	bool bIsRepeated = true;
	bool bIsSmooth = true;


public:
	Texture();
	~Texture();


	///
	/// Loading texture
	///
public:
	/**
	* Attempt to load a texture from an image
	* @param file		The path to the file to load from
	* @returns If succesful or not
	*/
	bool LoadFromFile(const string& file);

	/**
	* Attempt to load a texture from memory
	* @param data		The data to load
	* @param width		The width of the data
	* @param height		The height of the data
	* @param format		The (OpenGL) format of the data
	* @returns If succesful or not
	*/
	bool LoadFromMemory(const void* data, const uint32& width, const uint32& height, const uint32& format);

	/**
	* Attempt to load a cubemap from files
	* @param <side>file		The path to the file to load from
	* @returns If succesful or not
	*/
	bool LoadCubemapFromFiles(const string& frontFile, const string& backFile, const string& leftFile, const string& rightFile, const string& topFile, const string& bottomFile);



private:


	///
	/// Getters & Setters
	///
public:
	inline uint32 GetID() const { return m_id; }

	void SetRepeated(const bool& value);
	inline bool IsRepeated() const { return bIsRepeated; }

	void SetSmooth(const bool& value);
	inline bool IsSmooth() const { return bIsSmooth; }

	/// Requires reload
	void SetMipmapped(const bool& value) { bUsesMipmaps = value; }
	inline bool IsMipmapped() const { return bUsesMipmaps; }
};

