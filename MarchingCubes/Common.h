#pragma once
#include <glm.hpp>
#include <string>

#define Super __super


typedef __int8		int8;
typedef __int16		int16;
typedef __int32		int32;
typedef __int64		int64;

typedef unsigned __int8		uint8;
typedef unsigned __int16	uint16;
typedef unsigned __int32	uint32;
typedef unsigned __int64	uint64;


typedef std::string string;

typedef glm::vec2	vec2;
typedef glm::vec3	vec3;
typedef glm::vec4	vec4;

typedef glm::ivec2	ivec2;
typedef glm::ivec3	ivec3;
typedef glm::ivec4	ivec4;

typedef glm::uvec2	uvec2;
typedef glm::uvec3	uvec3;
typedef glm::uvec4	uvec4;

typedef glm::mat2	mat2;
typedef glm::mat3	mat3;
typedef glm::mat4	mat4;


/**
* The appropriate hashing functions which are needed to use vec3 as a key
* https://stackoverflow.com/questions/9047612/glmivec2-as-key-in-unordered-map
*/
struct vec3_KeyFuncs
{
	inline size_t operator()(const vec3& v)const
	{
		return std::hash<float>()(v.x) ^ std::hash<float>()(v.y) ^ std::hash<float>()(v.z);
	}

	inline bool operator()(const vec3& a, const vec3& b)const
	{
		return a.x == b.x && a.y == b.y && a.z == b.z;
	}
};

struct uvec3_KeyFuncs
{
	inline size_t operator()(const uvec3& v)const
	{
		return std::hash<uint32>()(v.x) ^ std::hash<uint32>()(v.y) ^ std::hash<uint32>()(v.z);
	}

	inline bool operator()(const uvec3& a, const uvec3& b)const
	{
		return a.x == b.x && a.y == b.y && a.z == b.z;
	}
};

struct ivec3_KeyFuncs
{
	inline size_t operator()(const ivec3& v)const
	{
		return std::hash<int32>()(v.x) ^ std::hash<int32>()(v.y) ^ std::hash<int32>()(v.z);
	}

	inline bool operator()(const ivec3& a, const ivec3& b)const
	{
		return a.x == b.x && a.y == b.y && a.z == b.z;
	}
};