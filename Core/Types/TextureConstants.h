#pragma once

#define STATIC_NAME(name) inline static const string name = #name

struct STextureNames
{
	STATIC_NAME(Default);
	STATIC_NAME(Debug);
	STATIC_NAME(White);
	STATIC_NAME(Water);
};

struct SMaterialNames
{
	STATIC_NAME(Picked);
	STATIC_NAME(Gold);
	STATIC_NAME(Silver);
	STATIC_NAME(Bronze);
	STATIC_NAME(Lambertian);
	STATIC_NAME(Metallic);
	STATIC_NAME(Mirror);
	STATIC_NAME(Glass);
	STATIC_NAME(Water);
	STATIC_NAME(Debug);
	STATIC_NAME(Default);
};