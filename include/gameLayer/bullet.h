#pragma once
#include <gl2d/gl2d.h>

struct Bullet
{
	glm::vec2 position = {};

	glm::vec2 fireDirection = {};

	bool isEnemy = 0;
	float speed = 3000;

	void render(gl2d::Renderer2D& renderer,
		gl2d::Texture bulletsTexture,
		gl2d::TextureAtlasPadding bulletAtlas);

	void Bullet::update(float deltaTime);
};