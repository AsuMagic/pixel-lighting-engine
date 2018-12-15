#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>

void rgb_to_indexed(sf::Image& image, const sf::Image& palette_image)
{
	auto find_matching = [&](sf::Color color) -> int {
		for (unsigned i = 0; i < palette_image.getSize().x; ++i)
		{
			if (color == palette_image.getPixel(i, 0))
			{
				return i;
			}
		}

		return -1;
	};

	for (unsigned y = 0; y < image.getSize().y; ++y)
	{
		for (unsigned x = 0; x < image.getSize().x; ++x)
		{
			auto match = find_matching(image.getPixel(x, y));

			if (match == -1)
			{
				std::cout << "MISSING PALETTE COLOR: (" << x << "; " << y
						  << ")!\n";

				continue;
			}

			image.setPixel(x, y, {std::uint8_t(match), 0, 0});
		}
	}
}

void load_texture_indexed(
	sf::Texture&      texture,
	const sf::String& path,
	const sf::Image&  palette_image)
{
	std::cout << "Loading texture from file " << path.toAnsiString() << '\n';

	sf::Image image;
	image.loadFromFile(path);
	rgb_to_indexed(image, palette_image);

	texture.loadFromImage(image);
}

int main()
{
	sf::RenderWindow win{sf::VideoMode::getFullscreenModes().front(),
						 "Lighting shader test",
						 sf::Style::Fullscreen};

	win.setVerticalSyncEnabled(true);
	win.setMouseCursorVisible(false);
	win.setMouseCursorGrabbed(false);

	float light_scale = 0.5f;

	// Light texture
	sf::Texture light_texture;
	light_texture.loadFromFile("light.png");
	sf::Sprite cursor_light_sprite{light_texture};
	cursor_light_sprite.setOrigin(sf::Vector2f(light_texture.getSize()) / 2.0f);

	// Textures used for full-screen shader effects
	sf::RenderTexture light_composite_texture;
	light_composite_texture.create(win.getSize().x, win.getSize().y);

	sf::RenderTexture composite_texture;
	composite_texture.create(win.getSize().x, win.getSize().y);

	sf::View view = win.getView();
	view.zoom(0.5);
	view.setCenter({500, 300});
	light_composite_texture.setView(view);
	composite_texture.setView(view);

	std::vector<sf::Sprite> light_sprites;

	// Generalistic textures
	sf::Texture palette_texture;
	sf::Texture palette_light_texture;
	sf::Texture tile_texture;
	sf::Texture turret_texture;

	{
		sf::Image palette_image;
		palette_image.loadFromFile("palette.png");

		palette_texture.loadFromImage(palette_image);

		load_texture_indexed(
			palette_light_texture, "palette_light_shift.png", palette_image);
		load_texture_indexed(tile_texture, "tile.png", palette_image);
		load_texture_indexed(turret_texture, "turret.png", palette_image);
	}

	sf::Shader indexed_to_rgb_shader;
	indexed_to_rgb_shader.loadFromFile(
		"indexed_to_rgb.vert", "indexed_to_rgb.frag");
	indexed_to_rgb_shader.setUniform("palette_texture", palette_texture);
	indexed_to_rgb_shader.setUniform("texture", composite_texture.getTexture());

	sf::Shader light_shift_shader;
	light_shift_shader.loadFromFile("light_shift.vert", "light_shift.frag");
	light_shift_shader.setUniform(
		"lightmap_texture", light_composite_texture.getTexture());
	light_shift_shader.setUniform("texture", composite_texture.getTexture());
	light_shift_shader.setUniform(
		"palette_shift_texture", palette_light_texture);
	light_shift_shader.setUniform("resolution", sf::Vector2f(win.getSize()));

	while (win.isOpen())
	{
		sf::Event ev;
		while (win.pollEvent(ev))
		{
			switch (ev.type)
			{
			case sf::Event::Closed:
			{
				win.close();
				break;
			}

			case sf::Event::MouseButtonPressed:
			{
				switch (ev.mouseButton.button)
				{
				case sf::Mouse::Left:
				{
					light_sprites.push_back(cursor_light_sprite);
					break;
				}

				default: break;
				}

				break;
			}

			case sf::Event::MouseWheelScrolled:
			{
				light_scale += ev.mouseWheelScroll.delta * 0.025;
				break;
			}

			case sf::Event::KeyPressed:
			{
				switch (ev.key.code)
				{
				case sf::Keyboard::Add:
				{
					auto current_color = cursor_light_sprite.getColor();
					current_color.a += 8;
					cursor_light_sprite.setColor(current_color);
					break;
				}

				case sf::Keyboard::Subtract:
				{
					auto current_color = cursor_light_sprite.getColor();
					current_color.a -= 8;
					cursor_light_sprite.setColor(current_color);
					break;
				}

				default: break;
				}
			}

			default: break;
			}
		}

		cursor_light_sprite.setPosition(sf::Vector2f(sf::Mouse::getPosition()));
		cursor_light_sprite.setScale(light_scale, light_scale);

		win.clear({34, 32, 52});
		composite_texture.clear({31, 0, 0, 255});

		// Draw tiles
		for (unsigned y : {256, 256 + 128})
		{
			for (unsigned x = 0; x < 800; x += 16)
			{
				sf::Sprite sprite;
				sprite.setTexture(tile_texture);

				if (y == 256)
				{
					sprite.setScale(1.0f, -1.0f);
				}

				sprite.setPosition(float(x), float(y));
				composite_texture.draw(sprite);
			}
		}

		sf::Sprite sprite;
		sprite.setTexture(turret_texture);
		sprite.setPosition(256, 256 + 128 - turret_texture.getSize().y);
		composite_texture.draw(sprite);

		composite_texture.display();

		// Draw lights to composite texture
		{
			light_composite_texture.clear();

			sf::RenderStates states;
			states.blendMode = sf::BlendAdd;

			light_composite_texture.draw(cursor_light_sprite, states);

			for (const auto& sprite : light_sprites)
			{
				light_composite_texture.draw(sprite, states);
			}

			light_composite_texture.display();
		}

		// Apply composite texture to palette shifting shader
		{
			sf::RenderStates states;
			states.shader = &light_shift_shader;

			sf::Sprite light_sprite;
			light_sprite.setTexture(light_composite_texture.getTexture());

			composite_texture.draw(light_sprite, states);
		}

		// Convert indexed to RGB in shader
		{
			sf::RenderStates states;
			states.shader = &indexed_to_rgb_shader;

			sf::Sprite composite_sprite;
			composite_sprite.setTexture(composite_texture.getTexture());

			win.draw(composite_sprite, states);
		}

		// Draw composite texture to window
		/*{
			sf::RenderStates states;
			states.blendMode = sf::BlendMultiply;

			sf::Sprite light_sprite;
			light_sprite.setTexture(composite_texture.getTexture());

			win.draw(light_sprite, states);
		}*/

		win.display();
	}
}
