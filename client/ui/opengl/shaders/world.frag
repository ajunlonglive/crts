#version 330 core

out vec4 clr;

flat in vec3 normal;
flat in vec4 inclr;
in vec3 frag_pos;
in vec4 frag_pos_light_space;

vec3 lightColor = vec3(1, 1, 1);
vec3 selColor = vec3(0.0, 0.0, 1.0);
float ambientStrength = 0.1;
float specularStrength = 0.04;

float dim = 0.5;

uniform vec3 view_pos;
uniform vec3 light_pos;

void main()
{
	vec3 norm = normalize(normal);

        vec3 lightDir = normalize(light_pos - frag_pos);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	vec3 viewDir = normalize(view_pos - frag_pos);

	vec3 halfwayDir = normalize(lightDir + viewDir);

	float spec = pow(max(dot(normal, halfwayDir), 0.0), 0.5);
	vec3 specular = lightColor * spec;

	vec3 ambient = ambientStrength * lightColor;

	clr = vec4(vec3(ambient + diffuse + specular) * inclr.xyz * dim, inclr.w);
	//clr = vec4(normal, 1.0);
}
