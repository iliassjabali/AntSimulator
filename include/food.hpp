#pragma once
#include <SFML/Graphics.hpp>
#include "config.hpp"
#include "flat_grid.hpp"


struct Food
{
	Food() = default;

	Food(float x, float y, float r, float quantity_)
		: position(x, y)
		, radius(r)
		, quantity(quantity_)
	{}

	void pick(FlatGrid& grid)
	{
		quantity -= 1.0f;
		if (isDone()) {
			Cell& cell = grid.getCellAt(position);
			cell.value = 50.0f;
			cell.decreasePermanent();
		}
	}

	bool isDone() const
	{
		return quantity <= 0.0f;
	}

	void render(sf::RenderTarget& target, const sf::RenderStates& states) const
	{
		sf::CircleShape circle(radius);
		circle.setOrigin(radius, radius);
		circle.setPosition(position);
		circle.setFillColor(Conf<>::FOOD_COLOR);

		target.draw(circle, states);
	}

	sf::Vector2f position;
	float radius;
	float quantity;
};
