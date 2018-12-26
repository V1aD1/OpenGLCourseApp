#version 330													
				

struct Light
{
	vec3 color;
	float ambientIntensity;
	float diffuseIntensity;
};
				
struct DirectionalLight {
	Light base;
	vec3 direction;
};				
		
struct PointLight{
	Light base;
	vec3 position;
	float constant;
	float linear;
	float exponent;
}; 
		
struct Material{
	float specularIntensity;
	float shininess;
};
			
//in variables are passed in from vertex shader,
//by setting the value of out variables in the vertex shader main()				
in vec4 vCol;	
in vec2 TexCoord;
in vec3 Normal;		
in vec3 FragPos;		
				
out	vec4 color;												
		
//should be the same value as MAX_POINT_LIGHTS in C++ project
const int MAX_POINT_LIGHTS = 3;		

		
//uniform variables are used to communicate with shaders from the outside
//this sampler is connected to a texture through the Texture object,
//which is bounded in the Texture::UseTexture() function, by using glActiveTexture(...)
//if we only have ONE texture in the frag shader, then GL_TEXTURE0 in the C++ code
//will automatically bind to theTexture
uniform sampler2D theTexture;
uniform Material material;		
		
uniform vec3 eyePosition;	
		
		
uniform int pointLightCount;

uniform DirectionalLight directionalLight;		
uniform PointLight pointLights[MAX_POINT_LIGHTS];		
		
vec4 CalcLightByDirection(Light light, vec3 direction){
	vec4 ambientColor = vec4(light.color, 1.0f) * light.ambientIntensity;
	
	//diffuse lighting is affected by angle between normal of surface and vector towards light
	float diffuseFactor = max(dot(normalize(Normal), normalize(direction)), 0.0f);	
	vec4 diffuseColor = vec4(light.color, 1.0f) * light.diffuseIntensity * diffuseFactor;
	
	vec4 specularColor = vec4(0,0,0,0);
	
	//specular color only exists if there's diffuse color as well
	if(diffuseFactor > 0.0f){
		vec3 fragToEye = normalize(eyePosition - FragPos);
		vec3 reflectedVertex = normalize(reflect(direction, normalize(Normal)));
		
		//since fragToEye and reflectedVertex are normalize, their dot product = cos(angle between them), so a value between -1 and 1
		//specularFactor describes how "close" the vectors are, the close the specularFactor is to 1, the closer they are
		//if fragToEye and reflectedVertex are "close" then, material will shine there
		float specularFactor = dot(fragToEye, reflectedVertex);
		
		if(specularFactor > 0.0f){
			specularFactor = pow(specularFactor, material.shininess);
			specularColor = vec4(light.color * material.specularIntensity * specularFactor, 1.0f);
		}
	}
	
	return ambientColor + diffuseColor + specularColor;
}		

vec4 CalcDirectionalLight(){
	return CalcLightByDirection(directionalLight.base, directionalLight.direction);
}

vec4 CalcPointLights(){
	vec4 totalColor = vec4(0, 0, 0, 0);
	
	for(int i =0; i < pointLightCount; i++){
		vec3 direction = FragPos - pointLights[i].position;
		float distance = length(direction);
		direction = normalize(direction);
		
		vec4 color = CalcLightByDirection(pointLights[i].base, direction);
		
		//y= ax^2 + bx + c
		float attenuation = pointLights[i].exponent * distance *distance + 
							pointLights[i].linear * distance + 
							pointLights[i].constant;
		totalColor += (color / attenuation);
	}
		
	return totalColor;
}
		
void main()												
{									
	vec4 finalColor = CalcDirectionalLight();
	finalColor += CalcPointLights();
	color = texture(theTexture, TexCoord) * finalColor;		
}