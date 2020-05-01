#pragma once
#include <SFML/Graphics.hpp>
#include "config.hpp"


struct Cell
{
	Cell()
		: value(0.0f)
		, permanent(0)
	{}

	Cell(float val, bool perm = false)
		: value(val)
		, permanent(perm)
	{}

	void decreasePermanent()
	{
		if (permanent) {
			--permanent;
		}
	}

	float value;
	uint32_t permanent;
};


struct FlatGrid
{
	FlatGrid(uint32_t width_, uint32_t height_, uint32_t cell_size_)
		: cell_size(cell_size_)
		, width(width_ / cell_size_)
		, height(height_ / cell_size_)
	{
		cells.resize(width * height);
		for (Cell& c : cells) {
			c.value = 0.0f;
			c.permanent = false;
		}
	}

	void add(const sf::Vector2f& position, float value, bool permanent = false)
	{
		Cell& cell = cells[getIndexFromCoords(getCellCoords(position))];
		cell.value += value;
		cell.permanent += permanent ? 1 : 0;

		if (!cell.permanent) {
			cell.value = std::min(Conf<>::MAX_CELL_INTENSITY, cell.value);
		}
	}

	Cell getAt(const sf::Vector2f& position, const sf::Vector2i offset = sf::Vector2i(0, 0)) const
	{
		return getCellAt(position, offset);
	}

	const Cell& getCellAt(const sf::Vector2f& position, const sf::Vector2i offset = sf::Vector2i(0, 0)) const
	{
		const sf::Vector2i cell_coords = fitCoords(getCellCoords(position) + offset);
		return cells[getIndexFromCoords(cell_coords)];
	}

	Cell& getCellAt(const sf::Vector2f& position, const sf::Vector2i offset = sf::Vector2i(0, 0))
	{
		const sf::Vector2i cell_coords = fitCoords(getCellCoords(position) + offset);
		return cells[getIndexFromCoords(cell_coords)];
	}

	bool checkCell(const sf::Vector2i& cell_coords)
	{
		return cell_coords.x > -1 && cell_coords.x < width && cell_coords.y > -1 && cell_coords.y < height;
	}

	uint64_t getIndexFromCoords(const sf::Vector2i& cell_coords) const
	{
		return cell_coords.x + cell_coords.y * width;
	}

	sf::Vector2i fitCoords(const sf::Vector2i& coords) const
	{
		sf::Vector2i result = coords;
		if (result.x < 0) {
			result.x += width;
		}
		else if (result.x >= width) {
			result.x -= width;
		}

		if (result.y < 0) {
			result.y += height;
		}
		else if (result.y >= height) {
			result.y -= height;
		}

		return result;
	}

	sf::Vector2i getCellCoords(const sf::Vector2f& position) const
	{
		const int32_t x_cell = position.x / cell_size;
		const int32_t y_cell = position.y / cell_size;

		return fitCoords(sf::Vector2i(x_cell, y_cell));
	}

	std::vector<Cell> cells;

	const uint32_t width, height, cell_size;
};