#pragma once

#include <list>
#include "marker.hpp"
#include "food.hpp"
#include "utils.hpp"
#include "world.hpp"
#include "config.hpp"
#include <iostream>


constexpr float max_reserve = 1000.0f;


struct Ant
{
	Ant() = default;

	Ant(float x, float y, float angle, uint32_t id_)
		: colony(x, y)
		, position(x, y)
		, direction(angle)
		, last_direction_update(getRandUnder(direction_update_period))
		, last_marker(getRandUnder(marker_period))
		, phase(Marker::Type::ToFood)
		, reserve(max_reserve)
		, id(id_)
	{}

	void update(const float dt, World& world)
	{
		updatePosition(dt);

		checkColony();
		if (phase == Marker::ToFood) {
			checkFood(world);
		}

		last_direction_update += dt;
		if (last_direction_update > direction_update_period) {
			findMarker(world);
			float range = PI * 0.08f;
			direction += getRandRange(range);
			last_direction_update = 0.0f;
		}

		last_marker += dt;
		if (last_marker >= marker_period) {
			addMarker(world);
		}
	}

	void updatePosition(const float dt)
	{
		const float speed = 50.0f;
		position += (dt * speed) * sf::Vector2f(cos(direction), sin(direction));

		position.x = position.x < 0.0f ? Conf<>::WIN_WIDTH : position.x;
		position.y = position.y < 0.0f ? Conf<>::WIN_HEIGHT : position.y;

		position.x = position.x > Conf<>::WIN_WIDTH ? 0.0f : position.x;
		position.y = position.y > Conf<>::WIN_HEIGHT ? 0.0f : position.y;
	}

	void checkFood(World& world)
	{
		const std::list<Food*> food_spots = world.grid_food.getAllAt(position);
		for (Food* fp : food_spots) {
			if (getLength(position - fp->position) < fp->radius) {
				phase = Marker::ToHome;
				direction += PI;
				reserve = max_reserve;
				fp->pick(world.grid_markers_food);
				return;
			}
		}
	}

	void checkColony()
	{
		const float colony_size = Conf<>::COLONY_SIZE;
		if (getLength(position - colony) < colony_size) {
			if (phase == Marker::ToHome) {
				phase = Marker::ToFood;
				direction += PI;
			}
			reserve = max_reserve;
		}
	}

	void findNewDirection(World& world)
	{
		findMarker(world);
	}

	void findMarker(World& world)
	{
		const int32_t max_dist = 10u;

		float total_intensity = 0.0f;
		sf::Vector2f point(0.0f, 0.0f);

		const sf::Vector2f dir_vec = sf::Vector2f(cos(direction), sin(direction));
		const float dir_x = sign(dir_vec.x);
		const float dir_y = sign(dir_vec.y);

		const FlatGrid* const grid = (phase == Marker::ToFood) ? &world.grid_markers_food : &world.grid_markers_home;

		const sf::Vector2i grid_pos = grid->getCellCoords(position);
		for (int32_t x(-max_dist); x < max_dist+1; ++x) {
			for (int32_t y(-max_dist); y < max_dist+1; ++y) {
				if (x && y) {
					const sf::Vector2i offset(dir_x * x, dir_y * y);
					const sf::Vector2f cell_world_pos = getWorldCoords(grid_pos + offset, *grid);
					const sf::Vector2f to_marker = cell_world_pos - position;
					if (dot(to_marker, dir_vec) > 0.0f && getLength(to_marker) < max_dist * grid->cell_size) {
						const Cell c = grid->getAt(position, offset);
						if (c.permanent) {
							setDirection(cell_world_pos);
							return;
						}
						const float intensity = c.value;
						total_intensity += intensity;
						point += intensity * cell_world_pos;
					}
				}
			}
		}

		if (total_intensity) {
			setDirection(point / total_intensity);
		}
	}

	void setDirection(const sf::Vector2f& target)
	{
		direction = getAngle(target - position);
	}

	const sf::Vector2f getWorldCoords(const sf::Vector2i& cell_coords, const FlatGrid& grid) const
	{
		return float(grid.cell_size) * sf::Vector2f(cell_coords.x + 0.5f, cell_coords.y + 0.5f);
	}

	void addMarker(World& world)
	{
		if (reserve > 1.0f) {
			world.addMarker(Marker(position, phase == Marker::ToFood ? Marker::ToHome : Marker::ToFood, reserve * 0.02f));
			reserve *= 0.98f;
		}

		last_marker = 0.0f;
	}

	void render(sf::RenderTarget& target, const sf::RenderStates& states) const
	{
		const float width = 2.0f;
		const float length = 7.0f;
		sf::RectangleShape body(sf::Vector2f(width, length));
		body.setOrigin(width * 0.5f, length * 0.5f);
		body.setPosition(position);
		body.setRotation(direction * 57.2958f + 90.0f);
		body.setFillColor(Conf<>::ANT_COLOR);

		if (phase == Marker::ToHome) {
			const float radius = 2.0f;
			sf::CircleShape circle(radius);
			circle.setOrigin(radius, radius);
			circle.setPosition(position + length * 0.5f * sf::Vector2f(cos(direction), sin(direction)));
			circle.setFillColor(Conf<>::FOOD_COLOR);
			target.draw(circle, states);
		}

		target.draw(body, states);
	}

	void render_in(sf::VertexArray& va, const uint64_t index) const
	{
		const float width = 2.0f;
		const float length = 10.0f;

		const sf::Vector2f dir_vec(cos(direction), sin(direction));

		va[index + 0].position = position + width * sf::Vector2f(-dir_vec.y, dir_vec.x);
		va[index + 1].position = position - width * sf::Vector2f(-dir_vec.y, dir_vec.x);
		va[index + 2].position = position + length * dir_vec;
		
		va[index + 0].color = Conf<>::ANT_COLOR;
		va[index + 1].color = Conf<>::ANT_COLOR;

		if (phase == Marker::ToHome) {			
			va[index + 2].color = Conf<>::FOOD_COLOR;
		}
		else {
			va[index + 2].color = Conf<>::ANT_COLOR;
		}
	}

	sf::Vector2f colony;
	sf::Vector2f position;
	float direction;
	float last_direction_update;
	float last_marker;
	Marker::Type phase;
	float reserve = 500.0f;
	const uint32_t id;

	const float direction_update_period = 0.125f;
	const float marker_period = 0.25f;
};
