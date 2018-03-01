#include "Texture.h"
#include "Logger.h"

#include <GL\glew.h>
#include <FreeImage.h>


Texture::Texture()
{
	glGenTextures(1, &m_id);
}

Texture::~Texture()
{
	if (m_id != 0)
		glDeleteTextures(1, &m_id);
}

bool Texture::LoadFromFile(const string& file)
{
	FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
	FIBITMAP* data = nullptr;
	bIsLoaded = false;


	// Attempt to work out format from signature
	format = FreeImage_GetFileType(file.c_str(), 0);

	// Attempt to get format from extension
	if (format == FIF_UNKNOWN)
		format = FreeImage_GetFIFFromFilename(file.c_str());

	// Unable to work out file format
	if (format == FIF_UNKNOWN)
	{
		LOG_ERROR("Unable to deduce format for file '%s'", file.c_str());
		return false;
	}

	if (!FreeImage_FIFSupportsReading(format))
	{
		LOG_ERROR("Unable to read file '%s' (Unsupported file format)", file.c_str());
		return false;
	}

	data = FreeImage_Load(format, file.c_str());
	if (data == nullptr)
	{
		LOG_ERROR("Unable to read file '%s' (Is the file readable?)", file.c_str());
		return false;
	}


	// Read data
	m_width = FreeImage_GetWidth(data);
	m_height = FreeImage_GetHeight(data);
	uint32 bpp = FreeImage_GetBPP(data); // Bits per Pixel

	if (m_width == 0 || m_height == 0 || bpp == 0)
	{
		LOG_ERROR("Unable to read file '%s' (Read invalid attribs)", file.c_str());
		return false;
	}

	// Convert to 32 bit
	if (bpp != 32)
	{
		FIBITMAP* oldData = data;
		data = FreeImage_ConvertTo32Bits(data);
		FreeImage_Unload(oldData);
	}


	// Load into OpenGL
	BYTE* rawBytes = FreeImage_GetBits(data);

	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_DECAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_DECAL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bUsesMipmaps ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bIsSmooth ? GL_LINEAR : GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, bIsRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, bIsRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, rawBytes);
	if (bUsesMipmaps)
		glGenerateMipmap(GL_TEXTURE_2D);

	FreeImage_Unload(data);
	bIsLoaded = true;
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

bool Texture::LoadFromMemory(const void* data, const uint32& width, const uint32& height, const uint32& format)
{
	bIsLoaded = false;
	m_width = width;
	m_height = height;

	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_DECAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_DECAL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bUsesMipmaps ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bIsSmooth ? GL_LINEAR : GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, bIsRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, bIsRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);
	if (bUsesMipmaps)
		glGenerateMipmap(GL_TEXTURE_2D);

	bIsLoaded = true;
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

bool Texture::LoadCubemapFromFiles(const string& frontFile, const string& backFile, const string& leftFile, const string& rightFile, const string& topFile, const string& bottomFile)
{
	const uint32 glSide[]
	{
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y
	};
	const string fileSide[] =
	{
		frontFile,
		backFile,
		leftFile,
		rightFile,
		topFile,
		bottomFile
	};
	bIsLoaded = false;

	// Attempt to load in each side and add it to the texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

	for (uint32 i = 0; i < 6; ++i)
	{
		FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
		FIBITMAP* data = nullptr;


		// Attempt to work out format from signature
		format = FreeImage_GetFileType(fileSide[i].c_str(), 0);

		// Attempt to get format from extension
		if (format == FIF_UNKNOWN)
			format = FreeImage_GetFIFFromFilename(fileSide[i].c_str());

		// Unable to work out file format
		if (format == FIF_UNKNOWN)
		{
			LOG_ERROR("Unable to deduce format for file '%s'", fileSide[i].c_str());
			return false;
		}

		if (!FreeImage_FIFSupportsReading(format))
		{
			LOG_ERROR("Unable to read file '%s' (Unsupported file format)", fileSide[i].c_str());
			return false;
		}

		data = FreeImage_Load(format, fileSide[i].c_str());
		if (data == nullptr)
		{
			LOG_ERROR("Unable to read file '%s' (Is the file readable?)", fileSide[i].c_str());
			return false;
		}


		// Read data
		m_width = FreeImage_GetWidth(data);
		m_height = FreeImage_GetHeight(data);
		uint32 bpp = FreeImage_GetBPP(data); // Bits per Pixel

		if (m_width == 0 || m_height == 0 || bpp == 0)
		{
			LOG_ERROR("Unable to read file '%s' (Read invalid attribs)", fileSide[i].c_str());
			return false;
		}

		// Convert to 32 bit
		if (bpp != 32)
		{
			FIBITMAP* oldData = data;
			data = FreeImage_ConvertTo32Bits(data);
			FreeImage_Unload(oldData);
		}


		// Load into OpenGL
		BYTE* rawBytes = FreeImage_GetBits(data);


		glTexImage2D(glSide[i], 0, GL_RGBA, m_width, m_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, rawBytes);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, bIsSmooth ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, bIsRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, bIsRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);

		FreeImage_Unload(data);
	}


	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	bIsLoaded = true;
	return true;
}

void Texture::SetRepeated(const bool& value)
{
	bIsRepeated = value;
	if (!bIsLoaded)
		return;

	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, bIsRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, bIsRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetSmooth(const bool& value)
{
	bIsSmooth = value;
	if (!bIsLoaded)
		return;

	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bIsSmooth ? GL_LINEAR : GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}