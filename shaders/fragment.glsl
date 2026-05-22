#version 330 core
in vec2 TexCoord;
in vec3 FragPos;
in mat3 TBN;

out vec4 FragColor;

uniform sampler2D uDiffuseMap;
uniform sampler2D uNormalMap;
uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uLightColor;
uniform vec3 uMaterialAmbient;
uniform vec3 uMaterialDiffuse;
uniform vec3 uMaterialSpecular;
uniform float uMaterialShininess;

void main() {
    vec3 normalMapColor = texture(uNormalMap, TexCoord).rgb;
    vec3 normal = normalMapColor * 2.0 - 1.0;   // из [0,1] в [-1,1]
    normal = normalize(TBN * normal);           // в мировое пространство

    vec4 diffuseColor = texture(uDiffuseMap, TexCoord);
    vec3 lightDir = normalize(uLightPos - FragPos);
    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);

    // Ambient
    float ambientStrength = 0.3f;
	vec3 ambient = ambientStrength * uLightColor;
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * uMaterialDiffuse * uLightColor;
    // Specular
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterialShininess);
    vec3 specular = spec * uMaterialSpecular * uLightColor;

    vec3 result = (ambient + diffuse + specular) * diffuseColor.rgb;
    FragColor = vec4(result, 1.0);
}