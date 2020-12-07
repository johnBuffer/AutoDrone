#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

const std::string vert_shader =
"void main()                                                  \
{                                                             \
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;   \
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0; \
	gl_FrontColor = gl_Color;                                 \
}";

const std::string w_shader =
"uniform sampler2D texture;                                      \
uniform float WIDTH;                                             \
vec4 weight = vec4(0.006, 0.061, 0.242, 0.383);                  \
float WIDTH_STEP = 1.0 / WIDTH;                                  \
void main()                                                      \
{                                                                \
	vec2 pos = gl_TexCoord[0].xy;                                \
	vec2 offset = vec2(WIDTH_STEP, 0.0);                         \
	vec4 color = texture2D(texture, pos) * weight[3];            \
	color += texture2D(texture, pos + offset * 1.5) * weight[2]; \
	color += texture2D(texture, pos + offset * 2.5) * weight[1]; \
	color += texture2D(texture, pos + offset * 3.5) * weight[0]; \
	color += texture2D(texture, pos - offset * 1.5) * weight[2]; \
	color += texture2D(texture, pos - offset * 2.5) * weight[1]; \
	color += texture2D(texture, pos - offset * 3.5) * weight[0]; \
	gl_FragColor = vec4(color.xyz, 1.0);                         \
}";

const std::string h_shader =
"uniform sampler2D texture;                                      \
uniform float HEIGHT;                                            \
vec4 weight = vec4(0.006, 0.061, 0.242, 0.383);                  \
float HEIGHT_STEP = 1.0 / HEIGHT;                                \
void main()                                                      \
{                                                                \
	vec2 pos = gl_TexCoord[0].xy;                                \
	vec2 offset = vec2(0.0, HEIGHT_STEP);                        \
	vec4 color = texture2D(texture, pos) * weight[3];            \
	color += texture2D(texture, pos + offset * 1.5) * weight[2]; \
	color += texture2D(texture, pos + offset * 2.5) * weight[1]; \
	color += texture2D(texture, pos + offset * 3.5) * weight[0]; \
	color += texture2D(texture, pos - offset * 1.5) * weight[2]; \
	color += texture2D(texture, pos - offset * 2.5) * weight[1]; \
	color += texture2D(texture, pos - offset * 3.5) * weight[0]; \
	gl_FragColor = vec4(color.xyz, 1.0);                         \
}";

class Blur
{
public:
	Blur(uint32_t width, uint32_t height, float quality = 1.0f) :
		m_region_x(0),
		m_region_y(0),
		m_region_width(width),
		m_region_height(height),
		m_quality(quality),
		m_width(uint32_t(width * quality)),
		m_height(uint32_t(height * quality))
	{
		// Initialize textures
		m_render_textures[0].create(m_width, m_height);
		m_render_textures[1].create(m_width, m_height);
		m_render_textures[0].setSmooth(true);
		m_render_textures[1].setSmooth(true);
		m_render_textures[0].setRepeated(true);
		m_render_textures[1].setRepeated(true);

		// Compile shaders
		m_blur_w.loadFromMemory(vert_shader, w_shader);
		m_blur_h.loadFromMemory(vert_shader, h_shader);
		
		// Initialize uniforms
		m_blur_w.setUniform("WIDTH" , float(m_width));
		m_blur_h.setUniform("HEIGHT", float(m_height));
	}

	void setRegion(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		m_region_x = x;
		m_region_y = y;

		m_region_width = width;
		m_region_height = height;
	}

	const sf::Sprite apply(const sf::Texture& texture, uint8_t intensity)
	{
		// Sprite of intput texture
		sf::Sprite input_sprite(texture);
		input_sprite.setTextureRect(sf::IntRect(m_region_x, m_region_y, uint32_t(m_region_width), uint32_t(m_region_height)));
		input_sprite.scale(m_quality, m_quality);
		m_render_textures[0].clear();
		m_render_textures[0].draw(input_sprite);
		m_render_textures[0].display();

		// Scale down and blur
		float scale(1.0f);
		float scale_fact(0.5f);
		float inv_scale_fact(1.0f / scale_fact);
		for (uint8_t i(intensity); i--;) {
			process(0, scale, scale_fact);
			scale *= scale_fact;
		}

		// Scale up and blur
		for (uint8_t i(intensity); i--;) {
			process(0, scale, inv_scale_fact);
			scale *= inv_scale_fact;
		}

		// Compute the needed upscale factor
		const float inv_quality(1.0f / m_quality);
		
		// Generate the sprite
		sf::Sprite result(m_render_textures[0].getTexture());
		result.setTextureRect(sf::IntRect(0, 0, uint32_t(m_region_width * m_quality), uint32_t(m_region_height * m_quality)));
		result.setPosition(float(m_region_x), float(m_region_y));
		result.scale(inv_quality, inv_quality);

		return result;
	}

private:
	uint32_t m_region_x;
	uint32_t m_region_y;
	uint32_t m_region_width;
	uint32_t m_region_height;

	const float m_quality;
	const uint32_t m_width;
	const uint32_t m_height;
	sf::Shader m_blur_w;
	sf::Shader m_blur_h;
	sf::RenderTexture m_render_textures[2];

	void process(uint8_t first, float in_scale, float scale)
	{
		// Clear "host" texture
		m_render_textures[!first].clear(sf::Color::Black);
		// Width pass
		sf::Sprite pass_w_sprite(m_render_textures[first].getTexture());
		pass_w_sprite.setTextureRect(sf::IntRect(0, 0, uint32_t(m_region_width * in_scale * m_quality), uint32_t(m_region_height * in_scale * m_quality)));
		m_render_textures[!first].draw(pass_w_sprite, &m_blur_w);
		m_render_textures[!first].display();

		// Height pass
		sf::Sprite pass_h_sprite(m_render_textures[!first].getTexture());
		pass_h_sprite.scale(scale, scale);
		m_render_textures[first].draw(pass_h_sprite, &m_blur_h);
		m_render_textures[first].display();
	}
};
