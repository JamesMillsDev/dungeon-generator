#pragma once

#include <string>
#include <vector>

class Texture;

using std::string;
using std::vector;

class Material
{
public:
	static Material* Make();
		
public:
	Color color;
	Color emissiveTint;
	float roughness;
	float metallic;

	Texture* baseColorMap;
	Texture* normalMap;
	Texture* ormMap;
	Texture* emissiveMap;

private:
	explicit Material();

public:
	void Render() const;

};