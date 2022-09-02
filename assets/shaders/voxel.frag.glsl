#version 330 core

in vec3 v_normal;
in vec2 v_texCoord;
in float v_occlusion;

uniform mat4 u_viewProjection;
uniform sampler2D u_atlas;
uniform vec4 u_color;

out vec4 color;

void main()
{
    vec3 lightPos = vec3(3, 10, 3);
    vec3 lightDirection = normalize(lightPos);

    float lightIntensity = 0.2;

    float dirLight = lightIntensity * max(dot(lightDirection, v_normal), 0);
    float maxAmbient = 0.5;
    float minAmbient = 0.2;

    float light = min((dirLight + (maxAmbient - minAmbient)) * v_occlusion + minAmbient, 1);

    color = u_color * light * texture(u_atlas, v_texCoord);
    color.a = 1;
}