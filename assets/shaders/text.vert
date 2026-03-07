#version 330 core
layout(location = 0) in vec4 a_pos_texcoord;
uniform mat4 u_proj;
out vec2 v_texcoord;
void main() {
    gl_Position = u_proj * vec4(a_pos_texcoord.xy, 0.0, 1.0);
    v_texcoord  = a_pos_texcoord.zw;
}
