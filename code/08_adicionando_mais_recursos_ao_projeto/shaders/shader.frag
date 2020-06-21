#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragViewVec;
layout(location = 4) in vec3 fragLightVec;

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;


const vec3 ambientLightColor = vec3(0.1);
const vec3 diffuseLightColor = vec3(1.0);
const vec3 specularLightColor = vec3(1.0);
const float shininess = 16.0;

void main() {
    
    vec3 n = normalize(fragNormal);
    vec3 l = normalize(fragLightVec);
    vec3 v = normalize(fragViewVec);
    vec3 r = reflect(l, n);
    
    vec3 ambient = ambientLightColor;
    vec3 diffuse = diffuseLightColor * max(dot(n, l), 0.0);
    vec3 specular = specularLightColor * pow(max(dot(r, v), 0.0), shininess);
    
    outColor = texture(texSampler, fragTexCoord) *
    vec4(ambient + diffuse + specular, 1.0);
}
