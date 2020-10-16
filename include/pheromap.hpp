#pragma once
#include <SFML/Graphics.hpp>


struct PheroDirection
{
	float intensity;
	sf::Vector2f direction;

	PheroDirection()
		: intensity(0.0f)
		, direction(0.0f, 0.0f)
	{}
};


struct Pheromap
{
	sf::Vector2i size;
	uint32_t scale;
	sf::Image image;
	const float time_to_decrease_one = 1.0f;
	float current_time;
	const int32_t max_value = 254;
	const float permanent_priority = 10000.0f;

	mutable sf::Texture texture;

	Pheromap(uint32_t width, uint32_t height, uint32_t s = 1)
		: size(width/s, height/s)
		, scale(s)
		, current_time(0.0f)
	{
		image.create(size.x, size.y, sf::Color::Black);
	}

	void addToFood(sf::Vector2f position, int32_t amount)
	{
		const sf::Vector2i image_coords = getImageCoord(position);
		if (checkCoords(image_coords)) {
			const sf::Color color = image.getPixel(image_coords.x, image_coords.y);
			image.setPixel(image_coords.x, image_coords.y, sf::Color(std::min(color.r + amount, max_value), color.g, color.b));
		}
	}

	void addToHome(sf::Vector2f position, int32_t amount)
	{
		const sf::Vector2i image_coords = getImageCoord(position);
		if (checkCoords(image_coords)) {
			const sf::Color color = image.getPixel(image_coords.x, image_coords.y);
			image.setPixel(image_coords.x, image_coords.y, sf::Color(color.r, std::min(color.g + amount, max_value), color.b));
		}
	}

	bool checkCoords(sf::Vector2i coords) const
	{
		return (coords.x >= 0 && coords.x < size.x && coords.y >= 0 && coords.y < size.y);
	}

	PheroDirection getFoodDirection(sf::Vector2f position, sf::Vector2f direction, float radius)
	{
		PheroDirection result;
		const int32_t pxl_radius = std::max(1u, uint32_t(radius) / scale);
		const sf::Vector2i image_coords = getImageCoord(position);

		float pxl_count = 0.0f;
		for (int32_t x(-pxl_radius); x <= pxl_radius; ++x) {
			for (int32_t y(-pxl_radius); y <= pxl_radius; ++y) {
				const sf::Vector2i pxl_coord = image_coords + sf::Vector2i(x, y);
				if (checkCoords(pxl_coord)) {
					const sf::Vector2f pxl_position((pxl_coord.x + 0.5f) * scale, (pxl_coord.y + 0.5f) * scale);
					const sf::Vector2f to_pxl = pxl_position - position;
					if (to_pxl.x * direction.x + to_pxl.y * direction.y > 0.25f) {
						const int32_t map_value = image.getPixel(pxl_coord.x, pxl_coord.y).r;
						const float value = map_value + (map_value == 255) * permanent_priority;
						result.intensity += value;
						result.direction += value * to_pxl;
						pxl_count += 1.0f;
					}
				}
			}
		}

		return result;
	}

	PheroDirection getHomeDirection(sf::Vector2f position, sf::Vector2f direction, float radius)
	{
		PheroDirection result;
		const int32_t pxl_radius = std::max(1u, uint32_t(radius) / scale);
		const sf::Vector2i image_coords = getImageCoord(position);

		float pxl_count = 0.0f;
		for (int32_t x(-pxl_radius); x <= pxl_radius; ++x) {
			for (int32_t y(-pxl_radius); y <= pxl_radius; ++y) {
				const sf::Vector2i pxl_coord = image_coords + sf::Vector2i(x, y);
				if (checkCoords(pxl_coord)) {
					const sf::Vector2f pxl_position((pxl_coord.x + 0.5f) * scale, (pxl_coord.y + 0.5f) * scale);
					const sf::Vector2f to_pxl = pxl_position - position;
					if (to_pxl.x * direction.x + to_pxl.y * direction.y > 0.0f) {
						const int32_t map_value = image.getPixel(pxl_coord.x, pxl_coord.y).g;
						const float value = map_value + (map_value == 255) * permanent_priority;
						result.intensity += value;
						result.direction += value * to_pxl;
						pxl_count += 1.0f;
					}
				}
			}
		}

		result.direction / pxl_count;

		return result;
	}

	void update(float dt)
	{
		current_time += dt;

		if (current_time >= time_to_decrease_one) {
			current_time -= time_to_decrease_one;
			for (int32_t x(0); x < size.x; ++x) {
				for (int32_t y(0); y < size.y; ++y) {
					const sf::Color values = image.getPixel(x, y);
					image.setPixel(x, y, sf::Color(std::max(values.r - 1 * (values.r < 255), 0), std::max(values.g - 1 * (values.g < 255), 0), values.b));
				}
			}
		}
	}

	void addPermanentHome(sf::Vector2f position)
	{
		const sf::Vector2i image_coords = getImageCoord(position);
		if (checkCoords(image_coords)) {
			const sf::Color values = image.getPixel(image_coords.x, image_coords.y);
			image.setPixel(image_coords.x, image_coords.y, sf::Color(values.r, 255, values.b));
		}
	}

	void addPermanentFood(sf::Vector2f position)
	{
		const sf::Vector2i image_coords = getImageCoord(position);
		if (checkCoords(image_coords)) {
			const sf::Color values = image.getPixel(image_coords.x, image_coords.y);
			image.setPixel(image_coords.x, image_coords.y, sf::Color(255, values.g, values.b));
		}
	}

	void removePermanentFood(sf::Vector2f position)
	{
		const sf::Vector2i image_coords = getImageCoord(position);
		if (checkCoords(image_coords)) {
			const sf::Color values = image.getPixel(image_coords.x, image_coords.y);
			image.setPixel(image_coords.x, image_coords.y, sf::Color(0, values.g, values.b));
		}
	}

	sf::Vector2i getImageCoord(sf::Vector2f position) const
	{
		return sf::Vector2i(static_cast<int32_t>(position.x) / scale, static_cast<int32_t>(position.y) / scale);
	}

	sf::Sprite getSprite() const
	{
		texture.loadFromImage(image);
		sf::Sprite sprite(texture);
		const float sprite_scale = static_cast<float>(scale);
		sprite.setScale(sprite_scale, sprite_scale);
		return sprite;
	}
};
