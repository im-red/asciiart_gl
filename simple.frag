#version 450 core

out vec4 fragColor;
in vec2 texCoord;

uniform sampler2D imageTexture;

void main()
{
    fragColor = texture(imageTexture, texCoord);
};
