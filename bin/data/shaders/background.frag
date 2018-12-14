// fragment shader

#version 120

//in vec2 uv;
//out vec4 fragColor;

uniform vec2 u_resolution;
uniform float u_time;
uniform float u_deltaTime;
uniform sampler2DRect flowTex;
uniform float u_fogAmount;


float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define OCTAVES 6
float fbm (in vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitude = .5;
    float frequency = 0.;
    //
    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++) {
        value += amplitude * noise(st);
        st *= 2.;
        amplitude *= .5;
    }
    return value;
}

void main()
{
    float r = 0.08; // gl_FragCoord.x / u_resolution.x;
    float g = 0.08; // 0.5 + sin(u_time) / 2;
    float b = 0.08; // 0.7 * gl_FragCoord.y / u_resolution.y + 0.3 + (0.5 + sin(u_time) / 2);
    float a = 1.0;

    vec2 st = gl_FragCoord.xy/u_resolution.xy;
    st.x *= u_resolution.x/u_resolution.y;

    //vec4 flowVal = texture(flowTex, gl_FragCoord.xy);
    //outputColor = flowVal;
    vec3 uvCheckColor = vec3(r, g, b);
    float fogColor = u_fogAmount * fbm(st*3.0 + u_time * 0.1);
    gl_FragColor= vec4(uvCheckColor + fogColor, 1.0);
}
