#include "button.h"

Button::Button(double x, double y, double width, double height, bool textured) : Component(x, y, width, height) {
	this->textured = textured;
	this->borderColor = GUI::borderColor;
	if (textured)
		this->borderWidth = 0;
	else
		this->borderWidth = GUI::borderWidth;
}

Button::Button(std::string text, double x, double y, bool textured) : Component(x, y) {
	this->fontColor = GUI::fontColor;
	this->fontSize = GUI::fontSize;
	this->textured = textured;
	this->text = text;
	this->borderColor = GUI::borderColor;
	if (textured)
		this->borderWidth = 0;
	else
		this->borderWidth = GUI::borderWidth;
}

void Button::renderPressed() {
	if (textured) {
		GUI::shader->update(pressTexture);
	} else {
		GUI::shader->update(pressColor);
	}

	GUI::quad->render();
}

void Button::renderHovering() {
	if (textured) {
		GUI::shader->update(hoverTexture);
	} else {
		GUI::shader->update(hoverColor);
	}

	GUI::quad->render();
}

void Button::renderIdle() {
	if (textured) {
		GUI::shader->update(idleTexture);
	} else {
		GUI::shader->update(idleColor);
	}

	GUI::quad->render();
}

void Button::render() {
	resize();

	GUI::quad->resize(position, dimension);

	if (borderWidth > 0) {
		GUI::shader->update(borderColor);
		GUI::quad->render();
		GUI::quad->resize(position + Vec2(borderWidth, -borderWidth), dimension - Vec2(borderWidth) * 2);
	}

	if (pressed)
		renderPressed();
	else if (hovering)
		renderHovering();
	else 
		renderIdle();

	if (!text.empty())
		GUI::font->render(text, position + Vec2(borderWidth, -borderWidth), fontColor, fontSize);
}

Vec2 Button::resize() {
	if (resizing) {
		dimension = GUI::font->size(text, fontSize) + Vec2(borderWidth) * 2;
	} 

	return dimension;
}

void Button::enter() {
	hovering = true;
}

void Button::exit() {
	hovering = false;
	pressed = false;
}

void Button::press(Vec2 point) {
	pressed = true;
}

void Button::release(Vec2 point) {
	pressed = false;
	(*action)(this);
}
