#pragma once
#include <list>
#include <vector>
#include <SFML/System.hpp>
#include <swarm.hpp>

#include "marker.hpp"
#include "food.hpp"
#include "grid.hpp"
#include "flat_grid.hpp"


constexpr uint32_t CELL_SIZE = 4u;


struct World
{
	World(uint32_t width, uint32_t height)
		: grid_markers_home(width, height, CELL_SIZE)
		, grid_markers_food(width, height, CELL_SIZE)
		, grid_food(width, height, 5)
		, size(width, height)
		, va_to_food(sf::Quads)
		, va_to_home(sf::Quads)
		, swarm(Conf<>::THREAD_COUNT)
	{
		va_to_food.resize(4 * grid_markers_home.cells.size());
		va_to_home.resize(4 * grid_markers_home.cells.size());
	}

	void removeExpiredFood()
	{
		for (std::list<Food>& l : grid_food.cells) {
			l.remove_if([&](const Food& m) {return m.isDone(); });
		}
	}

	void update(const float dt)
	{
		removeExpiredFood();

		markers_count = 0u;
		for (Cell& c : grid_markers_home.cells) {
			if (!c.permanent) {
				c.value = c.value < dt ? 0.0f : c.value - dt;
			}
		}

		for (Cell& c : grid_markers_food.cells) {
			if (!c.permanent) {
				c.value = c.value < dt ? 0.0f : c.value - dt;
			}
		}
	}

	void addMarker(const Marker& marker)
	{
		FlatGrid* const grid = (marker.type == Marker::ToFood) ? &grid_markers_food : &grid_markers_home;
		grid->add(marker.position, marker.intensity, marker.permanent);
	}

	void render(sf::RenderTarget& target, const sf::RenderStates& states) const
	{
		generateMarkersVertexArray(va_to_food, grid_markers_food, Conf<>::TO_FOOD_COLOR);
		generateMarkersVertexArray(va_to_home, grid_markers_home, Conf<>::TO_HOME_COLOR);
		target.draw(va_to_food, states);
		target.draw(va_to_home, states);

		for (const std::list<Food>& l : grid_food.cells) {
			for (const Food& f : l) {
				f.render(target, states);
			}
		}
	}

	void generateMarkersVertexArray(sf::VertexArray& va, const FlatGrid& grid, const sf::Color& base_color) const
	{
		uint64_t index = 0u;
		const float cell_size = grid.cell_size;
		for (uint32_t x(0); x < grid.width; ++x) {
			for (uint32_t y(0); y < grid.height; ++y) {
				va[4 * index + 0].position = sf::Vector2f(x * cell_size, y * cell_size);
				va[4 * index + 1].position = sf::Vector2f((x + 1) * cell_size, y * cell_size);
				va[4 * index + 2].position = sf::Vector2f((x + 1) * cell_size, (y + 1) * cell_size);
				va[4 * index + 3].position = sf::Vector2f(x * cell_size, (y + 1) * cell_size);

				const float ratio = std::min(1.0f, grid.getAt(cell_size * sf::Vector2f(x, y)).value / Conf<>::MAX_CELL_INTENSITY);
				const sf::Color color(base_color.r, base_color.g, base_color.b, 255 * ratio);

				va[4 * index + 0].color = color;
				va[4 * index + 1].color = color;
				va[4 * index + 2].color = color;
				va[4 * index + 3].color = color;
				++index;
			}
		}
	}

	void addFoodAt(float x, float y, float quantity)
	{
		addMarker(Marker(sf::Vector2f(x, y), Marker::ToFood, 1000.0f, true));
		grid_food.add(Food(x, y, 4.0f, quantity));
	}

	sf::Vector2f size;
	mutable sf::VertexArray va_to_food;
	mutable sf::VertexArray va_to_home;
	FlatGrid grid_markers_home;
	FlatGrid grid_markers_food;
	Grid<Food> grid_food;
	swrm::Swarm swarm;

	uint64_t markers_count;
};
