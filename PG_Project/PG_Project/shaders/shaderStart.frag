#version 410 core

#define NUMBER_OF_LIGHTS 3

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//lighting
uniform	vec3 lightDir[NUMBER_OF_LIGHTS];
uniform	vec3 lightColor[NUMBER_OF_LIGHTS];

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

float shadow = 1.0f;

float constant = 1.0f, linear = 0.0045f, quadratic = 0.0075f;
float dist, att;
vec3 lightDirN, reflection;
float specCoeff;

void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	ambient = vec3(0.0f);
	diffuse = vec3(0.0f);
	specular = vec3(0.0f);

	for(int i = 0; i < NUMBER_OF_LIGHTS; i++)
	{
		//compute light direction
		lightDirN = normalize(lightDir[i]);

		dist = length(lightDir[i]);
		att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));

		//compute view direction 
		vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
		//compute ambient light
		ambient += att * ambientStrength * lightColor[i];
	
		//compute diffuse light
		diffuse += att * max(dot(normalEye, lightDirN), 0.0f) * lightColor[i];
	
		//compute specular light
		reflection = reflect(-lightDirN, normalEye);
		specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
		specular += att * specularStrength * specCoeff * lightColor[i];
	}
}


float computeShadow()
{
	// perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// Transform to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	
	// Get closest depth value from light's perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

	// Get depth of current fragment from light's perspective
	float currentDepth = normalizedCoords.z;

	// Check whether current frag pos is in shadow
	float bias = max(0.05f * (1.0f - dot(fNormal, lightDir[0])), 0.005f);
	//float bias = 0.005f;
	float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
	
	if (normalizedCoords.z > 1.0f)
		return 0.0f;

	return shadow;
}

float computeFog()
{
	 float fogDensity = 0.05f;
	 float fragmentDistance = length(fPosEye);
	 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
 
	 return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
	computeLightComponents();
	
	vec3 baseColor = vec3(0.9f, 0.35f, 0.0f);//orange
	

	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

	vec4 colorFromTexture = texture(diffuseTexture, fTexCoords);
//	if(colorFromTexture.a < 0.1)
//		discard;

	//modulate with shadow
	shadow = computeShadow();
	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
	//vec3 color = min((ambient + diffuse) + specular, 1.0f);
    float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	//fColor = mix(fogColor, color, fogFactor);
	fColor = fogColor * (1 - fogFactor) + vec4(color, 1.0f) * fogFactor;

    //fColor = fogColor;//
}