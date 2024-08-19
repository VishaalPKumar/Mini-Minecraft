#version 150

in vec2 fs_UV;

out vec4 out_Col;

uniform sampler2D u_Texture;
uniform int u_WaterLava;
uniform int u_Time;


void main()
{

    vec2 result_UV = fs_UV;
    if (u_WaterLava != 0) {
        result_UV += vec2(sin(fs_UV.x * 5 + u_Time / 1000.f), cos(fs_UV.y * 5 + u_Time / 1000.f)) / 50; // distortion
    }
    vec4 diffuseColor = texture(u_Texture, result_UV);
    if (u_WaterLava == 1) {
        out_Col = vec4(diffuseColor.x * 0.65, diffuseColor.y * 0.65, clamp(diffuseColor.z * 1.4, 0.f, 1.f), 1.f);
    } else if (u_WaterLava == 2) {
        out_Col = vec4(clamp(diffuseColor.x * 1.4, 0.f, 1.f), diffuseColor.y * 0.65, diffuseColor.z * 0.65, 1.f);
    } else {
        out_Col = diffuseColor;
    }
    // out_Col = texture(u_Texture, fs_UV);
}
