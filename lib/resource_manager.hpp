#pragma once

#include <map>
#include <SFML/Graphics.hpp>

template<typename T>
struct ResourceManager
{
	// Type helper
	using StrCstRef = const std::string&;

private:
	std::string prefix;
	std::map<T, sf::Font> fonts;
	std::map<T, sf::Texture> textures;
	static ResourceManager<T> instance;

public:
	ResourceManager(StrCstRef resource_path_prefix = "")
		: prefix(resource_path_prefix)
	{}

	bool registerFont(StrCstRef filename, const T& alias) {
		sf::Font font;
		if (font.loadFromFile(prefix + filename)) {
			fonts[alias] = font;
			return true;
		}
		return false;
	}

	bool registerTexture(StrCstRef filename, const T& alias) {
		sf::Texture texture;
		if (texture.loadFromFile(prefix + filename)) {
			textures[alias] = texture;
			return true;
		}
		return false;
	}

	sf::Text createText(const T& font, uint32_t size, sf::Color color = sf::Color::White, StrCstRef str = "")
	{
		sf::Text text;
		auto it = fonts.find(font);
		if (it != fonts.end()) {
			text.setFont(it->second);
			text.setCharacterSize(size);
			text.setFillColor(color);
			text.setString(str);
		}
		return text;
	}

	sf::Sprite createSprite(const T& texture, sf::Vector2f scale = sf::Vector2f(1.0f, 1.0f), sf::Vector2f origin = {})
	{
		sf::Sprite sprite;
		auto it = textures.find(texture);
		if (it != textures.end()) {
			sprite.setTexture(it->second);
			sprite.setScale(scale);
			sprite.setOrigin(origin);
		}
		return sprite;
	}
	
	static void Initialize(const std::string& prefix)
	{
		instance.prefix = prefix;
	}

	static void Close()
	{
		instance.fonts.clear();
		instance.textures.clear();
	}

	static sf::Text CreateText(const T& font, uint32_t size, sf::Color color = sf::Color::White, StrCstRef str = "")
	{
		return instance.createText(font, size, color, str);
	}

	static sf::Sprite CreateSprite(const T& texture, sf::Vector2f scale = sf::Vector2f(1.0f, 1.0f), sf::Vector2f origin = {})
	{
		return instance.createSprite(texture, scale, origin);
	}

	static bool RegisterFont(StrCstRef filename, const T& alias) {
		return instance.registerFont(filename, alias);
	}

	static bool RegisterTexture(StrCstRef filename, const T& alias) {
		return instance.registerTexture(filename, alias);
	}
};

template<typename T>
ResourceManager<T> ResourceManager<T>::instance;

using BaseManager = ResourceManager<std::string>;

