#version 130

#define pixelation 2 

// shader written by nIghtorius
// have to idea what to make.

// data that comes from denver
uniform vec3        screen;         // size of glOrtho viewport.
uniform sampler2D   nesvideo;       // image from Denver (gamescreen)
uniform vec2        offset;         // shifted from viewport.

precision highp float;

float character(float n, vec2 p) // some compilers have the word "char" reserved
{
    p = floor(p * vec2(8.0,-8.0) + (vec2(-4.0,4.0) + vec2(1.0)) );

	if (clamp(p.x, 0.0, 4.0) == p.x && clamp(p.y, 0.0, 4.0) == p.y)
	{
    	float x = (5.0 * p.y + p.x);
        float signbit = (n < 0.0)
          ? 1.0 
          : 0.0 ;
        
        signbit = (x == 0.0) 
          ? signbit
          : 0.0 ;
        
        return ( fract( abs( n*exp2(-x-1.0))) >= 0.5) ? 1.0 : signbit; //works on AMD and intel
	}	
    return 0.0;
}

void main ()
{
	vec4 fragCoord = (gl_FragCoord - vec4 (offset.x, offset.y, 0, 0)) / float(pixelation);
	vec2 uv = fragCoord.xy;
	uv.y = 1.0 - uv.y;
	vec2 cursor_position = (floor(uv/8.0)*8.0+(0.5/float(pixelation)))/(screen.xy / float(pixelation)); //slight blur
	vec3 col = texture(nesvideo, cursor_position).rgb; 
	float luma = dot(col,vec3(0.2126, 0.7152, 0.0722));
	float gray = smoothstep(0.0,1.0,luma); //increase contrast
	float n = float[]( 0.,4194304.,131200.,324.,330.,283712.,12650880.,4532768.,
                       13191552.,10648704.,11195936.,15218734.,15255086.,15252014.,15324974.,11512810.
                     )[int(gray * 16.)]; 
	vec2 p = fract(uv * 0.125);
	col = pow(col,vec3(0.55));
	col = col*character(n, p);
	gl_FragColor = vec4(col,1.0);
}

