#version 130

// shader written by aleklesovoi
// shader ported to Denver by nIghtorius

#define CA_STRENGTH 4.0
#define CORNER_OFFSET 0.61
#define CORNER_MASK_INTENSITY_MULT 8.0
#define BORDER_OFFSET 0.04

// data that comes from denver
uniform vec3        screen;         // size of glOrtho viewport.
uniform sampler2D   nesvideo;       // image from Denver (gamescreen)
uniform vec2        offset;         // shifted from viewport.

vec3 ChromaticAberration(vec2 uv)
{
    vec3 color = texture(nesvideo, uv).rgb;
	color.r = texture(nesvideo, (uv - 0.5) * (1.0 + CA_STRENGTH / screen.xy) + 0.5).r;
	color.b = texture(nesvideo, (uv - 0.5) * (1.0 - CA_STRENGTH / screen.xy) + 0.5).b;
    return color;
}

vec2 BrownConradyDistortion(in vec2 uv)
{
    float k1 = 0.05;
    float k2 = 0.0025;

    uv = uv * 2.0 - 1.0; 
    
    float r2 = uv.x * uv.x + uv.y * uv.y;
    uv *= 1.0 + k1 * r2 + k2 * r2 * r2;
    
    uv = uv * 0.5 + 0.5; 

    float scale = abs(k1) < 1.0 ? 1.0 - abs(k1) : 1.0 / (k1 + 1.0);		
    
    uv = uv * scale - (scale * 0.5) + 0.5; 
    
    return uv;
}

float scanl(vec2 uv)
{
    return (abs(sin( screen.y * uv.y)) + abs(sin( screen.x * uv.x))) / 2.0;
}

float vig(vec2 uv)
{
    uv = uv - 0.5; 
    
    float res = length(uv)- CORNER_OFFSET;
    res *= CORNER_MASK_INTENSITY_MULT;
    res = clamp(res, 0.0, 1.0);
    res = 1.0 - res;
    
    uv = abs(uv); 
    uv = uv - (0.5 - BORDER_OFFSET);
    uv = smoothstep(1.0, 0.0, uv / BORDER_OFFSET);
    
	return min(uv.x, uv.y) * res;
}

void main()
{
    vec2 pixel = 1.0 / screen.xy;
	vec2 uv = (gl_FragCoord.xy - offset) * pixel;
    uv.y = 1.0 - uv.y;
    uv = BrownConradyDistortion(uv);
    vec3 result = ChromaticAberration(uv);
    result *= scanl(uv);
    result *= vig(uv);
    // final output
    gl_FragColor = vec4( result, 1.0 );    
}
