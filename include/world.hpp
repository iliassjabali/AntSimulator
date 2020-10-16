#pragma once
#include <list>
#include <vector>
#include <SFML/System.hpp>

#include "marker.hpp"
#include "food.hpp"
#include "utils.hpp"
#include "pheromap.hpp"
#include "wall.hpp"


template<typename T>
struct Grid
{
	Grid(int32_t width_, int32_t height_, uint32_t cell_size_)
		: cell_size(cell_size_)
		, width(width_ / cell_size_)
		, height(height_ / cell_size_)
	{
		cells.resize(width * height);
	}

	T* add(const T& obj)
	{
		return add(getCellCoords(obj.position), obj);
	}

	bool isEmpty(const sf::Vector2f& position) const
	{
		const sf::Vector2i cell_coords = getCellCoords(position);

		if (checkCell(cell_coords)) {
			return cells[getIndexFromCoords(cell_coords)].empty();
		}

		return true;
	}

	std::list<T>* getAt(const sf::Vector2f& position)
	{
		const sf::Vector2i cell_coords = getCellCoords(position);

		if (checkCell(cell_coords)) {
			return &cells[getIndexFromCoords(cell_coords)];
		}

		return nullptr;
	}

	std::list<T*> getAllAt(const sf::Vector2f& position)
	{
		std::list<T*> result(0);
		const sf::Vector2i cell_coords = getCellCoords(position);
		
		for (int32_t x(-1); x < 2; ++x) {
			for (int32_t y(-1); y < 2; ++y) {
				const sf::Vector2i coords = cell_coords + sf::Vector2i(x, y);
				if (checkCell(coords)) {
					const uint64_t index = getIndexFromCoords(coords);
					for (T& m : cells[index]) {
						result.push_back(&m);
					}
				}
			}
		}

		return result;
	}

	T* add(const sf::Vector2i& cell_coords, const T& obj)
	{
		if (checkCell(cell_coords)) {
			std::list<T>& l = cells[getIndexFromCoords(cell_coords)];
			if (Conf<>::MAX_MARKERS_PER_CELL > l.size()) {
				l.emplace_back(obj);
				return &l.back();
			}
		}
		return nullptr;
	}

	bool checkCell(const sf::Vector2i& cell_coords) const
	{
		return cell_coords.x > -1 && cell_coords.x < width && cell_coords.y > -1 && cell_coords.y < height;
	}

	uint64_t getIndexFromCoords(const sf::Vector2i& cell_coords) const
	{
		return cell_coords.x + cell_coords.y * width;
	}

	sf::Vector2i getCellCoords(const sf::Vector2f& position) const
	{
		const int32_t x_cell = to<int32_t>(position.x / cell_size);
		const int32_t y_cell = to<int32_t>(position.y / cell_size);

		return sf::Vector2i(x_cell, y_cell);
	}

	std::vector<std::list<T>> cells;

	const int32_t width, height, cell_size;
};


struct World
{
	sf::Vector2f size;
	mutable sf::VertexArray va;
	Pheromap markers_map;
	Grid<Wall> grid_walls;
	Grid<Food> grid_food;

	World(uint32_t width, uint32_t height)
		: markers_map(width, height, 2)
		, grid_walls(width, height, 20)
		, grid_food(width, height, 5)
		, size(to<float>(width), to<float>(height))
		, va(sf::Quads)
	{}

	void removeExpiredFood()
	{
		for (std::list<Food>& l : grid_food.cells) {
			l.remove_if([&](const Food& m) {
				if (m.isDone()) {
					markers_map.removePermanentFood(m.position);
					return true;
				}
				return false;
			});
		}
	}

	void update(float dt)
	{
		removeExpiredFood();
		markers_map.update(dt);
	}

	void addWall(const sf::Vector2f& position)
	{
		grid_walls.add(Wall{ position });
	}

	void render(sf::RenderTarget& target, const sf::RenderStates& states, bool draw_markers = true) const
	{
		if (draw_markers) {
			target.draw(markers_map.getSprite(), states);
		}

		uint64_t i = 0;
		sf::VertexArray va(sf::Quads, 4 * grid_walls.cells.size());
		const float cell_size = grid_walls.cell_size;
		for (int32_t x(0); x < grid_walls.width; x++) {
			for (int32_t y(0); y < grid_walls.height; y++) {
				const uint32_t index = y * grid_walls.width + x;
				sf::Color color(sf::Color(0, 0, 0, 0));
				if (!grid_walls.cells[index].empty()) {
					color = sf::Color(94, 87, 87);
				}
				sf::Vector2f position(x * cell_size, y * cell_size);

				va[4 * i].position = position;
				va[4 * i + 1].position = position + sf::Vector2f(cell_size, 0.0f);
				va[4 * i + 2].position = position + sf::Vector2f(cell_size, cell_size);
				va[4 * i + 3].position = position + sf::Vector2f(0.0f, cell_size);

				va[4 * i].color = color;
				va[4 * i + 1].color = color;
				va[4 * i + 2].color = color;
				va[4 * i + 3].color = color;
				++i;
			}
		}

		target.draw(va, states);

		for (const std::list<Food>& l : grid_food.cells) {
			for (const Food& f : l) {
				f.render(target, states);
			}
		}
	}

	void addFoodAt(float x, float y, float quantity)
	{
		grid_food.add(Food(x, y, 4.0f, quantity));
		markers_map.addPermanentFood(sf::Vector2f(x, y));
	}
};
