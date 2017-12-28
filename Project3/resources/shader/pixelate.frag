// shader récupéré sur l'exemple SFML (légèrement modifié)
// Donne un aspect 8-bit rétro au scramble.

uniform sampler2D texture;

void main()
{
    float factor = 300;
    vec2 pos = floor(gl_TexCoord[0].xy * factor + 0.5) / factor;
    gl_FragColor = texture2D(texture, pos) * gl_Color;
}
