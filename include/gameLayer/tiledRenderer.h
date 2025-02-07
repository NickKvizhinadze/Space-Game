#pragma once
#include <gl2d/gl2d.h>

struct TiledRenderer
{
	float backgroundSize = 3000;
	float paralaxStrength = 1;
	gl2d::Texture texture;

	void render(gl2d::Renderer2D &renderer);
};

void renderSpaceShip(
	gl2d::Renderer2D &renderer,
	glm::vec2 position, float size,
	gl2d::Texture texture,
	glm::vec4 uvs, glm::vec2 viewDirection);