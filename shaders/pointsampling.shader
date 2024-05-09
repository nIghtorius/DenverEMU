#version 130

// point sampling for pixel art.

// data that comes from denver
uniform vec3        screen;         // size of glOrtho viewport.
uniform sampler2D   nesvideo;       // image from Denver (gamescreen)
uniform vec2        offset;         // shifted from viewport.


void main()
{
   vec4 fragCoord = gl_FragCoord - vec4 (offset.x, offset.y, 0, 0);
   vec2 pix = fragCoord.xy;
   float scale = screen.x / 256.0;
   pix /= scale;
   pix = floor(pix) + min(fract(pix) / fwidth(pix), 1.0) - 0.5;
   pix.y = 1.0 - pix.y;
   gl_FragColor = vec4 (texture(nesvideo, pix / vec2(256.0, 240.0)).rgb, 1.0);    
}
