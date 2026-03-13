#version 330 core
out vec4 FragColor;
in vec3 ourColor;
in vec2 TexCoord;
uniform sampler2D MAIN_TEXTURE;
uniform vec4 COLOR;

void main()
{
   vec4 Image = texture2D(MAIN_TEXTURE, TexCoord * 10);
   if (Image.a == 0.0f) discard;
   FragColor = Image * COLOR;// * vec4(ourColor, 1.0);
}