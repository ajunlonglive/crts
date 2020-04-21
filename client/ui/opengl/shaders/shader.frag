#version 330 core

out vec4 clr;

in vec4 inclr;
in vec3 normal;
in vec3 frag_pos;

vec3 lightColor = vec3(1.0, 1.0, 1.0);
vec3 lightPos = vec3(100.0, 100.0, 100.0);
float ambientStrength = 0.1;
float specularStrength = 0.4;

uniform vec3 view_pos;

void main()
{
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightPos - frag_pos);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	vec3 viewDir = normalize(view_pos - frag_pos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

	vec3 ambient = ambientStrength * lightColor;
	clr = vec4((ambient + diffuse + specular) * vec3(inclr), 1.0);
}
