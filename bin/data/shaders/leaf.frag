// fragment shader

#version 150

in vec2 uv;
out vec4 outputColor;

uniform sampler2DRect leafTex;
uniform vec2 u_resolution;
uniform float u_time;

void main()
{
    float u = gl_FragCoord.x / u_resolution.x;
    float v = gl_FragCoord.y / u_resolution.y;
    float b = 0.5 + sin(u_time) / 2;
    float a = 1.0;

    vec4 flowVal = texture(leafTex, vec2(gl_FragCoord.x, u_resolution.y - gl_FragCoord.y));
    vec3 uvCheckColor = vec3(1 - u, 1 - v, 1 - b);
    outputColor = vec4(uvCheckColor, flowVal.x);
}
