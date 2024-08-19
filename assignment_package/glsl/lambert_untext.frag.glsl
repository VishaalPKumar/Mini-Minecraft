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

// uniform sampler2D u_Texture; // An object that lets us access the texture

// uniform int u_Time;
uniform vec3 u_PlayerPos;
uniform float u_RenderDistanceBlocks;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Pos;
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec4 fs_Col;

out vec4 out_Col; // This is the final output color that you will see on your
// screen for the pixel that is currently being processed.

// float BLK_SIZE = 0.0625;
// int NUM_INCR = 16;

void main()
{

    vec4 diffuseColor = fs_Col;//texture(u_Texture, uv);

    if (diffuseColor.a == 0.f) {
        discard;
    }

    // Calculate the diffuse term for Lambert shading
    float diffuseTerm = dot(normalize(fs_Nor), normalize(fs_LightVec));
    // Avoid negative lighting values
    diffuseTerm = clamp(diffuseTerm, 0, 1);

    float ambientTerm = 0.2;
    float lightIntensity = diffuseTerm + ambientTerm;   //Add a small float value to the color multiplier
    //to simulate ambient lighting. This ensures that faces that are not
    //lit by our point light are not completely black.


    vec4 lambert_Col = vec4(diffuseColor.rgb * lightIntensity, diffuseColor.a);

    // Interpolate fog color
    // vec4 fogColor = vec4(0.37f, 0.74f, 1.0f, 1);
    // float dist = length(vec4(u_PlayerPos, 1) - fs_Pos);
    // dist = min(dist, u_RenderDistanceBlocks);
    // float distPercent = dist / u_RenderDistanceBlocks;
    // float interpPercent = smoothstep(0.9, 1, distPercent);

    // Compute final shaded color
    out_Col = lambert_Col;//mix(lambert_Col, fogColor, interpPercent);
}
