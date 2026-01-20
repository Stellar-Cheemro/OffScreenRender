#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

void main()
{
    // Funky colors based on texture coordinates just to see something
    FragColor = vec4(TexCoords.x, TexCoords.y, 0.5, 1.0);
}
