/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include <iostream>
#include <sstream>

#include "Game.hpp"
#include "ResourceManager.hpp"
#include "SpriteRenderer.hpp"
#include "GameObject.hpp"
#include "Ball.hpp"
#include "ParticleGenerator.hpp"
#include "PostProcessor.hpp"
#include "TextRenderer.hpp"


//ISoundEngine* SoundEngine = createIrrKlangDevice();

// Game-related State data
SpriteRenderer    *Renderer;
GameObject        *Player;
BallObject        *Ball;
ParticleGenerator *Particles;
PostProcessor     *Effects;
TextRenderer    *Text;

GLfloat ShakeTime = 0.0f;

Game::Game(GLuint width, GLuint height)
		: State(GAME_MENU), Keys(), Width(width), Height(height), Lives(3)
{
}

Game::~Game()
{
	delete Renderer;
	delete Player;
	delete Ball;
	delete Particles;
	delete Effects;
}

void Game::Init()
{
	// Load shaders
	ResourceManager::LoadShader("Resource/sprite.vert", "Resource/sprite.frag", nullptr, "sprite");
	ResourceManager::LoadShader("Resource/particles.vert", "Resource/particles.frag", nullptr, "particle");
	ResourceManager::LoadShader("Resource/post_processor.vert", "Resource/post_processor.frag", nullptr, "post_processing");
	// Configure shaders
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
	ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
	ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
	ResourceManager::GetShader("particle").SetMatrix4("projection", projection);
	// Load textures
	ResourceManager::LoadTexture("Resource/background.jpg", GL_FALSE, "background");
	ResourceManager::LoadTexture("Resource/awesomeface.png", GL_TRUE, "face");
	ResourceManager::LoadTexture("Resource/block.png", GL_FALSE, "block");
	ResourceManager::LoadTexture("Resource/block_solid.png", GL_FALSE, "block_solid");
	ResourceManager::LoadTexture("Resource/paddle.png", GL_TRUE, "paddle");
	ResourceManager::LoadTexture("Resource/particle.png", GL_TRUE, "particle");
	ResourceManager::LoadTexture("Resource/powerup_speed.png", GL_TRUE, "powerup_speed");
	ResourceManager::LoadTexture("Resource/powerup_sticky.png", GL_TRUE, "powerup_sticky");
	ResourceManager::LoadTexture("Resource/powerup_increase.png", GL_TRUE, "powerup_increase");
	ResourceManager::LoadTexture("Resource/powerup_confuse.png", GL_TRUE, "powerup_confuse");
	ResourceManager::LoadTexture("Resource/powerup_chaos.png", GL_TRUE, "powerup_chaos");
	ResourceManager::LoadTexture("Resource/powerup_passthrough.png", GL_TRUE, "powerup_passthrough");
	// Set render-specific controls
	Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
	Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 500);
	Effects = new PostProcessor(ResourceManager::GetShader("post_processing"), this->Width, this->Height);
	// Load levels
	GameLevel one; one.Load("Resource/one.lvl", this->Width, this->Height * 0.5);
	GameLevel two; two.Load("Resource/two.lvl", this->Width, this->Height * 0.5);
	GameLevel three; three.Load("Resource/three.lvl", this->Width, this->Height * 0.5);
	GameLevel four; four.Load("Resource/four.lvl", this->Width, this->Height * 0.5);
	this->Levels.push_back(one);
	this->Levels.push_back(two);
	this->Levels.push_back(three);
	this->Levels.push_back(four);
	this->Level = 0;
	// Configure game objects
	glm::vec2 playerPos = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
	Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
	glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2);
	Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));

//	SoundEngine->play2D("Resource/breakout.mp3", GL_TRUE);

	Text = new TextRenderer(this->Width, this->Height);
	Text->Load("Resource/方正粗圆_GBK_0.ttf", 24);

}

void Game::Update(GLfloat dt)
{
	// Update objects
	Ball->Move(dt, this->Width);
	// Check for collisions
	this->DoCollisions(dt);
	// Update particles
	Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2));
	this->UpdatePowerUps(dt);
	if(ShakeTime > 0.0f)
	{
		ShakeTime -= dt;
		if(ShakeTime <= 0.0f)
		{
			Effects->Shake = GL_FALSE;
		}
	}
	//Note, need be at last, Check loss condition
	if (Ball->Position.y >= this->Height) // Did ball reach bottom edge?
	{
		--this->Lives;
		if(this->Lives == 0)
		{
			this->ResetLevel();
			this->ResetPlayer();
		}
		this->ResetPlayer();
	}

	if(this->State == GAME_ACTIVE && this->Levels[this->Level].IsCompleted())
	{
		this->ResetPlayer();
		this->ResetLevel();
		Effects->Chaos = GL_TRUE;
		this->State = GAME_WIN;
	}
}


void Game::ProcessInput(GLfloat dt)
{
	if (this->State == GAME_ACTIVE)
	{
		GLfloat velocity = PLAYER_VELOCITY * dt;
		// Move playerboard
		if (this->Keys[GLFW_KEY_A])
		{
			if (Player->Position.x >= 0)
			{
				Player->Position.x -= velocity;
				if (Ball->Stuck)
					Ball->Position.x -= velocity;
			}
		}
		if (this->Keys[GLFW_KEY_D])
		{
			if (Player->Position.x <= this->Width - Player->Size.x)
			{
				Player->Position.x += velocity;
				if (Ball->Stuck)
					Ball->Position.x += velocity;
			}
		}
		if (this->Keys[GLFW_KEY_SPACE])
			Ball->Stuck = false;
	}
	if(this->State == GAME_MENU)
	{
		if(this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
		{
			this->State = GAME_ACTIVE;
			this->KeysProcessed[GLFW_KEY_ENTER] = GL_TRUE;
		}
		if(this->Keys[GLFW_KEY_W] && !this->KeysProcessed[GLFW_KEY_W])
		{
			this->Level = (this->Level + 1) % 3;
			this->KeysProcessed[GLFW_KEY_W] = GL_TRUE;
		}
		if(this->Keys[GLFW_KEY_S] && !this->KeysProcessed[GLFW_KEY_S])
		{
			if(this->Level > 0)
				this->Level--;
			else
				this->Level = 2;
			this->KeysProcessed[GLFW_KEY_S] = GL_TRUE;
		}
	}

	if(this->State == GAME_WIN)
	{
		if(this->Keys[GLFW_KEY_ENTER])
		{
			this->KeysProcessed[GLFW_KEY_ENTER] = GL_TRUE;
			Effects->Chaos = GL_FALSE;
			this->State = GAME_ACTIVE;
		}
	}
}

void Game::Render()
{
	if (this->State == GAME_ACTIVE || this->State == GAME_MENU)
	{
		Effects->BeginRender();
		// Draw background
		Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f);
		// Draw level
		this->Levels[this->Level].Draw(*Renderer);
		// Draw player
		Player->Draw(*Renderer);
		// Draw particles
		Particles->Draw();
		// Draw ball
		Ball->Draw(*Renderer);

		for(PowerUp& powerUp : this->PowerUps)
		{
			if(!powerUp.Destroyed)
				powerUp.Draw(*Renderer);
		}

		Effects->EndRender();
		Effects->Render(glfwGetTime());

		std::stringstream ss;
		ss << this->Lives;
		Text->RenderText("Lives:" + ss.str(), 5.0f, 5.0f, 1.0f);
	}

	if(this->State == GAME_MENU)
	{
		Text->RenderText("Press Enter To Start", 250.0f, Height/2, 1.0f);
		Text->RenderText("Press W or S to select level", 245.0f, Height/2 + 40.0f, 0.76f);
	}

	if(this->State == GAME_WIN)
	{
		Text->RenderText("YOU WIN!!!", 250.0f, Height/2 - 20, 1.0f, glm::vec3(0.0, 1.0, 0.0));
		Text->RenderText("Press ENTER to retry or ESC to quit", 130.0f, Height/2, 1.0f, glm::vec3(1.0, 1.0, 0.0));
	}
}


void Game::ResetLevel()
{
	if (this->Level == 0)this->Levels[0].Load("Resource/one.lvl", this->Width, this->Height * 0.5f);
	else if (this->Level == 1)
		this->Levels[1].Load("Resource/two.lvl", this->Width, this->Height * 0.5f);
	else if (this->Level == 2)
		this->Levels[2].Load("Resource/three.lvl", this->Width, this->Height * 0.5f);
	else if (this->Level == 3)
		this->Levels[3].Load("Resource/four.lvl", this->Width, this->Height * 0.5f);

	this->Lives = 3;
}

void Game::ResetPlayer()
{
	// Reset player/ball stats
	Player->Size = PLAYER_SIZE;
	Player->Position = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
	Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -(BALL_RADIUS * 2)), INITIAL_BALL_VELOCITY);

	Effects->Chaos = Effects->Confuse = GL_FALSE;
	Ball->PassThrough = Ball->Sticky = GL_FALSE;
	Player->Color = glm::vec3(1.0f);
	Ball->Color = glm::vec3(1.0f);
}



// Collision detection
GLboolean CheckCollision(GameObject &one, GameObject &two);
Collision CheckCollision(BallObject &one, GameObject &two);
Direction VectorDirection(glm::vec2 closest);
void ActivatePowerUp(PowerUp& powerUp);


void Game::DoCollisions(float dt)
{
	for (GameObject &box : this->Levels[this->Level].Bricks)
	{
		if (!box.Destroyed)
		{
			Collision collision = CheckCollision(*Ball, box);
			if (std::get<0>(collision)) // If collision is true
			{
				// Destroy block if not solid
				if (!box.IsSolid)
				{
					box.Destroyed = GL_TRUE;
					this->SpawnPowerUps(box);
				}
				else
				{
					ShakeTime = 0.05f;
					Effects->Shake = true;
				}
				// Collision resolution
				Direction dir = std::get<1>(collision);
				glm::vec2 diff_vector = std::get<2>(collision);
				if(!(Ball->PassThrough))
				{
					if (dir == LEFT || dir == RIGHT) // Horizontal collision
					{
						Ball->Velocity.x = -Ball->Velocity.x; // Reverse horizontal velocity
						// Relocate
						GLfloat penetration = Ball->Radius - std::abs(diff_vector.x);
						if (dir == LEFT)
							Ball->Position.x += penetration; // Move ball to right
						else
							Ball->Position.x -= penetration; // Move ball to left;
					}
					else // Vertical collision
					{
						Ball->Velocity.y = -Ball->Velocity.y; // Reverse vertical velocity
						// Relocate
						GLfloat penetration = Ball->Radius - std::abs(diff_vector.y);
						if (dir == UP)
							Ball->Position.y -= penetration; // Move ball bback up
						else
							Ball->Position.y += penetration; // Move ball back down
					}

				}
			}
		}
	}
	// Also check collisions for player pad (unless stuck)
	Collision result = CheckCollision(*Ball, *Player);
	if (!Ball->Stuck && std::get<0>(result))
	{
		Ball->Stuck = Ball->Sticky;
		// Check where it hit the board, and change velocity based on where it hit the board
		GLfloat centerBoard = Player->Position.x + Player->Size.x / 2;
		GLfloat distance = (Ball->Position.x + Ball->Radius) - centerBoard;
		GLfloat percentage = distance / (Player->Size.x / 2);
		// Then move accordingly
		GLfloat strength = 2.0f;
		glm::vec2 oldVelocity = Ball->Velocity;
		Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
		//Ball->Velocity.y = -Ball->Velocity.y;
		Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity); // Keep speed consistent over both axes (multiply by length of old velocity, so total strength is not changed)
		// Fix sticky paddle
		Ball->Velocity.y = -1 * abs(Ball->Velocity.y);
	}

	for(PowerUp& powerUp : this->PowerUps)
	{
		if(!powerUp.Destroyed)
		{
			if(powerUp.Position.y >= this->Height)
				powerUp.Destroyed = GL_TRUE;
			if(CheckCollision(*Player, powerUp))
			{
				ActivatePowerUp(powerUp);
				powerUp.Destroyed = GL_TRUE;
				powerUp.Activated = GL_TRUE;
			}
		}
	}
}

void ActivatePowerUp(PowerUp& powerUp)
{
	if(powerUp.Type == "speed")
	{
		Ball->Velocity *= 1.2;
	}
	else if(powerUp.Type == "sticky")
	{
		Ball->Sticky = GL_TRUE;
		Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
	}
	else if(powerUp.Type == "pass-through")
	{
		Ball->PassThrough = GL_TRUE;
		Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
	}
	else if(powerUp.Type == "pad-size-increase")
	{
		Player->Size.x += 50;
	}
	else if(powerUp.Type == "confuse")
	{
		if(!Effects->Chaos)
			Effects->Confuse = GL_TRUE;
	}
	else if(powerUp.Type == "chaos")
	{
		if(!Effects->Confuse)
			Effects->Chaos = GL_TRUE;
	}
}

GLboolean ShouldSpawn(GLuint chance)
{
	GLuint random = rand() % chance;
	return random == 0;
}

void Game::SpawnPowerUps(GameObject &block)
{
	if(ShouldSpawn(75))
	{
		auto tex_speed = ResourceManager::GetTexture("powerup_speed");
		auto p = new PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, tex_speed);
		this->PowerUps.push_back(*p);
	}
	if(ShouldSpawn(75))
	{
		auto tex_sticky = ResourceManager::GetTexture("powerup_sticky");
		auto p = new PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, tex_sticky);
		this->PowerUps.push_back(*p);
	}
	if(ShouldSpawn(75))
	{
		auto tex_pass = ResourceManager::GetTexture("powerup_passthrough");
		auto p = new PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position, tex_pass);
		this->PowerUps.push_back(*p);
	}
	if(ShouldSpawn(75))
	{
		auto tex_size = ResourceManager::GetTexture("powerup_increase");
		auto p = new PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4f), 0.0f, block.Position, tex_size);
		this->PowerUps.push_back(*p);
	}
	if(ShouldSpawn(15))
	{
		auto tex_confuse = ResourceManager::GetTexture("powerup_confuse");
		auto p = new PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position, tex_confuse);
		this->PowerUps.push_back(*p);
	}
	if(ShouldSpawn(75))
	{
		auto tex_chaos = ResourceManager::GetTexture("powerup_chaose");
		auto p = new PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position, tex_chaos);
		this->PowerUps.push_back(*p);
	}
}

GLboolean isOtherPowerUpActive(std::vector<PowerUp> powerUps, std::string name)
{
	for(const PowerUp& powerUp : powerUps)
	{
		if(powerUp.Activated && powerUp.Type == name)
			return GL_TRUE;
	}
	return GL_FALSE;
}

void Game::UpdatePowerUps(GLfloat dt)
{
	for(PowerUp& powerUp : this->PowerUps)
	{
		powerUp.Position += powerUp.Velocity * dt;
		if(powerUp.Activated)
		{
			powerUp.Duration -= dt;
			if(powerUp.Duration <= 0.0f)
			{
				powerUp.Activated = GL_FALSE;

				if(powerUp.Type == "sticky")
				{
					if(!isOtherPowerUpActive(this->PowerUps, "sticky"))
					{
						Ball->Sticky = GL_FALSE;
						Player->Color = glm::vec3(1.0);
					}
				}
				else if(powerUp.Type == "pass-through")
				{
					if(!isOtherPowerUpActive(this->PowerUps, "pass-through"))
					{
						Ball->PassThrough = GL_FALSE;
						Ball->Color = glm::vec3(1.0f);
					}
				}
				else if(powerUp.Type == "confuse")
				{
					if(!isOtherPowerUpActive(this->PowerUps, "sticky"))
					{
						Effects->Confuse = GL_FALSE;
					}
				}
				else if(powerUp.Type == "chaos")
				{
					if(!isOtherPowerUpActive(this->PowerUps, "chaos"))
					{
						Effects->Chaos = GL_FALSE;
					}
				}
			}
		}
	}

	this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
			[](const PowerUp& powerUp) {
				return powerUp.Destroyed && !powerUp.Activated;
			}
	), this->PowerUps.end());
}

GLboolean CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
{
	// Collision x-axis?
	bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
	                  two.Position.x + two.Size.x >= one.Position.x;
	// Collision y-axis?
	bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
	                  two.Position.y + two.Size.y >= one.Position.y;
	// Collision only if on both axes
	return collisionX && collisionY;
}

Collision CheckCollision(BallObject &one, GameObject &two) // AABB - Circle collision
{
	// Get center point circle first
	glm::vec2 center(one.Position + one.Radius);
	// Calculate AABB info (center, half-extents)
	glm::vec2 aabb_half_extents(two.Size.x / 2, two.Size.y / 2);
	glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
	// Get difference vector between both centers
	glm::vec2 difference = center - aabb_center;
	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
	// Now that we know the the clamped values, add this to AABB_center and we get the value of box closest to circle
	glm::vec2 closest = aabb_center + clamped;
	// Now retrieve vector between center circle and closest point AABB and check if length < radius
	difference = closest - center;

	if (glm::length(difference) < one.Radius) // not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
		return std::make_tuple(GL_TRUE, VectorDirection(difference), difference);
	else
		return std::make_tuple(GL_FALSE, UP, glm::vec2(0, 0));
}

// Calculates which direction a vector is facing (N,E,S or W)
Direction VectorDirection(glm::vec2 target)
{
	glm::vec2 compass[] = {
			glm::vec2(0.0f, 1.0f),	// up
			glm::vec2(1.0f, 0.0f),	// right
			glm::vec2(0.0f, -1.0f),	// down
			glm::vec2(-1.0f, 0.0f)	// left
	};
	GLfloat max = 0.0f;
	GLuint best_match = -1;
	for (GLuint i = 0; i < 4; i++)
	{
		GLfloat dot_product = glm::dot(glm::normalize(target), compass[i]);
		if (dot_product > max)
		{
			max = dot_product;
			best_match = i;
		}
	}
	return (Direction)best_match;
}
