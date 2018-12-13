#version 330													
				
//in variables are passed in from vertex shader,
//by setting the value of out variables in the vertex shader main()				
in vec4 vCol;	
in vec2 TexCoord;
																																																													
out	vec4 colour;												
			
//uniform variables are used to communicate with shaders from the outside
//this sampler is connected to a texture through the Texture object,
//which is bounded in the Texture::UseTexture() function, by using glActiveTexture(...)
//if we only have ONE texture in the frag shader, then GL_TEXTURE0 in the C++ code
//will automatically bind to theTexture
uniform sampler2D theTexture;
			
void main()														
{																
	colour = texture(theTexture, TexCoord);							
}