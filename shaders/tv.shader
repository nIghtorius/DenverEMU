#version 130

// shader written by unknown
// shader ported to Denver by nIghtorius

#define CRT_STRENGTH 0.8
#define CA_MAX_PIXEL_DIST 2.0
#define BORDER_SIZE 0.2
#define BORDER_STRENGTH 0.8
#define SATURATION 1.1

// data that comes from denver
uniform vec3        screen;         // size of glOrtho viewport.
uniform sampler2D   nesvideo;       // image from Denver (gamescreen)
uniform vec2        offset;         // shifted from viewport.

vec4 read (vec2 uv)
{
    return texture (nesvideo, uv);
}
vec3 saturation (vec3 rgb, float amount)
{
    const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    vec3 intensity = vec3(dot(rgb, W));
    return mix(intensity, rgb, amount);
}
void main ()
{
    vec2[3] ca;
    ca[0] = vec2(-CA_MAX_PIXEL_DIST, -CA_MAX_PIXEL_DIST);
    ca[1] = vec2(0.0);
    ca[2] = vec2(CA_MAX_PIXEL_DIST, CA_MAX_PIXEL_DIST);
    
    vec2 pixel = 1.0 / screen.xy;    
    vec2 uv = (gl_FragCoord.xy - offset) * pixel; 
    uv.y = 1.0 - uv.y;
    
    int row = int(gl_FragCoord.y)%2;
    int col = (int(gl_FragCoord.x)+row)%3;
    
    vec2 nuv = 2. * abs(uv-vec2(0.5));
    vec2 caShift = (length(nuv) / sqrt(2.0)) * pixel;
    
    vec3 src = vec3(0.0);
    for(int i = 0; i < 3; i++)
    {
        src[i] = read(uv - ca[i] * caShift)[i] * ((i==col) ? 1.0 : 1.-CRT_STRENGTH);
    }
    float d = (1.-nuv.x) * (1.-nuv.y);
    d = smoothstep(0.0,0.01+0.1,sqrt(d)); 
    src *= d;
    src = saturation(src, SATURATION);
    gl_FragColor = vec4(src,1.0);
}
