#include <tiledRenderer.h>



void TiledRenderer::render(gl2d::Renderer2D& renderer)
{
	auto viewRect = renderer.getViewRect();

	glm::vec2 paralaxDistance = { viewRect.x, viewRect.y };
	paralaxDistance *= -paralaxStrength;

	//for (int y = -1; y <= 1; y++)
	for (int y = -2; y <= 2; y++)
	{
		for (int x = -2; x <= 2; x++)
		{
			int posX = x + int((viewRect.x - paralaxDistance.x) / backgroundSize);
			int posY = y + int((viewRect.y - paralaxDistance.y) / backgroundSize);

			renderer.renderRectangle(
				glm::vec4{ posX, posY, 1, 1 } *backgroundSize + glm::vec4(paralaxDistance, 0, 0),
				texture
			);

		}
	}
}

void renderSpaceShip(gl2d::Renderer2D& renderer,
	glm::vec2 positoin, float size,
	gl2d::Texture texture, glm::vec4 uvs, glm::vec2 viewDirection ) {

	float spaceShipAngle = atan2(viewDirection.y, -viewDirection.x);
	spaceShipAngle = glm::degrees(spaceShipAngle) + 90.f;

	renderer.renderRectangle(
		{ positoin - glm::vec2(size / 2,size / 2), size, size },
		texture,
		Colors_White,
		{},
		spaceShipAngle,
		uvs);
}