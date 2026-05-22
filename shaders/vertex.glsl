#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;

out vec2 TexCoord;
out vec3 FragPos;
out mat3 TBN;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    vec3 T = normalize(vec3(uModel * vec4(aTangent, 0.0)));
    vec3 N = normalize(vec3(uModel * vec4(aNormal, 0.0)));
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);
    TexCoord = aTexCoord;
    gl_Position = uProjection * uView * worldPos;
}