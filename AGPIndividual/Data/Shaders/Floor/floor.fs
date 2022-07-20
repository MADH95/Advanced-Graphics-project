#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse;
    float shininess;
};

struct Light {
	//vec3 position;
	vec3 direction;
	
	float constant;
	float linear;
	float quadratic;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
} fs_in;
  
uniform vec3 viewPos;
uniform Material material;
uniform Light light;
uniform sampler2D shadowMap;

float CalcShadow(vec3 normal, vec3 lightDir, vec4 fragPosLightSpace);
vec3 CalcDirLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	//properties
	vec3 norm = normalize(fs_in.Normal);
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	
	vec3 result = vec3(0.0, 0.0, 0.0);
	
	result += CalcDirLight(light, norm, fs_in.FragPos, viewDir);
	
	FragColor = vec4(result, 1.0);
}

float CalcShadow(vec3 normal, vec3 lightDir, vec4 fragPosLightSpace)
{
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	
	float closestDepth = texture(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;
	
	float bias = max(0.012 * (1.0 - dot(normal, lightDir)), 0.005);
	
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;
	
	if(projCoords.z > 1.0)
		shadow = 0.0;
		
	return 1.0 - shadow;
}

vec3 CalcDirLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);
	
	//diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	//specular shading
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess);
	//attenuation
	//float distance = length(light.position - fragPos);
	//float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	//shadow
	float shadow = CalcShadow(normal, lightDir, fs_in.FragPosLightSpace);
	
	//combine results
	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse, fs_in.TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse, fs_in.TexCoords));
	vec3 specular = light.specular * spec * vec3(texture(material.texture_diffuse, fs_in.TexCoords));
	//ambient *= attenuation;
	//diffuse *= attenuation;
	//specular *= attenuation;
	
	return ambient + (shadow * (diffuse + specular));
}