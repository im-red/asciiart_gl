#version 450 core

out vec4 fragColor;
in vec2 texCoord;

uniform float charWidth;
uniform float charHeight;

uniform sampler2D imageTexture;
uniform sampler2D asciiTexture;

void main()
{
    float gridWidth = charWidth;
    float gridHeight = charHeight;
    int xGrid = int(texCoord.x / gridWidth);
    int yGrid = int(texCoord.y / gridHeight);
    float xDelta = texCoord.x - xGrid * gridWidth;
    float yDelta = texCoord.y - yGrid * gridHeight;
    vec4 texColor = texture(imageTexture, vec2(xGrid * gridWidth, yGrid * gridHeight));
    float gray = texColor.x * 0.299 + texColor.y * 0.587 + texColor.z * 0.114;
    int index = int(gray / (1.0 / 255.0));
    fragColor = texture(asciiTexture, vec2(1.0 / 256.0 * (index + xDelta / gridWidth), yDelta / gridHeight));
};
