#version 400 core

// 2D vertex position (used for HUD or screen-space geometry)
layout(location = 0) in vec2 inPosition;

uniform mat4 projMatrix;

// Model-view matrix (used for positioning/scaling HUD elements)
uniform mat4 modelViewMatrix;

void main()
{
    // Transform 2D position into clip space
    // z = 0.0 since this is screen-space geometry
    gl_Position = projMatrix * modelViewMatrix * vec4(inPosition, 0.0, 1.0);
}