#version 330 core

out vec4 clr;

flat in vec3 normal;
flat in vec4 inclr;
in vec3 frag_pos;
in vec4 frag_pos_light_space;

vec3 lightColor = vec3(1, 1, 1);
float ambientStrength = 0.1;

float dim = 0.5;

uniform vec3 view_pos;
uniform vec3 light_pos;

uniform sampler2D shadow_map;

float in_shade(vec3 lightDir)
{
	vec3 proj = frag_pos_light_space.xyz / frag_pos_light_space.w;

	proj = proj * 0.5 + 0.5;

	float closest = texture(shadow_map, proj.xy).r;
	float current = proj.z;


	float bias = max(0.002 * (1.0 - dot(normal, lightDir)),
		 	 0.001);

	return current - bias > closest ? 0.1 : 1.0;
}

void main()
{
	vec3 norm = normalize(normal);

	vec3 ambient = ambientStrength * lightColor;

        vec3 lightDir = normalize(light_pos - frag_pos);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	diff = max(dot(norm, vec3(-2.0, 4.0, -2.0)), 0.0) * 0.01;

	vec3 viewDir = normalize(view_pos - frag_pos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0), 0.5);
	vec3 specular = lightColor * spec;

	vec3 sunlight = in_shade(lightDir) * (diffuse + specular)
		* clamp(light_pos.y * 0.01, 0.0, 1.0) + diff;

	clr = vec4(vec3(ambient + sunlight) * inclr.xyz * dim, inclr.w);
	//clr = vec4(normal, 1.0);
}
