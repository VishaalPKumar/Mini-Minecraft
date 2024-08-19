#version 150
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

uniform sampler2D u_Texture; // An object that lets us access the texture

uniform int u_Time;
uniform vec3 u_PlayerPos;
uniform float u_RenderDistanceBlocks;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Pos;
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec2 fs_UV;
in float fs_Animate;

out vec4 out_Col; // This is the final output color that you will see on your
// screen for the pixel that is currently being processed.

float BLK_SIZE = 0.0625;
int NUM_INCR = 16;

void main()
{
    vec2 uv = fs_UV;

    vec4 specular_color = vec4(0);

    if (fs_Animate >= 0.1) {
        float timeMod = mod(u_Time / 1000, NUM_INCR);
        uv -= vec2(0, timeMod * BLK_SIZE / NUM_INCR);

        // Only include specular on water and lava
        specular_color = vec4(1);
    }

    vec4 diffuseColor = texture(u_Texture, uv);

    if (diffuseColor.a == 0.f) {
        discard;
    }


    // Add black lines between blocks (REMOVE WHEN YOU APPLY TEXTURES)
    // bool xBound = fract(fs_Pos.x) < 0.0125 || fract(fs_Pos.x) > 0.9875;
    // bool yBound = fract(fs_Pos.y) < 0.0125 || fract(fs_Pos.y) > 0.9875;
    // bool zBound = fract(fs_Pos.z) < 0.0125 || fract(fs_Pos.z) > 0.9875;
    // if((xBound && yBound) || (xBound && zBound) || (yBound && zBound)) {
    //     diffuseColor.rgb = vec3(0,0,0);
    // }

    // Calculate the diffuse term for Lambert shading
    float diffuseTerm = dot(normalize(fs_Nor), normalize(fs_LightVec));
    // Avoid negative lighting values
    diffuseTerm = clamp(diffuseTerm, 0, 1);

    float ambientTerm = 0.2;
    float lightIntensity = diffuseTerm + ambientTerm;   //Add a small float value to the color multiplier
    //to simulate ambient lighting. This ensures that faces that are not
    //lit by our point light are not completely black.


    vec4 lambert_Col = vec4(diffuseColor.rgb * lightIntensity, diffuseColor.a);

    // Blinn-phong shading
    vec4 eye_vec = normalize(vec4(u_PlayerPos, 2) - fs_Pos);
    vec4 half_vec = (eye_vec + fs_LightVec) / 2;
    float shininess = 3;
    vec4 blinn_phong_Col = max(pow(dot(half_vec, fs_Nor), shininess), 0) * specular_color;

    vec4 final_Col = diffuseColor * 0.5 + lambert_Col * 0.5 + blinn_phong_Col * 1;

    // Interpolate fog color
    vec4 fogColor = vec4(0.37f, 0.74f, 1.0f, 1);
    float dist = length(vec4(u_PlayerPos, 1) - fs_Pos);
    dist = min(dist, u_RenderDistanceBlocks);
    float distPercent = dist / u_RenderDistanceBlocks;
    float interpPercent = smoothstep(0.9, 1, distPercent);

    // Compute final shaded color
    out_Col = mix(final_Col, fogColor, interpPercent);
}
