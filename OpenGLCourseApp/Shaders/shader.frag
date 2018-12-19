#version 330													
				

struct DirectionalLight {
	vec3 color;
	float ambientIntensity;
	vec3 direction;
	float diffuseIntensity;
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
		
//uniform variables are used to communicate with shaders from the outside
//this sampler is connected to a texture through the Texture object,
//which is bounded in the Texture::UseTexture() function, by using glActiveTexture(...)
//if we only have ONE texture in the frag shader, then GL_TEXTURE0 in the C++ code
//will automatically bind to theTexture
uniform sampler2D theTexture;
uniform DirectionalLight directionalLight;		
uniform Material material;		
		
uniform vec3 eyePosition;	
		
void main()														
{									
	vec4 ambientColor = vec4(directionalLight.color, 1.0f) * directionalLight.ambientIntensity;
	
	//diffuse lighting is affected by angle between normal of surface and vector towards light
	float diffuseFactor = max(dot(normalize(Normal), normalize(directionalLight.direction)), 0.0f);	
	vec4 diffuseColor = vec4(directionalLight.color, 1.0f) * directionalLight.diffuseIntensity * diffuseFactor;
	
	vec4 specularColor = vec4(0,0,0,0);
	
	//specular color only exists if there's diffuse color as well
	if(diffuseFactor > 0.0f){
		vec3 fragToEye = normalize(eyePosition - FragPos);
		vec3 reflectedVertex = normalize(reflect(directionalLight.direction, normalize(Normal)));
		
		//since fragToEye and reflectedVertex are normalize, their dot product = cos(angle between them), so a value between -1 and 1
		//specularFactor describes how "close" the vectors are, the close the specularFactor is to 1, the closer they are
		//if fragToEye and reflectedVertex are "close" then, material will shine there
		float specularFactor = dot(fragToEye, reflectedVertex);
		
		if(specularFactor > 0.0f){
			specularFactor = pow(specularFactor, material.shininess);
			specularColor = vec4(directionalLight.color * material.specularIntensity * specularFactor, 1.0f);
		}
	}
	
	color = texture(theTexture, TexCoord) * (ambientColor + diffuseColor + specularColor);		
}