#define GLM_ENABLE_EXPERIMENTAL
#include "gameLayer.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "platformInput.h"
#include "imgui.h"
#include <iostream>
#include <sstream>
#include "imfilebrowser.h"
#include <gl2d/gl2d.h>
#include <platformTools.h>
#include <tiledRenderer.h>
#include <bullet.h>
#include <enemy.h>
#include <glui\glui.h>
#include <raudio.h>

struct GameData {
	glm::vec2 playerPosition{ 100, 100 };
	std::vector<Bullet> bullets;
	std::vector<Enemy> enemies;

	float health = 1.f; // player's life 0 -> 1
	float spawnEnemyTimerSecconds = 0.f; // time until next enemy spawn
};

GameData gameData;
const int BACKGROUNDS = 4;

gl2d::Renderer2D renderer;

gl2d::Texture healthBarTexture;
gl2d::Texture healthTexture;

gl2d::Texture spaceShipsTexture;
gl2d::TextureAtlasPadding spaceShipsAtlas;

gl2d::Texture bulletTexture;
gl2d::TextureAtlasPadding bulletAtlas;

gl2d::Texture backgroundTexture[BACKGROUNDS];

TiledRenderer tiledRenderer[BACKGROUNDS];

Sound shootSound;

constexpr float shipSize = 250.f;

bool intersectBullet(glm::vec2 bulletPosition, glm::vec2 shipPosition, float shipSize)
{
	return glm::distance(bulletPosition, shipPosition) <= shipSize;
}

void spawnEnemies() {
	glm::uvec2 shipTypes[] = { {0,0}, {0,1}, {2,0}, {3,1} };

	Enemy e;
	e.position = gameData.playerPosition;

	glm::vec2 offset(2000, 0);
	offset = glm::vec2(glm::vec4(offset, 0, 1) * 
			glm::rotate(
				glm::mat4(1.f),
				glm::radians((float)(rand() % 360)),
				glm::vec3(0, 0, 1)
			)
	);

	e.position += offset;

	e.speed = 700 + rand() % 1000;
	e.turnSpeed = 2.f + (rand() % 1000) / 500.f;
	e.type = shipTypes[rand() % 4];
	e.fireRange = 1.5 + (rand() % 1000) / 2000.f;
	e.fireTimeReset = 0.1 + (rand() % 1000) / 500;
	e.bulletSpeed = rand() % 3000 + 1000;

	gameData.enemies.push_back(e);
}

void restartGame()
{
	gameData = {};
	renderer.currentCamera
		.follow(gameData.playerPosition, 550, 0, 150, renderer.windowW, renderer.windowH);
}

bool initGame()
{
	//initializing stuff for the renderer
	gl2d::init();
	renderer.create();

	spaceShipsTexture.loadFromFileWithPixelPadding(RESOURCES_PATH "spaceShip/stitchedFiles/spaceships.png", 128, true);
	spaceShipsAtlas = gl2d::TextureAtlasPadding(5, 2, spaceShipsTexture.GetSize().x, spaceShipsTexture.GetSize().y);

	bulletTexture.loadFromFileWithPixelPadding(RESOURCES_PATH "spaceShip/stitchedFiles/projectiles.png", 500, true);
	bulletAtlas = gl2d::TextureAtlasPadding(3, 2, bulletTexture.GetSize().x, bulletTexture.GetSize().y);

	healthBarTexture.loadFromFile(RESOURCES_PATH "healthBar.png", true);
	healthTexture.loadFromFile(RESOURCES_PATH "health.png", true);

	shootSound = LoadSound(RESOURCES_PATH "shoot.flac");
	SetSoundVolume(shootSound, 0.5f);

	//spaceShipsTexture.loadFromFile(RESOURCES_PATH "spaceShip/ships/green.png", true);
	backgroundTexture[0].loadFromFile(RESOURCES_PATH "background1.png", true);
	backgroundTexture[1].loadFromFile(RESOURCES_PATH "background2.png", true);
	backgroundTexture[2].loadFromFile(RESOURCES_PATH "background3.png", true);
	backgroundTexture[3].loadFromFile(RESOURCES_PATH "background4.png", true);

	tiledRenderer[0].texture = backgroundTexture[0];
	tiledRenderer[1].texture = backgroundTexture[1];
	tiledRenderer[2].texture = backgroundTexture[2];
	tiledRenderer[3].texture = backgroundTexture[3];

	tiledRenderer[0].paralaxStrength = 0;
	tiledRenderer[1].paralaxStrength = 0.4;
	tiledRenderer[2].paralaxStrength = 0.6;
	tiledRenderer[3].paralaxStrength = 0.7;

	restartGame();

	return true;
}



bool gameLogic(float deltaTime)
{
#pragma region init stuff
	int w = 0; int h = 0;
	w = platform::getFrameBufferSizeX(); //window w
	h = platform::getFrameBufferSizeY(); //window h

	glViewport(0, 0, w, h);
	glClear(GL_COLOR_BUFFER_BIT); //clear screen

	renderer.updateWindowMetrics(w, h);
#pragma endregion

#pragma region movement
	glm::vec2 move{ };

	if (platform::isButtonHeld(platform::Button::W) ||
		platform::isButtonHeld(platform::Button::Up))
	{
		move.y = -1;
	}

	if (platform::isButtonHeld(platform::Button::S) ||
		platform::isButtonHeld(platform::Button::Down))
	{
		move.y = 1;
	}

	if (platform::isButtonHeld(platform::Button::A) ||
		platform::isButtonHeld(platform::Button::Left))
	{
		move.x = -1;
	}

	if (platform::isButtonHeld(platform::Button::D) ||
		platform::isButtonHeld(platform::Button::Right))
	{
		move.x = 1;
	}

	if (move.x != 0 || move.y != 0)
	{
		move = glm::normalize(move);

		move *= deltaTime * 1000; // 1000 pixels per second
		gameData.playerPosition += move;
	}


#pragma endregion

#pragma region follow
	renderer.currentCamera.follow(gameData.playerPosition, deltaTime * 450, 10, 50, w, h);
#pragma endregion

#pragma region render background
	renderer.currentCamera.zoom = 0.5;
	for (int i = 0; i < BACKGROUNDS; i++)
	{
		tiledRenderer[i].render(renderer);
	}
#pragma endregion

#pragma region mouse pos

	glm::vec2 mousePos = platform::getRelMousePosition();
	glm::vec2 screenCenter(w / 2.f, h / 2.f);

	glm::vec2 mouseDirection = mousePos - screenCenter;

	if (glm::length(mouseDirection) == 0)
	{
		mouseDirection = { 0, 1 };
	}
	else
	{
		mouseDirection = glm::normalize(mouseDirection);
	}

	float spaceShipAngle = atan2(mouseDirection.y, -mouseDirection.x);
#pragma endregion

#pragma region handle bullets

	if (platform::isLMousePressed())
	{
		Bullet b;

		b.position = gameData.playerPosition;
		b.fireDirection = mouseDirection;

		gameData.bullets.push_back(b);

		PlaySound(shootSound);
	}

	for (int i = 0; i < gameData.bullets.size(); i++)
	{
		Bullet* bullet = &gameData.bullets[i];

		if (glm::distance(bullet->position, gameData.playerPosition) > 5000)
		{
			gameData.bullets.erase(gameData.bullets.begin() + i);
			i--;
			continue;
		}

		if (!bullet->isEnemy)
		{
			bool shouldContinueBulletCycle = false;
			for (int j = 0; j < gameData.enemies.size(); j++)
			{
				Enemy* enemy = &gameData.enemies[j];
				if (intersectBullet(bullet->position, enemy->position, enemyShipSize))
				{
					enemy->health -= 0.1f;
					shouldContinueBulletCycle = true;

					if (enemy->health <= 0)
						gameData.enemies.erase(gameData.enemies.begin() + j);

					break;
				}
			}

			if (shouldContinueBulletCycle)
			{
				gameData.bullets.erase(gameData.bullets.begin() + i);
				i--;
				continue;
			}
		}
		else
		{
			if (intersectBullet(bullet->position, gameData.playerPosition, shipSize))
			{
				gameData.health -= 0.1f;
				gameData.bullets.erase(gameData.bullets.begin() + i);
				i--;
				continue;
			}
		}

		bullet->update(deltaTime);
	}

	if (gameData.health <= 0)
	{
		restartGame();
	}
	else
	{
		gameData.health += deltaTime * 0.01f;
		gameData.health = glm::clamp(gameData.health, 0.f, 1.f);

	}
#pragma endregion


#pragma region handle enemies

	if (gameData.enemies.size() < 15)
	{
		gameData.spawnEnemyTimerSecconds -= deltaTime;

		if (gameData.spawnEnemyTimerSecconds < 0)
		{
			gameData.spawnEnemyTimerSecconds = rand() % 5 + 1;

			spawnEnemies();

			if (rand() % 3 == 0)
				spawnEnemies();

		}
	}

	for (int i = 0; i < gameData.enemies.size(); i++)
	{
		Enemy* enemy = &gameData.enemies[i];

		if (glm::distance(enemy->position, gameData.playerPosition) > 4000.f)
		{
			gameData.enemies.erase(gameData.enemies.begin() + i);
			i--;
			continue;
		}

		if (enemy->update(deltaTime, gameData.playerPosition))
		{
			Bullet b;
			b.position = enemy->position;
			b.fireDirection = enemy->viewDirection;
			b.isEnemy = 1;
			b.speed = enemy->bulletSpeed;

			gameData.bullets.push_back(b);

			if (!IsSoundPlaying(shootSound))
				PlaySound(shootSound);
		}

	}

#pragma endregion

#pragma region ui
	renderer.pushCamera();
	{
		glui::Frame f({ 0,0, w,h });
		glui::Box healthBox = glui::Box().xLeftPerc(0.65)
			.yTopPerc(0.1)
			.xDimensionPercentage(0.3)
			.yAspectRatio(1.f / 8.f);

		renderer.renderRectangle(healthBox, healthBarTexture);

		glm::vec4 newRect = healthBox();
		newRect.z *= gameData.health;

		glm::vec4 textCoords = { 0,0,1,1 };
		textCoords.z *= gameData.health;

		renderer.renderRectangle(newRect, healthTexture, Colors_White, {}, {}, textCoords);
	}
	renderer.popCamera();

#pragma endregion

#pragma region render enemies

	for (auto& e : gameData.enemies)
	{
		e.render(renderer, spaceShipsTexture, spaceShipsAtlas);
	}

#pragma endregion

#pragma region render ship

	renderSpaceShip(renderer, gameData.playerPosition,
		shipSize,
		spaceShipsTexture, spaceShipsAtlas.get(1, 0),
		mouseDirection);
#pragma endregion

#pragma region render bullets

	for (auto& b : gameData.bullets)
	{
		b.render(renderer, bulletTexture, bulletAtlas);
	}

#pragma endregion

	renderer.flush();

	//ImGui::ShowDemoWindow();

	ImGui::Begin("Debug");

	ImGui::Text("Bulles Count: %d", (int)gameData.bullets.size());
	ImGui::Text("Enemies Count: %d", (int)gameData.enemies.size());

	if (ImGui::Button("Spawn Enemy"))
	{
		spawnEnemies();
	}

	if (ImGui::Button("Reset game"))
	{
		restartGame();
	}

	ImGui::SliderFloat("Player Health", &gameData.health, 0, 1);

	ImGui::End();

	return true;
#pragma endregion

}

//This function might not be be called if the program is forced closed
void closeGame()
{



}
